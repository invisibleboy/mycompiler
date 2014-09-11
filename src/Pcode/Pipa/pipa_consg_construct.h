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
 *
 *      File:    pipa_consg_construct.h
 *      Author:  Erik Nystrom
 *      Copyright (c) 2003  Erik Nystrom, Wen-mei Hwu
 *              and The Board of Trustees of the University of Illinois.
 *              All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#ifndef _PIPA_CONSG_CONSTRUCT_H_
#define _PIPA_CONSG_CONSTRUCT_H_

#include "pipa_common.h"
#include "pipa_program.h"
#include "pipa_consg.h"

typedef enum buildcg_mode_t 
{B_FOLLOW, B_BUILD} buildcg_mode_t;

typedef enum buildcg_status_t 
{B_VAR, B_ADDR, B_ASSIGN, B_DEREF} buildcg_status_t;

extern char *IPA_bcg_status_string[];

typedef struct buildcg_t 
{
  /* Func info for original request */
  IPA_funcsymbol_info_t * fninfo;
  /* The real constraint graph (SCC merging 
     can make this differ from the request) */
  IPA_cgraph_t       *consg;
  /* The first node in the frontier list 
   *  BUILD MODE: there should be only one node
   *  FOLLOW MODE: merging etc can lead to multiple
   *               nodes on the same kind of edge.
   *               AT THE END of the following for
   *               and expr it does not matter which
   *               one you use from a points-to perspective.
   *               node contains the first.
   */
  IPA_cgraph_node_t  *node;
  /* A list of all nodes matching the query up
   *   to this point. There should be only one
   *   unless the mode is FOLLOW (post-analysis).
   */
  List                node_frontier;
  /* Symbol kind for the node
   */
  int                 kind;
  /* List of tmp nodes created so far for this query
   */
  List                tmp_nodes;
  /* Current offset (i.e.  obj.f)
   */
  int                 offset;
  /* Current skew (i.e. (*obj).f)
   */
  int                 skew;
  /* Current node status (obj, *obj, ...)
   */
  buildcg_status_t    status;
  /* Building or following
   */
  buildcg_mode_t      mode;

  char isarray;
} buildcg_t;

buildcg_t*
IPA_buildcg_start(IPA_prog_info_t * info, 
		  IPA_funcsymbol_info_t * fninfo,
		  int var_id,
		  int var_subscr,
		  buildcg_mode_t mode);

int
IPA_bcg_follow_edge(IPA_prog_info_t * info, buildcg_t *bcg, 
		    IPA_cgraph_edgelist_e edge_type,
		    int t_off, int size, int s_off);

void
IPA_buildcg_free(buildcg_t *bcg);

void
IPA_buildcg_kill(buildcg_t *bcg);

void
IPA_bcg_addrof(IPA_prog_info_t * info, buildcg_t *bcg);

void
IPA_bcg_offset(IPA_prog_info_t * info, buildcg_t *bcg,
	       int offset);
void
IPA_bcg_add(IPA_prog_info_t * info, buildcg_t *bcg, 
	    int offset);
void
IPA_bcg_reduce(IPA_prog_info_t * info, buildcg_t *bcg);

void
IPA_bcg_deref(IPA_prog_info_t * info, buildcg_t *bcg);

void
IPA_bcg_assign(IPA_prog_info_t * info, 
	       buildcg_t *l_bcg,buildcg_t *r_bcg,
	       int size);

IPA_symbol_info_t *
IPA_new_tmp_var(IPA_prog_info_t * info, 
		IPA_funcsymbol_info_t * fninfo,
		int kind);

#endif
