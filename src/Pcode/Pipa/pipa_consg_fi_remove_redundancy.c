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


#if 0

/*****************************************************************************\
 *
 *      File:    pipa_consg_fi_remove_redundancy.c
 *      Author:  Hong-Seok Kim
 *      Copyright (c) 2003  Hong-Seok Kim, Erik Nystrom, Wen-mei Hwu
 *              and The Board of Trustees of the University of Illinois.
 *              All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#include "pipa_consg_fi.h"

#define DEBUG_PARTITION 0

/*****************************************************************************\
 * Hash function supports                                                    *
\*****************************************************************************/

IPA_Hashtab_t*
IPA_htab_copy_unique (IPA_Hashtab_t *htab);

IPA_Hashtab_t*
IPA_htab_substract_unique (IPA_Hashtab_t *htab1, IPA_Hashtab_t *htab2);

IPA_Hashtab_t *
IPA_htab_merge_unique (IPA_Hashtab_t *htab1, IPA_Hashtab_t *htab2);

void*
IPA_htab_find_unique (IPA_Hashtab_t *htab, unsigned int key);

void
IPA_htab_remove_unique (IPA_Hashtab_t *htab, unsigned int key);

/*****************************************************************************\
 * Static function declarations                                              *
\*****************************************************************************/
 
static IPA_Hashtab_t* make_partition (IPA_cgraph_t *consg);
static int is_addr_taken (IPA_cgraph_node_t *node);

static IPA_Hashtab_t* make_out_set (IPA_Hashtab_t *set, 
				    IPA_cgraph_edgelist_e edgetype);

#if DEBUG_PARTITION
static void print_node_set (IPA_Hashtab_t *node_set);
static void print_partition (IPA_Hashtab_t *partition);
#endif

/*****************************************************************************\
 * Redundancy removal function: an adaption of Hopcroft's algorithm          *
\*****************************************************************************/

void
IPA_consg_fi_remove_redundancy (IPA_cgraph_t *consg)
{
  // All nodes in the same part are equivalent in the sense that they
  // point to the same set of variables.

  IPA_Hashtab_t *partition, *part;
  IPA_HTAB_ITER part_enum;

  partition = make_partition (consg);

  // Merge all nodes in the same partition.

#if DEBUG_PARTITION
  printf ("mergining redundant nodes\n");
#endif

  IPA_HTAB_START(part_enum, partition);
  IPA_HTAB_LOOP (part_enum)
    {
      IPA_cgraph_node_t *first_node;
      IPA_HTAB_ITER node_enum;

      part = IPA_HTAB_CUR (part_enum);
      first_node = NULL;

      IPA_HTAB_START(node_enum, part);
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
	      printf ("%d <= %d\n", first_node->data.consg.var_id, 
		      node->data.consg.var_id);
#endif
	      IPA_cg_merge_nodes (first_node, node, 1);
	    }
	}

      IPA_htab_free (part);
    }

  IPA_htab_free (partition);

#if DEBUG_PARTITION
  printf ("\n");
#endif
}

/*****************************************************************************\
 * Static function declarations                                              *
\*****************************************************************************/

static IPA_Hashtab_t*
make_partition (IPA_cgraph_t *consg)
{
  // Optimistic partitioning algorithm: an adaption of Hopcroft's algorithm.
  //
  // partition  := empty set;
  // commonpart := empty set;
  //
  // for each node in graph
  //   {
  //     if (node is parm || return || callee || global 
  //         || address of node is taken)
  //       {
  //         add { node } in partition;
  //       }
  //     else
  //       {
  //         add node in commonpart;
  //       }
  //   }
  //
  // if (commonpart is not empty)
  //   {
  //     add commonpart in partition;
  //   }   
  //
  // worklist := empty list;
  //
  // for each part in partition
  //   {
  //     add part in worklist;
  //   }
  //
  // while worklist is not empty
  //  {
  //    part := remove first from worklist;
  //    
  //    for each edgetype
  //	  {
  //	    outset := set of every node that has an edge of edgetype
  //	              from a node in n part;
  //
  //        for each part in partition
  //          {
  //            if (intersect (part, outset) && !subset (part, outset))
  //              {
  //                newpart := part - outset;
  //                
  //                if (part in worklist)
  //                  {
  //                    add newpart in worklist;
  //                  }
  //                else
  //                  {
  //                    if (|newpart| <= |part|)
  //                      {
  //                        add newpart in worklist;
  //                      }
  //                    else
  //                      {
  //                        add part in worklist;
  //                      }
  //                  }
  //              }
  //          }
  //	  }
  //  }

  IPA_Hashtab_t *common_part, *part, *partition, *work_set;
  List work_list;
  IPA_HTAB_ITER niter;

  partition = IPA_htab_new (0);
  common_part = IPA_htab_new (0);

  IPA_HTAB_START(niter, consg->nodes);
  IPA_HTAB_LOOP (niter)
    { 
      IPA_cgraph_node_t *node;

      node = IPA_HTAB_CUR(niter);

      if (IPA_FLAG_ISSET (node->flags, (IPA_CG_NODE_FLAGS_PARAM |
					IPA_CG_NODE_FLAGS_RETURN |
					IPA_CG_NODE_FLAGS_CALLEE |
					IPA_CG_NODE_FLAGS_GLOBAL |
					IPA_CG_NODE_FLAGS_NOCNTXT)) ||
	  is_addr_taken (node))
	{
	  IPA_Hashtab_t *new_part;

	  new_part = IPA_htab_new (1);
	  new_part = IPA_htab_insert (new_part, node, (int)(long)node);
	  partition = IPA_htab_insert (partition, new_part, 
				       (int)(long)new_part);
	}
      else
	{
	  common_part = IPA_htab_insert (common_part, node, (int)(long)node);
	}
    }

  if (IPA_htab_size (common_part) > 0)
    {
      partition = IPA_htab_insert (partition, common_part, (int)(long)common_part);
    }
  else
    {
      IPA_htab_free (common_part);
    }

#if DEBUG_PARTITION
  printf ("initial parition\n");
  print_partition (partition);
  printf ("\n");
#endif

  work_list = IPA_htab2list (partition);
  work_set = IPA_htab_copy_unique (partition);

  List_start (work_list);

  while ((part = List_next (work_list)))
    {
      IPA_cgraph_edgelist_e edgetype;

      work_list = List_delete_current (work_list);
      IPA_htab_remove_unique (work_set, (int)(long)part);

      for (edgetype = ASSIGN; edgetype <= ASSIGN_DEREF; edgetype++)
	{
	  IPA_Hashtab_t *out_set;
	  IPA_HTAB_ITER part_enum;

	  out_set = make_out_set (part, edgetype);

	  IPA_HTAB_START(part_enum, partition);
	  IPA_HTAB_LOOP (part_enum)
	    {
	      IPA_Hashtab_t *part, *new_part;
	      
	      part = IPA_HTAB_CUR (part_enum);
	      new_part = IPA_htab_substract_unique (part, out_set);
	      
	      if (IPA_htab_size (new_part) != 0)
		{
		  if (IPA_htab_size (part) != 0)
		    {
		      // Only in this case, we need to do partition.
		      // Add new_part into partition.
		      
		      partition = IPA_htab_insert (partition, new_part, 
						   (int)(long)new_part);
		      
		      if (IPA_htab_find_unique (work_set, 
						(unsigned int)(long)part))
			{
			  // Since we've not processed part yet,
			  // new_part must be put into work_list.
			  
			  work_list = List_insert_last (work_list, new_part);
			  work_set = IPA_htab_insert (work_set, new_part, 
						      (int)(long)new_part);
			}
		      else
			{
			  // Since part is already processed, only one of
			  // part or new_part must be put in work_list.
			  // Pick up the smaller one.
			  
			  if (IPA_htab_size (part) >= IPA_htab_size (new_part))
			    {
			      work_list = List_insert_last (work_list, part);
			      work_set = IPA_htab_insert (work_set, part, 
							  (int)(long)new_part);
			    }
			  else
			    {
			      work_list = List_insert_last (work_list, 
							    new_part);

			      work_set = IPA_htab_insert (work_set, new_part, 
							  (int)(long)new_part);
			    }
			}
		    }
		  else
		    {
		      // Since part was a subset of inverse, don't partition.
		      // However, since we've moved everyting into new_part,
		      // we need to move it back to part.
		      
		      part = IPA_htab_merge_unique (part, new_part);
		    }
		}
	      else
		{
		  // There is no need to do partition and new_part is obsolete.
		  // Simply, delete new_part.

		  IPA_htab_free (new_part);
		}
	    }
	}
    }

#if 0
  assert (IPA_htab_size (work_set) == 0);
#else
  if (IPA_htab_size (work_set) != 0)
    printf("WORKSET NOT EMPTy\n");
#endif

  List_reset (work_list);

#if DEBUG_PARTITION
  printf ("final parition\n");
  print_partition (partition);
  printf ("\n");
#endif

  return partition;
}

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

static IPA_Hashtab_t*
make_out_set (IPA_Hashtab_t *set, IPA_cgraph_edgelist_e edge_type)
{
  // If edgetype is ASSIGN, we ignore outgoing edges into the same partition.
  // Is is safe to do this? Does it incur any addition costs?
  
  IPA_Hashtab_t *out_set;
  IPA_HTAB_ITER set_enum;


  out_set = IPA_htab_new (0);

  if (edge_type == DEREF_ASSIGN)
    {
      // If edge_type is DEREF_ASSIGN, the actual target variable is
      // not updated. Therefore, it will not incur a split.
      // Is it correct?

      return out_set;
    }

  IPA_HTAB_START(set_enum, set);
  IPA_HTAB_LOOP (set_enum)
    {
      IPA_cgraph_node_t *node;
      IPA_cgraph_edge_list_t *edge_list;
      IPA_HTAB_ITER eiter;

      node = IPA_HTAB_CUR (set_enum);
      edge_list = IPA_cg_edge_list_find(node, edge_type);
      if (edge_list)
	{
	  IPA_HTAB_START(eiter, edge_list->out);
	  IPA_HTAB_LOOP(eiter)
	    {
	      IPA_cgraph_edge_t *out_edge;
	      IPA_cgraph_node_t *out_node;
	      
	      out_edge = IPA_HTAB_CUR(eiter);
	      out_node = out_edge->dst_elist->node;
	      
	      if (edge_type == ASSIGN && 
		  IPA_htab_find_unique (set, 
					(unsigned int)(long)out_node))
		{
		  continue;
		}
	      else
		{
		  out_set = IPA_htab_insert (out_set, out_node, 
					     (int)(long)out_node);
		}
	    }
	}
    }

  return out_set;
}

#if DEBUG_PARTITION

static void
print_node_set (IPA_Hashtab_t *node_set)
{
  IPA_HashtabEnum_t hash_enum;
  int first;

  printf ("[");

  hash_enum.htab = node_set;
  first = 1;

  HTAB_ENUM_LOOP (hash_enum)
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

      node = HTAB_ENUM_CURRENT (hash_enum);
      printf ("%d", node->data.consg.var_id);
    }

  printf ("]");
}

static void
print_partition (IPA_Hashtab_t *partition)
{
  IPA_HashtabEnum_t part_enum;
  
  part_enum.htab = partition;

  HTAB_ENUM_LOOP (part_enum)
    {
      IPA_Hashtab_t *part;

      part = HTAB_ENUM_CURRENT (part_enum);
      print_node_set (part);
      printf ("\n");
    }
}

#endif

unsigned int cmp_item;

static int
compare_addr (void *item)
{
  if ((unsigned int)(long)item == cmp_item)
    return 1;

  return 0;
}

IPA_Hashtab_t*
IPA_htab_copy_unique (IPA_Hashtab_t *htab)
{
  IPA_Hashtab_t *new_htab;
  IPA_HTAB_ITER htab_enum;

  new_htab = IPA_htab_new (0);

  IPA_HTAB_START(htab_enum, htab);
  IPA_HTAB_LOOP (htab_enum)
    {
      void *item;

      item = IPA_HTAB_CUR (htab_enum);
      new_htab = IPA_htab_insert (new_htab, item, (int)(long)item);
    }

  return new_htab;
}

IPA_Hashtab_t*
IPA_htab_substract_unique (IPA_Hashtab_t *htab1, IPA_Hashtab_t *htab2)
{
  // For each item in htab2, if it is also in htab2, 
  // move it to htab3.

  IPA_Hashtab_t *htab3;
  IPA_HTAB_ITER htab1_enum, htab3_enum;

  htab3 = IPA_htab_new (0);

  IPA_HTAB_START(htab1_enum, htab1);
  IPA_HTAB_LOOP (htab1_enum)
    {
      void *item;

      item = IPA_HTAB_CUR (htab1_enum);

      if (IPA_htab_find_unique (htab2, 
				(unsigned int)(long)item))
	{
	  htab3 = IPA_htab_insert (htab3, item, (int)(long)item);
	}
    }

  htab3_enum.htab = htab3;

  IPA_HTAB_START(htab3_enum, htab3);
  IPA_HTAB_LOOP (htab3_enum)
    {
      void *item;

      item = IPA_HTAB_CUR (htab3_enum);

      IPA_htab_remove_unique (htab1, (int)(long)item);
    }

  return htab3;
}

IPA_Hashtab_t *
IPA_htab_merge_unique (IPA_Hashtab_t *htab1, IPA_Hashtab_t *htab2)
{
  // Move everything in htab2 into htab1 and free htab2.
  // Do not worry about performance now.

  IPA_HTAB_ITER htab2_enum;

  IPA_HTAB_START(htab2_enum, htab2);
  IPA_HTAB_LOOP (htab2_enum)
    {
      void *item;

      item = IPA_HTAB_CUR (htab2_enum);
      htab1 = IPA_htab_insert (htab1, item, (int)(long)item);
    }

  IPA_htab_free (htab2);

  return htab1;
}

void*
IPA_htab_find_unique (IPA_Hashtab_t *htab, unsigned int key)
{
  cmp_item = (unsigned int)(long)key;
  return IPA_htab_find (htab, key, compare_addr);
}

void
IPA_htab_remove_unique (IPA_Hashtab_t *htab, unsigned int key)
{
  cmp_item = (unsigned int)(long)key;
  IPA_htab_remove (htab, key, compare_addr);
}

#endif

