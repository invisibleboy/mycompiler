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
 *      File :          l_histogram.h
 *      Description :   Histogram data structure and related functs
 *      Creation Date : June 1994
 *      Author :        Scott Mahlke
 *
 *==========================================================================*/
#ifndef L_HISTOGRAM_H
#define L_HISTOGRAM_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <stdio.h>
#include <malloc.h>


#define L_DEFAULT_HISTOGRAM_SIZE        1024
#define L_MAX_HISTOGRAM_SIZE            L_DEFAULT_HISTOGRAM_SIZE*256

typedef struct L_Histogram
{
  char *name;
  double *data;
  int size;
  double over_max_data;
}
L_Histogram;


/* print modes */
#define L_PRINT_ALL_ENTRIES             1
#define L_PRINT_GROUPED_VALUES          2
#define L_PRINT_GROUPED_PERCENTAGES     3
#define L_PRINT_INDIVIDUAL_PERCENTAGES  4
#define L_PRINT_AVERAGE_VALUE           5

#ifdef __cplusplus
extern "C"
{
#endif

  extern L_Histogram *L_create_histogram (char *, int);
  extern void L_delete_histogram (L_Histogram *);
  extern void L_realloc_histogram_data (L_Histogram *, int);
  extern void L_update_histogram (L_Histogram *, int, double);
  extern void L_clear_histogram (L_Histogram *);
  extern void L_scale_histogram_entries (L_Histogram *, double);
  extern void L_copy_histogram (L_Histogram *, L_Histogram *);
  extern void L_add_histograms (L_Histogram *, L_Histogram *, L_Histogram *);
  extern void L_print_histogram (FILE *, L_Histogram *, int, double);

#ifdef __cplusplus
}
#endif

#endif
