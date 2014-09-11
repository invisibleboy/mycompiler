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
 *      File:    pipa_escape_analysis.h
 *      Author:  Erik Nystrom
 *      Copyright (c) 2003  Erik Nystrom, Wen-mei Hwu
 *              and The Board of Trustees of the University of Illinois.
 *              All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#ifndef _PIPA_ESCAPE_ANALYSIS_H_
#define _PIPA_ESCAPE_ANALYSIS_H_

#include "pipa_consg.h"
#include "pipa_graph.h"
#include "pipa_print_graph.h"
#include "pipa_options.h"


#define EA_LFULL_ESC    (IPA_CG_NODE_FLAGS_GENERIC1)
#define EA_LPROP_ESC    (IPA_CG_NODE_FLAGS_GENERIC2)
#define EA_RETPROP_ESC  (IPA_CG_NODE_FLAGS_GENERIC3)
#define EA_LCONT_ESC    (IPA_CG_NODE_FLAGS_GENERIC4)
#define EA_LCONT_ESCLCL (IPA_CG_NODE_FLAGS_GENERIC9)

#define EA_INPROGRESS  (IPA_CG_NODE_FLAGS_GENERIC5)

#define EA_SURVIVES    (IPA_CG_NODE_FLAGS_GENERIC6)
#define EA_PERMANENT   (IPA_CG_NODE_FLAGS_GENERIC7)

#define EA_FLAGSET     (IPA_CG_NODE_FLAGS_GENERIC8)

#define ESCAPE_PARAM_IFACE  0
#define ESCAPE_PARAM_FLAG   1

void
IPA_init_escape_analysis(IPA_prog_info_t * __info, 
			 IPA_cgraph_t * __cg,
			 IPA_interface_t *__iface,
			 IPA_funcsymbol_info_t *__param_fninfo);

void
IPA_new_escape_CE_node(IPA_cgraph_node_t *node, int flags);

void
IPA_new_escape_PE_node(IPA_cgraph_node_t *node, int flags);

void
IPA_new_escape_FE_node(IPA_cgraph_node_t *node, int flags);

List
IPA_do_escape_analysis();

int
IPA_any_new_marked (IPA_cgraph_t * consg);

#endif
