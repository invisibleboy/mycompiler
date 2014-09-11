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
/*! \file
 * \brief Routines to write Pcode to a file.
 *
 * \author Robert Kidd, Wen-mei Hwu
 *
 * This file contains routines to write Pcode to a file.
 *
 * For each rule in the grammar (parse.y), there should be a function
 * in this file to write that structure to disk.  The naming
 * convention is P_write_<rule name>.  Each function should take a
 * FILE * (the output file) as the first argument and an int
 * indicating the indent level as the third.  The appropriate data
 * type for the rule is passed as the second argument.  Functions that
 * write non-terminal symbols take a pointer to an int as the fourth
 * argument.
 *
 * Layout works as follows.  It is the caller's responsibility to
 * handle indenting.  Each function can assume the cursor is
 * positioned properly when it is called.  Likewise, functions should
 * not insert spaces or newlines after their output (immediately
 * before exiting).
 *
 * Before calling another function, a function should insert any
 * necessary newlines and spaces (see P_write_newline_indent), and
 * should pass the callee's indent level (typically indent + 1) to the
 * callee.
 *
 * All functions return the number of bytes written.  Functions that
 * write non-terminal symbols return the number of newlines written in
 * the int pointed to by the fourth argument.  If a function writes a
 * non-terminal without writing any newlines, it should return 0 in
 * the fourth argument.
 */
/*****************************************************************************/

#include <config.h>
#include <stdio.h>
#include <library/i_list.h>
#include "pcode.h"
#include "parse.h"
#include "struct.h"
#include "query.h"
#include "write.h"
#include "util.h"
#include "extension.h"

#undef PRETTY

/* These macros were an attempt to help write pretty output without making
 * code here too nasty. */
/*! \brief Wraps a write function to help format the output.
 *
 * \param b
 *  accumulates the number of bytes written.
 * \param l
 *  accumulates the number of newlines written.
 * \param m
 *  used to return the number of newlines to the caller.  Corresponds
 *  to the \a lines argument to the write function.
 * \param f
 *  the write function to call.  The write function should not write
 *  any newlines (does not take a \a lines argument).
 * \param o
 *  the file to write.  Corresponds to the \a out argument to the write
 *  function.
 * \param d
 *  the data to pass to the write function.
 * \param i
 *  the indent level.  Corresponds to the \a indent argument to the write
 *  function.
 *
 * Wraps a write function to help format the output.  If \a o is null and
 * the accumulated bytes written is greater than the line length, this
 * macro exits the calling function immediately.  A function can therefore
 * quickly determine if calling a write function will require the line to be
 * split.
 */
#define write_formatted(b, l, m, f, o, d, i) \
          (b) += (f)((o), (d), (i)); \
          if ((o) == NULL && (b) > LINE_LENGTH) \
          { \
            if (m) *(m) = (l); \
            return ((b)); \
	  }

/*! \brief Wraps a write function to help format the output.
 *
 * \param b
 *  accumulates the number of bytes written.
 * \param l
 *  accumulates the number of newlines written.
 * \param m
 *  used to return the number of newlines to the caller.  Corresponds
 *  to the \a lines argument to the write function.
 * \param f
 *  the write function to call.  The write function should write newlines
 *  (takes a \a lines argument).
 * \param o
 *  the file to write.  Corresponds to the \a out argument to the write
 *  function.
 * \param d
 *  the data to pass to the write function.
 * \param i
 *  the indent level.  Corresponds to the \a indent argument to the write
 *  function.
 *
 * Wraps a write function to help format the output.  If \a o is null and
 * the accumulated bytes written is greater than the line length, this
 * macro exits the calling function immediately.  A function can therefore
 * quickly determine if calling a write function will require the line to be
 * split.
 */
#define write_formatted_lines(b, l, m, f, o, d, i) \
          (b) += (f)((o), (d), (i), (m)); \
          if (m) (l) += *(m); \
          if ((o) == NULL && (b) > LINE_LENGTH) \
          { \
            if (m) *(m) = (l); \
            return ((b)); \
          } 

/*! \brief Wraps the P_write_newline_indent function to help format the output.
 *
 * \param b
 *  accumulates the number of bytes written.
 * \param l
 *  accumulates the number of newlines written.
 * \param m
 *  used to return the number of newlines to the caller.  Corresponds
 *  to the \a  lines argument to the write function.
 * \param o
 *  the file to write.  Corresponds to the \a out argument to the write
 *  function.
 * \param i
 *  the indent level.  Corresponds to the \a indent argument to the write
 *  function.
 *
 * Wraps the P_writ_newline_indent function to help format the output.  If
 * \a o is null, and the accumulated bytes written is greater than the line
 * length, this macro exits the calling function immediately.  A function
 * can therefore quickly determine if calling a write function will require
 * the line to be split.
 */
#define write_formatted_nli(b, l, m, o, i) \
          (b) += P_write_newline_indent ((o), (i)); \
          (l)++; \
          if ((o) == NULL && (b) > LINE_LENGTH) \
          { \
            *(m) = (l); \
            return ((b)); \
          }

/*! An array to map opcode values to tokens. */
static const long op_to_token[OP_last] = 
{
  0,              /*!< No opcode 0 */
  VAR,            /*!< OP_var */
  ENUM,           /*!< OP_enum */
  INT,            /*!< OP_int */
  REAL,           /*!< OP_real */
  ERROR,          /*!< OP_error */
  CHAR,           /*!< OP_char */
  STRING,         /*!< OP_string */
  DOT,            /*!< OP_dot */
  ARROW,          /*!< OP_arrow */
  CAST,           /*!< OP_cast */
  EXPRSIZE,       /*!< OP_expr_size */
  TYPESIZE,       /*!< OP_type_size */
  QUEST,          /*!< OP_quest */
  DISJ,           /*!< OP_disj */
  CONJ,           /*!< OP_conj */
  COMPEXPR,       /*!< OP_compexpr */
  ASSIGN,         /*!< OP_assign */
  OR,             /*!< OP_or */
  XOR,            /*!< OP_xor */
  AND,            /*!< OP_and */
  EQ,             /*!< OP_eq */
  NE,             /*!< OP_ne */
  LT,             /*!< OP_lt */
  LE,             /*!< OP_le */
  GE,             /*!< OP_ge */
  GT,             /*!< OP_gt */
  RSHFT,          /*!< OP_rshft */
  LSHFT,          /*!< OP_lshft */
  ADD,            /*!< OP_add */
  SUB,            /*!< OP_sub */
  MUL,            /*!< OP_mul */
  DIV,            /*!< OP_div */
  MOD,            /*!< OP_mod */
  NEG,            /*!< OP_neg */
  NOT,            /*!< OP_not */
  INV,            /*!< OP_inv */
  0,              /*!< No opcode 37 */
  PREINC,         /*!< OP_preinc */
  PREDEC,         /*!< OP_predec */
  POSTINC,        /*!< OP_postinc */
  POSTDEC,        /*!< OP_postdec */
  A_ADD,          /*!< OP_Aadd */
  A_SUB,          /*!< OP_Asub */
  A_MUL,          /*!< OP_Amul */
  A_DIV,          /*!< OP_Adiv */
  A_MOD,          /*!< OP_Amod */
  A_RSHFT,        /*!< OP_Arshft */
  A_LSHFT,        /*!< OP_Alshft */
  A_AND,          /*!< OP_Aand */
  A_OR,           /*!< OP_Aor */
  A_XOR,          /*!< OP_Axor */
  INDR,           /*!< OP_indr */
  ADDR,           /*!< OP_addr */
  INDEX,          /*!< OP_index */
  CALL,           /*!< OP_call */
  FLOAT,          /*!< OP_float */
  DOUBLE,         /*!< OP_double */
  KW_NULL,        /*!< OP_null */
  SYNC,           /*!< OP_sync */
  STMTEXPR,       /*!< OP_stmt_expr */
  ASMOPRD,        /*!< OP_asm_oprd */
  PHI,            /*!< OP_phi */
};

/*! \brief Writes Pcode to disk.
 *
 * \param out
 *  the file to write.
 * \param dcl
 *  the ::Dcl to write.
 *
 * \return
 *  The number of bytes written.
 *
 * This function should be called to write Pcode to disk.
 *
 * \sa P_write_dcllist()
 */
int
P_write (FILE *out, Dcl dcl)
{
  int bytes = 0;
  int local_lines = 0;

  bytes += P_write_dcl (out, dcl, 0, &local_lines);

  bytes += P_write_newline (out, 1);

  return (bytes);
}

/*! \brief Writes Pcode to disk.
 *
 * \param out
 *  the file to write.
 * \param dcllist
 *  the ::DclList to write.
 *
 * \return
 *  The number of bytes written.
 *
 * This function can be called to write several Pcode Dcls in a DclList to
 * disk.
 *
 * \sa P_write()
 */
int
P_write_dcllist (FILE *out, DclList dcllist)
{
  int bytes = 0;
  int local_lines = 0;
  Dcl dcl;

  List_start (dcllist);
  dcl = (Dcl)List_next (dcllist);

  while (dcl)
    {
      bytes += P_write_dcl (out, dcl, 0, &local_lines);

      if ((dcl = (Dcl)List_next (dcllist)))
	bytes += P_write_newline (out, 1);
    }

  return (bytes);
}

/* Functions to write non-terminal symbols */

/*! \brief Writes the dcl rule to disk.
 *
 * \param out
 *  the file to write.
 * \param dcl
 *  the ::Dcl to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the dcl rule to disk.
 */
int
P_write_dcl (FILE *out, Dcl dcl, int indent, int *lines)
{
  int bytes = 0;
  int local_lines = 0;

  switch (P_GetDclType (dcl))
    {
    case TT_FUNC:
      write_formatted (bytes, local_lines, lines, 
		       P_write_comment, out, "FuncDcl", indent);
      write_formatted_nli (bytes, local_lines, lines, out, indent);
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_func_dcl, out, P_GetDclFuncDcl (dcl),
			     indent);
      break;
    case TT_VAR:
      write_formatted (bytes, local_lines, lines,
		       P_write_comment, out, "Gvar", indent);
      write_formatted_nli (bytes, local_lines, lines, out, indent);
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_var_dcl, out, P_GetDclVarDcl (dcl),
			     indent);
      break;
    case TT_ASM:
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_asm_dcl, out, P_GetDclAsmDcl (dcl),
			     indent);
      break;
    case TT_INCLUDE:
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_include, out, P_GetDclInclude (dcl),
			     indent);
      break;
    case TT_TYPE:
    case TT_STRUCT:
    case TT_UNION:
    case TT_ENUM:
      write_formatted (bytes, local_lines, lines,
		       P_write_comment, out, "TypeDef", indent);
      write_formatted_nli (bytes, local_lines, lines, out, indent);
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_type_dcl, out, dcl, indent);
      break;
    case TT_SYMBOLTABLE:
      write_formatted (bytes, local_lines, lines,
		       P_write_comment, out, "Symbol Table", indent);
      write_formatted_nli (bytes, local_lines, lines, out, indent);
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_symbol_table, out,
			     P_GetDclSymbolTable (dcl), indent);
      break;
    case TT_IPSYMTABENT:
      write_formatted (bytes, local_lines, lines,
		       P_write_comment, out,
		       "Interprocedural Symbol Table Entry", indent);
      write_formatted_nli (bytes, local_lines, lines, out, indent);
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_ip_sym_tab_ent, out,
			     P_GetDclIPSymTabEnt (dcl), indent);
      break;
    case TT_SYMTABENTRY:
      write_formatted (bytes, local_lines, lines,
		       P_write_comment, out, "Symbol Table Entyr", indent);
      write_formatted_nli (bytes, local_lines, lines, out, indent);
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_symbol_table_entry, out,
			     P_GetDclSymTabEntry (dcl), indent);
      break;
    default:
      P_punt ("write.c:P_write_dcl:%d Unknown Dcl type %d", __LINE__,
	      P_GetDclType (dcl));
    }

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the alignment rule to disk.
 *
 * \param out
 *  the file to write.
 * \param alignment
 *  the alignment to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the alignment rule to disk.
 */
int
P_write_alignment (FILE *out, int alignment, int indent, int *lines)
{
  int bytes = 0;
  int local_lines = 0;

  if (alignment > 0)
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, ALIGNMENT, indent);
      write_formatted (bytes, local_lines, lines,
		       P_write_I_LIT, out, alignment, indent);
    }

  if (lines) if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the asm_clobbers rule to disk.
 *
 * \param out
 *  the file to write.
 * \param asm_clobbers
 *  the asm_clobbers ::Expr to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the asm_clobbers rule to disk.
 */
int
P_write_asm_clobbers (FILE *out, Expr asm_clobbers, int indent, int *lines)
{
  int bytes = 0;
  int local_lines = 0;

  if (asm_clobbers)
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, CLOBBERS, indent);
      write_formatted_nli (bytes, local_lines, lines, out, indent);
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_expr_list_container, out, asm_clobbers,
			     indent);
    }

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the asm_dcl rule to disk.
 *
 * \param out
 *  the file to write.
 * \param asm_dcl
 *  the ::AsmDcl to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the asm_dcl rule to disk.
 */
int
P_write_asm_dcl (FILE *out, AsmDcl asm_dcl, int indent, int *lines)
{
  AsmStmt asm_stmt = P_NewAsmStmt ();
  Position position = P_GetAsmDclPosition (asm_dcl);
  Pragma pragma;
  int bytes = 0;
  int local_lines = 0;

  /* If any write handlers are defined, save the ext field to the pragma
   * list. */
  if (Handlers[ES_ASM])
    P_ExtWrite (ES_ASM, (void *)asm_dcl);

  pragma = P_GetAsmDclPragma (asm_dcl);

  /* Build an AsmStmt to pass to P_write_asm_stmt. */
  P_SetAsmStmtIsVolatile (asm_stmt, P_GetAsmDclIsVolatile (asm_dcl));
  P_SetAsmStmtAsmClobbers (asm_stmt, P_GetAsmDclAsmClobbers (asm_dcl));
  P_SetAsmStmtAsmString (asm_stmt, P_GetAsmDclAsmString (asm_dcl));
  P_SetAsmStmtAsmOperands (asm_stmt, P_GetAsmDclAsmOperands (asm_dcl));

  write_formatted (bytes, local_lines, lines, P_write_token, out, '(', indent);
  write_formatted_nli (bytes, local_lines, lines, out, indent + 1);
  write_formatted_lines (bytes, local_lines, lines,
			 P_write_asm_stmt, out, asm_stmt, indent + 1)

  /* The fields of the AsmStmt are stored in the AsmDcl, so we just
   * need to clear the pointers and free the AsmStmt using P_FreeAsmStmt. */
  P_SetAsmStmtAsmClobbers (asm_stmt, NULL);
  P_SetAsmStmtAsmString (asm_stmt, NULL);
  P_SetAsmStmtAsmOperands (asm_stmt, NULL);
  asm_stmt = P_FreeAsmStmt (asm_stmt);

  write_formatted_nli (bytes, local_lines, lines, out, indent + 1);
  write_formatted_lines (bytes, local_lines, lines,
			 P_write_key, out, P_GetAsmDclKey (asm_dcl),
			 indent + 1);

  if (pragma)
    {
      write_formatted_nli (bytes, local_lines, lines, out, indent + 1);
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_pragma_list, out, pragma, indent + 1);
    }

  if (position && P_ValidPosition (position))
    {
      write_formatted_nli (bytes, local_lines, lines, out, indent + 1);
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_position, out, position, indent + 1);
    }

  write_formatted_nli (bytes, local_lines, lines, out, indent);
  write_formatted (bytes, local_lines, lines, P_write_token, out, ')', indent);

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the asm_operands rule to disk.
 *
 * \param out
 *  the file to write.
 * \param asm_operands
 *  the asm_operands ::Expr to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the asm_operands rule to disk.
 */
int
P_write_asm_operands (FILE *out, Expr asm_operands, int indent, int *lines)
{
  int bytes = 0;
  int local_lines = 0;

  if (asm_operands)
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, OPERANDS, indent);
      write_formatted_nli (bytes, local_lines, lines, out, indent);
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_expr_list_container, out, asm_operands,
			     indent);
    }

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the asm_stmt rule to disk.
 *
 * \param out
 *  the file to write.
 * \param asm_stmt
 *  the ::AsmStmt to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the asm_stmt rule to disk.
 */
int
P_write_asm_stmt (FILE *out, AsmStmt asm_stmt, int indent, int *lines)
{
  Expr operands = P_GetAsmStmtAsmOperands (asm_stmt);
  Expr clobbers = P_GetAsmStmtAsmClobbers (asm_stmt);
  int bytes = 0;
  int local_lines = 0;
	
  write_formatted (bytes, local_lines, lines,
		   P_write_token, out, ASM, indent);

  if (P_GetAsmStmtIsVolatile (asm_stmt))
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, VOLATILE, indent);
    }
  
  write_formatted_lines (bytes, local_lines, lines,
			 P_write_expr, out, P_GetAsmStmtAsmString (asm_stmt),
			 indent);

  if (operands)
    {
      write_formatted_nli (bytes, local_lines, lines, out, indent + 1);
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_asm_operands, out, operands, indent + 1);
    }

  if (clobbers)
    {
      write_formatted_nli (bytes, local_lines, lines, out, indent + 1);
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_asm_clobbers, out, clobbers, indent + 1);
    }

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the basic_type rule to disk.
 *
 * \param out
 *  the file to write.
 * \param basic_type
 *  the ::_BasicType to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the basic_type rule to disk.
 */
int
P_write_basic_type (FILE *out, _BasicType basic_type, int indent, int *lines)
{
  int bytes = 0;
  int local_lines = 0;

  if (basic_type & BT_UNSIGNED)
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, UNSIGNED, indent);
    }
  if (basic_type & BT_VOID)
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, VOID, indent);
    }
  if (basic_type & BT_CHAR)
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, CHAR, indent);
    }
  if (basic_type & BT_SHORT)
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, SHORT, indent);
    }
  if (basic_type & BT_INT)
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, INT, indent);
    }
  if (basic_type & BT_LONG)
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, LONG, indent);
    }
  if (basic_type & BT_LONGLONG)
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, LONGLONG, indent);
    }
  if (basic_type & BT_FLOAT)
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, FLOAT, indent);
    }
  if (basic_type & BT_DOUBLE)
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, DOUBLE, indent);
    }
  if (basic_type & BT_LONGDOUBLE)
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, LONGDOUBLE, indent);
    }
  if (basic_type & BT_VARARG)
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, VARARG, indent);
    }
  if (basic_type & BT_BIT_FIELD)
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, BITFIELD, indent);
    }

  if (lines) *lines = 0;
  return (bytes);
}

/*! \brief Writes the basic_type_list rule to disk.
 *
 * \param out
 *  the file to write.
 * \param basic_type_list
 *  the basic_type_list _BasicType to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the basic_type_list rule to disk.
 */
int
P_write_basic_type_list (FILE *out, _BasicType basic_type_list, int indent,
			 int *lines)
{
  int bytes = 0;
  int local_lines = 0;

  write_formatted_lines (bytes, local_lines, lines,
			 P_write_basic_type, out, basic_type_list, indent);

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the comp_stmt rule to disk.
 *
 * \param out
 *  the file to write.
 * \param comp_stmt
 *  the comp_stmt ::Stmt to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the comp_stmt rule to disk.
 */
int
P_write_comp_stmt (FILE *out, Stmt comp_stmt, int indent, int *lines)
{
  Compound c = P_GetStmtCompound (comp_stmt);
  int bytes = 0;
  int local_lines = 0;

  write_formatted (bytes, local_lines, lines,
		   P_write_token, out, COMPSTMT, indent);
  write_formatted (bytes, local_lines, lines,
		   P_write_I_LIT, out, P_GetCompoundUniqueVarID (c), indent);
  write_formatted_nli (bytes, local_lines, lines, out, indent + 1);
  write_formatted_lines (bytes, local_lines, lines,
			 P_write_type_list_container, out,
			 P_GetCompoundTypeList (c), indent + 1);
  write_formatted_nli (bytes, local_lines, lines, out, indent + 1);
  write_formatted_lines (bytes, local_lines, lines,
			 P_write_var_dcl_list_container, out,
			 P_GetCompoundVarList (c), indent + 1);
  write_formatted_nli (bytes, local_lines, lines, out, indent + 1);
  write_formatted_lines (bytes, local_lines, lines,
			 P_write_stmt_list_container, out,
			 P_GetCompoundStmtList (c), indent + 1);

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the comp_stmt_container rule to disk.
 *
 * \param out
 *  the file to write.
 * \param comp_stmt_container
 *  the comp_stmt_container ::Stmt to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the comp_stmt_container rule to disk.
 */
int
P_write_comp_stmt_container (FILE *out, Stmt comp_stmt_container, int indent,
			     int *lines)
{
  int bytes = 0;
  int local_lines = 0;

  write_formatted (bytes, local_lines, lines, P_write_token, out, '(', indent);
  write_formatted_lines (bytes, local_lines, lines,
			 P_write_comp_stmt, out, comp_stmt_container, indent);
  write_formatted_nli (bytes, local_lines, lines, out, indent);
  write_formatted (bytes, local_lines, lines, P_write_token, out, ')', indent);

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the entry_type rule to disk.
 *
 * \param out
 *  the file to write.
 * \param entry_type
 *  the ::_EntryType to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the entry_type rule to disk.
 */
int
P_write_entry_type (FILE *out, _EntryType entry_type, int indent, int *lines)
{
  int bytes = 0;
  int local_lines = 0;

  switch (entry_type)
    {
    case ET_FUNC:
      {
	write_formatted (bytes, local_lines, lines,
			 P_write_token, out, FUNC, indent);
      }
      break;
    case ET_TYPE_LOCAL:
      {
	write_formatted (bytes, local_lines, lines,
			 P_write_token, out, LTYPE, indent);
      }
      break;
    case ET_TYPE_GLOBAL:
      {
	write_formatted (bytes, local_lines, lines,
			 P_write_token, out, TYPE, indent);
      }
      break;
    case ET_VAR_LOCAL:
      {
	write_formatted (bytes, local_lines, lines,
			 P_write_token, out, VAR, indent);
      }
      break;
    case ET_VAR_GLOBAL:
      {
	write_formatted (bytes, local_lines, lines,
			 P_write_token, out, GVAR, indent);
      }
      break;
    case ET_STRUCT:
      {
	write_formatted (bytes, local_lines, lines,
			 P_write_token, out, STRUCT, indent);
      }
      break;
    case ET_UNION:
      {
	write_formatted (bytes, local_lines, lines,
			 P_write_token, out, UNION, indent);
      }
      break;
    case ET_ENUM:
      {
	write_formatted (bytes, local_lines, lines,
			 P_write_token, out, ENUM, indent);
      }
      break;
    case ET_ASM:
      {
	write_formatted (bytes, local_lines, lines,
			 P_write_token, out, ASM, indent);
      }
      break;
    case ET_STMT:
      {
	write_formatted (bytes, local_lines, lines,
			 P_write_token, out, STMT, indent);
      }
      break;
    case ET_EXPR:
      {
	write_formatted (bytes, local_lines, lines,
			 P_write_token, out, EXPR, indent);
      }
      break;
    case ET_FIELD:
      {
	write_formatted (bytes, local_lines, lines,
			 P_write_token, out, FIELD, indent);
      }
      break;
    case ET_ENUMFIELD:
      {
	write_formatted (bytes, local_lines, lines,
			 P_write_token, out, ENUMFIELD, indent);
      }
      break;
    case ET_LABEL:
      {
	write_formatted (bytes, local_lines, lines,
			 P_write_token, out, LABEL, indent);
      }
      break;
    case ET_SCOPE:
      {
	write_formatted (bytes, local_lines, lines,
			 P_write_token, out, SCOPE, indent);
      }
      break;
    default:
      P_punt ("write.c:P_write_entry_type:%d Unknown entry type %d", __LINE__,
	      entry_type);
    }

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the enum_field rule to disk.
 *
 * \param out
 *  the file to write.
 * \param enum_field
 *  the ::EnumField to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the enum_field rule to disk.
 */
int
P_write_enum_field (FILE *out, EnumField enum_field, int indent, int *lines)
{
  Expr e = P_GetEnumFieldValue (enum_field);
  Identifier identifier = P_GetEnumFieldIdentifier (enum_field);
  int bytes = 0;
  int local_lines = 0;

  write_formatted (bytes, local_lines, lines, P_write_token, out, '(', indent);
  write_formatted_lines (bytes, local_lines, lines,
			 P_write_identifier, out, identifier, indent);

  if (e != NULL)
    {
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_expr, out, e, indent);
    }
  write_formatted (bytes, local_lines, lines, P_write_token, out, ')', indent);

  identifier = P_RemoveIdentifier (identifier);

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the enum_field_list rule to disk.
 *
 * \param out
 *  the file to write.
 * \param enum_field_list
 *  the enum_field_list ::EnumField to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the enum_field_list rule to disk.
 */
int
P_write_enum_field_list (FILE *out, EnumField enum_field_list, int indent,
			 int *lines)
{
  int bytes = 0;
  int local_lines = 0;

  while (enum_field_list)
    {
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_enum_field, out, enum_field_list, indent);

      if ((enum_field_list = P_GetEnumFieldNext (enum_field_list)))
	{
	  write_formatted_nli (bytes, local_lines, lines, out, indent);
	}
    }

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the expr rule to disk.
 *
 * \param out
 *  the file to write.
 * \param expr
 *  the ::Expr to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the expr rule to disk.
 */
int
P_write_expr (FILE *out, Expr expr, int indent, int *lines)
{
  ProfEXPR profexpr = P_GetExprProfile (expr);
  Pragma pragma;
  double profexpr_count;
  int bytes = 0;
  int local_lines = 0;

  /* If any write handlers are defined, save the ext field to the pragma
   * list. */
  if (Handlers[ES_EXPR] && expr && P_GetExprParentStmt (expr))
    P_ExtWrite (ES_EXPR, (void *)expr);

  pragma = P_GetExprPragma (expr);

  write_formatted (bytes, local_lines, lines, P_write_token, out, '(', indent);
  write_formatted (bytes, local_lines, lines,
		   P_write_I_LIT, out, P_GetExprID (expr), indent);
  write_formatted_lines (bytes, local_lines, lines,
			 P_write_expr_core, out, expr, indent);

  if (pragma)
    {
      write_formatted_nli (bytes, local_lines, lines, out, indent);
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_pragma_list, out, pragma, indent);
    }

  if (profexpr && (profexpr_count = P_GetProfEXPRCount (profexpr)) != 0.0)
    {
      write_formatted_nli (bytes, local_lines, lines, out, indent);
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_profile, out, profexpr_count, indent);
    }

  if (local_lines > 0)
    write_formatted_nli (bytes, local_lines, lines, out, indent);

  write_formatted (bytes, local_lines, lines, P_write_token, out, ')', indent);

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the expr_container rule to disk.
 *
 * \param out
 *  the file to write.
 * \param expr_container
 *  the expr_container ::Expr to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the expr_container rule to disk.
 */
int
P_write_expr_container (FILE *out, Expr expr_container, int indent, int *lines)
{
  int bytes = 0;
  int local_lines = 0;

  write_formatted (bytes, local_lines, lines, P_write_token, out, '(', indent);

  if (expr_container)
    {
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_expr, out, expr_container, indent);
    }

  write_formatted (bytes, local_lines, lines, P_write_token, out, ')', indent);

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the expr_core rule to disk.
 *
 * \param out
 *  the file to write.
 * \param expr_core
 *  the expr_core ::Expr to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the expr_core rule to disk.
 */
int
P_write_expr_core (FILE *out, Expr expr_core, int indent, int *lines)
{
  _Opcode opcode = P_GetExprOpcode (expr_core);
  Expr operand0 = 0, operand1 = 0, operand2 = 0;
  int bytes = 0;
  int local_lines = 0;
#if PRETTY
  int head_length = P_write_token (NULL, '(', indent);
  int tail_length = P_write_token (NULL, ')', indent);
#else
  int head_length = LINE_LENGTH;
  int tail_length = LINE_LENGTH;
#endif

  operand0 = P_GetExprOperands (expr_core);
  if (operand0)
    {
      operand1 = P_GetExprSibling (operand0);

      if (operand1)
	operand2 = P_GetExprSibling (operand1);
    }

  switch (opcode)
    {
    case OP_var:
      {
	Identifier identifier = P_GetExprVarIdentifier (expr_core);

	write_formatted (bytes, local_lines, lines,
			 P_write_token, out, op_to_token[opcode], indent);
	write_formatted_lines (bytes, local_lines, lines,
			       P_write_identifier, out, identifier, indent);

	identifier = P_RemoveIdentifier (identifier);
      }
      break;

    case OP_int:
      if (P_GetExprFlags (expr_core) & EF_UNSIGNED)
	{
	  write_formatted (bytes, local_lines, lines,
			   P_write_token, out, UNSIGNED, indent);
	}
      else
	{
	  write_formatted (bytes, local_lines, lines,
			   P_write_token, out, SIGNED, indent);
	}
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, op_to_token[opcode], indent);
      if (P_GetExprFlags (expr_core) & EF_UNSIGNED)
	{
	  write_formatted (bytes, local_lines, lines,
			   P_write_I_LIT, out, P_GetExprUScalar (expr_core),
			   indent);
	}
      else
	{
	  write_formatted (bytes, local_lines, lines,
			   P_write_I_LIT, out, P_GetExprScalar (expr_core),
			   indent);
	}
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_key, out, P_GetExprType (expr_core),
			     indent);
      break;

    case OP_real:
    case OP_float:
    case OP_double:
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, op_to_token[opcode], indent);
      write_formatted (bytes, local_lines, lines,
		       P_write_F_LIT, out, P_GetExprReal (expr_core), indent);
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_key, out, P_GetExprType (expr_core),
			     indent);
      break;

    case OP_char:
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, op_to_token[opcode], indent);
      write_formatted (bytes, local_lines, lines,
		       P_write_C_LIT, out, *P_GetExprString (expr_core),
		       indent);
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_key, out, P_GetExprType (expr_core),
			     indent);
      break;

    case OP_string:
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, op_to_token[opcode], indent);
      write_formatted (bytes, local_lines, lines,
		       P_write_ST_LIT, out, P_GetExprString (expr_core),
		       indent);
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_key, out, P_GetExprType (expr_core),
			     indent);
      break;

    case OP_dot:
    case OP_arrow:
      {
	Identifier identifier = P_GetExprVarIdentifier (expr_core);
	int operand0_bytes = 0, operand0_lines = 0;
	int split_before_identifier = 0;
	int identifier_indent, token_length;

	token_length = P_write_token (out, op_to_token[opcode], indent);
	bytes += token_length;
	/* Account for the token written by P_write_expr. */
	token_length += head_length;
	
#if PRETTY
	operand0_bytes = P_write_expr (NULL, operand0, indent + 1,
				       &operand0_lines);
#else
	operand0_bytes = LINE_LENGTH;
#endif

	identifier_indent = indent + 1;
	if ((head_length + indent + bytes + operand0_bytes + \
	     tail_length >= LINE_LENGTH) || (operand0_lines > 0))
	  {
	    split_before_identifier = 1;

	    write_formatted_nli (bytes, local_lines, lines, out, indent + 1);
	  }

	write_formatted_lines (bytes, local_lines, lines,
			       P_write_expr, out, operand0, indent + 1);

	if (!split_before_identifier)
	  {
	    int identifier_bytes, identifier_lines = 0;

	    /* See if the identifier can fit on the same line. */
#if PRETTY
	    identifier_bytes = P_write_identifier (NULL, identifier,
						   indent + 1,
						   &identifier_lines);
#else
	    identifier_bytes = LINE_LENGTH;
#endif
	    
	    if ((head_length + indent + bytes + identifier_bytes + \
		 tail_length >= LINE_LENGTH) || (identifier_lines > 0))
	      {
		split_before_identifier = 1;
		
		/* See if the identifier can line up with the opcode token. */
#if PRETTY
		identifier_bytes = P_write_identifier (NULL, identifier,
						       indent + token_length,
						       &identifier_lines);
#else
                identifier_bytes = LINE_LENGTH;
#endif

		if (head_length + indent + identifier_bytes + \
		    tail_length < LINE_LENGTH)
		  identifier_indent = indent + token_length;
	      }
	  }
		  
	if (split_before_identifier)
	  {
	    write_formatted_nli (bytes, local_lines, lines,
				 out, identifier_indent);
	  }
	    
	write_formatted_lines (bytes, local_lines, lines,
			       P_write_identifier, out, identifier,
			       identifier_indent);

	identifier = P_RemoveIdentifier (identifier);
      }
      break;

    case OP_cast:
      {
	Key key = P_GetExprType (expr_core);
	int key_bytes, key_lines = 0;
	int split_before_operand0 = 0;
	int operand0_indent, token_length;

	token_length = P_write_token (out, op_to_token[opcode], indent);
	bytes += token_length;
	/* Account for the token written by P_write_expr. */
	token_length += head_length;

#if PRETTY
	key_bytes = P_write_key (NULL, key, indent + 1, &key_lines);
#else
	key_bytes = LINE_LENGTH;
#endif

	operand0_indent = indent + 1;
	if ((head_length + indent + bytes + key_bytes + \
	     tail_length >= LINE_LENGTH) || (key_lines > 0))
	  {
	    split_before_operand0 = 1;

	    write_formatted_nli (bytes, local_lines, lines, out, indent + 1);
	  }

	write_formatted_lines (bytes, local_lines, lines,
			       P_write_key, out, key, indent + 1);

	if (!split_before_operand0)
	  {
	    int operand0_bytes, operand0_lines = 0;

	    /* See if operand0 can fit on the same line. */
#if PRETTY
	    operand0_bytes = P_write_expr (NULL, operand0, indent + 1,
					   &operand0_lines);
#else
	    operand0_bytes = LINE_LENGTH;
#endif

	    if ((head_length + indent + bytes + operand0_bytes + \
		 tail_length >= LINE_LENGTH) || (operand0_lines > 0))
	      {
		split_before_operand0 = 1;

		/* See if operand0 can line up with the opcode token. */
#if PRETTY
		operand0_bytes = P_write_expr (NULL, operand0,
					       indent + token_length,
					       &operand0_lines);
#else
		operand0_bytes = LINE_LENGTH;
#endif

		if (head_length + indent + operand0_bytes + \
		    tail_length < LINE_LENGTH)
		  operand0_indent = indent + token_length;
	      }
	  }

	if (split_before_operand0)
	  {
	    write_formatted_nli (bytes, local_lines, lines,
				 out, operand0_indent);
	  }
	    
	write_formatted_lines (bytes, local_lines, lines,
			       P_write_expr, out, operand0, indent + 1);
      }
      break;

    case OP_expr_size:
    case OP_type_size:
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, op_to_token[opcode], indent);
    case OP_quest:
      {
	int operand0_bytes, operand1_bytes, operand2_bytes;
	int operand0_lines = 0, operand1_lines = 0, operand2_lines = 0;
	int split_operands = 0;
	int operand_indent, token_length;

#if PRETTY
	operand0_bytes = P_write_expr (NULL, operand0, indent + 1,
				       &operand0_lines);
	operand1_bytes = P_write_expr (NULL, operand1, indent + 1,
				       &operand1_lines);
	operand2_bytes = P_write_expr (NULL, operand2, indent + 1,
				       &operand2_lines);
#else
	operand0_bytes = LINE_LENGTH;
	operand1_bytes = LINE_LENGTH;
	operand2_bytes = LINE_LENGTH;
#endif

	token_length = P_write_token (out, op_to_token[opcode], indent);
	bytes += token_length;
	/* Account for the token written by P_write_expr. */
	token_length += head_length;

	operand_indent = indent + 1;
	/* Decide where we need to split the line.  We will either put
	 * all three operands on the same line or on separate lines. */
	if ((head_length + indent + bytes + operand0_bytes + operand1_bytes + \
	     operand2_bytes + tail_length >= LINE_LENGTH) || \
	    (operand0_lines > 0) || (operand1_lines > 0) || \
	    (operand2_lines > 0))
	  {
	    split_operands = 1;

	    /* See if we can line the operands up with the token string. */
#if PRETTY
	    operand0_bytes = P_write_expr (NULL, operand0,
					   indent + token_length,
					   &operand0_lines);
	    operand1_bytes = P_write_expr (NULL, operand1,
					   indent + token_length,
					   &operand1_lines);
	    operand2_bytes = P_write_expr (NULL, operand2,
					   indent + token_length,
					   &operand2_lines);
#else
	    operand0_bytes = LINE_LENGTH;
	    operand1_bytes = LINE_LENGTH;
	    operand2_bytes = LINE_LENGTH;
#endif
	  
	    if ((indent + token_length + operand0_bytes < LINE_LENGTH) && \
		(indent + token_length + operand1_bytes < LINE_LENGTH) && \
		(indent + token_length + operand2_bytes < LINE_LENGTH))
	      operand_indent = indent + token_length;
	  }

	if (split_operands)
	  {
	    write_formatted_nli (bytes, local_lines, lines,
				 out, operand_indent);
	  }

	write_formatted_lines (bytes, local_lines, lines,
			       P_write_expr, out, operand0, operand_indent);

	if (split_operands)
	  {
	    write_formatted_nli (bytes, local_lines, lines,
				 out, operand_indent);
	  }

	write_formatted_lines (bytes, local_lines, lines,
			       P_write_expr, out, operand1, operand_indent);

	if (split_operands)
	  {
	    write_formatted_nli (bytes, local_lines, lines,
				 out, operand_indent);
	  }

	write_formatted_lines (bytes, local_lines, lines,
			       P_write_expr, out, operand2, operand_indent);
      }
      break;

    case OP_disj:
    case OP_conj:
    case OP_assign:
    case OP_or:
    case OP_xor:
    case OP_and:
    case OP_eq:
    case OP_ne:
    case OP_lt:
    case OP_le:
    case OP_ge:
    case OP_gt:
    case OP_rshft:
    case OP_lshft:
    case OP_add:
    case OP_sub:
    case OP_mul:
    case OP_div:
    case OP_mod:
    case OP_Aadd:
    case OP_Asub:
    case OP_Amul:
    case OP_Adiv:
    case OP_Amod:
    case OP_Alshft:
    case OP_Arshft:
    case OP_Aand:
    case OP_Aor:
    case OP_Axor:
      {
	int operand0_bytes, operand0_lines = 0;
	int split_before_operand1 = 0;
	int operand1_indent, token_length;

	token_length = P_write_token (out, op_to_token[opcode], indent);
	bytes += token_length;
	/* Account for the token written by P_write_expr. */
	token_length += head_length;

#if PRETTY
	operand0_bytes = P_write_expr (NULL, operand0, indent + 1,
				       &operand0_lines);
#else
	operand0_bytes = LINE_LENGTH;
#endif
	
	operand1_indent = indent + 1;
	if ((head_length + indent + bytes + operand0_bytes + \
	     tail_length >= LINE_LENGTH) || (operand0_lines > 0))
	  {
	    split_before_operand1 = 1;

	    write_formatted_nli (bytes, local_lines, lines, out, indent + 1);
	  }

	write_formatted_lines (bytes, local_lines, lines,
			       P_write_expr, out, operand0, indent + 1);

	if (!split_before_operand1)
	  {
	    int operand1_bytes, operand1_lines = 0;

	    /* See if operand1 can fit on the same line. */
#if PRETTY
	    operand1_bytes = P_write_expr (NULL, operand1, indent + 1,
					   &operand1_lines);
#else
	    operand1_bytes = LINE_LENGTH;
#endif

	    if ((head_length + indent + bytes + operand1_bytes + \
		 tail_length >= LINE_LENGTH) || (operand1_lines > 0))
	      {
		split_before_operand1 = 1;

		/* See if operand1 can line up with the opcode token. */
#if PRETTY
		operand1_bytes = P_write_expr (NULL, operand1,
					       indent + token_length,
					       &operand1_lines);
#else
		operand1_bytes = LINE_LENGTH;
#endif

		if (head_length + indent + operand1_bytes + \
		    tail_length < LINE_LENGTH)
		  operand1_indent = indent + token_length;
	      }
	  }
	   
	if (split_before_operand1)
	  {
	    write_formatted_nli (bytes, local_lines, lines,
				 out, operand1_indent);
	  }

	write_formatted_lines (bytes, local_lines, lines,
			       P_write_expr, out, operand1, operand1_indent);
      }
      break;

    case OP_compexpr:
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, op_to_token[opcode], indent);
      write_formatted_nli (bytes, local_lines, lines, out, indent + 1);
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_expr_list_container, out, operand0,
			     indent);
      break;
      
    case OP_neg:
    case OP_not:
    case OP_inv:
    case OP_preinc:
    case OP_predec:
    case OP_postinc:
    case OP_postdec:
    case OP_indr:
    case OP_addr:
      {
	int operand0_bytes, operand0_lines = 0;

#if PRETTY
	operand0_bytes = P_write_expr (NULL, operand0, indent + 1,
				       &operand0_lines);
#else
	operand0_bytes = LINE_LENGTH;
#endif

	write_formatted (bytes, local_lines, lines,
			 P_write_token, out, op_to_token[opcode], indent);

	/* Decide if we need to split the line. */
	if ((head_length + indent + bytes + operand0_bytes + \
	     tail_length >= LINE_LENGTH) || (operand0_lines > 0))
	  write_formatted_nli (bytes, local_lines, lines, out, indent + 1);

	write_formatted_lines (bytes, local_lines, lines,
			       P_write_expr, out, operand0, indent + 1);
      }
      break;

    case OP_index:
    case OP_call:
    case OP_phi:
      {
	int operand0_bytes, operand0_lines = 0;
	int operand1_bytes, operand1_lines = 0;
	int split_before_operand0 = 0, split_before_operand1 = 0;

#if PRETTY
	operand0_bytes = P_write_expr (NULL, operand0, indent + 1,
				       &operand0_lines);
	operand1_bytes = P_write_expr_list_container (NULL, operand1,
						      indent + 1,
						      &operand1_lines);
#else
	operand0_bytes = LINE_LENGTH;
	operand1_bytes = LINE_LENGTH;
#endif

	write_formatted (bytes, local_lines, lines,
			 P_write_token, out, op_to_token[opcode], indent);
	
	/* Decide where we need to split the line. */
	if ((head_length + indent + bytes + operand0_bytes + operand1_bytes + \
	     tail_length >= LINE_LENGTH) || (operand0_lines > 0) || \
	    (operand1_lines > 0))
	  {
	    split_before_operand1 = 1;

	    if ((head_length + indent + bytes + operand0_bytes + \
		 tail_length >= LINE_LENGTH) || (operand0_lines > 0))
	      split_before_operand0 = 1;
	  }

	if (split_before_operand0)
	  write_formatted_nli (bytes, local_lines, lines, out, indent + 1);

	write_formatted_lines (bytes, local_lines, lines,
			       P_write_expr, out, operand0, indent + 1);

	if (split_before_operand1)
	  write_formatted_nli (bytes, local_lines, lines, out, indent + 1);

	write_formatted_lines (bytes, local_lines, lines,
			       P_write_expr_list_container, out, operand1,
			       indent);
      }
      break;

    case OP_asm_oprd:
      {
	Asmoprd a = P_GetExprAsmoprd (expr_core);
	char *mod = P_Asmmod2String (P_GetAsmoprdModifiers (a));
	char *con = P_GetAsmoprdConstraints (a);
	int st_lit0_bytes, st_lit1_bytes;
	int split_before_st_lit0 = 0, split_before_st_lit1 = 0;

#if PRETTY
	st_lit0_bytes = P_write_ST_LIT (NULL, mod, indent + 1);
	st_lit1_bytes = P_write_ST_LIT (NULL, con, indent + 1);
#else
	st_lit0_bytes = LINE_LENGTH;
	st_lit1_bytes = LINE_LENGTH;
#endif

	write_formatted (bytes, local_lines, lines,
			 P_write_token, out, op_to_token[opcode], indent);

	if (head_length + indent + bytes + st_lit0_bytes + st_lit1_bytes + \
	    tail_length >= LINE_LENGTH)
	  {
	    split_before_st_lit1 = 1;

	    if (head_length + indent + bytes + st_lit0_bytes + \
		tail_length >= LINE_LENGTH)
	      split_before_st_lit0 = 1;
	  }

	if (split_before_st_lit0)
	  write_formatted_nli (bytes, local_lines, lines, out, indent + 1);

	write_formatted (bytes, local_lines, lines,
			 P_write_ST_LIT, out, mod, indent + 1);

	if (split_before_st_lit1)
	  write_formatted_nli (bytes, local_lines, lines, out, indent + 1);

	write_formatted (bytes, local_lines, lines,
			 P_write_ST_LIT, out, con, indent + 1);
      }
      break;

    case OP_stmt_expr:
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, op_to_token[opcode], indent);
      write_formatted_nli (bytes, local_lines, lines, out, indent + 1);
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_stmt, out, P_GetExprStmt (expr_core),
			     indent + 1);
      break;

    case OP_null:
    case OP_sync:
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, op_to_token[opcode], indent);
      break;

    default:
      P_punt ("write.c:P_write_expr_core:%d Unknown Opcode %d", __LINE__,
	      P_GetExprOpcode (expr_core));
    }

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the expr_list rule to disk.
 *
 * \param out
 *  the file to write.
 * \param expr_list
 *  the expr_list ::Expr to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 */
int
P_write_expr_list (FILE *out, Expr expr_list, int indent, int *lines)
{
  int bytes = 0;
  int local_lines = 0;

  while (expr_list)
    {
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_expr, out, expr_list, indent);

      if ((expr_list = P_GetExprNext (expr_list)))
	write_formatted_nli (bytes, local_lines, lines, out, indent);
    }

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the expr_list_container rule to disk.
 *
 * \param out
 *  the file to write.
 * \param expr_list_container
 *  the expr_list_container ::Expr to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the expr_list_container rule to disk.
 */
int
P_write_expr_list_container (FILE *out, Expr expr_list_container, int indent,
			     int *lines)
{
  int bytes = 0;
  int local_lines = 0;
  int expr_list_lines = 0;
#if PRETTY
  int expr_list_bytes = P_write_expr_list (NULL, expr_list_container,
					   indent + 1, &expr_list_lines);
  int head_length = P_write_token (NULL, '(', indent);
  int tail_length = P_write_token (NULL, ')', indent);
#else
  int expr_list_bytes = LINE_LENGTH;
  int head_length = LINE_LENGTH;
  int tail_length = LINE_LENGTH;
#endif

  write_formatted (bytes, local_lines, lines, P_write_token, out, '(', indent);

  if ((head_length + indent + bytes + expr_list_bytes + \
       tail_length >= LINE_LENGTH) || (expr_list_lines > 0))
    write_formatted_nli (bytes, local_lines, lines, out, indent + 1);

  write_formatted_lines (bytes, local_lines, lines,
			 P_write_expr_list, out, expr_list_container,
			 indent + 1);

  if (local_lines > 0)
    write_formatted_nli (bytes, local_lines, lines, out, indent);

  write_formatted (bytes, local_lines, lines, P_write_token, out, ')', indent);

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the field rule to disk.
 *
 * \param out
 *  the file to write.
 * \param field
 *  the ::Field to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the field rule to disk.
 */
int
P_write_field (FILE *out, Field field, int indent, int *lines)
{
  Identifier identifier = P_GetFieldIdentifier (field);
  Type type = P_GetFieldType (field);
  int bytes = 0;
  int local_lines = 0;
  int type_lines = 0, bitfield_lines = 0;
  int type_bytes = 0, bitfield_bytes = 0;
  int split_before_type = 0, split_before_bitfield = 0;
#if PRETTY
  int head_length = P_write_token (NULL, '(', indent);
  int tail_length = P_write_token (NULL, ')', indent);

  type_bytes = P_write_key (NULL, type, indent + 1, &type_lines);

  if (bitfield)
    {
      bitfield_bytes = P_write_token (NULL, BITFIELD, indent + 1);
      bitfield_bytes += P_write_expr (NULL, bitfield, indent + 1,
				      &bitfield_lines);
    }
#else
  int head_length = LINE_LENGTH;
  int tail_length = LINE_LENGTH;

  type_bytes = LINE_LENGTH;

  if (P_GetFieldIsBitField(field))
    bitfield_bytes = LINE_LENGTH;
#endif

  write_formatted (bytes, local_lines, lines, P_write_token, out, '(', indent);
  write_formatted_lines (bytes, local_lines, lines,
			 P_write_identifier, out, identifier, indent);

  /* Determine where we need to split the line. */
  if ((head_length + indent + bytes + type_bytes + bitfield_bytes + \
       tail_length >= LINE_LENGTH) || (type_lines > 0) || (bitfield_lines > 0))
    {
      split_before_type = 1;

      if ((head_length + indent + type_bytes + bitfield_bytes + \
	   tail_length >= LINE_LENGTH) || (bitfield_lines > 0))
	split_before_bitfield = 1;
    }

  if (split_before_type)
    write_formatted_nli (bytes, local_lines, lines, out, indent + 1);

  write_formatted (bytes, local_lines, lines,
		   P_write_token, out, TYPE, indent + 1);
  write_formatted_lines (bytes, local_lines, lines,
			 P_write_key, out, type, indent + 1);

  if (P_GetFieldIsBitField(field))
    {
      if (split_before_bitfield)
	write_formatted_nli (bytes, local_lines, lines, out, indent + 1);

      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, BITFIELD, indent + 1);
      write_formatted (bytes, local_lines, lines,
		       P_write_I_LIT, out, P_GetFieldBitSize (field),
		       indent + 1);
      write_formatted (bytes, local_lines, lines,
		       P_write_I_LIT, out,
		       P_GetFieldBitOffsetRemainder (field), indent + 1);
    }

  write_formatted (bytes, local_lines, lines,
		   P_write_token, out, OFFSET, indent + 1);
  write_formatted (bytes, local_lines, lines,
		   P_write_I_LIT, out, P_GetFieldOffset (field), indent + 1);

  write_formatted (bytes, local_lines, lines,
		   P_write_token, out, PARENT, indent + 1);
  write_formatted_lines (bytes, local_lines, lines,
			 P_write_key, out, P_GetFieldParentKey (field),
			 indent + 1);
  write_formatted_lines (bytes, local_lines, lines,
			 P_write_pragma_list, out, P_GetFieldPragma (field),
			 indent);

  if (local_lines > 0)
    write_formatted_nli (bytes, local_lines, lines, out, indent);

  write_formatted (bytes, local_lines, lines, P_write_token, out, ')', indent);

  identifier = P_RemoveIdentifier (identifier);

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the field_list rule to disk.
 *
 * \param out
 *  the file to write.
 * \param field_list
 *  the field_list ::Field to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the field_list rule to disk.
 */
int
P_write_field_list (FILE *out, Field field_list, int indent, int *lines)
{
  int bytes = 0;
  int local_lines = 0;

  while (field_list)
    {
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_field, out, field_list, indent);

      if ((field_list = P_GetFieldNext (field_list)))
	write_formatted_nli (bytes, local_lines, lines, out, indent);
    }

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the file_type rule to disk.
 *
 * \param out
 *  the file to write.
 * \param file_type
 *  the ::_FileType to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the file_type rule to disk.
 */
int
P_write_file_type (FILE *out, _FileType file_type, int indent, int *lines)
{
  int bytes = 0;
  int local_lines = 0;

  switch (file_type)
    {
    case FT_SOURCE:
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, SOURCE, indent);
      break;

    case FT_HEADER:
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, HEADER, indent);
      break;
    }

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the func_dcl rule to disk.
 *
 * \param out
 *  the file to write.
 * \param func_dcl
 *  the ::FuncDcl to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the func_dcl rule to disk.
 */
int
P_write_func_dcl (FILE *out, FuncDcl func_dcl, int indent, int *lines)
{
  Identifier identifier = P_GetFuncDclIdentifier (func_dcl);
  Position position = P_GetFuncDclPosition (func_dcl);
  Pragma pragma;
  Stmt stmt = P_GetFuncDclStmt (func_dcl);
  int bytes = 0;
  int local_lines = 0;

  /* If any write handlers are defined, save the ext field to the pragma
   * list. */
  if (Handlers[ES_FUNC])
    P_ExtWrite (ES_FUNC, (void *)func_dcl);

  pragma = P_GetFuncDclPragma (func_dcl);

  write_formatted (bytes, local_lines, lines, P_write_token, out, '(', indent);
  write_formatted (bytes, local_lines, lines,
		   P_write_token, out, FUNCTION, indent);
  write_formatted_lines (bytes, local_lines, lines,
			 P_write_identifier, out, identifier, indent);
  write_formatted (bytes, local_lines, lines,
		   P_write_token, out, TYPE, indent + 1);
  write_formatted_lines (bytes, local_lines, lines,
			 P_write_key, out, P_GetFuncDclType (func_dcl),
			 indent + 1);
  write_formatted_lines (bytes, local_lines, lines,
			 P_write_var_qual_list, out,
			 P_GetFuncDclQualifier (func_dcl), indent + 1);
  write_formatted (bytes, local_lines, lines,
		   P_write_I_LIT, out, P_GetFuncDclMaxExprID (func_dcl),
		   indent);

  if (pragma)
    {
      write_formatted_nli (bytes, local_lines, lines, out, indent + 1);
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_pragma_list, out, pragma, indent + 1);
    }

  if (stmt)
    {
      write_formatted_nli (bytes, local_lines, lines, out, indent + 1);
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_var_dcl_list_container, out,
			     P_GetFuncDclParam (func_dcl), indent + 1);
      write_formatted_nli (bytes, local_lines, lines, out, indent + 1);
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_stmt, out, stmt, indent + 1);
    }

  if (P_ValidPosition (position))
    {
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_position, out, position, indent);
    }

  write_formatted (bytes, local_lines, lines, P_write_token, out, ')', indent);

  position = P_RemovePosition (position);
  identifier = P_RemoveIdentifier (identifier);

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the identifier rule to disk.
 *
 * \param out
 *  the file to write.
 * \param identifier
 *  the ::Identifier to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the identifier rule to disk.
 */
int
P_write_identifier (FILE *out, Identifier identifier, int indent, int *lines)
{
  int bytes = 0;
  int local_lines = 0;
  char *quoted_name;

  write_formatted (bytes, local_lines, lines,
		   P_write_token, out, ID, indent);

  if (P_GetIdentifierName (identifier))
    {
      quoted_name = P_GetIdentifierName (identifier);
      write_formatted (bytes, local_lines, lines,
		       P_write_ST_LIT, out, quoted_name, indent);
    }
  write_formatted_lines (bytes, local_lines, lines,
			 P_write_key, out, P_GetIdentifierKey (identifier),
			 indent);

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the in_name rule to disk.
 *
 * \param out
 *  the file to write.
 * \param in_name
 *  the in_name string to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the in_name rule to disk.
 */
int
P_write_in_name (FILE *out, char *in_name, int indent, int *lines)
{
  int bytes = 0;
  int local_lines = 0;
  char *quoted_name;

  if (in_name)
    {
      quoted_name = in_name;
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, IN, indent);
      write_formatted (bytes, local_lines, lines,
		       P_write_ST_LIT, out, quoted_name, indent);
    }

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the include rule to disk.
 *
 * \param out
 *  the file to write.
 * \param include
 *  the include filename string to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the include rule to disk.
 */
int
P_write_include (FILE *out, char *include, int indent, int *lines)
{
  int bytes = 0;
  int local_lines = 0;
  char *quoted_name;

  write_formatted (bytes, local_lines, lines, P_write_token, out, '(', indent);
  write_formatted (bytes, local_lines, lines,
		   P_write_token, out, INCLUDE, indent);

  quoted_name = include;
  write_formatted (bytes, local_lines, lines,
		   P_write_ST_LIT, out, quoted_name, indent);

  write_formatted (bytes, local_lines, lines, P_write_token, out, ')', indent);

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the initializer rule to disk.
 *
 * \param out
 *  the file to write.
 * \param initializer
 *  the initializer ::Init to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the initializer rule to disk.
 */
int
P_write_initializer (FILE *out, Init initializer, int indent, int *lines)
{
  Expr expr = P_GetInitExpr (initializer);
  Init set = P_GetInitSet (initializer);
  Pragma pragma;
  int bytes = 0;
  int local_lines = 0;

  /* If any write handlers are defined, save the ext field to the pragma
   * list. */
  if (Handlers[ES_INIT])
    P_ExtRead (ES_INIT, (void *)initializer);

  pragma = P_GetInitPragma (initializer);

  if (expr)
    {
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_expr, out, expr, indent);
    }
  else if (set)
    {
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_initializer_list_container, out,
			     P_GetInitSet (initializer), indent);
    }

  if (pragma)
    {
      write_formatted_nli (bytes, local_lines, lines, out, indent);
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_pragma_list, out, pragma, indent);
    }

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the initializer_list rule to disk.
 *
 * \param out
 *  the file to write.
 * \param initializer_list
 *  the initailizer_list ::Init to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the initializer_list rule to disk.
 */
int
P_write_initializer_list (FILE *out, Init initializer_list, int indent,
			  int *lines)
{
  Init list = initializer_list;
  int bytes = 0;
  int local_lines = 0;
  int initializer_list_bytes = 0, initializer_list_lines = 0;
  int split_initializer_list = 0;

  while (list)
    {
#if PRETTY
      initializer_list_bytes += P_write_initializer (NULL, list, indent,
						     lines);
#else
      initializer_list_bytes = LINE_LENGTH;
#endif
      if (lines) initializer_list_lines += *lines;

      list = P_GetInitNext (list);
    }

  if ((indent + initializer_list_bytes >= LINE_LENGTH) || \
      (initializer_list_lines > 0))
    split_initializer_list = 1;

  while (initializer_list)
    {
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_initializer, out, initializer_list,
			     indent);

      if ((initializer_list = P_GetInitNext (initializer_list)) && \
	  split_initializer_list)
	write_formatted_nli (bytes, local_lines, lines, out, indent + 1);
    }

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the initializer_list_container rule to disk.
 *
 * \param out
 *  the file to write.
 * \param initializer_list_container
 *  the initialzier_list_container ::Init to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the initializer_list_container rule to disk.
 */
int
P_write_initializer_list_container (FILE *out, Init initializer_list_container,
				    int indent, int *lines)
{
  int bytes = 0;
  int local_lines = 0;
  int initializer_list_lines = 0;
#if PRETTY
  int initializer_list_bytes = \
    P_write_initializer_list (NULL, initializer_list_container, indent, 
			      &initializer_list_lines);
  int head_length = P_write_token (NULL, '(', indent);
  int tail_length = P_write_token (NULL, ')', indent);
#else
  int initializer_list_bytes = LINE_LENGTH;
  int head_length = LINE_LENGTH;
  int tail_length = LINE_LENGTH;
#endif
  int split_initializer_list_container = 0;

  if ((indent + head_length + initializer_list_bytes + \
       tail_length >= LINE_LENGTH) || (initializer_list_lines > 0))
    split_initializer_list_container = 1;

  write_formatted (bytes, local_lines, lines, P_write_token, out, '(', indent);

  if (split_initializer_list_container)
    write_formatted_nli (bytes, local_lines, lines, out, indent + 1);

  write_formatted_lines (bytes, local_lines, lines,
			 P_write_initializer_list, out,
			 initializer_list_container, indent + 1);

  if (local_lines > 0)
    write_formatted_nli (bytes, local_lines, lines, out, indent);

  write_formatted (bytes, local_lines, lines, P_write_token, out, ')', indent);

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the ipste_flags rule to disk.
 *
 * \param out
 *  the file to write.
 * \param ipste_flags
 *  the _IPSTEFlags to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  return the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the ipste_flags rule to disk.  Only IPSTEF_NOT_AVAIL is written
 * at this time.
 */
int
P_write_ipste_flags (FILE *out, _IPSTEFlags ipste_flags, int indent, int *lines)
{
  int bytes = 0;
  int local_lines = 0;

  if (ipste_flags & IPSTEF_NOT_AVAIL)
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, NOT_AVAIL, indent);
    }

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the ipste_flags_list rule to disk.
 *
 * \param out
 *  the file to write.
 * \param ipste_flags_list
 *  the _IPSTEFlags to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  return the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the ipste_flags_list rule to disk.
 */
int
P_write_ipste_flags_list (FILE *out, _IPSTEFlags ipste_flags_list, int indent,
			  int *lines)
{
  int bytes = 0;
  int local_lines = 0;

  write_formatted_lines (bytes, local_lines, lines,
			 P_write_ipste_flags, out, ipste_flags_list, indent);

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the ip_sym_tab_ent rule to disk.
 *
 * \param out
 *  the file to write.
 * \param ip_sym_tab_ent
 *  the ::IPSymTabEnt to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the ip_sym_tab_ent rule to disk.
 */
int
P_write_ip_sym_tab_ent (FILE *out, IPSymTabEnt ip_sym_tab_ent, int indent,
			int *lines)
{
  char *source_name = P_GetIPSymTabEntSourceName (ip_sym_tab_ent);
  char *in_name = P_GetIPSymTabEntInName (ip_sym_tab_ent);
  char *out_name = P_GetIPSymTabEntOutName (ip_sym_tab_ent);
  Pragma pragma;
  char *quoted_name;
  int bytes = 0;
  int local_lines = 0;

  /* If any write handlers are defined, save the ext field to the eragma
   * list. */
  if (Handlers[ES_IPSYMTABENT])
    P_ExtWrite (ES_IPSYMTABENT, (void *)ip_sym_tab_ent);

  pragma = P_GetIPSymTabEntPragma (ip_sym_tab_ent);

  write_formatted (bytes, local_lines, lines, P_write_token, out, '(', indent);
  write_formatted (bytes, local_lines, lines,
		   P_write_token, out, IP_TABLE, indent);

  quoted_name = source_name;
  write_formatted (bytes, local_lines, lines,
		   P_write_ST_LIT, out, quoted_name, indent);
  write_formatted (bytes, local_lines, lines,
		   P_write_I_LIT, out, P_GetIPSymTabEntKey (ip_sym_tab_ent),
		   indent);
  write_formatted_lines (bytes, local_lines, lines,
			 P_write_file_type, out,
			 P_GetIPSymTabEntFileType (ip_sym_tab_ent), indent);

  write_formatted_lines (bytes, local_lines, lines,
			 P_write_in_name, out, in_name, indent);
  write_formatted_lines (bytes, local_lines, lines,
			 P_write_out_name, out, out_name, indent);
  write_formatted (bytes, local_lines, lines,
		   P_write_token, out, NUM_ENTRIES, indent);
  write_formatted (bytes, local_lines, lines,
		   P_write_I_LIT, out,
		   P_GetIPSymTabEntNumEntries (ip_sym_tab_ent), indent);
  write_formatted (bytes, local_lines, lines,
		   P_write_token, out, OFFSET, indent);
  write_formatted (bytes, local_lines, lines,
		   P_write_offset, out,
		   &P_GetIPSymTabEntOffset (ip_sym_tab_ent), indent);

  if (pragma)
    {
      write_formatted_nli (bytes, local_lines, lines, out, indent);
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_pragma_list, out, pragma, indent);
    }

  write_formatted (bytes, local_lines, lines, P_write_token, out, ')', indent);
  
  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the key rule to disk.
 *
 * \param out
 *  the file to write.
 * \param key
 *  the ::Key to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the key rule to disk.
 */
int
P_write_key (FILE *out, Key key, int indent, int *lines)
{
  int bytes = 0;
  int local_lines = 0;

  write_formatted (bytes, local_lines, lines, P_write_token, out, '(', indent);
  write_formatted (bytes, local_lines, lines,
		   P_write_I_LIT, out, key.file, indent);
  write_formatted (bytes, local_lines, lines,
		   P_write_I_LIT, out, key.sym, indent);
  write_formatted (bytes, local_lines, lines, P_write_token, out, ')', indent);

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the label rule to disk.
 *
 * \param out
 *  the file to write.
 * \param label
 *  the ::Label to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the label rule to disk.
 */
int
P_write_label (FILE *out, Label label, int indent, int *lines)
{
  int bytes = 0;
  int local_lines = 0;

  switch (P_GetLabelType (label))
    {
    case LB_LABEL:
      {
	Identifier identifier = P_GetLabelIdentifier (label);

	write_formatted (bytes, local_lines, lines,
			 P_write_token, out, '(', indent);
	write_formatted (bytes, local_lines, lines,
			 P_write_token, out, LABEL, indent);
	write_formatted_lines (bytes, local_lines, lines,
			       P_write_identifier, out, identifier, indent);
	write_formatted (bytes, local_lines, lines,
			 P_write_token, out, ')', indent);

	identifier = P_RemoveIdentifier (identifier);
      }
      break;

    case LB_CASE:
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, '(', indent);
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, CASE, indent);
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_expr, out, P_GetLabelExpression (label),
			     indent);
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, ')', indent);
      break;

    case LB_DEFAULT:
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, '(', indent);
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, DEFAULT, indent);
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, ')', indent);
      break;

    default:
      P_punt ("write.c:P_write_label:%d Unknown Label type %d", __LINE__,
	      P_GetLabelType (label));
    }

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the label_list rule to disk.
 *
 * \param out
 *  the file to write.
 * \param label_list
 *  the label_list ::Label to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the label_list rule to disk.
 */
int
P_write_label_list (FILE *out, Label label_list, int indent, int *lines)
{
  int bytes = 0;
  int local_lines = 0;

  while (label_list)
    {
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_label, out, label_list, indent);

      if ((label_list = P_GetLabelNext (label_list)))
	write_formatted_nli (bytes, local_lines, lines, out, indent + 1);
    }

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the named_basic_type rule to disk.
 *
 * \param out
 *  the file to write.
 * \param named_basic_type
 *  the named_basic_type ::_BasicType to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the named_basic_type rule to disk.
 */
int
P_write_named_basic_type (FILE *out, _BasicType named_basic_type, int indent,
			  int *lines)
{
  int bytes = 0;
  int local_lines = 0;

  switch (named_basic_type)
    {
    case BT_STRUCT:
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, STRUCT, indent);
      break;
    case BT_UNION:
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, UNION, indent);
      break;
    case BT_ENUM:
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, ENUM, indent);
      break;
    default:
      P_punt ("write.c:P_write_named_basic_type:%d Unknown named_basic_type "
	      "%d", __LINE__ - 1, named_basic_type);
    }

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the out_name rule to disk.
 *
 * \param out
 *  the file to write.
 * \param out_name
 *  the out_name string to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the out_name rule to disk.
 */
int
P_write_out_name (FILE *out, char *out_name, int indent, int *lines)
{
  int bytes = 0;
  int local_lines = 0;
  char *quoted_name;

  if (out_name)
    {
      quoted_name = out_name;
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, OUT, indent);
      write_formatted (bytes, local_lines, lines,
		       P_write_ST_LIT, out, quoted_name, indent);
    }

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the param rule to disk.
 *
 * \param out
 *  the file to write.
 * \param param
 *  the ::Param to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the param rule to disk.
 */
int
P_write_param (FILE *out, Param param, int indent, int *lines)
{
  int bytes = 0;
  int local_lines = 0;

  write_formatted (bytes, local_lines, lines, P_write_token, out, '(', indent);
  write_formatted (bytes, local_lines, lines,
		   P_write_token, out, PARAM, indent);
  write_formatted (bytes, local_lines, lines,
		   P_write_token, out, TYPE, indent);
  write_formatted_lines (bytes, local_lines, lines,
			 P_write_key, out, P_GetParamKey (param), indent);
  write_formatted (bytes, local_lines, lines, P_write_token, out, ')', indent);

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the param_list rule to disk.
 *
 * \param out
 *  the file to write.
 * \param param_list
 *  the param_list ::Param to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the param_list rule to disk.
 */
int
P_write_param_list (FILE *out, Param param_list, int indent, int *lines)
{
  int bytes = 0;
  int local_lines = 0;

  while (param_list)
    {
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_param, out, param_list, indent);

      if ((param_list = P_GetParamNext (param_list)))
	write_formatted_nli (bytes, local_lines, lines, out, indent);
    }

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the parloop_index rule to disk.
 *
 * \param out
 *  the file to write.
 * \param parloop_index
 *  the parloop_index ::Expr to write.
 * \param indent
 *  the indent level
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the parloop_index rule to disk.
 */
int
P_write_parloop_index (FILE *out, Expr parloop_index, int indent, int *lines)
{
  Identifier identifier = P_GetExprVarIdentifier (parloop_index);
  int bytes = 0;
  int local_lines = 0;
  
  write_formatted_lines (bytes, local_lines, lines,
			 P_write_identifier, out, identifier, indent);

  identifier = P_RemoveIdentifier (identifier);

  if (lines) *lines = local_lines;
  return (bytes);
}
		       
/*! \brief Writes the position rule to disk.
 *
 * \param out
 *  the file to write.
 * \param position
 *  the ::Position to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the position rule to disk.
 */
int
P_write_position (FILE *out, Position position, int indent, int *lines)
{
  int bytes = 0;
  int local_lines = 0;

  if (P_ValidPosition (position))
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, POS, indent);
      write_formatted (bytes, local_lines, lines,
		       P_write_ST_LIT, out, P_GetPositionFilename (position),
		       indent);
      write_formatted (bytes, local_lines, lines,
		       P_write_I_LIT, out, P_GetPositionLineno (position),
		       indent);
      write_formatted (bytes, local_lines, lines,
		       P_write_I_LIT, out, P_GetPositionColno (position),
		       indent);
    }

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the pragma rule to disk.
 *
 * \param out
 *  the file to write.
 * \param pragma
 *  the ::Pragma to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the pragma rule to disk.
 */
int
P_write_pragma (FILE *out, Pragma pragma, int indent, int *lines)
{
  int bytes = 0;
  int local_lines = 0;

  write_formatted (bytes, local_lines, lines,
		   P_write_token, out, PRAGMA, indent);
  write_formatted (bytes, local_lines, lines,
		   P_write_ST_LIT, out, P_GetPragmaSpecifier (pragma), indent);
  write_formatted_lines (bytes, local_lines, lines,
			 P_write_expr_list_container, out,
			 P_GetPragmaExpr (pragma), indent);

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the pragma_list rule to disk.
 *
 * \param out
 *  the file to write.
 * \param pragma_list
 *  the pragma_list ::Pragma to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the pragma_list rule to disk.
 */
int
P_write_pragma_list (FILE *out, Pragma pragma_list, int indent, int *lines)
{
  int bytes = 0;
  int local_lines = 0;

  while (pragma_list)
    {
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_pragma, out, pragma_list, indent);

      pragma_list = P_GetPragmaNext (pragma_list);
    }

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the prof_st rule to disk.
 *
 * \param out
 *  the file to write.
 * \param prof_st
 *  the ProfST to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the prof_st rule to disk.
 */
int
P_write_prof_st (FILE *out, ProfST prof_st, int indent, int *lines)
{
  int bytes = 0;
  int local_lines = 0;

  if (prof_st)
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, PROFILE, indent);
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_prof_st_list, out, prof_st, indent);
    }

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the prof_st_list rule to disk.
 *
 * \param out
 *  the file to write.
 * \param prof_st_list
 *  the prof_st_list ProfST to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the prof_st_list rule to disk.
 */
int
P_write_prof_st_list (FILE *out, ProfST prof_st_list, int indent, int *lines)
{
  int bytes = 0;
  int local_lines = 0;

  while (prof_st_list)
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_F_LIT, out, P_GetProfSTCount (prof_st_list),
		       indent);
      prof_st_list = prof_st_list->next;
    }

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the profile rule to disk.
 *
 * \param out
 *  the file to write.
 * \param profile
 *  the profile value to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the profile rule to disk.
 */
int 
P_write_profile (FILE *out, double profile, int indent, int *lines)
{
  int bytes = 0;
  int local_lines = 0;

  if (profile != 0.0)
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, PROFILE, indent);
      write_formatted (bytes, local_lines, lines,
		       P_write_F_LIT, out, profile, indent);
    }

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the pstmt rule to disk.
 *
 * \param out
 *  the file to write.
 * \param pstmt
 *  the ::Pstmt to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the pstmt rule to disk.
 */
int
P_write_pstmt (FILE *out, Pstmt pstmt, int indent, int *lines)
{
  Position position = P_GetPstmtPosition (pstmt);
  Pragma pragma;
  int bytes = 0;
  int local_lines = 0;

  /* If any write handlers are defined, save the ext field to the pragma
   * list. */
  if (Handlers[ES_PSTMT])
    P_ExtWrite (ES_PSTMT, (void *)pstmt);

  pragma = P_GetPstmtPragma (pstmt);

  write_formatted (bytes, local_lines, lines,
		   P_write_token, out, PSTMT, indent);
  if (P_ValidPosition (position))
    {
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_position, out, position, indent);
    }

  write_formatted_nli (bytes, local_lines, lines, out, indent + 1);
  write_formatted_lines (bytes, local_lines, lines,
			 P_write_stmt, out, P_GetPstmtStmt (pstmt),
			 indent + 1);
  write_formatted_nli (bytes, local_lines, lines, out, indent);

  if (pragma)
    {
      write_formatted_nli (bytes, local_lines, lines, out, indent);
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_pragma_list, out, pragma, indent);
    }


  position = P_RemovePosition (position);

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the scope rule to disk.
 *
 * \param out
 *  the file to write.
 * \param scope
 *  the ::Scope to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the scope rule to disk.
 */
int
P_write_scope (FILE *out, Scope scope, int indent, int *lines)
{
  int bytes = 0;
  int local_lines = 0;

  write_formatted (bytes, local_lines, lines,
		   P_write_token, out, SCOPE, indent);
  write_formatted_lines (bytes, local_lines, lines,
			 P_write_key, out, P_GetScopeKey (scope), indent);
  write_formatted_lines (bytes, local_lines, lines,
			 P_write_scope_entry, out,
			 P_GetScopeScopeEntry (scope), indent + bytes);

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the size rule to disk.
 *
 * \param out
 *  the file to write.
 * \param size
 *  the size value to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the size rule to disk.
 */
int
P_write_size (FILE *out, int size, int indent, int *lines)
{
  int bytes = 0;
  int local_lines = 0;

  if (size > 0)
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, SIZE, indent);
      write_formatted (bytes, local_lines, lines,
		       P_write_I_LIT, out, size, indent);
    }

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the scope_entry rule to disk.
 *
 * \param out
 *  the file to write.
 * \param scope_entry
 *  the ::ScopeEntry to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the scope_entry rule to disk.
 */
int
P_write_scope_entry (FILE *out, ScopeEntry scope_entry, int indent, int *lines)
{
  int bytes = 0, key_bytes = 0;
  int local_lines = 0, key_lines = 0;
  int column = indent;

  while (scope_entry)
    {
#if PRETTY
      key_bytes = P_write_key (NULL, P_GetScopeEntryKey (scope_entry), indent,
			       &key_lines);
#else
      key_bytes = LINE_LENGTH;
#endif
      if ((column + key_bytes >= LINE_LENGTH) || (key_lines > 0))
	{
	  column = P_write_newline_indent (out, indent);
	  bytes += column;
	  local_lines++;
	}

      key_bytes = P_write_key (out, P_GetScopeEntryKey (scope_entry), indent,
			    lines);
      column += key_bytes;
      bytes += key_bytes;
      if (lines) local_lines += *lines;

      scope_entry = P_GetScopeEntryNext (scope_entry);
    }

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the shadow rule to disk.
 *
 * \param out
 *  the file to write.
 * \param shadow
 *  the shadow ID to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the shadow rule to disk.
 */
int
P_write_shadow (FILE *out, int shadow, int indent, int *lines)
{
  int bytes = 0;
  int local_lines = 0;

  write_formatted (bytes, local_lines, lines,
		   P_write_token, out, SHADOW, indent);
  write_formatted (bytes, local_lines, lines,
		   P_write_I_LIT, out, shadow, indent);

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the stmt rule to disk.
 *
 * \param out
 *  the file to write.
 * \param stmt
 *  the ::Stmt to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the stmt rule to disk.
 */
int
P_write_stmt (FILE *out, Stmt stmt, int indent, int *lines)
{
  Position position = P_GetStmtPosition (stmt);
  ProfST profst = P_GetStmtProfile (stmt);
  Shadow shadow = P_GetStmtShadow (stmt);
  Pragma pragma;
  Label label_list = P_GetStmtLabels (stmt);
  int bytes = 0;
  int local_lines = 0;
  
  /* If any write handlers are defined, save the ext field to the pragma
   * list. */
  if (Handlers[ES_STMT])
    P_ExtWrite (ES_STMT, (void *)stmt);

  pragma = P_GetStmtPragma (stmt);

  write_formatted (bytes, local_lines, lines, P_write_token, out, '(', indent);

  if (label_list)
    {
      write_formatted_nli (bytes, local_lines, lines, out, indent + 1);
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_label_list, out, P_GetStmtLabels (stmt),
			     indent + 1);
    }

  if (local_lines > 0)
    write_formatted_nli (bytes, local_lines, lines, out, indent + 1);

  write_formatted (bytes, local_lines, lines, P_write_token, out, KEY, indent);
  write_formatted_lines (bytes, local_lines, lines,
			 P_write_key, out, P_GetStmtKey (stmt), indent + 1);
  write_formatted_nli (bytes, local_lines, lines, out, indent + 1);
  write_formatted_lines (bytes, local_lines, lines,
			 P_write_stmt_core, out, stmt, indent + 1);

  if (pragma)
    {
      write_formatted_nli (bytes, local_lines, lines, out, indent + 1);
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_pragma_list, out, P_GetStmtPragma (stmt),
			     indent + 1);
    }

  if (shadow)
    {
      write_formatted_nli (bytes, local_lines, lines, out, indent + 1);
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_shadow, out, P_GetShadowParamID (shadow),
			     indent + 1);
    }

  if (P_ValidPosition (position))
    {
      write_formatted_nli (bytes, local_lines, lines, out, indent + 1);
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_position, out, position, indent + 1);
    }

  if (profst)
    {
      write_formatted_nli (bytes, local_lines, lines, out, indent + 1);
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_prof_st, out, profst, indent + 1);
    }

  write_formatted_nli (bytes, local_lines, lines, out, indent);
  write_formatted (bytes, local_lines, lines, P_write_token, out, ')', indent);

  position = P_RemovePosition (position);

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the stmt_core rule to disk.
 *
 * \param out
 *  the file to write.
 * \param stmt_core
 *  the stmt_core ::Stmt to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the stmt_core rule to disk.
 */
int
P_write_stmt_core (FILE *out, Stmt stmt_core, int indent, int *lines)
{
  int bytes = 0;
  int local_lines = 0;
#if PRETTY
  int head_length = P_write_token (NULL, '(', indent);
  int tail_length = P_write_token (NULL, ')', indent);
#else
  int head_length = LINE_LENGTH;
  int tail_length = LINE_LENGTH;
#endif

  switch (P_GetStmtType (stmt_core))
    {
    case ST_EXPR:
      write_formatted_lines (bytes, local_lines, lines, 
			     P_write_expr, out, P_GetStmtExpr (stmt_core),
			     indent);
      break;

    case ST_COMPOUND:
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_comp_stmt, out, stmt_core, indent);
      break;

    case ST_SERLOOP:
      {
	SerLoop s = P_GetStmtSerLoop (stmt_core);
	
	switch (P_GetSerLoopLoopType (s))
	  {
	  case LT_DO:
	    {
	      Expr cond_expr = P_GetSerLoopCondExpr (s);
	      int cond_expr_lines = 0;
#if PRETTY
	      int cond_expr_bytes = P_write_expr (NULL, cond_expr, indent + 1,
						  &cond_expr_lines);
#else
	      int cond_expr_bytes = LINE_LENGTH;
#endif

	      write_formatted (bytes, local_lines, lines,
			       P_write_token, out, DO, indent);
	      write_formatted_nli (bytes, local_lines, lines, out, indent + 1);
	      write_formatted_lines (bytes, local_lines, lines,
				     P_write_stmt, out,
				     P_GetSerLoopLoopBody (s), indent + 1);
	      write_formatted_nli (bytes, local_lines, lines, out, indent);
	      write_formatted (bytes, local_lines, lines,
			       P_write_token, out, WHILE, indent);

	      /* Determine if we need to split the line. */
	      if ((head_length + indent + bytes + cond_expr_bytes + \
		   tail_length >= LINE_LENGTH) || (cond_expr_lines > 0))
		{
		  write_formatted_nli (bytes, local_lines, lines, out,
				       indent + 1);
		}

	      write_formatted_lines (bytes, local_lines, lines,
				     P_write_expr, out, cond_expr, indent + 1);
	    }
	    break;
	    
	  case LT_WHILE:
	    {
	      Expr cond_expr = P_GetSerLoopCondExpr (s);
	      int cond_expr_lines = 0;
#if PRETTY
	      int cond_expr_bytes = P_write_expr (NULL, cond_expr, indent + 1,
						  &cond_expr_lines);
#else
	      int cond_expr_bytes = LINE_LENGTH;
#endif

	      write_formatted (bytes, local_lines, lines,
			       P_write_token, out, WHILE, indent);

	      /* Determine if we need to split the line. */
	      if ((head_length + indent + bytes + cond_expr_bytes + \
		   tail_length >= LINE_LENGTH) || (cond_expr_lines > 0))
		{
		  write_formatted_nli (bytes, local_lines, lines, out,
				       indent + 1);
		}

	      write_formatted_lines (bytes, local_lines, lines,
				     P_write_expr, out, cond_expr, indent + 1);
	      write_formatted_nli (bytes, local_lines, lines, out, indent);
	      write_formatted (bytes, local_lines, lines,
			       P_write_token, out, DO, indent);
	      write_formatted_nli (bytes, local_lines, lines, out, indent + 1);
	      write_formatted_lines (bytes, local_lines, lines,
				     P_write_stmt, out,
				     P_GetSerLoopLoopBody (s), indent + 1);

	      if (lines) local_lines += *lines;
	    }
	    break;
	    
	  case LT_FOR:
	    write_formatted (bytes, local_lines, lines,
			     P_write_token, out, FOR, indent);
	    write_formatted_nli (bytes, local_lines, lines, out, indent + 1);
	    write_formatted_lines (bytes, local_lines, lines, 
				   P_write_expr_container, out,
				   P_GetSerLoopInitExpr (s), indent + 1);
	    write_formatted_nli (bytes, local_lines, lines, out, indent + 1);
	    write_formatted_lines (bytes, local_lines, lines,
				   P_write_expr_container, out,
				   P_GetSerLoopCondExpr (s), indent + 1);
	    write_formatted_nli (bytes, local_lines, lines, out, indent + 1);
	    write_formatted_lines (bytes, local_lines, lines,
				   P_write_expr_container, out,
				   P_GetSerLoopIterExpr (s), indent + 1);
	    write_formatted_nli (bytes, local_lines, lines, out, indent + 1);
	    write_formatted (bytes, local_lines, lines,
			     P_write_token, out, DO, indent);
	    write_formatted_nli (bytes, local_lines, lines, out, indent + 1);
	    write_formatted_lines (bytes, local_lines, lines,
				   P_write_stmt, out,
				   P_GetSerLoopLoopBody (s), indent + 1);
	    break;
	    
	  default:
	    P_punt ("write.c:P_write_stmt_core:%d Unknown SerLoop type %d",
		    __LINE__ - 1, P_GetSerLoopLoopType (s));
	  }
      }
      break;

    case ST_IF:
      {
	IfStmt i = P_GetStmtIfStmt (stmt_core);
	Expr cond_expr = P_GetIfStmtCondExpr (i);
	Stmt else_block = P_GetIfStmtElseBlock (i);
	int cond_expr_lines = 0;
#if PRETTY
	int cond_expr_bytes = P_write_expr (NULL, cond_expr, indent + 1,
					    &cond_expr_lines);
#else
	int cond_expr_bytes = LINE_LENGTH;
#endif

	write_formatted (bytes, local_lines, lines,
			 P_write_token, out, IF, indent);

	/* Determine if we need to split the line. */
	if ((head_length + indent + bytes + cond_expr_bytes + \
	     tail_length >= LINE_LENGTH) || (cond_expr_lines > 0))
	  write_formatted_nli (bytes, local_lines, lines, out, indent + 1);

	write_formatted_lines (bytes, local_lines, lines,
			       P_write_expr, out, cond_expr, indent + 1);
	write_formatted_nli (bytes, local_lines, lines, out, indent);
	write_formatted (bytes, local_lines, lines,
			 P_write_token, out, THEN, indent);
	write_formatted_nli (bytes, local_lines, lines, out, indent + 1);
	write_formatted_lines (bytes, local_lines, lines,
			       P_write_stmt, out, P_GetIfStmtThenBlock (i),
			       indent + 1);

	if (else_block)
	  {
	    write_formatted_nli (bytes, local_lines, lines, out, indent);
	    write_formatted (bytes, local_lines, lines,
			     P_write_token, out, ELSE, indent);
	    write_formatted_nli (bytes, local_lines, lines, out, indent + 1);
	    write_formatted_lines (bytes, local_lines, lines,
				   P_write_stmt, out, else_block, indent + 1);
	  }
      }
      break;

    case ST_SWITCH:
      {
	SwitchStmt s = P_GetStmtSwitchStmt (stmt_core);
	Expr expression = P_GetSwitchStmtExpression (s);
	int expression_lines = 0;
#if PRETTY
	int expression_bytes = P_write_expr (NULL, expression, indent + 1,
					     &expression_lines);
#else
	int expression_bytes = LINE_LENGTH;
#endif

	write_formatted (bytes, local_lines, lines,
			 P_write_token, out, SWITCH, indent);

	/* Determine if we need to split the line. */
	if ((head_length + indent + bytes + expression_bytes + \
	     tail_length >= LINE_LENGTH) || (expression_lines > 0))
	  write_formatted_nli (bytes, local_lines, lines, out, indent + 1);

	write_formatted_lines (bytes, local_lines, lines,
			       P_write_expr, out, expression, indent + 1);
	write_formatted_nli (bytes, local_lines, lines, out, indent + 1);
	write_formatted_lines (bytes, local_lines, lines,
			       P_write_stmt, out,
			       P_GetSwitchStmtSwitchBody (s), indent + 1);
      }
      break;

    case ST_RETURN:
      {
	Expr ret = P_GetStmtRet (stmt_core);

	write_formatted (bytes, local_lines, lines,
			 P_write_token, out, RETURN, indent);

	if (ret)
	  {
	    int ret_lines = 0;
#if PRETTY
	    int ret_bytes = P_write_expr (NULL, ret, indent + 1, &ret_lines);
#else
	    int ret_bytes = LINE_LENGTH;
#endif

	    /* Determine if we need to split the line. */
	    if ((head_length + indent + bytes + ret_bytes + \
		 tail_length >= LINE_LENGTH) || (ret_lines > 0))
	      write_formatted_nli (bytes, local_lines, lines, out, indent + 1);

	    write_formatted_lines (bytes, local_lines, lines, 
				   P_write_expr, out, P_GetStmtRet (stmt_core),
				   indent + 1);
	  }
      }
      break;

    case ST_GOTO:
      {
	Identifier identifier = P_GetStmtGotoIdentifier (stmt_core);

	write_formatted (bytes, local_lines, lines,
			 P_write_token, out, GOTO, indent);
	write_formatted_lines (bytes, local_lines, lines,
			       P_write_identifier, out, identifier, indent);

	identifier = P_RemoveIdentifier (identifier);
      }
      break;

    case ST_PSTMT:
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_pstmt, out, P_GetStmtPstmt (stmt_core),
			     indent);
      break;

    case ST_ADVANCE:
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, ADVANCE, indent);
      write_formatted (bytes, local_lines, lines,
		       P_write_I_LIT, out,
		       P_GetAdvanceMarker (P_GetStmtAdvance (stmt_core)),
		       indent);
      break;

    case ST_AWAIT:
      {
	Await a = P_GetStmtAwait (stmt_core);

	write_formatted (bytes, local_lines, lines,
			 P_write_token, out, AWAIT, indent);
	write_formatted (bytes, local_lines, lines,
			 P_write_I_LIT, out, P_GetAwaitMarker (a), indent);
	write_formatted (bytes, local_lines, lines,
			 P_write_I_LIT, out, P_GetAwaitDistance (a), indent);
      }
      break;

    case ST_PARLOOP:
      {
	ParLoop p = P_GetStmtParLoop (stmt_core);
	
	switch (P_GetParLoopLoopType (p))
	  {
	  case LT_DOSERIAL:
	    write_formatted (bytes, local_lines, lines,
			     P_write_token, out, DOSERIAL, indent);
	    break;
	    
	  case LT_DOALL:
	    write_formatted (bytes, local_lines, lines,
			     P_write_token, out, DOALL, indent);
	    break;
	    
	  case LT_DOACROSS:
	    write_formatted (bytes, local_lines, lines,
			     P_write_token, out, DOACROSS, indent);
	    break;
	    
	  case LT_DOSUPER:
	    write_formatted (bytes, local_lines, lines,
			     P_write_token, out, DOSUPER, indent);
	    break;
	    
	  default:
	    P_punt ("write.c:P_write_stmt_core:%d Unknown ParLoop type %d",
		    __LINE__ - 1, P_GetParLoopLoopType (p));
	  }

	write_formatted_nli (bytes, local_lines, lines, out, indent);
	write_formatted (bytes, local_lines, lines,
			 P_write_token, out, INDEX, indent);
	write_formatted_lines (bytes, local_lines, lines,
			       P_write_parloop_index, out,
			       P_GetParLoopIterationVar (p), indent);
	write_formatted_nli (bytes, local_lines, lines, out, indent);
	write_formatted (bytes, local_lines, lines,
			 P_write_token, out, INIT, indent);
	write_formatted_lines (bytes, local_lines, lines,
			       P_write_expr, out, P_GetParLoopInitValue (p),
			       indent);
	write_formatted_nli (bytes, local_lines, lines, out, indent);
	write_formatted (bytes, local_lines, lines,
			 P_write_token, out, FINAL, indent);
	write_formatted_lines (bytes, local_lines, lines,
			       P_write_expr, out, P_GetParLoopFinalValue (p),
			       indent);
	write_formatted_nli (bytes, local_lines, lines, out, indent);
	write_formatted (bytes, local_lines, lines,
			 P_write_token, out, INC, indent);
	write_formatted_lines (bytes, local_lines, lines,
			       P_write_expr, out, P_GetParLoopIncrValue (p),
			       indent);
	write_formatted_nli (bytes, local_lines, lines, out, indent + 1);
	write_formatted_lines (bytes, local_lines, lines,
			       P_write_pstmt, out, P_GetParLoopPstmt (p),
			       indent + 1);
      }
      break;

    case ST_MUTEX:
      {
	Mutex m = P_GetStmtMutex (stmt_core);

	write_formatted (bytes, local_lines, lines,
			 P_write_token, out, MUTEX, indent);
	write_formatted_lines (bytes, local_lines, lines,
			       P_write_expr, out, P_GetMutexExpression (m),
			       indent);
	write_formatted_nli (bytes, local_lines, lines, out, indent + 1);
	write_formatted_lines (bytes, local_lines, lines,
			       P_write_stmt, out, P_GetMutexStatement (m),
			       indent + 1);
      }
      break;

    case ST_COBEGIN:
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, COBEGIN, indent);
      write_formatted_nli (bytes, local_lines, lines, out, indent + 1);
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_stmt, out,
			     P_GetCobeginStatements \
			       (P_GetStmtCobegin (stmt_core)), indent + 1);
      break;

    case ST_ASM:
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_asm_stmt, out,
			     P_GetStmtAsmStmt (stmt_core), indent);
      break;

    case ST_BREAK:
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, BREAK, indent);
      break;

    case ST_CONT:
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, CONTINUE, indent);
      break;

    case ST_NOOP:
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, KW_NULL, indent);
      break;

    default:
      P_punt ("write.c:P_write_stmt_core:%d Unknown Stmt type %d", __LINE__,
	      P_GetStmtType (stmt_core));
    }

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the stmt_list rule to disk.
 *
 * \param out
 *  the file to write.
 * \param stmt_list
 *  the stmt_list ::Stmt to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the stmt_list rule to disk.
 */
int
P_write_stmt_list (FILE *out, Stmt stmt_list, int indent, int *lines)
{
  int bytes = 0;
  int local_lines = 0;

  while (stmt_list)
    {
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_stmt, out, stmt_list, indent);

      if ((stmt_list = P_GetStmtLexNext (stmt_list)))
	write_formatted_nli (bytes, local_lines, lines, out, indent);
    }

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the stmt_list_container rule to disk.
 *
 * \param out
 *  the file to write.
 * \param stmt_list_container
 *  the stmt_list_container ::Stmt to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the stmt_list_container rule to disk.
 */
int
P_write_stmt_list_container (FILE *out, Stmt stmt_list_container, int indent,
			     int *lines)
{
  int bytes = 0;
  int local_lines = 0;

  write_formatted (bytes, local_lines, lines, P_write_token, out, '(', indent);
  write_formatted_nli (bytes, local_lines, lines, out, indent + 1);
  write_formatted_lines (bytes, local_lines, lines,
			 P_write_stmt_list, out, stmt_list_container,
			 indent + 1);
  write_formatted_nli (bytes, local_lines, lines, out, indent);
  write_formatted (bytes, local_lines, lines, P_write_token, out, ')', indent);

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the structqual rule to disk.
 *
 * \param out
 *  the file to write.
 * \param struct_qual
 *  the _StructQual to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the struct_qual rule to disk.  SQ_COMPARING is not written as it
 * should not be preserved between modules.
 */
int
P_write_struct_qual (FILE *out, _StructQual struct_qual, int indent,
		     int *lines)
{
  int bytes = 0;
  int local_lines = 0;

  if (struct_qual & SQ_EMPTY)
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, EMPTY, indent);
    }
  if (struct_qual & SQ_INCOMPLETE)
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, INCOMPLETE, indent);
    }
  if (struct_qual & SQ_UNNAMED)
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, UNNAMED, indent);
    }
  if (struct_qual & SQ_LINKMULTI)
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, LINKMULTI, indent);
    }

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the struct_qual_list rule to disk.
 *
 * \param out
 *  the file to write.
 * \param struct_qual_list
 *  the _StructQual to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the struct_qual_list rule to disk.
 */
int
P_write_struct_qual_list (FILE *out, _StructQual struct_qual_list, int indent,
			  int *lines)
{
  int bytes = 0;
  int local_lines = 0;

  write_formatted_lines (bytes, local_lines, lines,
			 P_write_struct_qual, out, struct_qual_list, indent);

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the symbol_table rule to disk.
 *
 * \param out
 *  the file to write.
 * \param symbol_table
 *  the ::SymbolTable to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the symbol_table rule to disk.
 */
int
P_write_symbol_table (FILE *out, SymbolTable symbol_table, int indent,
		      int *lines)
{
  int bytes = 0;
  int local_lines = 0;

  write_formatted (bytes, local_lines, lines, P_write_token, out, '(', indent);

  if (symbol_table)
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, SYMBOL_TABLE, indent);
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, NUM_ENTRIES, indent);
      write_formatted (bytes, local_lines, lines,
		       P_write_I_LIT, out,
		       P_GetSymbolTableNumFiles (symbol_table), indent);
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_symbol_table_flags_list, out,
			     P_GetSymbolTableFlags (symbol_table), indent);
    }
  else
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, SYMBOL_TABLE, indent);
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, END, indent);
    }

  write_formatted (bytes, local_lines, lines, P_write_token, out, ')', indent);

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the symbol_table_entry rule to disk.
 *
 * \param out
 *  the file to write.
 * \param symbol_table_entry
 *  the ::SymTabEntry to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the symbol_table_entry rule to disk.
 */
int
P_write_symbol_table_entry (FILE *out, SymTabEntry symbol_table_entry,
			    int indent, int *lines)
{
  Identifier identifier;
  Scope scope;
  Pragma pragma;
  int bytes = 0;
  int local_lines = 0;

  write_formatted (bytes, local_lines, lines, P_write_token, out, '(', indent);
  write_formatted (bytes, local_lines, lines, P_write_token, out, SYM, indent);

  if (symbol_table_entry)
    {
      identifier = P_GetSymTabEntryIdentifier (symbol_table_entry);
      scope = P_GetSymTabEntryScope (symbol_table_entry);
      pragma = P_GetSymTabEntryPragma (symbol_table_entry);

      /* If any write handlers are defined, save the ext field to the pragma
       * list. */
      if (Handlers[ES_SYMTABENTRY])
	P_ExtWrite (ES_SYMTABENTRY, (void *)symbol_table_entry);
      
      if (P_GetSymTabEntryType (symbol_table_entry) == ET_BLOCK)
	{
	  write_formatted (bytes, local_lines, lines,
			   P_write_token, out, BLOCK, indent);
	  write_formatted_lines (bytes, local_lines, lines,
				 P_write_key, out,
				 P_GetSymTabEntryBlockStart \
				   (symbol_table_entry), indent);
	  write_formatted (bytes, local_lines, lines,
			   P_write_I_LIT, out,
			   P_GetSymTabEntryBlockSize (symbol_table_entry),
			   indent);
	}
      else
	{
	  write_formatted_lines (bytes, local_lines, lines,
				 P_write_identifier, out, identifier, indent);
	  write_formatted_lines (bytes, local_lines, lines,
				 P_write_key, out,
				 P_GetSymTabEntryScopeKey (symbol_table_entry),
				 indent);
	  write_formatted_lines (bytes, local_lines, lines,
				 P_write_entry_type, out,
				 P_GetSymTabEntryType (symbol_table_entry),
				 indent);
	  
	  if (scope)
	    {
	      write_formatted_nli (bytes, local_lines, lines, out, indent + 1);
	      write_formatted_lines (bytes, local_lines, lines,
				     P_write_scope, out, scope, indent + 1);
	    }
	  
	  write_formatted (bytes, local_lines, lines,
			   P_write_token, out, OFFSET, indent);
	  write_formatted (bytes, local_lines, lines,
			   P_write_offset, out,
			   &P_GetSymTabEntryOffset (symbol_table_entry),
			   indent);
	  
	  if (pragma)
	    {
	      write_formatted_nli (bytes, local_lines, lines, out, indent);
	      write_formatted_lines (bytes, local_lines, lines,
				     P_write_pragma_list, out, pragma, indent);
	    }
	}

      identifier = P_RemoveIdentifier (identifier);
    }
  else
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, END, indent);
    }

  write_formatted (bytes, local_lines, lines, P_write_token, out, ')', indent);

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the symbol_table_flags rule to disk.
 *
 * \param out
 *  the file to write.
 * \param symbol_table_flags
 *  the _STFlags to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  return the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the symbol_table_flags rule to disk.  Only STF_LINKED is written
 * at this time.
 */
int
P_write_symbol_table_flags (FILE *out, _STFlags symbol_table_flags, int indent,
			    int *lines)
{
  int bytes = 0;
  int local_lines = 0;

  if (symbol_table_flags & STF_LINKED)
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, LINKED, indent);
    }

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the symbol_table_flags_list rule to disk.
 *
 * \param out
 *  the file to write.
 * \param symbol_table_flags_list
 *  the _STFlags to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  return the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the symbol_table_flags_list rule to disk.
 */
int
P_write_symbol_table_flags_list (FILE *out, _STFlags symbol_table_flags_list,
				 int indent, int *lines)
{
  int bytes = 0;
  int local_lines = 0;

  write_formatted_lines (bytes, local_lines, lines,
			 P_write_symbol_table_flags, out,
			 symbol_table_flags_list, indent);

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the type_dcl rule to disk.
 *
 * \param out
 *  the file to write.
 * \param type_dcl
 *  the type_dcl ::Dcl to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the type_dcl rule to disk.
 */
int
P_write_type_dcl (FILE *out, Dcl type_dcl, int indent, int *lines)
{
  int bytes = 0;
  int local_lines = 0;

  write_formatted (bytes, local_lines, lines, P_write_token, out, '(', indent);
  write_formatted (bytes, local_lines, lines, P_write_token, out, DEF, indent);
  write_formatted_nli (bytes, local_lines, lines, out, indent + 1);
  write_formatted_lines (bytes, local_lines, lines,
			 P_write_type_definition, out, type_dcl, indent + 1);
  write_formatted_nli (bytes, local_lines, lines, out, indent);
  write_formatted (bytes, local_lines, lines, P_write_token, out, ')', indent);

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the type_definition rule to disk.
 *
 * \param out
 *  the file to write.
 * \param type_definition
 *  the type_definition ::Dcl to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of lines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the type_definition rule to disk.
 */
int
P_write_type_definition (FILE *out, Dcl type_definition, int indent,
			 int *lines)
{
  int bytes = 0;
  int local_lines = 0;

  switch (P_GetDclType (type_definition))
    {
    case TT_TYPE:
      /* A TT_TYPE can either be a type declaration (defines a type)
       * or a typedef (names a type). */
      if (P_GetTypeDclBasicType (P_GetDclTypeDcl (type_definition)) == \
	  BT_TYPEDEF_E)
	{
	  /* Explicit typedef */
	  TypeDcl t = P_GetDclTypeDcl (type_definition);
	  Identifier identifier = P_GetTypeDclIdentifier (t);
	  Position position = P_GetTypeDclPosition (t);
	  Pragma pragma;

	  /* If any write handlers are defined, save the ext field to the
	   * pragma list. */
	  if (Handlers[ES_TYPE])
	    P_ExtWrite (ES_TYPE, (void *)t);

	  pragma = P_GetTypeDclPragma (t);
	  
	  write_formatted (bytes, local_lines, lines,
			   P_write_token, out, '(', indent);
	  write_formatted (bytes, local_lines, lines,
			   P_write_token, out, TYPEDEF, indent);
	  write_formatted_lines (bytes, local_lines, lines,
				 P_write_identifier, out, identifier, indent);
	  write_formatted_lines (bytes, local_lines, lines,
				 P_write_type_qual_list, out,
				 P_GetTypeDclQualifier (t), indent);
	  write_formatted_lines (bytes, local_lines, lines,
				 P_write_alignment, out,
				 P_GetTypeDclAlignment (t), indent);
	  write_formatted (bytes, local_lines, lines,
			   P_write_token, out, REFS, indent);
	  write_formatted (bytes, local_lines, lines,
			   P_write_I_LIT, out, P_GetTypeDclRefs (t), indent);
	  write_formatted_lines (bytes, local_lines, lines,
				 P_write_key, out,
				 P_GetTypeDclType (t), indent);
 
	  if (pragma)
	    {
	      write_formatted_nli (bytes, local_lines, lines, out, indent);
	      write_formatted_lines (bytes, local_lines, lines,
				     P_write_pragma_list, out, pragma, indent);
	    }

	  if (P_ValidPosition (position))
	    {
	      write_formatted_nli (bytes, local_lines, lines, out, indent);
	      write_formatted_lines (bytes, local_lines, lines,
				     P_write_position, out, position, indent);
	    }

	  write_formatted (bytes, local_lines, lines,
			   P_write_token, out, ')', indent);

	  position = P_RemovePosition (position);
	  identifier = P_RemoveIdentifier (identifier);
	}
      else
	{
	  /* Type declaration (defines a type) */
	  write_formatted (bytes, local_lines, lines,
			   P_write_token, out, '(', indent);
	  write_formatted (bytes, local_lines, lines,
			   P_write_token, out, TYPE, indent);
	  write_formatted_lines (bytes, local_lines, lines,
				 P_write_type_spec, out,
				 P_GetDclTypeDcl (type_definition), indent);
	  write_formatted (bytes, local_lines, lines,
			   P_write_token, out, ')', indent);
	}
      break;

    case TT_STRUCT:
      {
	StructDcl s = P_GetDclStructDcl (type_definition);
	Identifier identifier = P_GetStructDclIdentifier (s);
	Position position = P_GetStructDclPosition (s);
	Field fields = P_GetStructDclFields (s);
	_StructQual qualifier = P_GetStructDclQualifier (s);
	Pragma pragma;

	/* If any write handlers are defined, save the ext field to the
	 * pragma list. */
	if (Handlers[ES_STRUCT])
	  P_ExtWrite (ES_STRUCT, (void *)s);

	pragma = P_GetStructDclPragma (s);

	write_formatted (bytes, local_lines, lines,
			 P_write_token, out, '(', indent);
	write_formatted (bytes, local_lines, lines,
			 P_write_token, out, STRUCT, indent);
	write_formatted_lines (bytes, local_lines, lines,
			       P_write_identifier, out, identifier, indent);

	if (fields)
	  {
	    write_formatted_nli (bytes, local_lines, lines, out, indent + 1);
	    write_formatted_lines (bytes, local_lines, lines,
				   P_write_field_list, out, fields,
				   indent + 1);
	  }

	write_formatted_lines (bytes, local_lines, lines,
			       P_write_struct_qual_list, out, qualifier,
			       indent + 1);

	if (pragma)
	  {
	    write_formatted_nli (bytes, local_lines, lines, out, indent + 1);
	    write_formatted_lines (bytes, local_lines, lines,
				   P_write_pragma_list, out, pragma,
				   indent + 1);
	  }

	if (P_ValidPosition (position))
	  {
	    write_formatted_nli (bytes, local_lines, lines, out, indent + 1);
	    write_formatted_lines (bytes, local_lines, lines,
				   P_write_position, out, position,
				   indent + 1);
	  }

	if (local_lines > 0)
	  write_formatted_nli (bytes, local_lines, lines, out, indent);

	write_formatted (bytes, local_lines, lines,
			 P_write_token, out, ')', indent);

	position = P_RemovePosition (position);
	identifier = P_RemoveIdentifier (identifier);
      }
      break;

    case TT_UNION:
      {
	UnionDcl u = P_GetDclUnionDcl (type_definition);
	Identifier identifier = P_GetUnionDclIdentifier (u);
	Position position = P_GetUnionDclPosition (u);
	Field fields = P_GetUnionDclFields (u);
	_StructQual qualifier = P_GetUnionDclQualifier (u);
	Pragma pragma;

	/* If any write handlers are defined, save the ext field to the
	 * pragma list. */
	if (Handlers[ES_UNION])
	  P_ExtWrite (ES_UNION, (void *)u);

	pragma = P_GetUnionDclPragma (u);

	write_formatted (bytes, local_lines, lines,
			 P_write_token, out, '(', indent);
	write_formatted (bytes, local_lines, lines,
			 P_write_token, out, UNION, indent);
	write_formatted_lines (bytes, local_lines, lines,
			       P_write_identifier, out, identifier, indent);

	if (fields)
	  {
	    write_formatted_nli (bytes, local_lines, lines, out, indent + 1);
	    write_formatted_lines (bytes, local_lines, lines,
				   P_write_field_list, out, fields,
				   indent + 1);
	  }

	write_formatted_lines (bytes, local_lines, lines,
			       P_write_struct_qual_list, out, qualifier,
			       indent + 1);

	if (pragma)
	  {
	    write_formatted_nli (bytes, local_lines, lines, out, indent + 1);
	    write_formatted_lines (bytes, local_lines, lines,
				   P_write_pragma_list, out, pragma,
				   indent + 1);
	  }

	if (P_ValidPosition (position))
	  {
	    write_formatted_nli (bytes, local_lines, lines, out, indent + 1);
	    write_formatted_lines (bytes, local_lines, lines,
				   P_write_position, out, position,
				   indent + 1);
	  }

	if (local_lines > 0)
	  write_formatted_nli (bytes, local_lines, lines, out, indent);

	write_formatted (bytes, local_lines, lines,
			 P_write_token, out, ')', indent);

	position = P_RemovePosition (position);
	identifier = P_RemoveIdentifier (identifier);
      }
      break;

    case TT_ENUM:
      {
	EnumDcl e = P_GetDclEnumDcl (type_definition);
	Identifier identifier = P_GetEnumDclIdentifier (e);
	Position position = P_GetEnumDclPosition (e);
	EnumField fields = P_GetEnumDclFields (e);
	Pragma pragma;

	/* If any write handlers are defined, save the ext field to the
	 * pragma list. */
	if (Handlers[ES_ENUM])
	  P_ExtWrite (ES_ENUM, (void *)e);

	pragma = P_GetEnumDclPragma (e);

	write_formatted (bytes, local_lines, lines,
			 P_write_token, out, '(', indent);
	write_formatted (bytes, local_lines, lines,
			 P_write_token, out, ENUM, indent);
	write_formatted_lines (bytes, local_lines, lines,
			       P_write_identifier, out, identifier, indent);

	if (fields)
	  {
	    write_formatted_nli (bytes, local_lines, lines, out, indent + 1);
	    write_formatted_lines (bytes, local_lines, lines,
				   P_write_enum_field_list, out, fields,
				   indent + 1);
	  }

	if (pragma)
	  {
	    write_formatted_nli (bytes, local_lines, lines, out, indent + 1);
	    write_formatted_lines (bytes, local_lines, lines,
				   P_write_pragma_list, out, pragma,
				   indent + 1);
	  }

	if (P_ValidPosition (position))
	  {
	    write_formatted_nli (bytes, local_lines, lines, out, indent + 1);
	    write_formatted_lines (bytes, local_lines, lines,
				   P_write_position, out, position,
				   indent + 1);
	  }

	if (local_lines > 0)
	  write_formatted_nli (bytes, local_lines, lines, out, indent);

	write_formatted (bytes, local_lines, lines,
			 P_write_token, out, ')', indent);

	position = P_RemovePosition (position);
	identifier = P_RemoveIdentifier (identifier);
      }
      break;

    case TT_FUNC:
    case TT_VAR:
    case TT_ASM:
    case TT_SYMBOLTABLE:
    case TT_IPSYMTABENT:
    case TT_SYMTABENTRY:
      P_punt ("write.c:P_write_type_definition:%d Invalid Dcl type %d",
	      __LINE__ - 1, P_GetDclType (type_definition));

    default:
      P_punt ("write.c:P_write_type_definition:%d Unknown Dcl type %d",
	      __LINE__ - 1, P_GetDclType (type_definition));
    }

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the type_list rule to disk.
 *
 * \param out
 *  the file to write.
 * \param type_list
 *  the ::TypeList to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the type_list rule to disk.
 */
int
P_write_type_list (FILE *out, TypeList type_list, int indent, int *lines)
{
  int bytes = 0;
  int local_lines = 0;
  TypeDcl type;

  List_start (type_list);
  type = (TypeDcl)List_next (type_list);

  while (type)
    {
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_type_spec, out, type, indent);

      if ((type = (TypeDcl)List_next (type_list)))
	write_formatted_nli (bytes, local_lines, lines, out, indent);
    }

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the type_list_container rule to disk.
 *
 * \param out
 *  the file to write.
 * \param type_list_container
 *  the type_list_container ::TypeList to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of lines written.
 *
 * Writes the type_list_container rule to disk.
 */
int
P_write_type_list_container (FILE *out, TypeList type_list_container,
			     int indent, int *lines)
{
  int bytes = 0;
  int local_lines = 0;

  write_formatted (bytes, local_lines, lines, P_write_token, out, '(', indent);

  if (type_list_container)
    write_formatted_nli (bytes, local_lines, lines, out, indent + 1);

  write_formatted_lines (bytes, local_lines, lines,
			 P_write_type_list, out, type_list_container,
			 indent + 1);

  if (local_lines > 0)
    write_formatted_nli (bytes, local_lines, lines, out, indent);

  write_formatted (bytes, local_lines, lines, P_write_token, out, ')', indent);

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the type_qual rule to disk.
 *
 * \param out
 *  the file to write.
 * \param type_qual
 *  the ::_TypeQual to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the type_qual rule to disk.
 */
int
P_write_type_qual (FILE *out, _TypeQual type_qual, int indent, int *lines)
{
  int bytes = 0;
  int local_lines = 0;

  if (type_qual & TY_CONST)
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, CONST, indent);
    }
  if (type_qual & TY_VOLATILE)
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, VOLATILE, indent);
    }
  if (type_qual & TY_SYNC)
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, SYNC, indent);
    }
  if (type_qual & TY_IMPLICIT)
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, IMPLICIT, indent);
    }
  if (type_qual & TY_DEFAULT)
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, DEFAULT, indent);
    }
  if (type_qual & TY_EXP_ALIGN)
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, EXPLICIT_ALIGNMENT, indent);
    }
  if (type_qual & TY_UNNAMED)
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, UNNAMED, indent);
    }

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the type_qual_list rule to disk.
 *
 * \param out
 *  the file to write.
 * \param type_qual_list
 *  the type_qual_list ::_TypeQual to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the type_qual_list rule to disk.
 */
int
P_write_type_qual_list (FILE *out, _TypeQual type_qual_list, int indent,
			int *lines)
{
  int bytes = 0;
  int local_lines = 0;

  write_formatted_lines (bytes, local_lines, lines,
			 P_write_type_qual, out, type_qual_list, indent);

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the type_spec rule to disk.
 *
 * \param out
 *  the file to write.
 * \param type_spec
 *  the type_spec ::TypeDcl to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the type_spec rule to disk.
 */
int
P_write_type_spec (FILE *out, TypeDcl type_spec, int indent, int *lines)
{
  _BasicType type_basic_type = P_GetTypeDclBasicType (type_spec);
  Position position = P_GetTypeDclPosition (type_spec);
  Pragma pragma;
  int bytes = 0;
  int local_lines = 0;

  /* If any write handlers are defined, write the ext field to the pragma
   * list. */

  if (Handlers[ES_TYPE])
    P_ExtWrite (ES_TYPE, (void *)type_spec);

  pragma = P_GetTypeDclPragma (type_spec);

  write_formatted (bytes, local_lines, lines, P_write_token, out, '(', indent);
  if (type_basic_type & BT_NAMED_TYPE)
    {
      Identifier identifier = P_GetTypeDclIdentifier (type_spec);

      write_formatted_lines (bytes, local_lines, lines,
			     P_write_named_basic_type, out, type_basic_type,
			     indent);
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_identifier, out, identifier, indent);

      identifier = P_RemoveIdentifier (identifier);
    }
  else
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, KEY, indent);
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_key, out, P_GetTypeDclKey (type_spec),
			     indent);
    }

  write_formatted_lines (bytes, local_lines, lines,
			 P_write_type_qual_list, out,
			 P_GetTypeDclQualifier (type_spec), indent);

  if (type_basic_type & BT_ARRAY)
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, ARRAY, indent);
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, TYPE, indent);
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_key, out, P_GetTypeDclType (type_spec),
			     indent);
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, DIM, indent);
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_expr_container, out,
			     P_GetTypeDclArraySize (type_spec), indent);
    }
  else if (type_basic_type & BT_FUNC)
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, FUNCTION, indent);
      write_formatted (bytes, local_lines, lines,
		       P_write_ST_LIT, out, P_GetTypeDclName (type_spec),
		       indent);
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, RETURN, indent);
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, TYPE, indent);
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_key, out, P_GetTypeDclType (type_spec),
			     indent);
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_param_list, out,
			     P_GetTypeDclParam (type_spec), indent);
    }
  else if (type_basic_type & BT_POINTER)
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, POINTER, indent);
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, TYPE, indent);
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_key, out, P_GetTypeDclType (type_spec),
			     indent);
    }
  else if (type_basic_type & BT_NAMED_TYPE)
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, TYPE, indent);
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_key, out, P_GetTypeDclType (type_spec),
			     indent);
    }
  else if (type_basic_type & BT_TYPEDEF)
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, TYPEDEF, indent);
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, TYPE, indent);
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_key, out, P_GetTypeDclType (type_spec),
			     indent);
    }
  else
    {
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_basic_type_list, out, type_basic_type,
			     indent);
    }

  write_formatted_lines (bytes, local_lines, lines,
			 P_write_size, out, P_GetTypeDclSize (type_spec),
			 indent);
  write_formatted_lines (bytes, local_lines, lines,
			 P_write_alignment, out,
			 P_GetTypeDclAlignment (type_spec), indent);
  write_formatted (bytes, local_lines, lines,
		   P_write_token, out, REFS, indent);
  write_formatted (bytes, local_lines, lines,
		   P_write_I_LIT, out, P_GetTypeDclRefs (type_spec), indent);

  if (pragma)
    {
      write_formatted_nli (bytes, local_lines, lines, out, indent);
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_pragma_list, out, pragma, indent);
    }

  if (P_ValidPosition (position))
    {
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_position, out, position, indent);
    }

  write_formatted (bytes, local_lines, lines, P_write_token, out, ')', indent);

  position = P_RemovePosition (position);

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the var_dcl rule to disk.
 *
 * \param out
 *  the file to write.
 * \param var_dcl
 *  the ::VarDcl to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the var_dcl rule to disk.
 */
int
P_write_var_dcl (FILE *out, VarDcl var_dcl, int indent, int *lines)
{
  Identifier identifier = P_GetVarDclIdentifier (var_dcl);
  Position position = P_GetVarDclPosition (var_dcl);
  _VarQual qualifier = P_GetVarDclQualifier (var_dcl);
  Pragma pragma;
  Init init = P_GetVarDclInit (var_dcl);
  int bytes = 0;
  int local_lines = 0;
  int init_lines = 0, position_lines = 0;
  int init_bytes = 0;
#if PRETTY
  int position_bytes = P_write_position (NULL, position, indent + 1,
					 &position_lines);
  int head_length = P_write_token (NULL, '(', indent);
  int tail_length = P_write_token (NULL, ')', indent);
#else
  int position_bytes = LINE_LENGTH;
  int head_length = LINE_LENGTH;
  int tail_length = LINE_LENGTH;
#endif
  int split_before_init = 0;
  int split_before_position = 0;

  /* If any write handlers are defined, save the ext field to the pragma
   * list. */
  if (Handlers[ES_VAR])
    P_ExtWrite (ES_VAR, (void *)var_dcl);

  pragma = P_GetVarDclPragma (var_dcl);

  if (init)
#if PRETTY
    init_bytes = P_write_initializer_list_container (NULL, init, indent + 1,
						     &init_lines);
#else
    init_bytes = LINE_LENGTH;
#endif

  write_formatted (bytes, local_lines, lines, P_write_token, out, '(', indent);
  write_formatted_lines (bytes, local_lines, lines,
			 P_write_identifier, out, identifier, indent);
  write_formatted (bytes, local_lines, lines,
		   P_write_token, out, TYPE, indent);
  write_formatted_lines (bytes, local_lines, lines,
			 P_write_key, out, P_GetVarDclType (var_dcl), indent);
  write_formatted_lines (bytes, local_lines, lines,
			 P_write_var_qual_list, out, qualifier, indent);

  /* Determine where we need to split the line. If there is a pragma,
   * always print the pragma and position on their own lines. */
  if ((head_length + indent + bytes + init_bytes + \
       position_bytes + tail_length >= LINE_LENGTH) || \
      (position_lines > 0) || pragma != NULL)
    {
      split_before_position = 1;

      if ((head_length + indent + bytes + init_bytes + \
	   tail_length >= LINE_LENGTH) || (init_lines > 0))
	{
	  split_before_init = 1;
	}
    }

  if (init)
    {
      if (split_before_init)
	write_formatted_nli (bytes, local_lines, lines, out, indent + 1);

      write_formatted_lines (bytes, local_lines, lines,
			     P_write_initializer_list_container, out, init, 
			     indent + 1);
    }

  if (pragma)
    {
      write_formatted_nli (bytes, local_lines, lines, out, indent + 1);
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_pragma_list, out,
			     P_GetVarDclPragma (var_dcl), indent + 1);
    }

  if (P_ValidPosition (position))
    {
      if (split_before_position)
	write_formatted_nli (bytes, local_lines, lines, out, indent + 1);

      write_formatted_lines (bytes, local_lines, lines,
			     P_write_position, out, position, indent + 1);
    }

  if (local_lines > 0)
    write_formatted_nli (bytes, local_lines, lines, out, indent);

  write_formatted (bytes, local_lines, lines, P_write_token, out, ')', indent);

  position = P_RemovePosition (position);
  identifier = P_RemoveIdentifier (identifier);

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the var_dcl_list rule to disk.
 *
 * \param out
 *  the file to write.
 * \param var_dcl_list
 *  the var_dcl_list ::VarList to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the var_dcl_list rule to disk.
 */
int
P_write_var_dcl_list (FILE *out, VarList var_dcl_list, int indent, int *lines)
{
  int bytes = 0;
  int local_lines = 0;
  VarDcl var_dcl;

  List_start (var_dcl_list);
  var_dcl = (VarDcl)List_next (var_dcl_list);

  while (var_dcl)
    {
      write_formatted_lines (bytes, local_lines, lines,
			     P_write_var_dcl, out, var_dcl, indent);

      if ((var_dcl = (VarDcl)List_next (var_dcl_list)))
	write_formatted_nli (bytes, local_lines, lines, out, indent);
    }

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the var_dcl_list_container rule to disk.
 *
 * \param out
 *  the file to write.
 * \param var_dcl_list_container
 *  the var_dcl_list_container ::VarList to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of lines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the var_dcl_list_container rule to disk.
 */
int
P_write_var_dcl_list_container (FILE *out, VarList var_dcl_list_container,
				int indent, int *lines)
{
  int bytes = 0;
  int local_lines = 0;

  write_formatted (bytes, local_lines, lines, P_write_token, out, '(', indent);

  if (var_dcl_list_container)
    write_formatted_nli (bytes, local_lines, lines, out, indent + 1);

  write_formatted_lines (bytes, local_lines, lines,
			 P_write_var_dcl_list, out, var_dcl_list_container,
			 indent + 1);

  if (local_lines > 0)
    write_formatted_nli (bytes, local_lines, lines, out, indent);

  write_formatted (bytes, local_lines, lines, P_write_token, out, ')', indent);

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the var_qual rule to disk.
 *
 * \param out
 *  the file to write.
 * \param var_qual
 *  the _VarQual to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the var_qual rule to disk.
 */
int
P_write_var_qual (FILE *out, _VarQual var_qual, int indent, int *lines)
{
  int bytes = 0;
  int local_lines = 0;

  if (var_qual & VQ_DEFINED)
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, DEFINED, indent);
    }
  if (var_qual & VQ_COMMON)
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, COMMON, indent);
    }
  if (var_qual & VQ_REGISTER)
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, REGISTER, indent);
    }
  if (var_qual & VQ_AUTO)
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, AUTO, indent);
    }
  if (var_qual & VQ_STATIC)
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, STATIC, indent);
    }
  if (var_qual & VQ_EXTERN)
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, EXTERN, indent);
    }
  if (var_qual & VQ_GLOBAL)
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, GLOBAL, indent);
    }
  if (var_qual & VQ_PARAMETER)
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, PARAMETER, indent);
    }
  if (var_qual & VQ_IMPLICIT)
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, IMPLICIT, indent);
    }
  if (var_qual & VQ_CDECL)
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, CDECL, indent);
    }
  if (var_qual & VQ_STDCALL)
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, STDCALL, indent);
    }
  if (var_qual & VQ_FASTCALL)
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, FASTCALL, indent);
    }
  if (var_qual & VQ_WEAK)
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, WEAK, indent);
    }
  if (var_qual & VQ_COMDAT)
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, COMDAT, indent);
    }
  if (var_qual & VQ_CONSTRUCTOR)
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, CONSTRUCTOR, indent);
    }
  if (var_qual & VQ_DESTRUCTOR)
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, DESTRUCTOR, indent);
    }
  if (var_qual & VQ_APP_ELLIPSIS)
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, ELLIPSIS, indent);
    }
  if (var_qual & VQ_OLD_PARAM)
    {
      write_formatted (bytes, local_lines, lines,
		       P_write_token, out, OLD_PARAM, indent);
    }

  if (lines) *lines = local_lines;
  return (bytes);
}

/*! \brief Writes the var_qual_list rule to disk.
 *
 * \param out
 *  the file to write.
 * \param var_qual_list
 *  the _VarQual to write.
 * \param indent
 *  the indent level.
 * \param lines
 *  returns the number of newlines written.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the var_qual_list rule to disk.
 */
int
P_write_var_qual_list (FILE *out, _VarQual var_qual_list, int indent,
		       int *lines)
{
  int bytes = 0;
  int local_lines = 0;

  write_formatted_lines (bytes, local_lines, lines,
			 P_write_var_qual, out, var_qual_list, indent);

  if (lines) *lines = local_lines;
  return (bytes);
}

/* Functions to write terminal symbols. */

/*! \brief Writes a token to disk.
 *
 * \param out
 *  the file to write.
 * \param token
 *  the token to write.
 * \param indent
 *  the indent level.
 *
 * \return
 *  The number of bytes written
 *
 * Writes a token to disk.  If \a out is null, this function returns the
 * number of bytes it would have written.
 */
int
P_write_token (FILE *out, long token, int indent)
{
  int bytes = 0;

  switch (token)
    {
    case ADD:
      if (out)
	fprintf (out, "ADD");
      bytes += strlen ("ADD");
      break;
    case ADDR:
      if (out)
	fprintf (out, "ADDR");
      bytes += strlen ("ADDR");
      break;
    case ADVANCE:
      if (out)
	fprintf (out, "ADVANCE");
      bytes += strlen ("ADVANCE");
      break;
    case ALIGNMENT:
      if (out)
	fprintf (out, "ALIGNMENT");
      bytes += strlen ("ALIGNMENT");
      break;
    case AND:
      if (out)
	fprintf (out, "AND");
      bytes += strlen ("AND");
      break;
    case ARRAY:
      if (out)
	fprintf (out, "ARRAY");
      bytes += strlen ("ARRAY");
      break;
    case ARROW:
      if (out)
	fprintf (out, "ARROW");
      bytes += strlen ("ARROW");
      break;
    case ASM:
      if (out)
	fprintf (out, "ASM");
      bytes += strlen ("ASM");
      break;
    case ASMOPRD:
      if (out)
	fprintf (out, "ASMOPRD");
      bytes += strlen ("ASMOPRD");
      break;
    case ASSIGN:
      if (out)
	fprintf (out, "ASSIGN");
      bytes += strlen ("ASSIGN");
      break;
    case AUTO:
      if (out)
	fprintf (out, "AUTO");
      bytes += strlen ("AUTO");
      break;
    case AWAIT:
      if (out)
	fprintf (out, "AWAIT");
      bytes += strlen ("AWAIT");
      break;
    case A_ADD:
      if (out)
	fprintf (out, "A_ADD");
      bytes += strlen ("A_ADD");
      break;
    case A_AND:
      if (out)
	fprintf (out, "A_AND");
      bytes += strlen ("A_AND");
      break;
    case A_DIV:
      if (out)
	fprintf (out, "A_DIV");
      bytes += strlen ("A_DIV");
      break;
    case A_LSHFT:
      if (out)
	fprintf (out, "A_LSHFT");
      bytes += strlen ("A_LSHFT");
      break;
    case A_MOD:
      if (out)
	fprintf (out, "A_MOD");
      bytes += strlen ("A_MOD");
      break;
    case A_MUL:
      if (out)
	fprintf (out, "A_MUL");
      bytes += strlen ("A_MUL");
      break;
    case A_OR:
      if (out)
	fprintf (out, "A_OR");
      bytes += strlen ("A_OR");
      break;
    case A_RSHFT:
      if (out)
	fprintf (out, "A_RSHFT");
      bytes += strlen ("A_RSHFT");
      break;
    case A_SUB:
      if (out)
	fprintf (out, "A_SUB");
      bytes += strlen ("A_SUB");
      break;
    case A_XOR:
      if (out)
	fprintf (out, "A_XOR");
      bytes += strlen ("A_XOR");
      break;
    case BITFIELD:
      if (out)
	fprintf (out, "BITFIELD");
      bytes += strlen ("BITFIELD");
      break;
    case BLOCK:
      if (out)
	fprintf (out, "BLOCK");
      bytes += strlen ("BLOCK");
      break;
    case BREAK:
      if (out)
	fprintf (out, "BREAK");
      bytes += strlen ("BREAK");
      break;
    case CALL:
      if (out)
	fprintf (out, "CALL");
      bytes += strlen ("CALL");
      break;
    case CASE:
      if (out)
	fprintf (out, "CASE");
      bytes += strlen ("CASE");
      break;
    case CAST:
      if (out)
	fprintf (out, "CAST");
      bytes += strlen ("CAST");
      break;
    case CDECL:
      if (out)
	fprintf (out, "CDECL");
      bytes += strlen ("CDECL");
      break;
    case CHAR:
      if (out)
	fprintf (out, "CHAR");
      bytes += strlen ("CHAR");
      break;
    case CLOBBERS:
      if (out)
	fprintf (out, "CLOBBERS");
      bytes += strlen ("CLOBBERS");
      break;
    case COBEGIN:
      if (out)
	fprintf (out, "COBEGIN");
      bytes += strlen ("COBEGIN");
      break;
    case COMDAT:
      if (out)
	fprintf (out, "COMDAT");
      bytes += strlen ("COMDAT");
      break;
    case COMMON:
      if (out)
	fprintf (out, "COMMON");
      bytes += strlen ("COMMON");
      break;
    case COMPEXPR:
      if (out)
	fprintf (out, "COMPEXPR");
      bytes += strlen ("COMPEXPR");
      break;
    case COMPSTMT:
      if (out)
	fprintf (out, "COMPSTMT");
      bytes += strlen ("COMPSTMT");
      break;
    case CONJ:
      if (out)
	fprintf (out, "CONJ");
      bytes += strlen ("CONJ");
      break;
    case CONST:
      if (out)
	fprintf (out, "CONST");
      bytes += strlen ("CONST");
      break;
    case CONSTRUCTOR:
      if (out)
	fprintf (out, "CONSTRUCTOR");
      bytes += strlen ("CONSTRUCTOR");
      break;
    case CONTINUE:
      if (out)
	fprintf (out, "CONTINUE");
      bytes += strlen ("CONTINUE");
      break;
    case DEF:
      if (out)
	fprintf (out, "DEF");
      bytes += strlen ("DEF");
      break;
    case DEFAULT:
      if (out)
	fprintf (out, "DEFAULT");
      bytes += strlen ("DEFAULT");
      break;
    case DEFINED:
      if (out)
	fprintf (out, "DEFINED");
      bytes += strlen ("DEFINED");
      break;
    case DESTRUCTOR:
      if (out)
	fprintf (out, "DESTRUCTOR");
      bytes += strlen ("DESTRUCTOR");
      break;
    case DIM:
      if (out)
	fprintf (out, "DIM");
      bytes += strlen ("DIM");
      break;
    case DISJ:
      if (out)
	fprintf (out, "DISJ");
      bytes += strlen ("DISJ");
      break;
    case DIV:
      if (out)
	fprintf (out, "DIV");
      bytes += strlen ("DIV");
      break;
    case DO:
      if (out)
	fprintf (out, "DO");
      bytes += strlen ("DO");
      break;
    case DOACROSS:
      if (out)
	fprintf (out, "DOACROSS");
      bytes += strlen ("DOACROSS");
      break;
    case DOALL:
      if (out)
	fprintf (out, "DOALL");
      bytes += strlen ("DOALL");
      break;
    case DOSERIAL:
      if (out)
	fprintf (out, "DOSERIAL");
      bytes += strlen ("DOSERIAL");
      break;
    case DOSUPER:
      if (out)
	fprintf (out, "DOSUPER");
      bytes += strlen ("DOSUPER");
      break;
    case DOT:
      if (out)
	fprintf (out, "DOT");
      bytes += strlen ("DOT");
      break;
    case DOUBLE:
      if (out)
	fprintf (out, "DOUBLE");
      bytes += strlen ("DOUBLE");
      break;
    case ELLIPSIS:
      if (out)
	fprintf (out, "ELLIPSIS");
      bytes += strlen ("ELLIPSIS");
      break;
    case ELSE:
      if (out)
	fprintf (out, "ELSE");
      bytes += strlen ("ELSE");
      break;
    case EMPTY:
      if (out)
	fprintf (out, "EMPTY");
      bytes += strlen ("EMPTY");
      break;
    case END:
      if (out)
	fprintf (out, "END");
      bytes += strlen ("END");
      break;
    case ENUM:
      if (out)
	fprintf (out, "ENUM");
      bytes += strlen ("ENUM");
      break;
    case ENUMFIELD:
      if (out)
	fprintf (out, "ENUMFIELD");
      bytes += strlen ("ENUMFIELD");
      break;
    case EQ:
      if (out)
	fprintf (out, "EQ");
      bytes += strlen ("EQ");
      break;
    case ERROR:
      if (out)
	fprintf (out, "ERROR");
      bytes += strlen ("ERROR");
      break;
    case EXPLICIT_ALIGNMENT:
      if (out)
	fprintf (out, "EXPLICIT_ALIGNMENT");
      bytes += strlen ("EXPLICIT_ALIGNMENT");
      break;
    case EXPR:
      if (out)
	fprintf (out, "EXPR");
      bytes += strlen ("EXPR");
      break;
    case EXPRSIZE:
      if (out)
	fprintf (out, "EXPRSIZE");
      bytes += strlen ("EXPRSIZE");
      break;
    case EXTERN:
      if (out)
	fprintf (out, "EXTERN");
      bytes += strlen ("EXTERN");
      break;
    case FASTCALL:
      if (out)
	fprintf (out, "FASTCALL");
      bytes += strlen ("FASTCALL");
      break;
    case FIELD:
      if (out)
	fprintf (out, "FIELD");
      bytes += strlen ("FIELD");
      break;
    case FINAL:
      if (out)
	fprintf (out, "FINAL");
      bytes += strlen ("FINAL");
      break;
    case FLOAT:
      if (out)
	fprintf (out, "FLOAT");
      bytes += strlen ("FLOAT");
      break;
    case FOR:
      if (out)
	fprintf (out, "FOR");
      bytes += strlen ("FOR");
      break;
    case FUNC:
      if (out)
	fprintf (out, "FUNC");
      bytes += strlen ("FUNCTION");
      break;
    case FUNCTION:
      if (out)
	fprintf (out, "FUNCTION");
      bytes += strlen ("FUNCTION");
      break;
    case GE:
      if (out)
	fprintf (out, "GE");
      bytes += strlen ("GE");
      break;
    case GLOBAL:
      if (out)
	fprintf (out, "GLOBAL");
      bytes += strlen ("GLOBAL");
      break;
    case GOTO:
      if (out)
	fprintf (out, "GOTO");
      bytes += strlen ("GOTO");
      break;
    case GT:
      if (out)
	fprintf (out, "GT");
      bytes += strlen ("GT");
      break;
    case GVAR:
      if (out)
	fprintf (out, "GVAR");
      bytes += strlen ("GVAR");
      break;
    case HEADER:
      if (out)
	fprintf (out, "HEADER");
      bytes += strlen ("HEADER");
      break;
    case ID:
      if (out)
	fprintf (out, "ID");
      bytes += strlen ("ID");
      break;
    case IF:
      if (out)
	fprintf (out, "IF");
      bytes += strlen ("IF");
      break;
    case IMPLICIT:
      if (out)
	fprintf (out, "IMPLICIT");
      bytes += strlen ("IMPLICIT");
      break;
    case IN:
      if (out)
	fprintf (out, "IN");
      bytes += strlen ("IN");
      break;
    case INC:
      if (out)
	fprintf (out, "INC");
      bytes += strlen ("INC");
      break;
    case INCLUDE:
      if (out)
	fprintf (out, "INCLUDE");
      bytes += strlen ("INCLUDE");
      break;
    case INCOMPLETE:
      if (out)
	fprintf (out, "INCOMPLETE");
      bytes += strlen ("INCOMPLETE");
      break;
    case INDEX:
      if (out)
	fprintf (out, "INDEX");
      bytes += strlen ("INDEX");
      break;
    case INDR:
      if (out)
	fprintf (out, "INDR");
      bytes += strlen ("INDR");
      break;
    case INIT:
      if (out)
	fprintf (out, "INIT");
      bytes += strlen ("INIT");
      break;
    case INT:
      if (out)
	fprintf (out, "INT");
      bytes += strlen ("INT");
      break;
    case INV:
      if (out)
	fprintf (out, "INV");
      bytes += strlen ("INV");
      break;
    case IP_TABLE:
      if (out)
	fprintf (out, "IP_TABLE");
      bytes += strlen ("IP_TABLE");
      break;
    case KEY:
      if (out)
	fprintf (out, "KEY");
      bytes += strlen ("KEY");
      break;
    case LABEL:
      if (out)
	fprintf (out, "LABEL");
      bytes += strlen ("LABEL");
      break;
    case LE:
      if (out)
	fprintf (out, "LE");
      bytes += strlen ("LE");
      break;
    case LINKED:
      if (out)
	fprintf (out, "LINKED");
      bytes += strlen ("LINKED");
      break;
    case LINKMULTI:
      if (out)
	fprintf (out, "LINKMULTI");
      bytes += strlen ("LINKMULTI");
      break;
    case LONG:
      if (out)
	fprintf (out, "LONG");
      bytes += strlen ("LONG");
      break;
    case LONGDOUBLE:
      if (out)
	fprintf (out, "LONGDOUBLE");
      bytes += strlen ("LONGDOUBLE");
      break;
    case LONGLONG:
      if (out)
	fprintf (out, "LONGLONG");
      bytes += strlen ("LONGLONG");
      break;
    case LSHFT:
      if (out)
	fprintf (out, "LSHFT");
      bytes += strlen ("LSHFT");
      break;
    case LT:
      if (out)
	fprintf (out, "LT");
      bytes += strlen ("LT");
      break;
    case LTYPE:
      if (out)
	fprintf (out, "LTYPE");
      bytes += strlen ("LTYPE");
      break;
    case MOD:
      if (out)
	fprintf (out, "MOD");
      bytes += strlen ("MOD");
      break;
    case MUL:
      if (out)
	fprintf (out, "MUL");
      bytes += strlen ("MUL");
      break;
    case MUTEX:
      if (out)
	fprintf (out, "MUTEX");
      bytes += strlen ("MUTEX");
      break;
    case NE:
      if (out)
	fprintf (out, "NE");
      bytes += strlen ("NE");
      break;
    case NEG:
      if (out)
	fprintf (out, "NEG");
      bytes += strlen ("NEG");
      break;
    case NOT:
      if (out)
	fprintf (out, "NOT");
      bytes += strlen ("NOT");
      break;
    case NOT_AVAIL:
      if (out)
	fprintf (out, "NOT_AVAIL");
      bytes += strlen ("NOT_AVAIL");
      break;
    case KW_NULL:   /* NULL keyword - prevent conflict with pointer NULL. */
      if (out)
	fprintf (out, "NULL");
      bytes += strlen ("NULL");
      break;
    case NUM_ENTRIES:
      if (out)
	fprintf (out, "NUM_ENTRIES");
      bytes += strlen ("NUM_ENTRIES");
      break;
    case OFFSET:
      if (out)
	fprintf (out, "OFFSET");
      bytes += strlen ("OFFSET");
      break;
    case OLD_PARAM:
      if (out)
	fprintf (out, "OLD_PARAM");
      bytes += strlen ("OLD_PARAM");
      break;
    case OPERANDS:
      if (out)
	fprintf (out, "OPERANDS");
      bytes += strlen ("OPERANDS");
      break;
    case OR:
      if (out)
	fprintf (out, "OR");
      bytes += strlen ("OR");
      break;
    case OUT:
      if (out)
	fprintf (out, "OUT");
      bytes += strlen ("OUT");
      break;
    case PARAM:
      if (out)
	fprintf (out, "PARAM");
      bytes += strlen ("PARAM");
      break;
    case PARAMETER:
      if (out)
	fprintf (out, "PARAMETER");
      bytes += strlen ("PARAMETER");
      break;
    case PARENT:
      if (out)
	fprintf (out, "PARENT");
      bytes += strlen ("PARENT");
      break;
    case PHI:
      if (out)
	fprintf (out, "PHI");
      bytes += strlen ("PHI");
      break;
    case POINTER:
      if (out)
	fprintf (out, "POINTER");
      bytes += strlen ("POINTER");
      break;
    case POS:
      if (out)
	fprintf (out, "POS");
      bytes += strlen ("POS");
      break;
    case POSTDEC:
      if (out)
	fprintf (out, "POSTDEC");
      bytes += strlen ("POSTDEC");
      break;
    case POSTINC:
      if (out)
	fprintf (out, "POSTINC");
      bytes += strlen ("POSTINC");
      break;
    case PRAGMA:
      if (out)
	fprintf (out, "PRAGMA");
      bytes += strlen ("PRAGMA");
      break;
    case PREDEC:
      if (out)
	fprintf (out, "PREDEC");
      bytes += strlen ("PREDEC");
      break;
    case PREINC:
      if (out)
	fprintf (out, "PREINC");
      bytes += strlen ("PREINC");
      break;
    case PROFILE:
      if (out)
	fprintf (out, "PROFILE");
      bytes += strlen ("PROFILE");
      break;
    case PSTMT:
      if (out)
	fprintf (out, "PSTMT");
      bytes += strlen ("PSTMT");
      break;
    case QUEST:
      if (out)
	fprintf (out, "QUEST");
      bytes += strlen ("QUEST");
      break;
    case REFS:
      if (out)
	fprintf (out, "REFS");
      bytes += strlen ("REFS");
      break;
    case REGISTER:
      if (out)
	fprintf (out, "REGISTER");
      bytes += strlen ("REGISTER");
      break;
    case RETURN:
      if (out)
	fprintf (out, "RETURN");
      bytes += strlen ("RETURN");
      break;
    case RSHFT:
      if (out)
	fprintf (out, "RSHFT");
      bytes += strlen ("RSHFT");
      break;
    case SCOPE:
      if (out)
	fprintf (out, "SCOPE");
      bytes += strlen ("SCOPE");
      break;
    case SIGNED:
      if (out)
	fprintf (out, "SIGNED");
      bytes += strlen ("SIGNED");
      break;
    case SIZE:
      if (out)
	fprintf (out, "SIZE");
      bytes += strlen ("SIZE");
      break;
    case SHADOW:
      if (out)
	fprintf (out, "SHADOW");
      bytes += strlen ("SHADOW");
      break;
    case SHORT:
      if (out)
	fprintf (out, "SHORT");
      bytes += strlen ("SHORT");
      break;
    case SOURCE:
      if (out)
	fprintf (out, "SOURCE");
      bytes += strlen ("SOURCE");
      break;
    case STATIC:
      if (out)
	fprintf (out, "STATIC");
      bytes += strlen ("STATIC");
      break;
    case STDCALL:
      if (out)
	fprintf (out, "STDCALL");
      bytes += strlen ("STDCALL");
      break;
    case STMT:
      if (out)
	fprintf (out, "STMT");
      bytes += strlen ("STMT");
      break;
    case STMTEXPR:
      if (out)
	fprintf (out, "STMTEXPR");
      bytes += strlen ("STMTEXPR");
      break;
    case STRING:
      if (out)
	fprintf (out, "STRING");
      bytes += strlen ("STRING");
      break;
    case STRUCT:
      if (out)
	fprintf (out, "STRUCT");
      bytes += strlen ("STRUCT");
      break;
    case SUB:
      if (out)
	fprintf (out, "SUB");
      bytes += strlen ("SUB");
      break;
    case SWITCH:
      if (out)
	fprintf (out, "SWITCH");
      bytes += strlen ("SWITCH");
      break;
    case SYM:
      if (out)
	fprintf (out, "SYM");
      bytes += strlen ("SYM");
      break;
    case SYMBOL_TABLE:
      if (out)
	fprintf (out, "SYMBOL_TABLE");
      bytes += strlen ("SYMBOL_TABLE");
      break;
    case SYNC:
      if (out)
	fprintf (out, "SYNC");
      bytes += strlen ("SYNC");
      break;
    case THEN:
      if (out)
	fprintf (out, "THEN");
      bytes += strlen ("THEN");
      break;
    case TYPE:
      if (out)
	fprintf (out, "TYPE");
      bytes += strlen ("TYPE");
      break;
    case TYPEDEF:
      if (out)
	fprintf (out, "TYPEDEF");
      bytes += strlen ("TYPEDEF");
      break;
    case TYPESIZE:
      if (out)
	fprintf (out, "TYPESIZE");
      bytes += strlen ("TYPESIZE");
      break;
    case UNION:
      if (out)
	fprintf (out, "UNION");
      bytes += strlen ("UNION");
      break;
    case UNSIGNED:
      if (out)
	fprintf (out, "UNSIGNED");
      bytes += strlen ("UNSIGNED");
      break;
    case UNNAMED:
      if (out)
	fprintf (out, "UNNAMED");
      bytes += strlen ("UNNAMED");
      break;
    case VAR:
      if (out)
	fprintf (out, "VAR");
      bytes += strlen ("VAR");
      break;
    case VARARG:
      if (out)
	fprintf (out, "VARARG");
      bytes += strlen ("VARARG");
      break;
    case VOID:
      if (out)
	fprintf (out, "VOID");
      bytes += strlen ("VOID");
      break;
    case VOLATILE:
      if (out)
	fprintf (out, "VOLATILE");
      bytes += strlen ("VOLATILE");
      break;
    case WEAK:
      if (out)
	fprintf (out, "WEAK");
      bytes += strlen ("WEAK");
      break;
    case WHILE:
      if (out)
	fprintf (out, "WHILE");
      bytes += strlen ("WHILE");
      break;
    case XOR:
      if (out)
	fprintf (out, "XOR");
      bytes += strlen ("XOR");
      break;
    case '(':
      if (out)
	fprintf (out, "(");
      bytes += strlen ("(");
      break;
    case ')':
      if (out)
	fprintf (out, ")");
      bytes += strlen (")");
      break;
    default:
      P_punt ("write.c:P_write_token:%d Unknown token %d", __LINE__, token);
    }

  bytes += P_write_space (out, 1);

  return (bytes);
}

/*! \brief Writes an expressions opcode
 * 
 * \param out
 *  the file to write.
 * \param opcode
 *  the opcode to write.
 * \param indent
 *  the indent level.
 *
 * \return
 *  The number of bytes written
 *
 * Wrapper for P_write_token that takes an opcode because op_to_token
 * is not available outside of this file
 */
int
P_write_opcode (FILE *out, _Opcode opcode, int indent)
{
  return P_write_token (out, op_to_token[opcode], indent);
}

/*! \brief Writes an integer literal to disk.
 *
 * \param out
 *  the file to write.
 * \param i_lit
 *  the integer value to write.
 * \param indent
 *  the indent level.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes an integer literal to disk.  If \a out is null, this function returns
 * the number of bytes it would have written.
 */
int
P_write_I_LIT (FILE *out, long i_lit, int indent)
{
  int bytes = 0;
  char buf[65];

  bytes += snprintf (buf, sizeof (buf), "%ld", i_lit);

  if (out)
    fprintf (out, buf);

  bytes += P_write_space (out, 1);

  return (bytes);
}

/*! \brief Writes a floating point literal to disk.
 *
 * \param out
 *  the file to write.
 * \param f_lit
 *  the float to write.
 * \param indent
 *  the indent level.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes a floating point literal to disk.  If \a out is null, this function
 * returns the number of bytes it would have written.
 */
int
P_write_F_LIT (FILE *out, double f_lit, int indent)
{
  int bytes = 0;
  char buf[65];

  bytes += snprintf (buf, sizeof (buf), "%1.16e", f_lit);

  if (out)
    fprintf (out, buf);

  bytes += P_write_space (out, 1);

  return (bytes);
}

/*! \brief Writes a character literal to disk.
 *
 * \param out
 *  the file to write.
 * \param c_lit
 *  the character to write.
 * \param indent
 *  the indent level.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes a character literal to disk.  If \a out is null, this function
 * returns the number of bytes it would have written.
 */
int
P_write_C_LIT (FILE *out, char c_lit, int indent)
{
  int bytes = 0;
  char buf[65];

  bytes += snprintf (buf, sizeof (buf), "'%c'", c_lit);

  if (out)
    fprintf (out, buf);

  bytes += P_write_space (out, 1);

  return (bytes);
}

/*! \brief Writes a string literal to disk.
 *
 * \param out
 *  the file to write.
 * \param st_lit
 *  the string to write.
 * \param indent
 *  the indent level.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes a string literal to disk.  If \a out is null, this function returns
 * the number of bytes it would have written.
 */
int
P_write_ST_LIT (FILE *out, char *st_lit, int indent)
{
  int bytes = 0;

  if (st_lit)
    {
      bytes += strlen (st_lit);
      if (out)
	fprintf (out, "\"%s\"", st_lit);
    }
  else
    {
      bytes += strlen ("\"\"");
      if (out)
	fprintf (out, "\"\"");
    }

  bytes += P_write_space (out, 1);

  return (bytes);
}

/* Utility functions */

/*! \brief Writes the offset field to disk.
 *
 * \param out
 *  the file to write.
 * \param offset
 *  the offset to write.
 * \param indent
 *  the indent level.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes the offset field to disk.  If \a out is null, this function returns
 * the number of bytes it would have written.  If \a offset is zero, the
 * file position of this field is returned in \a offset.  If \a offset is
 * non-zero, the current file position is written at the offset passed
 * as offset.  In either case, the file position is set to the end of the
 * file before returning.  The offset field is an integer, written at a fixed
 * width of 15 digits.
 *
 * This function is called twice as the symbol table is written.  It is
 * first called (with a zero \a offset) when the table entry (the
 * ::SymTabEntry) is written.  It is later called when the corresponding
 * Pcode structure is written, this time with \a offset set to the file
 * position of the previously written offset field.
 *
 * Returns 0 after the second call.  The second call simply overwrites the
 * field written by the first call, so the file size does not change.
 */
int
P_write_offset (FILE *out, int *offset, int indent)
{
  int cur_position;
  int bytes = 0;

  if (out == NULL)
    {
      if (*offset == 0)
	{
	  return (15 + 1); /* trailing space */
	}
      else
	{
	  return (0);
	}
    }

  cur_position = ftell (out);

  /* If this is the first call, offset is 0.  Write the offset and return
   * the file position. */
  if (*offset == 0)
    {
      bytes += fprintf (out, "%15d", *offset);
      bytes += P_write_space (out, 1);
      *offset = cur_position;
    }
  else
    {
      if (fseek (out, *offset, SEEK_SET) != 0)
	{
	  perror ("could not seek");
	  P_punt ("write.c:P_write_offset:%d Could not seek to offset %d",
		  __LINE__ - 1, *offset);
	}

      fprintf (out, "%15d", cur_position);

      if (fseek (out, 0L, SEEK_END) != 0)
	{
	  perror ("could not seek");
	  P_punt ("write.c:P_write_offset:%d Could not seek to end of file",
		  __LINE__ - 1);
	}
    }

  return (bytes);
}

/*! \brief Writes one or more spaces to disk.
 *
 * \param out
 *  the file to write.
 * \param count
 *  the number of spaces to write.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes one or more spaces to disk.  If \a out is null, this function returns
 * the number of bytes it would have written.
 */
int
P_write_space (FILE *out, int count)
{
  int i;

  if (out)
    for (i = 0; i < count; i++)
      fprintf (out, " ");

  return (count);
}

/*! \brief Writes one or more newlines to disk.
 *
 * \param out
 *  the file to write.
 * \param count
 *  the number of newlines to write.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes one or more spaces to disk.  If \a out is null, this function returns
 * the number of bytes it would have written.
 */
int
P_write_newline (FILE *out, int count)
{
  int bytes = count;

  while (out && count--)
    fprintf (out, "\n");

  return (bytes);
}

/*! \brief Writes a newline and indents.
 *
 * \param out
 *  the file to write.
 * \param indent
 *  the indent level.
 *
 * \return
 *  The number of bytes written.
 *
 * Writes a newline and indents.
 */
int
P_write_newline_indent (FILE *out, int indent)
{
  int bytes = 0;

  bytes += P_write_newline (out, 1);
  bytes += P_write_space (out, indent);

  return (bytes);
}

/*! \brief Writes a comment.
 *
 * \param out
 *  the file to write.
 * \param comment
 *  the comment text.
 * \param indent
 *  the indent level.
 *
 * \return
 *  The number of bytes written.
 *
 * Adds the comment characters to the comment text and writes a
 * comment.  If \a out is null, this function returns the number of
 * bytes it would have written.  */
int
P_write_comment (FILE *out, char *comment, int indent)
{
  int bytes = 0;

  bytes = strlen (comment) + 6;
  
  if (out)
    fprintf (out, "/* %s */", comment);

  return (bytes);
}

/* Allocates a new string which the caller must free. */
char *
P_Asmmod2String(int modifiers)
{
  char *str = malloc (sizeof (char) * 32);
  int i = 0;

  if (str == NULL)
    P_punt ("write.c:P_Asmmod2String:%d could not allocate string", __LINE__);

  memset (str, '\0', sizeof (char) * 32);

  if ((modifiers & 0x01) &&
      (modifiers & 0x02))
    str[i++] = '+';	
  else if (modifiers & 0x02)
    str[i++] = '=';
  
  if (modifiers & 0x04)
    str[i++] = '&';

  if (modifiers & 0x08)
    str[i++] = '%';
  
  if (modifiers & 0x10)
    str[i++] = '*';
  
  if (modifiers & 0x20)
    str[i++] = '#';
  
  if (modifiers & 0x40)
    str[i++] = '?';      
  
  if (modifiers & 0x80)
    str[i++] = '!';
  
  assert (i < 32);

  return (str);
}
