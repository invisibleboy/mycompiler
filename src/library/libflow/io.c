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
 *      File:   io.c
 *      Author: Pohua Chang
 *      Copyright (c) 1991 Pohua Chang, Wen-Mei Hwu. All rights reserved.  
 *      All rights granted to the University of Illinois.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <stdlib.h>
#include <string.h>
#include <library/io.h>

/*
 *      graph =
 *              root node* arc*
 *      node =
 *              (node (id %d) (type %d) (status %d) (weight %f) (ext ...))
 *      arc = 
 *              (arc (src %d) (dest %d) (type %d) (weight %f) (ext ...))
 *      root =
 *              (root node_id%d (ext ...))
 *
 *      ... are passed to the ext_handlers.
 */
void *(*graph_ext_handler) (LIST) = 0;
void *(*node_ext_handler) (LIST) = 0;
void *(*arc_ext_handler) (LIST) = 0;

int (*graph_ext_output) (_GraphExtension, FILE *) = 0;
int (*node_ext_output) (_NodeExtension, FILE *) = 0;
int (*arc_ext_output) (_ArcExtension, FILE *) = 0;

static void
Punt (char *mesg)
{
  fprintf (stderr, "Error in io.c: %s\n", mesg);
  exit (1);
}
/*-----------------------------------------------------------------------*/
static void
DecodeRoot (LIST Largs, FGraph graph)
{
  LIST Lid, Lopcode, Lpars;
  Node root;
  char *opcode;
  int id;
  Lid = Largs;
  if ((Lid == 0) || (NodeType (Lid) != T_INT))
    Punt ("illegal (root) descriptor");
  id = IntegerOf (Lid);
  Lpars = SiblingOf (Lid);
  /*
   *  Find the node(id)
   */
  root = FindNode (graph, id);
  if (root == 0)
    {
      root = NewNode ();
      nodeId (root) = id;
      AddNode (graph, root);
    }
  graph->root = root;
  for (Largs = SiblingOf (Largs); Largs != 0; Largs = SiblingOf (Largs))
    {
      if (NodeType (Largs) != T_LIST)
        continue;
      Lopcode = ChildOf (Largs);
      if ((Lopcode == 0) || (NodeType (Lopcode) != T_ID))
        continue;
      opcode = StringOf (Lopcode);
      Lpars = SiblingOf (Lopcode);
      if (!strcmp (opcode, "ext"))
        {
          if (graph_ext_handler != 0)
            graph->extension = (*graph_ext_handler) (Lpars);
        }
    }
}
static void
DecodeNode (LIST Largs, FGraph graph)
{
  LIST Lopcode, Lpars;
  Node node;
  char *opcode;
  node = 0;
  for (; Largs != 0; Largs = SiblingOf (Largs))
    {
      if (NodeType (Largs) != T_LIST)
        continue;
      Lopcode = ChildOf (Largs);
      if ((Lopcode == 0) || (NodeType (Lopcode) != T_ID))
        continue;
      opcode = StringOf (Lopcode);
      Lpars = SiblingOf (Lopcode);
      if (!strcmp (opcode, "id"))
        {
          int n_id;
          if ((Lpars == 0) || (NodeType (Lpars) != T_INT))
            continue;
          n_id = IntegerOf (Lpars);
          if ((node = FindNode (graph, n_id)) == 0)
            {
              node = NewNode ();
              nodeId (node) = n_id;
              AddNode (graph, node);
            }
        }
      else if (!strcmp (opcode, "type"))
        {
          if (node == 0)
            Punt ("illegal (node) descriptor 1");
          if ((Lpars == 0) || (NodeType (Lpars) != T_INT))
            continue;
          nodeType (node) = IntegerOf (Lpars);
        }
      else if (!strcmp (opcode, "status"))
        {
          if (node == 0)
            Punt ("illegal (node) descriptor 1");
          if ((Lpars == 0) || (NodeType (Lpars) != T_INT))
            continue;
          nodeStatus (node) = IntegerOf (Lpars);
        }
      else if (!strcmp (opcode, "weight"))
        {
          if (node == 0)
            Punt ("illegal (node) descriptor 1");
          if ((Lpars == 0) || (NodeType (Lpars) != T_REAL))
            continue;
          nodeWeight (node) = RealOf (Lpars);
        }
      else if (!strcmp (opcode, "ext"))
        {
          if (node == 0)
            Punt ("illegal (node) descriptor 1");
          if (node_ext_handler != 0)
            nodeExt (node) = (*node_ext_handler) (Lpars);
        }
      else
        {
          /* do nothing */
        }
    }
}
static void
DecodeArc (LIST Largs, FGraph graph)
{
  LIST Lopcode, Lpars;
  char *opcode;
  int src_id, dest_id, type;
  double weight;
  Node node, src_node = NULL, dest_node = NULL;
  void *extension = NULL;
  Arc arc;
  src_id = dest_id = type = 0;
  weight = 0.0;
  for (; Largs != 0; Largs = SiblingOf (Largs))
    {
      if (NodeType (Largs) != T_LIST)
        continue;
      Lopcode = ChildOf (Largs);
      if ((Lopcode == 0) || (NodeType (Lopcode) != T_ID))
        continue;
      opcode = StringOf (Lopcode);
      Lpars = SiblingOf (Lopcode);
      if (!strcmp (opcode, "src"))
        {
          if ((Lpars == 0) || (NodeType (Lpars) != T_INT))
            continue;
          src_id = IntegerOf (Lpars);
          if ((node = FindNode (graph, src_id)) == 0)
            {
              node = NewNode ();
              nodeId (node) = src_id;
              AddNode (graph, node);
            }
          src_node = node;
        }
      else if (!strcmp (opcode, "dest"))
        {
          if ((Lpars == 0) || (NodeType (Lpars) != T_INT))
            continue;
          dest_id = IntegerOf (Lpars);
          if ((node = FindNode (graph, dest_id)) == 0)
            {
              node = NewNode ();
              nodeId (node) = dest_id;
              AddNode (graph, node);
            }
          dest_node = node;
        }
      else if (!strcmp (opcode, "type"))
        {
          if ((Lpars == 0) || (NodeType (Lpars) != T_INT))
            continue;
          type = IntegerOf (Lpars);
        }
      else if (!strcmp (opcode, "weight"))
        {
          if ((Lpars == 0) || (NodeType (Lpars) != T_REAL))
            continue;
          weight = RealOf (Lpars);
        }
      else if (!strcmp (opcode, "ext"))
        {
          if (arc_ext_handler != 0)
            extension = (*arc_ext_handler) (Lpars);
        }
      else
        {
          /* do nothing */
        }
    }
  /*
   *  Insert arc into the graph.
   */
  ConnectNodes (src_node, dest_node, type);
  arc = FindSrcArc (src_node, dest_node, type);
  arcWeight (arc) = weight;
  arcExt (arc) = extension;
  arc = FindDestArc (src_node, dest_node, type);
  arcWeight (arc) = weight;
  arcExt (arc) = extension;
}
/*
 *      Read the graph specification from a file,
 *      construct a graph structure, and return
 *      a pointer to the root of the graph (the first node)
 */
FGraph ReadGraph (char *file_name)
{
  FGraph graph;
  LIST node;
  FILE *F;
  if (!strcmp (file_name, "stdin"))
    {
      F = stdin;
    }
  else
    {
      F = fopen (file_name, "r");
    }
  if (F == 0)
    Punt ("ReadGraph: cannot open file");
  graph = NewGraph ();          /* create a new graph */
  SwitchFile (F);
  while ((node = GetNode ()) != 0)
    {
      if (NodeType (node) == T_EOF)
        break;
      if (NodeType (node) == T_LIST)
        {
          LIST opcode, args;
          char *opc;
          opcode = ChildOf (node);
          if ((opcode == 0) || (NodeType (opcode) != T_ID))
            goto SKIP;
          opc = StringOf (opcode);
          args = SiblingOf (opcode);
          if (!strcmp (opc, "root"))
            {
              DecodeRoot (args, graph);
            }
          else if (!strcmp (opc, "node"))
            {
              DecodeNode (args, graph);
            }
          else if (!strcmp (opc, "arc"))
            {
              DecodeArc (args, graph);
            }
          else
            goto SKIP;
        }
    SKIP:
      DisposeNode (node);
    }
  return graph;
}
/*-----------------------------------------------------------------------*/
/*
 *      Write a graph structure to a file.
 */
void
WriteGraph (char *file_name, FGraph graph)
{
  FILE *F;
  Node nptr;
  Arc aptr;
  if (!strcmp (file_name, "stdout"))
    {
      F = stdout;
    }
  else if (!strcmp (file_name, "stderr"))
    {
      F = stderr;
    }
  else
    {
      F = fopen (file_name, "w");
    }
  if (F == 0)
    Punt ("WriteGraph: cannot open file");
  if (graph == 0)
    return;
  /*
   *  root
   */
  if (graph->root != 0)
    {
      fprintf (F, "(root %d", nodeId (graph->root));
      if (graph_ext_output != 0)
        (*graph_ext_output) (graph->extension, F);
      fprintf (F, ")\n");
    }
  /*
   *  node*
   */
  for (nptr = graph->nodes; nptr != 0; nptr = nextNode (nptr))
    {
      fprintf (F, "(node (id %d)(type %d)(status %d)(weight %f)",
               nodeId (nptr), nodeType (nptr), nodeStatus (nptr),
               nodeWeight (nptr));
      if (node_ext_output != 0)
        (*node_ext_output) (nodeExt (nptr), F);
      fprintf (F, ")\n");
    }
  /*
   *  arc*
   */
  for (nptr = graph->nodes; nptr != 0; nptr = nextNode (nptr))
    {
      for (aptr = destinationArcs (nptr); aptr != 0; aptr = nextArc (aptr))
        {
          fprintf (F, "(arc (src %d)(dest %d)(type %d)(weight %f)",
                   nodeId (sourceNode (aptr)),
                   nodeId (destinationNode (aptr)),
                   arcType (aptr), arcWeight (aptr));
          if (arc_ext_output != 0)
            (*arc_ext_output) (arcExt (aptr), F);
          fprintf (F, ")\n");
        }
    }
  fflush (F);
  if ((F != stdout) && (F != stderr))
    fclose (F);
}

/*-----------------------------------------------------------------------*/
#ifdef DEBUG_IO
main ()
{
  FGraph g;
  g = ReadGraph ("stdin");
  WriteGraph ("stdout", g);
}
#endif
