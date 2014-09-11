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
 *      File:    pipa_graph.h
 *      Author:  Erik Nystrom
 *      Copyright (c) 2003  Erik Nystrom, Wen-mei Hwu
 *              and The Board of Trustees of the University of Illinois.
 *              All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#define DAVINCI_SUPPORT 0

#include "pipa_graph.h"
#include "pipa_print_graph.h"

void
check_edge(IPA_cgraph_edge_t *edge);

void
IPA_consg_apply_edge_flags(IPA_cgraph_edge_t *edge,
			   int new_flags);

static L_Alloc_Pool *IPA_cgraph_edge_pool = NULL;
static L_Alloc_Pool *IPA_cgraph_node_pool = NULL;
static L_Alloc_Pool *IPA_cgraph_edgelist_pool = NULL;
static L_Alloc_Pool *IPA_cgraph_pool = NULL;
static L_Alloc_Pool *IPA_cgraph_link_pool = NULL;

char *edge_types[] = 
  { "ZERO", 
    "ASSIGN", "ASSIGN_ADDR", "DEREF_ASSIGN", 
    "ASSIGN_DEREF", "SKEW",
    "MAX_EDGELIST_TYPE"
  };


/*************************************************************************
 * Pool Routines
 *************************************************************************/

void
IPA_cg_edgepool_print_info(FILE *file)
{
  L_print_alloc_info (file, IPA_cgraph_edge_pool, 0);
}

void
IPA_cg_nodepool_print_info(FILE *file)
{
  L_print_alloc_info (file, IPA_cgraph_node_pool, 0);
}


/*************************************************************************
 * EDGE DATA OPERATIONS
 *************************************************************************/

static int
IPA_cg_edgedata_same (IPA_cgraph_edgelist_e edge_type,
                      IPA_cgraph_edge_data_t * data1,
                      IPA_cgraph_edge_data_t * data2)
{
  assert(data1 && data2);

  switch(edge_type)
    {
    case ASSIGN:
    case ASSIGN_ADDR:
    case DEREF_ASSIGN:
    case ASSIGN_DEREF:
      if (data1->source_offset != data2->source_offset ||
	  data1->target_offset != data2->target_offset ||
	  data1->source_stride != data2->source_stride ||
	  data1->target_stride != data2->target_stride)
	return 0;
      break;
    case SKEW:
      if (data1->source_offset != data2->source_offset ||
	  data1->target_offset != data2->target_offset ||
	  data1->source_stride != data2->source_stride ||
	  data1->target_stride != data2->target_stride ||
	  data1->assign_size != data2->assign_size)
	return 0;
      break;
    default:
      assert(0);
    }

  return 1;
}

static void
IPA_cg_edgedata_assign (IPA_cgraph_edgelist_e edge_type,
                        IPA_cgraph_edge_data_t * data_d,
                        IPA_cgraph_edge_data_t * data_s)
{
  *data_d = *data_s;
}

static int
IPA_cg_edgedata_key (IPA_cgraph_edgelist_e edge_type,
		     IPA_cgraph_edge_data_t * data)
{
  assert(data);

  switch(edge_type)
    {
    case ASSIGN:
    case ASSIGN_ADDR:
    case DEREF_ASSIGN:
    case ASSIGN_DEREF:
      return (/*(data->assign_size) ^ */
	      (data->target_offset << 12) ^
	      (data->source_offset << 20));
    case SKEW:
      return ((data->assign_size) ^ 
	      (data->target_offset << 12) ^
	      (data->source_offset << 20));
    default:
      assert(0);
    }

  assert(0);
  return 0;
}


/*************************************************************************
 * EDGE
 *************************************************************************/

IPA_cgraph_edge_t *
IPA_cg_edge_new ()
{
  IPA_cgraph_edge_t *edge;

  edge = (IPA_cgraph_edge_t *) L_alloc (IPA_cgraph_edge_pool);
  bzero (edge, sizeof (IPA_cgraph_edge_t));

  return edge;
}


void
IPA_cg_edge_free (IPA_cgraph_edge_t * edge)
{
  if (!edge)
    return;

  L_free (IPA_cgraph_edge_pool, edge);
}

static void
IPA_cg_edge_error_check (IPA_cgraph_node_t * src_node,
                         IPA_cgraph_node_t * dst_node,
                         IPA_cgraph_edgelist_e edge_type)
{
  char fail = 0;

  switch (edge_type)
    {
    case DEREF_ASSIGN:
    case ASSIGN_DEREF:
    case ASSIGN_ADDR:
    case ASSIGN:
    case SKEW:
      if (src_node->rep_parent != src_node)
	{
	  IPA_cgraph_edge_list_t *elist;
	  for (elist=src_node->first_list; elist; elist=elist->nxt_list)
	    {
	      if (elist->in && IPA_htab_size(elist->in) != 0)
		fail = 1;
	      if (elist->out && IPA_htab_size(elist->out) != 0)
		fail = 1;
	    }
	}
      if (fail)
	{
	  IPA_cg_node_print(stdout, src_node, IPA_PRINT_ASCI);
	  assert(0);
	}
      if (dst_node->rep_parent != dst_node)
	{
	  IPA_cgraph_edge_list_t *elist;
	  for (elist=dst_node->first_list; elist; elist=elist->nxt_list)
	    {
	      if (elist->in && IPA_htab_size(elist->in) != 0)
		fail = 1;
	      if (elist->out && IPA_htab_size(elist->out) != 0)
		fail = 1;
	    }
	}
      if (fail)
	{
	  IPA_cg_node_print(stdout, dst_node, IPA_PRINT_ASCI);
	  assert(0);
	}
      break;
    default:
      assert(0);
      break;
    }
}


static IPA_cgraph_edge_list_t * cmp_src_elist;
static IPA_cgraph_edge_list_t * cmp_dst_elist;
static IPA_cgraph_edgelist_e    cmp_edge_type;
static IPA_cgraph_edge_data_t * cmp_edata;

static int ecprint = 0;

static int
IPA_cg_edge_compare(void * edge)
{
#if 0
  if (ecprint)
    printf("edgecompare %p\n",edge);
#endif
  if ( (((IPA_cgraph_edge_t*)edge)->src_elist == cmp_src_elist) &&
       (((IPA_cgraph_edge_t*)edge)->dst_elist == cmp_dst_elist) &&
       IPA_cg_edgedata_same (cmp_edge_type, 
			     &((IPA_cgraph_edge_t*)edge)->data, cmp_edata))
    return 1;
  return 0;
}


#define GETKEY(se, de, et, ed)  ((((int)(long)se) ^ ((int)(long)de)) ^ \
                                 (IPA_cg_edgedata_key(et, ed)))


static IPA_cgraph_edge_t *
IPA_cg_edge_find_el (IPA_cgraph_edge_list_t * src_elist,
		     IPA_cgraph_edge_list_t * dst_elist,
		     IPA_cgraph_edgelist_e edge_type,
		     IPA_cgraph_edge_data_t * edata)
{
  int key;
  void *item;
  IPA_cgraph_edge_t *edge;

  key = GETKEY(src_elist, dst_elist, edge_type, edata);
  cmp_src_elist = src_elist;
  cmp_dst_elist = dst_elist;
  cmp_edge_type = edge_type;
  cmp_edata = edata;
  item = IPA_htab_find(dst_elist->in, key, IPA_cg_edge_compare);

  edge = (IPA_cgraph_edge_t *)(item);
  if (edge)
    CG_EDGE_CLRNEW(edge);
  
  return edge;
}


IPA_cgraph_edge_t *
IPA_cg_edge_find (IPA_cgraph_node_t * src_node,
                  IPA_cgraph_node_t * dst_node,
                  IPA_cgraph_edgelist_e edge_type,
                  IPA_cgraph_edge_data_t * edata)
{
  IPA_cgraph_edge_t *edge;
  IPA_cgraph_edge_list_t *src_elist;
  IPA_cgraph_edge_list_t *dst_elist;

  src_elist = IPA_cg_edge_list_find (src_node, edge_type);
  dst_elist = IPA_cg_edge_list_find (dst_node, edge_type);
  if (!src_elist || !dst_elist)
    {
      return NULL;
    }

  edge = IPA_cg_edge_find_el(src_elist, dst_elist, edge_type, edata);
  if (!edge)
    {
      int so = edata->source_offset;
      int ss = edata->source_stride;
      edata->source_offset = 0;
      edata->source_stride = 1;
      edge = IPA_cg_edge_find_el(src_elist, dst_elist, edge_type, edata);
      edata->source_offset = so;
      edata->source_stride = 0;
    }
  
  return edge;
}


static IPA_cgraph_edge_t *
IPA_cg_edge_add_el (IPA_cgraph_edge_list_t * src_elist,
                    IPA_cgraph_edge_list_t * dst_elist,
                    IPA_cgraph_edgelist_e edge_type,
                    IPA_cgraph_edge_data_t * edata)
{
  IPA_cgraph_edge_t *new_edge;
  int key;

  new_edge = IPA_cg_edge_new ();
  new_edge->src_elist = src_elist;
  new_edge->dst_elist = dst_elist;
  IPA_cg_edgedata_assign (edge_type, &new_edge->data, edata);

  key = GETKEY(src_elist, dst_elist, edge_type, edata);

  if (!src_elist->out)
    src_elist->out = IPA_htab_new(0);
  if (!dst_elist->in)
    dst_elist->in = IPA_htab_new(0);

  /* CSENTRY */ 
  if ((edge_type != DEREF_ASSIGN && edge_type != ASSIGN_DEREF &&
       edata->source_stride != 0) ||
      (edge_type == ASSIGN && edata->assign_size > IPA_POINTER_SIZE))
    src_elist->out = IPA_htab_insert_cs(src_elist->out, new_edge, key, IPA_HTAB_CS_MULTI);
  else if (edge_type == ASSIGN_ADDR || edge_type == SKEW ||  edge_type == ASSIGN)
    src_elist->out = IPA_htab_insert_cs(src_elist->out, new_edge, key, edata->source_offset);
  else
    src_elist->out = IPA_htab_insert(src_elist->out, new_edge, key);
    
  if ((edge_type != DEREF_ASSIGN && edge_type != ASSIGN_DEREF &&
       edata->target_stride != 0) ||
      (edge_type == ASSIGN && edata->assign_size > IPA_POINTER_SIZE))
    dst_elist->in = IPA_htab_insert_cs(dst_elist->in, new_edge, key, IPA_HTAB_CS_MULTI);
  else if (edge_type == ASSIGN_ADDR || edge_type == SKEW ||  edge_type == ASSIGN)
    dst_elist->in = IPA_htab_insert_cs(dst_elist->in, new_edge, key, edata->target_offset);
  else
    dst_elist->in = IPA_htab_insert(dst_elist->in, new_edge, key);
    
#if 0
  if (edge_type == ASSIGN_ADDR ||
      edge_type == SKEW)
    {
      src_elist->out = IPA_htab_insert_cs(src_elist->out, new_edge, key, edata->source_offset);
      dst_elist->in = IPA_htab_insert_cs(dst_elist->in, new_edge, key, edata->target_offset);
    }
  else if (edge_type == ASSIGN)
    {
      assert(edata->assign_size > 0);
      if (edata->assign_size == IPA_POINTER_SIZE)
	{
	  src_elist->out = IPA_htab_insert_cs(src_elist->out, new_edge, key, edata->source_offset);
	  dst_elist->in = IPA_htab_insert_cs(dst_elist->in, new_edge, key, edata->target_offset);
	}
      else
	{
	  src_elist->out = IPA_htab_insert_cs(src_elist->out, new_edge, key, IPA_HTAB_CS_MULTI);
	  dst_elist->in = IPA_htab_insert_cs(dst_elist->in, new_edge, key, IPA_HTAB_CS_MULTI);
	}
    }
  else
    {
      assert(edata->assign_size > 0);
      src_elist->out = IPA_htab_insert(src_elist->out, new_edge, key);
      dst_elist->in = IPA_htab_insert(dst_elist->in, new_edge, key);
    }
#endif

  CG_EDGE_SETNEW(new_edge);

  return new_edge;
}

static void
print_edge(IPA_cgraph_edge_t *e)
{
  IPA_cg_node_print(stdout,e->dst_elist->node, IPA_PRINT_ASCI);
  printf(" <- %14s %x %d,%d,%d ",
	 edge_types[e->dst_elist->edge_type],
	 e->flags,
	 e->data.target_offset, 
	 e->data.assign_size,
	 e->data.source_offset);
  if (IPA_FLAG_ISSET(e->flags,IPA_CG_EDGE_FLAGS_UP))
    printf("UP ");
  if (IPA_FLAG_ISSET(e->flags,IPA_CG_EDGE_FLAGS_DN))
    printf("DN ");
  if (IPA_FLAG_ISSET(e->flags,IPA_CG_EDGE_FLAGS_GBL))
    printf("GBL ");
  IPA_cg_node_print(stdout,e->src_elist->node, IPA_PRINT_ASCI);
  printf("\n");
}

IPA_cgraph_edge_t *
IPA_cg_edge_add (IPA_cgraph_node_t * src_node,
                 IPA_cgraph_node_t * dst_node,
                 IPA_cgraph_edgelist_e edge_type,
                 IPA_cgraph_edge_data_t * edata)
{
  IPA_cgraph_edge_t *new_edge;
  IPA_cgraph_edge_list_t *src_elist;
  IPA_cgraph_edge_list_t *dst_elist;

  /* Do the work */
  if (!(src_elist = IPA_cg_edge_list_find(src_node, edge_type)))
    src_elist = IPA_cg_edge_list_add (src_node, edge_type);
  if (!(dst_elist = IPA_cg_edge_list_find (dst_node, edge_type)))
    dst_elist = IPA_cg_edge_list_add (dst_node, edge_type);


  /* Avoid duplicate edges */
#if 1
  if (!(new_edge = IPA_cg_edge_find_el (src_elist, dst_elist, edge_type, edata)) &&
      edata->source_stride == 0)
    {
      int so = edata->source_offset;
      edata->source_offset = 0;
      edata->source_stride = 1;
      new_edge = IPA_cg_edge_find_el(src_elist, dst_elist, edge_type, edata);
      edata->source_offset = so;
      edata->source_stride = 0;
    }
#else
  new_edge = IPA_cg_edge_find_el (src_elist, dst_elist, edge_type, edata);
#endif

  if (!new_edge)
    {
      /* Error Check */
      DEBUG(IPA_cg_edge_error_check (src_node, dst_node, edge_type););
      assert(src_node->rep_parent == src_node);
      assert(dst_node->rep_parent == dst_node);

      new_edge = IPA_cg_edge_add_el (src_elist, dst_elist, edge_type, edata);

#if 0
        {
	  static char cnt1 = 0;
	  static char cnt2 = 0;
	  printf("New edge %p :  ",new_edge);
	  print_edge(new_edge);
	}
#endif
    }

  return new_edge;
}


static void
IPA_cg_edge_rem (IPA_cgraph_edge_t * edge, char flag)
{
  int key;
  IPA_cgraph_edge_t *rm_edge;
#if 0
  static int cnt = 0;
  cnt++;
  if (cnt > 49995)
    {
      printf("Deleting: ");
      print_edge(edge);
      printf("\n");
    }
  if (cnt == 50000)
    {
      printf("DELETE 50k\n");
      cnt = 0;
    }
#endif

  assert(edge);
  assert(IPA_FLAG_ISCLR(edge->flags, IPA_CG_EDGE_FLAGS_GENERIC2));
  
#if 0
  printf("REM %p \n",edge);
#endif

  key = GETKEY(edge->src_elist, edge->dst_elist, 
	       edge->src_elist->edge_type, &edge->data);
  cmp_src_elist = edge->src_elist;
  cmp_dst_elist = edge->dst_elist;
  cmp_edge_type = edge->src_elist->edge_type;
  cmp_edata = &edge->data;

  /* Remove from graph htabs */
  if (flag & 0x1)
    assert((rm_edge = IPA_htab_remove(edge->src_elist->out, key, IPA_cg_edge_compare)) == edge);
  if (flag & 0x2)
    assert((rm_edge = IPA_htab_remove(edge->dst_elist->in, key, IPA_cg_edge_compare)) == edge);

  edge->src_elist = NULL;
  edge->dst_elist = NULL;
}

void
IPA_cg_edge_delete (IPA_cgraph_edge_t * edge)
{
  IPA_cg_edge_rem (edge, 0x3);
  IPA_cg_edge_free (edge);
}


/*************************************************************************
 * EDGE CHANGE TRACKING
 *************************************************************************/
static List  etrack_list = NULL;
static int   etrack_id = 1;

static edgetrack_t *
IPA_cg_edgetrack_find(int id)
{
  edgetrack_t *et;

  List_start(etrack_list);
  while ((et = List_next(etrack_list)))
    {
      if (et->id == id)
	return et;
    }
  return NULL;
}

void
IPA_cg_edgetrack_end (int id)
{
  edgetrack_t *et;
#if 0
  printf("------------------- RESET %d\n",id);
#endif
  et = IPA_cg_edgetrack_find(id);
  if (et)
    {
      etrack_list = List_delete_current(etrack_list);
      free(et);
      if (List_size(etrack_list) == 0)
	{
	  List_reset(etrack_list);
	  etrack_list = NULL;
	}
    }
}


int
IPA_cg_edgetrack_start (IPA_edgetrack_e mode)
{
  edgetrack_t *et;
#if 0
  printf("------------------- START\n");
#endif
  et = calloc(1,sizeof(edgetrack_t));
  et->id = etrack_id++;
  et->mode = mode;
  et->delta = IPA_htab_new(3);
  etrack_list = List_insert_first(etrack_list, et);

  return et->id;
}


static void
IPA_cg_hash_check(IPA_Hashtab_t *htab)
{
  IPA_cgraph_edge_t   *edge;
  IPA_HTAB_ITER iter;

  IPA_HTAB_START(iter, htab);
  IPA_HTAB_LOOP(iter)
    {
      edge = IPA_HTAB_CUR(iter);
      assert(edge->src_elist);
      assert(edge->dst_elist);
    }
}


void 
IPA_cg_edgetrack_newedge (IPA_cgraph_edge_t * edge)
{
  IPA_cgraph_edgelist_e edge_type;
  edgetrack_t *et;
  int key;

  if (etrack_list == NULL)
    return;

  edge_type = edge->src_elist->edge_type;

  List_start(etrack_list);
  while ((et = List_next(etrack_list)))
    {
      switch (et->mode)
	{
	case IPA_ET_ALL:
	  break;
	case IPA_ET_CALLEE:
	  if (edge_type != ASSIGN_ADDR)
	    continue;
	  {
	    IPA_cgraph_node_t *node;
	    int found;
	    found = 0;
	    node = IPA_cg_node_get_rep(edge->dst_elist->node);
	    for (; node; node = node->rep_child)
	      {
		if (IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_CALLEE))
		  {
		    found = 1;
		    break;
		  }
	      }
	    if (!found)
	      continue;
	    found = 0;
	    node = IPA_cg_node_get_rep(edge->src_elist->node);
	    for (; node; node = node->rep_child)
	      {
		if (IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_FUNC))
		  {
		    found = 1;
		    break;
		  }
	      }
	    if (!found)
	      continue;
	  }
	  break;
	default:
	  I_punt ("IPA_cg_edgetrack_add: unknown mode\n");
	}
      
#if 0
      printf("edgetrack %d add edge %p\n",et->id,edge);
#endif

      /* These edges are guaranteed to be unique */
      key = GETKEY(edge->src_elist, edge->dst_elist, 
		   edge->src_elist->edge_type, &edge->data);
      
#if 0
      printf("EDGETRACK [%p]: ",edge);
      IPA_cg_node_print(stdout,edge->dst_elist->node, IPA_PRINT_ASCI);
      printf(" <- ");
      IPA_cg_node_print(stdout,edge->src_elist->node, IPA_PRINT_ASCI);
      printf("\n");
#endif
      
      et->delta =  IPA_htab_insert(et->delta, edge, key);
    }
}

void 
IPA_cg_edgetrack_remedge (IPA_cgraph_edge_t * edge, int key) 
{
  edgetrack_t *et;

  if (etrack_list == NULL)
    return; 

  List_start(etrack_list);
  while ((et = List_next(etrack_list)))
    {
      IPA_cgraph_edge_t *rm_edge;

#if 0
      {
	IPA_HTAB_ITER iter;
	printf("ET-REM [%d] : \n",
	       IPA_htab_size(et->delta));
	IPA_HTAB_START(iter, et->delta);
	IPA_HTAB_LOOP(iter)
	  {
	    IPA_cgraph_edge_t *edge = IPA_HTAB_CUR(iter);
	    printf("%p %d %d %d\n",
		   edge,
		   edge->data.source_offset,
		   edge->data.assign_size,
		   edge->data.target_offset);
	  }
	ecprint = 1;
      }
#endif

      if ((rm_edge = IPA_htab_remove(et->delta, key, IPA_cg_edge_compare)))
	{
	  assert(rm_edge == edge);
#if 0
	  printf("ET-REM : edge removed\n");
#endif
	}
      DEBUG(IPA_cg_hash_check(et->delta););
    }
  ecprint = 0;
}


IPA_Hashtab_t *
IPA_cg_edgetrack_delta (int id)
{
  edgetrack_t *et;
#if 0
  printf("------------------- GET %d\n",id);
#endif
  et = IPA_cg_edgetrack_find(id);
  assert(et);

  return et->delta;
}


/*************************************************************************
 * EDGE LIST
 *************************************************************************/

IPA_cgraph_edge_list_t *
IPA_cg_edge_list_new (IPA_cgraph_node_t * node,
                      IPA_cgraph_edgelist_e edge_type)
{
  IPA_cgraph_edge_list_t *elist;

  elist = (IPA_cgraph_edge_list_t *) L_alloc (IPA_cgraph_edgelist_pool);
  bzero (elist, sizeof (IPA_cgraph_edge_list_t));

  assert(node);
  elist->edge_type = edge_type;
  elist->node = node;

  elist->nxt_list = node->first_list;
  node->first_list = elist;

  return elist;
}


void
IPA_cg_edge_list_free (IPA_cgraph_edge_list_t * elist)
{
  if (!elist)
    return;

  L_free (IPA_cgraph_edgelist_pool, elist);
}


void
IPA_cg_edge_list_delete (IPA_cgraph_edge_list_t * elist)
{
  IPA_cgraph_edge_t *edge;
  IPA_HTAB_ITER      iter;

  if (!elist)
    return;

  /* OUT */
  IPA_HTAB_START(iter, elist->out);
  IPA_HTAB_LOOP(iter)
    {
      edge = (IPA_cgraph_edge_t *)IPA_HTAB_CUR(iter);
      IPA_cg_edge_rem (edge, 0x2);
      IPA_cg_edge_free (edge);      
    }
  IPA_htab_free(elist->out);
  elist->out = NULL;
  
  /* IN */
  IPA_HTAB_START(iter, elist->in);
  IPA_HTAB_LOOP(iter) 
    {
      edge = (IPA_cgraph_edge_t *)IPA_HTAB_CUR(iter);
      IPA_cg_edge_rem (edge, 0x1);
      IPA_cg_edge_free (edge);      
    }
  IPA_htab_free(elist->in);
  elist->in = NULL;

  IPA_cg_edge_list_free (elist);
}

IPA_cgraph_edge_list_t *
IPA_cg_edge_list_find (IPA_cgraph_node_t * node,
		       IPA_cgraph_edgelist_e edge_type)
{
  IPA_cgraph_edge_list_t *elist;

  for (elist = node->first_list; elist; elist = elist->nxt_list)
    {
      if (elist->edge_type != edge_type)
        continue;
      return elist;
    }

  return NULL;
}

IPA_cgraph_edge_list_t *
IPA_cg_edge_list_add (IPA_cgraph_node_t * node,
		      IPA_cgraph_edgelist_e edge_type)
{
  IPA_cgraph_edge_list_t *elist;

  elist = IPA_cg_edge_list_new (node, edge_type);

  return elist;
}

char *
IPA_cg_edge_flag_name(IPA_cgraph_edge_t *edge)
{
  static char name[1024];
  
  name[0] = 0;
  if (IPA_FLAG_ISSET(edge->flags, IPA_CG_EDGE_FLAGS_EXPLICIT))
    strcat(name, "EXP ");
  else if (IPA_FLAG_ISSET(edge->flags, IPA_CG_EDGE_FLAGS_IMPLICIT))
    strcat(name, "IMP ");
  else
    strcat(name, "??? ");
  
  if (IPA_FLAG_ISSET(edge->flags, IPA_CG_EDGE_FLAGS_GBL))
    strcat(name, "GBL ");
  if (IPA_FLAG_ISSET(edge->flags, IPA_CG_EDGE_FLAGS_HZ))
    strcat(name, "HZ ");
  if (IPA_FLAG_ISSET(edge->flags, IPA_CG_EDGE_FLAGS_DN))
    strcat(name, "DN ");
  if (IPA_FLAG_ISSET(edge->flags, IPA_CG_EDGE_FLAGS_UP))
    strcat(name, "UP ");

  return name;
}

int
IPA_edge_list_valid (IPA_cgraph_edge_list_t * elist, int valid_edges)
{
  switch (elist->edge_type)
    {
    case ASSIGN_ADDR:
      if (!(valid_edges & IPA_CG_ETYPE_ASSIGN_ADDR))
        return 0;
      break;
    case ASSIGN:
      if (!(valid_edges & IPA_CG_ETYPE_ASSIGN))
        return 0;
      break;
    case DEREF_ASSIGN:
      if (!(valid_edges & IPA_CG_ETYPE_DEREF_ASSIGN))
        return 0;
      break;
    case ASSIGN_DEREF:
      if (!(valid_edges & IPA_CG_ETYPE_ASSIGN_DEREF))
        return 0;
      break;
    case SKEW:
      if (!(valid_edges & IPA_CG_ETYPE_SKEW))
        return 0;
      break;
    default:
      I_punt ("IPA_edge_list_valid: unsupported edge type\n");
    }

  return 1;
}


/*************************************************************************
 * NODE SPECIFIC 
 *************************************************************************/

static int
IPA_cg_nodedata_hash (IPA_cgraph_node_data_t * data1)
{
  int hash;
  hash = ((data1->version << 16) ^ 
	  (data1->var_id));
#if 0
  printf("Hash: %d %d %d = %d\n", 
	 data1->var_id, 
	 data1->version,
	 hash);
#endif
  return hash;
}

static int
IPA_cg_nodedata_same (IPA_cgraph_node_data_t * data1,
                      IPA_cgraph_node_data_t * data2)
{
#if 0
  printf("Same: %d %d %d = %d %d %d\n", 
	 data1->var_id, 
	 data1->version,
	 data2->var_id, 
	 data2->version);
#endif
  if (data1->var_id != data2->var_id ||
      data1->version != data2->version)
    return 0;

  return 1;
}

static void
IPA_cg_nodedata_assign (IPA_cgraph_node_data_t * data_d,
                        IPA_cgraph_node_data_t * data_s)
{
  *data_d = *data_s;
}


/*************************************************************************
 * NODE
 *************************************************************************/

static IPA_cgraph_node_data_t * cmp_ndata;

static int
IPA_cg_node_compare(void *node)
{
  if (IPA_cg_nodedata_same (&((IPA_cgraph_node_t*)node)->data, 
			       cmp_ndata))
    return 1;
  return 0;
}

IPA_cgraph_node_t *
IPA_cg_node_find (IPA_cgraph_t * cgraph,
		  IPA_cgraph_node_data_t * ndata)
{
  IPA_cgraph_node_t *node;
  void *item;
  int key;
  
  assert(cgraph);

  key = IPA_cg_nodedata_hash(ndata);
  cmp_ndata = ndata;
  item = IPA_htab_find(cgraph->nodes, key, IPA_cg_node_compare);

  node = (IPA_cgraph_node_t *)(item);
  if (node)
    CG_NODE_CLRNEW(node);
 
  return node;
}

IPA_cgraph_node_t *
IPA_cg_node_new (IPA_cgraph_t * cgraph,
                 IPA_cgraph_node_data_t * ndata)
{
  IPA_cgraph_node_t *node;
  int key;

  assert(cgraph);
  assert(ndata);

  node = (IPA_cgraph_node_t *) L_alloc (IPA_cgraph_node_pool);
  bzero (node, sizeof (IPA_cgraph_node_t));

  node->cgraph = cgraph;
  node->rep_parent = node;
  IPA_cg_nodedata_assign (&node->data, ndata);

  key = IPA_cg_nodedata_hash(ndata);
  cgraph->nodes = IPA_htab_insert(cgraph->nodes, node, key);

#if 0
  printf("newnode: %p \n",node);
#endif

  CG_NODE_SETNEW(node);
  
  return node;
}

IPA_cgraph_node_t *
IPA_cg_node_add (IPA_cgraph_t * cgraph,
                 IPA_cgraph_node_data_t * ndata)
{
  IPA_cgraph_node_t *node;

  if (!(node = IPA_cg_node_find (cgraph, ndata)))
    {
      node = IPA_cg_node_new (cgraph, ndata);
    }

  return node;
}

void
IPA_cg_node_reset (IPA_cgraph_node_t * node)
{
  IPA_cgraph_edge_list_t *elist;
  IPA_cgraph_edge_list_t *elist_nxt;

  if (!node)
    return;

  for (elist = node->first_list; elist; elist = elist_nxt)
    {
      elist_nxt = elist->nxt_list;
      IPA_cg_edge_list_delete (elist);
    }

  node->first_list = NULL;
}


void
IPA_cg_node_delete (IPA_cgraph_node_t * node)
{
  int key;

  if (!node)
    return;

#if 0
  printf("freenode: %p ",
	 node);
#endif

#if UNIFY
  /* Allow deletion of a non-single node */
    {
      int count1, count2, found;
      IPA_cgraph_node_t *parent;
      {
	IPA_cgraph_node_t *cur;
	count1 = 0;
	found = 0;
	parent = IPA_cg_node_get_rep(node);
	for (cur = parent; cur; cur = cur->rep_child)
	  {
	    count1++;
	    if (cur == node)
	      found = 1;
	  }
	assert(found);
      }
      if (node->rep_parent == node)
	{
	  /* This is the head of the chain */
	  IPA_cgraph_node_t *new_par = node->rep_child;
	  IPA_cgraph_node_t *cur = node;

	  parent = new_par;

	  /* Skip node and then redue the parent ptrs */
	  while ((cur = cur->rep_child))
	    cur->rep_parent = new_par;

	  /* Clear child ptr */
	  node->rep_child = NULL; 
	  if (new_par)
	    {
	      IPA_cgraph_edge_list_t *elist;
	      assert(new_par->first_list == NULL);
	      new_par->first_list = node->first_list;
	      node->first_list = NULL;
	      for (elist=new_par->first_list; elist; elist=elist->nxt_list)
		{
		  elist->node = new_par;
		}
	    }
	}
      else
	{
	  /* This is an inner node */
	  IPA_cgraph_node_t *par;
	  IPA_cgraph_node_t *cur;
	  IPA_cgraph_node_t *prev = NULL;

	  /* For safety, point everynode to real parent 
	   *  Also, find preceeding node
	   */
	  par = IPA_cg_node_get_rep(node);
	  for (cur = par; cur; cur = cur->rep_child)
	    {
	      cur->rep_parent = par;
	      if (cur->rep_child == node)
		prev = cur;
	    }
	  assert(prev);
	  
	  /* Bypass this node */
	  prev->rep_child = node->rep_child;

	  /* Clear parent/child ptr */
	  node->rep_child = NULL; 
	  node->rep_parent = node;
	}

      {
	IPA_cgraph_node_t *cur;
	count2 = 0;
	for (cur = parent; cur; cur = cur->rep_child)
	  count2++;
	assert(count2 == count1 - 1);
#if 0
	printf(" %d %d\n",count1,count2);
#endif
      }
    }
#endif
  assert(node->rep_child == NULL);
  assert(node->rep_parent == node);

  key = IPA_cg_nodedata_hash(&node->data);
  cmp_ndata = &node->data;
  assert(node == IPA_htab_remove(node->cgraph->nodes, key, IPA_cg_node_compare));

  IPA_cg_node_reset (node);
  node->rep_parent = NULL;
  node->cgraph = NULL;
  node->data.var_id = 0;
  node->data.syminfo = NULL;
  L_free (IPA_cgraph_node_pool, node);
}

int
IPA_cg_node_same (IPA_cgraph_node_t * node1, IPA_cgraph_node_t * node2)
{
  if (!IPA_cg_nodedata_same (&node1->data, &node2->data))
    return 0;

  return 1;
}

int
IPA_cg_node_is_child (IPA_cgraph_node_t * node)
{
  assert(node->rep_parent);

  if (node->rep_parent != node)
    return 1;

  return 0;
}

IPA_cgraph_node_t *
IPA_cg_node_get_rep (IPA_cgraph_node_t * node)
{
  if (!node)
    return NULL;

  /* Find and set absolute parent */
  while (node->rep_parent != node->rep_parent->rep_parent)
    {
      node->rep_parent = node->rep_parent->rep_parent;
    }
  assert(node->rep_parent);

  return node->rep_parent;
}


void
IPA_cg_nodes_clr_flags (IPA_cgraph_t * cgraph, unsigned int flags)
{
  IPA_cgraph_node_t   *node;
  IPA_HTAB_ITER niter;

  IPA_HTAB_START(niter, cgraph->nodes);
  IPA_HTAB_LOOP(niter)
    {
      node = (IPA_cgraph_node_t *)IPA_HTAB_CUR(niter);
      IPA_FLAG_CLR (node->flags, flags);
    }
}

void
IPA_cg_nodes_assert_clr_flags (IPA_cgraph_t * cgraph, unsigned int flags)
{
  IPA_cgraph_node_t   *node;
  IPA_HTAB_ITER niter;

  IPA_HTAB_START(niter, cgraph->nodes);
  IPA_HTAB_LOOP(niter)
    {
      node = (IPA_cgraph_node_t *)IPA_HTAB_CUR(niter);
      assert(IPA_FLAG_ISCLR (node->flags, flags));
    }
}

/*************************************************************************
 * GRAPH
 *************************************************************************/

IPA_cgraph_t *
IPA_cg_cgraph_new (struct IPA_funcsymbol_info_t *fninfo)
{
  IPA_cgraph_t *cgraph;

  cgraph = L_alloc (IPA_cgraph_pool);
  bzero (cgraph, sizeof (IPA_cgraph_t));

  cgraph->nodes = IPA_htab_new(9);

  cgraph->data.fninfo = fninfo;
  return cgraph;
}


void
IPA_cg_cgraph_free (IPA_cgraph_t * cgraph)
{
  IPA_cgraph_node_t *node;
  IPA_HTAB_ITER iter;

  if (!cgraph)
    return;

  IPA_HTAB_START(iter, cgraph->nodes);
  IPA_HTAB_LOOP(iter)
    {
      node = (IPA_cgraph_node_t *)IPA_HTAB_CUR(iter);
      IPA_cg_node_delete (node);      
    }
  IPA_htab_free(cgraph->nodes);

  L_free (IPA_cgraph_pool, cgraph);
}

void
IPA_cgraph_minit ()
{
  static char init = 0;

  if (init)
    I_punt ("IPA_cgraph_minit: already called\n");
  init = 1;

  IPA_cgraph_edge_pool =
    L_create_alloc_pool ("cgraph_edge", sizeof (IPA_cgraph_edge_t), 1024);
  IPA_cgraph_node_pool =
    L_create_alloc_pool ("cgraph_node", sizeof (IPA_cgraph_node_t), 128);
  IPA_cgraph_edgelist_pool =
    L_create_alloc_pool ("cgraph_edgelist", 
			 (sizeof (IPA_cgraph_edge_list_t)),
                         32);
  IPA_cgraph_pool = 
    L_create_alloc_pool ("cgraph", sizeof (IPA_cgraph_t), 16);

  IPA_cgraph_link_pool = 
    L_create_alloc_pool ("cgraph_link", sizeof (IPA_cgraph_link_t), 16);
}

void
IPA_cgraph_pool_info()
{
  L_print_alloc_info(stdout,  IPA_cgraph_edge_pool, 1); 
  L_print_alloc_info(stdout,  IPA_cgraph_edgelist_pool, 1); 
  L_print_alloc_info(stdout,  IPA_cgraph_node_pool, 1); 
  L_print_alloc_info(stdout,  IPA_cgraph_pool, 1); 
  L_print_alloc_info(stdout,  IPA_cgraph_link_pool, 1); 
}

void
IPA_cgraph_mfree ()
{
  fflush(stdout);
  fflush(stderr);
  L_free_alloc_pool (IPA_cgraph_edge_pool);
  IPA_cgraph_edge_pool = NULL;

  L_free_alloc_pool (IPA_cgraph_node_pool);
  IPA_cgraph_node_pool = NULL;

  L_free_alloc_pool (IPA_cgraph_edgelist_pool);
  IPA_cgraph_edgelist_pool = NULL;

  L_free_alloc_pool (IPA_cgraph_pool);
  IPA_cgraph_pool = NULL;

  L_free_alloc_pool (IPA_cgraph_link_pool);
  IPA_cgraph_link_pool = NULL;
}


/*************************************************************************
 * SCC Detection
 *************************************************************************/
#define IPA_CG_SCC_WHITE    IPA_CG_NODE_FLAGS_GENERIC1
#define IPA_CG_SCC_ONSTACK  IPA_CG_NODE_FLAGS_GENERIC2

IPA_cgraph_scc_stack_t *
IPA_cg_scc_stack_new ()
{
  IPA_cgraph_scc_stack_t *stack;

  stack =
    (IPA_cgraph_scc_stack_t *) calloc (1, sizeof (IPA_cgraph_scc_stack_t));

  return stack;
}


void
IPA_cg_scc_stack_free (IPA_cgraph_scc_stack_t * stack)
{
  free (stack);
}


void
IPA_cg_search_SCC (IPA_cgraph_t * cgraph, IPA_cgraph_node_t * node,
		   List * stack, int *count, List * scc_lists,
		   int valid_edges)
{
  IPA_cgraph_edge_t *edge;
  IPA_cgraph_edge_list_t *edge_list;
  IPA_cgraph_scc_stack_t *st;
  IPA_HTAB_ITER eiter;

  IPA_FLAG_CLR (node->flags, IPA_CG_SCC_WHITE);

  st = IPA_cg_scc_stack_new ();
  st->def_num = (*count)++;
  st->low_link = st->def_num;
  st->node = node;
  node->misc.st = st;
  (*stack) = List_insert_first ((*stack), st);

  /* For all output edges */
  for (edge_list = node->first_list; edge_list;
       edge_list = edge_list->nxt_list)
    {
      if (!IPA_edge_list_valid (edge_list, valid_edges))
        continue;

      IPA_HTAB_START(eiter, edge_list->out);
      IPA_HTAB_LOOP(eiter)
	{
          IPA_cgraph_node_t *dst_node;

	  edge = (IPA_cgraph_edge_t *)IPA_HTAB_CUR(eiter);
          dst_node = edge->dst_elist->node;

	  if (edge->src_elist->edge_type >= ASSIGN &&
	      (edge->data.source_offset != 0 ||
	       edge->data.target_offset != 0))
	    continue;
	  if (dst_node->cgraph != cgraph)
	    continue;
          if (dst_node == node)
            {
              /* Trivial SCC */
              continue;
            }

          if (IPA_FLAG_ISSET (dst_node->flags, IPA_CG_SCC_WHITE))
            {
              IPA_cg_search_SCC (cgraph, dst_node, stack,
				 count, scc_lists, valid_edges);
              /* dst_node may be removed if it was a part of an SCC */
              if (dst_node->misc.st)
		{
		  st->low_link = Min (st->low_link, dst_node->misc.st->low_link);
		}
            }
          else if (dst_node->misc.st)
            {
              if (dst_node->misc.st->def_num < st->def_num)
                st->low_link = Min (st->low_link, dst_node->misc.st->def_num);
            }
        }
    }

  if (st->low_link == st->def_num)
    {
      List new_list = NULL;
      IPA_cgraph_node_t *scc_node;
      IPA_cgraph_scc_stack_t *scc_st;

      /*printf("Found SCC : "); */
      do
        {
          scc_st = List_first ((*stack));
          scc_node = scc_st->node;

          (*stack) = List_delete_current ((*stack));
	  scc_node->misc.st = NULL;
	  
          new_list = List_insert_last (new_list, scc_node);
          IPA_cg_scc_stack_free (scc_st);

          /*printf("%d ", scc_node->data.var_id);*/
        }
      while (node != scc_node);
      /*printf("\n");*/

      st = NULL;
      (*scc_lists) = List_insert_last ((*scc_lists), new_list);
    }
}


List
IPA_cg_find_SCC (IPA_cgraph_t * cgraph, int valid_edges)
{
  IPA_cgraph_node_t *node;
  List scc_lists = NULL;
  List stack = NULL;
  IPA_HTAB_ITER niter;
  int count;

  /* Init vars */
  count = 1;

  /* Init graph */
  IPA_HTAB_START(niter, cgraph->nodes);
  IPA_HTAB_LOOP(niter)
    {
      node = (IPA_cgraph_node_t *)IPA_HTAB_CUR(niter);
      IPA_FLAG_SET (node->flags, IPA_CG_SCC_WHITE);
      node->misc.st = NULL;
    }

  /* Run SCC detection */
  IPA_HTAB_START(niter, cgraph->nodes);
  IPA_HTAB_LOOP(niter)
    {
      node = (IPA_cgraph_node_t *)IPA_HTAB_CUR(niter);
      
      /* Only search parent nodes */
      if (IPA_cg_node_is_child (node))
	continue;
      
      if (IPA_FLAG_ISSET (node->flags, IPA_CG_SCC_WHITE))
	{
	  IPA_cg_search_SCC (cgraph, node, &stack, &count,
			     &scc_lists, valid_edges);
	}
    }

  /* Free stack */
  assert(List_size(stack) == 0);
  List_reset (stack);

  /* Clear flags */
  IPA_HTAB_START(niter, cgraph->nodes);
  IPA_HTAB_LOOP(niter)
    {
      node = (IPA_cgraph_node_t *)IPA_HTAB_CUR(niter);
      node->misc.st = NULL;
      IPA_FLAG_CLR (node->flags, IPA_CG_SCC_WHITE);
    }

  return scc_lists;
}

void
IPA_cg_free_SCC (IPA_cgraph_t * cgraph, List list)
{
  List scc;

  List_start (list);
  while ((scc = List_next (list)))
    {
      List_reset (scc);
    }
  List_reset (list);
}

void
IPA_cg_print_SCC (FILE * file, List list)
{
  IPA_cgraph_node_t *node;
  List scc;
  int i;

  fprintf (file, "----------------------\n");
  fprintf (file, "### %d SCCs\n", List_size (list));
  i = 0;
  List_start (list);
  while ((scc = List_next (list)))
    {
      fprintf (file, "[%d] ---- %d nodes\n", i++, List_size (scc));
      List_start (scc);
      while ((node = List_next (scc)))
        {
          IPA_cg_node_print (file, node,IPA_PRINT_ASCI);
          fprintf (file, "\n");
        }
    }
  fprintf (file, "----------------------\n");
}

/*************************************************************************
 * Move nodes between graphs
 *************************************************************************/

void
IPA_cg_move_node(IPA_cgraph_t *dst_cg,
		 IPA_cgraph_t *src_cg,
		 IPA_cgraph_node_t *node)
{
  IPA_cgraph_node_t *exist_node;
  IPA_cgraph_node_t *rem_node;
  int key;

#if 0
  printf("CG Moving node %p : ",node);
  IPA_cg_node_print(stdout, node, IPA_PRINT_ASCI);
  printf("\n");
#endif

  /* Remove the node from the src constraint graph */
  key = IPA_cg_nodedata_hash(&node->data);
  cmp_ndata = &node->data;
  rem_node = IPA_htab_remove(src_cg->nodes, key, IPA_cg_node_compare);
  assert(rem_node == node);
  
  if ((exist_node = IPA_cg_node_find (dst_cg, &node->data)))
    {
      /* The node already exists in the destination graph .
	 THIS SHOULD NO LONGER OCCUR */
      assert(0);
    }
  else
    {
      /* Do a fast move into the dst contraint graph */
      node->cgraph = dst_cg;
      dst_cg->nodes = IPA_htab_insert(dst_cg->nodes, node, key);
    }
}

void
IPA_cg_merge_graph(IPA_cgraph_t *dst_cg,
		   IPA_cgraph_t *src_cg)
{
  IPA_cgraph_node_t *node;
  IPA_HTAB_ITER niter;
  List merge_list = NULL;

#if 0
  printf("CG Merging graphs [%s] [%s] \n",
	 dst_cg->data.fninfo->func_name,
	 src_cg->data.fninfo->func_name);
#endif
  
  IPA_HTAB_START(niter, src_cg->nodes);
  IPA_HTAB_LOOP(niter)
    {
      node = IPA_HTAB_CUR(niter);
      merge_list = List_insert_last(merge_list, node);
    }
  assert(List_size(merge_list) == IPA_htab_size(src_cg->nodes));

  List_start(merge_list);
  while ((node = List_next(merge_list)))
    {
      IPA_cg_move_node(dst_cg, src_cg, node);
    }
  List_reset(merge_list);

  if (IPA_htab_size(src_cg->nodes) != 0)
    {
      IPA_htab_print(src_cg->nodes);
      assert(0);
    }
}


/*************************************************************************
 * Merge nodes
 *************************************************************************/

#define MAKE_FI_DELETE IPA_CG_EDGE_FLAGS_GENERIC1

static List
add_to_worklist(List work_list, int in_wl_flag, IPA_cgraph_edge_t *edge)
{
  if (!in_wl_flag)
    return work_list;
  if(IPA_FLAG_ISSET(edge->flags, in_wl_flag))
    return work_list;

  work_list = List_insert_last (work_list, edge);
  IPA_FLAG_SET(edge->flags, in_wl_flag);
  return work_list;
}

static int
mod_offset(int offset, int mod)
{
  if (mod == 1)
    return 0;
  if (offset < mod)
    return offset;
  return offset % mod;
}

List
IPA_cgraph_make_fi(IPA_cgraph_node_t *node, List work_list, int in_wl_flag)
{
  IPA_cgraph_edge_list_t *elist;
  IPA_cgraph_edge_t      *edge;
  IPA_cgraph_edge_t      *new_edge;
  IPA_cgraph_edge_data_t edata;
  IPA_HTAB_ITER eiter;
  int new_to, new_so;
  List del_list = NULL;
  int in_new = 0, out_new = 0;
  int in_all = 0, out_all = 0;
  int mod;

  /* Compute the modulos */
  assert(node->data.mod >= 1);
  mod = node->data.mod;

  if (mod != 1)
    assert(!IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_NOFIELD));
  if (mod == 1)
    IPA_consg_set_nofield(node);      
  
  printf("WILL MERGE id %d.%d \n",
	 node->data.var_id,
	 node->data.version);

  for (elist=node->first_list; elist; elist=elist->nxt_list)
    {
      IPA_htab_blockrehash(elist->in);
      IPA_HTAB_START(eiter, elist->in);
      IPA_HTAB_LOOP(eiter)
	{
	  edge = IPA_HTAB_CUR(eiter);

	  new_to = mod_offset(edge->data.target_offset, mod);
	  new_so = mod_offset(edge->data.source_offset, mod);

	  /* If the target is changing or
	     this is a self edge and the source is changing */
	  if (edge->src_elist->node == node)
	    {
	      if ((edge->data.target_offset != new_to) ||
		  (edge->data.source_offset != new_so))
		{
		  /* SELF EDGE: Modify both src/trg offsets */
		  edata.target_stride = edge->data.target_stride;
		  edata.source_stride = edge->data.source_stride;
		  edata.assign_size = edge->data.assign_size;
		  edata.target_offset = new_to;
		  edata.source_offset = new_so;
		  new_edge = IPA_consg_ensure_edge_d(elist->edge_type,
						     node, node, &edata,
						     (edge->flags & ~in_wl_flag));

		  if (new_edge && CG_EDGE_ISNEW(new_edge))
		    {
		      work_list = add_to_worklist(work_list, in_wl_flag, new_edge);
		      in_new++;
		    }

		  if (!IPA_FLAG_ISSET(edge->flags, MAKE_FI_DELETE))
		    {
		      del_list = List_insert_last(del_list, edge);
		      IPA_FLAG_SET(edge->flags, MAKE_FI_DELETE);
		    }
		}
	    }
	  else if (edge->data.target_offset != new_to)
	    {
	      /* IN EDGE: modify only target offset */
	      edata.target_stride = edge->data.target_stride;
	      edata.source_stride = edge->data.source_stride;
	      edata.assign_size = edge->data.assign_size;
	      edata.source_offset = edge->data.source_offset;
	      edata.target_offset = new_to;
	      new_edge = IPA_consg_ensure_edge_d(elist->edge_type,
						 edge->src_elist->node, 
						 node, 
						 &edata, 
						 (edge->flags & ~in_wl_flag));

	      if (new_edge && CG_EDGE_ISNEW(new_edge))
		{
		  work_list = add_to_worklist(work_list, in_wl_flag, new_edge);
		  in_new++;
		}

	      if (!IPA_FLAG_ISSET(edge->flags, MAKE_FI_DELETE))
		{
		  del_list = List_insert_last(del_list, edge);
		  IPA_FLAG_SET(edge->flags, MAKE_FI_DELETE);
		}
	    }
	}
      IPA_htab_unblockrehash(elist->in);

      IPA_htab_blockrehash(elist->out);
      IPA_HTAB_START(eiter, elist->out);
      IPA_HTAB_LOOP(eiter)
	{
	  edge = IPA_HTAB_CUR(eiter);

	  /* The first loop will have handled self-edges. Avoid
	     reprocessing them */
	  if(edge->dst_elist->node == node)
	    continue;

	  new_so = mod_offset(edge->data.source_offset, mod);

	  if (edge->data.source_offset != new_so)
	    {
	      /* OUT EDGE: modify only source offset */
	      edata.target_stride = edge->data.target_stride;
	      edata.source_stride = edge->data.source_stride;
	      edata.assign_size = edge->data.assign_size;
	      edata.target_offset = edge->data.target_offset;
	      edata.source_offset = new_so;
	      new_edge = IPA_consg_ensure_edge_d(elist->edge_type,
						 node, 
						 edge->dst_elist->node, 
						 &edata, 
						 (edge->flags & ~in_wl_flag));

	      if (new_edge && CG_EDGE_ISNEW(new_edge))
		{
		  work_list = add_to_worklist(work_list, in_wl_flag, new_edge);
		  out_new++;
		}

	      if (!IPA_FLAG_ISSET(edge->flags, MAKE_FI_DELETE))
		{
		  del_list = List_insert_last(del_list, edge);
		  IPA_FLAG_SET(edge->flags, MAKE_FI_DELETE);
		}
	    }
	}
      IPA_htab_unblockrehash(elist->out);
    }

#if 0
  if (mod == 1)
    {
      for (elist=node->first_list; elist; elist=elist->nxt_list)
	{
	  IPA_HTAB_START(eiter, elist->in);
	  IPA_HTAB_LOOP(eiter)
	    {
	      edge = IPA_HTAB_CUR(eiter);
	      if (IPA_FLAG_ISSET(edge->flags, MAKE_FI_DELETE))
		continue;
	      assert(edge->data.target_offset == 0);
	    }
	  IPA_HTAB_START(eiter, elist->out);
	  IPA_HTAB_LOOP(eiter)
	    {
	      edge = IPA_HTAB_CUR(eiter);
	      if (IPA_FLAG_ISSET(edge->flags, MAKE_FI_DELETE))
		continue;
	      assert(edge->data.source_offset == 0);
	    }
	}
    }
#endif

  for (elist=node->first_list; elist; elist=elist->nxt_list)
    {
      in_all += IPA_htab_size(elist->in);
      out_all += IPA_htab_size(elist->out);
    }

  printf("MERGED %d.%d : del_list = %d edges | all i %d o %d | new i %d o %d\n",
	 node->data.var_id,
	 node->data.version,
	 List_size(del_list),
	 in_all, out_all,
	 in_new, out_new);

  List_start(work_list);
  while ((edge = List_next(work_list)))
    {
      if (!IPA_FLAG_ISSET(edge->flags, MAKE_FI_DELETE))
	continue;
      work_list = List_delete_current (work_list);
      IPA_FLAG_CLR(edge->flags, in_wl_flag);
    }
  
  List_start(del_list);
  while ((edge = List_next(del_list)))
    {
#if 0
      printf("ACT DELETE %p %p %p : ", edge, edge->src_elist, edge->dst_elist);
      print_edge(edge);
#endif
      IPA_cg_edge_delete(edge);
    }
  List_reset(del_list);
  del_list = NULL;
  List_start(work_list);

  return work_list;
}


/* Will merge dst node and src node (ie. dst represents src)
 * - merges both incoming and outgoing edges
 * - CALL and ASSIGN self-edges will be deleted
 *           all other edges will be retained
 */

List
IPA_cg_merge_nodes (IPA_cgraph_node_t *dst, IPA_cgraph_node_t * src,
		    int delete,
		    List work_list, int in_wl_flag)
{
  IPA_cgraph_edge_list_t *elist;
  IPA_cgraph_edge_t *edge, *new_edge;
  int edge_origin;
  IPA_HTAB_ITER eiter;

#if 0
  printf ("CG Merging nodes : ");
  IPA_cg_node_print (stdout, dst, IPA_PRINT_ASCI);
  IPA_cg_node_print (stdout, src, IPA_PRINT_ASCI);
  printf ("\n");
#endif

  /* To avoid really annoying errors/situations, never merge
     functions and non-functions */
  assert((IPA_FLAG_ISCLR(dst->flags, IPA_CG_NODE_FLAGS_FUNC) && 
	  IPA_FLAG_ISCLR(src->flags, IPA_CG_NODE_FLAGS_FUNC)) ||
	 (IPA_FLAG_ISSET(dst->flags, IPA_CG_NODE_FLAGS_FUNC) && 
	  IPA_FLAG_ISSET(src->flags, IPA_CG_NODE_FLAGS_FUNC)));

  /* These nodes are special and should never be merged 
   */
  assert(IPA_FLAG_ISCLR(dst->flags, (IPA_CG_NODE_FLAGS_PARAM |
				     IPA_CG_NODE_FLAGS_RETURN)));
  assert(IPA_FLAG_ISCLR(src->flags, (IPA_CG_NODE_FLAGS_PARAM |
				     IPA_CG_NODE_FLAGS_RETURN)));

  dst->data.var_size = Max(dst->data.var_size, src->data.var_size);
  if (dst->data.mod > src->data.mod)
    {
      dst->data.mod = src->data.mod;
      work_list = IPA_cgraph_make_fi(dst, work_list, in_wl_flag);
    }
  if (dst->data.in_k_cycle == 0)
    dst->data.in_k_cycle = src->data.in_k_cycle;
  else if (src->data.in_k_cycle != 0)
    dst->data.in_k_cycle = Min(dst->data.in_k_cycle, src->data.in_k_cycle);

  dst->flags |= (src->flags & (IPA_CG_NODE_FLAGS_HEAP |
			       IPA_CG_NODE_FLAGS_GLOBAL |
			       IPA_CG_NODE_FLAGS_NOFIELD |
			       IPA_CG_NODE_FLAGS_NOCNTXT |
			       IPA_CG_NODE_FLAGS_NOLOCAL
			       ));
 
  for (elist = src->first_list; elist; elist = elist->nxt_list)
    {
      IPA_HTAB_START(eiter, elist->in);
      IPA_HTAB_LOOP(eiter)
	{
	  IPA_cgraph_edge_data_t ndata;
	  edge = (IPA_cgraph_edge_t *)IPA_HTAB_CUR(eiter);

	  /* Self-loops handled in next loop */
	  if (edge->src_elist->node == edge->dst_elist->node)
	    continue;

	  ndata = edge->data;
	  ndata.target_offset = ndata.target_offset % dst->data.mod;

          /* Avoid creating self cycles */
          if ((dst == edge->src_elist->node) &&
	      (elist->edge_type == ASSIGN &&
	       ndata.source_offset == ndata.target_offset))
            continue;

	  edge_origin = edge->flags & IPA_CG_EDGE_FLAGS_DIR_ALL;
	  new_edge = IPA_cg_edge_add (edge->src_elist->node, dst, 
				      elist->edge_type, &(ndata));

	  if (new_edge && CG_EDGE_ISNEW(new_edge))
	    {
	      IPA_FLAG_SET (new_edge->flags, edge_origin);
	      IPA_FLAG_CLR (new_edge->flags, (IPA_CG_EDGE_FLAGS_PROCESSED|
					      IPA_CG_EDGE_FLAGS_CALLG_PROCESSED|
					      IPA_CG_EDGE_FLAGS_GENERICALL));
	      work_list = add_to_worklist(work_list, in_wl_flag, new_edge);
	    }
	  DEBUG(check_edge(new_edge););
        }

      IPA_HTAB_START(eiter, elist->out);
      IPA_HTAB_LOOP(eiter)
	{
	  IPA_cgraph_edge_data_t ndata;
	  IPA_cgraph_node_t *new_src = NULL;
	  IPA_cgraph_node_t *new_dst = NULL;
	  edge = (IPA_cgraph_edge_t *)IPA_HTAB_CUR(eiter);

	  ndata = edge->data;
	  ndata.source_offset = ndata.source_offset % dst->data.mod;
	  
	  new_src = dst;
	  if (edge->src_elist->node != edge->dst_elist->node)
	    new_dst = edge->dst_elist->node;
	  else
	    {
	      new_dst = dst;
	      ndata.target_offset = ndata.target_offset % dst->data.mod;
	    }
	    
          /* Avoid creating self cycles */
          if ((dst == edge->dst_elist->node) &&
               (elist->edge_type == ASSIGN &&
		ndata.source_offset == ndata.target_offset))
            continue;

	  edge_origin = edge->flags & IPA_CG_EDGE_FLAGS_DIR_ALL;
	  new_edge = IPA_cg_edge_add (new_src, new_dst,
				      elist->edge_type, &(ndata));
	  if (new_edge && CG_EDGE_ISNEW(new_edge))
	    {
	      IPA_FLAG_SET (new_edge->flags, edge_origin);
	      IPA_FLAG_CLR (new_edge->flags, (IPA_CG_EDGE_FLAGS_PROCESSED|
					      IPA_CG_EDGE_FLAGS_CALLG_PROCESSED|
					      IPA_CG_EDGE_FLAGS_GENERICALL));
	      work_list = add_to_worklist(work_list, in_wl_flag, new_edge);
	    }
	  DEBUG(check_edge(new_edge););
        }
    }

  /* src is being merged away. 
     Remove its edges from work list */
  List_start(work_list);
  while ((edge = List_next(work_list)))
    {
      if (edge->src_elist->node != src &&
	  edge->dst_elist->node != src)
	continue;
      work_list = List_delete_current(work_list);
      IPA_FLAG_CLR(edge->flags, in_wl_flag);
    }  
  
  if (!IPA_cg_node_same (dst, src) && !delete)
    {
      IPA_cgraph_node_t *tmp_node;

      /* Reset edges in src */
      IPA_cg_node_reset (src);

      /* Get last child in dst's list */
      for (tmp_node = dst; tmp_node->rep_child;
	   tmp_node = tmp_node->rep_child);

      /* Last child of dst points to src */
      tmp_node->rep_child = src;

      /* dst's parent is src's parent */
      src->rep_parent = IPA_cg_node_get_rep(dst);
    }
  else
    {
      assert(src != dst);
      IPA_cg_node_delete (src);
    }

#if 1
  for (elist = src->first_list; elist; elist = elist->nxt_list)
    {
      assert(IPA_htab_size(elist->out) == 0);
      assert(IPA_htab_size(elist->in) == 0);
    }
#endif
  DEBUG(check_node(dst));

  return work_list;
}


/*************************************************************************
 * Topological Sort
 *************************************************************************/

#define TOPO_VISITED IPA_CG_NODE_FLAGS_GENERIC1

static List
IPA_cg_toposort_rec(IPA_cgraph_t * cgraph,
		    IPA_cgraph_node_t * node,
		    List node_list, int valid_edges)
{
  IPA_cgraph_edge_list_t *elist;
  IPA_cgraph_edge_t      *edge;
  IPA_HTAB_ITER           eiter;

  /* If can include and not already included in list */
  if (IPA_FLAG_ISSET (node->flags, TOPO_VISITED))
    return node_list;
  IPA_FLAG_SET (node->flags, TOPO_VISITED);

  /* Include outgoing arcs from children of rep node */
  for (elist = node->first_list; elist; elist = elist->nxt_list)
    {
      if (!IPA_edge_list_valid (elist, valid_edges))
        continue;

      IPA_HTAB_START(eiter, elist->out);
      IPA_HTAB_LOOP(eiter)
	{
	  edge = (IPA_cgraph_edge_t *)IPA_HTAB_CUR(eiter);
          node_list =
            IPA_cg_toposort_rec (cgraph, edge->dst_elist->node,
				 node_list, valid_edges);
        }
    }
  
  node_list = List_insert_first (node_list, node);
  
  return node_list;
}

List
IPA_cg_find_toposort (IPA_cgraph_t * cgraph, 
		      IPA_cgraph_node_t * root,
		      int valid_edges)
{
  List topo_list = NULL;

  /* Clear flags */
  IPA_cg_nodes_clr_flags (cgraph, (TOPO_VISITED));
    
  topo_list =
    IPA_cg_toposort_rec (cgraph, root, topo_list, valid_edges);

  /* Clear flags */
  IPA_cg_nodes_clr_flags (cgraph, (TOPO_VISITED));

  return topo_list;
}

void
IPA_cg_topo_print (FILE * file, List list)
{
  IPA_cgraph_node_t *node;

  fprintf (file, "TOPO: ");
  List_start (list);
  while ((node = List_next (list)))
    {
      IPA_cg_node_print (file, node,IPA_PRINT_ASCI);
    }
  fprintf (file, "\n");
}


/*************************************************************************
 * Apply one graph onto another
 *************************************************************************/
static IPA_cgraph_link_t *
IPA_cg_link_new ()
{
  IPA_cgraph_link_t *l;

  l = (IPA_cgraph_link_t *) L_alloc (IPA_cgraph_link_pool);
  bzero (l, sizeof (IPA_cgraph_link_t));

  return l;
}

static void
IPA_cg_link_free (IPA_cgraph_link_t *link)
{
  L_free (IPA_cgraph_link_pool, link);
}


void
IPA_cg_clear_links (IPA_cgraph_t * cg)
{
  IPA_cgraph_node_t *node;
  IPA_HTAB_ITER niter;
  IPA_cgraph_link_t *pl;
  IPA_cgraph_link_t *l;

  IPA_HTAB_START(niter, cg->nodes);
  IPA_HTAB_LOOP(niter)
    {
      node = (IPA_cgraph_node_t *)IPA_HTAB_CUR(niter);
      if (!node->lk)
	continue;
#if 0
      printf("Clear Linking ");
      IPA_cg_node_print(stdout, node->link, IPA_PRINT_ASCI);
      IPA_cg_node_print(stdout, node, IPA_PRINT_ASCI);
      printf("\n");
#endif

      for (l=node->lk;l;l=pl)
	{
	  pl = l->nxt_l;
	  IPA_cg_link_free(l);
	}
      node->lk = NULL;
    }
}

void
IPA_cg_link_nodes (IPA_cgraph_t * s_cg, IPA_cgraph_node_t * s_node,
                   IPA_cgraph_t * d_cg, IPA_cgraph_node_t * d_node)
{
  IPA_cgraph_link_t *newlk;
#if 0
  printf("Linking ");
  IPA_cg_node_print(stdout, s_node, IPA_PRINT_ASCI);
  IPA_cg_node_print(stdout, d_node, IPA_PRINT_ASCI);
  printf("\n");
#endif

  assert (d_node->lk == NULL);

  newlk = IPA_cg_link_new ();
  newlk->nxt_l = s_node->lk;
  s_node->lk = newlk;
  s_node->lk->to = d_node;
}


void
IPA_cg_apply_graph (IPA_cgraph_t * cg_d, IPA_cgraph_t * cg_s)
{
  IPA_cgraph_node_t *src_s_node;
  IPA_cgraph_node_t *src_d_node;
  IPA_cgraph_node_t *d_node;
  IPA_cgraph_node_t *s_node;
  IPA_cgraph_edge_list_t *elist;
  IPA_cgraph_edge_t *edge;
  IPA_cgraph_edge_t *new_edge;
  IPA_cgraph_link_t *s_lk;
  IPA_cgraph_link_t *d_lk;
  IPA_HTAB_ITER niter;
  IPA_HTAB_ITER eiter;
  IPA_cgraph_edge_data_t new_data;

  /* At this point, there should be a node in cg_d for every
   *  node in cg_s. Now we just have to connect them 
   */
  IPA_HTAB_START(niter, cg_s->nodes);
  IPA_HTAB_LOOP(niter)
    {
      src_s_node = (IPA_cgraph_node_t *)IPA_HTAB_CUR(niter);

      /* Find all edges out of this node and create similar 
       *  edges in cg_d
       */
      for (elist = src_s_node->first_list; elist; elist = elist->nxt_list)
        {
	  IPA_HTAB_START(eiter, elist->out);
	  IPA_HTAB_LOOP(eiter)
	    {
	      edge = (IPA_cgraph_edge_t *)IPA_HTAB_CUR(eiter);
	      src_d_node = edge->dst_elist->node;

              /* We have an edge in the src graph. Add this
                 edge for each linked src/dst combination in the
                 dst graph */
	      for (s_lk = src_s_node->lk; s_lk; s_lk = s_lk->nxt_l)
		for (d_lk = src_d_node->lk; d_lk; d_lk = d_lk->nxt_l)
		  {
		    s_node = s_lk->to;
		    d_node = d_lk->to;
#if 0
		    printf("Original: ");
		    IPA_cg_node_print(stdout,node,IPA_PRINT_ASCI);
		    printf(" -> ");
		    IPA_cg_node_print(stdout,edge->dst_elist->node,IPA_PRINT_ASCI);
		    printf("\nConnect : ");
		    IPA_cg_node_print(stdout,s_node,IPA_PRINT_ASCI);
		    printf(" -> ");
		    IPA_cg_node_print(stdout,d_node,IPA_PRINT_ASCI);
		    printf("\n");
#endif

		    new_data.source_offset = mod_offset(edge->data.source_offset,
							s_node->data.mod);
		    new_data.source_stride = edge->data.source_stride;
		    new_data.assign_size = edge->data.assign_size;
		    new_data.target_offset = mod_offset(edge->data.target_offset,
							d_node->data.mod);;
		    new_data.target_stride = edge->data.target_stride;

		    new_edge = IPA_cg_edge_add (s_node, d_node,
						elist->edge_type,
						&new_data);
		    /*CG_EDGE_COPYFLAGS(new_edge, edge);*/
		    IPA_consg_apply_edge_flags(new_edge, edge->flags);
		    DEBUG(check_edge(new_edge););
		  }                   /* edges out of node */
	    } /*link for loops */
        }                       /* edge lists */
    }                           /* nodes */
 
}


