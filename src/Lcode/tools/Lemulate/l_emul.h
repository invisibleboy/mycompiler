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
 *      File: l_emul.h (version 2)
 *      Authors: IMPACT Technologies, Inc. (John C. Gyllenhaal)
 *      Creation Date:  March 1999
 * 
 *      This is a complete reengineering and rewrite of version 1 of
 *      of Lemulate, which was written by Qudus Olaniran, Dan Connors,
 *      IMPACT Technologies, Inc, and Wen-mei Hwu.  This new version is
 *      optimized for portability, performance, and hopefully clarity.
 *
\*****************************************************************************/
/* 08/05/02 REK Changing the definition of C_extern_file_name,
 *              C_struct_file_name, and C_include_file_name from pointers to
 *              character arrays.
 */

#ifndef __LEM_EMUL_H__
#define __LEM_EMUL_H__

/* 10/29/02 REK Adding config.h> */
#include <config.h>
/* 08/05/02 REK Including sys/param.h to get MAXPATHLEN. */
#include <sys/param.h>
#include <Lcode/l_main.h>
#include <Lcode/l_build_prototype_info.h>
#define MD_DEBUG_MACROS		/* Use type-checking versions of md.h macros */
#include <library/md.h>
#include <library/dynamic_symbol.h>
#include <library/heap.h>
#include <library/mbuf.h>

#define INTRINSICS

/* Structure to keep track of all the registers use, 
 * within a particular scope (i.e, within a function scope)
 */
typedef struct
{
  char *name;
  INT_Symbol_Table *pred_regs_used;
  INT_Symbol_Table *int_regs_used;
  INT_Symbol_Table *float_regs_used;
  INT_Symbol_Table *double_regs_used;
  INT_Symbol_Table *pred_macs_used;
  INT_Symbol_Table *int_macs_used;
  INT_Symbol_Table *float_macs_used;
  INT_Symbol_Table *double_macs_used;
}
Reg_Usage;

/* Structure to keep track of all the memory usage info.
 * Also used to keep a small amount of IMPACT data state info
 */
typedef struct
{
  char *name;

  /* IMPACT data label state info */
  char *data_label_name;	/* Label info is for (may be NULL) */
  int data_label_global;	/* Is this a global label? */
  int data_label_align;		/* Alignment of label, -1 if none */
  int data_label_element_size;	/* Element_size for label, -1 if none */
  char data_label_decl[TYPE_BUF_SIZE];	/* Label declaration, empty if none */
  char data_label_cast[TYPE_BUF_SIZE];	/* Label cast, empty if none */
  int object_id;

  /* Memory usage stats for the current function */
  int incoming_parm_size;
  int outgoing_parm_size;
  int reg_swap_size;
  int local_var_size;
  int alloc_size;

  /* The tables below have file scope */
  STRING_Symbol_Table *file_scope_data_labels_defined;
  STRING_Symbol_Table *file_scope_data_labels_used;
  STRING_Symbol_Table *file_scope_code_labels_defined;
  STRING_Symbol_Table *file_scope_code_labels_used;

  /* The tables below have program wide scope */
  STRING_Symbol_Table *program_scope_data_labels_defined;
  STRING_Symbol_Table *program_scope_data_labels_used;
  STRING_Symbol_Table *program_scope_code_labels_defined;
  STRING_Symbol_Table *program_scope_code_labels_used;

  STRING_Symbol_Table *struct_names_defined;
  STRING_Symbol_Table *union_names_created;	/*By C_generate_union_name() */
  STRING_Symbol_Table *init_routines_created;	/*By C_emit_reserve_data() */
}
Mem_Usage;

/*
 * Global variables (not set directly by parameters)
 */

extern L_Func *C_fn;
extern L_Data *C_data;
extern int C_token_type;

/* The peaked-ahead versions */
extern L_Func *C_peeked_fn;
extern L_Data *C_peeked_data;
extern int C_peeked_token_type;

/* Holds the host layout info */
extern MD *C_layout_database;

/* Prefix to use for all variables and labels created by Lemulate */
extern char *C_prefix;

/* Indentation used by C code generated */
extern char *C_indent;

/* Machine register ctype MCM 7/2000 */
extern int C_native_machine_ctype;

/* Machine register type string MCM 7/2000 */
extern char C_native_machine_ctype_str[32];
extern char C_native_machine_ctype_unsigned_str[32];

/* Monotonically increasing number used to create unique ids */
extern int C_unique_id;

/* File that will be written to hold external variable declarations. */
/* 08/05/02 REK Changing this variable to a character buffer. */
/*extern char *C_extern_file_name;*/
extern char C_extern_file_name[MAXPATHLEN];

/* File that will be written to hold structure declarations. */
/* 08/05/02 REK Changing this variable to a character buffer. */
/*extern char *C_struct_file_name;*/
extern char C_struct_file_name[MAXPATHLEN];

/* File that will be written to hold ancillary includes. */
/* 08/05/02 REK Changing this variable to a character buffer. */
/*extern char *C_include_file_name;*/
extern char C_include_file_name[MAXPATHLEN];

/* If 1, inserts trace system code and specified instrumentation */
extern int C_insert_probes;

/* Start function ids from 1000 to simplify interpreting the trace */
extern int C_trace_func_id;

/* For each function, start numbering jsr calls from 1 */
extern int C_trace_jsr_id;

/* Warn only once about potential problems detected */
extern int C_warned_about_promoted_preds;

/* Flag for custom mode, to support mem obj profiling. */
extern int C_custom_profiling;

/* 
 * Parameters controlling instrumentation 
 */

extern char *C_probe_for;

/* If set to 1, the probe code is predicated on the operation's pred[0] */
extern int C_predicate_probe_code;

/* If set to 1, trace control flow (functions and cb trace) */
extern int C_trace_control_flow;

/* If set to 1 and C_trace_control_flow is 1, trace empty cb ids */
extern int C_trace_empty_cbs;

/* If set to 1, trace load and store memory addresses */
extern int C_trace_mem_addrs;

/* If set to 1, trace MASKED_SEG_FAULT/NO_SEG_FAULT for non-trapping loads */
extern int C_trace_masked_load_faults;

/* If set to 1, trace src[1] of jump_rg's (the hashing jump condition) */
extern int C_trace_jump_rg_src1;

/* If set to 1, trace predicate's value at each use */
extern int C_trace_pred_uses;

/* If set to 1, trace predicate's value at its def(s) */
extern int C_trace_pred_defs;

/* If set to 1, trace pred[1]'s value (if exists) at each use */
extern int C_trace_promoted_preds;

/* If set to 1, adds L_TRACE_BRTHRU to trace if predicated jump falls-thru */
extern int C_trace_pred_jump_fall_thru;

/* If set to 1, adds additional headers to facilitate parsing of trace
 * even if benchmark assembly is not available (doubles trace size) 
 */
extern int C_trace_extra_headers;

/* If set to 1, emit op->id of every operation executed into trace */
extern int C_trace_op_ids;

/* If set to 1, emit fn's id & op->id of every operation executed into trace */
extern int C_trace_enhanced_op_ids;

/* If set to 1, emit dest reg values into trace (much larger trace) */
extern int C_trace_dest_reg_values;

/* If set to 1, emit src reg values into trace (much larger trace) */
extern int C_trace_src_reg_values;

/* If set to 1, emit src literal values into trace (much larger trace) */
extern int C_trace_src_lit_values;

extern int C_trace_objects;

/*
 * Other Lemulate parameters 
 */

/* If set to 1, output Ansi-C compatible code.  Otherwise, output 
 * output K&R-C compatible code.
 */
extern int C_ansi_c_mode;

/* Allow verification and generation of info files for host_info,
 * preprocess_info, and impact_info attributes to be turned off 
 */
extern int C_generate_info_files;

/* Allow use of register arrays instead of individual register variables.
 * Using arrays slows down emulation but facilitates some user extensions.
 */
extern int C_use_register_arrays;


/* File containing basic and user type alignment and size information.
 * Generated by gen_CtoP.
 */
extern char *C_layout_database_name;

/* Generate only the .c files for the inputs lcode files
 */
extern int C_cfile_only;


/* Maps strings to ids so that all the identical strings in
 * a function are represented by exactly one string.  Most compilers
 * combine strings in this way (including all IMPACT backends)
 * but not all C compilers do this (so it is non-portable for Lemulate
 * to assume it will be done), so Lemulate must do this explicitly.  -JCG 2/00
 */
extern STRING_Symbol_Table *C_string_map;

/* Reuse emulation support */
extern int C_insert_reuse_emulation;

#endif
