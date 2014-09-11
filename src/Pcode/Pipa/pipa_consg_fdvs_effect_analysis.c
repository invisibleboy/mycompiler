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
 *      File:    pipa_consg_fdvs_effect_analysis.c
 *      Author:  Erik Nystrom
 *      Copyright (c) 2003  Erik Nystrom, Wen-mei Hwu
 *              and The Board of Trustees of the University of Illinois.
 *              All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#include "pipa_consg_fdvs.h"
#include "pipa_consg_construct.h"

/* These should not overlap with the escape analysis flags */

#define TARGET(s)    (s).target_offset
#define TGTSTR(s)    (s).target_stride
#define SIZE(s)      (s).assign_size
#define SOURCE(s)    (s).source_offset
#define SRCSTR(s)    (s).source_stride
#define BOUND(l,v,u) ((l <= v) && (v <= u)) 
#define MIN(a,b)     ((a<b)?(a):(b))
   
#define DB_EFF 0

static IPA_cgraph_t *cg = NULL;
static IPA_cgraph_t *local_sum = NULL;
static IPA_interface_t *formal_iface = NULL;
static IPA_prog_info_t *prog_info;
static IPA_funcsymbol_info_t *fn_info;

#define HISTORY 1
#if HISTORY
static IPA_Hashtab_t *effect_htab = NULL;
static IPA_cgraph_node_t * cmp_src;
static IPA_cgraph_node_t * cmp_dst;
static IPA_cgraph_edgelist_e    cmp_edge_type;
static IPA_cgraph_edge_data_t * cmp_edata;
static int cmp_skew;
#define GETKEY(s,d,to,as,so)   (((int)(long)(s) ^ (int)(long)(d)) ^ ((as ^ to ^ so) << 16)) 
#else
static List effect_hist = NULL;
#endif

static List effect_list = NULL;

typedef struct effect_t 
{
  IPA_cgraph_edgelist_e   edge_type;
  IPA_cgraph_node_t      *src;
  IPA_cgraph_node_t      *dst;
  IPA_cgraph_edge_data_t  edata;
  int                     skew;
} effect_t;

#if DB_EFF
static void
debug_effect(char *str1, effect_t *effect, char *str2)
{
  printf("%s : %d ",str1, effect->edge_type);
  IPA_cg_node_print(stdout, effect->dst, IPA_PRINT_ASCI);
  printf(" <- %14s %d,%d,%d [%d] - ",
	 edge_types[effect->edge_type],
	 effect->edata.target_offset,
	 effect->edata.assign_size,
	 effect->edata.source_offset,
	 effect->skew);
  IPA_cg_node_print(stdout, effect->src, IPA_PRINT_ASCI);
  printf("%s",str2);
}
#endif

#if 0
static void
debug_print_edge(IPA_cgraph_edge_t *e)
{
  IPA_cg_node_print(stdout,e->dst_elist->node, IPA_PRINT_ASCI);
  printf(" <- %14s %d,%d,%d -",
	 edge_types[e->dst_elist->edge_type],
	 e->data.target_offset, 
	 e->data.assign_size,
	 e->data.source_offset);
  IPA_cg_node_print(stdout,e->src_elist->node, IPA_PRINT_ASCI);
  printf("\n");
}
#endif

#if 0
static void
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
#endif

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



static void
SE_addto_summary(IPA_cgraph_edgelist_e edge_type, 
		 IPA_cgraph_node_t *src_node, 
		 IPA_cgraph_node_t *dst_node, 
		 IPA_cgraph_edge_data_t *edata)
{
  IPA_cgraph_node_t *ls;
  IPA_cgraph_node_t *ld;

  if (src_node->cgraph == local_sum)
    {
      ls = src_node;
    }
  else
    {
      assert(src_node->cgraph == cg);
      ls = copy_node(local_sum, src_node, 0);
    }

  if (dst_node->cgraph == local_sum)
    {
      ld = dst_node;
    }
  else    
    {
      assert(dst_node->cgraph == cg);
      ld = copy_node(local_sum, dst_node, 0);
    }

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
	 edata->target_offset,
	 edata->assign_size,
	 edata->source_offset);
  IPA_cg_node_print(stdout, src_node, IPA_PRINT_ASCI);
  printf("\n");
#endif

  IPA_consg_ensure_edge_d (edge_type, ls, ld,
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
       (((effect_t*)effect)->edata.source_offset == cmp_edata->source_offset) &&
       (((effect_t*)effect)->edata.target_offset == cmp_edata->target_offset) &&       
       (((effect_t*)effect)->edata.assign_size == cmp_edata->assign_size) &&
       (((effect_t*)effect)->skew == cmp_skew)
       )
    return 1;
  return 0;
}

static effect_t *
SE_new_effect(IPA_cgraph_edgelist_e edge_type, 
	      IPA_cgraph_node_t *src, 
	      IPA_cgraph_node_t *dst,
	      IPA_cgraph_edge_data_t  *edata,
	      int skew)
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
  cmp_skew = skew;
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
  new_effect->edata.target_stride = edata->target_stride;
  new_effect->edata.assign_size = edata->assign_size;
  new_effect->edata.source_offset = edata->source_offset;
  new_effect->edata.source_stride = edata->source_stride;
  new_effect->skew = skew;

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
		&edge->data,
		0);
}


static void
SE_addto_effectlist(IPA_cgraph_edgelist_e edge_type, 
		    IPA_cgraph_node_t *src, 
		    IPA_cgraph_node_t *dst,
		    IPA_cgraph_edge_data_t  *edata,
		    int skew)
{
  SE_new_effect(edge_type, src, dst, edata, skew);
}


static void
SE_free_effects()
{
  effect_t            *effect;
#if HISTORY
  IPA_HTAB_ITER       iter;
#endif

#if HISTORY
  IPA_HTAB_START(iter, effect_htab);
  IPA_HTAB_LOOP(iter)
    {
      effect = IPA_HTAB_CUR(iter);
      free(effect);
    }

  IPA_htab_free(effect_htab);
  effect_htab = NULL;
#else
  List_start(effect_hist);
  while ((effect = List_next(effect_hist)))
    {
      free(effect);
    }
  List_reset(effect_hist);
  effect_hist = NULL;
#endif
}




static void
DFS_node(IPA_cgraph_node_t *node)
{
  IPA_cgraph_edge_list_t *elist;
  IPA_cgraph_edge_t      *edge;
  IPA_HTAB_ITER eiter;

  for (elist = node->first_list; elist; elist = elist->nxt_list)
    {
      if (elist->edge_type == ASSIGN_ADDR ||
	  elist->edge_type == DEREF_ASSIGN)
	continue;
	  
      IPA_HTAB_START(eiter, elist->out);
      IPA_HTAB_LOOP(eiter)
	{
	  edge = IPA_HTAB_CUR(eiter);
	  if (IPA_FLAG_ISSET(edge->flags, (IPA_CG_EDGE_FLAGS_UD |
					   IPA_CG_EDGE_FLAGS_GBL)))
	    continue;
	  if (!IPA_FLAG_ISSET(edge->src_elist->node->flags,
			      (EA_LCONT_ESC)))
	    continue;

	  if (elist->edge_type != ASSIGN)
	    {
	      if (edge->dst_elist->node->misc.depth > node->misc.depth + 1)
		{
		  edge->dst_elist->node->misc.depth = node->misc.depth + 1;
		  DFS_node(edge->dst_elist->node);
		}
	    }
	  else
	    {
	      if (edge->dst_elist->node->misc.depth > node->misc.depth)
		{
		  edge->dst_elist->node->misc.depth = node->misc.depth;
		  DFS_node(edge->dst_elist->node);
		}	      
	    }
	}
    }
}


/***********************************************************************
 * Initial effect generation
 ***********************************************************************/

static void
SE_initial_effects()
{
  IPA_cgraph_edge_t *edge;
  IPA_cgraph_node_t *src_node;
  IPA_cgraph_node_t *dst_node;
  IPA_cgraph_edge_list_t *elist;
  IPA_HTAB_ITER niter;
  IPA_HTAB_ITER eiter;
  List initial_node_list = NULL;
  List da_list = NULL;
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
      if (!src_node || fn_info->is_noexit)
	continue;
#if DB_EFF
      printf("Survive (PRM): ");
      IPA_cg_node_print(stdout,src_node, IPA_PRINT_ASCI);
      printf("\n");
#endif
      IPA_FLAG_SET(src_node->flags, (EA_SURVIVES | EA_PERMANENT));
      initial_node_list = List_insert_last(initial_node_list, src_node);
   }
  
  /* Return */
  formal_id = IPA_interface_get_ret_id (formal_iface);
  if (formal_id)
    {
      src_node = IPA_consg_find_node (cg, formal_id, 1);
      if (src_node && !fn_info->is_noexit)
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
      /* return(u), esc(u), param(u)
       * ------------------------------------
       *        survive(u)
       */
      if (IPA_FLAG_ISSET(src_node->flags, (EA_LFULL_ESC)))
	{
#if DB_EFF
	  printf("Survive (GBL): ");
	  IPA_cg_node_print(stdout,src_node, IPA_PRINT_ASCI);
	  printf("\n");
#endif
	  IPA_FLAG_SET(src_node->flags, (EA_SURVIVES | EA_PERMANENT));
	  initial_node_list = List_insert_last(initial_node_list, src_node);
	}
    }

  /* EFFECTS and SURVIVES
   */
  
  IPA_HTAB_START(niter, cg->nodes);
  IPA_HTAB_LOOP(niter)
    {
      src_node = IPA_HTAB_CUR(niter);
      src_node->misc.depth = (unsigned short)(-1);
      for (elist = src_node->first_list; elist; elist = elist->nxt_list)
	{
	  IPA_HTAB_START(eiter, elist->out);
	  IPA_HTAB_LOOP(eiter)
	    {
	      edge = IPA_HTAB_CUR(eiter);
	      dst_node = edge->dst_elist->node;
#if 0
	      /* Might be an edge from an inter-graph node */
	      if (src_node->cgraph != cg && dst_node->cgraph != cg)
		continue;
#endif
	      if (IPA_FLAG_ISSET(edge->flags, (IPA_CG_EDGE_FLAGS_UD |
					       IPA_CG_EDGE_FLAGS_GBL)))
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
		  if (IPA_FLAG_ISSET(src_node->flags,(EA_LCONT_ESC|EA_LCONT_ESCLCL)) &&
		      IPA_FLAG_ISSET(dst_node->flags,(EA_LCONT_ESC)))
		    {
		      SE_addedgeto_effectlist(edge);
		      da_list = List_insert_last(da_list, src_node);
		    }
		  break;
		default:
		  assert(0);
		}
	    }
	} /* EDGES */
    } /* NODES */
  List_reset(da_list);

  List_start(initial_node_list);
  while ((src_node = List_next(initial_node_list)))
    {
      src_node->misc.depth = 0;
      DFS_node(src_node);
    }
  List_reset(initial_node_list);

#if DB_EFF
  printf("effect-init-end\n");
#endif
}



/***********************************************************************
 * Core effect compactor
 ***********************************************************************/

static IPA_cgraph_node_t*
new_temp_node(IPA_cgraph_t *cg, int flags)
{
  IPA_symbol_info_t *syminfo;
  IPA_cgraph_node_t *tmp_node;


  syminfo = IPA_new_tmp_var(prog_info, cg->data.fninfo, 0);
  tmp_node = IPA_consg_ensure_node(cg, syminfo->id, 1,
				   IPA_Pcode_sizeof(prog_info, syminfo->type_key),
				   syminfo,
				   (flags | IPA_CG_NODE_FLAGS_TEMP));

  printf("  COMPACTTEMP: %d %s\n",
	 syminfo->id, 
	 syminfo->symbol_name);

  return tmp_node; 
}

#define NO_EFFECT   0
#define HAS_EFFECT  1

static int
compact_effect_edge(effect_t *effect, IPA_cgraph_edge_t *edge, 
		    IPA_cgraph_edgelist_e edge_type, 
		    int issrc, int retain_oldeffect, int update)
{
  IPA_cgraph_node_t *n_src = NULL;
  IPA_cgraph_node_t *n_dst = NULL;
  IPA_cgraph_edgelist_e n_type = 0;
  IPA_cgraph_edge_data_t n_data;
  int n_skew;
  int t1, z1, s1, t2, z2, s2;
  int ts1, ss1, ts2, ss2;
  int compaction_action;

  compaction_action = NO_EFFECT;

  /* All addr effects are from initial effects and
     should be kept */
  if (effect->edge_type == ASSIGN_ADDR)
    {
      /* This is not really an error but reaching this is
	 an inefficiency */
      assert(0);
      return 1;
    }

  /* AA and DA edges are added during initial effect
     generation only */
  if ((edge_type == ASSIGN_ADDR 
      /* FIXTEST */
      && !(issrc && effect->edge_type == DEREF_ASSIGN)
      ) || edge_type == DEREF_ASSIGN)
    {
      /* do nothing */
#if DB_EFF > 1
      printf("AD/AA : no action\n");
#endif      
      return 0;
    }

  /* Override if src is PARAM and we're handling the
   *   dst of an DEREF_ASSIGN  */
  if (!issrc && IPA_FLAG_ISSET(edge->src_elist->node->flags,
			       IPA_CG_NODE_FLAGS_PARAM))
    {
#if DB_EFF > 1
      printf("FORCE - ");
#endif      
      retain_oldeffect = 1;
    }

  t1 = TARGET(effect->edata);
  ts1 = TGTSTR(effect->edata);
  z1 = SIZE(effect->edata);
  ss1 = SRCSTR(effect->edata);
  s1 = SOURCE(effect->edata);

  n_skew = effect->skew;

  t2 = TARGET(edge->data);
  ts2 = TGTSTR(edge->data);
  z2 = SIZE(edge->data);
  ss2 = SRCSTR(edge->data);
  s2 = SOURCE(edge->data);

  switch(effect->edge_type)
    {
      /***************** ASSIGN ********************/
    case ASSIGN:
      assert(issrc);
      n_src = edge->src_elist->node;
      n_dst = effect->dst;

      if (ss1 != 0 || ts2 != 0)
	{
	  /* Meshing stride, retain effect */
	  compaction_action = HAS_EFFECT;
	  retain_oldeffect = 1;
	  break;
	}

      switch(edge_type)
	{
	case ASSIGN:
	  n_type = ASSIGN;
	  if (BOUND(t2, s1, t2 + z2 - IPA_POINTER_SIZE))
	    {
#if DB_EFF > 1
	      printf("A-A : new 1");
#endif
	      compaction_action = HAS_EFFECT;
	      TARGET(n_data) = t1;
	      TGTSTR(n_data) = ts1;
	      SIZE(n_data) = MIN(z1, z2 - (s1-t2));
	      SRCSTR(n_data) = ss2;
	      SOURCE(n_data) = s2 + (s1-t2);
	    }
	  else if (BOUND(s1, t2, s1 + z1 - IPA_POINTER_SIZE))
	    {
#if DB_EFF > 1
	      printf("A-A : new 2");
#endif
	      compaction_action = HAS_EFFECT;
	      TARGET(n_data) = t1 + (t2-s1);
	      TGTSTR(n_data) = ts1;
	      SIZE(n_data) = MIN(z2, z1 - (t2-s1));
	      SRCSTR(n_data) = ss2;
	      SOURCE(n_data) = s2;
	    }
#if DB_EFF > 1
	  else
	    printf("A-A : none");
#endif
	  break;
	case SKEW:
	  n_type = SKEW;
	  if (BOUND(s1, t2, s1 + z1 - IPA_POINTER_SIZE))
	    {
#if DB_EFF > 1
	      printf("A-K : new 1");
#endif
	      compaction_action = HAS_EFFECT;
	      TARGET(n_data) = t1 + (t2-s1);
	      TGTSTR(n_data) = ts1;
	      SIZE(n_data) = z2;
	      SRCSTR(n_data) = ss2;
	      SOURCE(n_data) = s2;
	    }	
#if DB_EFF > 1
	  else
	    printf("A-K : none");
#endif
	  break;
	case ASSIGN_DEREF:
	  n_type = ASSIGN_DEREF;
	  if (BOUND(t2, s1, t2 + z2 - IPA_POINTER_SIZE))
	    {
#if DB_EFF > 1
	      printf("A-AD : new 1");
#endif
	      compaction_action = HAS_EFFECT;
	      TARGET(n_data) = t1;
	      TGTSTR(n_data) = ts1;
	      SIZE(n_data) = MIN(z1, z2-(s1-t2));
	      SRCSTR(n_data) = ss2;
	      SOURCE(n_data) = s2;
	      n_skew += (s1 - t2);
	    }
	  else if (BOUND(s1, t2, s1 + z1 - IPA_POINTER_SIZE))
	    {
#if DB_EFF > 1
	      printf("A-AD : new 2");
#endif
	      compaction_action = HAS_EFFECT;
	      TARGET(n_data) = t1 + (t2-s1);
	      TGTSTR(n_data) = ts1;
	      SIZE(n_data) = MIN(z2, z1 - (t2-s1));;
	      SRCSTR(n_data) = ss2;
	      SOURCE(n_data) = s2;
	    }
#if DB_EFF > 1
	  else
	    printf("A-AD : none");
#endif
	  break;
	default:
	  assert(0);
	}
      break;
    case SKEW:
      /***************** SKEW ********************/
      assert(issrc);
      n_src = edge->src_elist->node;
      n_dst = effect->dst;

      if (ss1 != 0 || ts2 != 0)
	{
	  /* Meshing stride, retain effect */
	  compaction_action = HAS_EFFECT;
	  retain_oldeffect = 1;
	  break;
	}

      switch(edge_type)
	{
	case ASSIGN:
	  n_type = SKEW;
	  if (BOUND(t2, s1, t2 + z2 - IPA_POINTER_SIZE))
	    {
#if DB_EFF > 1
	      printf("K-A : new 1");
#endif
	      compaction_action = HAS_EFFECT;
	      TARGET(n_data) = t1;
	      TGTSTR(n_data) = ts1;
	      SIZE(n_data) = z1;
	      SRCSTR(n_data) = ss2;
	      SOURCE(n_data) = s2 + (s1 - t2);
	    }
#if DB_EFF > 1
	  else
	    printf("K-A : none");
#endif
	  break;
	case SKEW:
	  n_type = SKEW;
	  if (s1 == t2)
	    {
#if DB_EFF > 1
	      printf("K-K : new 1");
#endif
	      compaction_action = HAS_EFFECT;
	      TARGET(n_data) = t1;
	      TGTSTR(n_data) = ts1;
	      SIZE(n_data) = (z1 + z2);
	      SRCSTR(n_data) = ss2;
	      SOURCE(n_data) = s2;
	    }
#if DB_EFF > 1
	  else
	    printf("K-K : none");
#endif
	  break;
	case ASSIGN_DEREF:
	  if (BOUND(t2, s1, t2 + z2 - IPA_POINTER_SIZE))
	    {
#if DB_EFF > 1
	      printf("K-AD : retain 1");
#endif
	      compaction_action = HAS_EFFECT;
	    }
#if DB_EFF > 1
	  else
	    printf("K-AD : none");
#endif
	  retain_oldeffect = 1;
	  break;
	default:
	  assert(0);
	}
      break;
    case ASSIGN_DEREF:
      /***************** ASSIGN_DEREF ********************/
      assert(issrc);
      n_src = edge->src_elist->node;
      n_dst = effect->dst;

      if (ss1 != 0 || ts2 != 0)
	{
	  /* Meshing stride, retain effect */
	  compaction_action = HAS_EFFECT;
	  retain_oldeffect = 1;
	  break;
	}      

      switch(edge_type)
	{
	case ASSIGN:
	  n_type = ASSIGN_DEREF;
	  if (BOUND(t2, s1, t2 + z2 - IPA_POINTER_SIZE))
	    {
#if DB_EFF > 1
	      printf("AD-A : new 1");
#endif
	      compaction_action = HAS_EFFECT;
	      TARGET(n_data) = t1;
	      TGTSTR(n_data) = ts1;
	      SIZE(n_data) = z1;
	      SRCSTR(n_data) = ss2;
	      SOURCE(n_data) = s2 + (s1-t2);
	    }
#if DB_EFF > 1
	  else
	    printf("AD-A : none");
#endif
	  break;
	case SKEW:
	  if (s1 == t2)
	    {
#if DB_EFF > 1
	      printf("AD-K : retain 1");
#endif
	      compaction_action = HAS_EFFECT;
	    }
#if DB_EFF > 1
	  else
	    printf("AD-K : none");
#endif
	  retain_oldeffect = 1;
	  break;
	case ASSIGN_DEREF:
	  if (BOUND(t2, s1, t2 + z2 - IPA_POINTER_SIZE))
	    {
#if DB_EFF > 1
	      printf("AD-AD : retain 1");
#endif
	      compaction_action = HAS_EFFECT;
	    }
#if DB_EFF > 1
	  else
	    printf("AD-AD : none");
#endif
	  retain_oldeffect = 1;
	  break;
	default:
	  assert(0);
	}
      break;
    case DEREF_ASSIGN:
      if (issrc)
	{
	  /***************** (source) DEREF_ASSIGN ********************/

	  if (ss1 != 0 || ts2 != 0)
	    {
	      /* Meshing stride, retain effect */
	      compaction_action = HAS_EFFECT;
	      retain_oldeffect = 1;
	      break;
	    }

	  switch(edge_type)
	    {
	    case ASSIGN:
	      n_type = DEREF_ASSIGN;
	      n_src = edge->src_elist->node;
	      n_dst = effect->dst;
	      if (BOUND(t2, s1, t2 + z2 - IPA_POINTER_SIZE))
		{
#if DB_EFF > 1
		  printf("DA-A : src new 1");
#endif
		  compaction_action = HAS_EFFECT;
		  TARGET(n_data) = t1;
		  TGTSTR(n_data) = ts1;
		  SIZE(n_data) = MIN(z1, z2-(s1-t2));
		  SRCSTR(n_data) = ss2;
		  SOURCE(n_data) = s2 + (s1-t2);
		}
	      else if (BOUND(s1, t2, s1 + z1 - IPA_POINTER_SIZE))
		{
#if DB_EFF > 1
		  printf("DA-A : src new 2");
#endif
		  compaction_action = HAS_EFFECT;
		  TARGET(n_data) = t1;
		  TGTSTR(n_data) = ts1;
		  SIZE(n_data) = MIN(z2, z1 - (t2-s1));
		  SRCSTR(n_data) = ss2;
		  SOURCE(n_data) = s2;
		  n_skew += (t2 - s1);
		}
#if DB_EFF > 1
	      else
		printf("DA-A : src none");
#endif
	      
	      break;
	    case SKEW:
	      if (BOUND(s1, t2, s1 + z1 - IPA_POINTER_SIZE))
		{
#if DB_EFF > 1
		  printf("DA-K : src retain 1");
#endif
		  compaction_action = HAS_EFFECT;
		}
#if DB_EFF > 1
	      else
		printf("DA-K : src none");
#endif
	      retain_oldeffect = 1;
	      break;
	    case ASSIGN_DEREF:
	      if (BOUND(t2, s1, t2 + z2 - IPA_POINTER_SIZE) ||
		  BOUND(s1, t2, s1 + z1 - IPA_POINTER_SIZE))
		{
#if DB_EFF > 1
		  printf("DA-K/AD : src retain 1");
#endif
		  compaction_action = HAS_EFFECT;
		}
#if DB_EFF > 1
	      else
		printf("DA-K/AD : src none");
#endif
	      retain_oldeffect = 1;
	      break;


	      /* FIXTEST */
	    case ASSIGN_ADDR:
	      if (s1 == t2)
		{
#if DB_EFF > 1
		  printf("AD-ADDR : retain 1");
#endif
		  compaction_action = HAS_EFFECT;
		}
#if DB_EFF > 1
	      else
		printf("AD-ADDR : none");
#endif
	      retain_oldeffect = 1;
	      break;

	    default:
	      assert(0);
	    }
	}
      else
	{
	  /***************** (dest) DEREF_ASSIGN ********************/

	  if (ts1 != 0 || ts2 != 0)
	    {
	      /* Meshing stride, retain effect */
	      compaction_action = HAS_EFFECT;
	      retain_oldeffect = 1;
	      break;
	    }

	  switch(edge_type)
	    {
	    case ASSIGN:
	      n_type = DEREF_ASSIGN;
	      n_src = effect->src;
	      n_dst = edge->src_elist->node;
	      if (BOUND(t2, t1, t2 + z2 - IPA_POINTER_SIZE))
		{
#if DB_EFF > 1
		  printf("DA-A : dst new 1");
#endif
		  compaction_action = HAS_EFFECT;
		  TARGET(n_data) = s2 + (t1-t2);
		  TGTSTR(n_data) = ss2;
		  SIZE(n_data) = z1;
		  SRCSTR(n_data) = ss1;
		  SOURCE(n_data) = s1;
		}
#if DB_EFF > 1
	      else
		printf("DA-A : dst none");
#endif
	      break;
	    case SKEW:
	      if (t1 == t2)
		{
#if DB_EFF > 1
		  printf("DA-K : dst retain 1");
#endif
		  compaction_action = HAS_EFFECT;
		}
#if DB_EFF > 1
	      else
		printf("DA-K : dst none");
#endif
	      retain_oldeffect = 1;
	      break;	      
	    case ASSIGN_DEREF:
	      if (BOUND(t2, t1, t2 + z2 - IPA_POINTER_SIZE))
		{
#if DB_EFF > 1
		  printf("DA-AD : dst retain 1");
#endif
		  compaction_action = HAS_EFFECT;
		}
#if DB_EFF > 1
	      else
		printf("DA-AD : dst none");
#endif
	      retain_oldeffect = 1;
	      break;
	    default:
	      assert(0);
	    }
	}
      break;
    default:
      assert(0);
    }

  if (SRCSTR(n_data) != 0)
    {
      SOURCE(n_data) = 0;
      SIZE(n_data) = IPA_POINTER_SIZE;
    }
  if (TGTSTR(n_data) != 0)
    {
      TARGET(n_data) = 0;
      SIZE(n_data) = IPA_POINTER_SIZE;
    }

  if (compaction_action == HAS_EFFECT)
    {
      if (retain_oldeffect) 
	{
#if DB_EFF > 1
	  printf(" : has effect, retain old effect\n");
#endif
	  if (update)
	    SE_addedgeto_effectlist(edge);
	  return 1;
	}
      else
	{
#if DB_EFF > 1
	  printf(" : has effect, compact\n");
#endif
	  if (update)
	    SE_addto_effectlist(n_type, n_src, n_dst, &n_data, n_skew);
	  return 0;
	}
    }

#if DB_EFF > 1
  printf(" : no effect\n");
#endif
  return 0;
}

#define ADD_EFFECT      0
#define DISCARD_EFFECT  1

static int
SE_reflect_effect(effect_t *effect, int issrc)
{
  IPA_cgraph_node_t      *node;
  IPA_cgraph_edge_t      *edge;
  IPA_cgraph_edge_list_t *elist;
  IPA_HTAB_ITER eiter;
  int must_retain;

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
      /* This node is already present in the summary 
	 so add the effect */
#if DB_EFF > 1
      printf("Add effect (PERM)\n");
#endif
      return ADD_EFFECT;
    }

  if (node->cgraph != cg)
    {
#if DB_EFF
      debug_effect("Skip Effect (CONS)", effect, "\n");
#endif
      return DISCARD_EFFECT;
    }

  /* All addr effects are from initial effects and
     should be kept */  
  if (effect->edge_type == ASSIGN_ADDR)
    {
#if DB_EFF > 1
      printf("Add effect (ADDR)\n");
#endif
      return ADD_EFFECT;
    }

  /* Check for edge retainment */
  must_retain = 0;
  for (elist = node->first_list; elist; elist = elist->nxt_list)
    {
      /* These types either generate nothing or 
	 always compact */
      if (elist->edge_type == ASSIGN_ADDR 
	  /* FIXTEST */
	  && !(issrc && effect->edge_type == DEREF_ASSIGN)
#if 0
	  || elist->edge_type == DEREF_ASSIGN 
	  || (issrc && elist->edge_type == ASSIGN)
#endif
	  )
	continue;
	  
      /* Check for any uncompactable cases */
      IPA_HTAB_START(eiter, elist->in);
      IPA_HTAB_LOOP(eiter)
	{
	  edge = IPA_HTAB_CUR(eiter);
	  if (IPA_FLAG_ISSET(edge->flags, (IPA_CG_EDGE_FLAGS_UD |
					   IPA_CG_EDGE_FLAGS_GBL)))
	    continue;
	  if (!IPA_FLAG_ISSET(edge->src_elist->node->flags,
			      (EA_LCONT_ESC)))
	    continue;
#if 1
	  if (edge->src_elist->node->misc.depth > node->misc.depth)
	    {
#if DB_EFF
	      IPA_cg_node_print(stdout,edge->src_elist->node, IPA_PRINT_ASCI);
	      printf("\n");
	      IPA_cg_node_print(stdout,node, IPA_PRINT_ASCI);
	      printf("\n");
	      printf("DEPTH FORCE RETAIN %d > %d\n",
		     edge->src_elist->node->misc.depth, node->misc.depth);
#endif
	      must_retain = 1;
	      goto END_RETAIN_CHECK;
	    }
#endif  
	  if (edge->src_elist->edge_type == SKEW && 
	      edge->src_elist->node == edge->dst_elist->node)
	    {
	      must_retain = 1;
	      goto END_RETAIN_CHECK;
	    }
	  if (compact_effect_edge(effect, edge, elist->edge_type,
				  issrc, 0, 0))
	    {
	      must_retain = 1;
	      goto END_RETAIN_CHECK;
	    }
	}
    }
 END_RETAIN_CHECK:

  for (elist = node->first_list; elist; elist = elist->nxt_list)
    {
      /* These types either generate nothing */
      if ((elist->edge_type == ASSIGN_ADDR 
	   /* FIXTEST */
	   && !(issrc && effect->edge_type == DEREF_ASSIGN)
	   ) || elist->edge_type == DEREF_ASSIGN)
	continue;
	  
      /* Perform compaction */
      IPA_HTAB_START(eiter, elist->in);
      IPA_HTAB_LOOP(eiter)
	{
	  edge = IPA_HTAB_CUR(eiter);
	  if (IPA_FLAG_ISSET(edge->flags, (IPA_CG_EDGE_FLAGS_UD |
					   IPA_CG_EDGE_FLAGS_GBL)))
	    continue;
	  if (!IPA_FLAG_ISSET(edge->src_elist->node->flags,
			      (EA_LCONT_ESC)))
	    continue;
	  
	  compact_effect_edge(effect, edge, elist->edge_type,
			      issrc, must_retain, 1);
	}
    }
  
  if (must_retain)
    return ADD_EFFECT;

  return DISCARD_EFFECT;
}


/***********************************************************************
 * Process all effects
 ***********************************************************************/

static void
SE_process_effect(effect_t *effect)
{
  switch(effect->edge_type)
    {
    case DEREF_ASSIGN:
      if (SE_reflect_effect(effect, 1) == ADD_EFFECT)
	{
#if 1
	  if (effect->skew != 0)
	    {
	      IPA_cgraph_edge_data_t data;
	      IPA_cgraph_node_t *tmp_node;
	      
	      tmp_node = new_temp_node(local_sum, IPA_CG_NODE_FLAGS_SUMMARY);

	      /* SUMMARY:  tmp(0) *=(z) src(s) */
	      TARGET(data) = 0;
	      TGTSTR(data) = 0;
	      SIZE(data) = SIZE(effect->edata);
	      SRCSTR(data) = SRCSTR(effect->edata);
	      SOURCE(data) = SOURCE(effect->edata);
	      SE_addto_summary(effect->edge_type, 
			       effect->src, tmp_node, &data);
	      
	      /* EFFECT :  tmp(0) =+(k) dst(t) */
	      TARGET(data) = 0;
	      TGTSTR(data) = 0;
	      SIZE(data) = effect->skew;
	      SRCSTR(data) = TGTSTR(effect->edata);
	      SOURCE(data) = TARGET(effect->edata);
	      SE_addto_effectlist(SKEW, effect->dst, tmp_node, &data, 0);
	      break;
	    }
#endif
	  if (SE_reflect_effect(effect, 0) == ADD_EFFECT)
	    {
	      /* Effect not skipped */
	      SE_addto_summary(effect->edge_type, effect->src,
			       effect->dst, &effect->edata);
	    }
	}
      break;

    case ASSIGN_DEREF:
#if 1
      if (effect->skew != 0)
	{
	  IPA_cgraph_edge_data_t data;
	  IPA_cgraph_node_t *tmp_node;

	  tmp_node = new_temp_node(local_sum, IPA_CG_NODE_FLAGS_SUMMARY);
	  
	  /* SUMMARY:  dst(t) =*(z) tmp(0) */
	  TARGET(data) = TARGET(effect->edata);
	  TGTSTR(data) = TGTSTR(effect->edata);
	  SIZE(data) = SIZE(effect->edata);
	  SRCSTR(data) = 0;
	  SOURCE(data) = 0;
	  SE_addto_summary(effect->edge_type, 
			   tmp_node, effect->dst, &data);
	  
	  /* EFFECT :  tmp(0) =+(k) src(s) */
	  TARGET(data) = 0;
	  TGTSTR(data) = 0;
	  SIZE(data) = effect->skew;
	  SRCSTR(data) = SRCSTR(effect->edata);
	  SOURCE(data) = SOURCE(effect->edata);
	  SE_addto_effectlist(SKEW, effect->src, tmp_node, &data, 0);
	  break;
	}
#endif
    case ASSIGN:
    case SKEW:
      if (SE_reflect_effect(effect, 1) == ADD_EFFECT)
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


/***********************************************************************
 *
 ***********************************************************************/

static void
create_compact_summary()
{
  effect_t *effect;
  int cnt = 0;

  local_sum = IPA_cg_cgraph_new(cg->data.fninfo);
  
  effect_list = NULL;
  SE_initial_effects();

  DEBUG_IPA(2, printf("\nInitial Effects: %d (%d cg nodes)\n",
		      List_size(effect_list),
		      IPA_htab_size(cg->nodes)););

  List_start(effect_list);
  while ((effect = List_next(effect_list)))
    {
      effect_list = List_delete_current(effect_list);
      SE_process_effect(effect);
      cnt++;
    }

  SE_free_effects();
  List_reset(effect_list);

  DEBUG_IPA(2, printf("Effects: %d\n",cnt););
}


/***********************************************************************
 * Master summarization function
 ***********************************************************************/

void
IPA_consg_fdvs_summarize (IPA_prog_info_t * __info, 
			  IPA_cgraph_t * __cg,
			  IPA_interface_t *__iface,
			  IPA_funcsymbol_info_t *__fninfo,
			  IPA_cgraph_t **local_sum_ptr, 
			  char *func_name)
{
  List setlist;
  IPA_cgraph_node_t *node;

  cg   = __cg;
  formal_iface = __iface;
  prog_info = __info;
  fn_info = __fninfo;

  IPA_init_escape_analysis(__info, cg, formal_iface, __fninfo);
  setlist = IPA_do_escape_analysis();
  
  /* GENERIC flags are supposed to be cleared after use.
     Double check that things appear ok.
  */  
  IPA_cg_nodes_assert_clr_flags(cg, (EA_SURVIVES | EA_PERMANENT));

  create_compact_summary();

  DEBUG_IPA(2, printf("OPTI: Initial of [%s] : (Base)  %d nodes\n",
		      func_name, 
		      IPA_htab_size(cg->nodes));
	       printf("OPTI: Summary of [%s] : (Comp)  %d nodes\n",
		      func_name, 
		      IPA_htab_size(local_sum->nodes)););

  fn_info->summary_size = IPA_htab_size(local_sum->nodes);

#if 1
  IPA_cycle_detection(local_sum, 			
		      (IPA_CG_NODE_FLAGS_GLOBAL |
		       IPA_CG_NODE_FLAGS_NOCNTXT |
		       IPA_CG_NODE_FLAGS_CALLEE),
		      CD_DELETE_ALL);
  DEBUG_IPA(2, printf("OPTI: Summary of [%s] : (Cycle) %d nodes\n",
		      func_name, 
		      IPA_htab_size(local_sum->nodes)););
#endif
#if 1
  IPA_consg_fdvs_remove_redundancy(prog_info, fn_info, local_sum);
  DEBUG_IPA(2, printf("OPTI: Summary of [%s] : (Red)   %d nodes\n",
		      func_name, 
		      IPA_htab_size(local_sum->nodes)););
#endif

#if 0
  if (IPA_htab_size(local_sum->nodes) > 100)
    {
      IPA_consg_fi_remove_redundancy(local_sum);
      printf("OPTI: Summary of [%s] : (RedFI) %d nodes\n",
	     func_name, 
	     IPA_htab_size(local_sum->nodes));
    }
#endif
  
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
				 EA_PERMANENT |
				 EA_LCONT_ESCLCL));
#if 0
      printf("CLRSET:");
      IPA_cg_node_print(stdout,node,IPA_PRINT_ASCI);
      printf(" %X \n", node->flags);
#endif
    }
  List_reset(setlist);

  IPA_cg_nodes_assert_clr_flags(cg, (EA_INPROGRESS | EA_LCONT_ESC |
				     EA_LPROP_ESC | EA_RETPROP_ESC |
				     EA_LFULL_ESC | EA_LCONT_ESCLCL)
				);
}
