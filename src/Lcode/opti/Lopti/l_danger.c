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
 *      File :          l_danger.c
 *      Description :   calculate danger information
 *      Creation Date : July, 1990
 *      Authors :       Scott Mahlke, Pohua Chang.
 *
 *      (C) Copyright 1990, Pohua Chang, Scott Mahlke.
 *      All rights granted to University of Illinois Board of Regents.
 *==========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "l_opti.h"

static L_Danger_Ext *
L_new_danger_ext ()
{
  L_Danger_Ext *ext;

  ext = (L_Danger_Ext *) L_alloc (L_alloc_danger_ext);
  ext->identifier = L_DANGER_EXT_IDENTIFIER;
  ext->sub_call_between = NULL;
  ext->general_sub_call_between = NULL;
  ext->sync_between = NULL;
  ext->store_between = NULL;

  return ext;
}

void
L_delete_danger_ext (L_Cb * cb)
{
  L_Danger_Ext *ext;

  if (cb->ext == NULL)
    return;

  ext = cb->ext;
  if (ext->identifier != L_DANGER_EXT_IDENTIFIER)
    L_punt ("L_delete_danger_ext: unknow cb extension on cb %d\n", cb->id);

  /* reset the cb */
  cb->ext = NULL;

  /* free sets if necessary */
  if (ext->sub_call_between != NULL)
    Set_dispose (ext->sub_call_between);
  if (ext->general_sub_call_between != NULL)
    Set_dispose (ext->general_sub_call_between);
  if (ext->sync_between != NULL)
    Set_dispose (ext->sync_between);
  if (ext->store_between != NULL)
    Set_dispose (ext->store_between);

  /* free the danger extension */
  L_free (L_alloc_danger_ext, ext);
}

void
L_delete_all_danger_ext (L_Func * fn)
{
  L_Cb *cb;
  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      if (cb->ext != NULL)
        L_delete_danger_ext (cb);
    }
}

void
L_print_all_danger_info (L_Func * fn)
{
  L_Cb *cb;
  L_Danger_Ext *ext;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      ext = cb->ext;
      fprintf (stderr, "Danger info cb %d\n", cb->id);
      Set_print (stderr, "sub_call_between", ext->sub_call_between);
      Set_print (stderr, "general_sub_call_between",
                 ext->general_sub_call_between);
      Set_print (stderr, "sync_between", ext->sync_between);
      Set_print (stderr, "store_between", ext->store_between);
    }
}

static void
L_reset_visited_flag (L_Func * fn)
{
  L_Cb *cb;
  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      cb->flags = L_CLR_BIT_FLAG (cb->flags, L_CB_VISITED);
    }
}

static void
L_search_predecessors (int *sub_call, int *gen_sub_call, int *sync,
                       int *store, L_Cb * cb, L_Cb * dest_cb)
{
  L_Flow *src;
  L_Cb *src_cb;
  for (src = cb->src_flow; src != NULL; src = src->next_flow)
    {
      src_cb = src->src_cb;
      if (src_cb == dest_cb)
        continue;
      if (L_EXTRACT_BIT_VAL (src_cb->flags, L_CB_VISITED))
        continue;
      if (L_EXTRACT_BIT_VAL (src_cb->flags, L_CB_SUB_CALL))
        *sub_call = 1;
      if (L_EXTRACT_BIT_VAL (src_cb->flags, L_CB_GENERAL_SUB_CALL))
        *gen_sub_call = 1;
      if (L_EXTRACT_BIT_VAL (src_cb->flags, L_CB_SYNC))
        *sync = 1;
      if (L_EXTRACT_BIT_VAL (src_cb->flags, L_CB_STORE))
        *store = 1;
      src_cb->flags = L_SET_BIT_FLAG (src_cb->flags, L_CB_VISITED);
      L_search_predecessors (sub_call, gen_sub_call, sync, store, src_cb,
                             dest_cb);
    }
}

/*
 *      Danger information - used for inter basic block optimizations
 *         sub_call_flag = 1 if there is a jsr operation in cb
 *         general_sub_call_flag = 1 if there is op that will result in func
 *                     call in the cb (ex is divide in many arch)
 *         sync_flag = 1 if there is co_processor, break, trap or sync op in cb
 *         store_flag = 1 if there is a store op in cb
 */
void
L_compute_danger_info (L_Func * fn)
{
  int i, num_dom, *dom;
  L_Cb *cb, *cb1, *cb2;
  L_Oper *oper;
  L_Danger_Ext *ext;
  Set dom_set;

  /*
   *  De-allocate any previous danger info
   */
  L_delete_all_danger_ext (fn);

  /*
   *  Initialization
   */
  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {

      if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_EPILOGUE | L_CB_PROLOGUE) &&
          L_EXTRACT_BIT_VAL (cb->flags, L_CB_BOUNDARY))
        continue;

      ext = L_new_danger_ext ();
      cb->ext = ext;
      cb->flags = L_CLR_BIT_FLAG (cb->flags, L_CB_SUB_CALL);
      cb->flags = L_CLR_BIT_FLAG (cb->flags, L_CB_GENERAL_SUB_CALL);
      cb->flags = L_CLR_BIT_FLAG (cb->flags, L_CB_SYNC);
      cb->flags = L_CLR_BIT_FLAG (cb->flags, L_CB_STORE);
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
        {
          if ((L_subroutine_call_opcode (oper)) &&
              (!(L_side_effect_free_sub_call (oper))))
            cb->flags = L_SET_BIT_FLAG (cb->flags, L_CB_SUB_CALL);
          if (L_general_subroutine_call_opcode (oper))
            cb->flags = L_SET_BIT_FLAG (cb->flags, L_CB_GENERAL_SUB_CALL);
          if (L_sync_opcode (oper))
            cb->flags = L_SET_BIT_FLAG (cb->flags, L_CB_SYNC);
          if (L_general_store_opcode (oper))
            cb->flags = L_SET_BIT_FLAG (cb->flags, L_CB_STORE);
        }
    }
  dom = (int *) Lcode_malloc (sizeof (int) * fn->n_cb);
  for (cb2 = fn->first_cb; cb2 != NULL; cb2 = cb2->next_cb)
    {

      if (L_EXTRACT_BIT_VAL (cb2->flags, L_CB_EPILOGUE | L_CB_PROLOGUE) &&
          L_EXTRACT_BIT_VAL (cb2->flags, L_CB_BOUNDARY))
        continue;

      ext = cb2->ext;
      dom_set = L_get_cb_DOM_set (cb2);
      num_dom = Set_size (dom_set);
      Set_2array (dom_set, dom);
      for (i = 0; i < num_dom; i++)
        {
          int sub_call, gen_sub_call, sync, store;
          cb1 = L_cb_hash_tbl_find (fn->cb_hash_tbl, dom[i]);
          if (cb1 == cb2)
            continue;
          /* search backwards from cb2 along all paths until cb1 is reached */
          sub_call = gen_sub_call = sync = store = 0;
          L_reset_visited_flag (fn);
          L_search_predecessors (&sub_call, &gen_sub_call, &sync, &store, cb2,
                                 cb1);
          if (sub_call == 1)
            ext->sub_call_between = Set_add (ext->sub_call_between, cb1->id);
          if (gen_sub_call == 1)
            ext->general_sub_call_between =
              Set_add (ext->general_sub_call_between, cb1->id);
          if (sync == 1)
            ext->sync_between = Set_add (ext->sync_between, cb1->id);
          if (store == 1)
            ext->store_between = Set_add (ext->store_between, cb1->id);
        }
    }
  Lcode_free (dom);
}
