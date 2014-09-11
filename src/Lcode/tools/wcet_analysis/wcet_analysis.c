/*
 * wcet_analysis.c
 *
 *  Created on: May 8, 2011
 *      Author: morty
 */
#include "wcet_analysis.h"

int ILP_get_backedges_data(Graph bb_g){
	//system("cp /home/morty/Desktop/test_final_Basic/LOOP_data/loops_data.txt ./");
	FILE* file = fopen("loops_data.txt","r");
	List arcs_list = bb_g->arcs;
	ListElement arcs_el;
	for (arcs_el = arcs_list->first ; arcs_el ; arcs_el = arcs_el->next){
		GraphArc arc = (GraphArc) arcs_el->ptr;
		if ( arc->flags & GRAPH_ARC_BACKEDGE )
			//fscanf(file,"%d",&(arc->num_iteration));
			arc->num_iteration=10;
		((L_Flow *)arc->flow)->max_iteration=arc->num_iteration;
		((L_Flow *)arc->flow)->dst_cb->max_iteration=arc->num_iteration;
	}
}
void reset_ILP(L_Func * fn)
{
	L_Cb* cb;
	L_Flow * flow;
	for(cb=fn->first_cb;cb;cb=cb->next_cb){
		cb->ILP_is_in_wcet_path=0;
		cb->wcet=0;
		for(flow=cb->dest_flow;flow;flow=flow->next_flow){
			flow->ILP_is_in_wcet_path=0;
			flow->wcet_weight=0;
		}
	}

}

Graph ILP_init(L_Func* fn){
	Graph bb_g = L_create_cb_graph(fn);
	Graph_dominator (bb_g);
	Graph_post_dominator (bb_g);
	Graph_imm_dominator (bb_g);
	Graph_imm_post_dominator (bb_g);
	Graph_control_dependence (bb_g);
	Graph_topological_sort (bb_g);
	Graph_preorder_dfs_sort(bb_g,1);
	Graph_preorder_dfs_visit(bb_g,bb_g->root,1); // for creating backedges.
	graph_test(fn);
	return bb_g;
}
char* ILP_get_arc_name(GraphArc arc){
	GraphNode pred = arc->pred;
	GraphNode succ = arc->succ;
	char* temp = malloc(30);
	sprintf(temp,"d%d_%d",pred->id,succ->id);
	return temp;
}
void ILP_create_eq_list(FILE* file,List arcs_list,char operator){
	if ( arcs_list == NULL)
		return;
	ListElement arcs_el;
	GraphArc arc;
	for ( arcs_el = arcs_list->first ; arcs_el != arcs_list->last ; arcs_el = arcs_el->next){
		arc = (GraphArc) arcs_el->ptr;
		fprintf(file,"%s %c ",ILP_get_arc_name(arc),operator);
	}
	arc = (GraphArc) arcs_el->ptr;
	fprintf(file,"%s ", ILP_get_arc_name(arc));
}
void ILP_create_incoming_list(FILE* file,GraphNode node){
	ILP_create_eq_list(file,node->pred,'+');
}
void ILP_create_target_list(FILE* file,GraphNode node){
	ILP_create_eq_list(file,node->succ,'-');
}
int ILP_create_nodes_eq(FILE* file,Graph bb_g){
	fprintf(file,"\nSubject to\n\n");
	fprintf(file,"\\ ==== Nodes Equations ====\n\n");
	List nodes_list = bb_g->topo_list;
	ListElement nodes_el;
	nodes_el = nodes_list->first;
	GraphNode node = (GraphNode) nodes_el->ptr;
	// Creating first Equation:
	fprintf(file,"s = 1\n");
	fprintf(file, "s - ");
	ILP_create_target_list(file,node);
	fprintf(file," = 0\n");

	for ( nodes_el = nodes_el->next ; nodes_el !=nodes_list->last ; nodes_el = nodes_el->next){
		node = (GraphNode) nodes_el->ptr;
		ILP_create_incoming_list(file,node);
		fprintf(file,"- ");
		ILP_create_target_list(file,node);
		fprintf(file," = 0\n");
	}
	// Creating last Equation:
	node = (GraphNode) nodes_el->ptr;
	ILP_create_incoming_list(file,node);
	fprintf(file, " - t = 0\n");
	fprintf(file, "t = 1 \n");
}
int ILP_create_backedges_eq(FILE* file,Graph bb_g){
	FILE * my_back=fopen("my_back.txt","w");
	fprintf(file,"\n\\ ==== Backedges Equations ====\n\n");
	fprintf(my_back,"\n\\ ==== Backedges Equations ====\n\n");

	List arcs_list = bb_g->arcs;
	ListElement arcs_el;
	for (arcs_el = arcs_list->first ; arcs_el ; arcs_el = arcs_el->next){
		GraphArc arc = (GraphArc) arcs_el->ptr;
		if ( arc->flags & GRAPH_ARC_BACKEDGE ){
			fprintf(file,"%s <= %d \n",ILP_get_arc_name(arc),arc->num_iteration);
			fprintf(my_back,"%s <= %d \n",ILP_get_arc_name(arc),arc->num_iteration);

		}
	}
	fclose(my_back);
}
int ILP_create_arcs_list(FILE* file,Graph bb_g){
	List arcs_list = bb_g->arcs;
	ListElement arcs_el;
	GraphArc arc;
	fprintf(file,"\nGenerals\n\n");
	fprintf(file,"s\n"); // First dummy edge.
	for ( arcs_el = arcs_list->first ; arcs_el; arcs_el = arcs_el->next){
		arc = (GraphArc) arcs_el->ptr;
		fprintf(file,"%s\n",ILP_get_arc_name(arc));
	}
	fprintf(file,"t\n");
	fprintf(file,"\nEnd\n");
}

/*
int ILP_estimate_wcet_for_block(L_Cb* cb){
	// Estimate every operation takes one unit time.
	int counter = 1 ;
	L_Oper* oper;
	for ( oper = cb->first_op ; oper != cb->last_op ; oper = oper->next_op)
		counter++;
	return counter;
}
int ILP_estimate_wcet_for_graph(Graph bb_g){
	List nodes_list = bb_g->topo_list;
	ListElement nodes_el;
	nodes_el = nodes_list->first;
	GraphNode node = (GraphNode) nodes_el->ptr;
	fprintf(stderr, "YYYYYYYYY\n");
	for ( nodes_el = nodes_el->next ; nodes_el ; nodes_el = nodes_el->next){
		node = (GraphNode) nodes_el->ptr;
		//L_Cb* cb = GraphNodeContents(node);
		//cb->wcet = ILP_estimate_wcet_for_block(cb);
	}
}
*/
int ILP_create_max_eq(FILE* file,Graph bb_g){
	List arcs_list = bb_g->arcs;
	ListElement arcs_el;
	GraphArc arc;
	L_Cb* cb;
	fprintf(file,"Maximize\n\n");
	for ( arcs_el = arcs_list->first ; arcs_el; arcs_el = arcs_el->next){
			arc = (GraphArc) arcs_el->ptr;
			cb = (L_Cb*) GraphNodeContents((arc->pred));
			fprintf(file,"%d %s + " ,( arc->wcet_weight > 0 ? arc->wcet_weight : 0 ) , ILP_get_arc_name(arc));
	}
	// Assume that we have only one exit.
	const int LAST_CB_WCET = 1;
	fprintf(file,"%d t \n",LAST_CB_WCET);
}
void ILP_lp_solve(){
	system("/home/ut/trimaran/openimpact/src/Lcode/tools/lp_solve/lp_solve -rxli ../lp_solve/libxli_CPLEX.so ilp.lp > res.txt");
}
// Return an array. First element is WCET, others are arcs data.

int* ILP_parse_lp_solver(Graph bb_g){
	int number_of_arcs = bb_g->arcs->size;
	int* ret = (int*) malloc((number_of_arcs+1)*sizeof(int));
	int i;
	ILP_lp_solve(number_of_arcs);
	FILE* file = fopen("res.txt","r");
	if ( file == NULL )
		return NULL;
	char temp[1000];

	// First part of Garbage:
	const int first_out = 8;
	for ( i = 0 ; i < first_out ; i++)
		fscanf(file,"%s",temp);

	//WCET
	fscanf(file,"%d",&ret[0]);

	// Second part of Garbage:
	const int second_out = 5;
	for (  i = 0 ; i < second_out ; i++)
		fscanf(file,"%s",temp);

	// Arcs with Garbage:
	for ( i = 0 ; i < number_of_arcs ; i++){
		fscanf(file,"%s",temp);
		fscanf(file,"%d",&ret[1+i]);
	}

	fclose(file);
	system("rm res.txt");

	return ret;
}
int ILP_cfg_wcet_analyize(L_Func* fn,char* filename){

	reset_ILP(fn);
	find_flow_wight_2(fn);


	Graph bb_g = ILP_init(fn);
	//ILP_estimate_wcet_for_graph(bb_g);
	ILP_get_backedges_data(bb_g);
	FILE* file = fopen("ilp.lp","w");



	ILP_create_max_eq(file,bb_g);
	ILP_create_nodes_eq(file,bb_g);
	ILP_create_backedges_eq(file,bb_g);
	ILP_create_arcs_list(file,bb_g);



	fclose(file);

	int* arr = ILP_parse_lp_solver(bb_g);
	FILE* tfile = fopen(filename,"w");

	List arcs_list = bb_g->arcs;
	ListElement arcs_el;
	GraphArc arc;

	fprintf(tfile,"wcet = %d\n",arr[0]);

	int i = 1 ;
	for ( arcs_el = arcs_list->first ; arcs_el; arcs_el = arcs_el->next){
		arc = (GraphArc) arcs_el->ptr;
		fprintf(tfile,"%s = %d \n",ILP_get_arc_name(arc),arr[i++]);
		if ( arr[i-1] ){
			arc->ILP_is_in_wcet_path = 1;
			((L_Flow *)arc->flow)->ILP_is_in_wcet_path=1;
			((L_Flow *)arc->flow)->wcep_num_exec=arr[i-1];

			((L_Cb *)arc->pred->ptr)->ILP_is_in_wcet_path=1;
			((L_Cb *)arc->pred->ptr)->wcep_num_exec=arr[i-1];

			((L_Cb *)arc->succ->ptr)->ILP_is_in_wcet_path=1;
			((L_Cb *)arc->succ->ptr)->wcep_num_exec=arr[i-1];

		}
		else{
			arc->ILP_is_in_wcet_path = 0;
			((L_Flow *)arc->flow)->ILP_is_in_wcet_path=0;
			//((L_Cb *)arc->pred->ptr)->ILP_is_in_wcet_path=0;
			//((L_Cb *)arc->succ->ptr)->ILP_is_in_wcet_path=0;

		}
	}
	fclose(tfile);
}
// End {Sepehr}.
//Morteza
int ILP_trace_wcet_analyize(Graph bb_g){


	Graph_dominator (bb_g);
	Graph_post_dominator (bb_g);
	Graph_imm_dominator (bb_g);
	Graph_imm_post_dominator (bb_g);
	Graph_control_dependence (bb_g);
	Graph_topological_sort (bb_g);
	Graph_preorder_dfs_sort(bb_g,1);
	Graph_preorder_dfs_visit(bb_g,bb_g->root,1); // for creating backedges.

	//fprintf(stderr, "0");
	List_start(bb_g->arcs);
	GraphArc arc;
	L_Flow * flow;
	//fprintf(stderr, "1");
	while(arc=(GraphArc) List_next(bb_g->arcs))
	{
		if(arc){
		flow= (L_Flow *) arc->flow;
			if(flow!=NULL){
				if(flow->wcet_weight!=NULL){
				fprintf(stderr, "flow(%d): %d->%d\n",flow->wcet_weight,flow->src_cb->id,flow->dst_cb->id);
				arc->wcet_weight=flow->wcet_weight;}
			}
			else
				arc->wcet_weight=0;
		}
	}
	fprintf(stderr, "attttttttttttttt\n");
	//ILP_estimate_wcet_for_graph(bb_g);
	FILE* file = fopen("ilp.lp","w");
	fprintf(stderr, "b");
	ILP_create_max_eq(file,bb_g);
	fprintf(stderr, "c");
	ILP_create_nodes_eq(file,bb_g);
	ILP_create_backedges_eq(file,bb_g);
	ILP_create_arcs_list(file,bb_g);
	fprintf(stderr, "d");
	fclose(file);

	int* arr = ILP_parse_lp_solver(bb_g);
//	FILE* tfile = fopen(filename,"w");

	List arcs_list = bb_g->arcs;
	ListElement arcs_el;
	//GraphArc arc;

//	fprintf(tfile,"wcet = %d\n",arr[0]);

	int i = 1 ;
	for ( arcs_el = arcs_list->first ; arcs_el; arcs_el = arcs_el->next){
		arc = (GraphArc) arcs_el->ptr;
//		fprintf(tfile,"%s = %d \n",ILP_get_arc_name(arc),arr[i++]);
		if ( arr[i-1] ){
			arc->ILP_is_in_wcet_path = 1;
			//if()
			//(arc->flow)->ILP_is_in_wcet_path=1;
//			((L_Cb *)arc->pred->ptr)->ILP_is_in_wcet_path=1;
//			((L_Cb *)arc->succ->ptr)->ILP_is_in_wcet_path=1;
		}
		else{
			arc->ILP_is_in_wcet_path = 0;
			//((L_Flow *)arc->ptr)->ILP_is_in_wcet_path=0;
			//((L_Cb *)arc->pred->ptr)->ILP_is_in_wcet_path=0;
			//((L_Cb *)arc->succ->ptr)->ILP_is_in_wcet_path=0;

		}
	}
	fprintf(stderr, "e\n");
	return arr[0];
	//fclose(tfile);
}



