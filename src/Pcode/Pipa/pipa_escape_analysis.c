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
 *      File:    pipa_escape_analysis.c
 *      Author:  Erik Nystrom
 *      Copyright (c) 2003  Erik Nystrom, Wen-mei Hwu
 *              and The Board of Trustees of the University of Illinois.
 *              All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#include "pipa_escape_analysis.h"

#define DB_ESC 0

static List node_worklist = NULL;
static IPA_cgraph_t *cg = NULL;
static IPA_interface_t *formal_iface = NULL;
static IPA_prog_info_t * info = NULL;
static IPA_funcsymbol_info_t * param_fninfo = NULL;

#define add_to_worklist(node)    ((node_worklist = List_insert_last(node_worklist, (node))),\
                                 IPA_FLAG_SET((node)->flags, EA_INPROGRESS))

#define rem_from_worklist(node)  ((node_worklist = List_delete_current(node_worklist)),\
                                 IPA_FLAG_CLR((node)->flags, EA_INPROGRESS))

static int observed = 0;

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

static int global_escape = 0;
static int local_escape = 0;
static int t_global_escape = 0;
static int t_local_escape = 0;

static List setlist = NULL;
static void
add_setlist(IPA_cgraph_node_t *node)
{
  if (!IPA_FLAG_ISSET(node->flags, EA_FLAGSET))
    {
#if 0
      printf("SET:");
      IPA_cg_node_print(stdout,node,IPA_PRINT_ASCI);
      printf("\n");
#endif
      setlist = List_insert_last(setlist, node);
      IPA_FLAG_SET(node->flags, EA_FLAGSET);
    }
}


int
IPA_does_observe_node (IPA_cgraph_node_t *node)
{
  IPA_cgraph_edge_list_t *elist;
  IPA_cgraph_edge_t *edge;
  IPA_HTAB_ITER eiter;
#if !PARTIALSUM
  return 1;
#endif

  elist = IPA_cg_edge_list_find(node, ASSIGN_ADDR);
  if (!elist)
    return 0;

  IPA_HTAB_START(eiter, elist->in);
  IPA_HTAB_LOOP(eiter)
    {
      edge = (IPA_cgraph_edge_t *)IPA_HTAB_CUR(eiter);
      
      if (IPA_FLAG_ISSET(edge->flags,IPA_CG_EDGE_FLAGS_DN))
	{
	  observed++;
	  return 1;
	}  
    }

  return 0;
}

/*****************************************************************************
 * Escape
 *****************************************************************************/

static void
new_CE_node(IPA_cgraph_node_t *node, int flags)
{
  if (node->cgraph != cg)
    {
#if 0
      printf("OUTSIDE CE %p %p ",node->cgraph,info->globals);
      IPA_cg_node_print(stdout, node, IPA_PRINT_ASCI);
      printf("\n");
#endif
      return;
    }

  if (!IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_PARAM) &&
      !IPA_does_observe_node(node))
    return;

  if (IPA_FLAG_ISSET(flags, EA_LCONT_ESC))
    {
      /* Globals are never marked for local summarization */
      if (IPA_FLAG_ISSET(node->flags, EA_LCONT_ESC) ||
	  IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_GLOBAL) ||
	  IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_NOCNTXT))
	return;
      IPA_FLAG_SET(node->flags, EA_LCONT_ESC);
      add_to_worklist(node);
#if DB_ESC
      printf("LCE: ");
      IPA_cg_node_print(stdout, node, IPA_PRINT_ASCI);
      printf("\n");
#endif
    }
}

static void
new_PE_node(IPA_cgraph_node_t *node, int flags)
{
  if (node->cgraph != cg)
    {
#if 0
      printf("OUTSIDE PE %p %p",node->cgraph,info->globals);
      IPA_cg_node_print(stdout, node, IPA_PRINT_ASCI);
      printf("\n");
#endif
      return;
    }

  if (IPA_FLAG_ISSET(flags, EA_LPROP_ESC))
    {
      /* Globals are never marked for local summarization */
      if (IPA_FLAG_ISSET(node->flags, EA_LPROP_ESC) ||
	  IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_GLOBAL ||
	  IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_NOCNTXT)))
	return;
      IPA_FLAG_SET(node->flags, EA_LPROP_ESC);
      add_to_worklist(node);
#if DB_ESC
      printf("LPE: ");
      IPA_cg_node_print(stdout, node, IPA_PRINT_ASCI);
      printf("\n");
#endif
    }

  if (IPA_FLAG_ISSET(flags, EA_RETPROP_ESC))
    {
      if (IPA_FLAG_ISSET(node->flags, EA_RETPROP_ESC))
	return;
      IPA_FLAG_SET(node->flags, EA_RETPROP_ESC);
      add_to_worklist(node);
#if DB_ESC
      printf("RPE: ");
      IPA_cg_node_print(stdout, node, IPA_PRINT_ASCI);
      printf("\n");
#endif
    }
}

static void
new_FE_node(IPA_cgraph_node_t *node, int flags)
{
  if (node->cgraph != cg)
    {
#if 0
      printf("OUTSIDE FE ");
      IPA_cg_node_print(stdout, node, IPA_PRINT_ASCI);
      printf("\n");
#endif
      return;
    }

  if (IPA_FLAG_ISSET(flags, (EA_LFULL_ESC | EA_LPROP_ESC)))
    {
      /* Globals are never marked for local summarization */
      if (IPA_FLAG_ISSET(node->flags, EA_LFULL_ESC) ||
	  IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_GLOBAL) ||
	  IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_NOCNTXT))
	return;
      
      /* If node is original to a function, it can only
	 escape through its own parameters - technically
	 a summary node is owned by the owner of its callsite
	 that incorporated it ... however there is no access to
	 this info at the moment */
      if (IPA_FLAG_ISCLR(node->flags, IPA_CG_NODE_FLAGS_SUMMARY))
	{
	  IPA_symbol_info_t *sym;
	  sym = IPA_symbol_find_by_id(info, node->data.var_id);
	  if (sym->fninfo != param_fninfo)
	    {
	      /*debug_print("\nDOES NOT REALLY esclocal: ",node,NULL,"\n");*/
	      return;
	    }
	}

      IPA_FLAG_SET(node->flags, (EA_LFULL_ESC | EA_LCONT_ESC | 
				 EA_LPROP_ESC));
      add_to_worklist(node);

      assert(IPA_FLAG_ISCLR(node->flags, (IPA_CG_NODE_FLAGS_GLOBAL|
					  IPA_CG_NODE_FLAGS_NOCNTXT)));
#if DB_ESC
      debug_print("\nesclocal: ",node,NULL,"");
#endif
      local_escape++;
      IPA_FLAG_SET(node->flags, IPA_CG_NODE_FLAGS_ESCLOCAL);
      
#if DB_ESC
      printf("LFE: ");
      IPA_cg_node_print(stdout, node, IPA_PRINT_ASCI);
      printf("\n");
#endif
    }


  /* Only addresses that are allowed to escape through returns
   *   can become fully escaping at a RETPROP node 
   */
  if (IPA_FLAG_ISSET(flags, (EA_RETPROP_ESC)))
    {
      if (IPA_FLAG_ISCLR(node->flags, IPA_CG_NODE_FLAGS_NOLOCAL) ||
	  IPA_FLAG_ISSET(node->flags, (IPA_CG_NODE_FLAGS_GLOBAL|
				       IPA_CG_NODE_FLAGS_NOCNTXT)))
	return;
      if (IPA_FLAG_ISSET(node->flags, EA_LFULL_ESC))
	return;

      IPA_FLAG_SET(node->flags, (EA_LFULL_ESC | EA_LCONT_ESC | 
				 EA_LPROP_ESC));
      add_to_worklist(node);

      assert(IPA_FLAG_ISCLR(node->flags, (IPA_CG_NODE_FLAGS_GLOBAL|
					  IPA_CG_NODE_FLAGS_NOCNTXT)));
#if DB_ESC
      debug_print("\nescheap: ",node,NULL,"");
#endif
      IPA_FLAG_SET(node->flags, IPA_CG_NODE_FLAGS_ESCLOCAL);

#if DB_ESC
      printf("LFE (RETURN): ");
      IPA_cg_node_print(stdout, node, IPA_PRINT_ASCI);
      printf("\n");
#endif
    }
}

static void
escape_init()
{
  IPA_cgraph_node_t    *node;
  int formal_id, formal_bound;
  int i;

  /* param(u)  return(u)  global(u)
   * --------  ---------  ---------
   *   CE(u)      PE(u)     FE(u)
   */

#if DB_ESC
  printf("escape-init\n");
#endif
  observed = 0;

  /* Params */
  formal_bound = IPA_interface_get_num_params (formal_iface);
  for (i = 0; i < formal_bound; i++)
    {
      formal_id = IPA_interface_get_param_id (formal_iface, i);
      node = IPA_consg_find_node (cg, formal_id, 1);

      if (param_fninfo->is_heap_alloc || 
	  param_fninfo->is_heap_free ||
	  param_fninfo->is_noexit)
	add_setlist(node);
      else
	new_CE_node(node, EA_LCONT_ESC);
    }

  /* Return */
  formal_id = IPA_interface_get_ret_id (formal_iface);
  if (formal_id)
    {
      node = IPA_consg_find_node (cg, formal_id, 1);
      if (node)
	{
	  if (param_fninfo->is_heap_free || 
	      param_fninfo->is_noexit)
	    add_setlist(node);
	  else
	    new_PE_node(node, EA_RETPROP_ESC);
	}
    }

#if DB_ESC
  printf("escape-init-end\n");
#endif
}


static void
escape_PE_node(IPA_cgraph_node_t *node)
{
  IPA_cgraph_edge_t *edge;
  IPA_cgraph_edge_list_t *elist;
  IPA_HTAB_ITER eiter;

  /* New PE node effects:
   *    - IN : assign, assign_addr
   *    - OUT: 
   */

  for (elist = node->first_list; elist; elist = elist->nxt_list)
    {
      /* u = v , FE(u) or PE(u)        u = v , PE(u)
       * ----------------------  ===>  -------------  UP
       *         PE(v)                      PE(v) 
       */
      if (elist->edge_type == ASSIGN || elist->edge_type == SKEW)
	{
	  IPA_HTAB_START(eiter, elist->in);
	  IPA_HTAB_LOOP(eiter)
	    {
	      edge = IPA_HTAB_CUR(eiter);
	      if (IPA_FLAG_ISSET(edge->flags, IPA_CG_EDGE_FLAGS_GBL) ||
		  IPA_FLAG_ISSET(edge->flags, IPA_CG_EDGE_FLAGS_UD))
		continue;
#if DB_ESC
	      printf("R1: ");
#endif
	      new_PE_node(edge->src_elist->node, node->flags);
	    }
	}

      /* u = &v, FE(u) or PE(u)         u = &v , PE(u)
       * ----------------------  ===>   --------------  UP
       *       FE(v)                         FE(v)
       */
      if (elist->edge_type == ASSIGN_ADDR)
	{
	  IPA_HTAB_START(eiter, elist->in);
	  IPA_HTAB_LOOP(eiter)
	    {
	      edge = IPA_HTAB_CUR(eiter);
	      if (IPA_FLAG_ISSET(edge->flags, IPA_CG_EDGE_FLAGS_GBL) ||
		  IPA_FLAG_ISSET(edge->flags, IPA_CG_EDGE_FLAGS_UD))
		continue;
#if DB_ESC
	      printf("R2: ");
#endif
	      new_FE_node(edge->src_elist->node, node->flags);
	    }
	}
    }
}


static void
escape_CE_node(IPA_cgraph_node_t *node)
{
  IPA_cgraph_edge_t *edge;
  IPA_cgraph_edge_list_t *elist;
  IPA_HTAB_ITER eiter;


  /* New CE node effects:
   *    - IN : deref_assign 
   *    - OUT: assign, assign_deref
   */

  for (elist = node->first_list; elist; elist = elist->nxt_list)
    {
      /* u = v , FE(v) or CE(v)        u = v , CE(v)
       * ----------------------  ===>  -------------  DOWN
       *          CE(u)                     CE(u)
       */
      if (elist->edge_type == ASSIGN || elist->edge_type == SKEW)
	{
	  IPA_HTAB_START(eiter, elist->out);
	  IPA_HTAB_LOOP(eiter)
	    {
	      edge = IPA_HTAB_CUR(eiter);
	      if (IPA_FLAG_ISSET(edge->flags, IPA_CG_EDGE_FLAGS_GBL) ||
		  IPA_FLAG_ISSET(edge->flags, IPA_CG_EDGE_FLAGS_UD))
		continue;
#if DB_ESC
	      printf("R3: ");
#endif
	      new_CE_node(edge->dst_elist->node, node->flags);
	    }
	}
  
      /* u = *v, FE(v) or CE(v)        u = *v , CE(v) 
       * ----------------------  ===>  --------------  DOWN
       *         CE(u)                      CE(u)
       */
      if (elist->edge_type == ASSIGN_DEREF)
	{
	  IPA_HTAB_START(eiter, elist->out);
	  IPA_HTAB_LOOP(eiter)
	    {
	      edge = IPA_HTAB_CUR(eiter);
	      if (IPA_FLAG_ISSET(edge->flags, IPA_CG_EDGE_FLAGS_GBL) ||
		  IPA_FLAG_ISSET(edge->flags, IPA_CG_EDGE_FLAGS_UD))
		continue;
#if DB_ESC
	      printf("R4: ");
#endif
	      new_CE_node(edge->dst_elist->node, node->flags);
	    }
	}

      /* *u = v, FE(u) or CE(u)        *u = v , CE(u) 
       * ----------------------  ===>  --------------  UP
       *          PE(v)                     PE(v)
       */
      if (elist->edge_type == DEREF_ASSIGN)
	{
	  IPA_HTAB_START(eiter, elist->in);
	  IPA_HTAB_LOOP(eiter)
	    {
	      edge = IPA_HTAB_CUR(eiter);
	      if (IPA_FLAG_ISSET(edge->flags, IPA_CG_EDGE_FLAGS_GBL) ||
		  IPA_FLAG_ISSET(edge->flags, IPA_CG_EDGE_FLAGS_UD))
		continue;
#if DB_ESC
	      printf("R5: ");
#endif
	      assert(IPA_FLAG_ISSET(node->flags, EA_LCONT_ESC));
	      new_PE_node(edge->src_elist->node, EA_LPROP_ESC);
	    }
	}
    }
}


static void
escape_FE_node(IPA_cgraph_node_t *node)
{
  IPA_cgraph_edge_t *edge;
  IPA_cgraph_edge_list_t *elist;
  IPA_HTAB_ITER eiter;

  /* New FE node effects:
   *    - IN : assign, assign_addr, deref_assign
   *    - OUT: assign, (assign_addr), assign_deref
   */

  for (elist = node->first_list; elist; elist = elist->nxt_list)
    {
      /* u = &v , FE(v)                 u = &v , FE(v)
       * --------------          ===>   --------------  DOWN
       *     CE(u)                           CE(u)
       */
      if (elist->edge_type == ASSIGN_ADDR)
	{
	  IPA_HTAB_START(eiter, elist->out);
	  IPA_HTAB_LOOP(eiter)
	    {
	      edge = IPA_HTAB_CUR(eiter);
	      if (IPA_FLAG_ISSET(edge->flags, IPA_CG_EDGE_FLAGS_GBL) ||
		  IPA_FLAG_ISSET(edge->flags, IPA_CG_EDGE_FLAGS_UD))
		continue;
#if 0
#if DB_ESC
	      printf("R6: ");
#endif
	      new_CE_node(edge->dst_elist->node, node->flags);
#endif
	      if ((edge->dst_elist->node->cgraph == cg) &&
		  !IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_GLOBAL) &&
		  !IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_NOCNTXT))
		{
		  IPA_FLAG_SET(edge->dst_elist->node->flags, EA_LCONT_ESCLCL);
		  /* To get node into set list */
		  add_setlist(edge->dst_elist->node);
		}
	    }
	}
    }
}



/*****************************************************************************
 * Interface
 *****************************************************************************/

void
IPA_init_escape_analysis(IPA_prog_info_t * __info, 
			 IPA_cgraph_t * __cg,
			 IPA_interface_t *__iface,
			 IPA_funcsymbol_info_t *__param_fninfo)
{
  info = __info;
  cg   = __cg;
  formal_iface = __iface;
  param_fninfo = __param_fninfo;

  assert(formal_iface && param_fninfo);

  /* GENERIC flags are supposed to be cleared after use.
     Double check that things appear ok.
   */
  IPA_cg_nodes_assert_clr_flags(cg, (EA_INPROGRESS | EA_LCONT_ESC |
				     EA_LPROP_ESC | EA_RETPROP_ESC |
				     EA_LFULL_ESC | EA_LCONT_ESCLCL)
				);

  /* We'll figure this out again */
  IPA_cg_nodes_clr_flags(cg, IPA_CG_NODE_FLAGS_ESCLOCAL);

  node_worklist = NULL;
  setlist = NULL;
  escape_init();
}

void
IPA_new_escape_CE_node(IPA_cgraph_node_t *node, int flags)
{
  new_CE_node(node, flags);
}

void
IPA_new_escape_PE_node(IPA_cgraph_node_t *node, int flags)
{
  new_PE_node(node, flags);
}

void
IPA_new_escape_FE_node(IPA_cgraph_node_t *node, int flags)
{
  new_FE_node(node, flags);
}

List
IPA_do_escape_analysis()
{
  int cnt = 0;
  IPA_cgraph_node_t *node;

  global_escape = 0;
  local_escape = 0;

  List_start(node_worklist);
  while ((node = List_next(node_worklist)))
    {
      rem_from_worklist(node);

      /* Build a list of all nodes touched
       */
      add_setlist(node);

#if DB_ESC
      printf("NODE:");
      IPA_cg_node_print(stdout,node,IPA_PRINT_ASCI);
      printf("\n");
#endif

      if (IPA_FLAG_ISSET(node->flags, (EA_LPROP_ESC | EA_RETPROP_ESC)))
	escape_PE_node(node);
      if (IPA_FLAG_ISSET(node->flags, (EA_LCONT_ESC)))
	escape_CE_node(node);
      if (IPA_FLAG_ISSET(node->flags, (EA_LFULL_ESC)))
	escape_FE_node(node);
      cnt++;
    }
  List_reset(node_worklist);
  node_worklist = NULL;
  
#if 0
  printf("ESC Processed: %d\n",cnt);
#endif
  t_global_escape += global_escape;
  t_local_escape += local_escape;
#if 0
  printf("ESC local escape : %d %d\n",t_local_escape, local_escape);
  printf("ESC global escape: %d %d\n",t_global_escape, global_escape);
#endif

  param_fninfo->observed = observed;
/*   printf("OBSERVED %d\n",observed); */
  return setlist;
}



