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
/*===========================================================================*\
 *      File:   heap.h
 *      Author: Richard Hank, Wen-mei Hwu
 *      Modified By: XXX, date, time, why
 *
 * Revision 1.1  94/03/16  20:50:34  20:50:34  hank (Richard E. Hank)
 * Initial revision
 *
 *
 *===========================================================================*/

#ifndef HEAP_H
#define HEAP_H

/* 10/29/02 REK Adding config.h */
#include <config.h>

typedef struct h_node
{
  double weight;
  void *element;
  struct h_node *p;
  struct h_node *left;
  struct h_node *right;
}
HeapNode;

#define HEAP_MAX	1
#define HEAP_MIN	2

typedef struct Heap
{
  HeapNode *heap;
  int size;
  int dir;
}
Heap;

#ifdef __cplusplus
extern "C"
{
#endif

#if (defined(__STDC__) || defined(__cplusplus))

  extern Heap *Heap_Create (int dir);
  extern Heap *Heap_Dispose (Heap * root, void (*element_dispose) ());
  extern void Heap_Insert (Heap * root, void *element, double weight);
  extern void *Heap_Top (Heap * root);
  extern double *Heap_TopWeight (Heap * root);
  extern void *Heap_ExtractTop (Heap * root);

#else

  extern Heap *Heap_Create ( /* int dir */ );
  extern Heap *Heap_Dispose ( /* Heap *root, void (*element_dispose)() */ );
  extern void Heap_Insert ( /* Heap *root, void *element, double weight */ );
  extern void *Heap_Top ( /* Heap *root */ );
  extern double *Heap_TopWeight ( /* Heap *root */ );
  extern void *Heap_ExtractTop ( /* Heap *root */ );

#endif

#ifdef __cplusplus
}
#endif

#endif
