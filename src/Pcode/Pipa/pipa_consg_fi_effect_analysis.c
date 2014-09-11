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
 *      File:    pipa_consg_fi_effect_analysis.c
 *      Author:  Erik Nystrom
 *      Copyright (c) 2003  Erik Nystrom, Wen-mei Hwu
 *              and The Board of Trustees of the University of Illinois.
 *              All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#include "pipa_consg_fi.h"

#define DB_EFF 0

static IPA_cgraph_t *cg = NULL;
static IPA_cgraph_t *local_sum = NULL; 
static IPA_interface_t *formal_iface = NULL;
static List effect_list = NULL;


static IPA_Hashtab_t *effect_htab = NULL;
static IPA_cgraph_node_t * cmp_src;
static IPA_cgraph_node_t * cmp_dst;
static IPA_cgraph_edgelist_e    cmp_edge_type;
static IPA_cgraph_edge_data_t * cmp_edata;
#define GETKEY(s,d,to,as,so)   (((int)(long)(s) ^ (int)(long)(d)) ^ ((as ^ to ^ so) << 16)) 


typedef struct effect_t 
{
  IPA_cgraph_edgelist_e   edge_type;
  IPA_cgraph_node_t      *src;
  IPA_cgraph_node_t      *dst;
  IPA_cgraph_edge_data_t  edata;
} effect_t;


void
debug_print(char *str1, IPA_cgraph_node_t *src,
	    IPA_cgraph_node_t *dst, char *str2)
{
  printf("%s ", str1);
  if (dst)
    IPA_cg_node_print(stdout, dst, IPA_PRINT_ASCI);
  if (dst && src)
    printf(" <- ");
  if (src)
    IPA_cg_node_print(stdout, src, IPA_PRINT_ASCI);
  printf("%s",str2);
}

static IPA_cgraph_node_t *
copy_node(IPA_cgraph_t *into_cg,  
	  IPA_cgraph_node_t *node,
	  int node_flags)
{
  IPA_cgraph_node_t *new_node;

  /* Get the node in summary graph */
  new_node = IPA_consg_ensure_node_d(into_cg, &node->data,
				     (node->flags | node_flags |
				      IPA_CG_NODE_FLAGS_SUMMARY));

  return new_node;
}


#if DB_EFF
static void
debug_effect(char *str1, effect_t *effect, char *str2)
{
  printf("%s : %d ",str1, effect->edge_type);
  IPA_cg_node_print(stdout, effect->dst, IPA_PRINT_ASCI);
  printf(" <-%d,%d,%d- ",
	 effect->edata.consg.target_offset,
	 effect->edata.consg.assign_size,
	 effect->edata.consg.source_offset);
  IPA_cg_node_print(stdout, effect->src, IPA_PRINT_ASCI);
  printf("%s",str2);
}
#endif

static void
SE_addto_summary(IPA_cgraph_edgelist_e edge_type, 
		 IPA_cgraph_node_t *src_node, 
		 IPA_cgraph_node_t *dst_node,
		 IPA_cgraph_edge_data_t *edata)
{
  assert(src_node->cgraph == cg && dst_node->cgraph == cg);
  if (IPA_FLAG_ISSET(src_node->flags, IPA_CG_NODE_FLAGS_GLOBAL) ||
      IPA_FLAG_ISSET(dst_node->flags, IPA_CG_NODE_FLAGS_GLOBAL))
    {
      assert(0);
    }
  if (IPA_FLAG_ISSET(src_node->flags, IPA_CG_NODE_FLAGS_NOCNTXT) ||
      IPA_FLAG_ISSET(dst_node->flags, IPA_CG_NODE_FLAGS_NOCNTXT))
    {
      assert(0);
    }

  IPA_FLAG_SET(src_node->flags, EA_PERMANENT);
  IPA_FLAG_SET(dst_node->flags, EA_PERMANENT);

#if DB_EFF
  printf("Final Effect : ");
  IPA_cg_node_print(stdout, dst_node, IPA_PRINT_ASCI);
  printf(" <- %14s %d,%d,%d - ",
	 edge_types[edge_type],
	 edata->consg.target_offset,
	 edata->consg.assign_size,
	 edata->consg.source_offset);
  IPA_cg_node_print(stdout, src_node, IPA_PRINT_ASCI);
  printf("\n");
#endif

  IPA_consg_ensure_edge_d (edge_type, 
			   copy_node(local_sum, src_node, 0),
			   copy_node(local_sum, dst_node, 0),
			   edata,
			   (IPA_CG_EDGE_FLAGS_EXPLICIT |
			    IPA_CG_EDGE_FLAGS_HZ));
}


static int
SE_effect_compare(void * effect)
{
  if ( (((effect_t*)effect)->src == cmp_src) &&
       (((effect_t*)effect)->dst == cmp_dst) &&
       (((effect_t*)effect)->edge_type == cmp_edge_type) &&
       (((effect_t*)effect)->edata.source_offset == 
	cmp_edata->source_offset) &&
       (((effect_t*)effect)->edata.target_offset == 
	cmp_edata->target_offset) &&       
       (((effect_t*)effect)->edata.assign_size == 
	cmp_edata->assign_size)
       )
    return 1;
  return 0;
}

static effect_t *
SE_new_effect(IPA_cgraph_edgelist_e edge_type, 
	      IPA_cgraph_node_t *src, 
	      IPA_cgraph_node_t *dst,
	      IPA_cgraph_edge_data_t  *edata)
{
  effect_t *new_effect;
  void *item;
  int key;

  /* Don't follow inter-graph edges through globals into
     other graphs */
  if (src->cgraph != cg || dst->cgraph != cg)
    return NULL;

  /* Has the effect already been handled
   */
  key = GETKEY(src, dst, 
	       edata->target_offset,
	       edata->assign_size,
	       edata->source_offset);
  cmp_src = src;
  cmp_dst = dst;
  cmp_edge_type = edge_type;
  cmp_edata = edata;
  item = IPA_htab_find(effect_htab, key, SE_effect_compare);

  if (item != NULL)
    return NULL;

  /* Create new effect
   */
  new_effect = calloc(1,sizeof(effect_t));
  new_effect->edge_type = edge_type;
  new_effect->src = src;
  new_effect->dst = dst;
  new_effect->edata.target_offset = edata->target_offset;
  new_effect->edata.assign_size = edata->assign_size;
  new_effect->edata.source_offset = edata->source_offset;

#if DB_EFF
  debug_effect("New Effect", new_effect, "\n");
#endif

  effect_list = List_insert_last(effect_list, new_effect);

  /* Add effect to the history
   */
  if (!effect_htab)
    effect_htab = IPA_htab_new(6);
  effect_htab = IPA_htab_insert(effect_htab, new_effect, key);

  return new_effect;
}


static void
SE_addedgeto_effectlist(IPA_cgraph_edge_t *edge)
{
  SE_new_effect(edge->src_elist->edge_type, 
		edge->src_elist->node,
		edge->dst_elist->node,
		&edge->data);
}


static void
SE_addto_effectlist(IPA_cgraph_edgelist_e edge_type, 
		    IPA_cgraph_node_t *src, 
		    IPA_cgraph_node_t *dst,
		    IPA_cgraph_edge_data_t  *edata)
{
  SE_new_effect(edge_type, src, dst, edata);
}


static void
SE_free_effects()
{
  effect_t            *effect;
  IPA_HTAB_ITER eiter;

  IPA_HTAB_START(eiter, effect_htab);
  IPA_HTAB_LOOP(eiter)
    {
      effect = IPA_HTAB_CUR(eiter);
      free(effect);
    }

  IPA_htab_free(effect_htab);
  effect_htab = NULL;
}




static void
SE_initial_effects()
{
  IPA_cgraph_edge_t *edge;
  IPA_cgraph_node_t *src_node;
  IPA_cgraph_node_t *dst_node;
  IPA_cgraph_edge_list_t *elist;
  IPA_HTAB_ITER niter;
  IPA_HTAB_ITER eiter;
  int i, formal_bound, formal_id;

#if DB_EFF
  printf("\neffect-init\n");
#endif

  /* SURVIVES
   */

  /* Params */
  formal_bound = IPA_interface_get_num_params (formal_iface);
  for (i = 0; i < formal_bound; i++)
    {
      formal_id = IPA_interface_get_param_id (formal_iface, i);
      src_node = IPA_consg_find_node (cg, formal_id, 1);
      if (!src_node)
	continue;
#if DB_EFF
      printf("Survive (PRM): ");
      IPA_cg_node_print(stdout,src_node, IPA_PRINT_ASCI);
      printf("\n");
#endif
      IPA_FLAG_SET(src_node->flags, (EA_SURVIVES | EA_PERMANENT));
   }
  
  /* Return */
  formal_id = IPA_interface_get_ret_id (formal_iface);
  if (formal_id)
    {
      src_node = IPA_consg_find_node (cg, formal_id, 1);
      if (src_node)
	{
#if DB_EFF
	  printf("Survive (RET): ");
	  IPA_cg_node_print(stdout,src_node, IPA_PRINT_ASCI);
	  printf("\n");
#endif
	  IPA_FLAG_SET(src_node->flags, (EA_SURVIVES | EA_PERMANENT));
	}
    }

  IPA_HTAB_START(niter, cg->nodes);
  IPA_HTAB_LOOP(niter)
    {
      src_node = IPA_HTAB_CUR(niter);
 
      /* return(u), esc(u), param(u), coni(u)
       * ------------------------------------
       *        survive(u)
       */
      if (IPA_FLAG_ISSET(src_node->flags, EA_LFULL_ESC))
	{
#if DB_EFF
	  printf("Survive (GBL): ");
	  IPA_cg_node_print(stdout,src_node, IPA_PRINT_ASCI);
	  printf("\n");
#endif
	  IPA_FLAG_SET(src_node->flags, (EA_SURVIVES | EA_PERMANENT));
	}
    }

  /* EFFECTS and SURVIVES
   */
  IPA_HTAB_START(niter, cg->nodes);
  IPA_HTAB_LOOP(niter)
    {
      src_node = IPA_HTAB_CUR(niter);
      
      for (elist = src_node->first_list; elist; elist = elist->nxt_list)
	{
	  IPA_HTAB_START(eiter, elist->out);
	  IPA_HTAB_LOOP(eiter)
	    {
	      edge = IPA_HTAB_CUR(eiter);
	      
	      dst_node = edge->dst_elist->node;
	      if (dst_node->cgraph != cg)
		continue;
	      
	      switch(elist->edge_type)
		{
		case SKEW:
		case ASSIGN:
		  /* u := v , survive(u) cont(v) */
		  if (IPA_FLAG_ISSET(src_node->flags, (EA_LCONT_ESC)) &&
		      IPA_FLAG_ISSET(dst_node->flags, EA_SURVIVES))
		    {
		      SE_addedgeto_effectlist(edge);
		    }
		  break;
		case ASSIGN_ADDR:
		  /* u := &v, survive(u) esc(v) */
		  if (IPA_FLAG_ISSET(src_node->flags, (EA_LFULL_ESC)) &&
		      IPA_FLAG_ISSET(dst_node->flags, EA_SURVIVES))
		    {
		      SE_addedgeto_effectlist(edge);
		    }
		  break;
		case ASSIGN_DEREF:
		  /* u := *v, survive(u) cont(v) */
		  if (IPA_FLAG_ISSET(src_node->flags, (EA_LCONT_ESC)) &&
		      IPA_FLAG_ISSET(dst_node->flags, EA_SURVIVES))
		    {
		      SE_addedgeto_effectlist(edge);
		    }
		  break;
		case DEREF_ASSIGN:
		  /* *u := v, cont(u) cont(v) */
		  if (IPA_FLAG_ISSET(src_node->flags,(EA_LCONT_ESC)) &&
		      IPA_FLAG_ISSET(dst_node->flags,(EA_LCONT_ESC)))
		    {
		      SE_addedgeto_effectlist(edge);
		    }
		  break;
		default:
		  assert(0);
		}
	    }
	} /* EDGES */
    } /* NODES */

#if DB_EFF
  printf("effect-init-end\n");
#endif
}


int
SE_reflect_effect(effect_t *effect, int issrc)
{
  IPA_cgraph_node_t      *node;
  IPA_cgraph_edge_t      *edge;
  IPA_cgraph_edge_list_t *elist;
  IPA_cgraph_edge_list_t *in_elist;
  IPA_HTAB_ITER eiter;
  int nonassign_in;

#if DB_EFF
  debug_effect("# Reflect Effect", effect, " ");
  printf("%d \n", issrc);
#endif

  if (issrc)
    node = effect->src;
  else
    node = effect->dst;

  if (IPA_FLAG_ISSET(node->flags, EA_PERMANENT))
    {
      /* This node is already in the summary, or
	 is context-insensitive - just add effect */
#if DB_EFF
      debug_effect("Final Effect (PERM)", effect, "\n");
#endif
      return 0;
    }
  if (node->cgraph != cg)
    {
#if DB_EFF
      debug_effect("Skip Effect (CONS)", effect, "\n");
#endif
      return 1;
    }

  nonassign_in = 0;
  in_elist = NULL;
  for (elist = node->first_list; elist; elist = elist->nxt_list)
    {
      if (elist->edge_type == ASSIGN)
	{
	  in_elist = elist;
	}
      else if ((elist->edge_type != DEREF_ASSIGN) &&
	       IPA_htab_size(elist->in) > 0)
	{
	  nonassign_in = 1;
	}
    }

  if (!nonassign_in && in_elist)
    {
      elist = in_elist;
      IPA_HTAB_START(eiter, elist->in);
      IPA_HTAB_LOOP(eiter)
	{
	  edge = IPA_HTAB_CUR(eiter);
	  if (IPA_FLAG_ISSET(edge->src_elist->node->flags,
			     IPA_CG_NODE_FLAGS_PARAM))
	    {
	      nonassign_in = 1;
	      break;
	    }
	}
    }

  if (effect->edge_type != ASSIGN &&
      nonassign_in)
    {
#if DB_EFF
      printf("Break effect : edge is non-ASSIGN and effect is non-ASSIGN\n");
#endif
      for (elist = node->first_list; elist; elist = elist->nxt_list)
	{
	  switch(elist->edge_type)
	    {
	    case DEREF_ASSIGN:
	      break;
            case SKEW:
	    case ASSIGN:
	    case ASSIGN_DEREF:
	      IPA_HTAB_START(eiter, elist->in);
	      IPA_HTAB_LOOP(eiter)
		{
		  edge = IPA_HTAB_CUR(eiter);
		  if (IPA_FLAG_ISSET(edge->flags, IPA_CG_EDGE_FLAGS_UD))
		    continue;
		  if (!IPA_FLAG_ISSET(edge->src_elist->node->flags, 
				      (EA_LCONT_ESC)))
		    continue;
    
		  SE_addto_effectlist(elist->edge_type, 
				      edge->src_elist->node, 
				      node, &edge->data);
		}
	      break;
	    case ASSIGN_ADDR:
	      IPA_HTAB_START(eiter, elist->in);
	      IPA_HTAB_LOOP(eiter)
		{
		  edge = IPA_HTAB_CUR(eiter);
		  if (IPA_FLAG_ISSET(edge->flags, IPA_CG_EDGE_FLAGS_UD))
		    continue;
		  if (!IPA_FLAG_ISSET(edge->src_elist->node->flags, 
				      (EA_LFULL_ESC)))
		    continue;

		  SE_addto_effectlist(elist->edge_type, 
				      edge->src_elist->node, 
				      node, &edge->data);
		}
	      break;
	    default:
	      assert(0);
	    }
	}
      return 0;
    }
  else if (effect->edge_type == ASSIGN)
    {
      /* Propagate ASSIGN effect up through ANY edge */
#if DB_EFF
      printf("Propagate ASSIGN effect up through ANY edge\n");
#endif
      for (elist = node->first_list; elist; elist = elist->nxt_list)
	{
	  IPA_HTAB_START(eiter, elist->in);
	  IPA_HTAB_LOOP(eiter)
	    {
	      edge = IPA_HTAB_CUR(eiter);
	      if (elist->edge_type == DEREF_ASSIGN)
		continue;
	      if (IPA_FLAG_ISSET(edge->flags, IPA_CG_EDGE_FLAGS_UD))
		continue;
	      if (IPA_FLAG_ISCLR(edge->src_elist->node->flags, 
				 (EA_LCONT_ESC)))
		continue;
	      if (elist->edge_type == ASSIGN_ADDR &&
		  (!IPA_FLAG_ISSET(edge->src_elist->node->flags, 
				   (EA_LFULL_ESC))))
		continue;
	      
	      if (issrc)
		{
		  SE_addto_effectlist(elist->edge_type,
				      edge->src_elist->node,
				      effect->dst, 
				      &edge->data);
		}
	      else
		{
		  assert(0);
		}
	    }
	}     
    }
  else
    {
      /* Propagate ANY effect up through ASSIGN edge */
#if DB_EFF
      printf("Propagate ANY effect up through ASSIGN edge\n");
#endif
      for (elist = node->first_list; elist; elist = elist->nxt_list)
	{
	  IPA_HTAB_START(eiter, elist->in);
	  IPA_HTAB_LOOP(eiter)
	    {
	      edge = IPA_HTAB_CUR(eiter);
	      if (elist->edge_type == DEREF_ASSIGN)
		continue;
	      if (IPA_FLAG_ISSET(edge->flags, IPA_CG_EDGE_FLAGS_UD))
		continue;
	      if (IPA_FLAG_ISCLR(edge->src_elist->node->flags, 
				 (EA_LCONT_ESC)))
		continue;

	      assert(elist->edge_type == ASSIGN);
	      if (effect->edge_type == ASSIGN_ADDR)
		{
		  assert(!IPA_FLAG_ISSET(edge->src_elist->node->flags, 
					 (EA_LFULL_ESC)));
		  continue;
		}

	      if (issrc)
		{
		  SE_addto_effectlist(effect->edge_type, 
				      edge->src_elist->node, 
				      effect->dst, 
				      &effect->edata);
		}
	      else
		{
		  assert(effect->edge_type == DEREF_ASSIGN);
		  assert(!IPA_FLAG_ISSET(edge->src_elist->node->flags,
					 IPA_CG_NODE_FLAGS_PARAM));
		  
		  SE_addto_effectlist(effect->edge_type,
				      effect->src, 
				      edge->src_elist->node, 
				      &effect->edata);
		}
	    }
	}
    }

  return 1;
}



static void
SE_process_effect(effect_t *effect)
{
  switch(effect->edge_type)
    {
    case DEREF_ASSIGN:
      if (SE_reflect_effect(effect, 1) == 0)
	{
	  if (SE_reflect_effect(effect, 0) == 0)
	    {
	      /* Effect not skipped */
	      SE_addto_summary(effect->edge_type, effect->src,
			       effect->dst, &effect->edata);
	    }
	}
      break;

    case ASSIGN_DEREF:
    case ASSIGN:
    case SKEW:
      if (SE_reflect_effect(effect, 1) == 0)
	{
	  /* Effect not skipped */
	  SE_addto_summary(effect->edge_type, effect->src,
			   effect->dst, &effect->edata);
	}
      break;
      
    case ASSIGN_ADDR:
      SE_addto_summary(effect->edge_type, effect->src,
		       effect->dst, &effect->edata);
      break;
      
    default:
      assert(0);
    }
}

static void
create_compact_summary()
{
  effect_t *effect;
  int cnt = 0;

  local_sum = IPA_cg_cgraph_new(cg->data.fninfo);
  
  effect_list = NULL;
  SE_initial_effects();

  printf("\nInitial Effects: %d (%d cg nodes)\n",
	 List_size(effect_list),
	 IPA_htab_size(cg->nodes));

  List_start(effect_list);
  while ((effect = List_next(effect_list)))
    {
      effect_list = List_delete_current(effect_list);
      SE_process_effect(effect);
      cnt++;
    }

  SE_free_effects();
  List_reset(effect_list);

  printf("Effects: %d\n",cnt);
}


void
IPA_consg_fi_summarize (IPA_prog_info_t * __info, 
			IPA_cgraph_t * __cg,
			IPA_interface_t *__iface,
			IPA_funcsymbol_info_t *__fninfo,
			IPA_cgraph_t **local_sum_ptr, 
			char *func_name)
{
  List setlist;
  IPA_cgraph_node_t *node;
  int base_count, comp_count, red_count;

  cg   = __cg;
  formal_iface = __iface;

  /*************************************
   * Mark nodes with escape and survive
   *  flags
   */

  IPA_init_escape_analysis(__info, cg, formal_iface, __fninfo);
  setlist = IPA_do_escape_analysis();
  
  /* GENERIC flags are supposed to be cleared after use.
     Double check that things appear ok.
  */  
  IPA_cg_nodes_assert_clr_flags(cg, (EA_SURVIVES | EA_PERMANENT));

  create_compact_summary();

#if 1
  printf("OPTI: Initial of [%s] : (Base)  %d nodes\n",
	 func_name, 
	 IPA_htab_size(cg->nodes));
  base_count = IPA_htab_size(cg->nodes);
  printf("OPTI: Summary of [%s] : (Comp)  %d nodes\n",
	 func_name, 
	 IPA_htab_size(local_sum->nodes));
  comp_count = IPA_htab_size(local_sum->nodes);
#if 0
  if (strstr(func_name, "jpeg_alloc_quant_table"))
    {
      char name[256];
      sprintf(name, "%s.%s.COMP%d", 
	      func_name,
	      "fi",
	      debug.round);
      IPA_cg_DVprint (local_sum, name, IPA_CG_ETYPEALL_FLAG);
      if (debug.round == 2)
	exit(1);
    }
#endif
#endif
#if 1
  IPA_cycle_detection(local_sum, 
		      (IPA_CG_NODE_FLAGS_GLOBAL |
		       IPA_CG_NODE_FLAGS_NOCNTXT |
		       IPA_CG_NODE_FLAGS_CALLEE),
		      CD_DELETE_ALL);
  printf("OPTI: Summary of [%s] : (Cycle) %d nodes\n",
	 func_name, 
	 IPA_htab_size(local_sum->nodes));
#endif
#if 1
  IPA_consg_fi_remove_redundancy(local_sum);
  printf("OPTI: Summary of [%s] : (Red)   %d nodes\n",
	 func_name, 
	 IPA_htab_size(local_sum->nodes));
  red_count = IPA_htab_size(local_sum->nodes);
#endif

  if (red_count > 0)
    {
      int i;

      if (base_count <= 20)
	i = 0;
      else if (base_count <= 2000)
	i = 1;
      else 
	i = 2;
      
      __info->nonzero_occur[i]++;
      __info->total_base_count[i] += base_count;
      __info->total_comp_count[i] += comp_count;
      __info->total_red_count[i] += red_count;
    }
  else
    __info->zero_occur++;

  *local_sum_ptr = local_sum;

  /* Use the touched list to clear all generic flags
   * that were set.
   */
  List_start(setlist);
  while ((node = List_next(setlist)))
    {
      IPA_FLAG_CLR(node->flags, (EA_FLAGSET |
				 EA_INPROGRESS | 
				 EA_LCONT_ESC |
				 EA_LPROP_ESC | 
				 EA_RETPROP_ESC |
				 EA_LFULL_ESC |
				 EA_SURVIVES | 
				 EA_PERMANENT));
#if 0
      printf("CLRSET:");
      IPA_cg_node_print(stdout,node,IPA_PRINT_ASCI);
      printf(" %X \n", node->flags);
#endif
    }
  List_reset(setlist);  
}

#endif

