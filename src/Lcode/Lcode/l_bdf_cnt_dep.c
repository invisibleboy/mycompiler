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

#include <config.h>
#include <Lcode/l_main.h>
#include "l_bdf_graph.h"
#include "l_bdf_cnt_dep.h"

/*
 * Dominator / Postdominator
 * ----------------------------------------------------------------------
 *
 */

void
BDF_flow_dom_pdom (BDF_Graph *g, int mode)
{
  BDF_Node *cb;
  int change;

  BDF_FOREACH_CB (cb, g->cb)
    {
      if (mode & (DOMINATOR | DOMINATOR_CB | DOMINATOR_INT))
	{
	  if (cb->info.dom)
	    cb->info.dom = Set_dispose (cb->info.dom);
	  if (cb->flags & BDF_NODE_START)
	    cb->info.dom = Set_add (NULL, cb->id);
	  else
	    cb->info.dom = Set_copy (g->node_U);
	}

      if (mode & (POST_DOMINATOR | POST_DOMINATOR_CB | POST_DOMINATOR_INT))
	{
	  if (cb->info.pdom)
	    cb->info.pdom = Set_dispose (cb->info.pdom);
	  if (cb->flags & BDF_NODE_STOP)
	    cb->info.pdom = Set_add (NULL, cb->id);
	  else
	    cb->info.pdom = Set_copy (g->node_U);
	}
    }

  /* GLOBAL DOMINATOR */

  if (mode & (DOMINATOR | DOMINATOR_CB | DOMINATOR_INT))
    {
      do
	{
	  change = 0;
	  BDF_FOREACH_CB (cb, g->cb)
	    {
	      BDF_Arc *pred;
	      Set tmp;

	      tmp = Set_copy (cb->info.dom);
	      
	      BDF_FOREACH_ARC (pred, cb->pred)
		{
		  BDF_Node *pcb = pred->pred;
		  tmp = Set_intersect_acc (tmp, pcb->info.dom);
		}
	      
	      tmp = Set_add(tmp, cb->id);
	      
	      if (!Set_same (tmp, cb->info.dom))
		{
		  change++;
		  Set_dispose (cb->info.dom);
		  cb->info.dom = tmp;
		}
	      else
		{
		  Set_dispose (tmp);
		}
	    }
	}
      while (change);
    }

  /* GLOBAL POST-DOMINATOR */

  if (mode & (POST_DOMINATOR | POST_DOMINATOR_CB | POST_DOMINATOR_INT))
    {
      do
	{
	  change = 0;
	  BDF_FOREACH_CB (cb, g->cb)
	    {
	      BDF_Arc *succ;
	      Set tmp;

	      tmp = Set_copy (cb->info.dom);
	      
	      BDF_FOREACH_ARC (succ, cb->exit)
		{
		  BDF_Node *scb = succ->succ;
		  tmp = Set_intersect_acc (tmp, scb->info.pdom);
		}
	      
	      tmp = Set_add(tmp, cb->id);
	      
	      if (!Set_same (tmp, cb->info.pdom))
		{
		  change++;
		  Set_dispose (cb->info.pdom);
		  cb->info.pdom = tmp;
		}
	      else
		{
		  Set_dispose (tmp);
		}
	    }
	}
      while (change);
    }
  
  return;
}

