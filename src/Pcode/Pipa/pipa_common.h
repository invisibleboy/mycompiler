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
 *      File:    pipa_common.h
 *      Author:  Erik Nystrom
 *      Copyright (c) 2003  Erik Nystrom, Wen-mei Hwu
 *              and The Board of Trustees of the University of Illinois.
 *              All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/


#ifndef __PIPA_COMMON_H_
#define __PIPA_COMMON_H_

#include <Pcode/pcode.h>
#include <Pcode/symtab.h>
#include <Pcode/parms.h>
#include <Pcode/struct.h>
#include <Pcode/struct_symtab.h>
#include <Pcode/query.h>
#include <Pcode/query_symtab.h>
#include <Pcode/write.h>
#include <Pcode/write_symtab.h>
#include <Pcode/extension.h>

#include <library/l_alloc_new.h>
#include <library/i_hash.h>
#include <library/i_error.h>
#include <library/string_symbol.h>
#include <library/i_list.h>
#include <library/c_basic.h>

#include "dyn_array.h"
#include "pipa_options.h"
#include "pipa_hash.h"

#define HS_CI        1
#define UNIFY        1
#define SAFEOFFSET   1
#define PARTIALSUM   0

#define EDGE_HISTORY 0
#define EDGE_STATS   0

#define Min(a,b)  ((a)<(b)?(a):(b))
#define Max(a,b)  ((a)<(b)?(b):(a))

#if 0
#define DEBUG_IPA_LEVEL    0
#define DEBUG_IPA(val,s)   if (val <= DEBUG_IPA_LEVEL) {s}
#define DEBUG(s)           s
#else
#define DEBUG_IPA(val,s)
#define DEBUG(s)
#endif

#define IPA_FLAG_CLR(f,m)   ((f) = (f) & (~(m)))
#define IPA_FLAG_SET(f,m)   ((f) = (f) | (m))
#define IPA_FLAG_ISSET(f,m) (((f) & (m)) != 0)
#define IPA_FLAG_ISCLR(f,m) (((f) & (m)) == 0)

/**************************************************************************
 * Prototypes and general defines
 **************************************************************************/

extern int ipa_pointer_size;
#define IPA_POINTER_SIZE  ipa_pointer_size

struct IPA_interface_t;
struct IPA_callsite_t;
struct IPA_symbol_info_t;
struct IPA_funcsymbol_info_t;
struct IPA_prog_info_t;

/*******************************************************************
 * Simple Helper functions
 *******************************************************************/
double IPA_GetTime ();
void IPA_TrackTime();

int IPA_find_file_isnext_char (FILE * file, char token);
void IPA_find_file_strtoken (FILE * file, char *string);
void IPA_find_file_chartoken (FILE * file, char token);

FILE *IPA_db_stdout ();
int   IPA_mapid (HashTable id_htab, int id);
FILE *IPA_fopen(char *basedir, char *ipa_subdir, char *name, 
		char *ext, char *rw);

#endif
