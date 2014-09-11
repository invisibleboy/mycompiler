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

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <stdlib.h>
#include "ltahoe_main.h"
#include "phase3.h"
#include <library/string_symbol.h>

static STRING_Symbol_Table *P_symtab = NULL;

#define P_SYM_TY_OBJECT     0x0001
#define P_SYM_TY_FUNC       0x0002
#define P_SYM_FL_LOCALDEF   0x0100

void
P_symtab_init (void)
{
  if (!P_symtab)
    P_symtab = STRING_new_symbol_table ("P_symtab", 1024);

  return;
}


void
P_symtab_deinit (void)
{
  if (P_symtab)
    {
      STRING_delete_symbol_table (P_symtab, NULL);
      P_symtab = NULL;
    }

  return;
}


void
P_symtab_add_label (char *label, int isfunc)
{
  if (!STRING_find_symbol (P_symtab, label))
    {
      long flags = isfunc ? P_SYM_TY_FUNC : P_SYM_TY_OBJECT;
      STRING_add_symbol (P_symtab, label, (void *) flags);
    }

  return;
}


void
P_symtab_add_def (char *label, int isfunc)
{
  STRING_Symbol *sym;

  if (!(sym = STRING_find_symbol (P_symtab, label)))
    {
      long flags;
      flags = isfunc ? P_SYM_TY_FUNC : P_SYM_TY_OBJECT;
      sym = STRING_add_symbol (P_symtab, label, (void *) flags);
    }

  sym->data = (void *) (((long) sym->data) | P_SYM_FL_LOCALDEF);

  return;
}

int
kcmp (const void *p1, const void *p2)
{
  return strcmp ((const char *) *(char **) p1, (const char *) *(char **) p2);
}

void
P_symtab_print_extern (void)
{
  char **karray;
  int kcount, i;
  STRING_Symbol *sym;

  if (!P_symtab || !(kcount = P_symtab->symbol_count))
    return;

  P_print_section_title (L_MS_DATA);

  karray = malloc (kcount * sizeof (char *));

  for (sym = P_symtab->head_symbol, i = 0; sym; sym = sym->next_symbol)
    karray[i++] = sym->name;

  qsort (karray, kcount, sizeof (char *), kcmp);

  fprintf (L_OUT, "\t// EXTERNAL FUNCTION SYMBOLS\n");

  for (i = 0; i < kcount; i++)
    {
      char *label;
      unsigned long flags;

      label = karray[i];
      flags = (unsigned long) STRING_find_symbol_data (P_symtab, label);

      if (flags & P_SYM_FL_LOCALDEF)
	continue;

      if (flags & P_SYM_TY_FUNC)
	{
	  fprintf (L_OUT, "\t.type\t%s#, @function\n", label);
	  fprintf (L_OUT, "\t.global\t%s#\n", label);
	}
    }

  fprintf (L_OUT, "\t// EXTERNAL OBJECT SYMBOLS\n");

  for (i = 0; i < kcount; i++)
    {
      char *label;
      unsigned long flags;

      label = karray[i];
      flags = (unsigned long) STRING_find_symbol_data (P_symtab, label);

      if (flags & P_SYM_FL_LOCALDEF)
	continue;

      if (!(flags & P_SYM_TY_FUNC))
	{
	  fprintf (L_OUT, "\t.global\t%s#\n", label);
	}
    }

  free (karray);
  return;
}
