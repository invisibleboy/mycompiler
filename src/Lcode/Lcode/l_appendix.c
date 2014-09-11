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
 *      File:  l_appendix.c
 *      Author: Richard E. Hank, Wen-mei Hwu
 *      Creation Date:  June 1995
\*****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>

STRING_Symbol_Table *L_cb_attribute_table = NULL;
STRING_Symbol_Table *L_oper_attribute_table = NULL;

void
L_init_appendix_symbol_tables ()
{
  if (L_oper_attribute_table == NULL)
    L_oper_attribute_table = L_new_symbol_table ("Oper Attributes", 64);
  if (L_cb_attribute_table == NULL)
    L_cb_attribute_table = L_new_symbol_table ("Cb Attributes", 64);
}

static int
L_scan_list_for_name (char *list, char *name)
{
  char *head, *tail;
  int done, found;

  found = 0;
  done = 0;
  head = list;

  while (!done)
    {
      while (isspace (*head) && *head != '\0')
        head++;

      if (*head == '\0')
        {
          done = 1;
        }
      else
        {
          tail = head;

          while (!isspace (*tail) && *tail != '\0' && *tail != '*')
            tail++;

          if (*tail == '*' && !(isspace (*(tail + 1)) || *(tail + 1) == '\0'))
            L_punt ("L_scan_list_for_name: Illegal name list -> %s\n", list);

          if (!strncmp (head, name, tail - head))
            {
              /* we have a match if tail is pointing to a '*' */
              /* or if tail is not pointing to a '*' and      */
              /* name[tail-head] == '\0'                          */
              if (*tail == '*' || name[tail - head] == '\0')
                {
                  found = 1;
                  done = 1;
                }
            }

          if (!done)
            {
              if (*tail == '*')
                tail++;

              head = tail;
            }
        }
    }
  return (found);
}

int
L_should_go_in_oper_appendix (L_Attr * attr)
{
  int action;

  /*
   * Check to see if attr->name is in symbol table
   *
   * if action == 0 -> not in symbol table
   *    action == 1 -> place in code
   *    action == 2 -> palce in appendix
   */

#ifdef LP64_ARCHITECTURE
  action = (int)((long)(STRING_find_symbol_data (L_oper_attribute_table,
						 attr->name)));
#else
  action = (int) STRING_find_symbol_data (L_oper_attribute_table, attr->name);
#endif

  if (action == 1)
    {
      return (0);
    }
  else if (action == 2)
    {
      return (1);
    }
  else
    {
      if (L_scan_list_for_name (L_oper_code_attr_list, attr->name))
        {
          STRING_add_symbol (L_oper_attribute_table, attr->name, (void *) 1);
          return (0);
        }
      else if (L_oper_default_to_appendix ||
               L_scan_list_for_name (L_oper_appendix_attr_list, attr->name))
        {
          STRING_add_symbol (L_oper_attribute_table, attr->name, (void *) 2);
          return (1);
        }
      else
        {
          STRING_add_symbol (L_oper_attribute_table, attr->name, (void *) 1);
          return (0);
        }
    }
}

int
L_should_go_in_cb_appendix (L_Attr * attr)
{
  int action;

  /*
   * Check to see if attr->name is in symbol table
   *
   * if action == 0 -> not in symbol table
   *    action == 1 -> place in code
   *    action == 2 -> palce in appendix
   */
#ifdef LP64_ARCHITECTURE
  action = (int)((long)(STRING_find_symbol_data (L_cb_attribute_table,
						 attr->name)));
#else
  action = (int) STRING_find_symbol_data (L_cb_attribute_table, attr->name);
#endif

  if (action == 1)
    return (0);
  else if (action == 2)
    return (1);
  /* 
   * Scan the attibute lists to determine location
   * of the attribute
   */
  else
    {
      /*
       * Scan the code attribute list
       */
      if (L_scan_list_for_name (L_cb_code_attr_list, attr->name))
        {
          STRING_add_symbol (L_cb_attribute_table, attr->name, (void *) 1);
          return (0);
        }
      /* 
       * Scan the appendix attribute list
       */
      else if (L_cb_default_to_appendix ||
               L_scan_list_for_name (L_cb_appendix_attr_list, attr->name))
        {
          STRING_add_symbol (L_cb_attribute_table, attr->name, (void *) 2);
          return (1);
        }
      else
        {
          STRING_add_symbol (L_cb_attribute_table, attr->name, (void *) 1);
          return (0);
        }
    }
}
