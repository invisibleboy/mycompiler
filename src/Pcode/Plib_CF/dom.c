/*****************************************************************************\
 *
 *                    Illinois Open Source License
 *                     University of Illinois/NCSA
 *                         Open Source License
 *
 * Copyright (c) 2004, The University of Illinois at Urbana-Champaign.
 * All rights reserved.
 *
 * Developed by:
 *
 *              IMPACT Research Group
 *
 *              University of Illinois at Urbana-Champaign
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
#include <string.h>
#include <library/set.h>
#include <library/stack.h>
#include <library/llist.h>
#include <Pcode/pcode.h>
#include <Pcode/query.h>
#include <Pcode/struct.h>
#include <Pcode/parms.h>
#include <Pcode/reduce.h>
#include <Pcode/symtab_i.h>
#include <Pcode/parloop.h>

#include "cfg.h"
#include "dom.h"
#include "loop.h"


void PC_InitDomSets (PC_Graph PC_cfg);
void PC_ComputeDomSets (PC_Graph PC_cfg);
void PC_BuildDomSets (PC_Graph PC_cfg);
bool PC_BB1DominatesBB2 (PC_Block bb1, PC_Block bb2);
static void PC_BuildDoms (PC_Graph cfg);



/*! \brief Initializes dominator sets of all PC_Blocks to be the universal set 
 *   of all PC_Blocks in the CFG. The entry block is dominated only by itself.
 *
 * \param cfg
 *  control flow graph for the func being processed
 *
 * \return void
 */
void
PC_InitDomSets (PC_Graph cfg)
{
  PC_Block bb = NULL, f_bb = cfg->first_bb;
  Set univ = NULL;

  if (f_bb == NULL || f_bb->next == NULL)
    P_punt ("PC_InitDomSets: f_bb or f_bb->next is NULL.");

  /* func entry bb dominates itself, by definition. */
  f_bb->doms = Set_add (NULL, f_bb->ID);

  /* Build a universal set of all bb IDs. */
  for (bb = f_bb; bb != NULL; bb = bb->next)
    univ = Set_add (univ, bb->ID);

  /*Initialize all non-func-entry bbs to be dominated by all bbs in PC_Graph. */
  for (bb = f_bb->next; bb != NULL; bb = bb->next)
    bb->doms = Set_copy (univ);

  Set_dispose (univ);
}


/*! \brief Computes dominator sets using algo in Dragon book.
 *
 * \param cfg
 *  control flow graph for the func being processed
 *
 * \return void
 *
 * \sa cfg.c : PC_BuildUnReachableBBSet ()
 */
void
PC_ComputeDomSets (PC_Graph cfg)
{
  PC_Block bb = NULL;
  PC_Flow fl = NULL;
  Set oldDoms = NULL, iSet = NULL, uBBs = NULL;
  bool changed = FALSE;

  uBBs = PC_BuildUnReachableBBSet (cfg);

  /* Set the doms sets of all unreachable BBs to NULL. */
  for (bb = cfg->first_bb->next; bb; bb = bb->next)
    if (Set_in (uBBs, bb->ID))
      bb->doms = Set_dispose (bb->doms);

  /* Keep re-computing domintor sets until none change.  */
  do
    {
      changed = FALSE;
      /* compute the dominator set for each CFG bb (skip first bb). */
      for (bb = cfg->first_bb->next; bb; bb = bb->next)
	{
	  /* ignore any bbs that are unreachable */
	  if (Set_in (uBBs, bb->ID))
	    continue;

	  oldDoms = Set_copy (bb->doms); /* need this to compare later. */

	  iSet = NULL;
	  /* compute intersection of dom sets of all reachable preds of bb. */
	  for (fl = bb->p_flow; fl; fl = fl->p_next_flow)
	    if (!Set_in (uBBs, fl->src_bb->ID))
	      {
		if (!iSet)	/* first pred's dom set is copied over */
		  iSet = Set_copy (fl->src_bb->doms);
		else		/* all other preds' dom sets are intersected */
		  iSet = Set_intersect_acc (iSet, fl->src_bb->doms);
	      }

	  iSet = Set_add (iSet, bb->ID); /* add bb to its own dom set. */
	  Set_dispose (bb->doms);
	  bb->doms = iSet;               /* save this set in bb. */
	  if (!Set_same (oldDoms, bb->doms))
	    changed = TRUE;

	  Set_dispose (oldDoms);
	}

    }
  while (changed);

  Set_dispose (uBBs);
}


/*! \brief Checks if a PC_Block dominates another.
 *
 * \param bb1
 *  PC_Block that is a potential dominator
 * \param bb2
 *  PC_Block that is a potential dominated block
 *
 * \return TRUE if bb1 dominates bb2, otherwise FALSE
 */
bool
PC_BB1DominatesBB2 (PC_Block bb1, PC_Block bb2)
{
  return Set_in (bb2->doms, bb1->ID);
}


/*! \brief Initializes and computes dominator sets for each PC_Block in the CFG.
 *
 * \param cfg
 *  control flow graph for the func being processed
 *
 * \return void
 */
void
PC_BuildDomSets (PC_Graph cfg)
{
  /* we are going to switch over to a new implementation (JWP)
   * that is n log(n) instead of exponential.  if you have
   * problems, feel free to switch back to the old slow one */
#if 0
  PC_InitDomSets (cfg);
  PC_ComputeDomSets (cfg);
#else
  PC_BuildDoms (cfg);
#endif
}


static PC_Block 
PC_FindImmDom (PC_Graph cfg, PC_Block bb, int *buf)
{
  PC_Block dom_bb;
  int n_dom, i_dom, i, bb_id = bb->ID;
  
  if ((n_dom = Set_2array(bb->doms, buf)) == 1)
    {
      assert(buf[0] == bb->ID);
      return NULL;
    } 

  if (buf[0] != bb_id)
    i_dom = 0;
  else
    i_dom = 1;

  for (i = i_dom + 1 ; i < n_dom ; i++)
    {
      dom_bb = PC_FindBlock (cfg, buf[i]);
      if ((buf[i] != bb_id) && Set_in(dom_bb->doms, buf[i_dom]))
        i_dom = i;
    }

  return PC_FindBlock (cfg, buf[i_dom]);
}


/*! \brief Constructs dom sets, idom, and dom tree
 *
 * \param cfg
 *  control flow graph for the func being processed
 *
 * \return void
 */
void
PC_BuildDomTree (PC_Graph cfg)
{
  PC_Block bb, idom;
  int *buf;

  PC_BuildDomSets (cfg);

  if (!(buf = (int *) calloc (cfg->num_bbs, sizeof (int))))
    P_punt ("PC_BuildDomTree: unable to allocate buffer");

  for (bb = cfg->first_bb; bb; bb = bb->next)
    {
      idom = PC_FindImmDom (cfg, bb, buf);
      bb->idom = idom;
      if (idom)
	idom->dom_tree_children = 
	  List_insert_last (idom->dom_tree_children, (void *)bb);
    }

  free (buf);
  return;
}

/*
 * Assumption:
 *	- Immediate dominators available, FindImmediateDominatorTree().
 */
static void
PC_FindDomFrontier (PC_Block bb)
{
  PC_Flow succ_link;
  PC_Block succ_bb, child_bb, child_bb_dom_front;
  List children;

  children = bb->dom_tree_children;
  List_start (children);
  while ((child_bb = (PC_Block) List_next (children)))
    PC_FindDomFrontier (child_bb);

  bb->dom_frontier = NULL;

  for (succ_link = bb->s_flow; succ_link; succ_link = succ_link->s_next_flow)
    {
      if (!(succ_link->flags & PC_FL_NEVER_TAKEN))
        {
          succ_bb = succ_link->dest_bb;
          if (succ_bb->idom && (succ_bb->idom->ID != bb->ID))
	    bb->dom_frontier = List_insert_last (bb->dom_frontier, succ_bb);
        }
    }

  List_start (children);
  while ((child_bb = (PC_Block) List_next (children)))
   {
     List df = child_bb->dom_frontier;
     while ((child_bb_dom_front = (PC_Block) List_next (df)))
       {
          if (child_bb_dom_front->idom && 
	      (child_bb_dom_front->idom->ID != bb->ID))
            bb->dom_frontier =  List_insert_last (bb->dom_frontier,
						  child_bb_dom_front);
        }
   }

  return;
}


void 
PC_FindDominatorFrontier(PC_Graph cfg)
{
  PC_FindDomFrontier (cfg->first_bb);
}

/**********************************************
 * New DOM Implementation - JWP - 2004 
 ***********************************************/

typedef struct _DomCalc
{
  int semi;
  PC_Block parent;
  PC_Block ancestor;
  PC_Block label;
  List bucket;
}
_DomCalc, *DomCalc;

typedef struct _DomFrontCalc
{
  int treelvl;
  List jnodes;
}
_DomFrontCalc, *DomFrontCalc;

/* Macros used to access DomCalc fields
 * from the ext field of a PC_Block */
#define LABEL(a)    (((DomCalc)(a)->ext)->label)
#define SEMI(a)     (((DomCalc)(a)->ext)->semi)
#define ANCESTOR(a) (((DomCalc)(a)->ext)->ancestor)
#define BUCKET(a)   (((DomCalc)(a)->ext)->bucket)
#define PARENT(a)   (((DomCalc)(a)->ext)->parent)

/* Macros used to access DomFrontCalc fields
 * from the ext field of a PC_Block */
#define TREELVL(a)  (((DomFrontCalc)(a)->ext)->treelvl)
#define JNODES(a)   (((DomFrontCalc)(a)->ext)->jnodes)

#define SEMI_INIT    -1
#define SEMI_VISITED -2

static DomCalc PC_NewDomCalc (PC_Block parent);
static DomCalc PC_FreeDomCalc (DomCalc dc);
static DomFrontCalc PC_NewDomFrontCalc ();
static DomFrontCalc PC_FreeDomFrontCalc (DomFrontCalc dfc);

static PC_Block RemoveUnreachableBBs (PC_Graph cfg, PC_Block bb);


/*! \brief Performs a depth-first search on the CFG and retruns a list of
 *   PC_Blocks in reverse order of the DFS traversal.
 *
 *  \param cfg
 *   control flow graph for a prcedure to process
 */
static void
PC_DOM_dfs (PC_Graph cfg)
{
  int i = 0;
  PC_Block bb = NULL;
  Stack *bb_stack = New_Stack ();

  /* Push NULL onto the stack to indicate the bottom */
  Push_Top (bb_stack, NULL);
  Push_Top (bb_stack, cfg->first_bb);

  while ((bb = (PC_Block) Pop (bb_stack)))
    {
      PC_Flow succ;

      /* push unmarked successor blocks onto the stack */
      for (succ = bb->s_flow; succ; succ = succ->s_next_flow)
	if (SEMI (succ->dest_bb) == SEMI_INIT)
	  {
	    PARENT (succ->dest_bb) = bb;
	    Push_Top (bb_stack, succ->dest_bb);
	    SEMI (succ->dest_bb) = SEMI_VISITED; /* prevent duplicates */
	  }					 /* in stack */

      SEMI (bb) = i++;
    }
  
  Free_Stack (bb_stack);
}

static void
PC_DOM_compress (PC_Block v)
{
  if (ANCESTOR (v) != NULL && ANCESTOR (ANCESTOR (v)) != NULL)
    {
      PC_DOM_compress (ANCESTOR (v));

      if (SEMI (LABEL (ANCESTOR (v))) < SEMI (LABEL (v)))
	LABEL (v) = LABEL (ANCESTOR (v));

      ANCESTOR (v) = ANCESTOR (ANCESTOR (v));
    }

  return;
}


static PC_Block
PC_DOM_eval (PC_Block v)
{
  PC_Block ret = v;

  if (ANCESTOR (v) != NULL)
    {
      PC_DOM_compress (v);
      ret = LABEL (v);
    }

  return ret;
}


#  define PC_DOM_link(a,b) ANCESTOR(b) = a;


/*! \brief Uses a fast dominator algorithm to build the Dominator Tree for a
 *   procedure.  Then fills in other members of the PC_Blocks with dominator
 *   information.
 *   
 * \param cfg
 * control flow graph for a function to process
 */
static void
PC_BuildDoms (PC_Graph cfg)
{
  int i;
  PC_Block next_bb = NULL;
  PC_Block t = NULL;
  PC_Block u = NULL;
  PC_Block v = NULL;
  PC_Block w = NULL;
  PC_Block *vertex;

  /* Allocate and attach a DomCalc member to the ext field
   * of each PC_Block */
  for (t = cfg->first_bb; t; t = t->next)
    {
      if (t->ext)
	P_punt ("PC_BuildDoms: PC_Block ext field is not NULL.");
      t->ext = PC_NewDomCalc (t);
    }

  /* step 1 */
  PC_DOM_dfs (cfg);

  /* construct an array to index bb's by DFS number */
  vertex = (PC_Block *) alloca (sizeof (PC_Block) * cfg->num_bbs);
  for (t = cfg->first_bb; t != NULL; t = next_bb)
    {
      next_bb = t->next;
      
      if (SEMI (t) >= 0)
	vertex[SEMI (t)] = t;
      
      else /* Prune unreachable BBs */
	{
	  P_warn ("PC_BuildDoms: BB #%d in function \"%s\"\n"
		  "  has no predecessors.  Removing BB.",
		  t->ID, cfg->func->name);

	  /* Removing unreachable BBs may alter the list of BBs.  Get the next
	   * BB from the return value of RemoveUnreachableBBs() */
	  next_bb = RemoveUnreachableBBs (cfg, t);
	}
    }

  /* traverse the DFS nodes in reverse order */
  for (i = cfg->num_bbs - 1; i > 0; i--)
    {
      PC_Flow pred = NULL;
      w = vertex[i];

      /* step 2 */
      for (pred = w->p_flow; pred; pred = pred->p_next_flow)
	if (SEMI (pred->src_bb) >= 0)
	  {
	    u = PC_DOM_eval (pred->src_bb);
	    if (SEMI (u) < SEMI (w))
	      SEMI (w) = SEMI (u);
	  }

      {	/* add w to bucket(vertex(semi(w))) */
	PC_Block temp = vertex[SEMI (w)];
	BUCKET (temp) = List_insert_first (BUCKET (temp), w);
      }

      PC_DOM_link (PARENT (w), w);

      /* step 3 */
      {
	List w_par_buck = BUCKET (PARENT (w));
	for (v = List_first (w_par_buck); v; v = List_next (w_par_buck))
	  {
	    w_par_buck = List_delete_current (w_par_buck);
	    u = PC_DOM_eval (v);
	    v->idom = (SEMI (u) < SEMI (v)) ? u : PARENT (w);
	  }
	BUCKET (PARENT (w)) = w_par_buck;
      }
    }

  /* step 4 */
  for (i = 1; i > cfg->num_bbs; i++)
    {
      w = vertex[i];
      if (w->idom->ID != vertex[SEMI (w)]->ID)
	w->idom = w->idom->idom;
    }

  /* Clear out all dom_tree_children lists */
  for (w = cfg->first_bb; w; w = w->next)
    w->dom_tree_children = List_reset (w->dom_tree_children);

  /* Fill in the dom_tree_children lists */
  for (w = cfg->first_bb; w; w = w->next)
    if (w->idom)
      w->idom->dom_tree_children =
	List_insert_first (w->idom->dom_tree_children, w);

  /* Fill in the doms sets */
  for (w = cfg->first_bb; w; w = w->next)
    {
      if (w->doms == NULL)
	w->doms = Set_new ();
      else
	w->doms = Set_dispose (w->doms);

      for (u = w; u; u = u->idom)
	w->doms = Set_add (w->doms, u->ID);
    }

  /* Free all the DomCalc structs from the ext field of each PC_Block */
  for (t = cfg->first_bb; t; t = t->next)
    t->ext = PC_FreeDomCalc ((DomCalc) t->ext);
}

/*! \brief Uses a fast dominator algorithm to build the Dominator Tree for a
 *   procedure.  Then fills in other members of the PC_Blocks with dominator
 *   information.
 *   
 * \param cfg
 * control flow graph for a function to process
 */
void
PC_BuildDomFront (PC_Graph cfg)
{
  int i = 0;
  List curr = NULL;
  List next = NULL;
  PC_Block bb = NULL;

  /* Allocate and attach a DomFrontCalc member to the ext field
   * of each PC_Block */
  for (bb = cfg->first_bb; bb; bb = bb->next)
    {
      if (bb->ext)
	P_punt ("PC_BuildDomFront: PC_Block ext field is not NULL.");
      bb->ext = PC_NewDomFrontCalc ();
    }

  /* Number the levels of the Dom Tree */
  for (curr = List_insert_first (curr, cfg->first_bb); curr; curr = next)
    {
      PC_Block t = NULL;
      next = NULL;
      for (t = List_first (curr); t; t = List_next (curr))
	{
	  next = List_append (next, List_copy (t->dom_tree_children));
	  TREELVL (t) = i;
	}
      curr = List_reset (curr);
      i++;
    }

  /* Build up Join Edges */
  for (bb = cfg->first_bb; bb; bb = bb->next)
    {
      PC_Flow fl = NULL;
      for (fl = bb->s_flow; fl; fl = fl->s_next_flow)
	{
	  if (TREELVL (bb) >= TREELVL (fl->dest_bb) ||
	      bb->ID == fl->dest_bb->ID ||
	      !PC_BB1DominatesBB2 (bb, fl->dest_bb))
	    JNODES (bb) = List_insert_first (JNODES (bb), fl->dest_bb);
	}
    }

  /* Compute the Dominance Frontier */
  for (bb = cfg->first_bb; bb; bb = bb->next)
    {
      List curr = NULL;
      List next = NULL;

      bb->dom_frontier = List_reset (bb->dom_frontier);
      for (curr = List_insert_first (curr, bb); curr; curr = next)
	{
	  PC_Block t = NULL;
	  next = NULL;

	  for (t = (PC_Block) List_first (curr); t; t = List_next (curr))
	    {
	      PC_Block j = NULL;
	      next = List_append (next, List_copy (t->dom_tree_children));

	      for (j = (PC_Block) List_first (JNODES (t)); j;
		   j = (PC_Block) List_next (JNODES (t)))
		if (TREELVL (j) <= TREELVL (bb))
		  bb->dom_frontier = List_insert_first (bb->dom_frontier, j);
	    }
	  curr = List_reset (curr);
	}
    }

  /* Free all the DomFrontCalc structs from the ext field of each PC_Block */
  for (bb = cfg->first_bb; bb; bb = bb->next)
    bb->ext = PC_FreeDomFrontCalc (bb->ext);
}

static DomCalc
PC_NewDomCalc (PC_Block parent)
{
  DomCalc dc = ALLOCATE (_DomCalc);

  dc->semi = SEMI_INIT;
  dc->parent = NULL;
  dc->ancestor = NULL;
  dc->label = parent;
  dc->bucket = NULL;

  return dc;
}

static DomCalc
PC_FreeDomCalc (DomCalc dc)
{
  if (!dc)
    return NULL;

  dc->bucket = List_reset (dc->bucket);

  DISPOSE (dc);

  return NULL;
}

static DomFrontCalc
PC_NewDomFrontCalc ()
{
  DomFrontCalc dfc = ALLOCATE (_DomFrontCalc);

  dfc->treelvl = -1;
  dfc->jnodes = NULL;

  return dfc;
}

static DomFrontCalc
PC_FreeDomFrontCalc (DomFrontCalc dfc)
{
  dfc->jnodes = List_reset (dfc->jnodes);

  DISPOSE (dfc);

  return NULL;
}

static PC_Block
RemoveUnreachableBBs (PC_Graph cfg, PC_Block bb)
{
  PC_Flow succ, pred, next;
  PC_Block next_bb;
  _PC_ExprIter ei;
  Expr expr;

  /* Annotate an UNREACHABLE pragma to all statements corresponding to an
   * expr in this BB */
  for (expr = PC_ExprIterFirst (bb, &ei, 1); expr;
       expr = PC_ExprIterNext (&ei, 1))
    if (expr->parentstmt && \
	!P_FindPragma (expr->parentstmt->pragma, "unreachable"))
      {
	Pragma p = P_NewPragmaWithSpecExpr ("unreachable", NULL);
	expr->parentstmt->pragma = P_AppendPragmaNext
	  (expr->parentstmt->pragma, p);
      }
  
  /* delete all successor flows */
  for (succ = bb->s_flow; succ; succ = next)
    {
      next = succ->s_next_flow;
      PC_RemoveFlow (succ);
      PC_FreeFlow (succ);
    }
  
  /* Recurse on all predecessors */
  for (pred = bb->p_flow; pred; pred = bb->p_flow)
    RemoveUnreachableBBs (cfg, pred->src_bb);

  /* Remove the BB from the list */
  if (bb->prev != NULL)
    bb->prev->next = bb->next;
  else
    P_punt ("RemoveUnreachableBBs: Attempting to remove the entry BB.");

  if (bb->next)
    bb->next->prev = bb->prev;

  next_bb = bb->next;
  
  /* Delete the BB */
  bb = PC_FreeBlock (bb);
  cfg->num_bbs--;

  /* Return the (potentially different) next BB */
  return next_bb;
}


