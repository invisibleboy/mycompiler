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




#include "pipa_misc_utils.h"
void
IPA_remove_single_assignment (IPA_cgraph_t *consg, int delete);



int 
PIPA_off_class_same(IPA_cgraph_node_t *node, int offset, off_class_t *oc, int use_consg)
{
  if (offset != oc->offset)
    return 0;
  if (oc->is_func != IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_FUNC))
    return 0;
  if (oc->is_str != IPA_FLAG_ISSET(node->data.syminfo->kind, IPA_VAR_KIND_STRING))
    return 0;
  if (use_consg && (oc->consg != node->cgraph))
    return 0;
  return 1;
}
void
PIPA_off_class_addnode(off_class_t *oc, IPA_cgraph_node_t *node)
{
  /* Avoid duplicates */
  if (!List_member(oc->list, node))
    oc->list = List_insert_last(oc->list, node);
}

off_class_t *
PIPA_off_class_new(IPA_cgraph_node_t *node, int offset)
{
  off_class_t *oc = calloc(1,sizeof(off_class_t));
  oc->list = List_insert_last(NULL, node);
  oc->offset = offset;
  oc->is_func = IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_FUNC);
  oc->is_str = IPA_FLAG_ISSET(node->data.syminfo->kind, IPA_VAR_KIND_STRING);
  oc->consg = node->cgraph;
  return oc;
}

List
PIPA_off_add_to_class(List oc_list, int offset, IPA_cgraph_node_t *node, 
		      int use_consg)
{
  off_class_t *oc;

  List_start(oc_list);
  while ((oc = List_next(oc_list)))
    {
      if (PIPA_off_class_same(node, offset, oc, use_consg))
	{
	  PIPA_off_class_addnode(oc, node);
	  return oc_list;
	}
    }
  oc = PIPA_off_class_new(node, offset);
  oc_list = List_insert_last(oc_list, oc);
  return oc_list;
}
		  

void
PIPA_off_class_free(off_class_t *oc)
{
  List_reset(oc->list);
  free(oc);
}



void
PIPA_find_merge_equiv(IPA_cgraph_t * consg)
{
  IPA_cgraph_edge_list_t *elist;
  IPA_cgraph_edge_list_t *src_elist;
  IPA_cgraph_node_t *node;
  IPA_cgraph_node_t *src_node;
  IPA_cgraph_edge_t *edge;
  IPA_HTAB_ITER niter;
  IPA_HTAB_ITER eiter;

  if (!consg)
    return;

  IPA_HTAB_START(niter, consg->nodes);
  IPA_HTAB_LOOP(niter)
    {
      List oc_list = NULL;
      off_class_t *oc;
      int found;
      node = IPA_HTAB_CUR(niter);

      /* Find nodes with multiple incomming addr edges */
      found = 0;
      for (elist = node->first_list; elist; elist = elist->nxt_list)
	{
	  if (elist->edge_type == ASSIGN_ADDR &&
	      IPA_htab_size(elist->in) > 1)
	    {
#if 0
	      IPA_cg_node_print(stdout, node, IPA_PRINT_ASCI);
	      printf("%d\n", IPA_htab_size(elist->in));
#endif
	      found = 1;
	      break;
	    }
	}
      if (found)
	{
	  /* Process each incoming address */
	  IPA_HTAB_START(eiter, elist->in);
	  IPA_HTAB_LOOP(eiter)
	    {
	      edge = IPA_HTAB_CUR(eiter);
	      src_node = edge->src_elist->node;
	      src_elist = IPA_cg_edge_list_find (src_node, ASSIGN_ADDR);
	      if (!src_elist)
		continue;

#if 0
	      printf("   - ");
	      IPA_cg_node_print(stdout, src_node, IPA_PRINT_ASCI);
	      printf("%d  ", IPA_htab_size(src_elist->out));
#endif

	      /* Skip certain kinds of nodes */
	      if (IPA_FLAG_ISSET(src_node->flags, IPA_CG_NODE_FLAGS_HEAP) ||
		  src_node->data.var_size > 4)
		{
#if 0
		  printf(" - skipping\n");
#endif
		  continue;
		}

	      /* Only one address target? */
	      if (IPA_htab_size(src_elist->out) == 1)
		{
		  /* Put each node whose addr is taken and put them
		     into classes */
		  oc_list = PIPA_off_add_to_class(oc_list, 
						  edge->data.target_offset,
						  src_node, 1);
#if 0
		  printf(" - ok\n");
#endif
		}
	      else
		{
#if 0
		  printf(" - not single\n");
#endif
		}
	    }
	}

      /* Merge nodes */
      List_start(oc_list);
      while ((oc = List_next(oc_list)))
	{
	  IPA_cgraph_node_t *iter_node;
	  IPA_cgraph_node_t *base_node;

	  if (List_size(oc->list) <= 1)
	    continue;
	  printf("Equiv Unifying [%s %d]: %d nodes \n",
		 consg->data.fninfo->func_name,
		 oc->offset,
		 List_size(oc->list));

	  base_node = List_first(oc->list);
	  base_node = IPA_cg_node_get_rep(base_node);

#if 1
	      printf("   - ");
	      IPA_cg_node_print(stdout, base_node, IPA_PRINT_ASCI);
	      printf("\n");
#endif
	  while ((iter_node = List_next(oc->list)))
	    {
	      iter_node = IPA_cg_node_get_rep(iter_node);
	      if (base_node == iter_node)
		continue;

	      IPA_cg_merge_nodes(base_node, iter_node, 0, NULL, 0);
#if 1
	      printf("   - ");
	      IPA_cg_node_print(stdout, iter_node, IPA_PRINT_ASCI);
	      printf("\n");
#endif
	    }
	}

      /* Free list */
      List_start(oc_list);
      while ((oc = List_next(oc_list)))
	{
	  PIPA_off_class_free(oc);
	}
      List_reset(oc_list);
    }
}


/*****************************************************************************\
 * Static function declarations                                              *
\*****************************************************************************/
#define DEBUG_PARTITION 0

typedef struct partition_t 
{
  char flags;
  int in_work_list;
  IPA_Hashtab_t *node_set;
  /*         class 1       2                 n
   *  List (List edges)(List edges) ... (List edges)
   */
  List           in_edge_classes;
  List           out_edge_classes;
} partition_t;

unsigned int cmp_item;

#define SPLIT_USEFUL 0x01

#define BUILD_OUT    0x01
#define BUILD_IN     0x02

#define NORMAL       0
#define FOR_SUMMARY  1
static  int          edge_rules = NORMAL;

#define NODE_VALID IPA_CG_NODE_FLAGS_GENERIC1

static int
is_addr_taken (IPA_cgraph_node_t *node)
{
  IPA_cgraph_edge_list_t *edge_list;
  
  edge_list = IPA_cg_edge_list_find (node, ASSIGN_ADDR);
  
  if (edge_list != NULL && 0 < IPA_htab_size(edge_list->out))
    {
      return 1;
    }
  else
    {
      return 0;
    }
}

static int
compare_addr (void *item)
{
  if ((unsigned int)(long)item == cmp_item)
    return 1;

  return 0;
}

static void*
IPA_htab_find_unique (IPA_Hashtab_t *htab, unsigned int key)
{
  cmp_item = (unsigned int)(long)key;
  return IPA_htab_find (htab, key, compare_addr);
}

static void
edge_classes_free(List edge_classes)
{
  List list;

  List_start(edge_classes);
  while ((list = List_next(edge_classes)))
    {
      List_reset(list);
    }
  List_reset(edge_classes);
}

static int
edge_class_match(List eclass, IPA_cgraph_edge_t *edge)
{
  IPA_cgraph_edge_t *rep;

  rep = List_first(eclass);
  assert(rep);

  if (rep->src_elist->edge_type != edge->src_elist->edge_type ||
      rep->data.source_offset != edge->data.source_offset ||
      rep->data.target_offset != edge->data.target_offset ||
      (rep->src_elist->edge_type == SKEW &&
       rep->data.assign_size != edge->data.assign_size))
    return 0;

  return 1;
}


static void
edge_classes_build_in(partition_t *part)
{
  IPA_HTAB_ITER hash_enum;
  IPA_cgraph_node_t *node;
  IPA_cgraph_edge_t *edge;
  IPA_cgraph_edge_list_t *elist;
  IPA_HTAB_ITER eiter;
  List eclass;
     
  if (part->in_edge_classes)
    {
      edge_classes_free(part->in_edge_classes);
    }
  part->in_edge_classes = NULL;

  IPA_HTAB_START(hash_enum, part->node_set);
  IPA_HTAB_LOOP(hash_enum)
    {
      node = IPA_HTAB_CUR (hash_enum);

      for (elist = node->first_list; elist; elist = elist->nxt_list)
	{
#if 0
          if (!(elist->edge_type == DEREF_ASSIGN || elist->edge_type == ASSIGN_ADDR))
            {
              continue;
            }
#endif

	  IPA_HTAB_START(eiter, elist->in);
	  IPA_HTAB_LOOP(eiter)
	    {
	      edge = IPA_HTAB_CUR(eiter);
	      /* Ignore UP/DOWN for summarization */
	      if ((edge_rules == FOR_SUMMARY) && 
		  IPA_FLAG_ISSET(edge->flags, IPA_CG_EDGE_FLAGS_UD))
		continue;

	      List_start(part->in_edge_classes);
	      while ((eclass = List_next(part->in_edge_classes)))
		{
		  if (!edge_class_match(eclass, edge))
		    continue;
		  assert(eclass == List_insert_last(eclass, edge));
		  goto eclass_done;
		}
	      eclass = List_insert_last(eclass, edge);
	      part->in_edge_classes = List_insert_first(part->in_edge_classes,
							eclass);
	    eclass_done:
	      ;
	    }
	}
    }
}


static void
edge_classes_build_out(partition_t *part)
{
  IPA_HTAB_ITER hash_enum;
  IPA_cgraph_node_t *node;
  IPA_cgraph_edge_t *edge;
  IPA_cgraph_edge_list_t *elist;
  IPA_HTAB_ITER eiter;
  List eclass;
     
  if (part->out_edge_classes)
    {
      edge_classes_free(part->out_edge_classes);
    }
  part->out_edge_classes = NULL;

  IPA_HTAB_START(hash_enum, part->node_set);
  IPA_HTAB_LOOP(hash_enum)
    {
      node = IPA_HTAB_CUR (hash_enum);

      for (elist = node->first_list; elist; elist = elist->nxt_list)
	{
	  if (elist->edge_type == DEREF_ASSIGN)
	    {
	      /* If edge_type is DEREF_ASSIGN, the actual target variable is
		 not updated. Therefore, it will not incur a split. */
	      continue;
	    }

	  IPA_HTAB_START(eiter, elist->out);
	  IPA_HTAB_LOOP(eiter)
	    {
	      edge = IPA_HTAB_CUR(eiter);
	      /* Ignore UP/DOWN for summarization */
	      if ((edge_rules == FOR_SUMMARY) &&
		  IPA_FLAG_ISSET(edge->flags, IPA_CG_EDGE_FLAGS_UD))
		continue;

	      List_start(part->out_edge_classes);
	      while ((eclass = List_next(part->out_edge_classes)))
		{
		  if (!edge_class_match(eclass, edge))
		    continue;
		  assert(eclass == List_insert_last(eclass, edge));
		  goto eclass_done;
		}
#if 0
	      printf("[%5d] NEW ECLASS %s %d %d %d %p\n",
		     List_size(part->out_edge_classes)+1,
		     edge_types[edge->src_elist->edge_type],
		     edge->data.source_offset,
		     edge->data.assign_size,
		     edge->data.target_offset,
		     edge);
#endif
	      eclass = List_insert_last(eclass, edge);
	      part->out_edge_classes = List_insert_first(part->out_edge_classes,
							 eclass);
	    eclass_done:
	      ;
	    }
	}
    }
}


static partition_t *
partition_new()
{
  partition_t *part = (partition_t*)calloc(1,sizeof(partition_t));
  assert(part);

  part->node_set = IPA_htab_new (4);

  return part;;
}

static void
partition_free(partition_t *part)
{
  if (!part)
    return;

  IPA_htab_free (part->node_set);
  edge_classes_free(part->in_edge_classes);
  edge_classes_free(part->out_edge_classes);

  free(part);
}


static void
partition_add_node(partition_t *part, IPA_cgraph_node_t *node)
{
  IPA_htab_insert (part->node_set, node, (int)(long)node);
  node->misc.ptr = part;
}


static IPA_Hashtab_t*
build_eclass_in_set (partition_t *part, List eclass)
{
  IPA_Hashtab_t     *in_set;
  IPA_cgraph_node_t *in_node;
  partition_t       *in_part;
  IPA_cgraph_edge_t *edge;

  in_set = IPA_htab_new (3);

  List_start(eclass);
  while ((edge = List_next(eclass)))
    {
      in_node = edge->src_elist->node;
      /*printf("   inset [%d] ",in_node->data.var_id);*/
      if (!IPA_FLAG_ISSET(in_node->flags, NODE_VALID))
	{
	  /*printf("invalid\n");*/
	  continue;
	}
      assert(in_node->misc.ptr);
      in_part = ((partition_t*)in_node->misc.ptr);

      /* Ignore nodes that belong to single node partitions */
      if (IPA_htab_size(in_part->node_set) <= 1)
	{
	  /*printf("<=1\n");*/
	  continue;
	}

      /* If edgetype is ASSIGN, we ignore incoming edges from
	 the same partition. */
      if (edge->src_elist->edge_type == ASSIGN && (in_part == part))
	{
	  /*printf("assign\n");*/
	  continue;
	}

      /*printf(" added\n");*/
      IPA_htab_insert (in_set, in_node, (int)(long)in_node);
    }
  
  return in_set;
}


static IPA_Hashtab_t*
build_eclass_out_set (partition_t *part, List eclass)
{
  IPA_Hashtab_t     *out_set;
  IPA_cgraph_node_t *out_node;
  partition_t       *out_part;
  IPA_cgraph_edge_t *edge;

  out_set = IPA_htab_new (3);

  List_start(eclass);
  while ((edge = List_next(eclass)))
    {
      out_node = edge->dst_elist->node;
      out_part = ((partition_t*)out_node->misc.ptr);

      /* Ignore nodes that belong to single node partitions */
      if (IPA_htab_size(out_part->node_set) <= 1)
	continue;

      /* If edgetype is ASSIGN, we ignore outgoing edges into 
	 the same partition. */
      if (edge->src_elist->edge_type == ASSIGN && (out_part == part))
	continue;
      
      IPA_htab_insert (out_set, out_node, (int)(long)out_node);
    }
  
  return out_set;
}


static partition_t *
split_partition(partition_t *part, IPA_Hashtab_t *out_set, int mode)
{
  partition_t *partA, *partB;
  IPA_HTAB_ITER part_enum;
  IPA_cgraph_node_t *node;

  partA = partition_new();
  partB = partition_new();

  part_enum.htab = part->node_set;
  IPA_HTAB_START(part_enum, part->node_set);
  IPA_HTAB_LOOP(part_enum)
    {
      node = IPA_HTAB_CUR (part_enum);

      if (IPA_htab_find_unique (out_set, (unsigned int)(long)node))
	{
	  partition_add_node(partA, node);
	}
      else
	{
	  partition_add_node(partB, node);	  
	}
      /* Override to keep pointing to part */
      node->misc.ptr = part;
    }

  /* At least one node should be in partA
   */
  assert(IPA_htab_size(partA->node_set) > 0);
  if (IPA_htab_size(partB->node_set) == 0)
    {
      /* no change to partition due to out_set
       */
      partition_free(partA);
      partition_free(partB);
      return NULL;
    }

  /* Make part become partA and return partB */
  IPA_htab_free (part->node_set);
  part->node_set = partA->node_set;
  partA->node_set = NULL;
  partition_free(partA);

  assert(mode);
  if (IPA_FLAG_ISSET(mode,BUILD_OUT))
    {
      edge_classes_build_out(part);
      edge_classes_build_out(partB);
    }
  if (IPA_FLAG_ISSET(mode,BUILD_IN))
    {
      edge_classes_build_in(part);
      edge_classes_build_in(partB);
    }

  /* Make all nodes in partB point to partB */
  IPA_HTAB_START(part_enum, partB->node_set);
  IPA_HTAB_LOOP (part_enum)
    {
      node = IPA_HTAB_CUR (part_enum);
      node->misc.ptr = partB;
    }

  return partB;
}



static List
flag_useful_partitions(IPA_Hashtab_t *out_set)
{
  List chk_partitions = NULL;
  IPA_cgraph_node_t *node;
  IPA_HTAB_ITER      out_enum;
  partition_t       *part;

  IPA_HTAB_START(out_enum, out_set);
  IPA_HTAB_LOOP(out_enum)
    {
      node = IPA_HTAB_CUR (out_enum);
      part = ((partition_t*)node->misc.ptr);
      if (IPA_htab_size(part->node_set) <= 1)
	continue;
      if (IPA_FLAG_ISSET(part->flags, SPLIT_USEFUL))
	continue;
      IPA_FLAG_SET(part->flags, SPLIT_USEFUL);

      chk_partitions = List_insert_last(chk_partitions, part);
    }

  return chk_partitions;
}

#if DEBUG_PARTITION
static void
print_eclass(List eclass)
{
  IPA_cgraph_edge_t *rep;
  
  rep = List_first(eclass);
  assert(rep);
  
  printf("  %s %d %d %d ",
	 edge_types[rep->src_elist->edge_type],
	 rep->data.source_offset,
	 rep->data.assign_size,
	 rep->data.target_offset);
}

static void
print_partition (partition_t *part)
{
  IPA_HTAB_ITER hash_enum;
  int first;

  printf ("[");

  IPA_HTAB_START(hash_enum, part->node_set);
  first = 1;

  IPA_HTAB_LOOP (hash_enum)
    {
      IPA_cgraph_node_t *node;

      if (!first)
	{
	  printf (",");
	}
      else
	{
	  first = 0;
	}

      node = IPA_HTAB_CUR (hash_enum);
      printf ("%d", node->data.var_id);
    }

  printf ("]");
}

static void
print_all_partitions (List partitions)
{
  partition_t *part;

  List_start(partitions);
  while((part = List_next(partitions)))
    {
      print_partition (part);
      printf ("\n");
    }
}

#endif

static List
make_partitions(List all_partitions, int mode, int __edge_rules)
{
  partition_t *cur_part = NULL;
  List work_list = NULL;
  int cnt_worklist, cnt_eclass;
  int cnt_splitchk, cnt_split;
  double curtime;
  double basetime;
  
#if DEBUG_PARTITION
  printf ("FDVS: initial paritions\n");
  print_all_partitions (all_partitions);
  printf ("\n");
#endif

  /* Setup partitions and the partition worklist 
   */
  edge_rules = __edge_rules;
  cnt_worklist = 0;
  cnt_eclass = 0;
  cnt_splitchk = 0;
  cnt_split = 0;
  basetime = IPA_GetTime();
  work_list = NULL;
  List_start(all_partitions);
  while ((cur_part = List_next(all_partitions)))
    {
      assert(mode);
      if (IPA_FLAG_ISSET(mode, BUILD_OUT))
	{
	  edge_classes_build_out(cur_part);
	}
      if (IPA_FLAG_ISSET(mode, BUILD_IN))
	{
	  edge_classes_build_in(cur_part);
	}
      
      work_list = List_insert_last(work_list, cur_part);	  
      cur_part->in_work_list = 1;
      IPA_FLAG_CLR(cur_part->flags, SPLIT_USEFUL);
    }
     
 
  /* Now process the partitions list until done 
   */
  List_start (work_list);
  while ((cur_part = List_next (work_list)))
    {
      List eclass;
      
      work_list = List_delete_current (work_list);
      cur_part->in_work_list = 0;
      cnt_worklist ++;
      
#if DEBUG_PARTITION
      printf ("Current parition\n");
      print_partition (cur_part);
      printf ("\n");
#endif
      
      List_start(cur_part->in_edge_classes);
      while ((eclass = List_next(cur_part->in_edge_classes)))
	{
	  IPA_Hashtab_t *set;
	  List chk_partitions;
	  partition_t *splitcandidate_part, *new_part;
	  
#if DEBUG_PARTITION
	  printf("  in-eclass: ");
	  print_eclass(eclass);
	  printf("\n");
#endif
	  cnt_eclass ++;
	  set = build_eclass_in_set (cur_part, eclass);

	  /* chk_partitions is a list of all partitions that
	   *   contain at least one element of the (in/out)set
	   *   and are larger than one node. Basically, cur_part's
	   *   (in/out)set is being use to split other partitions.
	   *   There is no point trying to split a completely 
	   *   disjoint partition nor a partition of one node.
	   */
	  chk_partitions = flag_useful_partitions(set);
	  
	  List_start(chk_partitions);
	  while ((splitcandidate_part = List_next(chk_partitions)))
	    {      
	      IPA_FLAG_CLR(splitcandidate_part->flags, SPLIT_USEFUL);
	      cnt_splitchk ++;
	      
	      /* Split Partition Cases:
	       * 1) splitcandidate subtract out_set = empty
	       *          - new_part will be empty
	       * 2) splitcandidate subtract out_set = splitcandidate
	       *          - new_part will be empty
	       * 3) else
	       *          - new_part       = splitcandidate subtract out_set
	       *          - splitcandidate = remainer of above
	       */
	      if ((new_part = split_partition(splitcandidate_part, set, mode)))
		{
#if DEBUG_PARTITION
		  printf("   # split\n");
		  print_partition (new_part);
		  print_partition (splitcandidate_part);
		  printf ("\n");		      
#endif
		  all_partitions = List_insert_last(all_partitions, new_part);
		  cnt_split ++;
		  
		  /* Add both partitions back into
		     the work list */
		  work_list = List_insert_last (work_list, new_part);
		  new_part->in_work_list = 1;
		  work_list = List_insert_last (work_list, splitcandidate_part);
		  splitcandidate_part->in_work_list = 1;		  
		} /*split*/
	    } /*all*/

#if DEBUG_PARTITION
	  printf("   next in-eclass\n");
#endif
	  
	  IPA_htab_free(set);
	  List_reset (chk_partitions);
	} /* in-eclass */

#if DEBUG_PARTITION
      printf("done in-eclass\n");
#endif

      List_start(cur_part->out_edge_classes);
      while ((eclass = List_next(cur_part->out_edge_classes)))
	{
	  IPA_Hashtab_t *set;
	  List chk_partitions;
	  partition_t *splitcandidate_part, *new_part;
	  
#if DEBUG_PARTITION
	  printf("  out-eclass: ");
	  print_eclass(eclass);
	  printf("\n");
#endif
	  cnt_eclass ++;
	  set = build_eclass_out_set (cur_part, eclass);
	  
	  /* chk_partitions is a list of all partitions that
	   *   contain at least one element of the (in/out)set
	   *   and are larger than one node. Basically, cur_part's
	   *   (in/out)set is being use to split other partitions.
	   *   There is no point trying to split a completely 
	   *   disjoint partition nor a partition of one node.
	   */
	  chk_partitions = flag_useful_partitions(set);
	  
	  List_start(chk_partitions);
	  while ((splitcandidate_part = List_next(chk_partitions)))
	    {      
	      IPA_FLAG_CLR(splitcandidate_part->flags, SPLIT_USEFUL);
	      cnt_splitchk ++;
	      
	      /* Split Partition Cases:
	       * 1) splitcandidate subtract out_set = empty
	       *          - new_part will be empty
	       * 2) splitcandidate subtract out_set = splitcandidate
	       *          - new_part will be empty
	       * 3) else
	       *          - new_part       = splitcandidate subtract out_set
	       *          - splitcandidate = remainer of above
	       */
	      if ((new_part = split_partition(splitcandidate_part, set, mode)))
		{
#if DEBUG_PARTITION
		  printf("   # split\n");
		  print_partition (new_part);
		  print_partition (splitcandidate_part);
		  printf ("\n");		      
#endif
		  all_partitions = List_insert_last(all_partitions, new_part);
		  cnt_split ++;
		  
		  /* Add both partitions back into
		     the work list */
		  work_list = List_insert_last (work_list, new_part);
		  new_part->in_work_list = 1;
		  work_list = List_insert_last (work_list, splitcandidate_part);
		  splitcandidate_part->in_work_list = 1;		  
		} /*split*/
	    } /*all*/

#if DEBUG_PARTITION
	  printf("   next out-eclass\n");
#endif	  
	  IPA_htab_free(set);
	  List_reset (chk_partitions);
	}/* out-eclass */

#if DEBUG_PARTITION
      printf("done out-eclass\n");
#endif

     } /*work_list*/
  
  List_reset (work_list);
  
#if DEBUG_PARTITION
  printf ("final paritions\n");
  print_all_partitions (all_partitions);
  printf ("\n");
#endif

  curtime = IPA_GetTime ();
  DEBUG_IPA(2, printf("OPTI: Parition [%1.1fs] worked %d eclass %d chk %d split %d = %d\n",
		      curtime - basetime,
		      cnt_worklist,
		      cnt_eclass,
		      cnt_splitchk,
		      cnt_split,
		      List_size(all_partitions)
		      ););
  fflush(stdout);

  return all_partitions;
}



void
IPA_find_merge_equiv2(IPA_cgraph_t * consg)
{
  IPA_cgraph_node_t *node;
  IPA_HTAB_ITER niter;
  List node_work_list;
  List all_partitions;
  List del_nodes;
  int mcnt = 0;

  if (!consg)
    return;

  /* Prime node work list 
   */
  node_work_list = NULL;
  IPA_HTAB_START(niter, consg->nodes);
  IPA_HTAB_LOOP(niter)
    {
      node = IPA_HTAB_CUR(niter);
      node_work_list = List_insert_last(node_work_list,
					node);
    }

  /* Go through nodes and look for merging opportunities
   */
  all_partitions = NULL;
  del_nodes = NULL;
  List_start(node_work_list);
  while ((node = List_next(node_work_list)))
    {
      IPA_cgraph_edge_list_t *elist;
      IPA_cgraph_edge_t *edge;
      IPA_HTAB_ITER eiter;
      partition_t *part = NULL;
      IPA_HTAB_ITER part_enum;
      List valid_list = NULL;
      int found;

      node_work_list = List_delete_current(node_work_list);

#if DEBUG_PARTITION
      printf("#### NODENODE %p ", node);
      IPA_cg_node_print(stdout, node, IPA_PRINT_ASCI);
      printf("\n");
#endif

      assert(node->data.var_id > 0 &&
	     node->rep_parent != NULL);
      if (node->rep_parent != node)
	continue;

      /* Look for potential opportunity
       */
      found = 0;
      for (elist = node->first_list; elist; elist = elist->nxt_list)
	{
	  if (IPA_htab_size(elist->in) > 1)
	    {
	      found = 1;
	    }
	}
      if (!found)
	continue;

      /* Form over estimated partition 
       */
      part = partition_new();
      for (elist = node->first_list; elist; elist = elist->nxt_list)
	{
	  IPA_HTAB_START(eiter, elist->in);
	  IPA_HTAB_LOOP(eiter)
	    {
	      edge = IPA_HTAB_CUR(eiter);

	      /* Don't allow these to be merged */
	      if (IPA_FLAG_ISSET (edge->src_elist->node->flags, 
				  (IPA_CG_NODE_FLAGS_PARAM |
				   IPA_CG_NODE_FLAGS_RETURN |
				   IPA_CG_NODE_FLAGS_CALLEE)))
		continue;
	      IPA_FLAG_SET(edge->src_elist->node->flags, NODE_VALID);
	      valid_list = List_insert_last(valid_list, edge->src_elist->node);

	      partition_add_node(part, edge->src_elist->node);
	    }
	}
      all_partitions = List_insert_last(all_partitions, part);


      /* Now we have a large partition 
       * Make paritions for individual nodes that are outputs
       *   of this partition
       */
      IPA_HTAB_START(part_enum, part->node_set);
      IPA_HTAB_LOOP(part_enum)
	{
	  IPA_cgraph_node_t *p_node;
	  p_node = IPA_HTAB_CUR (part_enum);
	  
	  for (elist = p_node->first_list; elist; elist = elist->nxt_list)
	    {
	      IPA_HTAB_START(eiter, elist->out);
	      IPA_HTAB_LOOP(eiter)
		{
		  partition_t *little_part = NULL;
		  edge = IPA_HTAB_CUR(eiter);

		  /* There may be repeats, we just want one 
		     partition per output node */
		  if (IPA_FLAG_ISSET(edge->dst_elist->node->flags, NODE_VALID))
		    continue;
		  IPA_FLAG_SET(edge->dst_elist->node->flags, NODE_VALID);
		  valid_list = List_insert_last(valid_list, edge->dst_elist->node);

		  little_part = partition_new();
		  partition_add_node(little_part, edge->dst_elist->node);
		  all_partitions = List_insert_last(all_partitions, 
						    little_part);
		}
	    }
	}
      part = NULL;



      /* Process and split partitions until equivalence classes
       *   are formed
       */
      all_partitions = make_partitions(all_partitions, BUILD_IN, NORMAL);



      /* Merge nodes inside equivalent partitions
       *
       */
      List_start(all_partitions);
      while ((part = List_next(all_partitions)))
	{
	  IPA_HTAB_ITER      node_enum;
	  List               oc_list = NULL;
	  off_class_t       *oc;
	  
	  if (IPA_htab_size(part->node_set) <= 1)
	    {
	      partition_free (part);
	      continue;
	    }

	  /* A final set of limitations. 
	   * Don't merge:
	   *   nodes in different constraint graphs
	   *   functions and non-functions
	   *   strings and non-strings
	   */
	  IPA_HTAB_START(node_enum, part->node_set);
	  IPA_HTAB_LOOP (node_enum)
	    {
	      IPA_cgraph_node_t *pnode;
	      pnode = IPA_HTAB_CUR (node_enum);

	      if (!IPA_FLAG_ISSET(pnode->flags, IPA_CG_NODE_FLAGS_FUNC) &&
		  /*!IPA_FLAG_ISSET(pnode->flags, IPA_CG_NODE_FLAGS_TEMP) &&*/
		  !IPA_FLAG_ISSET(pnode->data.syminfo->kind, IPA_VAR_KIND_STRING) &&
		  !IPA_FLAG_ISSET(pnode->flags, IPA_CG_NODE_FLAGS_GLOBAL)
		  )
		continue;
	      
	      oc_list = PIPA_off_add_to_class(oc_list, 0, pnode, 1);
	    }
	  
	  /* Now merge nodes within the final partitions
	   */
	  List_start(oc_list);
	  while ((oc = List_next(oc_list)))
	    {
	      IPA_cgraph_node_t *iter_node;
	      IPA_cgraph_node_t *base_node;

	      if (List_size(oc->list) <= 1)
		{
		  PIPA_off_class_free(oc);
		  continue;
		}

	      printf("# PARTITION [%d] \n",List_size(oc->list));
	      base_node = List_first(oc->list);
	      List_start(oc->list);
	      while ((iter_node = List_next(oc->list)))
		{
		  iter_node = IPA_cg_node_get_rep(iter_node);
#if 0
		  printf("#   - ");
		  IPA_cg_node_print(stdout, iter_node, IPA_PRINT_ASCI);
		  printf("\n");
#endif
		  if (base_node == iter_node)
		    continue;

		  mcnt++;
		  IPA_cg_merge_nodes(base_node, iter_node, 0, NULL, 0);
		  if (IPA_FLAG_ISSET(iter_node->flags, IPA_CG_NODE_FLAGS_TEMP) &&
		      !List_member(del_nodes, iter_node))
		    {
		      del_nodes = List_insert_last(del_nodes, iter_node);
		    }
		}
	      
	      node_work_list = List_insert_last(node_work_list, base_node);
	      PIPA_off_class_free(oc);
	    }

	  List_reset(oc_list);
	  partition_free (part);
	} /* all paritions */


      /* Cleanup
       */
      List_reset(all_partitions);
      all_partitions = NULL;
      List_start(valid_list);
      while ((node = List_next(valid_list)))
	{
	  IPA_FLAG_CLR(node->flags, NODE_VALID);
	  node->misc.ptr = NULL;
	}
      List_reset(valid_list);
      
    } /* nodes work list */


  if (mcnt > 0)
    printf("PARTITION MERGED %d\n",mcnt);
#if 0
  if (List_size(del_nodes) > 0)
    {
      printf("Deleteing %d nodes\n", 
	     List_size(del_nodes));
      List_start(del_nodes);
      while ((node = List_next(del_nodes)))
	{
	  IPA_cg_node_delete(node);
	}
    }
#endif
  List_reset(del_nodes);
}



void
IPA_find_merge_summary_equiv (IPA_cgraph_t *consg)
{
  partition_t *common_part, *cur_part;
  List all_partitions;
  IPA_HTAB_ITER niter;

  /* Make the initial partitions
   */
  all_partitions = NULL;
  common_part = partition_new();

  IPA_HTAB_START(niter, consg->nodes);
  IPA_HTAB_LOOP(niter)
    { 
      IPA_cgraph_node_t *node;

      node = IPA_HTAB_CUR(niter);
      node->misc.ptr = NULL;

      if (IPA_FLAG_ISSET (node->flags, (IPA_CG_NODE_FLAGS_PARAM |
					IPA_CG_NODE_FLAGS_RETURN |
					IPA_CG_NODE_FLAGS_CALLEE |
					IPA_CG_NODE_FLAGS_GLOBAL |
					IPA_CG_NODE_FLAGS_NOCNTXT)) ||
	  is_addr_taken (node))
	{
	  partition_t *new_part;

	  new_part = partition_new();
	  partition_add_node(new_part, node);

	  all_partitions = List_insert_last(all_partitions, new_part);
	}
      else
	{
	  partition_add_node(common_part, node);
	}
    }

  if (IPA_htab_size (common_part->node_set) > 0)
    {
      all_partitions = List_insert_last(all_partitions, common_part);
    }
  else
    {
      partition_free(common_part);
    }


  /* Process and split partitions until equivalence classes
   *   are formed
   */
  all_partitions = make_partitions(all_partitions, BUILD_OUT, FOR_SUMMARY);
  
  
#if DEBUG_PARTITION
  printf ("mergining redundant nodes\n");
#endif
  
  List_start(all_partitions);
  while ((cur_part = List_next(all_partitions)))
    {
      IPA_cgraph_node_t *first_node;
      IPA_HTAB_ITER      node_enum;
      
      first_node = NULL;
      IPA_HTAB_START(node_enum, cur_part->node_set);
      IPA_HTAB_LOOP (node_enum)
	{
	  IPA_cgraph_node_t *node;

	  node = IPA_HTAB_CUR (node_enum);

	  /* Don't merge nodes not in this graph */
	  if (node->cgraph != consg)
	    continue;
	  /* Don't merge globals or callee nodes */
	  if (IPA_FLAG_ISSET(node->flags, (IPA_CG_NODE_FLAGS_GLOBAL |
					   IPA_CG_NODE_FLAGS_NOCNTXT |
					   IPA_CG_NODE_FLAGS_CALLEE)))
	    continue;
	  
	  if (first_node == NULL)
	    {
	      first_node = node;
	    }
	  else
	    {
#if DEBUG_PARTITION
	      printf ("%d <= %d\n", first_node->data.var_id, 
		      node->data.var_id);
#endif
	      IPA_cg_merge_nodes (first_node, node, 1, NULL, 0);
	    }
	}

      partition_free (cur_part);
    }

  List_reset(all_partitions);

#if DEBUG_PARTITION
  printf ("\n");
#endif
}


/*************************************************************************
 *
 * There routines handle uncalled top-level procedures for analyzing
 *   program fragments that do not contain main()
 *
 *************************************************************************/

static IPA_cgraph_node_t *
get_formal(IPA_cgraph_t *formal_cg, int formal_id)
{
  IPA_cgraph_node_t *formal_node;

  formal_node = IPA_consg_find_node (formal_cg, formal_id, 1);
  if (formal_node)
    assert(formal_node->rep_parent == formal_node);

  return formal_node;
}

static IPA_cgraph_node_t *
new_tlpnode(IPA_prog_info_t * info, Key type_key)
{
  IPA_symbol_info_t *syminfo;
  IPA_cgraph_node_t *node;
  char name[50];
  Key sym_key;

  /* Make a new tmp var key */ 
  sym_key = IPA_symbol_tmpvarkey();

  sprintf(name,"__INPUT__%d", sym_key.sym);

  syminfo = IPA_symbol_add (info, info->globals,
			    name, sym_key,
			    (IPA_VAR_KIND_TEMP | IPA_VAR_KIND_GLOBAL), 
			    type_key);

  assert(!IPA_FLAG_ISSET(syminfo->kind, IPA_VAR_KIND_FUNC));

  /* Make a tmp node */
  node = IPA_consg_ensure_node(info->globals->consg, syminfo->id, 1,
			       IPA_Pcode_sizeof(info, syminfo->type_key),
			       syminfo, (IPA_CG_NODE_FLAGS_GLOBAL |
					 IPA_CG_NODE_FLAGS_TEMP |
					 IPA_CG_NODE_FLAGS_NOCNTXT |
					 IPA_CG_NODE_FLAGS_NOLOCAL)); 

  printf("New tlp node %d\n", node->data.var_id);

  return node;
}

static void
free_tl(List tl)
{
  tlparam_t *tlp;

  List_start(tl);
  while ((tlp = List_next(tl)))
    {
      List_reset(tlp->art_src_nodes);
      List_reset(tlp->dst_nodes);
      free(tlp);
    }
  List_reset(tl);
}

static tlparam_t *
find_tlparam(IPA_prog_info_t * info, 
	     List tl, 
	     Key type_key)
{
  tlparam_t *tlp;

  List_start(tl);
  while ((tlp = List_next(tl)))
    {
      if (P_MatchKey(tlp->type_key, type_key))
	return tlp;
    }

  return NULL;
}

static List
ensure_tlparam(IPA_prog_info_t * info, 
	       List tl, 
	       Key type_key)
{
  tlparam_t *tlp;

  if (!PST_IsPointerType(info->symboltable, type_key))
    return tl;

  if (PST_IsFunctionType(info->symboltable, type_key))
    return tl;

  if ((tlp = find_tlparam(info, tl, type_key)))
    return tl;

  tlp = calloc(1,sizeof(tlparam_t));
  tlp->type_key = type_key;

  {
    IPA_cgraph_node_t *node = NULL;
    IPA_cgraph_node_t *prev_node = NULL;
    Key type;
    
    prev_node = new_tlpnode(info, type_key);
    tlp->art_src_nodes = List_insert_last(tlp->art_src_nodes, prev_node);
    
    for (type=type_key; PST_IsPointerType(info->symboltable, type); )
      {
	type = PST_GetTypeType(info->symboltable, type);
	if (PST_IsFunctionType(info->symboltable, type))
	  break;

	node = new_tlpnode(info, type);
	IPA_consg_ensure_edge(ASSIGN_ADDR, node, prev_node, 
			      0, IPA_POINTER_SIZE, 0,
			      (IPA_CG_EDGE_FLAGS_EXPLICIT |
			       IPA_CG_EDGE_FLAGS_HZ | 
			       IPA_CG_EDGE_FLAGS_GBL));
	prev_node = node;
      }
  }

  tl = List_insert_last(tl, tlp);

  return tl;
}

void
IPA_connect_inputs(IPA_prog_info_t * info)
{
  IPA_callg_edge_t *edge;
  List tl = NULL;
  tlparam_t *tlp;

  List_start(info->globals->call_node->callee_edges);
  while ((edge = List_next(info->globals->call_node->callee_edges)))
    {
      IPA_funcsymbol_info_t *fninfo;
      IPA_interface_t       *iface;
      int i, bound;

      fninfo = edge->callee->fninfo;
      iface = edge->callee_if;

      bound = IPA_interface_get_num_params (iface);
      for (i = 0; i < bound; i++)
	{
	  int id = IPA_interface_get_param_id (iface, i);
	  IPA_cgraph_node_t *node = get_formal(fninfo->consg, id);
  
	  tl = ensure_tlparam(info, tl, node->data.syminfo->type_key);
	  tlp = find_tlparam(info, tl, node->data.syminfo->type_key);
	  /* Nothing is created for non-pointer parameters */
	  if (!tlp)
	    {
#if 0
	      printf("Skipping in func [%s]\n", fninfo->func_name);
#endif
	      continue;
	    }
	  
	  tlp->dst_nodes = List_insert_last(tlp->dst_nodes, node);
	}
    }

  List_start(tl);
  while ((tlp = List_next(tl)))
    {
      IPA_cgraph_node_t *src_node;
      IPA_cgraph_node_t *fm_dst_node;

      List_start(tlp->art_src_nodes);
      while ((src_node = List_next(tlp->art_src_nodes)))
	{
	  List_start(tlp->dst_nodes);
	  while ((fm_dst_node = List_next(tlp->dst_nodes)))
	    {
	      IPA_cgraph_edge_list_t *elist;
	      IPA_cgraph_edge_t *edge;
	      IPA_HTAB_ITER eiter;

	      printf("   ADDING TLP %d.%d %d -> %d %s \n",
		     tlp->type_key.file,
		     tlp->type_key.sym,
		     src_node->data.var_id,
		     fm_dst_node->data.var_id,
		     fm_dst_node->data.syminfo->fninfo->func_name);

	      for (elist = fm_dst_node->first_list; elist;
		   elist = elist->nxt_list)
		{
		  assert(elist->edge_type == ASSIGN);
		  IPA_HTAB_START(eiter, elist->out);
		  IPA_HTAB_LOOP(eiter)
		    {
		      edge = (IPA_cgraph_edge_t *)IPA_HTAB_CUR(eiter);
		      IPA_consg_ensure_edge_d (elist->edge_type, 
					       src_node, 
					       edge->dst_elist->node,
					       &edge->data,
					       edge->flags);
		    }
		} /* elist */
	    }
	}
    }

  free_tl(tl);
}






static int
has_deref_in(IPA_cgraph_node_t *node)
{
  IPA_cgraph_edge_list_t *elist;
  
  elist = IPA_cg_edge_list_find (node, DEREF_ASSIGN);
  if (!elist || IPA_htab_size(elist->in) == 0)
    return 0;
    
  return 1;
}

static List
IPA_setup_equiv(IPA_cgraph_t *consg, int mode)
{
  partition_t *common_part;
  List all_partitions;
  IPA_HTAB_ITER niter;

  /* Make the initial partitions
   */
  all_partitions = NULL;
  common_part = partition_new();

  IPA_HTAB_START(niter, consg->nodes);
  IPA_HTAB_LOOP(niter)
    { 
      IPA_cgraph_node_t *node;

      node = IPA_HTAB_CUR(niter);
      node->misc.ptr = NULL;

      if (node->rep_parent != node)
	continue;

      IPA_FLAG_SET(node->flags, NODE_VALID);

      if ( ((!IPA_FLAG_ISSET (node->flags, (IPA_CG_NODE_FLAGS_HEAP)) &&
	     IPA_FLAG_ISSET (node->flags, (IPA_CG_NODE_FLAGS_PARAM |
					   IPA_CG_NODE_FLAGS_RETURN |
					   IPA_CG_NODE_FLAGS_CALLEE |
					   IPA_CG_NODE_FLAGS_GLOBAL |
					   IPA_CG_NODE_FLAGS_NOCNTXT))))
#if 1
	   || (has_deref_in(node) && IPA_FLAG_ISCLR(mode, BUILD_OUT)) 
	   || (is_addr_taken(node) && IPA_FLAG_ISCLR(mode, BUILD_IN))
#endif
	   )
	{
	  partition_t *new_part;

	  new_part = partition_new();
	  partition_add_node(new_part, node);

	  all_partitions = List_insert_last(all_partitions, new_part);
	}
      else
	{
	  partition_add_node(common_part, node);
	}
    }

  if (IPA_htab_size (common_part->node_set) > 0)
    {
      all_partitions = List_insert_last(all_partitions, common_part);
    }
  else
    {
      partition_free(common_part);
    }

  return all_partitions;
}

void
IPA_merge_equiv(IPA_cgraph_t *consg, List all_partitions)
{
  partition_t *cur_part;
  
#if DEBUG_PARTITION
  printf ("mergining redundant nodes\n");
#endif

  while ((cur_part = List_next(all_partitions)))
    {
      IPA_cgraph_node_t *first_node;
      IPA_HTAB_ITER      node_enum;
      
      /* Select representative */
      first_node = NULL;
      IPA_HTAB_START(node_enum, cur_part->node_set);
      IPA_HTAB_LOOP (node_enum)
	{
	  IPA_cgraph_node_t *node;
	  node = IPA_HTAB_CUR (node_enum);

	  /* Don't merge nodes not in this graph */
	  if (node->cgraph != consg)
	    continue;

	  /* Don't merge globals or callee nodes */
	  assert(!IPA_FLAG_ISSET(node->flags, (IPA_CG_NODE_FLAGS_GLOBAL |
					       IPA_CG_NODE_FLAGS_CALLEE)));
	  if (first_node == NULL)
	    {
	      first_node = node;
	      continue;
	    }

	  if (IPA_FLAG_ISSET(first_node->flags, (IPA_CG_NODE_FLAGS_HEAP |
						 IPA_CG_NODE_FLAGS_ESCLOCAL)))
	    {
	      if (!IPA_FLAG_ISSET(node->flags, (IPA_CG_NODE_FLAGS_HEAP |
						IPA_CG_NODE_FLAGS_ESCLOCAL)))
		{
		  /* Keep heap/esc over others */
		  continue;
		}
	    }
	  else
	    {
	      if (IPA_FLAG_ISSET(node->flags, (IPA_CG_NODE_FLAGS_HEAP |
					       IPA_CG_NODE_FLAGS_ESCLOCAL)))
		{
		  /* Take heap/esc over others */
		  first_node = node;
		  continue;		  
		}
	    }

	  /* Otherwise take lowest version */
	  if (node->data.version < first_node->data.version)
	    {
	      first_node = node;
	      continue;
	    }
	}
      assert(first_node);

      IPA_HTAB_START(node_enum, cur_part->node_set);
      IPA_HTAB_LOOP (node_enum)
	{
	  IPA_cgraph_node_t *node;

	  node = IPA_HTAB_CUR (node_enum);

	  /* Don't merge nodes not in this graph */
	  if (node->cgraph != consg)
	    continue;
	  if (node == first_node)
	    continue;

#if 0
	  printf ("MERGE %d %s <= %d %s\n", 
		  first_node->data.var_id, 
		  first_node->data.syminfo->symbol_name,
		  node->data.var_id,
		  node->data.syminfo->symbol_name);
#endif

	  /* Must keep rep_children for heap/esc to facilitate  UP/DOWN edges later 
	   */
	  if (IPA_FLAG_ISSET(node->flags, (IPA_CG_NODE_FLAGS_HEAP |
					   IPA_CG_NODE_FLAGS_ESCLOCAL)))
	    IPA_cg_merge_nodes (first_node, node, 0, NULL, 0);
	  else
	    IPA_cg_merge_nodes (first_node, node, 1, NULL, 0);
	}

      partition_free (cur_part);
    }

  List_reset(all_partitions);
}

void
IPA_find_merge_summary_equiv_new (IPA_funcsymbol_info_t *fninfo,
				  IPA_cgraph_t *consg)
{
  List all_partitions;
  IPA_HTAB_ITER niter;
  
#if 0
  /* Basic simplification that helps avoid some stupid stuff
   */
  IPA_remove_single_assignment (consg, 1);
#endif

  /* Make the initial partitions
   */
  all_partitions = IPA_setup_equiv(consg, (BUILD_IN|BUILD_OUT));

  /* Process and split partitions until equivalence classes
   *   are formed
   */
  all_partitions = make_partitions(all_partitions, (BUILD_IN|BUILD_OUT), FOR_SUMMARY);
    
  /* Merge Partitions
   */
  IPA_merge_equiv(consg, all_partitions);
  all_partitions = NULL;


  IPA_HTAB_START(niter, consg->nodes);
  IPA_HTAB_LOOP(niter)
    { 
      IPA_cgraph_node_t *node;
      node = IPA_HTAB_CUR(niter);
      IPA_FLAG_CLR(node->flags, NODE_VALID);
    }
}








/*************************************************************************
 * Goal is to remove unnecessary, signle path assignments
 *  (e.g. a := b; If this is b's only input then a and b can be merged   
 *************************************************************************/

static IPA_cgraph_node_t *
IPA_has_single_assign_output(IPA_cgraph_node_t *node)
{
  IPA_cgraph_edge_list_t *elist, *assign_elist = NULL;
  IPA_cgraph_edge_t *edge;
  IPA_HTAB_ITER eiter;

  for (elist = node->first_list; elist;
       elist = elist->nxt_list)
    {
      switch(elist->edge_type)
	{
	case ASSIGN:
	  if (IPA_htab_size(elist->out) != 1)
	    return NULL;
	  assign_elist = elist;
	  break;
	case SKEW:
	case ASSIGN_ADDR:
	case ASSIGN_DEREF:
	  if (IPA_htab_size(elist->out) > 0)
	    return NULL;
	  break;
	case DEREF_ASSIGN:
	  if (IPA_htab_size(elist->in) > 0 ||
	      IPA_htab_size(elist->out) > 0)
	    return NULL;
	  break;
	default:
	  assert(0);
	}
    }
  if (!assign_elist)
    return NULL;

  edge = NULL;
  IPA_HTAB_START(eiter, assign_elist->out);
  IPA_HTAB_LOOP(eiter)
    {
      edge = (IPA_cgraph_edge_t *)IPA_HTAB_CUR(eiter);
      break;
    }

  if (IPA_FLAG_ISSET(edge->dst_elist->node->flags, (IPA_CG_NODE_FLAGS_PARAM|
						    IPA_CG_NODE_FLAGS_RETURN|
						    IPA_CG_NODE_FLAGS_CALLEE)) ||
      (edge->dst_elist->node == node))
    return NULL;

  return edge->dst_elist->node;
}


static IPA_cgraph_node_t *
IPA_has_single_assign_input(IPA_cgraph_node_t *node)
{
  IPA_cgraph_edge_list_t *elist, *assign_elist = NULL;
  IPA_cgraph_edge_t *edge;
  IPA_HTAB_ITER eiter;

#if 0
  printf("SIN-NODE %d.%d : ", node->data.var_id, node->data.version);
#endif
  for (elist = node->first_list; elist;
       elist = elist->nxt_list)
    {
      switch(elist->edge_type)
	{
	case ASSIGN:
	  if (IPA_htab_size(elist->in) != 1)
	    {
#if 0
	      printf("assign %d\n",IPA_htab_size(elist->in));
#endif
	      return NULL;
	    }
	  assign_elist = elist;
	  break;
	case SKEW:
	case ASSIGN_ADDR:
	case ASSIGN_DEREF:
	  if (IPA_htab_size(elist->in) > 0)
	    {
#if 0
	      printf("other %d\n",IPA_htab_size(elist->in));
#endif
	      return NULL;
	    }
	  break;
	case DEREF_ASSIGN: 
	  break;
	default:
	  assert(0);
	}
    }
  if (!assign_elist)
    {
#if 0
      printf("none\n");
#endif
      return NULL;
    }
  
  edge = NULL;
  IPA_HTAB_START(eiter, assign_elist->in);
  IPA_HTAB_LOOP(eiter)
    {
      edge = (IPA_cgraph_edge_t *)IPA_HTAB_CUR(eiter);
      break;
    }
  
  if (IPA_FLAG_ISSET(edge->src_elist->node->flags, (IPA_CG_NODE_FLAGS_PARAM|
						    IPA_CG_NODE_FLAGS_RETURN|
						    IPA_CG_NODE_FLAGS_CALLEE)) ||
      (edge->src_elist->node == node))
    {
#if 0
      printf("invalid\n");      
#endif
      return NULL;
    }

  
  return edge->src_elist->node;
}


void
IPA_remove_single_assignment (IPA_cgraph_t *consg, int delete)
{
  IPA_cgraph_node_t *node;
  IPA_HTAB_ITER niter;
  List del_list = NULL;
  int cnt = 0;
  
  IPA_HTAB_START(niter, consg->nodes);
  IPA_HTAB_LOOP(niter)
    { 
      IPA_cgraph_node_t *mnode;
      node = IPA_HTAB_CUR(niter);

      if (IPA_FLAG_ISSET(node->flags, (IPA_CG_NODE_FLAGS_PARAM|
				       IPA_CG_NODE_FLAGS_RETURN|
				       IPA_CG_NODE_FLAGS_CALLEE)))
	continue;

      if ((mnode = IPA_has_single_assign_input(node)))
	{
	  IPA_cg_merge_nodes(node, mnode, 0, NULL, 0);
	  if (delete && !List_member(del_list, mnode))
	    del_list = List_insert_last(del_list, mnode);
	  cnt++;
	}
      if ((mnode = IPA_has_single_assign_output(node)))
	{
	  IPA_cg_merge_nodes(node, mnode, 0, NULL, 0);
	  if (delete && !List_member(del_list, mnode))
	    del_list = List_insert_last(del_list, mnode);
	  cnt++;
	}
    }

#if 0
  printf("Single Assign Found %d Delete %d\n",cnt,List_size(del_list));
#endif
  List_start(del_list);
  while ((node = List_next(del_list)))
    {
      IPA_cg_node_delete(node);
    }
  List_reset(del_list);
}



/*************************************************************************
 *  Goal is to remove redundant deref loops such as
 *     x = *y; *y = x;   This tends to crop up and inters with simplification
 *************************************************************************/


void
IPA_find_delete_deref_loop(IPA_cgraph_t *consg)
{
  IPA_cgraph_node_t *node;
  IPA_HTAB_ITER niter;
  List del_list = NULL;

  IPA_HTAB_START(niter, consg->nodes);
  IPA_HTAB_LOOP(niter)
    { 
      IPA_cgraph_edge_list_t *elist;
      IPA_cgraph_node_t *ad_node = NULL;
      IPA_cgraph_node_t *da_node = NULL;
      int valid;

      node = IPA_HTAB_CUR(niter);
      
      if (IPA_FLAG_ISSET(node->flags, (IPA_CG_NODE_FLAGS_PARAM|
				       IPA_CG_NODE_FLAGS_RETURN|
				       IPA_CG_NODE_FLAGS_CALLEE)))
	continue;

#if 0
      printf("DDL-NODE %d.%d : ", node->data.var_id, node->data.version);
#endif
      
      /* Look for "x" in x = *y; *y = x; */
      valid = 1;
      for (elist = node->first_list; elist;
	   elist = elist->nxt_list)
	{
	  switch(elist->edge_type)
	    {
	    case SKEW:
	    case ASSIGN:
	    case ASSIGN_ADDR:
	      if (IPA_htab_size(elist->in) != 0 ||
		  IPA_htab_size(elist->out) != 0)
		{
#if 0
		  printf("saa %d %d\n",IPA_htab_size(elist->in),IPA_htab_size(elist->out));
#endif
		  valid = 0;
		}
	      break;
	    case ASSIGN_DEREF:
	      if (IPA_htab_size(elist->in) != 1 ||
		  IPA_htab_size(elist->out) != 0)
		{
#if 0
		  printf("ad %d %d\n",IPA_htab_size(elist->in),IPA_htab_size(elist->out));
#endif
		  valid = 0;
		}
	      if (IPA_htab_size(elist->in) == 1)
		{
		  IPA_cgraph_edge_t *edge;
		  edge = IPA_HTAB_FIRST(elist->in);
		  ad_node = edge->src_elist->node;
		}
	      break;
	    case DEREF_ASSIGN: 
	      if (IPA_htab_size(elist->in) != 0 ||
		  IPA_htab_size(elist->out) != 1)
		{
#if 0
		  printf("da %d %d\n",IPA_htab_size(elist->in),IPA_htab_size(elist->out));
#endif
		  valid = 0;
		}
	      if (IPA_htab_size(elist->out) == 1)
		{
		  IPA_cgraph_edge_t *edge;
		  edge = IPA_HTAB_FIRST(elist->out);
		  da_node = edge->dst_elist->node;
		}
	      break;
	    default:
	      assert(0);
	    }
	}

      if (!valid)
	continue;

      if (!ad_node || ad_node != da_node)
	{
#if 0
	  printf("mismatch %p %p \n",ad_node,da_node);
#endif
	  continue;
	}

#if 1
      printf("MATCH-NODE %d.%d : ", node->data.var_id, node->data.version);
#endif
      del_list = List_insert_last(del_list, node);
    }

      
  if (List_size(del_list) > 0)
    {
      /*printf("Deref loop %d\n",List_size(del_list));*/
      List_start(del_list);
      while ((node = List_next(del_list)))
	{
	  IPA_cg_node_delete(node);
	}
    }
  List_reset(del_list);
}

