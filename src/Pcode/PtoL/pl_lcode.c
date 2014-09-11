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
#include "pl_main.h"
#include <Pcode/pcode.h>
#include <Pcode/dd_interface.h>

L_Oper *
PL_new_loper (Expr expr, int opc)
{
  L_Oper *new_op;

  new_op = L_create_new_op (opc);
  new_op->ext = NULL;

  if (PL_gen_acc_specs)
    {
      switch (opc)
	{
	case Lop_DEFINE:
	case Lop_PROLOGUE:
        case Lop_RTS:
	  break;
	default:
	  if (((opc >= Lop_LD_UC) && (opc <= Lop_LD_POST_F2)) ||
	      (opc == Lop_PRED_LD) || (opc == Lop_PRED_LD_BLK) ||
	      ((opc >= Lop_ST_C) && (opc <= Lop_ST_POST_F2)) ||
	      (opc == Lop_PRED_ST) || (opc == Lop_PRED_ST_BLK) ||
	      (opc == Lop_JSR) || (opc == Lop_JSR_FS) ||
	      (opc == Lop_RTS) || (opc == Lop_RTS_FS))
	    {
#if EMN_DEBUG_SHADOW
	      printf ("CREATING A Memory Op %d %p\n", new_op->id, expr->id);
#endif
	      if (!expr)
		P_warn ("PL_new_loper: No expr for sync-arc opc %d \n", opc);

	      if (expr && (expr != ((void *) (-1))))
		new_op->ext = (void *) expr;
	    }
	  break;
	}
    }

  return new_op;
}


Expr
PL_get_shadow (FuncDcl fn, int index)
{
  Shadow shadow;

  for (shadow = fn->shadow; shadow; shadow = shadow->next)
    {
      if (shadow->param_id == (index + 1))
	{
	  assert (shadow->expr->opcode == OP_assign);
	  assert (shadow->expr->operands);
	  return shadow->expr->operands;
	}
    }

  return NULL;
}


void
PL_extract_shadow_info (FuncDcl func)
{
  Stmt first_stmt;
  Stmt cur_stmt;
  Stmt next_stmt;
  Shadow last_shadow = NULL;

  /* EMN 9/2002 - Remove shadow statements and create
   *    shadow list on function for use during lcode gen
   */

  first_stmt = func->stmt->stmtstruct.compound->stmt_list;
  while (first_stmt->artificial)
    first_stmt = first_stmt->lex_next;
  func->shadow = NULL;

#if EMN_DEBUG_SHADOW
  printf ("####### Func %s\n", func->name);
#endif

  for (cur_stmt = first_stmt; cur_stmt; cur_stmt = next_stmt)
    {
      next_stmt = cur_stmt->lex_next;
      if (!cur_stmt->shadow)
	continue;

      if (cur_stmt->shadow->expr != cur_stmt->stmtstruct.expr)
	P_punt ("PtoL Shadow: Shadow expr does not match stmt expr\n");
      /* Copy to allow deletion of stmt 
       *  use the operands portion because the shadow is an ASSIGN and
       *  we want the VAR here.
       */
      cur_stmt->shadow->expr = PST_CopyExpr (PL_symtab, cur_stmt->shadow->expr);
      cur_stmt->shadow->expr->parentstmt = NULL;

      /* Create shadow list on function */
#if EMN_DEBUG_SHADOW
      printf ("PtoL Shadow: Adding %d\n", cur_stmt->shadow->param_id);
#endif
      if (func->shadow == NULL)
	func->shadow = cur_stmt->shadow;
      else
	last_shadow->next = cur_stmt->shadow;
      last_shadow = cur_stmt->shadow;
    }

#if EMN_DEBUG_SHADOW
  printf ("#########\n");
#endif
  return;
}

#define USELESS (nonloop_carried | inner_carried | outer_carried | \
                 distance_unknown)

void
PL_draw_sync_arcs (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *op, *hop;
  HashTable hash = NULL;
  List tail_ops = NULL;

  if (PL_annotate_omega)
    hash = HashTable_create (4096);

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      for (op = cb->first_op; op; op = op->next_op)
	{
	  L_AccSpec *mas, *last = NULL;
	  P_memdep_t md;
	  Expr expr;
	  P_memdep_core_t dep;
	  P_DepList dl;

	  if (!(expr = ((Expr) op->ext)))
	    continue;

	  /* Communicate U/D acc specs into Lcode */

	  md = P_GetMemDep(expr);

	  List_start(md->deps);
	  while ((dep = List_next(md->deps)))
	    {
	      mas = L_new_mem_acc_spec (dep->is_def, dep->id, dep->version,
					dep->offset, dep->size);
	      if (last)
		last->next = mas;
	      else
		op->acc_info = mas;
	      last = mas;
	    }

	  if (PL_annotate_omega)
	    {
	      /* Build table of DDO heads */

	      if ((dl = P_GetExprExtL (expr, expr_ext_deplist_idx)))
		{
		  P_DepInfo di;

		  List_start (dl->deps);
		  while ((di = (P_DepInfo) List_next (dl->deps)))
		    {
		      if (di->node_type == DEP_HEAD)
			{
			  /* head - put in table */
			  HashTable_insert (hash, di->id, op);
			}
		      else
			{
			  /* save tail ops in list for later processing */
			  tail_ops = List_insert_last (tail_ops, op);
			}
		    }
		}
	    }
	}
    }

  if (tail_ops)
    {
      L_Sync *h_sync, *t_sync;

      List_start (tail_ops);
      while ((op = (L_Oper *)List_next (tail_ops)))
	{
	  Expr expr;
	  P_DepList dl;
	  P_DepInfo di;
	  int i;

	  if (!(expr = ((Expr) op->ext)))
	    continue;

	  /* Build sync arc tail structures */
	  
	  if (!(dl = P_GetExprExtL (expr, expr_ext_deplist_idx)))
	    continue;

	  List_start (dl->deps);
	  while ((di = (P_DepInfo) List_next (dl->deps)))
	    {
	      int info = 0, dist = 0, outer_loop_carried = 0;
	      int outer_loop_only = 0, ddir;

	      if (di->node_type == DEP_HEAD)
		continue;

              assert ((di->node_type == DEP_TAIL));

	      hop = HashTable_find (hash, di->id);

	      /* Examine outer dimensions */

	      for (i = 1; i < di->depth; i++)
		{
		  ddir = di->dir[i - 1];

		  switch (ddir)
		    {
		    case DDIR_ALL:
		    case DDIR_GE:
		    case DDIR_GT:	
		      P_punt ("Invalid dependence - outer vector element "
			      "is *, 0-, or -");
		      break;
		    case DDIR_LT:
		      outer_loop_only = TRUE;
		      break;
		    case DDIR_LE:
		      outer_loop_carried = TRUE;
		      /* DML - don't want to break here.  If there is
			 a + element for any of the loops inside this
			 level but before the innermost loop,
			 dependence will get marked
			 outer_loop_only. */
		      break;
		    case DDIR_EQ:
		      break;
		    default:
		      P_punt ("Invalid dependence - "
			      "unknown dir vector element");
		      break;
		    }

		  if (outer_loop_only)
		    break;

		  /* direction must be 0 or 0+ at this level - keep going */
		}

	      /* DML - Cannot count on the value of depth after
		 unrolling and/or software pipelining.  An arc that is
		 both outer and nonloop can become outer only after
		 unrolling and/or software pipelining.  In this case,
		 the distance field will not have been set to the
		 depth.  Also, an outer/inner dependence can become an
		 outer/nonloop dependence after modulo scheduling and
		 then become outer only after prologue/epilogue
		 generation.  It looks like in these cases, the
		 distance will eventually be set to 0 when the
		 dependence becomes outer/nonloop.  So may be able to
		 detect that 0 is an invalid nesting depth for outer
		 loop only. */

	      /* Examine inner dimension */

	      if (di->depth == 0)
		{
		  dist = 0;
		  info = 0;
		}
	      else if (outer_loop_only)
		{
		  dist = di->depth;
		  info = outer_carried | distance_unknown;
		}
	      else if ((ddir = di->dir[di->depth - 1]) == DDIR_LE)
		{
		  /* the 0+ case */
		  info = inner_carried | nonloop_carried | distance_unknown;
		  dist = 0;
		}
	      else if (ddir == DDIR_EQ)
		{
		  /* the 0 case */
		  info = nonloop_carried;
		  dist = 0;
		}
	      else if (ddir == DDIR_LT)
		{
		  /* the + case */
		  info = inner_carried;

		  if (!di->known[di->depth - 1])
		    {
		      info |= distance_unknown;
		      dist = 1;
		    }
		  else
		    {
		      int j;

		      for (j = 1; j <= di->depth; j++)
			if (di->dir[j - 1] != DDIR_EQ)
			  break;

		      if (j > di->depth)
			dist = 0;
		      else if (!di->known[j - 1])
			dist = 1;
		      else
			dist = di->dist[j - 1];
		    }
		}
	      else
		{
		  P_punt ("DD_extract_dep_info: unexpected case");
		}

#if 0
	      /* "Conservate" in some case? */

	      if (CFGHasLoopDetected (DD_GetFunctionCFG(func)) &&
		  (loop = Nearest_common_loop(expr, head_expr)) &&
		  Get_Loop_iv(loop)) 
		info = USELESS;
#endif

	      if (outer_loop_carried)
		info |= outer_carried;

	      /* JWS - this merger should be more sophisticated. */

	      if (op->sync_info && (h_sync = L_find_head_sync (op, hop)))
		{
		  t_sync = L_find_tail_sync (hop, op);
		  assert (t_sync);
		    
		  if ((info & (inner_carried | nonloop_carried)) &&
		      (dist < h_sync->dist))
		    h_sync->dist = t_sync->dist = dist;

		  h_sync->info = t_sync->info |= info;

		  continue;
		}

	      L_add_specific_sync_between_opers (op, hop, 
						 info | sometimes_sync, 
						 dist, 0);
	    }
	}

      {
	int cnt = 0;

	/* Clean up useless syncs */

	List_start (tail_ops);
	while ((op = (L_Oper *)List_next (tail_ops)))
	  {
	    int i;
	    
	    if (!op->sync_info)
	      continue;
	    
	    for (i = 0; i < op->sync_info->num_sync_in; i++)
	      {
		h_sync = op->sync_info->sync_in[i];

		if ((h_sync->info & USELESS) != USELESS)
		  continue;

		hop = h_sync->dep_oper;
		L_find_and_delete_head_sync (hop, op);
		L_delete_tail_sync (op, h_sync);
		cnt++;
	      }
	  }

	if (cnt)
	  L_warn ("Removed %d useless syncs", cnt);
      }

      tail_ops = List_reset (tail_ops);
    }

  if (PL_annotate_omega)
    HashTable_free (hash);

  return;
}

