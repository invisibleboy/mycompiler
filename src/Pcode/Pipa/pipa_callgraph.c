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
 *      File:    pipa_callgraph.c
 *      Author:  Erik Nystrom
 *      Copyright (c) 2003  Erik Nystrom, Wen-mei Hwu
 *              and The Board of Trustees of the University of Illinois.
 *              All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#include <pipa_callgraph.h>
#include <pipa_program.h>
#include <pipa_driver_utils.h>

L_Alloc_Pool *IPA_callg_edge_pool = NULL;
L_Alloc_Pool *IPA_callg_node_pool = NULL;
L_Alloc_Pool *IPA_callg_pool = NULL;

static int is_ext = 0;

/*************************************************************************
 * UPDATE BUFFER
 *************************************************************************/

IPA_callg_update_t*
IPA_callg_update_new( )
{
  return (IPA_callg_update_t*)calloc(1,sizeof(IPA_callg_update_t));
}

void
IPA_callg_update_free(IPA_callg_update_t *cgu)
{
  if (cgu)
    free(cgu);
}

List
IPA_callg_update_add(List update_list,
		     IPA_callg_node_t *caller_node,
		     IPA_callsite_t   *caller_cs,
		     char *callee_name,
		     Key callee_key)
{
  IPA_callg_update_t *cgu;

  cgu = IPA_callg_update_new();
  assert(caller_node);
  assert(caller_cs);

  cgu->caller_node = caller_node;
  cgu->caller_cs = caller_cs;
  cgu->callee_symbol_name = C_findstr(callee_name);
  cgu->callee_symbol_key = callee_key;

  update_list = List_insert_last(update_list, cgu);
  return update_list;
}
	
int
IPA_callg_update_callg(IPA_prog_info_t * info,
		       List update_list,
		       int round)
{
  IPA_callg_update_t *cgu;
  List new_edge_list = NULL;
  int count = 0;

  List_start(update_list);
  while ((cgu = List_next(update_list)))
    {
      new_edge_list = 
	IPA_callgraph_build_direct (info, 
				    cgu->caller_node,
				    cgu->caller_cs, 
				    cgu->callee_symbol_name,
				    cgu->callee_symbol_key,
				    1, new_edge_list, round);
      IPA_callg_update_free(cgu);
    }
  List_reset(update_list);

  count = List_size(new_edge_list);
  List_reset(new_edge_list);
  return count;
}

/*************************************************************************
 * EDGE
 *************************************************************************/

static IPA_callg_edge_t *
IPA_callg_edge_new()
{
  IPA_callg_edge_t *edge;

  edge = L_alloc(IPA_callg_edge_pool);
  bzero (edge, sizeof (IPA_callg_edge_t));

  return edge;
}


static void
IPA_callg_edge_free(IPA_callg_edge_t *edge)
{
  if (!edge)
    return;
  L_free(IPA_callg_edge_pool, edge);
}

IPA_callg_edge_t *
IPA_callg_edge_find(struct IPA_callg_node_t *caller,
		    struct IPA_callsite_t   *caller_cs,
		    struct IPA_callg_node_t *callee,
		    struct IPA_interface_t  *callee_if)
{
  IPA_callg_edge_t *edge;

  List_start(callee->caller_edges);
  while ((edge = List_next(callee->caller_edges)))
    {
      if (edge->caller == caller &&
	  edge->caller_cs == caller_cs &&
	  edge->callee == callee)
	{
	  CALLG_EDGE_CLRNEW(edge);
	  return edge;
	}
    }
  
  return NULL;
}

IPA_callg_edge_t *
IPA_callg_edge_add(struct IPA_callg_node_t *caller,
		   struct IPA_callsite_t   *caller_cs,
		   struct IPA_callg_node_t *callee,
		   struct IPA_interface_t   *callee_if,
		   int is_indirect)
{
  IPA_callg_edge_t *edge;

  if (!(edge = IPA_callg_edge_find(caller, caller_cs,
				   callee, callee_if)))
    {
      edge = IPA_callg_edge_new();

      assert(caller);
      assert(callee);
      if (is_ext == 0)
	{
	  assert(caller_cs);
	  assert(callee_if);
	}

      edge->caller = caller;
      edge->caller_cs = caller_cs;
      edge->callee = callee;
      edge->callee_if = callee_if;

      /* Add to caller's callee list */
      caller->callee_edges = List_insert_last(caller->callee_edges, edge);

      /* Add to callee's caller list */
      callee->caller_edges = List_insert_last(callee->caller_edges, edge);

      if (is_indirect)
	IPA_FLAG_SET(edge->flags, IPA_CALLG_EDGE_FLAGS_INDIRECT);
      else
	IPA_FLAG_SET(edge->flags, IPA_CALLG_EDGE_FLAGS_DIRECT);	

      CALLG_EDGE_SETNEW(edge);
    }

  return edge;
}


void
IPA_callg_edge_delete (IPA_callg_edge_t * edge)
{
  assert(List_member(edge->caller->callee_edges, edge));
  edge->caller->callee_edges = List_remove(edge->caller->callee_edges, edge);

  assert(List_member(edge->callee->caller_edges, edge));
  edge->callee->caller_edges = List_remove(edge->callee->caller_edges, edge);

  edge->callee_if = NULL;
  edge->caller_cs = NULL;

  IPA_callg_edge_free(edge);
}



/*************************************************************************
 * NODE
 *************************************************************************/

static IPA_callg_node_t *
IPA_callg_node_new ()
{
  IPA_callg_node_t *node;

  node = L_alloc(IPA_callg_node_pool);
  bzero (node, sizeof (IPA_callg_node_t));

  return node;
}


static void
IPA_callg_node_free (IPA_callg_node_t *node)
{
  if (!node)
    return;
  L_free(IPA_callg_node_pool, node);
}


IPA_callg_node_t *
IPA_callg_node_find (struct IPA_callg_t *callg,
		     struct IPA_funcsymbol_info_t *fninfo)
{
  IPA_callg_node_t *node;

  List_start(callg->nodes);
  while ((node = List_next(callg->nodes)))
    {
      if (node->fninfo == fninfo)
	{
	  CALLG_NODE_CLRNEW(node);
	  return node;
	}
    }

  return NULL;
}

static IPA_callg_node_t *
IPA_callg_node_find_key (struct IPA_callg_t *callg,
			 Key key)
{
  IPA_callg_node_t *node;

  List_start(callg->nodes);
  while ((node = List_next(callg->nodes)))
    {
      if (P_MatchKey(node->fninfo->func_key, key))
	{
	  return node;
	}
    }

  return NULL;
}


IPA_callg_node_t *
IPA_callg_node_add (struct IPA_callg_t *callg,
		    struct IPA_funcsymbol_info_t *fninfo)
{
  IPA_callg_node_t *node;

  if (!(node = IPA_callg_node_find (callg, fninfo)))
    {
      node = IPA_callg_node_new();
      node->fninfo = fninfo;
      fninfo->call_node = node;
      node->callgraph = callg;
      node->rep_parent = node;

      callg->nodes = List_insert_last(callg->nodes, node);
      CALLG_NODE_SETNEW(node);
    }

  return node;
}


void
IPA_callg_node_delete (IPA_callg_node_t *node)
{
  IPA_callg_edge_t *edge;

  assert(List_member(node->callgraph->nodes, node));
  node->callgraph->nodes = List_remove(node->callgraph->nodes,
				       node);

  node->fninfo->call_node = NULL;
  node->fninfo = NULL;

  List_start(node->caller_edges);
  while ((edge = List_next(node->caller_edges)))
    {
      IPA_callg_edge_delete(edge);
    }
  List_reset(node->caller_edges);
  node->caller_edges = NULL;

  List_start(node->callee_edges);
  while ((edge = List_next(node->callee_edges)))
    {
      IPA_callg_edge_delete(edge);
    }
  List_reset(node->callee_edges);
  node->callee_edges = NULL;

  IPA_callg_node_free(node);
}


IPA_callg_node_t *
IPA_callg_node_get_rep (IPA_callg_node_t *node)
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


/*************************************************************************
 * GRAPH
 *************************************************************************/

IPA_callg_t*
IPA_callg_new()
{
  IPA_callg_t *callg;

  callg = L_alloc (IPA_callg_pool);
  bzero (callg, sizeof (IPA_callg_t));

  return callg;
}

void
IPA_callg_free(IPA_callg_t *callg)
{
  IPA_callg_node_t *node;

  List_start(callg->nodes);
  while ((node = List_next(callg->nodes)))
    {
      IPA_callg_node_delete(node);
    }

  L_free (IPA_callg_pool, callg);
}

void
IPA_callg_minit ()
{
  static char init = 0;

  if (init)
    I_punt ("IPA_cgraph_minit: already called\n");
  init = 1;

  IPA_callg_edge_pool =
    L_create_alloc_pool ("callg_edge", sizeof (IPA_callg_edge_t), 1024);
  IPA_callg_node_pool =
    L_create_alloc_pool ("callg_node", sizeof (IPA_callg_node_t), 128);
  IPA_callg_pool = 
    L_create_alloc_pool ("callg", sizeof (IPA_callg_t), 16);
}

void
IPA_callg_pool_info()
{
  L_print_alloc_info(stdout,  IPA_callg_edge_pool, 1); 
  L_print_alloc_info(stdout,  IPA_callg_node_pool, 1); 
  L_print_alloc_info(stdout,  IPA_callg_pool, 1); 
}

void
IPA_callg_mfree ()
{
  L_free_alloc_pool (IPA_callg_edge_pool);
  IPA_callg_edge_pool = NULL;

  L_free_alloc_pool (IPA_callg_node_pool);
  IPA_callg_node_pool = NULL;

  L_free_alloc_pool (IPA_callg_pool);
  IPA_callg_pool = NULL;
}


/*************************************************************************
 * Merge Operations
 *************************************************************************/

void
IPA_callg_merge_nodes (IPA_callg_node_t *dst,
		       IPA_callg_node_t *src)
{
  IPA_callg_node_t *node;

  assert(dst && src);
#if 0
  printf ("CALLG Merging nodes [%s] <- [%s]\n",
	  dst->fninfo->func_name,
	  src->fninfo->func_name);
#endif
  /* This is "special" merge in that the rep_XXX fields
     are set but the edges stay put. */
  
  /* Find the end of the rep_parent for dst */
  for (node = dst; node->rep_child; 
       node = node->rep_child);

  node->rep_child = src;

  for (node = src; node; node = node->rep_child)
    node->rep_parent = dst;
}


/*************************************************************************
 * Miscellaneous
 *************************************************************************/

void
IPA_callg_nodes_clr_flags (IPA_callg_t * callg, unsigned int flags)
{
  IPA_callg_node_t   *node;

  List_start(callg->nodes);
  while ((node = List_next(callg->nodes)))
    {
      IPA_FLAG_CLR (node->flags, flags);
    }
}

/*************************************************************************
 * Topological Sort
 *************************************************************************/

#define TOPO_VISITED IPA_CALLG_NODE_FLAGS_GENERIC1

static List
IPA_callg_toposort_rec(IPA_callg_t * callg,
		       IPA_callg_node_t * node,
		       List node_list)
{
  IPA_callg_edge_t      *edge;

  /* If can include and not already included in list */
  if (IPA_FLAG_ISSET (node->flags, TOPO_VISITED))
    return node_list;
  IPA_FLAG_SET (node->flags, TOPO_VISITED);

  /* Include outgoing arcs from children of rep node */
  List_start(node->callee_edges);
  while ((edge = List_next(node->callee_edges)))
    {
      /* Don't go into noexit routines except from top-level */
      if (edge->callee->fninfo->is_noexit &&
	  !edge->caller->fninfo->is_globals)
	continue;
      node_list =
	IPA_callg_toposort_rec (callg, edge->callee,
				node_list);
    }
  
  /* Only list SCC parents */
  if (node->rep_parent == node)
    node_list = List_insert_first (node_list, node);
  
  return node_list;
}

List
IPA_callg_find_toposort (IPA_callg_t * callg, IPA_callg_node_t *root) 
{
  List topo_list = NULL;

  /* Clear flags */
  IPA_callg_nodes_clr_flags (callg, (TOPO_VISITED));
    
  /* Process all of the roots */
  topo_list =
    IPA_callg_toposort_rec (callg, root, topo_list);

  return topo_list;
}

void
IPA_callg_topo_print (FILE * file, List list)
{
  IPA_callg_node_t *node;

  fprintf (file, "TOPO: ");
  List_start (list);
  while ((node = List_next (list)))
    {
      fprintf(file, "[%s] ",node->fninfo->func_name);
    }
  fprintf (file, "\n");
}


/*************************************************************************
 * SCC Detection
 *************************************************************************/
#define IPA_CALLG_SCC_WHITE    IPA_CALLG_NODE_FLAGS_GENERIC1
#define IPA_CALLG_SCC_ONSTACK  IPA_CALLG_NODE_FLAGS_GENERIC2

IPA_callg_scc_stack_t *
IPA_callg_scc_stack_new ()
{
  IPA_callg_scc_stack_t *stack;

  stack =
    (IPA_callg_scc_stack_t *) calloc (1, sizeof (IPA_callg_scc_stack_t));

  return stack;
}


void
IPA_callg_scc_stack_free (IPA_callg_scc_stack_t * stack)
{
  free (stack);
}

void
IPA_callg_free_SCC (IPA_callg_t * callg, List list)
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
IPA_callg_print_SCC (FILE * file, List list)
{
  IPA_callg_node_t *node;
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
	  fprintf(file, "[%s] \n",node->fninfo->func_name);
        }
    }
  fprintf (file, "----------------------\n");
}



void
IPA_callg_search_SCC (IPA_callg_t * cgraph, IPA_callg_node_t * node,
		      List * stack, int *count, List * scc_lists)
{
  IPA_callg_edge_t *edge;
  IPA_callg_scc_stack_t *st;

  IPA_FLAG_CLR (node->flags, IPA_CALLG_SCC_WHITE);
  /*printf("Node %d\n", node->data.var_id); */

  st = IPA_callg_scc_stack_new ();
  st->def_num = (*count)++;
  st->low_link = st->def_num;
  st->node = node;
  node->st = st;
  (*stack) = List_insert_first ((*stack), st);

  /* For all output edges */
  List_start(node->callee_edges);
  while ((edge = List_next(node->callee_edges)))
    {
      IPA_callg_node_t *dst_node;

      /* Don't go into noexit routines */
      if (edge->callee->fninfo->is_noexit)
	continue;
      
      dst_node = edge->callee;

      if (dst_node == node)
	{
	  /* Trivial SCC */
	  continue;
	}
      
      if (IPA_FLAG_ISSET (dst_node->flags, IPA_CALLG_SCC_WHITE))
	{
	  IPA_callg_search_SCC (cgraph, dst_node, stack,
				count, scc_lists);
	  /* dst_node may be removed if it was a part of an SCC */
	  if (dst_node->st)
	    st->low_link = Min (st->low_link, dst_node->st->low_link);
	}
      else if (dst_node->st)
	{
	  if (dst_node->st->def_num < node->st->def_num)
	    st->low_link = Min (st->low_link, 
				dst_node->st->def_num);
	}
    }

  if (st->low_link == st->def_num)
    {
      List new_list = NULL;
      IPA_callg_node_t *scc_node;
      IPA_callg_scc_stack_t *scc_st;

      /*printf("Found SCC : "); */
      do
        {
          scc_st = List_first ((*stack));
          scc_node = scc_st->node;

          (*stack) = List_delete_current ((*stack));
	  scc_node->st = NULL;

          new_list = List_insert_last (new_list, scc_node);
          IPA_callg_scc_stack_free (scc_st);

          /*printf("%d ", scc_node->data.var_id); */
        }
      while (node != scc_node);
      /*printf("\n"); */

      st = NULL;
      (*scc_lists) = List_insert_last ((*scc_lists), new_list);
    }
}


List
IPA_callg_find_SCC (IPA_callg_t * callg)
{
  IPA_callg_node_t *node;
  List scc_lists = NULL;
  List stack = NULL;
  int count;

  /* Init vars */
  count = 1;

  /* Init graph */
  List_start(callg->nodes);
  while ((node = List_next(callg->nodes)))
    {
      IPA_FLAG_SET (node->flags, IPA_CALLG_SCC_WHITE);
      node->st = NULL;
    }

  /* Run SCC detection */
  List_start(callg->nodes);
  while ((node = List_next(callg->nodes)))
    {
      /* Only search parent nodes */
      if (node->rep_parent != node)
	continue;

      if (IPA_FLAG_ISSET (node->flags, IPA_CALLG_SCC_WHITE))
	{
	  IPA_callg_search_SCC (callg, node, &stack, &count,
				&scc_lists);
	}
    }

  /* Free stack */
  assert(List_size(stack) == 0);
  List_reset (stack);

  return scc_lists;
}








/*************************************************************************
 * Dotty Printing Interface
 *************************************************************************/

#define IPA_CALLG_DVPRINT_VISITED IPA_CALLG_NODE_FLAGS_GENERIC1

void
IPA_callg_DVname_node(IPA_callg_node_t * node, char *node_name)
{
  int p;
	  
  p = sprintf (node_name, "[%s]",
	       node->fninfo->func_name);
}

char *
IPA_callg_DVedge_color(IPA_callg_edge_t *edge)
{
  if (IPA_FLAG_ISSET(edge->flags, IPA_CALLG_EDGE_FLAGS_INDIRECT))
    return "red";
  else
    return "black";

  assert(0);
  return NULL;
}

char *
IPA_callg_DVnode_color(IPA_callg_node_t *node)
{
  char *color;

  color = "lightgrey";
  
  return color;
}

char *
IPA_callg_DVnode_border(IPA_callg_t *cgraph, IPA_callg_node_t *node)
{
  char *border;

  border = "single";

  return border;
}

static void
IPA_callg_DVprint_node (FILE * file, IPA_callg_t *callg,
			IPA_callg_node_t * node)
{
  fprintf(file, "   \"%s\" [label=\"%s\",style=filled,color=ivory,height=0.1,fontsize=9];\n",
	  node->fninfo->func_name,
	  node->fninfo->func_name);
}

static void
IPA_callg_DVprint_edge(FILE * file, IPA_callg_edge_t *edge)
{
  char *color;  
  color = IPA_callg_DVedge_color(edge);  
  fprintf(file, "    \"%s\" -> \"%s\" [color = %s];\n",
	  edge->caller->fninfo->func_name,
	  edge->callee->fninfo->func_name,
	  color);  
}

void
IPA_callg_DVprint (IPA_callg_t * callg, char *name)
{
  FILE *file;
  IPA_callg_node_t *node;

  file = IPA_fopen(NULL, IPA_file_subdir, name, "dot", "w");

  fprintf (file, "digraph G { \n");

  List_start(callg->nodes);
  while ((node = List_next(callg->nodes)))
    {
      IPA_callg_DVprint_node (file, callg, node);      
    }

  fprintf (file, "\n/*\n ALL EDGES \n*/\n");

  List_start(callg->nodes);
  while ((node = List_next(callg->nodes)))
    {
      IPA_callg_edge_t *edge;
      List_start(node->callee_edges);
      while ((edge = List_next(node->callee_edges)))
	{
	  IPA_callg_DVprint_edge (file, edge);
	}
    }

  fprintf (file, "}\n");  
  fclose (file);
}


/*************************************************************************
 * Simple Printing Interface
 *************************************************************************/

void
IPA_callg_print(IPA_callg_t * callg, char *name)
{
  IPA_callg_node_t *node;
  IPA_callg_edge_t *edge;
  FILE *file;

  file = IPA_fopen(NULL, IPA_file_subdir, name, "cg", "w");

  List_start(callg->nodes);
  while ((node = List_next(callg->nodes)))
    {
      List_start(node->callee_edges);
      while ((edge = List_next(node->callee_edges)))
	{
	  fprintf (file, "CALLER %s CALLEE %s\n",
		   node->fninfo->func_name,
		   edge->callee->fninfo->func_name);
	}
    }  
}


/*************************************************************************
 * File Printing Interface
 *************************************************************************/

static void
IPA_callg_FLprint_node (FILE * file, 
			IPA_callg_t *callg,
			IPA_callg_node_t * node)
{
  fprintf(file, "    %s %d.%d ",
	  node->fninfo->func_name, 
	  node->fninfo->func_key.file,
	  node->fninfo->func_key.sym);

  if (node != node->rep_parent)
    {
      node = IPA_callg_node_get_rep(node);

      fprintf(file, " -  %s %d.%d",
	      node->fninfo->func_name, 
	      node->fninfo->func_key.file,
	      node->fninfo->func_key.sym);  
    }

  fprintf(file, "\n");
}

static void
IPA_callg_FLprint_edge(FILE * file, IPA_callg_edge_t *edge)
{
  IPA_funcsymbol_info_t *cr;
  IPA_funcsymbol_info_t *ce;

  cr = edge->caller->fninfo;
  ce = edge->callee->fninfo;
  
  if (is_ext == 0)
    {
      if (edge->caller_cs->call_expr)
	fprintf(file, "%-6d ", edge->caller_cs->call_expr->id);
      else
	fprintf(file, "%-6d ",0);
    }
  else
    {
#if LP64_ARCHITECTURE
      fprintf (file, "%-6ld", (long)edge->caller_cs);
#else
      fprintf(file, "%-6d ",(int)edge->caller_cs);
#endif
    }
    
  if (IPA_FLAG_ISSET(edge->flags, IPA_CALLG_EDGE_FLAGS_INDIRECT))
    fprintf(file, "I");
  else
    fprintf(file, "D");

  fprintf(file, "    %s %d.%d  %s %d.%d ",
	  cr->func_name, 
	  cr->func_key.file,
	  cr->func_key.sym,
	  ce->func_name, 
	  ce->func_key.file,
	  ce->func_key.sym);

  fprintf(file, "\n");
}

void
IPA_callg_FLprint (IPA_callg_t * callg, char *name)
{
  FILE *file;
  IPA_callg_node_t *node;

  file = IPA_fopen(NULL, ".", name, "fl", "w");

  List_start(callg->nodes);
  while ((node = List_next(callg->nodes)))
    {
      IPA_callg_FLprint_node (file, callg, node);      
    }

  fprintf (file, "\nEND\n");

  List_start(callg->nodes);
  while ((node = List_next(callg->nodes)))
    {
      IPA_callg_edge_t *edge;
      List_start(node->callee_edges);
      while ((edge = List_next(node->callee_edges)))
	{
	  IPA_callg_FLprint_edge (file, edge);
	}
    }

  fprintf (file, "\nEND\n");
  fclose (file);
}

static IPA_callg_node_t *
IPA_callg_FLread_node_core (FILE * file, 
			    IPA_callg_t *callg)
{
  IPA_callg_node_t *node = NULL;
  char name[256];
  Key key;
  
  assert(fscanf(file, "%256s", name) == 1);
  if (strcmp(name, "END") == 0)
    return NULL;

  assert(fscanf(file, "%d.%d", &key.file, &key.sym) == 2);
  assert(strlen(name) < 256);
#if 0
  printf("%s %d %d \n",  name, key.file, key.sym);
#endif
  if (!(node = IPA_callg_node_find_key(callg, key)))
    {
      IPA_funcsymbol_info_t *fninfo = IPA_funcsymbol_add_ext(key, name);
      node = IPA_callg_node_add (callg, fninfo);
    }
  
  return node;
}

static IPA_callg_node_t *
IPA_callg_FLread_node (FILE * file, 
		       IPA_callg_t *callg)
{
  IPA_callg_node_t * node;

  node = IPA_callg_FLread_node_core(file, callg);
  if (!node)
    return NULL;

  if (IPA_find_file_isnext_char (file, '-'))
    {
      IPA_callg_node_t *par_node = IPA_callg_FLread_node_core(file, callg);
      IPA_callg_merge_nodes (par_node, node);
    }

  fscanf(file, "\n");

  return node;
}

static IPA_callg_edge_t *
IPA_callg_FLread_edge(FILE * file, 
		      IPA_callg_t *callg)
{
  IPA_callg_node_t * src_n;
  IPA_callg_node_t * dst_n;
  IPA_callg_edge_t *edge;
  int exprid;
  char dir;

  if (fscanf(file, "%d %c", &exprid, &dir) != 2)
    {
      char name[10];
      assert(fscanf(file, "%10s", name) == 1);
      assert(strcmp(name,"END") == 0);
      return NULL;
    }
  
  src_n = IPA_callg_FLread_node_core(file, callg);
  dst_n = IPA_callg_FLread_node_core(file, callg);

  edge = IPA_callg_edge_add(src_n,
#if LP64_ARCHITECTURE
			    (void*)((long)exprid),
#else
			    (void*)exprid,
#endif
			    dst_n,
			    NULL,
			    (dir == 'I'));

  fscanf(file, "\n");

  return edge;
}

IPA_callg_t * 
IPA_callg_FLread (char *name)
{
  FILE *file;
  IPA_callg_t *callg;

  is_ext = 1;

  file = IPA_fopen(NULL, ".", name, "fl", "r");

  callg = IPA_callg_new();

  while (IPA_callg_FLread_node (file, callg));

  while (IPA_callg_FLread_edge (file, callg));

  fclose (file);

  return callg;
}


/*************************************************************************
 * Stats
 *************************************************************************/

typedef struct cs_stat_t 
{
  IPA_callsite_t *cs;
  int cnt;
} cs_stat_t;

void
IPA_callg_stats(IPA_callg_t * callg)
{
#if 0
  IPA_callg_node_t *node;
  IPA_callg_edge_t *edge;
  IPA_callsite_t *cs;
  int nodes =0;
  int dir_edges =0;
  int ind_edges =0;
  int dir_callsites =0;
  int ind_callsites =0;
  cs_stat_t css[100];
  int max;
  int i;

  List_start(callg->nodes);
  while ((node = List_next(callg->nodes)))
    {
      if (!node->fninfo->has_been_called)
	continue;
      nodes++;

      max = 0;
      List_start(node->callee_edges);
      while ((edge = List_next(node->callee_edges)))
	{
	  if (IPA_FLAG_ISSET(edge->flags, IPA_CALLG_EDGE_FLAGS_INDIRECT))
	    {
	      ind_edges++;
	      for (i=0; i<max; i++)
		{
		  if (css[i].cs == edge->caller_cs)
		    break;
		}
	      if (i == max)
		{
		  assert(max < 99);
		  max++;
		  css[i].cs = edge->caller_cs;
		  css[i].cnt = 0;
		}
	      css[i].cnt++;
	    }
	  else
	    dir_edges++;
	}

      for (i=0; i<max; i++)
	{
	  printf("IND CALLSITE %s %d\n",
		 node->fninfo->func_name,
		 css[i].cnt);
	}

      List_start(node->fninfo->callsites);
      while ((cs = List_next(node->fninfo->callsites)))
	{
	  if (cs->indirect)
	    ind_callsites++;
	  else
	    dir_callsites++;	    
	}
    }  

  printf("CALLG: nodes     %d\n", nodes);
  printf("CALLG: calls     %d %d\n", dir_edges, ind_edges);
  printf("CALLG: callsites %d %d\n", dir_callsites, ind_callsites);
#endif
}
