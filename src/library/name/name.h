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
 *      File:   name.h
 *      Author: Pohua Paul Chang
 *      Copyright (c) 1991 Pohua Paul Chang, Wen-Mei Hwu. All rights reserved.  
 *      All rights granted to the University of Illinois.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/
#ifndef IMPACT_NAME_H
#define IMPACT_NAME_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
/*
 *      Variable renaming is a common technique in compilation.
 *      This package of functions can be used for building a such
 *      optimization.
 *
 *      Each symbol should have a unique name within a certain
 *      compilation scope (e.g. function). However, when symbols
 *      are moved across the boundary of symbol scopes (inter-procedural
 *      code motion, inline expansion), renaming is often necessary
 *      to eliminate naming conflict.
 *
 *      We can model each scope as a set of unique names. When adding
 *      a new symbol into the set, one must first check that there is
 *      no naming conflict.
 *
 *      A scope is a set of symbols.
 *      The initialization routine simply assigns everything to zero.
 *      The old_name field of a symbol corresponds to the original
 *      name given to the symbol. One or more symbols in a scope
 *      may have identical old_name. However, the new_name will be
 *      made unique automatically by the system.
 *      We provide U_MAX_SCOPE number of independent scopes.
 *      1) int U_InitScope(size)
 *              int size;
 *      The user can also recycle a scope.
 *      2) void U_ClearScope(scope)
 *              int scope;
 */
#define U_MAX_SCOPE             20

#ifdef __cplusplus
extern "C"
{
#endif

  extern int U_InitScope (int size);
  extern void U_ClearScope (int id);

#ifdef __cplusplus
}
#endif

/*
 *      When adding a new symbol, if the original name of the
 *      symbol does not conflict with the new_name of any symbol,
 *      a new entry can be quickly created (new_name=old_name).
 *      3) char *U_AddName(scope, name, rename)
 *              int scope;
 *              char *name;
 *              int *rename;
 *      If there is a conflict, a new_name is automatically
 *      created by appending to the old_name a suffix and a
 *      number. For example, ABC --> ABC_$4 (suffix= "_$")
 *      The rename argument will be set to 0 if no conflict, or
 *      to 1 if renaming was necessary.
 *      In any case, the new name is returned.
 *      The user can choose his/her favorite suffix.
 *      4) void U_UseSuffix(scope, suffix)
 *              int scope;
 *              char *suffix;
 *      The system guarantees that the new_name generated does
 *      not conflict with any of the existing new_names.
 */
#define U_DEFAULT_SUFFIX                "_$"

#ifdef __cplusplus
extern "C"
{
#endif

  extern char *U_AddName (int id, char *name, int *rename);
  extern void U_UseSuffix (int id, char *suffix);
  extern int U_Defined (int id, char *name);

#ifdef __cplusplus
}
#endif

#endif
