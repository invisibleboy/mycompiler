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
/*===========================================================================
 *
 *      File :          bdd_interface.c
 *      Description :   Functions to interface to the bdd.
 *      Creation Date : May 1998
 *      Author :        David August
 *
 *      Copyright (c) 1998 David August, John Sias, Wen-mei Hwu and 
 *                         The Board of Trustees of the University of Illinois.
 *                         All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
 *===========================================================================*/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <stdlib.h>
#include <library/i_error.h>
#include <library/l_alloc_new.h>
#include <library/i_global.h>
#include <library/i_list.h>
#include <bdd/cudd.h>
#include <library/i_bdd_interface.h>

#include <bdd/cuddInt.h>


unsigned int
I_BDD_num_terms (DdManager * dd)
{
  return dd->size;
}

static void
I_BDD_minterms_aux (DdManager * dd, DdNode * node, char *list,
                    List * minterm_list)
{
  DdNode *N, *Nv, *Nnv;
  int i, index;
  char *new_list;

  N = Cudd_Regular (node);

  if (cuddIsConstant (N))
    {
      /* Terminal case: Print one cube based on the current recursion
         ** path, unless we have reached the background value (ADDs) or
         ** the logical zero (BDDs).
       */
      if (node != dd->background && node != Cudd_Not (dd->one))
        {
          if (!(new_list = (char *) malloc (sizeof (char) * dd->size)))
            I_punt ("I_BDD_minterms_aux: cannot allocate array");

          for (i = 0; i < dd->size; i++)
	    new_list[i] = list[i];

          if (cuddV (node) != 1)
            I_punt ("I_BDD_minterms_aux: bad BDD");

          *minterm_list = List_insert_last (*minterm_list, new_list);
        }
    }
  else
    {
      Nv = cuddT (N);
      Nnv = cuddE (N);
      if (Cudd_IsComplement (node))
        {
          Nv = Cudd_Not (Nv);
          Nnv = Cudd_Not (Nnv);
        }
      index = N->index;
      list[index] = 0;
      I_BDD_minterms_aux (dd, Nnv, list, minterm_list);
      list[index] = 1;
      I_BDD_minterms_aux (dd, Nv, list, minterm_list);
      list[index] = 2;
    }

  return;
}


List
I_BDD_minterms (DdManager * dd, DdNode * node)
{
  int i;
  List minterm_list;
  char *list;

  list = (char *) malloc (sizeof (char) * dd->size);

  if (list == NULL)
    I_punt ("I_BDD_minterms_aux: cannot allocate array");

  for (i = 0; i < dd->size; i++)
    list[i] = 2;

  minterm_list = NULL;
  I_BDD_minterms_aux (dd, node, list, &minterm_list);

  free (list);

  return (minterm_list);
}

void
I_BDD_print_minterm (char *m, int n)
{
  unsigned int i;

  for (i = 0; i < n; i++)
    if (m[i] > 1)
      fprintf (stderr, "-");
    else
      fprintf (stderr, "%d", m[i]);
}

void
I_BDD_print_minterms (List mins, int terms)
{
  char *minterm;
  int indx;

  List_start (mins);
  while ((minterm = (char *) List_next (mins)))
    {
      for (indx = 0; indx < terms; indx++)
        {
          if (minterm[indx] == 2)
            printf ("-");
          else
            printf ("%d", (int) minterm[indx]);
        }
      printf ("\n");
    }
}

#undef I_BDD_MUTEX_TEST
#undef I_BDD_MUTEX_DEBUG

/*
 * void
 * I_BDD_new_mutex_domain (DdManager *dd, DdNode *node[], unsigned int size)
 * ----------------------------------------------------------------------
 * JWS 20010406
 * Create a new finite domain of size "size", filling the (preallocated)
 * buffer "node" with pointers to the included nodes.  Each node has a 
 * reference count of one on return.
 *
 * A finite domain of size N is a set of N mutually exclusive, collectively
 * exhaustive nodes.
 */

void
I_BDD_new_mutex_domain (DdManager *dd, DdNode *node[], unsigned int size)
{
  unsigned int shbuf, vars, ctr;
  DdNode **ddVar, *one, *newnode, *oldnode, *bitnode;
  int i,j, extra;

  /* base cases */

  if (size == 0)
    {
      return;
    }

  one = Cudd_ReadOne(dd);

  if (size == 1)
    {
      node[0] = one;
      Cudd_Ref (node[0]);
      return;
    }

  for(shbuf = size - 1, vars = 0; shbuf != 0; shbuf >>= 1, vars++);

  extra = (1 << vars) - size;

  ddVar = malloc(vars * sizeof(DdNode *));

  for (j = 0; j < vars; j++)
    {
      ddVar[j] = bitnode = Cudd_bddNewVar (dd);
      Cudd_Ref (bitnode);
    }

  ctr = 0;

  for (i = 0; i < size; i++, ctr++)
    {
      newnode = one;
      Cudd_Ref (newnode);

#ifdef I_BDD_MUTEX_DEBUG
      printf("i%d:", i);
#endif

      for (j = (extra > 0); j < vars; j++)
	{
	  bitnode = (ctr & (1 << j)) ? ddVar[j] : Cudd_Not(ddVar[j]);

	  newnode = Cudd_bddAnd (dd, bitnode, oldnode = newnode);
	  Cudd_Ref (newnode);
	  Cudd_RecursiveDeref (dd, oldnode);

#ifdef I_BDD_MUTEX_DEBUG
	  if (ctr & (1 << j))
	    printf("+ v%d ", j);
	  else
	    printf("+ !v%d ", j);
#endif
	}

#ifdef I_BDD_MUTEX_DEBUG
      printf ("\n");
#endif

      node[i] = newnode;

      if (extra > 0)
	{
	  extra--;
	  ctr++;
	}
    }

#ifdef I_BDD_MUTEX_TEST
  {
    DdNode *test, *old_test;
    int i;
    test = Cudd_ReadLogicZero(dd);
    Cudd_Ref (test);
    for (i = 0; i < size; i++)
      {
	if (Cudd_bddIte (dd, test, node[i], zero) != zero)
	  I_punt("I_BDD_new_mutex_complex: not mutex!");

	test = Cudd_bddIte(dd, node[i], one, old_test = test);
	Cudd_Ref (test);
	Cudd_RecursiveDeref (dd, old_test);
      }
    if (test != one)
      I_punt("I_BDD_new_mutex_complex: not exhaustive!");

    Cudd_RecursiveDeref (dd, test);
    }
#endif

  for (j = 0; j < vars; j++)
    Cudd_RecursiveDeref (dd, ddVar[j]);

  free (ddVar);
  return;
}
