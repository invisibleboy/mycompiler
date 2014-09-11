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
 *      File:   l_main.h
 *      Author: Po-hua Chang, Wen-mei Hwu
 *      Creation Date:  June 1990
 *      Modified:
 *          Roger A. Bringmann - 2/12/93
 *          Added support for new parameter file format
 *
\*****************************************************************************/
#ifndef L_MAIN_H
#define L_MAIN_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_extern.h>

#include <library/l_alloc_new.h>
#include <library/i_global.h>
#include <library/i_hash.h>
#include <library/i_list.h>
#include <library/i_graph.h>

#include <Lcode/l_opc.h>
#include <Lcode/l_code.h>
#include <Lcode/l_lcode.h>
#include <Lcode/l_mcode.h>
#include <Lcode/l_dominator.h>
#include <Lcode/l_loop.h>
#include <Lcode/l_check.h>
#include <Lcode/l_hashtbl.h>
#include <Lcode/l_memory.h>
#include <Lcode/l_flags.h>
#include <Lcode/l_controlflow.h>
#include <Lcode/l_dataflow.h>
#include <Lcode/l_pred_flow.h>
#include <Lcode/r_dataflow.h>
#include <Lcode/l_compare.h>
#include <Lcode/l_predicate.h>
#include <Lcode/l_graph.h>
#include <Lcode/l_int_range.h>
#include <Lcode/l_pred_graph.h>
#include <Lcode/l_safe.h>
#include <Lcode/l_speculate.h>
#include <Lcode/l_sync.h>
#include <Lcode/l_symbol.h>
#include <Lcode/l_appendix.h>
#include <Lcode/l_event.h>
#include <Lcode/l_region.h>
#include <Lcode/l_io.h>
#include <Lcode/l_binaryio.h>
#include <Lcode/l_error.h>
#include <Lcode/l_stat.h>
#include <Lcode/l_process_file.h>
#include <Lcode/l_callgraph.h>
#include <Lcode/l_intrinsic.h>
#include <Lcode/l_constant.h>

#include <library/l_parms.h>
#include <machine/m_spec.h>
#include <machine/lmdes.h>
#include <Lcode/l_time.h>

#include <Lcode/l_dot.h>

/*====================================================================*/
/*
 *      File vars
 */
/*====================================================================*/

#ifdef __WIN32__
#define strcasecmp _stricmp
#endif

#ifdef __cplusplus
extern "C"
{
#endif


  extern char *L_parm_file;
  extern char *L_input_file;
  extern char *L_output_file;
  extern char *L_filelist_file;
  extern char *L_lmdes_file_name;
  extern Parm_Macro_List *L_command_line_macro_list;

  extern FILE *L_IN;
  extern FILE *L_OUT;
  extern FILE *L_ERR;

  extern char *L_output_file_extension;
  extern char *L_input_file_extension;
  extern char *L_file_directory;
  extern int L_file_index;
  extern FILE *L_FILELIST;
  extern List L_filelist;
  extern L_File *L_file;

  extern int L_file_processing_model;
  extern char *L_file_processing_model_name;

/*====================================================================*/
/*
 *      Global Lcode parameters read from STD_PARMS
 */
/*====================================================================*/

  extern char *L_arch;
  extern char *L_model;
  extern char *L_swarch;

  extern int L_funtime_message;
  extern int L_nice_value;
  extern int L_output_generation_info;

  extern int L_determine_cpu_time;
  extern double L_cpu_time_print_threshold;
  extern int L_print_parent_op; /* print out parents of Mcode */

/* JLB 2/28/97 - parameter for outputing absolete Lcode format */
  extern int L_output_obsolete_ctype_format;

/* REH 4/11/96 - Lcode file format paramters */
  extern int L_output_binary_format;
  extern int L_input_binary_format;

/* allow callgraph to be output in daVinci format */
  extern int L_print_davinci_callgraph;
  extern char *L_davinci_callgraph_file;

/* Lcode structure configuration parameters */
  extern int L_max_dest_operand;
  extern int L_max_src_operand;
  extern int L_max_pred_operand;
  extern int L_oper_hash_tbl_size;
  extern int L_cb_hash_tbl_size;
  extern int L_region_hash_tbl_size;
  extern int L_expression_hash_tbl_size;

/* Predicate define structure */
  extern int L_max_pred_masks;
  extern int L_max_preds_per_mask;
  extern int L_max_pred_srcs;

/* Lcode fs branch model */
  extern int L_min_fs_weight;

/* Lcode exception model */
  extern int L_non_excepting_ops;
  extern int L_mask_potential_exceptions;
  extern int L_spec_model;
  extern char *L_speculation_model;

/* Lcode structure checking parameters */
  extern int L_check;
  extern int L_verbose_check;

/* memory disambiguation paramaters */
  extern int L_use_attr_label;
  extern int L_use_loop_iter;
  extern int L_new_memory_disamb;
  extern int L_incoming_param_indep;
  extern int L_diff_data_types_indep;
  extern int L_label_and_reg_access_indep;
  extern int L_sp_and_reg_access_indep;
  extern int L_debug_memory_disamb;
  extern int L_use_sync_arcs;
  extern int L_debug_sync_arcs;
  extern int L_eliminate_sync_arcs;
  extern int L_punt_on_sync_arcs_failure;

  extern int L_use_access_specs;

/* loop detection parameters */
  extern int L_static_loop_iter_count;
  extern int L_debug_loop;
  extern int L_debug_inner_loop;
  extern int L_debug_static_weight;

/* hash table repair parameters */
  extern int L_debug_hash_table_repair;
  extern int L_region_hash_table_management;

/* control flow function parameters */
  extern int L_debug_color_cb;

/* Lcode structure allocation parameters */
  extern int L_check_data_struct_alloc;
  extern int L_debug_data_struct_alloc;

/* Lcode Function Appendix parameters */
  extern int L_use_appendix_for_sync_arcs;
  extern int L_use_appendix_for_attr;
  extern int L_oper_default_to_appendix;
  extern char *L_oper_appendix_attr_list;
  extern char *L_oper_code_attr_list;
  extern int L_cb_default_to_appendix;
  extern char *L_cb_appendix_attr_list;
  extern char *L_cb_code_attr_list;

/* Lcode dataflow analysis parameters */
  extern int L_debug_df_cb_construction;
  extern int L_debug_df_live_in_out;
  extern int L_debug_df_reaching_defs;
  extern int L_debug_df_available_defs;
  extern int L_debug_df_mem_reaching_defs;
  extern int L_debug_df_mem_available_defs;
  extern int L_debug_df_dead_code;
  extern int L_df_max_pred_paths;
  extern int L_df_use_max_graph_builder;

/* Predicate define combining */
  extern int L_combine_only_same_class_preds;

/* JLB - 04-27-97 ctype propagagion */
  extern int L_propagate_sign_size_ctype_info;

  extern void *L_data_segment_start;

  extern char *L_code_file_extension;
  extern int L_code_file_index;

/* Intrinsic information -ITI/JWJ 7.6.1999 */
  extern char *L_intrinsic_database_filename;
  extern int L_intrinsic_support_enabled;

/* IA64 sias 20000517 */
  extern int L_ia64;

  extern int L_generate_spec_checks;

  extern int L_print_int_ranges;

  extern int L_versioned_acc_specs;

  extern int L_dump_core_on_punt;

/*====================================================================*/
/*
 *      Data structure alloction pools
 */
/*====================================================================*/

  extern L_Alloc_Pool *L_alloc_flow;
  extern L_Alloc_Pool *L_alloc_operand;
  extern L_Alloc_Pool *L_alloc_attr;
  extern L_Alloc_Pool *L_alloc_sync;
  extern L_Alloc_Pool *L_alloc_sync_info;
  extern L_Alloc_Pool *L_alloc_acc_spec;
  extern L_Alloc_Pool *L_alloc_oper;
  extern int L_oper_size;
  extern int L_expression_size;
  extern L_Alloc_Pool *L_alloc_cb;
  extern L_Alloc_Pool *L_alloc_expression;
  extern L_Alloc_Pool *L_alloc_oper_hash_entry;
  extern L_Alloc_Pool *L_alloc_oper_hash_tbl;
  extern L_Alloc_Pool *L_alloc_cb_hash_entry;
  extern L_Alloc_Pool *L_alloc_cb_hash_tbl;
  extern L_Alloc_Pool *L_alloc_expression_hash_entry;
  extern L_Alloc_Pool *L_alloc_func;
  extern L_Alloc_Pool *L_alloc_program;
  extern L_Alloc_Pool *L_alloc_expr;
  extern L_Alloc_Pool *L_alloc_data;
  extern L_Alloc_Pool *L_alloc_datalist_element;
  extern L_Alloc_Pool *L_alloc_datalist;
  extern L_Alloc_Pool *L_alloc_loop;
  extern L_Alloc_Pool *L_alloc_inner_loop;
  extern L_Alloc_Pool *L_alloc_ind_info;
  extern L_Alloc_Pool *L_alloc_oper_list;
  extern L_Alloc_Pool *L_alloc_l_cg_arc;
  extern L_Alloc_Pool *L_alloc_l_cg_node;
  extern L_Alloc_Pool *L_alloc_cg_dfs_info;
  extern L_Alloc_Pool *L_alloc_event;
  extern L_Alloc_Pool *L_alloc_event_map;
  extern L_Alloc_Pool *L_alloc_region;
  extern L_Alloc_Pool *L_alloc_region_member;
  extern L_Alloc_Pool *L_alloc_region_boundary;
  extern L_Alloc_Pool *L_alloc_region_regmap;
  extern L_Alloc_Pool *L_alloc_region_regcon;
  extern L_Alloc_Pool *L_alloc_region_hash_entry;
  extern L_Alloc_Pool *L_alloc_region_hash_tbl;

/*
 * LCW - new structures for preserving debugging information -- 9/14/95
 */
  extern L_Alloc_Pool *L_alloc_type;
  extern L_Alloc_Pool *L_alloc_dcltr;
  extern L_Alloc_Pool *L_alloc_struct_dcl;
  extern L_Alloc_Pool *L_alloc_union_dcl;
  extern L_Alloc_Pool *L_alloc_field;
  extern L_Alloc_Pool *L_alloc_enum_dcl;
  extern L_Alloc_Pool *L_alloc_enum_field;
/* LCW - structures for local variables - 4/15/97 */
  extern L_Alloc_Pool *L_alloc_local_var;

  extern L_Alloc_Pool *L_alloc_file;

#ifdef __cplusplus
}
#endif

/*====================================================================*/
/*
 *      ERROR HANDLING.
 */
/*====================================================================*/

#define L_ERR_INTERNAL          0
#define L_ERR_UNKNOWN           1
#define L_ERR_ILLEGAL_COMMAND   2
#define L_ERR_FAIL_OPEN_FILE    3
#define L_ERR_BAD_INPUT         4
#define L_ERR_CORRUPT_LCODE     5

#ifdef __cplusplus
extern "C"
{
#endif

  extern char *L_curr_pass_name;        /* current Lcode pass invoked */

/*====================================================================*/
/*
 *      Ouput files
 */
/*====================================================================*/

  extern FILE *L_open_output_file (char *);
  extern void L_close_output_file (FILE *);
  extern void L_insert_generic_info_to_output_file (FILE *);

/*====================================================================*/
/*
 *      Reading parameters
 */
/*====================================================================*/

  extern void L_read_parm_arch (Parm_Parse_Info *);
  extern void L_read_parm_file (Parm_Parse_Info *);
  extern void L_read_parm_global (Parm_Parse_Info *);
  extern void L_read_parm_lcode (Parm_Parse_Info *);

/*====================================================================*/
/*
 *      Processing parameters
 */
/*====================================================================*/
  extern int L_in_func_list (L_Func * fn, char *fn_list);

/*====================================================================*/
/*
 *      Memory allocation pools
 */
/*====================================================================*/

  extern void L_setup_alloc_pools (void);
  extern void L_check_alloc_for_func (void);
  extern void L_check_alloc_for_data (void);
  extern void L_check_alloc_for_region (void);

  extern void *Lcode_malloc (size_t size);
  extern void *Lcode_calloc (size_t nelem, size_t elsize);
  extern void Lcode_free (void *ptr);

/*====================================================================*/
/*
 *      Entrance routine for Lcode modules
 */
/*====================================================================*/

  extern void L_gen_code (Parm_Macro_List * external_list);

#ifdef __cplusplus
}
#endif

/*====================================================================*/
/*
 *      Macro definitions for Lcode
 */
/*====================================================================*/

#define IMPACT_MAX(x,y) ((x>y)?x:y)
#define IMPACT_MIN(x,y) ((x<y)?x:y)
#define ZERO_EQUIVALENT 0.00001
#define L_WEIGHT_VALUE  1000

#define TRUE 1
#define FALSE 0

/*====================================================================*/
/*
 *      Defines carried over from Hcode
 */
/*====================================================================*/

#define L_SWITCH_DEFAULT_CC             0x7FFFFFFF

#endif
