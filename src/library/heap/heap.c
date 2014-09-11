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
 *      File:   heap.c
 *      Author: Richard Hank, Wen-mei Hwu
 *      Modified By: XXX, date, time, why
 *
 * Revision 1.1  94/03/16  20:47:51  20:47:51  hank (Richard E. Hank)
 * Initial revision
 *
 *
 *===========================================================================*/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <library/l_alloc_new.h>
#include <library/heap.h>

static L_Alloc_Pool *heapNodePool = NULL;

#if 0
static void
Heap_NodeSwap (HeapNode * a, HeapNode * b)
{
  HeapNode node;

  node.weight = a->weight;
  node.element = a->element;

  a->weight = b->weight;
  a->element = b->element;

  b->weight = node.weight;
  b->element = node.element;
}

#else

#define Heap_NodeSwap(a,b)      {  HeapNode node; \
                                   node.weight = (a)->weight; \
                                   node.element = (a)->element; \
                                   (a)->weight = (b)->weight; \
                                   (a)->element = (b)->element; \
                                   (b)->weight = node.weight; \
                                   (b)->element = node.element; \
                                }

#endif

static void
HeapifyMax (Heap * root)
{
  HeapNode *left, *right, *current, *largest;

  current = root->heap;

  while (1)
    {
      left = current->left;
      right = current->right;

      if ((left != NULL) && (left->weight > current->weight))
	largest = left;
      else
	largest = current;

      if ((right != NULL) && (right->weight > largest->weight))
	largest = right;

      if (largest != current)
	{
	  Heap_NodeSwap (current, largest);
	  current = largest;
	}
      else
	return;
    }
}

static void
HeapifyMin (Heap * root)
{
  HeapNode *left, *right, *current, *smallest;

  current = root->heap;

  while (1)
    {
      left = current->left;
      right = current->right;

      if ((left != NULL) && (left->weight < current->weight))
	smallest = left;
      else
	smallest = current;

      if ((right != NULL) && (right->weight < smallest->weight))
	smallest = right;

      if (smallest != current)
	{
	  Heap_NodeSwap (current, smallest);
	  current = smallest;
	}
      else
	return;
    }

}

/*
 * Create a new Heap 
 */
Heap *
Heap_Create (int dir)
{
  Heap *new_heap;

  if (heapNodePool == NULL)
    heapNodePool = L_create_alloc_pool ("HeapNode", sizeof (HeapNode), 512);

  new_heap = (Heap *) malloc (sizeof (Heap));
  new_heap->heap = NULL;
  new_heap->size = 0;
  new_heap->dir = dir;

  return (new_heap);
}

/*
 * Free up an existing heap, if passed a function pointer
 * (i.e. if element_dispose != NULL) this function will 
 * also free up the space allocated to each element
 */
Heap *
Heap_Dispose (Heap * root, void (*element_dispose) (void *))
{
  HeapNode *node, *parent;

  if (root == NULL)
    return (NULL);

  node = root->heap;
  while (node != NULL)
    {
      if (node->left != NULL)
	{
	  node = node->left;
	}
      else if (node->right != NULL)
	{
	  node = node->right;
	}
      else
	{
	  parent = node->p;
	  if ((element_dispose != NULL) && (node->element != NULL))
	    {
	      (*element_dispose) (node->element);
	    }
	  if (parent != NULL)
	    {
	      if (parent->left == node)
		parent->left = NULL;
	      else
		parent->right = NULL;
	    }
	  L_free (heapNodePool, node);
	  node = parent;
	}
    }
  free (root);
  return (NULL);
}

/* 
 * Insert the <element> with key <weight> into the heap 
 */
void
Heap_Insert (Heap * root, void *element, double weight)
{
  HeapNode *ins_node;
  int ins_pt = 0;
  int size;

  ins_node = root->heap;

  size = ++root->size;
  while (size >= (1 << ins_pt))
    ++ins_pt;
  --ins_pt;

  while (ins_pt > 1)
    {
      if (size & (1 << --ins_pt))
	ins_node = ins_node->right;
      else
	ins_node = ins_node->left;
    }

  if (size == 1)
    {
      ins_node = root->heap = (HeapNode *) L_alloc (heapNodePool);
      ins_node->p = NULL;
    }
  else if (size & 1)
    {
      ins_node->right = (HeapNode *) L_alloc (heapNodePool);
      ins_node->right->p = ins_node;
      ins_node = ins_node->right;
    }
  else
    {
      ins_node->left = (HeapNode *) L_alloc (heapNodePool);
      ins_node->left->p = ins_node;
      ins_node = ins_node->left;
    }
  ins_node->element = element;
  ins_node->weight = weight;
  ins_node->right = ins_node->left = NULL;

  if (root->dir == HEAP_MAX)
    {
      while (ins_node != NULL && ins_node->p != NULL &&
	     ins_node->p->weight < weight)
	{
	  Heap_NodeSwap (ins_node, ins_node->p);
	  ins_node = ins_node->p;
	}
    }
  else
    {
      while (ins_node != NULL && ins_node->p != NULL &&
	     ins_node->p->weight > weight)
	{
	  Heap_NodeSwap (ins_node, ins_node->p);
	  ins_node = ins_node->p;
	}
    }
}

/*
 * Retrieve the element at the top of the heap
 */
void *
Heap_Top (Heap * root)
{
  if (root->size >= 1)
    return (root->heap->element);

  return (NULL);
}

/*
 * Retrieve the weight of element at the top of the heap
 */
double *
Heap_TopWeight (Heap * root)
{
  if (root->size >= 1)
    return (&(root->heap->weight));

  return (NULL);
}

/*
 * Extract the element at the top of the heap 
 */
void *
Heap_ExtractTop (Heap * root)
{
  void *heap_max;
  HeapNode *rm_node;
  int rm_pt = 0;
  int size = root->size;

  if (size < 1)
    return (NULL);
  heap_max = root->heap->element;

  rm_node = root->heap;
  while (size >= (1 << rm_pt))
    ++rm_pt;
  --rm_pt;

  while (rm_pt > 1)
    {
      if (size & (1 << --rm_pt))
	rm_node = rm_node->right;
      else
	rm_node = rm_node->left;
    }
  if (size == 1)
    {
      root->size = 0;
      L_free (heapNodePool, root->heap);
      root->heap = NULL;
      return (heap_max);
    }
  else if (size & 1)
    {
      rm_node = rm_node->right;
      rm_node->p->right = NULL;
    }
  else
    {
      rm_node = rm_node->left;
      rm_node->p->left = NULL;
    }
  root->heap->weight = rm_node->weight;
  root->heap->element = rm_node->element;
  --root->size;
  L_free (heapNodePool, rm_node);

  if (root->dir == HEAP_MAX)
    HeapifyMax (root);
  else
    HeapifyMin (root);

  return (heap_max);
}
