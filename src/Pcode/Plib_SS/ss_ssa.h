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


#ifndef __SS_SSA_H__
#define __SS_SSA_H__

#include <stdio.h>
#include <stdlib.h>
#include <Pcode/pcode.h>
#include <Pcode/cfg.h>

/****************************************************************************/

typedef struct _P_SSALink { 
  struct _PC_Block * def_bb;
  Expr def_expr;
  struct _PC_Block * use_bb;
  Expr use_expr;
  struct _P_SSALink *next_use;
} _P_SSALink, *P_SSALink;

/****************************************************************************
        Export function header
****************************************************************************/

extern void P_CF_BuildSSA (struct _PC_Graph *cfg);
extern void P_CF_DeleteSSA (struct _PC_Graph *cfg);
extern Expr P_CF_GetSSA (Expr expr);
extern Expr P_CF_EnumReverseSSAFirst (Expr expr, 
				      struct _P_SSALink **enumerator);
extern Expr P_CF_EnumReverseSSANext (struct _P_SSALink **enumerator); 
extern void P_CF_Dump_SSA (FILE *out_file, struct _PC_Graph *cfg, char *title);
extern void P_CF_Dump_SSA_BB (FILE *out_file, struct _PC_Block * bb);
extern void P_CF_Dump_SSA_Expr (FILE *out_file, Expr expr, int n_indent);
extern void Dump_SSALink_List(FILE *out_file, void *list);

extern int PS_var_ext;

/* Extension fields: ss_ext.c */

extern void PS_def_handlers (void);

#define PS_GetVarTblEntry(v) \
   (((VarTblEntry)P_GetVarDclExtL ((v), PS_var_ext)))
#define PS_SetVarTblEntry(v,x) \
   (((VarTblEntry)P_SetVarDclExtL ((v), PS_var_ext, (x))))

#endif
