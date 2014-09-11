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
 *      File:    pipa_consg_fi_solve_fully.c
 *      Author:  Erik Nystrom
 *      Copyright (c) 2003  Erik Nystrom, Hong-Seok Kim, Wen-mei Hwu
 *              and The Board of Trustees of the University of Illinois.
 *              All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#include "pipa_consg_fi.h"

static void process_assign (IPA_cgraph_edge_t * edge);
static void process_assign_addr (IPA_cgraph_edge_t * edge);
static void process_assign_deref (IPA_cgraph_edge_t * edge);
static void process_deref_assign (IPA_cgraph_edge_t * edge);

static void ensure_edge (IPA_cgraph_edgelist_e edge_type,
			 IPA_cgraph_node_t * src_node, 
			 IPA_cgraph_node_t * dst_node,
			 IPA_cgraph_edge_t * src_edge,
			 IPA_cgraph_edge_t * dst_edge);

static IPA_cgraph_t *cg;
static List work_list;
static int print = 0;

/*****************************************************************************
 * Given a constraint graph, make it closed under the following rules.
 *
 *  a :=  b  &&  b := &c => a := &c
 *  a := *b  &&  b := &c => a := c
 * *a :=  b  &&  a := &c => c := b 
 *
 *****************************************************************************/

void
IPA_consg_fi_solve_fully (IPA_cgraph_t * __cg, List __work_list)
{
  IPA_cgraph_edge_t *edge;
  int cnt = 0;

  cg = __cg;
  work_list = __work_list;
  print = 0;

  // Put every AssignAddr edge into worklist

  List_start (work_list);

  while ((edge = List_next (work_list)))
    {
      work_list = List_delete_current (work_list);
      if ((cnt % 5000) == 0)
	{
	  printf("IPA_consg_fi_solve_fully: %d edges  [%d]\n",
		 List_size(work_list), cnt);
	  fflush(stdout);
	}
      cnt++;

#if 0
	{
	  printf("EDGE [%p] %s: ",edge, IPA_cg_edge_flag_name(edge));
	  IPA_cg_node_print(stdout,edge->dst_elist->node, IPA_PRINT_ASCI);
	  printf(" <- ");
	  IPA_cg_node_print(stdout,edge->src_elist->node, IPA_PRINT_ASCI);
	  printf("\n");
	}
#endif

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

        default:
          assert (0);
        }
    }

  List_reset (work_list);
  work_list = NULL;

  printf("Done\n");
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

  IPA_HTAB_START(eiter, edge_list->in);
  IPA_HTAB_LOOP(eiter)
    {
      // edge     = dst_node :=  src_node
      // tmp_edge = src_node := &tmp_node
      // new_edge = dst_node := &tmp_node

      tmp_edge = IPA_HTAB_CUR(eiter);
      tmp_node = tmp_edge->src_elist->node;
#if 0
      if (print)
	printf ("A/AA %d --> %d\n",
		tmp_node->data.consg.var_id, 
		dst_node->data.consg.var_id);
#endif
      ensure_edge (ASSIGN_ADDR, tmp_node, dst_node, tmp_edge, edge);
    }
}

/*****************************************************************************
 * Process AssignAddr edge.
 *****************************************************************************/

static void
process_assign_addr (IPA_cgraph_edge_t * edge)
{
  // edge = dst_node := & src_node

  IPA_cgraph_node_t *src_node, *dst_node, *tmp_node;
  IPA_cgraph_edge_list_t *edge_list;
  IPA_cgraph_edge_t *tmp_edge;
  IPA_HTAB_ITER eiter;

  dst_node = edge->dst_elist->node;
  src_node = edge->src_elist->node;

  edge_list = IPA_cg_edge_list_find (dst_node, ASSIGN);
  if (edge_list)
    {
      IPA_HTAB_START(eiter, edge_list->out);
      IPA_HTAB_LOOP(eiter)
	{
	  // edge     = dst_node := &src_node
	  // tmp_edge = tmp_node :=  dst_node
	  // new_edge = tmp_node := &src_node
	  
	  tmp_edge = IPA_HTAB_CUR(eiter);
	  tmp_node = tmp_edge->dst_elist->node;
#if 0
	  if (print)
	    printf ("AA/A %d --> %d\n",
		    src_node->data.consg.var_id, 
		    tmp_node->data.consg.var_id);
#endif
	  ensure_edge (ASSIGN_ADDR, src_node, tmp_node, edge, tmp_edge);
	}
    }

  edge_list = IPA_cg_edge_list_find (dst_node, ASSIGN_DEREF);
  if (edge_list)
    {
      IPA_HTAB_START(eiter, edge_list->out);
      IPA_HTAB_LOOP(eiter)
	{
	  // edge     = dst_node := &src_node
	  // tmp_edge = tmp_node := *dst_node
	  // new_edge = tmp_node :=  src_node
	  
	  tmp_edge = IPA_HTAB_CUR(eiter);
	  tmp_node = tmp_edge->dst_elist->node;
#if 0
	  if (print)
	    printf ("AA/AD %d --> %d\n",
		    src_node->data.consg.var_id, 
		    tmp_node->data.consg.var_id);
#endif
	  ensure_edge (ASSIGN, src_node, tmp_node, edge, tmp_edge);
	}
    }

  edge_list = IPA_cg_edge_list_find (dst_node, DEREF_ASSIGN);
  if (edge_list)
    {
      IPA_HTAB_START(eiter, edge_list->in);
      IPA_HTAB_LOOP(eiter)
	{
	  // edge     =  dst_node := &src_node
	  // tmp_edge = *dst_node :=  tmp_node
	  // new_edge =  src_node :=  tmp_node
	  
	  tmp_edge = IPA_HTAB_CUR(eiter);
	  tmp_node = tmp_edge->src_elist->node;
#if 0
	  if (print)
	    printf ("AA/DA %d --> %d\n",
		    tmp_node->data.consg.var_id, 
		    src_node->data.consg.var_id);
#endif
	  ensure_edge (ASSIGN, tmp_node, src_node, edge, tmp_edge);
	}
    }
}

/*****************************************************************************
 * Process AssignDeref edge.
 *****************************************************************************/

static void
process_assign_deref (IPA_cgraph_edge_t * edge)
{
  // dst := *src src := &tmp
  // -----------------------
  // dst := tmp

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
      // dst := *src src := &tmp
      // -----------------------
      // dst := tmp

      tmp_edge = IPA_HTAB_CUR(eiter);
      tmp_node = tmp_edge->src_elist->node;
#if 0
      if (print)
	printf ("AD/AA %d --> %d\n",
		tmp_node->data.consg.var_id, 
		dst_node->data.consg.var_id);
#endif
      ensure_edge (ASSIGN, tmp_node, dst_node, tmp_edge, edge);
    }
}

/*****************************************************************************
 * Process DerefAssign edge.
 *****************************************************************************/

static void
process_deref_assign (IPA_cgraph_edge_t * edge)
{
  // *dst := src dst := &tmp
  // -----------------------
  //  tmp := src 

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
      // *dst := src dst := &tmp
      // -----------------------
      //  tmp := src

      tmp_edge = IPA_HTAB_CUR(eiter);
      tmp_node = tmp_edge->src_elist->node;
#if 0
      if (print)
	printf ("DA/AA %d --> %d\n",
		src_node->data.consg.var_id, 
		tmp_node->data.consg.var_id);
#endif
      ensure_edge (ASSIGN, src_node, tmp_node, tmp_edge, edge);
    }
}

/*****************************************************************************
 * Misc functions
 *****************************************************************************/

static void
ensure_edge (IPA_cgraph_edgelist_e edge_type,
	     IPA_cgraph_node_t * src_node, 
	     IPA_cgraph_node_t * dst_node,
	     IPA_cgraph_edge_t * src_edge,
	     IPA_cgraph_edge_t * dst_edge)
{
  IPA_cgraph_edge_t *edge;
  int flag;

#if 0
  {
      printf("EDGE1 [%p] %s: ",src_edge, IPA_cg_edge_flag_name(src_edge));
      IPA_cg_node_print(stdout, src_edge->dst_elist->node, IPA_PRINT_ASCI);
      printf(" <- ");
      IPA_cg_node_print(stdout, src_edge->src_elist->node, IPA_PRINT_ASCI);
      printf("\n");
      printf("EDGE2 [%p] %s: ",dst_edge, IPA_cg_edge_flag_name(dst_edge));
      IPA_cg_node_print(stdout, dst_edge->dst_elist->node, IPA_PRINT_ASCI);
      printf(" <- ");
      IPA_cg_node_print(stdout, dst_edge->src_elist->node, IPA_PRINT_ASCI);
      printf("\n");
    }
#endif

  flag = IPA_consg_calc_edge_origin (src_edge, dst_edge);
  if (flag == 0)
    return;

  edge = IPA_consg_ensure_edge (edge_type, src_node, dst_node,
                                0, IPA_POINTER_SIZE, 0,
                                flag);

  if (edge && CG_EDGE_ISNEW(edge))
    {
      work_list = List_insert_last (work_list, edge);
    }
}


#endif

