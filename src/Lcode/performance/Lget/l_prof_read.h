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
 *	File :		l_prof_read.h
 *	Description :	Header for for Lcode profile file reader
 *	Author : 	Teresa Jonson, Pohua Paul Chang.
 *
 *	(C) Copyright 1990, Pohua Chang.
 *	All rights granted to University of Illinois Board of Regents.
 *==========================================================================*/
#ifndef L_PROF_READ_H
#define L_PROF_READ_H

#include <config.h>
#include <Lcode/l_main.h>

#define HEAP_OBJ_BASE_ADDR 100000
#define MALLOC_OBJ_BASE_ADDR 200000

typedef struct mem_obj
{
  int id;
  int version;
  int offset;
  int size;
  int weight;
  struct mem_obj * prev;
  struct mem_obj * next;
  
} mem_obj;

void	L_read_profile_func(FILE *profile,char *func);	/* (file, fn_name) */
double	L_profile_func_weight();	/* () */
double	L_cb_weight();			/* (cb_id) */
void    L_update_cb_loop_info(L_Cb *cb);
double	L_br_weight();			/* (id) */
double	L_taken_weight();		/* (id) */
int	L_cc_weight();			/* (id, cc[], weight[], len) */
int *   L_get_mem_glob_entry (int id, int *num);
int *   L_get_mem_heap_entry (int id, int *num);
int *   L_get_mem_missing_entry (int id, int *num);
int     L_get_mem_stack_count (int id);
int     L_get_malloc_id (int id);
int     L_matches_fn_name (char *fn_name, char *test_name);
void    L_cleanup_mem_arrays ();
#endif
