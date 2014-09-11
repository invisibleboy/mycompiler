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
 *      File:    pipa_consg_fdvs_remove_redundancy.c
 *      Author:  Erik Nystrom
 *      Copyright (c) 2003  Erik Nystrom, Wen-mei Hwu
 *              and The Board of Trustees of the University of Illinois.
 *              All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#include "pipa_consg_fdvs.h"
#include "pipa_consg_construct.h"
#include "pipa_misc_utils.h"


/*****************************************************************************\
 * Static function declarations                                              *
\*****************************************************************************/

static void
normalize_graph(IPA_cgraph_t *consg);

static void
delete_dead_nodes(IPA_cgraph_t *consg);


/*****************************************************************************\
 * Redundancy removal function: an adaption of Hopcroft's algorithm          *
\*****************************************************************************/

static IPA_prog_info_t *prog_info;
static IPA_funcsymbol_info_t *fn_info;

void
IPA_consg_fdvs_remove_redundancy (IPA_prog_info_t *__prog_info,
				  IPA_funcsymbol_info_t *__fn_info,
				  IPA_cgraph_t *consg)
{
  // All nodes in the same part are equivalent in the sense that they
  // point to the same set of variables.

  prog_info = __prog_info;
  fn_info = __fn_info;

#if 1
  normalize_graph(consg);
#endif

#if 1
  IPA_find_delete_deref_loop(consg);
#endif

#if 1
  delete_dead_nodes(consg);
#endif
  
#if 0
  IPA_find_merge_summary_equiv (consg);
#else
  IPA_find_merge_summary_equiv_new (__fn_info, consg);
#endif
}

/*****************************************************************************\
 * Static function declarations                                              *
\*****************************************************************************/


/* NORMALIZE ROUTINES
 *
 */


typedef struct norm_node_t 
{
  int offset;
  char has_def, has_use;
  List in;
  List out;
} norm_node_t;

typedef struct norm_t 
{
  int first_offset;
  IPA_Hashtab_t *htab;
} norm_t;


static norm_node_t *
norm_node_new()
{
  return (norm_node_t *)calloc(1, sizeof(norm_node_t));
}

static void
norm_node_free(norm_node_t *nn)
{
  if (!nn)
    return;
  List_reset(nn->out);
  List_reset(nn->in);
  free(nn);
}


int nn_cmp_offset;
#define NORM_DELETE IPA_CG_EDGE_FLAGS_GENERIC1

static int nn_compare(void *item)
{
  if (((norm_node_t *)(item))->offset == nn_cmp_offset)
    return 1;
  return 0;
}

static norm_node_t *
ensure_norm_node(norm_t *all_nn, int offset)
{
  norm_node_t *nn;

#if 0
  printf("  ensure_norm_node: %d \n",offset);
#endif
  nn_cmp_offset = offset;
  nn = IPA_htab_find (all_nn->htab, offset, nn_compare);
  
  if (!nn)
    {
      if (all_nn->first_offset == -1 || all_nn->first_offset > offset)
	all_nn->first_offset = offset;

      nn = norm_node_new();
      nn->offset = offset;
      IPA_htab_insert (all_nn->htab, nn, offset);
    }
  assert(nn->offset == offset);

  return nn;
}

static void
find_norm_nodes(norm_t *all_nn, IPA_cgraph_node_t *node)
{
  IPA_cgraph_edge_t *edge;
  IPA_cgraph_edge_list_t *elist;
  IPA_HTAB_ITER eiter;
  norm_node_t *nn;
  
  for (elist=node->first_list; elist; elist=elist->nxt_list)
    {
      IPA_HTAB_START(eiter, elist->in);
      IPA_HTAB_LOOP(eiter)
	{
	  edge = IPA_HTAB_CUR(eiter);
	  if (edge->data.target_offset > node->data.var_size - IPA_POINTER_SIZE)
	    continue;
	  nn = ensure_norm_node(all_nn, edge->data.target_offset);
	  nn->in = List_insert_last(nn->in, edge);
	  IPA_FLAG_CLR(edge->flags, NORM_DELETE);
	  if (elist->edge_type != DEREF_ASSIGN)
	    nn->has_def = 1;
	  else
	    nn->has_use = 1;
	}
      
      IPA_HTAB_START(eiter, elist->out);
      IPA_HTAB_LOOP(eiter)
	{
	  edge = IPA_HTAB_CUR(eiter);
	  if (edge->data.source_offset > node->data.var_size - IPA_POINTER_SIZE)
	    continue;
	  nn = ensure_norm_node(all_nn, edge->data.source_offset);
	  nn->out = List_insert_last(nn->out, edge);
	  IPA_FLAG_CLR(edge->flags, NORM_DELETE);
	  nn->has_use = 1;
	}
    }
}

static IPA_cgraph_node_t *
norm_tmp_node(IPA_cgraph_t *consg, int root_size, int flags)
{
  IPA_symbol_info_t *syminfo;
  IPA_cgraph_node_t *tmp_node;

  assert(root_size > 0);

  syminfo = IPA_new_tmp_var(prog_info, consg->data.fninfo, 0);

  tmp_node = IPA_consg_ensure_node (consg, syminfo->id, 1, 
				    root_size, syminfo,
				    (flags | IPA_CG_NODE_FLAGS_TEMP));
#if 0
  printf("  NORMTEMP: %d %s\n",
	 syminfo->id, 
	 syminfo->symbol_name);
#endif
  return tmp_node;
}

static void
normalize_node(IPA_cgraph_t *consg, IPA_cgraph_node_t *node)
{
  IPA_HTAB_ITER htab_enum; 
  IPA_cgraph_node_t *nn_node = NULL;
  IPA_cgraph_edge_t *edge;
  norm_node_t *nn;
  norm_t all_nn;
  int e_cnt = 0, n_cnt = 0;
  List del_list;

#if 0
  printf("Normalize node: %d \n",
	 node->data.var_id);
#endif

  all_nn.first_offset = (unsigned int)(-1);
  all_nn.htab =  IPA_htab_new (3);

  find_norm_nodes(&all_nn, node);
  del_list = NULL;

  IPA_HTAB_START(htab_enum, all_nn.htab);
  IPA_HTAB_LOOP (htab_enum)
    {
      nn = (norm_node_t*)IPA_HTAB_CUR (htab_enum);

#if 0
      printf("  NN offset %d\n", nn->offset);
#endif

      if (all_nn.first_offset == nn->offset || all_nn.first_offset == -1)
	nn_node = node;
      else if (nn->has_def && nn->has_use)
	{
	  n_cnt++;
	  nn_node = norm_tmp_node(consg, (node->data.var_size - 
					  nn->offset), node->flags);
	}
      
      List_start(nn->in);
      while ((edge = List_next(nn->in)))
	{
	  if (edge->data.target_offset == 0)
	    continue;
	  e_cnt++;

	  if (nn->has_use)
	    {
	      IPA_consg_ensure_edge(edge->src_elist->edge_type,
				    edge->src_elist->node, nn_node, 
				    0, edge->data.assign_size, 
				    edge->data.source_offset, 
				    edge->flags);
	    }

	  if (!IPA_FLAG_ISSET(edge->flags, NORM_DELETE))
	    {
	      del_list = List_insert_last(del_list, edge);
	      IPA_FLAG_SET(edge->flags, NORM_DELETE);
	    }
	}

      List_start(nn->out);
      while ((edge = List_next(nn->out)))
	{
	  if (edge->data.source_offset == 0)
	    continue;
	  e_cnt++;

	  if (nn->has_def)
	    {
	      IPA_consg_ensure_edge(edge->src_elist->edge_type,
				    nn_node, edge->dst_elist->node,
				    edge->data.target_offset, 
				    edge->data.assign_size, 0, 
				    edge->flags);
	    }
	  if (!IPA_FLAG_ISSET(edge->flags, NORM_DELETE))
	    {
	      del_list = List_insert_last(del_list, edge);
	      IPA_FLAG_SET(edge->flags, NORM_DELETE);
	    }
	}
      
      norm_node_free(nn);
    }

  IPA_htab_free(all_nn.htab);

  List_start(del_list);
  while ((edge = List_next(del_list)))
    {
      IPA_cg_edge_delete(edge);
    }
  List_reset(del_list);
  del_list = NULL;

  if (n_cnt > 0 || e_cnt > 0)
    {
      DEBUG_IPA(2, printf("OPTI: Split %d nodes, Normized %d edges\n",
			  n_cnt, e_cnt););
    }
}

static void
normalize_graph(IPA_cgraph_t *consg)
{
  IPA_cgraph_node_t *node;
  IPA_HTAB_ITER niter;
  List list;

  list = NULL;
  IPA_HTAB_START(niter, consg->nodes);
  IPA_HTAB_LOOP(niter)
    {
      node = IPA_HTAB_CUR(niter);

      if (IPA_FLAG_ISSET (node->flags, (IPA_CG_NODE_FLAGS_PARAM |
					IPA_CG_NODE_FLAGS_RETURN |
					IPA_CG_NODE_FLAGS_ESCLOCAL |
					IPA_CG_NODE_FLAGS_HEAP)))
	continue;

      list = List_insert_last(list, node);
    }
  
  List_start(list);
  while ((node = List_next(list)))
    {
      normalize_node(consg, node);
    }  

  List_reset(list);

#if 0
  if (strstr(fn_info->func_name, "decode_mcu_jdhuff"))
    {
      char name[256];
      sprintf(name, "%s.NORM%d", fn_info->func_name, debug.round);
      IPA_cg_DOTprint (consg, name, IPA_CG_ETYPEALL_FLAG);
    }
#endif
}




/* DEADCODE ROUTINES
 *
 */

#define IPA_DEAD_FLAG IPA_CG_NODE_FLAGS_GENERIC12
List dead_list;

static List
dead_check(List work_list, IPA_cgraph_node_t *node)
{
  IPA_cgraph_edge_t *edge;
  IPA_cgraph_edge_list_t *elist;
  IPA_HTAB_ITER eiter;
  int has_use, has_def;
  
  if (IPA_FLAG_ISSET(node->flags, IPA_DEAD_FLAG))
    return work_list;

  /* Look for defs/uses 
   */
  has_use = 0;
  has_def = 0;
  for (elist=node->first_list; elist; elist=elist->nxt_list)
    {
      IPA_HTAB_START(eiter, elist->in);
      IPA_HTAB_LOOP(eiter)
	{
	  edge = IPA_HTAB_CUR(eiter);
	  if (IPA_FLAG_ISSET(edge->src_elist->node->flags, IPA_DEAD_FLAG))
	    continue;
	  
	  if (elist->edge_type == DEREF_ASSIGN)
	    has_use = 1;
	  else
	    has_def = 1;
	  break;
	}

      IPA_HTAB_START(eiter, elist->out);
      IPA_HTAB_LOOP(eiter)
	{
	  edge = IPA_HTAB_CUR(eiter);
	  if (IPA_FLAG_ISSET(edge->dst_elist->node->flags, IPA_DEAD_FLAG))
	    continue;

	  has_use = 1;
	  break;
	}
    }

  /* Determine if node is dead
   */
  if (!has_def)
    {
      /* Add all consumers to work_list */
      for (elist=node->first_list; elist; elist=elist->nxt_list)
	{
	  if (elist->edge_type == DEREF_ASSIGN)
	    {
	      IPA_HTAB_START(eiter, elist->in);
	      IPA_HTAB_LOOP(eiter)
		{
		  edge = IPA_HTAB_CUR(eiter);
		  work_list = List_insert_last(work_list, edge->src_elist->node);
		}
	    }
	  IPA_HTAB_START(eiter, elist->out);
	  IPA_HTAB_LOOP(eiter)
	    {
	      edge = IPA_HTAB_CUR(eiter);
	      work_list = List_insert_last(work_list, edge->dst_elist->node);
	    }
	}
      IPA_FLAG_SET(node->flags, IPA_DEAD_FLAG);
      dead_list = List_insert_last(dead_list, node);
    }
  else if (!has_use)
    {
      /* Add all producers to work_list */
      for (elist=node->first_list; elist; elist=elist->nxt_list)
	{
	  if (elist->edge_type == DEREF_ASSIGN)
	    continue;
	  IPA_HTAB_START(eiter, elist->in);
	  IPA_HTAB_LOOP(eiter)
	    {
	      edge = IPA_HTAB_CUR(eiter);
	      work_list = List_insert_last(work_list, edge->src_elist->node);
	    }
	}
      IPA_FLAG_SET(node->flags, IPA_DEAD_FLAG);
      dead_list = List_insert_last(dead_list, node);
    }

  return work_list;
}

static void
delete_dead_nodes(IPA_cgraph_t *consg)
{
  List work_list = NULL;
  IPA_cgraph_node_t *node;
  IPA_HTAB_ITER niter;

  dead_list = NULL;

  /* Find initial set of dead nodes */
  IPA_HTAB_START(niter, consg->nodes);
  IPA_HTAB_LOOP(niter)
    {
      node = IPA_HTAB_CUR(niter);
      if (IPA_FLAG_ISSET (node->flags, (IPA_CG_NODE_FLAGS_PARAM |
					IPA_CG_NODE_FLAGS_RETURN |
					IPA_CG_NODE_FLAGS_ESCLOCAL)))
	continue;

      work_list = dead_check(work_list, node);
    }

  /* Process potentially dead nodes until none left
   */
  while ((node = List_first(work_list)))
    {
      work_list = List_delete_current(work_list);
      if (IPA_FLAG_ISSET (node->flags, (IPA_CG_NODE_FLAGS_PARAM |
					IPA_CG_NODE_FLAGS_RETURN |
					IPA_CG_NODE_FLAGS_ESCLOCAL)))
	continue;

      work_list = dead_check(work_list, node);
    }

  List_reset(work_list);

  DEBUG_IPA(2, printf("OPTI: Deleted dead nodes %d\n", List_size(dead_list)););
  List_start(dead_list);
  while ((node = List_next(dead_list)))
    {
      assert(!IPA_FLAG_ISSET (node->flags, (IPA_CG_NODE_FLAGS_PARAM |
					    IPA_CG_NODE_FLAGS_RETURN |
					    IPA_CG_NODE_FLAGS_ESCLOCAL)));
      IPA_cg_node_delete(node);
    }
  List_reset(dead_list);
}

