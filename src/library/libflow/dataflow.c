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
 *      File:   dataflow.c
 *      Author: Po-hua Chang
 *      Modified By: XXX, date, time, why
 *      Copyright (c) 1991 Po-hua Chang and Wen-mei Hwu. All rights reserved.  
 *      All rights granted to the University of Illinois.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <library/graph.h>
#include <library/dataflow.h>

#undef TEST


/* forward function declarations */
static void reaching_definition (FGraph graph, struct I_Node *nodes,
                                 struct I_Reg *regs);
static void ud_chain (FGraph graph, struct I_Node *nodes, struct I_Reg *regs,
                      Node entry);
static void live_variable (FGraph graph, struct I_Node *nodes,
                           struct I_Reg *regs);
static void dominator (FGraph graph, struct I_Node *nodes, Node entry);
static void post_dominator (FGraph graph, struct I_Node *nodes, Node entry);

static struct I_Reg regs[MAX_REG_ID];
static struct I_Node nodes[MAX_NODE_ID];
static int nodes_defined = 0;

#define GOOD_ID(id)             ((id>=0) && (id<MAX_NODE_ID))
#define GOOD_REG_ID(id)         ((id>=0) && (id<MAX_REG_ID))

static void
punt (char *mesg)
{
  fprintf (stderr, "error in dataflow analysis: %s\n", mesg);
  exit (-1);
}

/*---------------------------------------------------------------------------*/
static void
reset_nodes (void)
{
  register int i;
  if (nodes_defined != 0)
    {                           /* already defined */
      /*
       *      Garbage collection.
       */
      for (i = 0; i < MAX_NODE_ID; i++)
        {
          nodes[i].defined = 0;
          nodes[i].id = i;
          if (nodes[i].dest_reg != 0)
            {
              Set_dispose (nodes[i].dest_reg);
              nodes[i].dest_reg = 0;
            }
          if (nodes[i].src_reg != 0)
            {
              Set_dispose (nodes[i].src_reg);
              nodes[i].src_reg = 0;
            }
          if (nodes[i].gen != 0)
            {
              Set_dispose (nodes[i].gen);
              nodes[i].gen = 0;
            }
          if (nodes[i].kill != 0)
            {
              Set_dispose (nodes[i].kill);
              nodes[i].kill = 0;
            }
          if (nodes[i].use != 0)
            {
              Set_dispose (nodes[i].use);
              nodes[i].use = 0;
            }
          if (nodes[i].def != 0)
            {
              Set_dispose (nodes[i].def);
              nodes[i].def = 0;
            }
          if (nodes[i].e_in != 0)
            {
              Set_dispose (nodes[i].e_in);
              nodes[i].e_in = 0;
            }
          if (nodes[i].e_out != 0)
            {
              Set_dispose (nodes[i].e_out);
              nodes[i].e_out = 0;
            }
          if (nodes[i].a_in != 0)
            {
              Set_dispose (nodes[i].a_in);
              nodes[i].a_in = 0;
            }
          if (nodes[i].a_out != 0)
            {
              Set_dispose (nodes[i].a_out);
              nodes[i].a_out = 0;
            }
          if (nodes[i].v_in != 0)
            {
              Set_dispose (nodes[i].v_in);
              nodes[i].v_in = 0;
            }
          if (nodes[i].v_out != 0)
            {
              Set_dispose (nodes[i].v_out);
              nodes[i].v_out = 0;
            }
        }
      for (i = 0; i < MAX_REG_ID; i++)
        {
          regs[i].defined = 0;
          if (regs[i].e_def != 0)
            {
              Set_dispose (regs[i].e_def);
              regs[i].e_def = 0;
            }
          if (regs[i].e_use != 0)
            {
              Set_dispose (regs[i].e_use);
              regs[i].e_use = 0;
            }
        }
    }
  else
    {                           /* never defined */
      nodes_defined = 1;
      for (i = 0; i < MAX_NODE_ID; i++)
        {
          nodes[i].defined = 0;
          nodes[i].id = i;
          nodes[i].dest_reg = 0;
          nodes[i].src_reg = 0;
          nodes[i].gen = 0;
          nodes[i].kill = 0;
          nodes[i].use = 0;
          nodes[i].def = 0;
          nodes[i].e_in = 0;
          nodes[i].e_out = 0;
          nodes[i].a_in = 0;
          nodes[i].a_out = 0;
          nodes[i].v_in = 0;
          nodes[i].v_out = 0;
        }
      for (i = 0; i < MAX_REG_ID; i++)
        {
          regs[i].defined = 0;
          regs[i].e_def = 0;
          regs[i].e_use = 0;
        }
    }
}

struct I_Node *
find_df_node (int id)
{
  if (nodes_defined == 0)
    return 0;
  if (!GOOD_ID (id))
    punt ("find_df_node: illegal node id");
  if (nodes[id].defined == 0)
    return 0;
  return (nodes + id);
}

static struct I_Node *
add_node (int id)
{
  if (nodes_defined == 0)
    punt ("add_node: need to call define_df_graph() first");
  if (!GOOD_ID (id))
    punt ("add_node: illegal node id");
  if (nodes[id].defined == 0)
    {
      nodes[id].defined = 1;
    }
  return (nodes + id);
}

struct I_Reg *
find_df_reg (int reg_id)
{
  if (nodes_defined == 0)
    return 0;
  if (!GOOD_REG_ID (reg_id))
    punt ("find_df_reg: illegal register id");
  if (regs[reg_id].defined == 0)
    return 0;
  return (regs + reg_id);
}

static struct I_Reg *
add_reg (int reg_id)
{
  if (nodes_defined == 0)
    punt ("add_reg: need to call define_df_graph() first");
  if (!GOOD_REG_ID (reg_id))
    punt ("add_reg: illegal register id");
  if (regs[reg_id].defined == 0)
    {
      regs[reg_id].defined = 1;
    }
  return (regs + reg_id);
}

/*---------------------------------------------------------------------------*/
static FGraph graph = 0;
static struct I_Node *node = 0;
static Node entry = 0;

void
define_df_graph (void)
{
  /*
   *  Start from an empty structure.
   */
  if (graph != 0)
    {
      FreeGraph (&graph);
      graph = 0;
    }
  reset_nodes ();
  node = 0;
  entry = 0;
  /*
   *  Build a new graph.
   */
  graph = NewGraph ();
  if (graph == 0)
    punt ("define_df_graph: failed to create a graph");
}

/*
 *      The first call add_df_node after define_df_graph
 *      defines the entry node.
 */
void
add_df_node (int id)
{
  Node n;
  if (graph == 0)
    punt ("add_df_node: need to call define_df_graph() first");
  if (!GOOD_ID (id))
    punt ("add_df_node: bad node id");
  n = FindNode (graph, id);
  if (n == 0)
    {
      n = NewNode ();
      nodeId (n) = id;
      AddNode (graph, n);
    }
  node = find_df_node (id);
  if (node != 0)
    {
      punt ("add_df_node: cannot add a node twice");
    }
  node = add_node (id);
  nodeExt (n) = (void *) node;
  if (entry == 0)
    entry = n;
}

void
add_df_node_src_reg (int reg_id)
{
  struct I_Reg *r;
  if (graph == 0)
    punt ("add_df_node_src_reg: need to call define_df_graph() first");
  if (node == 0)
    punt ("add_df_node_src_reg: need to call add_df_node() first");
  if (!GOOD_REG_ID (reg_id))
    punt ("add_df_node_src_reg: bad register id");
  r = add_reg (reg_id);
  node->src_reg = Set_add (node->src_reg, reg_id);
  r->e_use = Set_add (r->e_use, node->id);
}

void
add_df_node_dest_reg (int reg_id)
{
  struct I_Reg *r;
  if (graph == 0)
    punt ("add_df_node_dest_reg: need to call define_df_graph() first");
  if (node == 0)
    punt ("add_df_node_dest_reg: need to call add_df_node() first");
  if (!GOOD_REG_ID (reg_id))
    punt ("add_df_node_dest_reg: bad register id");
  r = add_reg (reg_id);
  node->dest_reg = Set_add (node->dest_reg, reg_id);
  r->e_def = Set_add (r->e_def, node->id);
}

/*
 *      For nodes which have not been defined,
 *      this function will leave the extension field as 0.
 */
void
connect_df_node (int src_id, int dest_id)
{
  Node n1, n2;
  if (graph == 0)
    punt ("connect_df_node: need to call define_df_graph() first");
  n1 = FindNode (graph, src_id);
  if (n1 == 0)
    {
      n1 = NewNode ();
      nodeId (n1) = src_id;
      AddNode (graph, n1);
    }
  n2 = FindNode (graph, dest_id);
  if (n2 == 0)
    {
      n2 = NewNode ();
      nodeId (n2) = dest_id;
      AddNode (graph, n2);
    }
  ConnectNodes (n1, n2, 0);
}

void
end_df_graph (void)
{
  Node n;
  struct I_Node *p;
  if (graph == 0)
    punt ("end_df_graph: need to call define_df_graph() first");
  if (entry == 0)
    punt ("end_of_graph: empty function");
  for (n = graph->nodes; n != 0; n = nextNode (n))
    {
      p = (struct I_Node *) nodeExt (n);
      if (p == 0)
        punt ("incomplete definition: missing some nodes");
    }
}

/*===========================================================================*/
void
analyze_df_graph (int mode)
{
  /*
   *  1. compute the reaching definition.
   */
  if (mode & REACH_DEFINE)
    reaching_definition (graph, nodes, regs);
  /*
   *  2. compute the ud-chain.
   */
  if (mode & USE_DEF_CHAIN)
    ud_chain (graph, nodes, regs, entry);
  /*
   *  3. live-variable analysis.
   */
  if (mode & LIVE_VAR)
    live_variable (graph, nodes, regs);
  /*
   *  4. dominator analysis.
   */
  if (mode & DOMINATOR)
    dominator (graph, nodes, entry);
  if (mode & POST_DOMINATOR)
    post_dominator (graph, nodes, entry);
}

/*===========================================================================*/
static void
reaching_definition (FGraph graph, struct I_Node *nodes, struct I_Reg *regs)
{
  register int i, reg_id, change;
  Node node;
  /*
   *  1. compute the gen[] and kill[] sets.
   *          gen[S] = id(S)
   *          kill[S] = reg[dest(S)].e_def - {id(S)}
   */
  for (node = graph->nodes; node != 0; node = nextNode (node))
    {
      Set gen, kill, def, temp;
      i = nodeId (node);
      gen = Set_add (0, i);
      kill = 0;             /* kill = all nodes that modify the output of i */
      def = 0;
      for (reg_id = 0; reg_id < MAX_REG_ID; reg_id++)
        {
          if (regs[reg_id].defined == 0)
            continue;
          if (!Set_in (nodes[i].dest_reg, reg_id))
            continue;
          temp = def;
          def = Set_union (def, regs[reg_id].e_def);
          Set_dispose (temp);
        }
      kill = Set_subtract (def, gen);
      Set_dispose (def);
      nodes[i].gen = gen;
      nodes[i].kill = kill;
    }
  /*
   *  2. compute e_in[] and e_out[] sets.
   */
  for (node = graph->nodes; node != 0; node = nextNode (node))
    {
      i = nodeId (node);        /* e_out[] = gen[] */
      nodes[i].e_out = Set_union (nodes[i].gen, 0);
    }
  change = 1;
  while (change)
    {
      change = 0;
      for (node = graph->nodes; node != 0; node = nextNode (node))
        {
          Set in, out, temp;
          Arc arc;
          i = nodeId (node);
          /*
           *  in[] = union(out[] of all predecessors of i)
           */
          in = 0;
          for (arc = sourceArcs (node); arc != 0; arc = nextArc (arc))
            {
              Node source = sourceNode (arc);
              int src_id = nodeId (source);
              temp = in;
              in = Set_union (in, nodes[src_id].e_out);
              Set_dispose (temp);
            }
          /*
           *  out[] = gen[] + (in[] - kill[])
           */
          temp = Set_subtract (in, nodes[i].kill);
          out = Set_union (nodes[i].gen, temp);
          Set_dispose (temp);
          /*
           *  see if there is change.
           */
          temp = Set_subtract (nodes[i].e_out, out);
          if (Set_size (temp) != 0)
            {
              change += 1;
            }
          else
            {
              Set_dispose (temp);
              temp = Set_subtract (out, nodes[i].e_out);
              if (Set_size (temp) != 0)
                {
                  change += 1;
                }
            }
          Set_dispose (temp);
          /*
           *  update.
           */
          temp = nodes[i].e_in;
          nodes[i].e_in = in;
          Set_dispose (temp);
          temp = nodes[i].e_out;
          nodes[i].e_out = out;
          Set_dispose (temp);
        }
    }
}

/*===========================================================================*/
static void
ud_chain (FGraph graph, struct I_Node *nodes, struct I_Reg *regs, Node entry)
{
  register int i, change;
  Node node;
  Set U;
  struct I_Node *ptr;
  /*
   *  1. compute the gen[] and kill[] sets.
   *          gen[S] = id(S)
   *          kill[S] = reg[dest(S)].e_def - {id(S)}
   *  Here, we assume that the two sets have been computed.
   */
  /*
   *  2. compute a_in[] and a_out[] sets.
   *          in[entry] = 0
   *          out[entry] = gen[entry]
   *          for (S<>entry) 
   *                  out[S] = U - kill[S]
   */
  U = 0;
  for (node = graph->nodes; node != 0; node = nextNode (node))
    {
      i = nodeId (node);
      U = Set_add (U, i);
    }
  ptr = (struct I_Node *) nodeExt (entry);
  ptr->a_in = 0;
  ptr->a_out = Set_union (ptr->gen, 0);
  for (node = graph->nodes; node != 0; node = nextNode (node))
    {
      if (node == entry)
        continue;
      i = nodeId (node);
      nodes[i].a_in = 0;
      nodes[i].a_out = Set_subtract (U, nodes[i].kill);
    }
  /*
   *  Loop until a stable solution is available.
   */
  change = 1;
  while (change)
    {
      change = 0;
      for (node = graph->nodes; node != 0; node = nextNode (node))
        {
          Set in, out, temp;
          Arc arc;
          if (node == entry)    /* forget about the entry node */
            continue;
          i = nodeId (node);
          /*
           *  in[] = intersect(out[] of all predecessors of i)
           */
          in = Set_union (U, 0);
          for (arc = sourceArcs (node); arc != 0; arc = nextArc (arc))
            {
              Node source = sourceNode (arc);
              int src_id = nodeId (source);
              temp = in;
              in = Set_intersect (in, nodes[src_id].a_out);
              Set_dispose (temp);
            }
          /*
           *  out[] = gen[] + (in[] - kill[])
           */
          temp = Set_subtract (in, nodes[i].kill);
          out = Set_union (nodes[i].gen, temp);
          Set_dispose (temp);
          /*
           *  see if there is change.
           */
          temp = Set_subtract (nodes[i].a_out, out);
          if (Set_size (temp) != 0)
            {
              change += 1;
            }
          else
            {
              Set_dispose (temp);
              temp = Set_subtract (out, nodes[i].a_out);
              if (Set_size (temp) != 0)
                {
                  change += 1;
                }
            }
          Set_dispose (temp);
          /*
           *  update.
           */
          temp = nodes[i].a_in;
          nodes[i].a_in = in;
          Set_dispose (temp);
          temp = nodes[i].a_out;
          nodes[i].a_out = out;
          Set_dispose (temp);
        }
    }
  Set_dispose (U);
}

/*===========================================================================*/
static void
live_variable (FGraph graph, struct I_Node *nodes, struct I_Reg *regs)
{
  register int i, n, change;
  Node node;
  /*
   *  1. for each node,
   *          use[S] = all source registers.
   *          def[S] = all destination registers.
   *          in[S] = 0;
   *          out[S] = 0;
   */
  for (node = graph->nodes; node != 0; node = nextNode (node))
    {
      i = nodeId (node);
      nodes[i].use = Set_union (nodes[i].src_reg, 0);
      nodes[i].def = Set_union (nodes[i].dest_reg, 0);
      nodes[i].v_in = 0;
      nodes[i].v_out = 0;
    }
  /*
   *  2. iterate until a stable solution is available.
   */
  n = 0;
  change = 1;
  while (change)
    {
      change = 0;
#ifdef TEST
      n++;
      printf ("iteration[%d]\n", n);
#endif
      for (node = graph->nodes; node != 0; node = nextNode (node))
        {
          Set in, out, temp;
          Arc arc;
          i = nodeId (node);
#ifdef TEST
          printf ("node[%d] = \n", i);
#endif
          /*
           *  out[S] = union(in[] of all successors of S)
           */
          out = 0;
          for (arc = destinationArcs (node); arc != 0; arc = nextArc (arc))
            {
              Node dest = destinationNode (arc);
              int dest_id = nodeId (dest);
              temp = out;
              out = Set_union (out, nodes[dest_id].v_in);
              Set_dispose (temp);
            }
          /*
           *  in[S] = use[S] + (out[S] - def[S])
           */
          temp = Set_subtract (out, nodes[i].def);
          in = Set_union (nodes[i].use, temp);
          Set_dispose (temp);
          /*
           *  see if there is change.
           */
          temp = Set_subtract (nodes[i].v_in, in);
          if (Set_size (temp) != 0)
            {
              change += 1;
            }
          else
            {
              Set_dispose (temp);
              temp = Set_subtract (in, nodes[i].v_in);
              if (Set_size (temp) != 0)
                {
                  change += 1;
                }
            }
          Set_dispose (temp);
          /*
           *  update.
           */
#ifdef TEST
          PrintSet (stdout, "use", nodes[i].use);
          PrintSet (stdout, "def", nodes[i].def);
          PrintSet (stdout, "old_in", nodes[i].v_in);
          PrintSet (stdout, "old_out", nodes[i].v_out);
          PrintSet (stdout, "new_in", in);
          PrintSet (stdout, "new_out", out);
          printf ("change = %d\n", change);
#endif
          temp = nodes[i].v_in;
          nodes[i].v_in = in;
          Set_dispose (temp);
          temp = nodes[i].v_out;
          nodes[i].v_out = out;
          Set_dispose (temp);
        }
    }
}

/*===========================================================================*/
static void
dominator (FGraph graph, struct I_Node *nodes, Node entry)
{
  int id, source_id, change;
  Set all, old_dom, temp, inter;
  Node node, source;
  Arc arc;
  /*
   *  Initialization
   */
  all = 0;
  for (node = graph->nodes; node != 0; node = nextNode (node))
    {
      id = nodeId (node);
      if (nodes[id].dom != 0)
        nodes[id].dom = Set_dispose (nodes[id].dom);
      all = Set_add (all, id);
    }
  for (node = graph->nodes; node != 0; node = nextNode (node))
    {
      id = nodeId (node);
      if (node == entry)
        nodes[id].dom = Set_add (0, nodeId (entry));
      nodes[id].dom = Set_union (0, all);
    }
  /*
   *  Compute dominance relationship
   */
  change = 1;
  while (change > 0)
    {
      change = 0;
      for (node = graph->nodes; node != 0; node = nextNode (node))
        {
          id = nodeId (node);
          old_dom = nodes[id].dom;
          arc = sourceArcs (node);
          if (arc == 0)
            {
              temp = 0;
            }
          else
            {
              source = sourceNode (arc);
              source_id = nodeId (source);
              temp = Set_union (nodes[source_id].dom, 0);
              arc = nextArc (arc);
              for (; arc != 0; arc = nextArc (arc))
                {
                  source = sourceNode (arc);
                  source_id = nodeId (source);
                  inter = Set_intersect (temp, nodes[source_id].dom);
                  temp = Set_dispose (temp);
                  temp = inter;
                }
            }
          temp = Set_add (temp, id);
          if (!Set_same (temp, old_dom))
            change += 1;
          nodes[id].dom = Set_dispose (nodes[id].dom);
          nodes[id].dom = temp;
        }
    }
  all = Set_dispose (all);
}
/*===========================================================================*/
static void
post_dominator (FGraph graph, struct I_Node *nodes, Node entry)
{
  int id, dest_id, change;
  Set all, old_post_dom, temp, inter;
  Node node, dest;
  Arc arc;
  /*
   *  Initialization
   */
  all = 0;
  for (node = graph->nodes; node != 0; node = nextNode (node))
    {
      id = nodeId (node);
      if (nodes[id].post_dom != 0)
        nodes[id].post_dom = Set_dispose (nodes[id].post_dom);
      all = Set_add (all, id);
    }
  for (node = graph->nodes; node != 0; node = nextNode (node))
    {
      id = nodeId (node);
      if (destinationArcs (node) == 0)
        {
          nodes[id].post_dom = Set_add (0, id);
        }
      else
        {
          nodes[id].post_dom = Set_union (0, all);
        }
    }
  /*
   *  Compute post_dominance relationship
   */
  change = 1;
  while (change > 0)
    {
      change = 0;
      for (node = graph->nodes; node != 0; node = nextNode (node))
        {
          id = nodeId (node);
          old_post_dom = nodes[id].post_dom;
          arc = destinationArcs (node);
          if (arc == 0)
            {
              temp = 0;
            }
          else
            {
              dest = destinationNode (arc);
              dest_id = nodeId (dest);
              temp = Set_union (nodes[dest_id].post_dom, 0);
              arc = nextArc (arc);
              for (; arc != 0; arc = nextArc (arc))
                {
                  dest = destinationNode (arc);
                  dest_id = nodeId (dest);
                  inter = Set_intersect (temp, nodes[dest_id].post_dom);
                  temp = Set_dispose (temp);
                  temp = inter;
                }
            }
          temp = Set_add (temp, id);
          if (!Set_same (temp, old_post_dom))
            change += 1;
          nodes[id].post_dom = Set_dispose (nodes[id].post_dom);
          nodes[id].post_dom = temp;
        }
    }
  all = Set_dispose (all);
}
/*===========================================================================*/
#define MAX ((MAX_NODE_ID > MAX_REG_ID) ? MAX_NODE_ID : MAX_REG_ID)

static void
print_set (FILE * F, Set set, char *prefix)
{
  int i;
  fprintf (F, "{");
  for (i = 0; i < MAX; i++)
    {
      if (Set_in (set, i))
        {
          if (prefix != 0)
            fprintf (F, "%s", prefix);
          fprintf (F, "%d,", i);
        }
    }
  fprintf (F, "}\n");
}

void
print_df_node (FILE * F, struct I_Node *node)
{
  fprintf (F, "### NODE [%d] .%d\n", node->id, node->defined);
  fprintf (F, "# src_reg = ");
  print_set (F, node->src_reg, "r");
  fprintf (F, "# dest_reg = ");
  print_set (F, node->dest_reg, "r");
  fprintf (F, "# gen = ");
  print_set (F, node->gen, "");
  fprintf (F, "# kill = ");
  print_set (F, node->kill, "");
  fprintf (F, "# use = ");
  print_set (F, node->use, "r");
  fprintf (F, "# def = ");
  print_set (F, node->def, "r");
  fprintf (F, "# e_in = ");
  print_set (F, node->e_in, "");
  fprintf (F, "# e_out = ");
  print_set (F, node->e_out, "");
  fprintf (F, "# a_in = ");
  print_set (F, node->a_in, "");
  fprintf (F, "# a_out = ");
  print_set (F, node->a_out, "");
  fprintf (F, "# v_in = ");
  print_set (F, node->v_in, "r");
  fprintf (F, "# v_out = ");
  print_set (F, node->v_out, "r");
  fprintf (F, "# dom = ");
  print_set (F, node->dom, "");
  fprintf (F, "# post_dom = ");
  print_set (F, node->post_dom, "");

  fprintf (F, "###\n");
}
void
print_df_graph (FILE * F)
{
  int id;
  Node node;
  for (node = graph->nodes; node != 0; node = nextNode (node))
    {
      id = nodeId (node);
      print_df_node (F, &nodes[id]);
    }
}
/*===========================================================================*/
