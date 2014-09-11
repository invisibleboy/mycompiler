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


#include <library/l_parms.h>
#include <Pcode/pcode.h>
#include <Pcode/extension.h>
#include <Pcode/struct.h>
#include <Pcode/ss_setup.h>
#include "dd_setup.h"

/***************************************************************/
/* globals */

/* level to debug omega dependence analyzer problem instances */
int DD_DEBUG_OMEGA = 0;
int DD_PRINT_OMEGA = 0;

/***************************************************************/
/* Static variables */

static int dd_expr_ext_idx = -1; 
static int dd_deplist_idx = -1;

/***************************************************************/
/* Static function headers */

Extension P_ExprExtForDD_alloc (void);
Extension P_ExprExtForDD_free (Extension x);

/***************************************************************/
/* Exported functions */

void 
DD_ReadParameter (Parm_Parse_Info * ppi)
{
  L_read_parm_b (ppi, "dd_debug_omega", &DD_DEBUG_OMEGA);
  L_read_parm_b (ppi, "dd_print_omega", &DD_PRINT_OMEGA);
}

void
DD_SetUpExtension (void)
{
  assert (dd_expr_ext_idx == -1);
  assert (dd_deplist_idx == -1);
  dd_expr_ext_idx = P_ExtSetupL (ES_EXPR, 
                                 (AllocHandler) P_ExprExtForDD_alloc, 
                                 (FreeHandler) P_ExprExtForDD_free);

  dd_deplist_idx = P_ExtSetupL (ES_EXPR,
                                (AllocHandler) P_DepList_alloc,
                                (FreeHandler) P_DepList_free);

  P_ExtRegisterWriteL (ES_EXPR, dd_deplist_idx, (WriteHandler) P_DepList_write, "DepL");
  P_ExtRegisterCopyL (ES_EXPR, dd_deplist_idx, (CopyHandler) P_DepList_copy);
  SS_SetUpExtension ();
}

P_ExprExtForDD
Get_ExprExtForDD (Expr expr)
{
  return P_GetExprExtL(expr, dd_expr_ext_idx);
}

P_DepList
DD_GetExprDepList (Expr expr)
{
  return P_GetExprExtL(expr, dd_deplist_idx);
}

void
DD_PrintFuncDepInfo (FILE *outf, FuncDcl func)
{
  PrintFuncDepInfo (outf, func, dd_deplist_idx);
}

/***************************************************************/
/* Static functions */

Extension
P_ExprExtForDD_alloc (void)
{
  P_ExprExtForDD x;

  x = ALLOCATE (_P_ExprExtForDD);
  Set_ExprExtForDD_array_var (x, NULL);
  Set_ExprExtForDD_first_subi (x, NULL);
  Set_ExprExtForDD_first_context (x, NULL);
  return x;
}

Extension
P_ExprExtForDD_free (Extension x)
{
  DISPOSE(x); 
  return NULL;
}
