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
 *      File:   merge_sort.c
 *      Author: Pohua Chang
 *      Copyright (c) 1991 Pohua Chang, Wen-Mei Hwu. All rights reserved.  
 *      All rights granted to the University of Illinois.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <library/merge_sort.h>

static void
merge (struct Element *left, int size_left, struct Element *right,
       int size_right)
{
  struct Element temp[RECORD_SIZE];
  int i, j, k;
  i = j = k = 0;
  while ((i < size_left) && (j < size_right))
    {
      if (left[i].weight >= right[j].weight)
        {
          temp[k].weight = left[i].weight;
          temp[k].index = left[i].index;
          temp[k].ptr = left[i].ptr;
          i++;
          k++;
        }
      else
        {
          temp[k].weight = right[j].weight;
          temp[k].index = right[j].index;
          temp[k].ptr = right[j].ptr;
          j++;
          k++;
        }
    }
  while (i < size_left)
    {
      temp[k].weight = left[i].weight;
      temp[k].index = left[i].index;
      temp[k].ptr = left[i].ptr;
      i++;
      k++;
    }
  while (j < size_right)
    {
      temp[k].weight = right[j].weight;
      temp[k].index = right[j].index;
      temp[k].ptr = right[j].ptr;
      j++;
      k++;
    }
  for (i = 0; i < (size_left + size_right); i++)
    {
      left[i].weight = temp[i].weight;
      left[i].index = temp[i].index;
      left[i].ptr = temp[i].ptr;
    }
}

void
merge_sort (struct Element *list, struct Element *output, int len)
{
  int middle;
  if (len <= 1)
    {
      output[0].index = list[0].index;
      output[0].weight = list[0].weight;
      output[0].ptr = list[0].ptr;
      return;
    }
  else
    {
      middle = (len + 1) / 2;
      merge_sort (list, output, middle);
      merge_sort (list + middle, output + middle, len - middle);
      merge (output, middle, output + middle, len - middle);
    }
}
