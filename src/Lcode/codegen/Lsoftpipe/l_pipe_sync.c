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
 *      File: l_pipe_sync.c
 *      Description: Sync arc maintenance routines
 *      Creation Date: July, 1995
 *      Author: Daniel Lavery
 *
 *      Revision Date: Summer 1999
 *      Authors: IMPACT Technologies: Matthew Merten and John Gyllenhaal
\*****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "l_softpipe_int.h"
#include "l_pipe_sync.h"

/* After modulo scheduling, intra-iteration sync arcs can become
   cross-iteration sync arcs because the original loop iteration is
   stretched across multiple iterations of the kernel.  Also,
   cross-iteration sync arcs can become intra-iteration sync arcs
   because loop iterations are overlapped.  This routine changes the
   flags indicating whether the dependence is loop-carried or not, and
   adjusts the distance appropriately.  This routine is called after
   the loop has been scheduled and after it has been analyzed for
   modulo variable expansion, but before the original order of the
   opers is changed and before unrolling for modulo variable
   expansion.  */

void
Lpipe_adjust_syncs_for_modulo_sched (SM_Cb * sm_cb)
{
  L_Oper *oper;			/* oper at source of dependence */
  L_Oper *dep_oper;		/* oper at sink of dependence */
  Softpipe_Op_Info *softpipe_info;	/* info for source of dependence */
  Softpipe_Op_Info *dep_softpipe_info;	/* info for sink of dependence */
  L_Sync_Info *sync_info;
  L_Sync *sync, *dep_sync;
  L_Cb *dest_cb;		/* cb containing oper at sink of dependence */
  L_Cb *header_cb = sm_cb->lcode_cb;
  int i;
  int stage_diff;		/* Number of iterations of the kernel
				   between the execution of oper and 
				   execution of dep_oper.  This is by
				   definition the new dependence distance. */

  for (oper = header_cb->first_op; oper != NULL; oper = oper->next_op)
    {
      if (!(sync_info = oper->sync_info))
	continue;

      /* Look at the outgoing sync arcs for each oper. */

      for (i = 0; i < sync_info->num_sync_out; i++)
	{
	  sync = sync_info->sync_out[i];

	  dep_oper = sync->dep_oper;

	  dest_cb = L_oper_hash_tbl_find_cb (L_fn->oper_hash_tbl,
					     dep_oper->id);

	  /* We do not care about arcs coming in from opers outside 
	     the loop or arcs going to opers outside the loop.  */

	  if (dest_cb == header_cb)
	    {
	      /* Have found a sync arc between 2 opers which are both
	         inside the loop.  Check if it indicates a non-loop or
	         inner loop carried dependence.  Dependences carried
	         by the outer loop are not affected by modulo
	         scheduling.  Do this by checking the
	         inner/outer/nonloop flags for the sync arc.  At least
	         one of the flags must be set, so there are 7 valid
	         combinations of the 3 flags. */

	      softpipe_info = SOFTPIPE_OP_INFO (oper);
	      dep_softpipe_info = SOFTPIPE_OP_INFO (dep_oper);
	      dep_sync = L_find_tail_sync (dep_oper, oper);

	      /* non-loop-carried but not inner loop carried dependences */
	      if (IS_NONLOOP_CARRIED (sync->info) &&
		  !IS_INNER_CARRIED (sync->info))
		{		/* 2 of 7 cases */
		  /* The two opers are in the same iteration, so the
		     new distance for the dependence is equal to the
		     difference in the stage number of the two
		     opers. */
		  stage_diff = dep_softpipe_info->stage -
		    softpipe_info->stage;

		  /* punt if find dependence that is backward in time */
#if 0
		  /* Cannot check this anymore.  Sometimes
		     L_independent_memory_ops figures out that 
		     something that sync arcs say is dependent is 
		     really independent.  Set distance to 0 for now. */
		  if (stage_diff < 0)
		    {
		      L_punt ("Lpipe_adjust_syncs_for_modulo_sched: "
			      "stage difference cannot be negative - "
			      "oper: %d, dep_oper: %d\n",
			      oper->id, dep_oper->id);
		    }
#endif
		  if (stage_diff < 0)
		    {
		      stage_diff = 0;
		    }
		  sync->dist = stage_diff;
		  dep_sync->dist = stage_diff;
		  if (stage_diff != 0)
		    {
		      /* non-loop-carried dependence has become inner
		         loop carried */
		      sync->info &= ~SET_NONLOOP_CARRIED (0);
		      sync->info |= SET_INNER_CARRIED (0);
		      dep_sync->info &= ~SET_NONLOOP_CARRIED (0);
		      dep_sync->info |= SET_INNER_CARRIED (0);
		    }
		}
	      /* non-loop- and inner-carried dependences */
	      else if (IS_NONLOOP_CARRIED (sync->info) &&
		       IS_INNER_CARRIED (sync->info))
		{		/* 2 of 7 cases */

		  if (!IS_DISTANCE_UNKNOWN (sync->info))
		    {
		      L_punt ("Lpipe_adjust_syncs_for_modulo_sched: "
			      "Sync arc is marked as both nonloop and "
			      "inner loop carried but distance unknown "
			      "flag not set - cb: %d, from_oper: %d, "
			      "to_oper: %d\n", header_cb->id, oper->id,
			      dep_oper->id);
		    }

		  /* The worst case is that the two opers are in the same 
		     iteration, so the new conservative distance for the 
		     dependence is equal to the difference in the stage 
		     number of the two opers. */
		  stage_diff = dep_softpipe_info->stage -
		    softpipe_info->stage;

		  if (stage_diff < 0)
		    {
		      stage_diff = 0;
		    }
		  sync->dist = stage_diff;
		  dep_sync->dist = stage_diff;
		  if (stage_diff != 0)
		    {
		      /* worst case is now inner loop carried */
		      sync->info &= ~SET_NONLOOP_CARRIED (0);
		      dep_sync->info &= ~SET_NONLOOP_CARRIED (0);
		    }
		}
	      /* inner-loop carried but not non-loop dependence */
	      else if (IS_INNER_CARRIED (sync->info) &&
		       !IS_NONLOOP_CARRIED (sync->info))
		{		/* 2 of 7 cases */

		  if (sync->dist < 1)
		    {
		      L_punt ("Lpipe_adjust_syncs_for_modulo_sched: "
			      "Sync arc is marked as inner loop carried "
			      "but distance is less than 1 - cb: %d, "
			      "from_oper: %d, to_oper: %d\n",
			      header_cb->id, oper->id, dep_oper->id);
		    }

		  /* dep_softpipe_info->stage is stage containing dep_oper
		     for iteration which is sync->dist iterations later
		     than iteration containing oper.  Therefore, must add
		     sync->dist. */
		  stage_diff = dep_softpipe_info->stage -
		    softpipe_info->stage + sync->dist;

		  /* punt if find dependence that is backward in time */
		  /* this punt is OK because L_independent memory ops cannot
		     override a dependence that is only inner carried */
		  if (stage_diff < 0)
		    {
		      L_punt ("Lpipe_adjust_syncs_for_modulo_sched: "
			      "stage difference cannot be negative - "
			      "cb: %d, oper: %d, dep_oper: %d\n",
			      header_cb->id, oper->id, dep_oper->id);
		    }
		  sync->dist = stage_diff;
		  dep_sync->dist = stage_diff;

		  if (IS_DISTANCE_UNKNOWN (sync->info))
		    {
		      if (stage_diff == 0)
			{
			  /* worst case dependence has become non-loop
			     carried */
			  sync->info |= SET_NONLOOP_CARRIED (0);
			  dep_sync->info |= SET_NONLOOP_CARRIED (0);
			}
		    }
		  else
		    {
		      if (stage_diff == 0)
			{
			  /* dependence has become non-loop carried */
			  sync->info &= ~SET_INNER_CARRIED (0);
			  sync->info |= SET_NONLOOP_CARRIED (0);
			  dep_sync->info &= ~SET_INNER_CARRIED (0);
			  dep_sync->info |= SET_NONLOOP_CARRIED (0);
			}
		    }
		}
	      else if (!IS_INNER_CARRIED (sync->info) &&
		       !IS_NONLOOP_CARRIED (sync->info) &&
		       IS_OUTER_CARRIED (sync->info))
		{		/* 1 of 7 cases */
		  /* nothing needs to be done in this case */
		}
#if 0
	      else
		{
		  L_punt ("Lpipe_adjust_syncs_for_modulo_sched: "
			  "Invalid sync arc - at least one of "
			  "inner/outer/nonloop must be set - cb %d, "
			  "from_oper: %d, to_oper: %d, sync info: %d\n",
			  header_cb->id, oper->id, dep_oper->id, sync->info);
		}
#endif
	    }
	}
    }
  return;
}

/* Sync arcs are automatically copied when opers are copied.  Thus,
   during prologue generation, all necessary sync arcs are generated.
   However, unecessary sync arcs are also generated between the
   prologue and the kernel.  This routine removes unnecessary arcs to
   reduce sync arc explosion.  The sync arcs between opers in the
   prologue need to be adjusted so that they are correct for possible
   later passes of acyclic scheduling on the prologue cb.  I am not
   attempting to adjust the sync arcs from the prologue to the kernel
   or between the prologue and other cbs at this point. */

void
Lpipe_adjust_syncs_for_prologue (void)
{
  L_Oper *oper;			/* oper at sink of dependence */
  L_Oper *dep_oper;		/* oper at source of dependence */
  Softpipe_Op_Info *softpipe_info;	/* info for sink of dependence */
  Softpipe_Op_Info *src_softpipe_info;	/* info for source of dependence */
  L_Sync_Info *sync_info;
  L_Sync *sync, *src_sync;
  L_Cb *src_cb;			/* cb containing oper at src of dependence */
  int i;
  int iter_num;
  int src_iter_num;

  for (oper = prologue_cb->first_op; oper != NULL; oper = oper->next_op)
    {

      if (oper->sync_info == NULL)
	continue;

      sync_info = oper->sync_info;

      /* Look at the incoming sync arcs for each oper. */

      for (i = 0; i < sync_info->num_sync_in; i++)
	{
	  sync = sync_info->sync_in[i];

	  dep_oper = sync->dep_oper;

	  src_cb = L_oper_hash_tbl_find_cb (L_fn->oper_hash_tbl,
					    dep_oper->id);

	  /* Remove any arcs which go from the header_cb to the prologue.
	     Such dependences are impossible. */
	  if (src_cb == header_cb)
	    {
	      L_find_and_delete_head_sync (dep_oper, oper);
	      L_delete_tail_sync (oper, sync);
	      /* need to decrement i since deleted the sync */
	      i--;
	    }

	  if (src_cb == prologue_cb)
	    {

	      /* Have found a sync arc between 2 opers which are
	         both in the prologue.  Check if adjustment of the
	         arc is needed.  Do this by checking the inner/outer/nonloop 
	         flags for the sync arc.  At least one of the flags must 
	         be set, so there are 7 valid combinations of the 3 flags. */

	      softpipe_info = SOFTPIPE_OP_INFO (oper);
	      src_softpipe_info = SOFTPIPE_OP_INFO (dep_oper);
	      iter_num = softpipe_info->unrolled_iter_num;
	      src_iter_num = src_softpipe_info->unrolled_iter_num;
	      src_sync = L_find_head_sync (dep_oper, oper);

	      /* CASE 1: outer loop carried only */
	      if (!IS_NONLOOP_CARRIED (sync->info) &&
		  !IS_INNER_CARRIED (sync->info) &&
		  IS_OUTER_CARRIED (sync->info))
		{
		  /* Set inner carried and distance unknown with minimum
		     distance 1. We set inner carried because the prologue 
		     opers have been moved out of the inner loop and now 
		     may or may not now be in the "outer" loop which carried 
		     the dependence. */
		  sync->info = SET_INNER_CARRIED (sync->info) |
		    SET_DISTANCE_UNKNOWN (0);
		  sync->dist = 1;
		  src_sync->info = SET_INNER_CARRIED (src_sync->info) |
		    SET_DISTANCE_UNKNOWN (0);
		  src_sync->dist = 1;
		}
	      /* CASE 2: non loop carried only */
	      else if (IS_NONLOOP_CARRIED (sync->info) &&
		       !IS_INNER_CARRIED (sync->info) &&
		       !IS_OUTER_CARRIED (sync->info))
		{
		  /* If the two opers are from different iterations of
		     the unrolled kernel, then the arc can be deleted. */
		  if (iter_num != src_iter_num)
		    {
		      L_find_and_delete_head_sync (dep_oper, oper);
		      L_delete_tail_sync (oper, sync);
		      i--;	/* need to decrement i since deleted sync */
		    }
		}
	      /* CASE 3: non loop and outer loop carried */
	      else if (IS_NONLOOP_CARRIED (sync->info) &&
		       !IS_INNER_CARRIED (sync->info) &&
		       IS_OUTER_CARRIED (sync->info))
		{
		  /* If the two opers are from different iterations of
		     the unrolled kernel, then the non loop flag can be
		     cleared and the minimum distance becomes 1.  
		     Set inner carried and distance unknown
		     because the prologue opers have been moved out of 
		     the inner loop. */
		  if (iter_num != src_iter_num)
		    {
		      sync->info = sync->info & ~SET_NONLOOP_CARRIED (0);
		      sync->dist = 1;
		      src_sync->info = src_sync->info &
			~SET_NONLOOP_CARRIED (0);
		      src_sync->dist = 1;
		    }
		  sync->info = SET_INNER_CARRIED (sync->info) |
		    SET_DISTANCE_UNKNOWN (0);
		  src_sync->info = SET_INNER_CARRIED (src_sync->info) |
		    SET_DISTANCE_UNKNOWN (0);
		}
	      /* CASE 4: inner loop carried only */
	      else if (!IS_NONLOOP_CARRIED (sync->info) &&
		       IS_INNER_CARRIED (sync->info) &&
		       !IS_OUTER_CARRIED (sync->info))
		{
		  if (sync->dist < 1)
		    {
		      L_punt ("Lpipe_adjust_syncs_for_prologue: "
			      "Sync arc is marked as inner loop carried "
			      "but distance is less than 1 - cb: %d, "
			      "from_oper: %d, to_oper: %d\n",
			      prologue_cb->id, dep_oper->id, oper->id);
		    }

		  /* If the distance is known and the two opers are not
		     the right distance apart in iterations, the arc 
		     can be deleted.  If the distance is not known,
		     if the two opers are in the same iteration, the
		     arc can be deleted.  In all other cases, the
		     arc becomes non loop carried only instead of inner
		     carried, because the prologue does not iterate. */
		  if (!IS_DISTANCE_UNKNOWN (sync->info) &&
		      (src_iter_num + sync->dist != iter_num))
		    {
		      L_find_and_delete_head_sync (dep_oper, oper);
		      L_delete_tail_sync (oper, sync);
		      i--;	/* deleted sync so need to decrement i */
		    }
		  else if (src_iter_num == iter_num)
		    {
		      L_find_and_delete_head_sync (dep_oper, oper);
		      L_delete_tail_sync (oper, sync);
		      i--;	/* deleted sync so need to decrement i */
		    }
		  else
		    {
		      sync->info = SET_NONLOOP_CARRIED (sync->info) &
			~SET_INNER_CARRIED (0) & ~SET_DISTANCE_UNKNOWN (0);
		      sync->dist = 0;
		      src_sync->info = SET_NONLOOP_CARRIED (src_sync->info) &
			~SET_INNER_CARRIED (0) & ~SET_DISTANCE_UNKNOWN (0);
		      src_sync->dist = 0;
		    }
		}
	      /* CASE 5: inner and outer loop carried */
	      else if (!IS_NONLOOP_CARRIED (sync->info) &&
		       IS_INNER_CARRIED (sync->info) &&
		       IS_OUTER_CARRIED (sync->info))
		{
		  if (sync->dist < 1)
		    {
		      L_punt ("Lpipe_adjust_syncs_for_prologue: "
			      "Sync arc is marked as inner loop carried "
			      "but distance is less than 1 - cb: %d, "
			      "from_oper: %d, to_oper: %d\n",
			      prologue_cb->id, dep_oper->id, oper->id);
		    }

		  /* If the distance is known and the two opers are not
		     the right distance apart in iterations, the arc 
		     will not become nonloop carried.  If the distance is 
		     not known, if the two opers are in the same iteration, 
		     there was no inner loop carried dependence possible
		     and so the arc will not become nonloop carried.  In 
		     all other cases, the arc becomes non loop carried.
		     Leave inner carried set and set distance unknown. We 
		     leave inner carried because the prologue opers have
		     been moved out of the inner loop and now may or may
		     not be in the "outer" loop which carried the 
		     dependence. If the arc does not become non loop
		     carried, the minimum distance for the new "inner"
		     loop becomes 1. */
		  if (!IS_DISTANCE_UNKNOWN (sync->info) &&
		      (src_iter_num + sync->dist != iter_num))
		    {
		      sync->info = SET_DISTANCE_UNKNOWN (sync->info);
		      sync->dist = 1;
		      src_sync->info = SET_DISTANCE_UNKNOWN (src_sync->info);
		      src_sync->dist = 1;
		    }
		  else if (src_iter_num == iter_num)
		    {
		      sync->info = SET_DISTANCE_UNKNOWN (sync->info);
		      sync->dist = 1;
		      src_sync->info = SET_DISTANCE_UNKNOWN (src_sync->info);
		      src_sync->dist = 1;
		    }
		  else
		    {
		      sync->info = SET_NONLOOP_CARRIED (sync->info) |
			SET_DISTANCE_UNKNOWN (0);
		      sync->dist = 0;
		      src_sync->info = SET_NONLOOP_CARRIED (src_sync->info) |
			SET_DISTANCE_UNKNOWN (0);
		      src_sync->dist = 0;
		    }
		}
	      /* CASE 6: non loop and inner loop carried */
	      else if (IS_NONLOOP_CARRIED (sync->info) &&
		       IS_INNER_CARRIED (sync->info) &&
		       !IS_OUTER_CARRIED (sync->info))
		{

		  if (!IS_DISTANCE_UNKNOWN (sync->info))
		    {
		      L_punt ("Lpipe_adjust_syncs_for_prologue: "
			      "Sync arc is marked as both nonloop and "
			      "inner loop carried but distance unknown "
			      "flag not set - cb: %d, from_oper: %d, "
			      "to_oper: %d\n", prologue_cb->id, dep_oper->id,
			      oper->id);
		    }
		  /* Arc becomes non loop carried only if not inner
		     serloop.  If inner serloop is set, an outer loop
		     could be carrying the dependence.  That outer loop
		     could be the inner loop after prologue generation. */
		  if (!IS_INNER_SERLOOP (sync->info))
		    {
		      sync->info = sync->info & ~SET_INNER_CARRIED (0);
		      sync->info = sync->info & ~SET_DISTANCE_UNKNOWN (0);
		      sync->dist = 0;
		      src_sync->info =
			src_sync->info & ~SET_INNER_CARRIED (0);
		      src_sync->info =
			src_sync->info & ~SET_DISTANCE_UNKNOWN (0);
		      src_sync->dist = 0;
		    }
		}
	      /* CASE 7: non loop and inner and outer */
	      else if (IS_NONLOOP_CARRIED (sync->info) &&
		       IS_INNER_CARRIED (sync->info) &&
		       IS_OUTER_CARRIED (sync->info))
		{
		  /* Nothing needs to be done.  Arc remains conservative. */
		}
	      else
		{
		  L_punt ("Lpipe_adjust_syncs_for_prologue: "
			  "Invalid sync arc - at least one of "
			  "inner/outer/nonloop must be set - cb %d, "
			  "from_oper: %d, to_oper: %d, sync info: %d\n",
			  prologue_cb->id, dep_oper->id, oper->id,
			  sync->info);
		}
	    }
	}
    }
  /* We have not yet considered whether the arcs were forward or
     backward in the cb.  Remove/adjust backward arcs which are not
     possible. */
  L_adjust_invalid_sync_arcs_in_cb (prologue_cb);
}


/* Sync arcs are automatically copied when opers are copied.  Thus, during
   epilogue generation, all necessary sync arcs are generated.  However,
   unecessary sync arcs are also generated between the kernel and the
   epilogue.  This routine removes unnecessary arcs to reduce sync arc
   explosion.  The sync arcs between opers in the epilogue need to
   be adjusted so that they are correct for possible later passes of 
   acyclic scheduling on the epilogue cb.  I am not attempting to adjust 
   the sync arcs from the kernel to the epilogue or between the epilogue 
   and other cbs (including other epilogues) at this point. */

void
Lpipe_adjust_syncs_for_epilogue (L_Cb * epilogue_cb)
{
  L_Oper *oper;			/* oper at source of dependence */
  L_Oper *dep_oper;		/* oper at sink of dependence */
  Softpipe_Op_Info *softpipe_info;	/* info for source of dependence */
  Softpipe_Op_Info *dep_softpipe_info;	/* info for sink of dependence */
  L_Sync_Info *sync_info;
  L_Sync *sync, *dep_sync;
  L_Cb *dest_cb;		/* cb containing oper at sink of dependence */
  int i;
  int iter_num;
  int dep_iter_num;

  for (oper = epilogue_cb->first_op; oper != NULL; oper = oper->next_op)
    {

      if (oper->sync_info == NULL)
	continue;

      sync_info = oper->sync_info;

      /* Look at the outgoing sync arcs for each oper. */

      for (i = 0; i < sync_info->num_sync_out; i++)
	{
	  sync = sync_info->sync_out[i];

	  dep_oper = sync->dep_oper;

	  dest_cb = L_oper_hash_tbl_find_cb (L_fn->oper_hash_tbl,
					     dep_oper->id);

	  /* Remove any arcs which go from the epilogue to the header_cb.
	     Such dependences are impossible. */
	  if (dest_cb == header_cb)
	    {
	      L_delete_head_sync (oper, sync);
	      L_find_and_delete_tail_sync (dep_oper, oper);
	      /* need to decrement i since deleted the sync */
	      i--;
	    }

	  if (dest_cb == epilogue_cb)
	    {

	      /* Have found a sync arc between 2 opers which are
	         both in the epilogue.  Check if adjustment of the
	         arc is needed.  Do this by checking the inner/outer/nonloop 
	         flags for the sync arc.  At least one of the flags must 
	         be set, so there are 7 valid combinations of the 3 flags. */

	      softpipe_info = SOFTPIPE_OP_INFO (oper);
	      dep_softpipe_info = SOFTPIPE_OP_INFO (dep_oper);
	      iter_num = softpipe_info->unrolled_iter_num;
	      dep_iter_num = dep_softpipe_info->unrolled_iter_num;
	      dep_sync = L_find_tail_sync (dep_oper, oper);

	      /* CASE 1: outer loop carried only */
	      if (!IS_NONLOOP_CARRIED (sync->info) &&
		  !IS_INNER_CARRIED (sync->info) &&
		  IS_OUTER_CARRIED (sync->info))
		{
		  /* Set inner carried and distance unknown with minimum
		     distance 1. We set inner carried because the epilogue 
		     opers have been moved out of the inner loop and now 
		     may or may not now be in the "outer" loop which carried 
		     the dependence. */
		  sync->info = SET_INNER_CARRIED (sync->info) |
		    SET_DISTANCE_UNKNOWN (0);
		  sync->dist = 1;
		  dep_sync->info = SET_INNER_CARRIED (dep_sync->info) |
		    SET_DISTANCE_UNKNOWN (0);
		  dep_sync->dist = 1;
		}
	      /* CASE 2: non loop carried only */
	      else if (IS_NONLOOP_CARRIED (sync->info) &&
		       !IS_INNER_CARRIED (sync->info) &&
		       !IS_OUTER_CARRIED (sync->info))
		{
		  /* If the two opers are from different iterations of
		     the unrolled kernel, then the arc can be deleted. */
		  if (iter_num != dep_iter_num)
		    {
		      L_delete_head_sync (oper, sync);
		      L_find_and_delete_tail_sync (dep_oper, oper);
		      i--;	/* need to decrement i since deleted sync */
		    }
		}
	      /* CASE 3: non loop and outer loop carried */
	      else if (IS_NONLOOP_CARRIED (sync->info) &&
		       !IS_INNER_CARRIED (sync->info) &&
		       IS_OUTER_CARRIED (sync->info))
		{
		  /* If the two opers are from different iterations of
		     the unrolled kernel, then the non loop flag can be
		     cleared and the minimum distance becomes 1.  
		     Set inner carried and distance unknown
		     because the epilogue opers have been moved out of 
		     the inner loop. */
		  if (iter_num != dep_iter_num)
		    {
		      sync->info = sync->info & ~SET_NONLOOP_CARRIED (0);
		      sync->dist = 1;
		      dep_sync->info = dep_sync->info &
			~SET_NONLOOP_CARRIED (0);
		      dep_sync->dist = 1;
		    }
		  sync->info = SET_INNER_CARRIED (sync->info) |
		    SET_DISTANCE_UNKNOWN (0);
		  dep_sync->info = SET_INNER_CARRIED (dep_sync->info) |
		    SET_DISTANCE_UNKNOWN (0);
		}
	      /* CASE 4: inner loop carried only */
	      else if (!IS_NONLOOP_CARRIED (sync->info) &&
		       IS_INNER_CARRIED (sync->info) &&
		       !IS_OUTER_CARRIED (sync->info))
		{
		  if (sync->dist < 1)
		    {
		      L_punt ("Lpipe_adjust_syncs_for_epilogue: "
			      "Sync arc is marked as inner loop carried "
			      "but distance is less than 1 - cb: %d, "
			      "from_oper: %d, to_oper: %d\n",
			      epilogue_cb->id, oper->id, dep_oper->id);
		    }

		  /* If the distance is known and the two opers are not
		     the right distance apart in iterations, the arc 
		     can be deleted.  If the distance is not known,
		     if the two opers are in the same iteration, the
		     arc can be deleted.  In all other cases, the
		     arc becomes non loop carried only instead of inner
		     carried, because the epilogue does not iterate. */
		  if (!IS_DISTANCE_UNKNOWN (sync->info) &&
		      (iter_num + sync->dist != dep_iter_num))
		    {
		      L_delete_head_sync (oper, sync);
		      L_find_and_delete_tail_sync (dep_oper, oper);
		      i--;	/* deleted sync so need to decrement i */
		    }
		  else if (dep_iter_num == iter_num)
		    {
		      L_delete_head_sync (oper, sync);
		      L_find_and_delete_tail_sync (dep_oper, oper);
		      i--;	/* deleted sync so need to decrement i */
		    }
		  else
		    {
		      sync->info = SET_NONLOOP_CARRIED (sync->info) &
			~SET_INNER_CARRIED (0) & ~SET_DISTANCE_UNKNOWN (0);
		      sync->dist = 0;
		      dep_sync->info = SET_NONLOOP_CARRIED (dep_sync->info) &
			~SET_INNER_CARRIED (0) & ~SET_DISTANCE_UNKNOWN (0);
		      dep_sync->dist = 0;
		    }
		}
	      /* CASE 5: inner and outer loop carried */
	      else if (!IS_NONLOOP_CARRIED (sync->info) &&
		       IS_INNER_CARRIED (sync->info) &&
		       IS_OUTER_CARRIED (sync->info))
		{
		  if (sync->dist < 1)
		    {
		      L_punt ("Lpipe_adjust_syncs_for_epilogue: "
			      "Sync arc is marked as inner loop carried "
			      "but distance is less than 1 - cb: %d, "
			      "from_oper: %d, to_oper: %d\n",
			      epilogue_cb->id, oper->id, dep_oper->id);
		    }

		  /* If the distance is known and the two opers are not
		     the right distance apart in iterations, the arc 
		     will not become nonloop carried.  If the distance is 
		     not known, if the two opers are in the same iteration, 
		     there was no inner loop carried dependence possible
		     and so the arc will not become nonloop carried.  In 
		     all other cases, the arc becomes non loop carried.
		     Leave inner carried set and set distance unknown. We 
		     leave inner carried because the epilogue opers have
		     been moved out of the inner loop and now may or may
		     not be in the "outer" loop which carried the 
		     dependence. If the arc does not become non loop
		     carried, the minimum distance for the new "inner"
		     loop becomes 1. */
		  if (!IS_DISTANCE_UNKNOWN (sync->info) &&
		      (iter_num + sync->dist != dep_iter_num))
		    {
		      sync->info = SET_DISTANCE_UNKNOWN (sync->info);
		      sync->dist = 1;
		      dep_sync->info = SET_DISTANCE_UNKNOWN (dep_sync->info);
		      dep_sync->dist = 1;
		    }
		  else if (dep_iter_num == iter_num)
		    {
		      sync->info = SET_DISTANCE_UNKNOWN (sync->info);
		      sync->dist = 1;
		      dep_sync->info = SET_DISTANCE_UNKNOWN (dep_sync->info);
		      dep_sync->dist = 1;
		    }
		  else
		    {
		      sync->info = SET_NONLOOP_CARRIED (sync->info) |
			SET_DISTANCE_UNKNOWN (0);
		      sync->dist = 0;
		      dep_sync->info = SET_NONLOOP_CARRIED (dep_sync->info) |
			SET_DISTANCE_UNKNOWN (0);
		      dep_sync->dist = 0;
		    }
		}
	      /* CASE 6: non loop and inner loop carried */
	      else if (IS_NONLOOP_CARRIED (sync->info) &&
		       IS_INNER_CARRIED (sync->info) &&
		       !IS_OUTER_CARRIED (sync->info))
		{

		  if (!IS_DISTANCE_UNKNOWN (sync->info))
		    {
		      L_punt ("Lpipe_adjust_syncs_for_epilogue: "
			      "Sync arc is marked as both nonloop and "
			      "inner loop carried but distance unknown "
			      "flag not set - cb: %d, from_oper: %d, "
			      "to_oper: %d\n", epilogue_cb->id, oper->id,
			      dep_oper->id);
		    }
		  /* Arc becomes non loop carried only if not inner
		     serloop.  If inner serloop is set, an outer loop
		     could be carrying the dependence.  That outer loop
		     could be the inner loop after prologue generation. */
		  if (!IS_INNER_SERLOOP (sync->info))
		    {
		      sync->info = sync->info & ~SET_INNER_CARRIED (0);
		      sync->info = sync->info & ~SET_DISTANCE_UNKNOWN (0);
		      sync->dist = 0;
		      dep_sync->info =
			dep_sync->info & ~SET_INNER_CARRIED (0);
		      dep_sync->info =
			dep_sync->info & ~SET_DISTANCE_UNKNOWN (0);
		      dep_sync->dist = 0;
		    }
		}
	      /* CASE 7: non loop and inner and outer */
	      else if (IS_NONLOOP_CARRIED (sync->info) &&
		       IS_INNER_CARRIED (sync->info) &&
		       IS_OUTER_CARRIED (sync->info))
		{
		  /* Nothing needs to be done.  Arc remains conservative. */
		}
	      else
		{
		  L_punt ("Lpipe_adjust_syncs_for_epilogue: "
			  "Invalid sync arc - at least one of "
			  "inner/outer/nonloop must be set - cb %d, "
			  "from_oper: %d, to_oper: %d, sync info: %d\n",
			  epilogue_cb->id, oper->id, dep_oper->id,
			  sync->info);
		}
	    }
	}
    }
  /* We have not yet considered whether the arcs were forward or
     backward in the cb.  Remove/adjust backward arcs which are not
     possible. */
  L_adjust_invalid_sync_arcs_in_cb (epilogue_cb);
}
