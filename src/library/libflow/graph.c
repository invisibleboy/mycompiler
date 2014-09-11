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
 *      File:   graph.c
 *      Author: Po-hua Chang
 *      Modified By: XXX, date, time, why
 *      Copyright (c) 1991 Po-hua Chang and Wen-mei Hwu. All rights reserved.  
 *      All rights granted to the University of Illinois.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

 /****************************************************************************\
 *      In this file, many general graph operations are provided.
 *      These functions can be used to implement graph algorithms.
 *
\*===========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <library/graph.h>

#define AUTO_REMOVE_MULTIPLE_ARCS

static void
Punt (char *mesg)
{
  fprintf (stderr, "Error in graph: graph.c: %s\n", mesg);
  exit (1);
}
/*----------- node operations --------------------------*/
static Node d_node_list = 0;
/*
 *      Allocate space for a new node.
 */
Node NewNode (void)
{
  Node new_node;
  if (isNil (d_node_list))
    {
      new_node = (Node) malloc (sizeof (_Node));
      if (new_node == 0)
        Punt ("NewNode: cannot allocate space");
    }
  else
    {
      new_node = d_node_list;
      d_node_list = nextNode (new_node);
    }
  nodeId (new_node) = 0;
  nodeType (new_node) = 0;
  nodeStatus (new_node) = 0;
  nodeWeight (new_node) = 0.0;
  sourceArcs (new_node) = 0;
  destinationArcs (new_node) = 0;
  nextNode (new_node) = 0;
  nodeExt (new_node) = 0;
  return new_node;
}
/*
 *      Remove space for a node.
 */
Node FreeNode (Node * node)
{
  if ((node == 0) || ((*node) == 0))
    return 0;
  nextNode ((*node)) = d_node_list;
  d_node_list = *node;
  *node = 0;
  return 0;
}
/*----------- arc operations --------------------------*/
static Arc d_arc_list = 0;
/*
 *      Allocate space for a new arc.
 */
Arc NewArc (void)
{
  Arc new_arc;
  if (isNil (d_arc_list))
    {
      new_arc = (Arc) malloc (sizeof (_Arc));
      if (new_arc == 0)
        Punt ("NewArc: cannot allocate space");
    }
  else
    {
      new_arc = d_arc_list;
      d_arc_list = nextArc (new_arc);
    }
  arcType (new_arc) = 0;
  arcWeight (new_arc) = 0.0;
  sourceNode (new_arc) = 0;
  destinationNode (new_arc) = 0;
  nextArc (new_arc) = 0;
  arcExt (new_arc) = 0;
  return new_arc;
}
/*
 *      Remove space of an arc.
 */
Arc FreeArc (Arc * arc)
{
  if ((arc == 0) || ((*arc) == 0))
    return 0;
  nextArc ((*arc)) = d_arc_list;
  d_arc_list = *arc;
  *arc = 0;
  return 0;
}
/*----------- general operations --------------------------*/
/*
 *      Connect two nodes by creating a new arc.
 *      It is an error to make a connection twice.
 *      A connection is uniquely specified by
 *      (src_node, dest_node, relation)
 */
static int node_count;

void
ConnectNodes (Node src_node, Node dest_node, int relation)
{
  Arc arc;
  if ((src_node == 0) || (dest_node == 0))
    Punt ("ConnectNodes: nil node");
  /*
   *  First make sure that there is no such connection.
   */
  for (arc = destinationArcs (src_node); !isNil (arc); arc = nextArc (arc))
    {
      if ((arcType (arc) == relation) && (destinationNode (arc) == dest_node))
#ifdef AUTO_REMOVE_MULTIPLE_ARCS
        return;                 /* already exist */
#else
        Punt ("ConnectNodes: multiple connections");
#endif
    }
  arc = NewArc ();              /* create a new link for src_node */
  arcType (arc) = relation;
  sourceNode (arc) = src_node;
  destinationNode (arc) = dest_node;
  nextArc (arc) = destinationArcs (src_node);
  destinationArcs (src_node) = arc;
  arc = NewArc ();              /* create a new link for dest_node */
  arcType (arc) = relation;
  sourceNode (arc) = src_node;
  destinationNode (arc) = dest_node;
  nextArc (arc) = sourceArcs (dest_node);
  sourceArcs (dest_node) = arc;
  node_count++;
}

int
PrintNodeCount ()
{
  fprintf (stdout, "Total nodes connected : %d\n", node_count);
  node_count = 0;
  return (0);
}

/*
 *      Remove a connection.
 */
void
DisconnectNodes (Node src_node, Node dest_node, int relation)
{
  Arc arc;
  /*
   *  First make sure that there is a such connection.
   */
  for (arc = destinationArcs (src_node); !isNil (arc); arc = nextArc (arc))
    {
      Node dest;
      dest = destinationNode (arc);
      if ((arcType (arc) == relation) && (dest == dest_node))
        break;
    }
  if (!isNil (arc))
    {
      Arc ptr;
      ptr = destinationArcs (src_node);
      if (ptr == arc)
        {                       /* remove the first link */
          destinationArcs (src_node) = nextArc (arc);
          nextArc (arc) = 0;
          FreeArc (&arc);
        }
      else
        {
          while (nextArc (ptr) != arc)
            ptr = nextArc (ptr);
          nextArc (ptr) = nextArc (arc);
          nextArc (arc) = 0;
          FreeArc (&arc);
        }
    }
  /*
   *  Now repeat for the dest_node.
   */
  for (arc = sourceArcs (dest_node); !isNil (arc); arc = nextArc (arc))
    {
      Node src;
      src = sourceNode (arc);
      if ((arcType (arc) == relation) && (src == src_node))
        break;
    }
  if (!isNil (arc))
    {
      Arc ptr;
      ptr = sourceArcs (dest_node);
      if (ptr == arc)
        {                       /* remove the first link */
          sourceArcs (dest_node) = nextArc (arc);
          nextArc (arc) = 0;
          FreeArc (&arc);
        }
      else
        {
          while (nextArc (ptr) != arc)
            ptr = nextArc (ptr);
          nextArc (ptr) = nextArc (arc);
          nextArc (arc) = 0;
          FreeArc (&arc);
        }
    }
}
/*
 *      Find the arc of the dest_node.
 */
Arc FindSrcArc (Node src_node, Node dest_node, int type)
{
  Arc ptr;
  for (ptr = sourceArcs (dest_node); ptr != 0; ptr = nextArc (ptr))
    {
      if (arcType (ptr) != type)
        continue;
      if (sourceNode (ptr) != src_node)
        continue;
      if (destinationNode (ptr) != dest_node)
        continue;
      break;
    }
  return ptr;
}
/*
 *      Find the arc of the src_node.
 */
Arc FindDestArc (Node src_node, Node dest_node, int type)
{
  Arc ptr;
  for (ptr = destinationArcs (src_node); ptr != 0; ptr = nextArc (ptr))
    {
      if (arcType (ptr) != type)
        continue;
      if (sourceNode (ptr) != src_node)
        continue;
      if (destinationNode (ptr) != dest_node)
        continue;
      break;
    }
  return ptr;
}
/*
 *      Delete a graph.
 */
static Node nd_buf[MAX_GRAPH_SIZE];
static int nd_buf_size = 0;
static int
_Add_2_nd_buf (Node node)
{
  nd_buf[nd_buf_size++] = node;
  if (nd_buf_size >= MAX_GRAPH_SIZE)
    {
      Punt ("RemoveNodes: graph is too large");
      return (-1);
    }
  return (0);
}
void
RemoveNodes (Node * node)
{
  Node nd;
  int i;
  if ((node == 0) || ((*node) == 0))
    return;
  nd = *node;
  *node = 0;
  /*
   *  First place all reachable nodes in an array buffer.
   */
  nd_buf_size = 0;
  VisitAllNodes (nd, _Add_2_nd_buf);
  /*
   *  Return all space allocated.
   */
  for (i = 0; i < nd_buf_size; i++)
    {
      Arc arc, next;
      for (arc = sourceArcs (nd_buf[i]); !isNil (arc); arc = next)
        {
          next = nextArc (arc);
          FreeArc (&arc);
        }
      for (arc = destinationArcs (nd_buf[i]); !isNil (arc); arc = next)
        {
          next = nextArc (arc);
          FreeArc (&arc);
        }
      FreeNode (nd_buf + i);
    }
}
/*
 *      Set the (status) field of all nodes reachable from
 *      the formal parameter to 0.
 *      Returns the number of nodes marked.
 */
int
MarkNodesZero (Node node)
{
  int i;
  /*
   *  Find all nodes that are reachable from (node).
   */
  nd_buf_size = 0;
  VisitAllNodes (node, _Add_2_nd_buf);
  for (i = 0; i < nd_buf_size; i++)
    {
      nodeStatus (nd_buf[i]) = 0;       /* set status to 0 */
    }
  return nd_buf_size;
}
/*
 *      For all reachable nodes from the formal parameter,
 *      if the (status) field is not masked [status&~mask]
 *      set the status field to [mark]
 *      Returns the number of nodes marked.
 */
int
MarkNodes (Node node, int mask, int mark)
{
  int i, n;
  /*
   *  It is an error that mark overlaps with the RESERVED_BITS.
   */
  if (mark & RESERVED_STATUS_BITS)
    Punt ("MarkNodes: illegal mark");
  /*
   *  Find all nodes that are reachable from (node).
   */
  nd_buf_size = 0;
  VisitAllNodes (node, _Add_2_nd_buf);
  n = 0;
  for (i = 0; i < nd_buf_size; i++)
    {
      if ((nodeStatus (nd_buf[i]) & ~mask) != 0)
        {
          nodeStatus (nd_buf[i]) = mark;
          n++;
        }
    }
  return n;
}
/*
 *      For all reachable nodes from the formal parameter,
 *      if [status==mark] then apply fn() on the node, and then
 *      set the [status] field to 0.
 *      fn() is expected to return 0.
 *      Returns the number of nodes, for which fn() is invoked.
 */
int
VisitNodes (Node node, int mark, int (*fn) (Node))
{
  int i, n, code;
  /*
   *  Find all nodes that are reachable from (node).
   */
  nd_buf_size = 0;
  VisitAllNodes (node, _Add_2_nd_buf);
  n = 0;
  code = 0;
  for (i = 0; i < nd_buf_size; i++)
    {
      if (nodeStatus (nd_buf[i]) == mark)
        {
          code |= (*fn) (nd_buf[i]);
          n++;
        }
    }
  if (code != 0)
    return -1;
  return n;
}
/*
 *      For all reachable nodes from the formal parameter,
 *      apply fn(). fn() is expected to return 0.
 *      The number of nodes, for which fn() is invoked
 *      is returned. set the [status] field to 0.
 *
 *      !!! This function must work all by itself. It must also
 *      be extremely efficient.
 *      The RESERVED_STATUS_BITS can be used to mark all nodes
 *      that have been reached.
 *
 *      When this function is called, it is assumed that the
 *      RESERVED_STATUS_BITS of (node) are not set.
 *      If the RESERVED_STATUS_BITS are set, the node is
 *      considered to be visited, and will not be visited again.
 */
static int
MarkStatusBits (Node node)
{
  int n;
  Arc arc;
  if (node == 0)
    return 0;
  if ((nodeStatus (node) & RESERVED_STATUS_BITS) != 0)
    return 0;                   /* already visited */
  nodeStatus (node) |= RESERVED_STATUS_BITS;
  n = 1;
  for (arc = destinationArcs (node); !isNil (arc); arc = nextArc (arc))
    {
      n += MarkStatusBits (destinationNode (arc));
    }
  return n;
}
static int
ClearStatusBits (Node node, int (*fn) (Node))
{
  int code;
  Arc arc;
  if (node == 0)
    return 0;
  /*
   *  Ignore, nodes whose RESERVED_STATUS_BITS are not set.
   */
  if ((nodeStatus (node) & RESERVED_STATUS_BITS) == 0)
    return 0;                   /* already visited */
  nodeStatus (node) &= ~RESERVED_STATUS_BITS;   /* clear bits */
  code = (*fn) (node);          /* invoke the function */
  for (arc = destinationArcs (node); !isNil (arc); arc = nextArc (arc))
    {
      Node dest;
      dest = destinationNode (arc);
      if ((nodeStatus (dest) & RESERVED_STATUS_BITS) != 0)
        code |= ClearStatusBits (dest, fn);
    }
  return code;
}
int
VisitAllNodes (Node node, int (*fn) (Node))
{
  int size;
  /*
   *  Visit all nodes whose RESERVED_STATUS_BITS are 0, and
   *  are reachable from (node).
   */
  size = MarkStatusBits (node);
  /*
   *  Clear all RESERVED_STATUS_BITS.
   *  Execute the requested function.
   */
  if (ClearStatusBits (node, fn) != 0)
    return -1;                  /* error */
  return size;
}
/*
 *      Returns 1 if the graph is not an empty graph, and
 *      it is cyclic. Returns 0 if the graph is empty.
 *      Returns -1 if the graph is acyclic.
 *      The (status) field is set to all 1's if there
 *      is no cycle. The last node of a cycle is set to 2.
 */
static int
FindNonZero (Node node)
{
  int cycle;
  Arc arc;
  if (node == 0)
    return 0;
  if (nodeStatus (node) != 0)
    {                           /* found a cycle */
      return 1;
    }
  cycle = 0;
  nodeStatus (node) = 1;
  for (arc = destinationArcs (node); !isNil (arc); arc = nextArc (arc))
    {
      Node dest;
      dest = destinationNode (arc);
      if (nodeStatus (dest) != 0)
        {
          nodeStatus (node) = 2;        /* last node of a cycle */
          cycle = 1;
        }
      else
        {
          cycle |= FindNonZero (dest);
        }
    }
  return (cycle != 0);
}
int
IsCyclic (Node node)
{
  MarkNodesZero (node);         /* mark all status bits to 0 */
  return FindNonZero (node);
}
/*---------------------------------------------------------*/
static FGraph d_graph_list = 0;
/*
 *      Allocate space for a graph.
 */
FGraph NewGraph (void)
{
  FGraph new_graph;
  if (isNil (d_graph_list))
    {
      new_graph = (FGraph) malloc (sizeof (_FGraph));
      if (new_graph == 0)
        Punt ("NewGraph: cannot allocate space");
    }
  else
    {
      new_graph = d_graph_list;
      d_graph_list = new_graph->next;
    }
  new_graph->root = 0;
  new_graph->nodes = 0;
  new_graph->next = 0;
  new_graph->extension = 0;
  return new_graph;
}
FGraph FreeGraph (FGraph * graph)
{
  Node *nodes;
  if ((graph == 0) || ((*graph) == 0))
    return 0;
  nodes = &((*graph)->nodes);
  if ((*nodes) != 0)
    {
      RemoveNodes (nodes);
    }
  (*graph)->next = d_graph_list;
  d_graph_list = *graph;
  *graph = 0;
  return 0;
}
/*
 *      Add a node into a graph structure.
 *      Must preserve the sequential order.
 *      So it is inserted at the end of the list.
 */
void
AddNode (FGraph graph, Node node)
{
  Node ptr;
  if ((graph == 0) || (node == 0))
    return;
  ptr = graph->nodes;
  if (ptr == 0)
    {
      graph->nodes = node;
    }
  else
    {
      while (nextNode (ptr) != 0)
        ptr = nextNode (ptr);
      nextNode (ptr) = node;
    }
}
/*
 *      Find a node in a graph.
 *      If not found, return 0.
 */
Node FindNode (FGraph graph, int node_id)
{
  Node ptr;
  for (ptr = graph->nodes; ptr != 0; ptr = nextNode (ptr))
    {
      if (nodeId (ptr) == node_id)
        break;
    }
  return ptr;
}
Node DeleteNode (FGraph graph, int node_id)
{
  Node target, ptr;
  target = FindNode (graph, node_id);
  if (target == 0)
    return 0;
  if (graph->nodes == target)
    {
      graph->nodes = nextNode (target);
      nextNode (target) = 0;
    }
  else
    {
      for (ptr = graph->nodes; nextNode (ptr) != target;
           ptr = nextNode (ptr));
      nextNode (ptr) = nextNode (target);
      nextNode (target) = 0;
    }
  return target;
}
/*---------------------------------------------------------*/

#ifdef DEBUG_G
PrintNode (node)
     Node node;
{
  Arc arc;
  printf ("node (id=%d) (status=%d) \n", nodeId (node), nodeStatus (node));
  printf ("  (source =");
  for (arc = sourceArcs (node); !isNil (arc); arc = nextArc (arc))
    {
      printf (" %d", arc->source->id);
    }
  printf (")\n");
  printf ("  (destination =");
  for (arc = destinationArcs (node); !isNil (arc); arc = nextArc (arc))
    {
      printf (" %d", arc->destination->id);
    }
  printf (")\n");
  return 0;
}
PrintGraph (root)
     Node root;
{
  int n;
  n = VisitAllNodes (root, PrintNode);
  printf ("> total %d nodes printed\n\n", n);
}
main ()
{
  int i;
  Node graph;
  Node node1, node2;
  Arc arc1, arc2;
        /** test NewNode(), FreeNode(), NewArc(), FreeArc(),
         ** ConnectNodes(), DisconnectNodes(), RemoveNodes()
         **/
  graph = NewNode ();
  graph->id = 0;
  for (i = 1; i < 5; i++)
    {
      node1 = NewNode ();
      node1->id = i;
      ConnectNodes (graph, node1, i);
      ConnectNodes (node1, graph, i);
    }
  printf ("> is cyclic? = %d\n", IsCyclic (graph));
  PrintGraph (graph);

  RemoveNodes (&graph);         /* 0 */
  printf ("> is cyclic? = %d\n", IsCyclic (graph));
  PrintGraph (graph);

  graph = NewNode ();
  graph->id = 0;
  for (i = 1; i < 5; i++)
    {
      node1 = NewNode ();
      node1->id = i;
      node2 = NewNode ();
      node2->id = -i;
      ConnectNodes (graph, node1, i);
      ConnectNodes (node1, node2, i);
    }
  printf ("> is cyclic? = %d\n", IsCyclic (graph));
  PrintGraph (graph);

  DisconnectNodes (graph, node1, i - 1);
  printf ("> is cyclic? = %d\n", IsCyclic (graph));
  PrintGraph (graph);
  /*
   *      Test MarkNodesZero(), MarkNodes(), VisitNodes()
   */
  MarkNodesZero (graph);
  PrintGraph (graph);

  graph = NewNode ();
  graph->id = 0;
  graph->status = 0;
  for (i = 1; i < 5; i++)
    {
      node1 = NewNode ();
      node1->id = i;
      node1->status = i;
      node2 = NewNode ();
      node2->id = -i;
      node2->status = i;
      ConnectNodes (graph, node1, i);
      ConnectNodes (node1, node2, i);
      ConnectNodes (node1, graph, i);
    }
  PrintGraph (graph);

  MarkNodes (graph, 3, 100);
  PrintGraph (graph);

  VisitNodes (graph, 3, PrintGraph);
}
#endif
