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
 *      File :          l_opti_count.h
 *      Description :   Routines for reporting the occurrence of optimizations
 *      Creation Date : March, 1998
 *      Author :        John Sias, Wen-mei Hwu
 *
 *==========================================================================*/

#ifndef L_OPTI_COUNT_H
#define L_OPTI_COUNT_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>

/*
 * Defining DO_OPTI_COUNT enables optimization counting for Lopti and 
 * Lsuperscalar.
 * Note that modules which use Lopti or Lsuperscalar functions but do not have
 * STAT_INIT() before those calls in those modules will punt.
 */
#undef DO_OPTI_COUNT

#define STAT_INITIAL_SIZE 64

#ifdef DO_OPTI_COUNT
#define STAT_INIT(string,fn) STAT_counter_init(string, fn)
#define STAT_COUNT(string,count,cb) STAT_opticount(string, count, cb)
#define STAT_DUMP() STAT_counter_dump()
#else
#define STAT_INIT(string,fn)
#define STAT_COUNT(string,count,cb)
#define STAT_DUMP()
#endif

typedef struct _STAT_DB
{
  STRING_Symbol_Table *table;
}
STAT_DB;

extern STAT_DB *STAT_db;

void STAT_counter_init (char *title, L_Func * fn);

void STAT_opticount (char *opti_name, int count, L_Cb * cb);

void STAT_counter_dump (void);

#endif
