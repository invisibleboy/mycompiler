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
 *      File :          l_alloc_new.h
 *      Description :   New and improved data structure allocation/mgmt
 *      Creation Date : May 1993
 *      Authors :       John C. Gyllenhaal and Wen-mei Hwu
 *      Contributors:   Roger Bringmann and Scott Mahlke
 *
 *==========================================================================*/
#ifndef L_ALLOC_NEW_H
#define L_ALLOC_NEW_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <stdio.h>

typedef struct _L_Alloc_Pool_Header
{
  struct _L_Alloc_Pool_Header *next;
}
L_Alloc_Pool_Header;

typedef struct _L_Alloc_Pool
{
  char *name;
  int element_size;
  int block_size;               /* Block size to allocate */
  int num_in_block;             /* Num elements in block */
  struct _L_Alloc_Pool_Header *head;     /* Pointer to head of free list */
  int allocated;                /* Number allocated */
  int free;                     /* Number free */
  int blocks_allocated;
  struct _L_Alloc_Pool_Header *block_list; /* Pointer to blocks allocated */
  int bypass_routines;
  struct _L_Alloc_Pool *next;
}
L_Alloc_Pool;

#define L_ALLOC_POOL(typ) L_Alloc_Pool *L_alloc_pool_##typ = NULL
#define L_INIT_ALLOC_POOL(typ,sz) { if (!L_alloc_pool_##typ)                  \
                    L_alloc_pool_##typ =  L_create_alloc_pool ("typ",         \
                    sizeof(typ), sz); }
#define L_DEINIT_ALLOC_POOL(typ) { if (L_alloc_pool_##typ) {                  \
                    L_free_alloc_pool (L_alloc_pool_##typ)                    \
                    L_alloc_pool_##typ = NULL; }}
#define L_ALLOC(typ)      (typ *)L_alloc(L_alloc_pool_##typ)
#define L_FREE(typ, p)    {L_free(L_alloc_pool_##typ, p),p = NULL;}

/* Prototypes */
#ifdef __cplusplus
extern "C"
{
#endif

  extern L_Alloc_Pool *L_create_alloc_pool (char *, int, int);
  /* (char *name, int size, int num_in_block) */
  extern void L_free_alloc_pool (L_Alloc_Pool *);
  /* (L_Alloc_Pool *pool) */

  extern void *L_alloc (L_Alloc_Pool *);
  /* (L_Alloc_Pool *pool) */
  extern void L_free (L_Alloc_Pool *, void *);
  /* (L_Alloc_Pool *pool, void *ptr) */
  extern long L_print_alloc_info (FILE *, L_Alloc_Pool *, int);
  /* (FILE *F, L_Alloc_Pool *pool, int verbose) */

  extern void L_summarize_allocation_info (FILE *fp);

#ifdef __cplusplus
}
#endif


/* 
 * Set this to 1 before creating an alloc pool to bypass alloc routines
 * so that malloc and free are called each time L_alloc and L_free are called
 * with that alloc pool. 
 * This allows efficient use of the debug malloc routines.
 */

#ifdef __cplusplus
extern "C"
{
#endif

  extern int bypass_alloc_routines;

#ifdef __cplusplus
}
#endif

#endif
