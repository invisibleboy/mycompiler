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
 *      File:    pipa_driver_utils.c
 *      Author:  Erik Nystrom
 *      Copyright (c) 2003  Erik Nystrom, Wen-mei Hwu
 *              and The Board of Trustees of the University of Illinois.
 *              All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#include "pipa_driver_utils.h"
#include "pipa_misc_utils.h"

void
invalidate_single(IPA_callg_node_t *callg_node);

void
IPA_callgraph_node_invalidate_summaries2(IPA_callg_node_t *callg_node);

/*****************************************************************************
 * Given a callgraph node, callsite, and callee_name add call
 *   and all connected direct-calls
 *****************************************************************************/

List
IPA_callgraph_build_direct (IPA_prog_info_t   *info,
                            IPA_callg_node_t  *caller_node,
                            IPA_callsite_t    *caller_cs,
                            char *callee_name,
			    Key callee_key,
			    int is_indirect,
			    List new_edge_list,
			    int round)
{
  IPA_funcsymbol_info_t *callee_fn;
  IPA_callg_node_t *callee_node;
  IPA_callg_edge_t *edge;
  int ab, fb;

  assert(caller_node);
  assert(caller_cs);

  assert(P_ValidKey(callee_key));
  if (!(callee_fn = IPA_funcsymbol_find(info, callee_key)))
    {
      /* This function is not in memory */
      printf("NOT IN MEMORY: [%s]\n",callee_name);
      if (!IPA_allow_missing_ipa)
	assert(0);
      return new_edge_list;
    }
  
  assert(caller_cs->iface && callee_fn->iface);
  ab = IPA_interface_get_num_params (caller_cs->iface);
  fb = IPA_interface_get_num_params (callee_fn->iface);

  if (IPA_use_actualformal_filter &&
      strcmp(callee_name, "main") && 
      !callee_fn->is_vararg &&
      is_indirect &&
      (ab < fb))
    {
      printf("CALLGRAPH BUILD: [%s] : Skipping unmatched parameters atl %d < fml %d \n",
	     callee_fn->func_name,
	     ab, fb);
      return new_edge_list;
    }
	  
#if 1
  if (!callee_fn->is_vararg && (ab != fb))
    {
      if (is_indirect)
	fprintf(info->errfile,"CALLGRAPH BUILD: [%s] : mismatched IND act %d != fml %d\n", 
		callee_fn->func_name, ab, fb);
      else
	fprintf(info->errfile,"CALLGRAPH BUILD: [%s] : mismatched DIR act %d != fml %d\n", 
		callee_fn->func_name, ab, fb);
    }
#endif


  if (!callee_fn->has_been_called)
    {
      IPA_callsite_t *cs;
      List cslist;

      /* This is the first time this func has been called.
	 Recursively build its part of the call graph */
      callee_fn->has_been_called = 1;
/*       printf("ACCESSING [%s]", callee_name); */
/*       if (callee_fn->is_vararg) */
/* 	printf(" VARARG"); */
/*       printf("\n"); */

      callee_fn->summary_valid = 0;

      callee_fn->call_node = 
	IPA_callg_node_add (info->call_graph, callee_fn);
      callee_node = callee_fn->call_node;
      
      /* Recursive call */
      cslist = callee_fn->callsites;
      List_start (cslist);
      while ((cs = List_next (cslist)))
        {
          if (cs->indirect)
            continue;

          new_edge_list = 
	    IPA_callgraph_build_direct (info, callee_node, cs,
					cs->callee.dir.name, 
					cs->callee.dir.key, 0,
					new_edge_list, round);
        }
    }
  callee_node = callee_fn->call_node;


#if HS_CI
  if (IPA_cloning_option == IPA_HEAP_CLONING &&
      IPA_context_option == IPA_CONTEXT_INSENSITIVE)
    {
      if (strcmp(callee_name, "malloc") == 0 ||
          strcmp(callee_name, "calloc") == 0 ||
	  strcmp(callee_name, "valloc") == 0 ||
	  strcmp(callee_name, "alloca") == 0)
        {
          IPA_cgraph_node_t *actual_node;
          IPA_cgraph_node_t *heap_node;
          int actual_id, heap_id;
          IPA_HTAB_ITER iter;

          printf("Specializing %s in %s at %d\n",
                 callee_name, 
                 caller_node->fninfo->func_name,
                 caller_cs->cs_id);

          actual_id = IPA_interface_get_ret_id (caller_cs->iface);
          heap_id = -1;
	  IPA_HTAB_START(iter, callee_node->fninfo->consg->nodes);
	  IPA_HTAB_LOOP(iter)
	    {
	      IPA_cgraph_node_t *tmpnode;
              IPA_symbol_info_t *syminfo;

	      tmpnode = IPA_HTAB_CUR(iter);
	      syminfo = tmpnode->data.syminfo;

              if (strncmp(syminfo->symbol_name, "RETURN_MEM_ALLOC",14) == 0)
                {
                  heap_id = syminfo->id;
                  printf("Found heap var %s %d\n",
                         syminfo->symbol_name, heap_id);
                  break;
                }
            }
          actual_node = IPA_consg_find_node (caller_node->fninfo->consg, 
                                             actual_id, 1);
          heap_node = IPA_consg_find_node (callee_node->fninfo->consg,
                                           heap_id, 1);
          IPA_FLAG_CLR(heap_node->flags, IPA_CG_NODE_FLAGS_GLOBAL);

          heap_node = 
            IPA_consg_node_new_version (info,
                                        heap_node,
                                        caller_node->fninfo->consg,
                                        NULL,
                                        caller_cs->version_htab);
	  heap_node->generation = 1;
	  heap_node->from_version = 1;

          printf("New heap version %d:%d\n",
                 heap_id, heap_node->data.version);
          IPA_consg_ensure_edge(ASSIGN_ADDR,
                                heap_node, actual_node,
                                0, 0, 0,
                                (IPA_CG_EDGE_FLAGS_EXPLICIT |
                                 IPA_CG_EDGE_FLAGS_HZ));

          // Annotate the callsite of the malloc with the new object id
          P_memdep_core_t dep = P_new_memdep_core();
          dep->id = heap_node->data.var_id;
          dep->version = heap_node->data.version;
          dep->offset = dep->size = -1;
          dep->is_def = 1;
          P_memdep_t md = P_GetMemDep(caller_cs->call_expr);
          assert(md);
          md->deps = List_insert_last(md->deps, dep);
                   
          return new_edge_list;
        }
    }
#endif

  edge = IPA_callg_edge_add (caller_node, caller_cs, 
			     callee_node, callee_fn->iface,
			     is_indirect);

  if (CALLG_EDGE_ISNEW(edge))
    {
      new_edge_list = List_insert_last(new_edge_list, edge);
    }

  return new_edge_list;
}



/*****************************************************************************
 * Given new callgraph edges, remove and self cycles
 *****************************************************************************/

void
IPA_callgraph_handle_selfedges (IPA_callg_node_t *call_node)
{
  IPA_cgraph_t *consg;
  IPA_callg_node_t *parent_caller_node;
  IPA_callg_node_t *parent_callee_node;
  IPA_callg_edge_t *edge;
  int nonself_cnt = 0, self_cnt = 0;

  assert(call_node);
  if (IPA_csrec_option == IPA_CSREC_FULL)
    parent_caller_node = call_node;
  else
    parent_caller_node = IPA_callg_node_get_rep(call_node);
  consg = parent_caller_node->fninfo->consg;

  /* Process callee edges and  handle and flag any self-edges */
  List_start(call_node->callee_edges);
  while ((edge = List_next(call_node->callee_edges)))
    {
      /* Don't re-process self-edges */
      if (IPA_FLAG_ISSET(edge->flags, IPA_CALLG_EDGE_FLAGS_SELFEDGE))
	{
	  assert(edge->sum_nodes == NULL);
	  continue;
	}

      /* Compare the parent nodes because of SCCs */
      if (IPA_csrec_option == IPA_CSREC_FULL)
	parent_callee_node = edge->callee;
      else
 	parent_callee_node = IPA_callg_node_get_rep(edge->callee);
      
     if (parent_callee_node != parent_caller_node)
	{
	  nonself_cnt++;
	  continue;
	}
      self_cnt++;

      IPA_consg_assign_params (consg, edge->caller_cs->iface,
			       consg, edge->callee_if,
			       IPA_CG_EDGE_FLAGS_EXPLICIT,
			       (IPA_CG_EDGE_FLAGS_EXPLICIT |
				IPA_CG_EDGE_FLAGS_HZ));
      IPA_consg_assign_return (consg, edge->caller_cs->iface,
			       consg, edge->callee_if,
			       IPA_CG_EDGE_FLAGS_EXPLICIT,
			       (IPA_CG_EDGE_FLAGS_EXPLICIT|
				IPA_CG_EDGE_FLAGS_HZ));


      /* Delete any summary nodes created that are now unnecessary
       */
      IPA_consg_delete_summary_nodes2(edge, NULL);

      IPA_FLAG_SET(edge->flags, IPA_CALLG_EDGE_FLAGS_SELFEDGE);
    }

#if 0
  printf("REMSELF: %d non-self edges\n", nonself_cnt);
  printf("REMSELF: %d self edges\n", self_cnt);
#endif
}


/*****************************************************************************
 * Merge two callgraph nodes and their internal constraint graphs
 *****************************************************************************/

void
IPA_callgraph_node_merge (IPA_callg_node_t * dst_node,
                          IPA_callg_node_t * src_node)
{
  IPA_cgraph_t *dst_consg;
  IPA_cgraph_t *src_consg;
  
  /* We should only try to merge parent nodes */
  assert(dst_node->rep_parent == dst_node);
  assert(src_node->rep_parent == src_node);
  
  if (dst_node == src_node)
    return;

  dst_consg = dst_node->fninfo->consg;
  src_consg = src_node->fninfo->consg;
  assert(dst_consg && src_consg);

  DEBUG_IPA(1, printf ("CALLG Merging nodes [%s] <= [%s]\n",
		       dst_node->fninfo->func_name,
		       src_node->fninfo->func_name););
#if 0
  {
    IPA_callg_node_t *node;
    for (node=dst_node; node; node=node->rep_child)
      printf("(%s)",node->fninfo->func_name);
    printf("\n");
    for (node=src_node; node; node=node->rep_child)
      printf("(%s)",node->fninfo->func_name);
    printf("\n");
  }
#endif

  if (IPA_csrec_option != IPA_CSREC_FULL)
    {
      /* Merge the contraint graphs */
      IPA_cg_merge_graph(dst_consg, src_consg);
    }

  /* Merge the callgraph nodes */
  IPA_callg_merge_nodes(dst_node, src_node);

  if (IPA_csrec_option != IPA_CSREC_FULL)
    {
      /* Delete the old consg graph and set src's to NULL  */
      IPA_cg_cgraph_free (src_consg);
      IPA_cg_cgraph_free (src_node->fninfo->lsum_consg);
      src_node->fninfo->consg = NULL;
      src_node->fninfo->lsum_consg = NULL;
    }
}


/*****************************************************************************
 *
 *****************************************************************************/

void
IPA_a_cycle_detection(IPA_cgraph_t *cg, 
		      int dont_merge,
		      int del_mode)
{
  IPA_cgraph_node_t *head_node;
  IPA_cgraph_node_t *node;
  IPA_cgraph_edge_list_t *elist;
  List sccs;
  List scc;
  int scc_cnt;
  
  sccs = IPA_cg_find_SCC (cg, IPA_CG_ETYPE_ASSIGN);

  scc_cnt = 0;
  List_start (sccs);
  while ((scc = List_next (sccs)))
    {
      /* Skip trivial ones (all self-recursion
         has already been handled during loading */
      if (List_size (scc) == 1)
        continue;

      scc_cnt++;

      DEBUG_IPA(1, if (del_mode == CD_SELECT)
	  	     printf ("ACYCLE-SELECT : [%d] %d nodes\n", scc_cnt, List_size (scc));
		   else
		printf ("ACYCLE-DEL : [%d] %d nodes\n", scc_cnt, List_size (scc));
		);

      head_node = NULL;
      List_start (scc);
      while ((node = List_next (scc)))
        {
	  if (node->data.var_size > IPA_POINTER_SIZE ||
	      IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_ESCLOCAL) ||
	      IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_HEAP))
	    continue;
#if 0
	  if (del_mode == CD_SELECT && node->data.var_size > IPA_POINTER_SIZE)
	    {
	      printf("  - skipping: size\n"); 
	      continue;
	    }
#endif
	  if (IPA_FLAG_ISSET(node->flags, dont_merge))
	    {
	      DEBUG_IPA(1, printf("  - skipping: dont_merge\n"););
	      continue;
	    }

	  /* Avoid nodes whose address is taken - very bad, but not sure why
	   */
	  elist = IPA_cg_edge_list_find (node, ASSIGN_ADDR);
	  if (elist != NULL && IPA_htab_size(elist->out) > 0)
	    continue;

if (node->data.var_size > IPA_POINTER_SIZE &&
    node->data.mod > IPA_POINTER_SIZE)
      continue;

	  if (head_node == NULL)
	    {
	      head_node = node;
	      continue;
	    }

	  assert(node != head_node);
	  assert(!IPA_cg_node_is_child (node));
	  assert(node->cgraph == cg);
	  assert(head_node->cgraph == cg);

	  DEBUG_IPA(1, printf("ACYCLE: merge :");
		    IPA_cg_node_print(stdout, head_node, IPA_PRINT_ASCI);
		    IPA_cg_node_print(stdout, node, IPA_PRINT_ASCI);
		    printf("\n"););

	  switch(del_mode)
	    {
	    case CD_DELETE_ALL:
	      IPA_cg_merge_nodes (head_node, node, 1, NULL, 0);
	      break;
	    case CD_MERGE_ALL:
	      IPA_cg_merge_nodes (head_node, node, 0, NULL, 0);
	      break;
	    case CD_SELECT:
	      /* Save non-summary, esclocal, and parent nodes */
	      if (IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_ESCLOCAL) ||
		  IPA_FLAG_ISCLR(node->flags, IPA_CG_NODE_FLAGS_SUMMARY) ||
		  node->rep_child != NULL)
		IPA_cg_merge_nodes (head_node, node, 0, NULL, 0);
	      else
		IPA_cg_merge_nodes (head_node, node, 1, NULL, 0);
	      break;
	    default:
	      assert(0);
	    }
        }
    }

  IPA_cg_free_SCC (cg, sccs);
}

void
IPA_ad_a_cycle_detection(IPA_cgraph_t *cg,
			 int dont_merge,
			 int del_mode)
{
  IPA_cgraph_node_t *head_node;
  IPA_cgraph_node_t *node;
  IPA_cgraph_edge_list_t *elist;
  List sccs;
  List scc;
  int scc_cnt;

  sccs = IPA_cg_find_SCC (cg, (IPA_CG_ETYPE_ASSIGN | 
			       IPA_CG_ETYPE_ASSIGN_DEREF));
  scc_cnt = 0;
  List_start (sccs);
  while ((scc = List_next (sccs)))
    {
      /* Skip trivial ones (all self-recusion
         has already been handled during loading */
      if (List_size (scc) == 1)
        continue;

      scc_cnt++;
#if 0
      printf ("A/AD-CYCLES : [%d] %d nodes\n", scc_cnt, List_size (scc));
#endif

      head_node = NULL;
      List_start (scc);
      while ((node = List_next (scc)))
        {
	  if (IPA_FLAG_ISSET(node->flags, dont_merge))
	    {
#if 0
	      printf("  - skipping dont_merge node\n");
#endif
	      continue;
	    }

	  /* Avoid nodes whose address is taken - very bad, but not sure why
	   */
	  elist = IPA_cg_edge_list_find (node, ASSIGN_ADDR);
	  if (elist != NULL && IPA_htab_size(elist->out) > 0)
	    continue;

if (node->data.var_size > IPA_POINTER_SIZE &&
    node->data.mod > IPA_POINTER_SIZE)
      continue;

	  if (head_node == NULL)
	    {
	      head_node = node;
	      continue;
	    }
	  assert(node != head_node);
	  assert(!IPA_cg_node_is_child (node));
	  assert(node->cgraph == cg);
	  assert(head_node->cgraph == cg);

	  switch(del_mode)
	    {
	    case CD_DELETE_ALL:
	      IPA_cg_merge_nodes (head_node, node, 1, NULL, 0);
	      break;
	    case CD_MERGE_ALL:
	      IPA_cg_merge_nodes (head_node, node, 0, NULL, 0);
	      break;
	    case CD_SELECT:
	      /* Save non-summary and esclocal nodes */
	      if (IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_ESCLOCAL) ||
		  IPA_FLAG_ISCLR(node->flags, IPA_CG_NODE_FLAGS_SUMMARY) ||
		  node->rep_child != NULL)
		IPA_cg_merge_nodes (head_node, node, 0, NULL, 0);
	      else
		IPA_cg_merge_nodes (head_node, node, 1, NULL, 0);
	      break;
	    default:
	      assert(0);
	    }
        }

      if (head_node)
	{
	  IPA_consg_ensure_edge(ASSIGN_DEREF, head_node, head_node, 
				0, IPA_POINTER_SIZE, 0, 
				(IPA_CG_EDGE_FLAGS_IMPLICIT |
				 IPA_CG_EDGE_FLAGS_HZ));
	}
    }
  IPA_cg_free_SCC (cg, sccs);  
}

void
IPA_k_a_cycle_detection(IPA_cgraph_t *cg)
{
  IPA_cgraph_edge_list_t *elist = NULL;
  IPA_cgraph_edge_t *edge = NULL;
  IPA_HTAB_ITER eiter;
  IPA_cgraph_node_t *node;
  IPA_cgraph_node_t *head_node;
  List sccs;
  List scc;
  int scc_cnt;
  int skew;
  static int cnt = 0;

  sccs = IPA_cg_find_SCC (cg, (IPA_CG_ETYPE_ASSIGN | 
			       IPA_CG_ETYPE_SKEW));
  scc_cnt = 0;
  List_start (sccs);
  while ((scc = List_next (sccs)))
    {
      /* This misses the simple cycles, but is
	 used for now to speed things up */
      if (List_size (scc) == 1)
        continue;

      List_start (scc);
      while ((node = List_next (scc)))
        {
	  IPA_FLAG_SET(node->flags, IPA_CG_NODE_FLAGS_GENERIC10);
	}

      edge = NULL;
      skew = 0;
      List_start (scc);
      while ((node = List_next (scc)))
	{
	  elist = IPA_cg_edge_list_find (node, SKEW);
	  if (!elist)
	    continue;
	  IPA_HTAB_START(eiter, elist->out);
	  IPA_HTAB_LOOP(eiter)
	    {
	      edge = IPA_HTAB_CUR(eiter);
	      if (!IPA_FLAG_ISSET(edge->dst_elist->node->flags, 
				  IPA_CG_NODE_FLAGS_GENERIC10))
		continue;
	      printf("A/K CYCLE skew %d\n",edge->data.assign_size);
	      skew = edge->data.assign_size;
	      break;
	    }
	}
      /* This detects pure cycles too, so skip the
	 pure ones */
      if (!edge || !skew)
	continue;

      scc_cnt++;
#if 1
      printf ("[%d] A/K-CYCLES : [%d] %d nodes\n", 
	      cnt++, scc_cnt, List_size (scc));
#endif

      /* First add in an ASSIGN edge identical to the SKEW 
	 in other respects (need to preserve the cycle). */
      IPA_consg_ensure_edge_d(ASSIGN,
			      edge->src_elist->node,
			      edge->dst_elist->node,
			      &edge->data, 
			      edge->flags);

      /* Now, delete the SKEW edge in question */
      IPA_cg_edge_delete(edge);

      /* Set the k_cycle to the skew and merge any nodes that
	 can be merged */
      head_node = NULL;
      List_start (scc);
      while ((node = List_next (scc)))
        {
	  printf("%s: %d.%d\n",
		 cg->data.fninfo->func_name,
		 node->data.var_id, node->data.version);
	  IPA_FLAG_CLR(node->flags, IPA_CG_NODE_FLAGS_GENERIC10);

	  /* Set k_cycle */
	  assert(skew > 0);
	  node->data.in_k_cycle = skew;

	  /* Avoid merging nodes whose address is taken  
	   */
	  elist = IPA_cg_edge_list_find (node, ASSIGN_ADDR);
	  if (elist  && IPA_htab_size(elist->out) > 0)
	    continue;

	  if (!head_node)
	    {
	      head_node = node;
	      continue;
	    }

	  IPA_cg_merge_nodes (head_node, node, 0, NULL, 0); 
        }
    }
  
  IPA_cg_free_SCC (cg, sccs);  
}

void
IPA_cycle_detection(IPA_cgraph_t *cg,
		    int dont_merge,
		    int del_mode)
{
#if 1
  IPA_a_cycle_detection(cg, dont_merge, del_mode);
#endif
#if 1
  IPA_ad_a_cycle_detection(cg, dont_merge, del_mode);
#endif
}

/*****************************************************************************
 * Find SCCs, merge nodes, merge/update constraint graphs. 
 * Converts callgraph into an acyclic graph.
 *****************************************************************************/

void
IPA_callgraph_merge_cycles (IPA_prog_info_t * info)
{
  IPA_callg_node_t *head_node;
  IPA_callg_node_t *node;
  IPA_callg_t *callg;
  List sccs;
  List scc;
  int scc_cnt;

  callg = info->call_graph;
  sccs = IPA_callg_find_SCC (callg);
  DEBUG_IPA(1, IPA_callg_print_SCC(stdout, sccs););

  scc_cnt = 0;
  List_start (sccs);
  while ((scc = List_next (sccs)))
    {
      /* Skip all trivial ones */
      if (List_size (scc) == 1)
        continue;
      scc_cnt++;
      printf ("CALLG Merging %d nodes in SCC %d\n",  
	      List_size (scc), scc_cnt);

      head_node = NULL;
      while ((node = List_next (scc)))
        {
#if 0
	  printf("  - [%s] \n",
		 node->fninfo->func_name);
#endif
	  /* This may look a little strange. Basically, don't
	     merge non-parents. */
	  if (node->rep_parent != node)
	    {
	      DEBUG_IPA(1, printf("  - [%s] already in [%s]\n",
				  node->fninfo->func_name,
				  node->rep_parent->fninfo->func_name););
	      continue;
	    }

	  /* Find a parent-node for the head */
	  if (head_node == NULL)
	    {
	      head_node = node;
	      continue;
	    }
	  assert(node != head_node);

	  /* This merges the call graph nodes and
	     merges the constraint graphs */
          IPA_callgraph_node_merge (head_node, node);
        }

      printf("CALLG Final SCC node count %d\n",
	     IPA_htab_size(head_node->fninfo->consg->nodes));
      if (IPA_htab_size(head_node->fninfo->consg->nodes) > 2500)
	{
	  IPA_callgraph_node_invalidate_summaries2(head_node);
	  IPA_consg_make_cg_ci(info, head_node->fninfo->consg);
	}
    }

  IPA_callg_free_SCC (callg, sccs);
}

/* This is used by the Context Sensitive Algorithm */
void
IPA_callgraph_prepare_all (IPA_prog_info_t * info)
{
  IPA_funcsymbol_info_t *fninfo;

  IPA_callgraph_merge_cycles (info);
  
  List_start(info->fninfos);
  while ((fninfo = List_next(info->fninfos)))
    {
      if (!fninfo->has_been_called)
	continue;

      /* There may have been self-call edges existing before or 
	 due to the merging. Make sure the constraints exist to
	 handle these situations.  */
      IPA_callgraph_handle_selfedges (fninfo->call_node);
      
      /* Make sure that the param/ret are attached */
      IPA_callgraph_attach_caller_params (info, fninfo->call_node);
      IPA_callgraph_attach_caller_rets (info, fninfo->call_node);
    } 
}


/* This is used by the Context _In_sensitive Algorithm */
void
IPA_callgraph_connect_all (IPA_prog_info_t * info)
{
  IPA_funcsymbol_info_t *fninfo;

  List_start(info->fninfos);
  while ((fninfo = List_next(info->fninfos)))
    {
      if (!fninfo->has_been_called)
	continue;

      /* There may have been self-call edges existing before or 
	 due to the merging. Make sure the constraints exist to
	 handle these situations.  */
      IPA_callgraph_handle_selfedges (fninfo->call_node);

      /* Make sure that the param/ret are attached */
      IPA_callgraph_attach_caller_params (info, fninfo->call_node);
      IPA_callgraph_attach_caller_rets (info, fninfo->call_node);
    }  
}


/*****************************************************************************
 * Given new constraint edges, add in any new calls
 *****************************************************************************/

List
IPA_callgraph_new_callees (IPA_prog_info_t * info, 
			   List update_list,
			   List callee_delta,
			   int round)
{
  IPA_callg_node_t      *caller_callg_node;
  IPA_funcsymbol_info_t *caller_fninfo;
  IPA_callsite_t        *caller_cs;
  IPA_cgraph_edge_t     *edge;
  IPA_cgraph_node_t     *consg_node;
  IPA_cgraph_node_t     *control_node;

  List_start (callee_delta);
  while ((edge = List_next (callee_delta)))
    {
      if (edge->src_elist->edge_type != ASSIGN_ADDR)
	continue;

      /* Look through all (callee naming) represented src_nodes */
      for (consg_node = edge->src_elist->node;
	   consg_node; consg_node = consg_node->rep_child)
	{
	  /* Because of some node merging, their may be non-funcs
	     mixed in with the funcs */
	  if (IPA_FLAG_ISCLR (consg_node->flags, IPA_CG_NODE_FLAGS_FUNC))
	    continue;

	  /* Find the caller */
	  control_node = edge->dst_elist->node;
	  caller_fninfo = control_node->data.syminfo->fninfo;
	  caller_callg_node = caller_fninfo->call_node;
	  
	  /* Look for indirect callsite in caller with root_id
	     of cc_node's var_id */
	  List_start (caller_fninfo->callsites);
	  while ((caller_cs = List_next (caller_fninfo->callsites)))
	    {
	      if (!caller_cs->indirect)
		continue;
	      if (caller_cs->callee.cnode_id == control_node->data.var_id)
		break;
	    }
	  if (!caller_cs ||
	      IPA_FLAG_ISCLR (edge->dst_elist->node->flags, IPA_CG_NODE_FLAGS_CALLEE))
	    {
	      printf("Could not find callee\n");
	      IPA_cg_node_print (stdout, control_node, IPA_PRINT_ASCI); 
	      printf("\n");
	      printf("fn %s fn %s\n",
		     control_node->cgraph->data.fninfo->func_name,
		     caller_fninfo->func_name);
	      printf("flags %x\n",control_node->flags);
	      printf("edge %p\n",edge);
	      printf("src_node ");
	      IPA_cg_node_print (stdout, consg_node, IPA_PRINT_ASCI);
	      printf("\n");	      
	      assert (0);
	    }

#if 0
	  printf ("New caller [%s:%d] -> callee [%s]\n",
		  caller_fninfo->func_name,
		  caller_cs->cs_id,
		  consg_node->data.syminfo->symbol_name);
#endif
	  
	  /* Schedule call graph update */
	  update_list = 
	    IPA_callg_update_add(update_list,
				 caller_callg_node,
				 caller_cs, 
				 consg_node->data.syminfo->symbol_name,
				 consg_node->data.syminfo->sym_key);
	}
    }

  return update_list;
}


/*****************************************************************************
 * If necessary, create a new summary for the specified function
 *****************************************************************************/
static IPA_funcsymbol_info_t*
IPA_get_consg_owner_fninfo(IPA_callg_node_t *call_node)
{
  if (IPA_csrec_option != IPA_CSREC_FULL)
    call_node = IPA_callg_node_get_rep(call_node);
  return call_node->fninfo;
}

static IPA_cgraph_t *
IPA_get_consg_owner_consg(IPA_callg_node_t *call_node)
{
  IPA_funcsymbol_info_t *fninfo;
  fninfo = IPA_get_consg_owner_fninfo(call_node);
  return fninfo->consg;
}

static List
IPA_list_all_heap_nodes(IPA_cgraph_t  *consg)
{
  IPA_cgraph_node_t *node;
  IPA_HTAB_ITER niter;
  List list = NULL;

  if (!consg)
    return NULL;

  /* Look at each node */
  IPA_HTAB_START(niter, consg->nodes);
  IPA_HTAB_LOOP(niter)
    {
      node = (IPA_cgraph_node_t *)IPA_HTAB_CUR(niter);
      if (!IPA_FLAG_ISSET (node->flags, IPA_CG_NODE_FLAGS_HEAP))
	continue;
      list = List_insert_last(list, node);
    }

  return list;
}

int
IPA_changed_observed(IPA_prog_info_t * info,
		     IPA_funcsymbol_info_t *fninfo)
{
  int old = fninfo->observed;
  List setlist;
  IPA_cgraph_t          *parent_consg;
  IPA_funcsymbol_info_t *parent_fninfo;
  IPA_cgraph_node_t     *node;

#if PARTIALSUM
  parent_fninfo = IPA_get_consg_owner_fninfo(fninfo->call_node);
  parent_consg = parent_fninfo->consg;

  if (!parent_consg || parent_fninfo->forced_ci)
    return 0;

  IPA_init_escape_analysis(info, parent_consg, 
			   fninfo->iface, fninfo);
  setlist = IPA_do_escape_analysis();
  List_reset(setlist);

  IPA_cg_nodes_clr_flags(parent_consg, (EA_FLAGSET | 
					EA_INPROGRESS | EA_LCONT_ESC |
					EA_LPROP_ESC | EA_RETPROP_ESC |
					EA_LFULL_ESC | EA_LCONT_ESCLCL)
			 );
  IPA_cg_nodes_assert_clr_flags(parent_consg, (EA_SURVIVES | EA_PERMANENT));

  if (old != fninfo->observed)
    {
/*       printf("OBSERVED %d -> %d\n", */
/* 	     old, fninfo->observed); */
      return 1;
    }
#endif
  return 0;
}

static void
IPA_callgraph_create_callee_summary2 (IPA_prog_info_t * info,
				      IPA_funcsymbol_info_t *callee_fninfo,
				      int round)
{
  IPA_cgraph_t          *parent_consg;
  IPA_funcsymbol_info_t *parent_fninfo;
  IPA_callg_node_t      *call_node;

  /* Summary using parent constraint graph and 
   *  particular callees param/return 
   */

  /* Get the parent (in case of SCC) */
  call_node = callee_fninfo->call_node;
  assert (call_node->fninfo != info->globals);

  /* If cs recursion, then summarize the function otherwise 
     the SCC parent function */
  parent_fninfo = IPA_get_consg_owner_fninfo(call_node);
  parent_consg = parent_fninfo->consg;

  if (callee_fninfo->summary_valid)
    {
      /* Summary is still valid, so do not rebuild it 
       */
      return;
    }

  /* Clear/Free old summary */
  IPA_cg_cgraph_free (callee_fninfo->lsum_consg);
  callee_fninfo->lsum_consg = NULL;

  /* Don't do anything if function has a known zero sized summary */
  if (!parent_consg || parent_fninfo->forced_ci)
    {
      DEBUG_IPA(0, printf("Summary of [%s] : Still valid. (No consg) \n",
			  callee_fninfo->func_name););
      callee_fninfo->summary_valid = 1;
      callee_fninfo->summary_size = 0;
      return;
    }
  
  /* Summarize the function (and set summary_size) */
  IPA_driver_summarize_graph (info, 
			      parent_consg, 
			      callee_fninfo->iface,
			      callee_fninfo,
			      &(callee_fninfo->lsum_consg),
			      callee_fninfo->func_name,
			      IPA_field_option);
  
  /* Summary is now valid. It is rebuilt if non-empty.  */
  callee_fninfo->summary_valid = 1;
  
  DEBUG_IPA(0, printf("Summary of [%s] : %d -> %d nodes\n",
		      callee_fninfo->func_name, 
		      IPA_htab_size(parent_consg->nodes),
		      IPA_htab_size(callee_fninfo->lsum_consg->nodes)););
  
  /* Debugging: Print pre-summary, local summary, and global summary */
  if (IPA_print_summary_cng)
    {
      char name[256];
      sprintf(name, "%s.SUM%d", callee_fninfo->func_name, round);
      IPA_cg_DOTprint (callee_fninfo->lsum_consg, name, IPA_CG_ETYPE_ALL);
    }

#if 0
  if (strstr(callee_fninfo->func_name, "simp_comp"))
    {
      static int count;
      char name[256];
      sprintf(name, "%s.SUM%d_%d_%d", 
	      callee_fninfo->func_name, 
	      round, 
	      count++,
	      IPA_htab_size(callee_fninfo->lsum_consg->nodes));
      IPA_cg_DOTprint (callee_fninfo->lsum_consg, name, IPA_CG_ETYPE_ALL);
    }
#endif
}



/*****************************************************************************
 * Invalidate all the summaries for all calls to this function
 *****************************************************************************/

void
invalidate_single(IPA_callg_node_t *callg_node)
{
  IPA_callg_edge_t *caller_edge;

  callg_node->fninfo->summary_valid = 0;
  
  /* For each caller of the func */
  List_start(callg_node->caller_edges);
  while ((caller_edge = List_next(callg_node->caller_edges)))
    {
      caller_edge->summary_incorporated = 0;
    }
}

void
IPA_callgraph_node_invalidate_summaries2(IPA_callg_node_t *callg_node)
{
  IPA_callg_edge_t *caller_edge;

  DEBUG_IPA(0, printf("Invalidate summaries of [%s]\n",
		      callg_node->fninfo->func_name););

  if (IPA_csrec_option == IPA_CSREC_FULL)
    {
      callg_node->fninfo->summary_valid = 0;
      
      /* For each caller of the func */
      List_start(callg_node->caller_edges);
      while ((caller_edge = List_next(callg_node->caller_edges)))
	{
	  caller_edge->summary_incorporated = 0;
	}      
    }
  else
    {
      assert(callg_node->rep_parent == callg_node);

      /* For each call graph node in the SCC */
      for (; callg_node; callg_node = callg_node->rep_child)
	{
	  callg_node->fninfo->summary_valid = 0;
	  
	  /* For each caller of the func */
	  List_start(callg_node->caller_edges);
	  while ((caller_edge = List_next(callg_node->caller_edges)))
	    {
	      caller_edge->summary_incorporated = 0;
	    }
	}
    }
}



/*****************************************************************************
 * Apply summaries to the function (Create, Invalidate as needed) 
 *****************************************************************************/

static List
prepare_for_summary(IPA_prog_info_t * info,
		    IPA_callg_node_t *root_node,
		    IPA_callg_edge_t *callee_edge,
		    List apply_list,
		    int round)
{
  IPA_funcsymbol_info_t *callee_fninfo;

  callee_fninfo = callee_edge->callee->fninfo;
  if (IPA_FLAG_ISSET(callee_edge->flags, IPA_CALLG_EDGE_FLAGS_SELFEDGE))
    {      
      return apply_list;
    }

  assert(callee_fninfo != root_node->fninfo);
  
  /* Generate summary as needed */
  IPA_callgraph_create_callee_summary2 (info, callee_fninfo, round);
  assert(callee_fninfo->summary_valid);

  /* summary_size should be the pre-simplification size. If it is
   *  unchanged the summary is unchanged (for now this is only used
   *  by the REC-CS mode, but may be used in the future for the general
   *  case.
   */
  if (IPA_csrec_option == IPA_CSREC_FULL && 
      callee_edge->previous_sumsize == callee_fninfo->summary_size)
    {
      DEBUG_IPA(0, printf("FNSum: [%s] : REC-SKIP %d %d\n",
			  callee_fninfo->func_name,
			  callee_fninfo->summary_size,
			  callee_edge->previous_sumsize););
      callee_edge->summary_incorporated = 1;
    }
  
  /* Has the summary for this call graph edge been accounted for? */
  if (callee_edge->summary_incorporated)
    {
      DEBUG_IPA(0, printf("FNSum: [%s] : SKIP1 \n",
			  callee_fninfo->func_name););
      return apply_list;
    }
  
  /* Clear out previous summary nodes */
  {
    List hplist;
    hplist = IPA_list_all_heap_nodes(callee_fninfo->lsum_consg);
    IPA_consg_delete_summary_nodes2(callee_edge, hplist);
    List_reset(hplist);
  }

  /* If the summary is zero sized, there is nothing to apply
   */
  if ((!callee_fninfo->lsum_consg) ||
      (IPA_htab_size(callee_fninfo->lsum_consg->nodes) == 0))
    {
      DEBUG_IPA(0, printf("FNSum: [%s] : SKIP2 \n",
			  callee_fninfo->func_name););
      callee_edge->summary_incorporated = 1;
      
      /* If, for the previous time the summary was applied for this call,
       *    the summary was non-zero, we want to invalidate
       *   1) the new one is non-zero and will be incorporated
       *   2) the new one is empty (e.g. FORCE CI) and callers
       *        may leverage a smaller summary from this function
       */
      if (callee_edge->previous_sumsize != 0)
	{
	  /* Case 2) described above */
	  IPA_callgraph_node_invalidate_summaries2(root_node);
	}

      callee_edge->previous_sumsize = 0;
      return apply_list;
    }

  /* Case 1) described above */
  IPA_callgraph_node_invalidate_summaries2(root_node);
  
  /* A new, non-zero summary */
  apply_list = List_insert_last(apply_list, callee_edge);

  return apply_list;
}


static void
apply_new_summary(IPA_prog_info_t * info,
		  IPA_callg_node_t * root_node,
		  IPA_callg_edge_t *callee_edge)
{
  IPA_callg_node_t *callee_parent_call_node;
  IPA_cgraph_t     *callee_parent_consg;
  IPA_funcsymbol_info_t *callee_fninfo;
  
  callee_fninfo = callee_edge->callee->fninfo;

  if (IPA_csrec_option == IPA_CSREC_FULL)
    callee_parent_call_node = callee_edge->callee;
  else
    callee_parent_call_node = IPA_callg_node_get_rep(callee_edge->callee);

  callee_parent_consg = callee_parent_call_node->fninfo->consg;
  
#if 0
  printf("FNSum: [%s:%d] : %d nodes \n", 
	 callee_fninfo->func_name, 
	 callee_edge->caller_cs->cs_id,
	 IPA_htab_size(callee_fninfo->lsum_consg->nodes));
#endif
  callee_edge->previous_sumsize =  callee_fninfo->summary_size;  
  IPA_consg_apply_summary2 (info,
			    root_node->fninfo->consg,
			    callee_edge->caller_cs->iface,
			    callee_fninfo->lsum_consg,
			    callee_edge->callee_if,
			    callee_edge->caller_cs->version_htab,
				/* Used for U/D edges for HEAP/ESC LOCALS */
			    callee_parent_consg,
				/* Holds a list of all summary nodes added */
			    callee_edge);

  /*IPA_prog_classify_consg(info, root_node->fninfo->consg);*/
}

static void
IPA_callgraph_apply_callee_summaries_SCC (IPA_prog_info_t * info,
					  IPA_callg_node_t * root_node,
					  int round);

void
IPA_callgraph_apply_callee_summaries2 (IPA_prog_info_t * info,
				       IPA_callg_node_t * root_node,
				       int round)
{
  IPA_callg_edge_t *callee_edge;
  IPA_callg_node_t *callg_node;
  List apply_list;
  int before_size;

  assert(root_node->rep_parent == root_node);
  if (IPA_csrec_option == IPA_CSREC_FULL && 
      root_node->rep_child != NULL)
    {
      /* Handle SCCs in a special manner */
      IPA_callgraph_apply_callee_summaries_SCC(info,root_node,round);
      return;
    }

  before_size = IPA_htab_size(root_node->fninfo->consg->nodes);

  /* STEP 1 - On-demand summarization of callees, clear out old summary 
   *          nodes, invalidate summary of this function, and queue up callees
   *          whose summaries need to be reapplied
   */

  /* For each call graph node in the SCC */
  apply_list = NULL;
  for (callg_node = root_node; callg_node; callg_node = callg_node->rep_child)
    {
      /* For each callee of the func */
      List_start(callg_node->callee_edges);
      while ((callee_edge = List_next(callg_node->callee_edges)))
	{
	  apply_list = prepare_for_summary(info, root_node, callee_edge, 
					   apply_list, round);
	} /* edge in func */
    } /* func in SCC */


  /* STEP 2 - Apply callee summaries (since funcs in SCCs share constraint
   *                   graphs this needs to be done _after_ all of the 
   *                   deletions - if B & C are in an SCC and a single
   *                   call site in A calls both B & C, B-sum and C-sum
   *                   may share nodes in A making it bad to reapply B-sum
   *                   and then delete C-sum (which the new B-sum may be
   *                   relying upon)
   */
  List_start(apply_list);
  while ((callee_edge = List_next(apply_list)))
    {
      apply_new_summary(info, root_node, callee_edge);
    }
  
  List_reset(apply_list);
  DEBUG_IPA(1, printf("FNSum: APPLIED [%s] : [%d -> %d] nodes \n", 
		      root_node->fninfo->func_name, 
		      before_size,
		      IPA_htab_size(root_node->fninfo->consg->nodes)););
}



/*
 * The following is EXPERIMENTAL. 
 */

static void
IPA_callgraph_apply_callee_summaries_SCC (IPA_prog_info_t * info,
					  IPA_callg_node_t * root_node,
					  int round)
{
  IPA_callg_node_t *callg_node;
  IPA_callg_edge_t *callee_edge;
  int changed;
  int sccr = 0;

  do
    {
      printf("REC_CS: ROUND %d\n", sccr++); fflush(stdout);
      if (sccr == 15)
	break;

      /* For each function in SCC
       */
      changed = 0;
      for (callg_node = root_node; callg_node; callg_node = callg_node->rep_child)
	{
	  List apply_list;
	  int before_size;

	  before_size = IPA_htab_size(callg_node->fninfo->consg->nodes);
	  
	  /* For each callee
	   */
	  apply_list = NULL;
	  List_start(callg_node->callee_edges);
	  while ((callee_edge = List_next(callg_node->callee_edges)))
	    {
	      apply_list = prepare_for_summary(info, callg_node, callee_edge, 
					       apply_list, round);
	    }
	     
	  if (List_size(apply_list) > 0)
	    {
	      changed = 1;

	      List_start(apply_list);
	      while ((callee_edge = List_next(apply_list)))
		{
		  apply_new_summary(info, callg_node, callee_edge);
		}
	    }

	  printf("REC_CS [%s]: APPLIED, FUNC [%d -> %d] nodes\n", 
		 callg_node->fninfo->func_name, 
		 before_size, 
		 IPA_htab_size(callg_node->fninfo->consg->nodes));
	  if ((IPA_htab_size(callg_node->fninfo->consg->nodes) > 50) && 
	      ((IPA_htab_size(callg_node->fninfo->consg->nodes) - callg_node->fninfo->orig_size) >
	       callg_node->fninfo->orig_size))
	    {
	      printf("REC_CS [%s]: APPLIED, FORCE CI   [%d -> %d]\n", 
		     callg_node->fninfo->func_name,
		     callg_node->fninfo->orig_size,
		     IPA_htab_size(callg_node->fninfo->consg->nodes));
	      IPA_callgraph_node_invalidate_summaries2(callg_node);
	      IPA_consg_make_cg_ci(info, callg_node->fninfo->consg);
	    }
	  else if (IPA_htab_size(callg_node->fninfo->consg->nodes) > callg_node->fninfo->orig_size)
	    {
	      printf("REC_CS [%s]: APPLIED, INCR   [%d -> %d]\n", 
		     callg_node->fninfo->func_name,
		     callg_node->fninfo->orig_size,
		     IPA_htab_size(callg_node->fninfo->consg->nodes));	      
	    }

	  fflush(stdout);
	  List_reset(apply_list);
	}

      sccr++;
    }
  while (changed);
}


/*********************************************************************
 *  A inter-graph connectioncopy based approach to incorporating 
 *    global/param info during top-down propagation
 *********************************************************************/

void
IPA_callgraph_attach_caller_params_edge(IPA_prog_info_t * info,
					IPA_callg_edge_t *edge)
{
  IPA_cgraph_t *caller_consg;
  IPA_cgraph_t *callee_consg;

  caller_consg = IPA_get_consg_owner_consg(edge->caller);
  callee_consg = IPA_get_consg_owner_consg(edge->callee);

  /* This means that they are part of the same function
     (SCC merge), so this should not be done */
  if (caller_consg == callee_consg)
    return;

#if 0
  printf("ATTACH PARAM: Caller %s %s CALLEE %s %s \n",
	 edge->caller->fninfo->func_name,
	 caller_consg->data.fninfo->func_name,
	 edge->callee->fninfo->func_name,
	 callee_consg->data.fninfo->func_name);
#endif  
  
  IPA_consg_assign_params (caller_consg, edge->caller_cs->iface,
			   callee_consg, edge->callee_if,
			   IPA_CG_EDGE_FLAGS_EXPLICIT,
			   (IPA_CG_EDGE_FLAGS_EXPLICIT |
			    IPA_CG_EDGE_FLAGS_DN));
}

void
IPA_callgraph_attach_caller_params (IPA_prog_info_t * info,
				    IPA_callg_node_t * callg_node)
{
  IPA_callg_edge_t *edge;
  IPA_callg_node_t *tmp_callg_node;

  if (IPA_csrec_option != IPA_CSREC_FULL)
    {
      if (callg_node->rep_parent != callg_node)
	return;
    }

  /* For each caller of the func */
  for (tmp_callg_node = callg_node;
       tmp_callg_node; tmp_callg_node = tmp_callg_node->rep_child)
    {
      List_start(tmp_callg_node->caller_edges);
      while ((edge = List_next(tmp_callg_node->caller_edges)))
	{
	  IPA_callgraph_attach_caller_params_edge(info, edge);
	}
    }
}

void
IPA_callgraph_attach_caller_rets_edge (IPA_prog_info_t * info,
				       IPA_callg_edge_t *edge)
{
  IPA_cgraph_t *caller_consg;
  IPA_cgraph_t *callee_consg;

  caller_consg = IPA_get_consg_owner_consg(edge->caller);
  callee_consg = IPA_get_consg_owner_consg(edge->callee);
      
  /* This means that they are part of the same function
     (SCC merge), so this should not be done */
  if (caller_consg == callee_consg)
    return;

#if 0
  printf("ATTACH RETURN: Caller %s %s CALLEE %s %s \n",
	 edge->caller->fninfo->func_name,
	 caller_consg->data.fninfo->func_name,
	 edge->callee->fninfo->func_name,
	 callee_consg->data.fninfo->func_name);
#endif
  
  IPA_consg_assign_return (caller_consg, edge->caller_cs->iface,
			   callee_consg, edge->callee_if,
			   IPA_CG_EDGE_FLAGS_EXPLICIT,
			   (IPA_CG_EDGE_FLAGS_EXPLICIT |
			    IPA_CG_EDGE_FLAGS_UP));
}

void
IPA_callgraph_attach_caller_rets (IPA_prog_info_t * info,
				  IPA_callg_node_t * callg_node)
{
  IPA_callg_edge_t *edge;
  IPA_callg_node_t *tmp_callg_node;

  if (IPA_csrec_option != IPA_CSREC_FULL)
    {
      if (callg_node->rep_parent != callg_node)
	return;
    }

  /* For each caller of the func */
  for (tmp_callg_node = callg_node;
       tmp_callg_node; tmp_callg_node = tmp_callg_node->rep_child)
    {
      List_start(tmp_callg_node->caller_edges);
      while ((edge = List_next(tmp_callg_node->caller_edges)))
	{
	  IPA_callgraph_attach_caller_rets_edge (info, edge);
	}
    }
}



