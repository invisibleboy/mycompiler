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
 *      File :          lb_hb_peel.c
 *      Description :   Loop peeling
 *      Creation Date : February 1994
 *      Authors :       Scott Mahlke, David August, John Sias
 *
 *      Loop peeling driver for hyperblock formation.
 *==========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "lb_hb_hyperblock.h"
#include "lb_b_internal.h"
#include "lb_peel.h"
#include "lb_hb_peel.h"

#define DBG_LB_PEEL 0

#define LB_IGNORE_EXCLUDED_NESTS 1

/* PARAMETERS
 * ----------------------------------------------------------------------
 *
 * LB_hb_peel_infinity_iter: Can peel 0 to this value, so invocations
 *    which iterate >= this value will always go to the recovery loop
 * 
 * LB_hb_peel_max_ops: Maximum size the peeled loop can become
 *
 * LB_hb_peel_min_overall_coverage: Minimum fraction of the dynamic
 *    invocations of the loop the peeled loop must contain for all
 *    invocations of the loop 
 *
 * LB_hb_peel_min_peelable_coverage: Minimum fraction of the dynamic
 *    invocations of the loop the peeled loop must contain for all
 *    invocations of the loop
 *
 * LB_hb_peel_inc_peelable_coverage: Minimum coverage to consider
 *    bumping up the number of peels 
 */


int
LB_hb_is_hand_marked_for_peel (L_Loop * loop)
{
  L_Attr *attr;

  attr = L_find_attr (loop->header->attr, LB_HAND_MARKED_FOR_LOOP_PEEL);
  return (attr != NULL);
}

static int
LB_hb_get_hand_marked_peel_amount (L_Loop * loop)
{
  int val;
  L_Operand *field;
  L_Attr *attr;

  if (!(attr = L_find_attr (loop->header->attr, LB_HAND_MARKED_FOR_LOOP_PEEL)))
    return 0;

  if (!(field = L_find_attr_field (attr, L_OPERAND_INT)))
    L_punt ("LB_hb_get_hand_marked_peel_amount: amount not specified in attr");

  val = field->value.i;
  if (val <= 0)
    L_punt ("LB_hb_get_hand_marked_peel_amount: "
	    "illegal num peel specified in attr (%d)", val);

  return (val);
}


static int
LB_hb_get_peel_amount (L_Loop * loop)
{
  int val;
  L_Operand *field;
  L_Attr *attr;

  if (!(attr = L_find_attr (loop->header->attr, LB_MARKED_FOR_LOOP_PEEL)))
    return 0;

  if (!(field = L_find_attr_field (attr, L_OPERAND_INT)))
    L_punt ("LB_hb_get_peel_amount: amount not specified in attr");

  val = field->value.i;
  if (val <= 0)
    L_punt ("LB_hb_get_peel_amount: illegal num peel specified in attr (%d)",
	    val);

  return (val);
}


/*
 *    Heuristic to calcuate the number of times a loop should be peeled
 */
int
LB_hb_find_num_peel (L_Loop * loop, Set peel_cb)
{
  int i, iter_indx, last_num_peel = 0, cur_num_peel, iter_size;
  L_Cb *header;
  L_Attr *attr;
  double num_iter, lp_invoc_cnt, tot_peelable_iter,
    overall_coverage, peelable_coverage, inc_peelable_coverage,
    *iter_count = (double *) alloca ((LB_hb_peel_infinity_iter + 1) *
				     sizeof (double));

  header = loop->header;

  /* See if the user premarked the loop for peeling */
  if (LB_hb_is_hand_marked_for_peel (loop))
    return (LB_hb_get_hand_marked_peel_amount (loop));

  /* For now, don't peel loops without iteration profile info! */
  if (!(attr = L_find_attr (header->attr, ITERATION_INFO_HEADER)))
    {
#if DBG_LB_PEEL
      fprintf (stderr, "> PEEL-HEURISTIC Loop %d (cb %d)"
	       " No iter profile\n", loop->id, header->id);
#endif
      return 0;
    }
  
  if (LB_hb_jsr_in_cb_set (peel_cb))
    {
#if DBG_LB_PEEL
      fprintf (stderr, "> PEEL-HEURISTIC Loop %d (cb %d)"
	       " JSR prevents peel.\n", loop->id, header->id);
#endif
      return 0;
    }

  num_iter = lp_invoc_cnt = tot_peelable_iter = 0.0;
  for (i = 0; i <= LB_hb_peel_infinity_iter; i++)
    iter_count[i] = 0.0;
      
  for (attr = header->attr; attr != NULL; attr=attr->next_attr)
    {
      if (!strncmp (attr->name, ITERATION_INFO_PREFIX, 
		    ITERATION_INFO_PREFIX_LENGTH))
	{
	  iter_indx = atoi (&attr->name[ITERATION_INFO_PREFIX_LENGTH]);
	  num_iter = attr->field[0]->value.f2;
	  lp_invoc_cnt += num_iter;
	  if (iter_indx <= LB_hb_peel_infinity_iter)
	    {
	      tot_peelable_iter += num_iter;
	      iter_count[iter_indx] += num_iter;
	    }
	}
    }
  
  /* Heuristically select peels */
  
  iter_size = LB_hb_num_ops_in_cb_set (peel_cb);

  if (LB_hb_peel_heuristic == 0)
    {
      cur_num_peel = 0;
      overall_coverage = peelable_coverage = 0.0;
      
      while (cur_num_peel <= LB_hb_peel_infinity_iter)
	{
	  cur_num_peel++;
	  if (iter_count[cur_num_peel] == 0.0)
	    continue;

	  overall_coverage += 
	    (iter_count[cur_num_peel] / lp_invoc_cnt);
	  inc_peelable_coverage = 
	    iter_count[cur_num_peel] / tot_peelable_iter;
	  peelable_coverage += 
	    inc_peelable_coverage;

	  /* check for minimum coverage */
	  
	  if (overall_coverage < LB_hb_peel_min_overall_coverage)
	    continue;
	  if (inc_peelable_coverage < LB_hb_peel_inc_peelable_coverage)
	    continue;
	  if (peelable_coverage < LB_hb_peel_min_peelable_coverage)
	    continue;

	  /* apply op count constraint */

	  if ((cur_num_peel * iter_size) > LB_hb_peel_max_ops)
	    break;

	  /* found a valid peel factor!! */
	  last_num_peel = cur_num_peel;
	}
    }
  else if (LB_hb_peel_heuristic == 1)
    {
      double finishing, covered = 0.0;
      
      cur_num_peel = 0;
      overall_coverage = peelable_coverage = 0.0;

      for (cur_num_peel = 1; 
	   cur_num_peel <= LB_hb_peel_infinity_iter;
	   cur_num_peel++)
	{
	  finishing = iter_count[cur_num_peel] / lp_invoc_cnt;
	  
	  /* P(useful) = (1 - covered) */

	  if ((1 - covered) <= LB_hb_peel_min_util)
	    break;

	  if ((cur_num_peel * iter_size) > LB_hb_peel_max_ops)
	    break;

	  last_num_peel = cur_num_peel;

	  covered += finishing;
	}
      
      if (covered < LB_hb_peel_min_overall_coverage)
	last_num_peel = 0;
    }
  else
    {
      L_punt ("Invalid peeling heuristic");
    }

#if DBG_LB_PEEL
  if (last_num_peel)
    {
      double finishing, useful, covered = 0.0;
      fprintf (stderr, "> PEEL-HEURISTIC(%d) Loop %d (cb %d) PEEL %d\n", 
	       LB_hb_peel_heuristic, loop->id, header->id, last_num_peel);
      fprintf (stderr, "  lp_invoc_cnt %f : tot_peelable_iter %f\n  ",
	       lp_invoc_cnt, tot_peelable_iter);
      Set_print (stderr, "cbs", peel_cb);
      fprintf (stderr, "  ops = %d\n", iter_size);
      for (i = 1; i <= LB_hb_peel_infinity_iter; i++)
	{
	  finishing = iter_count[i] / lp_invoc_cnt;
	  useful = 1 - covered;
	  covered += finishing;
	  fprintf (stderr, " %3d: %8.3e %8.2f incr %8.2f util %8.2f cov\n", 
		   i, iter_count[i], finishing, useful, covered);
	}
    }
  else
    {
      fprintf (stderr, "> PEEL-HEURISTIC(%d) Loop %d (cb %d) -- no peels\n", 
	       LB_hb_peel_heuristic, loop->id, header->id);
    }
#endif

  /* All nested loops must be marked for peeling */
  if (!Set_empty (loop->nested_loops))
    {
      L_Loop *l;

      for (l = L_fn->first_loop; l; l = l->next_loop)
	{
	  if (Set_in (loop->nested_loops, l->id) &&
#if LB_IGNORE_EXCLUDED_NESTS
	      !Set_intersect_empty (peel_cb, l->loop_cb) &&
#endif
	      !L_find_attr (l->header->attr, LB_MARKED_FOR_LOOP_PEEL))
	    {
	      /* found a loop in the peel region not marked for peeling */
#if DBG_LB_PEEL
	      fprintf (stderr, "> PEEL-HEURISTIC Loop %d (cb %d)"
		       " Unpeelable nested loop prevents peeling\n", 
		       loop->id, header->id);
#endif
	      return 0;
	    }
	}
    }

  return (last_num_peel);
}


/*
 *    1. User can use "HB_peel" attribute to mark loop for peeling,
 *       along with the number of times the loop should be peeled.
 *    2. Identify loops as candidiates for peeling.  For this first
 *       run we will determine the number of peels up front for
 *       simplicity.
 */
int
LB_hb_consider_loop_for_peeling (L_Loop * loop,
				 LB_TraceRegion_Header * header)
{
  LB_TraceRegion *tr;
  Set tr_set;
  int num_peel;

  if (!(tr = LB_find_traceregion_by_header (header, loop->header)))
    L_punt ("LB_hb_consider_loop_for_peeling: loop is not a region!");

  /* Make sure loop can indeed be peeled */

  tr_set = LB_return_cbs_region_as_set (tr);
  num_peel = LB_hb_find_num_peel (loop, tr_set);
  Set_dispose (tr_set);

  if (num_peel)
    {
      L_Attr *cb_attr;
      /* loop is valid for peeling, mark as such */
#if DBG_LB_PEEL
      fprintf (stderr, "  Loop %d (cb %d) marked for %d peel(s)\n",
	       loop->id, loop->header->id, num_peel);
#endif

      cb_attr = L_new_attr (LB_MARKED_FOR_LOOP_PEEL, 1);
      L_set_int_attr_field (cb_attr, 0, num_peel);
      LB_hb_mark_all_cbs_with_attr (loop->loop_cb, cb_attr);
      L_delete_attr (NULL, cb_attr);
    }

  return num_peel;
}


/*
 * LB_hb_do_loop_peel
 * ----------------------------------------------------------------------
 * Peel nested loops from traceregions. -- Added 20021009 JWS
 */
void
LB_hb_do_loop_peel (L_Func * fn, LB_TraceRegion * tr,
		    LB_TraceRegion_Header * tr_header)
{
  L_Loop *loop;
  L_Cb *header;
  int loop_count, num_peel, i, peel_id;
  Set tr_set, new_tr_set;
  Set considered_loops = NULL, peel_set = NULL, mod_cbs = NULL,
    residue_set = NULL;
#if DBG_LB_PEEL
  fprintf (stderr, "> PEEL Attempting to peel nested region from:\n");
  LB_summarize_tr (stderr, tr);
#endif

  tr_set = LB_return_cbs_region_as_set (tr);
  header = tr->header;

  /* 
   * ** First count how many loops there are.
   */

  for (loop = fn->first_loop, loop_count = 0; loop; loop = loop->next_loop)
    loop_count++;

  /*
   * ** Peel the inner most loop first.  Proced to outer most loop.
   */

  new_tr_set = Set_copy (tr_set);

  peel_id = 0;

  while (Set_size (considered_loops) < loop_count)
    {
      for (loop = fn->first_loop; loop; loop = loop->next_loop)
	{
	  LB_TraceRegion *peel_region;
	  Set peel_cbs = NULL, loop_peel_set = NULL;

	  /* Consider loops inner to outer and don't reconsider loops */

	  if (!Set_subtract_empty (loop->nested_loops, considered_loops))
	    continue;
	  if (Set_in (considered_loops, loop->id))
	    continue;

	  considered_loops = Set_add (considered_loops, loop->id);

	  /* header inside region */

	  if (!Set_in (new_tr_set, loop->header->id))
	    continue;

	  /* backedge cb inside region */

	  if (Set_intersect_empty (new_tr_set, loop->back_edge_cb))
	    continue;
	  
	  /* header is not the region header */

	  if (loop->header == tr->header)
	    continue;

	  /* If a region exists for the loop, peel only selected blocks */

	  if (LB_hb_peel_partial &&
	      (peel_region = LB_find_traceregion_by_header (tr_header, 
							    loop->header)) &&
	      (peel_region->flags & L_TRACEREGION_LOOP))
	    {
	      Set outer_select;

	      /*GAK 02/07/2003*/   
	      outer_select = Set_intersect (new_tr_set, loop->loop_cb);

	      peel_cbs = LB_return_cbs_region_as_set (peel_region);
	      peel_cbs = Set_union_acc (peel_cbs, outer_select);
#if 0
	      tr_header->traceregions = List_remove (tr_header->traceregions,
						     peel_region);
	      LB_free_traceregion (peel_region);
#endif
	      Set_dispose (outer_select);
	    }
	  else
	    {
	      peel_cbs = Set_copy (loop->loop_cb);
	    }

	  peel_cbs = Set_subtract_acc (peel_cbs, residue_set);

	  if ((num_peel = LB_hb_get_peel_amount (loop)) <= 0)
	    {
#if DBG_LB_PEEL
	      fprintf (stderr, "Loop was indicated to be peeled %d times.\n",
		       num_peel);
#endif
	      num_peel = 1;
	    }

	  {
	    int iter_size;
	    
	    iter_size = LB_hb_num_ops_in_cb_set (peel_cbs);
	    
	    if ((iter_size * num_peel) > LB_hb_peel_max_ops)
	      L_warn ("LB_hb_do_loop_peel: Peeling more than allowed!");
	  }

	  if (LB_hb_verbose_level >= 6)
	    {
	      fprintf (stderr, "> PEEL In TR %d (header cb %d) peeling "
		       "loop %d (header cb %d) %d times.\n",
		       tr->id, tr->header->id, 
		       loop->id, loop->header->id, num_peel);
	    }

	  for (i = 1; i <= num_peel; i++)
	    {
	      LB_peel_loop (fn, loop, peel_cbs, new_tr_set,
			    peel_id, i, &loop_peel_set, &mod_cbs);

	      new_tr_set = Set_union_acc (new_tr_set, loop_peel_set);
	    }

	  residue_set = Set_union_acc (residue_set, loop->loop_cb);
	  new_tr_set = Set_subtract_acc (new_tr_set, loop->loop_cb);

	  /*GAK 02/07/2003*/
          LB_update_traceregion (tr, fn, tr->header, new_tr_set);

	  peel_id++;

	  peel_cbs = Set_dispose (peel_cbs);
	  peel_set = Set_union_acc (peel_set, loop_peel_set);
	  loop_peel_set = Set_dispose (loop_peel_set);
	}
    }

  Set_dispose (considered_loops);
  Set_dispose (residue_set);

  new_tr_set = Set_union_acc (new_tr_set, peel_set);

  Set_dispose (peel_set);

  LB_update_traceregion (tr, fn, tr->header, new_tr_set);

  Set_dispose (new_tr_set);
  new_tr_set = LB_return_cbs_region_as_set (tr);

  {
    int level;
    Set other_tr_set;
    LB_TraceRegion *tr_indx;

    level = List_register_new_ptr (tr_header->traceregions);

    List_start_l (tr_header->traceregions, level);
    while ((tr_indx = (LB_TraceRegion *)
	    List_next_l (tr_header->traceregions, level)))
      {
	if (tr_indx == tr)
	  continue;

	other_tr_set = LB_return_cbs_region_as_set (tr_indx);

	if (!Set_intersect_empty (other_tr_set, mod_cbs) &&
	    !Set_subtract_empty (new_tr_set, other_tr_set))
	  {
	    L_warn ("LB_hb_do_loop_peel: Adjusting other traceregion");

	    other_tr_set = Set_subtract_acc (other_tr_set, tr_set);
	    other_tr_set = Set_union_acc (other_tr_set, new_tr_set);
	    
	    LB_update_traceregion (tr_indx, fn, tr_indx->header, other_tr_set);
	  }

	Set_dispose (other_tr_set);
      }
  }

#if DBG_LB_PEEL
  fprintf (stderr, "Region after peeling...\n");
  LB_summarize_tr (stderr, tr);
#endif

  Set_dispose (mod_cbs);
  Set_dispose (tr_set);
  Set_dispose (new_tr_set);

  return;
}
