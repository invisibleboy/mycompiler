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
 *      File:   name.c
 *      Author: Pohua Chang
 *      Copyright (c) 1991 Pohua Chang, Wen-Mei Hwu. All rights reserved.  
 *      All rights granted to the University of Illinois.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <stdio.h>
#include <library/symbol.h>
#include <library/name.h>

static struct
{
  char *suffix;                 /* suffix for renaming */
  int sym_tbl_id;               /* symbol table identification */
}
scope[U_MAX_SCOPE];
static int c_scope = 0;

int
U_InitScope (int size)
{
  int id;
  id = c_scope++;
  if (id >= U_MAX_SCOPE)        /* cannot allocate a new scope */
    return -1;
  scope[id].suffix = U_DEFAULT_SUFFIX;
  scope[id].sym_tbl_id = NewSymTbl (size);
  if (scope[id].sym_tbl_id == -1)
    return -1;                  /* lack of symbol table space */
  return id;
}

void
U_ClearScope (int id)
{
  ClearSymTbl (scope[id].sym_tbl_id);
}

char *
U_AddName (int id, char *name, int *rename)
{
  Symbol sym;
  char new_name[1024];
  sym = FindSym (scope[id].sym_tbl_id, name, 0);
  if (sym == 0)
    {                           /* no conflict */
      if (rename != 0)
        *rename = 0;
      sym = AddSym (scope[id].sym_tbl_id, name, 0);
      return sym->name;
    }
  else
    {                           /* need to rename */
      int index;
      if (rename != 0)
        *rename = 1;
      for (index = 0; 1; index++)
        {
          sprintf (new_name, "%s%s%d", name, scope[id].suffix, index);
          sym = FindSym (scope[id].sym_tbl_id, new_name, 0);
          if (sym == 0)
            {
              sym = AddSym (scope[id].sym_tbl_id, new_name, 0);
              break;
            }
        }
      return sym->name;
    }
}

void
U_UseSuffix (int id, char *suffix)
{
  scope[id].suffix = StrSave (suffix);
}

int
U_Defined (int id, char *name)
{
  Symbol sym;
  sym = FindSym (scope[id].sym_tbl_id, name, 0);
  return (sym != 0);
}


#ifdef DEBUG_NAME
#include <stdio.h>

void
main ()
{
  int id, rename;
  id = U_InitScope (1025);
  printf ("> %s %s\n", "abc", U_AddName (id, "abc", &rename));
  printf ("> %s %s\n", "abc", U_AddName (id, "abc", &rename));
  printf ("> %s %s\n", "abc", U_AddName (id, "abc", &rename));
  U_UseSuffix (id, "___");
  printf ("> %s %s\n", "abc", U_AddName (id, "abc", &rename));
  printf ("> %s %s\n", "abc", U_AddName (id, "abc", &rename));
  U_ClearScope (id);
  printf ("> %s %s\n", "abc", U_AddName (id, "abc", &rename));
  printf ("> %s %s\n", "ab", U_AddName (id, "ab", &rename));
}
#endif
