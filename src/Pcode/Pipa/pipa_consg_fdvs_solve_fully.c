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



/*****************************************************************************\
 *
 *      File:    pipa_consg_fdvs_solve_fully.c
 *      Author:  Erik Nystrom
 *      Copyright (c) 2003  Erik Nystrom, Wen-mei Hwu
 *              and The Board of Trustees of the University of Illinois.
 *              All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#include "pipa_consg_fdvs.h"
#include "pipa_misc_utils.h"

#define min(a,b) (a<b?a:b)

static void process_assign (IPA_cgraph_edge_t * edge);
static void process_assign_addr (IPA_cgraph_edge_t * edge);
static void process_assign_deref (IPA_cgraph_edge_t * edge);
static void process_deref_assign (IPA_cgraph_edge_t * edge);
static void process_skew (IPA_cgraph_edge_t * edge);

static IPA_cgraph_edge_t *
ensure_edge (IPA_cgraph_edgelist_e edge_type,
	     IPA_cgraph_node_t * src_node, IPA_cgraph_node_t * dst_node,
	     IPA_cgraph_edge_data_t *edata,
	     IPA_cgraph_edge_t * src_edge, IPA_cgraph_edge_t * dst_edge);

static IPA_cgraph_t *cg;
static List work_list;
static int print = 0;

static List make_fi_list = NULL;
static List make_col_list = NULL;
static List unify_list = NULL;

void unification(IPA_cgraph_node_t *node);

int mismatches = 0;
int newedges = 0;
int excledges = 0;
int existedges = 0;
int apply_rn_cnt = 0;
int apply_f_rn_cnt = 0;

#define IN_WORK_LIST IPA_CG_EDGE_FLAGS_GENERIC2
#define IN_FI_LIST   IPA_CG_NODE_FLAGS_GENERIC9
#define IN_HPCAND_LIST  IPA_CG_NODE_FLAGS_GENERIC11
List hpcand_list = NULL;

static void
IPA_merge_test(IPA_cgraph_t * cur_consg);

static void
collapse_all_dups(IPA_cgraph_node_t *src_node);

/*****************************************************************************
 * 
 *****************************************************************************/

void
dbflush()
{
  fflush(stdout);
  fflush(stderr);
}

#if 1
static void
print_edge(IPA_cgraph_edge_t *e)
{
  IPA_cg_node_print(stdout,e->dst_elist->node, IPA_PRINT_ASCI);
  printf(" <- %p %14s %x %d.%d,%d,%d.%d -",
	 e,
	 edge_types[e->dst_elist->edge_type],
	 e->flags,
	 e->data.target_offset, 
	 e->data.target_stride, 
	 e->data.assign_size,
	 e->data.source_offset,
	 e->data.source_stride);
  IPA_cg_node_print(stdout,e->src_elist->node, IPA_PRINT_ASCI);
  printf("\n");
}
#endif

static void
add_to_worklist(IPA_cgraph_edge_t *edge)
{
  if (IPA_FLAG_ISSET(edge->flags, IN_WORK_LIST))
    return;
#if 0
  if (edge->src_elist->node->data.var_id == 1032 &&
      (edge->dst_elist->node->data.var_id == 9725 &&
       edge->dst_elist->node->data.version == 54))
    {
      printf("WORKLIST: ");
      print_edge(edge);
    }
#endif

  work_list = List_insert_last (work_list, edge);
  IPA_FLAG_SET(edge->flags, IN_WORK_LIST);
  newedges++;
}

static void
del_current_from_worklist(IPA_cgraph_edge_t *edge)
{
  assert(List_current(work_list) == edge);
  work_list = List_delete_current (work_list);
  IPA_FLAG_CLR(edge->flags, IN_WORK_LIST);
}

/*****************************************************************************
 *****************************************************************************/

void
IPA_consg_fdvs_solve_fully (IPA_cgraph_t * __cg, List __work_list)
{
  IPA_cgraph_edge_t *edge;
  int cnt = 0, iter = 0;
  double ltime = IPA_GetTime();

  cg = __cg;
  work_list = __work_list;
  print = 0;

  if (cg)
    IPA_cg_nodes_assert_clr_flags(cg, (IN_FI_LIST));
  
  /* The in-worklist flags is used to avoid some redundant 
     edge comparisons */
  List_start (work_list);
  while ((edge = List_next (work_list)))
    {
      if (IPA_FLAG_ISSET(edge->flags, IN_WORK_LIST))
	{
	  work_list = List_delete_current (work_list);
	  continue;
	}
      IPA_FLAG_SET(edge->flags, IN_WORK_LIST);
    }

  List_start (work_list);
  hpcand_list = NULL;
  make_fi_list = NULL;
  make_col_list = NULL;
  mismatches = 0;
  newedges = 0;
  excledges = 0;
  existedges = 0;
  while ((edge = List_next (work_list)))
    {
      del_current_from_worklist(edge);
      
      /* Because globals are connected to everything upfront, not-known-to-be-called
	 procedures may get explored if the worklist is left to its own devices.
       */
      if (!edge->src_elist->node->data.syminfo->fninfo->call_node ||
	  !edge->dst_elist->node->data.syminfo->fninfo->call_node)
	continue;

      if (cnt == 20000)
	{
	  double ntime = IPA_GetTime();
	  
	  DEBUG_IPA(2, printf("IPA_consg_fdvs_solve_fully: %d edges [%d]\n",
			      List_size(work_list), cnt););
	  if (iter == 0)
	    printf("\n");
	  printf("%0.2f %5d : %6d : mm %9d nw %6d dp %9d xc %6d \n",
		 ntime - ltime,
		 iter++, 
		 List_size(work_list),
		 mismatches,newedges,
		 existedges, excledges);
	  ltime = ntime;
	  cnt = 0;
	  mismatches = 0;
	  newedges = 0; 
	  apply_rn_cnt = 0;
	  apply_f_rn_cnt = 0;
	  excledges = 0;
	  existedges = 0;
	  fflush(stdout);
#if 0
	  if (iter > 15) exit(0);
#endif
	}
      cnt++;

#if 0
      if (edge->src_elist->node->data.var_id == 2188 &&
	  edge->src_elist->node->data.version == 25)
	{
	  printf("PROCESS WORKLIST: ");
	  print_edge(edge);
	}
#endif

      IPA_FLAG_SET(edge->flags, IPA_CG_EDGE_FLAGS_PROCESSED);
      CG_EDGE_CLRNEW(edge);

      switch (edge->src_elist->edge_type)
        {
        case ASSIGN:
          process_assign (edge);
          break;

        case ASSIGN_ADDR:
          process_assign_addr (edge);
          break;

        case ASSIGN_DEREF:
          process_assign_deref (edge);
          break;

        case DEREF_ASSIGN:
          process_deref_assign (edge);
          break;

	case SKEW:
          process_skew (edge);
          break;	  

        default:
          assert (0);
        }

      if (make_fi_list)
	{
	  IPA_cgraph_node_t *node;
	  List_start(make_fi_list);
	  while ((node = List_next(make_fi_list)))
	    {
	      work_list = IPA_cgraph_make_fi(node, work_list, IN_WORK_LIST);
	      IPA_FLAG_CLR(node->flags, IN_FI_LIST);
	    }
	  List_reset(make_fi_list);
	  make_fi_list = NULL;
	}
      if (make_col_list)
	{
	  IPA_cgraph_node_t *node;
	  List_start(make_col_list);
	  while ((node = List_next(make_col_list)))
	    {
	      collapse_all_dups(node);
	    }
	  List_reset(make_col_list);
	  make_col_list = NULL;
	}
      if (unify_list)
	{
	  IPA_cgraph_node_t *node;

	  List_start(unify_list);
	  while ((node = List_next(unify_list)))
	    unification(node);
	  List_reset(unify_list);
	  unify_list = NULL;	  
	  List_start(work_list);
	}

      if (IPA_gcon_option == IPA_GCON_ANDERSEN)
	IPA_merge_test(cg);
    }

  assert(List_size(work_list) == 0);
  List_reset (work_list);
  work_list = NULL;
  
  {
    IPA_cgraph_node_t *node;
    List_start(hpcand_list);
    while ((node = List_next(hpcand_list)))
      {
	IPA_FLAG_CLR(node->flags, IN_HPCAND_LIST);
      }
    List_reset(hpcand_list);
    hpcand_list = NULL;
  }

  assert(make_fi_list == NULL);

  DEBUG_IPA(2, printf("Done\n"););
}



/*****************************************************************************
 * 
 *****************************************************************************/

#if UNIFY
void
unification(IPA_cgraph_node_t *node)
{
  IPA_cgraph_edge_list_t *elist;
  IPA_HTAB_ITER eiter;
  off_class_t *oc;
  List oc_list = NULL;

  /* Find the classes of address edges that
     should be merged */
  elist = IPA_cg_edge_list_find(node, ASSIGN_ADDR);
  if (!elist)
    return;

#if 0 /* THIS IS FOR DAS */
  if (IPA_htab_size(elist->out) == 0)
    return;
#endif

  IPA_HTAB_START(eiter, elist->in);
  IPA_HTAB_LOOP(eiter)
    {
      IPA_cgraph_edge_t *edge;
      IPA_cgraph_node_t *src_node;

      edge = IPA_HTAB_CUR(eiter);
      src_node = edge->src_elist->node;

      if (!IPA_FLAG_ISSET(edge->flags, (IPA_CG_EDGE_FLAGS_HZ|
					IPA_CG_EDGE_FLAGS_GBL)))
	continue;

      oc_list = PIPA_off_add_to_class(oc_list, 
				      edge->data.target_offset, 
				      src_node, 0);
    }

  /* Merge all of the nodes in each class */
  List_start(oc_list);
  while ((oc = List_next(oc_list)))
    {
      int merged = 0;
      IPA_cgraph_node_t *first_node;
      IPA_cgraph_node_t *cur_node;

      /* Because of previous merges for different offsets
	 a node may no longer be a parent */
      first_node = List_first(oc->list);
      first_node = IPA_cg_node_get_rep(first_node);

      while ((cur_node = List_next(oc->list)))
	{
	  /* Because of previous merges for different offsets
	     a node may no longer be a parent */
	  cur_node = IPA_cg_node_get_rep(cur_node);
	  if (first_node == cur_node)
	    continue;
#if 0
	  if (!IPA_FLAG_ISSET(first_node->flags, IPA_CG_NODE_FLAGS_HEAP) ||
	      !IPA_FLAG_ISSET(cur_node->flags, IPA_CG_NODE_FLAGS_HEAP))
	    continue;
#endif

	  printf("UNIF [%d]: ",oc->offset);
	  IPA_cg_node_print(stdout,node, IPA_PRINT_ASCI);
	  printf(" - ");
	  IPA_cg_node_print(stdout,first_node, IPA_PRINT_ASCI);
	  IPA_cg_node_print(stdout,cur_node, IPA_PRINT_ASCI);
	  printf("\n"); fflush(stdout);

	  /* Because of previous merges for different offsets
	     a node may no longer be a parent */
	  work_list = IPA_cg_merge_nodes(first_node, cur_node, 0, 
					 work_list, IN_WORK_LIST);
	  fflush(stdout);
	  merged = 1;
	}
      if (!merged)
	{
	  /* Delete the useless class */
	  oc_list = List_delete_current(oc_list);
	  PIPA_off_class_free(oc);
	}
    }

  /* Now call unification on the first (now parent node)
   *   node of each class list
   */
  List_start(oc_list);
  while ((oc = List_next(oc_list)))
    {
      IPA_cgraph_node_t *first_node;
      first_node = List_first(oc->list);
      /* Should be a parent node 
       *  However, the same node could have been merged due
       *  to different offsets so this can occur
       */
      if (first_node->rep_parent != first_node)
	continue;
      unify_list = List_insert_last(unify_list, first_node);
      PIPA_off_class_free(oc);
    }
  List_reset(oc_list);

  List_start(work_list);
}
#endif

/*****************************************************************************
 * Ensure an edge exists
 *****************************************************************************/
int merge_test_cnt = 0;

static void
IPA_merge_test(IPA_cgraph_t * cur_consg)
{
  IPA_cgraph_node_t *node;
  IPA_cgraph_node_t *cmp_node;
  List list = NULL;
  int match;
  int cnt, cmp_cnt;

  if (merge_test_cnt < 1000)
    return;
  merge_test_cnt = 0;

  List_start(hpcand_list);
  while ((node = List_next(hpcand_list)))
    {
      assert(node->rep_parent == node);
      match = 0;

      List_start(list);
      while ((cmp_node = List_next(list)))
	{
	  IPA_HTAB_ITER eiter;
	  IPA_cgraph_edge_t *edge;
	  IPA_cgraph_edge_list_t *elist;

	  if (cmp_node->cgraph != node->cgraph)
	    continue;

	  elist = IPA_cg_edge_list_find(cmp_node, ASSIGN_ADDR);
	  cmp_cnt = IPA_htab_size(elist->out);
	  
	  elist = IPA_cg_edge_list_find(node, ASSIGN_ADDR);
	  cnt = IPA_htab_size(elist->out);
	  
	  /* within 5% */
	  if (cmp_cnt < 2000 || cnt < 2000)
	    continue;
	  if (abs(20 * (cmp_cnt - cnt)) > cmp_cnt)
	    {
#if 0
	      printf("MISMATCH %d.%d %d; match %d.%d %d\n",
		     node->data.var_id,
		     node->data.version,
		     cnt,
		     cmp_node->data.var_id,
		     cmp_node->data.version,
		     cmp_cnt
		     );
#endif
	      continue;
	    }

	  /* As an opti, you are better off keeping the node
	     with the lower version */
	  if (cmp_node->data.version < node->data.version)
	    {
	      /* NODE is no longer a candidate */
	      hpcand_list = List_delete_current(hpcand_list);
	      IPA_FLAG_CLR(node->flags, IN_HPCAND_LIST);
	    }
	  else
	    {
	      IPA_cgraph_node_t *tmpnode;
	      /* remove cmp_node from list */
	      list = List_delete_current(list);
	      /* add node to list */
	      list = List_insert_last(list, node);
	      /* CMP_NODE is no longer a candidate */
	      hpcand_list = List_remove(hpcand_list, cmp_node);
	      IPA_FLAG_CLR(cmp_node->flags, IN_HPCAND_LIST);
	      tmpnode = cmp_node;
	      cmp_node = node;
	      node = tmpnode;
	    }

	  printf("NODE %d.%d %d; match %d.%d %d\n",
		 node->data.var_id,
		 node->data.version,
		 cnt,
		 cmp_node->data.var_id,
		 cmp_node->data.version,
		 cmp_cnt
		 );

	  /* Merge */
	  match = 1;
	  work_list = IPA_cg_merge_nodes(cmp_node, node, 0,
					 work_list, IN_WORK_LIST);
	  break;
	}
      
      if (!match)
	list = List_insert_last(list, node);      
    }

  if (List_size(hpcand_list) > 500)
    printf("CAND SIZE %d\n",List_size(hpcand_list));

  List_reset(list);
}

#define MAKE_FI_DELETE IPA_CG_EDGE_FLAGS_GENERIC1

static void
collapse_all_dups(IPA_cgraph_node_t *src_node)
{
  IPA_cgraph_edge_list_t *elist = NULL;
  IPA_HTAB_ITER eiter;
  IPA_cgraph_edge_t *edge;
  List del_list = NULL;

  for (elist=src_node->first_list; elist; elist=elist->nxt_list)
    {
      int total = 0;
      if (elist->edge_type != ASSIGN_ADDR)
	continue;

      /* GENERIC flags must always be left cleared */
      IPA_HTAB_START(eiter, elist->out);
      IPA_HTAB_LOOP(eiter)
	{
	  edge = IPA_HTAB_CUR(eiter);
	  IPA_FLAG_CLR(edge->dst_elist->node->flags, 
		       IPA_CG_NODE_FLAGS_GENERIC10);
	}

      /* Get an estimate of the number of unique targets and edges
       */
      IPA_HTAB_START(eiter, elist->out);
      IPA_HTAB_LOOP(eiter)
	{
	  List list;
	  IPA_HTAB_ITER eiter2;
	  int count;

	  edge = IPA_HTAB_CUR(eiter);
	  if (edge->data.source_stride == 1)
	    continue;

	  /* Skip over struct targets to avoid having to distinguish
	     between particular target offsets */
	  if (edge->dst_elist->node->data.var_size > IPA_POINTER_SIZE)
	    continue;

	  if (IPA_FLAG_ISSET(edge->dst_elist->node->flags, 
			     IPA_CG_NODE_FLAGS_GENERIC10))
	    continue;
	  IPA_FLAG_SET(edge->dst_elist->node->flags,
		       IPA_CG_NODE_FLAGS_GENERIC10);

	  /* Collect and merge dups between src/dst
	   */
	  list = NULL;
	  count = 0;
	  IPA_HTAB_START(eiter2, edge->dst_elist->in);
	  IPA_HTAB_LOOP(eiter2)
	    {
	      IPA_cgraph_edge_t *edge2;
	      edge2 = IPA_HTAB_CUR(eiter2);
	      if (edge2->src_elist->node != src_node)
		continue;
	      count++;
	      if (edge2->data.source_stride == 1)
		{
		  assert(edge2->data.source_offset == 0);
		  continue;
		}
	      list = List_insert_last(list, edge2);
	    }

	  if (count > 1)
	    {
	      IPA_cgraph_edge_t *new_edge;
	      IPA_cgraph_edge_data_t ndata;
#if 0
	      printf("   %d.%d Would replace %d\n",
		     edge->dst_elist->node->data.var_id,
		     edge->dst_elist->node->data.version,
		     List_size(list));
#endif
	      total += List_size(list);

	      ndata.source_offset = 0;
	      ndata.source_stride = 1;
	      ndata.assign_size = edge->data.assign_size;
	      ndata.target_offset = edge->data.target_offset;
	      ndata.target_stride = edge->data.target_stride;
	      assert(ndata.target_offset == 0);
	      new_edge = IPA_consg_ensure_edge_d(elist->edge_type,
						 src_node,
						 edge->dst_elist->node,
						 &ndata,
						 (edge->flags & ~IN_WORK_LIST));
	      assert (!List_member(list, new_edge));
	      del_list = List_append(del_list, list);
	      list = NULL;
	    }
	  List_reset(list);
	}

      /* GENERIC flags must always be left cleared */
      IPA_HTAB_START(eiter, elist->out);
      IPA_HTAB_LOOP(eiter)
	{
	  edge = IPA_HTAB_CUR(eiter);
	  IPA_FLAG_CLR(edge->dst_elist->node->flags, 
		       IPA_CG_NODE_FLAGS_GENERIC10);
	}
    }

#if 1
  printf("   Would replace %d\n",
	 List_size(del_list));
#endif

  List_start(del_list);
  while ((edge = List_next(del_list)))
    {
      IPA_FLAG_SET(edge->flags, MAKE_FI_DELETE);
    }

  List_start(work_list);
  while ((edge = List_next(work_list)))
    {
      if (!IPA_FLAG_ISSET(edge->flags, MAKE_FI_DELETE))
	continue;
      del_current_from_worklist(edge);
    }

  List_start(del_list);
  while ((edge = List_next(del_list)))
    {
      IPA_cg_edge_delete(edge);
    }

  List_reset(del_list);
  del_list = NULL;
  List_start(work_list);
  src_node->collapse++;
}

static void
old_comp(IPA_cgraph_node_t *src_node)
{
  IPA_cgraph_edge_list_t *elist = NULL;
  IPA_cgraph_edge_t *edge = NULL;
  IPA_HTAB_ITER eiter;
  int totalcnt = 0;
  int targetcnt = 0;

  /* Don't bother with nodes with a small number of edges */
  elist = IPA_cg_edge_list_find (src_node, ASSIGN_ADDR);    

  /* Get an estimate of the number of unique targets and edges
   */
  IPA_HTAB_START(eiter, elist->out);
  IPA_HTAB_LOOP(eiter)
    {
      edge = IPA_HTAB_CUR(eiter);

      /* Skip over struct targets to avoid having to distinguish
	 between particular target offsets */
      if (edge->dst_elist->node->data.var_size > IPA_POINTER_SIZE)
	{
	  continue;
	}
      if (!IPA_FLAG_ISSET(edge->dst_elist->node->flags, 
			  IPA_CG_NODE_FLAGS_GENERIC10))
	targetcnt++;
      IPA_FLAG_SET(edge->dst_elist->node->flags,
		   IPA_CG_NODE_FLAGS_GENERIC10);
      
      totalcnt++;
    }
    
  /* GENERIC flags must always be left cleared */
  IPA_HTAB_START(eiter, elist->out);
  IPA_HTAB_LOOP(eiter)
    {
      edge = IPA_HTAB_CUR(eiter);
      IPA_FLAG_CLR(edge->dst_elist->node->flags, 
		   IPA_CG_NODE_FLAGS_GENERIC10);
    }

  printf("\n%d.%d FIELDDATA %d / %d = %f \n",
	 src_node->data.var_id,
	 src_node->data.version,
	 totalcnt,targetcnt,
	 (double) totalcnt / (double)targetcnt);
}


static void
disable_field_sensitivity2(IPA_cgraph_edgelist_e edge_type,
			   IPA_cgraph_node_t * src_node, 
			   IPA_cgraph_node_t * dst_node)
{
  IPA_cgraph_edge_list_t *elist = NULL;
  IPA_cgraph_edge_t *edge = NULL;
  IPA_HTAB_ITER eiter;
  int totalcnt = 0;

  if (!IPA_solver_limit_fscost)
    return;

  if (IPA_FLAG_ISSET(src_node->flags, IPA_CG_NODE_FLAGS_NOFIELD))
    return;

  if (src_node->data.var_size <= IPA_POINTER_SIZE)
    return;

  if (edge_type != ASSIGN_ADDR)
    return;

  if (IPA_FLAG_ISSET(src_node->flags, IN_FI_LIST))
    return;

  /* There "should" be simple nodes that have the problem 
   *  multiple byte objects can be included, but offsets must
   *  be checked individually
   */
  if (dst_node->data.var_size > IPA_POINTER_SIZE)
    return;

  /* This is used as a delay between checks */
  src_node->delay2++;
  if (src_node->delay2 < src_node->sample_delay2)
    return;
  src_node->delay2 = 0;

  /* How many offsets of src does dst point to */
  elist = IPA_cg_edge_list_find (dst_node, ASSIGN_ADDR);
  IPA_HTAB_START(eiter, elist->in);
  IPA_HTAB_LOOP(eiter)
    {
      edge = IPA_HTAB_CUR(eiter);

      if (edge->src_elist->node != src_node)
	continue;
      totalcnt++;
    }

  /* If more than one BAD */
  if (totalcnt > 1)
    {
#if 0
      printf("MIX INCR %d.%d : %d %d\n",
	     src_node->data.var_id,
	     src_node->data.version,
	     totalcnt,
	     src_node->mix2);
#endif
      src_node->mix2 ++;/*= (totalcnt + 1);*/
    }
  else if (src_node->mix2 > 0)
    {
      src_node->mix2 --;
    }
  
  /* The worse the situation, the more
     frequent the checking */
  if (src_node->mix2 <= 3)
    src_node->sample_delay2 = 102;
  else if (src_node->mix2 <= 15)
    src_node->sample_delay2 = 21;
  else if (src_node->mix2 <= 30)
    src_node->sample_delay2 = 6;
  else 
    src_node->sample_delay2 = 1;

  /* If really bad, collapse the node */
  if (src_node->mix2 > 2000)
    {
      printf("MIX THRESHOLD %d.%d : %d\n",
	     src_node->data.var_id,
	     src_node->data.version,
	     src_node->mix2);

      old_comp(src_node);
#if 0
      if (src_node->collapse == 0)
	{
	  if (!List_member(make_col_list, src_node))
	    make_col_list = List_insert_last(make_col_list, src_node);
	}
      else
#endif
	{
	  src_node->data.mod = 1;
	  make_fi_list = List_insert_last(make_fi_list, src_node);
	  IPA_FLAG_SET(src_node->flags, IN_FI_LIST);
	}
      src_node->sample_delay2 = 102;
      src_node->mix2 = 0;
      return;
    }
}

static IPA_cgraph_edge_t *
handle_skew_cycles(IPA_cgraph_edgelist_e edge_type,
		   IPA_cgraph_edge_t *edge)
{
  IPA_cgraph_node_t *src_node;
  int known_cycle;

  src_node = edge->src_elist->node;
  
  if (IPA_FLAG_ISSET(src_node->flags, IPA_CG_NODE_FLAGS_NOFIELD))
    return edge;
  
  if (src_node->data.var_size <= IPA_POINTER_SIZE)
    return edge;

  if (edge_type != ASSIGN_ADDR)
    return edge;

  known_cycle = edge->dst_elist->node->data.in_k_cycle;
  
  /* Not a marked node */
  if (known_cycle == 0)
    return edge;
  
  /* Already handled */
  if (edge->data.source_stride == 1)
    {
      assert(edge->data.source_offset == 0);
      return edge;
    }

  if (edge->data.target_offset != 0)
    return edge;

  /* Object is already more conservative than
     the cycle (this should technically test for
     kc being a multiple of mod. If so, OK. If not,
     new cycle is the GCD of the two.)
  */
  if (src_node->data.mod <= known_cycle)
    return edge;

  /* NOTE: Since the in_k_cycle marking may be used IN PLACE OF
   *     the actual cycle, on of the following cases MUST occur.
   */

#if 1
  /* CASE 1 */
  if (known_cycle <= IPA_POINTER_SIZE)
    {
      /* Instead of using the node's mod:
	 - make a new edge with
              + the source offset of 0              
	      + the source stride of the edge to 1
              + all else the same
	 - delete this edge
      */
      IPA_cgraph_edge_data_t ndata;
      IPA_cgraph_edge_t *new_edge;

      ndata.source_offset = 0;
      ndata.source_stride = 1;
      ndata.assign_size = edge->data.assign_size;
      ndata.target_offset = edge->data.target_offset;
      ndata.target_stride = edge->data.target_stride;
      new_edge = IPA_consg_ensure_edge_d(edge->src_elist->edge_type,
					 edge->src_elist->node,
					 edge->dst_elist->node,
					 &ndata,
					 (edge->flags & ~IN_WORK_LIST));
#if 0
      printf("KNOWN (edge) %d.%d -> %d.%d\n",
	     edge->src_elist->node->data.var_id,
	     edge->src_elist->node->data.version,
	     edge->dst_elist->node->data.var_id,
	     edge->dst_elist->node->data.version);
#endif
      if (new_edge == edge)
	{
	  assert(new_edge->data.source_offset == 0);
	  new_edge->data.source_stride = 1;
	}
      else
	{
	  if (IPA_FLAG_ISSET(edge->flags,IN_WORK_LIST))
	    {
	      work_list = List_remove(work_list, edge);
	      IPA_FLAG_CLR(edge->flags, IN_WORK_LIST);
	    }
	  IPA_cg_edge_delete(edge);
	}

      return new_edge;
    }
  else
#endif /* END KNOWN CYCLE ifdef */
    /* CASE 2 */
    if (src_node->data.mod > known_cycle)
      {
	/* Set the mod on the node
	 */
	src_node->data.mod = known_cycle;

#if 1
	printf("\n%d.%d KNOWN CYCLE %d : %d.%d -> %d.%d\n",
	       src_node->data.var_id,
	       src_node->data.version,
	       known_cycle,
	       edge->src_elist->node->data.var_id,
	       edge->src_elist->node->data.version,
	       edge->dst_elist->node->data.var_id,
	       edge->dst_elist->node->data.version);
#endif
	
	/* Need to call make_fi on the source node */
	if (!IPA_FLAG_ISSET(src_node->flags, IN_FI_LIST))
	  {
	    make_fi_list = List_insert_last(make_fi_list, src_node);
	    IPA_FLAG_SET(src_node->flags, IN_FI_LIST);
	  }
	return edge;
      }
  
  assert(0);
  return edge;
}





static IPA_cgraph_edge_t *
ensure_edge (IPA_cgraph_edgelist_e edge_type,
	     IPA_cgraph_node_t * src_node, 
	     IPA_cgraph_node_t * dst_node,
	     IPA_cgraph_edge_data_t * edata,
	     IPA_cgraph_edge_t * src_edge,
	     IPA_cgraph_edge_t * dst_edge)
{
  IPA_cgraph_edge_t *edge;
  int flag;

#if 0
  printf("ENSURE: ");
  IPA_cg_node_print(stdout,dst_node, IPA_PRINT_ASCI);
  printf(" <- ");
  IPA_cg_node_print(stdout,src_node, IPA_PRINT_ASCI);
  printf("\n");
#endif

  /* The direct inclusion of lib.c makes this necessary
     because a GLOBAL graph edge may point into them */
  if (!src_node->data.syminfo->fninfo->has_been_called ||
      !dst_node->data.syminfo->fninfo->has_been_called)
    {
#if 0
      printf("not called [%s %d %s %d]\n",
	     src_node->data.syminfo->fninfo->func_name, 
	     src_node->data.syminfo->fninfo->has_been_called,
	     dst_node->data.syminfo->fninfo->func_name, 
	     dst_node->data.syminfo->fninfo->has_been_called);
#endif
      return NULL;
    }

  if (dst_edge)
    {
      flag = IPA_consg_calc_edge_origin (src_edge, dst_edge);
      if (flag == 0)
	{
#if 0
	  printf("inv eo\n");
#endif
	  return NULL;
	}
    }
  else
    {
      flag = (src_edge->flags & (IPA_CG_EDGE_FLAGS_DIR_ALL |
				 IPA_CG_EDGE_FLAGS_ARRAY));
    }
  
  if (edge_type == ASSIGN_ADDR &&
      IPA_FLAG_ISSET(src_edge->flags, IPA_CG_EDGE_FLAGS_ARRAY))
    IPA_FLAG_SET(flag, IPA_CG_EDGE_FLAGS_ARRAY);

  edge = IPA_consg_ensure_edge_d (edge_type, src_node, dst_node,
				  edata, flag);

  if (edge && CG_EDGE_ISNEW(edge))
    {
#if 0
      printf("NEW ENSURE: ");
      print_edge(edge);
#endif

#if 0
      {
	static int cnt = 0;
	if (src_node->data.var_id == 1242  &&
	    IPA_FLAG_ISSET(dst_node->flags, IPA_CG_NODE_FLAGS_HEAP) && 
	    edge_type == ASSIGN_ADDR
	    )
	  { 
	      {
		char name[30];
		sprintf(name,"DEBUG_%d",dst_node->data.version);
		IPA_print_DOThistory(name,
				     edge_type,
				     src_node, s_offset,
				     dst_node, t_offset,
				     0, 10000);
		IPA_cg_DOTprint (dst_node->cgraph, "TCOMP.dot", IPA_CG_ETYPE_ALL);
		exit(1);
	      }
	  }
      }
#endif
      
#if EDGE_HISTORY
      edge->e1 = src_edge;
      edge->e2 = dst_edge;
#endif

      edge = handle_skew_cycles(edge_type, edge);

      disable_field_sensitivity2(edge_type, src_node, dst_node);

      if ((edge_type == ASSIGN_ADDR) && 
	  (IPA_FLAG_ISSET(src_node->flags, IPA_CG_NODE_FLAGS_HEAP) ||
	   IPA_FLAG_ISSET(src_node->flags, IPA_CG_NODE_FLAGS_STACK)))
	{
	  merge_test_cnt++;
	  
	  if (!IPA_FLAG_ISSET(src_node->flags, IN_HPCAND_LIST))
	    {
	      hpcand_list = List_insert_last(hpcand_list, src_node);
	      IPA_FLAG_SET(src_node->flags, IN_HPCAND_LIST);
	    }
	}

#if UNIFY
      if (IPA_gcon_option != IPA_GCON_ANDERSEN &&
	  edge_type == ASSIGN_ADDR)
	{
	  unify_list = List_insert_last(unify_list, dst_node);
	}
#endif	  

      /* Note: Because edges can be "RENEWED" they might already be
	 in the work list */
      add_to_worklist(edge);
    }
  else if (edge)
    {
      existedges++;
    }
  else
    excledges++;

  return edge;
}


/*****************************************************************************
 *****************************************************************************/

static int
adjust_offset(IPA_cgraph_node_t *node, int offset, int mingap)
{
#if 0
  if (IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_HEAP))
    {
      /* OBJECTS WITH UNKNOWN SIZE:
	 This effectively lumps all out-of-bounds accesses
	 into a remainder field just beyond the normal object */
      if (offset > node->data.var_size)
	{
	  offset = node->data.var_size;
	}
    }
  else
    {
      /* OBJECTS WITH KNOWN SIZE:
	 If mingap is not met then outside bounds */
      if (offset + mingap > node->data.var_size)
	return -1;      
    }

  /* Is this thing field sensitive */
  if (IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_NOFIELD))
    return 0;
  else
    return offset;

#else
  /* THIS STUFF USES MODULOS */

  if (IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_HEAP))
    {
#if 0
      if (offset + mingap > node->data.var_size)
	offset = node->data.var_size;
#endif
    }
  else
    {
      /* OBJECTS WITH KNOWN SIZE:
       */
      if (offset + mingap > node->data.var_size)
	return -1;
    }

  /* Now, for the selected offset, adjust for the modulos 
   */
  if (IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_NOFIELD))
    assert(node->data.mod == 1);
  assert(node->data.mod >= 1);

  offset = offset % node->data.mod;
  /* NegNum mod K ==>  (NegNum % K) + K */
  if (offset < 0)
    offset = offset + node->data.mod;

  assert(offset >= 0 && offset < node->data.mod);
  return offset;
#endif
}


static void
skew_aa(IPA_cgraph_node_t *dst, IPA_cgraph_edge_t *sk_edge, 
	       IPA_cgraph_node_t *mid, IPA_cgraph_edge_t *aa_edge,
	       IPA_cgraph_node_t *src)
{
  IPA_cgraph_edge_data_t naa_edata;

  /* u <<[t1,k1,s1] v &&  v =&[t2,s2] w && t2=s1 && valid(w,s2+k1)
   *--------------------------------------------------------------
   *                   u =&[t1,s2+k1] w
   */

  assert(aa_edge->src_elist->edge_type == ASSIGN_ADDR);

  /* Do the edges match up
   */
  if (aa_edge->data.target_stride == 0 &&
      sk_edge->data.source_stride == 0 &&
      (aa_edge->data.target_offset != 
       sk_edge->data.source_offset))
    {
      mismatches++;
      return;
    }

  /* Skew the source offset
   */
  if (aa_edge->data.source_stride == 0)
    naa_edata.source_offset = (aa_edge->data.source_offset +
			       sk_edge->data.assign_size);
  else
    /* stride is 1 so it represents all offsets 
     *   (if stride is ever allowed to be > 1 then 
     *      no = oo + (skew % stride);
     */
    naa_edata.source_offset = aa_edge->data.source_offset;

  naa_edata.source_stride = aa_edge->data.source_stride;

  /* Get the target offset
   */
  naa_edata.target_offset = sk_edge->data.target_offset;
  naa_edata.target_stride = sk_edge->data.target_stride;
  if (naa_edata.target_offset + IPA_POINTER_SIZE > dst->data.var_size)
    {
      return;
    }
  
  /* mingap is 1 for the source because this could be the address of a char
   * mingap is POINTER for the target because it is holding a pointer
   */
  if ((naa_edata.source_offset = adjust_offset(src, naa_edata.source_offset, 1)) < 0)
    return;
  if ((naa_edata.target_offset = adjust_offset(dst, naa_edata.target_offset, 
					       IPA_POINTER_SIZE)) < 0)
    return;

  naa_edata.assign_size = 0;

  /* Make the edge
   */
  ensure_edge (ASSIGN_ADDR, src, dst, 
	       &naa_edata,
	       aa_edge, sk_edge);
}

static void
assign_aa(IPA_cgraph_node_t *dst, IPA_cgraph_edge_t *a_edge, 
	       IPA_cgraph_node_t *mid, IPA_cgraph_edge_t *aa_edge,
	       IPA_cgraph_node_t *src)
{
  IPA_cgraph_edge_data_t naa_edata;
  int aa_targ, a_src;

  /* u =[t1,z1,s1] v &&  v =&[t2,s2] w && s1 <= t2 <= s1+z1-4
   *--------------------------------------------------------------
   *                   u =&[t1+(t2-s1),s2] w
   */

  assert(aa_edge->src_elist->edge_type == ASSIGN_ADDR);
  aa_targ = aa_edge->data.target_offset;
  a_src = a_edge->data.source_offset;

  /* The target of the aa must be inside the a_src and a_src + size
   */
  if (aa_edge->data.target_stride == 0 &&
      a_edge->data.source_stride == 0 &&
      ((aa_targ < a_src) ||
       (aa_targ + IPA_POINTER_SIZE > a_src + a_edge->data.assign_size)))
    {
      mismatches++;
      return;
    }
  
  /* The source offset of the aa
   */
  naa_edata.source_offset = aa_edge->data.source_offset;
  naa_edata.source_stride = aa_edge->data.source_stride;

  /*       a_src   aa_targ
   *           |---|
   *
   *           |---|
   *      a_targ   naa_tgt_offset
   */
  
  naa_edata.target_stride = a_edge->data.target_stride;

  if (aa_edge->data.target_stride != 0 ||
      a_edge->data.source_stride != 0 ||
      a_edge->data.target_stride != 0)
    {
      naa_edata.target_offset = a_edge->data.target_offset;
    }
  else
    {
      naa_edata.target_offset = a_edge->data.target_offset + (aa_targ - a_src);
    }

  /* mingap is 1 for the source because this could be the address of a char
   * mingap is POINTER for the target because it is holding a pointer
   */
  if ((naa_edata.source_offset = adjust_offset(src, naa_edata.source_offset, 1)) < 0)
    return;
  if ((naa_edata.target_offset = adjust_offset(dst, naa_edata.target_offset, 
					       IPA_POINTER_SIZE)) < 0)
    return;

  naa_edata.assign_size = 0;

  ensure_edge (ASSIGN_ADDR, src, dst, 
	       &naa_edata,
	       aa_edge, a_edge);
}

static void
assignderef_aa(IPA_cgraph_node_t *dst, IPA_cgraph_edge_t *ad_edge, 
	       IPA_cgraph_node_t *mid, IPA_cgraph_edge_t *aa_edge,
	       IPA_cgraph_node_t *src)
{
  IPA_cgraph_edge_data_t naa_edata;


  /* u =*[t1,z1,s1] v &&  v =&[t2,s2] w && t2=s1
   *--------------------------------------------------------------
   *                   u =[t1,z1,s2] w
   */

  assert(aa_edge->src_elist->edge_type == ASSIGN_ADDR);

  /* The source and target must match
   */
  if (aa_edge->data.target_stride == 0 &&
      ad_edge->data.source_stride == 0 &&
      (aa_edge->data.target_offset !=
       ad_edge->data.source_offset))
    {
      mismatches++;
      return;
    }

  /* Assignment size
   */
  if (aa_edge->data.source_stride == 0 &&
      ad_edge->data.target_stride == 0)
    naa_edata.assign_size = ad_edge->data.assign_size;
  else
    naa_edata.assign_size = IPA_POINTER_SIZE;

  /* Source offset
   */
  naa_edata.source_offset = aa_edge->data.source_offset;
  naa_edata.source_stride = aa_edge->data.source_stride;

  /* Target offset
   */
  naa_edata.target_offset = ad_edge->data.target_offset;
  naa_edata.target_stride = ad_edge->data.target_stride;

  /* mingap is POINTER because the ASSIGN can only carry pointers
   * mingap is POINTER for the target because it is holding a pointer
   */
  if ((naa_edata.source_offset = adjust_offset(src,  naa_edata.source_offset, 
					       IPA_POINTER_SIZE)) < 0)
    return;
  if ((naa_edata.target_offset = adjust_offset(dst, naa_edata.target_offset, 
					       IPA_POINTER_SIZE)) < 0)
    return;
  
  ensure_edge (ASSIGN, src, dst, 
	       &naa_edata,
	       aa_edge, ad_edge);
}

static void
derefassign_aa(IPA_cgraph_node_t *dst, IPA_cgraph_edge_t *da_edge, 
	       IPA_cgraph_node_t *mid, IPA_cgraph_edge_t *aa_edge,
	       IPA_cgraph_node_t *src)
{
  IPA_cgraph_edge_data_t naa_edata;


  /* u *=[t1,z1,s1] v &&  u =&[t2,s2] w && t2=t1
   *--------------------------------------------------------------
   *                   w =[s2,z1,s1] v
   */

  assert(aa_edge->src_elist->edge_type == ASSIGN_ADDR);

  /* target offsets must match (note this is a *= edge)
   */
  if (aa_edge->data.target_stride == 0 &&
      da_edge->data.target_stride == 0 &&
      (aa_edge->data.target_offset !=
       da_edge->data.target_offset))
    {
      mismatches++;
      return;
    }

  /* Assignment size
   */
  if (aa_edge->data.target_stride == 0 &&
      da_edge->data.source_stride == 0)
    naa_edata.assign_size = da_edge->data.assign_size;
  else
    naa_edata.assign_size = IPA_POINTER_SIZE;

  /* Assign source is da's source
   */
  naa_edata.source_offset = da_edge->data.source_offset;
  naa_edata.source_stride = da_edge->data.source_stride;

  /* Assign target is aa's source
   */
  naa_edata.target_offset = aa_edge->data.source_offset;
  naa_edata.target_stride = aa_edge->data.source_stride;

  /* mingap is POINTER because the ASSIGN can only carry pointers
   * mingap is POINTER for the target because it is holding a pointer
   */
  if ((naa_edata.source_offset = adjust_offset(src, naa_edata.source_offset, 
					       IPA_POINTER_SIZE)) < 0)
    return;
  if ((naa_edata.target_offset = adjust_offset(dst, naa_edata.target_offset, 
					       IPA_POINTER_SIZE)) < 0)
    return;

  ensure_edge (ASSIGN, src, dst, 
	       &naa_edata,
	       aa_edge, da_edge);
}



/*****************************************************************************
 * Process Skew edge.
 *****************************************************************************/

static void
process_skew (IPA_cgraph_edge_t * edge)
{
  IPA_cgraph_node_t *src_node, *dst_node, *tmp_node;
  IPA_cgraph_edge_list_t *edge_list;
  IPA_cgraph_edge_t  *tmp_edge;
  IPA_HTAB_ITER eiter;

  dst_node = edge->dst_elist->node;
  src_node = edge->src_elist->node;

  edge_list = IPA_cg_edge_list_find (src_node, ASSIGN_ADDR);
  if (!edge_list)
    return;
  IPA_HTAB_START(eiter, edge_list->in);
  IPA_HTAB_LOOP(eiter)
    {
      tmp_edge = IPA_HTAB_CUR(eiter);
      tmp_node = tmp_edge->src_elist->node;

      skew_aa(dst_node, edge, src_node, tmp_edge, tmp_node);
    }
}


/*****************************************************************************
 * Process Assign edge.
 *****************************************************************************/

static void
process_assign (IPA_cgraph_edge_t * edge)
{
  IPA_cgraph_node_t *src_node, *dst_node, *tmp_node;
  IPA_cgraph_edge_list_t *edge_list;
  IPA_cgraph_edge_t *tmp_edge;
  IPA_HTAB_ITER eiter;

  dst_node = edge->dst_elist->node;
  src_node = edge->src_elist->node;

  edge_list = IPA_cg_edge_list_find (src_node, ASSIGN_ADDR); 
  if (!edge_list)
    return;

  
  if (edge->data.assign_size <= IPA_POINTER_SIZE &&
      edge->data.source_stride == 0)
    {
      IPA_HTAB_START(eiter, edge_list->in);
      IPA_HTAB_CS_LOOP(eiter, edge->data.source_offset)
	{
	  tmp_edge = IPA_HTAB_CUR(eiter);
	  if (IPA_FLAG_ISSET(tmp_edge->flags, IN_WORK_LIST))
	    continue;
	  tmp_node = tmp_edge->src_elist->node;

	  assert(tmp_edge->data.target_offset == edge->data.source_offset);
	  assert(tmp_edge->data.target_stride == 0);

	  assign_aa(dst_node, edge, src_node, tmp_edge, tmp_node);
	}

      /* Compare against address edges with target strides != 0
       */
      IPA_HTAB_START(eiter, edge_list->in);
      IPA_HTAB_CS_LOOP(eiter, IPA_HTAB_CS_MULTI)
	{
	  tmp_edge = IPA_HTAB_CUR(eiter);
	  if (IPA_FLAG_ISSET(tmp_edge->flags, IN_WORK_LIST))
	    continue;
	  tmp_node = tmp_edge->src_elist->node;

	  assert(tmp_edge->data.target_stride != 0);
	  
	  assign_aa(dst_node, edge, src_node, tmp_edge, tmp_node);
	}
    }
  else
    {
      IPA_HTAB_START(eiter, edge_list->in);
      IPA_HTAB_LOOP(eiter)
	{
	  tmp_edge = IPA_HTAB_CUR(eiter);
	  if (IPA_FLAG_ISSET(tmp_edge->flags, IN_WORK_LIST))
	    continue;
	  tmp_node = tmp_edge->src_elist->node;
	  
	  assign_aa(dst_node, edge, src_node, tmp_edge, tmp_node);
	}
    }
}

/*****************************************************************************
 * Process AssignAddr edge.
 *****************************************************************************/

static void
process_assign_addr (IPA_cgraph_edge_t * edge)
{
  IPA_cgraph_node_t *src_node, *dst_node, *tmp_node;
  IPA_cgraph_edge_list_t *edge_list;
  IPA_cgraph_edge_t *tmp_edge;
  IPA_HTAB_ITER eiter;

  dst_node = edge->dst_elist->node;
  src_node = edge->src_elist->node;

  for (edge_list = dst_node->first_list; edge_list; 
       edge_list = edge_list->nxt_list)
    {
      switch (edge_list->edge_type)
	{
	case SKEW:
	  if (edge->data.target_stride != 0)
	    {
	      /* Must process all targets since stride is set
	       */
	      IPA_HTAB_START(eiter, edge_list->out);
	      IPA_HTAB_LOOP(eiter)
		{
		  tmp_edge = IPA_HTAB_CUR(eiter);
		  if (IPA_FLAG_ISSET(tmp_edge->flags, IN_WORK_LIST))
		    continue;
		  tmp_node = tmp_edge->dst_elist->node;
		  
		  skew_aa(tmp_node, tmp_edge, dst_node, edge, src_node);
		}
	    }
	  else
	    {
	      IPA_HTAB_START(eiter, edge_list->out);
	      IPA_HTAB_CS_LOOP(eiter, edge->data.target_offset)
		{
		  tmp_edge = IPA_HTAB_CUR(eiter);
		  if (IPA_FLAG_ISSET(tmp_edge->flags, IN_WORK_LIST))
		    continue;
		  tmp_node = tmp_edge->dst_elist->node;
		  
		  assert(tmp_edge->data.source_offset == edge->data.target_offset);
		  assert(tmp_edge->data.source_stride == 0);
		  
		  skew_aa(tmp_node, tmp_edge, dst_node, edge, src_node);
		}

	      /* MULTI handles those skews with source strides set
	       */
	      IPA_HTAB_START(eiter, edge_list->out);
	      IPA_HTAB_CS_LOOP(eiter, IPA_HTAB_CS_MULTI)
		{
		  tmp_edge = IPA_HTAB_CUR(eiter);
		  if (IPA_FLAG_ISSET(tmp_edge->flags, IN_WORK_LIST))
		    continue;
		  tmp_node = tmp_edge->dst_elist->node;
		  
		  assert(tmp_edge->data.source_stride != 0);
		  
		  skew_aa(tmp_node, tmp_edge, dst_node, edge, src_node);
		}
	    }
	  break;
	case ASSIGN:
	  if (edge->data.target_stride != 0)
	    {
	      /* Must process all targets since stride is set
	       */
	      IPA_HTAB_START(eiter, edge_list->out);
	      IPA_HTAB_LOOP(eiter)
		{
		  tmp_edge = IPA_HTAB_CUR(eiter);
		  if (IPA_FLAG_ISSET(tmp_edge->flags, IN_WORK_LIST))
		    continue;
		  tmp_node = tmp_edge->dst_elist->node;
		  
		  assign_aa(tmp_node, tmp_edge, dst_node, edge, src_node);
		}
	    }
	  else
	    {
	      IPA_HTAB_START(eiter, edge_list->out);
	      IPA_HTAB_CS_LOOP(eiter, edge->data.target_offset)
		{
		  tmp_edge = IPA_HTAB_CUR(eiter);
		  if (IPA_FLAG_ISSET(tmp_edge->flags, IN_WORK_LIST))
		    continue;
		  tmp_node = tmp_edge->dst_elist->node;
		  
		  assert(tmp_edge->data.source_offset == edge->data.target_offset);
		  assert(tmp_edge->data.assign_size <= IPA_POINTER_SIZE);
		  assert(tmp_edge->data.source_stride == 0);

		  assign_aa(tmp_node, tmp_edge, dst_node, edge, src_node);
		}

	      /* MULTI handles non-ptr sized assignments and 
	       *      assignments with source strides set
	       */
	      IPA_HTAB_START(eiter, edge_list->out);
	      IPA_HTAB_CS_LOOP(eiter, IPA_HTAB_CS_MULTI)
		{
		  tmp_edge = IPA_HTAB_CUR(eiter);
		  if (IPA_FLAG_ISSET(tmp_edge->flags, IN_WORK_LIST))
		    continue;
		  tmp_node = tmp_edge->dst_elist->node;
		  
		  assert(tmp_edge->data.assign_size > IPA_POINTER_SIZE ||
			 tmp_edge->data.source_stride != 0);
		  
		  assign_aa(tmp_node, tmp_edge, dst_node, edge, src_node);
		}
	    }
	  break;
	case ASSIGN_DEREF:
	  IPA_HTAB_START(eiter, edge_list->out);
	  IPA_HTAB_LOOP(eiter)
	    {
	      tmp_edge = IPA_HTAB_CUR(eiter);
	      if (IPA_FLAG_ISSET(tmp_edge->flags, IN_WORK_LIST))
		continue;
	      tmp_node = tmp_edge->dst_elist->node;

	      assignderef_aa(tmp_node, tmp_edge, dst_node, edge, src_node);
	    }
	  break;
	case DEREF_ASSIGN:
	  IPA_HTAB_START(eiter, edge_list->in);
	  IPA_HTAB_LOOP(eiter)
	    {
	      tmp_edge = IPA_HTAB_CUR(eiter);
	      if (IPA_FLAG_ISSET(tmp_edge->flags, IN_WORK_LIST))
		continue;
	      tmp_node = tmp_edge->src_elist->node;

	      derefassign_aa(src_node, tmp_edge, dst_node, edge, tmp_node);
	    }
	  break;
	default:
	  break;
	}
    }

}

/*****************************************************************************
 * Process AssignDeref edge.
 *****************************************************************************/

static void
process_assign_deref (IPA_cgraph_edge_t * edge)
{
  IPA_cgraph_node_t *src_node, *dst_node, *tmp_node;
  IPA_cgraph_edge_list_t *edge_list;
  IPA_cgraph_edge_t *tmp_edge;
  IPA_HTAB_ITER eiter;

  dst_node = edge->dst_elist->node;
  src_node = edge->src_elist->node;

  edge_list = IPA_cg_edge_list_find (src_node, ASSIGN_ADDR);
  if (!edge_list)
    return;
  IPA_HTAB_START(eiter, edge_list->in);
  IPA_HTAB_LOOP(eiter)
    {
      tmp_edge = IPA_HTAB_CUR(eiter);
      if (IPA_FLAG_ISSET(tmp_edge->flags, IN_WORK_LIST))
	continue;
      tmp_node = tmp_edge->src_elist->node;

      assignderef_aa(dst_node, edge, src_node, tmp_edge, tmp_node);
    }
}

/*****************************************************************************
 * Process DerefAssign edge.
 *****************************************************************************/

static void
process_deref_assign (IPA_cgraph_edge_t * edge)
{
  IPA_cgraph_node_t *src_node, *dst_node, *tmp_node;
  IPA_cgraph_edge_list_t *edge_list;
  IPA_cgraph_edge_t *tmp_edge;
  IPA_HTAB_ITER eiter;

  dst_node = edge->dst_elist->node;
  src_node = edge->src_elist->node;

  edge_list = IPA_cg_edge_list_find (dst_node, ASSIGN_ADDR);
  if (!edge_list)
    return;
  IPA_HTAB_START(eiter, edge_list->in);
  IPA_HTAB_LOOP(eiter)
    {
      tmp_edge = IPA_HTAB_CUR(eiter);
      if (IPA_FLAG_ISSET(tmp_edge->flags, IN_WORK_LIST))
	continue;
      tmp_node = tmp_edge->src_elist->node;

      derefassign_aa(tmp_node, edge, dst_node, tmp_edge, src_node);
    }
}















/*********************************************************************************
 ********************************************************************************* 
 *********************************************************************************
 THE GRAVEYARD OF INTERESTING YET DEFICIENT ATTEMPTS
 ********************************************************************************* 
 *********************************************************************************
 *********************************************************************************/
#if 0

static int
array_member(int *array, int size, int val)
{
  int i;
  for (i=0; i<size; i++)
    {
      if (array[i] == val)
	{
	  return i+1;
	}
    }
  return 0;
}

static int
qsort_intcmp(const void *a, const void *b)
{
  return (*(int*)a - *(int*)b);
}

static int
qsort_rintcmp(const void *a, const void *b)
{
  return (*(int*)b - *(int*)a);
}

static int
compute_stride(IPA_cgraph_edge_list_t *elist,
	       IPA_cgraph_node_t *srcnode,
	       IPA_cgraph_node_t *dstnode)
{
  IPA_cgraph_edge_t *edge = NULL;
  IPA_HTAB_ITER eiter;
  int offarray[20];
  int offcnt;
  int strides[20];
  int scnt;
  int mod = 0;
  int s, i;
  int b_match, b_mod = 0;
  int cur_offset = -1;

  printf(" NODE %d.%d\n",
	 dstnode->data.var_id,
	 dstnode->data.version);

  offcnt = 0;
  IPA_HTAB_START(eiter, elist->out);
  IPA_HTAB_LOOP(eiter)
    {
      edge = IPA_HTAB_CUR(eiter);
      if (edge->dst_elist->node != dstnode)
	continue;

      /* Look at the first interesting offset of the target 
       */
      if (cur_offset == -1)
	cur_offset = edge->data.target_offset;
      if (edge->data.target_offset != cur_offset)
	continue;
      
      /* Record all source offsets that target it 
       */
      if (array_member(offarray, offcnt, 
		       edge->data.source_offset))
	continue;

      offarray[offcnt++] = edge->data.source_offset;
      if (offcnt >= 20)
	break;
    }

  assert(offcnt > 0);
#if 1
  printf ("  Offsets [%d]: ", offcnt);
  for (i=0; i<offcnt; i++)
    printf("%d ",offarray[i]);
  printf ("\n");
#endif

  if (offcnt < 2)
    return -2;

#if 0
  /* Sort the offsets */
  qsort(offarray, offcnt, sizeof(int), qsort_intcmp);

  /* Find a few possible strides 
   */
  {
    int x,y;
    scnt = 0;
    for (x=0; x<3 && x<offcnt; x++)
      {
	for (y=x+1; y<10 && y<offcnt; y++)
	  {
	    if (array_member(strides, scnt, 
			     offarray[y] - offarray[x]))
	      continue;
	    strides[scnt++] = offarray[y] - offarray[x];
	    if (scnt >= 20)
	      break;
	  }
      }
  }

  qsort(strides, scnt, sizeof(int), qsort_rintcmp);

#if 0
  printf (" Strides: ");
  for (s=0; s<scnt; s++)
    printf("%d ",strides[s]);
  printf ("\n");
#endif
    
  /* Test strides until one covers > 50% of the offsets */
  b_match = 0;
  for (s=0; s<scnt; s++)
    {
      int match = 0;

      mod = strides[s];

      for (i=0; i<offcnt; i++)
	{
	  if (array_member(offarray, offcnt, 
			   offarray[i] + mod))
	    match++;
	}
#if 0
      printf(" Testing stride [%d] %d / %d \n", 
	     mod, match, offcnt);
#endif
      if (match > b_match)
	{
	  b_match = match;
	  b_mod = mod;
	}
    }

  printf("BMATCH %d : %d %d \n", b_mod, b_match, offcnt);
  if (offcnt < 8)
    {
      if (3 * (b_match+1) < 2 * offcnt)
	{
	  return -1;
	}
    }
#endif
  
  b_match = 0;
  b_mod = -1;
  for (mod=srcnode->data.mod; mod > 0; mod--)
    {
      int match = 0;
      for (i=0; i<offcnt; i++)
	{
	  if ((offarray[i] % mod) == 0)
	    match++;
	}
      
      if (match >= b_match)
	{
	  if (match > b_match)
	    printf("MATCH %d %d <- %d %d \n", mod, match, b_mod, b_match);
	  b_match = match;
	  b_mod = mod;
	  if (b_match == offcnt)
	    break;
	  if (3*b_match > 2*offcnt)
	    break;
	}
    }

  if (b_mod <= 2*IPA_POINTER_SIZE)
    b_mod = 1;

#if 0
  printf(" Selected Modulos %d\n", b_mod);
#endif

  return b_mod;
}

static int
compute_modulos(IPA_cgraph_node_t *node)
{
  IPA_cgraph_edge_list_t *elist = NULL;
  IPA_cgraph_edge_t *edge = NULL;
  IPA_HTAB_ITER eiter;
  int mod = 0;
#if 0
  int modcnt;
  int modarray[20];
  int cnt[20];
  int i;
  int iter = 0;
#endif

  elist = IPA_cg_edge_list_find (node, ASSIGN_ADDR);    
  IPA_HTAB_START(eiter, elist->out);
  IPA_HTAB_LOOP(eiter)
    {
      edge = IPA_HTAB_CUR(eiter);
      if (IPA_FLAG_ISSET(edge->dst_elist->node->flags, 
			 IPA_CG_NODE_FLAGS_GENERIC10))
	continue;
      mod = compute_stride(elist, node, edge->dst_elist->node);
      if (mod != -2)
	break;
#if 0
      iter++;
      if (iter > 20)
	break;

      if ((i = array_member(modarray, modcnt, mod)))
	{
	  cnt[i-1]++;
	  continue;
	}
      cnt[modcnt] = 1;
      modarray[modcnt++] = mod;
      if (modcnt >= 20)
	break;
#endif
    }

#if 0
  mod = -1;
  printf ("  Mods : ");
  for (i=0; i<modcnt; i++)
    {
      if (cnt[i] > 10)
	mod = modarray[i];
      printf("[%d=%d]",modarray[i],cnt[i]);
    }
  printf ("\n");
#endif

  /* Clear out the flags */
  IPA_HTAB_START(eiter, elist->out);
  IPA_HTAB_LOOP(eiter)
    {
      edge = IPA_HTAB_CUR(eiter);
      IPA_FLAG_CLR(edge->dst_elist->node->flags, IPA_CG_NODE_FLAGS_GENERIC10);
    }

  printf(" MOD %d\n",mod);
  return mod;
}

static void
disable_field_sensitivity(IPA_cgraph_edgelist_e edge_type,
			  IPA_cgraph_node_t * src_node, 
			  IPA_cgraph_node_t * dst_node)
{
  IPA_cgraph_edge_list_t *elist = NULL;
  IPA_cgraph_edge_t *edge = NULL;
  IPA_HTAB_ITER eiter;
  int targetcnt = 0;
  int totalcnt = 0;
  int skipped = 0;    

  if (!IPA_solver_limit_fscost)
    return;

  if (IPA_FLAG_ISSET(src_node->flags, IPA_CG_NODE_FLAGS_NOFIELD))
    return;

  if (src_node->data.var_size <= IPA_POINTER_SIZE)
    return;

  if (edge_type != ASSIGN_ADDR)
    return;

  if (IPA_FLAG_ISSET(src_node->flags, IN_FI_LIST))
    return;

  /* This is used as a delay between checks */
  if (src_node->delay != 0)
    {
      src_node->delay--;
      return;
    }

  /* Don't bother with nodes with a small number of edges */
  elist = IPA_cg_edge_list_find (src_node, ASSIGN_ADDR);    
  if (IPA_htab_size(elist->out) < 200)
    return;

  /* Get an estimate of the number of unique targets and edges
   */
  IPA_HTAB_START(eiter, elist->out);
  IPA_HTAB_LOOP(eiter)
    {
      edge = IPA_HTAB_CUR(eiter);

      /* Skip over struct targets to avoid having to distinguish
	 between particular target offsets */
      if (edge->dst_elist->node->data.var_size > IPA_POINTER_SIZE)
	{
	  skipped++;
	  continue;
	}
      if (!IPA_FLAG_ISSET(edge->dst_elist->node->flags, 
			  IPA_CG_NODE_FLAGS_GENERIC10))
	targetcnt++;
      IPA_FLAG_SET(edge->dst_elist->node->flags,
		   IPA_CG_NODE_FLAGS_GENERIC10);
      
      totalcnt++;
    }
    
  /* GENERIC flags must always be left cleared */
  IPA_HTAB_START(eiter, elist->out);
  IPA_HTAB_LOOP(eiter)
    {
      edge = IPA_HTAB_CUR(eiter);
      IPA_FLAG_CLR(edge->dst_elist->node->flags, 
		   IPA_CG_NODE_FLAGS_GENERIC10);
    }

  /* 
   * 
   * c/t > 1.5
   *
   */
  if ((10 * totalcnt) > (15 * targetcnt))
    {
      int mod;

#if 1
      printf("\n%d.%d FIELDDATA %d / %d = %f \n",
	     src_node->data.var_id,
	     src_node->data.version,
	     totalcnt,targetcnt,
	     (double) totalcnt / (double)targetcnt);
#endif

      /* Try to compute a stride/modulous for the skewing */
      mod = compute_modulos(src_node);
      if (mod < 0)
	{
	  src_node->delay = 50;
	  return;
	}

      /* Make the thing FI or use the modulos */
      src_node->data.mod = mod;

      make_fi_list = List_insert_last(make_fi_list, src_node);
      IPA_FLAG_SET(src_node->flags, IN_FI_LIST);
      return;
    }
  else
    {
      /* These are decay values to delay the next check 
       *  - Delay for at least 200 
       *  - If there are 1000 targets and 1100 edges then 
       *      you should delay at least (2200 - 1000) because
       *      that is the minimum necessary to make the above
       *      check true.
       */
      int diff = ((18 * targetcnt)/10) - (totalcnt);
      if (diff > 200)
	src_node->delay = diff;
      else
	src_node->delay = 200;
#if 0
      if (src_node->data.var_id == 5327 ||
	  src_node->data.var_id == 119658)
	{
	  printf("%d.%d DELAY %d : %d %d [%d]\n", 
		 src_node->data.var_id,
		 src_node->data.version,
		 src_node->delay,
		 targetcnt, totalcnt, skipped);
	}
#endif
    }

  return;
}




static int
array_member(int *array, int size, int val)
{
  int i;
  for (i=0; i<size; i++)
    {
      if (array[i] == val)
	{
	  return i+1;
	}
    }
  return 0;
}

static int
qsort_intcmp(const void *a, const void *b)
{
  return (*(int*)a - *(int*)b);
}

static int
qsort_rintcmp(const void *a, const void *b)
{
  return (*(int*)b - *(int*)a);
}

static int
compute_stride(IPA_cgraph_edge_list_t *elist,
	       IPA_cgraph_node_t *srcnode,
	       IPA_cgraph_node_t *dstnode)
{
  IPA_cgraph_edge_t *edge = NULL;
  IPA_HTAB_ITER eiter;
  int offarray[20];
  int offcnt;
  int strides[20];
  int scnt;
  int s, i;
  int b_match, b_mod = 0;
  int cur_offset = -1;

  printf(" NODE %d.%d\n",
	 dstnode->data.var_id,
	 dstnode->data.version);

  offcnt = 0;
  IPA_HTAB_START(eiter, elist->out);
  IPA_HTAB_LOOP(eiter)
    {
      edge = IPA_HTAB_CUR(eiter);
      if (edge->dst_elist->node != dstnode)
	continue;

      /* Look at the first interesting offset of the target 
       */
      if (cur_offset == -1)
	cur_offset = edge->data.target_offset;
      if (edge->data.target_offset != cur_offset)
	continue;
      
      /* Record all source offsets that target it 
       */
      if (array_member(offarray, offcnt, 
		       edge->data.source_offset))
	continue;

      offarray[offcnt++] = edge->data.source_offset;
      if (offcnt >= 20)
	break;
    }

  assert(offcnt > 0);

  if (offcnt < 2)
    return -2;

  /* Sort the offsets */
  qsort(offarray, offcnt, sizeof(int), qsort_intcmp);

#if 1
  printf ("  Offsets [%d]: ", offcnt);
  for (i=0; i<offcnt; i++)
    printf("%d ",offarray[i]);
  printf ("\n");
#endif

  /* Find a few possible strides 
   */
  {
    int x,y;
    scnt = 0;
    for (x=0; x<3 && x<offcnt; x++)
      {
	for (y=x+1; y<10 && y<offcnt; y++)
	  {
	    if (array_member(strides, scnt, 
			     offarray[y] - offarray[x]))
	      continue;
	    strides[scnt++] = offarray[y] - offarray[x];
	    if (scnt >= 20)
	      break;
	  }
      }
  }

  qsort(strides, scnt, sizeof(int), qsort_rintcmp);

#if 0
  printf (" Strides: ");
  for (s=0; s<scnt; s++)
    printf("%d ",strides[s]);
  printf ("\n");
#endif
    
  /* Test strides until one covers > 50% of the offsets */
  b_match = 0;
  for (s=0; s<scnt; s++)
    {
      int newcnt = 0;
      int newarray[20];
      int mod;

      mod = strides[s];
      for (i=0; i<offcnt; i++)
	{
	  int val = (offarray[i] % mod);
	  if (array_member(newarray, newcnt, val))
	    continue;
	  newarray[newcnt++] = val;
	}
#if 1
      printf(" Testing stride [%d] full %d new %d \n", 
	     mod, offcnt, newcnt);
#endif
      if (2*newcnt <= offcnt)
	{
	  b_mod = mod;
	  break;
	}
    }
#if 0
  if (b_mod <= 2*IPA_POINTER_SIZE)
    b_mod = 1;
#endif
  return b_mod;
}

static int
compute_modulos(IPA_cgraph_node_t *node)
{
  IPA_cgraph_edge_list_t *elist = NULL;
  IPA_cgraph_edge_t *edge = NULL;
  IPA_HTAB_ITER eiter;
  int mod = 0;

  elist = IPA_cg_edge_list_find (node, ASSIGN_ADDR);    
  IPA_HTAB_START(eiter, elist->out);
  IPA_HTAB_LOOP(eiter)
    {
      edge = IPA_HTAB_CUR(eiter);
      if (IPA_FLAG_ISSET(edge->dst_elist->node->flags, 
			 IPA_CG_NODE_FLAGS_GENERIC10))
	continue;
      mod = compute_stride(elist, node, edge->dst_elist->node);
      if (mod != -2)
	break;
    }

  /* Clear out the flags */
  IPA_HTAB_START(eiter, elist->out);
  IPA_HTAB_LOOP(eiter)
    {
      edge = IPA_HTAB_CUR(eiter);
      IPA_FLAG_CLR(edge->dst_elist->node->flags, 
		   IPA_CG_NODE_FLAGS_GENERIC10);
    }

  printf(" MOD %d\n",mod);
  assert(mod > 0);
  return mod;
}


#endif
