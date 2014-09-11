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


#include "pipa_sync_gen.h"

#define DEBUG_EXPR   0
#define DEBUG_ALIAS  0
#define DEBUG_INTER  0
#define DEBUG_UNION  0
#define DEBUG_ANNOT  0
#define DEBUG_COMP   0
#define DEBUG_NOSRC  0

#define DEBUG_MARK_NOALIAS     0

#define ACC_LIST               0
#define DEBUG_LOCAL            0

#define DEBUG_CALLEES          0
#define DEBUG_CALL             0
#define DEBUG_CALL_PREPRUNE    0

#define PRINT_DEPS   0
#define DO_CONFLICTS 0

#define CALLER_PRAGMA "CALLED"


/* #define DEBUG_IPA_LEVEL 2  */

static void
sync_gen_func_intra_simple(IPA_prog_info_t *info, 
			   IPA_funcsymbol_info_t * fninfo,
			   FuncDcl funcdcl);

static void
sync_gen_func_inter_simple(IPA_prog_info_t *info, 
			   IPA_funcsymbol_info_t * fninfo,
			   FuncDcl funcdcl);


static List
get_aliasnodes_for_expr(IPA_prog_info_t *info, 
			IPA_funcsymbol_info_t * fninfo,
			Expr expr, int size);

static buildcg_t *
get_expr(IPA_prog_info_t *info, 
	 IPA_funcsymbol_info_t * fninfo,
	 Expr expr,
	 buildcg_t *bcg);

#if 0
static int
aliasnode_intersect(List list1, List list2);
#endif

static List
call_list_compress_acc(List list);

static List
call_list_union_acc(IPA_funcsymbol_info_t * fninfo,
		    List call_list, List new_list,
		    int rec_filter);
static List
call_list_intersect(List list1, List list2);

static void annotate_call_targets(IPA_funcsymbol_info_t *fninfo);
static void annotate_callers(FuncDcl fn, IPA_funcsymbol_info_t *fninfo);

static L_Alloc_Pool *IPA_aliasnode_pool = NULL;
static IPA_prog_info_t *_pinfo = NULL;

#define IPA_ALIASNODE_TO_DELETE 0x00000001

typedef struct aliasnode_t 
{
  int flags;
  IPA_cgraph_node_t  *node;
  _P_memdep_core_t    core;
  struct aliasnode_t *nxt;
} aliasnode_t;


/* Define CAST_AS_LVALUE_OK if your compiler accepts casts as lvalues and
 * does not support statement expressions.  This does not need to be defined
 * for gcc or icc.  This must not be defined for gcc >= 4. */
#undef CAST_AS_LVALUE_OK

#ifdef CAST_AS_LVALUE_OK
#define add_list(p,n)   (n->nxt = ((aliasnode_t*)p), ((aliasnode_t*)p) = n)
#else
#define add_list(p,n)   (n->nxt = ((aliasnode_t*)p), (p) = n)
#endif
#define clr_list(p)     (p = NULL)

#define do_overlap(o1,s1,o2,s2)  ((((o1) <= (o2)) && ( ((s1) == -1) || ((o1) + (s1) > (o2)) )) || \
				  (((o2) <= (o1)) && ( ((s2) == -1) || ((o2) + (s2) > (o1)) )))

#define do_mesh(o1,s1,o2,s2)     ((((o1) <= (o2)) && ( ((s1) == -1) || ((o1) + (s1) >= (o2)) )) || \
				  (((o2) <= (o1)) && ( ((s2) == -1) || ((o2) + (s2) >= (o1)) )))

#define do_subsumes(o1,s1,o2,s2) (((o1) <= (o2)) && ((((o1) + (s1)) >= ((o2) + (s2))) || (s1 == -1)))

static aliasnode_t *
aliasnode_new(IPA_cgraph_node_t *node,
	      int offset, 
	      int size)
{
  aliasnode_t *an;
  an = (aliasnode_t *) L_alloc (IPA_aliasnode_pool);
  an->node = node;
  an->core.id = node->data.var_id;
  an->core.version = node->data.version;
  an->core.offset = offset;
  an->core.size = size;
  an->nxt = NULL;
  return an;
}

static void
aliasnode_free(aliasnode_t *an)
{
  if (!an) return;
  L_free (IPA_aliasnode_pool, an);
}

static void
aliaslist_free(List list)
{
  aliasnode_t *an;
  List_start(list);
  while ((an = List_next(list)))
    {
      aliasnode_free(an);
    }
  List_reset(list);
}

#if DEBUG || DEBUG_LOCAL || DEBUG_CALLEES || DEBUG_CALL || DEBUG_CALL_PREPRUNE
|| DEBUG_COMP || DEBUG_UNION
static void
an_print(aliasnode_t *an)
{
#if 0
  if (an->node->data.var_id != 6486)
    return;
#endif
  printf("[%s-%d+%d:%d:%s] ",
	 an->node->data.syminfo->symbol_name,
	 an->core.version,
	 an->core.offset,
	 an->core.size,
	 an->node->cgraph->data.fninfo->func_name);
}
#endif

#if DEBUG || DEBUG_LOCAL || DEBUG_CALLEES || DEBUG_CALL || DEBUG_CALL_PREPRUNE
static void
anl_print(char *msg, List list)
{
  aliasnode_t *an;

  printf("%s: ",msg);
  List_start(list);
  while ((an = List_next(list)))
    {
      an_print(an);
    } 
  printf("\n");
}
#endif

#if ACC_LIST
static IPA_symaccess_t *
find_acc_list(IPA_symbol_info_t *sym, int ver, int off)
{
  IPA_symaccess_t *sal = NULL;
  
  if (!sym->obj_acc_list)
    return NULL;

  List_start(sym->obj_acc_list[ver]);
  while ((sal = List_next(sym->obj_acc_list[ver])))
    {
      assert(sal->version == ver);
      if (sal->offset == off)
	return sal;
    }
  return NULL;
}


static IPA_symaccess_t *
add_acc_list(IPA_symbol_info_t *sym, int ver, int off)
{
  IPA_symaccess_t *sal = NULL;
  if ((sal=find_acc_list(sym, ver, off)))
    return sal;

  if (sym->obj_acc_list == NULL)
    {
      sym->obj_acc_list = calloc(sym->max_version+1, sizeof(List));
      sym->acc_list_size = sym->max_version+1;
    }
  assert(ver <= sym->max_version);

  sal = calloc(1,sizeof(IPA_symaccess_t));
  sal->version = ver;
  sal->offset = off;
  sal->merge_reps = 1;
  sal->ssaReps = NULL;
  sal->ld_sym_acc = NULL;
  sal->st_sym_acc = NULL;
  sal->ld_hash = NULL;
  sal->st_hash = NULL;
  sym->obj_acc_list[ver] = List_insert_last(sym->obj_acc_list[ver], sal);
  return sal;
}

static void
free_acc_list(IPA_symbol_info_t *sym)
{
  IPA_symaccess_t *sal = NULL;
  int i;

  for (i=0; i<=sym->max_version; i++)
    {
      List_start(sym->obj_acc_list[i]);
      while ((sal = List_next(sym->obj_acc_list[i])))
	{
	  if (sal->ssaReps != NULL) sal->ssaReps = Set_dispose (sal->ssaReps);
	  free(sal);
	}  
      List_reset(sym->obj_acc_list[i]);
    }
  free(sym->obj_acc_list);
  sym->obj_acc_list = NULL;
}
#endif

IPA_funcsymbol_info_t *__fninfo = NULL;

static void
update_acc_list(Expr expr, 
		IPA_cgraph_node_t *node, int is_def, int is_jsr, 
		int off, int size, int counter)
{
  IPA_symaccess_t *sal = NULL;
  IPA_symbol_info_t *sym;

  int num_merged_nodes = 1;

  sym = node->data.syminfo;  

  if (!sym->obj_acc_list)
    return;

  /* Resize the obj_acc_list array if the max_version has changed. */
  assert(node->data.version <= sym->max_version);
  if (sym->max_version >= sym->acc_list_size)
    {
      void *temp = calloc(sym->max_version+1, sizeof(List));
      temp = memcpy (temp, sym->obj_acc_list, sym->acc_list_size * sizeof(List));
      free (sym->obj_acc_list);

      sym->obj_acc_list = temp;
      sym->acc_list_size = sym->max_version+1;
    }

  List_start(sym->obj_acc_list[node->data.version]);
  while ((sal = List_next(sym->obj_acc_list[node->data.version])))
    {
      
      assert(counter);
      assert(sal->version == node->data.version);

      if (sal->exprid == counter)
	continue;
      
      if (is_jsr)
	{
	  /* At present, the way CALLS are processed makes 
	     this impossible because indirect calls may
	     get processed multiple times */
	}
      else if (is_def)
	{
	  sal->hp_rep->st_acc += num_merged_nodes;

	  if (sal->hp_rep->st_sym_acc == NULL)
	    sal->hp_rep->st_sym_acc = Set_new ();

	  sal->hp_rep->st_sym_acc = Set_add (sal->hp_rep->st_sym_acc, expr->id);

	  if (node->rep_child)
	    update_acc_list(expr, node->rep_child, is_def, is_jsr, off, size, counter);
	}
      else
	{
	  sal->hp_rep->ld_acc += num_merged_nodes;

	  if (sal->hp_rep->ld_sym_acc == NULL)
	    sal->hp_rep->ld_sym_acc = Set_new ();

	  sal->hp_rep->ld_sym_acc = Set_add (sal->hp_rep->ld_sym_acc, expr->id);

	  if (node->rep_child)
	    update_acc_list(expr, node->rep_child, is_def, is_jsr, off, size, counter);
	}
      
      sal->exprid = counter;
    }

 
#if 0
  if (is_def && 
      IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_HEAP))
    {
      static int cnt = 0;
      cnt++;
      printf("[%4d] %s-%d+%d %d : z %d : %d",
	     cnt,
	     sym->symbol_name,
	     node->data.version,
	     off,
	     sym->id,
	     size,
	     expr->id);
      if (is_def)
	printf("ST");
      else
	printf("LD");
      printf("\n");
    }
#endif
}

#if ACC_LIST
static void
create_acc_list(IPA_cgraph_node_t *node)
{
  IPA_symbol_info_t *sym;

  if (IPA_FLAG_ISSET(node->flags, (IPA_CG_NODE_FLAGS_SUMMARY |
				   IPA_CG_NODE_FLAGS_PARAM |
				   IPA_CG_NODE_FLAGS_RETURN |
				   IPA_CG_NODE_FLAGS_TEMP)))
    {
      if (!IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_HEAP))
	return;
    }
  sym = node->data.syminfo;

  /* Every node has an offset 0 */
#if 0
  if (IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_HEAP))
    printf("HEAP %d %d %d \n",
	   node->data.syminfo->id,
	   node->data.version,
	   node->data.var_size);
#endif   

  {
    IPA_symaccess_t *sal;
    sal = add_acc_list(sym, node->data.version, 0);
    
    if ((IPA_cloning_option == IPA_HEAP_CLONING) &&
	IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_HEAP))
      {
	IPA_cgraph_node_t *rep_node;

	/* Find the representative (HEAP GEN 1) */
	rep_node = node;
	do
	  {
	    IPA_callg_node_t *call_node;
	    IPA_callg_edge_t *call_edge;
	    int found;

	    if (rep_node->generation <= 1)
	      {
#if 0
		printf("REP FOR %d:%d gen %d is %d gen %d\n",
		       node->data.var_id,
		       node->data.version,
		       node->generation,
		       rep_node->data.version,
		       rep_node->generation);
#endif
		sal->hp_rep = add_acc_list(sym, rep_node->data.version, 0);
		sal->hp_rep->reps ++;
		break;
	      }
	    
	    /* rep should be in callee of one if these */
	    found = 0;
	    call_node = rep_node->cgraph->data.fninfo->call_node;
	    for (call_node = IPA_callg_node_get_rep(call_node); call_node;
		 call_node = call_node->rep_child)
	      {
		List_start(call_node->callee_edges);
		while((call_edge = List_next(call_node->callee_edges)))
		  {
		    IPA_cgraph_t *consg;
		    IPA_cgraph_node_t *tmpnode;
		 
		    /* This handles CSREC and NORMAL */
		    if ((consg = call_edge->callee->fninfo->consg))
		      {
#if 0
			printf("   CHECKING %s %d\n",
			       call_edge->callee->fninfo->func_name,
			       IPA_htab_size(consg->nodes));
#endif
		      }
		    else
		      {
			consg = (IPA_callg_node_get_rep(call_edge->callee))->fninfo->consg;
#if 0
			printf("   CHECKING %s %d\n",
			       (IPA_callg_node_get_rep(call_edge->callee))->fninfo->func_name,
			       IPA_htab_size(consg->nodes));
#endif
		      }
		      
		    assert(consg);
		    assert(rep_node->from_version > 0);
		    tmpnode = IPA_consg_find_node(consg, rep_node->data.var_id, rep_node->from_version);
		    if (tmpnode)
		      {
			assert(rep_node != tmpnode);
			assert(tmpnode->generation == rep_node->generation - 1);
			assert(tmpnode->from_version < rep_node->from_version);
#if 0
			printf("REP FOR %d [%d:%d] -> [%d:%d]\n",
			       rep_node->data.var_id,
			       rep_node->data.version,
			       rep_node->generation,
			       tmpnode->data.version,
			       tmpnode->generation);
#endif
			rep_node = tmpnode;
			assert(rep_node);
			assert(rep_node->generation > 0);
			found = 1;
			break;
		      }
		  }
		if (found)
		  break;
	      }
	    assert(found);
	  }
	while (1);
      }
    else if (IPA_flow_sensitive_type != IPA_FLOW_NONE)
      {
	IPA_symaccess_t *sal, *rep;
	
	sal = add_acc_list(sym, node->data.version, 0);
	rep = (node->data.version == 1) ? sal : add_acc_list(sym, 1, 0);

	assert (rep);

	/* ensure that rep points to itself */
	if (rep->hp_rep == NULL)
	  rep->hp_rep = rep;

	assert (rep->hp_rep == rep);
	
	/* Insert this node's version into the Set of versions
	 * sal->hp_rep represents. */
	if (rep->ssaReps == NULL) rep->ssaReps = Set_new ();
	rep->ssaReps = Set_add (rep->ssaReps, node->data.version);
	
	sal->hp_rep = rep;
	sal->hp_rep->reps = Set_size (sal->hp_rep->ssaReps);
      }
    else
      {
	sal->hp_rep = sal;
	sal->reps = 1;
      }

    assert (sal->hp_rep);
  }
}

static void
init_acc_lists(IPA_prog_info_t *info, 
	       List tsort)
{
  IPA_funcsymbol_info_t *fninfo;
  
  List_start(info->fninfos);
  while ((fninfo = List_next(info->fninfos)))
    {
      IPA_HTAB_ITER niter;
      IPA_cgraph_node_t *node;

      if (!fninfo->has_been_called ||
	  !fninfo->consg)
	continue;
 
      IPA_HTAB_START(niter, fninfo->consg->nodes);
      IPA_HTAB_LOOP(niter)
	{
	  node = IPA_HTAB_CUR(niter);
	  create_acc_list(node);
	}
    }
}

static void
print_acc_list_stats(IPA_prog_info_t *info, 
		     List tsort)
{
  int obj_parts = 0;
  int hobj_parts = 0;
  int sym_parts = 0;
  int lds_to_sym = 0;
  int sts_to_sym = 0;
  double lds_to_obj = 0;
  double sts_to_obj = 0;
  double lds_to_hobj = 0;
  double sts_to_hobj = 0;
  IPA_symbol_info_t *sym;
  IPA_HTAB_ITER iter;


  IPA_HTAB_START(iter, info->symtab);
  IPA_HTAB_LOOP(iter)
    {
      IPA_symaccess_t *sal = NULL;
      int i;
      
      sym = IPA_HTAB_CUR(iter);
      if (!sym->obj_acc_list)
	continue;
      if (!sym->fninfo->has_been_called)
	continue;
      
      for (i=0; i<=sym->max_version; i++)
	{
	  List_start(sym->obj_acc_list[i]);
	  while ((sal = List_next(sym->obj_acc_list[i])))
	    {
	      if (sal->hp_rep != sal)
		continue;
	      assert(sal->reps >= 1);
	      
	      if (IPA_FLAG_ISSET(sym->kind, IPA_VAR_KIND_HEAP))
		{
		  /* Increment stats for the average of all represented
		     nodes */
		  hobj_parts ++;
		  lds_to_hobj += ((double)sal->ld_acc / (double)sal->reps);
		  sts_to_hobj += ((double)sal->st_acc / (double)sal->reps);
		}
	      else
		{
		  obj_parts += sal->reps;
		  lds_to_obj += ((double)sal->ld_acc / (double)sal->reps);
		  sts_to_obj += ((double)sal->st_acc / (double)sal->reps);
		  
		  sym_parts++;
		  lds_to_sym += Set_size (sal->ld_sym_acc);
		  sts_to_sym += Set_size (sal->st_sym_acc);
		}
	    }
	} /* version */
      
      /*free_acc_list(sym);*/
    }

  IPA_HTAB_START(iter, info->symtab);
  IPA_HTAB_LOOP(iter)
    {
      IPA_symaccess_t *sal = NULL;
      int i;
      
      sym = IPA_HTAB_CUR(iter);
      if (!sym->obj_acc_list)
	continue;
      if (!sym->fninfo->has_been_called)
	continue;
      
      for (i=0; i<=sym->max_version; i++)
	{
	  List_start(sym->obj_acc_list[i]);
	  while ((sal = List_next(sym->obj_acc_list[i])))
	    {
	      if (sal->hp_rep != sal)
		continue;
	      
	      if (IPA_FLAG_ISSET(sym->kind, IPA_VAR_KIND_HEAP))
		{
		  /* Increment stats for the average of all represented
		     nodes */

		  if (((double)sal->ld_acc / (double)sal->reps) > 
		      (4*lds_to_hobj/(double)hobj_parts))
		    printf("HEAP %s:%d.%d+%d : %d / %d %d\n",
			   sym->symbol_name,
			   sym->id,
			   sal->version, 
			   sal->offset,
			   sal->reps,
			   sal->ld_acc,
			   sal->st_acc);
		}
	      else
		{
		  if (((double)sal->ld_acc / (double)sal->reps) >
		      (4*lds_to_obj/(double)obj_parts))
		    printf("NORM %s:%d.%d+%d : %d / %d %d | %d %d\n",
			   sym->symbol_name,
			   sym->id,
			   sal->version, 
			   sal->offset,
			   sal->reps,
			   sal->ld_acc,
			   sal->st_acc,
			   Set_size (sal->ld_sym_acc),
			   Set_size (sal->st_sym_acc));
		}
	    }
	} /* version */
      
      free_acc_list(sym);
    }

  printf("%d : %0.3f %0.3f : NORM %0.3f %0.3f \n"
	 "%d : %0.3f %0.3f : SYMB %0.3f %0.3f \n"
	 "%d : %0.3f %0.3f : HEAP %0.3f %0.3f \n",
	 obj_parts,
	 lds_to_obj, 
	 sts_to_obj, 
	 lds_to_obj/(double)obj_parts,
	 sts_to_obj/(double)obj_parts,
	 
	 sym_parts,
	 (double)lds_to_sym,
	 (double)sts_to_sym,
	 (double)lds_to_sym/(double)sym_parts,
	 (double)sts_to_sym/(double)sym_parts,
	 
	 hobj_parts,
	 lds_to_hobj, 
	 sts_to_hobj, 
	 lds_to_hobj/(double)hobj_parts,
	 sts_to_hobj/(double)hobj_parts
	 );
}
#endif

static void
anl_annotate(Expr expr, List list, int is_def, int is_jsr,
	     int no_alias)
{
  aliasnode_t *an;
  static int counter = 1;

#if DEBUG_ANNOT
  printf("Adding %d [",expr->id);
#endif 
  List_start(list);
  while ((an = List_next(list)))
    {
      an->core.is_def = is_def;
#if DEBUG_ANNOT
      printf("%d.%d ",an->core.id, an->core.offset);
#endif

#if ACC_LIST
      update_acc_list(expr, an->node, is_def, is_jsr, 
		      an->core.offset, an->core.size, counter);
#endif

      if (IPA_sync_gen_testonly)
	continue;

      P_add_expr_memdep(expr, &an->core);	
    } 

  counter ++;

#if DEBUG_MARK_NOALIAS
  if (no_alias)
    {
      static int warned = 0;
      char buf [64];

      switch (expr->opcode)
	{
	  /* Dependences need only be appended onto a few
	   *   kinds of exprs. 
	   */
	case OP_var:
	case OP_indr:
	case OP_dot:
	case OP_arrow:
	case OP_index:
	  sprintf (buf, "NOALIAS-%d", warned);
	  expr->pragma = P_AppendPragmaNext (expr->pragma,
					     P_NewPragmaWithSpecExpr (strdup (buf), NULL));
	  
	  if (!(warned++))
	    P_warn ("Found load/store with no alias info");

	  break;
	default:
	  break;
	}
    }
#endif

#if DEBUG_ANNOT
  printf("]\n");
#endif 

  return;
}

int all_dep = 0;
int all_cmp = 0;
int dep_skipped = 0;


static int
is_recursive(IPA_funcsymbol_info_t * fninfo)
{
  IPA_callg_node_t *call_node;

  call_node = fninfo->call_node;

  if (call_node->rep_child || call_node->rep_parent != call_node)
    {
      /* Multi-func recursion */
      return 1;
    }
  else
    {
      /* Simple, self recursion */
      IPA_callg_edge_t *edge;
      
      List_start(call_node->callee_edges);
      while ((edge = List_next(call_node->callee_edges)))
	{
	  if (IPA_FLAG_ISSET(edge->flags, IPA_CALLG_EDGE_FLAGS_SELFEDGE))
	    return 1;
	}
    }
  return 0;
}


void
sync_gen (IPA_prog_info_t *info)
{
  IPA_funcsymbol_info_t * fninfo;
  IPA_callg_node_t *call_node;
  FuncDcl funcdcl;
  List tsort;
  IPA_callg_node_t *cur_call_node;
  List call_def_list;
  List call_use_list;
  double stime, synctime;

  stime = IPA_GetTime ();

  IPA_aliasnode_pool = 
    L_create_alloc_pool ("aliasnode", sizeof (aliasnode_t), 100);
  _pinfo = info;

  if (IPA_sync_append_all_obj)
    printf("APPENDING ALL ACCESSED OBJS TO EXPRS\n");

  /* Get the topological sort */
  tsort = IPA_callg_find_toposort (info->call_graph, info->globals->call_node);
  DEBUG_IPA(1, IPA_callg_topo_print(stdout, tsort););

  /* For simplicity, the annotations are done through
     three rounds:
     1) Annotation of all expressions (this is intra procedural)
     2) Computation of def/use for every procedure (this is inter procedural)
     3) Annotation if jsrs with def/use info interesected with local def/use
  */

#if ACC_LIST
  init_acc_lists(info, tsort);
#endif

  /* PHASE 0 - INITIALIZATION */
/*   printf("PHASE 0\n"); */
  IPA_TrackTime(1);
  List_start(info->fninfos);
  while ((fninfo = List_next(info->fninfos)))
    {
      IPA_HTAB_ITER niter;
      IPA_cgraph_node_t *node;

      if (!fninfo->consg)
	continue;
      
      IPA_HTAB_START(niter,fninfo->consg->nodes);
      IPA_HTAB_LOOP(niter)
	{
	  node = IPA_HTAB_CUR(niter);
	  node->misc.sync_ptr = NULL;
	}
    }

  /* PHASE 1 - EXPR ANNOTATION */
/*   printf("PHASE 1\n"); */
  IPA_TrackTime(1);
  all_dep = 0;
  all_cmp = 0;
  call_node = List_last (tsort);
  do
    {
      /* INTRA expr-expr: Consider each SCC function in turn */
      for (cur_call_node = call_node;
	   cur_call_node; cur_call_node = cur_call_node->rep_child)
	{
	  fninfo = cur_call_node->fninfo;
	  if (fninfo == info->globals)
	    continue;
	  DEBUG_IPA(2, printf ("## GEN [%s]\n", fninfo->func_name););
	  
	  funcdcl = PST_GetFuncDclEntry(info->symboltable,
					fninfo->func_key);
	  __fninfo = fninfo;
	  sync_gen_func_intra_simple(info, fninfo, funcdcl);
	}
    }
  while ((call_node = List_prev (tsort)));
  
  /* PHASE 2 - JSR CALCULATION */
/*   printf("PHASE 2\n"); */
  IPA_TrackTime(1);
  call_node = List_last (tsort);
  do
    { 
      /* Combine all call_aliasnode lists for SCC into parent list */
      call_def_list = call_node->fninfo->def_aliasnode_list;
      call_use_list = call_node->fninfo->use_aliasnode_list;
      for (cur_call_node = call_node;
	   cur_call_node; cur_call_node = cur_call_node->rep_child)
	{
	  IPA_callg_edge_t *call_edge;

	  fninfo = cur_call_node->fninfo;
	  if (fninfo == info->globals)
	    continue;

	  /* Union with callees of that SCC procedure */
	  /* THIS IS PRETTY INEFFICIENT IN THAT IT DOES NOT FILTER
	     OUT REPEATED CALLS TO THE SAME PROCEDURE */
	  List_start(fninfo->call_node->callee_edges);
	  while ((call_edge = List_next(fninfo->call_node->callee_edges)))
	    {
	      IPA_funcsymbol_info_t *cfninfo;
	      /* Get parent call node for callee */
	      cfninfo = (IPA_callg_node_get_rep(call_edge->callee))->fninfo;

#if DEBUG_CALLEES
	      printf ("## CALLEE-COMB [SCC %s SCCFN %s : CALLEE %s SCCCALLEE %s]\n", 
		      call_node->fninfo->func_name,
		      fninfo->func_name,
		      call_edge->callee->fninfo->func_name,
		      cfninfo->func_name);
#endif

	      /* Don't combine with itself */
	      if (cfninfo == call_node->fninfo)
		continue;
#if DEBUG_CALLEES
	      {
		char msg[256];
		sprintf(msg, "  CALLEE_DEF ");
		anl_print(msg,  cfninfo->def_aliasnode_list);
		
		sprintf(msg, "  CALLEE_USE ");
		anl_print(msg,  cfninfo->use_aliasnode_list);
	      }
#endif
	      call_def_list = call_list_union_acc(call_node->fninfo,
						  call_def_list, 
						  cfninfo->def_aliasnode_list, 0);
	      call_use_list = call_list_union_acc(call_node->fninfo,
						  call_use_list, 
						  cfninfo->use_aliasnode_list, 0);
#if 0
	      {
		char msg[256];
		sprintf(msg, "  CUR_DEF ");
		anl_print(msg,  call_def_list);
		
		sprintf(msg, "  CUR_USE ");
		anl_print(msg,  call_use_list);
	      }
#endif
	    }
	  
	  /* Don't combine with itself */
	  if (cur_call_node == call_node)
	    continue;

#if DEBUG_CALLEES
	  printf ("## SCC-CALLCOMB [%s]\n", fninfo->func_name);
	  {
	    char msg[256];
	    sprintf(msg, "  SCCCALL_DEF ");
	    anl_print(msg,  fninfo->def_aliasnode_list);
	    
	    sprintf(msg, "  SCCCALL_USE ");
	    anl_print(msg,  fninfo->use_aliasnode_list);
	  }
#endif
	  /* Union with other SCC procedure */
	  call_def_list = call_list_union_acc(call_node->fninfo,
					      call_def_list, 
					      fninfo->def_aliasnode_list, 0);
	  call_use_list = call_list_union_acc(call_node->fninfo,
					      call_use_list, 
					      fninfo->use_aliasnode_list, 0);
	  aliaslist_free(fninfo->def_aliasnode_list);
	  aliaslist_free(fninfo->use_aliasnode_list);
	  fninfo->def_aliasnode_list = NULL;
	  fninfo->use_aliasnode_list = NULL;

	}
      call_node->fninfo->def_aliasnode_list = call_list_compress_acc(call_def_list);
      call_node->fninfo->use_aliasnode_list = call_list_compress_acc(call_use_list);
    }
  while ((call_node = List_prev (tsort)));

  /* PHASE 3 - JSR ANNOTATION */
/*   printf("PHASE 3\n"); */
  IPA_TrackTime(1);
  call_node = List_last (tsort);
  do
    { 
      /* INTER expr-callee: Consider each SCC function in turn */
      for (cur_call_node = call_node;
	   cur_call_node; cur_call_node = cur_call_node->rep_child)
	{
	  fninfo = cur_call_node->fninfo;
	  if (fninfo == info->globals)
	    continue;
	  DEBUG_IPA(1, printf ("## CALL [%s]\n", fninfo->func_name););

	  funcdcl = PST_GetFuncDclEntry(info->symboltable,
					fninfo->func_key);
	  sync_gen_func_inter_simple(info, fninfo, funcdcl);	  
	}
   }
  while ((call_node = List_prev (tsort)));

  /* CLEANUP */
  List_start(info->fninfos);
  while ((fninfo = List_next(info->fninfos)))
    {
      if (fninfo != info->globals)
	{
	  if (!fninfo->has_been_called)
	    {
	      Pragma prag;
	      FuncDcl fn;
	      
	      fn = PST_GetFuncDclEntry(info->symboltable, 
				       fninfo->func_key);
	      prag = P_NewPragmaWithSpecExpr("NOT-CALLED", NULL);
	      fn->pragma = P_AppendPragmaNext(fn->pragma, prag);
	    }
	  else
	    {
	      Pragma prag;
	      FuncDcl fn;
	      
	      fn = PST_GetFuncDclEntry(info->symboltable, 
				       fninfo->func_key);
	      prag = P_NewPragmaWithSpecExpr("CALLED", NULL);
	      fn->pragma = P_AppendPragmaNext(fn->pragma, prag);
	      annotate_callers (fn, fninfo);

	      if (is_recursive(fninfo))
		{
		  fn = PST_GetFuncDclEntry(info->symboltable, 
					   fninfo->func_key);
		  prag = P_NewPragmaWithSpecExpr("RECURSIVE", NULL);
		  fn->pragma = P_AppendPragmaNext(fn->pragma, prag);	  
		}
	    }
	}

      aliaslist_free(fninfo->def_aliasnode_list);
      aliaslist_free(fninfo->use_aliasnode_list);
      aliaslist_free(fninfo->intra_du_list);
      fninfo->def_aliasnode_list = NULL;
      fninfo->use_aliasnode_list = NULL;
      fninfo->intra_du_list = NULL;
    }

  L_free_alloc_pool (IPA_aliasnode_pool);
  IPA_aliasnode_pool = NULL;
  fflush(stdout);

/*   printf("DONEDONE\n"); */
  IPA_TrackTime(1);

  synctime = (IPA_GetTime () - stime);
/*   printf("GDATA SYNC TIME %f\n", synctime); */

#if ACC_LIST
  print_acc_list_stats(info, tsort);
#endif

#if 0
  printf("ALLDEP %d ALLCMP %d %0.4f\n",all_dep,all_cmp,all_dep/all_cmp);
#endif

  return;
}


static Expr
getparent(Expr expr)
{
  Expr pexpr = expr;

  do
    {
      pexpr = pexpr->parentexpr;
    }
  while (pexpr && 
	 (pexpr->opcode == OP_cast ||
	  pexpr->opcode == OP_compexpr));

  return pexpr;
}

static int
isdef(Expr expr)
{
  Expr pexpr;

  pexpr = expr->parentexpr;
  if (pexpr == NULL)
    return 0;

  while (pexpr && 
	 (pexpr->opcode == OP_cast ||
	  pexpr->opcode == OP_compexpr))
    {
      pexpr = expr->parentexpr;
      expr = pexpr;
    }
  if (!pexpr)
    return 0;

  if (pexpr->opcode == OP_assign)
    {
      /* See if expr is lhs */
      if (expr == pexpr->operands)
	return 1;
    }
  return 0;
}

static int
isdotted(Expr expr)
{
  Expr pexpr;

  pexpr = getparent(expr);
  if (pexpr == NULL)
    return 0;

  if (pexpr->opcode == OP_dot)
    return 1;

  return 0;
}

static int
isaddr(Expr expr)
{
  Expr pexpr;

  pexpr = getparent(expr);
  if (pexpr == NULL)
    return 0;

  if (pexpr->opcode == OP_addr)
    return 1;

  return 0;
}

		       
static void
sync_gen_func_intra_simple(IPA_prog_info_t *info, 
			   IPA_funcsymbol_info_t * fninfo,
			   FuncDcl funcdcl)
{
  List exprlist = NULL;
  List aliasnode_list = NULL;
  List call_def_list = NULL;
  List call_use_list = NULL;
  List intra_du_list = NULL;
  Expr expr;
  int size;

  exprlist = IPA_Find_All_Expr_In_Stmts (info, funcdcl->stmt, NULL, B_FOLLOW);

  if (List_size(exprlist) == 0)
    return;

  /* EXPR TO EXPR DEPENDENCES */      
#if DEBUG_LOCAL
  printf("Exprs [%d]\n",List_size(exprlist));
#endif
  List_start(exprlist);
  while ((expr = List_next(exprlist)))
    {
      if (expr->opcode == OP_compexpr ||
	  expr->opcode == OP_assign ||
	  expr->opcode == OP_call)
	continue;

      if (isdotted(expr) || isaddr(expr))
	{
#if DEBUG_LOCAL
	  printf("%d: addr/dot\n",expr->id);
#endif
	  continue;
	}
      size = IPA_Pcode_sizeof(info, IPA_ExprType(info, expr));
      
#if 0
      if (expr->id == 571 && 
	  strstr(__fninfo->func_name,"DecodeVolHeader"))
	{
	  printf("expr # %d size %d %s\n",
		 expr->id, size,
		 __fninfo->func_name);      
	}
#endif

#if DEBUG_EXPR
      printf("EXPR1\n");
      P_write_expr(stdout, expr, 0, NULL);
      printf("[%d]\n", size);     
#endif

      aliasnode_list = get_aliasnodes_for_expr(info, fninfo, 
					       expr, size);

#if DEBUG_LOCAL
      {
	char msg[256];
	sprintf(msg, "   %d ", expr->id);
	anl_print(msg,  aliasnode_list);
      }
#endif
      anl_annotate(expr, aliasnode_list, isdef(expr), 0,
		   (List_size (aliasnode_list) == 0));
#if DEBUG_LOCAL
      printf("%d: DEF/USE\n",expr->id);
#endif

#if !ACC_LIST
      if (isdef(expr))
	call_def_list = call_list_union_acc(fninfo, call_def_list, 
					    aliasnode_list, 1);
      else 
	call_use_list = call_list_union_acc(fninfo, call_use_list, 
					    aliasnode_list, 1);
      intra_du_list = call_list_union_acc(fninfo, intra_du_list, 
					  aliasnode_list, 0);
#endif

#if DEBUG_LOCAL
      {
	char msg[256];
	sprintf(msg, "  CALL_DEF %d ", expr->id);
	anl_print(msg,  call_def_list);

	sprintf(msg, "  CALL_USE %d ", expr->id);
	anl_print(msg,  call_use_list);

	sprintf(msg, "  FNDU %d ", expr->id);
	anl_print(msg,  intra_du_list);
      }
#endif
      
      aliaslist_free(aliasnode_list);
    }

#if 0
  printf("Listsize d%d u%d in%d \n",
	 List_size(call_def_list),
	 List_size(call_use_list),
	 List_size(intra_du_list));
#endif
  fninfo->def_aliasnode_list = call_def_list;
  fninfo->use_aliasnode_list = call_use_list;
  fninfo->intra_du_list = intra_du_list;
  List_reset(exprlist);

  return;
}


static void
sync_gen_func_inter_simple(IPA_prog_info_t *info, 
			   IPA_funcsymbol_info_t * fninfo,
			   FuncDcl funcdcl)
{
  IPA_callg_edge_t *call_edge = NULL;
  List intra_du_list = NULL;
  List tmp_list = NULL;

  /* strianta: This function will annotate call targets of function
     pointer calls on the code.  This is probably not the best place
     to call it, but it works!
   */
  annotate_call_targets(fninfo);

  if (List_size(fninfo->call_node->callee_edges) == 0)
    return;
  
  /* This must be the CALLER's du list. Its used to reduce
   *   the def/use of jsrs to only the set the caller cares
   *   about (otherwise, these sets can get large).
   */
  fninfo->intra_du_list = call_list_compress_acc(fninfo->intra_du_list);
  intra_du_list = fninfo->intra_du_list;

  List_start(fninfo->call_node->callee_edges);
  while ((call_edge = List_next(fninfo->call_node->callee_edges)))
    {
      IPA_funcsymbol_info_t *pcallee_fninfo = NULL;
      List call_def_list = NULL;
      List call_use_list = NULL;
      Expr expr = NULL;

      /* Alias nodes are kept on SCC parent nodes */
      pcallee_fninfo = (IPA_callg_node_get_rep(call_edge->callee))->fninfo;
#if DEBUG_CALL_PREPRUNE
      printf("CALLEE %s\n",
	     pcallee_fninfo->func_name);
#endif
      call_def_list = pcallee_fninfo->def_aliasnode_list;
      call_use_list = pcallee_fninfo->use_aliasnode_list;

      /* The call expr */
      expr = call_edge->caller_cs->call_expr;

#if DEBUG_CALL_PREPRUNE
      {
	char msg[256];
	sprintf(msg, "  USE %d ", expr->id);
	anl_print(msg,  call_use_list);
	sprintf(msg, "  DEF %d ", expr->id);
	anl_print(msg,  call_def_list);
	sprintf(msg, " FNDU %d ", expr->id);
	anl_print(msg,  intra_du_list);
      }
#endif

      if (!IPA_sync_annotate_all_obj)
	{
	  tmp_list = call_list_intersect(intra_du_list, call_use_list);
	  anl_annotate(expr, tmp_list, 0, 1,
		       (List_size (call_use_list) == 0));

#if DEBUG_CALL
	  {
	    char msg[256];
	    sprintf(msg, "  PRUNE-USE %d ", expr->id);
	    anl_print(msg,  tmp_list);
	  }
#endif
	  aliaslist_free(tmp_list);
	}
      else
	{
	  anl_annotate(expr, call_use_list, 0, 1,
		       (List_size (call_use_list) == 0));
	}

      if (!IPA_sync_annotate_all_obj)
	{
	  tmp_list = call_list_intersect(intra_du_list, call_def_list);
	  anl_annotate(expr, tmp_list, 1, 1,
		       (List_size (call_use_list) == 0));
#if DEBUG_CALL
	  {
	    char msg[256];
	    sprintf(msg, "  PRUNE-DEF %d ", expr->id);
	    anl_print(msg,  tmp_list);
	  }
#endif
	  aliaslist_free(tmp_list);
	}
      else
	{
	  anl_annotate(expr, call_def_list, 1, 1,
		       (List_size (call_use_list) == 0));
	}
    }

  return;
}





static int
is_possible_call_dep(IPA_funcsymbol_info_t * fninfo,
		     IPA_cgraph_node_t *node)
{
  IPA_callg_node_t *callg;
  IPA_callg_node_t *parent_callg;
  IPA_cgraph_edge_list_t *elist;
  IPA_cgraph_edge_t *edge;
  IPA_cgraph_node_t *phys_dst_node;
  IPA_cgraph_node_t *dst_node;
  IPA_HTAB_ITER eiter;

  /* To make deps due to recursion less conservative
     some dep nodes are filtered. The following are
     the requirements to add a node to call-dep-list:
     1) Address taken   AND
        Address assigned into FORMAL/ESCLOCAL of a
     member of the SCC (including the func itself)
     or any GLOBAL (this could be futher refined if
     need be 
     2) The node is itself a GLOBAL or ESCLOCAL 
        (i.e. its address is already escaping)
  */
  if (IPA_FLAG_ISSET(node->flags, (IPA_CG_NODE_FLAGS_GLOBAL |
				   IPA_CG_NODE_FLAGS_ESCLOCAL |
				   IPA_CG_NODE_FLAGS_HEAP)))
    {
#if DEBUG_NOSRC
      if (node->data.var_id == 57192)
	printf("PC [%s]: base\n", fninfo->func_name);
#endif
      return 1;
    }

  elist = IPA_cg_edge_list_find(node, ASSIGN_ADDR);
  if (!elist)
    {
#if DEBUG_NOSRC
      if (node->data.var_id == 57192)
	printf("PC [%s]: noaddr\n", fninfo->func_name);
#endif
      return 0;
    }

  parent_callg = IPA_callg_node_get_rep(fninfo->call_node);
  IPA_HTAB_START(eiter, elist->out);
  IPA_HTAB_LOOP(eiter)
    {
      edge = IPA_HTAB_CUR(eiter);
      phys_dst_node = edge->dst_elist->node;
      
      for (dst_node=phys_dst_node; dst_node; dst_node=dst_node->rep_child)
	{
	  callg = IPA_callg_node_get_rep(dst_node->cgraph->data.fninfo->call_node);
#if DEBUG_NOSRC
	  if (node->data.var_id == 57192)
	    {
	      printf("PC [%s]: target %d.%d %s | parent %s | dstparent %s \n", 
		     fninfo->func_name,
		     dst_node->data.var_id,
		     dst_node->data.version,
		     dst_node->data.syminfo->symbol_name,
		     parent_callg->fninfo->func_name,
		     callg->fninfo->func_name);
	    }
#endif
	  if (IPA_FLAG_ISSET(dst_node->flags, (IPA_CG_NODE_FLAGS_GLOBAL |
					       IPA_CG_NODE_FLAGS_HEAP)))
	    {
#if DEBUG_NOSRC
	      if (node->data.var_id == 57192)
		printf("PC [%s]: global\n", fninfo->func_name);
#endif
	      return 1;
	    }
	  else if (IPA_FLAG_ISSET(dst_node->flags, (IPA_CG_NODE_FLAGS_REALPARAM |
						    IPA_CG_NODE_FLAGS_ESCLOCAL)))
	    {
	      if (node->data.var_id == 57192)
		printf("PC [%s]: realparam\n", fninfo->func_name);
	      if (callg == parent_callg)
		{
#if DEBUG_NOSRC
		  if (node->data.var_id == 57192)
		    printf("PC [%s]: formal match\n", fninfo->func_name);
#endif
		  return 1;
		}
	    }
	}
    } /* addr edges */

#if DEBUG_NOSRC
  if (node->data.var_id == 57192)
    printf("PC [%s]: nomatch\n", fninfo->func_name);
#endif
  return 0;
}


static List
call_list_compress_acc(List list)
{
  aliasnode_t *an;
  IPA_cgraph_node_t *node;
  List node_list = NULL;
#if DEBUG_COMP
  int del = 0;
#endif

  /* Build lists of accesses to same node (many offsets)
     for all target accesses */
#if DEBUG_COMP
  printf("----------\n"); 
#endif
  List_start(list);
  while ((an = List_next(list)))
    {
#if DEBUG_COMP
	{
	  printf("CPRE: "); 
	  an_print(an);
	  printf("\n");
	}
#endif
      
      IPA_FLAG_CLR(an->flags, IPA_ALIASNODE_TO_DELETE);
      add_list(an->node->misc.sync_ptr, an);
      if (!List_member(node_list, an->node))
	node_list = List_insert_last(node_list, an->node);
    }
  
  /* Now go through the list again and compress
   *  accesses to each node 
   */
  List_start(node_list);
  while ((node = List_next(node_list)))
    {
      aliasnode_t *cur1;
      aliasnode_t *cur2;

      /* cur is the target list access 
       * an is the source (possibly new) access
       */
      for (cur1=node->misc.sync_ptr; cur1; cur1 = cur1->nxt)
	{
	  if (IPA_FLAG_ISSET(cur1->flags, IPA_ALIASNODE_TO_DELETE))
	    continue;
#if DEBUG_COMP
	  printf("  C1: ");
	  an_print(cur1);
	  printf("\n");
#endif

	  for (cur2=cur1->nxt; cur2; cur2 = cur2->nxt)
	    {
	      if (IPA_FLAG_ISSET(cur2->flags, IPA_ALIASNODE_TO_DELETE))
		continue;
#if DEBUG_COMP
	      printf("  C2: ");
	      an_print(cur2);
#endif
	      
	      if (do_subsumes(cur1->core.offset, cur1->core.size,
			      cur2->core.offset, cur2->core.size))
		{
		  /* Delete cur2 */
#if DEBUG_COMP
		  printf("  DEL2\n");
#endif
		  IPA_FLAG_SET(cur2->flags, IPA_ALIASNODE_TO_DELETE);
		  continue;
		}
	      else if (do_subsumes(cur2->core.offset, cur2->core.size,
				   cur1->core.offset, cur1->core.size))
		{
		  /* Delete cur1 */
#if DEBUG_COMP
		  printf("  DEL1\n");
#endif
		  IPA_FLAG_SET(cur1->flags, IPA_ALIASNODE_TO_DELETE);
		  break;
		}
	      else if (do_mesh(cur2->core.offset, cur2->core.size,
			       cur1->core.offset, cur1->core.size))
		{
		  int nsize, noff;
		  /* Delete cur1 and replace cur2 with larger version */
#if DEBUG_COMP
		  printf("  MSH\n");
#endif
		  if (cur1->core.offset <= cur2->core.offset)
		    {
		      if (cur2->core.size == -1)
			{
			  printf("%d c2\n",cur2->node->data.var_id);
			  nsize = -1;
			}
		      else
			{
			  nsize = ((cur2->core.offset - cur1->core.offset) + 
				   cur2->core.size);
			}
		      noff = cur1->core.offset;
		    }
		  else
		    {
		      if (cur1->core.size == -1)
			{
			  printf("%d c1\n",cur2->node->data.var_id);
			  nsize = -1;
			}
		      else
			nsize = ((cur1->core.offset - cur2->core.offset) + 
				 cur1->core.size);
		      noff = cur2->core.offset;
		    }

		  assert(nsize == -1 || nsize > 0);
		  cur2->core.offset = noff;
		  cur2->core.size = nsize;
		  IPA_FLAG_SET(cur1->flags, IPA_ALIASNODE_TO_DELETE);
		  break;
		}
	    } /* cur2 */
	} /* cur1 */
    } /* node */


  /* Clear out the sync ptr lists 
   */
  List_start(list);
  while ((an = List_next(list)))
    {
      clr_list(an->node->misc.sync_ptr);
      if (IPA_FLAG_ISSET(an->flags, IPA_ALIASNODE_TO_DELETE))
	{
	  List_delete_current(list);
	  aliasnode_free(an);
#if DEBUG_COMP
	  del++;
#endif
	}
    }

#if DEBUG_COMP
  printf("Deleting %d nodes\n", del);
#endif

  List_reset(node_list);

  return list;
}



static List
call_list_union_acc(IPA_funcsymbol_info_t * fninfo,
		    List target_list, List source_list,
		    int rec_filter)
{
  aliasnode_t *an;

  /* Build lists of accesses to same node (many offsets)
     for all target accesses */
#if DEBUG_UNION
      printf("----------\n"); 
#endif
  List_start(target_list);
  while ((an = List_next(target_list)))
    {
#if DEBUG_UNION
      printf("UPRE: "); 
      an_print(an);
      printf("\n");
#endif

      add_list(an->node->misc.sync_ptr, an);
    }

  /* Now go through the source list and add any
   *  new ones to the target list 
   */
  List_start(source_list);
  while ((an = List_next(source_list)))
    {
      aliasnode_t *cur;
      int found;

      if (rec_filter && !is_possible_call_dep(fninfo, an->node))
	{
#if DEBUG_UNION
	  printf("NOSRC: ");
	  an_print(an);
	  printf("\n");
#endif
	  continue;
	}
#if DEBUG_UNION
      printf("  SRC: ");
      an_print(an);
      printf("\n");
#endif

      /* cur is the target list access 
       * an is the source (possibly new) access
       */
      found = 0;
      for (cur=an->node->misc.sync_ptr; cur; cur = cur->nxt)
	{
#if DEBUG_UNION
	  printf("  TGT: ");
	  an_print(cur);
	  printf("\n");
#endif

	  if (do_subsumes(an->core.offset, an->core.size,
			  cur->core.offset, cur->core.size))
	    {
	      /* source access subsumes target 
	       *   replace target access with source one 
	       */
#if DEBUG_UNION
	      printf("  RPL\n");
#endif
	      cur->core.offset = an->core.offset;
	      cur->core.size = an->core.size;
	      found = 1;
	      break;
	    }
	  else if (do_subsumes(cur->core.offset, cur->core.size,
			       an->core.offset, an->core.size))
	    {
	      /* Target subsumes source, do nothing */
#if DEBUG_UNION
	      printf("  SUB\n");
#endif
	      found = 1;
	      break;
	    }
	}
      
      if (!found)
	{
	  /* This is a little suboptimal in number of accesses,
	   * but add all other accesses (this could be better by
	   * combining overlapping accesses in a single, bigger access)
	   */
#if DEBUG_UNION
	  printf("  NEW\n");
#endif
	  target_list = List_insert_last(target_list,
					 aliasnode_new(an->node, 
						       an->core.offset, 
						       an->core.size));
	  
	}
    }

  /* Clear out the sync ptr lists 
   */
  List_start(target_list);
  while ((an = List_next(target_list)))
    {
#if DEBUG_UNION
      printf("UPOST: ");
      an_print(an);
      printf("\n");
#endif
      clr_list(an->node->misc.sync_ptr);
    }

  return target_list;
}


static List
call_list_intersect(List list1, List list2)
{
  List call_list = NULL;
  aliasnode_t *an;

  /* Mark all nodes in list1 and see
     if any node in list2 was marked */
#if 0
  printf("intersect Filter [%d] Def/Use [%d]\n",
	 List_size(list1),
	 List_size(list2));
#endif
#if 0
  if (List_size(list1) > List_size(list2))
    {
      List tmp = list1;
      list1 = list2;
      list2 = tmp;
    }
#endif

  /*
    #define add_list(p,n)   (n->nxt = ((aliasnode_t*)p), ((aliasnode_t*)p) = n)
  */
  List_start(list1);
  while ((an = List_next(list1)))
    {
#if 0
      printf("Filter ");
      an_print(an); 
      printf(" %p\n",an->node);
#endif
      add_list(an->node->misc.sync_ptr, an);
    }

  
  List_start(list2);
  while ((an = List_next(list2)))
    {
      aliasnode_t *cur;
#if 0
      printf("Def/Use ");
      an_print(an); 
      printf(" %p\n",an->node);
#endif
      for (cur=an->node->misc.sync_ptr; cur; cur = cur->nxt)
	{
#if 0
	  printf("      ");
	  an_print(cur);
	  printf("    Overlap: [%d s%d] [%d s%d] \n",
		 an->core.offset, an->core.size,
		 cur->core.offset, cur->core.size);
#endif
	  if (do_overlap(an->core.offset, an->core.size,
			 cur->core.offset, cur->core.size))
	    {
	      call_list = List_insert_last(call_list,
					   aliasnode_new(an->node, 
							 an->core.offset, 
							 an->core.size));
	    }
	}
    }

  List_start(list1);
  while ((an = List_next(list1)))
    {
      clr_list(an->node->misc.sync_ptr);
    }

  return call_list;
}


#if 0
static int
aliasnode_intersect(List list1, List list2)
{
  int ret_val = 0;
  aliasnode_t *an;

  /* Mark all nodes in list1 and see
     if any node in list2 was marked */
#if DEBUG_INTER
  printf("intersect [%d] [%d]\n",
	 List_size(list1),
	 List_size(list2));
#endif

  if (List_size(list1) > List_size(list2))
    {
      List tmp = list1;
      list1 = list2;
      list2 = tmp;
    }
  /*
    #define add_list(p,n)   (n->nxt = ((aliasnode_t*)p), ((aliasnode_t*)p) = n)
  */
  List_start(list1);
  while ((an = List_next(list1)))
    {
      add_list(an->node->misc.sync_ptr, an);
    }

  
  List_start(list2);
  while ((an = List_next(list2)))
    {
      aliasnode_t *cur;
      for (cur=an->node->misc.sync_ptr; cur; cur = cur->nxt)
	{
#if DEBUG_INTER
	  printf("    Overlap: [%d s%d] [%d s%d]\n",
		 an->core.offset, an->core.size,
		 cur->core.offset, cur->core.size);
#endif
	  if (do_overlap(an->core.offset, an->core.size,
			 cur->core.offset, cur->core.size))
	    {
	      ret_val = 1;
	      goto DONE_INTERSECT;
	    }
	}
    }
 DONE_INTERSECT:

  List_start(list1);
  while ((an = List_next(list1)))
    {
      clr_list(an->node->misc.sync_ptr);
    }

  return ret_val;
}
#endif











static buildcg_t *
get_var(IPA_prog_info_t *info, 
	IPA_funcsymbol_info_t * fninfo,
	Expr expr)
{
  IPA_symbol_info_t *sym;
  buildcg_t *bcg;

  sym = IPA_symbol_find (info, expr->value.var.key);
  /* If the symbol was never involved in any pointer expression
     then it may not be in the symbol table */
  if (!sym)
    return NULL;
  assert(sym->id > 0);

  /* Added the notion of subsricpt versioning - JWP */
  {
    int subscr;
    
    if (expr->value.var.ssa)
      subscr = expr->value.var.ssa->name;
    else
      subscr = 1;
    
    bcg = IPA_buildcg_start(info, fninfo, sym->id, subscr, B_FOLLOW);
  }

  return bcg;  
}

#if 0
static void
get_offset(IPA_prog_info_t *info, 
	   IPA_funcsymbol_info_t * fninfo,
	   buildcg_t *bcg,
	   Key type, char *fieldname)
{
  Field field;

  assert(0);
  field = IPA_Pcode_get_field (info, type, fieldname);

  IPA_bcg_offset(info, bcg, PST_GetFieldContainerOffset (info->symboltable, 
							 field));
}

static void
get_arrow(IPA_prog_info_t *info, 
	  IPA_funcsymbol_info_t * fninfo,
	  buildcg_t *bcg,
	  Key type, char *fieldname)
{
  Field field;
  assert(0);
  /* Incorporate dereference */
  IPA_bcg_deref(info, bcg);

  /* Should look  like BT_POINTER then BT_STRUCTURE */
  assert(IPA_Pcode_IsPointerType(info, type));

  /* Now use structure type and field name to get info */
  type = PST_GetTypeType(info->symboltable, type);
  field = IPA_Pcode_get_field (info, type, fieldname);

  /* Incorporate offset */
  IPA_bcg_offset(info, bcg, PST_GetFieldContainerOffset (info->symboltable, 
							 field));
}
#endif

static List
add_node(List list, 
	 IPA_cgraph_node_t *node,
	 int offset,
	 int size,
	 int add_only)
{
  /* Adjust for nodes treated field insensitively */
  if (IPA_field_option == IPA_FIELD_INDEPENDENT ||
      IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_NOFIELD))
    {
      offset = 0;
      /*printf("%d -1 FIELD\n",node->data.var_id);*/
      size = -1;
    }

  if (!IPA_sync_append_all_obj || add_only)
    {
      aliasnode_t *an = aliasnode_new(node,offset,size);
      list = List_insert_last(list, an);
    }
  else
    {
      /* Add all equivalent nodes */
      for(;node;node = node->rep_child)
	{
	  aliasnode_t *an = aliasnode_new(node,offset,size);
	  list = List_insert_last(list, an);
	}
    }

  return list;
}

static List
get_pointsto(IPA_prog_info_t *info, 
	     IPA_funcsymbol_info_t * fninfo,
	     Expr expr,
	     IPA_cgraph_node_t *node,
	     int offset,
	     int skew,
	     int size)
{
  IPA_cgraph_edge_list_t *elist;
  IPA_cgraph_edge_t *edge;
  IPA_HTAB_ITER eiter;
  List list = NULL;
  int this_size;
  int s_mod;

  elist = IPA_cg_edge_list_find(node, ASSIGN_ADDR);
  if (!elist)
    return list;

  /* Find all addresses into the target at the
     desired offset */
  IPA_HTAB_START(eiter, elist->in);
  IPA_HTAB_LOOP(eiter)
    {
      edge = IPA_HTAB_CUR(eiter);
      if (edge->data.target_offset == offset)
	{
#if DEBUG_ALIAS
	  printf("   PT [%d s%d]",
		 edge->data.source_offset + skew,
		 size);
	  IPA_cg_node_print(stdout,edge->src_elist->node,IPA_PRINT_ASCI);
	  printf("\n");
#endif

	  this_size = size;
	  if (IPA_FLAG_ISSET(edge->flags, IPA_CG_EDGE_FLAGS_ARRAY) ||
	      edge->data.source_stride != 0)
	    {
	      this_size = -1;
	    }

	  /* The actual address of interest is the offset of the source
	   *  plus any desired skew  */
	  if (expr->id == 571 && 
	      strstr(__fninfo->func_name,"DecodeVolHeader"))
	    {
	      printf("expr #2 %d size %d %s\n",
		     expr->id, this_size,
		     __fninfo->func_name);      
	    }

	  s_mod = edge->src_elist->node->data.mod;
	  list = add_node(list,
			  edge->src_elist->node,
			  (edge->data.source_offset + skew) % s_mod,
			  this_size, 0);
	}
    }

  return list;
}


static List
get_nodeof(IPA_prog_info_t *info, 
	   IPA_funcsymbol_info_t * fninfo,
	   Expr expr,
	   IPA_cgraph_node_t *node,
	   int offset,
	   int size)
{
  List list = NULL;
  int s_mod;

  /* Only bother with this if the node has a representative
     and is not address taken */
  if ((node->rep_parent != node) &&
      (!IPA_cg_edge_list_find(node, ASSIGN_ADDR)))
    {
      IPA_symbol_info_t *sym;
      IPA_cgraph_t *consg;

      /* Must find the actual node for this var expr.
	 While the node maybe unif with others this does
	 not affect the dependence in this case (unif means
	 its contents are the same not that the nodes are
	 always found together) */
      
      while ((expr->opcode != OP_var))
	expr = expr->operands;
      sym = IPA_symbol_find (info, expr->value.var.key);
      assert(sym && sym->id > 0);
      
      if (IPA_FLAG_ISSET(sym->kind, IPA_VAR_KIND_GLOBAL))
	{
	  fninfo = info->globals;
	}
      if (!fninfo->consg)
	{
	  IPA_callg_node_t *cn;
	  cn = IPA_callg_node_get_rep(fninfo->call_node);
	  consg = cn->fninfo->consg;
	}
      else
	{
	  consg = fninfo->consg;
	}
      node = IPA_consg_find_node (consg, sym->id, 1);
      assert(node);
    }

#if DEBUG_ALIAS
  printf("   VR [%d s%d]", offset, size);
  IPA_cg_node_print(stdout,node,IPA_PRINT_ASCI);
  printf("\n");
#endif

  if (expr->id == 571 && 
      strstr(__fninfo->func_name,"DecodeVolHeader"))
    {
      printf("expr #2 %d size %d %s\n",
	     expr->id, size,
	     __fninfo->func_name);      
    }

  s_mod = node->data.mod;
  list = add_node(list, node, (offset % s_mod), size, 1);

  return list;
}


static List
get_aliasnodes_for_expr(IPA_prog_info_t *info, 
			IPA_funcsymbol_info_t * fninfo,
			Expr expr,
			int size)
{
  List list;
  buildcg_t *bcg;

  switch (expr->opcode)
    {
      /* Dependences need only be appended onto a few
       *   kinds of exprs. 
       */
    case OP_var:
    case OP_indr:
    case OP_dot:
    case OP_arrow:
    case OP_index:
      break;
    default:
      return NULL;
      break;
    }

  bcg = get_expr(info, fninfo, expr, NULL);
  if (bcg == NULL)
    return NULL;

  switch(bcg->status)
    {
    case B_VAR:
      /* a, a.f 
       */
      assert(bcg->skew == 0);

      /* Alias: "a" at offset "bcg->offset" */
      list = get_nodeof(info, fninfo, expr, bcg->node, bcg->offset, size);
      break;
    case B_ADDR:
      /* &a 
       */
      assert(bcg->skew == 0);
      /* Alias: NONE */
      
      list = NULL;
      break;
    case B_ASSIGN:
      /* &(*a), &(a->f) 
       */

      /* Alias: technically this is just an addition. For
	 now, I'm assuming that the dependence on the subexpression
	 is sufficient */
      list = NULL;
      break;
    case B_DEREF:
      /* *a, a->f 
       */

      /* Alias: points-to("bcg->node" at "bcg->offset") 
	        NOTE: node is in points-to if target_offset of addr_edge
		matches "bcg->offset". While the desired node is
		the src_node of the addr_edge, the location must include
		the source_offset of the addr_edge
	 If there is any SKEW, the actual points-to is
	 the above with and additional skew 
      */
      list = get_pointsto(info, fninfo, expr, bcg->node, bcg->offset,
			  bcg->skew, size);
      break;
    default:
      assert(0);
    }

  list = call_list_compress_acc(list);
  return list;
}


static buildcg_t *
get_expr(IPA_prog_info_t *info, 
	 IPA_funcsymbol_info_t * fninfo,
	 Expr expr,
	 buildcg_t *bcg)
{
  int check_for_array = 0;
  assert(expr);

  switch (expr->opcode)
    {
    case OP_var:
      bcg = get_var(info, fninfo, expr);
      break;
    case OP_int:
    case OP_string:
    case OP_float:
    case OP_double:
      /* non-variable base */
      dep_skipped = 1;
      return NULL;
      break;
    case OP_call:
      dep_skipped = 1;
      return NULL;
      break;
    case OP_sub:
    case OP_add:
    default:
      bcg = get_expr(info, fninfo, expr->operands, bcg);
    }

  if (bcg == NULL)
    return NULL;

#if 0
  printf("[%s:%d] : ", op_to_value[expr->opcode],expr->id);
  print_bcg(bcg);
  printf(" -> ");
#endif

  switch (expr->opcode)
    {
    case OP_var:
      if (PST_IsFunctionType(info->symboltable, IPA_ExprType(info, expr)) &&
	  !PST_IsPointerType(info->symboltable, IPA_ExprType(info, expr)))
	{
	  IPA_Expr_Addr_Eqn(info, expr, bcg);
	}
      else
	{
	  check_for_array = 1;
	}
      break;
    case OP_addr:
      IPA_Expr_Addr_Eqn(info, expr, bcg);
      check_for_array = 1;
      break;
    case OP_indr:
      IPA_Expr_Deref_Eqn(info, expr, bcg);
      check_for_array = 1;
      break;
    case OP_dot:
      IPA_Expr_Field_Eqn(info, expr, bcg);
      check_for_array = 1;
      break;
    case OP_arrow:
      IPA_Expr_Arrow_Eqn(info, expr, bcg);
      check_for_array = 1;
      break;
    case OP_index:
#if SAFEOFFSET
      IPA_Expr_Handle_Addition(info, expr, bcg);
#endif

      IPA_Expr_Deref_Eqn(info, expr, bcg);
      check_for_array = 1;
      break;
    case OP_compexpr:
    case OP_cast:
      break;
    case OP_add:
    case OP_sub:
#if SAFEOFFSET
      IPA_Expr_Handle_Addition(info, expr, bcg);
#endif
      break;
    default:
      if (IPA_is_ptr_invalidating (expr->opcode))
	return NULL;
      assert(0);
      break;
    }

  if (check_for_array &&
      (PST_IsFunctionType(info->symboltable, IPA_ExprType(info, expr)) ||
       PST_IsArrayType(info->symboltable, IPA_ExprType(info, expr))))
    {
      IPA_bcg_addrof(info, bcg);      
    }

#if 0
  print_bcg(bcg);
  printf("\n");
  fflush(stdout);
  fflush(stderr);
#endif

  return bcg;
}




/**************************************************************
 * strianta: I added this code to annotate the call targets of
 * function pointer calls on the code, like the old CALLNAME
 * attribute.  This is probably not the best place for the code to
 * be...
 */

static void debug_print(char *fmt, ...) {
#if 0
  va_list ap;

  fprintf(stderr, "ST-> ");

  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);

  fprintf(stderr, "\n");
#endif
}


#define CALLNAME_PRAGMA "CALLNAME"

static void add_call_name_pragma(Expr call_site_expr, char *callee_name) {
  Pragma callname_prag;
  Expr callee_str_expr;
  int i;

  callee_str_expr = P_NewStringExpr(callee_name);

  callname_prag = P_FindPragma(call_site_expr->pragma, CALLNAME_PRAGMA);
  if (callname_prag == NULL) {
    debug_print("    Pragma %s does not exist yet on expr %d",
		CALLNAME_PRAGMA, call_site_expr->id);

    callname_prag = P_NewPragmaWithSpecExpr(CALLNAME_PRAGMA, callee_str_expr);
    call_site_expr->pragma = P_AppendPragmaNext(call_site_expr->pragma,
						callname_prag);
  }
  else {
    debug_print("    Pragma %s already exists on expr %d", CALLNAME_PRAGMA,
		call_site_expr->id);
    callname_prag->expr = P_AppendExprNext(callname_prag->expr,
					   callee_str_expr);
  }

  // self check

#if 0
  callname_prag = P_FindPragma(call_site_expr->pragma, CALLNAME_PRAGMA);
  debug_print("    Pragma %s for expr %d now is:", CALLNAME_PRAGMA,
	      call_site_expr->id);
  P_write_pragma(stderr, callname_prag, 10, &i);
  fprintf(stderr, "\n");
#endif

  return;
}


static void add_caller_pragma(FuncDcl fn, IPA_funcsymbol_info_t *caller)
{
  Pragma caller_prag;
  Expr caller_expr, caller_count_expr;
  int found = 0;
 
  caller_prag = P_FindPragma(fn->pragma, CALLER_PRAGMA);
  if (caller_prag == NULL)
    {
      P_punt("    Pragma %s does not exist on func %s", CALLER_PRAGMA,
	     fn->name);
    }
  else
    {
      debug_print("    Pragma %s already exists on func %s", CALLNAME_PRAGMA,
		  fn->name);

      for (caller_expr = caller_prag->expr; caller_expr;
	   caller_expr = caller_expr->next->next)
	{
	  if (strcmp (caller_expr->value.var.name, caller->func_name) == 0)
	    {
	      caller_count_expr = caller_expr->next;
	      caller_count_expr->value.uscalar++;
	      found = 1;
	      break;
	    }
	}

      if (!found)
	{
	  caller_expr = P_NewExprWithOpcode(OP_var);
	  caller_expr->value.var.key = caller->func_key;
	  caller_expr->value.var.name = strdup (caller->func_name);
	  caller_prag->expr = P_AppendExprNext(caller_prag->expr,
					       caller_expr);
	  caller_count_expr = P_NewUIntExpr (1);
	  caller_prag->expr = P_AppendExprNext(caller_prag->expr,
					       caller_count_expr);
	}
    }
}


static void annotate_call_targets(IPA_funcsymbol_info_t *fninfo) {
  IPA_callg_edge_t *call_edge;
  IPA_callsite_t *call_site;
  int call_site_id;
  Expr call_site_expr;
  IPA_funcsymbol_info_t *callee;

  // don't care about library functions and functions with no callees
  if (List_size(fninfo->call_node->callee_edges) == 0)
    return;
  if (fninfo->from_library)
    return;

  List_start(fninfo->call_node->callee_edges);
  while ((call_edge = List_next(fninfo->call_node->callee_edges))) {
    callee = call_edge->callee->fninfo;
    call_site = call_edge->caller_cs;
    call_site_id = call_site->cs_id;
    call_site_expr = call_site->call_expr;

    if (!call_site->indirect)
      continue;

    debug_print("Indirect Edge: caller=%s, callee=%s, cs id=%d, expr id=%d, expr opc=%s",
                fninfo->func_name,
                callee->func_name,
                call_site_id,
                call_site_expr->id,
                P_OpcodeToString(call_site_expr->opcode));

    add_call_name_pragma(call_site_expr, callee->func_name);
  }
}


static void annotate_callers(FuncDcl fn, IPA_funcsymbol_info_t *fninfo)
{
  IPA_callg_edge_t *call_edge;
  IPA_funcsymbol_info_t *caller;

  /* skip functions without callers */
  if (List_size(fninfo->call_node->caller_edges) == 0)
    return;

  List_start(fninfo->call_node->caller_edges);
  while ((call_edge = List_next(fninfo->call_node->caller_edges))) {
    caller = call_edge->caller->fninfo;

    add_caller_pragma(fn, caller);
  }
}
