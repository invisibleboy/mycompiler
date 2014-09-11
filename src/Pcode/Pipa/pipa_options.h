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
 *      File:    pipa_options.h
 *      Author:  Erik Nystrom
 *      Copyright (c) 2003  Erik Nystrom, Wen-mei Hwu
 *              and The Board of Trustees of the University of Illinois.
 *              All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#ifndef _PIPA_OPTIONS_H_
#define _PIPA_OPTIONS_H_

#include "pipa_common.h"


#define IPA_FIELD_INDEPENDENT_STR             "fi"
#define IPA_FIELD_INDEPENDENT                 0
#define IPA_FIELD_DEPENDENT_VARIABLE_SIZE_STR "fdvs"
#define IPA_FIELD_DEPENDENT_VARIABLE_SIZE     2
extern int IPA_field_option;
extern char *IPA_field_string[];


#define IPA_CONTEXT_INSENSITIVE_STR          "ci"
#define IPA_CONTEXT_INSENSITIVE              0
#define IPA_CONTEXT_SENSITIVE_STR            "cs"
#define IPA_CONTEXT_SENSITIVE                1
extern int IPA_context_option;
extern char *IPA_context_string[];

#define IPA_CSREC_NONE_STR                   "none"
#define IPA_CSREC_NONE                       0
#define IPA_CSREC_BREAK_STR                  "break"
#define IPA_CSREC_BREAK                      1
#define IPA_CSREC_FULL_STR                   "full"
#define IPA_CSREC_FULL                       2
extern int IPA_csrec_option;
extern char *IPA_csrec_string[];

#define IPA_GCON_ANDERSEN_STR                "and"
#define IPA_GCON_ANDERSEN                    0
#define IPA_GCON_STEENSDGARD_STR             "stgd"
#define IPA_GCON_STEENSDGARD                 1 
extern int IPA_gcon_option;
extern char *IPA_gcon_string[];

#define IPA_NO_HEAP_CLONING     0
#define IPA_HEAP_CLONING        1
extern int IPA_cloning_option;
extern int IPA_cloning_gen;
extern char *IPA_cloning_string[];

#define IPA_ALLOW_PTR_AGGR           0
#define IPA_ALLOW_PTR_AGGR_STR       "pa"
#define IPA_ALLOW_PTR_AGGR_INDIR     1
#define IPA_ALLOW_PTR_AGGR_INDIR_STR "pai"
#define IPA_ALLOW_ALL                2
#define IPA_ALLOW_ALL_STR            "all"
extern int IPA_allow_pcode_expr;
extern char *IPA_allow_pcode_expr_string[];

#define IPA_SAFETY_LEVEL0               0
#define IPA_SAFETY_LEVEL1               1
#define IPA_SAFETY_LEVEL2               2
extern int IPA_field_safety;
extern char *IPA_field_safety_string[];

#define IPA_FLOW_NONE 			0
#define IPA_FLOW_FULL 			1
#define IPA_FLOW_LOOP 			2
#define IPA_FLOW_PHIS 			3
#define IPA_FLOW_DJLT 			4
extern int IPA_flow_sensitive_type;

extern int IPA_sync_gen;
extern int IPA_sync_gen_testonly;
extern int IPA_sync_append_all_obj;
extern int IPA_sync_annotate_all_obj;

extern int IPA_solver_limit_fscost;

extern int IPA_exclude_zero_profile;
extern int IPA_allow_missing_ipa;
extern int IPA_use_actualformal_filter;

extern int IPA_print_merged_fn;
extern int IPA_print_initial_cng;
extern int IPA_print_summary_cng;
extern int IPA_print_final_cng;
extern int IPA_print_solved_cng;

extern char *IPA_library_dir;
extern char *IPA_file_subdir;

extern int IPA_slow_callgraph;

extern int IPA_myflag;
struct debuginfo_t {
  int round;
  int print;
  int check;
};
extern struct debuginfo_t debug;

void IPA_read_parms (Parm_Parse_Info * ppi);

#endif
