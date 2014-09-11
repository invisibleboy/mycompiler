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
 * \author Robert Kidd and Wen-mei Hwu
 *
 * This file defines data structures needed by Pflatten.
 */

#ifndef _PFLATTEN_DATA_H_
#define _PFLATTEN_DATA_H_

#include <config.h>
#include <Pcode/pcode.h>

extern int PF_Indices[ES_LAST];

/*! The struct to attach to FuncDcls. */
typedef struct _PF_func_data
{
  int flattened:1;              /*!< Set when the function has been
				 * flattened. */
}
_PF_func_data, *PF_func_data;

/*! Flags to set on the Expr structure. */
typedef enum _PF_expr_flags
{
  PF_SIDE_EFFECT    = 0x00000001, /*!< Function call has side effect. */
  PF_BREAK          = 0x00000002, /*!< Expr needs to be broken. */
  PF_CONTAIN_BREAK  = 0x00000004, /*!< Expr contains an expr that needs to
				   * be broken. */
  PF_ARGUMENT       = 0x00000008, /*!< Expr is a function argument. */
  PF_RETAIN         = 0x00000010  /*!< Expr is not useless. */
}
_PF_expr_flags;

/*! The struct to attach to Exprs. */
typedef struct _PF_expr_data
{
  _PF_expr_flags flags;
}
_PF_expr_data, *PF_expr_data;

/*! The struct ot attach to stmts. */
typedef struct _PF_stmt_data
{
  int emptyloop:1;              /*!< Set on empty loop stmts. */
  _SerLoopType loop_type;       /*!< Set to the loop type on a loop body. */
  int loop_line;                /*!< The line number of the original loop. */
}
_PF_stmt_data, *PF_stmt_data;

extern PF_func_data PF_alloc_func_data (void);
extern PF_func_data PF_free_func_data (PF_func_data d);
extern PF_expr_data PF_alloc_expr_data (void);
extern PF_expr_data PF_free_expr_data (PF_expr_data d);
extern PF_expr_data PF_copy_expr_data (PF_expr_data d);
extern PF_stmt_data PF_alloc_stmt_data (void);
extern PF_stmt_data PF_free_stmt_data (PF_stmt_data d);

/*! \brief Returns the \a flattened field for a FuncDcl.
 *
 * \param f
 *  the FuncDcl.
 *
 * \return
 *  The \a flattened field for the FuncDcl.
 */
#define PF_func_flattened(f) \
          (((PF_func_data)P_GetFuncDclExtL (f, \
                                            PF_Indices[ES_FUNC]))->flattened)

/*! \brief Sets bits in the PF_expr_data.flags field on an Expr.
 *
 * \param e
 *  the Expr.
 * \param f
 *  the flags to set in the field.
 *
 * \sa #PF_get_expr_flags(), #PF_clr_expr_flags()
 */
#define PF_set_expr_flags(e, f) \
          (((PF_expr_data)P_GetExprExtL (e, \
                                         PF_Indices[ES_EXPR]))->flags |= (f))

/*! \brief Gets the value of the PF_expr_data.flags field on an Expr.
 *
 * \param e
 *  the Expr.
 *
 * \return
 *  The value of the PF_expr_data.flags field.
 *
 * \sa #PF_set_expr_flags(), #PF_clr_expr_flags()
 */
#define PF_get_expr_flags(e) \
          (((PF_expr_data)P_GetExprExtL (e, PF_Indices[ES_EXPR]))->flags)

/*! \brief Clears bits in the PF_expr_data.flags field on an Expr.
 *
 * \param e
 *  the Expr.
 * \param f
 *  the flags to clear in the field.
 *
 * \sa #PF_set_expr_flags(), #PF_get_expr_flags()
 */
#define PF_clr_expr_flags(e, f) \
          (((PF_expr_data)P_GetExprExtL (e, \
                                         PF_Indices[ES_EXPR]))->flags &= ~(f))

/*! \brief Returns the \a emptyloop field for a Stmt.
 *
 * \param s
 *  the Stmt.
 *
 * \return
 *  The \a emptyloop field for the Stmt.
 *
 * \sa #PF_stmt_loop_type(), #PF_stmt_loop_line()
 */
#define PF_stmt_emptyloop(s) \
          (((PF_stmt_data)P_GetStmtExtL (s, PF_Indices[ES_STMT]))->emptyloop)

/*! \brief Returns the \a loop_type field for a Stmt.
 *
 * \param s
 *  the Stmt.
 *
 * \return
 *  The \a loop_type field for the Stmt.
 *
 * \sa #PF_stmt_emptyloop(), #PF_stmt_loop_line()
 */
#define PF_stmt_loop_type(s) \
          (((PF_stmt_data)P_GetStmtExtL (s, PF_Indices[ES_STMT]))->loop_type)

/*! \brief Returns the \a loop_line field for a Stmt.
 *
 * \param s
 *  the Stmt.
 *
 * \return
 *  The \a loop_line field for the Stmt.
 *
 * \sa #PF_stmt_emptyloop(), #PF_stmt_loop_type()
 */
#define PF_stmt_loop_line(s) \
          (((PF_stmt_data)P_GetStmtExtL (s, PF_Indices[ES_STMT]))->loop_line)

#endif

