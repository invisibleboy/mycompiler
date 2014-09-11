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
 *      File:    pipa_consg_construct.c
 *      Author:  Erik Nystrom
 *      Copyright (c) 2003  Erik Nystrom, Wen-mei Hwu
 *              and The Board of Trustees of the University of Illinois.
 *              All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#include "pipa_pcode2pointsto.h"
#include "pipa_consg_construct.h"
#include "pipa_callgraph.h"

char *IPA_bcg_status_string[] = {[B_VAR] "VAR", 
				 [B_ADDR] "ADDR",
				 [B_ASSIGN] "ASSIGN",
				 [B_DEREF] "DEREF"};

int
IPA_bcg_follow_edge(IPA_prog_info_t * info, buildcg_t *bcg, 
		    IPA_cgraph_edgelist_e edge_type,
		    int t_off, int size, int s_off);

void
print_bcg(buildcg_t *bcg)
{
  printf("%s %d.%d offset %d skew %d ",
	 IPA_bcg_status_string[bcg->status],
	 bcg->node->data.var_id,
	 bcg->node->data.version,
	 bcg->offset,
	 bcg->skew);
}

IPA_symbol_info_t *
IPA_new_tmp_var(IPA_prog_info_t * info, 
		IPA_funcsymbol_info_t * fninfo,
		int kind)
{
  IPA_symbol_info_t *syminfo;
  char name[50];
  Key type_key;
  Key sym_key;

  /* No type for now */
  type_key = IPA_symbol_tmptypekey();

  /* Make a new tmp var key */ 
  sym_key = IPA_symbol_tmpvarkey();

  sprintf(name,"__ITV__%d", sym_key.sym);

  syminfo = IPA_symbol_add (info, fninfo,
			    name, sym_key,
			    (IPA_VAR_KIND_TEMP | kind), type_key);

  DEBUG_IPA(3, printf("  CONSTRUCTTEMP: %d %s\n",
		      syminfo->id, 
		      syminfo->symbol_name); );

  return syminfo;
}

static void
new_frontier(buildcg_t *bcg, IPA_cgraph_node_t *node, int del_frontier)
{
  if (bcg->node_frontier && del_frontier)
    {
      List_reset(bcg->node_frontier);
    }
  bcg->node_frontier = NULL;
  bcg->node = node;
  if (node)
    {
      bcg->node_frontier = List_insert_first(bcg->node_frontier, node);
    }
}

static void
add_frontier(buildcg_t *bcg, IPA_cgraph_node_t *node)
{
  assert(node);
  bcg->node = node;
  if (!List_member(bcg->node_frontier, node))
    bcg->node_frontier = List_insert_first(bcg->node_frontier, node);
} 

static void
IPA_bcg_tmp_node(IPA_prog_info_t * info, buildcg_t *bcg)
{
  IPA_symbol_info_t *syminfo;
  IPA_cgraph_node_t *node;

  syminfo = IPA_new_tmp_var(info, bcg->fninfo, bcg->kind);

  /* Make a tmp node */
  node = IPA_consg_ensure_node(bcg->consg, syminfo->id, 1,
			       IPA_Pcode_sizeof(info, syminfo->type_key),
			       syminfo,0);
  new_frontier(bcg, node, 1);
  bcg->tmp_nodes = List_insert_last(bcg->tmp_nodes, bcg->node);
}


static void
IPA_bcg_realize_edge(IPA_prog_info_t * info, buildcg_t *bcg, 
		     IPA_cgraph_edgelist_e edge_type,
		     int t_off, int size, int s_off)
{
  /* This routine should only be used to create
     - intermediate nodes to hold SKEWed values 
     - intermediate deref values (ex. "*v" of "a = **v")
         One exception: for safer library code, 
             some intermediate "*v" nodes are set to
             the largest type size.
     Any non-pointer sized edges should be created 
        through the IPA_bcg_assign routine.
  */
  assert(edge_type == SKEW || size ==  IPA_POINTER_SIZE
	 || size == IPA_max_type_size);

  /* Look for existing node first, 
     if not found create one (but only of in BUILD mode)
  */
  if (!IPA_bcg_follow_edge(info, bcg, edge_type, t_off, size, s_off))
    {
      IPA_cgraph_node_t *base_node;
      IPA_cgraph_edge_t *edge;

      assert(bcg->mode == B_BUILD);
      assert(bcg->node);

      base_node = bcg->node;
      IPA_bcg_tmp_node(info, bcg);
      
      edge = IPA_consg_ensure_edge (edge_type, base_node, bcg->node,
				    t_off, size, s_off,
				    (IPA_CG_EDGE_FLAGS_EXPLICIT |
				     IPA_CG_EDGE_FLAGS_HZ));
      if (edge_type == ASSIGN_ADDR && bcg->isarray)
	{
	  IPA_FLAG_SET(edge->flags, IPA_CG_EDGE_FLAGS_ARRAY);
	}
      
      bcg->offset = 0;
      bcg->skew = 0;
    }
}

int
IPA_bcg_follow_edge(IPA_prog_info_t * info, buildcg_t *bcg, 
		    IPA_cgraph_edgelist_e edge_type,
		    int t_off, int size, int s_off)
{
  IPA_cgraph_node_t *search_node = NULL;
  IPA_cgraph_edge_list_t *elist;
  IPA_cgraph_edge_t *edge;
  IPA_cgraph_node_t *node;
  List  frontier;
  IPA_HTAB_ITER eiter;
  int i = 0;
  int first_time;

  frontier = bcg->node_frontier;
  first_time = 1;
  List_start(frontier);
  while ((node = List_next(frontier)))
    {
      i++;

      /* Special case */
      if (bcg->mode == B_FOLLOW && edge_type == SKEW &&
	  node->data.in_k_cycle != 0 &&
	  ((size % node->data.in_k_cycle) == 0))
	{
	  /* Cyclic SKEW, this node represents any mulitiple
	     of this SKEW */
	  if (first_time)
	    {
	      /* Only clear the frontier if there is at least one found
	       */
	      new_frontier(bcg, NULL, 0);
	      first_time = 0;
	      assert(bcg->node_frontier == NULL);
	    }
#if 0
	  printf("node %d.%d : Skew %d k_cycle %d \n",
		 node->data.var_id, node->data.version,
		 size, node->data.in_k_cycle);
#endif
	  add_frontier(bcg, node);
	}

      /* Normal case */
      elist = IPA_cg_edge_list_find(node, edge_type);
      if (elist)
	{
	  IPA_HTAB_START(eiter, elist->out);
	  IPA_HTAB_LOOP(eiter)
	    {
	      int t_mod, s_mod;
	      edge = IPA_HTAB_CUR(eiter);

	      /* Can only reuse temporaries for building consg */
	      if (bcg->mode == B_BUILD && 
		  IPA_FLAG_ISCLR(edge->dst_elist->node->data.syminfo->kind, 
				 IPA_VAR_KIND_TEMP))
		continue;

	      /* Offsets must match. 
		 SKEW: size is the skew size so they must be identical
		 OTHER: existing size must subsume (>=) requested size
	      */
	      t_mod = edge->dst_elist->node->data.mod;
	      s_mod = edge->src_elist->node->data.mod;
	      if (edge->data.target_offset == (t_off % t_mod) &&
		  edge->data.source_offset == (s_off % s_mod) &&
		  ((edge_type == SKEW && edge->data.assign_size == size) ||
		   (edge_type != SKEW && edge->data.assign_size >= size)) &&
		  (!IPA_FLAG_ISSET(edge->dst_elist->node->data.syminfo->kind, 
				   IPA_VAR_KIND_RETURN))
		  )
		{
#if 0
		  printf("[%d] %p -> %p\n",
			 i, search_node,
			 edge->dst_elist->node); 
		  fflush(stdout);
#endif
		  if (first_time)
		    {
		      /* Only clear the frontier if there is at least one found
		       */
		      new_frontier(bcg, NULL, 0);
		      first_time = 0;
		      assert(bcg->node_frontier == NULL);
		    }
		  search_node = edge->dst_elist->node;
		  add_frontier(bcg, search_node);
		}
	    }
	}
    }
  /*
    If no edge found:
    BUILD: This will be ok as one will be created
    FOLLOW: Can't create edges in follow mode so this is an error
   */
  if (first_time)
    return 0;

  bcg->offset = 0;
  bcg->skew = 0;  

  return 1;
}


static int 
IPA_Pcode_true_sizeof (IPA_prog_info_t *info,
		       Key type_key)
{
  int size;

  /* Temporary variable type created for the consg
     but unknown to real symbol table */
  if (type_key.file == IPA_TEMPTYPE_FILEID)
    return IPA_POINTER_SIZE;

  assert(P_ValidKey(type_key));

#if 0
  /* This is ok as long as IPA_Pcode_sizeof is never
   * used for struct layout (which shouldn't occur)
   *
   * The goal is to return the size of the underlying object
   *   past any array portions. This isn't a default 
   *   pointer-sized situation because we want a struct array
   *   to end up with a node the size of the struct.
   */
  while (PST_IsArrayType(info->symboltable, type_key))
    {
      type_key = PST_GetTypeType(info->symboltable, type_key); 
    }
#endif
  
  if (PST_IsFunctionType(info->symboltable, type_key))
    size = IPA_POINTER_SIZE;
  else
    size = PST_GetTypeSize(info->symboltable, type_key);

  if (size <= 0)
    {
#if 0
      _Dcl dcl;
      TypeDcl tdcl;

      fprintf(info->errfile,"### TYPE SIZE MUST BE > 0 forcing to %d\n",
	      IPA_POINTER_SIZE);

      tdcl = PST_GetTypeDclEntry(info->symboltable, type_key);
      dcl.type = TT_TYPE;
      dcl.ptr.typeDcl = tdcl;
      P_write_type_dcl(info->errfile, &dcl, 5, NULL);
      fprintf(info->errfile,"\n");
#endif
      return IPA_POINTER_SIZE;
    }
  
  return size;
}


buildcg_t*
IPA_buildcg_start(IPA_prog_info_t * info,
		  IPA_funcsymbol_info_t * fninfo,
		  int var_id,
		  int var_subscr,
		  buildcg_mode_t mode)
{
  buildcg_t *bcg;
  IPA_symbol_info_t *syminfo;
  IPA_cgraph_node_t *node;
  int size;

  bcg = calloc(1,sizeof(buildcg_t));

  syminfo = IPA_symbol_find_by_id(info, var_id);
  assert(syminfo);

  /* JWP -  1/6/05  update syminfo->max_version */
  if (syminfo->max_version <= var_subscr)
    syminfo->max_version = var_subscr + 1;

  /* Create globals in the global consg and
   *  locals, heap in the particular function
   */
  if (IPA_FLAG_ISSET(syminfo->kind, IPA_VAR_KIND_GLOBAL))
    {
      fninfo = info->globals;
    }

  bcg->fninfo = fninfo;
  if (!fninfo->consg)
    {
      /* During the dep gen process AFTER the analysis the CALLGRAPH
       *   can have merged SCCs. Find the parent.
       */
      IPA_callg_node_t *cn;
      assert(bcg->mode == B_FOLLOW);
      cn = IPA_callg_node_get_rep(fninfo->call_node);
      bcg->consg = cn->fninfo->consg;
    }
  else
    {
      bcg->consg = fninfo->consg;
    }
  assert(bcg->consg);
  bcg->mode = mode;

#if 0
  printf("CONNODE [%s:%d-%s]",
	 syminfo->symbol_name,
	 var_id,
	 bcg->consg->data.fninfo->func_name);
#endif

  if (IPA_field_safety != IPA_SAFETY_LEVEL0)
    size = IPA_Pcode_true_sizeof(info, syminfo->type_key);
  else
    size = IPA_Pcode_sizeof(info, syminfo->type_key);

  /* This will create node even in FOLLOW mode but such nodes
   *   should be uninteresting w/r to pointers
   */
  node = IPA_consg_ensure_node(bcg->consg, var_id, var_subscr, size,
			       syminfo, 0);

  if (IPA_field_safety != IPA_SAFETY_LEVEL0)
    {
      size = IPA_Pcode_sizeof(info, syminfo->type_key);
      if (size < node->data.mod)
	node->data.mod = size;
    }

  if (node->rep_parent != node)
    {
      /* The constraint graph may have
       * merged var nodes. Find the parent.
       */
      assert(bcg->mode == B_FOLLOW);
      node = IPA_cg_node_get_rep(node);
    }
  assert(node);
  new_frontier(bcg, node, 0);

  if (IPA_FLAG_ISSET(syminfo->kind, IPA_VAR_KIND_GLOBAL))
    IPA_FLAG_SET(bcg->kind, IPA_VAR_KIND_GLOBAL);

  bcg->status = B_VAR;

#if 0
  printf(" %s\n",
	 bcg->node->data.syminfo->fninfo->func_name);
#endif

  return bcg;
}


void
IPA_buildcg_free(buildcg_t *bcg)
{
  if (!bcg)
    return;
  bcg->status = -1;
  List_reset(bcg->tmp_nodes);
  List_reset(bcg->node_frontier);
  bcg->tmp_nodes = NULL;
  bcg->node_frontier = NULL;
  free(bcg);
}


buildcg_t *
IPA_buildcg_copy(buildcg_t *bcg)
{
  buildcg_t *cbcg;
  IPA_cgraph_node_t *node;

  if (!bcg)
    return NULL;

  cbcg = calloc(1,sizeof(buildcg_t));

  cbcg->fninfo = bcg->fninfo;
  cbcg->consg = bcg->consg;
  cbcg->node = bcg->node;
  cbcg->kind = bcg->kind;
  cbcg->offset = bcg->offset;
  cbcg->skew = bcg->skew;
  cbcg->status = bcg->status;
  cbcg->isarray = bcg->isarray;
  cbcg->mode = bcg->mode;

  cbcg->node_frontier = NULL;
  List_start(bcg->node_frontier);
  while ((node = List_next(bcg->node_frontier)))
    cbcg->node_frontier = List_insert_last(cbcg->node_frontier,
					   node);

  cbcg->tmp_nodes = NULL;
  List_start(bcg->tmp_nodes);
  while ((node = List_next(bcg->tmp_nodes)))
    cbcg->tmp_nodes = List_insert_last(cbcg->tmp_nodes,
				       node);

  return cbcg;
}


void
IPA_buildcg_kill(buildcg_t *bcg)
{
  IPA_cgraph_node_t *node;

  assert(bcg);

  List_start(bcg->tmp_nodes);
  while ((node = List_next(bcg->tmp_nodes)))
    {
      IPA_cg_node_delete(node);
    }

  IPA_buildcg_free(bcg);
}


void
IPA_bcg_addrof(IPA_prog_info_t * info, buildcg_t *bcg)
{
  switch(bcg->status)
    {
    case B_VAR:
    case B_ADDR:
      bcg->status = B_ADDR; 
      break;
    case B_ASSIGN:
      assert(0); 
      break;
    case B_DEREF:
      bcg->status = B_ASSIGN;
      break;
    default:
      assert(0);
    }
}



/* THIS ROUTINE IS FOR THE dot AND arrow OPERATORS */
void
IPA_bcg_offset(IPA_prog_info_t * info, buildcg_t *bcg, int offset)
{
  switch (IPA_field_option)
    {
    case IPA_FIELD_INDEPENDENT:
      return;
      break;
    case IPA_FIELD_DEPENDENT_VARIABLE_SIZE:
      break;
    default:
      assert(0);
      break;
    }

  switch(bcg->status)
    {
    case B_VAR:
      /* Don't add in offset for nodes treated
	 field insensitively. Skewing does not
	 need this check because the solver should
	 take care of it */
      if (!IPA_FLAG_ISSET(bcg->node->flags,
			  IPA_CG_NODE_FLAGS_NOFIELD))
	bcg->offset = bcg->offset + offset;
      break;
    case B_ADDR:  
      assert(0);
      break;
    case B_ASSIGN:
      assert(0); 
      break;
    case B_DEREF:
      bcg->skew = bcg->skew + offset;
      break;
    default:
      assert(0);
    }
}

void
IPA_bcg_reduce(IPA_prog_info_t * info, buildcg_t *bcg)
{
  if (bcg->skew != 0)
    {
      IPA_bcg_realize_edge(info, bcg, SKEW, 
			   0, bcg->skew, bcg->offset);
    }
  if (bcg->status == B_DEREF)
    {
      IPA_bcg_realize_edge(info, bcg, ASSIGN_DEREF, 
			   0, IPA_POINTER_SIZE, bcg->offset);
      bcg->status = B_ASSIGN;
    }
}

/* THIS ROUTINE IS FOR EXPLICIT ADDITION */
void
IPA_bcg_add(IPA_prog_info_t * info, buildcg_t *bcg, int offset)
{
   switch (IPA_field_option)
     {
    case IPA_FIELD_INDEPENDENT:
      return;
      break;
    case IPA_FIELD_DEPENDENT_VARIABLE_SIZE:
      break;
    default:
      assert(0);
      break;
    }

   switch(bcg->status)
     {
     case B_VAR:
       /* Adding an int to var is "skewing" 
	*   e.g.  x = p + 7  (same as &p->f7)
	*/
       if (!IPA_FLAG_ISSET(bcg->node->flags,
			   IPA_CG_NODE_FLAGS_NOFIELD))
	 bcg->skew = bcg->skew + offset;
       break;
     case B_ADDR:  
       /* Adding an int to an addr is "offsetting" 
	*    x = &p + 7   (same as p.f7)
	*/
       if (!IPA_FLAG_ISSET(bcg->node->flags,
			   IPA_CG_NODE_FLAGS_NOFIELD))
	bcg->offset = bcg->offset + offset;
       break;
     case B_ASSIGN:
       /* Adding an int to var is "skewing" 
	*   e.g.  x = p + 7  (same as &p->f7)
	*/
       if (!IPA_FLAG_ISSET(bcg->node->flags,
			   IPA_CG_NODE_FLAGS_NOFIELD))
	 bcg->skew = bcg->skew + offset;       
       break;
     case B_DEREF:
       /* Adding an int to a deref requires reduction
	*  e.g.  *(p + 4) + 7 => (*p + 4) -> x, x + 7
	*/
       IPA_bcg_reduce(info, bcg);
       bcg->skew = bcg->skew + offset;
       break;
     default:
       assert(0);
     } 
}

void
IPA_bcg_deref(IPA_prog_info_t * info, buildcg_t *bcg)
{
  switch(bcg->status)
    {
    case B_VAR:
      bcg->status = B_DEREF;
      break;
    case B_ADDR:
      /*assert(bcg->offset == 0);*/
      bcg->status = B_VAR;
      break;
    case B_ASSIGN:
      bcg->status = B_DEREF;      
      break;
    case B_DEREF:
      if (bcg->skew != 0)
	{
	  IPA_bcg_realize_edge(info, bcg, SKEW, 
			       0, bcg->skew, bcg->offset);
	}
      IPA_bcg_realize_edge(info, bcg, ASSIGN_DEREF, 
			   0, IPA_POINTER_SIZE, bcg->offset);
      bcg->status = B_DEREF;      
      break;
    default:
      assert(0);
    }
}


void
IPA_bcg_assign(IPA_prog_info_t * info, 
	       buildcg_t *l_bcg, buildcg_t *r_bcg,
	       int size)
{
  IPA_cgraph_edgelist_e edge_type;
  IPA_cgraph_node_t *src_node;
  IPA_cgraph_node_t *dst_node;
  IPA_cgraph_edge_t *edge;
  int t_offset, s_offset;
  int tmp_size;
  
#if 0
  print_bcg(l_bcg);
  printf(" = ");
  print_bcg(r_bcg);
  printf("\n");
#endif

  if (l_bcg->status == B_VAR)
    {
      /* Simple var on the left, no temporary needed */
      switch(r_bcg->status)
	{
	case B_VAR:
	  if (r_bcg->skew != 0)
	    {
	      IPA_bcg_realize_edge(info, r_bcg, SKEW, 
				   0, r_bcg->skew, r_bcg->offset);
	    }
	  edge_type = ASSIGN;
	  break;
	case B_ADDR:
	  assert(r_bcg->skew == 0);
	  edge_type = ASSIGN_ADDR;
	  break;
	case B_ASSIGN:
	  if (r_bcg->skew == 0)
	    edge_type = ASSIGN;
	  else
	    edge_type = SKEW;
	  break;
	case B_DEREF:
	  if (r_bcg->skew != 0)
	    {
	      IPA_bcg_realize_edge(info, r_bcg, SKEW, 
				   0, r_bcg->skew, r_bcg->offset);
	    }
	  edge_type = ASSIGN_DEREF;
	  break;
	default:
	  assert(0);
	}
    }
  else if (l_bcg->status == B_DEREF)
    {
      /* Derefenced var on the left, 
	 temporary needed if not simple var on right */
      if (l_bcg->skew != 0)
	{
	  IPA_bcg_realize_edge(info, l_bcg, SKEW, 
			       0, l_bcg->skew, l_bcg->offset);
	}
      edge_type = DEREF_ASSIGN;

      switch(r_bcg->status)
	{
	case B_VAR:
	  if (r_bcg->skew != 0)
	    {
	      IPA_bcg_realize_edge(info, r_bcg, SKEW, 
				   0, r_bcg->skew, r_bcg->offset);
	    }
	  break;
	case B_ADDR:
	  assert(r_bcg->skew == 0);
	  IPA_bcg_realize_edge(info, r_bcg, ASSIGN_ADDR, 
			       0, IPA_POINTER_SIZE, r_bcg->offset);
	  break;
	case B_ASSIGN:
	  if (r_bcg->skew != 0)
	    IPA_bcg_realize_edge(info, r_bcg, SKEW, 
				 0, r_bcg->skew, r_bcg->offset);
	  break;
	case B_DEREF:
	  if (r_bcg->skew != 0)
	    {
	      IPA_bcg_realize_edge(info, r_bcg, SKEW, 
				   0, r_bcg->skew, r_bcg->offset);
	    }

	  /* This is hack PART 1 to give library code greater
	     safety. Specifically, it is necessary to allow
             memcpy to act as expected  */
	  if (info->in_library)
	    tmp_size = IPA_max_type_size;
	  else
	    tmp_size = IPA_POINTER_SIZE;

	  IPA_bcg_realize_edge(info, r_bcg, ASSIGN_DEREF, 
			       0, tmp_size, r_bcg->offset);
	  break;
	default:
	  assert(0);
	}

      /* This is hack PART 2 from above */
      if (info->in_library)
	size = IPA_max_type_size;
    }
  else
    assert(0);

  if (edge_type == ASSIGN_ADDR ||
      edge_type == SKEW)
    size = IPA_POINTER_SIZE;
#if 0
  if (!info->in_library && edge_type != ASSIGN_ADDR
      size < IPA_POINTER_SIZE)
    {
      printf("Filtering %d\n",size);
      return;
    }
#endif

  assert(l_bcg->skew == 0);
  dst_node = l_bcg->node;
  t_offset = l_bcg->offset;

  if (IPA_FLAG_ISSET(dst_node->flags,  IPA_CG_NODE_FLAGS_RETURN))
    {
      /* The RETURN should never be a source */
      assert(edge_type != DEREF_ASSIGN);
      assert(t_offset == 0);

      if (edge_type != ASSIGN)
	{
	  /* For simplicity reasons, always use an ASSIGN edge
	     into the RETURN var */
	  IPA_bcg_realize_edge(info, r_bcg, edge_type, 
			       0, IPA_POINTER_SIZE, r_bcg->offset);
	  edge_type = ASSIGN;
	  /* Need to make the tmp assign (above)
	     bigger if this is not the case */
	  assert(size == IPA_POINTER_SIZE);
	}
    }

  src_node = r_bcg->node;
  s_offset = r_bcg->offset;
  if (edge_type != SKEW)
    {
      assert(r_bcg->skew == 0);
    }
  else
    {
      assert(size == IPA_POINTER_SIZE);
      size = r_bcg->skew;
    }

  edge = IPA_consg_ensure_edge (edge_type, src_node, dst_node,
				t_offset, size, s_offset,
				(IPA_CG_EDGE_FLAGS_EXPLICIT |
				 IPA_CG_EDGE_FLAGS_HZ));
  if (edge_type == ASSIGN_ADDR && r_bcg->isarray)
    {
      IPA_FLAG_SET(edge->flags, IPA_CG_EDGE_FLAGS_ARRAY);
    }
}

