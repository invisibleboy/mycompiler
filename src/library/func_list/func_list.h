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
 *                   Used for processing parms prevent_hyperblock_functions, 
 *                   prevent_superblock_functions, etc.
 *
 *      Creation Date: June 1999
 *
 *      Authors: John C. Gyllenhaal and Wen-mei Hwu
 *
 *      Revisions:
 *
\*****************************************************************************/

#ifndef FUNC_LIST_H
#define FUNC_LIST_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <library/char_array_symbol.h>

typedef CHAR_ARRAY_Symbol_Table Func_Name_List;

#ifdef __cplusplus
extern "C"
{
#endif

  Func_Name_List *new_Func_Name_List (char *name_list);
  void *delete_Func_Name_List (Func_Name_List * list);
  int search_Func_Name_List (Func_Name_List * list, char *func_name);
  int is_in_Func_Name_List (Func_Name_List * list, char *func_name);
#ifdef __cplusplus
}
#endif

#endif
