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
 *      File:   inline.h
 *      Author: Ben-Chung Cheng
 *      Copyright (c) 1995 Ben-Chung Cheng, Wen-mei Hwu. All rights reserved.
 *      All rights granted to the University of Illinois.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#ifndef	PIN_INLINE_H
#define	PIN_INLINE_H

#include <config.h>
#include <stdio.h>
#include <library/list.h>
#include <library/c_symbol.h>
#include <library/heap.h>
#include <library/func_list.h>
#include <Pcode/pcode.h>
#include <Pcode/struct.h>
#include <Pcode/parms.h>

/**************************************************************************
 * Fixed issue with detection of single and double recursive functions.
 * The original (x->caller == x->callee) wasn't sufficient for detection 
 * of recursive functions and would continuously inline until inliner reached
 * the budget. Required for the inliner to dig a level deeper to find if
 * function was recursive ->func and ->owner. 
 * C.O.K
 *************************************************************************/
#if 1
#define RECURSIVE(x)    	(x->caller->func==x->callee->func || \
                                 x->caller->owner==x->callee->owner)
#else
#define RECURSIVE(x)    	(x->caller==x->callee)
#endif

#define PIN_PRG_CS "PIN-CSID"
#define PIN_PRG_IPC_PFX "IPC-"
#define PIN_PRG_IPC_PFX_LEN 4

typedef struct _Pragma_List
{
  char funcname[1024];
  double weight;
  struct _Pragma_List *next;
}
 *Pragma_List;

/* pin_callgraph */

#include "pin_callgraph.h"

/* #define Pin_fdcl_cgf(fd) ((PinCG_Func)(P_GetFuncDclExtM (fd))) */
/* Make it compile with gcc4 -KF 10/2005 */
#define Pin_fdcl_cgf(fd) (P_GetFuncDclExtM (fd))

/* pin_cg_inline */

extern void Pin_inline_callgraph (void);

/* pin_annotate */

extern void Pin_preprocess_func (PinCG_Func, FuncDcl, double);

/* pin_expand */

extern Stmt Pin_expand_inlined_arc (Stmt stmt, PinCG_Arc arc);

/* pin_inline */

extern SymbolTable Pin_callee_symtab;

extern double TotalBodySize;
extern double TotalEBodySize;
extern double TotalStackSize;
extern double TotalCallWeight;
extern double MaxBodySize;

extern double min_expansion_key;
extern int exclude_small_from_ratio_limit;
extern int inline_self_recursion;

extern int Pin_adjust_func_weight;

extern int prevent_cross_file_inlining;
extern int inline_inlined_body;
extern int Pin_trace_heap;
extern int print_inline_stats;
extern int print_inline_graphs;
extern int print_inline_trace;
extern double Pin_budget;

extern int Pin_body_size_metric;
#define PIN_BODY_TOTAL    1
#define PIN_BODY_TOUCH    2
#define PIN_BODY_EXECD    3 

extern int Pin_inline_key_cost;
#define PIN_KEY_SQRT_SIZE 1
#define PIN_KEY_SIZE      2
#define PIN_KEY_MC_SIZE   3

extern char *il_log_name;
extern int max_sf_size_limit;
extern double max_expansion_ratio;
extern int max_function_size;
extern double min_expansion_weight;
extern char *sp_output_spec;
extern char *il_output_dir_string;
extern int inline_function_pointers;
extern int inline_indir_by_profile;
extern double indir_thresh;
extern double assumed_body_size;
extern int size_only;
extern int favor_small_functions;
extern int small_function_thresh;
extern Func_Name_List *prevent_inline_list;
extern double Pin_min_arc_ratio;
extern int inline_all_functions;

extern Heap *Pin_arc_heap;

extern int Pin_reduce_budget (double sub);

extern int do_annotate_function;

extern FILE *Flog, *Fstat;

/* pin_util */

extern int Pin_find_call (Expr expr, Expr * pcall, Expr * plhs, Expr * pcast);
extern void Pin_function_size (FuncDcl func, long int *psz_stk, 
			       long int *psz_body, long int *psz_ebody);


#endif
