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
#ifndef _PSS_EXPR_H
#define _PSS_EXPR_H

#include <Pcode/pcode.h>
#include <Pcode/dom.h>
#include <Pcode/ss_ssa2.h>
#include "ss_induct2.h"

#undef  NEWPCODE
#define NEWPCODE	1

#include <Pcode/cfg.h>
#include <library/stack.h>
#include <Pcode/loop.h>
#include <Pcode/struct.h>
#include <Pcode/write.h>

/* global vars */
int ext_id;

/* typedefs */

/* PCODE INDUCTION VARIABLE EXPRESSIONS
 * The following typedefs are used in establishing expressions based on
 * numerical constants, SSA vars, and "fundamental" induction variables
 */

/* Fundamental induction var struct.
 * SER 041017:  Basically empty, will carry min, max, and final value
 * info as we develop this. 
 */

typedef enum
{
  FV_DISCRETE,
  FV_RANGE,
  FV_UNKNOWN
}
PSS_Fund_Ind_Var_final_value_type;

/*! \brief Basic structure for holding a loop's fundamental induction
 * variable and relevant information. */
typedef struct _PSS_Fund_Ind_Var
{
  PC_Loop pcloop;  /* link back to loop */

  /* Bounds/final value information */
  struct _PSS_Bound_Condition * bounds;
  PSS_Fund_Ind_Var_final_value_type final_type;

  void * ext;
}
_PSS_Fund_Ind_Var, *PSS_Fund_Ind_Var;

typedef struct _PSS_Bound_Condition
{
  struct _PSS_Expr            * cond;
  _Opcode                       cont_opcode;
  struct _PSS_Bound_Condition * next;
  PC_Loop			pcloop;
  PC_Block                      bb;
}
_PSS_Bound_Condition, *PSS_Bound_Condition;

/*! \brief Addition term of the induction variable expression */
typedef struct _PSS_Add_Term
{
  int int_coeff;
  int affine;
  struct _PSS_Add_Term * next;
  struct _PSS_Mul_Term * mul;
}
_PSS_Add_Term, *PSS_Add_Term;


typedef enum
{
  SELECT_PHI,
  SELECT_MIN,
  SELECT_MAX,
  SELECT_NONE
}
PSS_Select_Type;

typedef struct _PSS_Select_Term
{
  PSS_Select_Type           sel_type;
  struct _PSS_Select_Term * next;
  struct _PSS_Add_Term    * add;
}
_PSS_Select_Term, *PSS_Select_Term;


typedef enum
{
  TERM_UNDEF,
  TERM_FUND_IND_VAR,
  TERM_SSA_DEF,
  TERM_SELECT,
  TERM_LIN_MON_INFO,
  TERM_PCODE_EXPR
}
PSS_Term_Type;

/* Multiplication term of the induction variable expression */
typedef struct _PSS_Mul_Term
{
  int final_value;
  struct _PSS_Mul_Term  * next;
  PSS_Term_Type    type;
  union
    {
      struct _PSS_Select_Term  * select;
      struct _PSS_Fund_Ind_Var * fund_ind_var;
      struct _PSS_Lin_Mon_Info * lin_mon;
      struct _PSS_Def          * ssa_def;
      struct _Expr             * pcode_expr;
      void                     * nothing;
    } term;
}
_PSS_Mul_Term, *PSS_Mul_Term;

#define INFINITY         0x0001
#define LINEAR_IV        0x0008
#define LIN_MONOTONIC_IV 0x0010
#define UNKNOWN_IV       0x0020

/* This is a struct that defines a Pcode expression as a linear induction
 * variable, with coefficients for loop invariants and the fundamental
 * induction variable of the loop.  The coefficients are in Sum-Of-Products
 * (SOP) form, consisting of linked lists of addition terms, which point to
 * multiplication terms or expressions from outer loops. */
/* In the case of phi functions, each _PSS_Add_Ter in invar_coeff is
 * an input to the phi term, rather than added together.
 */
typedef struct _PSS_Expr
{
  struct _PSS_Add_Term     *terms;
  int flags;					/* See above. */
}
_PSS_Expr, *PSS_Expr;


typedef struct _PSS_Lin_Mon_Info
{
  struct _PSS_Select_Term * min_incr;
  struct _PSS_Select_Term * max_incr;

  int visited;
}
_PSS_Lin_Mon_Info, *PSS_Lin_Mon_Info;

/********************
  Function headers 
********************/

extern void PC_IndExpr_Setup (PC_Graph);
extern void PC_IndExpr_Cleanup (PC_Graph);

extern PSS_Expr PSS_Create_Expr (PSS_Expr ind_expr);
extern PSS_Expr PSS_Create_Int_Expr (int);
extern PSS_Mul_Term PSS_Create_Mul_Term (PSS_Mul_Term);
extern int PSS_Same_Mul_Term (PSS_Mul_Term, PSS_Mul_Term);
extern PSS_Expr PSS_Delete_Expr (PSS_Expr);
extern PSS_Bound_Condition PSS_Delete_Bound_Condition (PSS_Bound_Condition);
extern PSS_Expr PSS_Expr_Extract_Ind_Coeff (PSS_Expr, PC_Loop);
extern PSS_Expr PSS_Expr_Extract_Invar_Coeff (PSS_Expr, PC_Loop);

extern int PSS_Same_Expr (PSS_Expr, PSS_Expr);
extern int PSS_Expr_Is_Int (PSS_Expr, int *);
extern PSS_Expr PSS_Arithmetic_Exprs (PSS_Expr, PSS_Expr, int);
extern PSS_Expr PSS_Multiply_Expr_By_Int (PSS_Expr, int);
extern PSS_Expr PSS_Multiply_Exprs (PSS_Expr, PSS_Expr);

extern PSS_Expr PSS_Get_Expr (Expr, int, PC_Loop);

extern void P_PrintContOpcode(FILE *, _Opcode);
extern void PSS_Find_IVs (PC_Loop);
extern void PSS_Find_All_Loop_Bounds (PC_Loop);
extern void PSS_Find_Expr_Bounds (Expr, PC_Loop);
extern PSS_Bound_Condition PSS_Find_Loop_Bound (PC_Loop);
extern void PSS_Find_Nested_Loop_Bounds (PC_Loop);
extern List PSS_Find_Relevant_Loop_Bounds (PSS_Expr, List);
extern void PSS_Correct_Bounds_For_Access_Position (PC_Block, List);

extern void PSS_Print_Expr (FILE *, PSS_Expr);
extern void PSS_Print_All_Expr (FILE *, PC_Graph, PC_Loop);
extern void PSS_Print_Loop_Bounds (FILE *, PSS_Bound_Condition);
extern void PSS_Print_Relevant_Loop_Bounds (FILE *, PSS_Expr);

extern int PSS_expr_ext;
extern void PSS_def_handlers();
#define PSS_GetExprIE(e) \
  ((PSS_Expr) (P_GetExprExtL(e, PSS_expr_ext)))
#define PSS_SetExprIE(e,v) \
  ((P_SetExprExtL(e, PSS_expr_ext, (v))))

#endif
