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

#ifndef _L_SSA_H
#define _L_SSA_H

#include <config.h>
#include <Lcode/l_main.h>

typedef struct _L_SSA_BaseName {
  L_Operand *opd;
  Set defnodes;
  Set pseudodefnodes;
  Set phinodes;
  List ssas;
  Set ssaset;
  List def_stk;
  ITint32 maxid;
} L_SSA_BaseName;

typedef struct _L_SSA {
  ITint32 id;
  ITint32 gid;
  ITint32 dfid;      /* reg/mac index                             */
  ITint32 flags;
  L_Oper *def;
  L_Operand *defopd;
  List rd_pr;        /* predecessor reaching defs                 */
  List rd_lb;        /* loopback reaching defs (mu-function only) */
  List uses;
  L_SSA_BaseName *bn;
  int dfn;
  void *info;
} L_SSA;

extern void L_form_ssa (L_Func *fn);
extern int L_exit_ssa (L_Func *fn);

extern L_SSA *L_new_ssa (L_Oper *def_op, L_Operand *def_opd, ITint32 node_id);
extern L_SSA *L_delete_ssa (L_SSA *ssa);
extern void L_ssa_populate_uses (L_Func *fn);
extern HashTable LS_hash_ssa_id;

#endif

