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
 *      File:   select_sort.h
 *      Author: Pohua Paul Chang
 *      Copyright (c) 1991 Pohua Paul Chang, Wen-Mei Hwu. All rights reserved.  
 *      All rights granted to the University of Illinois.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/
#ifndef IMPACT_SELECT_SORT_H
#define IMPACT_SELECT_SORT_H

/* 10/29/02 REK Adding config.h */
#include <config.h>

typedef struct ST_Entry
{
  void *ptr;                    /* a general pointer attribute */
  int index;                    /* a general integer attribute */
  double weight;                /* the weight of this entry */
}
ST_Entry;

#ifdef __cplusplus
extern "C"
{
#endif

/*
 *      Sort according to the least weight order.
 *      Return 0 if the array is uniquely sorted
 *      (all elements have different weights).
 *      Otherwise the array is not uniquely sorted
 *      for entries with identical weights, and a
 *      non-zero value is returned.
 */
  extern int min_sort (ST_Entry * array, int size);


/*
 *      Sort according to the maximum weight order.
 *      Return 0 if the array is uniquely sorted
 *      (all elements have different weights).
 *      Otherwise the array is not uniquely sorted
 *      for entries with identical weights, and a
 *      non-zero value is returned.
 */
  extern int max_sort (ST_Entry * array, int size);

#ifdef __cplusplus
}
#endif

#endif
