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
 *      File:   func_list.h
 *
 *      Description: Function name list that offers quick access and removes 
 *                   the random renaming done on static function names.
 *
 *      Creation Date: June 1999
 *
 *      Authors: John C. Gyllenhaal and Wen-mei Hwu
 *
 *      Revisions:
 *
\*****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <stdio.h>
#include <ctype.h>
#include <library/i_error.h>
#include "func_list.h"

/* Returns the length of the function name that
 * should be considered during comparison, handling
 * EDG's renaming scheme for static variables:
 * foo_file_i_6543___FXxxxx
 *         ^------- last relevant character in name.
 *                 ^------ keys off of three ___
 * For non-static (and therefore non-renamed) function
 * names, this will return the same answer as strlen().
 * 
 * This function will consider whitespace a terminating
 * character, so that pointers into 'name_list' can
 * be passed to this function directly.
 */
static int
relevant_len (char *func_name)
{
  int len, ren_len;
  char *ptr, *ren_ptr;

  len = 0;
  ptr = func_name;

  while ((*ptr != 0) && !isspace (*ptr))
    {
      /* Look for a digit followed by three '_', this should
       * signal a renaming has occured.  The minimum
       * pattern that will be matched is 'a_b_i_1___Fxx'
       *                                         ^---
       * So no need to test until character 8 (len == 7).
       */
      if ((len >= 7) &&
          (ptr[0] == '_') &&
          (ptr[1] == '_') && (ptr[2] == '_') && (isdigit (ptr[-1])))
        {
          /* Goto just before number:
           *   'a_b_i_1___Fxx'
           *         ^------
           */
          ren_ptr = ptr - 1;
          ren_len = len - 1;
          while ((ren_len >= 5) && isdigit (ren_ptr[0]))
            {
              ren_ptr--;
              ren_len--;
            }

          /* Do we have match? */
          if ((ren_ptr[0] == '_') &&
              (ren_ptr[-1] == 'i') && (ren_ptr[-2] == '_'))
            {
              /* Yes, return length until first '_':
               *    'a_b_i_1___Fxx'
               *        ^---------- (return 4 here)
               */
              return (ren_len - 1);
            }
        }
      ptr++;
      len++;
    }

  return (len);
}

/* Create function name list from space separated list of
 * function names, that may have EDG's renaming performed
 * on them (if static function names).
 */
Func_Name_List *
new_Func_Name_List (char *name_list)
{
  CHAR_ARRAY_Symbol_Table *name_table;
  char *ptr;
  int len;

  name_table = CHAR_ARRAY_new_symbol_table ("Func_Name_List", 0);

  /* Put the relevant portion of each name in the table */
  if ((ptr = name_list))
    {
      while (*ptr != 0)
	{
	  /* Strip leading whitespace */
	  if (isspace (*ptr))
	    {
	      ptr++;
	      continue;
	    }

	  /* Get relevant length of name */
	  len = relevant_len (ptr);

	  /* Sanity check, better be > 1 */
	  if (len < 1)
	    I_punt ("new_Func_Name_List: len 0 unexpected!\n");
	  
	  /* Add if not already in table */
	  if (CHAR_ARRAY_find_symbol (name_table, ptr, len) == NULL)
	    {
	      CHAR_ARRAY_add_symbol (name_table, ptr, len, NULL);
	    }
	  
	  /* Goto next name */
	  while ((*ptr != 0) && !isspace (*ptr))
	    ptr++;
	  
	  while ((*ptr != 0) && isspace (*ptr))
	    ptr++;
	}
    }

  /* Return name table as function list */
  return ((Func_Name_List *) name_table);
}

/* Delete list using table delete routine */
void *
delete_Func_Name_List (Func_Name_List * list)
{
  CHAR_ARRAY_Symbol_Table *table;

  /* Sanity check */
  if (list == NULL)
    I_punt ("delete_Func_Name_List: list is NULL!");

  table = (CHAR_ARRAY_Symbol_Table *) list;
  CHAR_ARRAY_delete_symbol_table (table, NULL);
  return (NULL);
}

/* Returns 1 if the relevant portion of func_name is in
 * the function list.
 */
int
is_in_Func_Name_List (Func_Name_List * list, char *func_name)
{
  int len;
  CHAR_ARRAY_Symbol_Table *table;

  /* Sanity check */
  if (list == NULL)
    I_punt ("is_in_Func_Name_List: list is NULL!");

  /* Get relevant length of func_name */
  len = relevant_len (func_name);

  /* Expect to be >= 1 */
  if (len < 1)
    {
      I_punt ("is_in_Func_Name_List: '%s' has relevant length %i!",
              func_name, len);
    }

  table = (CHAR_ARRAY_Symbol_Table *) list;
  if (CHAR_ARRAY_find_symbol (table, func_name, len) != NULL)
    return (1);
  else
    return (0);
}
