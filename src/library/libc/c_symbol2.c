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
 *      File:   c_symbol2.c
 *      Author: Po-hua Chang 
 *      Creation Date:  June 1990
 *      Modified By: XXX, date, time, why
 *      Copyright (c) 1991 Po-hua Chang and Wen-mei Hwu. All rights reserved.  
 *      All rights granted to the University of Illinois.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

/*===========================================================================
 *      Description :   Multiple level symbol table.
 *==========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <library/c_symbol2.h>

static int scope[C2_MAX_SCOPE];
static int scope_id[C2_MAX_SCOPE];
static int next_scope_id = 0;
static int n_scope = 0;

int
C2_init_scope (int num_scope, int s_global, int s_local)
{
  int i;
  scope[0] = C_open (s_global, C_MATCH_BY_AND_TYPE);
  for (i = 1; i < num_scope; i++)
    scope[i] = C_open (s_local, C_MATCH_BY_AND_TYPE);
  for (i = 0; i < num_scope; i++)
    if (scope[i] == -1)
      {
        C_assert (0, "C2_init_scope: failed to open table");
        return 0;
      }
  next_scope_id = 0;            /* start from a fresh scope counter */
  n_scope = 0;                  /* start from the global scope */
  C2_scope_in ();               /* activate global level */
  return 1;
}

void
C2_kill_scope_above (int n)
{
  int i;
  if (n < 0)
    C_assert (0, "C2_kill_scope_above: argument can not be negative");
  if (n < n_scope)
    {
      for (i = n; i < n_scope; i++)
        {
          C_clear (scope[i]);
        }
      n_scope = n;
    }
}

void
C2_reset_attribute (int n)
{
  int i;
  if (n < 0)
    C_assert (0, "C2_reset_attribute: argument can not be negative");
  for (i = n; i < n_scope; i++)
    {
      C_clear_attr (scope[i]);
    }
}

int
C2_scope_in (void)
{
  int i;
  i = n_scope++;
  if (i >= C2_MAX_SCOPE)
    C_assert (0, "C2_scope_in: too deep");
  C_clear (scope[i]);
  scope_id[i] = next_scope_id++;
  return scope_id[i];
}

int
C2_scope_out (void)
{
  n_scope--;
  if (n_scope < 1)
    C_assert (0, "C2_scope_out: underflow");
  return scope_id[n_scope - 1];
}

int
C2_find_symbol (char *name, int type, Integer * value, Pointer * ptr)
{
  int i, s;
  s = 0;
  for (i = n_scope - 1; (i >= 0) && (!s); i--)
    {
      s = C_find (scope[i], name, type, value, ptr);
    }
  if (s)
    return scope_id[i];
  else
    return -1;
}

int
C2_update_symbol (char *name, int type, Integer value, Pointer ptr)
{
  int i;
  i = n_scope - 1;
  C_update (scope[i], name, type, value, ptr);
  return scope_id[i];
}

int
C2_find_local_symbol (char *name, int type, Integer * value, Pointer * ptr)
{
  int i, s;
  i = n_scope - 1;
  s = C_find (scope[i], name, type, value, ptr);
  if (s)
    return scope_id[i];
  else
    return -1;
}

int
C2_update_local_symbol (char *name, int type, Integer value, Pointer ptr)
{
  int i;
  i = n_scope - 1;
  C_update (scope[i], name, type, value, ptr);
  return scope_id[i];
}

int
C2_find_global_symbol (char *name, int type, Integer * value, Pointer * ptr)
{
  int s;
  s = C_find (scope[0], name, type, value, ptr);
  if (s)
    return scope_id[0];
  else
    return -1;
}

int
C2_update_global_symbol (char *name, int type, Integer value, Pointer ptr)
{
  C_update (scope[0], name, type, value, ptr);
  return scope_id[0];
}

int
C2_forall (C_Function fn)
{
  return C_forall (scope[n_scope - 1], fn);
}

int
C2_forall_global (C_Function fn)
{
  return C_forall (scope[0], fn);
}
