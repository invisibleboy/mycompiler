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
 *      File:   set.c
 *      Author: Po-hua Chang
 *      Creation Date:  June 1990
 *      Modified By: XXX, date, time, why
 *              Richard Hank 5/18/92:
 *                  Added "Set_empty" function, to avoid using Set_size
 *                  to determine whether a set is empty.
 *              Richard Hank 1/93:
 *                  Totally reworked the set library for speed purposes
 *              David August 6/97:
 *                  Reworked memory allocation and deallocation
 *      Copyright (c) 1993 Po-hua Chang, Richard Hank, David August, and
 *      Wen-mei Hwu.
 *      All rights reserved.  
 *      All rights granted to the University of Illinois.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/
/* 10/03/02 REK Optimizing Set_size. */
/* 10/04/02 REK Optimizing Set_union_acc. */

#undef DEBUG_SET_ADD
#undef DEBUG_SET_DELETE
#undef DEBUG_SET_UNION
#undef DEBUG_SET_INTERSECT
#undef DEBUG_SET_SUBTRACT
#undef DEBUG_SET_SAME

/*===========================================================================
 *      Description :   Bit Vector Set Operations.
 *==========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <library/set.h>
#include <library/l_alloc_new.h>
#include <library/i_error.h>

#define ABS(x)          (x<0 ? -x:x)
#define MAX(x,y)        ((x>y)?x:y)
#define SIGN(x)         ((x>=0)?0:1)

/* 10/03/02 REK A macro to inline the popcnt opcode. */
#ifdef IA64LIN_SOURCE
#ifdef __INTEL_COMPILER
#include <ia64intrin.h>
#define POPCNT(c, x)    (c)=_m64_popcnt((unsigned long)(x))
#elif defined(__GNUC__)
#define POPCNT(c, x)    asm ("popcnt %0=%1" : "=r"((c)) : "r" ((x)))
#else
#undef POPCNT
#endif
#else
#undef POPCNT
#endif

/*-----------------------------------------------------------------------*/
int total = 0;
int n_varray = 0;
int n_vector = 0;
int n_set = 0;

static long mallocated = 0;
static long maxmallocated = 0;

Set OUT = NULL;

int SetId = 0;

L_Alloc_Pool *SET_VArray_pool = NULL;
L_Alloc_Pool *SET_Vector_pool = NULL;
L_Alloc_Pool *SET_Set_pool = NULL;

L_Alloc_Pool *SET_SetIterator_pool = NULL;


static Set_Vector *
NewVArray (int size)
{
  int indx;
  Set_Vector *new_vector;

  if (size == Set_VARRAY_SIZE)
    {
      if (SET_VArray_pool == NULL)
        SET_VArray_pool = L_create_alloc_pool ("SET_VArray",
                                               sizeof (Set_Vector) *
                                               Set_VARRAY_SIZE, 512);

      new_vector = (Set_Vector *) L_alloc (SET_VArray_pool);
    }
  else
    {
      new_vector = (Set_Vector *) malloc (sizeof (Set_Vector) * size);
      if (new_vector == NULL)
        I_punt ("out of memory space (Set_Vector *)");
      mallocated += sizeof (Set_Vector) * size;
      if (mallocated > maxmallocated)
	maxmallocated = mallocated;
    }

  for (indx = 0; indx < size; indx++)
    new_vector[indx] = NULL;

  return (new_vector);
}
static void
FreeVArray (int size, Set_Vector * varray)
{
  if (varray == NULL)
    return;

  if (size == Set_VARRAY_SIZE)
    {
      L_free (SET_VArray_pool, varray);
    }
  else
    {
      free (varray);
      mallocated -= sizeof (Set_Vector) * size;
    }
}

/* 10/04/02 REK Optimizing this function. */
static void
EnlargeVArray (int size, int *base, Set_Vector ** varray)
{
  int new_size;
  Set_Vector *new_vector;

  /* calcuate size of new vector array */
  new_size = ((size >> 3) + 1) << 3;

  new_vector = NewVArray (new_size);
  memcpy (new_vector, *varray, *base * sizeof (Set_Vector));

  /* free old vector array */
  FreeVArray (*base, *varray);

  *base = new_size;
  *varray = new_vector;
}

static Set_Vector
NewVector (void)
{
  Set_Vector new_vector;

  if (SET_Vector_pool == NULL)
    SET_Vector_pool = L_create_alloc_pool ("SET_Vector",
                                           sizeof (_32bit) * Set_VECTOR_SIZE,
                                           128);

  new_vector = (Set_Vector) L_alloc (SET_Vector_pool);

  new_vector[0] = 0;
  new_vector[1] = 0;
  new_vector[2] = 0;
  new_vector[3] = 0;
  new_vector[4] = 0;
  new_vector[5] = 0;
  new_vector[6] = 0;
  new_vector[7] = 0;

  return (new_vector);
}

static void
FreeVector (Set_Vector vector)
{
  if (vector == NULL)
    return;

  L_free (SET_Vector_pool, vector);
}

static Set
NewSet (void)
{
  Set new_set;

  if (SET_Set_pool == NULL)
    SET_Set_pool = L_create_alloc_pool ("SET_Set", sizeof (_Set), 128);

  new_set = (Set) L_alloc (SET_Set_pool);

  new_set->max_base[0] = Set_VARRAY_SIZE;
  new_set->varray[0] = NewVArray (Set_VARRAY_SIZE);
  new_set->max_base[1] = 0;
  new_set->varray[1] = NULL;
  new_set->dummy = NULL;
  new_set->id = SetId++;

  return (new_set);
}

Set Set_new (void)
{
  return NewSet ();
}

static void
FreeSet (Set set)
{
  int i, b;

  if (!set)
    return;

  if (Set_in (OUT, set->id))
    exit (-1);

  OUT = Set_add (OUT, set->id);

  for (b = 0; b < 2; b++)
    {
      Set_Vector *varray;
      if (!(varray = set->varray[b]))
        continue;
      for (i = 0; i < set->max_base[b]; i++)
        {
	  Set_Vector vector;
          if (!(vector = set->varray[b][i]))
	    continue;
	  FreeVector (vector);
        }
      FreeVArray (set->max_base[b], set->varray[b]);
      set->varray[b] = NULL;
    }

  L_free (SET_Set_pool, set);
}





/*-----------------------------------------------------------------------*/

static int
ReadBit (Set_Vector vector, int element)
{
  int index, select, bit_position;
  _32bit value, mask;

  index = ABS (element);
  index = Set_INDEX (index);
  select = Set_WORD_SELECT (index);
  bit_position = Set_BIT_SELECT (index);
  value = vector[select];
  mask = 1 << bit_position;     /* generate bit mask */
  return ((value & mask) != 0);
}

static int
SetBit (Set_Vector vector, int element)
{
  int index, select, bit_position;
  _32bit mask;

  index = ABS (element);
  index = Set_INDEX (index);
  select = Set_WORD_SELECT (index);
  bit_position = Set_BIT_SELECT (index);
  mask = (1 << bit_position);
  return (vector[select] |= mask);
}

static int
ClearBit (Set_Vector vector, int element)
{
  int index, select, bit_position;
  _32bit mask;

  index = ABS (element);
  index = Set_INDEX (index);
  select = Set_WORD_SELECT (index);
  bit_position = Set_BIT_SELECT (index);
  mask = ~(1 << bit_position);
  return (vector[select] &= mask);
}

/*-----------------------------------------------------------------------*/
int
Set_in (Set set, int element)
{
  int sign, base, max_base;
  Set_Vector *vectors;

  if (set == 0)
    return 0;

  sign = SIGN (element);

  max_base = set->max_base[sign];
  vectors = set->varray[sign];

  base = ABS (element);
  base = Set_BASE (base);
  if (base >= max_base || !vectors[base])
    return (0);

  return ReadBit (vectors[base], element);
}

/* 10/03/02 REK Optimizing this function a little. */
int
Set_size (Set set)
{
  int i, b, b1, b2, b3, b4, b5, b6, b7, b8;
  Set_Vector *varray;

  if (set == 0)                 /* empty set */
    return 0;

  b1 = b2 = b3 = b4 = b5 = b6 = b7 = b8 = 0;
  for (b = 0; b < 2; b++)
    {
      varray = set->varray[b];
      for (i = 0; i < set->max_base[b]; i++)
        {
          Set_Vector vc;
#ifdef POPCNT
	  int t1, t2, t3, t4, t5, t6, t7, t8;
#else
	  int j;
          _32bit mask;
#endif

          if (!(vc = varray[i]))
            continue;

#ifdef POPCNT
  	  POPCNT (t1, vc[0]);
  	  POPCNT (t2, vc[1]);
  	  POPCNT (t3, vc[2]);
  	  POPCNT (t4, vc[3]);
  	  POPCNT (t5, vc[4]);
  	  POPCNT (t6, vc[5]);
  	  POPCNT (t7, vc[6]);
  	  POPCNT (t8, vc[7]);
	  
  	  b1 += t1;
  	  b2 += t2;
  	  b3 += t3;
  	  b4 += t4;
  	  b5 += t5;
  	  b6 += t6;
  	  b7 += t7;
  	  b8 += t8;
#else
          mask = 1;

          for (j = 0; j < 32; j++)
            {                   /* for each bit */
              b1 += ((vc[0] & mask) != 0);
              b2 += ((vc[1] & mask) != 0);
              b3 += ((vc[2] & mask) != 0);
              b4 += ((vc[3] & mask) != 0);
              b5 += ((vc[4] & mask) != 0);
              b6 += ((vc[5] & mask) != 0);
              b7 += ((vc[6] & mask) != 0);
              b8 += ((vc[7] & mask) != 0);
              mask = mask << 1;
            }
#endif
        }
    }

  return (((b1 + b2) + (b3 + b4)) + ((b5 + b6) + (b7 + b8)));
}

int
Set_empty (Set set)
{
  int b, i, j;

  if (set == 0)
    return (1);

  for (b = 0; b < 2; b++)
    {
      Set_Vector *varray = set->varray[b];
      if (!varray)
        continue;
      for (i = 0; i < set->max_base[b]; i++)
        {
          Set_Vector vc = varray[i];
          if (!vc)
            continue;
          for (j = 0; j < Set_VECTOR_SIZE; j++)
            if (vc[j])
              return (0);
        }
    }
  return (1);
}

/*--------------------------------------------------------------------------*/

Set Set_add (Set set, int element)
{
  Set_Vector *varray;
  int sign, base;

  if (set == 0)                 /* create a set descriptor if necessary */
    set = NewSet ();

#ifdef DEBUG_SET_ADD
  fprintf (stdout, "Adding %d\n", element);
  Set_print (stdout, "Set_add: before", set);
#endif
  sign = SIGN (element);
  base = ABS (element);
  base = Set_BASE (base);

  if (base >= set->max_base[sign])
    EnlargeVArray (base, &set->max_base[sign], &set->varray[sign]);

  varray = set->varray[sign];
  if (varray[base] == NULL)
    {                           /* need to create a Set_Vector */
      varray[base] = NewVector ();      /* used to be this (sign, base) */
    }

  SetBit (varray[base], element);

#ifdef DEBUG_SET_ADD
  Set_print (stdout, "Set_add: after", set);
  fprintf (stdout, "\n");
#endif

  return (set);
}

Set Set_delete (Set set, int element)
{
  Set_Vector v;
  int sign, base;

  if (set == 0)
    return (0);                 /* empty set */

#ifdef DEBUG_SET_DELETE
  fprintf (stdout, "Deleting %d\n", element);
  Set_print (stdout, "Set_delete: before", set);
#endif

  sign = SIGN (element);
  base = ABS (element);
  base = Set_BASE (base);

  if (base >= set->max_base[sign])      /* element not in set */
    return (set);

  if (!(v = set->varray[sign][base]))   /* element not in set */
    return (set);

  ClearBit (v, element);

#ifdef DEBUG_SET_DELETE
  Set_print (stdout, "Set_delete: after", set);
  fprintf (stdout, "\n");
#endif

  return (set);
}

Set Set_copy (Set set1)
{
  int i, b;
  Set new_set = NULL;
  Set_Vector *fvarray, *tvarray;
  Set_Vector fv, tv;
  _32bit t0, t1, t2, t3, t4, t5, t6, t7;

  if (set1)
    {
      /* copy set1 to new */
      new_set = NewSet ();
      for (b = 0; b < 2; b++)
        {
          fvarray = set1->varray[b];
          if (!fvarray)
            continue;

          EnlargeVArray (set1->max_base[b] - 1, &new_set->max_base[b],
                         &new_set->varray[b]);

          tvarray = new_set->varray[b];

          for (i = 0; i < set1->max_base[b]; i++)
            {
              fv = fvarray[i];
              if (!fv)
                continue;
              tv = tvarray[i] = NewVector ();

              t0 = fv[0];
              t1 = fv[1];
              t2 = fv[2];
              t3 = fv[3];
              t4 = fv[4];
              t5 = fv[5];
              t6 = fv[6];
              t7 = fv[7];
              tv[0] = t0;
              tv[1] = t1;
              tv[2] = t2;
              tv[3] = t3;
              tv[4] = t4;
              tv[5] = t5;
              tv[6] = t6;
              tv[7] = t7;
            }
        }
    }
  else
    return NULL;

  return new_set;
}

Set Set_union (Set set1, Set set2)
{
  int i, b, pos_max, neg_max;
  Set new_set;
  Set_Vector *fvarray, *tvarray;
  Set_Vector fv, tv;
  _32bit t0, t1, t2, t3, t4, t5, t6, t7;

#ifdef DEBUG_SET_UNION
  Set_print (stdout, "Set_union: set1", set1);
  Set_print (stdout, "Set_union: set2", set2);
#endif

  if (set1 && set2)
    {
      pos_max = MAX (set1->max_base[0], set2->max_base[0]);
      neg_max = MAX (set1->max_base[1], set2->max_base[1]);
    }
  else if (set1)
    {
      pos_max = set1->max_base[0];
      neg_max = set1->max_base[1];
    }
  else if (set2)
    {
      pos_max = set2->max_base[0];
      neg_max = set2->max_base[1];
    }
  else
    {
      pos_max = 0;
      neg_max = 0;
    }

  new_set = NewSet ();

  if (pos_max > Set_VARRAY_SIZE)
    EnlargeVArray (pos_max - 1, &new_set->max_base[0], &new_set->varray[0]);
  if (neg_max > 0)
    EnlargeVArray (neg_max - 1, &new_set->max_base[1], &new_set->varray[1]);

  if (set1 != 0)
    {                           /* copy set1 to new */
      for (b = 0; b < 2; b++)
        {
          fvarray = set1->varray[b];
          if (!fvarray)
            continue;
          tvarray = new_set->varray[b];

          for (i = 0; i < set1->max_base[b]; i++)
            {
              fv = fvarray[i];
              if (!fv)
                continue;
              tv = tvarray[i] = NewVector ();

              t0 = fv[0];
              t1 = fv[1];
              t2 = fv[2];
              t3 = fv[3];
              t4 = fv[4];
              t5 = fv[5];
              t6 = fv[6];
              t7 = fv[7];
              tv[0] = t0;
              tv[1] = t1;
              tv[2] = t2;
              tv[3] = t3;
              tv[4] = t4;
              tv[5] = t5;
              tv[6] = t6;
              tv[7] = t7;
            }
        }
    }
  if (set2 != 0)
    {                           /* copy set2 to new */
      for (b = 0; b < 2; b++)
        {
          fvarray = set2->varray[b];
          if (!fvarray)
            continue;
          tvarray = new_set->varray[b];

          for (i = 0; i < set2->max_base[b]; i++)
            {
              fv = fvarray[i];
              if (!fv)
                continue;
              if (!(tv = tvarray[i]))
                tv = tvarray[i] = NewVector ();

              t0 = fv[0];
              t1 = fv[1];
              t2 = fv[2];
              t3 = fv[3];
              t4 = fv[4];
              t5 = fv[5];
              t6 = fv[6];
              t7 = fv[7];
              tv[0] |= t0;
              tv[1] |= t1;
              tv[2] |= t2;
              tv[3] |= t3;
              tv[4] |= t4;
              tv[5] |= t5;
              tv[6] |= t6;
              tv[7] |= t7;
            }
        }
    }
#ifdef DEBUG_SET_UNION
  Set_print (stdout, "Set_union: after", new_set);
  fprintf (stdout, "\n");
#endif

  return (new_set);
}

/*
 *  Perform a set union and accumulate the result in set1.
 *  (i.e. do not create a new set:  set1 = set1 U set2)
 */
/* 10/04/02 REK Optimizing this function. */

Set Set_union_acc (Set set1, Set set2)
{
  int i, b;
  Set new_set;
  Set_Vector *fvarray, *tvarray;
  Set_Vector fv, tv;
  _32bit t0, t1, t2, t3, t4, t5, t6, t7;

#ifdef DEBUG_SET_UNION
  Set_print (stdout, "Set_union: set1", set1);
  Set_print (stdout, "Set_union: set2", set2);
#endif

  if (set1 == NULL)
    new_set = NewSet ();
  else
    new_set = set1;

  if (set2 != NULL)
    {                           /* copy set2 to new */
      for (b = 0; b < 2; b++)
        {
          fvarray = set2->varray[b];
          if (!fvarray)
            continue;
          tvarray = new_set->varray[b];
#if 0
	  /* 10/04/02 REK If we will need to enlarge the varray, do it now
	   *              once instead of several times inside the loop. */
	  if (set2->max_base[b] > new_set->max_base[b])
	    EnlargeVArray (set2->max_base[b], &new_set->max_base[b],
			   &new_set->varray[b]);
#endif
          /* SER: Go backwards from end: new_set grows once to necessary size. */
  	  for (i = set2->max_base[b] - 1; i >= 0; i--)
            {
	      fv = fvarray[i];
  	      if (!fv)
		continue;

  	      t0 = fv[0];
  	      t1 = fv[1];
  	      t2 = fv[2];
  	      t3 = fv[3];
  	      t4 = fv[4];
  	      t5 = fv[5];
  	      t6 = fv[6];
  	      t7 = fv[7];

	      /* 10/04/02 REK We only care if a single bit in these 8
	       *              is set, so if we do the or inside the if
	       *              statement, we can fall through as soon as
	       *              we find a bit. */
	      /*              all = ((t0 | t1) | (t2 | t3)) | ((t4 | t5) | (t6 | t7)); */
	      /*              if (all) */
	      if (t0 || t1 || t2 || t3 || t4 || t5 || t6 || t7)
		{
		  /* 10/04/02 REK The array is now expanded once before
		   *              the loop. */
		  if (i >= new_set->max_base[b])
		    EnlargeVArray (i, &new_set->max_base[b],
				   &new_set->varray[b]);

    		  if ((tv = new_set->varray[b][i]) == NULL)
   		    {
    		      tv = new_set->varray[b][i] = NewVector ();
		      tv[0] = t0;
		      tv[1] = t1;
		      tv[2] = t2;
		      tv[3] = t3;
		      tv[4] = t4;
		      tv[5] = t5;
		      tv[6] = t6;
		      tv[7] = t7;
		    }
		  else
		    {
		      tv[0] |= t0;
		      tv[1] |= t1;
		      tv[2] |= t2;
		      tv[3] |= t3;
		      tv[4] |= t4;
		      tv[5] |= t5;
		      tv[6] |= t6;
		      tv[7] |= t7;
		    }
                }
            }
        }
    }

#ifdef DEBUG_SET_UNION
  Set_print (stdout, "Set_union: after", new_set);
  fprintf (stdout, "\n");
#endif

  return (new_set);
}

Set
Set_union_acc_ch(Set set1, Set set2, int *change)
{
  int i, b;
  Set new_set;
  Set_Vector *fvarray, *tvarray;
  Set_Vector fv, tv;
  _32bit t0, t1, t2, t3, t4, t5, t6, t7, all;

#ifdef DEBUG_SET_UNION
  Set_print (stdout, "Set_union: set1", set1);
  Set_print (stdout, "Set_union: set2", set2);
#endif

  *change = 0;
  if (set1 == NULL)
    new_set = NewSet ();
  else
    new_set = set1;

  if (set2 != NULL)
    {                           /* copy set2 to new */
      for (b = 0; b < 2; b++)
        {
          fvarray = set2->varray[b];
          if (!fvarray)
            continue;
          tvarray = new_set->varray[b];

          for (i = 0; i < set2->max_base[b]; i++)
            {
              fv = fvarray[i];
              if (!fv)
                continue;

              t0 = fv[0];
              t1 = fv[1];
              t2 = fv[2];
              t3 = fv[3];
              t4 = fv[4];
              t5 = fv[5];
              t6 = fv[6];
              t7 = fv[7];
              all = ((t0 | t1) | (t2 | t3)) | ((t4 | t5) | (t6 | t7));
              if (all)
                {
                  if (i >= new_set->max_base[b])
                    EnlargeVArray (i, &new_set->max_base[b],
                                   &new_set->varray[b]);
                  if ((tv = new_set->varray[b][i]) == NULL)
                    {
                      tv = new_set->varray[b][i] = NewVector ();

                      tv[0] = t0;
                      tv[1] = t1;
                      tv[2] = t2;
                      tv[3] = t3;
                      tv[4] = t4;
                      tv[5] = t5;
                      tv[6] = t6;
                      tv[7] = t7;
		      *change = 1;
                    }
		  else
		    {
		      if (*change == 0)
			{
			  if ( (~(tv[0]) & t0) ||
			       (~(tv[1]) & t1) ||
			       (~(tv[2]) & t2) ||
			       (~(tv[3]) & t3) ||
			       (~(tv[4]) & t4) ||
			       (~(tv[5]) & t5) ||
			       (~(tv[6]) & t6) ||
			       (~(tv[7]) & t7) )
			    *change = 1;
			}

                      tv[0] |= t0;
                      tv[1] |= t1;
                      tv[2] |= t2;
                      tv[3] |= t3;
                      tv[4] |= t4;
                      tv[5] |= t5;
                      tv[6] |= t6;
                      tv[7] |= t7;
                    }
                }
            }
        }
    }

#ifdef DEBUG_SET_UNION
  Set_print (stdout, "Set_union: after", new_set);
  fprintf (stdout, "\n");
#endif

  return (new_set);
}


Set Set_intersect (Set set1, Set set2)
{
  Set new_set;
  Set_Vector *varray1, *varray2;
  Set_Vector v1, v2, vn;
  _32bit t0, t1, t2, t3, t4, t5, t6, t7, all;
  int b, i;

  if ((set1 == 0) || (set2 == 0))       /* empty set */
    return (NULL);

#ifdef DEBUG_SET_INTERSECT
  Set_print (stdout, "Set_intersect: set1", set1);
  Set_print (stdout, "Set_intersect: set2", set2);
#endif

  new_set = NewSet ();

  for (b = 0; b < 2; b++)
    {
      varray1 = set1->varray[b];
      if (!varray1)
        continue;
      varray2 = set2->varray[b];
      if (!varray2)
        continue;
      for (i = 0; i < set1->max_base[b]; i++)
        {
          v1 = varray1[i];
          if (!v1 || i >= set2->max_base[b])
            continue;
          v2 = varray2[i];
          if (!v2)
            continue;

          t0 = v1[0] & v2[0];
          t1 = v1[1] & v2[1];
          t2 = v1[2] & v2[2];
          t3 = v1[3] & v2[3];
          t4 = v1[4] & v2[4];
          t5 = v1[5] & v2[5];
          t6 = v1[6] & v2[6];
          t7 = v1[7] & v2[7];

          all = ((t0 | t1) | (t2 | t3)) | ((t4 | t5) | (t6 | t7));
          if (all)
            {
              if (i >= new_set->max_base[b])
                EnlargeVArray (i, &new_set->max_base[b], &new_set->varray[b]);
              vn = new_set->varray[b][i] = NewVector ();
              vn[0] = t0;
              vn[1] = t1;
              vn[2] = t2;
              vn[3] = t3;
              vn[4] = t4;
              vn[5] = t5;
              vn[6] = t6;
              vn[7] = t7;
            }
        }
    }
#ifdef DEBUG_SET_INTERSECT
  Set_print (stdout, "Set_intersect: after", new_set);
  fprintf (stdout, "\n");
#endif

  return (new_set);
}

/*
 *  Perform a set intersect and accumulate the result in set1.
 *  (i.e. does not create a new set unless set2 == NULL )
 */
Set Set_intersect_acc (Set set1, Set set2)
{
  Set_Vector *varray1, *varray2;
  Set_Vector v1, v2;
  _32bit t0, t1, t2, t3, t4, t5, t6, t7, all;
  int b, i;

#ifdef DEBUG_SET_INTERSECT
  Set_print (stdout, "Set_intersect_acc: set1", set1);
  Set_print (stdout, "Set_intersect_acc: set2", set2);
#endif

  if (set1 == NULL)
    return (NULL);

  if (set2 == NULL)
    return (Set_dispose (set1));

  for (b = 0; b < 2; b++)
    {
      varray1 = set1->varray[b];
      if (!varray1)
        continue;
      varray2 = set2->varray[b];
      if (!varray2)
        continue;
      for (i = 0; i < set1->max_base[b]; i++)
        {
          v1 = varray1[i];
          if (!v1)
            continue;

          if ((i >= set2->max_base[b]) || ((v2 = varray2[i]) == NULL))
            {
              FreeVector (v1);
              varray1[i] = NULL;
              continue;
            }

          t0 = v1[0] & v2[0];
          t1 = v1[1] & v2[1];
          t2 = v1[2] & v2[2];
          t3 = v1[3] & v2[3];
          t4 = v1[4] & v2[4];
          t5 = v1[5] & v2[5];
          t6 = v1[6] & v2[6];
          t7 = v1[7] & v2[7];

          all = ((t0 | t1) | (t2 | t3)) | ((t4 | t5) | (t6 | t7));
          if (all)
            {
              v1[0] = t0;
              v1[1] = t1;
              v1[2] = t2;
              v1[3] = t3;
              v1[4] = t4;
              v1[5] = t5;
              v1[6] = t6;
              v1[7] = t7;
            }
          else
            {
              FreeVector (v1);
              varray1[i] = NULL;
            }
        }
    }
#ifdef DEBUG_SET_INTERSECT
  Set_print (stdout, "Set_intersect_acc: after", set1);
  fprintf (stdout, "\n");
#endif

  return (set1);
}

int
Set_intersect_empty (Set set1, Set set2)
{
  Set_Vector *varray1, *varray2;
  Set_Vector v1, v2;
  _32bit t0, t1, t2, t3, t4, t5, t6, t7, all;
  int b, i;

  if ((set1 == 0) || (set2 == 0))       /* empty set */
    return (1);

  for (b = 0; b < 2; b++)
    {
      varray1 = set1->varray[b];
      if (!varray1)
        continue;
      varray2 = set2->varray[b];
      if (!varray2)
        continue;
      for (i = 0; i < set1->max_base[b]; i++)
        {
          v1 = varray1[i];
          if (!v1 || i >= set2->max_base[b])
            continue;
          v2 = varray2[i];
          if (!v2)
            continue;

          t0 = v1[0] & v2[0];
          t1 = v1[1] & v2[1];
          t2 = v1[2] & v2[2];
          t3 = v1[3] & v2[3];
          t4 = v1[4] & v2[4];
          t5 = v1[5] & v2[5];
          t6 = v1[6] & v2[6];
          t7 = v1[7] & v2[7];
          all = ((t0 | t1) | (t2 | t3)) | ((t4 | t5) | (t6 | t7));
          if (all)
            {
              return (0);
            }
        }
    }
  return (1);
}

Set Set_subtract (Set set1, Set set2)
{
  Set new_set;
  Set_Vector *varray1, *varray2;
  Set_Vector v1, v2, vn;
  _32bit t0, t1, t2, t3, t4, t5, t6, t7, all;
  int b, i;

  if (set1 == 0)
    return (NULL);

  if (set2 == 0)
    return (Set_union (set1, 0));

  new_set = NewSet ();

#ifdef DEBUG_SET_SUBTRACT
  Set_print (stdout, "Set_subtract: set1", set1);
  Set_print (stdout, "Set_subtract: set2", set2);
#endif

  for (b = 0; b < 2; b++)
    {
      varray1 = set1->varray[b];
      if (!varray1)
        continue;
      varray2 = set2->varray[b];

      for (i = 0; i < set1->max_base[b]; i++)
        {
          v1 = varray1[i];
          if (!v1)
            continue;

          if (!varray2 || (i >= set2->max_base[b]) || !(v2 = varray2[i]))
            {
              t0 = v1[0];
              t1 = v1[1];
              t2 = v1[2];
              t3 = v1[3];
              t4 = v1[4];
              t5 = v1[5];
              t6 = v1[6];
              t7 = v1[7];
            }
          else
            {
              t0 = v1[0] & ~v2[0];
              t1 = v1[1] & ~v2[1];
              t2 = v1[2] & ~v2[2];
              t3 = v1[3] & ~v2[3];
              t4 = v1[4] & ~v2[4];
              t5 = v1[5] & ~v2[5];
              t6 = v1[6] & ~v2[6];
              t7 = v1[7] & ~v2[7];
            }
          all = ((t0 | t1) | (t2 | t3)) | ((t4 | t5) | (t6 | t7));
          if (all)
            {
              if (i >= new_set->max_base[b])
                EnlargeVArray (i, &new_set->max_base[b], &new_set->varray[b]);
              vn = new_set->varray[b][i] = NewVector ();
              vn[0] = t0;
              vn[1] = t1;
              vn[2] = t2;
              vn[3] = t3;
              vn[4] = t4;
              vn[5] = t5;
              vn[6] = t6;
              vn[7] = t7;
            }
        }
    }
#ifdef DEBUG_SET_SUBTRACT
  Set_print (stdout, "Set_subtract: after", new_set);
  fprintf (stdout, "\n");
#endif

  return (new_set);
}

/*
 * GEH - added this function to do a subtract-accumulate.
 * Performs set1 <- set1 - set2.
 */
Set Set_subtract_acc (Set set1, Set set2)
{
  Set_Vector *varray1, *varray2;
  Set_Vector v1, v2;
  _32bit t0, t1, t2, t3, t4, t5, t6, t7;
  int b, i;

  if (set1 == 0)
    return (NULL);

  if (set2 == 0)
    return (set1);

#ifdef DEBUG_SET_SUBTRACT
  Set_print (stdout, "Set_subtract_acc: set1", set1);
  Set_print (stdout, "Set_subtract_acc: set2", set2);
#endif

  for (b = 0; b < 2; b++)
    {
      varray1 = set1->varray[b];
      if (!varray1)
        continue;
      varray2 = set2->varray[b];

      for (i = 0; i < set1->max_base[b]; i++)
        {
          v1 = varray1[i];
          if (!v1)
            continue;

          if (!varray2 || (i >= set2->max_base[b]) || !(v2 = varray2[i]))
            continue;

          t0 = v2[0];
          t1 = v2[1];
          t2 = v2[2];
          t3 = v2[3];
          t4 = v2[4];
          t5 = v2[5];
          t6 = v2[6];
          t7 = v2[7];
          v1[0] &= ~t0;
          v1[1] &= ~t1;
          v1[2] &= ~t2;
          v1[3] &= ~t3;
          v1[4] &= ~t4;
          v1[5] &= ~t5;
          v1[6] &= ~t6;
          v1[7] &= ~t7;
        }
    }
#ifdef DEBUG_SET_SUBTRACT
  Set_print (stdout, "Set_subtract_acc: after", set1);
  fprintf (stdout, "\n");
#endif

  return (set1);
}


int
Set_subtract_empty (Set set1, Set set2)
{
  Set_Vector *varray1, *varray2;
  Set_Vector v1, v2;
  _32bit t0, t1, t2, t3, t4, t5, t6, t7, all;
  int b, i;

  if (set1 == NULL)
    return (1);

  if (set2 == NULL)
    return (Set_empty (set1));

  for (b = 0; b < 2; b++)
    {
      varray1 = set1->varray[b];
      if (!varray1)
        continue;
      varray2 = set2->varray[b];

      for (i = 0; i < set1->max_base[b]; i++)
        {
          v1 = varray1[i];
          if (!v1)
            continue;

          if (!varray2 || (i >= set2->max_base[b]) || !(v2 = varray2[i]))
            {
              t0 = v1[0];
              t1 = v1[1];
              t2 = v1[2];
              t3 = v1[3];
              t4 = v1[4];
              t5 = v1[5];
              t6 = v1[6];
              t7 = v1[7];
            }
          else
            {
              t0 = v1[0] & ~v2[0];
              t1 = v1[1] & ~v2[1];
              t2 = v1[2] & ~v2[2];
              t3 = v1[3] & ~v2[3];
              t4 = v1[4] & ~v2[4];
              t5 = v1[5] & ~v2[5];
              t6 = v1[6] & ~v2[6];
              t7 = v1[7] & ~v2[7];
            }
          all = ((t0 | t1) | (t2 | t3)) | ((t4 | t5) | (t6 | t7));
          if (all)
            {
              return (0);
            }
        }
    }
  return (1);
}

Set Set_subtract_union (Set set1, Set set2, Set set3)
{
  Set new_set;
  Set_Vector *varray1, *varray2, *varray3;
  Set_Vector v1, v2, v3, vn;
  _32bit t0, t1, t2, t3, t4, t5, t6, t7, all;
  int b, i;
  int pos_max, neg_max;

  if (set1 == 0)
    return (Set_union (set3, 0));

  if (set3 == 0)
    return (Set_subtract (set1, set2));

  if (set2 == 0)
    return (Set_union (set1, set3));

  pos_max = MAX (set1->max_base[0], set3->max_base[0]);
  neg_max = MAX (set1->max_base[1], set3->max_base[1]);

  new_set = NewSet ();

  if (pos_max > Set_VARRAY_SIZE)
    EnlargeVArray (pos_max - 1, &new_set->max_base[0], &new_set->varray[0]);
  if (neg_max > 0)
    EnlargeVArray (neg_max - 1, &new_set->max_base[1], &new_set->varray[1]);

  for (b = 0; b < 2; b++)
    {
      varray1 = set1->varray[b];
      varray2 = set2->varray[b];
      varray3 = set3->varray[b];

      for (i = 0; i < set1->max_base[b]; i++)
        {
          v1 = varray1[i];
          if (!v1)
            continue;

          if (!varray2 || (i >= set2->max_base[b]) || !(v2 = varray2[i]))
            {
              t0 = v1[0];
              t1 = v1[1];
              t2 = v1[2];
              t3 = v1[3];
              t4 = v1[4];
              t5 = v1[5];
              t6 = v1[6];
              t7 = v1[7];
            }
          else
            {
              t0 = v1[0] & ~v2[0];
              t1 = v1[1] & ~v2[1];
              t2 = v1[2] & ~v2[2];
              t3 = v1[3] & ~v2[3];
              t4 = v1[4] & ~v2[4];
              t5 = v1[5] & ~v2[5];
              t6 = v1[6] & ~v2[6];
              t7 = v1[7] & ~v2[7];
            }
          all = ((t0 | t1) | (t2 | t3)) | ((t4 | t5) | (t6 | t7));
          if (all)
            {
              vn = new_set->varray[b][i] = NewVector ();
              vn[0] = t0;
              vn[1] = t1;
              vn[2] = t2;
              vn[3] = t3;
              vn[4] = t4;
              vn[5] = t5;
              vn[6] = t6;
              vn[7] = t7;
            }
        }
      for (i = 0; i < set3->max_base[b]; i++)
        {
          v3 = varray3[i];
          if (!v3)
            continue;
          if (!(vn = new_set->varray[b][i]))
            vn = new_set->varray[b][i] = NewVector ();

          t0 = v3[0];
          t1 = v3[1];
          t2 = v3[2];
          t3 = v3[3];
          t4 = v3[4];
          t5 = v3[5];
          t6 = v3[6];
          t7 = v3[7];
          vn[0] |= t0;
          vn[1] |= t1;
          vn[2] |= t2;
          vn[3] |= t3;
          vn[4] |= t4;
          vn[5] |= t5;
          vn[6] |= t6;
          vn[7] |= t7;
        }
    }

  return (new_set);
}

/*
 * GEH - added this function.  Performs set1 <- (set1 - set2) U set3.
 * Does not create a new set (unless set1 is NIL), but rather modifies set1.
 */
Set Set_subtract_union_acc (Set set1, Set set2, Set set3)
{
  Set_Vector *varray1, *varray2, *varray3;
  Set_Vector v1, v2, v3;
  _32bit t0, t1, t2, t3, t4, t5, t6, t7;
  int b, i;

  if (set1 == 0)
    return (Set_union (set3, 0));

  if (set3 == 0)
    return (Set_subtract_acc (set1, set2));

  if (set2 == 0)
    return (Set_union_acc (set1, set3));

  if (set1->max_base[0] < set3->max_base[0])
    EnlargeVArray (set3->max_base[0] - 1, &set1->max_base[0],
                   &set1->varray[0]);
  if (set1->max_base[1] < set3->max_base[1])
    EnlargeVArray (set3->max_base[1] - 1, &set1->max_base[1],
                   &set1->varray[1]);

  for (b = 0; b < 2; b++)
    {
      varray1 = set1->varray[b];
      varray2 = set2->varray[b];
      varray3 = set3->varray[b];

      for (i = 0; i < set1->max_base[b]; i++)
        {
          v1 = varray1[i];
          if (!v1)
            continue;

          if (!varray2 || (i >= set2->max_base[b]) || !(v2 = varray2[i]))
            continue;

          t0 = v2[0];
          t1 = v2[1];
          t2 = v2[2];
          t3 = v2[3];
          t4 = v2[4];
          t5 = v2[5];
          t6 = v2[6];
          t7 = v2[7];
          v1[0] &= ~t0;
          v1[1] &= ~t1;
          v1[2] &= ~t2;
          v1[3] &= ~t3;
          v1[4] &= ~t4;
          v1[5] &= ~t5;
          v1[6] &= ~t6;
          v1[7] &= ~t7;
        }
      for (i = 0; i < set3->max_base[b]; i++)
        {
          v3 = varray3[i];
          if (!v3)
            continue;
          if (!(v1 = varray1[i]))
            v1 = varray1[i] = NewVector ();

          t0 = v3[0];
          t1 = v3[1];
          t2 = v3[2];
          t3 = v3[3];
          t4 = v3[4];
          t5 = v3[5];
          t6 = v3[6];
          t7 = v3[7];
          v1[0] |= t0;
          v1[1] |= t1;
          v1[2] |= t2;
          v1[3] |= t3;
          v1[4] |= t4;
          v1[5] |= t5;
          v1[6] |= t6;
          v1[7] |= t7;
        }
    }

  return (set1);
}


Set Set_dispose (Set set)
{
  if (set != NULL)
    FreeSet (set);

  return (NULL);
}

Set Set_compress (Set set)
{
  Set new_set;
  Set_Vector *varray1;
  Set_Vector v1, vn;
  _32bit t0, t1, t2, t3, t4, t5, t6, t7, all;
  int b, i;

  if (set == 0)
    return 0;                   /* empty set */

  new_set = NewSet ();

  for (b = 0; b < 2; b++)
    {
      varray1 = set->varray[b];
      if (!varray1)
        continue;

      EnlargeVArray (set->max_base[b] - 1, &new_set->max_base[b],
		     &new_set->varray[b]);

      for (i = 0; i < set->max_base[b]; i++)
        {
          v1 = varray1[i];
          if (!v1)
            continue;

          t0 = v1[0];
          t1 = v1[1];
          t2 = v1[2];
          t3 = v1[3];
          t4 = v1[4];
          t5 = v1[5];
          t6 = v1[6];
          t7 = v1[7];

          all = ((t0 | t1) | (t2 | t3)) | ((t4 | t5) | (t6 | t7));
          if (all)
            {
              vn = new_set->varray[b][i] = NewVector ();
              vn[0] = t0;
              vn[1] = t1;
              vn[2] = t2;
              vn[3] = t3;
              vn[4] = t4;
              vn[5] = t5;
              vn[6] = t6;
              vn[7] = t7;
            }
        }
    }
  return (new_set);
}

void
Set_print (FILE * F, char *name, Set set)
{
  Set_Vector *varray1;
  Set_Vector v1;
  char *sign;
  int b, i, j, k, l, base, offset;

  fprintf (F, "(set %s (", name);

  if (set == 0)
    {
      fprintf (F, "))\n");
      return;
    }

  k = 0;
  for (b = 0; b < 2; b++)
    {
      varray1 = set->varray[b];
      if (!varray1)
        continue;

      sign = ((b == 1) ? " -" : " ");

      for (l = 0; l < set->max_base[b]; l++)
        {
          _32bit value;
          _32bit b1, b2, b3, b4;
          v1 = varray1[l];
          if (v1 != NULL)
            {
              base = Set_CONSTRUCT_BASE (l);
              offset = 0;
              for (i = 0; i < Set_VECTOR_SIZE; i++)
                {
                  value = v1[i];
                  for (j = 0; j < 32; j += 4)
                    {
                      b1 = value & 1;
                      b2 = ((value & 2) != 0);
                      b3 = ((value & 4) != 0);
                      b4 = ((value & 8) != 0);
                      value = value >> 4;
                      if (b1)
                        fprintf (F, "%s%d", sign, (base + offset + j));
                      if (b2)
                        fprintf (F, "%s%d", sign, (base + offset + j + 1));
                      if (b3)
                        fprintf (F, "%s%d", sign, (base + offset + j + 2));
                      if (b4)
                        fprintf (F, "%s%d", sign, (base + offset + j + 3));
                    }
                  offset += 32;
                }
            }
        }
    }
  fprintf (F, "))\n");
}

int
Set_2array (Set set, int *buf)
{
  Set_Vector *varray1;
  Set_Vector v1;
  int b, i, j, k, l, base, offset;
  int sign;

  if (set == NULL)
    return (0);                 /* empty set */
  k = 0;

  for (b = 0; b < 2; b++)
    {
      varray1 = set->varray[b];
      if (!varray1)
        continue;

      sign = ((b == 0) ? 1 : -1);

      for (l = 0; l < set->max_base[b]; l++)
        {
          _32bit value;
          _32bit b1, b2, b3, b4;
          v1 = varray1[l];
          if (v1 != NULL)
            {
              base = Set_CONSTRUCT_BASE (l);
              offset = 0;
              for (i = 0; i < Set_VECTOR_SIZE; i++)
                {
                  value = v1[i];
                  for (j = 0; j < 32; j += 4)
                    {
                      b1 = value & 1;
                      b2 = ((value & 2) != 0);
                      b3 = ((value & 4) != 0);
                      b4 = ((value & 8) != 0);
                      value = value >> 4;
                      if (b1)
                        buf[k++] = (sign * (base + offset + j));
                      if (b2)
                        buf[k++] = (sign * (base + offset + j + 1));
                      if (b3)
                        buf[k++] = (sign * (base + offset + j + 2));
                      if (b4)
                        buf[k++] = (sign * (base + offset + j + 3));
                    }
                  offset += 32;
                }
            }
        }
    }
  return k;
}

int
Set_same (Set set1, Set set2)
{
  Set_Vector *varray1, *varray2;
  Set_Vector v1, v2;
  _32bit all, t0, t1, t2, t3, t4, t5, t6, t7;
  int b, i;

#ifdef DEBUG_SET_SAME
  Set_print (stdout, "Set_same: set1", set1);
  Set_print (stdout, "Set_same: set2", set2);
#endif
  if (set1 == 0 && set2 == 0)
    return (1);

  if (set1 == 0)
    return (Set_empty (set2));

  if (set2 == 0)
    return (Set_empty (set1));

  for (b = 0; b < 2; b++)
    {
      varray1 = set1->varray[b];
      if (!varray1)
        continue;
      varray2 = set2->varray[b];

      for (i = 0; i < set1->max_base[b]; i++)
        {
          v1 = varray1[i];
          if (!v1)
            continue;

          if (!varray2 || (i >= set2->max_base[b]) || !(v2 = varray2[i]))
            {
              t0 = v1[0];
              t1 = v1[1];
              t2 = v1[2];
              t3 = v1[3];
              t4 = v1[4];
              t5 = v1[5];
              t6 = v1[6];
              t7 = v1[7];
            }
          else
            {
              t0 = v1[0] != v2[0];
              t1 = v1[1] != v2[1];
              t2 = v1[2] != v2[2];
              t3 = v1[3] != v2[3];
              t4 = v1[4] != v2[4];
              t5 = v1[5] != v2[5];
              t6 = v1[6] != v2[6];
              t7 = v1[7] != v2[7];
            }
          all = ((t0 | t1) | (t2 | t3)) | ((t4 | t5) | (t6 | t7));
          if (all)
            {
#ifdef DEBUG_SET_SAME
              fprintf (stdout, "Set_same: returning 0\n\n");
#endif
              return (0);
            }
        }
    }
  for (b = 0; b < 2; b++)
    {
      varray2 = set2->varray[b];
      if (!varray2)
        continue;
      varray1 = set1->varray[b];

      for (i = 0; i < set2->max_base[b]; i++)
        {
          v2 = varray2[i];
          if (!v2)
            continue;

          if (!varray1 || (i >= set1->max_base[b]) || !(v1 = varray1[i]))
            {
              t0 = v2[0];
              t1 = v2[1];
              t2 = v2[2];
              t3 = v2[3];
              t4 = v2[4];
              t5 = v2[5];
              t6 = v2[6];
              t7 = v2[7];
            }
          else
            {
              t0 = v1[0] != v2[0];
              t1 = v1[1] != v2[1];
              t2 = v1[2] != v2[2];
              t3 = v1[3] != v2[3];
              t4 = v1[4] != v2[4];
              t5 = v1[5] != v2[5];
              t6 = v1[6] != v2[6];
              t7 = v1[7] != v2[7];
            }
          all = ((t0 | t1) | (t2 | t3)) | ((t4 | t5) | (t6 | t7));
          if (all)
            {
#ifdef DEBUG_SET_SAME
              fprintf (stdout, "Set_same: returning 0\n\n");
#endif
              return (0);
            }
        }
    }
#ifdef DEBUG_SET_SAME
  fprintf (stdout, "Set_same: returning 1\n\n");
#endif
  return (1);
}



SetIterator *
Set_iterator (Set set, int finis)
{
  SetIterator *si;

  if (SET_SetIterator_pool == NULL)
    SET_SetIterator_pool = L_create_alloc_pool ("SET_SetIterator",
                                                sizeof (SetIterator), 32);

  si = (SetIterator *) L_alloc (SET_SetIterator_pool);

  si->set = set;

  si->current_varray = 0;
  si->current_base = 0;
  si->current_bit = 0;
  si->finis = finis;
  if (set)
    Set_next (si);
  else
    si->current_varray = 2;
  return si;
}

int
Set_exhausted (SetIterator * si)
{
  if (si->set && (si->current_varray < 2))
    return 0;
  else
    return 1;
}

int
Set_next (SetIterator * si)
{
  int tmp;
  int va, bo, bi, mo[2], wo, startwo, bit, startbit;
  _32bit *val, **var;

  if (si->set && (si->current_varray < 2))
    {
      tmp = si->nxt;

      /* Find new si->nxt */

      va = si->current_varray;
      bo = si->current_base;
      bi = si->current_bit;

      mo[0] = si->set->max_base[0];
      mo[1] = si->set->max_base[1];

      while (va < 2)
        {
          var = si->set->varray[va];
          if (var)
            while (bo < mo[va])
              {
                if ((val = var[bo]))
                  {
                    startwo = bi >> 5;
                    for (wo = startwo; wo < Set_VECTOR_SIZE; wo++)
                      {
                        if (val[wo])
                          {
                            startbit = bi & 0x1F;
                            for (bit = startbit; bit < 32; bit++, bi++)
                              {
                                if (val[wo] & (1 << bit))
                                  goto Set_next_FOUND;
                              }
                          }
                        else
                          bi += 32;
                      }
                  }
                bo++;
                bi = 0;
              }
          va++;
          bo = 0;
        }
    Set_next_FOUND:
      si->nxt = (va ? -1 : 1) * (Set_CONSTRUCT_BASE (bo) + bi);
      si->current_varray = va;
      si->current_base = bo;
      si->current_bit = bi + 1;
    }
  else
    tmp = si->nxt = si->finis;
  return tmp;
}

void
Set_reset (SetIterator * si, Set set)
{
  if (!si->set)
    si->set = set;
  else if (si->set != set)
    I_punt ("Set_reset: Attempted to reset an iterator on a different set");
  si->current_varray = 0;
  si->current_base = 0;
  si->current_bit = 0;
  Set_next (si);
  return;
}

SetIterator *
Set_end_iterator (SetIterator * si)
{
  L_free (SET_SetIterator_pool, si);
  return NULL;
}

void
Set_memory_usage (long *pused, long *presv, long *pmax)
{
  long sfree, sresv, tused = 0, tresv = 0, tmax = 0;

  if (SET_VArray_pool)
    {
      sresv = SET_VArray_pool->allocated;
      sfree = SET_VArray_pool->free;
      tused += (sresv - sfree) * sizeof (Set_Vector);
      tresv += (sresv) * sizeof (Set_Vector);
    }
  if (SET_Vector_pool)
    {
      sresv = SET_Vector_pool->allocated;
      sfree = SET_Vector_pool->free;
      tused += (sresv - sfree) * sizeof (Set_Vector);
      tresv += (sresv) * sizeof (Set_Vector);
    }
  if (SET_Set_pool)
    {
      sresv = SET_Set_pool->allocated;
      sfree = SET_Set_pool->free;
      tused += (sresv - sfree) * sizeof (_Set);
      tresv += (sresv) * sizeof (_Set);
    }

  tused += mallocated;
  tmax = tresv + maxmallocated;
  tresv += mallocated;

  if (pused)
    *pused = tused;
  if (presv)
    *presv = tresv;
  if (pmax)
    *pmax = tmax;
  return;
}


void
Set_print_memory_usage (FILE *fp)
{
  long tused, tresv, tmax;

  if (!fp)
    fp = stderr;

  Set_memory_usage (&tused, &tresv, &tmax);

  fprintf (fp, "Set memory usage: (%ldk in use / %ldk allocated / %ldk max)\n",
	   tused / 1024, tresv / 1024, tmax / 1024);
  return;
}
