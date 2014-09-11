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
 *      File :          l_region_hammock
 *      Description :   Identify hammocks from non-loop regions 
 *      Creation Date : February 1994
 *      Authors :       Scott Mahlke
 *       Included with Lblock in its original form from Lhyper -- KMC 4/98 
 *        wirth minor changes for traceregion support
 *
 *==========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "lb_hb_hyperblock.h"

#define LB_HAMMOCK_NEEDS_EXPANSION               0x00000001

#define DEBUG_HAMMOCK
#undef DEBUG_REGION
#undef PRINT_HAMMOCK_SUMMARY
#undef PRINT_REGION_SUMMARY

#define LB_HB_SELECT_IMPROPER_HAMMOCKS
#undef LB_CONCAT_HAMMOCKS

typedef struct _LB_Hammock {
  int flags;
  L_Cb *start_cb, *end_cb;
  Set blocks;

  LB_TraceRegion *tr;
} LB_Hammock;

static LB_Hammock *LB_hb_new_hammock (L_Cb *, L_Cb *);
static void LB_hb_delete_hammock (LB_Hammock *);
static void LB_hb_delete_all_hammocks (void);
static LB_Hammock *LB_hb_find_hammock (L_Cb *, L_Cb *);
static int LB_hb_valid_hammock (LB_Hammock * hammock);
static void LB_remove_subsumed_hammock_traceregions (LB_TraceRegion_Header * header);

#ifdef DEBUG_HAMMOCK
static void LB_hb_print_hammock (FILE *, LB_Hammock *);
#endif

#ifdef PRINT_HAMMOCK_SUMMARY
static void LB_hb_print_hammock_list (FILE *);
#endif

/*===========================================================================*/
/*
 *    Global variables
 */
/*===========================================================================*/

static L_Alloc_Pool *LB_hb_hammock_pool = NULL;
static List LB_hb_hammocks = NULL;
static Set LB_hb_inner_cb = NULL;
static Set LB_hb_choke_cb = NULL;

/*===========================================================================*/
/*
 *      LB_Hammock creation/deletion routines
 */
/*===========================================================================*/

/* recursive routine!! */
static void
LB_hb_add_hammock_blocks (L_Cb * cb, L_Cb * end, LB_Hammock * hammock)
{
  L_Flow *flow;
  L_Cb *dst_cb;
  int hdr_id = hammock->start_cb->id;

  for (flow = cb->dest_flow; flow; flow = flow->next_flow)
    {
      dst_cb = flow->dst_cb;

      if (!L_in_cb_DOM_set (dst_cb, hdr_id))
	continue;
      if (!Set_in (hammock->blocks, dst_cb->id))
	{
	  hammock->blocks = Set_add (hammock->blocks, dst_cb->id);
	  LB_hb_add_hammock_blocks (dst_cb, end, hammock);
	}
    }
}


static LB_Hammock *
LB_hb_new_hammock (L_Cb * start_cb, L_Cb * end_cb)
{
  LB_Hammock *hammock;

  if (!LB_hb_hammock_pool)
    LB_hb_hammock_pool =
      L_create_alloc_pool ("LB_Hammock", sizeof (LB_Hammock), 16);

  hammock = L_alloc (LB_hb_hammock_pool);
  hammock->flags = 0;
  hammock->start_cb = start_cb;
  hammock->end_cb = end_cb;
  hammock->blocks = NULL;
  hammock->tr = NULL;

  hammock->flags = L_SET_BIT_FLAG (hammock->flags, LB_HAMMOCK_NEEDS_EXPANSION);

  /* Find blocks belonging to this hammock */

  hammock->blocks = Set_add (hammock->blocks, start_cb->id);

  if (L_in_cb_DOM_set (end_cb, start_cb->id))
    hammock->blocks = Set_add (hammock->blocks, end_cb->id);

  if (start_cb != end_cb)
    LB_hb_add_hammock_blocks (start_cb, end_cb, hammock);

  if (!LB_hb_valid_hammock (hammock))
    {
      LB_hb_delete_hammock (hammock);
      hammock = NULL;
    }

  return (hammock);
}


static void
LB_hb_delete_hammock (LB_Hammock * hammock)
{
  if (!hammock)
    return;

  if (hammock->blocks)
    hammock->blocks = Set_dispose (hammock->blocks);

  L_free (LB_hb_hammock_pool, hammock);

  return;
}


static void
LB_hb_delete_all_hammocks (void)
{
  LB_Hammock *ptr;

  List_start (LB_hb_hammocks);
  while ((ptr = (LB_Hammock *) List_next (LB_hb_hammocks)))
    LB_hb_delete_hammock (ptr);

  LB_hb_hammocks = List_reset (LB_hb_hammocks);

  return;
}


static LB_Hammock *
LB_hb_find_hammock (L_Cb * start, L_Cb * end)
{
  LB_Hammock *ptr;

  List_start (LB_hb_hammocks);
  while ((ptr = (LB_Hammock *) List_next (LB_hb_hammocks)))
    if ((ptr->start_cb == start) && (ptr->end_cb == end))
      return (ptr);

  return (NULL);
}


#if defined DEBUG_HAMMOCK || defined PRINT_HAMMOCK_SUMMARY
static void
LB_hb_print_hammock (FILE * F, LB_Hammock * hammock)
{
  fprintf (F, "HAM[%d:%d] ", hammock->start_cb->id, hammock->end_cb->id);
  Set_print (F, "blocks", hammock->blocks);
}
#endif

#ifdef PRINT_HAMMOCK_SUMMARY
static void
LB_hb_print_hammock_list (FILE * F)
{
  LB_Hammock *ptr;

  List_start (LB_hb_hammocks);
  while ((ptr = (LB_Hammock *) List_next (LB_hb_hammocks)))
    LB_hb_print_hammock (F, ptr);
  return;
}
#endif


static int
LB_hb_valid_hammock (LB_Hammock * hammock)
{
  int num_blocks, *buf, i, valid;
  L_Cb *cb;

  valid = 1;

  num_blocks = Set_size (hammock->blocks);
  if (num_blocks <= 0)
    L_punt ("LB_hb_valid_hammock: hammoc is empty!");

  if (num_blocks > LB_hb_max_cb_in_hammock)
    {
      valid = 0;
    }
  else if ((hammock->start_cb != hammock->end_cb) &&
	   (L_EXTRACT_BIT_VAL (hammock->end_cb->flags, L_CB_LOOP_HEADER)) &&
	   (hammock->end_cb->weight > 10000.0))
    {
      /* SAM 7-95 */
      /* Dont let the end_cb of a hammock be an important loop header.. */

      valid = 0;
    }
  else
    {
      buf = (int *) alloca (sizeof (int) * num_blocks);
      Set_2array (hammock->blocks, buf);

      for (i = 0; valid && (i < num_blocks); i++)
	{
	  cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, buf[i]);
	  if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK) ||
	      L_EXTRACT_BIT_VAL (cb->flags, L_CB_SUPERBLOCK))
	    valid = 0;

	  if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK_LOOP) &&
	      !L_find_attr (cb->attr, LB_MARKED_FOR_LOOP_PEEL))
	    valid = 0;

	  if (LB_hb_is_single_block_loop (cb) &&
	      !L_find_attr (cb->attr, LB_MARKED_FOR_LOOP_PEEL))
	    valid = 0;

	  /* For simple hammocks, check that all the cbs besides the start
	   * and end are "innermost blocks" */
	  if (LB_hb_form_simple_hammocks_only && 
	      (cb->id != hammock->start_cb->id) && 
	      (cb->id != hammock->end_cb->id) &&
	      !LB_hb_find_hammock (cb, cb))
	    valid = 0;

	  /* For now, hammock on StarCore cannot contain a JSR. */
	  if (M_arch == M_STARCORE && LB_hb_form_simple_hammocks_only)
	    {
	      L_Oper *oper;

	      for (oper = cb->first_op; oper; oper = oper->next_op)
		{
		  if (M_starcore_subroutine_call (oper->opc))
		    {
		      valid = 0;
		      break;
		    }
		}
	    }
	}
    }

  return valid;
}


static int
LB_hb_valid_traceregion (LB_TraceRegion *tr)
{
  if (tr == NULL)
    return 0;

  /* topo sort the graph */
  Graph_topological_sort (FlowGraph (tr));

  /* dom, post dom, and imm post dom info */
  Graph_dominator (FlowGraph (tr));
  Graph_post_dominator (FlowGraph (tr));
  Graph_imm_post_dominator (FlowGraph (tr));

  /* control dependencies */
  Graph_control_dependence (FlowGraph (tr));

  /* equiv cd determination */
  Graph_equiv_cd (FlowGraph (tr));

  if ((tr->flow_graph->equiv_cds->size - 2) > LB_hb_available_predicates)
    return 0;
  else
    return 1;
}

/*===========================================================================*/
/*
 *    Intelligent region formation of non-loop codes
 */
/*===========================================================================*/

static void
LB_hb_find_choke_points (L_Func * fn)
{
  LB_hb_choke_cb = L_get_cb_PDOM_set (fn->first_cb);
  return;
}


/*
 *    We are looking for cb's with 1 flow exit who dont dominate
 *      their successor
 */

static void
LB_hb_find_innermost_blocks (L_Func * fn)
{
  L_Cb *cb, *succ_cb;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
#ifndef LB_HB_SELECT_IMPROPER_HAMMOCKS
      if (cb->dest_flow && !cb->dest_flow->next_flow)
	{
	  succ_cb = cb->dest_flow->dst_cb;
	  if (!L_in_cb_DOM_set (succ_cb, cb->id))
	    LB_hb_inner_cb = Set_add (LB_hb_inner_cb, cb->id);
	}
#else
      L_Flow *fl;
      for (fl = cb->dest_flow; fl; fl = fl->next_flow)
	{
	  succ_cb = fl->dst_cb;
	  if (!L_in_cb_DOM_set (succ_cb, cb->id))
	    {
	      LB_hb_inner_cb = Set_add (LB_hb_inner_cb, cb->id);
	      break;
	    }
	}
#endif
    }

  return;
}


static L_Cb *
LB_hb_find_immediate_dominator (L_Cb * cb)
{
  L_Flow *flow;
  L_Cb *src_cb;

  if (!cb)
    L_punt ("LB_hb_find_immediate_dominator: cb is NULL");

  for (flow = cb->src_flow; flow != NULL; flow = flow->next_flow)
    {
      src_cb = flow->src_cb;
      if (src_cb == cb)
	continue;
      if (L_in_cb_DOM_set (cb, flow->src_cb->id))
	return (flow->src_cb);
    }

  return (NULL);
}


static L_Cb *
LB_hb_find_immediate_post_dominator (L_Cb * cb)
{
  L_Flow *flow;
  L_Cb *dst_cb;

  if (!cb)
    L_punt ("LB_hb_find_immediate_post_dominator: cb is NULL");

  for (flow = cb->dest_flow; flow != NULL; flow = flow->next_flow)
    {
      dst_cb = flow->dst_cb;
      if (dst_cb == cb)
	continue;
      if (L_in_cb_PDOM_set (cb, dst_cb->id) &&
	  !L_subroutine_return_opcode (dst_cb->last_op))
	return dst_cb;
    }

  return NULL;
}


static LB_TraceRegion *
LB_hb_form_hammock_traceregion (LB_Hammock *ham, 
				LB_TraceRegion_Header *header)
{
  LB_TraceRegion *tr;
  fprintf(stderr,"CB(%d)\n",ham->start_cb->id);
  if (LB_hb_select_all_blocks (ham->start_cb) ||
      LB_hb_select_exact_blocks (ham->start_cb))
    {
      /* Hand annotations */
	  fprintf(stderr,"CB(%d)\n",ham->start_cb->id);
      tr = LB_hb_select_trivial (L_TRACEREGION_HAMMOCK,
				 ham->start_cb, ham->end_cb, 
				 ham->blocks, header->next_id++);
    }
  else if (LB_use_block_enum_selector)
    {
      tr = LB_block_enumeration_selector (L_TRACEREGION_HAMMOCK,
					  ham->start_cb, ham->end_cb,
					  ham->blocks, header->next_id++);
    }
  else
    {
      if (!LB_hb_find_all_paths (ham->start_cb, ham->end_cb,
				 ham->blocks))
	{
#ifdef DEBUG_REGION
	  fprintf (stderr, "Hammock doesnt have any paths\n");
#endif
	  return NULL;
	}
		 
      tr = LB_hb_path_region_formation (ham->start_cb, ham->end_cb,
					ham->blocks, L_TRACEREGION_HAMMOCK,
					header);
     }

  if (tr && !LB_hb_valid_traceregion (tr))
    {
      LB_free_traceregion (tr);
      tr = NULL;
    }

  if (tr)
    {
#ifdef DEBUG_HAMMOCK
      fprintf (stderr, "  HAM-REG TR %d [%d:%d]\n", tr->id, 
	       ham->start_cb->id, ham->end_cb->id);
      LB_summarize_tr (stderr, tr);
#endif

#ifdef DEBUG_REGION
      Graph_daVinci (tr->flow_graph, "GRAPHH", LB_bb_print_hook);
#endif

      ham->tr = tr;
    }

  return tr;
}


/* LB_hb_find_hammock_endpoints
 * ----------------------------------------------------------------------
 * s_cb is, or precedes e_cb in the flow graph.  Attempt to form a
 * hammock region containing s_cb and ending with, or before, e_cb.
 */
static int
LB_hb_find_hammock_endpoints (L_Cb * s_cb, L_Cb * e_cb, 
			      L_Cb ** h_start, L_Cb ** h_end)
{
  L_Cb *new_s_cb, *new_e_cb;
  Set visited = NULL;

  if (!s_cb->src_flow ||
      Set_in (LB_hb_choke_cb, s_cb->id) ||
      !(new_s_cb = LB_hb_find_immediate_dominator (s_cb)))
    return 0;

  new_e_cb = e_cb;

  while (1)
    {
      if (!L_in_cb_PDOM_set (new_s_cb, new_e_cb->id))
	{
	  if (Set_in (LB_hb_choke_cb, new_e_cb->id) ||
	      !(new_e_cb = LB_hb_find_immediate_post_dominator (new_e_cb)))
	    {
	      visited = Set_dispose (visited);
	      return 0;
	    }

	  if (Set_in (visited, new_e_cb->id))
	    {
	      L_warn ("LB_hb_find_hammock_endpoints: "
		      "found infinite loop in %s()\n", L_fn->name);
	      visited = Set_dispose (visited);
	      return 0;
	    }

	  visited = Set_add (visited, new_e_cb->id);
	}
      else if (!L_in_cb_DOM_set (new_e_cb, new_s_cb->id))
	{
#ifdef LB_HB_SELECT_IMPROPER_HAMMOCKS
	  break;
#else
	  if (Set_in (LB_hb_choke_cb, new_s_cb->id) ||
	      !(new_s_cb = LB_hb_find_immediate_dominator (new_s_cb)))
	    {
	      visited = Set_dispose (visited);
	      return 0;
	    }
#endif
	}
      else
	{
	  break;
	}
    }

  *h_start = new_s_cb;
  *h_end = new_e_cb;
  visited = Set_dispose (visited);
  return 1;
}


static int
LB_hb_expand_hammocks (LB_TraceRegion_Header *header, int ham_level)
{
  int new_ham = 0;
  LB_Hammock *hammock, *new_hammock;
  L_Cb *start_cb, *end_cb;
  LB_TraceRegion *tr;

  List_start_l (LB_hb_hammocks, ham_level);
  while ((hammock = (LB_Hammock *)List_next_l (LB_hb_hammocks, 
					       ham_level)))
    {
      if (!L_EXTRACT_BIT_VAL (hammock->flags, 
			      LB_HAMMOCK_NEEDS_EXPANSION))
	continue;
	      
      if (!LB_hb_find_hammock_endpoints (hammock->start_cb, 
					 hammock->end_cb,
					 &start_cb, &end_cb))
	continue;

      hammock->flags = L_CLR_BIT_FLAG (hammock->flags,
				       LB_HAMMOCK_NEEDS_EXPANSION);
		  
      if (LB_hb_find_hammock (start_cb, end_cb))
	continue;

      if (!(new_hammock = LB_hb_new_hammock (start_cb, end_cb)))
	continue;

      LB_hb_hammocks = List_insert_first (LB_hb_hammocks, new_hammock);

#ifdef DEBUG_HAMMOCK
      fprintf (stderr, "  HAM-NEW(EXP) ");
      LB_hb_print_hammock (stderr, new_hammock);
      fprintf (stderr, "  FROM HAM ");
      LB_hb_print_hammock (stderr, hammock);
#endif
      new_ham++;
		  
      if ((tr = LB_hb_form_hammock_traceregion (new_hammock, header)))
	header->traceregions = List_insert_last (header->traceregions, tr);
    }
	  
  return new_ham;
}


static int
LB_hb_merge_hammocks (LB_TraceRegion_Header * header, 
		      int merge_level1, int merge_level2)
{
  LB_Hammock *h1, *h2, *hn;
  int created = 0;

  if (!LB_hb_hammocks)
    return 0;

  List_start_l (LB_hb_hammocks, merge_level1);
  while ((h1 = List_next_l (LB_hb_hammocks, merge_level1)))
    {
      List_copy_current_ptr (LB_hb_hammocks, merge_level2, merge_level1);
      while ((h2 = List_next_l (LB_hb_hammocks, merge_level2)))
	{
	  L_Cb *new_start, *new_end;
	  LB_TraceRegion *r1, *r2, *newregion = NULL;

	  if (h1->end_cb == h2->start_cb)
	    {
#ifdef LB_HB_SELECT_IMPROPER_HAMMOCKS
	      if (!L_in_cb_DOM_set (h1->end_cb, h1->start_cb->id))
		continue;
#endif

	      new_start = h1->start_cb;
	      new_end = h2->end_cb;
	    }
	  else if (h1->start_cb == h2->end_cb)
	    {
#ifdef LB_HB_SELECT_IMPROPER_HAMMOCKS
	      if (!L_in_cb_DOM_set (h2->end_cb, h2->start_cb->id))
		continue;
#endif

	      new_start = h2->start_cb;
	      new_end = h1->end_cb;
	    }
	  else
	    {
	      continue;
	    }

	  if (LB_hb_find_hammock (new_start, new_end))
	    continue;

	  if (!(hn = LB_hb_new_hammock (new_start, new_end)))
	    continue;

#ifdef DEBUG_HAMMOCK
	  fprintf (stderr, "  HAM-NEW(MRG) [%d:%d],[%d:%d] ",
		   h1->start_cb->id, h1->end_cb->id,
		   h2->start_cb->id, h2->end_cb->id);
	  LB_hb_print_hammock (stderr, hn);
#endif

	  h1->flags =
	    L_CLR_BIT_FLAG (h1->flags, LB_HAMMOCK_NEEDS_EXPANSION);
	  h2->flags =
	    L_CLR_BIT_FLAG (h2->flags, LB_HAMMOCK_NEEDS_EXPANSION);

	  if ((r1 = h1->tr) && (r2 = h2->tr))
	    {
#ifdef LB_CONCAT_HAMMOCKS
	      newregion = LB_concat_seq_trs (header, r1, r2);
#else
	      newregion = LB_hb_form_hammock_traceregion (hn, header);
#endif
	      if (!newregion)
		{
		  LB_hb_delete_hammock (hn);
		}
	      else
		{
#ifdef DEBUG_HAMMOCK
		  fprintf (stderr, "  REGION %d FORMED BY EXP-MERGE FOR ", 
			   newregion->id);
		  LB_hb_print_hammock (stderr, hn);
		  LB_summarize_tr (stderr, newregion);
#endif

		  created++;
	      
		  hn->tr = newregion;
		  header->traceregions =
		    List_insert_last (header->traceregions, newregion);
		  LB_hb_hammocks = List_insert_last (LB_hb_hammocks, hn);

		  /* nuke regions corresponding to h1 and h2 */
	      
		  h1->tr = NULL;
		  header->traceregions =
		    List_remove (header->traceregions, r1);
		  LB_free_traceregion (r1);
		  LB_hb_delete_hammock (h1);
		  LB_hb_hammocks = List_delete_current_l (LB_hb_hammocks, 
							  merge_level1);
	      
		  h2->tr = NULL;
		  header->traceregions =
		    List_remove (header->traceregions, r2);
		  LB_free_traceregion (r2);
		  LB_hb_delete_hammock (h2);
		  LB_hb_hammocks = List_delete_current_l (LB_hb_hammocks, 
							  merge_level2);

		  break;
		}
	    }
	  else
	    {
	      LB_hb_delete_hammock (hn);
	    }
	}
    }
  return created;
}


/*
 *    Identify innermost hammocks, form region, proceed outward to largest
 *      hammocks to which a region can be legally formed
 */

void
LB_hb_find_hammock_regions (L_Func * fn, LB_TraceRegion_Header * header)
{
  int i, num_blocks, *buf, new_ham = 0;
  L_Cb *cb;
  LB_Hammock *new_hammock;

  if (!LB_use_block_enum_selector)
    LB_hb_init_path_globals ();



#ifdef DEBUG_HAMMOCK
  fprintf (stderr, "> HAMMOCK region selection for function %s\n", fn->name);
#endif

  buf = (int *) Lcode_malloc (sizeof (int) * fn->n_cb);

  LB_hb_find_choke_points (fn);
#ifdef DEBUG_HAMMOCK
  Set_print (stderr, "choke_points", LB_hb_choke_cb);
#endif
  LB_hb_find_innermost_blocks (fn);
#ifdef DEBUG_HAMMOCK
  Set_print (stderr, "inner_blocks", LB_hb_inner_cb);
#endif

  /* make hammocks out of inner cb's */

  num_blocks = Set_size (LB_hb_inner_cb);
  Set_2array (LB_hb_inner_cb, buf);
  for (i = 0; i < num_blocks; i++)
    {
      cb = L_cb_hash_tbl_find (fn->cb_hash_tbl, buf[i]);
      if (!(new_hammock = LB_hb_new_hammock (cb, cb)))
	continue;

     // new_hammock->start_cb->attr=L_concat_attr(new_hammock->start_cb->attr,L_new_attr(HB_SELECT_ALL_ATTR,0));
      //if(L_find_attr (new_hammock->start_cb->attr, HB_SELECT_ALL_ATTR))
      //fprintf(stderr,"SSSSSSSSSSSSSS(%d)\n",new_hammock->start_cb->id);
      /*L_Attr * attr;
      fprintf(stderr,"CB(%d)\n",new_hammock->start_cb->id);
      for (attr = new_hammock->start_cb->attr; attr != NULL; attr = attr->next_attr)
              {
                fprintf (stderr, "\n\t");
                L_print_attr(stderr,new_hammock->start_cb->attr);
              }
	*/
      LB_hb_hammocks = List_insert_last (LB_hb_hammocks, new_hammock);
      new_ham ++;

#ifdef DEBUG_HAMMOCK
      fprintf (stderr, "  HAM-NEW(INN) ");
      LB_hb_print_hammock (stderr, new_hammock);
#endif
    }

  if (LB_hb_hammocks)
    {
      int l1, l2, l3;

      l1 = List_register_new_ptr (LB_hb_hammocks);
      l2 = List_register_new_ptr (LB_hb_hammocks);
      l3 = List_register_new_ptr (LB_hb_hammocks);

      while (new_ham)
	{
#ifdef DEBUG_HAMMOCK
	  fprintf (stderr, ">>>HAM START EXPAND ROUND\n");
#endif  
	  new_ham = LB_hb_expand_hammocks (header, l1);

#ifdef DEBUG_HAMMOCK
	  fprintf (stderr, ">>>HAM START MERGE ROUND\n");
#endif
	  new_ham += LB_hb_merge_hammocks (header, l2, l3);
	}

      List_free_all_ptrs (LB_hb_hammocks);

#ifdef PRINT_HAMMOCK_SUMMARY
      LB_hb_print_hammock_list (stderr);
#endif

      LB_remove_subsumed_hammock_traceregions (header);

#ifdef PRINT_REGION_SUMMARY
      LB_summarize_traceregions (stderr, header);
#endif

      LB_hb_delete_all_hammocks ();
    }

  /* free up memory used by this routine */
      
  LB_hb_inner_cb = Set_dispose (LB_hb_inner_cb);

  if (!LB_use_block_enum_selector)
    LB_hb_deinit_path_globals ();

  Lcode_free (buf);
  return;
}


static void
LB_remove_subsumed_hammock_traceregions (LB_TraceRegion_Header * header)
{
  LB_TraceRegion *tr;

  List_start (header->traceregions);
  while ((tr = (LB_TraceRegion *) List_next (header->traceregions)))
    {
      if (!(tr->flags & L_TRACEREGION_HAMMOCK))
	continue;
      if (LB_traceregion_is_subsumed (tr, header))
	{
#ifdef DEBUG_HAMMOCK
	  fprintf (stderr, "REMOVING SUBSUMED HAMMOCK TRACEREGION:\n");
	  LB_summarize_tr (stderr, tr);
#endif
	  header->traceregions = List_remove (header->traceregions, tr);
	  LB_free_traceregion (tr);
	}
    }
}
