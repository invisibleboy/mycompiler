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
 *      File:    pipa_stats.c
 *      Author:  Erik Nystrom
 *      Copyright (c) 2003  Erik Nystrom, Wen-mei Hwu
 *              and The Board of Trustees of the University of Illinois.
 *              All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#include <pipa_consg.h>
#include <pipa_callgraph.h>
#include <pipa_print_graph.h>

/*******************************************************************
 * Stats Logging
 *******************************************************************/

typedef struct ipa_bucket_t
{
  int cmp_val;
  int cnt_match;
  int cnt_abs;
} ipa_bucket_t;

typedef struct ipa_fld_t
{
  int offset;
  int count;
  struct ipa_fld_t *nxt;
} ipa_fld_t;


static void
IPS_stat_init_bucket(ipa_bucket_t *buckets)
{
  int cmp_val;
  int bk;

  for (cmp_val=1, bk = 2; bk < 25; bk++)
    {
      buckets[bk].cmp_val = cmp_val;
      buckets[bk].cnt_match = 0;
      buckets[bk].cnt_abs = 0;
      
      if (bk & 0x1)
	cmp_val = cmp_val * 2;
      else
	cmp_val = cmp_val * 5;
    }

  buckets[0].cmp_val = -1;
  buckets[0].cnt_match = 0;
  buckets[0].cnt_abs = 0;
  buckets[1].cmp_val = 0;
  buckets[1].cnt_match = 0;
  buckets[1].cnt_abs = 0;
}

static void
IPS_stat_bucket(ipa_bucket_t *buckets, int count)
{
  int bk;

  for (bk = 1; bk < 25; bk++)
    {
      if (count > buckets[bk].cmp_val)
	continue;
      buckets[bk].cnt_match++;
      buckets[bk].cnt_abs += count;
      break;
    }
  buckets[0].cnt_match++;
  buckets[0].cnt_abs += count;
}

static void
IPA_print_bucket(ipa_bucket_t *buckets, char *str)
{
  int bk, bk_max = 0;

  for (bk = 1; bk < 25; bk++)
    {
      if (buckets[bk].cnt_match != 0)
	bk_max = bk+1;
    }
  
  printf("%20s %10s",str,"");
  for (bk = 0; bk < bk_max; bk++)
    {
      if (bk != 0)
	printf("%8d ", buckets[bk].cmp_val);
      else
	printf("%8s ", "TOTAL");
    }
  printf("\n");

  printf("%20s %10s",str,"Match");
  for (bk = 0; bk < bk_max; bk++)
    {
      printf("%8d ", buckets[bk].cnt_match);
    }
  printf("\n");

  printf("%20s %10s",str,"Raw");
  for (bk = 0; bk < bk_max; bk++)
    {
      printf("%8d ", buckets[bk].cnt_abs);
    }
  printf("\n");
  
  if (buckets[0].cnt_abs > 0 &&
      buckets[0].cnt_match > 0)
    {
      printf("%20s %10s",str,"Match %");
      for (bk = 0; bk < bk_max; bk++)
	{
	  printf("%8.1f ", 100*((double)buckets[bk].cnt_match) / buckets[0].cnt_match);
	}
      printf("\n");

      printf("%20s %10s",str,"Raw %");
      for (bk = 0; bk < bk_max; bk++)
	{
	  printf("%8.1f ", 100*((double)buckets[bk].cnt_abs) / buckets[0].cnt_abs);
	}
      printf("\n");
    }
}

#if 0
static ipa_fld_t *
IPA_stat_count_pt(ipa_fld_t *fld_cnt, int off)
{
  ipa_fld_t *ptr;

  for (ptr=fld_cnt; ptr; ptr=ptr->nxt)
    {
      if (ptr->offset != off)
	continue;
      ptr->count++;
      return fld_cnt;
    }

  ptr = malloc(sizeof(ipa_fld_t));
  ptr->offset = off;
  ptr->count = 1;
  ptr->nxt = fld_cnt;

  return ptr;
}
#endif

void
IPA_bucket_field_free(ipa_fld_t *fld_cnt, ipa_bucket_t *bucket)
{
  ipa_fld_t *ptr, *nxt_ptr;

  for (ptr=fld_cnt; ptr; ptr=nxt_ptr)
    {
      nxt_ptr = ptr->nxt;
      IPS_stat_bucket(bucket, ptr->count);
      free(ptr);
    }
}

typedef struct qsort_t 
{
  IPA_symbol_info_t *syminfo;
  int                cnt;
} qsort_t;

#if 0
static void
mysort(qsort_t *qa, int cnt)
{
  int i, change;
  qsort_t cur, nxt;

  do 
    {
      change = 0;
      cur = qa[0];

      for (i=1; i<cnt; i++)
	{
	  nxt = qa[i];
	  if (strcmp(cur.syminfo->symbol_name, 
		     nxt.syminfo->symbol_name) < 0)
	    {
	      /* swap */
	      qa[i-1] = nxt;
	      qa[i] = cur;
	      change = 1;
	    }
	  else
	    {
	      cur = nxt;
	    }
	}
    }
  while (change);
}
#endif

void
IPA_prog_stats_count(IPA_prog_info_t *info)
{
  /* 
   *        In Addr                      0 1 5 10 50 ...  AVG 
   * Norm Nodes(per field) Nodes Fields
   * Summary Nodes         Noded Fields
   *
   */
  IPA_funcsymbol_info_t *fninfo;
  IPA_cgraph_node_t *node;
  IPA_cgraph_edge_t *edge;
  IPA_cgraph_edge_list_t *elist;
  IPA_HTAB_ITER niter;
  IPA_HTAB_ITER eiter;
  ipa_bucket_t node_in_bucket[2][30];
  ipa_bucket_t node_out_bucket[2][30];
  ipa_bucket_t heap_in_bucket[2][30];
  ipa_bucket_t heap_out_bucket[2][30];
  int in_from_nonheap_to_heap = 0;
  int in_from_heap_to_heap = 0;
  int in_from_nonheap_to_nonheap = 0;
  int in_from_heap_to_nonheap = 0;
  int in_from_nonheap_to_heap_w = 0;
  int in_from_heap_to_heap_w = 0;
  int in_from_nonheap_to_nonheap_w = 0;
  int in_from_heap_to_nonheap_w = 0;
  int in_from_nonheap_to_heap_hc = 0;
  int in_from_heap_to_heap_hc = 0;
  int in_from_heap_to_nonheap_hc = 0;
  int heaps = 0;
  int hh_done = 0;

#if 0
  qsort_t qarray[1000];
  int     qcnt;
#endif
  int i;

  for (i=0; i<2; i++)
    {
      IPS_stat_init_bucket(node_in_bucket[i]);
      IPS_stat_init_bucket(heap_in_bucket[i]);
      IPS_stat_init_bucket(node_out_bucket[i]);
      IPS_stat_init_bucket(heap_out_bucket[i]);
    }

  List_start(info->fninfos);
  while ((fninfo = List_next(info->fninfos)))
    {
      if (!fninfo->has_been_called)
	continue;
      if (fninfo->consg == NULL)
	continue;
      
      IPA_HTAB_START(niter, fninfo->consg->nodes);
      IPA_HTAB_LOOP(niter)
	{
	  IPA_cgraph_node_t *child_node;
	  int found_addr = 0, found_assign = 0;
	  int from_heap;
	  int from_nonheap;
	  int from_heap_w;
	  int from_nonheap_w;
	  int from_heap_hc;
	  int to_heap_hc;
	  int w;

	  node = IPA_HTAB_CUR(niter);

	  /* Only look at parent nodes */
	  if (node->rep_parent != node)
	    continue;

	  if (IPA_FLAG_ISSET(node->flags, (IPA_CG_NODE_FLAGS_HEAP)))
	    heaps++;

	  for (elist=node->first_list; elist; elist=elist->nxt_list)
	    {
	      int node_in_count;
	      int node_out_count;

	      if (elist->edge_type == ASSIGN_ADDR)
		{
		  i = 0;
		  found_addr = 1;
		}
	      else if (elist->edge_type == ASSIGN)
		{
		  i = 1;
		  found_assign = 1;
		}
	      else
		continue;

	      node_in_count = 0;
	      node_out_count = 0;
	      from_heap = 0;
	      from_nonheap = 0;
	      from_heap_w = 0;
	      from_nonheap_w = 0;
	      from_heap_hc = 0;
	      to_heap_hc = 0;

	      /* The src(dst) of each IN(OUT) edge may be
		 a unified set. If there are n represented nodes
		 one edge counts as n edges */
	      IPA_HTAB_START(eiter, elist->in);
	      IPA_HTAB_LOOP(eiter)
		{
		  edge = IPA_HTAB_CUR(eiter);
		  for (child_node = edge->src_elist->node; child_node; 
		       child_node = child_node->rep_child)
		    {
		      if (!IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_HEAP) ||
			  !IPA_FLAG_ISSET(child_node->flags, (IPA_CG_NODE_FLAGS_HEAP)))
		      node_in_count ++;

#if 0
		      if (IPA_field_option == IPA_FIELD_INDEPENDENT)
			w = List_size(child_node->data.syminfo->sym_type->valid_offsets);
		      else
#endif
			w = 1;
		      assert(w >= 0);

		      if (IPA_FLAG_ISSET(child_node->flags, (IPA_CG_NODE_FLAGS_HEAP)))
			{
			  from_heap++;
			  from_heap_w += w;
			  from_heap_hc = w;
			}
		      else
			{
			  from_nonheap++;
			  from_nonheap_w += w;
			}
		    }
		}

	      IPA_HTAB_START(eiter, elist->out);
	      IPA_HTAB_LOOP(eiter)
		{
		  edge = IPA_HTAB_CUR(eiter);
		  for (child_node = edge->dst_elist->node; child_node; 
		       child_node = child_node->rep_child)
		    {
		      if (!IPA_FLAG_ISSET(child_node->flags, (IPA_CG_NODE_FLAGS_TEMP |
							      IPA_CG_NODE_FLAGS_SUMMARY)))
			node_out_count ++;

		      if (IPA_FLAG_ISSET(child_node->flags, (IPA_CG_NODE_FLAGS_HEAP)))
			{
			  to_heap_hc = 1;
			}
		    }
		}

	      /* The node itself may be a unified set. Count each
		 node separately */
	      for (child_node = node; child_node; child_node = child_node->rep_child)
		{
		  if (i == 0 &&
		      (IPA_FLAG_ISSET(child_node->flags, IPA_CG_NODE_FLAGS_HEAP) ||
		       !IPA_FLAG_ISSET(child_node->flags, (IPA_CG_NODE_FLAGS_TEMP |
							   IPA_CG_NODE_FLAGS_SUMMARY))))
		    {
#if 0
		      if (IPA_field_option == IPA_FIELD_INDEPENDENT)
			w = List_size(child_node->data.syminfo->sym_type->valid_offsets);
		      else
#endif
			w = 1;
		      if (IPA_FLAG_ISSET(child_node->flags, IPA_CG_NODE_FLAGS_HEAP))
			{
			  in_from_nonheap_to_heap += from_nonheap;
			  in_from_heap_to_heap += from_heap;		      
			  in_from_nonheap_to_heap_w += from_nonheap_w * w;
			  in_from_heap_to_heap_w += from_heap_w * w;		      

			  if (!hh_done && from_heap_hc)
			    {
			      in_from_heap_to_heap_hc += from_heap_hc * w;
			      hh_done = 1;
			    }
			}
		      else
			{
			  in_from_nonheap_to_nonheap += from_nonheap;
			  in_from_heap_to_nonheap += from_heap;
			  in_from_nonheap_to_nonheap_w += from_nonheap_w * w;
			  in_from_heap_to_nonheap_w += from_heap_w * w;

			  in_from_heap_to_nonheap_hc += from_heap_hc * w;
			  in_from_nonheap_to_heap_hc += to_heap_hc * w;
			}
		    }

		  if (IPA_FLAG_ISSET(child_node->flags, IPA_CG_NODE_FLAGS_HEAP))
		    {
		      IPS_stat_bucket(heap_in_bucket[i], node_in_count);
		      IPS_stat_bucket(heap_out_bucket[i], node_out_count);
		    }
		  else if (!IPA_FLAG_ISSET(child_node->flags, (IPA_CG_NODE_FLAGS_TEMP |
							       IPA_CG_NODE_FLAGS_SUMMARY)))
		    {
		      IPS_stat_bucket(node_in_bucket[i], node_in_count);
		      IPS_stat_bucket(node_out_bucket[i], node_out_count);

#if 0
		      if (i == 0)
			{
			  IPA_symbol_info_t *sym;
			  sym = IPA_symbol_find_by_id (info, child_node->data.var_id);
			  printf("[%s] %d : %d\n",
				 sym->symbol_name, 
				 child_node->data.var_id,
				 node_in_count);
			  qarray[qcnt].syminfo = sym;
			  qarray[qcnt].cnt = node_in_count;
			  qcnt++;
			}
#endif
		    }
		}
	    } /* elist */

	  /* If there are no edges of a particular kind, account
	     for that here */
	  for (child_node = node; child_node; child_node = child_node->rep_child)
	    {
	      if (!found_addr)
		{
		  if (IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_HEAP))
		    {
		      IPS_stat_bucket(heap_in_bucket[0], 0);
		      IPS_stat_bucket(heap_out_bucket[0], 0);
		    }
		  else if (!IPA_FLAG_ISSET(node->flags, (IPA_CG_NODE_FLAGS_TEMP |
							 IPA_CG_NODE_FLAGS_SUMMARY)))
		    {
		      IPS_stat_bucket(node_in_bucket[0], 0);
		      IPS_stat_bucket(node_out_bucket[0], 0);
		    }
		}
	      if (!found_assign)
		{
		  if (IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_HEAP))
		    {
		      IPS_stat_bucket(heap_in_bucket[1], 0);
		      IPS_stat_bucket(heap_out_bucket[1], 0);
		    }
		  else if (!IPA_FLAG_ISSET(node->flags, (IPA_CG_NODE_FLAGS_TEMP |
							 IPA_CG_NODE_FLAGS_SUMMARY)))
		    {
		      IPS_stat_bucket(node_in_bucket[1], 0);
		      IPS_stat_bucket(node_out_bucket[1], 0);
		    }
		}
	    }
	} /* nodes */
    }

  IPA_print_bucket(node_in_bucket[0],  "ADDR  IN Nodes");
  IPA_print_bucket(node_out_bucket[0], "ADDR OUT Nodes");
  printf("\n");

  IPA_print_bucket(heap_in_bucket[0],  "ADDR  IN Heap");
  IPA_print_bucket(heap_out_bucket[0], "ADDR OUT Heap");
  printf("\n");

  IPA_print_bucket(node_in_bucket[1],  "ASSIGN  IN Nodes");
  IPA_print_bucket(node_out_bucket[1], "ASSIGN OUT Nodes");
  printf("\n");

  IPA_print_bucket(heap_in_bucket[1],  "ASSIGN  IN Heap");
  IPA_print_bucket(heap_out_bucket[1], "ASSIGN OUT Heap");
  printf("\n");

  printf("Zero summaries: %d\n", 
	 info->zero_occur);
  printf("1-20   : %5d: %8d %8d %8d\n", 
	 info->nonzero_occur[0],
	 info->total_base_count[0],
	 info->total_comp_count[0],
	 info->total_red_count[0]	 
	 );
  printf("21-2000: %5d: %8d %8d %8d\n", 
	 info->nonzero_occur[1],
	 info->total_base_count[1],
	 info->total_comp_count[1],
	 info->total_red_count[1]	 
	 );
  printf("2001+  : %5d: %8d %8d %8d\n", 
	 info->nonzero_occur[2],
	 info->total_base_count[2],
	 info->total_comp_count[2],
	 info->total_red_count[2]	 
	 );

  printf("GDATA NONHEAP -> HEAP    %10d  %10d  %10d\n", 
	 in_from_nonheap_to_heap,
	 in_from_nonheap_to_heap_w,
	 in_from_nonheap_to_heap_hc);
  printf("GDATA    HEAP -> HEAP    %10d  %10d  %10d\n", 
	 in_from_heap_to_heap, 
	 in_from_heap_to_heap_w,
	 in_from_heap_to_heap_hc);
  printf("GDATA NONHEAP -> NONHEAP %10d  %10d  %10d\n", 
	 in_from_nonheap_to_nonheap, 
	 in_from_nonheap_to_nonheap_w,
	 in_from_nonheap_to_nonheap_w);
  printf("GDATA    HEAP -> NONHEAP %10d  %10d  %10d\n", 
	 in_from_heap_to_nonheap, 
	 in_from_heap_to_nonheap_w,
	 in_from_heap_to_nonheap_hc);

#if 0
  mysort(qarray, qcnt);
  printf("QNODES [%d]\n",qcnt);
  for (i=0; i<qcnt; i++)
    {
      printf("[%s] %d\n",
	     qarray[i].syminfo->symbol_name,
	     qarray[i].cnt);
    }
#endif
}





typedef struct stats_core_t
{
  int str[MAX_EDGELIST_TYPE];
  int art[MAX_EDGELIST_TYPE];
  int sum[MAX_EDGELIST_TYPE];
  int hh[MAX_EDGELIST_TYPE];
  int nh[MAX_EDGELIST_TYPE];
  int gbl[MAX_EDGELIST_TYPE];
  int lcl[MAX_EDGELIST_TYPE];
  int all[MAX_EDGELIST_TYPE];
} stats_core_t;

typedef struct stats_t
{
  struct {
    int art;
    int sum;
    int hp;
    int gbl;
    int lcl;
    int all;
    int kcycle;
    int mod;
    int nosum;
    int yessum;
  } nodes, fn_nodes;
  stats_core_t inter;
  stats_core_t intra;
} stats_t;

static int no_expand = 0;

static int
is_artificial(IPA_cgraph_node_t *node)
{
  if (IPA_FLAG_ISSET(node->flags, (IPA_CG_NODE_FLAGS_PARAM|
				   IPA_CG_NODE_FLAGS_RETURN)))
    return 1;
  if (IPA_FLAG_ISSET(node->flags, (IPA_CG_NODE_FLAGS_TEMP)))
    return 1;
  return 0;
}

static int
is_heap(IPA_cgraph_node_t *node)
{
  if (IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_HEAP))
    return 1;
  return 0;
}

static int
is_summary(IPA_cgraph_node_t *node)
{
  if (is_heap(node))
    return 0;
  if (IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_SUMMARY))
    return 1;
  return 0;
}

static int
is_global(IPA_cgraph_node_t *node)
{
  if (IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_GLOBAL))
    return 1;
  return 0;
}

static void
classify_core(stats_core_t *stat,
	      IPA_cgraph_node_t *src_node,
	      IPA_cgraph_node_t *dst_node,
	      IPA_cgraph_edgelist_e edge_type)
{
  int *incstat;

  if (is_artificial(src_node) || is_artificial(dst_node))
    {
      incstat = stat->art;
    }
  else if (is_summary(src_node) || is_summary(dst_node))
    {
      incstat = stat->sum;
    }
  else if (is_heap(src_node) && is_heap(dst_node))
    {
      incstat = stat->hh;
    }
  else if (is_heap(src_node) || is_heap(dst_node))
    {
      incstat = stat->nh;
    }
  else if (is_global(src_node) || is_global(dst_node))
    {
      incstat = stat->gbl;
    }
  else
    {
      incstat = stat->lcl;
    }

#if 0
  printf("%d\n",edge_type);
#endif

  incstat[0] += 1;
  incstat[edge_type] += 1;
  stat->all[0] += 1; 
  stat->all[edge_type] += 1;
}

static void
classify_single(stats_t *stats,
		IPA_cgraph_node_t *src_node,
		IPA_cgraph_node_t *dst_node,
		IPA_cgraph_edgelist_e edge_type)
{
  if (src_node->cgraph != dst_node->cgraph)
    {
      classify_core(&stats->inter, src_node, dst_node, edge_type);
    }
  else
    {
      classify_core(&stats->intra, src_node, dst_node, edge_type);
    }
}


#if EDGE_STATS
IPA_cgraph_edge_t *topdup[50];
#endif

static void
classify_edge(stats_t *stats,
	      IPA_cgraph_edge_t *edge, 
	      IPA_cgraph_edgelist_e edge_type)
{
  IPA_cgraph_node_t *real_src_node = NULL;
  IPA_cgraph_node_t *real_dst_node = NULL;
  IPA_cgraph_node_t *s_node;
  IPA_cgraph_node_t *d_node;

#if EDGE_STATS
  if (topdup[0] == NULL || edge->dup > topdup[0]->dup)
    {
      int i;
      for (i=1; i<50; i++)
	{
	  topdup[i-1] = topdup[i];
	  if (topdup[i] != NULL && edge->dup < topdup[i]->dup)
	    break;
	}
      topdup[i-1] = edge;
    }
#endif

  s_node = edge->src_elist->node;
  d_node = edge->dst_elist->node;

  for (real_src_node = s_node; real_src_node; 
       real_src_node = real_src_node->rep_child)
    {
      if (!real_src_node->data.syminfo->fninfo->has_been_called)
	continue;

      for (real_dst_node = d_node; real_dst_node; 
	   real_dst_node = real_dst_node->rep_child)
	{
	  if (!real_dst_node->data.syminfo->fninfo->has_been_called)
	    continue;
	  
	  classify_single(stats, real_src_node, real_dst_node, edge_type);
	  if (edge->data.target_stride || edge->data.source_stride)
	    {
	      stats->intra.str[edge_type]++;
	    }

	  if (no_expand || edge_type != ASSIGN_ADDR)
	    return;
	}
    }
}

static void
classify_node(stats_t *stats,
	      IPA_cgraph_node_t *node)
{
  IPA_HTAB_ITER eiter;  
  IPA_cgraph_edge_list_t *elist;
  IPA_cgraph_edge_t *edge;
  int found = 0;

  if (is_artificial(node))
    {
      stats->nodes.art++;
      stats->fn_nodes.art++;
    }
  else if (is_summary(node))
    {
      stats->nodes.sum++;
      stats->fn_nodes.sum++;
    }
  else if (is_heap(node))
    {
      stats->nodes.hp++;
      stats->fn_nodes.hp++;
    }
  else if (is_global(node))
    {
      stats->nodes.gbl++;
      stats->fn_nodes.gbl++;
    }
  else
    {
      stats->nodes.lcl++;
      stats->fn_nodes.lcl++;
    }
  stats->nodes.all++;
  stats->fn_nodes.all++;

  if (node->data.in_k_cycle != 0)
    {
      stats->nodes.kcycle++;
      stats->fn_nodes.kcycle++;
    }
  if (node->data.mod != node->data.var_size)
    {
      stats->nodes.mod++;
      stats->fn_nodes.mod++;
    }

  /* Only look at parent nodes */
  if (node->rep_parent != node)
    return;

#if 0
  IPA_cg_node_print (stdout, node, IPA_PRINT_ASCI);
#endif

  for (elist=node->first_list; elist; elist=elist->nxt_list)
    {
      IPA_HTAB_START(eiter, elist->out);
      IPA_HTAB_LOOP(eiter)
	{
	  edge = IPA_HTAB_CUR(eiter);
	  classify_edge(stats, edge, elist->edge_type);

	  if (elist->edge_type == ASSIGN_ADDR &&
	      !IPA_FLAG_ISSET(edge->flags, IPA_CG_EDGE_FLAGS_GBL))
	    found = 1;
	}
    }
  if (!found)
    {
      stats->nodes.nosum++;
    }
  else
    {
      stats->nodes.yessum++;
    }

#if 1
  {
    int ia, iaa, oa, oaa;
    ia = iaa = oa = oaa = 0;
    for (elist=node->first_list; elist; elist=elist->nxt_list)
      {
	switch (elist->edge_type)
	  {
	  case ASSIGN_ADDR:
	    iaa = IPA_htab_size(elist->in);
	    oaa = IPA_htab_size(elist->out);
	    break;
	  case ASSIGN:
	    ia = IPA_htab_size(elist->in);
	    oa = IPA_htab_size(elist->out);
	    break;
	  default:
	    break;
	  }
      }
    if ((ia + iaa > 1000) ||
	(oa + oaa) > 1000)
      {
	printf("%d.%d %s %s : IN a %d aa %d | OUT a %d aa %d",
	       node->data.var_id, 
	       node->data.version,
	       node->cgraph->data.fninfo->func_name,
	       node->data.syminfo->symbol_name,
	       ia, iaa, 
	       oa, oaa);
	if (IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_NOFIELD))
	  printf("  FI ");
	printf("\n");
#if EDGE_STATS
	for (elist=node->first_list; elist; elist=elist->nxt_list)
	  {
	    printf("[IN %s] : ",edge_types[elist->edge_type]);
	    IPA_HTAB_START(eiter, elist->in);
	    IPA_HTAB_LOOP(eiter)
	      {
		IPA_cgraph_edge_t *edge = IPA_HTAB_CUR(eiter);
		printf("%d ",edge->accessed);
	      }
	    printf("\n");
	    printf("[OUT %s] : ",edge_types[elist->edge_type]);
	    IPA_HTAB_START(eiter, elist->out);
	    IPA_HTAB_LOOP(eiter)
	      {
		IPA_cgraph_edge_t *edge = IPA_HTAB_CUR(eiter);
		printf("%d ",edge->accessed);
	      }
	    printf("\n");
	  }
#endif
      }
  }
#endif
}

static void
classify_consg(stats_t *stats,
	       IPA_prog_info_t *info,
	       IPA_cgraph_t *consg)
{
  IPA_HTAB_ITER niter;
  IPA_cgraph_node_t *node;

  stats->fn_nodes.art = 0;
  stats->fn_nodes.sum = 0;
  stats->fn_nodes.hp = 0;
  stats->fn_nodes.gbl = 0;
  stats->fn_nodes.lcl = 0;
  stats->fn_nodes.all = 0;

  if (!consg)
    return;

#if 0
  printf("%s\n",fninfo->func_name);
#endif

  IPA_HTAB_START(niter, consg->nodes);
  IPA_HTAB_LOOP(niter)
    {
      node = IPA_HTAB_CUR(niter);
      classify_node(stats, node);
    }
}

static void
print_stats_core(stats_core_t *stats, int i)
{
  if (i != -1)
    {
      printf("%6d %6d %6d %6d %6d %6d %6d",
	     stats->art[i],
	     stats->sum[i],
	     stats->hh[i],
	     stats->nh[i],
	     stats->gbl[i],
	     stats->lcl[i],
	     stats->str[i]);
    }
  else
    {
      printf("%6s %6s %6s %6s %6s %6s %6s",
	     "ART", "SUM", "HH", "NH", "GBL", "LCL", "STR");
    }
}

static void
print_stats_all(stats_t *stats)
{
  int i;

  printf("      NODES %6s <- %6s %6s %6s %6s %6s %6s %6s %6s %6s\n",
	 "ALL", "ART", "SUM", "GBL", "HP", "LCL", "KCY", "MOD","NOSUM","YSUM");
  printf("      NODES %6d <- %6d %6d %6d %6d %6d %6d %6d %6d %6d\n\n",
	 stats->nodes.all, stats->nodes.art, stats->nodes.sum,
	 stats->nodes.gbl, stats->nodes.hp, stats->nodes.lcl,
	 stats->nodes.kcycle, stats->nodes.mod, 
	 stats->nodes.nosum, stats->nodes.yessum);
  for (i=-1; i<MAX_EDGELIST_TYPE; i++)
    {
      switch(i)
	{
	case MIN_EDGELIST_TYPE:
	case MAX_EDGELIST_TYPE:
	  continue;
	  break;
	case ASSIGN:
	case ASSIGN + MAX_EDGELIST_TYPE:
	  printf("      A   ");
	  break;
	case ASSIGN_DEREF:
	case ASSIGN_DEREF + MAX_EDGELIST_TYPE:
	  printf("      AD  ");
	  break;
	case DEREF_ASSIGN:
	case DEREF_ASSIGN + MAX_EDGELIST_TYPE:
	  printf("      DA  ");
	  break;
	case ASSIGN_ADDR:
	case ASSIGN_ADDR + MAX_EDGELIST_TYPE:
	  printf("      AA  ");
	  break;
	default:
	  printf("          ");
	}

      if (i != -1)
	printf("%8d <- (%8d %8d %8d) ",
	       stats->inter.all[i] + stats->intra.all[i],
	       stats->inter.art[i] + stats->intra.art[i] + stats->inter.sum[i] + stats->intra.sum[i],
	       stats->inter.hh[i] + stats->intra.hh[i] + stats->inter.nh[i] + stats->intra.nh[i],
	       stats->inter.lcl[i] + stats->intra.lcl[i] + stats->inter.gbl[i] + stats->intra.gbl[i]
	       );
      else
	printf("%8s <- (%8s %8s %8s) ",
	       "ALL", "IGN", "HP", "NRM");
      print_stats_core(&stats->inter, i);
      printf(" | ");
      print_stats_core(&stats->intra, i);
      printf("\n");
    }

  printf("      ALL %8d <- (%8d %8d %8d) ",
	 stats->inter.all[0] + stats->intra.all[0],
	 stats->inter.art[0] + stats->intra.art[0] + stats->inter.sum[0] + stats->intra.sum[0],
	 stats->inter.hh[0] + stats->intra.hh[0] + stats->inter.nh[0] + stats->intra.nh[0],
	 stats->inter.lcl[0] + stats->intra.lcl[0] + stats->inter.gbl[0] + stats->intra.gbl[0]
	 );
  print_stats_core(&stats->inter, 0);
  printf(" | ");
  print_stats_core(&stats->intra, 0);
  printf("\n");

  printf("\n");
}


static void
classify_all(IPA_prog_info_t *info)
{
  IPA_funcsymbol_info_t * fninfo;
  stats_t stats;
  stats_t sum_stats;

  memset(&stats, 0, sizeof(stats_t));
  memset(&sum_stats, 0, sizeof(stats_t));
  
  List_start(info->fninfos);
  while ((fninfo = List_next(info->fninfos)))
    {
      if (!fninfo->has_been_called)
	continue;

      classify_consg(&stats, info, fninfo->consg);
#if 0
      printf("    [%30.30s]      NODES %6d <- %6d %6d %6d %6d %6d\n",
	     fninfo->func_name,
	     stats.fn_nodes.all, stats.fn_nodes.art, stats.fn_nodes.sum,
	     stats.fn_nodes.gbl, stats.fn_nodes.hp, stats.fn_nodes.lcl);
#endif
    }

  List_start(info->fninfos);
  while ((fninfo = List_next(info->fninfos)))
    {
      if (!fninfo->has_been_called)
	continue;
 
      classify_consg(&sum_stats, info, fninfo->lsum_consg);
#if 0
      printf("SUM [%30.30s]      NODES %6d <- %6d %6d %6d %6d %6d\n",
	     fninfo->func_name,
	     sum_stats.fn_nodes.all, sum_stats.fn_nodes.art, sum_stats.fn_nodes.sum,
	     sum_stats.fn_nodes.gbl, sum_stats.fn_nodes.hp, sum_stats.fn_nodes.lcl);
      
#endif
    }

#if 0
  printf("SUMMARY STATS\n");
  print_stats_all(&sum_stats); 
  printf("PROGRAM STATS\n");
  print_stats_all(&stats); 
#endif
}

static void
debug_print_edge(IPA_cgraph_edge_t *e)
{
  IPA_cg_node_print(stdout,e->dst_elist->node, IPA_PRINT_ASCI);
  printf(" <- %14s %d,%d,%d -",
	 edge_types[e->dst_elist->edge_type],
	 e->data.target_offset, 
	 e->data.assign_size,
	 e->data.source_offset);
  IPA_cg_node_print(stdout,e->src_elist->node, IPA_PRINT_ASCI);
  printf("\n");
}

void
IPA_prog_classify_all(IPA_prog_info_t *info, List tsort)
{
#if EDGE_STATS
  int i;

  for (i=1; i<50; i++)
    {
      topdup[i] = NULL;
    }
#endif

#if 1
  no_expand = 1;
  classify_all(info);
#endif
#if 0
  no_expand = 0;
  classify_all(info);
#endif

#if EDGE_STATS
  printf("LIST TOP FIFTY\n");
  for (i=1; i<50; i++)
    {
      printf("[%d] %9d ",i,topdup[i]->dup);
      if (topdup[i])
	debug_print_edge(topdup[i]);
    }
#endif
}

void
IPA_prog_classify_consg(IPA_prog_info_t *info,
			IPA_cgraph_t *consg)
{
  stats_t stats;

  memset(&stats, 0, sizeof(stats_t));

  classify_consg(&stats, info, consg);
  
  print_stats_all(&stats); 
}
