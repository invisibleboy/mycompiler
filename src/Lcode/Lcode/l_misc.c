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
/*****************************************************************************\
 *      File :          l_misc.c
 *      Description :   I/O, Parameter reading, Allocation setup
 *      Authors: Scott Mahlke, Pohua Chang, Wen-mei Hwu
\*****************************************************************************/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>

#include <Lcode/l_event.h>
#include <Lcode/l_region.h>
#include <Lcode/l_debug.h>
#include "l_binaryio.h"

/*=======================================================================*\
 *
 *      File name parameters read in from STD_PARMS     
 *
\*=======================================================================*/

char *L_input_file = NULL;
char *L_output_file = NULL;
char *L_filelist_file = NULL;
char *L_parm_file = NULL;
char *L_lmdes_file_name = NULL;
Parm_Macro_List *L_command_line_macro_list = NULL;

FILE *L_IN = NULL;
FILE *L_OUT = NULL;
FILE *L_ERR = NULL;
FILE *L_FILELIST = NULL;

/* Creating new function files */
int L_file_index = 0;
char *L_file_directory = ".";
char *L_output_file_extension = "out";
char *L_input_file_extension = "lc";

/* List of files for processing model of filelist or extension */
List L_filelist;
L_File *L_file;

/* File processing model controls */
char *L_file_processing_model_name = "file";
int L_file_processing_model = L_FILE_MODEL_FILE;


/*=======================================================================*/
/*
 *      Global vars read in from appropriate file specified in
 *      file name section of STD_PARMS
 */
/*=======================================================================*/

int L_print_parent_op = 0;      /* SAM 7-96: leave default 0 */

char *L_arch = NULL;
char *L_model = NULL;
char *L_swarch = NULL;

int L_output_generation_info = 1;
int L_funtime_message = 1;
int L_nice_value = 10;          /* For auto renicing impact jobs */

int L_determine_cpu_time = 0;
double L_cpu_time_print_threshold = -1;

/* JLB - 2/27/92 */
int L_output_obsolete_ctype_format = 0;

/* REH - 4/11/96 */
int L_output_binary_format = 0;
int L_input_binary_format = 0;  /* This is not a parameter.  The value is */
                                /* determined by the first character      */
                                /* of the input file.                     */

/* allow callgraph to be output in daVinci format */
int L_print_davinci_callgraph = 0;
char *L_davinci_callgraph_file = "da_callgraph";

/* Lcode structure configuration parameters */
int L_max_dest_operand = 1;
int L_max_src_operand = 3;
int L_max_pred_operand = 0;
int L_oper_hash_tbl_size = 1;
int L_expression_hash_tbl_size = 1;
int L_cb_hash_tbl_size = 1;
int L_region_hash_tbl_size = 1;

/* Lcode predicate define models */

int L_max_pred_masks = 2;
int L_max_preds_per_mask = 1;
int L_max_pred_srcs = 1;

/* Lcode fs branch model */
int L_min_fs_weight = 50;

/* Lcode exception model */
int L_non_excepting_ops = 0;
int L_mask_potential_exceptions = 1;
int L_spec_model = GENERAL;
char *L_speculation_model = NULL;

/* Lcode structure checking parameters */
int L_check = 0;
int L_verbose_check = 0;

/* memory disambiguation parameters */
int L_use_attr_label = 0;
int L_use_loop_iter = 0;
int L_new_memory_disamb = 0;
int L_incoming_param_indep = 0;
int L_diff_data_types_indep = 0;
int L_label_and_reg_access_indep = 0;
int L_sp_and_reg_access_indep = 0;
int L_ambig_mem_always_indep = 0;
int L_load_store_always_indep = 0;
int L_mem_never_indep = 0;
int L_debug_memory_disamb = 0;
int L_use_sync_arcs = 0;
int L_debug_sync_arcs = 0;
int L_eliminate_sync_arcs = 0;
int L_punt_on_sync_arcs_failure = 0;
int L_ignore_sync_arcs_for_opti = 0;

int L_ignore_acc_specs = 0;


/* loop detection parameters */
int L_static_loop_iter_count = 0;
int L_debug_loop = 0;
int L_debug_inner_loop = 0;
int L_debug_static_weight = 0;

/* hash table repair parameters */
int L_debug_hash_table_repair = 0;
int L_region_hash_table_management = 0;

/* control flow function parameters */
int L_debug_color_cb = 0;

/* Lcode structure allocation parameters */
int L_check_data_struct_alloc = 1;
int L_debug_data_struct_alloc = 0;

/* Lcode Function Appendix parameters */
int L_use_appendix_for_sync_arcs = 0;
int L_use_appendix_for_attr = 0;
int L_oper_default_to_appendix = 0;
char *L_oper_appendix_attr_list = "";
char *L_oper_code_attr_list = "";
int L_cb_default_to_appendix = 0;
char *L_cb_appendix_attr_list = "";
char *L_cb_code_attr_list = "";

/* Dataflow analysis parameters */
int L_debug_df_cb_construction = 0;
int L_debug_df_live_in_out = 0;
int L_debug_df_reaching_defs = 0;
int L_debug_df_mem_reaching_defs = 0;
int L_debug_df_available_defs = 0;
int L_debug_df_mem_available_defs = 0;
int L_debug_df_dead_code = 0;
int L_df_max_pred_paths = -1;
int L_df_use_max_graph_builder = 0;

/* Predicate define combining */
int L_combine_only_same_class_preds = 0;

/* JLB 04-27-97 ctype propagation */
int L_propagate_sign_size_ctype_info = 0;

void *L_data_segment_start;

/* Intrinsic database filename -- initialize with a big, fat NULL
 * -ITI/JWJ 7.6.1999
 */
char *L_intrinsic_database_filename = NULL;
/* Enable intrinsic support?  -ITI/JWJ 7.6.1999 */
int L_intrinsic_support_enabled = 0;

/* IA64 sias 20000517 */
int L_ia64 = 0;

int L_print_int_ranges = 0;

int L_generate_spec_checks = 0;

int L_versioned_acc_specs = 1;

int L_dump_core_on_punt = 0;

/*=======================================================================*/
/*
 *      Error handling rountines - these are now in a separate file
 *      called l_error.c so people can redefine these routines.
 *      I don't really understand the reason for this but I guess it
 *      makes sense to those who do it :).. SAM 
 */
/*=======================================================================*/

char *L_curr_pass_name = NULL;

void
L_insert_generic_info_to_output_file (FILE * F)
{
  char time_buf[128], *user_name, host_buf[128];
  time_t start_time;

  if (F == NULL)
    L_punt ("L_insert_header_comments: file is NULL");

  start_time = time (NULL);
  if (start_time != -1)
    {
      strftime (time_buf, sizeof (time_buf),
                "%H:%M:%S %p, %A %B %d", localtime (&start_time));
    }
  else
    {
      sprintf (time_buf, "time unavailable");
    }

#ifndef __WIN32__
  user_name = (char *) getlogin ();

  if (gethostname (host_buf, sizeof (host_buf)) == -1)
    {
      sprintf (host_buf, "Unknown");
    }
#else
/* ADA 5/29/96: Win95/NT has diffenent way to get hostname and login, but
                we just hard-code it here */
  user_name = "Win32 User";
  strcpy (host_buf, "Microsoft Win32");
#endif

  fprintf (F, "#\n");
  fprintf (F, "#  IMPACT C Compiler, "
           "University of Illinois at Urbana-Champaign\n");
  fprintf (F, "#\n");
  fprintf (F, "#  Lcode file generation information\n");
  fprintf (F, "#      Generation time : %s\n", time_buf);
  if (user_name != NULL)
    fprintf (F, "#      User name       : %s\n", user_name);
  else
    fprintf (F, "#      User name       : Unknown\n");
  fprintf (F, "#      Host name       : %s\n", host_buf);
  fprintf (F, "#      Input file      : %s\n", L_input_file);
  fprintf (F, "#      Output file     : %s\n", L_output_file);
  fprintf (F, "#      Lcode module    : %s\n", L_curr_pass_name);
  fprintf (F, "#      Target arch     : %s / %s / %s\n",
	   L_arch, L_model, L_swarch);
  fprintf (F, "#      Parameter file  : %s\n", L_parm_file);
  fprintf (F, "#      Mdes file       : %s\n", L_lmdes_file_name);
  fprintf (F, "#\n\n");
}

/*=======================================================================*/
/*
 *      Parsing parameter file(s) for file and global settings
 */
/*=======================================================================*/

void
L_read_parm_arch (Parm_Parse_Info * ppi)
{
  L_read_parm_s (ppi, "arch", &L_arch);
  L_read_parm_s (ppi, "model", &L_model);
  L_read_parm_s (ppi, "swarch", &L_swarch);
  L_read_parm_s (ppi, "lmdes", &L_lmdes_file_name);

  M_read_parm (ppi);
}

void
L_read_parm_file (Parm_Parse_Info * ppi)
{
  L_read_parm_s (ppi, "input_file_name", &L_input_file);
  L_read_parm_s (ppi, "output_file_name", &L_output_file);
  L_read_parm_s (ppi, "filelist_file_name", &L_filelist_file);

  L_read_parm_s (ppi, "file_directory", &L_file_directory);
  L_read_parm_s (ppi, "input_file_extension", &L_input_file_extension);
  L_read_parm_s (ppi, "output_file_extension", &L_output_file_extension);
  L_read_parm_i (ppi, "file_start_index", &L_file_index);

  /* ITI/JWJ 7.6.1999  Intrinsic information */
  L_read_parm_s (ppi, "intrinsic_database_filename",
                 &L_intrinsic_database_filename);
  L_read_parm_b (ppi, "intrinsic_support_enabled",
                 &L_intrinsic_support_enabled);

  L_read_parm_s (ppi, "file_processing_model", &L_file_processing_model_name);
  if (!strcmp (L_file_processing_model_name, "file"))
    {
      L_file_processing_model = L_FILE_MODEL_FILE;
    }
  else if (!strcmp (L_file_processing_model_name, "filelist"))
    {
      L_file_processing_model = L_FILE_MODEL_LIST;
    }
  else if (!strcmp (L_file_processing_model_name, "extension"))
    {
      L_file_processing_model = L_FILE_MODEL_EXTENSION;
    }
  else
    {
      L_punt ("L_read_parm_file : invalid file processing model (%s)",
              L_file_processing_model_name);
    }

}

void
L_read_parm_global (Parm_Parse_Info * ppi)
{
  /* Parameter file configuration */
  L_read_parm_b (ppi, "?warn_old_parm_section_name_used",
                 &L_warn_old_parm_section_name_used);
  L_read_parm_b (ppi, "warn_parm_not_defined", &L_warn_parm_not_defined);
  L_read_parm_b (ppi, "?warn_dev_parm_not_defined",
                 &L_warn_dev_parm_not_defined);
  L_read_parm_b (ppi, "warn_parm_defined_twice", &L_warn_parm_defined_twice);
  L_read_parm_b (ppi, "warn_parm_not_used", &L_warn_parm_not_used);
  L_read_parm_b (ppi, "dump_parms", &L_dump_parms);
  L_read_parm_s (ppi, "parm_warn_file_name", &L_parm_warn_file_name);
  L_read_parm_s (ppi, "parm_dump_file_name", &L_parm_dump_file_name);

  /* REH 4/11/96 - Lcode output format */
  L_read_parm_b (ppi, "output_binary_format", &L_output_binary_format);
  /* JLB 2/28/97 */
  L_read_parm_b (ppi, "output_obsolete_ctype_format",
                 &L_output_obsolete_ctype_format);

  /* daVinci callgraph output parameters */
  L_read_parm_b (ppi, "print_davinci_callgraph", &L_print_davinci_callgraph);
  L_read_parm_s (ppi, "davinci_callgraph_file", &L_davinci_callgraph_file);

  /* Lcode structure configuration parameters */
  L_read_parm_i (ppi, "max_dest_operand", &L_max_dest_operand);
  L_read_parm_i (ppi, "max_src_operand", &L_max_src_operand);
  L_read_parm_i (ppi, "max_pred_operand", &L_max_pred_operand);
  L_read_parm_i (ppi, "oper_hash_tbl_size", &L_oper_hash_tbl_size);
  L_read_parm_i (ppi, "expression_hash_tbl_size", &L_expression_hash_tbl_size);
  L_read_parm_i (ppi, "cb_hash_tbl_size", &L_cb_hash_tbl_size);
  /* REH 4/17/95 - Region support */
  L_read_parm_i (ppi, "region_hash_tbl_size", &L_region_hash_tbl_size);

  /* JWS 9/7/98 - Predicate define structure */
  L_read_parm_i (ppi, "max_pred_masks", &L_max_pred_masks);
  L_read_parm_i (ppi, "max_preds_per_mask", &L_max_preds_per_mask);
  L_read_parm_i (ppi, "max_pred_srcs", &L_max_pred_srcs);

  /* Lcode fs branch model parameters */
  L_read_parm_i (ppi, "min_fs_weight", &L_min_fs_weight);

  L_read_parm_b (ppi, "funtime_message", &L_funtime_message);
  L_read_parm_i (ppi, "nice_value", &L_nice_value);
  L_read_parm_b (ppi, "output_generation_info", &L_output_generation_info);

  L_read_parm_b (ppi, "?determine_cpu_time", &L_determine_cpu_time);
  L_read_parm_lf (ppi, "cpu_time_print_threshold",
                  &L_cpu_time_print_threshold);

  /* Lcode exception model parameters */
  L_read_parm_b (ppi, "non_excepting_ops", &L_non_excepting_ops);
  L_read_parm_b (ppi, "mask_potential_exceptions",
                 &L_mask_potential_exceptions);
  L_read_parm_s (ppi, "speculation_model", &L_speculation_model);
  if (!L_speculation_model)
    {
      L_speculation_model = strdup ("general");
    }

  /* Lcode structure checking parameters */
  L_read_parm_b (ppi, "check", &L_check);
  L_read_parm_b (ppi, "verbose_check", &L_verbose_check);

  /* L_alloc library routines parameter */
  L_read_parm_b (ppi, "bypass_alloc_routines", &bypass_alloc_routines);

  /* memory disambiguation inputs */
  L_read_parm_b (ppi, "use_attr_label", &L_use_attr_label);
  L_read_parm_b (ppi, "use_loop_iter", &L_use_loop_iter);
  L_read_parm_b (ppi, "incoming_param_indep", &L_incoming_param_indep);
  L_read_parm_b (ppi, "diff_data_types_indep", &L_diff_data_types_indep);
  L_read_parm_b (ppi, "label_and_reg_access_indep",
                 &L_label_and_reg_access_indep);
  L_read_parm_b (ppi, "sp_and_reg_access_indep", &L_sp_and_reg_access_indep);
  L_read_parm_b (ppi, "ambig_mem_always_indep", &L_ambig_mem_always_indep);
  L_read_parm_b (ppi, "load_store_always_indep", &L_load_store_always_indep);
  L_read_parm_b (ppi, "mem_never_indep", &L_mem_never_indep);
  L_read_parm_b (ppi, "debug_memory_disamb", &L_debug_memory_disamb);
  L_read_parm_b (ppi, "use_sync_arcs", &L_use_sync_arcs);
  L_read_parm_b (ppi, "debug_sync_arcs", &L_debug_sync_arcs);
  L_read_parm_b (ppi, "eliminate_sync_arcs", &L_eliminate_sync_arcs);
  L_read_parm_b (ppi, "punt_on_sync_arcs_failure",
                 &L_punt_on_sync_arcs_failure);
  L_read_parm_b (ppi, "ignore_sync_arcs_for_opti",
                 &L_ignore_sync_arcs_for_opti);

  L_read_parm_b (ppi, "?ignore_acc_specs", &L_ignore_acc_specs);

  /* loop detection parameters */
  L_read_parm_i (ppi, "static_loop_iter_count", &L_static_loop_iter_count);
  L_read_parm_b (ppi, "debug_loop", &L_debug_loop);
  L_read_parm_b (ppi, "debug_inner_loop", &L_debug_inner_loop);
  L_read_parm_b (ppi, "debug_static_weight", &L_debug_static_weight);

  /* hash table repair parameters */
  L_read_parm_b (ppi, "debug_hash_table_repair", &L_debug_hash_table_repair);
  L_read_parm_b (ppi, "region_hash_table_management",
                 &L_region_hash_table_management);

  /* control flow function parameters */
  L_read_parm_b (ppi, "debug_color_cb", &L_debug_color_cb);

  /* Lcode structure allocation parameters */
  L_read_parm_b (ppi, "check_data_struct_alloc", &L_check_data_struct_alloc);
  L_read_parm_b (ppi, "debug_data_struct_alloc", &L_debug_data_struct_alloc);

  /* Lcode Function Appendix parameters */
  L_read_parm_b (ppi, "use_appendix_for_sync_arcs",
                 &L_use_appendix_for_sync_arcs);
  L_read_parm_b (ppi, "use_appendix_for_attr", &L_use_appendix_for_attr);
  L_read_parm_b (ppi, "oper_default_to_appendix",
                 &L_oper_default_to_appendix);
  L_read_parm_s (ppi, "oper_appendix_attr_list", &L_oper_appendix_attr_list);
  L_read_parm_s (ppi, "oper_code_attr_list", &L_oper_code_attr_list);
  L_read_parm_b (ppi, "cb_default_to_appendix", &L_cb_default_to_appendix);
  L_read_parm_s (ppi, "cb_appendix_attr_list", &L_cb_appendix_attr_list);
  L_read_parm_s (ppi, "cb_code_attr_list", &L_cb_code_attr_list);

  /* Lcode dataflow analysis parameters */
  L_read_parm_b (ppi, "debug_df_cb_construction",
                 &L_debug_df_cb_construction);
  L_read_parm_b (ppi, "debug_df_live_in_out", &L_debug_df_live_in_out);
  L_read_parm_b (ppi, "debug_df_reaching_defs", &L_debug_df_reaching_defs);
  L_read_parm_b (ppi, "debug_df_available_defs", &L_debug_df_available_defs);
  L_read_parm_i (ppi, "debug_df_mem_reaching_defs",
                 &L_debug_df_mem_reaching_defs);
  L_read_parm_i (ppi, "debug_df_mem_available_defs",
                 &L_debug_df_mem_available_defs);
  L_read_parm_i (ppi, "df_max_pred_paths", &L_df_max_pred_paths);
  L_read_parm_i (ppi, "df_use_max_graph_builder",
                 &L_df_use_max_graph_builder);

  L_read_parm_b (ppi, "debug_df_dead_code", &L_debug_df_dead_code);

  /* JLB 04-27-97 ctype sign and size propagation flag */
  L_read_parm_b (ppi, "propagate_sign_size_ctype_info",
                 &L_propagate_sign_size_ctype_info);
  L_read_parm_b (ppi, "?combine_only_same_class_preds",
                 &L_combine_only_same_class_preds);

  L_read_parm_b (ppi, "ia64_extensions", &L_ia64);

  L_read_parm_b (ppi, "?print_int_ranges", &L_print_int_ranges);

  L_read_parm_b (ppi, "?versioned_acc_specs", &L_versioned_acc_specs);

  L_read_parm_b (ppi, "generate_spec_checks", &L_generate_spec_checks);

  L_read_parm_b (ppi, "dump_core_on_punt", &L_dump_core_on_punt);
}

/*=======================================================================*/
/*
 *      Data structure allocation pools
 */
/*=======================================================================*/

L_Alloc_Pool *L_alloc_flow = NULL;
L_Alloc_Pool *L_alloc_operand = NULL;
L_Alloc_Pool *L_alloc_attr = NULL;
L_Alloc_Pool *L_alloc_sync = NULL;
L_Alloc_Pool *L_alloc_sync_info = NULL;
L_Alloc_Pool *L_alloc_oper = NULL;
int L_oper_size = 0;
int L_expression_size = 0;
L_Alloc_Pool *L_alloc_cb = NULL;
L_Alloc_Pool *L_alloc_expression = NULL;
L_Alloc_Pool *L_alloc_oper_hash_entry = NULL;
L_Alloc_Pool *L_alloc_oper_hash_tbl = NULL;
L_Alloc_Pool *L_alloc_cb_hash_entry = NULL;
L_Alloc_Pool *L_alloc_cb_hash_tbl = NULL;
L_Alloc_Pool *L_alloc_expression_hash_entry = NULL;
L_Alloc_Pool *L_alloc_func = NULL;
L_Alloc_Pool *L_alloc_program = NULL;
L_Alloc_Pool *L_alloc_expr = NULL;
L_Alloc_Pool *L_alloc_data = NULL;
L_Alloc_Pool *L_alloc_datalist_element = NULL;
L_Alloc_Pool *L_alloc_datalist = NULL;
L_Alloc_Pool *L_alloc_loop = NULL;
L_Alloc_Pool *L_alloc_inner_loop = NULL;
L_Alloc_Pool *L_alloc_ind_info = NULL;
L_Alloc_Pool *L_alloc_oper_list = NULL;
L_Alloc_Pool *L_alloc_l_cg_arc = NULL;
L_Alloc_Pool *L_alloc_l_cg_node = NULL;
L_Alloc_Pool *L_alloc_cg_dfs_info = NULL;

/* REH 4/17/95 - adding eventlist and region support */
L_Alloc_Pool *L_alloc_region = NULL;
L_Alloc_Pool *L_alloc_region_member = NULL;
L_Alloc_Pool *L_alloc_region_boundary = NULL;
L_Alloc_Pool *L_alloc_region_regmap = NULL;
L_Alloc_Pool *L_alloc_region_regcon = NULL;
L_Alloc_Pool *L_alloc_region_hash_entry = NULL;
L_Alloc_Pool *L_alloc_region_hash_tbl = NULL;
L_Alloc_Pool *L_alloc_event = NULL;
L_Alloc_Pool *L_alloc_event_map = NULL;

/*
 * LCW - adding structures for preserving debugging information -- 9/14/95 
 */
L_Alloc_Pool *L_alloc_type = NULL;
L_Alloc_Pool *L_alloc_dcltr = NULL;
L_Alloc_Pool *L_alloc_struct_dcl = NULL;
L_Alloc_Pool *L_alloc_union_dcl = NULL;
L_Alloc_Pool *L_alloc_field = NULL;
L_Alloc_Pool *L_alloc_enum_dcl = NULL;
L_Alloc_Pool *L_alloc_enum_field = NULL;
/* LCW - structures for local variables - 4/15/97 */
L_Alloc_Pool *L_alloc_local_var = NULL;
L_Alloc_Pool *L_alloc_file = NULL;

L_Alloc_Pool *L_alloc_acc_spec = NULL;

void
L_setup_alloc_pools ()
{
  L_alloc_flow = L_create_alloc_pool ("L_Flow", sizeof (struct L_Flow), 256);

  L_alloc_operand = L_create_alloc_pool ("L_Operand",
                                         sizeof (struct L_Operand), 2048);

  L_alloc_attr = L_create_alloc_pool ("L_Attr", sizeof (struct L_Attr), 512);

  L_alloc_sync = L_create_alloc_pool ("L_Sync", sizeof (struct L_Sync), 512);

  L_alloc_sync_info = L_create_alloc_pool ("L_Sync_Info",
                                           sizeof (struct L_Sync_Info), 128);

  L_alloc_acc_spec = L_create_alloc_pool ("L_AccSpec",
					  sizeof (struct _L_AccSpec), 128);


  L_oper_size = sizeof (struct L_Oper) + (sizeof (L_Operand *) *
					  (L_max_dest_operand + 
					   L_max_src_operand +
					   L_max_pred_operand));

  L_expression_size = sizeof (struct L_Expression) + (sizeof(L_Operand *) *
                                                      (L_max_src_operand +
						       L_max_dest_operand));

  L_alloc_oper = L_create_alloc_pool ("L_Oper", L_oper_size, 1024);

  L_alloc_cb = L_create_alloc_pool ("L_Cb", sizeof (struct L_Cb), 64);

  L_alloc_expression = L_create_alloc_pool ("L_Expression", 
                                            L_expression_size, 512);

  L_alloc_oper_hash_entry = L_create_alloc_pool ("L_Oper_Hash_Entry",
                                                 sizeof (struct
                                                         L_Oper_Hash_Entry),
                                                 256);

  L_alloc_oper_hash_tbl = L_create_alloc_pool ("L_Oper_Hash_Tbl",
                                               (sizeof
                                                (struct L_Oper_Hash_Entry *) *
                                                (L_oper_hash_tbl_size)), 1);

  L_alloc_cb_hash_entry = L_create_alloc_pool ("L_Cb_Hash_Entry",
                                               sizeof (struct
                                                       L_Cb_Hash_Entry), 64);

  L_alloc_cb_hash_tbl = L_create_alloc_pool ("L_Cb_Hash_Tbl",
                                             (sizeof
                                              (struct L_Cb_Hash_Entry *) *
                                              (L_cb_hash_tbl_size)), 1);

  L_alloc_expression_hash_entry = L_create_alloc_pool ("L_Expression_Hash_Entry",
					(sizeof (struct L_Expression_Hash_Entry)),
					 512);

  L_alloc_func = L_create_alloc_pool ("L_Func", sizeof (struct L_Func), 16);

  L_alloc_program = L_create_alloc_pool ("L_Program",
                                         sizeof (struct L_Program), 1);

  L_alloc_expr = L_create_alloc_pool ("L_Expr", sizeof (struct L_Expr), 16);

  L_alloc_data = L_create_alloc_pool ("L_Data", sizeof (struct L_Data), 1);

  L_alloc_datalist_element = L_create_alloc_pool ("L_Datalist_Element",
                                                  sizeof (struct
                                                          L_Datalist_Element),
                                                  16);

  L_alloc_datalist = L_create_alloc_pool ("L_Datalist",
                                          sizeof (struct L_Datalist), 1);

  L_alloc_loop = L_create_alloc_pool ("L_Loop", sizeof (struct L_Loop), 16);

  L_alloc_inner_loop = L_create_alloc_pool ("L_Inner_Loop",
                                            sizeof (struct L_Inner_Loop), 8);

  L_alloc_ind_info = L_create_alloc_pool ("L_Ind_Info",
                                          sizeof (struct L_Ind_Info), 16);

  L_alloc_oper_list = L_create_alloc_pool ("L_Oper_List",
                                           sizeof (struct L_Oper_List), 64);

  L_alloc_l_cg_arc = L_create_alloc_pool ("L_CG_Arc",
                                          sizeof (struct L_CG_Arc), 512);

  L_alloc_l_cg_node = L_create_alloc_pool ("L_CG_Node",
                                           sizeof (struct L_CG_Node), 128);

  L_alloc_cg_dfs_info = L_create_alloc_pool ("L_DFS_Info",
                                             sizeof (struct L_DFS_Info), 32);

  L_alloc_region = L_create_alloc_pool ("L_Region",
                                        sizeof (struct L_Region), 32);

  L_alloc_region_member = L_create_alloc_pool ("L_alloc_region_member",
                                               sizeof (struct
                                                       L_Region_Member), 128);

  L_alloc_region_boundary = L_create_alloc_pool ("L_alloc_region_boundary",
                                                 sizeof (struct
                                                         L_Region_Boundary),
                                                 32);

  L_alloc_region_regmap = L_create_alloc_pool ("L_alloc_region_regmap",
                                               sizeof (struct
                                                       L_Region_Regmap), 128);

  L_alloc_region_regcon = L_create_alloc_pool ("L_alloc_region_regcon",
                                               sizeof (struct
                                                       L_Region_Regcon), 256);

  L_alloc_region_hash_entry =
    L_create_alloc_pool ("L_Region_Hash_Entry",
                         sizeof (struct L_Region_Hash_Entry), 64);

  L_alloc_region_hash_tbl =
    L_create_alloc_pool ("L_Region_Hash_Tbl",
                         (sizeof (struct L_Region_Hash_Entry *) *
                          (L_region_hash_tbl_size)), 1);

  L_alloc_event = L_create_alloc_pool ("L_Event",
                                       sizeof (struct L_Event), 512);

  L_alloc_event_map = L_create_alloc_pool ("L_Event_Map",
                                           sizeof (struct L_Event_Map), 32);

  /*
   * LCW - allocate pools for data structures for debugging info. - 9/14/95
   */
  L_alloc_type = L_create_alloc_pool ("L_Type", sizeof (struct L_Type), 64);

  L_alloc_dcltr = L_create_alloc_pool ("L_Dcltr",
                                       sizeof (struct L_Dcltr), 64);

  L_alloc_struct_dcl = L_create_alloc_pool ("L_Struct_Dcl",
                                            sizeof (struct L_Struct_Dcl), 64);

  L_alloc_union_dcl = L_create_alloc_pool ("L_Union_Dcl",
                                           sizeof (struct L_Union_Dcl), 64);

  L_alloc_field = L_create_alloc_pool ("L_Field",
                                       sizeof (struct L_Field), 64);

  L_alloc_enum_dcl = L_create_alloc_pool ("L_Enum_Dcl",
                                          sizeof (struct L_Enum_Dcl), 64);

  L_alloc_enum_field = L_create_alloc_pool ("L_Enum_Field",
                                            sizeof (struct L_Enum_Field), 64);

  /* LCW - allocate pools for local variable structure - 4/15/97 */
  L_alloc_local_var = L_create_alloc_pool ("L_Local_Var",
                                           sizeof (struct L_Local_Var), 64);

  L_alloc_file = L_create_alloc_pool ("L_Input_File",
                                      sizeof (struct L_File), 64);

  /* BCC - allocate Lptr pool - 2/1/99 */
  Init_NewLptr ();
  Init_NewLint ();
}

void
L_check_alloc_for_func ()
{
  L_print_alloc_info (stderr, L_alloc_flow, L_debug_data_struct_alloc);
  L_print_alloc_info (stderr, L_alloc_operand, L_debug_data_struct_alloc);
  L_print_alloc_info (stderr, L_alloc_attr, L_debug_data_struct_alloc);
  L_print_alloc_info (stderr, L_alloc_sync, L_debug_data_struct_alloc);
  L_print_alloc_info (stderr, L_alloc_sync_info, L_debug_data_struct_alloc);
  L_print_alloc_info (stderr, L_alloc_acc_spec, L_debug_data_struct_alloc);
  L_print_alloc_info (stderr, L_alloc_oper, L_debug_data_struct_alloc);
  L_print_alloc_info (stderr, L_alloc_expression, L_debug_data_struct_alloc);
  L_print_alloc_info (stderr, L_alloc_cb, L_debug_data_struct_alloc);
  L_print_alloc_info (stderr, L_alloc_oper_hash_entry,
                      L_debug_data_struct_alloc);
  L_print_alloc_info (stderr, L_alloc_oper_hash_tbl,
                      L_debug_data_struct_alloc);
  L_print_alloc_info (stderr, L_alloc_expression_hash_entry,
                      L_debug_data_struct_alloc);
  L_print_alloc_info (stderr, L_alloc_cb_hash_entry,
                      L_debug_data_struct_alloc);
  L_print_alloc_info (stderr, L_alloc_cb_hash_tbl, L_debug_data_struct_alloc);
  L_print_alloc_info (stderr, L_alloc_loop, L_debug_data_struct_alloc);
  L_print_alloc_info (stderr, L_alloc_inner_loop, L_debug_data_struct_alloc);
  L_print_alloc_info (stderr, L_alloc_ind_info, L_debug_data_struct_alloc);
  L_print_alloc_info (stderr, L_alloc_func, L_debug_data_struct_alloc);
  L_print_alloc_info (stderr, L_alloc_program, L_debug_data_struct_alloc);
  L_print_alloc_info (stderr, L_alloc_oper_list, L_debug_data_struct_alloc);
  L_print_alloc_info (stderr, L_alloc_l_cg_arc, L_debug_data_struct_alloc);
  L_print_alloc_info (stderr, L_alloc_l_cg_node, L_debug_data_struct_alloc);
  L_print_alloc_info (stderr, L_alloc_cg_dfs_info, L_debug_data_struct_alloc);
}

void
L_check_alloc_for_data ()
{
  L_print_alloc_info (stderr, L_alloc_expr, L_debug_data_struct_alloc);
  L_print_alloc_info (stderr, L_alloc_data, L_debug_data_struct_alloc);
  L_print_alloc_info (stderr, L_alloc_datalist_element,
                      L_debug_data_struct_alloc);
  L_print_alloc_info (stderr, L_alloc_datalist, L_debug_data_struct_alloc);
}

void
L_check_alloc_for_region ()
{
  L_print_alloc_info (stderr, L_alloc_region, L_debug_data_struct_alloc);
  L_print_alloc_info (stderr, L_alloc_region_member,
                      L_debug_data_struct_alloc);
  L_print_alloc_info (stderr, L_alloc_region_boundary,
                      L_debug_data_struct_alloc);
  L_print_alloc_info (stderr, L_alloc_region_regmap,
                      L_debug_data_struct_alloc);
  L_print_alloc_info (stderr, L_alloc_region_regcon,
                      L_debug_data_struct_alloc);
  L_print_alloc_info (stderr, L_alloc_region_hash_tbl,
                      L_debug_data_struct_alloc);
  L_print_alloc_info (stderr, L_alloc_region_hash_entry,
                      L_debug_data_struct_alloc);
  L_print_alloc_info (stderr, L_alloc_event, L_debug_data_struct_alloc);
  L_print_alloc_info (stderr, L_alloc_event_map, L_debug_data_struct_alloc);
}

void *
Lcode_malloc (size_t size)
{
  void *ptr;

  if (size == 0)
    L_punt ("Lcode_malloc: illegal size");

  ptr = malloc (size);
  if (ptr == NULL)
    L_punt ("L_malloc: malloc out of space");

  return (ptr);

}

void *
Lcode_calloc (size_t nelem, size_t elsize)
{
  void *ptr;

  if ((nelem == 0) || (elsize == 0))
    L_punt ("Lcode_calloc: illegal number of elements or size");

  ptr = calloc (nelem, elsize);
  if (ptr == NULL)
    L_punt ("L_calloc: calloc out of space");

  return (ptr);
}

void
Lcode_free (void *ptr)
{
  if (ptr == NULL)
    L_punt ("Lcode_free: trying to free NULL ptr");

  free (ptr);

  ptr = NULL;
}

void
L_misc_init ()
{
  /* Initialize the integer equivalent to the speculation model */
  if (!strcmp (L_speculation_model, "general"))
    {
      L_spec_model = GENERAL;
    }
  else if (!strcmp (L_speculation_model, "srb") ||
           !strcmp (L_speculation_model, "sentinel_with_recovery_blocks"))
    {
      L_spec_model = SRB;
    }
  else if (!strcmp (L_speculation_model, "sentinel"))
    {
      L_spec_model = SENTINEL;
    }
  else if (!strcmp (L_speculation_model, "restricted"))
    {
      L_spec_model = RESTRICTED;
    }
  else if (!strcmp (L_speculation_model, "basic_block"))
    {
      L_spec_model = BASIC_BLOCK;
    }
  else if (!strcmp (L_speculation_model, "wbs") ||
           !strcmp (L_speculation_model, "writeback_suppression"))
    {
      L_spec_model = WRITEBACK_SUPPRESSION;
    }
  else if (!strcmp (L_speculation_model, "mcb"))
    {
      L_spec_model = MCB;
    }
  else if (!strcmp (L_speculation_model, "alat"))
    {
      L_spec_model = ALAT;
    }
  else if (!strcmp (L_speculation_model, "boosting"))
    {
      L_spec_model = BOOSTING;
    }
  else
    L_punt ("ERROR: invalid speculation_model specified!");
}
