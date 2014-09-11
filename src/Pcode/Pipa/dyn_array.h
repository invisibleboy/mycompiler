/*****************************************************************************\
 *
 *                    Illinois Open Source License
 *                     University of Illinois/NCSA
 *                         Open Source License
 *
 * Copyright (c) 2004, The University of Illinois at Urbana-Champaign.
 * All rights reserved.
 *
 * Developed by:
 *
 *              IMPACT Research Group
 *
 *              University of Illinois at Urbana-Champaign
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
 *      File:    dyn_array.h
 *      Author:  Erik Nystrom
 *      Copyright (c) 2003  Erik Nystrom, Wen-mei Hwu
 *              and The Board of Trustees of the University of Illinois.
 *              All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/


#ifndef __PIPA_DYNA_H_
#define __PIPA_DYNA_H_

#include "pipa_common.h"

/*******************************************************************
 * Dynamically sizing array
 *******************************************************************/

typedef struct DYNA_t
{
  int el_size;
  int maxindex;
  int max_assigned_index;
  void *ptr;
}
DYNA_t;

void DYNA_init (DYNA_t * darray, int maxindex, int el_size);
DYNA_t *DYNA_new (int maxindex, int el_size);
DYNA_t *DYNA_copy (DYNA_t * darray);
void DYNA_reset (DYNA_t * darray);
void DYNA_free (DYNA_t * darray);
void DYNA_pad (DYNA_t * darray);
void DYNA_add_element (DYNA_t * darray, int index, void *element);
void DYNA_shrink_to (DYNA_t * darray, int new_max);
void DYNA_shrink_one (DYNA_t * darray);
int DYNA_append_element (DYNA_t * darray, void *element);
void *DYNA_get_element (DYNA_t * darray, int index);
void *DYNA_get_last_element (DYNA_t * darray);
void DYNA_delete_element (DYNA_t * darray, int index);
void DYNA_rmlast_element (DYNA_t * darray);
int DYNA_get_indexbound (DYNA_t * darray);
int DYNA_find_ptr (DYNA_t * darray, void *ptr);

#endif
