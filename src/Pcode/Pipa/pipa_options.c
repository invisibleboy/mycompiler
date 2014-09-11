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
 *      File:    pipa_options.c
 *      Author:  Erik Nystrom
 *      Copyright (c) 2003  Erik Nystrom, Wen-mei Hwu
 *              and The Board of Trustees of the University of Illinois.
 *              All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#include "pipa_options.h"

char *IPA_field_string[] = {[IPA_FIELD_INDEPENDENT] "fi",
			    [IPA_FIELD_DEPENDENT_VARIABLE_SIZE] "fdvs"};
int IPA_field_option = IPA_FIELD_INDEPENDENT;


char *IPA_context_string[] = {[IPA_CONTEXT_INSENSITIVE] "ci",
			      [IPA_CONTEXT_SENSITIVE] "cs"};
int IPA_context_option = IPA_CONTEXT_INSENSITIVE;

char *IPA_csrec_string[] = {[IPA_CSREC_NONE] "none",
			    [IPA_CSREC_BREAK] "break",
			    [IPA_CSREC_FULL] "full"};
int IPA_csrec_option = IPA_CSREC_NONE;

char *IPA_gcon_string[] = {[IPA_GCON_ANDERSEN] "Andersens Style",
			   [IPA_GCON_STEENSDGARD] "Steensgard Style"};
int IPA_gcon_option = IPA_GCON_ANDERSEN;

char *IPA_cloning_string[] = {[IPA_NO_HEAP_CLONING] "nc",
			      [IPA_HEAP_CLONING] "hs"};
int IPA_cloning_option = IPA_NO_HEAP_CLONING;
int IPA_cloning_gen = 10000;

char *IPA_allow_pcode_expr_string[] = {[IPA_ALLOW_PTR_AGGR] "Ptr, Aggr Exprs",
                                       [IPA_ALLOW_PTR_AGGR_INDIR] "Ptr, Aggr, Indir Exprs",
                                       [IPA_ALLOW_ALL] "All Exprs"};
int IPA_allow_pcode_expr = IPA_ALLOW_PTR_AGGR;


char *IPA_field_safety_string[] = {[IPA_SAFETY_LEVEL0] "Level 0",
                                   [IPA_SAFETY_LEVEL1] "Level 1",
                                   [IPA_SAFETY_LEVEL2] "Level 2"};
int IPA_field_safety = IPA_SAFETY_LEVEL0;

int IPA_flow_sensitive_type = IPA_FLOW_NONE;

int IPA_sync_gen = 0;
int IPA_sync_gen_testonly = 0;
int IPA_sync_append_all_obj = 0;
int IPA_sync_annotate_all_obj = 0;

int IPA_solver_limit_fscost = 1;

int IPA_allow_missing_ipa = 0;
int IPA_exclude_zero_profile = 0;
int IPA_use_actualformal_filter = 0;

int IPA_print_merged_fn = 0;
int IPA_print_initial_cng = 0;
int IPA_print_summary_cng = 0;
int IPA_print_final_cng = 0;
int IPA_print_solved_cng = 0;

int IPA_slow_callgraph = 0;

char *IPA_library_dir = NULL;
char *IPA_file_subdir = "ipa";

struct debuginfo_t debug;
int IPA_myflag = 0;

void
IPA_read_parms (Parm_Parse_Info * ppi)
{
  char *string;
 
  L_read_parm_b (ppi, "ipa_allow_missing_ipa", &IPA_allow_missing_ipa);
  
  string = NULL;
  L_read_parm_s (ppi, "?ipa_flow_sense", &string);
  if (string == NULL)
    IPA_flow_sensitive_type = IPA_FLOW_NONE;
  else if (!strcmp (string, "none"))
    IPA_flow_sensitive_type = IPA_FLOW_NONE; /* No flow-sensitivity */
  else if (!strcmp (string, "full"))
    IPA_flow_sensitive_type = IPA_FLOW_FULL; /* Full SSA renaming */
  else if (!strcmp (string, "loop"))
    IPA_flow_sensitive_type = IPA_FLOW_LOOP; /* Merge names within loops */
  else if (!strcmp (string, "phis"))
    IPA_flow_sensitive_type = IPA_FLOW_PHIS; /* Inteligent PHI merging */
  else if (!strcmp (string, "djlt"))
    IPA_flow_sensitive_type = IPA_FLOW_DJLT; /* Merge within Disj. Lifetimes */
  else
    I_punt ("Unknown ipa_flow_sensitive_type [%s]\n", string);
  free (string);
  
  L_read_parm_s (ppi, "ipa_allow_pcode_expr", &string);
  if (strcmp (string, IPA_ALLOW_PTR_AGGR_STR) == 0)
    IPA_allow_pcode_expr = IPA_ALLOW_PTR_AGGR;
  else if (strcmp (string, IPA_ALLOW_PTR_AGGR_INDIR_STR) == 0)
    IPA_allow_pcode_expr = IPA_ALLOW_PTR_AGGR_INDIR;
  else if (strcmp (string, IPA_ALLOW_ALL_STR) == 0)
    IPA_allow_pcode_expr = IPA_ALLOW_ALL;
  else
    I_punt ("Unknown ipa_allow_pcode_expr [%s]\n", string);
  string[0] = 0;

  L_read_parm_b (ppi, "ipa_exclude_zero_profile", &IPA_exclude_zero_profile);
  L_read_parm_b (ppi, "ipa_use_actualformal_filter", &IPA_use_actualformal_filter);
  
  L_read_parm_i (ppi, "ipa_field_safety", &IPA_field_safety);
  
  L_read_parm_b (ppi, "ipa_print_merged_fn", &IPA_print_merged_fn);
  L_read_parm_b (ppi, "ipa_print_initial_cng", &IPA_print_initial_cng);
  L_read_parm_b (ppi, "ipa_print_summary_cng", &IPA_print_summary_cng);
  L_read_parm_b (ppi, "ipa_print_final_cng", &IPA_print_final_cng);
  L_read_parm_b (ppi, "ipa_print_solved_cng", &IPA_print_solved_cng);

  L_read_parm_i (ppi, "ipa_myflag", &IPA_myflag);

  L_read_parm_s (ppi, "ipa_field_option", &string);
  if (strcmp (string, IPA_FIELD_INDEPENDENT_STR) == 0)
    IPA_field_option = IPA_FIELD_INDEPENDENT;
  else if (strcmp (string, IPA_FIELD_DEPENDENT_VARIABLE_SIZE_STR) == 0)
    IPA_field_option = IPA_FIELD_DEPENDENT_VARIABLE_SIZE;
  else
    I_punt ("Unknown ipa_field_option  [%s]\n", string);
  string[0] = 0;
  
  L_read_parm_s (ppi, "ipa_context_option", &string);
  if (strcmp (string, IPA_CONTEXT_INSENSITIVE_STR) == 0)
    IPA_context_option = IPA_CONTEXT_INSENSITIVE;
  else if (strcmp (string, IPA_CONTEXT_SENSITIVE_STR) == 0)
    IPA_context_option = IPA_CONTEXT_SENSITIVE;
  else
    I_punt ("Unknown ipa_context_option  [%s]\n", string);
  string[0] = 0;

  L_read_parm_s (ppi, "ipa_csrec_option", &string);
  if (strcmp (string, IPA_CSREC_NONE_STR) == 0)
    IPA_csrec_option = IPA_CSREC_NONE;
  else if (strcmp (string, IPA_CSREC_BREAK_STR) == 0)
    IPA_csrec_option = IPA_CSREC_BREAK;
  else if (strcmp (string, IPA_CSREC_FULL_STR) == 0)
    IPA_csrec_option = IPA_CSREC_FULL;
#if 0
  else
    I_punt ("Unknown ipa_csrec_option  [%s]\n", string);
#endif
  string[0] = 0;

  L_read_parm_s (ppi, "ipa_gcon_option", &string);
  if (strcmp (string, IPA_GCON_ANDERSEN_STR) == 0)
    IPA_gcon_option = IPA_GCON_ANDERSEN;
  else if (strcmp (string, IPA_GCON_STEENSDGARD_STR) == 0)
    IPA_gcon_option = IPA_GCON_STEENSDGARD;
  else
    I_punt ("Unknown ipa_gcon_option [%s]\n", string);
  string[0] = 0;

  L_read_parm_b (ppi, "ipa_sync_gen", &IPA_sync_gen);
  L_read_parm_b (ppi, "ipa_sync_gen_testonly", &IPA_sync_gen_testonly);
  L_read_parm_b (ppi, "ipa_sync_append_all_obj", &IPA_sync_append_all_obj);
  L_read_parm_b (ppi, "ipa_sync_annotate_all_obj", &IPA_sync_annotate_all_obj);

  L_read_parm_b (ppi, "ipa_solver_limit_fscost", &IPA_solver_limit_fscost);

  L_read_parm_b (ppi, "ipa_cloning_option", &IPA_cloning_option);
  L_read_parm_i (ppi, "ipa_cloning_gen", &IPA_cloning_gen);

  L_read_parm_b (ppi, "ipa_slow_callgraph", &IPA_slow_callgraph);

  L_read_parm_s (ppi, "ipa_library_dir", &string);
  if (string[0] && strcmp(string,"none"))
    {
      IPA_library_dir = C_findstr (string);
      printf("CLIB TEMPLATES [%s]\n", IPA_library_dir);
    }
  else
    IPA_library_dir = NULL;
  string[0] = 0;

  L_read_parm_s (ppi, "ipa_file_subdir", &string);
  if (string[0])
    {
      IPA_file_subdir = C_findstr (string);
      printf("IPA FILE SUBDIR [%s]\n", IPA_file_subdir);
    }
  else
    {
      if (IPA_gcon_option != IPA_GCON_ANDERSEN)
	IPA_file_subdir = "ipa_s";
      else
	IPA_file_subdir = "ipa";
    }
}
