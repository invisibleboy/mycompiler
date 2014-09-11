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
 *      File:    pipa_driver.c
 *      Author:  Erik Nystrom
 *      Copyright (c) 2003  Erik Nystrom, Wen-mei Hwu
 *              and The Board of Trustees of the University of Illinois.
 *              All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/


#include "pipa_driver.h"
#include "pipa_driver_utils.h"
#include "pipa_misc_utils.h"

void
IPA_driver_solve_graph (IPA_prog_info_t * info, IPA_cgraph_t * consg, List edge_delta,
			int field_option);



/*************************************************************************
 *
 * SUPPORT ROUTINES
 *
 *************************************************************************/

static List
IPA_listof_newaccess(IPA_prog_info_t *info)
{
  IPA_funcsymbol_info_t *fninfo;
  List delta = NULL;

  List_start(info->fninfos);
  while ((fninfo = List_next(info->fninfos)))
    {
      if (fninfo->has_been_called != 1)
	continue;
      fninfo->has_been_called = 2;
      if (!fninfo->consg)
	continue;
      assert(fninfo->call_node);
      delta = IPA_consg_build_listof_edges(delta, fninfo->consg);
    }

  return delta;
}


static int
IPA_iscallee_nodeset(IPA_cgraph_node_t *node)
{
  node = IPA_cg_node_get_rep(node);
  for (; node; node = node->rep_child)
    {
      if (IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_CALLEE))
	return 1;
    }
  return 0;
}

static int
IPA_isfunc_nodeset(IPA_cgraph_node_t *node)
{
  node = IPA_cg_node_get_rep(node);
  for (; node; node = node->rep_child)
    {
      if (IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_FUNC))
	return 1;
    }
  return 0;
}

List
IPA_get_callee_delta(IPA_prog_info_t *info)
{
  IPA_funcsymbol_info_t *fninfo;
  List tmp_list = NULL;
  
  List_start(info->fninfos);
  while ((fninfo = List_next(info->fninfos)))
    {
      IPA_HTAB_ITER niter;
      IPA_cgraph_t * cur_consg;

      if (!fninfo->has_been_called)
	continue;
      if (!fninfo->consg)
	continue;
      cur_consg = fninfo->consg;

      IPA_HTAB_START(niter, cur_consg->nodes);
      IPA_HTAB_LOOP(niter)
	{
	  IPA_cgraph_node_t *node;
	  IPA_HTAB_ITER eiter;
	  IPA_cgraph_edge_list_t *elist;
	  
	  node = (IPA_cgraph_node_t *)IPA_HTAB_CUR(niter);
	  if (!IPA_iscallee_nodeset(node))
	    continue;
	  elist = IPA_cg_edge_list_find(node, ASSIGN_ADDR);
	  if (!elist)
	    continue;
	  
	  IPA_HTAB_START(eiter, elist->in);
	  IPA_HTAB_LOOP(eiter)
	    {
	      IPA_cgraph_edge_t *edge;
	      
	      edge = (IPA_cgraph_edge_t *)IPA_HTAB_CUR(eiter);
	      /* CIR 20060515
	       *  This flag can prevent detection of some indirect function calls.
	       *  The scan to discover new indirect calls occurs several times
	       *   throughout the analysis.  The flag is set for each edge the first
	       *   time it is scanned.  However, some indirect calls are discovered
	       *   after the first scan. To prevent such an error, the flag check
	       *   is disabled.
	       */
#if 0
	      if (IPA_FLAG_ISSET(edge->flags, IPA_CG_EDGE_FLAGS_CALLG_PROCESSED))
		continue;
#endif
	      IPA_FLAG_SET(edge->flags, IPA_CG_EDGE_FLAGS_CALLG_PROCESSED);

	      if (!IPA_isfunc_nodeset(edge->src_elist->node))
		continue;

	      tmp_list = List_insert_last (tmp_list, edge);
	    }
	}
    }

  if (List_size(tmp_list) > 0)
    printf("NEW CALLG ELIST %d\n",
	   List_size(tmp_list));

  return tmp_list;
}


/*************************************************************************
 *
 * BUILD INITIAL CALLGRAPH
 *
 *************************************************************************/

int
IPA_Interproc_Start (IPA_prog_info_t * info)
{
  List new_edge_list = NULL;
  int count = 0;

  assert(info);

  /* Build the initial callgraph
   */
  info->call_graph = IPA_callg_new ();
  info->globals->call_node = 
	IPA_callg_node_add (info->call_graph, info->globals);
  info->globals->has_been_called = 1;

  if (List_size (info->globals->callsites) == 0)
    {
      IPA_funcsymbol_info_t *fninfo = NULL;
      IPA_callsite_t *root_cs = NULL;
      Key dummy = {-1,-1};
      List del_list = NULL;
      IPA_callg_edge_t *edge;
      IPA_callg_node_t *node;

      fprintf(info->errfile, "main() NOT FOUND, DERIVING CALLGRAPH\n");
      printf("main() NOT FOUND, DERIVING CALLGRAPH\n");
      root_cs = IPA_callsite_new_prog (info, info->globals,
				       NULL, dummy);
      
      List_start(info->fninfos);
      while ((fninfo = List_next(info->fninfos)))
	{
#if 0
	  IPA_cgraph_node_t *cnode;
	  IPA_symbol_info_t *sym;
	  sym = IPA_symbol_find(info, fninfo->func_key);
	  cnode = IPA_consg_find_node(info->globals->consg, sym->id, 1);
	  if (IPA_cg_edge_list_find(cnode, ASSIGN_ADDR))
	    {
	      printf("ADDR");
	      continue;
	    }
#endif

	  if (fninfo == info->globals ||
	      fninfo->has_been_called)
	    continue;
	  new_edge_list = 
	    IPA_callgraph_build_direct (info, info->globals->call_node,
					root_cs, 
					fninfo->func_name,
					fninfo->func_key,
					1, new_edge_list, 1);
	}

      List_start(info->call_graph->nodes);
      while ((node = List_next(info->call_graph->nodes)))
	{
	  if (List_size(node->caller_edges) == 1)
	    continue;
	  List_start(node->caller_edges);
	  while ((edge = List_next(node->caller_edges)))
	    {
	      if (edge->caller->fninfo == info->globals)
		del_list = List_insert_last(del_list, edge);
	    }
	}
      
      printf("PRUNING %d edges as non-root\n",List_size(del_list));
      List_start(del_list);
      while ((edge = List_next(del_list)))
	{
	  IPA_callg_edge_delete(edge);
	}
      
/*       printf("####################\n"); */
/*       printf("DERIVED TOPLEVEL\n"); */
/*       List_start(info->globals->call_node->callee_edges); */
/*       while ((edge = List_next(info->globals->call_node->callee_edges))) */
/* 	{ */
/* 	  printf("%s \n", edge->callee->fninfo->func_name); */
/* 	} */
/*       printf("####################\n"); */

      IPA_connect_inputs(info);
      count = List_size(new_edge_list);
      List_reset(new_edge_list);
    }
  else
    {
      IPA_callsite_t *cs;
      int indir;

/*       printf("main() FOUND\n"); */

      indir = (List_size (info->globals->callsites) > 1);

      List_start (info->globals->callsites);
      while ((cs = (IPA_callsite_t *) List_next (info->globals->callsites)))
	{      
	  new_edge_list = 
	    IPA_callgraph_build_direct (info, info->globals->call_node,
					cs, 
					cs->callee.dir.name, 
					cs->callee.dir.key,
					indir, NULL, 1);
	  count += List_size (new_edge_list);
	  new_edge_list = List_reset (new_edge_list);
	}
    }

  IPA_callg_DVprint (info->call_graph, "CALLGRAPH_PRE");
/*   printf("Max Type Size = %d\n", IPA_max_type_size); */
  assert(IPA_max_type_size >= IPA_POINTER_SIZE);

  return count;
}




/*************************************************************************
 *
 * CONTEXT INSENSITIVE ANALYSIS CORE LOOP
 *
 *************************************************************************/

void
IPA_Context_Insensitive (IPA_prog_info_t *info)
{
  IPA_callg_node_t *prog_node;
  IPA_callg_node_t *callg_node;
  List callee_delta = NULL;
  List edge_delta = NULL;
  List callg_update_list = NULL;
  int round = 0;
  int changes = 0;
  double stime, citime = 0.0;

  prog_node = info->globals->call_node;
  
  /* Solve until convergence 
   */
  IPA_TrackTime(0);
  do
    {
      round++;
      debug.round = round;
/*       printf ("###### ROUND %d ##################################\n", round); */
      
      changes = 0;
      callee_delta = NULL;
      edge_delta = NULL;
      callg_update_list = NULL;

      stime = IPA_GetTime ();

      /* Merge all call nodes into one super-node */
      List_start(info->call_graph->nodes);
      while ((callg_node = List_next(info->call_graph->nodes)))
	{
          if (callg_node == prog_node)
	    continue;
	  if (callg_node->rep_parent != callg_node)
	    continue;

#if HS_CI
          if ((IPA_cloning_option == IPA_HEAP_CLONING))
            {
              if (strcmp(callg_node->fninfo->func_name, "malloc") == 0 ||
                  strcmp(callg_node->fninfo->func_name, "calloc") == 0)
                continue;
            }
#endif

	  IPA_callgraph_node_merge (prog_node, callg_node);
        }

      IPA_callgraph_connect_all  (info);

#if 0
      IPA_cg_DOTprint (prog_node->fninfo->consg, "MERGED",
                      IPA_CG_ETYPE_ALL);
#endif
      
      /* Calculate initial edge delta list */
      edge_delta = IPA_consg_build_listof_new_edges(NULL, prog_node->fninfo);
      DEBUG_IPA(1, printf("EDGE SEED %d\n",List_size(edge_delta)););

      /* Solve the graph */
      IPA_driver_solve_graph (info, prog_node->fninfo->consg,
			      edge_delta, IPA_field_option);
      callee_delta = IPA_get_callee_delta(info);

      /* Determine new callees */
      callg_update_list = 
	IPA_callgraph_new_callees (info, callg_update_list,
				   callee_delta, round);
      List_reset (callee_delta);
      callee_delta = NULL;

      /* Update the callgraph */
      changes = IPA_callg_update_callg(info, callg_update_list,
				       round);
      callg_update_list = NULL;

      citime += (IPA_GetTime () - stime);     
      IPA_prog_classify_all(info, NULL);
      IPA_cg_edgepool_print_info(stdout);
      IPA_cg_nodepool_print_info(stdout);
      IPA_TrackTime(1);
    }
  while (changes);
/*   printf ("###### DONE #########################\n"); */
  IPA_TrackTime(0);


  /* VARIOUS OUTPUTS/STATS */
  {
    char name[256];

#if 0
    sprintf(name,"SOLVED.%s.%s",
	    IPA_context_string[IPA_context_option],
	    IPA_field_string[IPA_field_option]);
    IPA_cg_DOTprint (prog_node->fninfo->consg, name, IPA_CG_ETYPE_ALL);
#endif

    sprintf(name,"CALLGRAPH_POST.%s.%s.%s",
	    IPA_context_string[IPA_context_option],
	    IPA_field_string[IPA_field_option],
	    IPA_cloning_string[IPA_cloning_option]);
    IPA_callg_DVprint (info->call_graph, name);
  }

/*   printf("GDATA CI TIME %f\n", citime); */

  IPA_callg_stats(info->call_graph);
  fflush(stdout);
  fflush(stderr);
}

/*************************************************************************
 *
 * CONTEXT SENSITIVE ANALYSIS CORE LOOP
 *
 *************************************************************************/

void
IPA_Context_Sensitive_Recycle (IPA_prog_info_t *info)
{
  IPA_callg_node_t *node;
  IPA_funcsymbol_info_t *fninfo;
  List tsort = NULL;
  List callee_delta = NULL;
  List edge_delta = NULL;
  List callg_update_list = NULL;
  int change = 0, done_once;
  int IPA_round = 0;
  char name[256];
  double stime, tdtime, butime;

  /* Solve until convergence 
   */
  callee_delta = NULL;
  tsort = NULL;
  IPA_TrackTime(1);
  tdtime = butime = 0.0;
  IPA_round = 0;

  do
    {
      change = 0;
      IPA_round++;
      debug.round = IPA_round;

/*       printf ("###### ROUND %d ##################################\n", IPA_round); */


      /*********************************
       *  TOP DOWN
       */
      DEBUG_IPA(1, printf ("###### ROUND %d TD ##################################\n", IPA_round););
      done_once = 0;
      stime = IPA_GetTime ();
      do
	{
	  /* Remove recursive cycles, self-cycles, 
	     distill globals, connect param/ret */
	  IPA_callgraph_prepare_all (info);
	  
	  /* Get edges from newly accessed funcs */
	  edge_delta = IPA_listof_newaccess(info);
	  
	  if (List_size(edge_delta) <= 0 ||
	      (IPA_slow_callgraph && IPA_round == 1) ||
	      (IPA_slow_callgraph && done_once))
	    break;
	  done_once = 1;
	  DEBUG_IPA(1, IPA_TrackTime(0);
		    printf("TD LIST: %d\n", List_size(edge_delta)););

	  IPA_driver_solve_graph (info, NULL, edge_delta, IPA_field_option);
	  callee_delta = IPA_get_callee_delta(info);

	  /* Update callgraph */
	  callg_update_list = 
	    IPA_callgraph_new_callees (info, NULL, callee_delta, IPA_round);
	  List_reset(callee_delta);
  
	  IPA_callg_update_callg(info, callg_update_list, IPA_round);
	  callg_update_list = NULL;
	}
      while (1);
      List_reset(edge_delta);
      edge_delta = NULL;
      tdtime += (IPA_GetTime () - stime);


      /*********************************
       * STATS AND PRINTING 
       */
      List_start(info->fninfos);
      while ((fninfo = List_next(info->fninfos)))
	{
	  if (!fninfo->has_been_called)
	    continue;
	  /* Skip those that were merged into an SCC */
	  if (fninfo->call_node->rep_parent != fninfo->call_node)
	    continue;
#if 1
	  if (!strcmp(fninfo->func_name, "jinit_write_bmp"))
	    {
	      sprintf(name, "%s.TD%d", fninfo->func_name, IPA_round);
	      IPA_cg_DOTprint (fninfo->consg, name, IPA_CG_ETYPE_ALL);
	    }
#endif

	  DEBUG(IPA_consg_check_graph(info, fninfo->consg););
	}
      DEBUG_IPA(1, IPA_TrackTime(0););

#if PARTIALSUM
      printf("CHECK PART 1\n");
      List_start(info->fninfos);
      while ((fninfo = List_next(info->fninfos)))
	{
	  if (!fninfo->has_been_called)
	    continue;
	  /* Skip those that were merged into an SCC */
	  if (fninfo->call_node->rep_parent != fninfo->call_node)
	    continue;
	  if (IPA_changed_observed(info, fninfo))
	    {
	      invalidate_single(fninfo->call_node);
	    }
	}
#endif

      /*********************************
       * SETUP BOTTOM UP
       */
      stime = IPA_GetTime ();
      DEBUG_IPA(1, printf ("###### ROUND %d BU ##################################\n", IPA_round););
      
      /* Get topological sort */
      List_reset(tsort);
      tsort = IPA_callg_find_toposort (info->call_graph, info->globals->call_node);
      DEBUG_IPA(1, IPA_callg_topo_print(stdout, tsort););

      /*********************************
       * BOTTOM UP
       */
      callg_update_list = NULL;
      node = List_last (tsort);
      do
        {
	  fninfo = node->fninfo;
	  DEBUG_IPA(0, IPA_TrackTime(0);
		    printf ("## BU [%s]\n", fninfo->func_name););
/* 	  printf("+"); */

          if (fninfo != info->globals)
            {
	      DEBUG_IPA(1,  printf("## INCORP SUM\n"););
	      /* Incorporate all callee summaries  */
	      IPA_callgraph_apply_callee_summaries2 (info, node, IPA_round);
	      
	      DEBUG_IPA(1, printf("## CYCLE MERGE\n"););
	      IPA_a_cycle_detection(fninfo->consg,
				    (IPA_CG_NODE_FLAGS_GLOBAL |
				     IPA_CG_NODE_FLAGS_CALLEE |
				     IPA_CG_NODE_FLAGS_SUMMARY),
				    CD_SELECT);
	    }

          edge_delta = IPA_consg_build_listof_new_edges(NULL, fninfo);
	  DEBUG_IPA(1, printf("EDGE SEED: %d\n",List_size(edge_delta)););

          /* Solve */
	  DEBUG_IPA(1, printf("## SOLVE\n"););
	  IPA_driver_solve_graph (info, fninfo->consg, edge_delta,
				  IPA_field_option);
	}
      while ((node = List_prev (tsort)));
      butime += (IPA_GetTime () - stime);
/*       printf("#\n"); */
      fflush(stdout);


      /*********************************
       * STATS AND PRINTING 
       */
      List_start(info->fninfos);
      while ((fninfo = List_next(info->fninfos)))
	{
	  if (!fninfo->has_been_called)
	    continue;
	  /* Skip those that were merged into an SCC */
	  if (fninfo->call_node->rep_parent != fninfo->call_node)
	    continue;
	  if (IPA_print_summary_cng)
	    {
	      sprintf(name, "%s.SV%d", 
		      fninfo->func_name, IPA_round);
	      IPA_cg_DOTprint (fninfo->consg, name, IPA_CG_ETYPE_ALL);
	    }
	  DEBUG(IPA_consg_check_graph(info, fninfo->consg););
	}


      /*********************************
       * Perform any callgraph updates 
       */
      callee_delta = IPA_get_callee_delta(info);
      callg_update_list = 
	IPA_callgraph_new_callees (info, callg_update_list,
				   callee_delta, IPA_round);
      List_reset(callee_delta);
      callee_delta = NULL;
      
      change = IPA_callg_update_callg(info, callg_update_list,
				      IPA_round);
      callg_update_list = NULL;

#if PARTIALSUM
      if (!change)
	{
	  printf("CHECK PART 2\n");
	  List_start(info->fninfos);
	  while ((fninfo = List_next(info->fninfos)))
	    {
	      if (!fninfo->has_been_called)
		continue;
	      /* Skip those that were merged into an SCC */
	      if (fninfo->call_node->rep_parent != fninfo->call_node)
		continue;
	      if (IPA_changed_observed(info, fninfo))
		{
		  printf("FOUND CHANGE\n");
		  change = 1;
		  break;
		}
	    }
	}
#endif

      IPA_prog_classify_all(info, tsort);
      IPA_cg_edgepool_print_info(stdout);
      IPA_cg_nodepool_print_info(stdout);
      IPA_TrackTime(1);
    }
  while(change > 0);
/*   printf ("\n###### DONE #########################\n"); */

  /*********************************
   * STATS AND PRINTING 
   */

#if 1
  List_start(info->fninfos);
  while ((fninfo = List_next(info->fninfos)))
    {
      IPA_HTAB_ITER niter;
      if (!fninfo->consg)
	continue;

      IPA_HTAB_START(niter, fninfo->consg->nodes);
      IPA_HTAB_LOOP(niter)
	{
	  IPA_cgraph_node_t *node;
	  IPA_cgraph_edge_list_t *elist;	  
	  IPA_HTAB_ITER eiter;

	  if (!fninfo->has_been_called)
	    continue;

	  node = IPA_HTAB_CUR(niter);
	  for (elist=node->first_list; elist; elist=elist->nxt_list)
	    {
	      IPA_cgraph_edge_t *edge;

	      IPA_HTAB_START(eiter, elist->in);
	      IPA_HTAB_LOOP(eiter)
		{
		  edge = IPA_HTAB_CUR(eiter);

		  if (!edge->src_elist->node->data.syminfo->fninfo->has_been_called)
		    continue;
		  assert(IPA_FLAG_ISSET(edge->flags, IPA_CG_EDGE_FLAGS_PROCESSED));
		}
	    }
	}
    }
#endif

  sprintf(name,"CALLGRAPH_POST.%s.%s.%s",
	  IPA_context_string[IPA_context_option],
	  IPA_field_string[IPA_field_option],
	  IPA_cloning_string[IPA_cloning_option]);
  IPA_callg_DVprint (info->call_graph, name);

  List_start (tsort);
  while ((node = List_next (tsort)))
    {
      fninfo = node->fninfo;
      if (IPA_print_final_cng)
	{
	  sprintf(name, "%s.%s.FINAL", 
		  fninfo->func_name,
		  IPA_field_string[IPA_field_option]);
	  IPA_cg_DOTprint (fninfo->consg, name, IPA_CG_ETYPE_ALL);

	  if (fninfo->lsum_consg)
	    {
	      sprintf(name, "%s.%s.SUMFINAL", 
		      fninfo->func_name,
		      IPA_field_string[IPA_field_option]);
	    }
	}
    }

  IPA_callg_stats(info->call_graph);
/*   printf("GDATA TD TIME %f\n", tdtime); */
/*   printf("GDATA BU TIME %f\n", butime); */
  fflush(stdout);
  fflush(stderr);
}


/*****************************************************************************
 * Solve a constraint graph 
 *****************************************************************************/

void
IPA_driver_solve_graph (IPA_prog_info_t *info, 
			IPA_cgraph_t * consg, List edge_delta,
			int field_option)
{
  switch (field_option)
    {
    case IPA_FIELD_INDEPENDENT:
#if 0
      IPA_consg_fi_solve_fully (consg, edge_delta);
#endif
      IPA_consg_fdvs_solve_fully (consg, edge_delta);      
      break;

    case IPA_FIELD_DEPENDENT_VARIABLE_SIZE:
      IPA_consg_fdvs_solve_fully (consg, edge_delta);
      break;

    default:
      I_punt ("IPA_consg_solve_graph : invalid field option\n");
      break;
    }
}



/*****************************************************************************
 * Compact a constraint graph 
 *****************************************************************************/

void
IPA_driver_summarize_graph (IPA_prog_info_t * info, 
			    IPA_cgraph_t * cg,
			    IPA_interface_t *iface,
			    IPA_funcsymbol_info_t *fninfo,
			    IPA_cgraph_t **local_sum_ptr, 
			    char *func_name,
			    int field_option)
{
  switch (field_option)
    {
    case IPA_FIELD_INDEPENDENT:
      IPA_consg_fdvs_summarize(info, cg, iface, fninfo,
			       local_sum_ptr, func_name);
#if 0
      IPA_consg_fi_summarize(info, cg, iface, fninfo,
			     local_sum_ptr, func_name);
#endif
      break;
      
    case IPA_FIELD_DEPENDENT_VARIABLE_SIZE:
      IPA_consg_fdvs_summarize(info, cg, iface, fninfo,
			       local_sum_ptr, func_name);
      break;
      
    default:
      I_punt ("IPA_consg_solve_graph : invalid field option\n");
      break;      
    }
}



/*****************************************************************************
 * Print name
 *****************************************************************************/

void
IPA_Interproc_Print_Analysis_Name (FILE * file)
{
/*   fprintf(file,"\n################\n"); */
/*   fprintf(file,"IPA FILE DIR : %s\n",IPA_file_subdir); */
/*   fprintf(file,"POINTER SIZE : %d\n", IPA_POINTER_SIZE); */
/*   fprintf(file,"MYFLAG       : %d\n", IPA_myflag); */

/*   fprintf(file,"PCODE EXPR   : %s\n", IPA_allow_pcode_expr_string[IPA_allow_pcode_expr]); */

/*   fprintf(file,"MISSING FUNC : "); */
/*   if (IPA_allow_missing_ipa) */
/*     fprintf(file,"Only warn at function absence\n"); */
/*   else */
/*     fprintf(file,"Fail at function absence\n"); */

/*   fprintf(file,"ACT-FML      : "); */
/*   if (IPA_use_actualformal_filter) */
/*     fprintf(file,"Use prototype to filter indirect calls\n"); */
/*   else */
/*     fprintf(file,"Do not filter using function prototype\n"); */

/*   fprintf(file,"FS-COST      : "); */
/*   if (IPA_solver_limit_fscost) */
/*     fprintf(file,"Limit cost of field sensitivity\n"); */
/*   else */
/*     fprintf(file,"Normal field sensitivity\n"); */
/*   fprintf(file,"FS SAFETY    : %s\n", IPA_field_safety_string[IPA_field_safety]); */
  
/*   fprintf(file,"CONTEXT      : %s\n", IPA_context_string[IPA_context_option]); */
/*   fprintf(file,"CS RECURSION : %s\n", IPA_csrec_string[IPA_csrec_option]); */
/*   fprintf(file,"FIELD        : %s\n", IPA_field_string[IPA_field_option]); */
/*   fprintf(file,"FIELD SAFETY : %s\n", IPA_field_safety_string[IPA_field_safety]); */
/*   fprintf(file,"HEAP         : %s\n", IPA_cloning_string[IPA_cloning_option]); */
/*   fprintf(file,"HEAP GEN     : %d\n", IPA_cloning_gen); */
/*   fprintf(file,"GRAPH CON    : %s\n", IPA_gcon_string[IPA_gcon_option]); */

/*   fprintf(file,"################\n"); */
}



/*****************************************************************************
 * Do Analysis
 *****************************************************************************/

void
IPA_Interproc_PointsTo (IPA_prog_info_t *info)
{
  IPA_funcsymbol_info_t * fninfo;
  char name[256];

  debug.print = 0;
  debug.round = 0;
  debug.check = 0;

  List_start(info->fninfos);
  while ((fninfo = List_next(info->fninfos)))
    {
      IPA_consg_setup_nodes(info, fninfo, fninfo->consg);
      IPA_consg_check_graph(info, fninfo->consg);
      if (!fninfo->consg)
	continue;
      fninfo->orig_size = IPA_htab_size(fninfo->consg->nodes);
    }

  printf ("\n");
  IPA_Interproc_Print_Analysis_Name (stdout);
  printf ("\n\n");

  List_start(info->fninfos);
  while ((fninfo = List_next(info->fninfos)))
    {
      IPA_find_merge_equiv2(fninfo->consg);
    }

  IPA_Interproc_Start (info);

  List_start(info->fninfos);
  while ((fninfo = List_next(info->fninfos)))
    {
#if 1
      if (!fninfo->from_library)
	{
	  sprintf(name, "%s.INIT", fninfo->func_name);
	  IPA_cg_DOTprint (fninfo->consg, name, IPA_CG_ETYPE_ALL);
	}
#endif
      IPA_k_a_cycle_detection(fninfo->consg);
    }

  switch (IPA_context_option)
    {
    case IPA_CONTEXT_INSENSITIVE:
      IPA_Context_Insensitive (info);
      break;

    case IPA_CONTEXT_SENSITIVE:
      IPA_Context_Sensitive_Recycle (info);
      break;

    default:
      I_punt ("IPA_Interproc_PointsTo: unexpected case\n");
    }

  IPA_callg_FLprint(info->call_graph, "CALLGRAPH");
}

