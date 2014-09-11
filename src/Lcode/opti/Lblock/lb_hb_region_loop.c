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
 *      File :          l_region_loop.c
 *      Description :   Identify regions in loop for hyperblock formation,
 *      Creation Date : February 1994
 *      Authors :       Scott Mahlke
 *       Included with Lblock in its original form from Lhyper -- KMC 4/98 
 *        wirth minor changes for traceregion support
 *
 *==========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "lb_hb_hyperblock.h"
#include "lb_hb_peel.h"

#undef DEBUG_REGION
#undef PRINT_REGION_SUMMARY

#define LB_MAX_LOOP_SIZE_TO_CONSIDER	180

#if 0
/* JWS removing 20021207 --- this never worked well anyway */

/*
 * LB_hb_simple_loop_region_formation
 * ----------------------------------------------------------------------
 * When normal path enumeration and selection fails, default to
 * this simple algorithm to form loop regions.
 */

static LB_TraceRegion *
LB_hb_simple_loop_region_formation (L_Cb *header_cb, L_Cb *exit_cb,
				    Set loop_cb, 
				    LB_TraceRegion_Header *header)
{
  Set blocks = NULL;
  LB_TraceRegion *tr = NULL;

  L_warn ("Lblock: Using simple loop region former on loop with header %d",
	  header_cb->id);

#ifdef DEBUG_REGION
  fprintf (stderr, "Enter LB_hb_simple_loop_region_formation (header %d)\n",
	   header_cb->id);
  Set_print (stderr, "loop_cb", loop_cb);
#endif

  LB_hb_cb_info = (LB_Cb_Info *) Lcode_malloc (sizeof (LB_Cb_Info) *
					       (L_fn->max_cb_id + 1));
  LB_hb_find_cb_info (L_fn);

  LB_hb_select_blocks (header_cb, header_cb, &blocks, loop_cb, 1, 1.0,
		       LB_BLOCK_SEL_NULL, header);

  Lcode_free (LB_hb_cb_info);

  LB_hb_cb_info = NULL;

  if (blocks && (Set_size(blocks) > 1) && Set_in (blocks, exit_cb->id))
    {
      tr = LB_create_traceregion (L_fn, header->next_id++, 
				  header_cb, blocks,
				  L_TRACEREGION_LOOP);

      if (!LB_traceregion_is_subsumed (tr, header))
	{
#ifdef DEBUG_REGION
	  fprintf (stderr, "Hyperblock formed (header %d)\n", header_cb->id);
	  fprintf (stderr, "\t");
	  Set_print (stderr, "blocks", blocks);
#endif

	  tr->slots_used = LB_hb_curr_slots_used;
	  tr->slots_avail = LB_hb_curr_slots_avail;
	  tr->dep_height = LB_hb_curr_dep_height;
	}
      else
	{
	  LB_free_traceregion (tr);
	  tr = NULL;
	}
    }

  Set_dispose (blocks);
  return tr;
}
#endif


/*
 * LB_subgraph_chokepoints
 * ----------------------------------------------------------------------
 * Returns a sorted list of the upward chokepoints between start_cb and 
 * end_cb (dominators of end_cb in topological order), inclusive, within
 * the specified set of blocks.  It is of course the caller's responsibility
 * to free the list.
 */
static List
LB_subgraph_chokepoints (L_Func *fn, L_Cb *start_cb, L_Cb *end_cb, Set blocks)
{
  int i, j, tmp, n_ckpt, *ckpt_arr = NULL;
  Set ckpt_set = NULL;
  List ckpt_list = NULL;
  L_Cb *cb;

  ckpt_set = Set_intersect (L_get_cb_DOM_set (end_cb), blocks);
  ckpt_set = Set_add (ckpt_set, start_cb->id);
  ckpt_set = Set_add (ckpt_set, end_cb->id);
  n_ckpt = Set_size (ckpt_set);
  ckpt_arr = alloca (n_ckpt * sizeof (int));
  Set_2array (ckpt_set, ckpt_arr);
  for (i = 0; i < n_ckpt; i++)
    {
      cb  = L_cb_hash_tbl_find (fn->cb_hash_tbl, ckpt_arr[i]);
      for (j = i+1; j < n_ckpt; j++)
	{
	  if (L_in_cb_DOM_set (cb, ckpt_arr[j]))
	    {
	      tmp = ckpt_arr[i];
	      ckpt_arr[i] = ckpt_arr[j];
	      ckpt_arr[j] = tmp;
	      cb = L_cb_hash_tbl_find (fn->cb_hash_tbl, ckpt_arr[i]);
	    }
	}

      ckpt_list = List_insert_last (ckpt_list, cb);
    }

  Set_dispose (ckpt_set);
  
  return ckpt_list;
}


static LB_TraceRegion *
LB_hb_loop_path_selector (L_Func *fn, 
			  L_Cb *start_cb, L_Cb *end_cb, Set candidates,
			  LB_TraceRegion_Header *header)
{
  LB_TraceRegion *tr = NULL, *new_tr;
  int paths, failure = 0;
  List ckpts = NULL;
  L_Cb *cba, *cbb;
  int n_ckpts;

  /* Accuracy is lost when region is decomposed vertically at choke
   * points.  Try examining the region as a whole first.
   */

  if ((paths = LB_hb_find_all_paths (start_cb, end_cb, candidates)) &&
      !LB_hb_path_max_path_exceeded &&
      (tr = LB_hb_path_region_formation (start_cb, end_cb, candidates,
					 L_TRACEREGION_LOOP,
					 header)))
    return tr;

  ckpts = LB_subgraph_chokepoints (fn, start_cb, end_cb, candidates);

#ifdef DEBUG_REGION
  fprintf (stderr, "TRYING LOOPREGION hdr %d:", start_cb->id);
#endif

  n_ckpts = List_size (ckpts);

  if (n_ckpts == 0)
    L_punt ("LB_hb_loop_path_selector: at least one chokepoints required");
  else if (n_ckpts == 1)
    ckpts = List_insert_last (ckpts, List_first (ckpts));

  List_start (ckpts);
  cba = List_next (ckpts);
  while ((cbb = List_next (ckpts)))
    {
#ifdef DEBUG_REGION
      fprintf (stderr, "(bkt %d:%d)", cba->id, cbb->id);
#endif
      
      if (!(paths = LB_hb_find_all_paths (cba, cbb, candidates)) ||
	  LB_hb_path_max_path_exceeded ||
	  !(new_tr = LB_hb_path_region_formation (cba, cbb, candidates, 
						  L_TRACEREGION_LOOP,
						  header)))
	{
	  failure = 1;
	  break;
	}

#ifdef DEBUG_REGION
      fprintf (stderr, "(ht %d)", new_tr->dep_height);
#endif

      if (!tr)
	{
	  tr = new_tr;
	}
      else
	{
	  LB_TraceRegion *temp_tr;

	  temp_tr = LB_concat_seq_trs (header, tr, new_tr);
	  LB_free_traceregion (tr);
	  LB_free_traceregion (new_tr);
	  tr = temp_tr;
	}
      cba = cbb;
    }

#ifdef DEBUG_REGION
  fprintf (stderr, "\n");
#endif

  if (failure)
    {
      L_warn ("LB_hb_find_loop_regions: "
	      "Treating loop with header %d "
	      "as non-loop", start_cb->id);
  
      if (tr)
	{
	  LB_free_traceregion (tr);
	  tr =  NULL;
	}
    }

#ifdef DEBUG_REGION
  if (tr)
    LB_summarize_tr (stderr, tr);
#endif
	  
  if (ckpts)
    List_reset (ckpts);

  return tr;
}


/*=======================================================================*/
/*
 *    Main rountine
 */
/*=======================================================================*/

void
LB_hb_find_loop_regions (L_Func * fn, LB_TraceRegion_Header * header)
{
  int loop_count, exit_cb_id;
  L_Loop *loop;
  L_Cb *header_cb, *exit_cb;
  Set loop_done = NULL;
  LB_TraceRegion *tr = NULL;

  if (!LB_use_block_enum_selector)
    LB_hb_init_path_globals ();

#ifdef DEBUG_REGION
  fprintf (stderr, "> LOOP-REGION (fn %s)\n", fn->name);
#endif

  /* count how many loops there are */
  loop_count = 0;
  for (loop = fn->first_loop; loop; loop = loop->next_loop)
    loop_count++;

  /* do the loops first, inner to outer */

  while (Set_size (loop_done) < loop_count)
    {
      for (loop = fn->first_loop; loop != NULL; loop = loop->next_loop)
	{
	  if (Set_in (loop_done, loop->id) ||
	      !Set_subtract_empty (loop->nested_loops, loop_done))
	    continue;

#ifdef DEBUG_REGION
	  fprintf (stderr, "> LOOP-REGION (loop %d) (hdr cb %d)\n", 
		   loop->id, loop->header->id);
#endif
	  loop_done = Set_add (loop_done, loop->id);

	  header_cb = loop->header;

	  if (!header_cb)
	    L_punt ("LB_hb_find_loop_regions: loop has NULL header block");

#if 0
	  if ((loop->num_invocation < 1.0) ||
	      ((loop->header->weight / loop->num_invocation) < 2.0))
	    {
#ifdef DEBUG_REGION
	      fprintf (stderr, "  REJECT -- avg iter below threshold\n");
#endif
	      continue;
	    }
#endif

	  if (Set_size (loop->back_edge_cb) != 1)
	    L_punt ("LB_hb_find_loop_regions: multiple backedges in loop %d",
		    loop->id);

	  Set_2array (loop->back_edge_cb, &exit_cb_id);
	  exit_cb = L_cb_hash_tbl_find (fn->cb_hash_tbl, exit_cb_id);

	  /* CHECK: HEADER CB WEIGHT */

	  if ((header_cb->weight < LB_hb_min_cb_weight) &&
	      (!LB_make_zero_weight_regions))
	    {
#ifdef DEBUG_REGION
	      fprintf (stderr, "  REJECT -- hdr weight below threshold (w=%f)\n",
		       header_cb->weight);
#endif
	      continue;
	    }

	  /* CHECK: REGISTER JUMP */

	  if (L_EXTRACT_BIT_VAL (header_cb->flags, L_CB_HAS_JRG))
	    {
#ifdef DEBUG_REGION
	      fprintf (stderr, "  REJECT -- loop has jrg lp %d cb %d\n",
		       loop->id, header_cb->id);
#endif
	      continue;
	    }

	  /* CHECK: MAXIMUM LOOP SIZE (compilation time circuit breaker) */

	  if ((Set_size (loop->loop_cb) > LB_MAX_LOOP_SIZE_TO_CONSIDER) &&
	      (Set_size (loop->nested_loops) > 0))
	    {
#ifdef DEBUG_REGION
	      fprintf (stderr, "  REJECT -- loop too large (%d cbs)\n",
		       Set_size (loop->loop_cb));
#endif
	      continue;
	    }

	  /* SELECT LOOP PATHS */

	  if (LB_hb_select_all_blocks (header_cb) ||
	      LB_hb_select_exact_blocks (header_cb))
	    {
	      /* Hand annotations */

	      tr = LB_hb_select_trivial (L_TRACEREGION_LOOP,
					 header_cb, exit_cb, loop->loop_cb, 
					 header->next_id++);
	    }
	  else if (LB_use_block_enum_selector)
	    {
	      /* Cheap block-based selector */

	      tr = LB_block_enumeration_selector (L_TRACEREGION_LOOP, 
						  header_cb, exit_cb,
						  loop->loop_cb,
						  header->next_id++);
	    }
	  else
	    {
	      /* Path-based selector */

	      tr = LB_hb_loop_path_selector (fn, header_cb, exit_cb, 
					     loop->loop_cb, header);

	      if (!tr)
		{
		  /* Loop collapsing
		   * -----------------------------------------------------
		   * If loop is a simple outer loop around an existing
		   * HB loop region, and the inner loop is of relatively
		   * low tripcount, collapse the loops into one.
		   */

		  if (LB_hb_do_loop_collapsing && 
		      LB_hb_loop_collapsible (header, loop))
		    {
		      L_Loop *result_loop;
#ifdef DEBUG_REGION
		      L_warn ("  COLLAPSING loop %d\n", loop->id);
#endif
		      LB_hb_deinit_path_globals ();

		      result_loop = LB_hb_do_collapse_loops (header,
							     fn, loop);
		      
		      if (!result_loop)
			L_punt ("Loop collapsing failed.");

		      /* succeeded */
		      L_check_func (fn);

		      L_do_flow_analysis (fn, DOMINATOR | POST_DOMINATOR |
					  LIVE_VARIABLE);
		      LB_elim_loop_backedges (fn, result_loop);
		      L_rebuild_src_flow (fn);

#ifdef DEBUG_REGION
		      DB_spit_func (fn,"POSTCOL");
#endif
		      /* Flag retry of inner loop, now with
		       * outer loop collapsed into it
		       */
		      loop_done = Set_delete (loop_done, result_loop->id);
		      
		      LB_hb_init_path_globals ();
		    }
		  else
		    {
#ifdef DEBUG_REGION
		      fprintf (stderr, "  REJECT -- has no paths\n");
#endif
		    }
		  continue;
		}
	    }

	  if (tr)
	    {
#ifdef DEBUG_REGION
	      fprintf (stderr, "  ACCEPT -- Created traceregion loop %d\n",
		       loop->id);
	      LB_summarize_tr (stderr, tr);
#endif
	      header->traceregions = 
		List_insert_last (header->traceregions, tr);
	      if (LB_hb_peel_enable)
		LB_hb_consider_loop_for_peeling (loop, header);
	    }
	}
    }
  
#ifdef PRINT_REGION_SUMMARY
  LB_summarize_traceregions (stderr, header);
#endif
  if (!LB_use_block_enum_selector)
    LB_hb_deinit_path_globals ();
  Set_dispose (loop_done);

  return;
}
