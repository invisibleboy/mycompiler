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
 *      File:   c_symbol2.h
 *      Author: Po-hua Chang
 *      Creation Date:  June 1990
 *      Modified By: XXX, date, time, why
 *      Copyright (c) 1991 Po-hua Chang and Wen-mei Hwu. All rights reserved.  
 *      All rights granted to the University of Illinois.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

/*===========================================================================
 *      Description :   Multi-level symbol table.
 *==========================================================================*/
#ifndef C_SYMBOL2_H
#define C_SYMBOL2_H
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <library/c_symbol.h>

/*
 *      We maintain an unique scope_id number with each
 *      lexical scope we encounter. The same scope_id
 *      information is written into every symbol, so we
 *      know exactly which lexical scope every symbol
 *      belongs to. This will help us flatten out the
 *      symbol table in the semantic analyzer portion.
 *      C2_CurrentScope designates the ID number of the current
 *      lexical scope. C2_PeakScope designates the highest ID
 *      we have achieved. When a new scope is desired, we
 *      assign it with (C2_PeakScope+1).
 *
 *      Use C_MATCH_BY_AND_TYPE.
 *
 *      EXPORT functions :
 *
 *      C2_init_scope(n_scope, s_global, s_local);
 *              This function must be called before any of the other
 *              functions are called. It initialize the entire symbol
 *              table the first time it is called.
 *              The current scope is set to default at level 0.
 *              n_scope is the number of scopes that we need.
 *              s_global is the maximum size of the global symbol
 *              table. s_local is the size of all local symbol tables.
 *      C2_kill_scope_above(N)
 *      int N;
 *              Kill all definition at and above level N.
 *              One must be very sure that the definition
 *              will not be accessed again. The major use of this
 *              function is that the compiler front-end can
 *              recycle all space after processing each function.
 *              This is safe if information does not live across
 *              function boundary (except level 0). Therefore,
 *              C2_kill_scope_above(1) is called after processing a
 *              function. C2_kill_scope_above(0) is called after processing 
 *              the entire file, since in C each file can be treated
 *              as an independent entity.
 *              If (N<=nest), the current scope is set to point to max(N-1,0).
 *              Otherwise, the current scope is maintained.
 *      C2_reset_attribute(n);
 *              int n;
 *              Clear all extension fields of all symbols above level N.
 *      C2_scope_in();
 *              Create a new scope in nest+1 level.
 *              Set the newly created scope to be the current scope.
 *      C2_scope_out();
 *              Return to the previous level (nest-1).
 *              Do not deallocate the scopes
 *
 *      int C2_find_symbol(name, type, &value, &ptr)
 *              Search from the current scope to lower level scopes
 *              for the first symbol with the specified name.
 *              Return nil_sym if the symbol is never defined
 *              in reachable scopes. Notice that the symbol may
 *              be defined in scopes unreachable from the current
 *              scope. In such cases, we treat as search failure.
 *              Match symbols of type specified in type only.
 *              Return the scope id where the symbol was found.
 *              If no such symbol, return -1.
 *
 *      int C2_update_symbol(name, type, value, ptr)
 *              Add a symbol to the current level.
 *              If the symbol has been defined in the current level,
 *              simply add the type information into the symbol.
 *              Return the scope id where the symbol was added.
 *               
 *      int C2_find_local_symbol(name, type, &value, &ptr);
 *              Search only the current scope. Returns nil_sym if not found.
 *              Match symbols of type specified in type only.
 *              Return the scope id where the symbol was found.
 *              If no such symbol, return -1.
 *
 *      int C2_update_local_symbol(name, type, value, ptr)
 *              Search only the current scope.
 *              Add a symbol to the current level if not yet defined.
 *              If the symbol has been defined in the current level,
 *              simply add the type information into the symbol.
 *              Return the scope id where the symbol was added.
 *               
 *      int C2_find_global_symbol(name, type, &value, &ptr);
 *              Search only the function level.
 *              Returns nil_sym if nothing found.
 *              Match symbols of type specified in type only.
 *              Return the scope id where the symbol was found.
 *              If no such symbol, return -1.
 *
 *      int C2_update_global_symbol(name, type, value, ptr)
 *              Search only the function level scope.
 *              Add a symbol to the function level if not yet defined.
 *              If the symbol has been defined,
 *              simply add the type information into the symbol.
 *              Return the scope id where the symbol was added.
 */

#define C2_MAX_SCOPE            20

#ifdef __cplusplus
extern "C"
{
#endif

  extern int C2_init_scope (int num_scope, int s_global, int s_local);
  extern void C2_kill_scope_above (int n);
  extern void C2_reset_attribute (int n);
  extern int C2_scope_in (void);
  extern int C2_scope_out (void);
  extern int C2_find_symbol (char *name, int type, Integer * value,
                             Pointer * ptr);
  extern int C2_update_symbol (char *name, int type, Integer value,
                               Pointer ptr);
  extern int C2_find_local_symbol (char *name, int type, Integer * value,
                                   Pointer * ptr);
  extern int C2_update_local_symbol (char *name, int type, Integer value,
                                     Pointer ptr);
  extern int C2_find_global_symbol (char *name, int type, Integer * value,
                                    Pointer * ptr);
  extern int C2_update_global_symbol (char *name, int type, Integer value,
                                      Pointer ptr);
  extern int C2_forall (C_Function fn);
  extern int C2_forall_global (C_Function fn);

#ifdef __cplusplus
}
#endif

#endif
