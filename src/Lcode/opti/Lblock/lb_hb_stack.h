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
/*===========================================================================
 *      File :          l_hb_stack.h
 *      Description :   Generic stack manipulation
 *      Creation Date : February 1994
 *      Authors :       Scott Mahlke
 *       Included with Lblock in its original form from Lhyper -- KMC 4/98 
 *
 *==========================================================================*/
#ifndef L_HB_STACK_H
#define L_HB_STACK_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>
#include <library/set.h>

/*==========================================================================*/
/*
 *      Stack data structure
 */
/*==========================================================================*/

typedef struct _Stack_item
{
  int data;
  struct _Stack_item *prev_item;
  struct _Stack_item *next_item;
}
Stack_item;

typedef struct _Stack
{
  struct _Stack_item *head;
  struct _Stack_item *tail;
  int size;
}
Stack;

#ifdef __cplusplus
extern "C"
{
#endif

/*==========================================================================*/
/*
 *      Stack manipulation functions
 */
/*==========================================================================*/

  extern Stack_item *Stack_item_create (void);
  extern Stack *Stack_create (void);
  extern void Stack_item_delete (Stack_item *);
  extern void Stack_delete (Stack *);

  extern void Stack_print (FILE *, Stack *);

  extern int Stack_size (Stack *);
  extern int Stack_in (Stack *, int);
  extern void Stack_clear (Stack *);
  extern int Stack_first (Stack *);
  extern int Stack_last (Stack *);
  extern int Stack_get_contents (Stack *, int **);
  extern Set Stack_get_content_set (Stack *);
  extern void Stack_push_top (Stack *, int);
  extern void Stack_push_bottom (Stack *, int);
  extern int Stack_pop (Stack *);

#ifdef __cplusplus
}
#endif

#endif
