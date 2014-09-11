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
 *      File:   trace.c
 *      Author: Pohua Chang
 *      Copyright (c) 1991 Pohua Chang, Wen-mei Hwu. All rights reserved.  
 *      All rights granted to the University of Illinois.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <library/trace.h>
#include <library/select_sort.h>
#include <library/set.h>

#undef DEBUG

/*
 *      scale the importance of level information.
 */
#define SCALE           5
#define FAVOR_FORWARD   2

/*
 *      There was problems with the trace.c program.
 *      Ofter too may jumps are generated.
 *      Also, loop entry blocks are sometimes chosen
 *      as the seed and became the middle of a trace.
 *      In this version, I will try to do two things:
 *      for frequently executed sections, reduce the
 *      dynamic jumps, for less frequently executed
 *      sections, reduce the number of static jumps.
 *      The new scheme should require no second level
 *      trace placement.
 */
#undef WEIGHT_BASED
#define SECOND_LEVEL_SELECT

/*------------------------------------------------------------------*/
static void
Punt (char *mesg)
{
  fprintf (stderr, "> error in trace selection: %s\n", mesg);
  exit (1);
}
/*------------------------------------------------------------------*/
/*
 *      I will define the extension field.
 */
static struct Ext
{
  Set dominator;
  int level;
  int order;
}
others[MAX_GRAPH_SIZE];
static int n_others = 0;

static void
FreeExtensionFields (void)
{
  int i;
  for (i = 0; i < n_others; i++)
    {
      Set_dispose (others[i].dominator);
      others[i].dominator = 0;
    }
  n_others = 0;
}
static void
AddExtensionFields (FGraph graph)
{
  Node ptr;
  Arc arc;
  struct Ext *ext;
  int change;
  Set all;
  FreeExtensionFields ();
  if (graph->nodes == 0)
    return;
  if (graph->root == 0)
    Punt ("no root node");
  all = 0;
  for (ptr = graph->nodes; ptr != 0; ptr = nextNode (ptr))
    {
      nodeExt (ptr) = (others + n_others);
      others[n_others].dominator = 0;
      others[n_others].level = -1;
      others[n_others].order = n_others;
      all = Set_add (all, n_others);
      n_others += 1;
    }
  ext = (struct Ext *) nodeExt (graph->root);
  ext->level = 0;
  ext->dominator = Set_add (0, ext->order);
  for (ptr = graph->nodes; ptr != 0; ptr = nextNode (ptr))
    {
      if (ptr == graph->root)
        continue;
      ext = (struct Ext *) nodeExt (ptr);
      ext->dominator = Set_union (0, all);
    }
  /*
   *  compute dominators.
   */
  change = 1;
  while (change != 0)
    {
      change = 0;
      for (ptr = graph->nodes; ptr != 0; ptr = nextNode (ptr))
        {
          Set dom, temp;
          if (ptr == graph->root)
            continue;
          arc = sourceArcs (ptr);
          if (arc == 0)
            {
              dom = 0;
            }
          else
            {
              dom = Set_union (all, 0);
              for (; arc != 0; arc = nextArc (arc))
                {
                  ext = (struct Ext *) nodeExt (sourceNode (arc));
                  temp = Set_intersect (dom, ext->dominator);
                  Set_dispose (dom);
                  dom = temp;
                  temp = 0;
                }
              ext = (struct Ext *) nodeExt (ptr);
              dom = Set_add (dom, ext->order);
              if (Set_same (dom, ext->dominator))
                {
                  Set_dispose (dom);
                }
              else
                {
                  Set_dispose (ext->dominator);
                  ext->dominator = dom;
                  change += 1;
                }
            }
        }
    }
  all = Set_dispose (all);
  /*
   *  compute level.
   */
  change = 1;
  while (change != 0)
    {
      change = 0;
      for (ptr = graph->nodes; ptr != 0; ptr = nextNode (ptr))
        {
          int max;
          ext = (struct Ext *) nodeExt (ptr);
          if (ext->level >= 0)
            continue;
          max = -1;
          for (arc = sourceArcs (ptr); arc != 0; arc = arc->next)
            {
              Node src;
              struct Ext *ex;
              src = sourceNode (arc);
              ex = (struct Ext *) nodeExt (src);
              /* ignore back-edges */
              if (ex->order >= ext->order)
                continue;
              if (ex->level < 0)
                break;
              if (ex->level > max)
                max = ex->level;
            }
          if (arc == 0)
            {                   /* all source have been visited */
              ext->level = max + 1;
              change += 1;
            }
        }
    }
#ifdef DEBUG
  printf ("--------------------------------------------------\n");
  for (ptr = graph->nodes; ptr != 0; ptr = nextNode (ptr))
    {
      ext = nodeExt (ptr);
      printf ("# id=%d, order=%d, level=%d, ",
              ptr->id, ext->order, ext->level);
      PrintSet (stdout, "dominator", ext->dominator);
    }
#endif
}
/*------------------------------------------------------------------*/
static double
successor_node_weight (Node src, Node dest)
{
  struct Ext *extSRC, *extDEST;
  int diff;
  double weight;
  extSRC = (struct Ext *) nodeExt (src);
  extDEST = (struct Ext *) nodeExt (dest);
  /*
   *  when choosing a successor, select one
   *  that is closer to it (level information)
   *  when the dynamic count is low.
   */
  diff = extDEST->level - extSRC->level;
  if (diff > 0)
    {
      weight = nodeWeight (dest) + ((FAVOR_FORWARD - diff) * SCALE);
    }
  else
    {
      weight = nodeWeight (dest);
    }
  return weight;
}
static double
predecessor_node_weight (Node src, Node dest)
{
  struct Ext *extSRC, *extDEST;
  int diff;
  double weight;
  extSRC = (struct Ext *) nodeExt (src);
  extDEST = (struct Ext *) nodeExt (dest);
  diff = extDEST->level - extSRC->level;
  if (diff > 0)
    {
      weight = nodeWeight (src) + (diff * SCALE);
    }
  else
    {
      weight = nodeWeight (src);
    }
  return weight;
}
static double
successor_arc_weight (Arc arc)
{
  struct Ext *extSRC, *extDEST;
  int diff;
  double weight;
  extSRC = (struct Ext *) nodeExt (sourceNode (arc));
  extDEST = (struct Ext *) nodeExt (destinationNode (arc));
  diff = extDEST->level - extSRC->level;
  /*
   *  when choosing a successor, select one
   *  that is closer to it (level information)
   *  when the dynamic count is low.
   */
  if (diff > 0)
    {
      weight = arcWeight (arc) + ((FAVOR_FORWARD - diff) * SCALE);
    }
  else
    {
      weight = arcWeight (arc);
    }
  return weight;
}
static double
predecessor_arc_weight (Arc arc)
{
  struct Ext *extSRC, *extDEST;
  int diff;
  double weight;
  extSRC = (struct Ext *) nodeExt (sourceNode (arc));
  extDEST = (struct Ext *) nodeExt (destinationNode (arc));
  diff = extDEST->level - extSRC->level;
  if (diff > 0)
    {
      weight = arcWeight (arc) + (diff * SCALE);
    }
  else
    {
      weight = arcWeight (arc);
    }
  return weight;
}
/*------------------------------------------------------------------*/
/*
 *      Heuristic solution.
 */
static Arc (*best_successor_of) (Node);
static Arc (*best_predecessor_of) (Node);
static double min_prob_requirement = 0.0;
/*
 *      Selection by node weight.
 */
static Arc
best_successor_1 (Node node)
{
  Node src, dest;
  Arc ln, best;
  src = node;
  /*
   *  Find a destination node with the largest weight.
   */
  dest = 0;
  best = 0;
  for (ln = destinationArcs (src); ln != 0; ln = nextArc (ln))
    {
      Node new_node;
      new_node = destinationNode (ln);
      if (dest == 0)
        {
          dest = new_node;
          best = ln;
        }
      else
        {
          if (successor_node_weight (src, new_node)
              > successor_node_weight (src, dest))
            {
              dest = new_node;
              best = ln;
            }
        }
    }
  /*
   *  The selected node must be unvisited, and
   *  has non-zero weight.
   */
  if ((dest == 0) || (nodeStatus (dest) & VISITED))
    return 0;
  if (nodeWeight (dest) == 0.0)
    return 0;
  /*
   *  Return the best link.
   */
  return best;
}
static Arc
best_predecessor_1 (Node node)
{
  Node src, dest;
  Arc ln, best;
  dest = node;
  /*
   *  Find a source node with the largest weight.
   */
  src = 0;
  best = 0;
  for (ln = sourceArcs (dest); ln != 0; ln = nextArc (ln))
    {
      Node new_node;
      new_node = sourceNode (ln);
      if (src == 0)
        {
          src = new_node;
          best = ln;
        }
      else
        {
          if (predecessor_node_weight (new_node, dest)
              > predecessor_node_weight (src, dest))
            {
              src = new_node;
              best = ln;
            }
        }
    }
  /*
   *  The selected node must be unvisited, and
   *  has non-zero weight.
   */
  if ((src == 0) || (nodeStatus (src) & VISITED))
    return 0;
  if (nodeWeight (src) == 0.0)
    return 0;
  /*
   *  Return the best link.
   */
  return best;
}
/*
 *      Selection by arc weight.
 */
static Arc
best_successor_2 (Node node)
{
  Node src, dest;
  Arc ln, best;
  src = node;
  /*
   *  Find an outgoing arc with the highest execution count.
   */
  best = 0;
  for (ln = destinationArcs (src); ln != 0; ln = nextArc (ln))
    {
      if (best == 0)
        {
          best = ln;
        }
      else
        {
          if (successor_arc_weight (ln) > successor_arc_weight (best))
            best = ln;
        }
    }
  /*
   *  The best link must have non-zero weight.
   */
  if ((best == 0) || (arcWeight (best) == 0.0))
    return 0;
  dest = destinationNode (best);
  /*
   *  The destination node must be un-visited.
   */
  if (nodeStatus (dest) & VISITED)
    return 0;
  /*
   *  Return the best link.
   */
  return best;
}
static Arc
best_predecessor_2 (Node node)
{
  Node src, dest;
  Arc ln, best;
  dest = node;
  /*
   *  Find an incoming arc with the highest execution count.
   */
  best = 0;
  for (ln = sourceArcs (dest); ln != 0; ln = nextArc (ln))
    {
      if (best == 0)
        {
          best = ln;
        }
      else
        {
          if (predecessor_arc_weight (ln) > predecessor_arc_weight (best))
            best = ln;
        }
    }
  /*
   *  The best link must have non-zero weight.
   */
  if ((best == 0) || (arcWeight (best) == 0.0))
    return 0;
  src = sourceNode (best);
  /*
   *  The source node must be un-visited.
   */
  if (nodeStatus (src) & VISITED)
    return 0;
  /*
   *  Return the best link.
   */
  return best;
}
/*
 *      Selection by arc weight with minimum branch 
 *      probability requirement.
 */
static Arc
best_successor_3 (Node node)
{
  Node src, dest;
  Arc ln, best;
  src = node;
  /*
   *  Find an outgoing arc with the highest execution count.
   */
  best = 0;
  for (ln = destinationArcs (src); ln != 0; ln = nextArc (ln))
    {
      if (best == 0)
        {
          best = ln;
        }
      else
        {
          if (successor_arc_weight (ln) > successor_arc_weight (best))
            best = ln;
        }
    }
  /*
   *  The best link must have non-zero weight.
   */
  if ((best == 0) || (arcWeight (best) == 0.0))
    return 0;
  dest = destinationNode (best);
  /*
   *  The destination node must be un-visited.
   */
  if (nodeStatus (dest) & VISITED)
    return 0;
  /*
   *  Must fulfill the min_prob_requirement.
   */
  if ((arcWeight (best) / nodeWeight (src)) < min_prob_requirement)
    return 0;
  if ((arcWeight (best) / nodeWeight (dest)) < min_prob_requirement)
    return 0;
  /*
   *  Return the best link.
   */
  return best;
}
static Arc
best_predecessor_3 (Node node)
{
  Node src, dest;
  Arc ln, best;
  dest = node;
  /*
   *  Find an incoming arc with the highest execution count.
   */
  best = 0;
  for (ln = sourceArcs (dest); ln != 0; ln = nextArc (ln))
    {
      if (best == 0)
        {
          best = ln;
        }
      else
        {
          if (predecessor_arc_weight (ln) > predecessor_arc_weight (best))
            best = ln;
        }
    }
  /*
   *  The best link must have non-zero weight.
   */
  if ((best == 0) || (arcWeight (best) == 0.0))
    return 0;
  src = sourceNode (best);
  /*
   *  The source node must be un-visited.
   */
  if (nodeStatus (src) & VISITED)
    return 0;
  if ((arcWeight (best) / nodeWeight (src)) < min_prob_requirement)
    return 0;
  if ((arcWeight (best) / nodeWeight (dest)) < min_prob_requirement)
    return 0;
  /*
   *  Return the best link.
   */
  return best;
}
/*
 *      Heuristic solution to the trace selection problem.
 */
static void
SelectTraces (FGraph graph)
{
  ST_Entry sorted_list[MAX_GRAPH_SIZE];
  int list_length;
  Node ptr;
  Arc arc;
  int n, trace_id;
  Node ENTRY;
  Node seed, current, last;
  struct Ext *extCurrent, *extNext;
  /*
   *  Compute level information.
   */
  AddExtensionFields (graph);
  /*
   *  Mark all nodes unvisited.
   *  Also clear all status bits.
   */
  for (ptr = graph->nodes; ptr != 0; ptr = nextNode (ptr))
    {
      nodeType (ptr) = 0;       /* trace id = 0 */
      nodeStatus (ptr) = NOT_VISITED;
      for (arc = sourceArcs (ptr); arc != 0; arc = nextArc (arc))
        arcType (arc) = 0;
      for (arc = destinationArcs (ptr); arc != 0; arc = nextArc (arc))
        arcType (arc) = 0;
    }
  /*
   *  Sort all nodes according to the node weight.
   *  From the most important node to the least important node.
   */
  list_length = 0;
  for (ptr = graph->nodes; ptr != 0; ptr = nextNode (ptr))
    {
      sorted_list[list_length].weight = nodeWeight (ptr);
      sorted_list[list_length].ptr = ptr;
      sorted_list[list_length].index = list_length;
      list_length += 1;
      if (list_length >= MAX_GRAPH_SIZE)
        Punt ("SelectTrace: graph is too large");
    }
  max_sort (sorted_list, list_length);  /* library/sort.c */
#ifdef WEIGHT_BASED
  /*
   *  Give higher priority to the execution weight.
   */
  ENTRY = graph->root;
  trace_id = 1;
  for (n = 0; n < list_length; n++)
    {
      /*
       *      Select the highest weight un-visited node.
       */
      seed = sorted_list[n].ptr;
      if (nodeStatus (seed) & VISITED)  /* if visited, try next */
        continue;               /* highest node */
      /*
       *      Start a new trace.
       *      Mark seed visited.
       */
      nodeType (seed) = trace_id++;
      nodeStatus (seed) |= VISITED;
      /*
       *      Grow the trace forward.
       */
      current = seed;
      for (;;)
        {
          Arc ln, arc;
          Node dest;
          ln = best_successor_of (current);
          if (ln == 0)
            {
              nodeStatus (current) |= TRACE_TAIL;
              break;
            }
          /*
           *  Cannot include ENTRY node.
           */
          dest = destinationNode (ln);
          if (dest == ENTRY)
            {
              nodeStatus (current) |= TRACE_TAIL;
              break;
            }
          /*
           *  Can not include a dominator.
           */
          extCurrent = nodeExt (current);
          extNext = nodeExt (dest);
          if ((extCurrent == 0) | (extNext == 0))
            {
              Punt ("corrupted extension field");
            }
          if (Set_in (extCurrent->dominator, extNext->order))
            {
              nodeStatus (current) |= TRACE_TAIL;
              break;
            }
          /*
           *  Mark the link selected.
           */
          arc = FindSrcArc (current, dest, 0);
          if (arc != 0)
            {
              arcType (arc) = nodeType (seed);
            }
          else
            Punt ("SelectTraces: corrupted source link");
          arc = FindDestArc (current, dest, 0);
          if (arc != 0)
            {
              arcType (arc) = nodeType (seed);
            }
          else
            Punt ("SelectTraces: corrupted destination link");
          /*
           *  Add it to the trace.
           */
          nodeType (dest) = nodeType (seed);
          nodeStatus (dest) |= VISITED;
          current = dest;
        }
      /*
       *      Grow the trace backward.
       */
      current = seed;
      for (;;)
        {
          Arc ln, arc;
          Node src;
          /*
           *  Cannot grow from the ENTRY node.
           */
          if (current == ENTRY)
            {
              nodeStatus (current) |= TRACE_HEAD;
              break;
            }
          ln = best_predecessor_of (current);
          if (ln == 0)
            {
              nodeStatus (current) |= TRACE_HEAD;
              break;
            }
          src = sourceNode (ln);
          /*
           *  Can not include a dominee.
           */
          extCurrent = nodeExt (current);
          extNext = nodeExt (src);
          if ((extCurrent == 0) | (extNext == 0))
            {
              Punt ("corrupted extension field");
            }
          if (Set_in (extNext->dominator, extCurrent->order))
            {
              nodeStatus (current) |= TRACE_HEAD;
              break;
            }
          /*
           *  Mark the link selected.
           */
          arc = FindSrcArc (src, current, 0);
          if (arc != 0)
            {
              arcType (arc) = nodeType (seed);
            }
          else
            Punt ("SelectTraces: corrupted source link");
          arc = FindDestArc (src, current, 0);
          if (arc != 0)
            {
              arcType (arc) = nodeType (seed);
            }
          else
            Punt ("SelectTraces: corrupted destination link");
          /*
           *  Add it to the trace.
           */
          nodeType (src) = nodeType (seed);
          nodeStatus (src) |= VISITED;
          current = src;
        }
    }
#else
  /*
   *  Give more attention to the natural flow of the
   *  program.
   */
  ENTRY = graph->root;
  trace_id = 1;
  current = ENTRY;              /* start from the entry block */
  last = 0;
  for (n = 0; n < list_length; n++)
    {
      if (!(nodeStatus (current) & VISITED))
        {
          /*
           *  1) start from the start node first.
           */
          seed = current;
        }
      else
        {
          /*
           *  2) select a fall_thru node if possible.
           *  3) select the most important node.
           */
          Node best, dest;
          Arc pc;
          if (last == 0)
            Punt ("this should not happen");
          best = 0;
          for (pc = destinationArcs (last); pc != 0; pc = nextArc (pc))
            {
              dest = destinationNode (pc);
              if (nodeStatus (dest) & VISITED)
                continue;
              if (best == 0)
                {
                  best = dest;
                }
              else
                if (successor_node_weight (last, dest)
                    > successor_node_weight (last, best))
                {
                  best = dest;
                }
            }
          if (best == 0)
            {
              int k;
              for (k = 0; k < list_length; k++)
                {
                  dest = (Node) sorted_list[k].ptr;
                  if (nodeStatus (dest) & VISITED)
                    continue;
                  best = dest;
                  break;
                }
            }
          if (best == 0)
            {
              break;
            }
          seed = best;
        }
      if (nodeStatus (seed) & VISITED)
        Punt ("incorrect seed selection");
      /*
       *      Start a new trace.
       *      Mark seed visited.
       */
      nodeType (seed) = trace_id++;
      nodeStatus (seed) |= VISITED;
      /*
       *      Grow the trace forward.
       */
      current = seed;
      for (;;)
        {
          Arc ln, arc;
          Node dest;
          ln = best_successor_of (current);
          if (ln == 0)
            {
              nodeStatus (current) |= TRACE_TAIL;
              break;
            }
          /*
           *  Cannot include ENTRY node.
           */
          dest = destinationNode (ln);
          if (dest == ENTRY)
            {
              nodeStatus (current) |= TRACE_TAIL;
              break;
            }
          /*
           *  Can not include a dominator.
           */
          extCurrent = (struct Ext *) nodeExt (current);
          extNext = (struct Ext *) nodeExt (dest);
          if ((extCurrent == 0) || (extNext == 0))
            {
              Punt ("corrupted extension field");
            }
          if (Set_in (extCurrent->dominator, extNext->order))
            {
              nodeStatus (current) |= TRACE_TAIL;
              break;
            }
          /*
           *  Mark the link selected.
           */
          arc = FindSrcArc (current, dest, 0);
          if (arc != 0)
            {
              arcType (arc) = nodeType (seed);
            }
          else
            Punt ("SelectTraces: corrupted source link");
          arc = FindDestArc (current, dest, 0);
          if (arc != 0)
            {
              arcType (arc) = nodeType (seed);
            }
          else
            Punt ("SelectTraces: corrupted destination link");
          /*
           *  Add it to the trace.
           */
          nodeType (dest) = nodeType (seed);
          nodeStatus (dest) |= VISITED;
          current = dest;
        }
      last = current;
      /*
       *      Grow the trace backward.
       */
      current = seed;
      for (;;)
        {
          Arc ln, arc;
          Node src;
          /*
           *  Cannot grow from the ENTRY node.
           */
          if (current == ENTRY)
            {
              nodeStatus (current) |= TRACE_HEAD;
              break;
            }
          ln = best_predecessor_of (current);
          if (ln == 0)
            {
              nodeStatus (current) |= TRACE_HEAD;
              break;
            }
          src = sourceNode (ln);
          /*
           *  Can not include a dominee.
           */
          extCurrent = (struct Ext *) nodeExt (current);
          extNext = (struct Ext *) nodeExt (src);
          if ((extCurrent == 0) || (extNext == 0))
            {
              Punt ("corrupted extension field");
            }
          if (Set_in (extNext->dominator, extCurrent->order))
            {
              nodeStatus (current) |= TRACE_HEAD;
              break;
            }
          /*
           *  Mark the link selected.
           */
          arc = FindSrcArc (src, current, 0);
          if (arc != 0)
            {
              arcType (arc) = nodeType (seed);
            }
          else
            Punt ("SelectTraces: corrupted source link");
          arc = FindDestArc (src, current, 0);
          if (arc != 0)
            {
              arcType (arc) = nodeType (seed);
            }
          else
            Punt ("SelectTraces: corrupted destination link");
          /*
           *  Add it to the trace.
           */
          nodeType (src) = nodeType (seed);
          nodeStatus (src) |= VISITED;
          current = src;
        }
    }
#endif
  /*
   *  Make sure that all nodes have been visited.
   */
  for (ptr = graph->nodes; ptr != 0; ptr = nextNode (ptr))
    {
      if (!(nodeStatus (ptr) & VISITED))
        Punt ("SelectTraces: failed to select all nodes");
    }
  /*  
   *  Detect inner loops.
   */
  for (ptr = graph->nodes; ptr != 0; ptr = nextNode (ptr))
    {
      Arc ln;
      if (!(nodeStatus (ptr) & TRACE_TAIL))
        continue;
      for (ln = destinationArcs (ptr); ln != 0; ln = nextArc (ln))
        {
          Node dest;
          dest = destinationNode (ln);
          if ((nodeStatus (dest) & TRACE_HEAD) &&
              (nodeType (dest) == nodeType (ptr)))
            {
              nodeStatus (dest) |= LOOP_HEAD;
              break;
            }
        }
    }
}
/*
 *      Trace selection by sorting all arcs and
 *      select from the most important to the least important.
 */
void
SelectBySortArc (FGraph graph)
{
#define MAX_SIZE (MAX_GRAPH_SIZE*2)
  ST_Entry sorted_list[MAX_SIZE];       /* expected fan-out = 2 */
  register int list_length;
  Node node_list[MAX_GRAPH_SIZE];
  register int node_list_length;
  register Node ptr;
  register Arc arc;
  register int i;
  int next_trace_id;
  /*
   *  Mark all nodes unvisited.
   *  Also clear all status bits.
   */
  node_list_length = 0;
  for (ptr = graph->nodes; ptr != 0; ptr = nextNode (ptr))
    {
      node_list[node_list_length++] = ptr;
      if (node_list_length >= MAX_GRAPH_SIZE)
        Punt ("SelectBySortArc: graph is too large");
      nodeType (ptr) = 0;       /* trace id = 0 */
      nodeStatus (ptr) = NOT_VISITED;
      for (arc = sourceArcs (ptr); arc != 0; arc = nextArc (arc))
        arcType (arc) = 0;
      for (arc = destinationArcs (ptr); arc != 0; arc = nextArc (arc))
        arcType (arc) = 0;
    }
  /*
   *  Place all outgoing arcs in the list.
   *  Sort all arcs by weight.
   */
  list_length = 0;
  for (ptr = graph->nodes; ptr != 0; ptr = nextNode (ptr))
    {
      for (arc = destinationArcs (ptr); arc != 0; arc = nextArc (arc))
        {
          sorted_list[list_length].weight = arcWeight (arc);
          sorted_list[list_length].ptr = arc;
          list_length += 1;
          if (list_length >= MAX_SIZE)
            Punt ("SelectBySortArc: graph is too large");
        }
    }
  max_sort (sorted_list, list_length);  /* library/sort.c */
  /*
   *  From the most important arc to the least important arc,
   *  connect basic blocks into traces.
   */
  next_trace_id = 1;
  for (i = 0; i < list_length; i++)
    {
      int trace_id;
      Node src, dest;
      arc = (Arc) sorted_list[i].ptr;
      src = sourceNode (arc);
      dest = destinationNode (arc);
      /*
       *      Check if this arc can still be selected.
       *      dest cannot be the root of the graph.
       *      case1 : src = free node, dest = free node
       *      case2 : src is a tail node, dest is a head node,
       *              except when src and dest belong to the same trace.
       */
      if (dest == graph->root)
        continue;
      if (!
          ((nodeStatus (src) == NOT_VISITED)
           || (nodeStatus (src) & TRACE_TAIL)))
        continue;
      if (!
          ((nodeStatus (dest) == NOT_VISITED)
           || (nodeStatus (dest) & TRACE_HEAD)))
        continue;
      /*
       *      If both src and dest are defined (in some trace),
       *      we do not allow this connection if src and dest
       *      belong to the same trace.
       *      This takes care of self-recursions.
       */
      if ((nodeStatus (src) & TRACE_TAIL) && (nodeStatus (dest) & TRACE_HEAD))
        if (nodeType (src) == nodeType (dest))
          continue;
      /*
       *      It is also not allow to connect, when src and dest
       *      are the same node.
       */
      if (src == dest)
        continue;
      /*
       *      After connection, src will no longer be tail, and
       *      dest will no longer be head.
       */
      nodeStatus (src) &= ~TRACE_TAIL;
      nodeStatus (dest) &= ~TRACE_HEAD;
      /*
       *      Now this arc can be selected.
       */
      if (nodeStatus (src) & VISITED)
        {
          trace_id = nodeType (src);
          arcType (arc) = trace_id;
          /*
           *  If the dest node is in another trace, need to
           *  combine the two traces.
           */
          if (nodeStatus (dest) & VISITED)
            {
              register int j;
              register int bad_id = nodeType (dest);
              /*
               * For all nodes of type (nodeType(dest)), change
               * to trace_id.
               */
              for (j = 0; j < node_list_length; j++)
                if (nodeType (node_list[j]) == bad_id)
                  nodeType (node_list[j]) = trace_id;
              /*
               * For all arcs of type (nodeType(dest)), change
               * to trace_id.
               */
              for (j = 0; j < list_length; j++)
                {
                  Arc ac;
                  ac = (Arc) sorted_list[j].ptr;
                  if (arcType (ac) == bad_id)
                    arcType (ac) = trace_id;
                }
            }
          else
            {
              /*
               * If dest is still a free node, it becomes the
               * trace tail.
               */
              nodeStatus (dest) |= (VISITED | TRACE_TAIL);
              nodeType (dest) = trace_id;
            }
        }
      else if (nodeStatus (dest) & VISITED)
        {
          trace_id = nodeType (dest);
          arcType (arc) = trace_id;
          /*
           *  Since src is a free node, src becomes a trace head.
           */
          nodeStatus (src) |= (VISITED | TRACE_HEAD);
          nodeType (src) = trace_id;
        }
      else
        {
          trace_id = next_trace_id++;
          arcType (arc) = trace_id;
          nodeStatus (src) |= (VISITED | TRACE_HEAD);
          nodeType (src) = trace_id;
          nodeStatus (dest) |= (VISITED | TRACE_TAIL);
          nodeType (dest) = trace_id;
        }
    }
  /*
   *  The remaining NOT_VISITED nodes, each forms a trace.
   */
  for (ptr = graph->nodes; ptr != 0; ptr = nextNode (ptr))
    {
      if (nodeStatus (ptr) & VISITED)
        continue;
      nodeStatus (ptr) |= (VISITED | TRACE_HEAD | TRACE_TAIL);
      nodeType (ptr) = next_trace_id++;
    }
  /*
   *  Detect inner loops.
   */
  for (ptr = graph->nodes; ptr != 0; ptr = nextNode (ptr))
    {
      if (!(nodeStatus (ptr) & TRACE_TAIL))
        continue;
      for (arc = destinationArcs (ptr); arc != 0; arc = nextArc (arc))
        {
          Node dest;
          dest = destinationNode (arc);
          if ((nodeStatus (dest) & TRACE_HEAD) &&
              (nodeType (dest) == nodeType (ptr)))
            {
              nodeStatus (dest) |= LOOP_HEAD;
              break;
            }
        }
    }
}
/*
 *      Places traces in a particular linear order
 *      to maximize sequential transition.
 *      A good way to achieve this is to construct a
 *      higher level graph, using traces as nodes.
 *      An arc is added between traces whose head
 *      and tail are connected by a transition.
 */
static void
PlaceTraces (FGraph graph)
{
  FGraph new_graph;
  Node node, current;
  Node node_order[MAX_GRAPH_SIZE];
  int i, size;
#ifndef SECOND_LEVEL_SELECT
  int min_trace_id, max_trace_id;
#endif
  if (graph->nodes == 0)
    return;
#ifdef SECOND_LEVEL_SELECT
  new_graph = NewGraph ();      /* create a high level graph */
  for (node = graph->nodes; node != 0; node = nextNode (node))
    {
      int trace_id;
      Node temp;
      trace_id = nodeType (node);
      temp = FindNode (new_graph, trace_id);
      if (temp == 0)
        {
          temp = NewNode ();
          nodeId (temp) = trace_id;
          AddNode (new_graph, temp);
        }
      if (node == graph->root)
        new_graph->root = temp;
    }
  for (node = graph->nodes; node != 0; node = nextNode (node))
    {
      Arc arc;
      if (!(nodeStatus (node) & TRACE_TAIL))
        continue;
      /*
       *      Find transitions to the head of other traces.
       *      Inner loop back-edge is not considered.
       */
      for (arc = destinationArcs (node); arc != 0; arc = nextArc (arc))
        {
          Node dest;
          dest = destinationNode (arc);
          if ((nodeType (dest) != nodeType (node)) &&
              (nodeStatus (dest) & TRACE_HEAD))
            {
              /*
               *      Add a link (trace[node]->trace[dest])
               */
              int src_trace_id, dest_trace_id;
              Node src_node, dest_node;
              Arc ar;
              src_trace_id = nodeType (node);
              dest_trace_id = nodeType (dest);
              src_node = FindNode (new_graph, src_trace_id);
              dest_node = FindNode (new_graph, dest_trace_id);
              ConnectNodes (src_node, dest_node, 0);
              ar = FindSrcArc (src_node, dest_node, 0);
              arcWeight (ar) = arcWeight (arc);
              ar = FindDestArc (src_node, dest_node, 0);
              arcWeight (ar) = arcWeight (arc);
            }
        }
    }
  /*
   *  Simply assign the node weights to max(connecting arc)
   */
  for (node = new_graph->nodes; node != 0; node = nextNode (node))
    {
      Arc arc;
      double max = 1.0;
      for (arc = sourceArcs (node); arc != 0; arc = nextArc (arc))
        if (arcWeight (arc) > max)
          max = arcWeight (arc);
      for (arc = destinationArcs (node); arc != 0; arc = nextArc (arc))
        if (arcWeight (arc) > max)
          max = arcWeight (arc);
      nodeWeight (node) = max;
    }
  /*
   *  Apply SelectTraces() on the new graph.
   *  Use SELECT_BY_ARC_WEIGHT
   */
  best_successor_of = best_successor_2;
  best_predecessor_of = best_predecessor_2;
  SelectTraces (new_graph);
  /*
   *  Determine the best sequential order of the traces.
   *  Essentially, we have the original problem again.
   *  However, after the second level trace selection,
   *  we expect most of the sequential transitions are
   *  captured. A naive heuristic is sufficient here.
   *  The sequential order must start with the ENTRY trace.
   */
#ifdef DEBUG_TRACE1
  printf ("... second level graph = \n");
  WriteGraph ("stdout", new_graph);
#endif
  /*
   *  Clear the valid bit of all nodes.
   */
  for (node = new_graph->nodes; node != 0; node = nextNode (node))
    {
      nodeStatus (node) &= ~VISITED;
    }
  /*
   *  Start from the root node.
   */
  size = 0;
  current = new_graph->root;
  while (current != 0)
    {
      Node ptr;
      Arc ar;
      int trace_id;
      if (nodeStatus (current) & VISITED)
        Punt ("PlaceTraces: reached a VISITed node");
      nodeStatus (current) |= VISITED;
      trace_id = nodeId (current);
      /*
       *      Layout the trace.
       */
      for (ptr = graph->nodes; ptr != 0; ptr = nextNode (ptr))
        {
          if ((nodeType (ptr) == trace_id) && (nodeStatus (ptr) & TRACE_HEAD))
            break;              /* find the starting node of the trace */
        }
      if (ptr == 0)
        Punt ("PlaceTraces: internal error (1)");
      while (ptr != 0)
        {
          Arc next;
          node_order[size++] = ptr;
          /*
           *  Follow the in-trace transition.
           */
          if (nodeStatus (ptr) & TRACE_TAIL)
            break;              /* reached the end of trace */
          for (next = destinationArcs (ptr); next != 0; next = nextArc (next))
            {
              if (arcType (next) == nodeType (ptr))
                break;          /* find a in-trace transition */
            }
          if (next == 0)
            break;
          ptr = destinationNode (next);
        }
      /*
       *      Select the next trace to be visited next.
       *      Follow an in-trace transition (of the higher level
       *      graph) if possible. 
       */
      for (ar = destinationArcs (current); ar != 0; ar = nextArc (ar))
        {
          if (arcType (ar) == nodeType (current))
            break;              /* find an in-trace transition */
        }
      if (ar != 0)
        {                       /* transition is still in-trace */
          current = destinationNode (ar);
        }
      else
        {                       /* must find another trace */
          /*
           *  Find the most important trace left.
           */
          Node nn, best;
          best = 0;
          for (nn = new_graph->nodes; nn != 0; nn = nextNode (nn))
            {
              if (nodeStatus (nn) & VISITED)
                continue;       /* skip over VISITED nodes */
              if (!(nodeStatus (nn) & TRACE_HEAD))
                continue;       /* skip over non-trace headers */
              if (best == 0)
                {
                  best = nn;
                }
              else
                {
                  if (nodeWeight (nn) > nodeWeight (best))
                    best = nn;
                }
            }
          current = best;       /* go out of trace if best=0 */
        }
    }
  /*
   *  Make sure that all traces have been layout.
   */
  for (node = new_graph->nodes; node != 0; node = nextNode (node))
    {
      if (!(nodeStatus (node) & VISITED))
        {
          Punt ("PlaceTraces: missing some traces");
        }
    }
  /*
   *  No longer need the higher level graph.
   */
  FreeGraph (&new_graph);       /* destroy the high level graph */
#else
  min_trace_id = 0x1FFFFFFF;
  max_trace_id = -0x1FFFFFFF;
  for (node = graph->nodes; node != 0; node = nextNode (node))
    {
      int trace_id;
      trace_id = nodeType (node);
      if (trace_id > max_trace_id)
        max_trace_id = trace_id;
      if (trace_id < min_trace_id)
        min_trace_id = trace_id;
    }
  for (node = graph->nodes; node != 0; node = nextNode (node))
    {
      nodeStatus (node) &= ~VISITED;
    }
  size = 0;
  for (i = min_trace_id; i <= max_trace_id; i++)
    {
      Node ptr;
      /*
       * 1. find the trace header.
       */
      for (ptr = graph->nodes; ptr != 0; ptr = nextNode (ptr))
        {
          if ((nodeType (ptr) == i) & ((nodeStatus (ptr) & TRACE_HEAD) != 0))
            break;
        }
      if (ptr == 0)
        continue;
      while (ptr != 0)
        {
          Arc next;
          if (nodeStatus (ptr) & VISITED)
            Punt ("PlaceTraces: visited a node twice");
          nodeStatus (ptr) |= VISITED;
          node_order[size++] = ptr;
          if (nodeStatus (ptr) & TRACE_TAIL)
            break;
          for (next = destinationArcs (ptr); next != 0; next = nextArc (next))
            {
              if (arcType (next) == nodeType (ptr))
                break;
            }
          if (next == 0)
            break;
          ptr = destinationNode (next);
        }
    }
  /*
   *  Make sure that all traces have been layout.
   */
  for (node = graph->nodes; node != 0; node = nextNode (node))
    {
      if (!(nodeStatus (node) & VISITED))
        {
          fprintf (stderr, "min trace id = %d\n", min_trace_id);
          fprintf (stderr, "max trace id = %d\n", max_trace_id);
          fprintf (stderr, "size = %d\n", size);
          WriteGraph ("stderr", graph);
          Punt ("PlaceTraces: missing some traces");
        }
    }
#endif
  /*
   *  Rearrange the order of nodes, according to the
   *  node_order[] order.
   */
  node_order[size] = 0;
  for (i = 0; i < size; i++)
    {
      nextNode (node_order[i]) = node_order[i + 1];
    }
  graph->nodes = node_order[0];
}
/*------------------------------------------------------------------*/
/*
 *      Apple trace selection on a given graph.
 */
void
TraceSelection (FGraph graph, int method, double min_prob)
{
  if (graph == 0)
    Punt ("TraceSelection: nil graph");
  if ((min_prob < 0.0) || (min_prob > 1.0))
    Punt ("TraceSelection: min_prob must be within [0..1]");
#ifdef DEBUG
  printf ("### BEFORE\n");
  WriteGraph ("stdout", graph);
#endif
  /*
   *  The old selection program.
   */
  switch (method)
    {
    case SELECT_BY_NODE_WEIGHT:
      best_successor_of = best_successor_1;
      best_predecessor_of = best_predecessor_1;
      min_prob_requirement = 0;
      SelectTraces (graph);
      PlaceTraces (graph);
      break;
    case SELECT_BY_ARC_WEIGHT:
      best_successor_of = best_successor_2;
      best_predecessor_of = best_predecessor_2;
      min_prob_requirement = 0;
      SelectTraces (graph);
      PlaceTraces (graph);
      break;
    case SELECT_BY_MIN_PROB:
      best_successor_of = best_successor_3;
      best_predecessor_of = best_predecessor_3;
      min_prob_requirement = min_prob;
      SelectTraces (graph);
      PlaceTraces (graph);
      break;
    case SELECT_BY_MAX_ARC:
      SelectBySortArc (graph);
      PlaceTraces (graph);
      break;
    default:
      Punt ("TraceSelection: illegal selection method");
    }
#ifdef DEBUG
  printf ("### AFTER\n");
  WriteGraph ("stdout", graph);
#endif
}
/*
 *      Measure the trace selection result.
 *      Trace selection must have been applied.
 */
void
ReportSelectionResult (FGraph graph, double *matrix)
{
  int i;
  Node node;
  Arc arc;
  if (graph == 0)
    Punt ("ReportSelectionResult: nil graph");
  if (matrix == 0)
    return;
  for (i = 0; i <= N_LEAF; i++)
    matrix[i] = 0.0;
  /*
   *  Compute the sequential locality.
   */
  for (node = graph->nodes; node != 0; node = nextNode (node))
    {
      Arc ptr;
      Node next_node;
      next_node = nextNode (node);
      if (next_node == 0)       /* ignore the last node */
        break;
      for (ptr = destinationArcs (node); ptr != 0; ptr = nextArc (ptr))
        {
          if (destinationNode (ptr) == next_node)
            {
              matrix[W_SEQUENTIAL] += arcWeight (ptr);
            }
        }
    }
  /*
   *  Measure various transition counts.
   */
  for (node = graph->nodes; node != 0; node = nextNode (node))
    {
      for (arc = destinationArcs (node); arc != 0; arc = nextArc (arc))
        {
          Node src, dest;
          int Sh, St, Dh, Dt;
          src = sourceNode (arc);
          dest = destinationNode (arc);
          Sh = nodeStatus (src) & TRACE_HEAD;
          St = nodeStatus (src) & TRACE_TAIL;
          Dh = nodeStatus (dest) & TRACE_HEAD;
          Dt = nodeStatus (dest) & TRACE_TAIL;
          if ((arcType (arc) != 0) && (arcType (arc) == nodeType (node)))
            {
              /*
               *      In-trace.
               */
              matrix[W_E] += arcWeight (arc);
            }
          else if (arcType (arc) != 0)
            {
              Punt ("ReportSelectionResult: illegal arc type");
            }
          else
            {
              if (St && Dh)
                {               /* terminal to head */
                  matrix[W_A] += arcWeight (arc);
                  /* detect loop back-edge */
                  if (nodeType (src) == nodeType (dest))
                    {
                      matrix[W_BACK_EDGE] += arcWeight (arc);
                    }
                }
              else if (St && !Dh)
                {               /* terminal to middle */
                  matrix[W_B] += arcWeight (arc);
                }
              else if (!St && Dh)
                {               /* middle to head */
                  matrix[W_C] += arcWeight (arc);
                }
              else
                {               /* middle to middle */
                  matrix[W_D] += arcWeight (arc);
                }
            }
        }
    }
#ifdef DEBUG_TRACE2
  if (matrix[W_E] > matrix[W_SEQUENTIAL])
    {
      WriteGraph ("zzz.debug", graph);
      fprintf (stderr, "in-trace = %f\n", matrix[W_E]);
      fprintf (stderr, "sequential = %f\n", matrix[W_SEQUENTIAL]);
      Punt ("ReportSelection: incorrect in-trace");
    }
#endif
  /*
   *  Measure several graph parameters.
   */
  for (node = graph->nodes; node != 0; node = nextNode (node))
    {
      nodeStatus (node) &= ~VISITED;    /* borrow the valid bit */
      matrix[N_NODES] += 1.0;
      matrix[W_NODES] += nodeWeight (node);
      if (destinationArcs (node) == 0)
        {
          matrix[N_LEAF] += 1.0;
        }
      if (sourceArcs (node) == 0)
        {
          matrix[N_ROOT] += 1.0;
        }
      for (arc = destinationArcs (node); arc != 0; arc = nextArc (arc))
        {
          matrix[N_ARCS] += 1.0;
          matrix[W_ARCS] += arcWeight (arc);
        }
    }
  for (node = graph->nodes; node != 0; node = nextNode (node))
    {
      Node ptr;
      int trace_id, loop, size;
      if (nodeStatus (node) & VISITED)
        continue;
      trace_id = nodeType (node);
      loop = 0;
      size = 0;
      for (ptr = graph->nodes; ptr != 0; ptr = nextNode (ptr))
        {
          if (nodeType (ptr) == trace_id)
            {
              nodeStatus (ptr) |= VISITED;
              size++;
              loop |= (nodeStatus (node) & LOOP_HEAD);
            }
        }
      matrix[N_TRACE] += 1.0;
      matrix[L_TRACE] += size;
      if (loop)
        {
          matrix[N_LOOP] += 1.0;
          matrix[L_LOOP] += size;
        }
    }
  if (matrix[L_TRACE] != matrix[N_NODES])
    Punt ("ReportSelectionResult: incorrect accounting");
  if (matrix[N_TRACE] != 0.0)
    matrix[L_TRACE] /= matrix[N_TRACE];
  if (matrix[N_LOOP] != 0.0)
    matrix[L_LOOP] /= matrix[N_LOOP];
}
/*------------------------------------------------------------------*/

#ifdef DEBUG_TRACE
#include <library/io.h>
main (argc, argv)
     int argc;
     char **argv;
{
  FGraph g;
  double stat[17];
  int i, n;
  if (argc < 2)
    Punt ("trace N");
  n = atoi (argv[1]);
  /* get a graph from input */
  g = ReadGraph ("stdin");
  printf ("> Original graph\n");
  WriteGraph ("stdout", g);
  fflush (stdout);
  /* apply trace selection */
  switch (n)
    {
    case 0:
      printf ("> Selection by arc weight (min_prob=.80)\n");
      TraceSelection (g, SELECT_BY_MIN_PROB, 0.80);
      ReportSelectionResult (g, stat);
      WriteGraph ("stdout", g);
      for (i = 0; i <= 16; i++)
        {
          printf ("  %f\n", stat[i]);
        }
      fflush (stdout);
      break;
    case 1:
      printf ("> Selection by arc weight (min_prob=.70)\n");
      TraceSelection (g, SELECT_BY_MIN_PROB, 0.70);
      ReportSelectionResult (g, stat);
      WriteGraph ("stdout", g);
      for (i = 0; i <= 16; i++)
        {
          printf ("  %f\n", stat[i]);
        }
      fflush (stdout);
      break;
    case 2:
      printf ("> Selection by arc weight\n");
      TraceSelection (g, SELECT_BY_ARC_WEIGHT, 0.0);
      ReportSelectionResult (g, stat);
      WriteGraph ("stdout", g);
      for (i = 0; i <= 16; i++)
        {
          printf ("  %f\n", stat[i]);
        }
      fflush (stdout);
      break;
    case 3:
      printf ("> Selection by sort all arcs\n");
      TraceSelection (g, SELECT_BY_MAX_ARC, 0.70);
      ReportSelectionResult (g, stat);
      WriteGraph ("stdout", g);
      for (i = 0; i <= 16; i++)
        {
          printf ("  %f\n", stat[i]);
        }
      fflush (stdout);
      break;
    case 4:
      printf ("> Selection by node weight\n");
      TraceSelection (g, SELECT_BY_NODE_WEIGHT, 0.0);
      ReportSelectionResult (g, stat);
      WriteGraph ("stdout", g);
      for (i = 0; i <= 16; i++)
        {
          printf ("  %f\n", stat[i]);
        }
      fflush (stdout);
      break;
    default:
      break;
    }
}
#endif
