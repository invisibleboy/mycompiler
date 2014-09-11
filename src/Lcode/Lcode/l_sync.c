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
 *      File :          l_sync.c
 *      Description :   Procedures for handling sync arcs in Lcode
 *      Author: Dave Gallagher, Wen-mei Hwu
 *      Modifications:
 *          Dan Lavery, July 1995
 *          -Added L_analyze_syncs_for_cross_iter_independence and
 *           L_adjust_syncs_for_remainder
 *          Dan Lavery, March 1996
 *          - Added L_remove_invalid_sync_arcs_in_func and
 *            L_remove_invalid_sync_arcs_in_cb
 *          Dan Connors, Sept 1997
 *          - Added L_remove_invalid_sync_arcs_in_func and
 *            L_remove_invalid_sync_arcs_in_cb
\*****************************************************************************/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>

#define TRUE 1
#define FALSE 0

#undef DEBUG_SYNC_UNION

/*=======================================================================*\
 *
 *      File name parameters read in from STD_PARMS     
 *
\*=======================================================================*/

L_Oper_List **Child_List;
int Child_List_Size;


void
L_create_child_list (L_Func * fn)
{
  int i;

  Child_List_Size = fn->max_oper_id + 1;

  Child_List = (L_Oper_List **) malloc (sizeof (L_Oper_List *) *
                                        Child_List_Size);

  for (i = 0; i < Child_List_Size; i++)
    Child_List[i] = NULL;
}


void
L_delete_child_list ()
{
  int i;

  for (i = 0; i < Child_List_Size; i++)
    {
      L_delete_all_oper_list (Child_List[i]);
      Child_List[i] = NULL;
    }

  free (Child_List);
}


void
L_init_child_list (L_Func * fn)
{
  /* Check if function is using sync arcs */
  if (L_func_contains_dep_pragmas || L_func_acc_omega)
    L_create_child_list (fn);
}


void
L_deinit_child_list (L_Func * fn)
{
  /* Check if function is using sync arcs */
  if (L_func_contains_dep_pragmas || L_func_acc_omega)
    L_delete_child_list ();
}


void
L_add_to_child_list (L_Oper * parent_oper, L_Oper * child_oper)
{
  L_Oper_List *new_oper_list;

  new_oper_list = L_new_oper_list ();
  new_oper_list->oper = child_oper;

  new_oper_list->next_list = Child_List[parent_oper->id];
  Child_List[parent_oper->id] = new_oper_list;
}


void
L_relink_child_sync_arcs (L_Func * fn)
{
  int i;
  L_Cb *cb;
  L_Oper *oper, *dep_parent;
  L_Oper_List *dep_child_list, *list;
  L_Sync_Info *sync_info;
  L_Sync *new_sync;

  /* All sync arcs in the children are pointing to the dependent
     parent, not the dependent child.  We use the Child_List
     array, indexed by parent id, to find all children of a parent */

  if (!(L_func_contains_dep_pragmas || L_func_acc_omega))
    return;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
        {

          sync_info = oper->sync_info;

          if (sync_info == NULL)
            continue;

          for (i = 0; i < sync_info->num_sync_in; i++)
            {
              dep_parent = sync_info->sync_in[i]->dep_oper;

              if (!L_EXTRACT_BIT_VAL (dep_parent->flags, L_OPER_PARENT))
                L_punt ("L_relink_child_sync_arcs: dep oper not a parent");

              dep_child_list = Child_List[dep_parent->id];

              if (dep_child_list == NULL)
                L_punt ("L_relink_child_sync_arcs: dep child not found");

              /* the child oper may have been deleted */
              if (dep_child_list->oper == NULL)
                {
                  L_delete_tail_sync (oper, sync_info->sync_in[i]);
                  continue;
                }

              sync_info->sync_in[i]->dep_oper = dep_child_list->oper;

              for (list = dep_child_list->next_list; list != NULL;
                   list = list->next_list)
                {
                  new_sync = L_copy_sync (sync_info->sync_in[i]);
                  new_sync->dep_oper = list->oper;
                  L_insert_tail_sync_in_oper (oper, new_sync);
                }
            }

          for (i = 0; i < sync_info->num_sync_out; i++)
            {
              dep_parent = sync_info->sync_out[i]->dep_oper;

              if (!L_EXTRACT_BIT_VAL (dep_parent->flags, L_OPER_PARENT))
                L_punt ("L_relink_child_sync_arcs: dep oper not a parent");

              dep_child_list = Child_List[dep_parent->id];

              if (dep_child_list == NULL)
                L_punt ("L_relink_child_sync_arcs: dep child not found");

              /* the child oper may have been deleted */
              if (dep_child_list->oper == NULL)
                {
                  L_delete_head_sync (oper, sync_info->sync_out[i]);
                  continue;
                }

              sync_info->sync_out[i]->dep_oper = dep_child_list->oper;

              for (list = dep_child_list->next_list; list != NULL;
                   list = list->next_list)
                {
                  new_sync = L_copy_sync (sync_info->sync_out[i]);
                  new_sync->dep_oper = list->oper;
                  L_insert_head_sync_in_oper (oper, new_sync);
                }
            }
        }
    }
}


int
L_analyze_redundant_sync_arcs (L_Oper * op1, L_Oper * op2, int dep_flags)
{
  L_Sync_Info *sync_info1, *sync_info2;
  L_Sync *sync;
  int i;

  if (dep_flags == 0)
    L_warn ("L_analyze_redundant_sync_arcs: "
            "dep_flags is 0, result is always independence");

  sync_info1 = op1->sync_info;
  sync_info2 = op2->sync_info;

  if ((sync_info1 == NULL) || (sync_info2 == NULL))
    return (1);

  for (i = 0; i < sync_info1->num_sync_out; i++)
    {
      sync = sync_info1->sync_out[i];

      if (sync->dep_oper == op2)
        if (sync->info & dep_flags)
          return (0);
    }

  return (1);
}


int
L_difference_in_nesting_level (L_Oper * oper, int loop_nesting_level)
{
  L_Cb *cb;
  int diff, cb_nest;

  cb = L_oper_hash_tbl_find_cb (L_fn->oper_hash_tbl, oper->id);

  cb_nest = cb->deepest_loop->nesting_level;

  diff = cb_nest - loop_nesting_level;

  if (diff < 0)
    L_punt ("L_difference_in_nesting_level: oper less nested than loop");

  return (diff);
}


static int
L_from_same_loop_nest (L_Cb * orig_cb, L_Cb * new_cb)
{
  return (orig_cb->deepest_loop == new_cb->deepest_loop);
}


static void
L_adjust_sync_arcs_out_one_level (L_Oper * oper, L_Cb * new_cb,
                                  L_Cb * orig_cb)
{
  L_Sync_Info *sync_info;
  L_Sync *dep_sync;
  L_Cb *dep_cb;
  L_Oper *dep_oper;
  int i, info;

  /* we must adjust both incoming and outgoing dependences from oper.
     If arc to to/from orig_cb, the arc must be adjusted to reflect
     that new_cb is one iteration earlier.  If arc is between two
     opers in new_cb, inner loop dependences are eliminated, because
     we are essentially peeling first iter. If the arc is to another 
     cb other than orig_cb or new_cb, leave it unchanged */

  sync_info = oper->sync_info;

  for (i = 0; i < sync_info->num_sync_in; i++)
    {
      dep_oper = sync_info->sync_in[i]->dep_oper;
      dep_cb = L_oper_hash_tbl_find_cb (L_fn->oper_hash_tbl, dep_oper->id);

      info = sync_info->sync_in[i]->info;

      if (dep_cb == orig_cb)
        {

          if (IS_INNER_SERLOOP (info) || IS_OUTER_CARRIED (info))
            ;

          else if (IS_NONLOOP_CARRIED (info) || (IS_INNER_CARRIED (info)))
            {
              L_find_and_delete_head_sync (dep_oper, oper);
              L_delete_tail_sync (oper, sync_info->sync_in[i]);
              i--; /* to compensate because sync_in[] has been shifted */
            }
        }

      else if (dep_cb == new_cb)
        {

          /* the new dependence is within a new loop nest, so we
             don't know alot about it.  If it was non-loop carried,
             it still will be.  If it was inner or outer carried, we
             have to assume it is now inner carried with unknown_dist */

          if (IS_INNER_SERLOOP (info) || IS_OUTER_CARRIED (info))
            {
              sync_info->sync_in[i]->info |= SET_DISTANCE_UNKNOWN (0) |
                SET_INNER_CARRIED (0);
              sync_info->sync_in[i]->dist = 1;
              dep_sync = L_find_head_sync (dep_oper, oper);
              dep_sync->info |= SET_DISTANCE_UNKNOWN (0) |
                SET_INNER_CARRIED (0);
              dep_sync->dist = 1;
            }
        }
    }

  for (i = 0; i < sync_info->num_sync_out; i++)
    {

      dep_oper = sync_info->sync_out[i]->dep_oper;
      dep_cb = L_oper_hash_tbl_find_cb (L_fn->oper_hash_tbl, dep_oper->id);

      info = sync_info->sync_out[i]->info;

      if (dep_cb == orig_cb)
        {

          if (IS_INNER_SERLOOP (info) || IS_OUTER_CARRIED (info))
            ;

          else if (IS_NONLOOP_CARRIED (info) && (!IS_INNER_CARRIED (info)))
            {
              L_find_and_delete_tail_sync (dep_oper, oper);
              L_delete_head_sync (oper, sync_info->sync_out[i]);
              i--; /* to compensate because sync_in[] has been shifted */
            }
          else if (IS_INNER_CARRIED (info) && (!IS_NONLOOP_CARRIED (info)))
            {
              if ((!IS_DISTANCE_UNKNOWN (info)) &&
                  (sync_info->sync_out[i]->dist == 1))
                {
                  sync_info->sync_out[i]->info |= SET_NONLOOP_CARRIED (0);
                  sync_info->sync_out[i]->info &= ~SET_INNER_CARRIED (0);
                  sync_info->sync_out[i]->dist = 0;
                  dep_sync = L_find_tail_sync (dep_oper, oper);
                  dep_sync->info |= SET_NONLOOP_CARRIED (0);
                  dep_sync->info &= ~SET_INNER_CARRIED (0);
                  dep_sync->dist = 0;
                }
              else
                {
                  sync_info->sync_out[i]->info |= SET_DISTANCE_UNKNOWN (0);
                  sync_info->sync_out[i]->dist = 1;
                  dep_sync = L_find_tail_sync (dep_oper, oper);
                  dep_sync->info |= SET_DISTANCE_UNKNOWN (0);
                  dep_sync->dist = 1;
                }
            }
        }
    }
}


static int
L_was_previously_existing_oper (L_Cb * cb, L_Oper * dep_oper,
                                L_Oper * first_new_oper)
{
  L_Oper *oper;

  for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
    {
      if (oper == dep_oper || oper == first_new_oper)
        break;
    }

  if ((oper == dep_oper) && (oper != first_new_oper))
    return (1);

  if (oper == NULL)
    L_punt ("L_was_previously_existing_oper: first_new_oper not found");

  return (0);
}


static void
L_adjust_sync_arcs_one_iteration (L_Oper * oper, L_Oper * first_new_oper,
                                  L_Cb * new_cb, L_Cb * orig_cb)
{
  L_Sync_Info *sync_info;
  L_Sync *dep_sync;
  L_Cb *dep_cb;
  L_Oper *dep_oper;
  int i, info, dist;

  /* we must adjust both incoming and outgoing dependences from oper.
     If arc to to/from orig_cb, the arc must be adjusted to reflect
     that new_cb is one iteration later.  If arc is between two
     opers in new_cb, we must be conservative on distances for
     loop-carried dependences, because the orig_cb may get unrolled,
     and it may be an unknown number of iterations (>= 2) before
     control returns.  If the arc is to another cb other than orig_cb
     or new_cb, leave it unchanged */

  sync_info = oper->sync_info;

  for (i = 0; i < sync_info->num_sync_in; i++)
    {

      dep_oper = sync_info->sync_in[i]->dep_oper;
      dep_cb = L_oper_hash_tbl_find_cb (L_fn->oper_hash_tbl, dep_oper->id);

      info = sync_info->sync_in[i]->info;

      if (dep_cb == orig_cb)
        {

          if (IS_INNER_SERLOOP (info))
            {
              sync_info->sync_in[i]->info |= SET_NONLOOP_CARRIED (0) |
                SET_INNER_CARRIED (0) | SET_DISTANCE_UNKNOWN (0);
              dep_sync = L_find_head_sync (dep_oper, oper);
              dep_sync->info |= SET_NONLOOP_CARRIED (0) |
                SET_INNER_CARRIED (0) | SET_DISTANCE_UNKNOWN (0);
            }

          else if (IS_NONLOOP_CARRIED (info) && !IS_DISTANCE_UNKNOWN (info))
            {
              L_find_and_delete_head_sync (dep_oper, oper);
              L_delete_tail_sync (oper, sync_info->sync_in[i]);
              i--; /* to compensate because sync_in[] has been shifted */
            }

          else if (IS_INNER_CARRIED (info))
            {
              if (IS_DISTANCE_UNKNOWN (info))
                {
                  sync_info->sync_in[i]->info |= SET_NONLOOP_CARRIED (0) |
                    SET_INNER_CARRIED (0);
                  sync_info->sync_in[i]->dist = 1;
                  dep_sync = L_find_head_sync (dep_oper, oper);
                  dep_sync->info |= SET_NONLOOP_CARRIED (0) |
                    SET_INNER_CARRIED (0);
                  dep_sync->dist = 1;
                }
              else
                {
                  dist = sync_info->sync_in[i]->dist;
                  if (dist == 0)
                    {
                      L_punt ("L_adjust_sync_arcs_one_iteration: "
                              "inner carried distance=0");
                    }
                  else if (dist == 1)
                    {
                      sync_info->sync_in[i]->info |= SET_NONLOOP_CARRIED (0);
                      sync_info->sync_in[i]->info &= ~SET_INNER_CARRIED (0);
                      sync_info->sync_in[i]->dist = 0;
                      dep_sync = L_find_head_sync (dep_oper, oper);
                      dep_sync->info |= SET_NONLOOP_CARRIED (0);
                      dep_sync->info &= ~SET_INNER_CARRIED (0);
                      dep_sync->dist = 0;
                    }
                  else
                    {
                      sync_info->sync_in[i]->info |= SET_DISTANCE_UNKNOWN (0);
                      sync_info->sync_in[i]->dist = 1;
                      dep_sync = L_find_head_sync (dep_oper, oper);
                      dep_sync->info |= SET_DISTANCE_UNKNOWN (0);
                      dep_sync->dist = 1;
                    }
                }
            }
        }
      /* we only adjust intra_newcb arcs by looking at incoming arcs,
         to avoid duplication of work (unless arc goes from
         a new oper in newcb to an originally existing oper) */

      else if (dep_cb == new_cb)
        {

          if (L_was_previously_existing_oper
              (new_cb, dep_oper, first_new_oper))
            {

              if (IS_INNER_SERLOOP (info))
                {
                  sync_info->sync_in[i]->info |= SET_NONLOOP_CARRIED (0) |
                    SET_INNER_CARRIED (0) | SET_DISTANCE_UNKNOWN (0);
                  dep_sync = L_find_head_sync (dep_oper, oper);
                  dep_sync->info |= SET_NONLOOP_CARRIED (0) |
                    SET_INNER_CARRIED (0) | SET_DISTANCE_UNKNOWN (0);
                }

              else if (IS_NONLOOP_CARRIED (info) &&
                       !IS_DISTANCE_UNKNOWN (info))
                {
                  L_find_and_delete_head_sync (dep_oper, oper);
                  L_delete_tail_sync (oper, sync_info->sync_in[i]);
                  i--; /* to compensate because sync_in[] has been shifted */
                }

              else if (IS_INNER_CARRIED (info))
                {
                  if (IS_DISTANCE_UNKNOWN (info))
                    {
                      sync_info->sync_in[i]->info |= SET_NONLOOP_CARRIED (0) |
                        SET_INNER_CARRIED (0);
                      sync_info->sync_in[i]->dist = 1;
                      dep_sync = L_find_head_sync (dep_oper, oper);
                      dep_sync->info |= SET_NONLOOP_CARRIED (0) |
                        SET_INNER_CARRIED (0);
                      dep_sync->dist = 1;
                    }
                  else
                    {
                      dist = sync_info->sync_in[i]->dist;
                      if (dist == 0)
                        {
                          L_punt ("L_adjust_sync_arcs_one_iteration: "
                                  "inner carried distance=0");
                        }
                      else if (dist == 1)
                        {
                          sync_info->sync_in[i]->info |=
                            SET_NONLOOP_CARRIED (0);
                          sync_info->sync_in[i]->info &=
                            ~SET_INNER_CARRIED (0);
                          sync_info->sync_in[i]->dist = 0;
                          dep_sync = L_find_head_sync (dep_oper, oper);
                          dep_sync->info |= SET_NONLOOP_CARRIED (0);
                          dep_sync->info &= ~SET_INNER_CARRIED (0);
                          dep_sync->dist = 0;
                        }
                      else
                        {
                          sync_info->sync_in[i]->info =
                            SET_DISTANCE_UNKNOWN (0);
                          sync_info->sync_in[i]->dist = 1;
                          dep_sync = L_find_head_sync (dep_oper, oper);
                          dep_sync->info |= SET_DISTANCE_UNKNOWN (0);
                          dep_sync->dist = 1;
                        }
                    }
                }
            }
          else
            {
              if (IS_INNER_SERLOOP (info) || IS_NONLOOP_CARRIED (info) ||
                  IS_DISTANCE_UNKNOWN (info))
                {
                  ;
                }

              else if (IS_INNER_CARRIED (info))
                {
                  dist = sync_info->sync_in[i]->dist;
                  if (dist == 0)
                    {
                      L_punt ("L_adjust_sync_arcs_one_iteration: "
                              "inner carried distance=0");
                    }
                  else if (dist == 1)
                    {
                      L_find_and_delete_head_sync (dep_oper, oper);
                      L_delete_tail_sync (oper, sync_info->sync_in[i]);
                      i--;
                    }
                  else
                    {
                      sync_info->sync_in[i]->info |= SET_DISTANCE_UNKNOWN (0);
                      sync_info->sync_in[i]->dist = 1;
                      dep_sync = L_find_head_sync (dep_oper, oper);
                      dep_sync->info |= SET_DISTANCE_UNKNOWN (0);
                      dep_sync->dist = 1;
                    }
                }
            }
        }
    }

  for (i = 0; i < sync_info->num_sync_out; i++)
    {

      dep_oper = sync_info->sync_out[i]->dep_oper;
      dep_cb = L_oper_hash_tbl_find_cb (L_fn->oper_hash_tbl, dep_oper->id);

      info = sync_info->sync_out[i]->info;

      if (dep_cb == orig_cb)
        {

          if (IS_INNER_SERLOOP (info))
            {
              sync_info->sync_out[i]->info |= SET_NONLOOP_CARRIED (0) |
                SET_INNER_CARRIED (0) | SET_DISTANCE_UNKNOWN (0);
              dep_sync = L_find_tail_sync (dep_oper, oper);
              dep_sync->info |= SET_NONLOOP_CARRIED (0) |
                SET_INNER_CARRIED (0) | SET_DISTANCE_UNKNOWN (0);
            }

          else if (IS_NONLOOP_CARRIED (info))
            {
              L_find_and_delete_tail_sync (dep_oper, oper);
              L_delete_head_sync (oper, sync_info->sync_out[i]);
              i--; /* to compensate because sync_out[] has been shifted */
            }

          else if (IS_INNER_CARRIED (info))
            {
              if (IS_DISTANCE_UNKNOWN (info))
                {
                  sync_info->sync_out[i]->info |= SET_NONLOOP_CARRIED (0) |
                    SET_INNER_CARRIED (0);
                  sync_info->sync_out[i]->dist = 1;
                  dep_sync = L_find_tail_sync (dep_oper, oper);
                  dep_sync->info |=
                    SET_NONLOOP_CARRIED (0) | SET_INNER_CARRIED (0);
                  dep_sync->dist = 1;
                }
              else
                {
                  dist = sync_info->sync_out[i]->dist;
                  if (dist == 0)
                    {
                      L_punt ("L_adjust_sync_arcs_one_iteration: "
                              "inner carried distance=0");
                    }
                  else if (dist == 1)
                    {
                      sync_info->sync_out[i]->info |= SET_NONLOOP_CARRIED (0);
                      sync_info->sync_out[i]->info &= ~SET_INNER_CARRIED (0);
                      sync_info->sync_out[i]->dist = 0;
                      dep_sync = L_find_tail_sync (dep_oper, oper);
                      dep_sync->info |= SET_NONLOOP_CARRIED (0);
                      dep_sync->info &= ~SET_INNER_CARRIED (0);
                      dep_sync->dist = 0;
                    }
                  else
                    {
                      sync_info->sync_out[i]->info |=
                        SET_DISTANCE_UNKNOWN (0);
                      sync_info->sync_out[i]->dist = 1;
                      dep_sync = L_find_tail_sync (dep_oper, oper);
                      dep_sync->info |= SET_DISTANCE_UNKNOWN (0);
                      dep_sync->dist = 1;
                    }
                }
            }
        }
      /* if the outgoing arc goes to another oper in the new_cb, which
         was/will be adjusted on its incoming arc, we don't want to 
         adjust here.  However, if this arc is from a new_oper to
         an oper which was here prior to br tgt exp, we need to
         adjust the arc */
      else if (dep_cb == new_cb)
        {
          if (L_was_previously_existing_oper
              (new_cb, dep_oper, first_new_oper))
            {

              if (IS_INNER_SERLOOP (info))
                {
                  sync_info->sync_out[i]->info |= SET_NONLOOP_CARRIED (0) |
                    SET_INNER_CARRIED (0) | SET_DISTANCE_UNKNOWN (0);
                  dep_sync = L_find_tail_sync (dep_oper, oper);
                  dep_sync->info |= SET_NONLOOP_CARRIED (0) |
                    SET_INNER_CARRIED (0) | SET_DISTANCE_UNKNOWN (0);
                }

              else if (IS_NONLOOP_CARRIED (info))
                {
                  L_find_and_delete_tail_sync (dep_oper, oper);
                  L_delete_head_sync (oper, sync_info->sync_out[i]);
                  i--; /* to compensate because sync_out[] has been shifted */
                }

              else if (IS_INNER_CARRIED (info))
                {
                  if (IS_DISTANCE_UNKNOWN (info))
                    {
                      sync_info->sync_out[i]->info |=
                        SET_NONLOOP_CARRIED (0) | SET_INNER_CARRIED (0);
                      sync_info->sync_out[i]->dist = 1;
                      dep_sync = L_find_tail_sync (dep_oper, oper);
                      dep_sync->info |= SET_NONLOOP_CARRIED (0) |
                        SET_INNER_CARRIED (0);
                      dep_sync->dist = 1;
                    }
                  else
                    {
                      dist = sync_info->sync_out[i]->dist;
                      if (dist == 0)
                        {
                          L_punt ("L_adjust_sync_arcs_one_iteration: "
                                  "inner carried distance=0");
                        }
                      else if (dist == 1)
                        {
                          sync_info->sync_out[i]->info |=
                            SET_NONLOOP_CARRIED (0);
                          sync_info->sync_out[i]->info &=
                            ~SET_INNER_CARRIED (0);
                          sync_info->sync_out[i]->dist = 0;
                          dep_sync = L_find_tail_sync (dep_oper, oper);
                          dep_sync->info |= SET_NONLOOP_CARRIED (0);
                          dep_sync->info &= ~SET_INNER_CARRIED (0);
                          dep_sync->dist = 0;
                        }
                      else
                        {
                          sync_info->sync_out[i]->info =
                            SET_DISTANCE_UNKNOWN (0);
                          sync_info->sync_out[i]->dist = 1;
                          dep_sync = L_find_tail_sync (dep_oper, oper);
                          dep_sync->info |= SET_DISTANCE_UNKNOWN (0);
                          dep_sync->dist = 1;
                        }
                    }
                }
            }
        }
    }
}


void
L_adjust_syncs_for_target_expansion (L_Oper * first_oper, L_Cb * orig_cb,
                                     L_Cb * new_cb)
{
  L_Oper *oper;
  int is_loop_header, same_loop_nest;

  /* here we want to adjust the sync dist/flags for a newly copied
     oper, which may have changed nesting level.  If the nesting level
     changed, then the dependence relationship to other opers may have
     also changed */

  /* first determine whether the original cb was a loop header and 
     whether the 2 cbs were in the same loop nest */

  if ((orig_cb->deepest_loop == NULL) ||
      (orig_cb->deepest_loop->header != orig_cb))
    is_loop_header = FALSE;
  else
    is_loop_header = TRUE;

  same_loop_nest = L_from_same_loop_nest (orig_cb, new_cb);

  for (oper = first_oper; oper != NULL; oper = oper->next_op)
    {

      /* CASE 1:  the new_cb is in a different loop nest, and we are copying
         from a loop header cb.  We have essentially peeled the first
         iteration of the loop (we assume we are not copying into a 
         deeper loop nest).  Thus, any sync arcs which were carried by the
         inner loop level must be adjusted for the new cb. */

      if (oper->sync_info == NULL)
        continue;
      if ((!same_loop_nest) && (is_loop_header))
        L_adjust_sync_arcs_out_one_level (oper, new_cb, orig_cb);

      /* CASE 2: the new cb is in the same loop nest, and we are copying
         from a loop header cb.  The code being duplicated will essentially
         be one iteration after the code from the original cb, so this is
         similar to loop unrolling. */

      else if ((same_loop_nest) && (is_loop_header))
        L_adjust_sync_arcs_one_iteration (oper, first_oper, new_cb, orig_cb);

      /* CASE 3: the new cb is in the same loop nest, and we are not copying
         from a loop header cb.  Sync arcs do not need adjustment. */

      else if ((same_loop_nest) && (!is_loop_header))
        return;

      /* CASE 4: the new cb is not in the same loop nest, and we are not 
         copying from a loop header cb.  We currently don't anticipate this
         as a legal code motion and punt */

      else if ((!same_loop_nest) && (!is_loop_header))
        return;
      /*
         L_punt ("L_adjust_syncs_for_target_expansion: "
         "unexpected duplication");
       */
    }
}


    /* find level of common loop.  If one is not nested in other, then just
       return the nesting level of the lowest nested, minus 1 */
static int
L_find_deepest_common_loop_level (L_Cb * cb1, L_Cb * cb2)
{
  int nest_level1, nest_level2;

  if (cb1->deepest_loop != NULL)
    nest_level1 = cb1->deepest_loop->nesting_level;
  else
    return (0);

  if (cb2->deepest_loop != NULL)
    nest_level2 = cb2->deepest_loop->nesting_level;
  else
    return (0);

  if (nest_level1 >= nest_level2)
    {

      if (Set_in (cb2->deepest_loop->loop_cb, cb1->id))
        return (nest_level2);
      else
        return (nest_level2 - 1);
    }
  else
    {

      if (Set_in (cb1->deepest_loop->loop_cb, cb2->id))
        return (nest_level1);
      else
        return (nest_level1 - 1);
    }
}


void
L_adjust_syncs_for_movement_out_of_loop (L_Oper * oper, L_Cb * new_cb)
{
  L_Cb *dep_cb;
  L_Oper *dep_oper;
  L_Sync_Info *sync_info;
  int i, nest_level, dep_nest_level, deepest_common_loop_level, info;

  /* oper has been moved out to an outer loop nest; if the dep_oper
     is at a nesting level deeper than oper, the dependence should
     only be outer loop carried (warn if not and make conservative).
     For arcs going to dep_opers at a nesting level <= new_cb, no
     change to arc. */

  sync_info = oper->sync_info;

  if (sync_info == NULL)
    return;

  if (new_cb->deepest_loop != NULL)
    nest_level = new_cb->deepest_loop->nesting_level;
  else
    nest_level = 0;

  for (i = 0; i < sync_info->num_sync_in; i++)
    {
      dep_oper = sync_info->sync_in[i]->dep_oper;
      dep_cb = L_oper_hash_tbl_find_cb (L_fn->oper_hash_tbl, dep_oper->id);
      if (dep_cb->deepest_loop != NULL)
        dep_nest_level = dep_cb->deepest_loop->nesting_level;
      else
        dep_nest_level = 0;

      deepest_common_loop_level =
        L_find_deepest_common_loop_level (new_cb, dep_cb);

      /* if nest level of dep_oper is <= nest of oper, then no change */
      if (((deepest_common_loop_level < nest_level) ||
           (nest_level >= dep_nest_level)) &&
          (nest_level != 0) && (dep_nest_level != 0))
        continue;

      info = sync_info->sync_in[i]->info;
      /*
         if ( !IS_OUTER_CARRIED(info) && IS_INNER_CARRIED(info) )
         L_warn ("L_adjust_syncs_for_movement_out_of_loop: "
         "not outer carried");
       */

      if (IS_OUTER_CARRIED (info) || IS_INNER_CARRIED (info))
        {
          info |= SET_OUTER_CARRIED (0) | SET_INNER_CARRIED (0) |
            SET_NONLOOP_CARRIED (0) | SET_DISTANCE_UNKNOWN (0);
          sync_info->sync_in[i]->info = info;
          sync_info->sync_in[i]->dist = 1;
        }
    }

  for (i = 0; i < sync_info->num_sync_out; i++)
    {
      dep_oper = sync_info->sync_out[i]->dep_oper;
      dep_cb = L_oper_hash_tbl_find_cb (L_fn->oper_hash_tbl, dep_oper->id);
      if (dep_cb->deepest_loop != NULL)
        dep_nest_level = dep_cb->deepest_loop->nesting_level;
      else
        dep_nest_level = 0;

      deepest_common_loop_level =
        L_find_deepest_common_loop_level (new_cb, dep_cb);

      /* if nest level of dep_oper is <= nest of oper, then no change */
      if ((deepest_common_loop_level < nest_level) ||
          (nest_level >= dep_nest_level))
        continue;

      info = sync_info->sync_out[i]->info;
      /*
         if ( !IS_OUTER_CARRIED(info) && IS_INNER_CARRIED(info) )
         L_warn ("L_adjust_syncs_for_movement_out_of_loop: "
         "not outer carried");
       */

      if (IS_OUTER_CARRIED (info) || IS_INNER_CARRIED (info))
        {
          info |= SET_OUTER_CARRIED (0) | SET_INNER_CARRIED (0) |
            SET_NONLOOP_CARRIED (0) | SET_DISTANCE_UNKNOWN (0);
          sync_info->sync_out[i]->info = info;
          sync_info->sync_out[i]->dist = 1;
        }
    }

}


void
L_make_sync_arcs_conservative (L_Oper * oper)
{
  L_Sync_Info *sync_info;
  L_Sync *dep_sync;
  L_Oper *dep_oper;
  int i;

  /* We are giving up on the sync arcs for this oper, and making them
     all conservative.  We convert both sync_in and sync_out fields
     to outer, inner, nonloop, dist=1 */

  sync_info = oper->sync_info;

  for (i = 0; i < sync_info->num_sync_in; i++)
    {

      sync_info->sync_in[i]->info |=
        SET_OUTER_CARRIED (0) | SET_NONLOOP_CARRIED (0) |
        SET_INNER_CARRIED (0) | SET_DISTANCE_UNKNOWN (0);
      sync_info->sync_in[i]->dist = 1;

      dep_oper = sync_info->sync_in[i]->dep_oper;
      dep_sync = L_find_head_sync (dep_oper, oper);
      dep_sync->info |= SET_OUTER_CARRIED (0) | SET_NONLOOP_CARRIED (0) |
        SET_INNER_CARRIED (0) | SET_DISTANCE_UNKNOWN (0);
      dep_sync->dist = 1;
    }

  for (i = 0; i < sync_info->num_sync_out; i++)
    {

      sync_info->sync_out[i]->info |=
        SET_OUTER_CARRIED (0) | SET_NONLOOP_CARRIED (0) |
        SET_INNER_CARRIED (0) | SET_DISTANCE_UNKNOWN (0);
      sync_info->sync_out[i]->dist = 1;

      dep_oper = sync_info->sync_out[i]->dep_oper;
      dep_sync = L_find_tail_sync (dep_oper, oper);
      dep_sync->info |= SET_OUTER_CARRIED (0) | SET_NONLOOP_CARRIED (0) |
        SET_INNER_CARRIED (0) | SET_DISTANCE_UNKNOWN (0);
      dep_sync->dist = 1;
    }
}


void
L_update_sync_arcs_for_new_cb (L_Cb * old_cb, L_Cb * new_cb, L_Oper * oper)
{
  /* if the old and new cbs are from the same loopnest, the sync
     arcs don't need to be updated.  However, if the new_cb is in
     a different loop nest, we make the arcs conservative, i.e.
     outer, inner, and nonloop carried */

  if (old_cb->deepest_loop != new_cb->deepest_loop)
    L_make_sync_arcs_conservative (oper);
}


void
L_add_specific_sync_between_opers (L_Oper * oper1, L_Oper * oper2,
				   short info, char dist, char prof_info)
{
  L_Sync *sync, *sync1, *sync2;
  L_Sync_Info *sync_info;
  int i;

  sync_info = oper1->sync_info;

  if (sync_info != NULL)
    {
      for (i = 0; i < sync_info->num_sync_out; i++)
	{
	  sync = sync_info->sync_out[i];
	  
	  if ((sync->dep_oper == oper2) &&
	      (sync->info == info) &&
	      (sync->dist == dist) &&
	      (sync->prof_info == prof_info))
	    return;
	}
    }

  sync1 = L_new_sync (oper2);
  sync1->dist = dist;
  sync1->info = info;
  sync1->prof_info = prof_info;

  sync2 = L_new_sync (oper1);
  sync2->dist = dist;
  sync2->info = info;
  sync2->prof_info = prof_info;

  L_insert_head_sync_in_oper (oper1, sync1);
  L_insert_tail_sync_in_oper (oper2, sync2);
  return;
}


void
L_add_sync_between_opers (L_Oper * oper1, L_Oper * oper2)
{
  L_Sync *sync, *sync1, *sync2;
  L_Sync_Info *sync_info;
  int i;

  sync_info = oper1->sync_info;

  if (sync_info != NULL)
    {
      for (i = 0; i < sync_info->num_sync_out; i++)
        {
          sync = sync_info->sync_out[i];

          if (sync->dep_oper == oper2)
            return;
        }
    }

  sync1 = L_new_sync (oper2);
  sync1->dist = 0;

  /* BCC - 2/2/99 
   * For now all arcs are the same
   */
  sync1->info = (short) (SET_NONLOOP_CARRIED (0) | SET_INNER_CARRIED (0) |
                         SET_OUTER_CARRIED (0) | SET_DISTANCE_UNKNOWN (0) |
                         sometimes_sync);
  sync1->prof_info = 0;

  sync2 = L_new_sync (oper1);
  sync2->dist = 0;
  sync2->info = sync1->info;
  sync2->prof_info = 0;

  L_insert_head_sync_in_oper (oper1, sync1);
  L_insert_tail_sync_in_oper (oper2, sync2);
}


void
L_add_maybe_sync_between_opers (L_Oper * oper1, L_Oper * oper2)
{
  L_Sync *sync1, *sync2;

  sync1 = L_new_sync (oper2);
  sync1->dist = 0;
  sync1->info = SET_NONLOOP_CARRIED (0) | SET_INNER_CARRIED (0) |
    SET_DISTANCE_UNKNOWN (0);
  sync1->info |= 0x4000;        /* this makes the arc MS rather than MA */
  sync1->prof_info = 0;

  sync2 = L_new_sync (oper1);
  sync2->dist = 0;
  sync2->info = sync1->info;
  sync2->prof_info = 0;

  L_insert_head_sync_in_oper (oper1, sync1);
  L_insert_tail_sync_in_oper (oper2, sync2);
}


void
L_add_maybe_backwards_sync_between_opers (L_Oper * oper1, L_Oper * oper2)
{
  int same_cb;
  L_Oper *first_op, *second_op;

  same_cb = L_find_first_seq_op (oper1, oper2, &first_op, &second_op);
  if (!same_cb)
    L_punt ("L_add_maybe_backwards_sync_between_opers: "
            "ops %d and %d not in same cb", oper1->id, oper2->id);

  L_add_maybe_sync_between_opers (second_op, first_op);
}


int
L_address_varies_between_iterations (L_Oper * oper)
{
  return (0);
}


void
L_add_sync_between_unrolled_opers (L_Oper * oper1, L_Oper * oper2)
{

  if (!L_address_varies_between_iterations (oper1))
    L_add_sync_between_opers (oper1, oper2);
}


int
L_find_unroll_iter (L_Oper * oper)
{
  L_Attr *attr;
  int iter;

  attr = L_find_attr (oper->attr, "iter");

  if (attr == NULL)
    L_punt ("L_find_unroll_iter: iteration attr not found");

  iter = (int) attr->field[0]->value.i;

  /* iterations are numbered 1-n;  we need 0 - (n-1) */
  return (iter - 1);
}


int
L_oper_is_in_cb (L_Cb * cb, L_Oper * oper)
{
  L_Cb *oper_cb;

  oper_cb = L_oper_hash_tbl_find_cb (L_fn->oper_hash_tbl, oper->id);

  return (oper_cb == cb);
}


void
L_adjust_syncs_after_unrolling (L_Cb * cb, int num_unroll)
{
  L_Oper *oper, *dep_oper;
  L_Sync_Info *sync_info;
  L_Sync *sync, *dep_sync;
  int head_iter, tail_iter, tgt_iter, distance, new_dist;
  int i;
  Set oper_set = 0; /* set of opers that have been looked at so far */

  /* During unrolling, we have blindly replicated sync arcs, ignoring
     dependence distance.  Many of the arcs we formed, as well as some
     of the original arcs, are no longer correct.  Here, we adjust the
     dependence distance, deleting unnecessary arcs.  We only do this
     for dependences which are innerloop or nonloop carried, with known
     distance. */

  for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
    {

      if (oper->sync_info == NULL)
        {
          oper_set = Set_add (oper_set, oper->id);
          continue;
        }

      sync_info = oper->sync_info;

      head_iter = L_find_unroll_iter (oper);

      /* since we will walk all opers, we can just look at the head syncs
         for each oper */

      for (i = 0; i < sync_info->num_sync_out; i++)
        {
          sync = sync_info->sync_out[i];

          dep_oper = sync->dep_oper;

          /* we do not adjust syncs going to outside the cb.  Since
             incoming syncs from outside the cb are not even looked at,
             they will also be left alone */
          if (!L_oper_is_in_cb (cb, dep_oper))
            continue;

          tail_iter = L_find_unroll_iter (dep_oper);

          if (IS_OUTER_CARRIED (sync->info))
            {
              if ((IS_NONLOOP_CARRIED (sync->info)) &&
                  (!IS_INNER_CARRIED (sync->info)) &&
                  (head_iter != tail_iter))
                {

                  dep_sync = L_find_tail_sync (dep_oper, oper);

                  /* DML - Do not need to set inner carried here.  If
                     dependence is non loop carried, it cannot become
                     inner loop carried by unrolling the loop. */
                  sync->info = SET_DISTANCE_UNKNOWN (sync->info) &
                    ~SET_NONLOOP_CARRIED (0);
                  sync->dist = 0;
                  dep_sync->info = SET_DISTANCE_UNKNOWN (dep_sync->info) &
                    ~SET_NONLOOP_CARRIED (0);
                  dep_sync->dist = 0;

                  continue;
                }
              else if ((!IS_NONLOOP_CARRIED (sync->info)) &&
                       (IS_INNER_CARRIED (sync->info)))
                {
                  dep_sync = L_find_tail_sync (dep_oper, oper);

                  /* DML - may be able to make this case less conservative
                     in the future */
                  if (!IS_DISTANCE_UNKNOWN (sync->info))
                    {
                      L_warn ("L_adjust_syncs_after_unrolling: "
                              "inner/outer carried dependence with "
                              "known inner distance.  Can use formula "
                              "instead of being conservative here.  "
                              "Cb = %d, oper = %d\n", cb->id, oper->id);
                    }

                  /* DML - making dependence outer/inner/non, need to
                     also set distance unknown */
                  sync->info = SET_NONLOOP_CARRIED (sync->info) |
                    SET_DISTANCE_UNKNOWN (0);
                  sync->dist = 0;
                  dep_sync->info = SET_NONLOOP_CARRIED (dep_sync->info) |
                    SET_DISTANCE_UNKNOWN (0);
                  dep_sync->dist = 0;

                  continue;
                }
              else if ((!IS_NONLOOP_CARRIED (sync->info)) &&
                       (!IS_INNER_CARRIED (sync->info)))
                {
                  continue;
                }
            }

          /* DML - If control reaches this point, the dependence is
             either an inner/outer/nonloop conservative dependence
             or it is not outer loop carried at all.  If it is
             inner carried with distance unknown, the forward arcs must
             be set to nonloop also after unrolling.  If the arc is
             backward (i.e. from a later oper to an earlier oper in
             the cb), an nonloop carried dependence is impossible and
             so nonloop is cleared. */
          /* scalars, and other loop invariant variables will be
             distance unknown, and no arcs should be deleted */

          if (IS_DISTANCE_UNKNOWN (sync->info))
            {
              if (IS_INNER_CARRIED (sync->info))
                {
                  dep_sync = L_find_tail_sync (dep_oper, oper);
                  if (Set_in (oper_set, dep_oper->id))
                    {
                      sync->info = sync->info & ~SET_NONLOOP_CARRIED (0);
                      sync->dist = 1;
                      dep_sync->info = dep_sync->info &
                        ~SET_NONLOOP_CARRIED (0);
                      dep_sync->dist = 1;
                    }
                  else
                    {
                      sync->info = SET_NONLOOP_CARRIED (sync->info);
                      sync->dist = 0;
                      dep_sync->info = SET_NONLOOP_CARRIED (dep_sync->info);
                      dep_sync->dist = 0;
                    }
                }
              continue;
            }

          /* Now we know the iter # for both ends of the arc, as well
             as the dep distance and num_unroll.  From this we can use
             simple mod arith to determine if the arc is valid */

          distance = sync->dist;

          tgt_iter = (head_iter + distance) % num_unroll;

          /* if the sync arc does not make sense, delete it */
          if (tgt_iter != tail_iter)
            {
              L_delete_head_sync (oper, sync);
              L_find_and_delete_tail_sync (dep_oper, oper);
              /* need to decrement i since deleted the sync */
              i--;
              continue;
            }

          /* the sync needs to remain, but we need to adjust distance */

          new_dist = (head_iter + distance) / num_unroll;

          sync->dist = new_dist;
          dep_sync = L_find_tail_sync (dep_oper, oper);
          dep_sync->dist = new_dist;

          if (new_dist == 0)
            {
              sync->info = SET_NONLOOP_CARRIED (sync->info) &
                ~SET_INNER_CARRIED (0);
              dep_sync->info = SET_NONLOOP_CARRIED (dep_sync->info) &
                ~SET_INNER_CARRIED (0);
            }
        }
      oper_set = Set_add (oper_set, oper->id);
    }
  oper_set = Set_dispose (oper_set);
}


/* adjust sync arcs after creation of remainder loop when unrolling
   with remainder loop */
void
L_adjust_syncs_for_remainder (L_Cb * header_cb, L_Cb * remainder_cb)
{
  L_Oper *oper, *dep_oper;
  L_Sync_Info *sync_info;
  L_Sync *sync, *dep_sync;
  L_Cb *dest_cb;
  int i;

  for (oper = remainder_cb->first_op; oper != NULL; oper = oper->next_op)
    {

      if (oper->sync_info == NULL)
        continue;

      sync_info = oper->sync_info;

      /* look at the head syncs for each oper */

      for (i = 0; i < sync_info->num_sync_out; i++)
        {
          sync = sync_info->sync_out[i];

          dep_oper = sync->dep_oper;

          dest_cb = L_oper_hash_tbl_find_cb (L_fn->oper_hash_tbl,
                                             dep_oper->id);

          /* Remove any arcs which go from the remainder cb to the
             header cb.  Such dependences are impossible. */
          if (dest_cb == header_cb)
            {
              L_delete_head_sync (oper, sync);
              L_find_and_delete_tail_sync (dep_oper, oper);
              /* need to decrement i since deleted the sync */
              i--;
            }
          /* Remove any arcs which go from the remainder cb to itself
             and are carried by the inner loop.  There is never more 
             than 1 iteration of the remainder loop, so these dependences 
             are impossible. */
          else if (dest_cb == remainder_cb)
            {
              if (IS_INNER_CARRIED (sync->info) &&
                  !IS_OUTER_CARRIED (sync->info) &&
                  !IS_NONLOOP_CARRIED (sync->info))
                {
                  L_delete_head_sync (oper, sync);
                  L_find_and_delete_tail_sync (dep_oper, oper);
                  /* need to decrement i since deleted the sync */
                  i--;
                }
            }
        }
    }

  for (oper = header_cb->first_op; oper != NULL; oper = oper->next_op)
    {

      if (oper->sync_info == NULL)
        continue;

      sync_info = oper->sync_info;

      /* look at the head syncs for each oper */

      for (i = 0; i < sync_info->num_sync_out; i++)
        {
          sync = sync_info->sync_out[i];

          dep_oper = sync->dep_oper;

          dest_cb = L_oper_hash_tbl_find_cb (L_fn->oper_hash_tbl,
                                             dep_oper->id);

          /* Remove any arcs which go from the header cb to the
             remainder cb and are non-loop carried.  The remainder
             cb represents a new iteration of the original loop, so
             such dependences are impossible.  If the arc is
             not intra-iteration, make it conservative. */
          if (dest_cb == remainder_cb)
            {
              if (IS_NONLOOP_CARRIED (sync->info) &&
                  !IS_INNER_CARRIED (sync->info) &&
                  !IS_OUTER_CARRIED (sync->info))
                {
                  L_delete_head_sync (oper, sync);
                  L_find_and_delete_tail_sync (dep_oper, oper);
                  /* need to decrement i since deleted the sync */
                  i--;
                }
              else
                {
                  sync->info |= SET_OUTER_CARRIED (0) |
                    SET_NONLOOP_CARRIED (0) |
                    SET_INNER_CARRIED (0) | SET_DISTANCE_UNKNOWN (0);
                  sync->dist = 0;

                  dep_sync = L_find_tail_sync (dep_oper, oper);
                  dep_sync->info |= SET_OUTER_CARRIED (0) |
                    SET_NONLOOP_CARRIED (0) |
                    SET_INNER_CARRIED (0) | SET_DISTANCE_UNKNOWN (0);
                  dep_sync->dist = 0;

                }
            }
        }
    }
}


/****************************************************************************
 *
 * routine: L_adjust_outgoing_sync_arc_for_backedge_movement()
 * purpose: Change the arcs on the given op.  The op was originally in the
 *          loop and was rotated up around the loop backedge to the bottom
 *          of the loop.
 * input:
 * output: 
 * returns:
 * modified: Bob McGowan - 5/97 - created
 * note:
 *-------------------------------------------------------------------------*/
void
L_adjust_outgoing_sync_arc_for_backedge_movement (L_Oper * op, L_Sync * sync)
{
  L_Sync *dep_incoming_sync;

  dep_incoming_sync = L_find_tail_sync (sync->dep_oper, op);

  if (IS_NONLOOP_CARRIED (sync->info))
    {
      if (IS_INNER_CARRIED (sync->info))
        {
          /* non, inner, ?outer (case 3 & 7) */
          sync->info &= SET_NONLOOP_CARRIED (0);
          sync->info = SET_DISTANCE_UNKNOWN (sync->info);
          sync->dist = 1;
          dep_incoming_sync->info = sync->info;
          dep_incoming_sync->dist = 1;
        }
      else
        {
          /* non, !inner, ?outer (case 1 & 5) */
          sync->info &= ~SET_NONLOOP_CARRIED (0);
          sync->info = SET_INNER_CARRIED (sync->info);
          sync->dist = 1;
          dep_incoming_sync->info = sync->info;
          dep_incoming_sync->dist = 1;
        }
    }
  else
    {
      /* not non loop carried */
      if (IS_INNER_CARRIED (sync->info))
        {
          /* !non, inner, ?outer (case 2 & 6) */
          (sync->dist)++;
          dep_incoming_sync->dist = sync->dist;
        }
    }
}


/****************************************************************************
 *
 * routine: L_adjust_incoming_sync_arc_for_backedge_movement()
 * purpose: Change the arcs on the given op.  The op was originally in the
 *          loop and was rotated up around the loop backedge to the bottom
 *          of the loop.
 * input:
 * output: 
 * returns:
 * modified: Bob McGowan - 5/97 - created
 * note:
 *-------------------------------------------------------------------------*/
void
L_adjust_incoming_sync_arc_for_backedge_movement (L_Oper * op, L_Sync * sync)
{
  L_Sync *dep_outgoing_sync;

  if (!IS_NONLOOP_CARRIED (sync->info) && IS_INNER_CARRIED (sync->info))
    {
      /* !non, inner, ?outer (case 2 & 6) */
      dep_outgoing_sync = L_find_head_sync (sync->dep_oper, op);
      (sync->dist)--;
      dep_outgoing_sync->dist = sync->dist;
      if (sync->dist == 0)
        {
          sync->info = SET_NONLOOP_CARRIED (sync->info);
          dep_outgoing_sync->info = sync->info;
        }
    }
}


/* DML - Remove/adjust sync arcs which are impossible in cb.  Also remove
   arcs between 2 loads.  Pcode does not currently generate sync arcs for 
   input dependences, but some arcs get generated at HtoL between loads
   because of the handling of += operators. */
void
L_adjust_invalid_sync_arcs_in_cb (L_Cb * cb)
{
  L_Oper *oper, *dep_oper;
  L_Sync_Info *sync_info;
  L_Sync *sync, *dep_sync;
  int i;
  Set oper_set = 0; /* set of opers that have been looked at so far */

  if (cb == NULL)
    L_punt ("L_remove_invalid_sync_arcs_in_cb: cb cannot be NULL\n");

  for (oper = cb->last_op; oper != NULL; oper = oper->prev_op)
    {

      oper_set = Set_add (oper_set, oper->id);

      if (oper->sync_info == NULL)
        continue;

      sync_info = oper->sync_info;

      /* Look at the tail syncs for each oper.  Must do this
         because this function is called during HtoL and HtoL only
         generates the tail syncs. */

      for (i = 0; i < sync_info->num_sync_in; i++)
        {
          sync = sync_info->sync_in[i];

          dep_oper = sync->dep_oper;

          /* Remove arcs between 2 loads.  Must try to find
             head sync and check for NULL before deleting it or
             accessing it because HtoL only generates the tail syncs and
             this routine is called from HtoL among other places. */
          if (L_load_opcode (oper) && L_load_opcode (dep_oper))
            {
              L_delete_tail_sync (oper, sync);
              if (dep_oper->sync_info != NULL)
                {
                  dep_sync = L_find_head_sync (dep_oper, oper);
                  if (dep_sync != NULL)
                    {
                      L_delete_head_sync (dep_oper, dep_sync);
                    }
                }
              /* need to decrement i since deleted the sync */
              i--;
            }

          /* Adjust any arcs which come to this oper from a later oper in
             the cb (or from the oper itself) and are non-loop carried. 
             Such non loop carried dependences are not possible.  4
             possible cases for non-loop carried arcs.  Must try to find
             head sync and check for NULL before deleting it or
             accessing it because HtoL only generates the tail syncs and
             this routine is called from HtoL among other places. */
          else if (IS_NONLOOP_CARRIED (sync->info) &&
                   Set_in (oper_set, dep_oper->id))
            {
              /* non loop carried only - 1 of 4 cases */
              if (!IS_INNER_CARRIED (sync->info) &&
                  !IS_OUTER_CARRIED (sync->info))
                {
                  L_delete_tail_sync (oper, sync);
                  if (dep_oper->sync_info != NULL)
                    {
                      dep_sync = L_find_head_sync (dep_oper, oper);
                      if (dep_sync != NULL)
                        {
                          L_delete_head_sync (dep_oper, dep_sync);
                        }
                    }
                  /* need to decrement i since deleted the sync */
                  i--;
                }
              /* inner/non loop carried or inner/outer/non - 2 of 4 cases */
              else if (IS_INNER_CARRIED (sync->info))
                {

                  if (!IS_DISTANCE_UNKNOWN (sync->info))
                    {
                      L_punt ("L_adjust_invalid_sync_arcs_in_cb: "
                              "Sync arc is marked as both nonloop and "
                              "inner loop carried but distance unknown "
                              "flag not set - function: %s, cb: %d, "
                              "from_oper: %d, to_oper: %d\n", L_fn->name,
                              cb->id, oper->id, dep_oper->id);
                    }
                  /* Arc becomes inner carried with minimum distance 1 */
                  sync->info = sync->info & ~SET_NONLOOP_CARRIED (0);
                  sync->dist = 1;
                  if (dep_oper->sync_info != NULL)
                    {
                      dep_sync = L_find_head_sync (dep_oper, oper);
                      if (dep_sync != NULL)
                        {
                          dep_sync->info = dep_sync->info &
                            ~SET_NONLOOP_CARRIED (0);
                          dep_sync->dist = 1;
                        }
                    }
                }
              /* outer and non loop carried - 1 of 4 cases */
              else if (!IS_INNER_CARRIED (sync->info) &&
                       IS_OUTER_CARRIED (sync->info))
                {
                  /* Arc becomes outer carried only.  Set distance to 
                     an invalid value for the depth. */
                  sync->info = SET_DISTANCE_UNKNOWN (sync->info) &
                    ~SET_NONLOOP_CARRIED (0);
                  sync->dist = 0;
                  if (dep_oper->sync_info != NULL)
                    {
                      dep_sync = L_find_head_sync (dep_oper, oper);
                      if (dep_sync != NULL)
                        {
                          dep_sync->info =
                            SET_DISTANCE_UNKNOWN (dep_sync->info) &
                            ~SET_NONLOOP_CARRIED (0);
                          dep_sync->dist = 0;
                        }
                    }
                }
            }
        }
    }

  oper_set = Set_dispose (oper_set);
}


/* DML - Remove/adjust sync arcs which are impossible in function */
void
L_adjust_invalid_sync_arcs_in_func (L_Func * fn)
{
  /* BCC - this routine masks off the NONLOOP_CARRIED bit for some sync
   * arcs which are non-loop carried. So it is temporarily fixed before
   * further investigation is made.
   * 5/23/99 
   */
  return;
}


/* DAC - Union of sync arcs, to be called when combining memory ops */
/* The first parameter oper (opA) will get the union of the sync arcs */
void
L_union_sync_out_arc_info (L_Oper * opA, L_Oper * opB)
{
  L_Sync *sync;
  L_Sync_Info *sync_info = opB->sync_info;
  int i;

  if (sync_info == NULL)
    return;

  if (sync_info->num_sync_out == 0)
    return;

  /* Now do a search for each opB's sync, in opA, */
  /* works for subcase of when opA has no arcs    */
  for (i = 0; i < sync_info->num_sync_out; i++)
    {
      sync = sync_info->sync_out[i];

      if (L_find_head_sync (opA, sync->dep_oper))
        continue;

      L_add_sync_between_opers (opA, sync->dep_oper);
    }
}


void
L_union_sync_in_arc_info (L_Oper * opA, L_Oper * opB)
{
  L_Sync *sync;
  L_Sync_Info *sync_info = opB->sync_info;
  int i;

  if (sync_info == NULL)
    return;

  if (sync_info->num_sync_in == 0)
    return;

  /* Now do a search for each opB's sync, in opA, */
  /* works for subcase of when opA has no arcs    */

  for (i = 0; i < sync_info->num_sync_in; i++)
    {
      sync = sync_info->sync_in[i];

      if (L_find_tail_sync (opA, sync->dep_oper))
        continue;

      L_add_sync_between_opers (sync->dep_oper, opA);
    }
}


void
L_union_sync_arc_info (L_Oper * opA, L_Oper * opB)
{
  L_Sync_Info *sync_info;

  /* sync_info_A will be remaining, it gets all of the syncs */
  if (opB->sync_info == NULL)
    return;

  if (opA->sync_info == NULL)
    {

      /* Need to copy all of syncs and change op */
      sync_info = opB->sync_info;
      opA->sync_info = L_copy_sync_info (sync_info);

      /* for all other opers which are dependent upon op, 
         add sync arcs to op */
      L_insert_all_syncs_in_dep_opers (opA);
      return;
    }

#ifdef DEBUG_SYNC_UNION
  fprintf (stderr, "-> Combining syncs for op%d op%d into op%d\n",
           opA->id, opB->id, opA->id);
#endif

  /* Need to take the sync arc union of in and union of out */
  L_union_sync_in_arc_info (opA, opB);
  L_union_sync_out_arc_info (opA, opB);

}


/*
 *   SAM, 4-98.  Added to build sync arcs from the simple Lcode disambiguator.
 *   This is just to represent all the memory dependence info in 1 structure
 *   when the code is fed to external modules, such as Elcor.
 *   Currently sync arcs are assumed to have a direction associated with them,
 *   so arcs are added in a bidirectional fashion.  This leads to extra arcs,
 *   but is conservative.
 *
 *   sync_type is a bit field indicating what deps to represent as sync arcs
 *           L_SYNC_TYPE_NONE - no arcs of any type should be added
 *           L_SYNC_TYPE_LS_LOCAL - add arcs between locally dependent lds/sts
 *           L_SYNC_TYPE_LS_GLOBAL - add arcs between all dependent lds/sts,
 *                                   implies local is done as well.
 *           L_SYNC_TYPE_JSR_LOCAL - add arcs between locally dep 
 *                                   jsrs/lds+sts+jsrs
 *            L_SYNC_TYPE_JSR_GLOBAL - add arcs between all dep 
 *                                     jsrs/lds+sts+jsrs,
 */
void
L_build_sync_arcs_from_lcode_disamb (L_Func * fn, int sync_type,
                                     int insert_consv_cross_iter_arcs)
{
  int i, j, jsr_count, ld_count, st_count, dep_flags, sef_i, sef_j;
  L_Cb *cb;
  L_Oper *op, **ld_ops, **st_ops, **jsr_ops;
  L_Cb **ld_cbs, **st_cbs, **jsr_cbs;
  L_Attr *attr;

  if (sync_type == L_SYNC_TYPE_NONE)
    return; /* No syncs are to be added, so get out of here */

  if (sync_type & L_SYNC_TYPE_LS_GLOBAL)
    sync_type |= L_SYNC_TYPE_LS_LOCAL;
  if (sync_type & L_SYNC_TYPE_JSR_GLOBAL)
    sync_type |= L_SYNC_TYPE_JSR_LOCAL;

  /* Build up a list of the lds, sts, and jsrs */
  jsr_count = 0;
  ld_count = 0;
  st_count = 0;
  ld_ops = (L_Oper **) Lcode_malloc (fn->n_oper * sizeof (L_Oper *));
  ld_cbs = (L_Cb **) Lcode_malloc (fn->n_oper * sizeof (L_Oper *));
  st_ops = (L_Oper **) Lcode_malloc (fn->n_oper * sizeof (L_Oper *));
  st_cbs = (L_Cb **) Lcode_malloc (fn->n_oper * sizeof (L_Oper *));
  jsr_ops = (L_Oper **) Lcode_malloc (fn->n_oper * sizeof (L_Oper *));
  jsr_cbs = (L_Cb **) Lcode_malloc (fn->n_oper * sizeof (L_Oper *));

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (op = cb->first_op; op != NULL; op = op->next_op)
        {
          if (L_load_opcode (op))
            {
              ld_ops[ld_count] = op;
              ld_cbs[ld_count] = cb;
              ld_count++;
            }
          else if (L_store_opcode (op))
            {
              st_ops[st_count] = op;
              st_cbs[st_count] = cb;
              st_count++;
            }
          else if (L_subroutine_call_opcode (op))
            {
              jsr_ops[jsr_count] = op;
              jsr_cbs[jsr_count] = cb;
              jsr_count++;
            }
        }
    }

  /* Connect the various categories of ops up */

  dep_flags = SET_NONLOOP_CARRIED (0);
  /* 1. loads and stores that are not independent */
  for (i = 0; i < st_count; i++)
    {
      for (j = 0; j < ld_count; j++)
        {
          if ((st_cbs[i] == ld_cbs[j]) && (sync_type & L_SYNC_TYPE_LS_LOCAL))
            {
              if (!L_independent_memory_ops (st_cbs[i], st_ops[i], ld_ops[j],
                                             dep_flags))
                {
                  L_add_maybe_sync_between_opers (st_ops[i], ld_ops[j]);
                  L_add_maybe_sync_between_opers (ld_ops[j], st_ops[i]);
                }
              else if (insert_consv_cross_iter_arcs)
                {
                  L_add_maybe_backwards_sync_between_opers (st_ops[i],
                                                            ld_ops[j]);
                }
            }
          else if (sync_type & L_SYNC_TYPE_LS_GLOBAL)
            {
              if (!L_independent_memory_ops (NULL, st_ops[i], ld_ops[j],
                                             dep_flags))
                {
                  L_add_maybe_sync_between_opers (st_ops[i], ld_ops[j]);
                  L_add_maybe_sync_between_opers (ld_ops[j], st_ops[i]);
                }
            }
        }
    }

  /* 2. stores and other stores that are not independent */
  for (i = 0; i < st_count; i++)
    {
      for (j = i + 1; j < st_count; j++)
        {
          if ((st_cbs[i] == st_cbs[j]) && (sync_type & L_SYNC_TYPE_LS_LOCAL))
            {
              if (!L_independent_memory_ops (st_cbs[i], st_ops[i], st_ops[j],
                                             dep_flags))
                {
                  L_add_maybe_sync_between_opers (st_ops[i], st_ops[j]);
                  L_add_maybe_sync_between_opers (st_ops[j], st_ops[i]);
                }
              else if (insert_consv_cross_iter_arcs)
                {
                  L_add_maybe_backwards_sync_between_opers (st_ops[i],
                                                            st_ops[j]);
                }
            }
          else if (sync_type & L_SYNC_TYPE_LS_GLOBAL)
            {
              if (!L_independent_memory_ops (NULL, st_ops[i], st_ops[j],
                                             dep_flags))
                {
                  L_add_maybe_sync_between_opers (st_ops[i], st_ops[j]);
                  L_add_maybe_sync_between_opers (st_ops[j], st_ops[i]);
                }
            }
        }
    }

  /* 3. all jsrs and stores */
  for (i = 0; i < jsr_count; i++)
    {
      for (j = 0; j < st_count; j++)
        {
          if ((jsr_cbs[i] == st_cbs[j])
              && (sync_type & L_SYNC_TYPE_JSR_LOCAL))
            {
              L_add_maybe_sync_between_opers (jsr_ops[i], st_ops[j]);
              L_add_maybe_sync_between_opers (st_ops[j], jsr_ops[i]);
            }
          else if (sync_type & L_SYNC_TYPE_JSR_GLOBAL)
            {
              L_add_maybe_sync_between_opers (jsr_ops[i], st_ops[j]);
              L_add_maybe_sync_between_opers (st_ops[j], jsr_ops[i]);
            }
        }
    }

  /* 4. jsrs that have side effects and loads */
  for (i = 0; i < jsr_count; i++)
    {
      if (L_side_effect_free_sub_call (jsr_ops[i]))
        continue;
      for (j = 0; j < ld_count; j++)
        {
          if ((jsr_cbs[i] == ld_cbs[j])
              && (sync_type & L_SYNC_TYPE_JSR_LOCAL))
            {
              L_add_maybe_sync_between_opers (jsr_ops[i], ld_ops[j]);
              L_add_maybe_sync_between_opers (ld_ops[j], jsr_ops[i]);
            }
          else if (sync_type & L_SYNC_TYPE_JSR_GLOBAL)
            {
              L_add_maybe_sync_between_opers (jsr_ops[i], ld_ops[j]);
              L_add_maybe_sync_between_opers (ld_ops[j], jsr_ops[i]);
            }
        }
    }

  /* jsrs that have side effects and jsrs */
  for (i = 0; i < jsr_count; i++)
    {
      sef_i = L_side_effect_free_sub_call (jsr_ops[i]);
      for (j = i + 1; j < jsr_count; j++)
        {
          sef_j = L_side_effect_free_sub_call (jsr_ops[j]);
          if (sef_i && sef_j)
            continue;
          if ((jsr_cbs[i] == jsr_cbs[j])
              && (sync_type & L_SYNC_TYPE_JSR_LOCAL))
            {
              L_add_maybe_sync_between_opers (jsr_ops[i], jsr_ops[j]);
              L_add_maybe_sync_between_opers (jsr_ops[j], jsr_ops[i]);
            }
          else if (sync_type & L_SYNC_TYPE_JSR_GLOBAL)
            {
              L_add_maybe_sync_between_opers (jsr_ops[i], jsr_ops[j]);
              L_add_maybe_sync_between_opers (jsr_ops[j], jsr_ops[i]);
            }
        }
    }

  Lcode_free (ld_ops);
  Lcode_free (ld_cbs);
  Lcode_free (st_ops);
  Lcode_free (st_cbs);
  Lcode_free (jsr_ops);
  Lcode_free (jsr_cbs);

  /*
   * Attach appropriate attribute to the function to indicate type of arcs.
   * Also, reset the appropriate global syncarc state variables which are set 
   * when the function is read in depending on whether the input contains arcs.
   */
  /* These flags mean global info is present */
  L_func_contains_dep_pragmas = 0;

  L_func_contains_jsr_dep_pragmas = 0;
  if (sync_type & L_SYNC_TYPE_LS_GLOBAL)
    {
      attr = L_new_attr ("DEP_PRAGMAS", 0);
      fn->attr = L_concat_attr (fn->attr, attr);
      L_func_contains_dep_pragmas = 1;
    }
  else if (sync_type & L_SYNC_TYPE_LS_LOCAL)
    {
      attr = L_new_attr ("DEP_PRAGMAS_LOCAL", 0);
      fn->attr = L_concat_attr (fn->attr, attr);
    }
  if (sync_type & L_SYNC_TYPE_JSR_GLOBAL)
    {
      attr = L_new_attr ("JSR_DEP_PRAGMAS", 0);
      fn->attr = L_concat_attr (fn->attr, attr);
      L_func_contains_jsr_dep_pragmas = 1;
    }
  else if (sync_type & L_SYNC_TYPE_JSR_LOCAL)
    {
      attr = L_new_attr ("JSR_DEP_PRAGMAS_LOCAL", 0);
      fn->attr = L_concat_attr (fn->attr, attr);
    }
}


/*! \brief Analyzes sync arcs for 0-Omega potential dependence
 *
 * \param op1
 *  First operation to test for dependence
 * \param op2
 *  Second operation to test for dependence
 * \param dep_flags
 *  Sensitivity mask for different types of sync arcs
 *
 * \return
 *  2 - No relevant sync arcs (INDEPENDENT when no acc specs; 
 *                             DEPENDENT w/ acc specs)
 *  1 - Relevant sync arc present, but is independent - INDEPENDENT
 *  0 - Relevant sync arc demonstrating dependence - DEPENDENT
 *
 * \note
 * This query is insensitive to sync arc direction.  It will, for example,
 * indicate a dependence between a load and a store which have only an
 * anti-dependence relation, regardless of their order of presentation.
 *
 * When using sync arcs only (no acc specs), independence is indicated by
 * a result greater than 0 (either 1 or 2).  When acc specs are present,
 * only a result of 1 indicates independence (a result of 2 indicates no
 * sync arcs are present, so sync arcs provide no additional information
 * on the dependence relation of \a op1 and \a op2.
 */
int
L_analyze_syncs (L_Oper * op1, L_Oper * op2, int dep_flags)
{
  L_Sync_Info *sync_info1, *sync_info2;
  L_Sync *sync;
  int i, hit = 0;

  if (dep_flags == 0)
    L_warn ("L_analyze_syncs: dep_flags is 0; result is always independence");

  if (!(sync_info1 = op1->sync_info) || !(sync_info2 = op2->sync_info))
    return 2;

  for (i = 0; i < sync_info1->num_sync_in; i++)
    {
      sync = sync_info1->sync_in[i];

      if (sync->dep_oper == op2)
	{
	  hit++;
	  if (sync->info & dep_flags)
	    return (0);
	}
    }

  for (i = 0; i < sync_info1->num_sync_out; i++)
    {
      sync = sync_info1->sync_out[i];

      if (sync->dep_oper == op2)
	{
	  hit++;
	  if (sync->info & dep_flags)
	    return (0);
	}
    }

  return (hit ? 1 : 2);
}


/* Interface for non-acc-spec situations */
int
L_analyze_syncs_for_independence (L_Oper * op1, L_Oper * op2, int dep_flags)
{
  return (L_analyze_syncs (op1, op2, dep_flags) > 0);
}


/*! \brief Analyzes sync arcs for (N>0)-Omega potential dependence
 *
 * \param from_oper
 *  First operation to test for dependence
 * \param to_oper
 *  Second operation to test for dependence
 * \param dep_flags
 *  Sensitivity mask for different types of sync arcs
 * \param forward
 *  Flag indicating that \a to_oper appears after \a from_oper in the cb.  
 * \param distance
 *  Pointer to integer to hold the dependence distance result, if found.
 *
 * \return
 *  2 - No relevant sync arcs (INDEPENDENT when no acc specs; 
 *                             DEPENDENT w/ acc specs)
 *  1 - Relevant sync arc present, but is independent - INDEPENDENT
 *  0 - Relevant sync arc demonstrating dependence - DEPENDENT
 *
 * \note
 * This query is *sensitive* to sync arc direction, unlike 
 * \f L_analyze_syncs.
 *
 * When using sync arcs only (no acc specs), independence is indicated by
 * a result greater than 0 (either 1 or 2).  When acc specs are present,
 * only a result of 1 indicates independence (a result of 2 indicates no
 * sync arcs are present, so sync arcs provide no additional information
 * on the dependence relation of \a from_oper and \a to_oper.
 * 
 * If a cross-iteration dependence exists, 0 is returned and an Omega
 * distance is written to *\a distance.  Only dependences carried by
 * the inner loop are supported at this point.
 */
int
L_analyze_syncs_cross (L_Oper * from_oper, L_Oper * to_oper, int dep_flags,
		       int forward, int *distance)
{
  L_Sync_Info *from_sync_info, *to_sync_info;
  L_Sync *sync;
  int i, hit = 0;

  if (!from_oper)
    L_punt ("L_analyze_syncs_cross: "
            "from_oper cannot be NULL - to_oper: %d\n", to_oper->id);
  if (!to_oper)
    L_punt ("L_analyze_syncs_cross: "
            "to_oper cannot be NULL - from_oper: %d\n", from_oper->id);
  if (!distance)
    L_punt ("L_analyze_syncs_cross: "
            "distance cannot be NULL - from_oper: %d, to_oper: %d\n",
            from_oper->id, to_oper->id);

  /* Only search for sync arcs which indicate that a cross-iteration
     dependence for the inner loop should be added.  Cross-iteration
     dependences for outer loops not supported by this routine. */

  if (!IS_INNER_CARRIED (dep_flags))
    {
      L_punt ("L_analyze_syncs_cross: only inner loop "
              "carried dependences supported - from_oper: %d, to_oper: %d, "
              "dep_flags: %d\n", from_oper->id, to_oper->id, dep_flags);
    }
  else
    {
      /* get sync arcs for each oper */

      if (!(from_sync_info = from_oper->sync_info) ||
	  !(to_sync_info = to_oper->sync_info))
	{
	  *distance = 1;
	  return 2;
	}

      /* examine the outgoing sync arcs from from_oper */
      for (i = 0; i < from_sync_info->num_sync_out; i++)
        {
          sync = from_sync_info->sync_out[i];
          if (sync->dep_oper == to_oper)
            {
              /* Have found a sync arc between the 2 opers.  Check if 
                 it indicates a cross-iteration dependence carried 
                 by the inner loop.  Do this by checking the 
                 inner/outer/nonloop flags for the sync arc.  At least
                 one of the flags must be set, so there are 7 valid
                 combinations of the 3 flags. */

	      hit++;

              if (IS_NONLOOP_CARRIED (sync->info) &&
                  !IS_INNER_CARRIED (sync->info))
                {
                  /* This is a non-loop carried dependence.  It should
                     not matter if the outer flag is set as
                     far as scheduling is concerned.  The non-loop carried
                     case is handled by L_analyze_syncs_for_independence, 
                     so don't add any arcs here.  2 of 7 cases. */
                  return (1);
                }
              else if (IS_NONLOOP_CARRIED (sync->info) &&
                       IS_INNER_CARRIED (sync->info))
                {               /* 0+ */
                  /* It should not matter if the outer flag is set as
                     far as scheduling is concerned.  If the dependence
                     is lexically forward, an omega 0 arc was 
                     already added by L_analyze_syncs_for_independence.
                     It is redundant to add another distance 1 arc.
                     If the dependence is lexically backward, a non-loop
                     carried dependence does not make sense and
                     L_compute_memory_dependence never adds a backward
                     arc.  So better add a distance 1 arc.  2 of 7 cases.
                     Note that L_analyze_syncs_for_independence and
                     L_compute_memory_dependence may mistakenly add
                     a forward arc between the 2 opers in this case. 
                     Thus it is best to avoid generating nonsense arcs. */

                  if (!IS_DISTANCE_UNKNOWN (sync->info))
                    {
                      L_punt ("L_analyze_syncs_cross: "
                              "Sync arc is marked as both nonloop and "
                              "inner loop carried but distance unknown "
                              "flag not set - from_oper: %d, to_oper: %d\n",
                              from_oper->id, to_oper->id);
                    }

                  if (!forward)
                    {
                      *distance = 1;
                      return (0);
                    }
                }
              else if (IS_INNER_CARRIED (sync->info) &&
                       !IS_NONLOOP_CARRIED (sync->info))
                {
                  /* This is an inner loop carried arc.  This
                     may or may not also be a dependence between the
                     outer loop iterations.  That is irrelevant
                     as far as scheduling the inner loop.  2 of 7 cases. */
                  if (IS_DISTANCE_UNKNOWN (sync->info))
		    *distance = 1;
                  else
		    *distance = sync->dist;

		  return 0;
                }
              else if (IS_OUTER_CARRIED (sync->info) &&
                       !IS_INNER_CARRIED (sync->info) &&
                       !IS_NONLOOP_CARRIED (sync->info))
                {
                  /* Outer alone means no cross-iter dep for inner loop.
                     1 of 7 cases. */
                  return 1;
                }
#if 0
              else
                {
                  L_punt ("L_analyze_syncs_cross: "
                          "Invalid sync arc - at least one of "
                          "inner/outer/nonloop must be set - from_oper: %d, "
                          "to_oper: %d, sync info: %d\n", from_oper->id,
                          to_oper->id, sync->info);
                }
#endif
            }
        }

#if 0
      if (L_func_acc_omega && !hit)
	for (i = 0; i < from_sync_info->num_sync_in; i++)
	  {
	    sync = from_sync_info->sync_in[i];
	    if (sync->dep_oper == to_oper)
	      {
		hit++;
		break;
	      }
	  }
#endif
    }

  if (!hit)
    {
      *distance = 1;
      return 2;
    }
  else
    {
      return 1;
    }
}


/* Interface for non-acc-spec situations */
int
L_analyze_syncs_for_cross_iter_independence (L_Oper * from_oper,
                                             L_Oper * to_oper, int dep_flags,
                                             int forward, int *distance)
{
  return (L_analyze_syncs_cross (from_oper, to_oper, dep_flags, 
				 forward, distance) > 0);
}
