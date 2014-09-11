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
 *      File:  l_symbol.c
 *      Author: John C. Gyllenhaal, Wen-mei Hwu
 *      Creation Date:  October 1994
 *      Modified:  Richard E. Hank - derived from John's preprocessor
 *                      symbol tables.  The purpose is to improve the
 *                      symbol table capabilities of Lcode.
\*****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_symbol.h>

/*
 * Lcode symbol table management
 */
STRING_Symbol_Table *
L_new_symbol_table (char *name, int size)
{
  return (STRING_new_symbol_table (name, size));
}

/*
 * Lcode id symbol table management -ITI/JWJ 7/99
 */
INT_Symbol_Table *
L_new_id_symbol_table (char *name, int size)
{
  return (INT_new_symbol_table (name, size));
}

/*
 * Add the string <name> to the symbol table and return
 * a pointer to that string in the symbol table
 */
char *
L_add_string (STRING_Symbol_Table * table, char *name)
{
  STRING_Symbol *symbol = STRING_find_symbol (table, name);

  if (symbol == NULL)
    {
      symbol = STRING_add_symbol (table, name, NULL);
    }
  return (symbol->name);
}

int
L_find_symbol_id (STRING_Symbol_Table * table, char *name)
{
  STRING_Symbol *symbol = STRING_find_symbol (table, name);

  if (symbol == NULL)
    return (-1);
  else
#ifdef LP64_ARCHITECTURE
    return ((int)((long)symbol->data));
#else
    return ((int) symbol->data);
#endif
}

/* ITI/JWJ 7/99 */
char *
L_find_id_symbol (INT_Symbol_Table * table, int id)
{
  INT_Symbol *symbol = INT_find_symbol (table, id);

  if (symbol == NULL)
    return ((char *) (-1));
  else
    return ((char *) symbol->data);
}
