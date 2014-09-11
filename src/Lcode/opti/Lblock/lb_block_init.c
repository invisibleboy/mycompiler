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
 *	File :		l_block_init.c
 *	Description :	Initialization routines and parameters
 *	Author :        David August, Kevin Crozier
 * 
 *	(C) Copyright 1997, David August and Kevin Crozier
 *	All rights granted to University of Illinois Board of Regents.
 *==========================================================================*/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/lb_block.h>
#include "lb_b_internal.h"

#define LB_HAZARD_UNDEFINED                 0
#define LB_HAZARD_EXCLUDE_ALL               1
#define LB_HAZARD_EXCLUDE_NON_TRACE         2
#define LB_HAZARD_EXCLUDE_HEURISTIC         3
#define LB_HAZARD_IGNORE                    4

/*
 * Lblock
 * ----------------------------------------------------------------------
 */

char *LB_block_formation_type = "hyperblock";
char *LB_predicate_formation_type = "prp";
int LB_use_block_enum_selector = 0;
int LB_make_zero_weight_regions = 0;
char *LB_prevent_block_functions = "";
Func_Name_List *LB_prevent_block_list = NULL;
int LB_verbose_prevent_block = 1;

/*
 * Hyperblock formation
 * ----------------------------------------------------------------------
 */

int    LB_hb_pred_all_paths = 0;
int    LB_hb_exclude_all_jsrs = 0;
int    LB_hb_exclude_all_pointer_stores = 0;

int    LB_hb_peel_enable = 1;
int    LB_hb_peel_heuristic = 0;
int    LB_hb_peel_partial = 1;

int      LB_hb_peel_max_ops = 35;
int      LB_hb_peel_infinity_iter = 6;
double   LB_hb_peel_min_overall_coverage = 0.75;

double   LB_hb_peel_min_peelable_coverage = 0.85;
double   LB_hb_peel_inc_peelable_coverage = 0.10;

double   LB_hb_peel_min_util = 0.80;

int    LB_hb_issue_width = 8;
double LB_hb_path_max_op_growth = 2.0;
double LB_hb_path_max_dep_growth = 3.6;
int    LB_hb_min_ratio_sens_opct = 2;
double LB_hb_path_min_exec_ratio = 0.00075;
double LB_hb_path_min_main_exec_ratio = 0.05;
double LB_hb_path_main_rel_exec_ratio = 0.10;
double LB_hb_path_min_priority_ratio = 0.10;

double LB_hb_min_cb_weight = 50.0;
double LB_hb_block_min_weight_ratio = 0.005;
double LB_hb_block_min_path_ratio = 0.015;
int    LB_hb_max_cb_in_hammock = 64;
int    LB_hb_form_simple_hammocks_only = 0;
int    LB_hb_available_predicates = 64;

int    LB_hb_branch_split = 0;
int    LB_hb_verbose_level = 0;

int    LB_hb_do_pred_merge = 1;

int    LB_hb_do_loop_collapsing = 0;

double LB_tail_dup_growth = 1.5;
double LB_max_static_tail_dup = 1.10;
double LB_max_dyn_tail_dup = 1.5;

char *LB_hb_unsafe_jsr_hazard_method_name = NULL;
char *LB_hb_safe_jsr_hazard_method_name = NULL;
char *LB_hb_pointer_st_hazard_method_name = NULL;
int LB_hb_unsafe_jsr_hazard_method = LB_HAZARD_UNDEFINED;
int LB_hb_safe_jsr_hazard_method = LB_HAZARD_UNDEFINED;
int LB_hb_pointer_st_hazard_method = LB_HAZARD_UNDEFINED;
double LB_hb_unsafe_jsr_priority_penalty = 1.0;
double LB_hb_safe_jsr_priority_penalty = 1.0;
double LB_hb_pointer_st_priority_penalty = 1.0;

int LB_hb_do_combine_exits = 1;

static void LB_hb_check_parm_values (void);
static void LB_hb_override_priority_penalties (void);
static int LB_hb_hazard_method_string_to_define (char *s);

int LB_setup_only = 0;
int LB_split_critical = 0;

/*
 * Superblock formation
 * ----------------------------------------------------------------------
 */

double LB_minimum_superblock_weight = 100.0;
double LB_maximum_code_growth = 5.0;
double LB_min_branch_ratio = 0.01;
double LB_trace_min_cb_ratio = 0.10;

int LB_do_lightweight_pred_opti = 1;

/*
 * **      Read module specific parameters
 */
void
LB_read_parm_lblock (Parm_Parse_Info * ppi)
{
  L_read_parm_s  (ppi, "block_formation_type", 
		  &LB_block_formation_type);

  L_read_parm_s  (ppi, "prevent_block_functions", 
		  &LB_prevent_block_functions);
  L_read_parm_b  (ppi, "verbose_prevent_block", 
		  &LB_verbose_prevent_block);

  L_read_parm_s  (ppi, "predicate_formation_type",
		  &LB_predicate_formation_type);
  L_read_parm_b  (ppi, "use_block_enum_selector", 
		  &LB_use_block_enum_selector);
  L_read_parm_b  (ppi, "make_zero_weight_regions",
		  &LB_make_zero_weight_regions);
  L_read_parm_lf (ppi, "minimum_superblock_weight",
		  &LB_minimum_superblock_weight);
  L_read_parm_lf (ppi, "maximum_code_growth", 
		  &LB_maximum_code_growth);
  L_read_parm_lf (ppi, "min_branch_ratio", 
		  &LB_min_branch_ratio);
  L_read_parm_lf (ppi, "trace_min_cb_ratio", 
		  &LB_trace_min_cb_ratio);
  L_read_parm_lf (ppi, "tail_dup_growth", 
		  &LB_tail_dup_growth);
  L_read_parm_lf (ppi, "max_static_tail_dup", 
		  &LB_max_static_tail_dup);
  L_read_parm_lf (ppi, "max_dyn_tail_dup", 
		  &LB_max_dyn_tail_dup);
  L_read_parm_b  (ppi, "pred_all_paths", 
		  &LB_hb_pred_all_paths);
  L_read_parm_b  (ppi, "exclude_all_jsrs", 
		  &LB_hb_exclude_all_jsrs);
  L_read_parm_b  (ppi, "exclude_all_pointer_stores",
		  &LB_hb_exclude_all_pointer_stores);

  L_read_parm_i  (ppi, "issue_width", 
		  &LB_hb_issue_width);
  L_read_parm_lf (ppi, "min_cb_weight", 
		  &LB_hb_min_cb_weight);

  L_read_parm_b  (ppi, "peel_enable", 
		  &LB_hb_peel_enable);

  L_read_parm_b  (ppi, "peel_heuristic", 
		  &LB_hb_peel_heuristic);

  L_read_parm_b  (ppi, "peel_partial", 
		  &LB_hb_peel_partial);

  L_read_parm_i  (ppi, "peel_max_ops", 
		  &LB_hb_peel_max_ops);
  L_read_parm_i  (ppi, "peel_infinity_iter", 
		  &LB_hb_peel_infinity_iter);
  L_read_parm_lf (ppi, "peel_min_overall_coverage",
		  &LB_hb_peel_min_overall_coverage);

  L_read_parm_lf (ppi, "peel_min_peelable_coverage",
		  &LB_hb_peel_min_peelable_coverage);
  L_read_parm_lf (ppi, "peel_inc_peelable_coverage",
		  &LB_hb_peel_inc_peelable_coverage);

  L_read_parm_lf (ppi, "peel_min_util",
		  &LB_hb_peel_min_util);

  L_read_parm_b  (ppi, "do_pred_merge", 
		  &LB_hb_do_pred_merge);

  L_read_parm_b  (ppi, "do_loop_collapsing", 
		  &LB_hb_do_loop_collapsing);

  L_read_parm_lf (ppi, "path_max_op_growth", 
		  &LB_hb_path_max_op_growth);
  L_read_parm_lf (ppi, "path_max_dep_growth", 
		  &LB_hb_path_max_dep_growth);
  L_read_parm_i  (ppi, "min_ratio_sens_opct",
		  &LB_hb_min_ratio_sens_opct);
  L_read_parm_lf (ppi, "path_min_exec_ratio", 
		  &LB_hb_path_min_exec_ratio);
  L_read_parm_lf (ppi, "path_min_main_exec_ratio",
		  &LB_hb_path_min_main_exec_ratio);
  L_read_parm_lf (ppi, "path_min_priority_ratio",
		  &LB_hb_path_min_priority_ratio);
  L_read_parm_lf (ppi, "path_main_rel_exec_ratio",
		  &LB_hb_path_main_rel_exec_ratio);

  L_read_parm_lf (ppi, "block_min_weight_ratio",
		  &LB_hb_block_min_weight_ratio);
  L_read_parm_lf (ppi, "block_min_path_ratio", 
		  &LB_hb_block_min_path_ratio);
  L_read_parm_i  (ppi, "max_cb_in_hammock",
		  &LB_hb_max_cb_in_hammock);
  L_read_parm_b  (ppi, "form_simple_hammocks_only",
		  &LB_hb_form_simple_hammocks_only);
  L_read_parm_i  (ppi, "available_predicates",
                  &LB_hb_available_predicates);

  L_read_parm_b  (ppi, "branch_split", 
		  &LB_hb_branch_split);

  L_read_parm_s  (ppi, "unsafe_jsr_hazard_method",
		  &LB_hb_unsafe_jsr_hazard_method_name);
  L_read_parm_s  (ppi, "safe_jsr_hazard_method",
		  &LB_hb_safe_jsr_hazard_method_name);
  L_read_parm_s  (ppi, "pointer_st_hazard_method",
		  &LB_hb_pointer_st_hazard_method_name);
  L_read_parm_lf (ppi, "unsafe_jsr_priority_penalty",
		  &LB_hb_unsafe_jsr_priority_penalty);
  L_read_parm_lf (ppi, "safe_jsr_priority_penalty",
		  &LB_hb_safe_jsr_priority_penalty);
  L_read_parm_lf (ppi, "pointer_st_priority_penalty",
		  &LB_hb_pointer_st_priority_penalty);

  L_read_parm_b (ppi, "combine_exits", &LB_hb_do_combine_exits);
  LB_hb_unsafe_jsr_hazard_method =
    LB_hb_hazard_method_string_to_define (LB_hb_unsafe_jsr_hazard_method_name);
  LB_hb_safe_jsr_hazard_method =
    LB_hb_hazard_method_string_to_define (LB_hb_safe_jsr_hazard_method_name);
  LB_hb_pointer_st_hazard_method =
    LB_hb_hazard_method_string_to_define (LB_hb_pointer_st_hazard_method_name);

  LB_hb_override_priority_penalties ();
  LB_hb_check_parm_values ();

  L_read_parm_i  (ppi, "verbose_level", 
		  &LB_hb_verbose_level);

  L_read_parm_b (ppi, "setup_only",
		 &LB_setup_only);

  L_read_parm_b (ppi, "split_critical",
		 &LB_split_critical);

  L_read_parm_b (ppi, "do_lightweight_pred_opti", &LB_do_lightweight_pred_opti);

  return;
}

void
LB_block_init (Parm_Macro_List * command_line_macro_list)
{
  /* Load the parameters specific to Lblock code generation */
  L_load_parameters (L_parm_file, L_command_line_macro_list,
		     "(Lblock", LB_read_parm_lblock);

  L_init_lmdes2 (L_lmdes_file_name, L_max_pred_operand, L_max_dest_operand,
		 L_max_src_operand, 4 /* Support up to 4 sync operands */ );

  if (lmdes->mdes2 == NULL)
    L_punt ("Lblock supports only .lmdes2 files now.\n"
	    "Cannot use '%s'\n", lmdes->file_name);

  LB_prevent_block_list = new_Func_Name_List (LB_prevent_block_functions);
}

/* Remove any existing trace attributes from the cbs */
LB_TraceRegion_Header *
LB_function_init (L_Func * fn)
{
  L_Cb *cb_ptr;
  L_Attr *attr;
  LB_TraceRegion_Header *header;

  /* Initialize for each funtion. */
  header = LB_create_tr_header (fn);

  for (cb_ptr = fn->first_cb; cb_ptr; cb_ptr = cb_ptr->next_cb)
    {
      if ((attr = L_find_attr (cb_ptr->attr, "trace")))
	cb_ptr->attr = L_delete_attr (cb_ptr->attr, attr);
    }
  return header;
}


void
LB_function_deinit (LB_TraceRegion_Header * header)
{
  LB_free_tr_header (header);
}

static void
LB_hb_check_parm_values (void)
{
  /* Check that the path ratio parms are between 0 and 1 */
  if ((LB_hb_path_min_exec_ratio < 0.0) ||
      (LB_hb_path_min_exec_ratio > 1.0))
    L_punt
      ("LB_hb_check_parm_values: illegal value for path_min_exec_ratio");
  if ((LB_hb_path_min_main_exec_ratio < 0.0)
      || (LB_hb_path_min_main_exec_ratio > 1.0))
    L_punt
      ("LB_hb_check_parm_values: illegal value for path_min_main_exec_ratio");
  if ((LB_hb_path_min_priority_ratio < 0.0)
      || (LB_hb_path_min_priority_ratio > 1.0))
    L_punt
      ("LB_hb_check_parm_values: illegal value for path_min_priority_ratio");

  /* Check that the growth parms are 1.0 or larger */
  if (LB_hb_path_max_op_growth < 1.0)
    L_punt ("LB_hb_check_parm_values: illegal value for path_max_op_growth");
  if (LB_hb_path_max_dep_growth < 1.0)
    L_punt
      ("LB_hb_check_parm_values: illegal value for path_max_dep_growth");

  /* check that the block ratio parms are between 0 and 1 */
  if ((LB_hb_block_min_weight_ratio < 0.0) ||
      (LB_hb_block_min_weight_ratio > 1.0))
    L_punt
      ("LB_hb_check_parm_values: illegal value for block_min_weight_ratio");
  if ((LB_hb_block_min_path_ratio < 0.0)
      || (LB_hb_block_min_path_ratio > 1.0))
    L_punt
      ("LB_hb_check_parm_values: illegal value for block_min_path_ratio");

  /* Check that the hazard handling methods are not UNDEFINED */
  if (LB_hb_unsafe_jsr_hazard_method == LB_HAZARD_UNDEFINED)
    L_punt
      ("LB_hb_check_parm_values: "
       "illegal value for unsafe_jsr_hazard_method");
  if (LB_hb_safe_jsr_hazard_method == LB_HAZARD_UNDEFINED)
    L_punt
      ("LB_hb_check_parm_values: "
       "illegal value for safe_jsr_hazard_method");
  if (LB_hb_pointer_st_hazard_method == LB_HAZARD_UNDEFINED)
    L_punt
      ("LB_hb_check_parm_values: "
       "illegal value for pointer_st_hazard_method");

  /* Check that the priority penalties are between 0 and 1 */
  if ((LB_hb_unsafe_jsr_priority_penalty < 0.0) ||
      (LB_hb_unsafe_jsr_priority_penalty > 1.0))
    L_punt
      ("LB_hb_check_parm_values: "
       "illegal value for unsafe_jsr_priority_penalty");
  if ((LB_hb_safe_jsr_priority_penalty < 0.0)
      || (LB_hb_safe_jsr_priority_penalty > 1.0))
    L_punt
      ("LB_hb_check_parm_values: "
       "illegal value for safe_jsr_priority_penalty");
  if ((LB_hb_pointer_st_priority_penalty < 0.0)
      || (LB_hb_pointer_st_priority_penalty > 1.0))
    L_punt
      ("LB_hb_check_parm_values: "
       "illegal value for pointer_st_priority_penalty");
}

static int
LB_hb_hazard_method_string_to_define (char *s)
{
  if (!s)
    return (LB_HAZARD_UNDEFINED);
  if ((!strcasecmp (s, "exclude_all")) || (!strcasecmp (s, "exclude-all")))
    return (LB_HAZARD_EXCLUDE_ALL);
  else if ((!strcasecmp (s, "exclude_non_trace")) ||
	   (!strcasecmp (s, "exclude-non-trace")))
    return (LB_HAZARD_EXCLUDE_NON_TRACE);
  else if ((!strcasecmp (s, "exclude_heuristic")) ||
	   (!strcasecmp (s, "exclude-heuristic")))
    return (LB_HAZARD_EXCLUDE_HEURISTIC);
  else if (!strcasecmp (s, "ignore"))
    return (LB_HAZARD_IGNORE);
  else
    return (LB_HAZARD_UNDEFINED);
}

/* The user's priority penalties are only used if the hazard method is set
   to heuristic.  Otherwise, they are hardwired to 1 or 0 for the other methods */
static void
LB_hb_override_priority_penalties (void)
{
  if (LB_hb_unsafe_jsr_hazard_method == LB_HAZARD_EXCLUDE_ALL)
    LB_hb_unsafe_jsr_priority_penalty = 0.0;
  else
    if ((LB_hb_unsafe_jsr_hazard_method == LB_HAZARD_EXCLUDE_NON_TRACE)
	|| (LB_hb_unsafe_jsr_hazard_method == LB_HAZARD_IGNORE))
    LB_hb_unsafe_jsr_priority_penalty = 1.0;

  if (LB_hb_safe_jsr_hazard_method == LB_HAZARD_EXCLUDE_ALL)
    LB_hb_safe_jsr_priority_penalty = 0.0;
  else if ((LB_hb_safe_jsr_hazard_method == LB_HAZARD_EXCLUDE_NON_TRACE)
	   || (LB_hb_safe_jsr_hazard_method == LB_HAZARD_IGNORE))
    LB_hb_safe_jsr_priority_penalty = 1.0;

  if (LB_hb_pointer_st_hazard_method == LB_HAZARD_EXCLUDE_ALL)
    LB_hb_pointer_st_priority_penalty = 0.0;
  else
    if ((LB_hb_pointer_st_hazard_method == LB_HAZARD_EXCLUDE_NON_TRACE)
	|| (LB_hb_pointer_st_hazard_method == LB_HAZARD_IGNORE))
    LB_hb_pointer_st_priority_penalty = 1.0;
}
