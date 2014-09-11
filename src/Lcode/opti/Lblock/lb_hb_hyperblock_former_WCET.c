/*****************************************************************************\
 *
 *		      Illinois Open Source License
 *                     University of Illinois/NCSA
 *                         Open Source License
 *
 * Copyright (c) 2004, The University of Illinois at Urbana-Champaign.
 * All rights reserved.
 *
 * Developed by:             
 *
 *		IMPACT Research Group
 *
 *		University of Illinois at Urbana-Champaign
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
/*===========================================================================
 *      File :          l_hyperblock_former.c
 *      Description :   Driver for hyperblock formation
 *      Creation Date : February 1998
 *      Authors :       Kevin Crozier
 *        modified from Scott Mahlke's l_hyperblock.c
 *
 *==========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "lb_hb_hyperblock.h"
#include "lb_hb_peel.h"
#include "../../tools/wcet_analysis/wcet_analysis.h"
LB_HB_Stat LB_hb_stat;

#undef DEBUG_HB_FORMER

// Added by {Morteza}:

#include <stdio.h>
#include <string.h>
//#include <./lb_hb_block_enum.c>

#undef SELECT_PATH
#undef FULL_SELECT_PATH
#undef SHARIF
#undef SHARIF2
#undef SHARIF3
#define SHARIF4
#undef My_basic_block
#undef COUNT_number_of_equal_branch
#undef SHARIF4_super
#undef SHARIF4_hyper

double Max_exec = 0;

L_Cb *
My_LB_hb_find_immediate_dominator(L_Cb * cb) {
	L_Flow *flow;
	L_Cb *src_cb;

	if (!cb)
		L_punt("LB_hb_find_immediate_dominator: cb is NULL");

	for (flow = cb->src_flow; flow != NULL; flow = flow->next_flow) {
		src_cb = flow->src_cb;
		if (src_cb == cb)
			continue;
		if (L_in_cb_DOM_set(cb, flow->src_cb->id))
			return (flow->src_cb);
	}

	return (NULL);
}

//int is_good(L_Cb * f_cb, L_Cb * l_cb) {
//	int cnt = 0;
//	L_Flow * flow;
//	L_Cb * t_cb;
//	for (flow = l_cb->src_flow; flow != NULL; flow = flow->next_flow)
//		cnt++;
//	if (cnt != 2)
//		return 0;
//
//	cnt = 0;
//	for (flow = f_cb->dest_flow; flow != NULL; flow = flow->next_flow) {
//		if (flow->dst_cb != l_cb)
//			t_cb = flow->dst_cb;
//		cnt++;
//	}
//	if (cnt != 2)
//		return 0;
//
//	cnt = 0;
//	for (flow = t_cb->dest_flow; flow != NULL; flow = flow->next_flow) {
//		if (flow->dst_cb != l_cb)
//			return 0;
//		cnt++;
//	}
//	if (cnt != 1)
//		return 0;
//
//	return 1;
//
//}
/*Set My_get_set_of_blocks(L_Cb * f_cb ,L_Cb * l_cb){
 Set temp=NULL;
 L_Flow * flow;
 while(1){

 for(flow=f_cb->dest_flow;flow!=NULL;flow=flow->next_flow)
 {



 }



 }



 }
 */

//void my_func(L_Func * fn) {
//	L_Cb *cb;
//	L_Cb *t_cb;
//
//	L_Flow * flow;
//	Set temp = NULL;
//	int cnt;
//	for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb) {
//		t_cb = My_LB_hb_find_immediate_dominator(cb);
//
//		if (t_cb == NULL)
//			continue;
//
//		if (Set_in(t_cb->pdom, cb->id)) {
//			//fprintf(stderr,"salam (%d) (%d)\n",cb->id,t_cb->id);
//			if (cb->src_flow->next_flow != NULL && t_cb->dest_flow->next_flow
//					!=NULL) {
//				Set_dispose(temp);
//				temp = 0;
//				for (flow = cb->src_flow; flow != NULL; flow = flow->next_flow) {
//					if (flow->src_cb->id == t_cb->id) {
//						if (is_good(t_cb, cb)) {
//							fprintf(stderr, "CB(%d) & CB(%d) is chock\n",
//									cb->id, t_cb->id);
//							temp = Set_add(temp, cb->id);
//							temp = Set_add(temp, t_cb->id);
//							if (t_cb->dest_flow->dst_cb != cb)
//								temp = Set_add(temp,
//										t_cb->dest_flow->dst_cb->id);
//							else
//								temp = Set_add(temp,
//										t_cb->dest_flow->next_flow->dst_cb->id);
//
//							t_cb->attr = L_concat_attr(t_cb->attr,
//									L_new_attr(HB_SELECT_ALL_ATTR, 0));
//							LB_TraceRegion * tr = LB_hb_select_trivial(
//									L_TRACEREGION_TRACE, t_cb, cb, temp, 1);
//							LB_summarize_tr(stderr, tr);
//
//							break;
//						}
//					}
//
//				}
//
//			}
//		}
//		/*
//		 cnt=0;
//		 Set_dispose(temp);
//		 temp=0;
//		 //fprintf(stderr,"CB(%d)(%d)\n",cb->id,cnt);
//		 for(flow=cb->src_flow;flow!=NULL;flow=flow->next_flow)
//		 {
//		 if(flow->src_cb){
//		 if(!Set_in(flow->src_cb->dom,cb->id)){
//		 //fprintf(stderr,"(%d)\n",flow->src_cb->id);
//		 cnt++;
//		 temp=Set_add(temp,flow->src_cb->id);
//
//		 }
//		 }
//		 }
//
//		 if(cnt<2)
//		 continue;
//
//		 int * buf = (int *) Lcode_malloc (sizeof (int) * cnt);
//		 Set_2array (temp, buf);
//		 int i;
//		 LB_hb_find_immediate_post_dominator()
//		 fprintf(stderr,"chock:%d\n",cb->id);
//		 for (i = 0; i < cnt; i++)
//		 {
//
//		 int j;
//		 for (j = 0; j < cnt; j++)
//		 {
//		 if(j!=i)
//		 {
//		 t_cb=L_cb_hash_tbl_find(L_fn->cb_hash_tbl, buf[j]);
//		 if(Set_in(t_cb->dom,buf[i]))
//		 {
//		 fprintf(stderr,"yes\t");
//
//		 while(t_cb->id!=buf[i])
//		 {
//		 fprintf(stderr,"<-(%d)",t_cb->id);
//		 t_cb=t_cb->src_flow->src_cb;
//
//
//		 }
//
//		 }
//		 }
//		 }
//		 }*/
//	}
//
//}
//*/
GraphNode get_imm_post_domin(List all_nodes, GraphNode node) {
	GraphNode sub_node;
	List_start(all_nodes);
	//fprintf(stderr,"NODE_M(%d)\n",node->id);
	while ((sub_node = (GraphNode) List_next(all_nodes))) {
		//	fprintf(stderr,"NODE(%d)\n",sub_node->id);
		if ((node->id != sub_node->id) && Set_in(node->post_dom, sub_node->id))
			return sub_node;

	}
	return NULL;
}

void List_set_postion(List list, void * ptr) {
	void * node;
	List_start(list);
	while (node = (GraphNode) List_next(list)) {

		if (node == ptr)
			return;
	}

}

void is_simple_2(List all_nodes, GraphNode first_node, GraphNode last_node) {
	GraphNode sub_node;
	((L_Cb *) (first_node->ptr))->chock = 1;
	List_set_postion(all_nodes, (void *) first_node);
	while ((sub_node = (GraphNode) List_next(all_nodes)) != last_node) {
		if (List_size(sub_node->succ) > 1) {
			((L_Cb *) (first_node->ptr))->chock = 0;
			if (!(((L_Cb *) (sub_node->ptr))->flags & L_CB_LOOP_HEADER)) {
				is_simple_2(all_nodes, sub_node,
						get_imm_post_domin(all_nodes, sub_node));
				List_set_postion(all_nodes, (void *) sub_node);
			}
		} else if (((L_Cb *) (sub_node->ptr))->flags & L_CB_LOOP_HEADER) {
			((L_Cb *) (first_node->ptr))->chock = 0;

		}

	}
	if (((L_Cb *) (first_node->ptr))->chock)
		fprintf(stderr, "%d->%d is simple\n", first_node->id, last_node->id);

}

void find_simples(Graph g) {
	GraphNode node, node2;
	Set p = ((GraphNode) List_get_first(g->topo_list))->post_dom;
	int n = Set_size(p);
	int * buf = (int *) malloc(sizeof(int) * n);
	Set_2array(p, buf);

	int i;
	for (i = 0; i < n; i++) {
		node = Graph_node_from_id(g, buf[i]);
		node2 = get_imm_post_domin(g->topo_list, node);

		if (node2 != NULL && List_size(node->succ) > 1) {
			if ((((L_Cb*) (node->ptr))->flags & L_CB_LOOP_HEADER)
					&& (((L_Cb*) (node2->ptr))->flags & L_CB_LOOP_HEADER))
				is_simple_2(g->topo_list, node, node2);
		}
	}

}

//void find_simples_in_loop(L_Loop * loop, GraphNode head, List all_nodes) {
//	GraphNode sub_node;
//	List_set_postion(all_nodes, head);
//	while ((sub_node = (GraphNode) List_next(all_nodes))) {
//		if (Set_in(loop->loop_cb, sub_node->id)) {
//			fprintf(stderr, "CB(%d)\n", sub_node->id);
//
//		}
//
//	}
//
//}

Set find_set_backedges(L_Func * fn) {
	Set temp = Set_new();
	L_Loop * loop;
	for (loop = fn->first_loop; loop; loop = loop->next_loop) {
		//fprintf(stderr,"loop(%d):\n",loop->header->id);
		//find_simples_in_loop(loop,Graph_node_from_id(bb_g,loop->header->id),bb_g->topo_list);
		temp = Set_union(temp, loop->back_edge_cb);

	}
	Set_print(stderr, "", temp);
	return temp;

}

Set find_set_headers(L_Func * fn) {
	Set temp = Set_new();
	L_Loop * loop;
	for (loop = fn->first_loop; loop; loop = loop->next_loop) {
		//fprintf(stderr,"loop(%d):\n",loop->header->id);
		//find_simples_in_loop(loop,Graph_node_from_id(bb_g,loop->header->id),bb_g->topo_list);
		temp = Set_add(temp, loop->header->id);

	}
	Set_print(stderr, "", temp);
	return temp;

}

//void find_chock(L_Func * fn)
//{
//	L_Cb * cb;
//	L_Cb
//	for(cb=fn->first_cb;cb;cb=cb->next_cb)
//	{
//
//
//
//
//	}
//
//
//
//}

LB_TraceRegion * My_simple_trace_formation(L_Func * fn,
		LB_TraceRegion_Header *header, int type, L_Cb *start, L_Cb * end,
		Set all_blocks, int id) {
	LB_TraceRegion *tr;
	Set blocks = NULL;
	blocks = Set_copy(all_blocks);

	tr = LB_create_traceregion(fn, id, start, blocks, type);

	L_Flow * flow;
	double w = 0;
	for (flow = end->src_flow; flow; flow = flow->next_flow) {
		if (Set_in(all_blocks, flow->src_cb->id))
			w += flow->weight;
	}

	tr->exec_ratio = w;
	tr->slots_used = 0;
	tr->slots_avail = 0;
	tr->dep_height = 0;

	tr->wcet2 = end->wcet;

	tr = LB_compute_traceregion_info(tr);

	Set_dispose(blocks);

	return tr;
}

void my_trace_formation(L_Func * fn, LB_TraceRegion_Header *header, int c) {

	//GraphNode node;
	//GraphNode first_node;

	Graph bb_g = L_create_cb_graph(fn);
	Graph_dominator(bb_g);
	Graph_post_dominator(bb_g);
	Graph_imm_dominator(bb_g);
	Graph_imm_post_dominator(bb_g);
	Graph_control_dependence(bb_g);
	Graph_topological_sort(bb_g);
	Graph_preorder_dfs_sort(bb_g, 1);
	Graph_preorder_dfs_visit(bb_g, bb_g->root, 1);
	/*
	 find_simples(bb_g);
	 L_Loop * loop;
	 for(loop=fn->first_loop;loop;loop=loop->next_loop){
	 fprintf(stderr,"loop(%d):\n",loop->header->id);
	 find_simples_in_loop(loop,Graph_node_from_id(bb_g,loop->header->id),bb_g->topo_list);

	 }
	 L_Cb * cb;
	 for(cb=fn->first_cb;cb;cb=cb->next_cb)
	 {
	 if(cb->flags & L_CB_LOOP_HEADER)
	 fprintf(stderr,"Header:(%d)",cb->id);


	 }*/

	Set backedges = find_set_backedges(fn);
	Set headers = find_set_headers(fn);

	L_Cb * cb;
	int t;
	GraphNode node;
	List_start(bb_g->nodes);
	if (c % 2 == 1) {
		while ((node = (GraphNode) List_next(bb_g->nodes))) {
			//GraphNode node=Graph_node_from_id(bb_g,cb->id);

			if (!Set_in(backedges, node->id) && List_size(node->succ) > 1) {
				GraphNode node2 = get_imm_post_domin(bb_g->topo_list, node);
				if (node2) {
					if (!Set_in(headers, node2->id)) {

						//fprintf(stderr,"(%d)->(%d) is candid\n",node->id,node2->id);
						List_set_postion(bb_g->topo_list, node);
						GraphNode sub_node;
						t = 1;
						Set temp = Set_new();
						while ((sub_node = (GraphNode) List_next(
								bb_g->topo_list)) != node2) {
							if (!Set_in(sub_node->dom, node->id)
									|| !Set_in(sub_node->post_dom, node2->id))
								continue;
							Set_add(temp, sub_node->id);

							if (Set_in(headers, sub_node->id)
									|| List_size(sub_node->succ) > 1
									|| Set_in(backedges, sub_node->id)) {
								t = 0;
								break;
							}

						}
						if (t == 1) {
							//fprintf(stderr,"*********\n(%d)->(%d):\n",node->id,node2->id);
							((L_Cb *) node->ptr)->chock = 1;
							((L_Cb *) node->ptr)->chock_end_cb =
									((L_Cb *) node2->ptr);
							((L_Cb *) node2->ptr)->chock_first_cb =
									((L_Cb *) node->ptr);
							((L_Cb *) node->ptr)->chock_middle_nodes = temp;
							Set_add(temp, node->id);
							//Set_add(temp,node2->id);
							//if(Set_in(temp,5))
							//Set_delete(temp,5);
							LB_TraceRegion * tr = My_simple_trace_formation(fn,
									header, L_TRACEREGION_GENERAL,
									(L_Cb *) node->ptr, (L_Cb *) node2->ptr,
									temp, header->next_id++);
							//LB_summarize_tr (stderr, tr);
							header->traceregions = List_insert_last(
									header->traceregions, tr);
							//Set_print(stderr,"",temp);

						}
					}

				}
			}
		}

	} else if (c % 2 == 0) {

		List_start(bb_g->nodes);

		while ((node = (GraphNode) List_next(bb_g->nodes))) {
			if (!Set_in(backedges, node->id) && List_size(node->succ) == 1) {
				GraphNode last_node, node2;
				Set temp = Set_new();
				node2 = get_imm_post_domin(bb_g->topo_list, node);
				last_node = node2;
				while (1) {
					if (!Set_in(headers, node2->id)
							&& List_size(node2->succ) == 1) {
						Set_add(temp, node2->id);
						last_node = node2;

						node2 = get_imm_post_domin(bb_g->topo_list, node2);

					} else {

						break;
					}

				}
				if (Set_size(temp) > 1) {

					((L_Cb *) node->ptr)->chock = 1;
					((L_Cb *) node->ptr)->chock_end_cb = ((L_Cb *) node2->ptr);
					((L_Cb *) node2->ptr)->chock_first_cb =
							((L_Cb *) node->ptr);
					((L_Cb *) node->ptr)->chock_middle_nodes = temp;
					Set_add(temp, node->id);
					LB_TraceRegion * tr = My_simple_trace_formation(fn, header,
					L_TRACEREGION_GENERAL, (L_Cb *) node->ptr,
							(L_Cb *) last_node->ptr, temp, header->next_id++);
					LB_summarize_tr(stderr, tr);
					header->traceregions = List_insert_last(
							header->traceregions, tr);

				}
				Set_dispose(temp);
			}

		}
	}

	Graph_free_graph(bb_g);
}
Set backedge;
Set headers;

void visit_2(L_Cb * node, L_Cb * first_node, L_Cb * last_node, Set blocks,
		LB_TraceRegion_Header *header, double wcet) {

	fprintf(stderr, "CB(%d)=%d\n", node->id, node->ILP_is_in_wcet_path);
	if (node->ILP_is_in_wcet_path == 1) {
		if (!Set_in(headers, node->id) && !Set_in(backedge, node->id)) {
			fprintf(stderr, "NONE\n");
			if (node->dest_flow) {
				if (!L_EXTRACT_BIT_VAL(node->flags,
						L_CB_SUPERBLOCK) && !L_EXTRACT_BIT_VAL(node->flags, L_CB_HYPERBLOCK)) {
					Set_add(blocks, node->id);
					L_Cb * node2;
					L_Flow * flow;
					for (flow = node->dest_flow; flow; flow = flow->next_flow) {
						if (flow->ILP_is_in_wcet_path) {
							node2 = flow->dst_cb;
							visit_2(node2, first_node, node, blocks, header,
									wcet + flow->wcet_weight);
						}
					}

				} else {
					L_Cb * node2;
					L_Flow * flow;
					for (flow = node->dest_flow; flow; flow = flow->next_flow) {
						if (flow->ILP_is_in_wcet_path) {
							node2 = flow->dst_cb;
							Set temp = Set_new();
							visit_2(node2, node2, node2, temp, header, 0);
							Set_dispose(temp);
						}
					}
					if (Set_size(blocks) > 1) {

						LB_TraceRegion * tr = My_simple_trace_formation(
								header->fn, header, L_TRACEREGION_HAMMOCK,
								first_node, last_node, blocks,
								header->next_id++);
						tr->wcet2 = wcet;
						header->traceregions = List_insert_last(
								header->traceregions, tr);
					}
				}

			} else {
				fprintf(stderr, "QWER\n");
				if (Set_size(blocks) > 1) {
					LB_TraceRegion * tr = My_simple_trace_formation(header->fn,
							header, L_TRACEREGION_HAMMOCK, first_node,
							last_node, blocks, header->next_id++);
					tr->wcet2 = wcet;
					header->traceregions = List_insert_last(
							header->traceregions, tr);
				}
				fprintf(stderr, "QWER\n");
			}

		} else if (Set_in(headers, node->id) && Set_in(backedge, node->id)) {
			L_Cb * node2;
			L_Flow * flow;
			fprintf(stderr, "BOTH\n");
			//fprintf(stderr,"QQQQQQQQQQQQ\n");
			for (flow = node->dest_flow; flow; flow = flow->next_flow) {
				if (flow->ILP_is_in_wcet_path && flow->dst_cb != node) {
					node2 = flow->dst_cb;
					Set temp = Set_new();
					visit_2(node2, node2, node2, temp, header, 0);
					Set_dispose(temp);

				}

			}

			if (Set_size(blocks) > 1) {

				LB_TraceRegion * tr = My_simple_trace_formation(header->fn,
						header, L_TRACEREGION_HAMMOCK, first_node, last_node,
						blocks, header->next_id++);
				tr->wcet2 = wcet;
				header->traceregions = List_insert_last(header->traceregions,
						tr);
			}

		} else if (!Set_in(headers, node->id) && Set_in(backedge, node->id)) {
			fprintf(stderr, "BACKEDGE\n");
			L_Cb * node2;
			L_Flow * flow;
			Set_add(blocks, node);
			for (flow = node->dest_flow; flow; flow = flow->next_flow) {
				node2 = flow->dst_cb;
				if (!Set_in(node->dom, node2->id)
						&& flow->ILP_is_in_wcet_path) {
					Set temp = Set_new();
					//Set_add(temp,s->id);
					visit_2(node2, node2, node2, temp, header, 0);
					Set_dispose(temp);
				}
			}
			if (Set_size(blocks) > 1) {

				LB_TraceRegion * tr = My_simple_trace_formation(header->fn,
						header, L_TRACEREGION_HAMMOCK, first_node, node, blocks,
						header->next_id++);
				tr->wcet2 = wcet;
				Set_print(stderr, "", blocks);
				//LB_summarize_tr (stderr, tr);
				header->traceregions = List_insert_last(header->traceregions,
						tr);

			}
		} else if (Set_in(headers, node->id) && !Set_in(backedge, node->id)) {
			L_Cb * node2;
			L_Flow * flow;
			fprintf(stderr, "Header\n");
			for (flow = node->dest_flow; flow; flow = flow->next_flow) {
				if (flow->ILP_is_in_wcet_path) {
					node2 = flow->dst_cb;
					//	fprintf(stderr,"CP(%d)\n",node2->id);

					Set temp = Set_new();

					char nb[100];
					L_cb_flags_to_string(nb, node->flags);
					//fprintf(stderr,"CB(%d):%s\n",node->id,nb);

					if (!L_EXTRACT_BIT_VAL(node->flags,
							L_CB_SUPERBLOCK) && !L_EXTRACT_BIT_VAL(node->flags, L_CB_HYPERBLOCK)) {
						Set_add(temp, node->id);
						visit_2(node2, node, node, temp, header,
								wcet + flow->wcet_weight);
					} else {
						visit_2(node2, node2, node2, temp, header, 0);

					}
					Set_dispose(temp);
				}
			}
			if (Set_size(blocks) > 1) {
				//fprintf(stderr,"AAAAAAAAAAAAAAAAAAAAAAAAAAAA\n");
				LB_TraceRegion * tr = My_simple_trace_formation(header->fn,
						header, L_TRACEREGION_HAMMOCK, first_node, last_node,
						blocks, header->next_id++);
				//LB_summarize_tr (stderr, tr);
				tr->wcet2 = wcet;
				header->traceregions = List_insert_last(header->traceregions,
						tr);
			}

		}
	}
	fprintf(stderr, "WEWERR\n");
	return;
}

void visit(L_Cb * node, L_Cb * first_node, L_Cb * last_node, Set blocks,
		LB_TraceRegion_Header *header, double wcet) {

	fprintf(stderr, "CB(%d)\n", node->id);
	if (node->ILP_is_in_wcet_path == 1) {
		if (!Set_in(headers, node->id) && !Set_in(backedge, node->id)) {
			if (node->dest_flow) {
				if (!L_EXTRACT_BIT_VAL(node->flags,
						L_CB_SUPERBLOCK) && !L_EXTRACT_BIT_VAL(node->flags, L_CB_HYPERBLOCK)) {
					Set_add(blocks, node->id);
					L_Cb * node2;
					L_Flow * flow;
					for (flow = node->dest_flow; flow; flow = flow->next_flow) {
						if (flow->ILP_is_in_wcet_path) {
							node2 = flow->dst_cb;
							visit(node2, first_node, node, blocks, header,
									wcet + flow->wcet_weight);
						}
					}

				} else {
					L_Cb * node2;
					L_Flow * flow;
					for (flow = node->dest_flow; flow; flow = flow->next_flow) {
						if (flow->ILP_is_in_wcet_path) {
							node2 = flow->dst_cb;
							Set temp = Set_new();
							visit(node2, node2, node2, temp, header, 0);
							Set_dispose(temp);
						}
					}
					if (Set_size(blocks) > 1) {

						LB_TraceRegion * tr = My_simple_trace_formation(
								header->fn, header, L_TRACEREGION_HAMMOCK,
								first_node, last_node, blocks,
								header->next_id++);
						tr->wcet2 = wcet;
						header->traceregions = List_insert_last(
								header->traceregions, tr);
					}
				}

			} else {
				if (Set_size(blocks) > 1) {
					LB_TraceRegion * tr = My_simple_trace_formation(header->fn,
							header, L_TRACEREGION_HAMMOCK, first_node,
							last_node, blocks, header->next_id++);
					tr->wcet2 = wcet;
					header->traceregions = List_insert_last(
							header->traceregions, tr);
				}

			}

		} else if (Set_in(headers, node->id) && Set_in(backedge, node->id)) {
			L_Cb * node2;
			L_Flow * flow;
			//fprintf(stderr,"QQQQQQQQQQQQ\n");
			for (flow = node->dest_flow; flow; flow = flow->next_flow) {
				if (flow->ILP_is_in_wcet_path && flow->dst_cb != node) {
					node2 = flow->dst_cb;
					Set temp = Set_new();
					visit(node2, node2, node2, temp, header, 0);
					Set_dispose(temp);

				}

			}

			if (Set_size(blocks) > 1) {

				LB_TraceRegion * tr = My_simple_trace_formation(header->fn,
						header, L_TRACEREGION_HAMMOCK, first_node, last_node,
						blocks, header->next_id++);
				tr->wcet2 = wcet;
				header->traceregions = List_insert_last(header->traceregions,
						tr);
			}

		} else if (!Set_in(headers, node->id) && Set_in(backedge, node->id)) {
			L_Cb * node2;
			L_Flow * flow;

			for (flow = node->dest_flow; flow; flow = flow->next_flow) {
				node2 = flow->dst_cb;
				if (!Set_in(node->dom, node2->id)
						&& flow->ILP_is_in_wcet_path) {
					Set temp = Set_new();
					//Set_add(temp,s->id);
					visit(node2, node2, node2, temp, header, 0);
					Set_dispose(temp);
				}
			}
			if (Set_size(blocks) > 1) {
				LB_TraceRegion * tr = My_simple_trace_formation(header->fn,
						header, L_TRACEREGION_HAMMOCK, first_node, last_node,
						blocks, header->next_id++);
				tr->wcet2 = wcet;
				//LB_summarize_tr (stderr, tr);
				header->traceregions = List_insert_last(header->traceregions,
						tr);

			}
		} else if (Set_in(headers, node->id) && !Set_in(backedge, node->id)) {
			L_Cb * node2;
			L_Flow * flow;
			for (flow = node->dest_flow; flow; flow = flow->next_flow) {
				if (flow->ILP_is_in_wcet_path) {
					node2 = flow->dst_cb;
					//	fprintf(stderr,"CP(%d)\n",node2->id);

					Set temp = Set_new();

					char nb[100];
					L_cb_flags_to_string(nb, node->flags);
					//fprintf(stderr,"CB(%d):%s\n",node->id,nb);

					if (!L_EXTRACT_BIT_VAL(node->flags,
							L_CB_SUPERBLOCK) && !L_EXTRACT_BIT_VAL(node->flags, L_CB_HYPERBLOCK)) {
						Set_add(temp, node->id);
						visit(node2, node, node, temp, header,
								wcet + flow->wcet_weight);
					} else {
						visit(node2, node2, node2, temp, header, 0);

					}
					Set_dispose(temp);
				}
			}
			if (Set_size(blocks) > 1) {
				//fprintf(stderr,"AAAAAAAAAAAAAAAAAAAAAAAAAAAA\n");
				LB_TraceRegion * tr = My_simple_trace_formation(header->fn,
						header, L_TRACEREGION_HAMMOCK, first_node, last_node,
						blocks, header->next_id++);
				//LB_summarize_tr (stderr, tr);
				tr->wcet2 = wcet;
				header->traceregions = List_insert_last(header->traceregions,
						tr);
			}

		}
	}
}

double find_max_weight_of_paths(L_Cb * first, L_Cb * last) {
	if (first == last)
		return 0;

	L_Flow * flow;
	double max = -100, temp_max;
	if (!Set_in(headers, first->id) && !Set_in(backedge, first->id)) {
		for (flow = first->dest_flow; flow; flow = flow->next_flow) {
			//fprintf(stderr, "PATH(%d,%d)\n",flow->dst_cb->id,last->id);
			temp_max = find_max_weight_of_paths(flow->dst_cb, last)
					+ flow->wcet_weight;
			if (temp_max > max)
				max = temp_max;

		}

	} else if (Set_in(headers, first->id) && Set_in(backedge, first->id)) {
		L_Cb * node = first->back_edge_node;

		for (flow = node->dest_flow; flow; flow = flow->next_flow) {
			if (flow->dst_cb != node) {
				//	fprintf(stderr, "PATH(%d,%d)\n",flow->dst_cb->id,last->id);
				temp_max = find_max_weight_of_paths(flow->dst_cb, last)
						+ flow->wcet_weight;
				if (temp_max > max)
					max = temp_max;
			}
		}
		max += first->wcet_loop;

	} else if (Set_in(headers, first->id) && !Set_in(backedge, first->id)) {
		L_Cb * node = first->back_edge_node;
		//fprintf(stderr,"BACK(%d)\n",node->id);

		for (flow = node->dest_flow; flow; flow = flow->next_flow) {
			if (flow->dst_cb != first) {
				//fprintf(stderr, "PATH(%d,%d)\n",flow->dst_cb->id,last->id);
				temp_max = find_max_weight_of_paths(flow->dst_cb, last)
						+ flow->wcet_weight;
				if (temp_max > max)
					max = temp_max;
			}
		}
		max += first->wcet_loop;

	}

	return max;

}

void my_trace_formation2_2(L_Func * fn, LB_TraceRegion_Header *header) {
	Graph bb_g = L_create_cb_graph(fn);
	Graph_dominator(bb_g);
	Graph_post_dominator(bb_g);
	Graph_imm_dominator(bb_g);
	Graph_imm_post_dominator(bb_g);
	Graph_control_dependence(bb_g);
	Graph_topological_sort(bb_g);
	Graph_preorder_dfs_sort(bb_g, 1);
	Graph_preorder_dfs_visit(bb_g, bb_g->root, 1);

	backedge = find_set_backedges(fn);
	headers = find_set_headers(fn);
	List_start(bb_g->topo_list);
	List_next(bb_g->topo_list);

	L_Cb * cb;
	GraphNode nodeg;
	L_Cb * node;
	L_Flow * flow;

	List_start(bb_g->nodes);

	while ((nodeg = (GraphNode) List_next(bb_g->nodes))) {
		//fprintf(stderr, "FF(%d)\n",nodeg->id);
		GraphNode tem = get_imm_post_domin(bb_g->topo_list, nodeg);
		if (tem != NULL)
			((L_Cb *) (nodeg->ptr))->chock_end_cb = (L_Cb *) (tem->ptr);

		//fprintf(stderr,"CHOCK(%d,%d)",node->id,node->chock_end_cb->id);

	}

#ifdef SHARIF
	for (node = fn->first_cb; node; node = node->next_cb) {
		double max = -100;
		fprintf(stderr, "MORTY(%d)\n", node->id);
		for (flow = node->dest_flow; flow; flow = flow->next_flow) {

			if (!Set_in(node->dom, flow->dst_cb->id)) {
				fprintf(stderr, "FLOW(%d)\n", flow->dst_cb->id);
				flow->wcet_until_chock = find_max_weight_of_paths(flow->dst_cb, node->chock_end_cb) + flow->wcet_weight;
				if (flow->wcet_until_chock > max)
				max = flow->wcet_until_chock;
			}

		}
		node->wcet_until_chock = max;
	}
#endif
	char nb[100];
	Set blocks = Set_new();

	List_start(bb_g->topo_list);
	List_next(bb_g->topo_list);
	while (nodeg = (GraphNode) List_next(bb_g->topo_list)) {
		//fprintf(stderr, "FG(%d)\n",nodeg->id);
		node = (L_Cb *) (nodeg->ptr);
		L_cb_flags_to_string(nb, node->flags);
		fprintf(stderr, "CR(%d):%s\n", node->id, nb);
		if (L_EXTRACT_BIT_VAL(node->flags, L_CB_SUPERBLOCK))
			continue;
		else
			break;

	}

	fprintf(stderr, "UUUUUU\n");
	//Set abc=Set_new();
	visit_2(node, node, node, Set_new(), header, 0);
	//Set_dispose(abc);
	fprintf(stderr, "OPOPOP\n");

	Graph_free_graph(bb_g);

}

void my_trace_formation2(L_Func * fn, LB_TraceRegion_Header *header) {
	Graph bb_g = L_create_cb_graph(fn);
	Graph_dominator(bb_g);
	Graph_post_dominator(bb_g);
	Graph_imm_dominator(bb_g);
	Graph_imm_post_dominator(bb_g);
	Graph_control_dependence(bb_g);
	Graph_topological_sort(bb_g);
	Graph_preorder_dfs_sort(bb_g, 1);
	Graph_preorder_dfs_visit(bb_g, bb_g->root, 1);

	backedge = find_set_backedges(fn);
	headers = find_set_headers(fn);
	List_start(bb_g->topo_list);
	List_next(bb_g->topo_list);

	L_Cb * cb;
	GraphNode nodeg;
	L_Cb * node;
	L_Flow * flow;

	List_start(bb_g->nodes);

	while ((nodeg = (GraphNode) List_next(bb_g->nodes))) {
		//fprintf(stderr, "FF(%d)\n",nodeg->id);
		GraphNode tem = get_imm_post_domin(bb_g->topo_list, nodeg);
		if (tem != NULL)
			((L_Cb *) (nodeg->ptr))->chock_end_cb = (L_Cb *) (tem->ptr);

		//fprintf(stderr,"CHOCK(%d,%d)",node->id,node->chock_end_cb->id);

	}

#ifdef SHARIF
	for (node = fn->first_cb; node; node = node->next_cb) {
		double max = -100;
		fprintf(stderr, "MORTY(%d)\n", node->id);
		for (flow = node->dest_flow; flow; flow = flow->next_flow) {

			if (!Set_in(node->dom, flow->dst_cb->id)) {
				fprintf(stderr, "FLOW(%d)\n", flow->dst_cb->id);
				flow->wcet_until_chock = find_max_weight_of_paths(flow->dst_cb, node->chock_end_cb) + flow->wcet_weight;
				if (flow->wcet_until_chock > max)
				max = flow->wcet_until_chock;
			}

		}
		node->wcet_until_chock = max;
	}
#endif
	char nb[100];
	Set blocks = Set_new();

	List_start(bb_g->topo_list);
	List_next(bb_g->topo_list);
	while (nodeg = (GraphNode) List_next(bb_g->topo_list)) {
		//fprintf(stderr, "FG(%d)\n",nodeg->id);
		node = (L_Cb *) (nodeg->ptr);
		L_cb_flags_to_string(nb, node->flags);
		fprintf(stderr, "CR(%d):%s\n", node->id, nb);
		if (L_EXTRACT_BIT_VAL(node->flags, L_CB_SUPERBLOCK))
			continue;
		else
			break;

	}

	visit(node, node, node, Set_new(), header, 0);
	//fprintf(stderr,"OPOPOP\n");

	Graph_free_graph(bb_g);

}

int my_select_trace(L_Func * fn, LB_TraceRegion_Header *header) {

	List_start(header->traceregions);
	LB_TraceRegion * tr;
	LB_TraceRegion * max_tr;

	List temp;
	max_tr = List_get_first(header->traceregions);
	while (tr = (LB_TraceRegion *) List_next(header->traceregions)) {
		if (tr->weight > max_tr->weight)
			max_tr = tr;
	}

	List_start(header->traceregions);

	while ((tr = (LB_TraceRegion *) List_next(header->traceregions))) {
		if (max_tr != tr)
			LB_free_traceregion(tr);
	}
	header->traceregions = List_reset(header->traceregions);
	header->inorder_trs = List_reset(header->inorder_trs);
	if (max_tr != NULL)
		header->traceregions = List_insert_last(header->traceregions, max_tr);

	if (max_tr != NULL)
		return max_tr->id;
	else
		return 0;

}

double find_wcet_path(L_Cb * start, L_Cb * end) {

	L_Flow * flow;
	double max = -100;
	double temp;
	int zarib = 0;
	L_Cb * temp_start = start;
	if (Set_in(headers, start->id)) {
		temp_start = start->back_edge_node;
		zarib = 1;
	}

	if (temp_start == end) {
		//fprintf(stderr,"DDD(%f)\n",zarib*start->wcet_loop*start->max_iteration);
		return end->wcet + zarib * temp_start->wcet_loop;
	}
	for (flow = start->dest_flow; flow; flow = flow->next_flow) {

		//fprintf(stderr,"NODE_2(%d)\n",flow->dst_cb->id);

		if (!Set_in(temp_start->dom, flow->dst_cb->id)) {
			//fprintf(stderr,"NODE_2(%d)\n",flow->dst_cb->id);
			temp = find_wcet_path(flow->dst_cb, end) + flow->wcet_weight
					+ zarib * temp_start->wcet_loop;

			if (max < temp)
				max = temp;
		}

	}

	return max;
}

int is_balance(L_Cb * node) {
	L_Flow * flow;

	double max = node->wcet_until_chock;

	for (flow = node->dest_flow; flow; flow = flow->next_flow) {
		if (max > flow->wcet_until_chock + 1)
			return 0;
	}

	return 1;

}

void find_loops_wcet(L_Func * fn) {
	L_Loop * loop;
	L_Loop * nest_loop;

	int max_level = 1;
	for (loop = fn->first_loop; loop; loop = loop->next_loop)
		if (max_level < loop->nesting_level)
			max_level = loop->nesting_level;

	fprintf(stderr, "RESID1\n");

	int i;
	for (i = max_level; i > 0; i--) {
		for (loop = fn->first_loop; loop; loop = loop->next_loop) {

			if (loop->nesting_level == i) {
				L_Cb * end;
				int buf[5];
				Set_2array(loop->back_edge_cb, buf);
				end = L_cb_hash_tbl_find(fn->cb_hash_tbl, buf[0]);
				fprintf(stderr, "LOOP(%d,%d)\n", loop->header->id, end->id);
				loop->header->back_edge_node = end;
				if (loop->header == end)
					loop->header->wcet_loop = end->wcet
							* loop->header->max_iteration;
				else {
					L_Flow * flow;
					double max = -100, temp_max;
					for (flow = loop->header->dest_flow; flow;
							flow = flow->next_flow) {

						if (Set_in(loop->loop_cb, flow->dst_cb)) {
							fprintf(stderr, "PATH(%d,%d)\n", flow->dst_cb->id,
									end->id);
							temp_max = find_max_weight_of_paths(flow->dst_cb,
									end) + flow->wcet_weight;
							if (temp_max > max)
								max = temp_max;
						}
					}

					loop->header->wcet_loop = (max + end->wcet)
							* loop->header->max_iteration;

				}
				//				L_Cb * end;
				//				double max_wcet=-100,temp;
				//				if(!Set_in(backedge,loop->header->id)){
				//					int buf[5];
				//					Set_2array(loop->back_edge_cb,buf);
				//
				//						end=L_cb_hash_tbl_find(fn->cb_hash_tbl,buf[0]);
				//
				//						loop->header->back_edge_node=end;
				//						L_Flow * flow;
				//
				//						for(flow=loop->header->dest_flow;flow;flow=flow->next_flow){
				//
				//							if(Set_in(flow->dst_cb->pdom,end->id)){
				//							//	fprintf(stderr,"BACK(%d)\n",loop->header->back_edge_node->id);
				//							//	fprintf(stderr,"NODE(%d)\n",flow->dst_cb->id);
				//
				//								temp=find_wcet_path(flow->dst_cb,end)+flow->wcet_weight;
				//
				//								if(temp>max_wcet) max_wcet=temp;
				//							}
				//
				//						}
				//
				//				}
				//				else{
				//					max_wcet=loop->header->max_iteration* loop->header->wcet+ loop->header->dest_flow->wcet_weight;
				//					loop->header->back_edge_node=loop->header;
				//				}
				//				loop->header->wcet_loop=max_wcet;
				//				fprintf(stderr, "WCET(%f)\n",loop->header->wcet_loop);

			}
		}
	}

}

void my_select_trace_2(L_Func * fn, LB_TraceRegion_Header *header) {

	fprintf(stderr, "SALLLLLAM\n");
	List_start(header->traceregions);
	LB_TraceRegion * tr;
	LB_TraceRegion * max_tr;

	List temp = List_copy(header->traceregions);
	List_start(temp);

	while ((tr = (LB_TraceRegion *) List_next(temp))) {
		if (tr != NULL) {
			fprintf(stderr, "GH\n");
			LB_summarize_tr(stderr, tr);
			fprintf(stderr, "TY(%d)\n", tr->header->chock_end_cb->id);
			if ((tr->wcet > tr->wcet2) || (is_balance(tr->header))) {
				List_remove(header->traceregions, tr);
				List_remove(header->inorder_trs, tr);
				LB_free_traceregion(tr);
				fprintf(stderr, "TY2(%d)\n", tr->header->chock_end_cb->id);
				if (List_size(header->traceregions) == 0)
					break;
				fprintf(stderr, "GH1\n");

			}
		}
	}
	List_free_all_ptrs(temp);

	fprintf(stderr, "SALLLLLAM2\n");

}

int find_chock(L_Cb * start, L_Cb* end, L_Cb * current, Set blocks,
		LB_TraceRegion_Header * header) {
	L_Flow * flow;
	fprintf(stderr, "(%d)----(%d)---(%d)\n", start->id, current->id, end->id);
	if (!Set_in(current->dom, start->id)   // || !Set_in(current->pdom, end->id)
	|| (Set_in(headers, current->id) && current != start)
			|| (Set_in(backedge, current->id) && current != end)
			|| (Set_in(backedge, current->id) && Set_in(headers, current->id)))
		return 0;
	Set_add(blocks, current->id);
	//fprintf(stderr,"**********\n");

	if (end == current)
		return 1;

	for (flow = current->dest_flow; flow; flow = flow->next_flow) {
		if (flow->dst_cb == current)
			return 0;
		if (!find_chock(start, end, flow->dst_cb, blocks, header))
			return 0;
	}

	if (current == start) {
		LB_TraceRegion * tr = My_simple_trace_formation(header->fn, header,
		L_TRACEREGION_HAMMOCK, start, end, blocks, header->next_id++);
		//LB_summarize_tr (stderr, tr);
		//tr->wcet2=end->wcet;
		//tr = LB_compute_traceregion_info (tr);
		header->traceregions = List_insert_last(header->traceregions, tr);
		Set_dispose(blocks);
	}

	return 1;

}

void my_trace_formation3(L_Func * fn, LB_TraceRegion_Header *header) {

	backedge = find_set_backedges(fn);
	headers = find_set_headers(fn);

	L_Cb * cb;
	for (cb = fn->first_cb; cb; cb = cb->next_cb) {
		int n = Set_size(cb->pdom);
		int * buf = (int *) Lcode_malloc(sizeof(int) * n);
		Set_2array(cb->pdom, buf);
		int i;

		for (i = 0; i < n; i++) {
			L_Cb * cb2 = L_cb_hash_tbl_find(fn->cb_hash_tbl, buf[i]);
			if (cb->id != 1 && cb2->dest_flow && cb != cb2) {
				fprintf(stderr, "FROM (%d)->(%d)\n", cb->id, cb2->id);
				find_chock(cb, cb2, cb, Set_new(), header);
			}

		}

		Lcode_free(buf);
	}

}

typedef struct {
	Set nodes;
	double wcet;

} My_path;

double my_find_special_chocks(L_Cb * start, L_Cb* end, L_Cb * current,
		Set blocks, LB_TraceRegion_Header * header, double wcet) {
	L_Flow * flow;
	double total_wcet = -10000;
	//fprintf(stderr,"(%d)   (%d)---(%d)\n",current->id,start->id,end->id);
	if (!Set_in(current->dom, start->id) || !Set_in(current->pdom, end->id)
			|| (Set_in(headers, current->id) && current != start)
			|| (Set_in(backedge, current->id) && current != end)
			|| (Set_in(backedge, current->id) && Set_in(headers, current->id)))
		return -1000;
	Set_add(blocks, current->id);
	//fprintf(stderr,"**********\n");

	if (end == current)
		return wcet + end->wcet;

	for (flow = current->dest_flow; flow; flow = flow->next_flow) {
		if (flow->dst_cb == current)
			return -1000;
		double new_wcet = my_find_special_chocks(start, end, flow->dst_cb,
				blocks, header, wcet + flow->wcet_weight);
		if (new_wcet < 0)
			return -1000;
		if (total_wcet < new_wcet)
			total_wcet = new_wcet;
	}

	if (current == start) {
		LB_TraceRegion * tr = My_simple_trace_formation(header->fn, header,
		L_TRACEREGION_HAMMOCK, start, end, blocks, header->next_id++);
		//LB_summarize_tr (stderr, tr);
		tr->wcet2 = total_wcet;
		if (tr->wcet2 + 1 >= tr->wcet) {
			//fprintf(stderr,"K(%d)\n",tr->id);
			header->traceregions = List_insert_last(header->traceregions, tr);
			Set_dispose(blocks);
		} else {
			fprintf(stderr, "this is a bad trace:\n");
			LB_summarize_tr(stderr, tr);
			LB_free_traceregion(tr);

		}
	}

	return total_wcet;

}

void my_pre_path_hb_formation(L_Func * fn, LB_TraceRegion_Header *header) {

	backedge = find_set_backedges(fn);
	headers = find_set_headers(fn);

	L_Cb * cb;
	for (cb = fn->first_cb; cb; cb = cb->next_cb) {
		int n = Set_size(cb->pdom);
		int * buf = (int *) Lcode_malloc(sizeof(int) * n);
		Set_2array(cb->pdom, buf);
		int i;

		for (i = 0; i < n; i++) {
			L_Cb * cb2 = L_cb_hash_tbl_find(fn->cb_hash_tbl, buf[i]);
			if (cb->id != 1 && cb2->dest_flow && cb != cb2) {
				//fprintf(stderr,"NNNNNNNNNNNNN (%d)->(%d)\n",cb->id,cb2->id);
				my_find_special_chocks(cb, cb2, cb, Set_new(), header, 0);
			}

		}

		Lcode_free(buf);
	}

}

//void find_kind_of_chock(L_Cb * start,L_Cb * end)
//{
//
//
//}

//
//void my_pre_path_chock_selection(L_Func * fn, LB_TraceRegion_Header *header) {
//
//	backedge = find_set_backedges(fn);
//	headers = find_set_headers(fn);
//
//	Graph bb_g = L_create_cb_graph(fn);
//	Graph_dominator(bb_g);
//	Graph_post_dominator(bb_g);
//	Graph_imm_dominator(bb_g);
//	Graph_imm_post_dominator(bb_g);
//	Graph_control_dependence(bb_g);
//	Graph_topological_sort(bb_g);
//	Graph_preorder_dfs_sort(bb_g, 1);
//	Graph_preorder_dfs_visit(bb_g, bb_g->root, 1);
//
//	backedge = find_set_backedges(fn);
//	headers = find_set_headers(fn);
//	List_start(bb_g->topo_list);
//	List_next(bb_g->topo_list);
//
//	L_Cb * cb;
//	GraphNode nodeg;
//    L_Cb * node;
//
//	List_start(bb_g->nodes);
//
//	while ((nodeg = (GraphNode) List_next(bb_g->nodes)))
//	{
//		//fprintf(stderr, "FF(%d)\n",nodeg->id);
//		GraphNode tem=get_imm_post_domin(bb_g->topo_list,nodeg);
//		if(tem!=NULL)
//		((L_Cb *)(nodeg->ptr))->chock_end_cb=(L_Cb *)(tem->ptr);
//
//
//	}
//
//	L_Cb * cb;
//	L_Cb * cb2;
//
//	for (cb = fn->first_cb; cb; cb = cb->next_cb) {
//
//			if (cb->id != 1 && cb2->dest_flow && cb != cb2) {
//				cb2=cb->chock_end_cb;
//			//fprintf(stderr,"NNNNNNNNNNNNN (%d)->(%d)\n",cb->id,cb2->id);
//			find_kind_of_chock(cb,cb2);
//
//
//		}
//
//		Lcode_free(buf);
//	}
//
//}

static int LB_mark_ncycle_regions(LB_TraceRegion_Header *header) {
	LB_TraceRegion *tr;
	int found = 0;

	List_start(header->traceregions);
	while ((tr = List_next(header->traceregions))) {
		Set tr_set = LB_return_cbs_region_as_set(tr);
		if (LB_hb_region_contains_cycle(tr_set, tr->header)) {
			tr->flags |= L_TRACEREGION_FLAG_NESTED_CYCLE;
			found++;
		}
		Set_dispose(tr_set);
	}

	return found;
}

//for wcet path selection
#ifdef  SELECT_PATH

void LB_hyperblock_formation_WCET(L_Func * fn) {

	//fprintf(stderr,"QQQQQQQQQQQQQQQQQQQQQQQ\nwwwwwwwwwwwwwwwwwwwwwwwww\nDDDDDDDDDDDDDDDDDDDDDDDDD\n");

	//if (strcmp(fn->name, "_main") == 0) {
	LB_TraceRegion_Header *header;
	//LB_TraceRegion_Header *temp_header;

	LB_TraceRegion *tr;
	L_Cb *cb;
	L_Oper *op;
	int do_peel, final_ops;
	if (fn->n_cb == 0)
	return;
	int i;

	memset(&LB_hb_stat, 0, sizeof (LB_HB_Stat));

	LB_split_exit_block(fn);

	header = LB_function_init(fn);

	//temp_header = LB_function_init(fn);

	L_breakup_pre_post_inc_ops(fn);

	LB_clr_hyperblock_flag(fn);

	L_compute_oper_weight(fn, 0, 1);
	/* make sure all cbs contain at most 2 targets */

	LB_convert_to_strict_basic_block_code(fn, L_CB_SUPERBLOCK |
			L_CB_HYPERBLOCK |
			L_CB_ENTRANCE_BOUNDARY |
			L_CB_EXIT_BOUNDARY);

	for (cb = fn->first_cb; cb; cb = cb->next_cb) {
		L_Oper *next_op;
		for (op = cb->first_op; op; op = next_op) {
			next_op = op->next_op;

			if (op->opc == Lop_NO_OP)
			L_delete_oper(cb, op);
			else
			LB_hb_stat.orig_ops++;
		}
	}

	L_do_flow_analysis(fn, DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE);

	L_loop_detection(fn, 0);

	if (LB_hb_verbose_level >= 7) {
		fprintf(stderr, "Initial Loop Detection Phase for (fn %s):\n", fn->name);
		L_print_loop_data(fn);
	}

	LB_elim_all_loop_backedges(fn);

	L_delete_unreachable_blocks(fn);

	L_do_flow_analysis(fn, DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE);
	L_reset_loop_headers(fn);
	L_loop_detection(fn, 0);

	if (LB_hb_verbose_level >= 7) {
		fprintf(stderr, "Final Loop Detection Phase for (fn %s):\n", fn->name);
		L_print_loop_data(fn);
	}

	L_compute_oper_weight(fn, 0, 1);
	LB_mark_jrg_flag(fn);

	//**********************************************************************

	L_partial_dead_code_removal(fn);
	L_do_flow_analysis(fn,
			DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE | SUPPRESS_PG);

	//fprintf(stderr,"Resid 0\n");
	{
		ILP_cfg_wcet_analyize(fn, "(0).txt");
		Ldot_display_cfg(fn, "t0.dot", 0);
		FILE * file = fopen("rrr.txt", "w");
		L_print_func(file, fn);
		fclose(file);
	}

	for (i = 1; i < 100; i++) {

		//	L_do_flow_analysis(fn,
		//				DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE | SUPPRESS_PG);
		//L_partial_dead_code_removal(fn);
		L_do_flow_analysis(fn,
				DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE | SUPPRESS_PG);
		L_reset_loop_headers(fn);
		L_loop_detection(fn, 0);

		{
			char tp[100];
			sprintf(tp, "t%d.dot", i);
			ILP_cfg_wcet_analyize(fn, "(2).txt");
			Ldot_display_cfg(fn, tp, 0);

		}
		//fprintf(stderr,"Resid1\n");

		//temp_header = LB_create_tr_header (fn);
		fprintf(stderr, "Round (%d)\n", i);
		//find_loops_wcet(fn);
		my_trace_formation2_2(fn, header);
		fprintf(stderr, "SLLLAM\n");
		LB_summarize_traceregions(stderr, header);

		int tid = my_select_trace(fn, header);

		fprintf(stderr, "Trace (%d) selected.\n", tid);

		if (List_size(header->traceregions) == 0)
		break;

		LB_hb_reset_max_oper_id(fn);

		LB_remove_partially_subsumed_traceregions(header);

		LB_summarize_traceregions(stderr, header);

		LB_remove_conflicting_traceregions(header);

		LB_set_hyperblock_flag(fn);

		LB_set_hyperblock_func_flag(fn);

		//*****************trace selection***************
		//tr=(LB_TraceRegion *)List_get_first(temp_header->traceregions);
		//header->traceregions=List_insert_last(header->traceregions,tr);

		//***********************************************

		LB_tail_duplicate(fn, header, LB_DUP_OUTSIDE_REGION);

		{
			L_check_func(fn);
		}

		LB_predicate_traceregions(fn, header);

		LB_set_hyperblock_flag(fn);
		LB_remove_unnec_hyperblock_flags(fn);
		LB_set_hyperblock_func_flag(fn);

		LB_free_all_traceregions(header);

		//temp_header->traceregions = List_reset (temp_header->traceregions);
		//temp_header->inorder_trs = List_reset (temp_header->inorder_trs);

		LB_remove_empty_cbs(fn);

		L_delete_unreachable_blocks(fn);

		LB_convert_to_strict_basic_block_code(fn, L_CB_HYPERBLOCK | L_CB_SUPERBLOCK);

		{
			DB_spit_func(fn, "PH1");
			L_check_func(fn);
		}

	}

	//**********************************************************************
	fprintf(stderr, "***************\n**************\n***************\n***************\n");

	{
		//L_partial_dead_code_removal(fn);
		L_do_flow_analysis(fn,
				DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE | SUPPRESS_PG);
		ILP_cfg_wcet_analyize(fn, "(99).txt");
		Ldot_display_cfg(fn, "t99.dot", 0);

		// LB_summarize_traceregions (stderr, header);
	}
	L_do_flow_analysis(fn,
			DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE | SUPPRESS_PG);

	//*******************************************************************

	//******************************************************************

	/*
	 *  Split branches into pred defines and predicated jumps
	 */
	if (LB_hb_branch_split)
	LB_branch_split_func(fn);

	/*
	 *        Generate multiple defn pred defines, generate Uncond pred defines
	 *  (Note initially only OR-type pred defines are created!
	 */

	L_create_uncond_pred_defines(fn);

	PG_setup_pred_graph(fn);
	if (LB_do_lightweight_pred_opti)
	L_lightweight_pred_opti(fn);
	L_combine_pred_defines(fn);

	/*
	 *        remove unnecessary uncond jumps
	 */

	LB_uncond_branch_elim(fn);

	/*
	 *        Merge ops on opposite predicates (partial redundancy elim)
	 */

	L_unmark_all_pre_post_increments(fn);

	if (LB_hb_do_pred_merge) {
		PG_setup_pred_graph(fn);
		L_do_flow_analysis(fn, LIVE_VARIABLE | REACHING_DEFINITION);
		LB_hb_pred_merging(fn);
	}

#ifdef DEBUG_HB_FORMER
	L_check_func(fn);
#endif

	PG_destroy_pred_graph();
	D_delete_dataflow(fn);

	/* For each source code function we process deinit */
	LB_function_deinit(header);

	{
		double rat = 1.0;
		L_Attr *eattr;

		final_ops = 0;

		for (cb = fn->first_cb; cb; cb = cb->next_cb) {
			L_Oper *next_op;
			for (op = cb->first_op; op; op = next_op) {
				next_op = op->next_op;

				if (op->opc != Lop_NO_OP)
				final_ops++;
			}
		}

		if (LB_hb_stat.orig_ops)
		rat = (double) final_ops / LB_hb_stat.orig_ops;

		eattr = L_new_attr("hbe", 2);
		L_set_int_attr_field(eattr, 0, LB_hb_stat.orig_ops);
		L_set_double_attr_field(eattr, 1, rat);
		fn->attr = L_concat_attr(fn->attr, eattr);
	}

	int num_oper = 0;
	for (cb = fn->first_cb; cb; cb = cb->next_cb) {
		L_Oper *next_op;
		for (op = cb->first_op; op; op = next_op) {
			next_op = op->next_op;

			if (op->opc != Lop_NO_OP)

			num_oper++;
		}

	}
	fprintf(stderr, "NUMBER OF OPERATION:(%d)\n", num_oper);

	//}

	return;
}

#endif

#ifdef FULL_SELECT_PATH

void LB_hyperblock_formation(L_Func * fn) {
	LB_TraceRegion_Header *header;
	LB_TraceRegion *tr;
	L_Cb *cb;
	L_Oper *op;
	int do_peel, final_ops;
	if (fn->n_cb == 0)
	return;
	int i;

	memset(&LB_hb_stat, 0, sizeof (LB_HB_Stat));

	LB_split_exit_block(fn);

	header = LB_function_init(fn);

	L_breakup_pre_post_inc_ops(fn);

	LB_clr_hyperblock_flag(fn);

	L_compute_oper_weight(fn, 0, 1);
	/* make sure all cbs contain at most 2 targets */

	LB_convert_to_strict_basic_block_code(fn, L_CB_SUPERBLOCK |
			L_CB_HYPERBLOCK |
			L_CB_ENTRANCE_BOUNDARY |
			L_CB_EXIT_BOUNDARY);

	for (cb = fn->first_cb; cb; cb = cb->next_cb) {
		L_Oper *next_op;
		for (op = cb->first_op; op; op = next_op) {
			next_op = op->next_op;

			if (op->opc == Lop_NO_OP)
			L_delete_oper(cb, op);
			else
			LB_hb_stat.orig_ops++;
		}
	}

	L_do_flow_analysis(fn, DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE);

	L_loop_detection(fn, 0);

	if (LB_hb_verbose_level >= 7) {
		fprintf(stderr, "Initial Loop Detection Phase for (fn %s):\n", fn->name);
		L_print_loop_data(fn);
	}

	LB_elim_all_loop_backedges(fn);

	L_delete_unreachable_blocks(fn);

	L_do_flow_analysis(fn, DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE);
	L_reset_loop_headers(fn);
	L_loop_detection(fn, 0);

	if (LB_hb_verbose_level >= 7) {
		fprintf(stderr, "Final Loop Detection Phase for (fn %s):\n", fn->name);
		L_print_loop_data(fn);
	}

	L_compute_oper_weight(fn, 0, 1);
	LB_mark_jrg_flag(fn);

	//**********************************************************************

	L_partial_dead_code_removal(fn);
	L_do_flow_analysis(fn,
			DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE | SUPPRESS_PG);

	if (strcmp(fn->name, "_main") == 0) {

		{
			ILP_cfg_wcet_analyize(fn, "(0).txt");
			Ldot_display_cfg(fn, "t0.dot", 0);
		}

		for (i = 1; i < 2; i++) {
			L_partial_dead_code_removal(fn);
			L_do_flow_analysis(fn,
					DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE | SUPPRESS_PG);

			{
				char tp[100];
				sprintf(tp, "t%d.dot", i);
				ILP_cfg_wcet_analyize(fn, "(2).txt");
				Ldot_display_cfg(fn, tp, 0);
				LB_summarize_traceregions(stderr, header);
			}

			my_trace_formation2(fn, header);

			if (List_size(header->traceregions) == 0)
			break;

			LB_hb_reset_max_oper_id(fn);

			LB_remove_partially_subsumed_traceregions(header);

			LB_summarize_traceregions(stderr, header);

			LB_remove_conflicting_traceregions(header);

			LB_set_hyperblock_flag(fn);

			LB_set_hyperblock_func_flag(fn);

			LB_tail_duplicate(fn, header, LB_DUP_OUTSIDE_REGION);

			{
				L_check_func(fn);
			}

			//Ldot_display_cfg(fn,"GGGGG.dot",0);

			LB_predicate_traceregions(fn, header);
			//fprintf(stderr,"Resid2\n");
			/*  Mark all cbs with any type of hyperblock flag with L_CB_HYPERBLOCK */

			//Ldot_display_cfg(fn,"MMMMM.dot",0);
			LB_set_hyperblock_flag(fn);
			LB_remove_unnec_hyperblock_flags(fn);
			LB_set_hyperblock_func_flag(fn);

			LB_free_all_traceregions(header);

			LB_remove_empty_cbs(fn);

			L_delete_unreachable_blocks(fn);

			LB_convert_to_strict_basic_block_code(fn, L_CB_HYPERBLOCK | L_CB_SUPERBLOCK);

			{
				DB_spit_func(fn, "PH1");
				L_check_func(fn);
			}

		}

		//**********************************************************************
		fprintf(stderr, "***************\n**************\n***************\n***************\n");
		fprintf(stderr, "QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ(%s)\n", fn->name);
		{
			L_partial_dead_code_removal(fn);
			L_do_flow_analysis(fn,
					DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE | SUPPRESS_PG);
			ILP_cfg_wcet_analyize(fn, "(99).txt");
			Ldot_display_cfg(fn, "t99.dot", 0);

			// LB_summarize_traceregions (stderr, header);
		}

		/*
		 *  Split branches into pred defines and predicated jumps
		 */
		if (LB_hb_branch_split)
		LB_branch_split_func(fn);

		/*
		 *        Generate multiple defn pred defines, generate Uncond pred defines
		 *  (Note initially only OR-type pred defines are created!
		 */

		L_create_uncond_pred_defines(fn);

		PG_setup_pred_graph(fn);
		if (LB_do_lightweight_pred_opti)
		L_lightweight_pred_opti(fn);
		L_combine_pred_defines(fn);

		/*
		 *        remove unnecessary uncond jumps
		 */

		LB_uncond_branch_elim(fn);

		/*
		 *        Merge ops on opposite predicates (partial redundancy elim)
		 */

		L_unmark_all_pre_post_increments(fn);

		if (LB_hb_do_pred_merge) {
			PG_setup_pred_graph(fn);
			L_do_flow_analysis(fn, LIVE_VARIABLE | REACHING_DEFINITION);
			LB_hb_pred_merging(fn);
		}

#ifdef DEBUG_HB_FORMER
		L_check_func(fn);
#endif

		PG_destroy_pred_graph();
		D_delete_dataflow(fn);

		/* For each source code function we process deinit */
		LB_function_deinit(header);

		{
			double rat = 1.0;
			L_Attr *eattr;

			final_ops = 0;

			for (cb = fn->first_cb; cb; cb = cb->next_cb) {
				L_Oper *next_op;
				for (op = cb->first_op; op; op = next_op) {
					next_op = op->next_op;

					if (op->opc != Lop_NO_OP)
					final_ops++;
				}
			}

			if (LB_hb_stat.orig_ops)
			rat = (double) final_ops / LB_hb_stat.orig_ops;

			eattr = L_new_attr("hbe", 2);
			L_set_int_attr_field(eattr, 0, LB_hb_stat.orig_ops);
			L_set_double_attr_field(eattr, 1, rat);
			fn->attr = L_concat_attr(fn->attr, eattr);
		}

	}

	return;
}
#endif

#ifdef SHARIF

void LB_hyperblock_formation(L_Func * fn) {

	//fprintf(stderr,"QQQQQQQQQQQQQQQQQQQQQQQ\nwwwwwwwwwwwwwwwwwwwwwwwww\nDDDDDDDDDDDDDDDDDDDDDDDDD\n");

	if (strcmp(fn->name, "_main") == 0) {
		LB_TraceRegion_Header *header;
		LB_TraceRegion *tr;
		L_Cb *cb;
		L_Oper *op;
		int do_peel, final_ops;
		if (fn->n_cb == 0)
		return;
		int i;

		memset(&LB_hb_stat, 0, sizeof (LB_HB_Stat));

		LB_split_exit_block(fn);

		header = LB_function_init(fn);

		//temp_header = LB_function_init(fn);

		L_breakup_pre_post_inc_ops(fn);

		LB_clr_hyperblock_flag(fn);

		L_compute_oper_weight(fn, 0, 1);
		/* make sure all cbs contain at most 2 targets */

		LB_convert_to_strict_basic_block_code(
				fn,
				L_CB_SUPERBLOCK | L_CB_HYPERBLOCK | L_CB_ENTRANCE_BOUNDARY
				| L_CB_EXIT_BOUNDARY);

		for (cb = fn->first_cb; cb; cb = cb->next_cb) {
			L_Oper *next_op;
			for (op = cb->first_op; op; op = next_op) {
				next_op = op->next_op;

				if (op->opc == Lop_NO_OP)
				L_delete_oper(cb, op);
				else
				LB_hb_stat.orig_ops++;
			}
		}

		L_do_flow_analysis(fn, DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE);

		L_loop_detection(fn, 0);

		if (LB_hb_verbose_level >= 7) {
			fprintf(stderr, "Initial Loop Detection Phase for (fn %s):\n",
					fn->name);
			L_print_loop_data(fn);
		}

		LB_elim_all_loop_backedges(fn);

		L_delete_unreachable_blocks(fn);

		L_do_flow_analysis(fn, DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE);
		L_reset_loop_headers(fn);
		L_loop_detection(fn, 0);

		if (LB_hb_verbose_level >= 7) {
			fprintf(stderr, "Final Loop Detection Phase for (fn %s):\n",
					fn->name);
			L_print_loop_data(fn);
		}

		L_compute_oper_weight(fn, 0, 1);
		LB_mark_jrg_flag(fn);

		//**********************************************************************

		L_partial_dead_code_removal(fn);
		L_do_flow_analysis(fn,
				DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE | SUPPRESS_PG);

		//fprintf(stderr,"Resid 0\n");
		{
			ILP_cfg_wcet_analyize(fn, "(0).txt");
			Ldot_display_cfg(fn, "t0.dot", 0);
			FILE * file = fopen("rrr.txt", "w");
			L_print_func(file, fn);
			fclose(file);
		}

		for (i = 1; i < 100; i++) {

			//	L_do_flow_analysis(fn,
			//				DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE | SUPPRESS_PG);
			//L_partial_dead_code_removal(fn);
			L_do_flow_analysis(fn,
					DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE | SUPPRESS_PG);
			L_reset_loop_headers(fn);
			L_loop_detection(fn, 0);
			backedge = find_set_backedges(fn);
			headers = find_set_headers(fn);

			find_loops_wcet(fn);
			fprintf(stderr, "QQQQQQQQQQQQQQQQQQQQQQQQQ\n");

			{
				char tp[100];

				sprintf(tp, "(%d).txt", i);
				ILP_cfg_wcet_analyize(fn, tp);

				sprintf(tp, "t%d.dot", i);
				Ldot_display_cfg(fn, tp, 0);

			}
			//fprintf(stderr,"Resid1\n");

			fprintf(stderr, "ROUND (%d)\n", i);

			if (i > 1) {

				my_trace_formation2(fn, header);

				if (header->traceregions == NULL)
				break;
				my_select_trace_2(fn, header);

				if (header->traceregions == NULL)
				break;
				if (header->traceregions->size <= 0)
				break;
				if (header->traceregions->first == NULL) {
					header->traceregions = NULL;
					break;
				}

				LB_summarize_traceregions(stderr, header);
			} else {
				//pre path hb formation
				my_pre_path_hb_formation(fn, header);

			}

			LB_hb_reset_max_oper_id(fn);

			LB_remove_partially_subsumed_traceregions(header);

			LB_remove_conflicting_traceregions(header);

			LB_set_hyperblock_flag(fn);

			LB_set_hyperblock_func_flag(fn);

			//*****************trace selection***************
			//tr=(LB_TraceRegion *)List_get_first(temp_header->traceregions);
			//header->traceregions=List_insert_last(header->traceregions,tr);

			//***********************************************

			LB_summarize_traceregions(stderr, header);

			LB_tail_duplicate(fn, header, LB_DUP_OUTSIDE_REGION);

			{
				L_check_func(fn);
			}

			LB_predicate_traceregions(fn, header);

			LB_set_hyperblock_flag(fn);
			LB_remove_unnec_hyperblock_flags(fn);
			LB_set_hyperblock_func_flag(fn);

			LB_free_all_traceregions(header);

			//temp_header->traceregions = List_reset (temp_header->traceregions);
			//temp_header->inorder_trs = List_reset (temp_header->inorder_trs);

			LB_remove_empty_cbs(fn);

			L_delete_unreachable_blocks(fn);

			LB_convert_to_strict_basic_block_code(fn,
					L_CB_HYPERBLOCK | L_CB_SUPERBLOCK);

			{
				DB_spit_func(fn, "PH1");
				L_check_func(fn);
			}

		}

		//**********************************************************************
		fprintf(stderr,
				"***************\n**************\n***************\n***************\n");

		{
			//L_partial_dead_code_removal(fn);
			L_do_flow_analysis(fn,
					DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE | SUPPRESS_PG);
			ILP_cfg_wcet_analyize(fn, "(99).txt");
			Ldot_display_cfg(fn, "t99.dot", 0);

			// LB_summarize_traceregions (stderr, header);
		}
		L_do_flow_analysis(fn,
				DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE | SUPPRESS_PG);

		//*******************************************************************

		//******************************************************************

		/*
		 *  Split branches into pred defines and predicated jumps
		 */
		if (LB_hb_branch_split)
		LB_branch_split_func(fn);

		/*
		 *        Generate multiple defn pred defines, generate Uncond pred defines
		 *  (Note initially only OR-type pred defines are created!
		 */

		L_create_uncond_pred_defines(fn);

		PG_setup_pred_graph(fn);
		if (LB_do_lightweight_pred_opti)
		L_lightweight_pred_opti(fn);
		L_combine_pred_defines(fn);

		/*
		 *        remove unnecessary uncond jumps
		 */

		LB_uncond_branch_elim(fn);

		/*
		 *        Merge ops on opposite predicates (partial redundancy elim)
		 */

		L_unmark_all_pre_post_increments(fn);

		if (LB_hb_do_pred_merge) {
			PG_setup_pred_graph(fn);
			L_do_flow_analysis(fn, LIVE_VARIABLE | REACHING_DEFINITION);
			LB_hb_pred_merging(fn);
		}

#ifdef DEBUG_HB_FORMER
		L_check_func(fn);
#endif

		PG_destroy_pred_graph();
		D_delete_dataflow(fn);

		/* For each source code function we process deinit */
		LB_function_deinit(header);

		{
			double rat = 1.0;
			L_Attr *eattr;

			final_ops = 0;

			for (cb = fn->first_cb; cb; cb = cb->next_cb) {
				L_Oper *next_op;
				for (op = cb->first_op; op; op = next_op) {
					next_op = op->next_op;

					if (op->opc != Lop_NO_OP)
					final_ops++;
				}
			}

			if (LB_hb_stat.orig_ops)
			rat = (double) final_ops / LB_hb_stat.orig_ops;

			eattr = L_new_attr("hbe", 2);
			L_set_int_attr_field(eattr, 0, LB_hb_stat.orig_ops);
			L_set_double_attr_field(eattr, 1, rat);
			fn->attr = L_concat_attr(fn->attr, eattr);
		}

		int num_oper = 0;
		for (cb = fn->first_cb; cb; cb = cb->next_cb) {
			L_Oper *next_op;
			for (op = cb->first_op; op; op = next_op) {
				next_op = op->next_op;

				if (op->opc != Lop_NO_OP)

				num_oper++;
			}

		}
		fprintf(stderr, "NUMBER OF OPERATION:(%d)\n", num_oper);

	}

	return;
}

#endif

#ifdef SHARIF2

void LB_hyperblock_formation(L_Func * fn) {

	//fprintf(stderr,"QQQQQQQQQQQQQQQQQQQQQQQ\nwwwwwwwwwwwwwwwwwwwwwwwww\nDDDDDDDDDDDDDDDDDDDDDDDDD\n");

	if (strcmp(fn->name, "_main") == 0) {
		LB_TraceRegion_Header *header;
		//LB_TraceRegion_Header *temp_header;

		LB_TraceRegion *tr;
		L_Cb *cb;
		L_Oper *op;
		int do_peel, final_ops;
		if (fn->n_cb == 0)
		return;
		int i;

		memset(&LB_hb_stat, 0, sizeof (LB_HB_Stat));

		LB_split_exit_block(fn);

		header = LB_function_init(fn);

		//temp_header = LB_function_init(fn);

		L_breakup_pre_post_inc_ops(fn);

		LB_clr_hyperblock_flag(fn);

		L_compute_oper_weight(fn, 0, 1);
		/* make sure all cbs contain at most 2 targets */

		LB_convert_to_strict_basic_block_code(
				fn,
				L_CB_SUPERBLOCK | L_CB_HYPERBLOCK | L_CB_ENTRANCE_BOUNDARY
				| L_CB_EXIT_BOUNDARY);

		for (cb = fn->first_cb; cb; cb = cb->next_cb) {
			L_Oper *next_op;
			for (op = cb->first_op; op; op = next_op) {
				next_op = op->next_op;

				if (op->opc == Lop_NO_OP)
				L_delete_oper(cb, op);
				else
				LB_hb_stat.orig_ops++;
			}
		}

		L_do_flow_analysis(fn, DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE);

		L_loop_detection(fn, 0);

		if (LB_hb_verbose_level >= 7) {
			fprintf(stderr, "Initial Loop Detection Phase for (fn %s):\n",
					fn->name);
			L_print_loop_data(fn);
		}

		LB_elim_all_loop_backedges(fn);

		L_delete_unreachable_blocks(fn);

		L_do_flow_analysis(fn, DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE);
		L_reset_loop_headers(fn);
		L_loop_detection(fn, 0);

		if (LB_hb_verbose_level >= 7) {
			fprintf(stderr, "Final Loop Detection Phase for (fn %s):\n",
					fn->name);
			L_print_loop_data(fn);
		}

		L_compute_oper_weight(fn, 0, 1);
		LB_mark_jrg_flag(fn);

		//**********************************************************************

		L_partial_dead_code_removal(fn);
		L_do_flow_analysis(fn,
				DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE | SUPPRESS_PG);

		//fprintf(stderr,"Resid 0\n");
		{
			ILP_cfg_wcet_analyize(fn, "(0).txt");
			Ldot_display_cfg(fn, "t0.dot", 0);
			FILE * file = fopen("rrr.txt", "w");
			L_print_func(file, fn);
			fclose(file);
		}

		for (i = 1; i < 100; i++) {

			//	L_do_flow_analysis(fn,
			//				DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE | SUPPRESS_PG);
			//L_partial_dead_code_removal(fn);
			L_do_flow_analysis(fn,
					DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE | SUPPRESS_PG);
			L_reset_loop_headers(fn);
			L_loop_detection(fn, 0);

			{
				char tp[100];

				sprintf(tp, "(%d).txt", i);
				ILP_cfg_wcet_analyize(fn, tp);

				sprintf(tp, "t%d.dot", i);
				Ldot_display_cfg(fn, tp, 0);

			}
			//fprintf(stderr,"Resid1\n");

			fprintf(stderr, "ROUND (%d)\n", i);

			if (i > 1) {

				my_trace_formation2_1(fn, header);

				//LB_summarize_traceregions(stderr, header);
				if (header->traceregions == NULL)
				break;
				my_select_trace_2(fn, header);
				fprintf(stderr, "TTTTTTTTTTTTTTTTTTTTTTTTTTTTTT\n");

				if (header->traceregions == NULL)
				break;
				if (header->traceregions->size <= 0)
				break;
				if (header->traceregions->first == NULL) {
					header->traceregions = NULL;
					break;
				}

				fprintf(stderr, "UUUUUUU\n");
				LB_summarize_traceregions(stderr, header);
			} else {
				//pre path hb formation
				my_pre_path_chock_selection(fn, header);

			}

			LB_hb_reset_max_oper_id(fn);

			LB_remove_partially_subsumed_traceregions(header);

			LB_remove_conflicting_traceregions(header);

			LB_set_hyperblock_flag(fn);

			LB_set_hyperblock_func_flag(fn);

			//*****************trace selection***************
			//tr=(LB_TraceRegion *)List_get_first(temp_header->traceregions);
			//header->traceregions=List_insert_last(header->traceregions,tr);

			//***********************************************

			LB_summarize_traceregions(stderr, header);

			LB_tail_duplicate(fn, header, LB_DUP_OUTSIDE_REGION);

			{
				L_check_func(fn);
			}

			LB_predicate_traceregions(fn, header);

			LB_set_hyperblock_flag(fn);
			LB_remove_unnec_hyperblock_flags(fn);
			LB_set_hyperblock_func_flag(fn);

			LB_free_all_traceregions(header);

			//temp_header->traceregions = List_reset (temp_header->traceregions);
			//temp_header->inorder_trs = List_reset (temp_header->inorder_trs);

			LB_remove_empty_cbs(fn);

			L_delete_unreachable_blocks(fn);

			LB_convert_to_strict_basic_block_code(fn,
					L_CB_HYPERBLOCK | L_CB_SUPERBLOCK);

			{
				DB_spit_func(fn, "PH1");
				L_check_func(fn);
			}

		}

		//**********************************************************************
		fprintf(stderr,
				"***************\n**************\n***************\n***************\n");

		{
			//L_partial_dead_code_removal(fn);
			L_do_flow_analysis(fn,
					DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE | SUPPRESS_PG);
			ILP_cfg_wcet_analyize(fn, "(99).txt");
			Ldot_display_cfg(fn, "t99.dot", 0);

			// LB_summarize_traceregions (stderr, header);
		}
		L_do_flow_analysis(fn,
				DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE | SUPPRESS_PG);

		//*******************************************************************

		//******************************************************************

		/*
		 *  Split branches into pred defines and predicated jumps
		 */
		if (LB_hb_branch_split)
		LB_branch_split_func(fn);

		/*
		 *        Generate multiple defn pred defines, generate Uncond pred defines
		 *  (Note initially only OR-type pred defines are created!
		 */

		L_create_uncond_pred_defines(fn);

		PG_setup_pred_graph(fn);
		if (LB_do_lightweight_pred_opti)
		L_lightweight_pred_opti(fn);
		L_combine_pred_defines(fn);

		/*
		 *        remove unnecessary uncond jumps
		 */

		LB_uncond_branch_elim(fn);

		/*
		 *        Merge ops on opposite predicates (partial redundancy elim)
		 */

		L_unmark_all_pre_post_increments(fn);

		if (LB_hb_do_pred_merge) {
			PG_setup_pred_graph(fn);
			L_do_flow_analysis(fn, LIVE_VARIABLE | REACHING_DEFINITION);
			LB_hb_pred_merging(fn);
		}

#ifdef DEBUG_HB_FORMER
		L_check_func(fn);
#endif

		PG_destroy_pred_graph();
		D_delete_dataflow(fn);

		/* For each source code function we process deinit */
		LB_function_deinit(header);

		{
			double rat = 1.0;
			L_Attr *eattr;

			final_ops = 0;

			for (cb = fn->first_cb; cb; cb = cb->next_cb) {
				L_Oper *next_op;
				for (op = cb->first_op; op; op = next_op) {
					next_op = op->next_op;

					if (op->opc != Lop_NO_OP)
					final_ops++;
				}
			}

			if (LB_hb_stat.orig_ops)
			rat = (double) final_ops / LB_hb_stat.orig_ops;

			eattr = L_new_attr("hbe", 2);
			L_set_int_attr_field(eattr, 0, LB_hb_stat.orig_ops);
			L_set_double_attr_field(eattr, 1, rat);
			fn->attr = L_concat_attr(fn->attr, eattr);
		}

	}

	return;
}

#endif

#ifdef My_basic_block

void LB_hyperblock_formation(L_Func * fn) {

	if (strcmp(fn->name, "_main") == 0) {

		LB_TraceRegion_Header *header;
		//LB_TraceRegion_Header *temp_header;

		LB_TraceRegion *tr;
		L_Cb *cb;
		L_Oper *op;
		int do_peel, final_ops;
		if (fn->n_cb == 0)
		return;
		int i;

		memset(&LB_hb_stat, 0, sizeof (LB_HB_Stat));

		LB_split_exit_block(fn);

		header = LB_function_init(fn);

		//temp_header = LB_function_init(fn);

		L_breakup_pre_post_inc_ops(fn);

		LB_clr_hyperblock_flag(fn);

		L_compute_oper_weight(fn, 0, 1);
		/* make sure all cbs contain at most 2 targets */

		LB_convert_to_strict_basic_block_code(
				fn,
				L_CB_SUPERBLOCK | L_CB_HYPERBLOCK | L_CB_ENTRANCE_BOUNDARY
				| L_CB_EXIT_BOUNDARY);

		for (cb = fn->first_cb; cb; cb = cb->next_cb) {
			L_Oper *next_op;
			for (op = cb->first_op; op; op = next_op) {
				next_op = op->next_op;

				if (op->opc == Lop_NO_OP)
				L_delete_oper(cb, op);
				else
				LB_hb_stat.orig_ops++;
			}
		}

		L_do_flow_analysis(fn, DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE);

		L_loop_detection(fn, 0);

		if (LB_hb_verbose_level >= 7) {
			fprintf(stderr, "Initial Loop Detection Phase for (fn %s):\n",
					fn->name);
			L_print_loop_data(fn);
		}

		LB_elim_all_loop_backedges(fn);

		L_delete_unreachable_blocks(fn);

		L_do_flow_analysis(fn, DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE);
		L_reset_loop_headers(fn);
		L_loop_detection(fn, 0);

		if (LB_hb_verbose_level >= 7) {
			fprintf(stderr, "Final Loop Detection Phase for (fn %s):\n",
					fn->name);
			L_print_loop_data(fn);
		}

		L_compute_oper_weight(fn, 0, 1);
		LB_mark_jrg_flag(fn);

		//**********************************************************************

		L_partial_dead_code_removal(fn);
		L_do_flow_analysis(fn,
				DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE | SUPPRESS_PG);

		//fprintf(stderr,"Resid 0\n");
		{
			ILP_cfg_wcet_analyize(fn, "(0).txt");
			Ldot_display_cfg(fn, "t0.dot", 0);
			FILE * file = fopen("rrr.txt", "w");
			L_print_func(file, fn);
			fclose(file);
		}

	}
	return;
}

#endif

#ifdef SHARIF3

int my_find_kind(L_Func * fn, L_Cb * start, L_Cb* end, L_Cb * current, Set blocks) {
	L_Flow * flow;

	//if (!Set_in(current->dom, start->id) || !Set_in(current->pdom, end->id)
	if ((Set_in(headers, current->id) && current != start) ||
			(Set_in(backedge, current->id) && current != end) ||
			(Set_in(backedge, current->id) && Set_in(headers, current->id))) {
		return 0;
	}
	Set_add(blocks, current->id);

	if (end == current)
	return 1;

	for (flow = current->dest_flow; flow; flow = flow->next_flow) {
		if (flow->dst_cb == current)
		return 0;
		if (!(flow->is_linear = my_find_kind(start, end, flow->dst_cb, blocks, header)))
		return 0;
	}

	if (current == start) {
		LB_TraceRegion * tr = My_simple_trace_formation(header->fn, header,
				L_TRACEREGION_HAMMOCK, start, end, blocks, header->next_id++);

		start->chock_middle_nodes = Set_copy(blocks);
		start->has_profit = my_profit(fn, blocks);

		//LB_summarize_tr (stderr, tr);
		//tr->wcet2=end->wcet;
		//tr = LB_compute_traceregion_info (tr);
		//header->traceregions = List_insert_last(header->traceregions, tr);
		Set_dispose(blocks);
	}

	return 1;
}

void my_get_node_info(L_Cb * cb) {
	cb->is_balance = is_balance(cb);
	Set temp = Set_new();
	my_find_kind(fn, cb, cb->chock_end_cb, cb, temp);
}

Graph my_last_init(L_Func * fn) {

	Graph bb_g = L_create_cb_graph(fn);
	Graph_dominator(bb_g);
	Graph_post_dominator(bb_g);
	Graph_imm_dominator(bb_g);
	Graph_imm_post_dominator(bb_g);
	Graph_control_dependence(bb_g);
	Graph_topological_sort(bb_g);
	Graph_preorder_dfs_sort(bb_g, 1);
	Graph_preorder_dfs_visit(bb_g, bb_g->root, 1);

	backedge = find_set_backedges(fn);
	headers = find_set_headers(fn);
	List_start(bb_g->topo_list);
	List_next(bb_g->topo_list);

	L_Cb * cb;
	GraphNode nodeg;
	L_Cb * node;
	L_Flow * flow;

	List_start(bb_g->nodes);

	while ((nodeg = (GraphNode) List_next(bb_g->nodes))) {
		//fprintf(stderr, "FF(%d)\n",nodeg->id);
		GraphNode tem = get_imm_post_domin(bb_g->topo_list, nodeg);
		if (tem != NULL)
		((L_Cb *) (nodeg->ptr))->chock_end_cb = (L_Cb *) (tem->ptr);

		//fprintf(stderr,"CHOCK(%d,%d)",node->id,node->chock_end_cb->id);

	}

	for (node = fn->first_cb; node; node = node->next_cb) {
		double max = -100;
		fprintf(stderr, "MORTY(%d)\n", node->id);
		for (flow = node->dest_flow; flow; flow = flow->next_flow) {

			if (!Set_in(node->dom, flow->dst_cb->id)) {
				fprintf(stderr, "FLOW(%d)\n", flow->dst_cb->id);
				flow->wcet_until_chock = find_max_weight_of_paths(flow->dst_cb, node->chock_end_cb) + flow->wcet_weight;
				if (flow->wcet_until_chock > max)
				max = flow->wcet_until_chock;
			}

		}

		node->wcet_until_chock = max;

		my_get_node_info(node);

	}

}

void my_last_visit(L_Func *fn, LB_TraceRegion_Header * header, L_Cb * cb, Set blocks, L_Loop * loop, L_Cb * start) {

	int n = 0;
	L_Flow * flow;
	for (flow = cb->dest_flow; flow; flow = flow->next_flow) n++;

	if (cb == loop->header) {
		if (n > 1) {
			if (!cb->is_balance) {

				for (flow = cb->dest_flow; flow; flow = flow->next_flow) {
					if (flow->ILP_is_in_wcet_path)
					break;

				}

				Set_add(blocks, cb->id);
				my_last_visit(fn, header, flow->dst_cb, blocks, loop, start);
			} else {

				if (cb->has_profit) {

					blocks = Set_union(blocks, cb->chock_middle_nodes);
					if (cb->is_closed) {
						if (Set_size(blocks) > 1) {
							LB_TraceRegion * tr = My_simple_trace_formation(fn, header, L_TRACEREGION_HAMMOCK,
									start, start->chock_end_cb, blocks, header->next_id++);
							header->traceregions = List_insert_last(header->traceregions, tr);
						}
						Set new_blocks = Set_new();
						my_last_visit(fn, header, cb->chock_end_cb, new_blocks, loop, cb->chock_end_cb);
						Set_dispose(new_blocks);
					} else {
						my_last_visit(fn, header, cb->chock_end_cb, blocks, loop, start);
					}

				} else {
					if (Set_size(blocks) > 1) {
						LB_TraceRegion * tr = My_simple_trace_formation(fn, header, L_TRACEREGION_HAMMOCK,
								start, cb, blocks, header->next_id++);
						header->traceregions = List_insert_last(header->traceregions, tr);
					}
					Set new_blocks = Set_new();
					my_last_visit(fn, header, cb->chock_end_cb, new_blocks, loop, cb->chock_end_cb);
					Set_dispose(new_blocks);
				}
			}
		} else if (n == 1) {
			Set_add(blocks, cb->id);
			my_last_visit(fn, header, flow->dst_cb, blocks, loop, start);

		}
		return;
	}

	if (Set_in(cb, loop->back_edge_cb)) {
		if (Set_size(blocks) > 1) {
			LB_TraceRegion * tr = My_simple_trace_formation(fn, header, L_TRACEREGION_HAMMOCK,
					start, cb, blocks, header->next_id++);
			header->traceregions = List_insert_last(header->traceregions, tr);
		}

		return;
	}

	if (!Set_in(headers, node->id) && !Set_in(backedge, node->id)) {
		if (n > 1) {
			if (!cb->is_balance) {
				Set_union(blocks, cb->chock_middle_nodes);

				//Set_add(blocks,cb->id);
				my_last_visit(fn, header, flow->dst_cb, blocks, loop, start);
			} else {

				if (cb->has_profit) {

					blocks = Set_union(blocks, cb->chock_middle_nodes);
					if (cb->is_closed) {
						if (Set_size(blocks) > 1) {
							LB_TraceRegion * tr = My_simple_trace_formation(fn, header, L_TRACEREGION_HAMMOCK,
									start, start->chock_end_cb, blocks, header->next_id++);
							header->traceregions = List_insert_last(header->traceregions, tr);
						}
						Set new_blocks = Set_new();
						my_last_visit(fn, header, cb->chock_end_cb, new_blocks, loop, cb->chock_end_cb);
						Set_dispose(new_blocks);
					} else {
						my_last_visit(fn, header, cb->chock_end_cb, blocks, loop, start);
					}

				} else {
					if (Set_size(blocks) > 1) {
						LB_TraceRegion * tr = My_simple_trace_formation(fn, header, L_TRACEREGION_HAMMOCK,
								start, cb, blocks, header->next_id++);
						header->traceregions = List_insert_last(header->traceregions, tr);
					}
					Set new_blocks = Set_new();
					my_last_visit(fn, header, cb->chock_end_cb, new_blocks, loop, cb->chock_end_cb);
					Set_dispose(new_blocks);
				}
			}
		} else if (n == 1) {
			Set_add(blocks, cb->id);
			my_last_visit(fn, header, flow->dst_cb, blocks, loop, start);

		}
		return;

	}

}

void my_last_trace_formation(L_Func * fn, LB_TraceRegion_Header * header) {

	find_loops_wcet(fn);
	Graph g = my_last_init(fn);

	L_Loop * loop;
	L_Cb * head;
	L_Cb * cb;
	Set blocks;

	for (loop = fn->first_loop; loop; loop = loop->next_loop) {

		head = loop->header;

		if (Set_size(loop) == 1 || head->ILP_is_in_wcet_path == 0) continue;

		//		List_start(g->topo_list);
		//		GraphNode node;
		//		while((node=(GraphNode)List_next(g->topo_list))->id==head->id);

		if (!L_EXTRACT_BIT_VAL(head->flags, L_CB_SUPERBLOCK)
				&& !L_EXTRACT_BIT_VAL(head->flags, L_CB_HYPERBLOCK)) {

			blocks = Set_new();
			my_last_visit(fn, header, head, blocks, loop);

		} else {

			List_start(g->topo_list);
			GraphNode node;
			while ((node = (GraphNode) List_next(g->topo_list))->id == head->id);
			while ((node = (GraphNode) List_next(g->topo_list))) {
				if (!L_EXTRACT_BIT_VAL(head->flags, L_CB_SUPERBLOCK)
						&& !L_EXTRACT_BIT_VAL(head->flags, L_CB_HYPERBLOCK)
						&& Set_in(loop->loop_cb, node->id)
						&& ((L_Cb *) (node->ptr))->ILP_is_in_wcet_path) {
					break;
				}

			}

			blocks = Set_new();
			my_last_visit(fn, header, (L_Cb*) (node->ptr), blocks, loop);

		}

	}
	Graph_free_graph(g);
}

void LB_hyperblock_formation(L_Func * fn) {

	//fprintf(stderr,"QQQQQQQQQQQQQQQQQQQQQQQ\nwwwwwwwwwwwwwwwwwwwwwwwww\nDDDDDDDDDDDDDDDDDDDDDDDDD\n");

	if (strcmp(fn->name, "_main") == 0) {
		LB_TraceRegion_Header *header;
		LB_TraceRegion *tr;
		L_Cb *cb;
		L_Oper *op;
		int do_peel, final_ops;
		if (fn->n_cb == 0)
		return;
		int i;

		memset(&LB_hb_stat, 0, sizeof (LB_HB_Stat));

		LB_split_exit_block(fn);

		header = LB_function_init(fn);

		//temp_header = LB_function_init(fn);

		L_breakup_pre_post_inc_ops(fn);

		LB_clr_hyperblock_flag(fn);

		L_compute_oper_weight(fn, 0, 1);
		/* make sure all cbs contain at most 2 targets */

		LB_convert_to_strict_basic_block_code(
				fn,
				L_CB_SUPERBLOCK | L_CB_HYPERBLOCK | L_CB_ENTRANCE_BOUNDARY
				| L_CB_EXIT_BOUNDARY);

		for (cb = fn->first_cb; cb; cb = cb->next_cb) {
			L_Oper *next_op;
			for (op = cb->first_op; op; op = next_op) {
				next_op = op->next_op;

				if (op->opc == Lop_NO_OP)
				L_delete_oper(cb, op);
				else
				LB_hb_stat.orig_ops++;
			}
		}

		L_do_flow_analysis(fn, DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE);

		L_loop_detection(fn, 0);

		if (LB_hb_verbose_level >= 7) {
			fprintf(stderr, "Initial Loop Detection Phase for (fn %s):\n",
					fn->name);
			L_print_loop_data(fn);
		}

		LB_elim_all_loop_backedges(fn);

		L_delete_unreachable_blocks(fn);

		L_do_flow_analysis(fn, DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE);
		L_reset_loop_headers(fn);
		L_loop_detection(fn, 0);

		if (LB_hb_verbose_level >= 7) {
			fprintf(stderr, "Final Loop Detection Phase for (fn %s):\n",
					fn->name);
			L_print_loop_data(fn);
		}

		L_compute_oper_weight(fn, 0, 1);
		LB_mark_jrg_flag(fn);

		//**********************************************************************

		L_partial_dead_code_removal(fn);
		L_do_flow_analysis(fn,
				DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE | SUPPRESS_PG);

		//fprintf(stderr,"Resid 0\n");
		{
			ILP_cfg_wcet_analyize(fn, "(0).txt");
			Ldot_display_cfg(fn, "t0.dot", 0);
			FILE * file = fopen("rrr.txt", "w");
			L_print_func(file, fn);
			fclose(file);
		}

		for (i = 1; i < 100; i++) {

			//	L_do_flow_analysis(fn,
			//				DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE | SUPPRESS_PG);
			//L_partial_dead_code_removal(fn);
			L_do_flow_analysis(fn,
					DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE | SUPPRESS_PG);
			L_reset_loop_headers(fn);

			backedge = find_set_backedges(fn);
			headers = find_set_headers(fn);

			{
				char tp[100];

				sprintf(tp, "(%d).txt", i);
				ILP_cfg_wcet_analyize(fn, tp);

				sprintf(tp, "t%d.dot", i);
				Ldot_display_cfg(fn, tp, 0);

			}
			//fprintf(stderr,"Resid1\n");

			fprintf(stderr, "ROUND (%d)\n", i);

			//1- for each loop that it's header is in wcep start from header and walk in wcep if you reache
			// to a two way node if to way is equal so until it is possible add nodes to this trace after
			//that if you reach to a illguel node stop trace growing
			my_last_trace_formation(fn, header);

			LB_hb_reset_max_oper_id(fn);

			LB_remove_partially_subsumed_traceregions(header);

			LB_remove_conflicting_traceregions(header);

			LB_set_hyperblock_flag(fn);

			LB_set_hyperblock_func_flag(fn);

			//*****************trace selection***************
			//tr=(LB_TraceRegion *)List_get_first(temp_header->traceregions);
			//header->traceregions=List_insert_last(header->traceregions,tr);

			//***********************************************

			LB_summarize_traceregions(stderr, header);

			LB_tail_duplicate(fn, header, LB_DUP_OUTSIDE_REGION);

			{
				L_check_func(fn);
			}

			LB_predicate_traceregions(fn, header);

			LB_set_hyperblock_flag(fn);
			LB_remove_unnec_hyperblock_flags(fn);
			LB_set_hyperblock_func_flag(fn);

			LB_free_all_traceregions(header);

			//temp_header->traceregions = List_reset (temp_header->traceregions);
			//temp_header->inorder_trs = List_reset (temp_header->inorder_trs);

			LB_remove_empty_cbs(fn);

			L_delete_unreachable_blocks(fn);

			LB_convert_to_strict_basic_block_code(fn,
					L_CB_HYPERBLOCK | L_CB_SUPERBLOCK);

			{
				DB_spit_func(fn, "PH1");
				L_check_func(fn);
			}

		}

		//**********************************************************************
		fprintf(stderr,
				"***************\n**************\n***************\n***************\n");

		{
			//L_partial_dead_code_removal(fn);
			L_do_flow_analysis(fn,
					DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE | SUPPRESS_PG);
			ILP_cfg_wcet_analyize(fn, "(99).txt");
			Ldot_display_cfg(fn, "t99.dot", 0);

			// LB_summarize_traceregions (stderr, header);
		}
		L_do_flow_analysis(fn,
				DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE | SUPPRESS_PG);

		//*******************************************************************

		//******************************************************************

		/*
		 *  Split branches into pred defines and predicated jumps
		 */
		if (LB_hb_branch_split)
		LB_branch_split_func(fn);

		/*
		 *        Generate multiple defn pred defines, generate Uncond pred defines
		 *  (Note initially only OR-type pred defines are created!
		 */

		L_create_uncond_pred_defines(fn);

		PG_setup_pred_graph(fn);
		if (LB_do_lightweight_pred_opti)
		L_lightweight_pred_opti(fn);
		L_combine_pred_defines(fn);

		/*
		 *        remove unnecessary uncond jumps
		 */

		LB_uncond_branch_elim(fn);

		/*
		 *        Merge ops on opposite predicates (partial redundancy elim)
		 */

		L_unmark_all_pre_post_increments(fn);

		if (LB_hb_do_pred_merge) {
			PG_setup_pred_graph(fn);
			L_do_flow_analysis(fn, LIVE_VARIABLE | REACHING_DEFINITION);
			LB_hb_pred_merging(fn);
		}

#ifdef DEBUG_HB_FORMER
		L_check_func(fn);
#endif

		PG_destroy_pred_graph();
		D_delete_dataflow(fn);

		/* For each source code function we process deinit */
		LB_function_deinit(header);

		{
			double rat = 1.0;
			L_Attr *eattr;

			final_ops = 0;

			for (cb = fn->first_cb; cb; cb = cb->next_cb) {
				L_Oper *next_op;
				for (op = cb->first_op; op; op = next_op) {
					next_op = op->next_op;

					if (op->opc != Lop_NO_OP)
					final_ops++;
				}
			}

			if (LB_hb_stat.orig_ops)
			rat = (double) final_ops / LB_hb_stat.orig_ops;

			eattr = L_new_attr("hbe", 2);
			L_set_int_attr_field(eattr, 0, LB_hb_stat.orig_ops);
			L_set_double_attr_field(eattr, 1, rat);
			fn->attr = L_concat_attr(fn->attr, eattr);
		}

	}

	return;
}

#endif

#ifdef COUNT_number_of_equal_branch

void LB_hyperblock_formation(L_Func * fn) {

	//fprintf(stderr,"QQQQQQQQQQQQQQQQQQQQQQQ\nwwwwwwwwwwwwwwwwwwwwwwwww\nDDDDDDDDDDDDDDDDDDDDDDDDD\n");

	if (strcmp(fn->name, "_main") == 0) {
		LB_TraceRegion_Header *header;
		//LB_TraceRegion_Header *temp_header;

		LB_TraceRegion *tr;
		L_Cb *cb;
		L_Oper *op;
		int do_peel, final_ops;
		if (fn->n_cb == 0)
		return;
		int i;

		memset(&LB_hb_stat, 0, sizeof (LB_HB_Stat));

		LB_split_exit_block(fn);

		header = LB_function_init(fn);

		//temp_header = LB_function_init(fn);

		L_breakup_pre_post_inc_ops(fn);

		LB_clr_hyperblock_flag(fn);

		L_compute_oper_weight(fn, 0, 1);
		/* make sure all cbs contain at most 2 targets */

		LB_convert_to_strict_basic_block_code(
				fn,
				L_CB_SUPERBLOCK | L_CB_HYPERBLOCK | L_CB_ENTRANCE_BOUNDARY
				| L_CB_EXIT_BOUNDARY);

		for (cb = fn->first_cb; cb; cb = cb->next_cb) {
			L_Oper *next_op;
			for (op = cb->first_op; op; op = next_op) {
				next_op = op->next_op;

				if (op->opc == Lop_NO_OP)
				L_delete_oper(cb, op);
				else
				LB_hb_stat.orig_ops++;
			}
		}

		L_do_flow_analysis(fn, DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE);

		L_loop_detection(fn, 0);

		if (LB_hb_verbose_level >= 7) {
			fprintf(stderr, "Initial Loop Detection Phase for (fn %s):\n",
					fn->name);
			L_print_loop_data(fn);
		}

		LB_elim_all_loop_backedges(fn);

		L_delete_unreachable_blocks(fn);

		L_do_flow_analysis(fn, DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE);
		L_reset_loop_headers(fn);
		L_loop_detection(fn, 0);

		if (LB_hb_verbose_level >= 7) {
			fprintf(stderr, "Final Loop Detection Phase for (fn %s):\n",
					fn->name);
			L_print_loop_data(fn);
		}

		L_compute_oper_weight(fn, 0, 1);
		LB_mark_jrg_flag(fn);

		//**********************************************************************

		L_partial_dead_code_removal(fn);
		L_do_flow_analysis(fn,
				DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE | SUPPRESS_PG);

		//fprintf(stderr,"Resid 0\n");
		{
			ILP_cfg_wcet_analyize(fn, "(0).txt");
			Ldot_display_cfg(fn, "t0.dot", 0);
			FILE * file = fopen("rrr.txt", "w");
			L_print_func(file, fn);
			fclose(file);
		}

		for (i = 1; i < 2; i++) {

			//	L_do_flow_analysis(fn,
			//				DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE | SUPPRESS_PG);
			//L_partial_dead_code_removal(fn);
			L_do_flow_analysis(fn,
					DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE | SUPPRESS_PG);
			L_reset_loop_headers(fn);
			L_loop_detection(fn, 0);

			{
				char tp[100];

				sprintf(tp, "(%d).txt", i);
				ILP_cfg_wcet_analyize(fn, tp);

				sprintf(tp, "t%d.dot", i);
				Ldot_display_cfg(fn, tp, 0);

			}
			//fprintf(stderr,"Resid1\n");
			find_loops_wcet(fn);

			fprintf(stderr, "ROUND (%d)\n", i);

			/*************************************************************************************/
			/*************************************************************************************/
			/*************************************************************************************/
			/*************************************************************************************/
			Graph bb_g = L_create_cb_graph(fn);
			Graph_dominator(bb_g);
			Graph_post_dominator(bb_g);
			Graph_imm_dominator(bb_g);
			Graph_imm_post_dominator(bb_g);
			Graph_control_dependence(bb_g);
			Graph_topological_sort(bb_g);
			Graph_preorder_dfs_sort(bb_g, 1);
			Graph_preorder_dfs_visit(bb_g, bb_g->root, 1);

			backedge = find_set_backedges(fn);
			headers = find_set_headers(fn);
			List_start(bb_g->topo_list);
			List_next(bb_g->topo_list);

			L_Cb * cb;
			GraphNode nodeg;
			L_Cb * node;
			L_Flow * flow;

			List_start(bb_g->nodes);

			while ((nodeg = (GraphNode) List_next(bb_g->nodes))) {

				//fprintf(stderr, "FF(%d)\n",nodeg->id);
				GraphNode tem = get_imm_post_domin(bb_g->topo_list, nodeg);

				if (tem != NULL)
				((L_Cb *) (nodeg->ptr))->chock_end_cb = (L_Cb *) (tem->ptr);
				//fprintf(stderr,"OOOOOOOOOOOOOOOOOOO\n");
				//fprintf(stderr,"CHOCK(%d,%d)",node->id,node->chock_end_cb->id);

			}
			int num_total_branch = 0;
			int num_balance_branch = 0;

			for (node = fn->first_cb; node; node = node->next_cb) {
				double max = -100;
				int num_flow = 0;

				//fprintf(stderr,"MORTY(%d)\n",node->id);
				for (flow = node->dest_flow; flow; flow = flow->next_flow) {
					if (!Set_in(node->dom, flow->dst_cb->id)) {
						num_flow++;

						//	fprintf(stderr,"FLOW(%d)\n",flow->dst_cb->id);
						flow->wcet_until_chock = find_max_weight_of_paths(flow->dst_cb, node->chock_end_cb) + flow->wcet_weight;
						if (flow->wcet_until_chock > max)
						max = flow->wcet_until_chock;
					}

				}
				node->wcet_until_chock = max;
				if (num_flow >= 2) {
					num_total_branch += node->wcep_num_exec;
					int is_balance = 1;
					for (flow = node->dest_flow; flow; flow = flow->next_flow) {
						if ((max - 1) > flow->wcet_until_chock) {
							is_balance = 0;
							break;
						}
					}
					if (is_balance == 1) num_balance_branch += node->wcep_num_exec;
				}

			}

			FILE * branch = fopen("branch.txt", "w");
			if (num_total_branch != 0)
			fprintf(branch, "%d\n%d\n%d\n", num_total_branch, num_balance_branch, ((num_balance_branch * 100) / num_total_branch));
			else
			fprintf(branch, "%d\n%d\n%d\n", num_total_branch, num_balance_branch, 0);

			fclose(branch);
			/****************************************************************************/
			/****************************************************************************/
			/****************************************************************************/
			/****************************************************************************/

			//			if (i > 1) {
			//
			//				my_trace_formation2(fn, header);
			//
			//				//LB_summarize_traceregions(stderr, header);
			//				if (header->traceregions==NULL)
			//									break;
			//				my_select_trace_2(fn, header);
			//				fprintf(stderr, "TTTTTTTTTTTTTTTTTTTTTTTTTTTTTT\n");
			//
			//
			//				if (header->traceregions==NULL)
			//					break;
			//				if(header->traceregions->size<=0)
			//					break;
			//				if(header->traceregions->first==NULL){
			//					header->traceregions=NULL;
			//					break;
			//				}
			//
			//				fprintf(stderr,"UUUUUUU\n");
			//				LB_summarize_traceregions(stderr, header);
			//			}
			//			else{
			//				//pre path hb formation
			//				my_pre_path_chock_selection(fn,header);
			//
			//			}
			LB_hb_reset_max_oper_id(fn);

			LB_remove_partially_subsumed_traceregions(header);

			LB_remove_conflicting_traceregions(header);

			LB_set_hyperblock_flag(fn);

			LB_set_hyperblock_func_flag(fn);

			//*****************trace selection***************
			//tr=(LB_TraceRegion *)List_get_first(temp_header->traceregions);
			//header->traceregions=List_insert_last(header->traceregions,tr);

			//***********************************************

			LB_summarize_traceregions(stderr, header);

			LB_tail_duplicate(fn, header, LB_DUP_OUTSIDE_REGION);

			{
				L_check_func(fn);
			}

			LB_predicate_traceregions(fn, header);

			LB_set_hyperblock_flag(fn);
			LB_remove_unnec_hyperblock_flags(fn);
			LB_set_hyperblock_func_flag(fn);

			LB_free_all_traceregions(header);

			//temp_header->traceregions = List_reset (temp_header->traceregions);
			//temp_header->inorder_trs = List_reset (temp_header->inorder_trs);

			LB_remove_empty_cbs(fn);

			L_delete_unreachable_blocks(fn);

			LB_convert_to_strict_basic_block_code(fn,
					L_CB_HYPERBLOCK | L_CB_SUPERBLOCK);

			{
				DB_spit_func(fn, "PH1");
				L_check_func(fn);
			}

		}

		//**********************************************************************
		fprintf(stderr,
				"***************\n**************\n***************\n***************\n");

		{
			//L_partial_dead_code_removal(fn);
			L_do_flow_analysis(fn,
					DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE | SUPPRESS_PG);
			ILP_cfg_wcet_analyize(fn, "(99).txt");
			Ldot_display_cfg(fn, "t99.dot", 0);

			// LB_summarize_traceregions (stderr, header);
		}
		L_do_flow_analysis(fn,
				DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE | SUPPRESS_PG);

		//*******************************************************************

		//******************************************************************

		/*
		 *  Split branches into pred defines and predicated jumps
		 */
		if (LB_hb_branch_split)
		LB_branch_split_func(fn);

		/*
		 *        Generate multiple defn pred defines, generate Uncond pred defines
		 *  (Note initially only OR-type pred defines are created!
		 */

		L_create_uncond_pred_defines(fn);

		PG_setup_pred_graph(fn);
		if (LB_do_lightweight_pred_opti)
		L_lightweight_pred_opti(fn);
		L_combine_pred_defines(fn);

		/*
		 *        remove unnecessary uncond jumps
		 */

		LB_uncond_branch_elim(fn);

		/*
		 *        Merge ops on opposite predicates (partial redundancy elim)
		 */

		L_unmark_all_pre_post_increments(fn);

		if (LB_hb_do_pred_merge) {
			PG_setup_pred_graph(fn);
			L_do_flow_analysis(fn, LIVE_VARIABLE | REACHING_DEFINITION);
			LB_hb_pred_merging(fn);
		}

#ifdef DEBUG_HB_FORMER
		L_check_func(fn);
#endif

		PG_destroy_pred_graph();
		D_delete_dataflow(fn);

		/* For each source code function we process deinit */
		LB_function_deinit(header);

		{
			double rat = 1.0;
			L_Attr *eattr;

			final_ops = 0;

			for (cb = fn->first_cb; cb; cb = cb->next_cb) {
				L_Oper *next_op;
				for (op = cb->first_op; op; op = next_op) {
					next_op = op->next_op;

					if (op->opc != Lop_NO_OP)
					final_ops++;
				}
			}

			if (LB_hb_stat.orig_ops)
			rat = (double) final_ops / LB_hb_stat.orig_ops;

			eattr = L_new_attr("hbe", 2);
			L_set_int_attr_field(eattr, 0, LB_hb_stat.orig_ops);
			L_set_double_attr_field(eattr, 1, rat);
			fn->attr = L_concat_attr(fn->attr, eattr);
		}

	}

	return;
}

#endif

#ifdef SHARIF4

int is_good2(L_Cb * first, L_Cb * last, Set blocks, L_Func * fn) {

	L_Flow * flow;

	int n = Set_size(blocks);
	int * buf = (int *) Lcode_malloc(sizeof (int) * n);
	Set_2array(blocks, buf);
	int i;
	for (i = 0; i < n; i++) {

		L_Cb * cb = L_cb_hash_tbl_find(fn->cb_hash_tbl, buf[i]);
		if (cb->id != first->id) {
			for (flow = cb->src_flow; flow; flow = flow->next_flow) {
				if (!Set_in(blocks, flow->src_cb->id)) {
					Lcode_free(buf);
					return 0;
				}
			}
		}

	}
	Lcode_free(buf);
	return 1;

}

int is_simple_sharif4(L_Cb * first, L_Cb * end, Set loops) {

	if (!Set_same(loops, first->loops)) return 0;
	if (first->id == end->id) return 1;

	L_Flow * flow;
	for (flow = first->dest_flow; flow != NULL; flow = flow->next_flow) {
			if(flow->dst_cb->id==first->id)
			{
				fprintf(stderr,"OOOOOOOOOOOOOOOOOOO\n");
				return 0;
			}
			if (is_simple_sharif4(flow->dst_cb, end, loops) == 0) return 0;

	}

	return 1;
	//	if(		(first->flags & L_CB_LOOP_HEADER) ||
	//			L_EXTRACT_BIT_VAL(first->flags,L_CB_SUPERBLOCK) ||
	//			L_EXTRACT_BIT_VAL(first->flags,L_CB_HYPERBLOCK) ) return 0;
	//
	//	if(first->id==end->id) return 1;
	//
	//	L_Flow * flow;
	//	for(flow=first->dest_flow;flow!=NULL;flow=flow->next_flow)
	//	{
	//		if(is_simple_sharif4(flow->dst_cb,end)==0) return 0;
	//
	//	}
	//
	//	return 1;

}
//we are sure that chock is simple

void find_chock_blocks_sharif4(L_Cb * first, L_Cb * end, Set blocks) {
	Set_add(blocks, first->id);
	if (first->id == end->id) return;
	L_Flow * flow;
	for (flow = first->dest_flow; flow != NULL; flow = flow->next_flow) {

		find_chock_blocks_sharif4(flow->dst_cb, end, blocks);
	}

}

void visit_sharif4(L_Cb * node, L_Cb * first_node, L_Cb * last_node, Set blocks,
		LB_TraceRegion_Header *header, double wcet, L_Func * fn) {

	int kind = 0;

	if (!Set_in(headers, node->id) && !Set_in(backedge, node->id)) kind = 0;
	else if (Set_in(headers, node->id) && !Set_in(backedge, node->id)) kind = 8;
	else if (!Set_in(headers, node->id) && Set_in(backedge, node->id)) kind = 4;
	else if (Set_in(headers, node->id) && Set_in(backedge, node->id)) kind = 12;

	if (node->is_balance) kind = kind | 2;

	if (node->is_simple) kind = kind | 1;

	if (L_EXTRACT_BIT_VAL(node->flags, L_CB_SUPERBLOCK) || L_EXTRACT_BIT_VAL(node->flags, L_CB_HYPERBLOCK))
	kind = kind | 16;

	if (node->dest_flow == NULL) kind = 32;

	fprintf(stderr, "CB(%d)=%d\n", node->id, kind);

	if ((node->ILP_is_in_wcet_path == 1) && kind != 32) {

		switch (kind) {

			case 0: case 1:
			{ // general_node not_balance (not_simple || simple)
				Set_add(blocks, node->id);

				L_Cb * node2;
				L_Flow * flow;

				for (flow = node->dest_flow; flow; flow = flow->next_flow) {
					if (flow->ILP_is_in_wcet_path) {
						node2 = flow->dst_cb;
						visit_sharif4(node2, first_node, node, blocks, header, wcet + flow->wcet_weight, fn);
					}
				}

				break;
			}
			case 2:
			{ // general_node balance not_simple
				Set_add(blocks, node->id);

				if (Set_size(blocks) > 1) {

					LB_TraceRegion * tr = My_simple_trace_formation(
							header->fn, header, L_TRACEREGION_HAMMOCK,
							first_node, node, blocks,
							header->next_id++);
					tr->wcet2 = wcet;
					header->traceregions = List_insert_last(
							header->traceregions, tr);
				}

				L_Cb * node2 = node->chock_end_cb;
				Set temp = Set_new();
				visit_sharif4(node2, node2, node2, temp, header, 0, fn);
				Set_dispose(temp);

				break;
			}
			case 3:
			{ // general_node balance simple
				blocks = Set_union(blocks, node->chock_middle_nodes);
				L_Cb * node2 = node->chock_end_cb;
				visit_sharif4(node2, first_node, node2, blocks, header, wcet + node->wcet_until_chock, fn);

				break;
			}
			case 8: case 9:
			{ // header_node not_balance (not_simple || simple)
				if (Set_size(blocks) > 1) {

					LB_TraceRegion * tr = My_simple_trace_formation(
							header->fn, header, L_TRACEREGION_HAMMOCK,
							first_node, last_node, blocks,
							header->next_id++);
					tr->wcet2 = wcet;
					header->traceregions = List_insert_last(
							header->traceregions, tr);
				}

				Set temp = Set_new();
				Set_add(temp, node->id);

				L_Cb * node2;
				L_Flow * flow;

				for (flow = node->dest_flow; flow; flow = flow->next_flow) {
					if (flow->ILP_is_in_wcet_path) {
						node2 = flow->dst_cb;
						visit_sharif4(node2, node, node, temp, header, wcet + flow->wcet_weight, fn);
					}
				}

				break;
			}
			case 10:
			{ // header_node balance not_simple
				if (Set_size(blocks) > 1) {

					LB_TraceRegion * tr = My_simple_trace_formation(
							header->fn, header, L_TRACEREGION_HAMMOCK,
							first_node, last_node, blocks,
							header->next_id++);
					tr->wcet2 = wcet;
					header->traceregions = List_insert_last(
							header->traceregions, tr);
				}

				L_Cb * node2 = node->chock_end_cb;
				Set temp = Set_new();
				visit_sharif4(node2, node2, node2, temp, header, 0, fn);
				Set_dispose(temp);

				break;
			}
			case 11:
			{ // header_node balance simple
				if (Set_size(blocks) > 1) {

					LB_TraceRegion * tr = My_simple_trace_formation(
							header->fn, header, L_TRACEREGION_HAMMOCK,
							first_node, last_node, blocks,
							header->next_id++);
					tr->wcet2 = wcet;
					header->traceregions = List_insert_last(
							header->traceregions, tr);
				}
				Set temp = Set_new();
				temp = Set_union(temp, node->chock_middle_nodes);
				L_Cb * node2 = node->chock_end_cb;
				visit_sharif4(node2, node, node2, temp, header, wcet + node->wcet_until_chock, fn);
				Set_dispose(temp);

				break;
			}
			case 12: case 13: case 14: case 15:
			{ // end_node && header node
				if (Set_size(blocks) > 1) {

					LB_TraceRegion * tr = My_simple_trace_formation(
							header->fn, header, L_TRACEREGION_HAMMOCK,
							first_node, last_node, blocks,
							header->next_id++);
					tr->wcet2 = wcet;
					header->traceregions = List_insert_last(
							header->traceregions, tr);
				}

				Set temp = Set_new();

				L_Cb * node2;
				L_Flow * flow;

				for (flow = node->dest_flow; flow; flow = flow->next_flow) {
					if (flow->ILP_is_in_wcet_path && (flow->is_backedge == 0)) {
						node2 = flow->dst_cb;
						visit_sharif4(node2, node2, node2, temp, header, wcet + flow->wcet_weight, fn);
					}
				}

				Set_dispose(temp);
				break;
			}
			case 4: case 5: case 6: case 7:
			{
				Set_add(blocks, node->id);
				//fprintf(stderr,"VG(%d)(%d)=(%d)\n",first_node->id,node->id,is_good(first_node,node,blocks));
				if (is_good2(first_node, node, blocks, fn) == 0) blocks = Set_delete(blocks, node->id);
				else last_node = node;
				if (Set_size(blocks) > 1) {

					LB_TraceRegion * tr = My_simple_trace_formation(
							header->fn, header, L_TRACEREGION_HAMMOCK,
							first_node, last_node, blocks,
							header->next_id++);
					tr->wcet2 = wcet;
					header->traceregions = List_insert_last(
							header->traceregions, tr);
				}

				Set temp = Set_new();

				L_Cb * node2;
				L_Flow * flow;

				for (flow = node->dest_flow; flow; flow = flow->next_flow) {
					if (flow->ILP_is_in_wcet_path && (flow->is_backedge == 0)) {
						node2 = flow->dst_cb;
						visit_sharif4(node2, node2, node2, temp, header, wcet + flow->wcet_weight, fn);
					}
				}

				Set_dispose(temp);

				break;
			}
			default:
			{ //super_block || hyper_block
				if (Set_size(blocks) > 1) {

					LB_TraceRegion * tr = My_simple_trace_formation(
							header->fn, header, L_TRACEREGION_HAMMOCK,
							first_node, last_node, blocks,
							header->next_id++);
					tr->wcet2 = wcet;
					header->traceregions = List_insert_last(
							header->traceregions, tr);
				}

				Set temp = Set_new();

				L_Cb * node2;
				L_Flow * flow;

				for (flow = node->dest_flow; flow; flow = flow->next_flow) {
					if (flow->ILP_is_in_wcet_path && (flow->is_backedge == 0)) {
						node2 = flow->dst_cb;
						visit_sharif4(node2, node2, node2, temp, header, wcet + flow->wcet_weight, fn);
					}
				}
				Set_dispose(temp);

				break;
			}
		}

	} else {
		if (Set_size(blocks) > 1) {

			LB_TraceRegion * tr = My_simple_trace_formation(
					header->fn, header, L_TRACEREGION_HAMMOCK,
					first_node, last_node, blocks,
					header->next_id++);
			tr->wcet2 = wcet;
			header->traceregions = List_insert_last(
					header->traceregions, tr);
		}

	}
	return;
}

void trace_formation_SHARIF4(L_Func * fn, LB_TraceRegion_Header *header) {

	find_loops_wcet(fn);
	Graph bb_g = L_create_cb_graph(fn);
	Graph_dominator(bb_g);
	Graph_post_dominator(bb_g);
	Graph_imm_dominator(bb_g);
	Graph_imm_post_dominator(bb_g);
	Graph_control_dependence(bb_g);
	Graph_topological_sort(bb_g);
	Graph_preorder_dfs_sort(bb_g, 1);
	Graph_preorder_dfs_visit(bb_g, bb_g->root, 1);

	backedge = find_set_backedges(fn);
	headers = find_set_headers(fn);
	List_start(bb_g->topo_list);
	List_next(bb_g->topo_list);

	L_Cb * cb;
	GraphNode nodeg;
	L_Cb * node;
	L_Flow * flow;

	List_start(bb_g->nodes);

	while ((nodeg = (GraphNode) List_next(bb_g->nodes))) {

		//fprintf(stderr, "FF(%d)\n",nodeg->id);
		GraphNode tem = get_imm_post_domin(bb_g->topo_list, nodeg);

		if (tem != NULL)
		((L_Cb *) (nodeg->ptr))->chock_end_cb = (L_Cb *) (tem->ptr);
		//fprintf(stderr,"OOOOOOOOOOOOOOOOOOO\n");
		//fprintf(stderr,"CHOCK(%d,%d)",node->id,node->chock_end_cb->id);

	}
	int num_total_branch = 0;
	int num_balance_branch = 0;

	L_Loop * loop;
	int * buf;
	int * buf2;
	for (loop = fn->first_loop; loop; loop = loop->next_loop) {

		int size = Set_size(loop->back_edge_cb);
		buf = (int *) malloc(sizeof (int) * size);
		Set_2array(loop->back_edge_cb, buf);
		int i;
		//Set_print(stderr,"WWW",loop->back_edge_cb);
		for (i = 0; i < size; i++) {
			L_Cb * end_node = L_cb_hash_tbl_find(fn->cb_hash_tbl, buf[i]);
			flow = end_node->dest_flow;
			while (flow->dst_cb->id != loop->header->id) flow = flow->next_flow;
			flow->is_backedge = 1;
			//fprintf(stderr,"www(%d)->(%d)\n",flow->src_cb->id,flow->dst_cb->id);
		}

		Lcode_free(buf);

		int size2 = Set_size(loop->loop_cb);
		buf2 = (int *) malloc(sizeof (int) * size2);
		Set_2array(loop->loop_cb, buf2);
		for (i = 0; i < size2; i++) {
			node = L_cb_hash_tbl_find(fn->cb_hash_tbl, buf2[i]);
			if (Set_empty(node->loops)) node->loops = 0;
			Set_add(node->loops, loop->id);
			//fprintf(stderr,"%d\t%d",node->id,loop->id); Set_print(stderr,"",node->loops);
		}

		Lcode_free(buf2);

	}

	for (node = fn->first_cb; node; node = node->next_cb) {
		double max = -100;
		int num_flow = 0;
		fprintf(stderr, "%d\n", node->id);
		//Set_print(stderr,"NNNNN",node->loops);
		//fprintf(stderr,"MORTY(%d)\n",node->id);
		for (flow = node->dest_flow; flow; flow = flow->next_flow) {

			if (flow->is_backedge == 0) {
				num_flow++;

				//	fprintf(stderr,"FLOW(%d)\n",flow->dst_cb->id);
				flow->wcet_until_chock = find_max_weight_of_paths(flow->dst_cb, node->chock_end_cb) + flow->wcet_weight;

				if (flow->wcet_until_chock > max) max = flow->wcet_until_chock;

				flow->is_simple = is_simple_sharif4(flow->dst_cb, node->chock_end_cb, node->loops);
			}

			//fprintf(stderr,"MMMMMMMMMM(%d)(%d)\n",flow->dst_cb->id,flow->is_simple);
		}
		//fprintf(stderr,"KKKKKKKKKKKKKKKKKKKK(%d)\n",node->id);
		node->wcet_until_chock = max;

		node->is_balance = 0;
		node->is_simple = 1;

		if (num_flow >= 2) {
			num_total_branch++;
			node->is_balance = 1;
			node->is_simple = 1;
			for (flow = node->dest_flow; flow; flow = flow->next_flow) {
				if ((max - 1) > flow->wcet_until_chock) {
					node->is_balance = 0;
				}
				if (flow->is_simple == 0) node->is_simple = 0;
			}

		}

		if (Set_in(backedge, node->id)) node->is_simple = 0;

		if (node->is_simple == 1 && node->is_balance == 1) {
			Set blocks = Set_new();
			find_chock_blocks_sharif4(node, node->chock_end_cb, blocks);
			node->chock_middle_nodes = blocks;
			//Set_dispose(blocks);
			//fprintf(stderr,"GOOOD:(%d)->(%d)",node->id,node->chock_end_cb->id);
			//Set_print(stderr,"",node->chock_middle_nodes);
		}

		fprintf(stderr, "node(%d)=%d\t%d", node->id, node->is_simple, node->is_balance);
		Set_print(stderr, "", node->chock_middle_nodes);
	}

	fprintf(stderr, "WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWww\n");

	List_start(bb_g->topo_list);
	List_next(bb_g->topo_list);
	nodeg = (GraphNode) List_next(bb_g->topo_list);
	node = (L_Cb *) (nodeg->ptr);
	visit_sharif4(node, node, node, Set_new(), header, 0, fn);

	Graph_free_graph(bb_g);
}

void LB_hyperblock_formation_WCET(L_Func * fn) {

	//fprintf(stderr,"QQQQQQQQQQQQQQQQQQQQQQQ\nwwwwwwwwwwwwwwwwwwwwwwwww\nDDDDDDDDDDDDDDDDDDDDDDDDD\n");

	if (strcmp(fn->name, "_main") == 0) {
		LB_TraceRegion_Header *header;
		LB_TraceRegion *tr;
		L_Cb *cb;
		L_Oper *op;
		int do_peel, final_ops;
		if (fn->n_cb == 0)
		return;
		int i;

		memset(&LB_hb_stat, 0, sizeof (LB_HB_Stat));

		LB_split_exit_block(fn);

		header = LB_function_init(fn);

		//temp_header = LB_function_init(fn);

		L_breakup_pre_post_inc_ops(fn);

		LB_clr_hyperblock_flag(fn);

		L_compute_oper_weight(fn, 0, 1);
		/* make sure all cbs contain at most 2 targets */

		LB_convert_to_strict_basic_block_code(
				fn,
				L_CB_SUPERBLOCK | L_CB_HYPERBLOCK | L_CB_ENTRANCE_BOUNDARY
				| L_CB_EXIT_BOUNDARY);

		for (cb = fn->first_cb; cb; cb = cb->next_cb) {
			L_Oper *next_op;
			for (op = cb->first_op; op; op = next_op) {
				next_op = op->next_op;

				if (op->opc == Lop_NO_OP)
				L_delete_oper(cb, op);
				else
				LB_hb_stat.orig_ops++;
			}
		}

		L_do_flow_analysis(fn, DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE);

		L_loop_detection(fn, 0);

		if (LB_hb_verbose_level >= 7) {
			fprintf(stderr, "Initial Loop Detection Phase for (fn %s):\n",
					fn->name);
			L_print_loop_data(fn);
		}

		LB_elim_all_loop_backedges(fn);

		L_delete_unreachable_blocks(fn);

		L_do_flow_analysis(fn, DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE);
		L_reset_loop_headers(fn);
		L_loop_detection(fn, 0);

		if (LB_hb_verbose_level >= 7) {
			fprintf(stderr, "Final Loop Detection Phase for (fn %s):\n",
					fn->name);
			L_print_loop_data(fn);
		}

		L_compute_oper_weight(fn, 0, 1);
		LB_mark_jrg_flag(fn);

		//**********************************************************************

		L_partial_dead_code_removal(fn);
		L_do_flow_analysis(fn,
				DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE | SUPPRESS_PG);

		//fprintf(stderr,"Resid 0\n");
		{
			ILP_cfg_wcet_analyize(fn, "(0).txt");
			Ldot_display_cfg(fn, "t0.dot", 0);
			FILE * file = fopen("rrr.txt", "w");
			L_print_func(file, fn);
			fclose(file);
		}

		for (i = 1; i < 10; i++) {

			//	L_do_flow_analysis(fn,
			//				DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE | SUPPRESS_PG);
			//L_partial_dead_code_removal(fn);
			L_do_flow_analysis(fn,
					DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE | SUPPRESS_PG);
			L_reset_loop_headers(fn);
			L_loop_detection(fn, 0);
			backedge = find_set_backedges(fn);
			headers = find_set_headers(fn);

			find_loops_wcet(fn);
			fprintf(stderr, "QQQQQQQQQQQQQQQQQQQQQQQQQ\n");

			{
				char tp[100];

				sprintf(tp, "(%d).txt", i);
				ILP_cfg_wcet_analyize(fn, tp);

				sprintf(tp, "t%d.dot", i);
				Ldot_display_cfg(fn, tp, 0);

			}
			//fprintf(stderr,"Resid1\n");

			fprintf(stderr, "ROUND (%d)\n", i);

			//
			//			if (i > 1) {
			//
			//				my_trace_formation2(fn, header);
			//
			//				if (header->traceregions==NULL)
			//									break;
			//				my_select_trace_2(fn, header);
			//
			//
			//				if (header->traceregions==NULL)
			//					break;
			//				if(header->traceregions->size<=0)
			//					break;
			//				if(header->traceregions->first==NULL){
			//					header->traceregions=NULL;
			//					break;
			//				}
			//
			//				LB_summarize_traceregions(stderr, header);
			//			}
			//			else{
			//				//pre path hb formation
			//				my_pre_path_hb_formation(fn,header);
			//
			//			}

			trace_formation_SHARIF4(fn, header);
			if (header->traceregions == NULL) break;

			LB_hb_reset_max_oper_id(fn);

			LB_remove_partially_subsumed_traceregions(header);

			LB_remove_conflicting_traceregions(header);

			LB_set_hyperblock_flag(fn);

			LB_set_hyperblock_func_flag(fn);

			//*****************trace selection***************
			//tr=(LB_TraceRegion *)List_get_first(temp_header->traceregions);
			//header->traceregions=List_insert_last(header->traceregions,tr);

			//***********************************************

			LB_summarize_traceregions(stderr, header);

			LB_tail_duplicate(fn, header, LB_DUP_OUTSIDE_REGION);

			{
				L_check_func(fn);
			}

			LB_predicate_traceregions(fn, header);

			LB_set_hyperblock_flag(fn);
			LB_remove_unnec_hyperblock_flags(fn);
			LB_set_hyperblock_func_flag(fn);

			LB_free_all_traceregions(header);

			//temp_header->traceregions = List_reset (temp_header->traceregions);
			//temp_header->inorder_trs = List_reset (temp_header->inorder_trs);

			LB_remove_empty_cbs(fn);

			L_delete_unreachable_blocks(fn);

			LB_convert_to_strict_basic_block_code(fn,
					L_CB_HYPERBLOCK | L_CB_SUPERBLOCK);

			{
				DB_spit_func(fn, "PH1");
				L_check_func(fn);
			}

		}

		//**********************************************************************
		L_do_flow_analysis(fn, DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE);
		L_reset_loop_headers(fn);
		L_loop_detection(fn, 0);
		if (Set_size(find_set_backedges(fn)) != Set_size(backedge))
		fprintf(stderr, "REYHANE\n");

		fprintf(stderr,
				"***************\n**************\n***************\n***************\n");

		{
			//L_partial_dead_code_removal(fn);
			L_do_flow_analysis(fn,
					DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE | SUPPRESS_PG);
			ILP_cfg_wcet_analyize(fn, "(99).txt");
			Ldot_display_cfg(fn, "t99.dot", 0);

			// LB_summarize_traceregions (stderr, header);
		}
		L_do_flow_analysis(fn,
				DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE | SUPPRESS_PG);

		//*******************************************************************

		//******************************************************************

		/*
		 *  Split branches into pred defines and predicated jumps
		 */
		if (LB_hb_branch_split)
		LB_branch_split_func(fn);

		/*
		 *        Generate multiple defn pred defines, generate Uncond pred defines
		 *  (Note initially only OR-type pred defines are created!
		 */

		L_create_uncond_pred_defines(fn);

		PG_setup_pred_graph(fn);
		if (LB_do_lightweight_pred_opti)
		L_lightweight_pred_opti(fn);
		L_combine_pred_defines(fn);

		/*
		 *        remove unnecessary uncond jumps
		 */

		LB_uncond_branch_elim(fn);

		/*
		 *        Merge ops on opposite predicates (partial redundancy elim)
		 */

		L_unmark_all_pre_post_increments(fn);

		if (LB_hb_do_pred_merge) {
			PG_setup_pred_graph(fn);
			L_do_flow_analysis(fn, LIVE_VARIABLE | REACHING_DEFINITION);
			LB_hb_pred_merging(fn);
		}

#ifdef DEBUG_HB_FORMER
		L_check_func(fn);
#endif

		PG_destroy_pred_graph();
		D_delete_dataflow(fn);

		/* For each source code function we process deinit */
		LB_function_deinit(header);

		{
			double rat = 1.0;
			L_Attr *eattr;

			final_ops = 0;

			for (cb = fn->first_cb; cb; cb = cb->next_cb) {
				L_Oper *next_op;
				for (op = cb->first_op; op; op = next_op) {
					next_op = op->next_op;

					if (op->opc != Lop_NO_OP)
					final_ops++;
				}
			}

			if (LB_hb_stat.orig_ops)
			rat = (double) final_ops / LB_hb_stat.orig_ops;

			eattr = L_new_attr("hbe", 2);
			L_set_int_attr_field(eattr, 0, LB_hb_stat.orig_ops);
			L_set_double_attr_field(eattr, 1, rat);
			fn->attr = L_concat_attr(fn->attr, eattr);
		}

		int num_oper = 0;
		for (cb = fn->first_cb; cb; cb = cb->next_cb) {
			L_Oper *next_op;
			for (op = cb->first_op; op; op = next_op) {
				next_op = op->next_op;

				if (op->opc != Lop_NO_OP)

				num_oper++;
			}

		}
		fprintf(stderr, "NUMBER OF OPERATION:(%d)\n", num_oper);

	}

	return;
}

#endif

#ifdef SHARIF4_super

int is_simple_sharif4(L_Cb * first, L_Cb * end, Set loops) {

	if (!Set_same(loops, first->loops)) return 0;
	if (first->id == end->id) return 1;

	L_Flow * flow;
	for (flow = first->dest_flow; flow != NULL; flow = flow->next_flow) {

		if (is_simple_sharif4(flow->dst_cb, end, loops) == 0) return 0;

	}

	return 1;
	//	if(		(first->flags & L_CB_LOOP_HEADER) ||
	//			L_EXTRACT_BIT_VAL(first->flags,L_CB_SUPERBLOCK) ||
	//			L_EXTRACT_BIT_VAL(first->flags,L_CB_HYPERBLOCK) ) return 0;
	//
	//	if(first->id==end->id) return 1;
	//
	//	L_Flow * flow;
	//	for(flow=first->dest_flow;flow!=NULL;flow=flow->next_flow)
	//	{
	//		if(is_simple_sharif4(flow->dst_cb,end)==0) return 0;
	//
	//	}
	//
	//	return 1;

}
//we are sure that chock is simple

void find_chock_blocks_sharif4(L_Cb * first, L_Cb * end, Set blocks) {
	Set_add(blocks, first->id);
	if (first->id == end->id) return;
	L_Flow * flow;
	for (flow = first->dest_flow; flow != NULL; flow = flow->next_flow) {

		find_chock_blocks_sharif4(flow->dst_cb, end, blocks);
	}

}

int is_good(L_Cb * first, L_Cb * last, Set blocks) {
	L_Flow * flow;
	if (!Set_in(blocks, first->id)) return 0;
	if (first->id == last->id) return 1;
	for (flow = first->dest_flow; flow; flow = flow->next_flow) {
		if (flow->is_backedge == 0) {
			if (is_good(flow->dst_cb, last, blocks) == 0) return 0;

		}

	}
	return 1;

}

int is_good2(L_Cb * first, L_Cb * last, Set blocks, L_Func * fn) {

	L_Flow * flow;

	int n = Set_size(blocks);
	int * buf = (int *) Lcode_malloc(sizeof (int) * n);
	Set_2array(blocks, buf);
	int i;
	for (i = 0; i < n; i++) {

		L_Cb * cb = L_cb_hash_tbl_find(fn->cb_hash_tbl, buf[i]);
		if (cb->id != first->id) {
			for (flow = cb->src_flow; flow; flow = flow->next_flow) {
				if (!Set_in(blocks, flow->src_cb->id)) {
					Lcode_free(buf);
					return 0;
				}
			}
		}

	}
	Lcode_free(buf);
	return 1;

}

void visit_sharif4(L_Cb * node, L_Cb * first_node, L_Cb * last_node, Set blocks,
		LB_TraceRegion_Header *header, double wcet, L_Func * fn) {

	int kind = 0;

	if (!Set_in(headers, node->id) && !Set_in(backedge, node->id)) kind = 0;
	else if (Set_in(headers, node->id) && !Set_in(backedge, node->id)) kind = 8;
	else if (!Set_in(headers, node->id) && Set_in(backedge, node->id)) kind = 4;
	else if (Set_in(headers, node->id) && Set_in(backedge, node->id)) kind = 12;

	if (node->is_balance) kind = kind | 2;

	if (node->is_simple) kind = kind | 1;

	if (L_EXTRACT_BIT_VAL(node->flags, L_CB_SUPERBLOCK) || L_EXTRACT_BIT_VAL(node->flags, L_CB_HYPERBLOCK))
	kind = kind | 16;

	if (node->dest_flow == NULL) kind = 32;

	fprintf(stderr, "CB(%d)=%d\n", node->id, kind);

	if ((node->ILP_is_in_wcet_path == 1) && kind != 32) {

		switch (kind) {

			case 0: case 1:
			{ // general_node not_balance (not_simple || simple)
				Set_add(blocks, node->id);

				L_Cb * node2;
				L_Flow * flow;

				for (flow = node->dest_flow; flow; flow = flow->next_flow) {
					if (flow->ILP_is_in_wcet_path) {
						node2 = flow->dst_cb;
						visit_sharif4(node2, first_node, node, blocks, header, wcet + flow->wcet_weight, fn);
					}
				}

				break;
			}
			case 2:
			{ // general_node balance not_simple
				Set_add(blocks, node->id);

				if (Set_size(blocks) > 1) {

					LB_TraceRegion * tr = My_simple_trace_formation(
							header->fn, header, L_TRACEREGION_HAMMOCK,
							first_node, node, blocks,
							header->next_id++);
					tr->wcet2 = wcet;
					header->traceregions = List_insert_last(
							header->traceregions, tr);
				}

				L_Cb * node2 = node->chock_end_cb;
				Set temp = Set_new();
				visit_sharif4(node2, node2, node2, temp, header, 0, fn);
				Set_dispose(temp);

				break;
			}
			case 3:
			{ // general_node balance simple
				Set_add(blocks, node->id);

				if (Set_size(blocks) > 1) {

					LB_TraceRegion * tr = My_simple_trace_formation(
							header->fn, header, L_TRACEREGION_HAMMOCK,
							first_node, node, blocks,
							header->next_id++);
					tr->wcet2 = wcet;
					header->traceregions = List_insert_last(
							header->traceregions, tr);
				}

				L_Cb * node2 = node->chock_end_cb;
				Set temp = Set_new();
				visit_sharif4(node2, node2, node2, temp, header, 0, fn);
				Set_dispose(temp);

				break;
			}
			case 8: case 9:
			{ // header_node not_balance (not_simple || simple)
				if (Set_size(blocks) > 1) {

					LB_TraceRegion * tr = My_simple_trace_formation(
							header->fn, header, L_TRACEREGION_HAMMOCK,
							first_node, last_node, blocks,
							header->next_id++);
					tr->wcet2 = wcet;
					header->traceregions = List_insert_last(
							header->traceregions, tr);
				}

				Set temp = Set_new();
				Set_add(temp, node->id);

				L_Cb * node2;
				L_Flow * flow;

				for (flow = node->dest_flow; flow; flow = flow->next_flow) {
					if (flow->ILP_is_in_wcet_path) {
						node2 = flow->dst_cb;
						visit_sharif4(node2, node, node, temp, header, wcet + flow->wcet_weight, fn);
					}
				}

				break;
			}
			case 10:
			{ // header_node balance not_simple
				if (Set_size(blocks) > 1) {

					LB_TraceRegion * tr = My_simple_trace_formation(
							header->fn, header, L_TRACEREGION_HAMMOCK,
							first_node, last_node, blocks,
							header->next_id++);
					tr->wcet2 = wcet;
					header->traceregions = List_insert_last(
							header->traceregions, tr);
				}

				L_Cb * node2 = node->chock_end_cb;
				Set temp = Set_new();
				visit_sharif4(node2, node2, node2, temp, header, 0, fn);
				Set_dispose(temp);

				break;
			}
			case 11:
			{ // header_node balance simple
				if (Set_size(blocks) > 1) {

					LB_TraceRegion * tr = My_simple_trace_formation(
							header->fn, header, L_TRACEREGION_HAMMOCK,
							first_node, last_node, blocks,
							header->next_id++);
					tr->wcet2 = wcet;
					header->traceregions = List_insert_last(
							header->traceregions, tr);
				}

				//				if(node->id!=4){
				L_Cb * node2 = node->chock_end_cb;
				Set temp = Set_new();
				visit_sharif4(node2, node2, node2, temp, header, 0, fn);
				Set_dispose(temp);
				//				}
				//				else{
				//					L_Cb * node2=L_cb_hash_tbl_find(fn->cb_hash_tbl, 8);
				//					Set temp=Set_new();
				//					Set_add(temp,node->id);
				//					visit_sharif4(node2, node, node, temp, header,0,fn);
				//					Set_dispose(temp);
				//
				//
				//				}

				break;
			}
			case 12: case 13: case 14: case 15:
			{ // end_node && header node
				if (Set_size(blocks) > 1) {

					LB_TraceRegion * tr = My_simple_trace_formation(
							header->fn, header, L_TRACEREGION_HAMMOCK,
							first_node, last_node, blocks,
							header->next_id++);
					tr->wcet2 = wcet;
					header->traceregions = List_insert_last(
							header->traceregions, tr);
				}

				Set temp = Set_new();

				L_Cb * node2;
				L_Flow * flow;

				for (flow = node->dest_flow; flow; flow = flow->next_flow) {
					if (flow->ILP_is_in_wcet_path && (flow->is_backedge == 0)) {
						node2 = flow->dst_cb;
						visit_sharif4(node2, node2, node2, temp, header, wcet + flow->wcet_weight, fn);
					}
				}

				Set_dispose(temp);
				break;
			}
			case 4: case 5: case 6: case 7:
			{
				Set_add(blocks, node->id);
				//fprintf(stderr,"VG(%d)(%d)=(%d)\n",first_node->id,node->id,is_good(first_node,node,blocks));

				if (is_good2(first_node, node, blocks, fn) == 0) blocks = Set_delete(blocks, node->id);
				else last_node = node;

				if (Set_size(blocks) > 1) {

					LB_TraceRegion * tr = My_simple_trace_formation(
							header->fn, header, L_TRACEREGION_HAMMOCK,
							first_node, last_node, blocks,
							header->next_id++);
					tr->wcet2 = wcet;
					header->traceregions = List_insert_last(
							header->traceregions, tr);
				}

				Set temp = Set_new();

				L_Cb * node2;
				L_Flow * flow;

				for (flow = node->dest_flow; flow; flow = flow->next_flow) {
					if (flow->ILP_is_in_wcet_path && (flow->is_backedge == 0)) {
						node2 = flow->dst_cb;
						visit_sharif4(node2, node2, node2, temp, header, wcet + flow->wcet_weight, fn);
					}
				}

				Set_dispose(temp);

				break;
			}
			default:
			{ //super_block || hyper_block
				if (Set_size(blocks) > 1) {

					LB_TraceRegion * tr = My_simple_trace_formation(
							header->fn, header, L_TRACEREGION_HAMMOCK,
							first_node, last_node, blocks,
							header->next_id++);
					tr->wcet2 = wcet;
					header->traceregions = List_insert_last(
							header->traceregions, tr);
				}

				Set temp = Set_new();

				L_Cb * node2;
				L_Flow * flow;

				for (flow = node->dest_flow; flow; flow = flow->next_flow) {
					if (flow->ILP_is_in_wcet_path && (flow->is_backedge == 0)) {
						node2 = flow->dst_cb;
						visit_sharif4(node2, node2, node2, temp, header, wcet + flow->wcet_weight, fn);
					}
				}
				Set_dispose(temp);

				break;
			}
		}

	} else {
		if (Set_size(blocks) > 1) {

			LB_TraceRegion * tr = My_simple_trace_formation(
					header->fn, header, L_TRACEREGION_HAMMOCK,
					first_node, last_node, blocks,
					header->next_id++);
			tr->wcet2 = wcet;
			header->traceregions = List_insert_last(
					header->traceregions, tr);
		}

	}
	return;
}

void trace_formation_SHARIF4(L_Func * fn, LB_TraceRegion_Header *header) {

	find_loops_wcet(fn);
	Graph bb_g = L_create_cb_graph(fn);
	Graph_dominator(bb_g);
	Graph_post_dominator(bb_g);
	Graph_imm_dominator(bb_g);
	Graph_imm_post_dominator(bb_g);
	Graph_control_dependence(bb_g);
	Graph_topological_sort(bb_g);
	Graph_preorder_dfs_sort(bb_g, 1);
	Graph_preorder_dfs_visit(bb_g, bb_g->root, 1);

	backedge = find_set_backedges(fn);
	headers = find_set_headers(fn);
	List_start(bb_g->topo_list);
	List_next(bb_g->topo_list);

	L_Cb * cb;
	GraphNode nodeg;
	L_Cb * node;
	L_Flow * flow;

	List_start(bb_g->nodes);

	while ((nodeg = (GraphNode) List_next(bb_g->nodes))) {

		//fprintf(stderr, "FF(%d)\n",nodeg->id);
		GraphNode tem = get_imm_post_domin(bb_g->topo_list, nodeg);

		if (tem != NULL)
		((L_Cb *) (nodeg->ptr))->chock_end_cb = (L_Cb *) (tem->ptr);
		//fprintf(stderr,"OOOOOOOOOOOOOOOOOOO\n");
		//fprintf(stderr,"CHOCK(%d,%d)",node->id,node->chock_end_cb->id);

	}
	int num_total_branch = 0;
	int num_balance_branch = 0;

	L_Loop * loop;
	int * buf;
	int * buf2;
	for (loop = fn->first_loop; loop; loop = loop->next_loop) {

		int size = Set_size(loop->back_edge_cb);
		buf = (int *) malloc(sizeof (int) * size);
		Set_2array(loop->back_edge_cb, buf);
		int i;
		//Set_print(stderr,"WWW",loop->back_edge_cb);
		for (i = 0; i < size; i++) {
			L_Cb * end_node = L_cb_hash_tbl_find(fn->cb_hash_tbl, buf[i]);
			flow = end_node->dest_flow;
			while (flow->dst_cb->id != loop->header->id) flow = flow->next_flow;
			flow->is_backedge = 1;
			//fprintf(stderr,"www(%d)->(%d)\n",flow->src_cb->id,flow->dst_cb->id);
		}

		Lcode_free(buf);

		int size2 = Set_size(loop->loop_cb);
		buf2 = (int *) malloc(sizeof (int) * size2);
		Set_2array(loop->loop_cb, buf2);
		for (i = 0; i < size2; i++) {
			node = L_cb_hash_tbl_find(fn->cb_hash_tbl, buf2[i]);
			if (node->loops == NULL) node->loops = Set_new();
			Set_add(node->loops, loop->id);
			//fprintf(stderr,"%d\t%d",node->id,loop->id); Set_print(stderr,"",node->loops);
		}

		Lcode_free(buf2);

	}

	for (node = fn->first_cb; node; node = node->next_cb) {
		double max = -100;
		int num_flow = 0;
		fprintf(stderr, "%d\n", node->id);
		//Set_print(stderr,"NNNNN",node->loops);
		//fprintf(stderr,"MORTY(%d)\n",node->id);
		for (flow = node->dest_flow; flow; flow = flow->next_flow) {

			if (flow->is_backedge == 0) {
				num_flow++;

				//	fprintf(stderr,"FLOW(%d)\n",flow->dst_cb->id);
				flow->wcet_until_chock = find_max_weight_of_paths(flow->dst_cb, node->chock_end_cb) + flow->wcet_weight;

				if (flow->wcet_until_chock > max) max = flow->wcet_until_chock;

				flow->is_simple = is_simple_sharif4(flow->dst_cb, node->chock_end_cb, node->loops);
			}

			//fprintf(stderr,"MMMMMMMMMM(%d)(%d)\n",flow->dst_cb->id,flow->is_simple);
		}
		//fprintf(stderr,"KKKKKKKKKKKKKKKKKKKK(%d)\n",node->id);
		node->wcet_until_chock = max;

		node->is_balance = 0;
		node->is_simple = 1;

		if (num_flow >= 2) {
			num_total_branch++;
			node->is_balance = 1;
			node->is_simple = 1;
			for (flow = node->dest_flow; flow; flow = flow->next_flow) {
				if ((max - 1) > flow->wcet_until_chock) {
					node->is_balance = 0;
				}
				if (flow->is_simple == 0) node->is_simple = 0;
			}

		}

		if (Set_in(backedge, node->id)) node->is_simple = 0;

		if (node->is_simple == 1 && node->is_balance == 1) {
			Set blocks = Set_new();
			find_chock_blocks_sharif4(node, node->chock_end_cb, blocks);
			node->chock_middle_nodes = blocks;
			//Set_dispose(blocks);
			//fprintf(stderr,"GOOOD:(%d)->(%d)",node->id,node->chock_end_cb->id);
			//Set_print(stderr,"",node->chock_middle_nodes);
		}

		fprintf(stderr, "node(%d)=%d\t%d", node->id, node->is_simple, node->is_balance);
		Set_print(stderr, "", node->chock_middle_nodes);

		//		if(node->chock_end_cb){
		//
		//			flow=node->chock_end_cb->src_flow;
		//			int num_src=0;
		//
		//			while(flow!=NULL){ flow=flow->next_flow; num_src++;}
		//
		//			if(Set_in(backedge,node->chock_end_cb->id) && node->is_balance==0 &&
		//					num_src>1 && Set_in(node->chock_end_cb->dom,node->id) && Set_same(node->loops,node->chock_end_cb->loops)){
		//				fprintf(stderr,"Hesam:(%d)(%d)\n",node->id,node->chock_end_cb->id);
		//			}
		//		}
	}

	fprintf(stderr, "WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWww\n");

	List_start(bb_g->topo_list);
	List_next(bb_g->topo_list);
	nodeg = (GraphNode) List_next(bb_g->topo_list);
	node = (L_Cb *) (nodeg->ptr);
	visit_sharif4(node, node, node, Set_new(), header, 0, fn);

	Graph_free_graph(bb_g);
}

void LB_hyperblock_formation(L_Func * fn) {

	//fprintf(stderr,"QQQQQQQQQQQQQQQQQQQQQQQ\nwwwwwwwwwwwwwwwwwwwwwwwww\nDDDDDDDDDDDDDDDDDDDDDDDDD\n");

	if (strcmp(fn->name, "_main") == 0) {
		LB_TraceRegion_Header *header;
		LB_TraceRegion *tr;
		L_Cb *cb;
		L_Oper *op;
		int do_peel, final_ops;
		if (fn->n_cb == 0)
		return;
		int i;

		memset(&LB_hb_stat, 0, sizeof (LB_HB_Stat));

		LB_split_exit_block(fn);

		header = LB_function_init(fn);

		//temp_header = LB_function_init(fn);

		L_breakup_pre_post_inc_ops(fn);

		LB_clr_hyperblock_flag(fn);

		L_compute_oper_weight(fn, 0, 1);
		/* make sure all cbs contain at most 2 targets */

		LB_convert_to_strict_basic_block_code(
				fn,
				L_CB_SUPERBLOCK | L_CB_HYPERBLOCK | L_CB_ENTRANCE_BOUNDARY
				| L_CB_EXIT_BOUNDARY);

		for (cb = fn->first_cb; cb; cb = cb->next_cb) {
			L_Oper *next_op;
			for (op = cb->first_op; op; op = next_op) {
				next_op = op->next_op;

				if (op->opc == Lop_NO_OP)
				L_delete_oper(cb, op);
				else
				LB_hb_stat.orig_ops++;
			}
		}

		L_do_flow_analysis(fn, DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE);

		L_loop_detection(fn, 0);

		if (LB_hb_verbose_level >= 7) {
			fprintf(stderr, "Initial Loop Detection Phase for (fn %s):\n",
					fn->name);
			L_print_loop_data(fn);
		}

		LB_elim_all_loop_backedges(fn);

		L_delete_unreachable_blocks(fn);

		L_do_flow_analysis(fn, DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE);
		L_reset_loop_headers(fn);
		L_loop_detection(fn, 0);

		if (LB_hb_verbose_level >= 7) {
			fprintf(stderr, "Final Loop Detection Phase for (fn %s):\n",
					fn->name);
			L_print_loop_data(fn);
		}

		L_compute_oper_weight(fn, 0, 1);
		LB_mark_jrg_flag(fn);

		//**********************************************************************

		L_partial_dead_code_removal(fn);
		L_do_flow_analysis(fn,
				DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE | SUPPRESS_PG);

		//fprintf(stderr,"Resid 0\n");
		{
			ILP_cfg_wcet_analyize(fn, "(0).txt");
			Ldot_display_cfg(fn, "t0.dot", 0);
			FILE * file = fopen("rrr.txt", "w");
			L_print_func(file, fn);
			fclose(file);
		}

		for (i = 1; i < 10; i++) {

			//	L_do_flow_analysis(fn,
			//				DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE | SUPPRESS_PG);
			//L_partial_dead_code_removal(fn);
			L_do_flow_analysis(fn,
					DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE | SUPPRESS_PG);
			L_reset_loop_headers(fn);
			L_loop_detection(fn, 0);
			backedge = find_set_backedges(fn);
			headers = find_set_headers(fn);

			find_loops_wcet(fn);
			fprintf(stderr, "QQQQQQQQQQQQQQQQQQQQQQQQQ\n");

			{
				char tp[100];

				sprintf(tp, "(%d).txt", i);
				ILP_cfg_wcet_analyize(fn, tp);

				sprintf(tp, "t%d.dot", i);
				Ldot_display_cfg(fn, tp, 0);

			}
			//fprintf(stderr,"Resid1\n");

			fprintf(stderr, "ROUND (%d)\n", i);

			//
			//			if (i > 1) {
			//
			//				my_trace_formation2(fn, header);
			//
			//				if (header->traceregions==NULL)
			//									break;
			//				my_select_trace_2(fn, header);
			//
			//
			//				if (header->traceregions==NULL)
			//					break;
			//				if(header->traceregions->size<=0)
			//					break;
			//				if(header->traceregions->first==NULL){
			//					header->traceregions=NULL;
			//					break;
			//				}
			//
			//				LB_summarize_traceregions(stderr, header);
			//			}
			//			else{
			//				//pre path hb formation
			//				my_pre_path_hb_formation(fn,header);
			//
			//			}

			trace_formation_SHARIF4(fn, header);
			if (header->traceregions == NULL) break;

			LB_hb_reset_max_oper_id(fn);

			LB_remove_partially_subsumed_traceregions(header);

			LB_remove_conflicting_traceregions(header);

			LB_set_hyperblock_flag(fn);

			LB_set_hyperblock_func_flag(fn);

			//*****************trace selection***************
			//tr=(LB_TraceRegion *)List_get_first(temp_header->traceregions);
			//header->traceregions=List_insert_last(header->traceregions,tr);

			//***********************************************

			LB_summarize_traceregions(stderr, header);

			LB_tail_duplicate(fn, header, LB_DUP_OUTSIDE_REGION);

			{
				L_check_func(fn);
			}

			LB_predicate_traceregions(fn, header);

			LB_set_hyperblock_flag(fn);
			LB_remove_unnec_hyperblock_flags(fn);
			LB_set_hyperblock_func_flag(fn);

			LB_free_all_traceregions(header);

			//temp_header->traceregions = List_reset (temp_header->traceregions);
			//temp_header->inorder_trs = List_reset (temp_header->inorder_trs);

			LB_remove_empty_cbs(fn);

			L_delete_unreachable_blocks(fn);

			LB_convert_to_strict_basic_block_code(fn,
					L_CB_HYPERBLOCK | L_CB_SUPERBLOCK);

			{
				DB_spit_func(fn, "PH1");
				L_check_func(fn);
			}

		}

		//**********************************************************************
		L_do_flow_analysis(fn, DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE);
		L_reset_loop_headers(fn);
		L_loop_detection(fn, 0);
		if (Set_size(find_set_backedges(fn)) != Set_size(backedge))
		fprintf(stderr, "REYHANE\n");

		fprintf(stderr,
				"***************\n**************\n***************\n***************\n");

		{
			//L_partial_dead_code_removal(fn);
			L_do_flow_analysis(fn,
					DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE | SUPPRESS_PG);
			ILP_cfg_wcet_analyize(fn, "(99).txt");
			Ldot_display_cfg(fn, "t99.dot", 0);

			// LB_summarize_traceregions (stderr, header);
		}
		L_do_flow_analysis(fn,
				DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE | SUPPRESS_PG);

		//*******************************************************************

		//******************************************************************

		/*
		 *  Split branches into pred defines and predicated jumps
		 */
		if (LB_hb_branch_split)
		LB_branch_split_func(fn);

		/*
		 *        Generate multiple defn pred defines, generate Uncond pred defines
		 *  (Note initially only OR-type pred defines are created!
		 */

		L_create_uncond_pred_defines(fn);

		PG_setup_pred_graph(fn);
		if (LB_do_lightweight_pred_opti)
		L_lightweight_pred_opti(fn);
		L_combine_pred_defines(fn);

		/*
		 *        remove unnecessary uncond jumps
		 */

		LB_uncond_branch_elim(fn);

		/*
		 *        Merge ops on opposite predicates (partial redundancy elim)
		 */

		L_unmark_all_pre_post_increments(fn);

		if (LB_hb_do_pred_merge) {
			PG_setup_pred_graph(fn);
			L_do_flow_analysis(fn, LIVE_VARIABLE | REACHING_DEFINITION);
			LB_hb_pred_merging(fn);
		}

#ifdef DEBUG_HB_FORMER
		L_check_func(fn);
#endif

		PG_destroy_pred_graph();
		D_delete_dataflow(fn);

		/* For each source code function we process deinit */
		LB_function_deinit(header);

		{
			double rat = 1.0;
			L_Attr *eattr;

			final_ops = 0;

			for (cb = fn->first_cb; cb; cb = cb->next_cb) {
				L_Oper *next_op;
				for (op = cb->first_op; op; op = next_op) {
					next_op = op->next_op;

					if (op->opc != Lop_NO_OP)
					final_ops++;
				}
			}

			if (LB_hb_stat.orig_ops)
			rat = (double) final_ops / LB_hb_stat.orig_ops;

			eattr = L_new_attr("hbe", 2);
			L_set_int_attr_field(eattr, 0, LB_hb_stat.orig_ops);
			L_set_double_attr_field(eattr, 1, rat);
			fn->attr = L_concat_attr(fn->attr, eattr);
		}

		int num_oper = 0;
		for (cb = fn->first_cb; cb; cb = cb->next_cb) {
			L_Oper *next_op;
			for (op = cb->first_op; op; op = next_op) {
				next_op = op->next_op;

				if (op->opc != Lop_NO_OP)

				num_oper++;
			}

		}
		fprintf(stderr, "NUMBER OF OPERATION:(%d)\n", num_oper);

	}

	return;
}

#endif

#ifdef SHARIF4_hyper

int is_good2(L_Cb * first, L_Cb * last, Set blocks, L_Func * fn) {

	L_Flow * flow;

	int n = Set_size(blocks);
	int * buf = (int *) Lcode_malloc(sizeof(int) * n);
	Set_2array(blocks, buf);
	int i;
	for (i = 0; i < n; i++) {

		L_Cb * cb = L_cb_hash_tbl_find(fn->cb_hash_tbl, buf[i]);
		if (cb->id != first->id) {
			for (flow = cb->src_flow; flow; flow = flow->next_flow) {
				if (!Set_in(blocks, flow->src_cb->id)) {
					Lcode_free(buf);
					return 0;
				}
			}
		}

	}
	Lcode_free(buf);
	return 1;

}

int is_simple_sharif4(L_Cb * first, L_Cb * end, Set loops) {

	if (!Set_same(loops, first->loops))
		return 0;
	if (first->id == end->id)
		return 1;

	L_Flow * flow;
	for (flow = first->dest_flow; flow != NULL; flow = flow->next_flow) {

		if (is_simple_sharif4(flow->dst_cb, end, loops) == 0)
			return 0;

	}

	return 1;
	//	if(		(first->flags & L_CB_LOOP_HEADER) ||
	//			L_EXTRACT_BIT_VAL(first->flags,L_CB_SUPERBLOCK) ||
	//			L_EXTRACT_BIT_VAL(first->flags,L_CB_HYPERBLOCK) ) return 0;
	//
	//	if(first->id==end->id) return 1;
	//
	//	L_Flow * flow;
	//	for(flow=first->dest_flow;flow!=NULL;flow=flow->next_flow)
	//	{
	//		if(is_simple_sharif4(flow->dst_cb,end)==0) return 0;
	//
	//	}
	//
	//	return 1;

}
//we are sure that chock is simple

void find_chock_blocks_sharif4(L_Cb * first, L_Cb * end, Set blocks) {
	Set_add(blocks, first->id);
	if (first->id == end->id)
		return;
	L_Flow * flow;
	for (flow = first->dest_flow; flow != NULL; flow = flow->next_flow) {

		find_chock_blocks_sharif4(flow->dst_cb, end, blocks);
	}

}

void visit_sharif4(L_Cb * node, L_Cb * first_node, L_Cb * last_node, Set blocks,
		LB_TraceRegion_Header *header, double wcet, L_Func * fn) {

	int kind = 0;

	if (!Set_in(headers, node->id) && !Set_in(backedge, node->id))
		kind = 0;
	else if (Set_in(headers, node->id) && !Set_in(backedge, node->id))
		kind = 8;
	else if (!Set_in(headers, node->id) && Set_in(backedge, node->id))
		kind = 4;
	else if (Set_in(headers, node->id) && Set_in(backedge, node->id))
		kind = 12;

	if (node->is_balance)
		kind = kind | 2;

	if (node->is_simple)
		kind = kind | 1;

	if (L_EXTRACT_BIT_VAL(node->flags,
			L_CB_SUPERBLOCK) || L_EXTRACT_BIT_VAL(node->flags, L_CB_HYPERBLOCK))
		kind = kind | 16;

	if (node->dest_flow == NULL)
		kind = 32;

	fprintf(stderr, "CB(%d)=%d\n", node->id, kind);

	if ((node->ILP_is_in_wcet_path == 1) && kind != 32) {

		switch (kind) {

		case 0: { // general_node not_balance not_simple
			Set_add(blocks, node->id);

			L_Cb * node2;
			L_Flow * flow;

			for (flow = node->dest_flow; flow; flow = flow->next_flow) {
				if (flow->ILP_is_in_wcet_path) {
					node2 = flow->dst_cb;
					visit_sharif4(node2, first_node, node, blocks, header,
							wcet + flow->wcet_weight, fn);
				}
			}

			break;
		}
		case 2: { // general_node balance not_simple
			Set_add(blocks, node->id);

			if (Set_size(blocks) > 1) {

				LB_TraceRegion * tr = My_simple_trace_formation(header->fn,
						header, L_TRACEREGION_HAMMOCK, first_node, node, blocks,
						header->next_id++);
				tr->wcet2 = wcet;
				header->traceregions = List_insert_last(header->traceregions,
						tr);
			}

			L_Cb * node2 = node->chock_end_cb;
			Set temp = Set_new();
			visit_sharif4(node2, node2, node2, temp, header, 0, fn);
			Set_dispose(temp);

			break;
		}
		case 3:
		case 1: { // general_node balance simple
			blocks = Set_union(blocks, node->chock_middle_nodes);
			L_Cb * node2 = node->chock_end_cb;
			visit_sharif4(node2, first_node, node2, blocks, header,
					wcet + node->wcet_until_chock, fn);

			break;
		}
		case 8:
		case 9: { // header_node not_balance (not_simple || simple)
			if (Set_size(blocks) > 1) {

				LB_TraceRegion * tr = My_simple_trace_formation(header->fn,
						header, L_TRACEREGION_HAMMOCK, first_node, last_node,
						blocks, header->next_id++);
				tr->wcet2 = wcet;
				header->traceregions = List_insert_last(header->traceregions,
						tr);
			}

			Set temp = Set_new();
			Set_add(temp, node->id);

			L_Cb * node2;
			L_Flow * flow;

			for (flow = node->dest_flow; flow; flow = flow->next_flow) {
				if (flow->ILP_is_in_wcet_path) {
					node2 = flow->dst_cb;
					visit_sharif4(node2, node, node, temp, header,
							wcet + flow->wcet_weight, fn);
				}
			}

			break;
		}
		case 10: { // header_node balance not_simple
			if (Set_size(blocks) > 1) {

				LB_TraceRegion * tr = My_simple_trace_formation(header->fn,
						header, L_TRACEREGION_HAMMOCK, first_node, last_node,
						blocks, header->next_id++);
				tr->wcet2 = wcet;
				header->traceregions = List_insert_last(header->traceregions,
						tr);
			}

			L_Cb * node2 = node->chock_end_cb;
			Set temp = Set_new();
			visit_sharif4(node2, node2, node2, temp, header, 0, fn);
			Set_dispose(temp);

			break;
		}
		case 11: { // header_node balance simple
			if (Set_size(blocks) > 1) {

				LB_TraceRegion * tr = My_simple_trace_formation(header->fn,
						header, L_TRACEREGION_HAMMOCK, first_node, last_node,
						blocks, header->next_id++);
				tr->wcet2 = wcet;
				header->traceregions = List_insert_last(header->traceregions,
						tr);
			}
			Set temp = Set_new();
			temp = Set_union(temp, node->chock_middle_nodes);
			L_Cb * node2 = node->chock_end_cb;
			visit_sharif4(node2, node, node2, temp, header,
					wcet + node->wcet_until_chock, fn);
			Set_dispose(temp);

			break;
		}
		case 12:
		case 13:
		case 14:
		case 15: { // end_node && header node
			if (Set_size(blocks) > 1) {

				LB_TraceRegion * tr = My_simple_trace_formation(header->fn,
						header, L_TRACEREGION_HAMMOCK, first_node, last_node,
						blocks, header->next_id++);
				tr->wcet2 = wcet;
				header->traceregions = List_insert_last(header->traceregions,
						tr);
			}

			Set temp = Set_new();

			L_Cb * node2;
			L_Flow * flow;

			for (flow = node->dest_flow; flow; flow = flow->next_flow) {
				if (flow->ILP_is_in_wcet_path && (flow->is_backedge == 0)) {
					node2 = flow->dst_cb;
					visit_sharif4(node2, node2, node2, temp, header,
							wcet + flow->wcet_weight, fn);
				}
			}

			Set_dispose(temp);
			break;
		}
		case 4:
		case 5:
		case 6:
		case 7: {
			Set_add(blocks, node->id);
			//fprintf(stderr,"VG(%d)(%d)=(%d)\n",first_node->id,node->id,is_good(first_node,node,blocks));
			if (is_good2(first_node, node, blocks, fn) == 0)
				blocks = Set_delete(blocks, node->id);
			else
				last_node = node;
			if (Set_size(blocks) > 1) {

				LB_TraceRegion * tr = My_simple_trace_formation(header->fn,
						header, L_TRACEREGION_HAMMOCK, first_node, last_node,
						blocks, header->next_id++);
				tr->wcet2 = wcet;
				header->traceregions = List_insert_last(header->traceregions,
						tr);
			}

			Set temp = Set_new();

			L_Cb * node2;
			L_Flow * flow;

			for (flow = node->dest_flow; flow; flow = flow->next_flow) {
				if (flow->ILP_is_in_wcet_path && (flow->is_backedge == 0)) {
					node2 = flow->dst_cb;
					visit_sharif4(node2, node2, node2, temp, header,
							wcet + flow->wcet_weight, fn);
				}
			}

			Set_dispose(temp);

			break;
		}
		default: { //super_block || hyper_block
			if (Set_size(blocks) > 1) {

				LB_TraceRegion * tr = My_simple_trace_formation(header->fn,
						header, L_TRACEREGION_HAMMOCK, first_node, last_node,
						blocks, header->next_id++);
				tr->wcet2 = wcet;
				header->traceregions = List_insert_last(header->traceregions,
						tr);
			}

			Set temp = Set_new();

			L_Cb * node2;
			L_Flow * flow;

			for (flow = node->dest_flow; flow; flow = flow->next_flow) {
				if (flow->ILP_is_in_wcet_path && (flow->is_backedge == 0)) {
					node2 = flow->dst_cb;
					visit_sharif4(node2, node2, node2, temp, header,
							wcet + flow->wcet_weight, fn);
				}
			}
			Set_dispose(temp);

			break;
		}
		}

	} else {
		if (Set_size(blocks) > 1) {

			LB_TraceRegion * tr = My_simple_trace_formation(header->fn, header,
					L_TRACEREGION_HAMMOCK, first_node, last_node, blocks,
					header->next_id++);
			tr->wcet2 = wcet;
			header->traceregions = List_insert_last(header->traceregions, tr);
		}

	}
	return;
}

void trace_formation_SHARIF4(L_Func * fn, LB_TraceRegion_Header *header) {

	find_loops_wcet(fn);
	Graph bb_g = L_create_cb_graph(fn);
	Graph_dominator(bb_g);
	Graph_post_dominator(bb_g);
	Graph_imm_dominator(bb_g);
	Graph_imm_post_dominator(bb_g);
	Graph_control_dependence(bb_g);
	Graph_topological_sort(bb_g);
	Graph_preorder_dfs_sort(bb_g, 1);
	Graph_preorder_dfs_visit(bb_g, bb_g->root, 1);

	backedge = find_set_backedges(fn);
	headers = find_set_headers(fn);
	List_start(bb_g->topo_list);
	List_next(bb_g->topo_list);

	L_Cb * cb;
	GraphNode nodeg;
	L_Cb * node;
	L_Flow * flow;

	List_start(bb_g->nodes);

	while ((nodeg = (GraphNode) List_next(bb_g->nodes))) {

		//fprintf(stderr, "FF(%d)\n",nodeg->id);
		GraphNode tem = get_imm_post_domin(bb_g->topo_list, nodeg);

		if (tem != NULL)
			((L_Cb *) (nodeg->ptr))->chock_end_cb = (L_Cb *) (tem->ptr);
		//fprintf(stderr,"OOOOOOOOOOOOOOOOOOO\n");
		//fprintf(stderr,"CHOCK(%d,%d)",node->id,node->chock_end_cb->id);

	}
	int num_total_branch = 0;
	int num_balance_branch = 0;

	L_Loop * loop;
	int * buf;
	int * buf2;
	for (loop = fn->first_loop; loop; loop = loop->next_loop) {

		int size = Set_size(loop->back_edge_cb);
		buf = (int *) malloc(sizeof(int) * size);
		Set_2array(loop->back_edge_cb, buf);
		int i;
		//Set_print(stderr,"WWW",loop->back_edge_cb);
		for (i = 0; i < size; i++) {
			L_Cb * end_node = L_cb_hash_tbl_find(fn->cb_hash_tbl, buf[i]);
			flow = end_node->dest_flow;
			while (flow->dst_cb->id != loop->header->id)
				flow = flow->next_flow;
			flow->is_backedge = 1;
			//fprintf(stderr,"www(%d)->(%d)\n",flow->src_cb->id,flow->dst_cb->id);
		}

		Lcode_free(buf);

		int size2 = Set_size(loop->loop_cb);
		buf2 = (int *) malloc(sizeof(int) * size2);
		Set_2array(loop->loop_cb, buf2);
		for (i = 0; i < size2; i++) {
			node = L_cb_hash_tbl_find(fn->cb_hash_tbl, buf2[i]);
			if (node->loops == 0) {
				fprintf(stderr, "node->loops==null");
				node->loops = Set_new();
			}
			//node->loops=0;
			if(Set_empty(node->loops))
				node->loops=0;
			fprintf(stderr, "mkt %d\t%d", node->id, loop->id);
			Set_print(stderr, "", node->loops);
			Set_add(node->loops, loop->id);
		}

		Lcode_free(buf2);

	}

	for (node = fn->first_cb; node; node = node->next_cb) {
		double max = -100;
		int num_flow = 0;
		fprintf(stderr, "%d\n", node->id);
		//Set_print(stderr,"NNNNN",node->loops);
		//fprintf(stderr,"MORTY(%d)\n",node->id);
		for (flow = node->dest_flow; flow; flow = flow->next_flow) {

			if (flow->is_backedge == 0) {
				num_flow++;

				//	fprintf(stderr,"FLOW(%d)\n",flow->dst_cb->id);
				flow->wcet_until_chock = find_max_weight_of_paths(flow->dst_cb,
						node->chock_end_cb) + flow->wcet_weight;

				if (flow->wcet_until_chock > max)
					max = flow->wcet_until_chock;

				flow->is_simple = is_simple_sharif4(flow->dst_cb,
						node->chock_end_cb, node->loops);
			}

			//fprintf(stderr,"MMMMMMMMMM(%d)(%d)\n",flow->dst_cb->id,flow->is_simple);
		}
		//fprintf(stderr,"KKKKKKKKKKKKKKKKKKKK(%d)\n",node->id);
		node->wcet_until_chock = max;

		node->is_balance = 0;
		node->is_simple = 1;

		if (num_flow >= 2) {
			num_total_branch++;
			node->is_balance = 1;
			node->is_simple = 1;
			for (flow = node->dest_flow; flow; flow = flow->next_flow) {
				if ((max - 1) > flow->wcet_until_chock) {
					node->is_balance = 0;
				}
				if (flow->is_simple == 0)
					node->is_simple = 0;
			}

		}

		if (Set_in(backedge, node->id))
			node->is_simple = 0;

		if (node->is_simple == 1 && node->is_balance == 1) {
			Set blocks = Set_new();
			find_chock_blocks_sharif4(node, node->chock_end_cb, blocks);
			node->chock_middle_nodes = blocks;
			//Set_dispose(blocks);
			//fprintf(stderr,"GOOOD:(%d)->(%d)",node->id,node->chock_end_cb->id);
			//Set_print(stderr,"",node->chock_middle_nodes);
		}

		fprintf(stderr, "node(%d)=%d\t%d", node->id, node->is_simple,
				node->is_balance);
		Set_print(stderr, "", node->chock_middle_nodes);
	}

	fprintf(stderr, "WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWww\n");

	List_start(bb_g->topo_list);
	List_next(bb_g->topo_list);
	nodeg = (GraphNode) List_next(bb_g->topo_list);
	node = (L_Cb *) (nodeg->ptr);
	visit_sharif4(node, node, node, Set_new(), header, 0, fn);

	Graph_free_graph(bb_g);
}

void LB_hyperblock_formation_WCET(L_Func * fn) {

	//fprintf(stderr,"QQQQQQQQQQQQQQQQQQQQQQQ\nwwwwwwwwwwwwwwwwwwwwwwwww\nDDDDDDDDDDDDDDDDDDDDDDDDD\n");

	if (strcmp(fn->name, "_main") == 0) {
		LB_TraceRegion_Header *header;
		LB_TraceRegion *tr;
		L_Cb *cb;
		L_Oper *op;
		int do_peel, final_ops;
		if (fn->n_cb == 0)
			return;
		int i;

		memset(&LB_hb_stat, 0, sizeof(LB_HB_Stat));

		LB_split_exit_block(fn);

		header = LB_function_init(fn);

		//temp_header = LB_function_init(fn);

		L_breakup_pre_post_inc_ops(fn);

		LB_clr_hyperblock_flag(fn);

		L_compute_oper_weight(fn, 0, 1);
		/* make sure all cbs contain at most 2 targets */

		LB_convert_to_strict_basic_block_code(fn,
				L_CB_SUPERBLOCK | L_CB_HYPERBLOCK | L_CB_ENTRANCE_BOUNDARY
						| L_CB_EXIT_BOUNDARY);

		for (cb = fn->first_cb; cb; cb = cb->next_cb) {
			L_Oper *next_op;
			for (op = cb->first_op; op; op = next_op) {
				next_op = op->next_op;

				if (op->opc == Lop_NO_OP)
					L_delete_oper(cb, op);
				else
					LB_hb_stat.orig_ops++;
			}
		}

		L_do_flow_analysis(fn, DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE);

		L_loop_detection(fn, 0);

		if (LB_hb_verbose_level >= 7) {
			fprintf(stderr, "Initial Loop Detection Phase for (fn %s):\n",
					fn->name);
			L_print_loop_data(fn);
		}

		LB_elim_all_loop_backedges(fn);

		L_delete_unreachable_blocks(fn);

		L_do_flow_analysis(fn, DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE);
		L_reset_loop_headers(fn);
		L_loop_detection(fn, 0);

		if (LB_hb_verbose_level >= 7) {
			fprintf(stderr, "Final Loop Detection Phase for (fn %s):\n",
					fn->name);
			L_print_loop_data(fn);
		}

		L_compute_oper_weight(fn, 0, 1);
		LB_mark_jrg_flag(fn);

		//**********************************************************************

		L_partial_dead_code_removal(fn);
		L_do_flow_analysis(fn,
		DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE | SUPPRESS_PG);

		//fprintf(stderr,"Resid 0\n");
		{
			ILP_cfg_wcet_analyize(fn, "(0).txt");
			Ldot_display_cfg(fn, "t0.dot", 0);
			FILE * file = fopen("rrr.txt", "w");
			L_print_func(file, fn);
			fclose(file);
		}

		for (i = 1; i < 10; i++) {

			//	L_do_flow_analysis(fn,
			//				DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE | SUPPRESS_PG);
			//L_partial_dead_code_removal(fn);
			L_do_flow_analysis(fn,
			DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE | SUPPRESS_PG);
			L_reset_loop_headers(fn);
			L_loop_detection(fn, 0);
			backedge = find_set_backedges(fn);
			headers = find_set_headers(fn);

			find_loops_wcet(fn);
			fprintf(stderr, "QQQQQQQQQQQQQQQQQQQQQQQQQ\n");

			{
				char tp[100];

				sprintf(tp, "(%d).txt", i);
				ILP_cfg_wcet_analyize(fn, tp);

				sprintf(tp, "t%d.dot", i);
				Ldot_display_cfg(fn, tp, 0);

			}
			//fprintf(stderr,"Resid1\n");

			fprintf(stderr, "ROUND (%d)\n", i);

			//
			//			if (i > 1) {
			//
			//				my_trace_formation2(fn, header);
			//
			//				if (header->traceregions==NULL)
			//									break;
			//				my_select_trace_2(fn, header);
			//
			//
			//				if (header->traceregions==NULL)
			//					break;
			//				if(header->traceregions->size<=0)
			//					break;
			//				if(header->traceregions->first==NULL){
			//					header->traceregions=NULL;
			//					break;
			//				}
			//
			//				LB_summarize_traceregions(stderr, header);
			//			}
			//			else{
			//				//pre path hb formation
			//				my_pre_path_hb_formation(fn,header);
			//
			//			}

			trace_formation_SHARIF4(fn, header);
			if (header->traceregions == NULL)
				break;

			LB_hb_reset_max_oper_id(fn);

			LB_remove_partially_subsumed_traceregions(header);

			LB_remove_conflicting_traceregions(header);

			LB_set_hyperblock_flag(fn);

			LB_set_hyperblock_func_flag(fn);

			//*****************trace selection***************
			//tr=(LB_TraceRegion *)List_get_first(temp_header->traceregions);
			//header->traceregions=List_insert_last(header->traceregions,tr);

			//***********************************************

			LB_summarize_traceregions(stderr, header);

			LB_tail_duplicate(fn, header, LB_DUP_OUTSIDE_REGION);

			{
				L_check_func(fn);
			}

			LB_predicate_traceregions(fn, header);

			LB_set_hyperblock_flag(fn);
			LB_remove_unnec_hyperblock_flags(fn);
			LB_set_hyperblock_func_flag(fn);

			LB_free_all_traceregions(header);

			//temp_header->traceregions = List_reset (temp_header->traceregions);
			//temp_header->inorder_trs = List_reset (temp_header->inorder_trs);

			LB_remove_empty_cbs(fn);

			L_delete_unreachable_blocks(fn);

			LB_convert_to_strict_basic_block_code(fn,
			L_CB_HYPERBLOCK | L_CB_SUPERBLOCK);

			{
				DB_spit_func(fn, "PH1");
				L_check_func(fn);
			}

		}

		//**********************************************************************
		L_do_flow_analysis(fn, DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE);
		L_reset_loop_headers(fn);
		L_loop_detection(fn, 0);
		if (Set_size(find_set_backedges(fn)) != Set_size(backedge))
			fprintf(stderr, "REYHANE\n");

		fprintf(stderr,
				"***************\n**************\n***************\n***************\n");

		{
			//L_partial_dead_code_removal(fn);
			L_do_flow_analysis(fn,
			DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE | SUPPRESS_PG);
			ILP_cfg_wcet_analyize(fn, "(99).txt");
			Ldot_display_cfg(fn, "t99.dot", 0);

			// LB_summarize_traceregions (stderr, header);
		}
		L_do_flow_analysis(fn,
		DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE | SUPPRESS_PG);

		//*******************************************************************

		//******************************************************************

		/*
		 *  Split branches into pred defines and predicated jumps
		 */
		if (LB_hb_branch_split)
			LB_branch_split_func(fn);

		/*
		 *        Generate multiple defn pred defines, generate Uncond pred defines
		 *  (Note initially only OR-type pred defines are created!
		 */

		L_create_uncond_pred_defines(fn);

		PG_setup_pred_graph(fn);
		if (LB_do_lightweight_pred_opti)
			L_lightweight_pred_opti(fn);
		L_combine_pred_defines(fn);

		/*
		 *        remove unnecessary uncond jumps
		 */

		LB_uncond_branch_elim(fn);

		/*
		 *        Merge ops on opposite predicates (partial redundancy elim)
		 */

		L_unmark_all_pre_post_increments(fn);

		if (LB_hb_do_pred_merge) {
			PG_setup_pred_graph(fn);
			L_do_flow_analysis(fn, LIVE_VARIABLE | REACHING_DEFINITION);
			LB_hb_pred_merging(fn);
		}

#ifdef DEBUG_HB_FORMER
		L_check_func(fn);
#endif

		PG_destroy_pred_graph();
		D_delete_dataflow(fn);

		/* For each source code function we process deinit */
		LB_function_deinit(header);

		{
			double rat = 1.0;
			L_Attr *eattr;

			final_ops = 0;

			for (cb = fn->first_cb; cb; cb = cb->next_cb) {
				L_Oper *next_op;
				for (op = cb->first_op; op; op = next_op) {
					next_op = op->next_op;

					if (op->opc != Lop_NO_OP)
						final_ops++;
				}
			}

			if (LB_hb_stat.orig_ops)
				rat = (double) final_ops / LB_hb_stat.orig_ops;

			eattr = L_new_attr("hbe", 2);
			L_set_int_attr_field(eattr, 0, LB_hb_stat.orig_ops);
			L_set_double_attr_field(eattr, 1, rat);
			fn->attr = L_concat_attr(fn->attr, eattr);
		}

		int num_oper = 0;
		for (cb = fn->first_cb; cb; cb = cb->next_cb) {
			L_Oper *next_op;
			for (op = cb->first_op; op; op = next_op) {
				next_op = op->next_op;

				if (op->opc != Lop_NO_OP)

					num_oper++;
			}

		}
		fprintf(stderr, "NUMBER OF OPERATION:(%d)\n", num_oper);

	}

	return;
}

#endif

