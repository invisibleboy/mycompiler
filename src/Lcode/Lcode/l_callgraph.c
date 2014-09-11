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
/*****************************************************************************\
 *      File :          l_callgraph.c
 *      Description :   Routines to manipulate the callgraph structures.
 *      Original : Brian Deitrich, Wen-mei Hwu 1997 (adapted from Roger 
 *                 Bringmann's work)
 *
\*****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>

int total_arcs = 0;
int total_nodes = 0;
FILE *GRAPH_OUT;

/*****************************************************************************\
 *
 * Call graph arcs
 *
\*****************************************************************************/

L_CG_Arc *
L_CG_copy_arc (L_CG_Arc * arc)
{
  L_CG_Arc *copy_arc;

  copy_arc = (L_CG_Arc *) L_alloc (L_alloc_l_cg_arc);
  copy_arc->id = total_arcs++;
  copy_arc->flags = arc->flags;
  copy_arc->jsr_id = arc->jsr_id;
  copy_arc->type = arc->type;
  copy_arc->weight = arc->weight;
  copy_arc->src_node = arc->src_node;
  copy_arc->prev_dst_arc = arc->prev_dst_arc;
  copy_arc->next_dst_arc = arc->next_dst_arc;
  copy_arc->dst_node = arc->dst_node;
  copy_arc->prev_src_arc = arc->prev_src_arc;
  copy_arc->next_src_arc = arc->next_src_arc;
  copy_arc->ext = NULL;

  return (copy_arc);
}

void
L_CG_delete_only_arc (L_CG_Arc * arc)
{
  L_free (L_alloc_l_cg_arc, arc);
}

void
L_CG_delete_arc (L_CG_Arc * arc)
{
  L_CG_Node *node;

  if (!arc)
    return;

  /* Remove the link from the src node */
  node = arc->src_node;
  if (node->first_dst_arc == arc)
    node->first_dst_arc = arc->next_dst_arc;
  if (node->last_dst_arc == arc)
    node->last_dst_arc = arc->prev_dst_arc;
  if (arc->prev_dst_arc)
    arc->prev_dst_arc->next_dst_arc = arc->next_dst_arc;
  if (arc->next_dst_arc)
    arc->next_dst_arc->prev_dst_arc = arc->prev_dst_arc;

  /* Remove the link to the dst node */
  node = arc->dst_node;
  if (node->first_src_arc == arc)
    node->first_src_arc = arc->next_src_arc;
  if (node->last_src_arc == arc)
    node->last_src_arc = arc->prev_src_arc;
  if (arc->prev_src_arc)
    arc->prev_src_arc->next_src_arc = arc->next_src_arc;
  if (arc->next_src_arc)
    arc->next_src_arc->prev_src_arc = arc->prev_src_arc;

  L_CG_delete_only_arc (arc);
}

void
L_CG_delete_all_dst_arcs (L_CG_Arc * first_dst_arc)
{
  L_CG_Arc *arc, *next_dst_arc;

  for (arc = first_dst_arc; arc != NULL; arc = next_dst_arc)
    {
      next_dst_arc = arc->next_dst_arc;
      L_CG_delete_arc (arc);
    }
}

void
L_CG_delete_all_src_arcs (L_CG_Arc * first_src_arc)
{
  L_CG_Arc *arc, *next_src_arc;

  for (arc = first_src_arc; arc != NULL; arc = next_src_arc)
    {
      next_src_arc = arc->next_src_arc;
      L_CG_delete_arc (arc);
    }
}

void
L_CG_remove_arc_from_src_and_dst (L_CG_Arc * arc)
{
  /* remove this arc from the dst arc list of src_node. */
  if (arc->src_node->first_dst_arc == arc->src_node->last_dst_arc)
    {
      arc->src_node->first_dst_arc = NULL;
      arc->src_node->last_dst_arc = NULL;
    }
  else if (arc->src_node->last_dst_arc == arc)
    {
      arc->src_node->last_dst_arc = arc->prev_dst_arc;
      arc->prev_dst_arc->next_dst_arc = NULL;
    }
  else if (arc->src_node->first_dst_arc == arc)
    {
      arc->src_node->first_dst_arc = arc->next_dst_arc;
      arc->next_dst_arc->prev_dst_arc = NULL;
    }
  else
    {
      arc->prev_dst_arc->next_dst_arc = arc->next_dst_arc;
      arc->next_dst_arc->prev_dst_arc = arc->prev_dst_arc;
    }

  /* remove this arc from the src arc list of dst_node. */
  if (arc->dst_node->first_src_arc == arc->dst_node->last_src_arc)
    {
      arc->dst_node->first_src_arc = NULL;
      arc->dst_node->last_src_arc = NULL;
    }
  else if (arc->dst_node->last_src_arc == arc)
    {
      arc->dst_node->last_src_arc = arc->prev_src_arc;
      arc->prev_src_arc->next_src_arc = NULL;
    }
  else if (arc->dst_node->first_src_arc == arc)
    {
      arc->dst_node->first_src_arc = arc->next_src_arc;
      arc->next_src_arc->prev_src_arc = NULL;
    }
  else
    {
      arc->prev_src_arc->next_src_arc = arc->next_src_arc;
      arc->next_src_arc->prev_src_arc = arc->prev_src_arc;
    }
}

void
L_CG_add_arc (L_CG_Node * src_node, L_Oper * oper, L_Operand * operand,
              L_CallGraph * callgraph)
{
  L_Attr *attr;
  L_CG_Node *dst_node;
  L_CG_Arc *arc;

  if (L_is_label (operand))
    {
      /*  ignore '_$fn' if present  */
      if (!strncmp (operand->value.l, "_$fn", 4))
        dst_node = L_CG_find_node (operand->value.l + 4, callgraph);
      else
        dst_node = L_CG_find_node (operand->value.l, callgraph);

      arc = (L_CG_Arc *) L_alloc (L_alloc_l_cg_arc);
      arc->level = 0;
      arc->id = total_arcs++;
      arc->flags = 0;
      arc->jsr_id = oper->id;
      if (!(attr = L_find_attr (oper->attr, "CALLNAME")))
        arc->type = NORMAL_JSR;
      else if (attr->max_field == 1)
        arc->type = RESOLVED_IND_JSR;
      else
        arc->type = UNRESOLVED_IND_JSR;
      arc->weight = oper->weight;

      /* Append the arc to the end of the chain of dst arcs 
         for this src node */
      arc->src_node = src_node;

      arc->next_dst_arc = NULL;
      if (!src_node->first_dst_arc)
        {
          src_node->first_dst_arc = arc;
        }

      if (src_node->last_dst_arc)
        {
          arc->prev_dst_arc = src_node->last_dst_arc;
          src_node->last_dst_arc->next_dst_arc = arc;
        }
      else
        {
          arc->prev_dst_arc = NULL;
        }
      src_node->last_dst_arc = arc;

      /* Append the arc to the end of the chain of src arcs 
         for this dst node */
      arc->dst_node = dst_node;
      arc->next_src_arc = NULL;

      if (!dst_node->first_src_arc)
        dst_node->first_src_arc = arc;

      if (dst_node->last_src_arc)
        {
          arc->prev_src_arc = dst_node->last_src_arc;
          dst_node->last_src_arc->next_src_arc = arc;
        }
      else
        {
          arc->prev_src_arc = NULL;
        }
      dst_node->last_src_arc = arc;

      arc->ext = NULL;
    }
  else if (L_is_register (operand) &&
           (attr = L_find_attr (oper->attr, "CALLNAME")))
    {
      /* Handle case of an indirect jsr, 
         where the CALLNAME attribute is present.
         * If no attribute is present, don't add any arcs. */
      L_Operand temp_operand;
      int num_targets, i, j, attr_name_length, temp_index;
      char temp_str[4096], temp_name[4096];

      num_targets = attr->max_field;
      if (num_targets == 0)
        L_punt ("INLINER's callgraph made an invalid assumption!");

      for (i = 0; i < num_targets; i++)
        {
          L_assign_type_label (&temp_operand);

          /* cluge fix to account for extra pair of "" in the name. */
          attr_name_length = strlen (attr->field[i]->value.l) + 1;
          temp_index = 0;
          for (j = 0; j < attr_name_length; j++)
            if (attr->field[i]->value.l[j] != '\"')
              temp_name[temp_index++] = attr->field[i]->value.l[j];

          strcpy (temp_str, "_\0");
          strcat (temp_str, temp_name);
          temp_operand.value.l =
            (char *) Lcode_malloc (strlen (temp_str) + 1);
          strcpy (temp_operand.value.l, temp_str);

          L_CG_add_arc (src_node, oper, &temp_operand, callgraph);
          Lcode_free (temp_operand.value.l);
        }
    }
}

void
L_CG_print_arc (L_CG_Arc * arc)
{
  printf ("---------- ARC ----------\n");
  printf ("   id = %d\n", arc->id);
  printf ("   jsr_id = %d\n", arc->jsr_id);
  printf ("   weight = %f\n", arc->weight);
  printf ("   src node = %s [%d]\n", arc->src_node->func_name,
          arc->src_node->id);
  printf ("   dst node = %s [%d]\n", arc->dst_node->func_name,
          arc->dst_node->id);
}

/*****************************************************************************\
 *
 * Call graph nodes
 *
\*****************************************************************************/

L_CG_Node *
L_CG_new_node (char *name)
{
  L_CG_Node *node;

  node = (L_CG_Node *) L_alloc (L_alloc_l_cg_node);

  node->id = total_nodes++;
  node->flags = 0;
  node->func_name = name;

  node->func_name = (char *) Lcode_malloc (strlen (name) + 1);
  strcpy (node->func_name, name);

  node->filename = NULL;

  node->func_file_num = 0;
  node->fn = NULL;

  node->first_src_arc = NULL;
  node->last_src_arc = NULL;

  node->first_dst_arc = NULL;
  node->last_dst_arc = NULL;

  node->prev_node = NULL;
  node->next_node = NULL;

  node->dfs = NULL;
  node->ext = NULL;

  return node;
}

void
L_CG_add_node (L_CG_Node * node, L_CallGraph * callgraph)
{
  if (callgraph->last_node)
    {
      callgraph->last_node->next_node = node;
      node->prev_node = callgraph->last_node;
      callgraph->last_node = node;
    }
  else
    {
      callgraph->first_node = node;
      callgraph->last_node = node;
    }
}

void
L_CG_delete_node (L_CG_Node * node)
{
  if (node->fn)
    {
      L_fn = node->fn;
      L_delete_func (node->fn);
    }
  if (node->filename)
    Lcode_free (node->filename);
  L_CG_delete_all_src_arcs (node->first_src_arc);
  L_CG_delete_all_dst_arcs (node->first_dst_arc);
  if (node->dfs)
    L_free (L_alloc_cg_dfs_info, node->dfs);

  L_free (L_alloc_l_cg_node, node);
  Lcode_free (node->func_name);
}

void
L_CG_delete_node_not_func (L_CG_Node * node)
{
  if (node->filename)
    Lcode_free (node->filename);
  L_CG_delete_all_src_arcs (node->first_src_arc);
  L_CG_delete_all_dst_arcs (node->first_dst_arc);
  if (node->dfs)
    L_free (L_alloc_cg_dfs_info, node->dfs);

  L_free (L_alloc_l_cg_node, node);
  Lcode_free (node->func_name);
}

L_CG_Node *
L_CG_find_node (char *fn_name, L_CallGraph * callgraph)
{
  L_CG_Node *node;
  char *name;

  name = M_fn_name_from_label (fn_name);

  /* Determine if the func exists in the list */
  for (node = callgraph->first_node; node != NULL; node = node->next_node)
    {
      if (!strcmp (node->func_name, name))
	return node;
    }

  /*
   * Since the func does not exist in the list, we will create an entry
   * for it.
   */
  node = L_CG_new_node (name);
  L_CG_add_node (node, callgraph);

  return node;
}

void
L_CG_print_node (L_CG_Node * node)
{
  printf ("*********** NODE ************\n");
  printf ("   id = %d\n", node->id);
  printf ("   func_name = %s\n", node->func_name);
  printf ("   filename = %s\n", node->filename);
  printf ("   func_file_num = %d\n", node->func_file_num);
}

L_Func *
L_read_node_func (L_CG_Node * node)
{
  int count = 0;
  L_Func *func = NULL;

  if (!node || !node->filename)
    return (NULL);

  if (node->fn)
    {
      L_fn = node->fn;
      return (node->fn);
    }

  L_open_input_file (node->filename);
  while (L_get_input () != L_INPUT_EOF)
    {
      if (L_token_type == L_INPUT_FUNCTION)
        {
          if (++count == node->func_file_num)
            {
              if (strcmp (node->func_name, M_fn_name_from_label (L_fn->name)))
                L_punt ("ERROR: L_read_node_func could not find the correct \
func.\n");
              func = L_fn;
            }
          else
            L_delete_func (L_fn);
        }
      else
        L_delete_data (L_data);
    }
  L_close_input_file (node->filename);

  L_fn = func;
  node->fn = func;
  return (func);
}

void
L_delete_node_func (L_CG_Node * node)
{
  if (!node->fn)
    return;

  L_fn = node->fn;
  L_delete_func (node->fn);
  L_fn = NULL;
  node->fn = NULL;
}

/*****************************************************************************\
 *
 * L_CallGraph support routines
 *
\*****************************************************************************/

L_CallGraph *
L_CG_new_callgraph (void)
{
  L_CallGraph *callgraph;

  callgraph = (L_CallGraph *) Lcode_malloc (sizeof (L_CallGraph));
  callgraph->first_node = NULL;
  callgraph->last_node = NULL;

  return callgraph;
}

void
L_CG_callgraph_delete (L_CallGraph * callgraph)
{
  L_CG_Node *node, *next_node;

  if (!callgraph)
    return;

  for (node = callgraph->first_node; node != NULL; node = next_node)
    {
      next_node = node->next_node;
      L_CG_delete_node (node);
    }

  Lcode_free (callgraph);
  callgraph = NULL;
}


void
L_CG_callgraph_delete_not_func (L_CallGraph * callgraph)
{
  L_CG_Node *node, *next_node;

  if (!callgraph)
    return;

  for (node = callgraph->first_node; node != NULL; node = next_node)
    {
      next_node = node->next_node;
      L_CG_delete_node_not_func (node);
    }

  Lcode_free (callgraph);
  callgraph = NULL;
}


L_CallGraph *
L_CG_callgraph_build (List filelist, void (*user_init_func) (L_CG_Node *))
{
  L_CallGraph *callgraph;
  int input_func_num;

  callgraph = L_CG_new_callgraph ();

  List_start (filelist);
  while ((L_file = List_next (filelist)))
    {
      L_input_file = L_file->input_file;
      L_output_file = L_file->output_file;
      
      L_OUT = L_open_output_file (L_output_file);
      L_generation_info_printed = 0;

      L_open_input_file (L_input_file);
      input_func_num = 0;

      while (L_get_input () != L_INPUT_EOF)
        {
          if (L_token_type == L_INPUT_FUNCTION)
            {
	      L_CG_Node *node;
	      L_Cb *cb;
	      L_Oper *oper;

              input_func_num++;
              node = L_CG_find_node (L_fn->name, callgraph);
              node->fn = L_fn;
	      node->filename = strdup (L_input_file);
              node->func_file_num = input_func_num;
              L_compute_oper_weight (L_fn, 0, 1);

              for (cb = L_fn->first_cb; cb != NULL; cb = cb->next_cb)
                {
                  for (oper = cb->first_op; oper != NULL;
                       oper = oper->next_op)
                    {
                      if (L_subroutine_call_opcode (oper))
                        L_CG_add_arc (node, oper, oper->src[0], callgraph);
                    }
                }

              user_init_func (node);

              L_delete_func (node->fn);
              node->fn = NULL;
            }
          else
	    {
	      L_delete_data (L_data);
	    }
        }

      L_close_input_file (L_input_file);
      L_close_output_file (L_OUT);
    }

  /* Print callgraph to file for graphical interface */
  if (L_print_davinci_callgraph)
    L_CG_print_davinci_callgraph (callgraph);

  return callgraph;
}

void
L_CG_print_callgraph (L_CallGraph * callgraph)
{
  L_CG_Node *node;
  L_CG_Arc *arc;

  printf ("FOLLOWING IS PRINT OUT OF CALLGRAPH\n");
  for (node = callgraph->first_node; node; node = node->next_node)
    {
      L_CG_print_node (node);
      printf ("source arcs:\n");
      for (arc = node->first_dst_arc; arc; arc = arc->next_dst_arc)
        L_CG_print_arc (arc);
      printf ("dest arcs:\n");
      for (arc = node->first_src_arc; arc; arc = arc->next_src_arc)
        L_CG_print_arc (arc);
    }
}

void
L_CG_davinci_visit (L_CG_Node * node)
{
  L_CG_Arc *arc;

  /* Define Node */
  if (node->dfs->color == BLACK)
    {
      fprintf (GRAPH_OUT, "r(\"FN %d\")", node->id);
      return;
    }
  node->dfs->color = BLACK;

  fprintf (GRAPH_OUT, "l(\"FN %d\",", node->id);

  if (!strncmp (node->func_name, "_$fn", 4))
    fprintf (GRAPH_OUT, "n(\"anything\", [a(\"OBJECT\", \"FN %s\")],",
             (node->func_name) + 4);
  else
    fprintf (GRAPH_OUT, "n(\"anything\", [a(\"OBJECT\", \"FN %s\")],",
             node->func_name);

  fprintf (GRAPH_OUT, "[");

  /* Print destination arcs */
  for (arc = node->first_dst_arc; arc; arc = arc->next_dst_arc)
    {
      fprintf (GRAPH_OUT, "e(\"anything\",[a(\"OBJECT\", \"%d\")],", arc->id);

      L_CG_davinci_visit (arc->dst_node);
      fprintf (GRAPH_OUT, ")");

      if (arc->next_dst_arc)
        fprintf (GRAPH_OUT, ",");
    }
  fprintf (GRAPH_OUT, "]))\n");
}

void
L_CG_print_davinci_callgraph (L_CallGraph * callgraph)
{
  L_CG_Node *node;

  /* Check if file exists on server */
  if ((GRAPH_OUT = fopen (L_davinci_callgraph_file, "w")) == NULL)
    {
      fprintf (stderr, "L_CG_print_davinci_callgraph: can not open %s for \
writing\n", L_davinci_callgraph_file);
      exit (1);
    }

  /* daVinci_print_header */
  fprintf (GRAPH_OUT, "[\n");

  /* Clear the visited bit flag */
  for (node = callgraph->first_node; node; node = node->next_node)
    {
      node->dfs = (L_DFS_Info *) L_alloc (L_alloc_cg_dfs_info);
      node->dfs->color = WHITE;
    }

  /* need to start with the top node */
  for (node = callgraph->first_node; node; node = node->next_node)
    {
      if (!strcmp (node->func_name, "_main"))
        break;
    }

  if (node == NULL)
    L_punt ("L_CG_print_davinci_callgraph: cannot find _main node");

  L_CG_davinci_visit (node);

  /* daVinci_print_footer */
  fprintf (GRAPH_OUT, "]\n");

  fclose (GRAPH_OUT);
}
