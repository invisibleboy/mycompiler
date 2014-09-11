
#include <stdio.h>
#include <Lcode/l_dot.h>
#include <Lcode/l_code.h>
#include <Lcode/l_main.h>

#define L_DOT_FONT "Helvetica"

static void
L_dot_print_file_header(FILE* out, char* graph_name)
{
  char* orientation = "portrait";
  double page_w = 8.5;
  double page_h = 11;
  double size_w = 10;
  double size_h = 7.5;

  fprintf(out, "digraph %s {\n", graph_name);
  fprintf(out, "    graph [ \n");
  fprintf(out, "            orientation = \"%s\",\n", orientation);
  fprintf(out, "            center = \"true\",\n");
  fprintf(out, "            page = \"%.2f,%.2f\",\n", page_w, page_h);
  fprintf(out, "            size = \"%.2f,%.2f\" ]\n", size_h, size_w);
}

static void
L_dot_print_file_tail(FILE* out)
{
  fprintf(out, "}\n");
}

/** Returns the current date/time as a string; caller must free the string. */
static char* 
L_dot_get_curr_time()
{
  const int max_str_size = 50;
  char* timestr = (char*) Lcode_malloc(sizeof(char) * max_str_size);
  time_t curr_t;
  struct tm* local_t;

  curr_t = time(&curr_t);
  local_t = localtime(&curr_t);
  strftime(timestr, max_str_size, "%b/%d/%Y  %I:%M %p", local_t);
  return timestr;
}

static void
L_dot_print_branch_cond(FILE* out, L_Oper* op)
{
  if (op->com[1]) {
    L_print_operand(out, op->src[0], 0);
    fprintf(out, " ");
    fprintf(out, "%s ", L_cmp_compl_name(op->com[1]));
    fprintf(out, " ");
    L_print_operand(out, op->src[1], 0);
  }
}

static void
write_tabs(FILE *out, int num_tabs)
{
  int i;
  for (i = 0; i < num_tabs; i++)
    fprintf(out, "\t");
}
      

static void
write_loop_cluster(FILE *out, Set *cbs_in_loops, int num_tabs, L_Loop *loop) 
{
  L_Cb *cb;
  L_Attr *attr;
  L_Flow *flow;
  L_Oper *br_op;
  int *loop_cb = NULL;
  int num_loop_cb, i;

  if (loop == NULL)
    return;

#if 0
  /* Optimize child loops *first* so they get included within already_shown. */
  write_loop_cluster(out, cbs_in_loops, loop->child_loop);
#endif

  /* Then focus on this loop. */
  if ((num_loop_cb = Set_size(loop->loop_cb))) {
    loop_cb = (int*) alloca(sizeof(int) * num_loop_cb);
    Set_2array (loop->loop_cb, loop_cb);
  }
  write_tabs(out, num_tabs);
  fprintf(out, "subgraph cluster%d {\n", loop->id);
  write_tabs(out, num_tabs + 1);
  fprintf(out, "label=\"Loop %d\";\n", loop->id);

  /* Write out children within this loop. */
  write_loop_cluster(out, cbs_in_loops, num_tabs + 1, loop->child_loop);

  for (i = 0; i < num_loop_cb; i++) {
    if (Set_in(*cbs_in_loops, loop_cb[i]))
      continue;
    
    *cbs_in_loops = Set_add(*cbs_in_loops, loop_cb[i]);
    cb = L_cb_hash_tbl_find(L_fn->cb_hash_tbl, loop_cb[i]);


    if ( (attr = L_find_attr(cb->attr, "Lthread_loop")) ) {
      int lthread_id = L_get_int_attr_field(attr, 0);
      int lthread_iter;
      attr = L_find_attr(cb->attr, "Lthread_iter");
      lthread_iter = L_get_int_attr_field(attr, 0);

      write_tabs(out, num_tabs + 1);
      fprintf(out, 
	  "cb_%d [fontname=\"%s\", label=\"cb %d\\n(loop %d.%d)\"];\n", 
	  cb->id, L_DOT_FONT, cb->id, lthread_id, lthread_iter);
    } else {
      write_tabs(out, num_tabs + 1);
      fprintf(out, "cb_%d [fontname=\"%s\", label=\"cb %d\"];\n", 
	  cb->id, L_DOT_FONT, cb->id);
    }
    

    /* 
     * Write out intraloop edges.
     */
    for (flow = cb->dest_flow; flow != NULL; flow = flow->next_flow) {
      if (!Set_in(loop->loop_cb, flow->dst_cb->id))
	continue;

      /* We don't use the results of this, but the call checks the CFG for *
       * errors.                                                           */
      L_find_matching_flow(flow->dst_cb->src_flow, flow);

      fprintf(out, "\tcb_%d -> cb_%d ", flow->src_cb->id, flow->dst_cb->id);
      fprintf(out, "[label= \"");
      br_op = L_find_branch_for_flow(flow->src_cb, flow);
      if (flow->weight != 0.0)
	fprintf(out, "%.1f", flow->weight);
      if (flow->weight != 0.0 && br_op != NULL)
	fprintf(out, "\\n");
      if (br_op != NULL)
	L_dot_print_branch_cond(out, br_op);
      fprintf(out, "\"];\n");
    }
  }
  write_tabs(out, num_tabs);
  fprintf(out, "}\n");


  /* Make sure we also take care of our siblings. */
  write_loop_cluster(out, cbs_in_loops, num_tabs, loop->sibling_loop);
}

static void
Ldot_print_loop_graph_loop(FILE *out, int num_tabs, L_Loop *loop)
{
  if (loop == NULL)
    return;

  write_tabs(out, num_tabs);
  fprintf(out, "subgraph cluster%d {\n", loop->id);
  write_tabs(out, num_tabs + 1);
  fprintf(out, "label=\"Loop %d\";\n", loop->id);
  write_tabs(out, num_tabs + 1);
  fprintf(out, "fakenode_loop%d [color=white,fontname=Courier,label=\"", loop->id);

  fprintf(out, "num_invocation     %f\\n", loop->num_invocation);
  if (loop->num_invocation > 0.0)
    fprintf (out, "ave_iter           %f\\n",
	loop->header->weight / loop->num_invocation);
  if (loop->preheader != NULL)
    fprintf (out, "preheader          %d\\n", loop->preheader->id);
  if (loop->header != NULL)
    fprintf (out, "\theader             %d\\n", loop->header->id);
  fprintf (out, "nesting_level      %d\\n", loop->nesting_level);
  fprintf (out, "loop_size (no. cb) %d\\n", Set_size (loop->loop_cb));
#if 0
  Set_print (out, "loop_cb", loop->loop_cb);
  Set_print (out, "back_edge_cb", loop->back_edge_cb);
  Set_print (out, "exit_cb", loop->exit_cb);
  Set_print (out, "out_cb", loop->out_cb);
  Set_print (out, "nested_loops", loop->nested_loops);
  Set_print (out, "basic_ind_var", loop->basic_ind_var);
  Set_print (out, "basic_ind_var_op", loop->basic_ind_var_op);
#endif
  fprintf(out, "\"];\n");

  /* Write out children within this loop. */
  Ldot_print_loop_graph_loop(out, num_tabs + 1, loop->child_loop);
  write_tabs(out, num_tabs);
  fprintf(out, "}\n");

  /* Now write out any related siblings. */
  Ldot_print_loop_graph_loop(out, num_tabs, loop->sibling_loop);
}

static void
Ldot_display_loop_graph(L_Func* fn, char* descr, int interactive)
{
  FILE *out;
  char* timestr = NULL;

  const int bufsize = 512;
  char *filename = (char*) Lcode_malloc(sizeof(char) * bufsize);
  char *command = NULL;

  /*
   * 1. Setup the file.
   */
  if (descr)
    snprintf(filename, bufsize, "DOT_LOOPGRAPH_%s_%s", fn->name, descr);
  else
    snprintf(filename, bufsize, "DOT_LOOPGRAPH_%s", fn->name);

  if (!(out = fopen(filename, "w")))
    L_punt("could not open file for output: %s", filename);


  L_dot_print_file_header(out, fn->name);
  timestr = L_dot_get_curr_time();
  fprintf(out, "\tthe_caption [fontname=\"%s\", shape=\"box\", ", L_DOT_FONT);
  if (descr)
    fprintf(out, "label=\"LOOPGRAPH for %s (%s), %s\"]\n", 
	fn->name, descr, timestr);
  else
    fprintf(out, "label=\"LOOPGRAPH for %s, %s\"]\n", fn->name, timestr);

  /*
   * 2. Actually print the graph.
   */
  Ldot_print_loop_graph_loop(out, 1, fn->first_loop);

  /*
   * 3. Close the file.
   */
  L_dot_print_file_tail(out);
  fclose(out);

  if (interactive) {
    command = (char*) Lcode_malloc(sizeof(char) * bufsize);
    snprintf(command, bufsize, "dot -Tps %s > %s.ps", filename, filename);
    system(command);
    Lcode_free(command);
  }
  
  Lcode_free(filename);
  Lcode_free(timestr);
}

extern void
Ldot_display_cfg(L_Func* fn, char* descr, int interactive)
{
  const int show_loops = 0;
  FILE* out;
  L_Cb* cb;
  L_Flow* flow;
  L_Attr* attr;
  L_Oper* br_op;
  char* timestr = NULL;
  Set cbs_in_loops = NULL;

  const int bufsize = 512;
  char *filename = (char*) Lcode_malloc(sizeof(char) * bufsize);
  char *command = NULL;


  if (descr)
    snprintf(filename, bufsize, "DOT_CFG_%s_%s", fn->name, descr);
  else
    snprintf(filename, bufsize, "DOT_CFG_%s", fn->name);

  if (!(out = fopen(filename, "w")))
    L_punt("could not open file for output: %s", filename);


  L_dot_print_file_header(out, fn->name);
  timestr = L_dot_get_curr_time();
  fprintf(out, "\tthe_caption [fontname=\"%s\", shape=\"box\", ", L_DOT_FONT);
  if (descr)
    fprintf(out, "label=\"CFG for %s (%s), %s\"]\n", fn->name, descr, timestr);
  else
    fprintf(out, "label=\"CFG for %s, %s\"]\n", fn->name, timestr);
  fprintf(out, "\tcompound=true;\n");

  /* Assumes that fn->first_loop returns an outermost loop (lowest # nesting
   * level) and that all loops are reachable by traversing sibling_loop and
   * child_loop.                                                            */
  if (show_loops)
    write_loop_cluster(out, &cbs_in_loops, 1, fn->first_loop);

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb) {
    if (Set_in(cbs_in_loops, cb->id))
      continue;

    if ( (attr = L_find_attr(cb->attr, "Lthread_loop")) ) {
      int lthread_id = L_get_int_attr_field(attr, 0);
      int lthread_iter;
      attr = L_find_attr(cb->attr, "Lthread_iter");
      lthread_iter = L_get_int_attr_field(attr, 0);

      fprintf(out,"cb_%d [fontname=\"%s\", label=\"cb %d\\n(loop %d.%d) wcet=%d",
	  cb->id, L_DOT_FONT, cb->id, lthread_id, lthread_iter,cb->wcet);
      if(cb->ILP_is_in_wcet_path)
    	  fprintf(out,"#");
      fprintf(out,"\"]\n");
    } else {
      fprintf(out, "cb_%d [fontname=\"%s\", label=\"cb %d wcet=%d",
	  cb->id, L_DOT_FONT, cb->id,cb->wcet);
      if(cb->ILP_is_in_wcet_path)
          	  fprintf(out,"#");
            fprintf(out,"\"]\n");
    }
  }

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb) {
    for (flow = cb->dest_flow; flow != NULL; flow = flow->next_flow) {

      /* We don't use the results of this, but the call checks the CFG for *
       * errors.                                                           */
      L_find_matching_flow(flow->dst_cb->src_flow, flow);

      fprintf(out, "\tcb_%d -> cb_%d ", flow->src_cb->id, flow->dst_cb->id);
      fprintf(out, "[label= \"");
      fprintf(out, "wcet=%d ",flow->wcet_weight);
      br_op = L_find_branch_for_flow(flow->src_cb, flow);
      if (flow->weight != 0.0)
	fprintf(out, "%.1f", flow->weight);
      if (flow->weight != 0.0 && br_op != NULL)
	fprintf(out, "\\n");
      if (br_op != NULL)
	L_dot_print_branch_cond(out, br_op);
      fprintf(out, "\"];\n");
    }
  }

  L_dot_print_file_tail(out);
  fclose(out);

  if (interactive) {
    command = (char*) Lcode_malloc(sizeof(char) * bufsize);
    snprintf(command, bufsize, "dot -Tps %s > %s.ps", filename, filename);
    system(command);
    Lcode_free(command);
  }
  
  Lcode_free(filename);
  Lcode_free(timestr);

}

#if 0
extern void
Ldot_display_cfg(L_Func* fn, char* descr, int interactive)
{
  FILE* out;
  L_Cb* cb;
  L_Flow* flow;
  L_Attr* attr;
  L_Oper* br_op;
  char* timestr = NULL;

  const int bufsize = 512;
  char *filename = (char*) Lcode_malloc(sizeof(char) * bufsize);
  char *command = NULL;


  if (descr)
    snprintf(filename, bufsize, "DOT_GRAPH_%s_%s", fn->name, descr);
  else
    snprintf(filename, bufsize, "DOT_GRAPH_%s", fn->name);

  if (!(out = fopen(filename, "w")))
    L_punt("could not open file for output: %s", filename);


  L_dot_print_file_header(out, fn->name);
  timestr = L_dot_get_curr_time();
  fprintf(out, "the_caption [fontname=\"%s\", shape=\"box\", ", L_DOT_FONT);
  if (descr)
    fprintf(out, "label=\"CFG for %s (%s), %s\"]\n", fn->name, descr, timestr);
  else
    fprintf(out, "label=\"CFG for %s, %s\"]\n", fn->name, timestr);

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb) {
    if ( (attr = L_find_attr(cb->attr, "Lthread_loop")) ) {
      int lthread_id = L_get_int_attr_field(attr, 0);
      int lthread_iter;
      attr = L_find_attr(cb->attr, "Lthread_iter");
      lthread_iter = L_get_int_attr_field(attr, 0);

      fprintf(out, "cb_%d [fontname=\"%s\", label=\"cb %d\\n(loop %d.%d)\"]\n", 
	  cb->id, L_DOT_FONT, cb->id, lthread_id, lthread_iter);
    } else {
      fprintf(out, "cb_%d [fontname=\"%s\", label=\"cb %d\"]\n", 
	  cb->id, L_DOT_FONT, cb->id);
    }
  }

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb) {
    for (flow = cb->dest_flow; flow != NULL; flow = flow->next_flow) {

      /* We don't use the results of this, but the call checks the CFG for *
       * errors.                                                           */
      L_find_matching_flow(flow->dst_cb->src_flow, flow);

      fprintf(out, "\tcb_%d -> cb_%d ", flow->src_cb->id, flow->dst_cb->id);
      fprintf(out, "[label= \"");
      br_op = L_find_branch_for_flow(flow->src_cb, flow);
      if (flow->weight != 0.0)
	fprintf(out, "%.1f", flow->weight);
      if (flow->weight != 0.0 && br_op != NULL)
	fprintf(out, "\\n");
      if (br_op != NULL)
	L_dot_print_branch_cond(out, br_op);
      fprintf(out, "\"];\n");
    }
  }

  L_dot_print_file_tail(out);
  fclose(out);

  if (interactive) {
    command = (char*) Lcode_malloc(sizeof(char) * bufsize);
    snprintf(command, bufsize, "dot -Tps %s > %s.ps", filename, filename);
    system(command);
    Lcode_free(command);
  }
  
  Lcode_free(filename);
  Lcode_free(timestr);

}
#endif

extern void
Ldot_process(L_Func* fn)
{
  L_do_flow_analysis (fn, DOMINATOR_CB); /* must come before loop detection */
  L_loop_detection(fn, 0);
  Ldot_display_loop_graph(fn, NULL, 1);

  Ldot_display_cfg(fn, NULL, 1);
}
