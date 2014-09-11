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
/*! \file ss_ext.c
 * \author John W. Sias and Wen-mei Hwu
 *
 * This file defines data structure etensions needed by Pcode SSA
 */

#include <config.h>
#include <Pcode/pcode.h>
#include <Pcode/extension.h>
#include "ss_ssa2.h"
#include "ss_ind_expr.h"

int PS_var_ext = -1;
int PSS_expr_ext = -1;

/* Use the ext field to store a ptr to a VarTblEntry -- no need to allocate /
 * deallocate anything here! */

void *PS_alloc_var_data (void)
{
  return NULL;
}

void *PS_free_var_data (void *d)
{
  return NULL;
}

void
PS_def_handlers (void)
{
  if (PS_var_ext == -1)
    PS_var_ext = P_ExtSetupL (ES_VAR, (Extension (*)(void))PS_alloc_var_data,
			      (Extension (*)(Extension e))PS_free_var_data);
  return;
}

/* use the ext field to store the induction expressions
 *  -- just use void pointers because it is too late
 *  to do it a better way
 */
Extension
PSS_alloc_ind_expr (void)
{
  return NULL;
}

Extension
PSS_free_ind_expr (void *e)
{
  return NULL;
}

void
PSS_def_handlers (void)
{
  if (PSS_expr_ext == -1)
    PSS_expr_ext = P_ExtSetupL (ES_EXPR, PSS_alloc_ind_expr,
				PSS_free_ind_expr);
}
