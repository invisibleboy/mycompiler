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
 *      File:    pipa_symbols.h
 *      Author:  Erik Nystrom
 *      Copyright (c) 2003  Erik Nystrom, Wen-mei Hwu
 *              and The Board of Trustees of the University of Illinois.
 *              All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#ifndef __PIPA_SYMBOLS_H_
#define __PIPA_SYMBOLS_H_

#include "pipa_common.h"
#include "pipa_callsite.h"
#include <dirent.h>

extern int IPA_max_type_size;

/**************************************************************************
 * Symbol information
 **************************************************************************/

#define IPA_VAR_KIND_GLOBAL   0x00000001
#define IPA_VAR_KIND_PARAM    0x00000002
#define IPA_VAR_KIND_RETURN   0x00000004
#define IPA_VAR_KIND_HEAP     0x00000008
#define IPA_VAR_KIND_FUNC     0x00000010
#define IPA_VAR_KIND_TEMP     0x00000020
#define IPA_VAR_KIND_STACK    0x00000040
#define IPA_VAR_KIND_STRING   0x00000080

#define IPA_TEMPVAR_FILEID             -13
#define IPA_TEMPTYPE_FILEID            -14
extern Key IPA_global_key;

typedef struct IPA_symaccess_t
{
  int version;
  int offset;

  int exprid;
  List expridlist;

  int ld_acc;
  int st_acc;
  int jsr_acc;  
  
  int merge_reps;
  Set ssaReps; 
  
  Set ld_sym_acc;
  Set st_sym_acc;

  HashTable ld_hash;
  HashTable st_hash;
  
  /* For heaps, who is the root ancestor heap object */
  struct IPA_symaccess_t *hp_rep;  
  /* For heaps, how many decendent heap objects does
     this represent */
  int reps;
} IPA_symaccess_t;

typedef struct IPA_symbol_info_t
{
  int id;
  int max_version;
  int acc_list_size;
  int kind;
  char *symbol_name;
  Key sym_key;
  Key type_key;
  struct IPA_funcsymbol_info_t *fninfo;
  /* If this symbol is a function this is its info */
  struct IPA_funcsymbol_info_t *its_fninfo;

  List *obj_acc_list;
}
IPA_symbol_info_t;

IPA_symbol_info_t *IPA_symbol_new ();
void IPA_symbol_free (IPA_symbol_info_t * sym);

Key
IPA_symbol_tmpvarkey();
Key
IPA_symbol_tmptypekey();

IPA_symbol_info_t *
IPA_symbol_add (struct IPA_prog_info_t * info,
		struct IPA_funcsymbol_info_t * fninfo,
                char *sym_name, Key sym_key,
		int kind, Key type_key);

IPA_symbol_info_t *
IPA_symbol_find (struct IPA_prog_info_t * info,
                 Key sym_key);

IPA_symbol_info_t *
IPA_symbol_find_by_id (struct IPA_prog_info_t *info,
		       int id);

void
IPA_symbol_dump(FILE *file, struct IPA_prog_info_t * info);


/**************************************************************************
 * Function and symbol information
 **************************************************************************/

typedef struct IPA_summary_info_t
{
  struct IPA_funcsymbol_info_t *fninfo;
  List   sum_nodes;
  struct IPA_summary_info_t *nxt;
} IPA_summary_info_t;

typedef struct IPA_funcsymbol_info_t
{
  Key func_key;
  char *file_name;
  char *func_name;

  char  from_library;
  char  is_heap_alloc;
  char  is_heap_free;
  char  is_blockcopy;
  char  is_noexit;
  char  is_globals;

  int   ellipse_id;
  int   lastfixed_id;
  char  is_vararg;

  IPA_interface_t *iface;
  List callsites;

  struct IPA_callg_node_t *call_node;
  struct IPA_cgraph_t *consg;           /* Assoc. Constraint Graph */
  IPA_summary_info_t  *sum_info;
  struct IPA_cgraph_t *lsum_consg;      /* Assoc. Summary of Constraint Graph */

  char has_been_called;
  char forced_ci;
  char summary_valid;      
  
  int  summary_size;
  int  orig_size;
  int  observed;

  List def_aliasnode_list;
  List use_aliasnode_list;
  List intra_du_list;
  void *ext;
}
IPA_funcsymbol_info_t;

IPA_funcsymbol_info_t *IPA_funcsymbol_new ();

void IPA_funcsymbol_free (struct IPA_prog_info_t *info,
                          IPA_funcsymbol_info_t * fninfo);

IPA_funcsymbol_info_t *IPA_funcsymbol_add (struct IPA_prog_info_t *info,
					   Key func_key,
                                           char *file_name, char *func_name);

IPA_funcsymbol_info_t *IPA_funcsymbol_find (struct IPA_prog_info_t *info,
                                            Key func_key);

void IPA_funcsymbol_set_ret_id (struct IPA_prog_info_t *info,
                                Key func_key, int id);

void IPA_funcsymbol_append_param_id (struct IPA_prog_info_t *info,
                                     Key func_key, int id);

IPA_funcsymbol_info_t *
IPA_funcsymbol_add_ext (Key func_key,
			char *func_name);

#endif
