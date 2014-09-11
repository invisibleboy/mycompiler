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
 *
 *      File :          i_list.h
 *      Description :   List function header file.
 *      Creation Date : September 1996
 *      Author :        David August
 *
 *      Copyright (c) 1996 David August, Wen-mei Hwu and 
 *                         The Board of Trustees of the University of Illinois.
 *                         All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
 *===========================================================================*/

#ifndef LIST_H
#define LIST_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <library/l_alloc_new.h>

typedef struct list_element
{
  void *ptr;
  struct list_element *next;
  struct list_element *prev;
}
 *ListElement, _ListElement;

typedef struct list
{
  int size;
  int num_currents;
  ListElement *current;
  ListElement first;
  ListElement last;
}
 *List, _List;

#ifdef __cplusplus
extern "C"
{
#endif

/*====================
 * List Globals
 *====================*/
  extern L_Alloc_Pool *ListPool;
  extern L_Alloc_Pool *ListElementPool;

/*====================
 * Function Prototypes
 *====================*/
  extern int List_size (List list);

/* These functions use the default pointer */
  extern void *List_get_first (List list);
  extern void *List_get_last (List list);
  extern void *List_get_next (List list);
  extern void *List_next (List list);
  extern void *List_prev (List list);
  extern void *List_current (List list);
  extern void *List_first (List list);
  extern void *List_last (List list);
  extern void List_start (List list);

/* These functions consume their input lists */
  extern List List_delete_current (List list);
  extern List List_reset (List list);
  extern List List_reset_free_ptrs (List list, void (*free_fn) (void *));
  extern List List_insert_first (List list, void *ptr);
  extern List List_insert_last (List list, void *ptr);
  extern List List_insert_prev (List list, void *ptr);
  extern List List_append (List list, List append_list);
  extern List List_insert_next (List list, void *ptr);

/* These functions use an explicit pointer */
  extern void *List_get_next_l (List list, int level);
  extern void *List_next_l (List list, int level);
  extern void *List_prev_l (List list, int level);
  extern void *List_current_l (List list, int level);
  extern void *List_first_l (List list, int level);
  extern void *List_last_l (List list, int level);
  extern void List_start_l (List list, int level);
  extern List List_delete_current_l (List list, int level);
  extern List List_insert_prev_l (List list, void *ptr, int level);
  extern List List_insert_next_l (List list, void *ptr, int level);

/* These functions manipulate pointers */
  extern void List_copy_current_ptr (List list, int to_level, int from_level);
  extern int List_max_registered_current(List list);

  extern int List_register_new_ptr (List list);
  extern void List_free_all_ptrs (List list);

/* Set like functions */
  extern List List_remove (List list, void *ptr);

  extern List List_copy (List list);

  extern void List_change_current (List list, void *new_ptr);
  extern void List_change_current_l (List list, void *new_ptr, int level);

  extern int List_member (List list, void *ptr);


#ifdef __cplusplus
}
#endif

#endif
