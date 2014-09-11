/*-----------------------------------------------------------------------------*/

#include <config.h>
#include <string.h>
#include <library/set.h>
#include <library/stack.h>
#include <library/llist.h>
#ifndef NEWPCODE
#include <Pcode/pcode.h>
#include <Pcode/struct.h>
#include <Pcode/parms.h>
#include <Pcode/reduce.h>
#else
#include <Pcode/pcode.h>
#include <Pcode/struct.h>
#include <Pcode/parms.h>
#include <Pcode/reduce.h>
#include <Pcode/symtab_i.h>
#include <Pcode/parloop.h>
#endif

#include "cfg.h"
#include "dom.h"
#include "loop.h"

void PC_FindBackEdges (PC_Graph cfg);
void PC_FindNaturalLoop (PC_Graph cfg, PC_Flow be);
void PC_FindLoops (PC_Graph cfg);
void PC_AnnotateLpPragma (PC_Graph cfg, int bb_id);
void PC_PrintLoop (FILE * f, PC_Loop lp);

PC_Loop PC_LoopTreeInsert (PC_Loop curr, PC_Loop ins, int lvl);

/*! \brief Uses dominator sets to find back edges in the CFG
 *
 * \param cfg
 *  control flow graph for the func being processed
 *
 *  *** CURRENTLY NOT IN USE ***
 *
 * \return void
 */
void
PC_FindBackEdges (PC_Graph cfg)
{
  PC_Block bb = NULL;
  PC_Flow fl = NULL;

  PC_BuildDomSets (cfg);

  /* process all bbs. */
  for (bb = cfg->first_bb; bb; bb = bb->next)
    /* process all flows out of this bb. */
    for (fl = bb->s_flow; fl != NULL; fl = fl->s_next_flow)
      if (PC_BB1DominatesBB2 (fl->dest_bb, fl->src_bb))	/* detect back edge */
	PC_FindNaturalLoop (cfg, fl);
}


/*! \brief Finds the natural loop corresponding to a back edge
 *
 * \param cfg
 *  control flow graph for the func being processed
 * \param be
 *  back edge going to the head of this particular natural loop
 *
 *  *** CURRENTLY NOT IN USE ***
 *
 * \return void
 */
void
PC_FindNaturalLoop (PC_Graph cfg, PC_Flow be)
{
  PC_Loop *loop = NULL;
  PC_Block bb = NULL;
  PC_Flow fl = NULL;
  Stack *st = New_Stack ();
  int lp_head = be->dest_bb->ID, lp_size = 0, i;
  int *lp_bod;
  Set lp = Set_new (), lp_exits = Set_new ();
  int num_back_edge = 0;
  int num_exit = 0;

  if (be->src_bb->ID == be->dest_bb->ID)
    P_warn ("PC_FindNaturalLoop: head and tail of back edge are same.");


  /* Build the set of all bbs in the lp body. */
  lp = Set_add (lp, lp_head);

  if (!Set_in (lp, be->src_bb->ID))
    {
      lp = Set_add (lp, be->src_bb->ID);
      Push_Top (st, be->src_bb);
    }

  while ((bb = Pop (st)) != ((void*)-1))
    {
      for (fl = bb->p_flow; fl; fl = fl->p_next_flow)
	{
	  if (!Set_in (lp, fl->src_bb->ID))
	    {
	      lp = Set_add (lp, fl->src_bb->ID);
	      Push_Top (st, fl->src_bb);
	    }
	}
    }


  /* Build the set of all bbs that are exits out of the lp. */
  lp_bod = (int *) calloc (Set_size (lp), sizeof (int));
  lp_size = Set_2array (lp, lp_bod);
  for (i = 0; i < lp_size; i++)
    {
      bb = PC_FindBlock (cfg, lp_bod[i]);
      for (fl = bb->s_flow; fl; fl = fl->s_next_flow)
	if (!Set_in (lp, fl->dest_bb->ID)) {
	  lp_exits = Set_add (lp_exits, fl->dest_bb->ID);
          num_exit++;
        }
    }
  free (lp_bod);

  /* Append this loop to the list of loops in the PC_Graph. */
  loop = &cfg->lp;
  while (*loop)
    {
      loop = &(*loop)->next;
    }
  *loop = PC_NewLoop (lp_head, lp, lp_exits, num_back_edge, num_exit);

  Set_dispose (lp);
  Set_dispose (lp_exits);
}


static void
PC_SetBlockSetLoop (PC_Graph cfg, PC_Loop lp, Set inner)
{
  int cnt, i, *buf;
  PC_Block blk;

  if (!(cnt = Set_size (inner)))
    return;

  buf = alloca (cnt * sizeof (int));

  Set_2array (inner, buf);

  for (i = 0; i < cnt; i++)
    {
      blk = PC_FindBlock (cfg, buf[i]);
      blk->loop = lp;
    }
  return;
}

static void
PC_SetBlockLoops (PC_Graph cfg)
{
  PC_Loop lp, clp;

  for (lp = cfg->lp; lp; lp = lp->next)
    {
      Set inner = NULL;

      inner = Set_copy (lp->body);

      for (clp = lp->child; clp; clp = clp->sibling)
	inner = Set_subtract_acc (inner, clp->body);

      PC_SetBlockSetLoop (cfg, lp, inner);

      inner = Set_dispose (inner);
    }
}

/*! \brief Finds loops in the CFG. Builds lp hdr, body and exit bb sets, and 
 *   puts this lp info in a PC_Loop struct, which is appended to the CFG lp list.
 *
 * \param cfg
 *  control flow graph for the func being processed
 *
 *  Multiple back edges coming into the same loop header are considered as one
 *  loop. The loop body is a union of the bodies of the loops corresponding to 
 *  each one of these back edges. 
 *
 * \return void
 */
void
PC_FindLoops (PC_Graph cfg)
{
  PC_Block h_bb;

  /* build dominator sets for each bb in the cfg */
  PC_BuildDomSets (cfg);

  /* check each cfg bb in turn to see if it's a lp header bb */
  for (h_bb = cfg->first_bb; h_bb; h_bb = h_bb->next)
    {
      PC_Flow be;
      PC_Loop *loop;
      int num_back_edge;
      int num_exit;
      int s_lp_head = -1;
      Set s_lp_body = Set_new (), s_lp_exits = Set_new ();

      /* check all edges coming into h_bb to see if they are back edges */
      num_back_edge = 0;
      for (be = h_bb->p_flow; be; be = be->p_next_flow)
	{
	  /* check to see if be is indeed a back edge */
	  if (PC_BB1DominatesBB2 (be->dest_bb, be->src_bb))
	    {
	      PC_Block bb;
	      Stack *st = New_Stack ();
	      Set lp_body = Set_new ();

              num_back_edge++;
	      s_lp_head = be->dest_bb->ID;

	      if (be->src_bb->ID == be->dest_bb->ID)
		P_warn
		  ("PC_FindNaturalLoop: back edge head & tail are same.");


	      /* Build the set of all bbs in the lp body. */
	      lp_body = Set_add (lp_body, s_lp_head);

	      if (!Set_in (lp_body, be->src_bb->ID))
		{
		  lp_body = Set_add (lp_body, be->src_bb->ID);
		  Push_Top (st, be->src_bb);
		}

	      while ((bb = Pop (st)) != ((void*)-1))
		{
		  PC_Flow fl;
		  for (fl = bb->p_flow; fl; fl = fl->p_next_flow)
		    {
		      if (!Set_in (lp_body, fl->src_bb->ID))
			{
			  lp_body = Set_add (lp_body, fl->src_bb->ID);
			  Push_Top (st, fl->src_bb);
			}
		    }
		}


	      /* the s_lp body is a union of all of these 'inner' lp bodies */
	      s_lp_body = Set_union (s_lp_body, lp_body);

	      Clear_Stack (st);
	      Set_dispose (lp_body);
	    }
	}

      /* If a loop was found, build its set of exits bbs, and append it to the 
         list of loops in the cfg. */
      if (s_lp_head != -1)
	{
	  int *s_lp_bod;
	  int lp_size = 0, i;

	  /* Build the set of all bbs that are exits out of the lp. */
	  s_lp_bod = (int *) calloc (Set_size (s_lp_body), sizeof (int));
	  lp_size = Set_2array (s_lp_body, s_lp_bod);
          num_exit = 0;
	  for (i = 0; i < lp_size; i++)
	    {
	      PC_Block bb;
	      PC_Flow fl;
	      bb = PC_FindBlock (cfg, s_lp_bod[i]);
	      for (fl = bb->s_flow; fl; fl = fl->s_next_flow)
		if (!Set_in (s_lp_body, fl->dest_bb->ID)) 
                  {
                    num_exit++;
                    s_lp_exits = Set_add (s_lp_exits, fl->dest_bb->ID);
                  }
	    }
	  free (s_lp_bod);

	  /* check for redundant loops */
	    {
	      PC_Loop lp = NULL;
	      int same = 0;
	      
	      for (lp = cfg->lp; lp; lp = lp->next)
		if (Set_same (s_lp_body, lp->body))
		  {
		    same = 1;
		    break;
		  }
	      
	      if (same)
		{
		  Set_dispose (s_lp_body);
		  Set_dispose (s_lp_exits);
		  continue;
		}
	    }

	  /* Create a new PC_Loop and append it to the current CFG
             loop list. */
	  loop = &cfg->lp;
	  while (*loop)
	    loop = &(*loop)->next;
	  *loop = PC_NewLoop (s_lp_head, s_lp_body, s_lp_exits, 
			      num_back_edge, num_exit);

	  cfg->lp_tree = PC_LoopTreeInsert (cfg->lp_tree, *loop, 1);
	}

      Set_dispose (s_lp_body);
      Set_dispose (s_lp_exits);
    }

  PC_SetBlockLoops (cfg);

  return;
}

PC_Loop
PC_LoopTreeInsert (PC_Loop curr, PC_Loop ins, int lvl)
{
  int inserted = 0;
  PC_Loop lp = NULL;
  PC_Loop ret = NULL;
  PC_Loop prev = NULL;

  for (lp = curr; lp; lp = lp->sibling)
    {
      if (Set_same (ins->body, lp->body))
	{
	  /* ins and lp are redundant loops */
          fprintf (stderr, "\nWARNING: PC_LoopTreeInsert: "
		   "Set_same(ins->body, lp->body)\n");
	  inserted = 1;
	  ret = curr;
	  break;
	}
      else if (Set_subtract_empty (lp->body, ins->body))
	{
	  /* ins is parent of lp */
	  ins->nesting_level = lvl;
	  ins->parent = lp->parent;
	  lp->parent = ins;
	  ins->child = lp;
	  ins->sibling = lp->sibling;
	  lp->sibling = NULL;
	  if (prev != NULL) prev->sibling = ins;
	  inserted = 1;
	  ret = prev ? curr : ins;
	  break;
	}
      else if (Set_subtract_empty (ins->body, lp->body))
	{
	  /* lp is parent of ins */
          if (lp->child)
	    {
	      lp->child = PC_LoopTreeInsert (lp->child, ins, lvl+1);
	    }
          else 
            {
              lp->child = ins;
              ins->parent = lp;
              ins->nesting_level = lvl+1;
            }
	  inserted = 1;
	  ret = curr;
	  break;
	}
      prev = lp;
    }

  if (!inserted)
    {
      /* ins belongs on current level */
      ins->nesting_level = lvl;
      ins->parent = curr ? curr->parent : NULL;
      ins->sibling = curr;
      ret = ins;
    }

  return ret;
}

/*! \brief Extracts lp pragma info out of a lp hdr bb. Uses this info to create 
 *   and annotate a lp pragma onto the pcode stmt/expr corresponding to the hdr.
 *
 * \param cfg
 *  control flow graph for the func being processed
 * \param bb_id
 *  ID of the lp hdr PC_Block
 *
 *  *** CURRENTLY NOT IN USE ***
 *  Annotating loop pragmas here is inelegant because we need to access pcode 
 *  structs, all of which aren't necessarily accessible via the CFG block that
 *  forms the loop header block. It's better to access all this info at the time
 *  of CFG construction, store this into a PC_LpPrag struct, and then store a
 *  pointer to this struct in the loop header block. 
 *
 * \return void
 */
void
PC_AnnotateLpPragma (PC_Graph cfg, int bb_id)
{
#if 0
  int LP_TYP = 0;
  int LP_TYP_FOR = 1, LP_TYP_DO = 2, LP_TYP_GOTO = 3;
  char lp_type[16];
  int lp_ln;
  Expr lp, fn, fl;
  PC_Block bb = PC_FindBlock (cfg, bb_id);

  /* !!! this makes the line #s even more inaccurate. !!! */

  /* figure out what kind of a loop we have here */

  /* bb is hdr of a for/while lp */
  if (bb->cond)
    LP_TYP = LP_TYP_FOR;
  /* bb is hdr of a goto -> label type lp, and bb contains a label */
  else if (bb->first_ps && bb->first_ps->data.stmt->labels)
    LP_TYP = LP_TYP_GOTO;
  /* the only other option is a do-while loop! */
  else
    LP_TYP = LP_TYP_DO;


  /* set lp_type, lp_ln, and bb->prag depending upon which lp type we have */
  switch (LP_TYP)
    {
      Stmt st;
      int loop_type;

    case LP_TYP_FOR:
      st = bb->cond->parentstmt;
      bb->lp_prag = &(bb->cond->pragma);
      lp_ln = st->lineno;
      loop_type = st->stmtstruct.serloop->loop_type;
      switch (loop_type)
	{
	case LT_FOR:
	  sprintf (lp_type, "for");
	  break;
	case LT_WHILE:
	  sprintf (lp_type, "while");
	  break;
	default:
	  P_punt ("PC_AnnotateLpPragma: Unexpected loop type: %d.",
		  loop_type);
	}

      break;

    case LP_TYP_GOTO:
      /* bb->first_ps may be NULL, look for first non-Null Stmt/Expr */
      st = bb->first_ps->data.stmt;
      bb->lp_prag = &(st->pragma);
      lp_ln = st->lineno;
      sprintf (lp_type, "goto");
      break;

      /* the do-while lp is kind of complicated with two possibilities */
    case LP_TYP_DO:
      /* 1st possibility, a non-empty do-while loop */
      if (bb->first_ps)
	{
	  st = bb->first_ps->data.stmt;
	  bb->lp_prag = &(st->pragma);
	}
      /* 2nd possibility, an empty do-while loop */
      else
	{
	  PC_Block test_bb = bb->s_flow->dest_bb;
	  if (test_bb && test_bb->cond)
	    {
	      Expr expr = test_bb->cond;
	      st = expr->parentstmt;
	      bb->lp_prag = &(expr->pragma);
	    }
	  else
	    P_punt ("PC_AnnotateLpPragma: Succ of empty lp body bb is NULL.");
	}
      lp_ln = st->lineno;
      sprintf (lp_type, "do");
      break;

    default:
      P_punt ("PC_AnnotateLpPragma: Unknown loop type: %d.", LP_TYP);
      break;
    }


  /* create Exprs for lp pragma info gathered above */
  {
    lp = P_NewStringExpr (lp_type);
    lp->next = P_NewIntExpr (lp_ln);

    fn = P_NewStringExpr (cfg->func->name);
    fn->next = P_NewIntExpr (cfg->func->stmt->lineno);

    fl = P_NewStringExpr (cfg->func->stmt->filename);
  }

  /* Get rid of any old lp pragmas from previous runs */
  if (*(bb->lp_prag))
    {
      P_warn ("PC_AnnotateLpPragma: Getting rid of existing loop pragmas.");
      P_RemovePragma (*(bb->lp_prag));
      *(bb->lp_prag) = NULL;
    }

  /* create new Pragmas using the Exprs created above */
  {
    *(bb->lp_prag) =
      P_AppendPragmaNext (*(bb->lp_prag),
			  P_NewPragmaWithSpecExpr ("LOOP", lp));
    *(bb->lp_prag) =
      P_AppendPragmaNext (*(bb->lp_prag),
			  P_NewPragmaWithSpecExpr ("FUNC", fn));
    *(bb->lp_prag) =
      P_AppendPragmaNext (*(bb->lp_prag),
			  P_NewPragmaWithSpecExpr ("FILE", fl));
  }

#endif
  return;
}


/* Allocation / deallocation functions
 * ----------------------------------------------------------------------
 */

PC_Loop
PC_NewLoop (int head, Set body, Set exits, int num_back_edge, int num_exit)
{
  PC_Loop lp;
  if (body == NULL)
    P_punt ("PC_NewLoop: body set is NULL.");
  if (exits == NULL)
    P_punt ("PC_NewLoop: exits set is NULL.");

  lp = ALLOCATE (_PC_Loop);
  lp->ID = PC_NextLoopID ();
  lp->head = head;
  lp->body = Set_copy (body);
  lp->exits = Set_copy (exits);
  lp->next  = NULL;

  lp->num_exit = num_exit;
  lp->num_back_edge = num_back_edge;
  lp->nesting_level = 0;
  lp->child = NULL;
  lp->sibling = NULL;
  lp->parent = NULL;

  return lp;
}


PC_Loop
PC_FreeLoop (PC_Loop lp)
{
  Set_dispose (lp->body);
  Set_dispose (lp->exits);

  DISPOSE (lp);
  return NULL;
}


/*
 * DEBUG
 * ----------------------------------------------------------------------
 */

void
PC_PrintLoop (FILE * f, PC_Loop lp)
{
#if 0
  return;
#endif

  if (lp == NULL)
    P_punt ("PC_PrintLoop: lp is NULL.");
  if (f == NULL)
    P_punt ("PC_PrintLoop: file f is NULL.");

  fprintf (f, "Loop ID: %d.\n", lp->ID);
  fprintf (f, "Loop Header: %d.\n", lp->head);
  Set_print (f, "Loop Body:", lp->body);
  Set_print (f, "Loop Exits:", lp->exits);
  fprintf (f, "\n\n");
}

/*
 * static function to determine if l2 is a subloop of l1.
 * if not, returns 0.  if so, returns the number of loops
 * down it is.  for example, if l1 is a loop inside of a loop
 * that is inside of l1, it will return 2.
 */
static int
LoopContains (PC_Loop l1, PC_Loop l2)
{
  PC_Loop l;
  int ret;

  /* stupid case */
  if (l1 == l2)
    return 0;

  /* loop through all the children */
  for (l = l1->child; l; l = l->sibling)
    {
      /* first check if the subloop is the loop we are looking for */
      if (l == l2) return 1;
      
      /* check the subloops of the current child */
      if ((ret = LoopContains (l, l2))) return ret + 1;
    }

  /* if nothing found, return 0 */
  return 0;
}

/*! \brief determines whether two loops are correlated
 *
 * \return 
 * the number of loops that l1 is within l2 (or visa versa).  
 * the sign is determined by whether l1 is inside of l2 (or visa versa)
 *
 * here are some examples:
 * -  l2 is loop inside of l1  =>   1
 * -  l1 is a loop inside of l2 =>  -1
 * -  l2 if a loop inside of a loop inside of l1 => 2
 * -  l2 and l1 are not correlated => 0
 *
 * also returns
 * -  l1 = NULL, l2 != NULL  => 1
 * -  l2 = NULL, l1 != NULL  => -1
 * -  l1 = L2 = NULL  => 0
 */
int 
PC_LoopContainsLoop (PC_Loop l1, PC_Loop l2)
{
  int a, b;

  /* first handle NULL loop cases */
  if (l1 == NULL && l2 != NULL)
    return 1;
  else if (l1 != NULL && l2 == NULL)
    return -1;
  else if (l1 == NULL && l2 == NULL)
    return 0;
  else 
    {
      a = LoopContains(l1, l2);
      b = LoopContains(l2, l1);

      /* check to make sure something bad is not going on */
      if (a && b)
	P_punt("PC_LoopContainsLoop: Found two loops that are both contained "
	       "within each other");
      
      /* because atleast one has to be zero (above check), this should be ok */
      return a - b;
    }
}

/* \brief determine if an expr is part of a loop
 *
 * This will search for a given expression in a loop.  This function
 * hsa two possible techniques of searching.  If the expression has
 * a expr->BB hash table entry for the cfg, then we will find the inner
 * loop associated with that BB, and simply search up the loop tree for
 * the pcloop that is passed to this function.  If the expression does
 * not have an expr->BB hash table entry, we will have to use the slower
 * algorithm that searches through all the expressions in all the BBs of
 * the loop.
 *
 * \param cfg
 *   current cfg
 * \param lp
 *   loop context to search in
 * \param find_expr
 *   expression we are searching for
 *
 * \return
 *   1 if found, 0 if not
 */
int 
PC_LoopContainsExpr (PC_Graph cfg, PC_Loop lp, Expr find_expr)
{
  PC_Block bb;
  if ((bb = PC_FindExprBB(cfg, find_expr->id)))
    {
      /* EXPR -> BB Hash worked!  O(loopnest) algo... */
      PC_Loop check_loop = bb->loop;
      if (check_loop == NULL)
	return 0;
      do
	{
	  if (check_loop == lp)
	    return 1;
	} while ((check_loop = check_loop->parent));
      return 0;
    }
  else /* doh...we have to do it the slow way */
    {
      int *bb_ids;
      int bb_ids_size, i;

      /* loop through the bb's in the loop */
      bb_ids = (int *) calloc (Set_size (lp->body), sizeof (int));
      bb_ids_size = Set_2array (lp->body, bb_ids);
      for (i = 0; i < bb_ids_size; i++)
	{
	  Expr expr;
	  _PC_ExprIter ei;
	  bb = PC_FindBlock (cfg, bb_ids[i]);

	  for (expr = PC_ExprIterFirst(bb, &ei, 1); expr;
	       expr = PC_ExprIterNext(&ei, 1))
	    {
	      if (find_expr == expr)
		return 1;
	    }
	} /* for all bb's in lp */
      /* not found... */
      return 0;
    }
}

/* SER 20041026 */
/* Returns 1 if a BB is in a child of the given loop.
 * Used in induction expression calculation. */
int PC_BB_In_Child_Loop (int bb_id, PC_Loop loop)
{
  PC_Loop search_loop;

  if (loop == NULL)
    P_punt ("PC_BB_In_Child_Loop: loop is NULL.");
  for (search_loop = Get_Loop_child(loop); search_loop; 
       search_loop = search_loop->sibling)
    {
      if (Set_in (Get_Loop_body(search_loop), bb_id))
	return 1;
    }
  return 0;
}


/* SER 20041019
 * Returns 1 if a BB is in a direct ancestor loop of the given loop (but
 * not any of its children), or if it is in a linear region of the function.
 * Used in induction expression calculation. */
int
PC_BB_In_Direct_Ancestor_Loop_Scope (PC_Graph cfg, PC_Loop loop, int bb_id)
{
  PC_Loop search_loop = loop;

  if (loop == NULL)
    P_punt ("PC_BB_In_Direct_Ancestor_Loop_Scope: loop is NULL.");

  /* Statement shouldn't be in the current loop */
  if (Set_in (Get_Loop_body(search_loop), bb_id))
    return 0;

  /* Next, walk up the chain until a loop contains the bb */
  while ((search_loop = Get_Loop_parent (search_loop)))
    {
      PC_Loop child_loop;

      if (!(Set_in(Get_Loop_body(search_loop), bb_id)))
        continue;

      /* At this point, we know that the bb is in search_loop and not in the
       * nesting we've come through; the question is whether it's in a
       * different set of loop nests.  Thus, check the children to see
       * if it's in any of them.  */
      child_loop = Get_Loop_child (search_loop);

      while ((child_loop = Get_Loop_sibling(child_loop)) != NULL)
        if (Set_in(Get_Loop_body (child_loop), bb_id))
          return 0;

      return 1;
    }
  /* At this point we've searched all the ancestor loops, but we don't know
   * if it might be in a sibling loop of the highest level loop.  Thus,
   * return 0 if it's in ANY loop in the CFG. */

  for (search_loop = cfg->lp_tree; search_loop;
       search_loop = search_loop->sibling)
    if (Set_in (Get_Loop_body(search_loop), bb_id))
      return 0;

  return 1;
}

/*! \brief get the depth of a loop
 *
 * returns the loopnest depth of a loop.  if there is no loop (NULL), then
 * zero is returned.  Otherwise, the number will be 1 or more */
int
PC_LoopDepth(PC_Loop pcloop)
{
  int i;

  i = 0;
  while (pcloop)
    {
      pcloop = pcloop->parent;
      i++;
    }
  return i;
}
