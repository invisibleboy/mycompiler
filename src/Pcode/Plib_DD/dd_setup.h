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

#include <Pcode/pcode.h>
#include <Pcode/ss_induct.h>
#include "dd_omega-ip.h"		/* to import Problem */
#include "dd_dir.h"
#include "dd_interface.h"

#define SKIP_OMEGA2

#define USE_LOOP_NESTING_LEVEL		1

/*******************************************************************/

extern int DD_DEBUG_OMEGA;
extern int DD_PRINT_OMEGA;
extern int DD_IGNORE_CF_INFO;
extern int DD_GENERATE_ACYCLIC_SYNC_ARC;

/*******************************************************************/

typedef struct _P_AffineTerm
{
  Alpha_var_id tiny_var;	/* Variable information for omega */
  int coefficient;		/* co-efficient */
}
_P_AffineTerm, *P_AffineTerm;

typedef struct _P_AffineExpr
{
  int nterms;
  _P_AffineTerm terms[maxVars];	/* 1st entry var is always 0 */
  struct _P_AffineExpr *other_branch;	/* if min or max */
}
_P_AffineExpr, *P_AffineExpr;

extern _P_AffineExpr Alpha_not_affine;

#define Alpha_is_affine(ae)     	((ae) != &Alpha_not_affine)

/*******************************************************************/

typedef enum
{ goleft, goright, panic }
which_branch;

/*******************************************************************/
/*
 * context stuff
 */
typedef struct _PO_Context
{
  struct _PC_Loop *loop;
  P_AffineExpr cond_affexpr;
#if !USE_LOOP_NESTING_LEVEL
  int depth;			/* parloop nesting level */
#endif
  struct _PO_Context *next;
}
_PO_Context, *PO_Context;

#define Get_PO_Context_loop(c)			((c)->loop)
#define Set_PO_Context_loop(c, l)		((c)->loop = (l))
#define Get_PO_Context_cond_affexpr(c)		((c)->cond_affexpr)
#define Set_PO_Context_cond_affexpr(c, ca)	((c)->cond_affexpr = (ca))

#if USE_LOOP_NESTING_LEVEL
#define Get_PO_Context_depth(c)			((c)->loop->nesting_level)
#else
#define Get_PO_Context_depth(c)			((c)->depth)
#define Set_PO_Context_depth(c, d)		((c)->depth = (d))
#endif

#define Get_PO_Context_next(c)			((c)->next)
#define Set_PO_Context_next(c, n)		((c)->next = (n))
#if 0
#define Get_PO_Context_loop_type(c)	 Get_Loop_type((c)->loop)
#define Set_PO_Context_loop_type(c, lt)	 (Get_Loop_type((c)->loop) = (lt))
#endif
#define Get_PO_Context_var_id(c)	 Get_Loop_iv(Get_PO_Context_loop(c))
#define Set_PO_Context_var_id(c, vi)	 (Get_Loop_iv(Get_PO_Context_loop(c)) \
                                          = (vi))
#define PO_Context_is_done(c)			((c) == NULL)

/*******************************************************************/
/*
 * Subscriptor stuff
 */

typedef struct _PO_Subscript
{
  Expr sub_expr;		/* Pcode subscript expression */
  P_AffineExpr affexpr;		/* affine expression for subscript */
  unsigned char mod_sub_var;	/* subscript symbolic vars modified */
  struct _PO_Subscript *next;
}
_PO_Subscript, *PO_Subscript;

#define Get_PO_Subscript_next(s)		((s)->next)
#define Set_PO_Subscript_next(s, n)		((s)->next = (n))
#define Get_PO_Subscript_affexpr(s)		((s)->affexpr)
#define Set_PO_Subscript_affexpr(s, a)		((s)->affexpr = (a))
#define Get_PO_Subscript_sub_expr(s)		((s)->sub_expr)
#define Set_PO_Subscript_sub_expr(s, se)	((s)->sub_expr = (se))
#define Get_PO_Subscript_mod_sub_var(s)		((s)->mod_sub_var)
#define Set_PO_Subscript_mod_sub_var(s, m)	((s)->mod_sub_var = (m))

#define PO_Subscript_is_done(s)			((s) == NULL)
#define PO_Subscript_is_affine(s)  Alpha_is_affine(Get_PO_Subscript_affexpr(s))

/* values for mod_sub_var in above sub_iterator structure */
#define NO_SUB_VAR_MOD          0
#define SUB_VAR_MOD_IN_PARLOOP  (1<<0)
#define SUB_VAR_MOD_AT_TOP_LEV  (1<<1)
#define SUB_VAR_MOD_IN_FUNC     (SUB_VAR_MOD_IN_PARLOOP|SUB_VAR_MOD_AT_TOP_LEV)


/*******************************************************************/

#define maxCommonNest 6		/* (32 bits - 6 used in dd_dir.h) / 4 bits per dd */

typedef struct
{
  uint nest;
  bool distanceKnown[maxCommonNest + 1];
  int distance[maxCommonNest + 1];
}
dist_info;

/* all info about dependence, as used by findDirectionVector,
   and read by noteDependence */

typedef struct
{
  dddirection direction;
  dddirection restraint;
  dist_info *dist;
}
dir_and_dist_info;

/* array of distances of length ddnest+1 (defined below) */
typedef sint *DDdistance;

/*******************************************************************/
/* dd_affine.h */

typedef enum
{
  posmax = -1,
  none = 0,
  posmin = 1
}
Min_or_max;

/*******************************************************************/
/*
 * interface to access table
 */

/* Used by Omega test to initialize tag field */
#define UNTAGGED -1

/* define the maximum size of string used in dependence analysis structures */
#define DD_MAX_STRING           512

typedef struct _P_ExprExtForDD
{
  Expr array_var;		/* array variable accessed */
  struct _PO_Subscript *first_subi;	/* the first subscriptor */
  struct _PO_Context *first_context;	/* the first context */
}
_P_ExprExtForDD, *P_ExprExtForDD;

#define Get_ExprExtForDD_array_var(e)		     ((e)->array_var)
#define Set_ExprExtForDD_array_var(e, a)	     ((e)->array_var = (a))
#define Get_ExprExtForDD_first_subi(e)		     ((e)->first_subi)
#define Set_ExprExtForDD_first_subi(e, f)	     ((e)->first_subi = (f))
#define Get_ExprExtForDD_first_context(e)	     ((e)->first_context)
#define Set_ExprExtForDD_first_context(e, f)	     ((e)->first_context = (f))

extern P_ExprExtForDD Get_ExprExtForDD (Expr e);

/*******************************************************************/

#define STRING_Symbol_data(s)			((s)->data)

/*******************************************************************/

extern PC_Graph DD_GetFunctionCFG (FuncDcl func);
extern P_DepList DD_GetExprDepList (Expr e);
extern void DD_PrintFuncDepInfo (FILE * outf, FuncDcl func);

/*******************************************************************/
