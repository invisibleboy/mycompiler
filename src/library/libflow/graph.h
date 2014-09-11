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
 *      File:   graph.h
 *      Author: Po-hua Chang
 *      Creation Date:  XXX
 *      Modified By: XXX, date, time, why
 *      Copyright (c) 1991 Po-hua Chang and Wen-mei Hwu. All rights reserved.  
 *      All rights granted to the University of Illinois.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

/*===========================================================================*\
 *
 *      Many interesting problems can be mapped to graph models.
 *      A particular powerful representation, weighted control
 *      graph, is a good data structure for representing programs.
 *      G = (n, N, E), where
 *      n = a designated entry node,
 *      N = a set of nodes,
 *      E = a set of arcs connecting nodes.
 *
 *      Each node is identified by a unique node_id attribute
 *      within a graph. The node_type attribute can be used to
 *      represent "group" information, the "access privilege",
 *      the "device type", and ... etc, in many interesting
 *      problems. Each node has a node_weight which indicates
 *      the "importance" of the node, relative to other nodes
 *      in the graph. For example, when modeling a program, the
 *      node_weight attribute tells the invocation frequency of
 *      a function (or a basic block). In many applications, it
 *      is necessary to distinguish the set of incoming arcs and
 *      the set of outgoing arcs. To achieve high efficiency,
 *      two linear lists of arcs are provided (instead of one
 *      unified list). Finally the extension field allows the
 *      application designer to design a special work-space.
 *
 *      Each arc has a 'type' attribute which tells us the
 *      relation between the two connecting nodes. For example,
 *      in a control flow graph, the relation is the condition
 *      on which the transition is taken (case, true, false, ...etc).
 *      Each arc has a 'weight' attribute, which tells us the
 *      relative importance of the arc. The source and destination
 *      pointers point to the connecting nodes. Finally, an extension
 *      field is reserved for special application requirement.
 *
\*===========================================================================*/
#ifndef IMPACT_GRAPH_H
#define IMPACT_GRAPH_H

/* 10/29/02 REK Adding config.h */
#include <config.h>

#define MAX_GRAPH_SIZE  (8*1024)        /* max. # of nodes in a graph */

/*
 *      The correct range of the (status) field is
 *      0<= status < RESERVED_STATUS_BITS
 *      The user must at all time make sure that the
 *      RESERVED_STATUS_BITS of the status fields are 
 *      all 0s. This bit is used for internal operations.
 */
#define RESERVED_STATUS_BITS    0x40000000

typedef void *_NodeExtension;

typedef struct _Node
{
  int id;                       /* node id */
  int type;                     /* the type of the node */
  int status;                   /* the status of the node */
  double weight;                /* the weight */
  struct _Arc *source;          /* incoming arcs */
  struct _Arc *destination;     /* outgoing arcs */
  struct _Node *next;           /* reserved: internal use only */
  _NodeExtension extension;     /* extension fields */
}
_Node, *Node;

typedef void *_ArcExtension;

typedef struct _Arc
{
  int type;                     /* the type of the arc */
  double weight;                /* the arc weight */
  struct _Node *source;         /* the source node */
  struct _Node *destination;    /* the destination node */
  struct _Arc *next;            /* reserved: internal use only */
  _ArcExtension extension;      /* extension fields */
}
_Arc, *Arc;

typedef void *_GraphExtension;

typedef struct _FGraph
{
  struct _Node *root;           /* the root of the graph */
  struct _Node *nodes;          /* all nodes in the graph */
  struct _FGraph *next;         /* reserved: internal use only */
  _GraphExtension extension;    /* extension fields */
}
_FGraph, *FGraph;

/*
 *      It is recommanded that the following MACROs be
 *      used to address the components of the node and
 *      arc structures. This makes the program more
 *      understandable, especially when short variable
 *      names are used (e.g. x,n,y). It is very easy to
 *      confuse between the components of arcs and nodes.
 */
#define isNil(n)                (n==0)
#define nodeId(n)               n->id
#define nodeType(n)             n->type
#define nodeStatus(n)           n->status
#define nodeWeight(n)           n->weight
#define sourceArcs(n)           n->source
#define destinationArcs(n)      n->destination
#define nextNode(n)             n->next
#define nodeExt(n)              n->extension
#define arcType(a)              a->type
#define arcWeight(a)            a->weight
#define sourceNode(a)           a->source
#define destinationNode(a)      a->destination
#define nextArc(a)              a->next
#define arcExt(a)               a->extension

#ifdef __cplusplus
extern "C"
{
#endif

/* graph.c 
 *      A connection is uniquely defined by (src_node, dest_node, type)
 *      It is an error to make a connection twice.
 */
  extern Node NewNode (void);
  extern Node FreeNode (Node * node);
  extern Arc NewArc (void);
  extern Arc FreeArc (Arc * arc);
  extern void ConnectNodes (Node src_node, Node dest_node, int relation);
  extern void DisconnectNodes (Node src_node, Node dest_node, int relation);
  extern Arc FindSrcArc (Node src_node, Node dest_node, int type);
  extern Arc FindDestArc (Node src_node, Node dest_node, int type);
  extern void RemoveNodes (Node * node);

/*
 *      The following functions modify the (status) field.
 *      MarkNodesZero() sets all status fileds to 0.
 *      VisitNodes() and VisitAllNodes() do not change the status fields.
 *      IsCyclic() changes the status fields to 1 or 2.
 */
  extern int MarkNodesZero (Node node);
  extern int MarkNodes (Node node, int mask, int mark);
  extern int VisitNodes (Node node, int mark, int (*fn) (Node));
  extern int VisitAllNodes (Node node, int (*fn) (Node));
  extern int IsCyclic (Node node);

/*
 *      The following functions operate on a graph structure.
 */
  extern FGraph NewGraph (void);
  extern FGraph FreeGraph (FGraph * graph);
  extern void AddNode (FGraph graph, Node node);
  extern Node FindNode (FGraph graph, int node_id);
  extern Node DeleteNode (FGraph graph, int node_id);

#ifdef __cplusplus
}
#endif

#endif
