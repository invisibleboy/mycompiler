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
 *      File :          l_loop.c
 *      Description :   Lcode loop detection
 *      Author :        Scott Mahlke, Pohua Chang, Wen-mei Hwu
 *      Creation Date : July, 1990
 *      Revised :       February 1993, Scott Mahlke
 *      external fns: 1. L_loop_detection(L_Func * fn, int find_preheaders)
 *                              requires - dominator analysis
 *                    2. L_print_loop_data(L_Func * fn)
 *                    3. L_inner_loop_detection(L_Func * fn, 
 *                                              int find_preheaders)
 *                    4. L_print_inner_loop_data(L_Func * fn)
 *                    5. L_compute_static_cb_weight(L_Func * fn)
 *                              requires - L_loop_detection()
 *
 *==========================================================================*/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>

#undef DEBUG_FIND_IND_VAR

static void L_find_nesting_level (L_Func * fn);

void
L_reset_loop_headers (L_Func * fn)
{
  L_Cb *cb;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    cb->flags = L_CLR_BIT_FLAG (cb->flags, L_CB_LOOP_HEADER);
}

/*=========================================================================*/
/*
 *      Basic loop structure manipulation functions
 */
/*==========================================================================*/

L_Loop *
L_new_loop (L_Cb * header)
{
  L_Loop *l;

  l = (L_Loop *) L_alloc (L_alloc_loop);

  L_fn->max_loop_id++;
  l->id = L_fn->max_loop_id;
  l->flags = 0;
  l->preheader = NULL;
  l->header = header;
  l->num_invocation = 0.0;
  l->nesting_level = -1;
  l->loop_cb = NULL;
  l->back_edge_cb = NULL;
  l->exit_cb = NULL;
  l->out_cb = NULL;
  l->nested_loops = NULL;
  l->basic_ind_var = NULL;
  l->basic_ind_var_op = NULL;
  l->ind_info = NULL;
  l->prev_loop = NULL;
  l->next_loop = NULL;
  l->parent_loop = NULL;
  l->sibling_loop = NULL;
  l->child_loop = NULL;

  return l;
}

void
L_delete_loop (L_Func * fn, L_Loop * loop)
{
  L_Loop *prev, *next;

  /* Handle case of null ptr */
  if (loop == NULL)
    return;

  /* disconnect loop from doubly linked list */
  prev = loop->prev_loop;
  next = loop->next_loop;
  if (prev != NULL)
    prev->next_loop = next;
  if (next != NULL)
    next->prev_loop = prev;
  loop->prev_loop = NULL;
  loop->next_loop = NULL;
  loop->parent_loop = NULL;
  loop->sibling_loop = NULL;
  loop->child_loop = NULL;

  /* fix up first_loop ptr if necessary */
  if (fn->first_loop == loop)
    fn->first_loop = next;

  /* free sets if necessary */
  if (loop->loop_cb != NULL)
    Set_dispose (loop->loop_cb);
  if (loop->back_edge_cb != NULL)
    Set_dispose (loop->back_edge_cb);
  if (loop->exit_cb != NULL)
    Set_dispose (loop->exit_cb);
  if (loop->out_cb != NULL)
    Set_dispose (loop->out_cb);
  if (loop->nested_loops != NULL)
    Set_dispose (loop->nested_loops);
  if (loop->basic_ind_var != NULL)
    Set_dispose (loop->basic_ind_var);
  if (loop->basic_ind_var_op != NULL)
    Set_dispose (loop->basic_ind_var_op);

  L_delete_all_ind_info (&(loop->ind_info));

  /* free the loop */
  L_free (L_alloc_loop, loop);
}

void
L_delete_all_loop (L_Loop * list)
{
  L_Loop *ptr, *next;

  for (ptr = list; ptr != NULL; ptr = next)
    {
      next = ptr->next_loop;
      ptr->prev_loop = NULL;
      ptr->next_loop = NULL;
      ptr->parent_loop = NULL;
      ptr->sibling_loop = NULL;
      ptr->child_loop = NULL;

      if (ptr->loop_cb != NULL)
        Set_dispose (ptr->loop_cb);
      if (ptr->back_edge_cb != NULL)
        Set_dispose (ptr->back_edge_cb);
      if (ptr->exit_cb != NULL)
        Set_dispose (ptr->exit_cb);
      if (ptr->out_cb != NULL)
        Set_dispose (ptr->out_cb);
      if (ptr->nested_loops != NULL)
        Set_dispose (ptr->nested_loops);
      if (ptr->basic_ind_var != NULL)
        Set_dispose (ptr->basic_ind_var);
      if (ptr->basic_ind_var_op != NULL)
        Set_dispose (ptr->basic_ind_var_op);

      L_delete_all_ind_info (&(ptr->ind_info));

      L_free (L_alloc_loop, ptr);
    }
}

L_Loop *
L_concat_loop (L_Loop * l1, L_Loop * l2)
{
  L_Loop *ptr;

  if (l1 == NULL)
    return l2;
  if (l2 == NULL)
    return l1;

  ptr = l1;
  while (ptr->next_loop != NULL)
    {
      ptr = ptr->next_loop;
    }
  l2->prev_loop = ptr;
  ptr->next_loop = l2;

  return l1;
}

/*=========================================================================*/
/*
 *      General loop detection functions
 */
/*==========================================================================*/

void
L_print_specific_loop_data (L_Loop * l)
{
  fprintf (stderr, "### Loop information (loop %d) ###\n", l->id);
  fprintf (stderr, "\tnum_invocation     %f\n", l->num_invocation);
  if (l->num_invocation > 0.0)
    fprintf (stderr, "\tave_iter           %f\n",
             l->header->weight / l->num_invocation);
  else
    fprintf (stderr, "\tave_iter           0.0\n");
  if (l->preheader != NULL)
    fprintf (stderr, "\tpreheader          %d\n", l->preheader->id);
  else
    fprintf (stderr, "\tpreheader        NULL\n");
  if (l->header != NULL)
    fprintf (stderr, "\theader             %d\n", l->header->id);
  else
    fprintf (stderr, "\theader           NULL\n");
  fprintf (stderr, "\tnesting_level      %d\n", l->nesting_level);
  fprintf (stderr, "\tloop_size (no. cb) %d\n", Set_size (l->loop_cb));
  fprintf (stderr, "\t");
  Set_print (stderr, "loop_cb", l->loop_cb);
  fprintf (stderr, "\t");
  Set_print (stderr, "back_edge_cb", l->back_edge_cb);
  fprintf (stderr, "\t");
  Set_print (stderr, "exit_cb", l->exit_cb);
  fprintf (stderr, "\t");
  Set_print (stderr, "out_cb", l->out_cb);
  fprintf (stderr, "\t");
  Set_print (stderr, "nested_loops", l->nested_loops);
  fprintf (stderr, "\t");
  Set_print (stderr, "basic_ind_var", l->basic_ind_var);
  fprintf (stderr, "\t");
  Set_print (stderr, "basic_ind_var_op", l->basic_ind_var_op);
  fprintf (stderr, "\n");
}

void
L_print_loop_data (L_Func * fn)
{
  L_Loop *l;
  fprintf (stderr, "### Loop information (fn %s) ###\n", fn->name);
  for (l = fn->first_loop; l != NULL; l = l->next_loop)
    {
      fprintf (stderr, "\tid                 %d\n", l->id);
      fprintf (stderr, "\tnum_invocation     %f\n", l->num_invocation);
      if (l->num_invocation > 0.0)
        fprintf (stderr, "\tave_iter           %f\n",
                 l->header->weight / l->num_invocation);
      else
        fprintf (stderr, "\tave_iter           0.0\n");
      if (l->preheader != NULL)
        fprintf (stderr, "\tpreheader          %d\n", l->preheader->id);
      else
        fprintf (stderr, "\tpreheader        NULL\n");
      if (l->header != NULL)
        fprintf (stderr, "\theader             %d\n", l->header->id);
      else
        fprintf (stderr, "\theader           NULL\n");
      fprintf (stderr, "\tnesting_level      %d\n", l->nesting_level);
      fprintf (stderr, "\tloop_size (no. cb) %d\n", Set_size (l->loop_cb));
      fprintf (stderr, "\t");
      Set_print (stderr, "loop_cb", l->loop_cb);
      fprintf (stderr, "\t");
      Set_print (stderr, "back_edge_cb", l->back_edge_cb);
      fprintf (stderr, "\t");
      Set_print (stderr, "exit_cb", l->exit_cb);
      fprintf (stderr, "\t");
      Set_print (stderr, "out_cb", l->out_cb);
      fprintf (stderr, "\t");
      Set_print (stderr, "nested_loops", l->nested_loops);
      fprintf (stderr, "\t");
      Set_print (stderr, "basic_ind_var", l->basic_ind_var);
      fprintf (stderr, "\t");
      Set_print (stderr, "basic_ind_var_op", l->basic_ind_var_op);
      fprintf (stderr, "\n");
    }
}

void
L_print_loop_data_only_innermost (L_Func * fn)
{
  L_Loop *l;
  fprintf (stderr, "### Loop information (fn %s) ###\n", fn->name);
  for (l = fn->first_loop; l != NULL; l = l->next_loop)
    {
      if (!Set_empty (l->nested_loops))
        continue;
      fprintf (stderr, "\tid                 %d\n", l->id);
      fprintf (stderr, "\tnum_invocation     %f\n", l->num_invocation);
      if (l->num_invocation > 0.0)
        fprintf (stderr, "\tave_iter           %f\n",
                 l->header->weight / l->num_invocation);
      else
        fprintf (stderr, "\tave_iter           0.0\n");
      if (l->preheader != NULL)
        fprintf (stderr, "\tpreheader          %d\n", l->preheader->id);
      else
        fprintf (stderr, "\tpreheader        NULL\n");
      if (l->header != NULL)
        fprintf (stderr, "\theader             %d\n", l->header->id);
      else
        fprintf (stderr, "\theader           NULL\n");
      fprintf (stderr, "\tnesting_level      %d\n", l->nesting_level);
      fprintf (stderr, "\tloop_size (no. cb) %d\n", Set_size (l->loop_cb));
      fprintf (stderr, "\t");
      Set_print (stderr, "loop_cb", l->loop_cb);
      fprintf (stderr, "\t");
      Set_print (stderr, "back_edge_cb", l->back_edge_cb);
      fprintf (stderr, "\t");
      Set_print (stderr, "exit_cb", l->exit_cb);
      fprintf (stderr, "\t");
      Set_print (stderr, "out_cb", l->out_cb);
      fprintf (stderr, "\t");
      Set_print (stderr, "nested_loops", l->nested_loops);
      fprintf (stderr, "\t");
      Set_print (stderr, "basic_ind_var", l->basic_ind_var);
      fprintf (stderr, "\t");
      Set_print (stderr, "basic_ind_var_op", l->basic_ind_var_op);
      fprintf (stderr, "\n");
    }
}

static void
L_reset_visited_flag (L_Func * fn)
{
  L_Cb *cb;
  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    cb->flags = L_CLR_BIT_FLAG (cb->flags, L_CB_VISITED);
}

static void
L_add_blocks (L_Loop * l, L_Cb * cb, L_Cb * header_cb)
{
  L_Flow *src;
  for (src = cb->src_flow; src != NULL; src = src->next_flow)
    {
      L_Cb *pred_cb;
      pred_cb = src->src_cb;
      if (pred_cb == header_cb)
        continue;
      if (L_EXTRACT_BIT_VAL (pred_cb->flags, L_CB_VISITED))
        continue;
      pred_cb->flags = L_SET_BIT_FLAG (pred_cb->flags, L_CB_VISITED);
      l->loop_cb = Set_add (l->loop_cb, pred_cb->id);
      L_add_blocks (l, pred_cb, header_cb);
    }
}

static void
L_find_blocks_in_loop (L_Func * fn, L_Loop * l)
{
  int b_edge_cb[1], back_edge_cb_id;
  L_Cb *header_cb, *back_edge_cb;
  if (l == NULL)
    L_punt ("L_find_blocks_in_loop: l cannot be NIL");
  if (Set_size (l->back_edge_cb) != 1)
    L_punt ("L_find_blocks_in_loop: must have only 1 back edge block");
  header_cb = l->header;
  Set_2array (l->back_edge_cb, b_edge_cb);
  back_edge_cb_id = b_edge_cb[0];
  back_edge_cb = L_cb_hash_tbl_find (fn->cb_hash_tbl, back_edge_cb_id);
  l->loop_cb = Set_add (l->loop_cb, l->header->id);
  if (back_edge_cb != header_cb)
    {
      l->loop_cb = Set_add (l->loop_cb, back_edge_cb_id);
      L_reset_visited_flag (fn);
      L_add_blocks (l, back_edge_cb, header_cb);
    }
}

static void
L_find_exit_blocks_in_loop (L_Func * fn)
{
  int j, num_cb, *loop_cb;
  L_Loop *l;
  loop_cb = (int *) malloc (sizeof (int) * fn->n_cb);
  for (l = fn->first_loop; l != NULL; l = l->next_loop)
    {
      if (l == NULL)
        L_punt ("L_find_exit_blocks_in_loop: l cannot be NIL");
      if (l->exit_cb != NULL)
        l->exit_cb = Set_dispose (l->exit_cb);
      if (l->out_cb != NULL)
        l->out_cb = Set_dispose (l->out_cb);
      num_cb = Set_size (l->loop_cb);
      if (num_cb <= 0)
        L_punt ("L_find_exit_blocks_in_loop: illegal no. cb in loop");
      Set_2array (l->loop_cb, loop_cb);
      for (j = 0; j < num_cb; j++)
        {
          L_Cb *cb;
          L_Flow *dest;
          cb = L_cb_hash_tbl_find (fn->cb_hash_tbl, loop_cb[j]);
          for (dest = cb->dest_flow; dest != NULL; dest = dest->next_flow)
            {
              if (Set_in (l->loop_cb, dest->dst_cb->id))
                continue;
              l->exit_cb = Set_add (l->exit_cb, cb->id);
              l->out_cb = Set_add (l->out_cb, dest->dst_cb->id);
            }
        }
    }
  free (loop_cb);
}

static void
L_merge_loops (L_Func * fn)
{
  L_Loop *loop1, *loop2, *next2;
  for (loop1 = fn->first_loop; loop1 != NULL; loop1 = loop1->next_loop)
    {
      for (loop2 = loop1->next_loop; loop2 != NULL; loop2 = next2)
        {
          Set temp;
          next2 = loop2->next_loop;
          if (loop1->header != loop2->header)
            continue;
          /* merge loop1 and loop2 into loop1 */
          temp = loop1->loop_cb;
          loop1->loop_cb = Set_union (loop1->loop_cb, loop2->loop_cb);
          temp = Set_dispose (temp);
          temp = loop1->back_edge_cb;
          loop1->back_edge_cb = Set_union (loop1->back_edge_cb,
                                           loop2->back_edge_cb);
          temp = Set_dispose (temp);
          L_delete_loop (fn, loop2);
        }
    }
}

void
L_merge_two_loops (L_Func * fn, L_Loop * loop1, L_Loop * loop2)
{
  L_Loop *loop_iter;

  if (loop1->header != loop2->header)
    return;

  /* merge loop1 and loop2 into loop1 */
  loop1->loop_cb = Set_union_acc (loop1->loop_cb, loop2->loop_cb);
  loop1->back_edge_cb = Set_union_acc (loop1->back_edge_cb,
                                       loop2->back_edge_cb);

  loop_iter = fn->first_loop;

  while (loop_iter)
    {
      if (loop_iter->nested_loops)
	loop_iter->nested_loops = Set_delete (loop_iter->nested_loops,
					      loop2->id);
      loop_iter = loop_iter->next_loop;
    }
  L_delete_loop (fn, loop2);
  L_find_nesting_level (fn);
  return;
}

static void
L_verify_header_dominance (L_Func * fn)
{
  int j, num_cb, *loop_cb;
  L_Cb *header;
  L_Loop *l, *next;
  loop_cb = (int *) malloc (sizeof (int) * fn->n_cb);
  for (l = fn->first_loop; l != NULL; l = next)
    {
      next = l->next_loop;
      header = l->header;
      num_cb = Set_size (l->loop_cb);
      if (num_cb <= 0)
        L_punt ("L_verify_header_dominance: illegal num_cb");
      Set_2array (l->loop_cb, loop_cb);
      for (j = 0; j < num_cb; j++)
        {
          L_Cb *cb;
          cb = L_cb_hash_tbl_find (fn->cb_hash_tbl, loop_cb[j]);
          if (!L_in_cb_DOM_set (cb, header->id))
            {
              printf ("************************************\n");
              printf (" Header does not dominate for loop %d\n", l->id);
              printf ("************************************\n");
              L_print_loop_data (fn);
              L_delete_loop (fn, l);
              break;
            }
        }
    }
  free (loop_cb);
}

static void
L_find_num_invocation (L_Func * fn)
{
  L_Loop *l;
  L_Cb *header;
  L_Attr *attr;
  L_Flow *src;
  double weight;

  for (l = fn->first_loop; l != NULL; l = l->next_loop)
    {
      header = l->header;
      if ((attr = L_find_attr (header->attr, L_ITER_INFO_HEADER)) != NULL)
        {
          weight = 0.0;

	  for (attr = header->attr; attr != NULL; attr=attr->next_attr)
	    {
	      if (!strncmp (attr->name, L_ITER_PREFIX, 
			    L_ITER_PREFIX_LENGTH)) 
		weight += attr->field[0]->value.f2;
	    }
        }
      else
        {
          weight = header->weight;
          for (src = header->src_flow; src != NULL; src = src->next_flow)
            {
              if (Set_in (l->back_edge_cb, src->src_cb->id))
                weight -= src->weight;
            }
        }
      l->num_invocation = weight;
    }
}

/*
 *      For now, remove loop if it not laid out correctly, may want to
 *      change this in future so still count loops like this in static
 *      weight estimate, etc, just dont optimize improperly laid out loops.
 */
void
L_verify_correct_layout (L_Func * fn)
{
  L_Cb *header;
  L_Loop *l, *next;
  for (l = fn->first_loop; l != NULL; l = next)
    {
      next = l->next_loop;
      header = l->header;
      if (header == NULL)
        L_punt ("L_verify_correct_layout: header is NULL");
      if (header == fn->first_cb)       
        /* true by default if header is first cb */
        continue;
      else if (Set_in (l->loop_cb, header->prev_cb->id))
        {
          if (L_debug_loop)
	    fprintf (stderr, "Not proper layout for loop %d\n", l->id);

          L_delete_loop (fn, l);
        }
    }
}

static void
L_loop_det (L_Func * fn, int merge)
{
  L_Cb *cb1, *cb2;
  L_Flow *dest;
  L_Loop *l;

  /* succeeding block that dominates means loop back edge */
  for (cb1 = fn->first_cb; cb1 != NULL; cb1 = cb1->next_cb)
    {
      for (dest = cb1->dest_flow; dest != NULL; dest = dest->next_flow)
        {
          cb2 = dest->dst_cb;
          if (L_in_cb_DOM_set (cb1, cb2->id))
            {
              l = L_new_loop (cb2);
              fn->first_loop = L_concat_loop (fn->first_loop, l);
              l->back_edge_cb = Set_add (l->back_edge_cb, cb1->id);
              L_find_blocks_in_loop (fn, l);
            }
        }
    }
  L_find_exit_blocks_in_loop (fn);
  if (merge)
    L_merge_loops (fn);
  L_find_exit_blocks_in_loop (fn);
  L_verify_header_dominance (fn);
  L_find_num_invocation (fn);
#if 0
  L_verify_correct_layout (fn);
#endif
}

static L_Cb *
L_create_loop_preheader (L_Func * fn, L_Loop * l)
{
  L_Cb *header_cb, *preheader_cb, *src_cb, *prev_cb;
  L_Flow *src, *new_flow, *next_src;
  L_Oper *last_op, *new_op;

  header_cb = l->header;

  if ((prev_cb = header_cb->prev_cb) &&
      Set_in (l->loop_cb, prev_cb->id) &&
      L_has_fallthru_to_next_cb (prev_cb))
    {
      last_op = prev_cb->last_op;
      if (L_cond_branch (last_op) || !L_general_branch_opcode (last_op))
	{
	  L_Flow *src_flow, *dest_flow;
	  
	  dest_flow = L_find_last_flow (prev_cb->dest_flow);
	  src_flow = L_find_matching_flow (header_cb->src_flow, dest_flow);
	  new_op = L_create_new_op (Lop_JUMP);
	  new_op->src[0] = L_new_cb_operand (header_cb);
	  L_insert_oper_after (prev_cb, last_op, new_op);
	  src_flow->cc = 1;
	  dest_flow->cc = 1;
	}
    }

  preheader_cb = L_create_cb (l->num_invocation);
  L_insert_cb_before (fn, header_cb, preheader_cb);

  /* fix control flow */
  for (src = header_cb->src_flow; src != NULL; src = next_src)
    {
      next_src = src->next_flow;
      src_cb = src->src_cb;
      if (Set_in (l->loop_cb, src_cb->id))
	continue;

      L_change_dest (src_cb->dest_flow, header_cb, preheader_cb);
      L_change_all_branch_dest (src_cb, header_cb, preheader_cb);
      new_flow = L_new_flow (src->cc, src->src_cb, preheader_cb, src->weight);
      preheader_cb->src_flow =
        L_concat_flow (preheader_cb->src_flow, new_flow);
      header_cb->src_flow = L_delete_flow (header_cb->src_flow, src);
    }

  /* Add flow between preheader and header */
  new_flow = L_new_flow (0, preheader_cb, header_cb, l->num_invocation);
  preheader_cb->dest_flow = L_concat_flow (preheader_cb->dest_flow, new_flow);
  new_flow = L_new_flow (0, preheader_cb, header_cb, l->num_invocation);
  header_cb->src_flow = L_concat_flow (header_cb->src_flow, new_flow);
  return preheader_cb;
}

static void
L_find_loop_preheaders (L_Func * fn)
{
  int need_preheader;
  L_Loop *l;
  L_Cb *header_cb, *prev_cb;
  L_Flow *src;
  for (l = fn->first_loop; l != NULL; l = l->next_loop)
    {
      need_preheader = 0;
      header_cb = l->header;
      prev_cb = header_cb->prev_cb;
      if (Set_in (l->loop_cb, prev_cb->id))
	{
	  need_preheader = 1;
	}
      else if (prev_cb == fn->first_cb)
        {
          need_preheader = 1;
        }
      else
        {
          for (src = header_cb->src_flow; src != NULL; src = src->next_flow)
            {
              if (src->src_cb == prev_cb)
                {
                  if (L_uncond_branch_opcode (src->src_cb->last_op))
                    {
                      need_preheader = 1;
                      break;
                    }
                  else
                    continue;
                }
              if (!Set_in (l->loop_cb, src->src_cb->id))
                {
                  need_preheader = 1;
                  break;
                }
            }
        }

      if (L_EXTRACT_BIT_VAL (header_cb->flags, L_CB_BOUNDARY))
        need_preheader = 0;

      if ((!need_preheader) &&
          (prev_cb->dest_flow) &&
          (prev_cb->dest_flow->dst_cb == header_cb) &&
          (prev_cb->dest_flow->next_flow == NULL))
        l->preheader = prev_cb;
      else
        l->preheader = L_create_loop_preheader (fn, l);
    }
}

/*
 *      Return 1 iff loop2 is nested within loop1
 */
static int
L_is_nested (L_Loop * loop1, L_Loop * loop2)
{
  int i, num_loop2_cb;
  int *loop2_cb;
  if ((loop1 == NULL) || (loop2 == NULL))
    L_punt ("L_is_nested: loop1 and loop2 cannot be NIL");
  if (loop1 == loop2)
    return 0;
  num_loop2_cb = Set_size (loop2->loop_cb);
  if (num_loop2_cb <= 0)
    L_punt ("L_is_nested: illegal num_loop2_cb");
  loop2_cb = (int *) malloc (sizeof (int) * num_loop2_cb);
  Set_2array (loop2->loop_cb, loop2_cb);
  for (i = 0; i < num_loop2_cb; i++)
    {
      if (!Set_in (loop1->loop_cb, loop2_cb[i]))
        {
          free (loop2_cb);
          return 0;
        }
    }
  free (loop2_cb);
  return 1;
}

static void
L_find_nesting_level (L_Func * fn)
{
  L_Loop *loop1, *loop2;
  for (loop1 = fn->first_loop; loop1 != NULL; loop1 = loop1->next_loop)
    {
      loop1->nesting_level = 1;
    }
  for (loop2 = fn->first_loop; loop2 != NULL; loop2 = loop2->next_loop)
    {
      for (loop1 = fn->first_loop; loop1 != NULL; loop1 = loop1->next_loop)
        {
          if (loop2 == loop1)
            continue;
          if (L_is_nested (loop1, loop2))
            {
              loop2->nesting_level += 1;
              loop1->nested_loops = Set_add (loop1->nested_loops, loop2->id);
              if (loop2->preheader)
                loop1->loop_cb =
                  Set_add (loop1->loop_cb, loop2->preheader->id);
            }
        }
    }
}

/*
 *      NJW - remove all inner parallel loops
 */
static int
L_is_parloop (L_Cb * cb)
{
  L_Attr *attr;
  for (attr = cb->attr; attr != NULL; attr = attr->next_attr)
    {
      if ((!strcmp (attr->name, "DOALL"))
          || (!strcmp (attr->name, "DOACROSS")))
        return (1);
    }
  return (0);
}

static void
L_delete_parloop (L_Cb * cb)
{
  L_Attr *attr, *next;
  for (attr = cb->attr; attr != 0; attr = next)
    {
      next = attr->next_attr;
      if ((!strcmp (attr->name, "DOALL"))
          || (!strcmp (attr->name, "DOACROSS")))
        {
          cb->attr = L_delete_attr (cb->attr, attr);
        }
    }
}

void
L_single_par_nesting (L_Func * fn)
{
  L_Cb *header1_cb, *header2_cb;
  L_Loop *loop1, *loop2;
  for (loop2 = fn->first_loop; loop2 != NULL; loop2 = loop2->next_loop)
    {
      header2_cb = loop2->header;
      if (!L_is_parloop (header2_cb))
        continue;
      for (loop1 = fn->first_loop; loop1 != NULL; loop1 = loop1->next_loop)
        {
          if (loop2 == loop1)
            continue;
          header1_cb = loop1->header;
          if (L_is_parloop (header1_cb) && L_is_nested (loop1, loop2))
            L_delete_parloop (header2_cb);
        }
    }
}

static void
L_mark_loop_headers (L_Func * fn)
{
  L_Loop *loop;

  for (loop = fn->first_loop; loop != NULL; loop = loop->next_loop)
    loop->header->flags = L_SET_BIT_FLAG (loop->header->flags,
					  L_CB_LOOP_HEADER);
}

static void
L_find_deepest_nest_for_cbs (L_Func * fn)
{
  L_Cb *cb;
  L_Loop *loop;
  int cb_nest;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      cb->deepest_loop = NULL;
      cb_nest = 0;
      for (loop = fn->first_loop; loop != NULL; loop = loop->next_loop)
        {
          if (loop->nesting_level > cb_nest)
            {
              if (Set_in (loop->loop_cb, cb->id))
                {
                  cb_nest = loop->nesting_level;
                  cb->deepest_loop = loop;
                }
            }
        }
    }
}

static void
L_make_first_loop_level_one (L_Func * fn)
{
  L_Loop *loop, *first_level_loop = NULL;

  if ((fn->first_loop != NULL) && (fn->first_loop->nesting_level != 1))
    {

      for (loop = fn->first_loop->next_loop; loop != NULL;
           loop = loop->next_loop)
        {
          if (loop->nesting_level == 1)
            {
              first_level_loop = loop;
              loop->prev_loop->next_loop = loop->next_loop;
              if (loop->next_loop != NULL)
                loop->next_loop->prev_loop = loop->prev_loop;
              break;
            }
        }

      if (first_level_loop == NULL)
        L_punt ("L_make_first_loop_level_one: no level one loop found");

      fn->first_loop->prev_loop = first_level_loop;
      first_level_loop->next_loop = fn->first_loop;
      first_level_loop->prev_loop = NULL;
      fn->first_loop = first_level_loop;
    }
}

static void
L_reconnect_loop_hierarchy (L_Func * fn)
{
  L_Loop *loop1, *loop2;

  /* find the parent, then link the children */

  for (loop2 = fn->first_loop; loop2 != NULL; loop2 = loop2->next_loop)
    {
      if (loop2->nesting_level == 1)
        continue;
      for (loop1 = fn->first_loop; loop1 != NULL; loop1 = loop1->next_loop)
        {
          if (loop2 == loop1)
            continue;
          if (loop2->nesting_level == (loop1->nesting_level + 1))
            {
              if (L_is_nested (loop1, loop2))
                {
                  loop2->parent_loop = loop1;
                  loop2->sibling_loop = loop1->child_loop;
                  loop1->child_loop = loop2;
                  break;
                }
            }
        }
      if (loop2->parent_loop == NULL)
        L_punt ("L_reconnect_loop_hierarchy: parent loop not found");
    }

  /* assure first loop in fn is a level one loop */

  L_make_first_loop_level_one (fn);

  if (fn->first_loop != NULL)
    {
      if (fn->first_loop->nesting_level != 1)
        L_punt ("L_reconnect_loop_hierarchy: fn first_loop not level 1");

      /* finally, link all level one siblings */

      loop2 = fn->first_loop;

      for (loop1 = fn->first_loop->next_loop; loop1 != NULL;
           loop1 = loop1->next_loop)
        {
          if (loop1->nesting_level == 1)
            {
              loop2->sibling_loop = loop1;
              loop2 = loop2->sibling_loop;
            }
        }
    }
}

void
L_loop_detection (L_Func * fn, int find_preheaders)
{
  /* Assume when do loop_det throw away all old info */
  fn->max_loop_id = 0;
  if (fn->first_loop != NULL)
    {
      L_delete_all_loop (fn->first_loop);
      fn->first_loop = NULL;
    }
  L_loop_det (fn, 1);
  if (find_preheaders)
    L_find_loop_preheaders (fn);
  L_find_nesting_level (fn);
  L_find_exit_blocks_in_loop (fn);
  L_mark_loop_headers (fn);
  L_find_deepest_nest_for_cbs (fn);
  L_reconnect_loop_hierarchy (fn);
  if (L_debug_loop)
    L_print_loop_data (fn);
}

void
L_loop_detection_nomerge (L_Func * fn, int find_preheaders)
{
  /* Assume when do loop_det throw away all old info */
  fn->max_loop_id = 0;
  if (fn->first_loop != NULL)
    {
      L_delete_all_loop (fn->first_loop);
      fn->first_loop = NULL;
    }
  L_loop_det (fn, 0);
  if (find_preheaders)
    L_find_loop_preheaders (fn);
  L_find_nesting_level (fn);
  L_find_exit_blocks_in_loop (fn);
  L_mark_loop_headers (fn);
  L_find_deepest_nest_for_cbs (fn);
  L_reconnect_loop_hierarchy (fn);
  if (L_debug_loop)
    L_print_loop_data (fn);
}

/*=========================================================================*/
/*
 *      Functions to fill in the basic_ind_var and basic_ind_var_op
 *      fields in the L_Loop data structure.
 */
/*==========================================================================*/

/*
 *      Detect all basic induction variables for each loop in fn
 */
void
L_find_basic_ind_var (L_Func * fn)
{
  int j, num_cb, *loop_cb = NULL;
  L_Cb *cb;
  L_Oper *op;
  L_Loop *loop;

  loop_cb = (int *) alloca (sizeof (int) * fn->n_cb);

  for (loop = fn->first_loop; loop; loop = loop->next_loop)
    {
      if (loop->basic_ind_var)
        loop->basic_ind_var = Set_dispose (loop->basic_ind_var);
      if (loop->basic_ind_var_op)
        loop->basic_ind_var_op = Set_dispose (loop->basic_ind_var_op);

      num_cb = Set_size (loop->loop_cb);
      Set_2array (loop->loop_cb, loop_cb);
      for (j = 0; j < num_cb; j++)
        {          
          if (!(cb = L_cb_hash_tbl_find (fn->cb_hash_tbl, loop_cb[j])))
            L_punt ("L_find_basic_ind_var: loop_cb is corrupt");

          for (op = cb->first_op; op != NULL; op = op->next_op)
            {
              /*
               * if op is a subtract of a constant, convert it to an
               * add of a negative constant, so all induction vars
               * only modified by adds.
               */
              if (L_int_sub_opcode (op) && L_is_int_constant (op->src[1]))
                {
                  if (L_is_opcode (Lop_SUB, op))
                    L_change_opcode (op, Lop_ADD);
                  else
                    L_change_opcode (op, Lop_ADD_U);
                  op->src[1]->value.i = -op->src[1]->value.i;
                }
              /*
               *      match pattern
               */
              if (!(L_int_add_opcode (op)))
                continue;
              if (!(L_is_register (op->dest[0])))
                continue;
              if (!(L_same_operand (op->dest[0], op->src[0])))
                continue;
              if (Set_in (loop->basic_ind_var, op->dest[0]->value.r))
                continue;
              if (!(L_is_loop_inv_operand (loop, loop_cb, num_cb, op->src[1])))
                continue;
              if (!(L_unique_def_in_loop (loop, loop_cb, num_cb, op)))
                continue;
              /*
               *      record as basic ind var
               */
#ifdef DEBUG_FIND_IND_VAR
              fprintf (stderr,
                       "record r%d as in var for loop %d (header %d)\n",
                       op->dest[0]->value.r, loop->id, loop->header->id);
#endif
              loop->basic_ind_var = Set_add (loop->basic_ind_var,
                                             op->dest[0]->value.r);
            }
        }
      L_find_all_ind_var_op (loop, loop_cb, num_cb);
      L_move_basic_ind_var_to_src1 (loop, loop_cb, num_cb);
    }
  return;
}

/*
 *      If oper is a constant this is trivially true.
 *      Otherwise, oper is invariant if there are no defs of it in the loop.
 */
int
L_is_loop_inv_operand (L_Loop * loop, int *loop_cb, int num_cb,
                       L_Operand * operand)
{
  int i, j;
  L_Cb *cb;
  L_Oper *op;
  if (!operand || L_is_constant (operand))
    return 1;

  /* This check is required to keep from
   *  moving macro return regs from JSRs
   *  out of the loop.
   */

  // Commented by Amir, see below for details.
  //if (L_is_fragile_macro(operand) 
  //  return 0;

  for (i = 0; i < num_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);
      for (op = cb->first_op; op != NULL; op = op->next_op)
        {
          for (j = 0; j < L_max_dest_operand; j++)
            {
              if (L_same_operand (op->dest[j], operand))
                return 0;
            }
          // Amir 08/06: I commented the check above this loop and added this 
          //             check. This is a more accurate check, because macro operands are ok if 
          //             there is no function call in the loop.
          if (L_is_fragile_macro(operand) && L_general_subroutine_call_opcode (op))
          {
            return 0;
          }
        }
    }

  return 1;
}

/*
 *  unique_def = 1 or more defs of an operand but all defs are equivalent
 */
int
L_unique_def_in_loop (L_Loop * loop, int *loop_cb, int num_cb, L_Oper * oper)
{
  int i, j, k, match;
  L_Operand *dest;
  L_Cb *cb;
  L_Oper *ptr;
  for (j = 0; j < L_max_dest_operand; j++)
    {
      if (!(dest = oper->dest[j]))
        continue;

      for (i = 0; i < num_cb; i++)
        {
          cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);
          for (ptr = cb->first_op; ptr != NULL; ptr = ptr->next_op)
            {
              if (ptr == oper)
                continue;
              match = 0;
              for (k = 0; k < L_max_dest_operand; k++)
                {
                  if (L_same_operand (dest, ptr->dest[k]))
                    {
                      match = 1;
                      break;
                    }
                }
              if (!match)
                continue;
              if (!L_same_operation (oper, ptr, USE_FS))
		return 0;
            }
        }
    }
  return 1;
}

void
L_find_all_ind_var_op (L_Loop * loop, int *loop_cb, int num_cb)
{
  int i;
  L_Cb *cb;
  L_Oper *oper;
  for (i = 0; i < num_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
        {
          if (!L_is_register (oper->dest[0]))
            continue;
          if (Set_in (loop->basic_ind_var, oper->dest[0]->value.r))
	    loop->basic_ind_var_op =
	      Set_add (loop->basic_ind_var_op, oper->id);
        }
    }
}

void
L_move_basic_ind_var_to_src1 (L_Loop * loop, int *loop_cb, int num_cb)
{
  int i, flag;
  L_Cb *cb;
  L_Oper *oper;
  L_Operand *temp;
  L_Loop *l;
  for (i = 0; i < num_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
        {
          if (L_is_register (oper->src[0]) &&
              Set_in (loop->basic_ind_var, oper->src[0]->value.r))
            continue;
          if (!(L_is_register (oper->src[1]) &&
                Set_in (loop->basic_ind_var, oper->src[1]->value.r)))
            continue;
          /* Check if src1 is basic_ind_var of an inner loop */
          flag = 0;
          if (L_is_register (oper->src[0]))
            {
              for (l = L_fn->first_loop; l != NULL; l = l->next_loop)
                {
                  if (!Set_in (loop->nested_loops, l->id))
                    continue;
                  if (Set_in (l->basic_ind_var, oper->src[0]->value.r))
                    {
                      flag = 1;
                      break;
                    }
                }
            }
          if (flag)
            continue;
          switch (oper->opc)
            {
            case Lop_BR:
            case Lop_BR_F:
              /*
               *  Swap src1 and src2, must also invert br condition
               */
              oper->com[1] = L_inverse_pred_completer (oper->com[1]);
              temp = oper->src[0];
              oper->src[0] = oper->src[1];
              oper->src[1] = temp;
              break;
            case Lop_ADD:
            case Lop_ADD_U:
            case Lop_MUL:
            case Lop_MUL_U:
              /*
               *  Swap src1 and src2
               */
              temp = oper->src[0];
              oper->src[0] = oper->src[1];
              oper->src[1] = temp;
              break;
            default:
              break;
            }
        }
    }
}

/*=========================================================================*/
/*
 *      Basic inner loop structure manipulation functions
 */
/*==========================================================================*/

L_Inner_Loop *
L_new_inner_loop (L_Cb * cb)
{
  L_Inner_Loop *l;

  l = (L_Inner_Loop *) L_alloc (L_alloc_inner_loop);

  L_fn->max_inner_loop_id++;
  l->id = L_fn->max_inner_loop_id;
  l->flags = 0;
  l->preheader = NULL;
  l->cb = cb;
  l->fall_thru = NULL;
  l->feedback_op = NULL;
  l->num_invocation = 0.0;
  l->weight = cb->weight;
  l->ave_iteration = 0.0;
  l->out_cb = NULL;
  l->basic_ind_var = NULL;
  l->basic_ind_var_op = NULL;
  l->ind_info = NULL;
  l->prev_inner_loop = NULL;
  l->next_inner_loop = NULL;

  return l;
}

void
L_delete_inner_loop (L_Func * fn, L_Inner_Loop * inner_loop)
{
  L_Inner_Loop *prev, *next;

  /* Handle case of null ptr */
  if (inner_loop == NULL)
    return;

  /* disconnect inner_loop from doubly linked list */
  prev = inner_loop->prev_inner_loop;
  next = inner_loop->next_inner_loop;
  if (prev != NULL)
    prev->next_inner_loop = next;
  if (next != NULL)
    next->prev_inner_loop = prev;
  inner_loop->prev_inner_loop = NULL;
  inner_loop->next_inner_loop = NULL;

  /* fix up first_inner_loop ptrs if necessary */
  if (fn->first_inner_loop == inner_loop)
    fn->first_inner_loop = next;

  /* free sets if necessary */
  if (inner_loop->out_cb != NULL)
    Set_dispose (inner_loop->out_cb);
  if (inner_loop->basic_ind_var != NULL)
    Set_dispose (inner_loop->basic_ind_var);
  if (inner_loop->basic_ind_var_op != NULL)
    Set_dispose (inner_loop->basic_ind_var_op);

  L_delete_all_ind_info (&(inner_loop->ind_info));

  /* free the inner loop */
  L_free (L_alloc_inner_loop, inner_loop);
}

void
L_delete_all_inner_loop (L_Inner_Loop * list)
{
  L_Inner_Loop *ptr, *next;

  for (ptr = list; ptr != NULL; ptr = next)
    {
      next = ptr->next_inner_loop;
      ptr->prev_inner_loop = NULL;
      ptr->next_inner_loop = NULL;

      if (ptr->out_cb != NULL)
        Set_dispose (ptr->out_cb);
      if (ptr->basic_ind_var != NULL)
        Set_dispose (ptr->basic_ind_var);
      if (ptr->basic_ind_var_op != NULL)
        Set_dispose (ptr->basic_ind_var_op);

      L_delete_all_ind_info (&(ptr->ind_info));

      L_free (L_alloc_inner_loop, ptr);
    }
}

L_Inner_Loop *
L_concat_inner_loop (L_Inner_Loop * l1, L_Inner_Loop * l2)
{
  L_Inner_Loop *ptr;

  if (l1 == NULL)
    return l2;
  if (l2 == NULL)
    return l1;

  ptr = l1;
  while (ptr->next_inner_loop != NULL)
    {
      ptr = ptr->next_inner_loop;
    }
  l2->prev_inner_loop = ptr;
  ptr->next_inner_loop = l2;

  return l1;
}

/*=========================================================================*/
/*
 *      Inner loop detection functions
 */
/*==========================================================================*/

void
L_print_inner_loop_data (L_Func * fn)
{
  L_Inner_Loop *l;
  fprintf (stderr, "### Inner loop information (fn %s) ###\n", fn->name);
  for (l = fn->first_inner_loop; l != NULL; l = l->next_inner_loop)
    {
      fprintf (stderr, "\tid              %d\n", l->id);
      if (l->preheader)
        fprintf (stderr, "\tpreheader       %d\n", l->preheader->id);
      else
        fprintf (stderr, "\tpreheader       NULL\n");
      fprintf (stderr, "\tcb              %d\n", l->cb->id);
      if (l->fall_thru != NULL)
        fprintf (stderr, "\tfall_thru       %d\n", l->fall_thru->id);
      else
        fprintf (stderr, "\tfall_thru       NULL\n");
      fprintf (stderr, "\tfeedback_op     %d\n", l->feedback_op->id);
      fprintf (stderr, "\tnum_invocation  %f\n", l->num_invocation);
      fprintf (stderr, "\tweight          %f\n", l->weight);
      fprintf (stderr, "\tave_iteration   %f\n", l->ave_iteration);
      fprintf (stderr, "\t");
      Set_print (stderr, "out_cb", l->out_cb);
      fprintf (stderr, "\n");
      fprintf (stderr, "\t");
      Set_print (stderr, "basic_ind_var", l->basic_ind_var);
      fprintf (stderr, "\n");
      fprintf (stderr, "\t");
      Set_print (stderr, "basic_ind_var_op", l->basic_ind_var_op);
      fprintf (stderr, "\n");
    }
}

static void
L_inner_loop_det (L_Func * fn)
{
  L_Inner_Loop *l;
  L_Oper *feedback_op;
  L_Flow *feedback_flow;
  L_Cb *cb;
  double back_edge_weight, ave_iter;
  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      if (cb->first_op == NULL) /* empty block */
        continue;
      feedback_op = cb->last_op;
      if (feedback_op == NULL)
        L_punt ("L_inner_loop_det: feedback_op is NULL");
      /* end with conditional branch */
      if (L_cond_branch_opcode (feedback_op))
        {
          if (feedback_op->src[2]->value.cb != cb)
            continue;
          feedback_flow = L_find_flow_for_branch (cb, feedback_op);
        }
      /* end with jump */
      else if (L_uncond_branch_opcode (feedback_op))
        {
          if (feedback_op->src[0]->value.cb != cb)
            {
              if (feedback_op->prev_op == NULL)
                continue;
              feedback_op = feedback_op->prev_op;
              if (L_cond_branch_opcode (feedback_op))
                {
                  if (feedback_op->src[2]->value.cb != cb)
                    continue;
                  feedback_flow = L_find_flow_for_branch (cb, feedback_op);
                }
              else
                {
                  continue;
                }
            }
          else
            {
              feedback_flow = L_find_flow_for_branch (cb, feedback_op);
            }
        }
      else
        {
          continue;
        }
      /*
       *      Inner loop detected, add entry into data structure.
       */
      back_edge_weight = feedback_flow->weight;
      if ((cb->weight - back_edge_weight) == 0.0)
        ave_iter = 0.0;
      else
        ave_iter = cb->weight / (cb->weight - back_edge_weight);
      l = L_new_inner_loop (cb);
      l->feedback_op = feedback_op;
      l->num_invocation = cb->weight - back_edge_weight;
      l->ave_iteration = ave_iter;
      fn->first_inner_loop = L_concat_inner_loop (fn->first_inner_loop, l);
    }
}

static void
L_find_inner_loop_out_cb (L_Func * fn)
{
  L_Inner_Loop *l;
  L_Cb *cb;
  L_Flow *dest;
  for (l = fn->first_inner_loop; l != NULL; l = l->next_inner_loop)
    {
      if (l->out_cb != NULL)
        l->out_cb = Set_dispose (l->out_cb);
      cb = l->cb;
      for (dest = cb->dest_flow; dest != NULL; dest = dest->next_flow)
        {
          if (dest->dst_cb == cb)
            continue;
          l->out_cb = Set_add (l->out_cb, dest->dst_cb->id);
        }
      /* find fall thru id of loop */
      if (l->feedback_op == cb->last_op)
        {
          if (L_has_fallthru_to_next_cb (cb))
            l->fall_thru = cb->next_cb;
          else
            l->fall_thru = NULL;
        }
      else
        {
          L_Oper *next;
          next = l->feedback_op->next_op;
          if ((cb->last_op != next) || (!L_uncond_branch_opcode (next)))
            L_punt ("L_find_out_cb: next must be jump");
          l->fall_thru = next->src[0]->value.cb;
        }
    }
}

static L_Cb *
L_create_inner_loop_preheader (L_Func * fn, L_Inner_Loop * loop)
{
  L_Cb *cb, *preheader_cb;
  L_Flow *src, *flow, *next, *new_flow;
  L_Oper *br;

  cb = loop->cb;
  preheader_cb = L_create_cb (loop->num_invocation);
  L_insert_cb_before (fn, cb, preheader_cb);
  for (src = cb->src_flow; src != NULL; src = next)
    {
      L_Cb *src_cb;
      next = src->next_flow;
      src_cb = src->src_cb;
      flow = L_find_matching_flow (src_cb->dest_flow, src);
      br = L_find_branch_for_flow (src_cb, flow);
      if (src_cb == cb)
        {
          if (br == NULL)
            L_punt ("L_create_inner_loop_preheader: cannot fallthru to self");
          /* normal case, br is feedback_op */
          if (br == loop->feedback_op)
            continue;
        }
      flow->dst_cb = preheader_cb;
      if (br != NULL)
        L_change_branch_dest (br, cb, preheader_cb);
      new_flow = L_new_flow (src->cc, src_cb, preheader_cb, src->weight);
      preheader_cb->src_flow =
        L_concat_flow (preheader_cb->src_flow, new_flow);
      cb->src_flow = L_delete_flow (cb->src_flow, src);
    }
  flow = L_new_flow (0, preheader_cb, cb, preheader_cb->weight);
  preheader_cb->dest_flow = L_concat_flow (preheader_cb->dest_flow, flow);
  flow = L_new_flow (0, preheader_cb, cb, preheader_cb->weight);
  cb->src_flow = L_concat_flow (cb->src_flow, flow);
  return preheader_cb;
}

static void
L_find_inner_loop_preheaders (L_Func * fn)
{
  int need_preheader;
  L_Inner_Loop *loop;
  L_Cb *cb, *prev_cb, *src_cb;
  L_Flow *src, *flow;
  L_Oper *br;

  for (loop = fn->first_inner_loop; loop != NULL;
       loop = loop->next_inner_loop)
    {
      need_preheader = 0;
      cb = loop->cb;
      prev_cb = cb->prev_cb;
      for (src = cb->src_flow; src != NULL; src = src->next_flow)
        {
          if (src->src_cb == prev_cb)
            continue;
          src_cb = src->src_cb;
          flow = L_find_matching_flow (src_cb->dest_flow, src);
          br = L_find_branch_for_flow (src_cb, flow);
          if (src_cb == cb)
            {
              if (br == NULL)
                L_punt
                  ("L_find_inner_loop_preheaders: cannot fallthru to self");
              /* normal case, br is feedback_op */
              if (br == loop->feedback_op)
                continue;
            }
          need_preheader = 1;
          break;
        }
      if (prev_cb == fn->first_cb)
        need_preheader = 1;
      else
        {
          for (src = cb->src_flow; src != NULL; src = src->next_flow)
            {
              if (src->src_cb == prev_cb)
                {
                  if (L_uncond_branch_opcode (src->src_cb->last_op))
                    {
                      need_preheader = 1;
                      break;
                    }
                  else
                    continue;
                }
            }
        }
      if ((!need_preheader) &&
          (prev_cb->dest_flow) &&
          (prev_cb->dest_flow->dst_cb == cb) &&
          (prev_cb->dest_flow->next_flow == NULL))
        loop->preheader = prev_cb;
      else
        loop->preheader = L_create_inner_loop_preheader (fn, loop);
    }
}

static void
L_mark_inner_loop_headers (L_Func * fn)
{
  L_Inner_Loop *loop;

  for (loop = fn->first_inner_loop; loop != NULL;
       loop = loop->next_inner_loop)
    {
      loop->cb->flags = L_SET_BIT_FLAG (loop->cb->flags, L_CB_LOOP_HEADER);
    }
}

void
L_inner_loop_detection (L_Func * fn, int find_preheaders)
{
  /* Assume when do loop_det throw away all old info */
  fn->max_inner_loop_id = 0;
  if (fn->first_inner_loop != NULL)
    {
      L_delete_all_inner_loop (fn->first_inner_loop);
      fn->first_inner_loop = NULL;
    }

  L_inner_loop_det (fn);
  L_find_inner_loop_out_cb (fn);
  if (find_preheaders)
    L_find_inner_loop_preheaders (fn);
  L_find_inner_loop_out_cb (fn);
  L_mark_inner_loop_headers (fn);
  if (L_debug_inner_loop)
    L_print_inner_loop_data (fn);
}

/*=========================================================================*/
/*
 *      Basic loop induction variable structure manipulation functions
 */
/*==========================================================================*/

L_Ind_Info *
L_new_ind_info (L_Operand * var, int valid_ind_var)
{
  L_Ind_Info *info;

  info = (L_Ind_Info *) L_alloc (L_alloc_ind_info);
  info->valid_ind_var = valid_ind_var;
  info->var = L_copy_operand (var);
  info->increment = NULL;
  info->valid_init_val = 0;
  info->coeff = 0;
  info->base = NULL;
  info->initop = NULL;
  info->offset = 0;
  info->prev_info = NULL;
  info->next_info = NULL;

  return info;
}

/*
 *      Operand fields just ptrs, so dont delallocate the L_Operands
 */
void
L_delete_ind_info (L_Ind_Info * info, L_Ind_Info ** loop_ind_info)
{
  L_Ind_Info *prev, *next;

  /* handle case of null ptr */
  if (!info)
    return;

  /* disconnect info from doubly linked list */
  prev = info->prev_info;
  next = info->next_info;
  if (prev)
    prev->next_info = next;
  if (next)
    next->prev_info = prev;
  info->prev_info = NULL;
  info->next_info = NULL;

  /* update loop->ind_info if info at head of list */
  if (*loop_ind_info == info)
    *loop_ind_info = next;

  /* free operands in info structure */
  L_delete_operand (info->var);
  L_delete_operand (info->increment);
  L_delete_operand (info->base);

  /* free the data structure */
  L_free (L_alloc_ind_info, info);
}

void
L_delete_all_invalid_ind_var (L_Ind_Info * ind_info)
{
  L_Ind_Info *ptr, *next;

  for (ptr = ind_info; ptr; ptr = next)
    {
      next = ptr->next_info;
      if (!ptr->valid_ind_var)
        L_delete_ind_info (ptr, &ind_info);
    }
}

void
L_delete_all_ind_info (L_Ind_Info ** loop_ind_info)
{
  L_Ind_Info *ptr, *next;

  for (ptr = *loop_ind_info; ptr; ptr = next)
    {
      next = ptr->next_info;
      ptr->prev_info = NULL;
      ptr->next_info = NULL;
      L_delete_operand (ptr->var);
      L_delete_operand (ptr->increment);
      L_delete_operand (ptr->base);
      L_free (L_alloc_ind_info, ptr);
    }

  *loop_ind_info = NULL;
}

L_Ind_Info *
L_concat_ind_info (L_Ind_Info * info1, L_Ind_Info * info2)
{
  if (!info1)
    {
      info1 = info2;
    }
  else if (info2)
    {
      L_Ind_Info *ptr = info1;

      while (ptr->next_info)
	ptr = ptr->next_info;

      info2->prev_info = ptr;
      ptr->next_info = info2;
    }

  return info1;
}

/*==========================================================================*/
/*
 *      Loop induction variable information functions
 */
/*==========================================================================*/

/*
 *      Find an entry for an operand, naive search since anticipate small
 *      number of ind vars per loop.  Match both var and init_val_flag.
 *      init_val_flag of 2 means match either.
 */
L_Ind_Info *
L_find_ind_info (L_Ind_Info * ind_info, L_Operand * var, int init_val_flag)
{
  L_Ind_Info *ptr;

  if (!var)
    return NULL;

  for (ptr = ind_info; ptr; ptr = ptr->next_info)
    if (L_same_operand (ptr->var, var))
      {
	if (init_val_flag == ptr->valid_init_val)
	  return ptr;
	else if (init_val_flag == 2)
	  return ptr;
	else
	  break;
      }

  return NULL;
}

void
L_invalidate_ind_var (L_Operand * var, L_Ind_Info * ind_info)
{
  L_Ind_Info *info;

  if ((info = L_find_ind_info (ind_info, var, 2)))
    info->valid_ind_var = 0;
}

void
L_invalidate_initial_val (L_Operand * var, L_Ind_Info * ind_info)
{
  L_Ind_Info *info;

  if ((info = L_find_ind_info (ind_info, var, 2)))
    info->valid_init_val = 0;
}

void
L_print_all_ind_info (L_Loop * loop, L_Ind_Info * ind_info)
{
  int i;
  L_Ind_Info *info;
  i = 0;
  fprintf (stderr, "=========================\n");
  fprintf (stderr, "Induction variable info for loop %d\n", loop->id);
  for (info = ind_info; info != NULL; info = info->next_info)
    {
      i++;
      fprintf (stderr, "ENTRY[%d]\n", i);
      fprintf (stderr, "\tvalid_ind_var = %d\n", info->valid_ind_var);
      fprintf (stderr, "\tvar = ");
      L_print_operand (stderr, info->var, 1);
      fprintf (stderr, "\n");
      fprintf (stderr, "\tincrement  = ");
      L_print_operand (stderr, info->increment, 1);
      fprintf (stderr, "\n");
      fprintf (stderr, "\tvalid_init_val = %d\n", info->valid_init_val);
      fprintf (stderr, "\tcoeff = %d\n", info->coeff);
      fprintf (stderr, "\tbase = ");
      L_print_operand (stderr, info->base, 1);
      fprintf (stderr, "\n");
      fprintf (stderr, "\toffset = %d\n", info->offset);
    }
  fprintf (stderr, "=========================\n");
}

void
L_find_initial_val_for_cb (L_Cb * cb, L_Ind_Info ** ind_info)
{
  int s1_const, s2_const, coeff, coeff1, coeff2, offset, offset1, offset2;
  L_Ind_Info *info, *info1 = NULL, *info2 = NULL;
  L_Oper *op;
  L_Operand *dest, *src1, *src2, *base, *base1, *base2, *old_base;

  for (op = cb->first_op; op; op = op->next_op)
    {
      if (!(dest = op->dest[0]))
        continue;

      src1 = op->src[0];
      src2 = op->src[1];

      if (!(s1_const = L_is_int_constant (src1)))
        info1 = L_find_ind_info (*ind_info, src1, 1);
      if (!(s2_const = L_is_int_constant (src2)))
        info2 = L_find_ind_info (*ind_info, src2, 1);

      coeff1 = coeff2 = 0;
      base1 = base2 = NULL;
      offset1 = offset2 = 0;
      if (op->pred[0])
	{
	  /* Partial writes are not safe initializers */
	  L_invalidate_initial_val (dest, *ind_info);
	  continue;
	}
      else if (L_is_opcode (Lop_MOV, op))
        {
	  ;
        }
      else if (L_int_add_opcode (op))
        {
          if (s2_const)
            {
              offset2 = (int) src2->value.i;
            }
          else if (!info2)
            {
              coeff2 = 1;
              base2 = src2;
            }
          else
            {
              coeff2 = info2->coeff;
              base2 = info2->base;
              offset2 = info2->offset;
            }
        }
      else if (L_int_sub_opcode (op))
        {
          if (s2_const)
            {
              offset2 = -((int) src2->value.i);
            }
          else if (!info2)
            {
              coeff2 = -1;
              base2 = src2;
            }
          else
            {
              coeff2 = -(info2->coeff);
              base2 = info2->base;
              offset2 = -(info2->offset);
            }
        }
      else if (L_int_mul_opcode (op))
        {
	  /* Second operand of multiply must be const */

          if (s2_const)
            {
              coeff2 = (int) src2->value.i;
            }
          else
            {
              L_invalidate_initial_val (dest, *ind_info);
	      continue;
            }
        }
      else if (L_is_opcode (Lop_LSL, op))
        {
	  /* Second operand of left shift must be const */

          if (s2_const)
            {
              coeff2 = C_pow2 ((int) src2->value.i);
            }
          else
            {
              L_invalidate_initial_val (dest, *ind_info);
	      continue;
            }
        }
      else
        {
          L_invalidate_initial_val (dest, *ind_info);
	  continue;
        }

      /* For any accepted opcode, the first operand is
       * treated in the same manner.
       */

      if (s1_const)
	{
	  offset1 = (int) src1->value.i;
	}
      else if (!info1)
	{
	  coeff1 = 1;
	  base1 = src1;
	}
      else
	{
	  coeff1 = info1->coeff;
	  base1 = info1->base;
	  offset1 = info1->offset;
	}

      /* Check for multiply by const */
      if (!base2 && (offset2 == 0) && (coeff2 != 0))
        {
          coeff = coeff2 * coeff1;
          base = base1;
          offset = coeff2 * offset1;
        }
      else if (L_same_operand (base1, base2))
        {
          coeff = coeff1 + coeff2;
          base = base1;
          offset = offset1 + offset2;
        }
      else if (!base1)
        {
          coeff = coeff2;
          base = base2;
          offset = offset1 + offset2;
        }
      else if (!base2)
        {
          coeff = coeff1;
          base = base1;
          offset = offset1 + offset2;
        }
      else
        {
          L_invalidate_initial_val (dest, *ind_info);
	  continue;
        }

      /* SAM, 1-97, do not allow something to be initialized by a
         function of itself */
      if (L_same_operand (dest, base))
        {
          L_invalidate_initial_val (dest, *ind_info);
	  continue;
        }

      /* create an entry for dest operand, this is not an ind var */

      if (!(info = L_find_ind_info (*ind_info, dest, 2)))
        {
          info = L_new_ind_info (dest, 0);
          *ind_info = L_concat_ind_info (*ind_info, info);
        }

      info->valid_init_val = 1;
      info->coeff = coeff;
      old_base = info->base;
      info->base = L_copy_operand (base);
      info->initop = op;
      info->offset = offset;
      if (old_base)
        L_delete_operand (old_base);
    }
  return;
}

void
L_find_all_ind_info (L_Loop * loop, int *loop_cb, int num_cb)
{
  int i;
  L_Cb *preheader, *cb;
  L_Oper *op;
  L_Operand *ind_var, *ind_inc;
  L_Ind_Info *info;

  /* delete previous structure */
  if (loop->ind_info)
    L_delete_all_ind_info (&(loop->ind_info));

  for (i = 0; i < num_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);
      for (op = cb->first_op; op; op = op->next_op)
        {
          if (!Set_in (loop->basic_ind_var_op, op->id))
            continue;
          if (!(L_int_add_opcode (op) || L_int_sub_opcode (op)))
            continue;
          ind_var = op->dest[0];
          ind_inc = op->src[1];
          
          if (!(info = L_find_ind_info (loop->ind_info, ind_var, 2)))
            {
              info = L_new_ind_info (ind_var, 1);
              loop->ind_info = L_concat_ind_info (loop->ind_info, info);
              info->increment = L_copy_operand (ind_inc);
            }
        }
    }

  /* If no ind vars just return */
  if (!loop->ind_info)
    return;

  /* Dont compute initial vals if jsr in preheader */
  preheader = loop->preheader;
  if (!L_no_danger_in_cb (preheader))
    return;

  /* Search both preheader and block before preheader if possible */
  if ((preheader->src_flow != NULL) &&
      (preheader->src_flow->next_flow == NULL) &&
      (L_no_danger_in_cb (preheader->src_flow->src_cb)))
    L_find_initial_val_for_cb (preheader->src_flow->src_cb,
                               &(loop->ind_info));
  L_find_initial_val_for_cb (preheader, &(loop->ind_info));
}

L_Operand *
L_find_basic_induction_increment (L_Operand * operand, L_Ind_Info * ind_info)
{

  L_Ind_Info *info;
  
  if (!(info = L_find_ind_info (ind_info, operand, 2)))
    L_punt ("L_find_basic_induction_increment: ind info not found");
  if (info->increment == NULL)
    L_punt ("L_find_basic_induction_increment: corrupt ind info");
  if (info->valid_ind_var == 0)
    L_punt ("L_find_basic_induction_increment: invalid info ptr");
  return (info->increment);
}

/*==========================================================================*/
/*
 *      Static weight computation functions
 */
/*==========================================================================*/

void
L_compute_static_cb_weight (L_Func * fn)
{
  int j, num_loop_cb, *loop_cb;
  L_Cb *cb;
  L_Loop *loop;
  /*
   *  Initialization
   */
  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    cb->weight2 = 1.0;
  /*
   *  Calculate static weight
   */
  loop_cb = (int *) malloc (sizeof (int) * fn->n_cb);
  for (loop = fn->first_loop; loop != NULL; loop = loop->next_loop)
    {
      num_loop_cb = Set_size (loop->loop_cb);
      Set_2array (loop->loop_cb, loop_cb);
      for (j = 0; j < num_loop_cb; j++)
        {
          cb = L_cb_hash_tbl_find (fn->cb_hash_tbl, loop_cb[j]);
          cb->weight2 *= L_static_loop_iter_count;
        }
    }
  free (loop_cb);
  if (L_debug_static_weight)
    {
      for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
	printf ("id = %4d     weight = %f\n", cb->id, cb->weight2);
    }
}


void
L_add_loop_structure_for_new_cb (L_Cb * new_cb, L_Cb * orig_cb)
{
  L_Loop *loop;

  new_cb->deepest_loop = orig_cb->deepest_loop;
  for (loop = new_cb->deepest_loop; loop != NULL; loop = loop->parent_loop)
    loop->loop_cb = Set_add (loop->loop_cb, new_cb->id);
}

/*==========================================================================*/
/*
 *      Other inner loop functions
 */
/*==========================================================================*/

/* check if oper is a simple unpredicated induction op.  If so, return 1. */
int
L_is_inner_loop_ind_op (L_Cb * cb, L_Oper * op)
{
  int is_ind_op = 0;
  L_Operand *dest;
  L_Oper *oper;
  int j;

  if (cb == NULL)
    L_punt ("L_is_inner_loop_ind_op: cb must not be NULL");

  if (op == NULL)
    L_punt ("L_is_inner_loop_ind_op: op must not be NULL");

  dest = op->dest[0];

  /* only check for add since Lcode converts induction subtracts to adds */
  if (!L_int_add_opcode (op))
    return 0;

  if (!L_is_register (dest))
    return 0;

  if (op->pred[0] != NULL)
    return 0;

  /*  check if a source == dest */
  if (L_same_operand (dest, op->src[0]))
    {
      /*  check if src[1] is loop invariant */
      if (L_is_constant (op->src[1]))
        {
          is_ind_op = 1;
        }
      else
        {
          for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
            {
              for (j = 0; j < L_max_dest_operand; j++)
                {
                  if (L_same_operand (oper->dest[j], op->src[1]))
                    return 0;
                }
            }
          is_ind_op = 1;
        }
    }
  else if (L_same_operand (dest, op->src[1]))
    {
      /*  check if src[0] is a loop invariant */
      if (L_is_constant (op->src[0]))
        {
          is_ind_op = 1;
        }
      else
        {
          for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
            {
              for (j = 0; j < L_max_dest_operand; j++)
                {
                  if (L_same_operand (oper->dest[j], op->src[0]))
                    return 0;
                }
            }
          is_ind_op = 1;
        }
    }
  else                          /* dest != any source */
    return 0;

  /* check if dest is unique definition in loop */
  for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
    {
      for (j = 0; j < L_max_dest_operand; j++)
	if ((L_same_operand (oper->dest[j], dest)) && (oper != op))
	  return 0;
    }

  return (is_ind_op);
}


/* Find simple unpredicated induction operation for loop iteration variable.
   If one does not exist, return NULL. */
L_Oper *
L_find_inner_loop_counter (L_Inner_Loop * loop)
{
  L_Cb *cb;
  L_Oper *oper;
  L_Oper *loop_back_br;
  int i;

  if (loop == NULL)
    L_punt ("L_find_inner_loop_counter: loop must not be NULL");

  cb = loop->cb;
  loop_back_br = loop->feedback_op;

  /* search for simple induction op */
  for (oper = loop_back_br; oper != NULL; oper = oper->prev_op)
    {
      if (L_is_inner_loop_ind_op (cb, oper))
        {
          /* is this induction variable used by loop back branch? */
          for (i = 0; i < L_max_src_operand; i++)
	    if (L_same_operand (oper->dest[0], loop_back_br->src[i]))
	      return (oper);
        }
    }

  return NULL;
}


/* Return 1 if loop back branch is a conditional branch based upon
   a loop counter (a single unpredicated induction operation). */
int
L_is_counted_inner_loop (L_Inner_Loop * loop)
{
  int counted_loop_flag = 0;
  L_Oper *loop_back_br;
  L_Oper *loop_incr_op;
  L_Cb *cb;
  L_Operand *bound_src;         /* loop bound */
  int j;
  L_Oper *ptr;

  if (!loop)
    L_punt ("L_is_counted_inner_loop: loop must not be NULL");

  cb = loop->cb;
  loop_back_br = loop->feedback_op;

  /* Verify that the loop back branch is conditional. */
  if (!L_cond_branch (loop_back_br))
    return 0;

  if ((loop_incr_op = L_find_inner_loop_counter (loop)))
    {
      if (L_same_operand (loop_incr_op->dest[0], loop_back_br->src[0]))
	bound_src = loop_back_br->src[1];
      else
	bound_src = loop_back_br->src[0];

      /* check if bound_src is loop invariant */
      counted_loop_flag = 1;

      if (!L_is_constant (bound_src))
        {
          /* check if written in loop body */
          for (ptr = cb->first_op; ptr; ptr = ptr->next_op)
            {
              for (j = 0; j < L_max_dest_operand; j++)
		if (L_same_operand (ptr->dest[j], bound_src))
		  counted_loop_flag = 0;
            }
        }
    }

  return (counted_loop_flag);
}

/* BCC - 2/8/99 */
int
L_is_oper_in_loop (L_Loop * loop, int *loop_cb, int num_cb, L_Oper * oper)
{
  int i;
  L_Cb *cb;
  L_Oper *op;

  for (i = 0; i < num_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);
      for (op = cb->first_op; op != NULL; op = op->next_op)
	if (op == oper)
	  return 1;
    }
  return 0;
}

#if 0
/* BCC - 2/8/99 */
int
L_is_mem_op_address_invariant_in_loop (L_Loop * loop, int *loop_cb,
                                       int num_cb, L_Oper * oper,
                                       char *acc_name)
{
  int i;
  /* BCC - dump code, but should work */
  char *operand_acc_name[100];
  int is_operand_invariant[100];
  char str_buffer[1024];

  for (i = 0; i < L_max_src_operand; i++)
    {
      operand_acc_name[i] = 0;
      is_operand_invariant[i] = 0;
      if (oper->src[i] == NULL)
        continue;
      if (L_is_loop_inv_operand (loop, loop_cb, num_cb, oper->src[i]))
        {
          is_operand_invariant[i] = 1;
          switch (oper->src[i].type)
            {
            case L_OPERAND_IMMED:
            case L_OPERAND_INT:
              sprintf (str_buffer, "%d", (int) oper->src[i]->value.i);
              operand_acc_name[i] = strdup (str_buffer);
              break;
            case L_OPERAND_OPERAND_LABEL:
              strcpy (str_buffer, oper->src[i]->value.l);
              operand_acc_name[i] = strdup (str_buffer);
              break;
            case L_OPERAND_REGISTER:
              sprintf (str_buffer, "r%d", oper->src[i]->value.r);
              operand_acc_name[i] = strdup (str_buffer);
              break;
            case L_OPERAND_RREGISTER:
              sprintf (str_buffer, "r%d", oper->src[i]->value.rr);
              operand_acc_name[i] = strdup (str_buffer);
              break;
            default:
              L_punt ("L_is_mem_op_address_invariant_in_loop: "
                      "unexpected address operand");
              break;
            }
        }
      else
        {

        }
    }
}
#endif
