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
 *      File :          l_controlflow.c
 *      Description :   control flow functions
 *      Creation Date : February 1993
 *      Author :        Scott Mahlke, Wen-mei Hwu
 *
 *==========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>

/*=======================================================================*/
/*
 *      Functions to reconstruct source control flow
 */
/*=======================================================================*/

void
L_clear_src_flow (L_Func * fn)
{
  L_Cb *cb;
  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      L_delete_all_flow (cb->src_flow);
      cb->src_flow = NULL;
    }
}

void
L_rebuild_src_flow (L_Func * fn)
{
  L_Flow *flow, *new_flow;
  L_Cb *cb, *dst_cb;

  /* release any source links that still exist */
  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      L_delete_all_flow (cb->src_flow);
      cb->src_flow = NULL;
    }

  /* construct src flow arcs from dest flow arcs */
  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (flow = cb->dest_flow; flow != NULL; flow = flow->next_flow)
        {
          dst_cb = flow->dst_cb;
          new_flow =
            L_new_flow (flow->cc, flow->src_cb, flow->dst_cb, flow->weight);
          dst_cb->src_flow = L_concat_flow (dst_cb->src_flow, new_flow);
        }
    }
}

/*=======================================================================*/
/*
 *      Functions to assign to coloring
 */
/*=======================================================================*/

void
L_remove_empty_cbs (L_Func * fn)
{
  L_Cb *cb_indx, *cb_fix, *next_cb_indx;
  L_Flow *flow_indx;
  L_Oper *branch;

  cb_indx = fn->first_cb;
  while (cb_indx)
    {
      next_cb_indx = cb_indx->next_cb;
      if (!cb_indx->first_op)
        {
          for (cb_fix = fn->first_cb; cb_fix; cb_fix = cb_fix->next_cb)
            {
              for (flow_indx = cb_fix->dest_flow;
                   flow_indx; flow_indx = flow_indx->next_flow)
                {
                  if (flow_indx->dst_cb == cb_indx)
                    {
                      flow_indx->dst_cb = cb_indx->next_cb;

                      branch = L_find_branch_for_flow (cb_fix, flow_indx);
                      /* Assume that the flow is fall thru */
                      if (branch == NULL)
                        {
                          flow_indx->cc = 0;
                          continue;
                        }
                      L_change_branch_dest (branch, cb_indx,
                                            cb_indx->next_cb);
                    }
                }
            }

          L_delete_cb (fn, cb_indx);
        }
      cb_indx = next_cb_indx;
    }
  L_clear_src_flow (fn);
  L_rebuild_src_flow (fn);
  return;
}

static int
L_is_not_jump (L_Oper * oper)
{
  /* anything which is not always taken */
  if (oper == NULL)
    return 1;
  switch (oper->opc)
    {
    case Lop_RTS:
    case Lop_RTS_FS:
    case Lop_JUMP_RG:
    case Lop_JUMP_RG_FS:
    case Lop_JUMP:
    case Lop_JUMP_FS:
      return 0;
    default:
      return 1;
    }
}

static void
L_realloc_color_arrays ()
{
  int i, tmp_n_entries_allocated;
  L_Oper **tmp_cnt_oper;
  L_Flow **tmp_cnt_flow;

  /* allocate new larger arrays */
  tmp_n_entries_allocated = L_n_color_entries_alloc * 2;
  tmp_cnt_oper =
    (L_Oper **) malloc (sizeof (L_Oper *) * tmp_n_entries_allocated);
  tmp_cnt_flow =
    (L_Flow **) malloc (sizeof (L_Flow *) * tmp_n_entries_allocated);

  /* copy values old arrays into new arrays */
  for (i = 0; i < L_n_color_entries_alloc; i++)
    {
      tmp_cnt_oper[i] = L_cnt_oper[i];
      tmp_cnt_flow[i] = L_cnt_flow[i];
    }

  /* free up old arrays */
  free (L_cnt_oper);
  free (L_cnt_flow);

  /* assign global vars to new arrays */
  L_cnt_oper = tmp_cnt_oper;
  L_cnt_flow = tmp_cnt_flow;
  L_n_color_entries_alloc = tmp_n_entries_allocated;
}

void
L_print_color_info (L_Cb * cb)
{
  int i;
  printf ("## Color info (cb %d) ##\n", cb->id);
  printf ("\t L_n_cnt_oper %d\n", L_n_cnt_oper);
  for (i = 0; i < L_n_cnt_oper; i++)
    {
      if (L_cnt_oper[i] != NULL)
        {
          printf ("\t exit %d op %d cc %d target %d weight %f\n",
                  i + 1,
                  L_cnt_oper[i]->id,
                  L_cnt_flow[i]->cc,
                  L_cnt_flow[i]->dst_cb->id, L_cnt_flow[i]->weight);
        }
      else
        {
          printf ("\t exit %d op fthru cc %d target %d weight %f\n",
                  i + 1,
                  L_cnt_flow[i]->cc,
                  L_cnt_flow[i]->dst_cb->id, L_cnt_flow[i]->weight);
        }
    }
}

void
L_color_cb (L_Cb * cb)
{
  int i, color;
  L_Oper *oper;
  L_Flow *flow;
  if ((L_cnt_oper == NULL) || (L_cnt_flow == NULL))
    {
      if (L_cnt_oper != NULL)
        L_punt ("L_color_cb: L_cnt_oper not freed");
      if (L_cnt_flow != NULL)
        L_punt ("L_color_cb: L_cnt_flow not freed");
      L_n_color_entries_alloc = 128;
      L_cnt_oper =
        (L_Oper **) malloc (sizeof (L_Oper *) * L_n_color_entries_alloc);
      L_cnt_flow =
        (L_Flow **) malloc (sizeof (L_Flow *) * L_n_color_entries_alloc);
    }
  /*
   *  define operations.
   */
  color = 0;
  flow = cb->dest_flow;
  for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
    {
      if (L_is_control_oper (oper))
        {
          if (color >= L_n_color_entries_alloc)
            L_realloc_color_arrays ();
          L_cnt_oper[color] = oper;
          if (flow == NULL)
            {
              fprintf (stderr, "# cb %d\n", cb->id);
              L_punt ("L_color_cb: too few flows");
            }
          L_cnt_flow[color] = flow;
          flow = flow->next_flow;
          color += 1;
        }
    }

  /* handle fall thru path */
  if (L_is_not_jump (cb->last_op))
    {
      if (color >= L_n_color_entries_alloc)
        L_realloc_color_arrays ();
      L_cnt_oper[color] = NULL;
      L_cnt_flow[color] = flow;
      if (flow == NULL)
        {
          fprintf (stderr, "# cb %d\n", cb->id);
          L_punt ("L_color_cb: missing fall-thru arc");
        }
      flow = flow->next_flow;
      color += 1;
    }
  L_n_cnt_oper = color;
  /*
   *  Check flow information.
   */
  for (i = 0; i < L_n_cnt_oper; i++)
    {
      L_Oper *oper;
      int opc;
      L_Operand *src;
      L_Flow *flow;
      oper = L_cnt_oper[i];
      flow = L_cnt_flow[i];
      if (oper == NULL)
        continue;
      opc = oper->opc;
      if ((opc == Lop_JUMP_RG) || (opc == Lop_JUMP_RG_FS))
        {
          /*
           *  1. JUMP_RG must be the last operation.
           */
          if (i != (L_n_cnt_oper - 1))
            {
              fprintf (stderr, "# cb %d\n", cb->id);
              L_punt ("L_color_cb: JUMP_RG must be the last operation");
            }
        }
      else
        {
          /*
           *  2. check flow information.
           */
          switch (opc)
            {
            case Lop_JUMP:
            case Lop_JUMP_FS:
              src = oper->src[0];
              if (!L_is_cb (src))
                {
                  fprintf (stderr, "# cb %d oper %d\n", cb->id, oper->id);
                  L_punt ("L_color_cb: 1st operand of JUMP must be cb");
                }
              if (src->value.cb != flow->dst_cb)
                {
                  fprintf (stderr, "# cb %d, oper %d\n", cb->id, oper->id);
                  fprintf (stderr, "# jump target = cb %d\n",
                           src->value.cb->id);
                  fprintf (stderr, "# flow = cb %d\n", flow->dst_cb->id);
                  L_punt ("L_color_cb: bad flow for JUMP");
                }
              break;
            case Lop_BR:
            case Lop_BR_F:
              src = oper->src[2];
              if (!L_is_cb (src))
                {
                  fprintf (stderr, "# cb %d oper %d\n", cb->id, oper->id);
                  L_punt ("L_color_cb: 3rd operand of BRANCH must be cb");
                }
              if (src->value.cb != flow->dst_cb)
                {
                  fprintf (stderr, "# cb %d, oper %d\n", cb->id, oper->id);
                  fprintf (stderr, "# br target = cb %d\n",
                           src->value.cb->id);
                  fprintf (stderr, "# flow = cb %d\n", flow->dst_cb->id);
                  L_punt ("L_color_cb: bad flow (cond br)");
                }
              break;
            case Lop_CHECK:
            case Lop_CHECK_ALAT:
              src = oper->src[1];
              if (!L_is_cb (src))
                {
                  fprintf (stderr, "# cb %d oper %d\n", cb->id, oper->id);
                  L_punt ("L_color_cb: 2nd operand of CHECK must be cb");
                }
              if (src->value.cb != flow->dst_cb)
                {
                  fprintf (stderr, "# cb %d, oper %d\n", cb->id, oper->id);
                  fprintf (stderr, "# check target = cb %d\n",
                           src->value.cb->id);
                  fprintf (stderr, "# flow = cb %d\n", flow->dst_cb->id);
                  L_punt ("L_color_cb: bad flow (check)");
                }
              break;
            default:
              L_punt ("L_color_cb: illegal control operation");
            }
        }
    }
  if (L_debug_color_cb)
    {
      L_print_color_info (cb);
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
L_visit (L_Cb * cb)
{
  L_Flow *out;
  if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_VISITED))
    return;
  cb->flags = L_SET_BIT_FLAG (cb->flags, L_CB_VISITED);
  for (out = cb->dest_flow; out != NULL; out = out->next_flow)
    L_visit (out->dst_cb);
}

static int
L_delete_dead_blocks (L_Func * fn)
{
  int change;
  L_Cb *cb, *next_cb;
  change = 0;

  for (cb = fn->first_cb; cb != NULL; cb = next_cb)
    {
      next_cb = cb->next_cb;
      if (!L_EXTRACT_BIT_VAL (cb->flags, L_CB_IS_DEAD))
        continue;
      L_delete_cb (fn, cb);
      change++;
    }
  return change;
}

int
L_delete_unreachable_blocks (L_Func * fn)
{
  int change;
  L_Cb *cb, *dst_cb;
  L_Flow *flow, *next_flow, *flow2;
  change = 0;

  /*
   *  visit each block.
   */
  L_reset_visited_flag (fn);
  L_visit (fn->first_cb);

  /*
   *    Mark all non-visited cbs as dead
   */
  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      if (!L_EXTRACT_BIT_VAL (cb->flags, L_CB_VISITED))
        {
          cb->flags = L_SET_BIT_FLAG (cb->flags, L_CB_IS_DEAD);
          /* disconnect all flow arcs with dests of this block */
          for (flow = cb->dest_flow; flow != NULL; flow = next_flow)
            {
              next_flow = flow->next_flow;
              dst_cb = flow->dst_cb;
              flow2 = L_find_matching_flow (dst_cb->src_flow, flow);
              dst_cb->src_flow = L_delete_flow (dst_cb->src_flow, flow2);
              cb->dest_flow = L_delete_flow (cb->dest_flow, flow);
            }
        }
      else
        {
          cb->flags = L_CLR_BIT_FLAG (cb->flags, L_CB_IS_DEAD);
        }
    }

  /*
   *  remove blocks marked as dead from control flow graph
   */
  change = L_delete_dead_blocks (fn);

  return change;
}



