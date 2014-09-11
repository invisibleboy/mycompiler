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
 *      File :          l_branch.c
 *      Description :   classify branches (analysis only, no opti here :P)
 *      Creation Date : October 1994
 *      Authors :       Scott Mahlke
 *
 *      (C) Copyright 1994 Scott Mahlke.
 *      All rights granted to University of Illinois Board of Regents.
 *==========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "l_opti.h"

/*===========================================================================*/
/*
 *
 *      Mark branches with bit flags to easily recognize in later analysis
 */
/*===========================================================================*/

void
L_mark_branches (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *oper;

  L_clear_all_reserved_oper_flags (fn);

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
        {
          if (L_cond_branch (oper))
            oper->flags = L_SET_BIT_FLAG (oper->flags, L_OPER_CBR);
          else if (L_uncond_branch (oper))
	    oper->flags = L_SET_BIT_FLAG (oper->flags, L_OPER_UBR);
          else if (L_register_branch_opcode (oper))
            oper->flags = L_SET_BIT_FLAG (oper->flags, L_OPER_JRG);
          else if (L_subroutine_call_opcode (oper))
            oper->flags = L_SET_BIT_FLAG (oper->flags, L_OPER_JSR);
          else if (L_subroutine_return_opcode (oper))
            oper->flags = L_SET_BIT_FLAG (oper->flags, L_OPER_RTS);
        }
    }
  return;
}


/*===========================================================================*/
/*
 *      Classifying branches by class (loop-back, loop-exit, nonloop)
 *      and location (inner-loop, outer-loop, non-loop)
 */
/*===========================================================================*/

static void
L_remove_old_classifications (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *oper;
  L_Attr *attr, *next;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
        {
          if (!(oper->flags & (L_OPER_CBR | L_OPER_UBR | L_OPER_JSR |
                               L_OPER_RTS | L_OPER_JRG)))
            continue;
          for (attr = oper->attr; attr != NULL; attr = next)
            {
              next = attr->next_attr;
              if (!strcmp (attr->name, L_BR_LOOPBACK_INNER_NAME) ||
                  !strcmp (attr->name, L_BR_LOOPBACK_OUTER_NAME) ||
                  !strcmp (attr->name, L_BR_LOOPEXIT_INNER_NAME) ||
                  !strcmp (attr->name, L_BR_LOOPEXIT_OUTER_NAME) ||
                  !strcmp (attr->name, L_BR_NONLOOP_INNER_NAME) ||
                  !strcmp (attr->name, L_BR_NONLOOP_OUTER_NAME) ||
                  !strcmp (attr->name, L_BR_NONLOOP_STLN_NAME) ||
                  !strcmp (attr->name, L_BR_LOOPNEST_NAME))
                {
                  oper->attr = L_delete_attr (oper->attr, attr);
                }
            }
        }
    }
}

static void
L_do_classify_branches (L_Func * fn)
{
  int i, j, num, *buf = NULL, *branch_class = NULL;
  int nonloop_flag, loopexit_flag, loopback_flag;
  int num_loops, *loop_array = NULL;
  L_Loop *loop;
  L_Cb *cb, *dest_cb;
  L_Oper *oper;
  L_Attr *attr;

  buf = (int *) Lcode_malloc (sizeof (int) * fn->n_cb);
  branch_class = (int *) Lcode_calloc ((fn->max_oper_id + 1), sizeof (int));

  num_loops = 0;
  for (loop = fn->first_loop; loop != NULL; loop = loop->next_loop)
    {
      num_loops++;
    }

  if (num_loops > 0)
    {
      loop_array = (int *) Lcode_malloc (sizeof (int) * num_loops);
      L_sort_loops (loop_array, num_loops);
    }


  for (i = 0; i < num_loops; i++)
    {
      loop = L_find_loop (fn, loop_array[i]);

      if (loop->nested_loops == NULL)
        {
          loopback_flag = L_BR_LOOPBACK_INNER;
          loopexit_flag = L_BR_LOOPEXIT_INNER;
          nonloop_flag = L_BR_NONLOOP_INNER;
        }
      else
        {
          loopback_flag = L_BR_LOOPBACK_OUTER;
          loopexit_flag = L_BR_LOOPEXIT_OUTER;
          nonloop_flag = L_BR_NONLOOP_OUTER;
        }

      /* record backedges */
      num = Set_2array (loop->loop_cb, buf);
      for (j = 0; j < num; j++)
        {
          cb = L_cb_hash_tbl_find (fn->cb_hash_tbl, buf[j]);
          for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
            {
              if (!(L_EXTRACT_BIT_VAL (oper->flags, L_OPER_CBR | L_OPER_UBR)))
                continue;
              if (branch_class[oper->id] != 0)
                continue;
              dest_cb = L_find_branch_dest (oper);

              /* check if loop back branch */
              if (Set_in (loop->back_edge_cb, cb->id))
                {
                  if (dest_cb == loop->header)
                    {
                      branch_class[oper->id] |= loopback_flag;
                    }
                  else if ((L_EXTRACT_BIT_VAL (oper->flags, L_OPER_CBR)) &&
                           (oper == cb->last_op) &&
                           (cb->next_cb == loop->header))
                    {
                      branch_class[oper->id] |= loopback_flag;
                    }
                }

              /* check if loop exit */
              if (Set_in (loop->exit_cb, cb->id))
                {
                  if (!Set_in (loop->loop_cb, dest_cb->id))
                    branch_class[oper->id] |= loopexit_flag;
                  else if ((L_EXTRACT_BIT_VAL (oper->flags, L_OPER_CBR)) &&
                           (oper == cb->last_op) &&
                           (!Set_in (loop->loop_cb, cb->next_cb->id)))
                    branch_class[oper->id] |= loopexit_flag;
                }

              /* if unmarked, mark as forward branch */
              if (branch_class[oper->id] == 0)
                branch_class[oper->id] |= nonloop_flag;
            }
        }
    }

  /* mark remaining branches */
  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
        {
          if (!(L_EXTRACT_BIT_VAL (oper->flags, L_OPER_CBR | L_OPER_UBR)))
            continue;
          if (branch_class[oper->id] == 0)
            branch_class[oper->id] |= L_BR_NONLOOP_STLN;
        }
    }

  /* install attributes on ops */
  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
        {
          if (!(L_EXTRACT_BIT_VAL (oper->flags, L_OPER_CBR | L_OPER_UBR)))
            continue;

          if (cb->deepest_loop)
            {
              attr = L_new_attr (L_BR_LOOPNEST_NAME, 1);
              oper->attr = L_concat_attr (oper->attr, attr);
              L_set_int_attr_field (attr, 0, cb->deepest_loop->nesting_level);
            }

          if (branch_class[oper->id] == 0)
            {
              L_punt ("L_do_classify_branches: no class for %d", oper->id);
            }
          if (branch_class[oper->id] & L_BR_LOOPBACK_INNER)
            {
              attr = L_new_attr (L_BR_LOOPBACK_INNER_NAME, 0);
              oper->attr = L_concat_attr (oper->attr, attr);
            }
          if (branch_class[oper->id] & L_BR_LOOPBACK_OUTER)
            {
              attr = L_new_attr (L_BR_LOOPBACK_OUTER_NAME, 0);
              oper->attr = L_concat_attr (oper->attr, attr);
            }
          if (branch_class[oper->id] & L_BR_LOOPEXIT_INNER)
            {
              attr = L_new_attr (L_BR_LOOPEXIT_INNER_NAME, 0);
              oper->attr = L_concat_attr (oper->attr, attr);
            }
          if (branch_class[oper->id] & L_BR_LOOPEXIT_OUTER)
            {
              attr = L_new_attr (L_BR_LOOPEXIT_OUTER_NAME, 0);
              oper->attr = L_concat_attr (oper->attr, attr);
            }
          if (branch_class[oper->id] & L_BR_NONLOOP_INNER)
            {
              attr = L_new_attr (L_BR_NONLOOP_INNER_NAME, 0);
              oper->attr = L_concat_attr (oper->attr, attr);
            }
          if (branch_class[oper->id] & L_BR_NONLOOP_OUTER)
            {
              attr = L_new_attr (L_BR_NONLOOP_OUTER_NAME, 0);
              oper->attr = L_concat_attr (oper->attr, attr);
            }
          if (branch_class[oper->id] & L_BR_NONLOOP_STLN)
            {
              attr = L_new_attr (L_BR_NONLOOP_STLN_NAME, 0);
              oper->attr = L_concat_attr (oper->attr, attr);
            }
        }
    }

  if (buf != NULL)
    Lcode_free (buf);
  if (branch_class != NULL)
    Lcode_free (branch_class);
  if (loop_array != NULL)
    Lcode_free (loop_array);
}

void
L_classify_branches (L_Func * fn)
{
  L_do_flow_analysis (fn, DOMINATOR_CB);
  L_loop_detection (fn, 0);
  L_mark_branches (fn);
  L_remove_old_classifications (fn);
  L_do_classify_branches (fn);
}
