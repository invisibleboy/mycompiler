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
 *      File:    pipa_print_graph.c
 *      Author:  Erik Nystrom
 *      Copyright (c) 2003  Erik Nystrom, Wen-mei Hwu
 *              and The Board of Trustees of the University of Illinois.
 *              All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#include <pipa_print_graph.h>
#include <pipa_consg.h>
#include <pipa_callgraph.h>


/*************************************************************************
 * Text/Binary Raw File
 *************************************************************************/
IPA_cgraph_node_t*
IPA_read_cgraph_node(FILE *file, IPA_cgraph_t *cg);

#define READ_CONSGRAPH  1
#define READ_CALLGRAPH  2

IPA_cgraph_node_t*
IPA_cg_read_node(FILE *file, 
		 IPA_prog_info_t *prog_info, 
		 IPA_cgraph_t *consg)
{
  IPA_symbol_info_t *syminfo;
  IPA_cgraph_node_t *node;
  IPA_funcsymbol_info_t *fninfo;
  int var_id, offset = 0, version, size, flags;
  int cnt;

  fninfo = NULL;

  cnt = fscanf (file, "%d,%d,%d,%d.",
		&var_id,
		&version,
		&size,
		&flags);
  if (cnt == 0)
    return NULL;
  assert(cnt == 5);      
  
  if (offset != 0)
    {
      syminfo = IPA_symbol_find_by_id (prog_info, var_id);
      size = IPA_Pcode_sizeof(prog_info, syminfo->type_key);
    }
  
  if (consg)
    {
      node = IPA_consg_ensure_node (consg, var_id, version, 
				    size, NULL, 0);
      assert(var_id <= prog_info->max_var_id);
      prog_info->node_loc[var_id] = consg->data.fninfo;
    }
  else
    {
      fninfo = prog_info->node_loc[var_id];
      assert(fninfo->consg);
      node = IPA_consg_ensure_node (fninfo->consg, var_id, 
				    version, size, NULL, 0);
    }
  node->flags |= flags;

  return node;
}

void
IPA_cg_node_print (FILE * file, IPA_cgraph_node_t * node, int mode)
{
  if (mode == IPA_PRINT_ASCI)
    {
      char *name;
      
      if (node->data.syminfo)
	name = node->data.syminfo->symbol_name;
      else
	name = "???";
      
      fprintf (file, "(%d.%s:%d",
	       node->data.var_id,
	       name,
	       node->data.version);
      fprintf (file, "<%d>",
	       node->data.var_size);
      fprintf (file, ") ");
    }
  else if (mode == IPA_PRINT_BINARY)
    {
      char *name;
      
      if (node->data.syminfo)
	name = node->data.syminfo->symbol_name;
      else
	name = "???";

      fprintf (file, "%d,%d,%d,%d.",
	       node->data.var_id,
	       node->data.version,
	       node->data.var_size,
	       node->flags);
    }
}

IPA_cgraph_edge_t *
IPA_cg_read_edge(FILE * file, IPA_prog_info_t *info)
{
  IPA_cgraph_edge_t *edge;
  IPA_cgraph_node_t *src_node;
  IPA_cgraph_node_t *dst_node;
  int target_offset, assign_size, source_offset, flags;
  int cnt;
  IPA_cgraph_edgelist_e edge_type;

  if (fscanf(file, "%d.", (int *)&edge_type) != 1)
    return NULL;

  dst_node = IPA_cg_read_node(file, info, NULL);
  cnt = fscanf(file,"%d,%d,%d,%d.",
	       &target_offset,
	       &assign_size,
	       &source_offset,
	       &flags);
  assert(cnt == 4);
  src_node = IPA_cg_read_node(file, info, NULL);
#if 0
  printf("EDGE: %d,%d,%d,%d\n",
	 target_offset, assign_size,
	 source_offset, flags);
#endif
  edge = IPA_consg_ensure_edge(edge_type, src_node, dst_node,
			       target_offset, assign_size, source_offset,
			       flags);

  return edge;
}


void
IPA_cg_print_edge(FILE * file, IPA_cgraph_edge_list_t *elist, 
		  IPA_cgraph_edge_t *edge, int mode)
{

  if (mode == IPA_PRINT_ASCI)
    {
      char *org;

      IPA_cg_node_print (file, edge->dst_elist->node, mode);

      org = IPA_cg_edge_flag_name(edge);
      fprintf(file," #%d,%d,%d,%s#",
	      edge->data.target_offset,
	      edge->data.assign_size,
	      edge->data.source_offset,
	      org);

      IPA_cg_node_print (file, edge->src_elist->node, mode);
    }
  else
    {
      fprintf(file,"%d.",elist->edge_type);
      IPA_cg_node_print (file, edge->dst_elist->node, mode);

      fprintf(file,"%d,%d,%d,%d.",
	      edge->data.target_offset,
	      edge->data.assign_size,
	      edge->data.source_offset,
	      edge->flags);

      IPA_cg_node_print (file, edge->src_elist->node, mode);
    }

  fprintf (file, ".\n");
}

IPA_cgraph_t *
IPA_cg_read_nodes(FILE *file, IPA_prog_info_t *info, 
		  IPA_funcsymbol_info_t *fninfo)
{
  IPA_cgraph_t *cg;
  IPA_cgraph_node_t *node;
  IPA_cgraph_node_t *cnode;

  assert(0);
  cg = IPA_cg_cgraph_new(fninfo);

  do
    {
      if (!(node = IPA_cg_read_node(file, info, cg)))
	break;
      do 
	{
	  if (!(cnode = IPA_cg_read_node(file, info, cg)))
	    break;
	  cnode->rep_child = node->rep_child;
	  cnode->rep_parent = node;
	  node->rep_child = cnode;
	}
      while(1);
      fscanf(file,".");
    }
  while (1);
  fscanf(file,".");

  return cg;
}

void
IPA_cg_func_print_nodes (FILE * file,
			 IPA_funcsymbol_info_t* fninfo,
			 IPA_cgraph_t * cgraph)
{
  IPA_cgraph_node_t *node;
  IPA_HTAB_ITER niter;

  if (fninfo == NULL)
    fprintf (file, "1.CALLGRAPH.\n");
  else
    fprintf (file, "1.%s.\n", fninfo->func_name);

  IPA_HTAB_START(niter, cgraph->nodes);
  IPA_HTAB_LOOP(niter)
    {
      node = IPA_HTAB_CUR(niter);
      if (node->rep_parent != node)
        continue;
      
      IPA_cg_node_print (file, node, IPA_PRINT_BINARY);
      for (node = node->rep_child; node; node = node->rep_child)
	IPA_cg_node_print (file, node, IPA_PRINT_BINARY);
      fprintf (file, ".\n");
    }
  fprintf (file, ".\n");
}


void
IPA_cg_read_edges (FILE * file, IPA_prog_info_t *info)
{
  do
    {
      if (!IPA_cg_read_edge(file, info))
	break;
      fscanf(file,".");
    }
  while (1);
}

void
IPA_cg_func_print_edges (FILE * file, IPA_cgraph_t * cgraph)
{
  IPA_cgraph_node_t   *node;
  IPA_HTAB_ITER niter;
  IPA_HTAB_ITER eiter;
  IPA_cgraph_edge_list_t *elist;
  IPA_cgraph_edge_t *edge;

  IPA_HTAB_START(niter, cgraph->nodes);
  IPA_HTAB_LOOP(niter)
    {
      node = IPA_HTAB_CUR(niter);
      if (IPA_cg_node_is_child (node))
        continue;
 
      for (elist = node->first_list; elist; elist = elist->nxt_list)
	{
	  IPA_HTAB_START(eiter, elist->out);
	  IPA_HTAB_LOOP(eiter)
	    {
	      edge = IPA_HTAB_CUR(eiter);
	      IPA_cg_print_edge(file, elist, edge, IPA_PRINT_BINARY);
	    }
	}
    }
}


void
IPA_cg_all_writefile (IPA_prog_info_t *prog_info, char *name)
{
  FILE *file;
  IPA_funcsymbol_info_t *fninfo;
  IPA_cgraph_t *consg;

  file = IPA_fopen(NULL, IPA_file_subdir, name, "bcg", "w");

  List_start(prog_info->fninfos);
  while ((fninfo = List_next(prog_info->fninfos)))
    {
      consg = fninfo->consg;
      if (!consg)
	continue;
      IPA_cg_func_print_nodes (file, fninfo, consg);
    }
  fprintf (file, ".\n");

  List_start(prog_info->fninfos);
  while ((fninfo = List_next(prog_info->fninfos)))
    {
      if (!fninfo->consg)
	continue;
      IPA_cg_func_print_edges (file, fninfo->consg);
    }
  fprintf (file, ".\n");

  fclose (file);
}


void
IPA_cg_consg_readfile(IPA_prog_info_t *info, char *name)
{
#if 0
  FILE *file;
  char buffer[256];

  file = IPA_fopen(NULL, IPA_file_subdir, name, "bcg", "r");
  if (!file)
    return;

  info->node_loc = calloc(info->max_var_id+1, 
			  sizeof(IPA_funcsymbol_info_t *));

  do 
    {
      IPA_funcsymbol_info_t* fninfo;
      int dummy = 0;
      
      if (fscanf(file, "%d.", &dummy) == 0)
	break;
      assert(dummy == 1);

      assert(fscanf (file, "%[A-Za-z0-9_].", buffer) == 1);
      assert(strlen(buffer) < 256);
	
      fninfo = IPA_funcsymbol_find(info, buffer);
      fninfo->consg = IPA_cg_read_nodes(file, info, fninfo);
    }
  while (1);
  fscanf(file,".");

  IPA_cg_read_edges(file, info);
  
  fclose(file);
#else
  assert(0);
#endif
}


void
IPA_cg_writefile (IPA_cgraph_t *consg, char *name)
{
  FILE *file;

  file = IPA_fopen(NULL, IPA_file_subdir, name, "bcg", "w");

  IPA_cg_func_print_nodes (file, consg->data.fninfo, 
			   consg);
  fprintf (file, ".\n");

  IPA_cg_func_print_edges (file, consg);
  fprintf (file, ".\n");
  
  fclose (file);
}

void
IPA_callg_writefile (IPA_callg_t *callg, char *name)
{
  FILE *file;

  file = IPA_fopen(NULL, IPA_file_subdir, name, "bcg", "w");

  assert(0);

  fclose (file);
}

void
IPA_callg_readfile(IPA_prog_info_t *info, char *name)
{
  FILE *file;

  file = IPA_fopen(NULL, IPA_file_subdir, name, "bcg", "r");
  if (!file)
    return;

  assert(0);

  fclose(file);
}




/*************************************************************************
 * DaVinci Interfaces
 *************************************************************************/

#define IPA_CG_DVPRINT_VISITED IPA_CG_NODE_FLAGS_GENERIC1

void
IPA_cg_DVname_node(IPA_cgraph_node_t * node, char *node_name)
{
  int p;

  char *name;
	  
  if (node->data.syminfo)
    name = node->data.syminfo->symbol_name;
  else
    name = "???";
  
  p = sprintf (node_name, "[%d.%s:%d",
	       node->data.var_id,
	       name,
	       node->data.version);
  p += sprintf(node_name+p,"<%d>",
	       (int)node->data.var_size);
  p += sprintf(node_name+p,"]");
}

char *
IPA_cg_DVedge_color(IPA_cgraph_edgelist_e edge_type)
{
  char *color = NULL;

  switch (edge_type)
    {
    case ASSIGN_ADDR:
      color = "red";
      break;
    case ASSIGN:
      color = "black";
      break;
    case DEREF_ASSIGN:
      color = "blue";
      break;
    case ASSIGN_DEREF:
      color = "green";
      break;
    case SKEW:
      color = "brown";
      break;
    default:
      I_punt ("IPA_DVedge_color: unsupported edge type\n");
    }

  return color;
}

char *
IPA_cg_DVnode_color(IPA_cgraph_node_t *node)
{
  char *color;

  if (IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_FUNC))
    color = "red"; 
  else if (IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_GLOBAL))
    color = "lightblue";
  else if (IPA_FLAG_ISSET(node->flags, (IPA_CG_NODE_FLAGS_PARAM |
					IPA_CG_NODE_FLAGS_RETURN)))
    color = "#40C040"; /* green */
  else if (IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_HEAP))
    color = "#D040D0"; 
  else if (IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_CALLEE))
    color = "orange"; 
  else if (IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_SUMMARY))
    color = "white"; 
  else
    color = "lightgrey";
  
  return color;
}

char *
IPA_cg_DVnode_border(IPA_cgraph_t *cgraph, IPA_cgraph_node_t *node)
{
  char *border;

  /* Is this a inter-graph node */
  if (node->cgraph != cgraph)
    border = "double";
  else if (IPA_FLAG_ISSET (node->flags, IPA_CG_NODE_FLAGS_ESCLOCAL))
    border = "none";
  else
    border = "single";

  return border;
}

static List
IPA_cg_DVprint_node (FILE * file, IPA_cgraph_t *cgraph, List inlist,
		     IPA_cgraph_node_t * node, int valid_edges)
{
  static int id = 0;
  char node_name[256];
  char *color, *go, *border, *pattern;
  IPA_cgraph_edge_t *edge;
  IPA_cgraph_edge_list_t *edge_list;
  IPA_cgraph_node_t *rep_node;
  IPA_HTAB_ITER eiter;
  int first;

  IPA_cg_DVname_node(node, node_name);
  if (IPA_FLAG_ISSET (node->flags, IPA_CG_DVPRINT_VISITED))
    {
      fprintf (file, "r(\"%s\")", node_name);
      return inlist;
    }
  IPA_FLAG_SET (node->flags, IPA_CG_DVPRINT_VISITED);

#if 0
  IPA_consg_summarize_node_color(node, &color, &go);
#else
  go = "box";
  color = IPA_cg_DVnode_color(node);
#endif

  /* Is this a inter-graph node */
  border = IPA_cg_DVnode_border(cgraph, node);

  fprintf (file, "l(\"%s\",", node_name);
  fprintf (file, "n(\"anything\", [a(\"OBJECT\", \"%s\")", node_name);
  fprintf (file, ",a(\"COLOR\", \"%s\"),a(\"_GO\", \"%s\"),a(\"BORDER\", \"%s\")],",
	   color, go, border);

  fprintf (file, "[");

  first = 1;
  
  for (edge_list = node->first_list; edge_list;
       edge_list = edge_list->nxt_list)
    {
      if (!IPA_edge_list_valid (edge_list, valid_edges))
	continue;

      switch (edge_list->edge_type)
        {
        case ASSIGN_ADDR:
          color = "red";
          break;
        case ASSIGN:
          color = "black";
          break;
        case DEREF_ASSIGN:
          color = "blue";
          break;
        case ASSIGN_DEREF:
          color = "green";
          break;
	case SKEW:
	  color = "brown";
	  break;
        default:
          I_punt ("IPA_consg_print_DVnode: unsupported edge type\n");
        }

      IPA_HTAB_START(eiter, edge_list->out);
      IPA_HTAB_LOOP(eiter)
	{
	  edge = IPA_HTAB_CUR(eiter);
#if 1
	  /* Only allow inter-graph dsts if node is non-intergraph */
      	  if (node->cgraph != cgraph &&
	      edge->dst_elist->node->cgraph != cgraph)
	    continue;
#endif
          if (!first)
            fprintf (file, ",");
          first = 0;

	  if (IPA_FLAG_ISSET(edge->flags, IPA_CG_EDGE_FLAGS_DN))
	    pattern = "dotted";
	  else if (IPA_FLAG_ISSET(edge->flags, IPA_CG_EDGE_FLAGS_UP))
	    pattern = "dashed";
	  else
	    pattern = "solid";

          fprintf (file, "e(\"anything\",[a(\"OBJECT\", \"E%d\")", id++);
          fprintf (file, ",a(\"EDGECOLOR\", \"%s\")", color);
	  fprintf (file, ",a(\"EDGEPATTERN\", \"%s\")", pattern);
          fprintf (file, "],");

          inlist = IPA_cg_DVprint_node (file, cgraph, inlist,
					edge->dst_elist->node, 
					valid_edges);

          fprintf (file, ")");
        }

      if (inlist != (void*)(-1) &&
	  node->cgraph == cgraph)
	{
	  IPA_HTAB_START(eiter, edge_list->in);
	  IPA_HTAB_LOOP(eiter)
	    {
	      edge = IPA_HTAB_CUR(eiter);
	      if (edge->src_elist->node->cgraph == cgraph)
		continue;
	      inlist = List_insert_last(inlist, edge->src_elist->node);
	    }
	}
    }

  if (!IPA_cg_node_is_child (node))
    {
      for (rep_node = node->rep_child; rep_node;
           rep_node = rep_node->rep_child)
        {
          if (!first)
            fprintf (file, ",");
          first = 0;
          color = "yellow";

          fprintf (file, "e(\"anything\",[a(\"OBJECT\", \"E%d\")", id++);
          fprintf (file, ",a(\"EDGECOLOR\", \"%s\")", color);
          fprintf (file, "],");

          inlist = IPA_cg_DVprint_node (file, cgraph, inlist,
					rep_node, valid_edges);

          fprintf (file, ")");
        }
    }

  fprintf (file, "]))\n");
  return inlist;
}

void
IPA_cg_DVprint (IPA_cgraph_t * cgraph, char *name, int valid_edges)
{
  FILE *file;
  IPA_cgraph_node_t *node;
  IPA_cgraph_node_t *tmp_node;
  int first;
  IPA_cgraph_edge_t      *edge;
  IPA_cgraph_edge_list_t *edge_list;
  IPA_HTAB_ITER niter;
  IPA_HTAB_ITER eiter;
  List inlist;

  file = IPA_fopen(NULL, IPA_file_subdir, name, "graph", "w");

  IPA_HTAB_START(niter, cgraph->nodes);
  IPA_HTAB_LOOP(niter)
    {
      node = IPA_HTAB_CUR(niter);
      IPA_FLAG_CLR (node->flags, IPA_CG_DVPRINT_VISITED);

      /* Clear incoming nodes to cover unrepresented 
	 inter graph nodes (i.e. params) */
      for (edge_list = node->first_list; edge_list;
	   edge_list = edge_list->nxt_list)
	{
	  IPA_HTAB_START(eiter, edge_list->in);
	  IPA_HTAB_LOOP(eiter)
	    {
	      edge = IPA_HTAB_CUR(eiter);
	      for (tmp_node = edge->src_elist->node; tmp_node;
		   tmp_node = tmp_node->rep_child)
		{
		  IPA_FLAG_CLR (tmp_node->flags, 
				IPA_CG_DVPRINT_VISITED);
		}
	    }
	  
	  IPA_HTAB_START(eiter, edge_list->out);
	  IPA_HTAB_LOOP(eiter)
	    {
	      edge = IPA_HTAB_CUR(eiter);
	      for (tmp_node = edge->dst_elist->node; tmp_node;
		   tmp_node = tmp_node->rep_child)
		{
		  IPA_FLAG_CLR (tmp_node->flags, 
				IPA_CG_DVPRINT_VISITED);
		}
	    }
	} /* edges */
    }

  fprintf (file, "[\n");

  first = 1;
  inlist = NULL;
  IPA_HTAB_START(niter, cgraph->nodes);
  IPA_HTAB_LOOP(niter)
    {
      node = IPA_HTAB_CUR(niter);
      if (IPA_FLAG_ISSET (node->flags, IPA_CG_DVPRINT_VISITED))
        continue;

      if (!first)
        fprintf (file, ",");
      first = 0;
      inlist = IPA_cg_DVprint_node (file, cgraph, inlist,
				    node, valid_edges);
    }

  List_start(inlist);
  while ((node = List_next(inlist)))
    {
      if (IPA_FLAG_ISSET (node->flags, IPA_CG_DVPRINT_VISITED))
        continue;
       fprintf (file, ",");
       IPA_cg_DVprint_node (file, cgraph, (void*)(-1),
			   node, valid_edges);
    }
  List_reset(inlist);

  fprintf (file, "]\n");
  fclose (file);

  /* Clear flags */
  IPA_cg_nodes_clr_flags (cgraph, (IPA_CG_DVPRINT_VISITED));
}






/*************************************************************************
 * Dot Interfaces
 *************************************************************************/

List
IPA_cg_DOT_addextfn(List sublist, IPA_cgraph_node_t *node)
{
  IPA_funcsymbol_info_t *fninfo;
  char *name, *fnname;
  List nlist;

  fninfo = node->cgraph->data.fninfo;
  if (fninfo)
    fnname = fninfo->func_name;
  else
    fnname = "???";
  
  List_start(sublist);
  while ((nlist = List_next(sublist)))
    {
      name = List_first(nlist);
      if (strcmp(name, fnname) == 0)
	{
	  goto SUB_LIST_EXISTS;
	}
    }
  
  nlist = List_insert_first(NULL, fnname);
  sublist = List_insert_last(sublist, nlist);
  
 SUB_LIST_EXISTS:
  if (!List_member(nlist, node))
    nlist = List_insert_last(nlist, node);
      
  return sublist;
}

void
IPA_cg_DOT_remextfn(List sublist)
{
  List nlist;

  List_start(sublist);
  while ((nlist = List_next(sublist)))
    {
      List_reset(nlist);
    }
  List_reset(sublist);
}

void
IPA_cg_DOT_name_node(IPA_cgraph_node_t * node, char *node_name)
{
  int p;

  char *name;
	
  if (node->data.syminfo)
    name = node->data.syminfo->symbol_name;
  else
    name = "???";
  
  if (node->data.in_k_cycle == 0)
    p = sprintf (node_name, "%d:%d\\n%s",
		 node->data.var_id,
		 node->data.version,
		 name);
  else
    p = sprintf (node_name, "%d:%d\\n[%d] %s",
		 node->data.var_id,
		 node->data.version,
		 node->data.in_k_cycle,
		 name);
}

char *
IPA_cg_DOTnode_color(IPA_cgraph_node_t *node)
{
  char *color;

  if (IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_FUNC))
    color = "red"; 
  else if (IPA_FLAG_ISSET(node->flags, (IPA_CG_NODE_FLAGS_PARAM |
					IPA_CG_NODE_FLAGS_RETURN)))
    color = "green";
  else if (IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_GLOBAL))
    color = "lightblue";
  else if (IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_HEAP))
    color = "purple"; 
  else if (IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_CALLEE))
    color = "orange"; 
  else if (IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_SUMMARY))
    color = "white"; 
  else
    color = "lightgrey";
  
  return color;
}

char *
IPA_cg_DOTnode_style(IPA_cgraph_node_t *node)
{
  char *style;

  if (IPA_FLAG_ISSET (node->flags, IPA_CG_NODE_FLAGS_ESCLOCAL))
    style = "none";
  else
    style = "filled";

  return style;
}

static void
IPA_cg_DOTprint_node (FILE * file, 
		      char *tab, IPA_cgraph_node_t * node)
{
  char *style;
  char label[256];
  char *shape;
  char *color;

  IPA_cg_DOT_name_node(node, label);
  shape = "oval";
  style = IPA_cg_DOTnode_style(node);
  color = IPA_cg_DOTnode_color(node);

  fprintf(file, "%s \"%d:%d\" [label=\"%s\",shape=%s,style=%s,color=%s,height=0.1,fontsize=9];\n",
	  tab,
	  node->data.var_id,
	  node->data.version,
	  label, shape, style, color);
}

static void
IPA_cg_DOTprint_edge (FILE * file, char *tab,
		      IPA_cgraph_edgelist_e etype,
		      IPA_cgraph_edge_t      *edge,
		      IPA_cgraph_node_t * srcnode,
		      IPA_cgraph_node_t * dstnode)
{
  char *color = NULL, *style = NULL, *expstr;

#if 0
  if (IPA_FLAG_ISSET(dge->flags, IPA_CG_EDGE_FLAGS_UD))
    return;
#endif
#if 0
  if (etype == ASSIGN_ADDR)
    {
      if (!((srcnode->data.var_id == 1242 && edge->data.source_offset == 0) ||
	    (dstnode->data.var_id == 15247)))
	return;
    }
#endif

  switch (etype)
    {
    case ASSIGN_ADDR:
      color = "red";
      style = "none";
      break;
    case ASSIGN:
      color = "black";
      style = "none";
      break;
    case DEREF_ASSIGN:
      color = "blue";
      style = "none";
      break;
    case ASSIGN_DEREF:
      color = "green";
      style = "none";
      break;
    case SKEW:
      color = "brown";
      style = "none";
      break;
    default:
      I_punt ("IPA_cg_DOTprint_edge: unsupported edge type\n");
    }

  if (IPA_FLAG_ISSET(edge->flags, IPA_CG_EDGE_FLAGS_GBL))
    {
    }
  if (IPA_FLAG_ISSET(edge->flags, IPA_CG_EDGE_FLAGS_DN))
    style = "dotted";
  else if (IPA_FLAG_ISSET(edge->flags, IPA_CG_EDGE_FLAGS_UP))
    style = "dashed";

  expstr = "";
  if (IPA_FLAG_ISSET(edge->flags, IPA_CG_EDGE_FLAGS_EXPLICIT))
    expstr = "E";

#if 0
  fprintf(file, "%s \"%d:%d\":o%d -> \"%d:%d\":o%d [color = %s, style=%s];\n",
	  tab,
	  srcnode->data.var_id,
	  srcnode->data.version,
	  srcnode->data.offset,
	  dstnode->data.var_id,
	  dstnode->data.version,
	  dstnode->data.offset,
	  color, style);
#endif
  if (edge->data.target_offset == 0 &&
      edge->data.source_offset == 0 
#if EDGE_HISTORY
      && edge->level == 0
#endif
      )
    {
      if (edge->data.target_stride != 0 ||
	  edge->data.source_stride != 0)
	{
	  fprintf(file, "%s \"%d:%d\" -> \"%d:%d\" [label=\"+%d%s+%d\", color = %s, style=%s];\n",
		  tab,
		  srcnode->data.var_id,
		  srcnode->data.version,
		  dstnode->data.var_id,
		  dstnode->data.version,
		  edge->data.target_stride,
		  expstr,
		  edge->data.source_stride,
		  color, style);
	}
      else if (edge->data.assign_size == 4)
	{
	  fprintf(file, "%s \"%d:%d\" -> \"%d:%d\" [label=\"%s\", color = %s, style=%s];\n",
		  tab,
		  srcnode->data.var_id,
		  srcnode->data.version,
		  dstnode->data.var_id,
		  dstnode->data.version,
		  expstr,
		  color, style);
	}
      else
	{
	  fprintf(file, "%s \"%d:%d\" -> \"%d:%d\" [label=\"%s%d\", color = %s, style=%s];\n",
		  tab,
		  srcnode->data.var_id,
		  srcnode->data.version,
		  dstnode->data.var_id,
		  dstnode->data.version,
		  expstr, 
		  edge->data.assign_size,
		  color, style);
	}
    }
  else 
    {
#if EDGE_HISTORY
      fprintf(file, "%s \"%d:%d\" -> \"%d:%d\" [label=\"%s<%d>%d:%d:%d\", color = %s, style=%s];\n",
	      tab,
	      srcnode->data.var_id,
	      srcnode->data.version,
	      dstnode->data.var_id,
	      dstnode->data.version,
	      expstr,
	      edge->level,
	      edge->data.target_offset,
	      edge->data.assign_size,
	      edge->data.source_offset,
	      color, style);
#else
      fprintf(file, "%s \"%d:%d\" -> \"%d:%d\" [label=\"%s%d:%d:%d\", color = %s, style=%s];\n",
	      tab,
	      srcnode->data.var_id,
	      srcnode->data.version,
	      dstnode->data.var_id,
	      dstnode->data.version,
	      expstr,
	      edge->data.target_offset,
	      edge->data.assign_size,
	      edge->data.source_offset,
	      color, style);
#endif
    }
}

int
IPA_cg_DOThasedge (IPA_cgraph_node_t *node)
{
  IPA_cgraph_edge_list_t *edge_list;
  if (node->rep_parent != node ||
      node->rep_child)
    return 1;
  for (edge_list = node->first_list; edge_list;
       edge_list = edge_list->nxt_list)
    {
      if (IPA_htab_size(edge_list->in) > 0 ||
	  IPA_htab_size(edge_list->out))
	return 1;
    }
  return 0;
}

void 
IPA_cg_DOTUnif (FILE *file, IPA_cgraph_node_t *node)
{
  IPA_cgraph_node_t *dstnode;

  if (node->rep_parent != node)
    return;

  for (dstnode = node->rep_child; dstnode; dstnode = dstnode->rep_child)
    {
      fprintf(file, "%s \"%d:%d\" -> \"%d:%d\" [color = %s];\n",
	      " ",
	      node->data.var_id,
	      node->data.version,
	      dstnode->data.var_id,
	      dstnode->data.version,
	      "yellow");
    }
}

void
IPA_cg_DOTprint (IPA_cgraph_t * cgraph, char *fname, int valid_edges)
{
  FILE *file;
  IPA_funcsymbol_info_t  *fninfo;
  IPA_cgraph_node_t      *node;
  IPA_cgraph_node_t      *tmp_node;
  IPA_HTAB_ITER niter;
  IPA_HTAB_ITER eiter;
  IPA_cgraph_edge_t      *edge;
  IPA_cgraph_edge_list_t *edge_list;
  List sublist, nlist;
  char *name;
  int cnt;

  /* JWP - Only write output for functions listed in the file "functions". */
  {
    static int fnlist_init = 1;
    if (fnlist_init)
      {
	IPA_init_func_names ();
	fnlist_init = 0;
      }
    
    if (!IPA_compare_func_name (fname)) return;
  }

  file = IPA_fopen(NULL, IPA_file_subdir, fname, "dot", "w");
  sublist = NULL;
  IPA_HTAB_START(niter, cgraph->nodes);
  IPA_HTAB_LOOP(niter)
    {
      node = IPA_HTAB_CUR(niter);
      if (node->rep_parent != node)
	sublist = IPA_cg_DOT_addextfn(sublist, 
				      IPA_cg_node_get_rep(node));

      if (node->cgraph == cgraph)
	{
	  /* Clear incoming nodes to cover unrepresented 
	     inter graph nodes (i.e. params) */
	  for (edge_list = node->first_list; edge_list;
	       edge_list = edge_list->nxt_list)
	    {
	      IPA_HTAB_START(eiter, edge_list->in);
	      IPA_HTAB_LOOP(eiter)
		{
		  edge = IPA_HTAB_CUR(eiter);
		  tmp_node = edge->src_elist->node;
		  if (tmp_node->cgraph != cgraph)
		    {
		      sublist = IPA_cg_DOT_addextfn(sublist, tmp_node);
		    }
		}
		  
	      IPA_HTAB_START(eiter, edge_list->out);
	      IPA_HTAB_LOOP(eiter)
		{
		  edge = IPA_HTAB_CUR(eiter);
		  tmp_node = edge->dst_elist->node;
		  if (tmp_node->cgraph != cgraph)
		    {
		      sublist = IPA_cg_DOT_addextfn(sublist, tmp_node);
		    }
		}
	    } /* edges */
	}
    }

  fprintf (file, "digraph G { \n");
  cnt = 0;

  fninfo = cgraph->data.fninfo;
  if (fninfo)
    name = fninfo->func_name;
  else
    name = "???";

  fprintf (file, "\n/*\n FUNCTION %s \n*/\n",name);
#if 0
  fprintf (file, "subgraph cluster%d { \n", cnt++);
  fprintf (file, "  label = \"%s\";\n", name);      
#endif

  IPA_HTAB_START(niter, cgraph->nodes);
  IPA_HTAB_LOOP(niter)
    {
      node = IPA_HTAB_CUR(niter);
      if (!IPA_cg_DOThasedge(node))
	continue;
      IPA_cg_DOTprint_node (file, "   ", node);
    }

#if 0
  fprintf (file, "}\n");
#endif

  List_start(sublist);
  while ((nlist = List_next(sublist)))
    {
      name = List_first(nlist);

      fprintf (file, "\n/*\n FUNCTION %s \n*/\n", name);
#if 0
      fprintf (file, "subgraph cluster%d { \n", cnt++);
      fprintf (file, "  label = \"%s\";\n", name);      
#endif

      while ((node = List_next(nlist)))
	{
	  if (!IPA_cg_DOThasedge(node))
	    continue;
	  IPA_cg_DOTprint_node (file, "   ", node);
	}

#if 0
      fprintf (file, "}\n");
#endif
    }


  fprintf (file, "\n/*\n ALL EDGES \n*/\n");
 
  IPA_HTAB_START(niter, cgraph->nodes);
  IPA_HTAB_LOOP(niter)
    {
      node = IPA_HTAB_CUR(niter);
      for (edge_list = node->first_list; edge_list;
	   edge_list = edge_list->nxt_list)
	{
	  IPA_HTAB_START(eiter, edge_list->in);
	  IPA_HTAB_LOOP(eiter)
	    {
	      edge = IPA_HTAB_CUR(eiter);
	      tmp_node = edge->src_elist->node;
	      if (tmp_node->cgraph != cgraph)
		{
		  IPA_cg_DOTprint_edge(file, " ", 
				       edge_list->edge_type,
				       edge,
				       tmp_node, node);
		}
	    }
	  
	  IPA_HTAB_START(eiter, edge_list->out);
	  IPA_HTAB_LOOP(eiter)
	    {
	      edge = IPA_HTAB_CUR(eiter);
	      tmp_node = edge->dst_elist->node;

	      IPA_cg_DOTprint_edge(file, " ", 
				   edge_list->edge_type,
				   edge,
				   node, tmp_node);
	    }
	}
      
      IPA_cg_DOTUnif (file, node);
    }

  List_start(sublist);
  while ((nlist = List_next(sublist)))
    {
      name = List_first(nlist);
      if (fninfo && !strcmp(name,fninfo->func_name))
	continue;
      fprintf (file, "\n/*\n FUNCTION %s \n*/\n", name);
      while ((node = List_next(nlist)))
	{
	  IPA_cg_DOTUnif (file, node);
	}
    }
  
  fprintf (file, "}\n");

  IPA_cg_DOT_remextfn(sublist);
  fclose (file);
}





#if EDGE_HISTORY 

void
IPA_print_DOThistory(char *fname,
		     IPA_cgraph_edgelist_e edge_type,
		     IPA_cgraph_node_t *src_node,
		     int s_offset,
		     IPA_cgraph_node_t *dst_node,
		     int d_offset,
		     int size,
		     int max_level)
{
  FILE *file;
  List node_list = NULL;
  List edge_list = NULL;
  List work_list = NULL;
  IPA_cgraph_edge_t *edge;
  IPA_cgraph_edge_t *root_edge;
  IPA_cgraph_node_t *node;
  IPA_cgraph_edge_data_t edata;
  int level = 0;

  edata.source_offset = s_offset;
  edata.target_offset = d_offset;
  edata.assign_size = size;
  root_edge = IPA_cg_edge_find(src_node, dst_node, edge_type, &edata);
  if (root_edge == NULL)
    return;
  
  root_edge->level = 1;
  work_list = List_insert_last(work_list, root_edge);
  edge_list = List_insert_last(edge_list, root_edge);
  printf("ROOT: %d %d\n",src_node->data.var_id, dst_node->data.var_id);
  while ((edge = List_first(work_list)))
    {
      work_list = List_delete_current(work_list);

      if (!List_member(node_list, edge->src_elist->node))
	node_list = List_insert_last(node_list, edge->src_elist->node);
      if (!List_member(node_list, edge->dst_elist->node))
	node_list = List_insert_last(node_list, edge->dst_elist->node);

      assert(edge->level);
      level++;
      
      if (edge->e1 && !List_member(edge_list, edge->e1))
	{
	  edge_list = List_insert_last(edge_list, edge->e1);
	  edge->e1->level = level;
	  if (level <= max_level)
	    work_list = List_insert_last(work_list, edge->e1);
	}
      if (edge->e2 && !List_member(edge_list, edge->e2))
	{
	  edge_list = List_insert_last(edge_list, edge->e2);
	  edge->e2->level = level;
	  if (level <= max_level)
	    work_list = List_insert_last(work_list, edge->e2);	  
	}
    }
  List_reset(work_list);

  file = IPA_fopen(NULL, IPA_file_subdir, fname, "dot", "w");

  fprintf (file, "digraph G { \n");

  List_start(node_list);
  while ((node = List_next(node_list)))
    {
      IPA_cg_DOTprint_node (file, "   ", node);
    }

  fprintf (file, "\n/*\n ALL EDGES \n*/\n");

  List_start(edge_list);
  while ((edge = List_next(edge_list)))
    {
      printf("EDGE: %d %d\n",edge->src_elist->node->data.var_id, edge->dst_elist->node->data.var_id);
      IPA_cg_DOTprint_edge(file, " ", 
			   edge->src_elist->edge_type,
			   edge,
			   edge->src_elist->node, 
			   edge->dst_elist->node);
    }

  fprintf (file, "}\n");
  fclose(file);

  List_reset(node_list);
  List_reset(edge_list);
}

#endif
