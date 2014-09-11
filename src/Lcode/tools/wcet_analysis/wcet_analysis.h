/*
 * wcet_analysis.c
 *
 *  Created on: May 8, 2011
 *      Author: morty
 */
#ifndef WCET_INCLUDED
#define WCET_INCLUDED



int ILP_get_backedges_data(Graph bb_g);
void reset_ILP(L_Func * fn);

Graph ILP_init(L_Func* fn);
char* ILP_get_arc_name(GraphArc arc);
void ILP_create_eq_list(FILE* file,List arcs_list,char operator);
void ILP_create_incoming_list(FILE* file,GraphNode node);
void ILP_create_target_list(FILE* file,GraphNode node);
int ILP_create_nodes_eq(FILE* file,Graph bb_g);
int ILP_create_backedges_eq(FILE* file,Graph bb_g);
int ILP_create_arcs_list(FILE* file,Graph bb_g);
/*
int ILP_estimate_wcet_for_block(L_Cb* cb);
int ILP_estimate_wcet_for_graph(Graph bb_g);
*/
int ILP_create_max_eq(FILE* file,Graph bb_g);
void ILP_lp_solve();
// Return an array. First element is WCET, others are arcs data.

int* ILP_parse_lp_solver(Graph bb_g);
int ILP_cfg_wcet_analyize(L_Func* fn,char* filename);
// End {Sepehr}.
//Morteza
int ILP_trace_wcet_analyize(Graph bb_g);


#endif
