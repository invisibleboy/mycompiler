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



#ifndef _PIPA_MISC_UTILS_H_
#define _PIPA_MISC_UTILS_H_

#include "pipa_common.h"
#include "pipa_consg_fdvs.h"


typedef struct off_class_t 
{
  int  offset;
  char is_func;
  char is_str;
  List list;
  IPA_cgraph_t *consg;
} off_class_t;

int 
PIPA_off_class_same(IPA_cgraph_node_t *node, int offset, off_class_t *oc, int use_consg);

void
PIPA_off_class_addnode(off_class_t *oc, IPA_cgraph_node_t *node);

off_class_t *
PIPA_off_class_new(IPA_cgraph_node_t *node, int offset);

void
PIPA_off_class_free(off_class_t *oc);

List
PIPA_off_add_to_class(List oc_list, int offset, IPA_cgraph_node_t *node,
		      int use_consg);

void
IPA_find_merge_equiv(IPA_cgraph_t * consg);

void
IPA_find_merge_equiv2(IPA_cgraph_t * consg);

void
IPA_find_merge_summary_equiv (IPA_cgraph_t *consg);

void
IPA_find_merge_summary_equiv_new (IPA_funcsymbol_info_t *fninfo, IPA_cgraph_t *consg);

void
IPA_find_delete_deref_loop(IPA_cgraph_t *consg);

/*************************************************************************
 *
 * There routines handle uncalled top-level procedures for analyzing
 *   program fragments that do not contain main()
 *
 *************************************************************************/

typedef struct tlparam_t 
{
  Key type_key;
  List art_src_nodes;
  List dst_nodes;
} tlparam_t;

void
IPA_connect_inputs(IPA_prog_info_t * info);

#endif
