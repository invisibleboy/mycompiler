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
 *      File :          l_dominator.c
 *      Description :   Dominator Analysis
 *      Creation Date : September 1997
 *      Author :        David August, Wen-mei Hwu
 *
 *==========================================================================*/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>


void
L_dominator_analysis (L_Func * fn, int mode)
{
  if (mode & (DOMINATOR | DOMINATOR_CB | DOMINATOR_INT))
    L_compute_dominator (fn);

  if (mode & (POST_DOMINATOR | POST_DOMINATOR_CB | POST_DOMINATOR_INT))
    L_compute_post_dominator (fn);

  return;
}


void
L_compute_dominator (L_Func * fn)
{
  L_Cb *cb;
  L_Cb *pred_cb;
  L_Flow *flow;
  Set all = NULL;
  int change;

  /*
   *    Initialization
   */

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      if (cb->dom)
        cb->dom = Set_dispose (cb->dom);
      all = Set_add (all, cb->id);
    }

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      cb->dom = cb->src_flow ? 
	Set_union (NULL, all) :
	Set_add (NULL, cb->id);
    }

  /*
   *    Compute dominance relationship
   */

  do
    {
      change = 0;
      for (cb = fn->first_cb; cb; cb = cb->next_cb)
        {
	  Set new_dom = NULL;

          for (flow = cb->src_flow; flow; flow = flow->next_flow)
            {
              pred_cb = flow->src_cb;
	      new_dom = new_dom ? 
		Set_intersect_acc (new_dom, pred_cb->dom) :
		Set_union (new_dom, pred_cb->dom);
            }

          new_dom = Set_add (new_dom, cb->id);

          if (!Set_same (new_dom, cb->dom))
            {
              change++;
              cb->dom = Set_dispose (cb->dom);
              cb->dom = new_dom;
            }
          else
	    {
	      Set_dispose (new_dom);
	    }
        }
    }
  while (change);

  all = Set_dispose (all);
}


void
L_compute_post_dominator (L_Func * fn)
{
  L_Cb *cb;
  L_Cb *succ_cb;
  L_Flow *flow;
  Set all;
  Set new_pdom;
  int change;

  /*
   *    Initialization
   */
  all = NULL;
  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      if (cb->pdom)
        cb->pdom = Set_dispose (cb->pdom);
      all = Set_add (all, cb->id);
    }

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      if (cb->dest_flow)
        cb->pdom = Set_union (0, all);
      else
        cb->pdom = Set_add (NULL, cb->id);
    }

  /*
   *    Compute post-dominance relationship
   */
  change = 1;
  while (change)
    {
      change = 0;
      for (cb = fn->first_cb; cb; cb = cb->next_cb)
        {
          new_pdom = NULL;

          for (flow = cb->dest_flow; flow; flow = flow->next_flow)
            {
              succ_cb = flow->dst_cb;
              if (new_pdom)
                new_pdom = Set_intersect_acc (new_pdom, succ_cb->pdom);
              else
                new_pdom = Set_union (new_pdom, succ_cb->pdom);
            }
          new_pdom = Set_add (new_pdom, cb->id);
          if (!Set_same (new_pdom, cb->pdom))
            {
              change += 1;
              cb->pdom = Set_dispose (cb->pdom);
              cb->pdom = new_pdom;
            }
	  else
	    {
	      Set_dispose (new_pdom);
	    }
        }
    }
  all = Set_dispose (all);
}


/*========================================================================
 *
 * Dominator/Post Dominator Analysis Information
 *
 *========================================================================*/


int
L_in_cb_DOM_set (L_Cb * cb, int check_cb_id)
{
  return (Set_in (cb->dom, check_cb_id));
}

int
L_in_cb_PDOM_set (L_Cb * cb, int check_cb_id)
{
  return (Set_in (cb->pdom, check_cb_id));
}

Set
L_get_cb_DOM_set (L_Cb * cb)
{
  return (cb->dom);
}

Set
L_get_cb_PDOM_set (L_Cb * cb)
{
  return (cb->pdom);
}
