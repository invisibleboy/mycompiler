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
 *      File: l_emul.c (version 2)
 *      Authors: IMPACT Technologies, Inc. (John C. Gyllenhaal)
 *      Creation Date:  March 1999
 * 
 *      This is a complete reengineering and rewrite of version 1 of
 *      of Lemulate, which was written by Qudus Olaniran, Dan Connors,
 *      IMPACT Technologies, Inc, and Wen-mei Hwu.  This new version is
 *      optimized for portability, performance, and hopefully clarity.
 *
\*****************************************************************************/
/* 07/12/02 REK Adding temporary debugging output */
/* 08/05/02 REK Modifying this file so that the extra headers (_EM_*h) are
 *              included with absolute paths instead of relative paths.
 *              Compilation may take place across several directories, so
 *              all source files may not be in the same directory as the
 *              extra headers.  C_extern_file_name, C_struct_file_name, and
 *              C_include_file_name are now initialized in L_gen_code.
 */
/* 11/19/02 REK Modifying this so it writes main to _EM_data_init.c instead
 *              of data_init.c.  The new filename should be less likely to
 *              conflict with a user's file.
 */

/* 10/29/02 REK Adding config.h */
#include <config.h>
/* 08/05/02 REK Including sys/param.h to get MAXPATHLEN. */
#include <sys/param.h>
#include <Lcode/l_emul.h>
#include "l_emul_decl.h"
#include "l_emul_emit_builtin.h"
#include "l_emul_emit_data.h"
#include "l_emul_emit_op.h"
#include "l_emul_emit_operand.h"
#include "l_emul_intrinsic.h"
#include "l_emul_trace.h"
#include "l_emul_util.h"

#ifdef INTRINSICS
#include "l_emul_intrinsic.h"
#endif

/*
 * Global variables (not set directly by parameters)
 */

/* Implement wrappers for L_get_input(), so can peek ahead
 * cleanly (needed by C_emit_reserve_data() ).
 * In addition, allows everything read in to be scanned for 
 * code and data labels (so that they may be properly externed).
 * 
 * Use C_get_input(), C_peek_input(), C_fn, C_data, 
 * C_token_type instead of Lcode's version.
 */
L_Func *C_fn = NULL;
L_Data *C_data = NULL;
int C_token_type = 0;

/* The peaked-ahead versions */
L_Func *C_peeked_fn = NULL;
L_Data *C_peeked_data = NULL;
int C_peeked_token_type = -1;

/* Holds the host layout info */
MD *C_layout_database = NULL;

/* Prefix to use for all variables and labels created by Lemulate */
char *C_prefix = "_EM_";

/* Indentation used by C code generated */
char *C_indent = "    ";

/* Machine register ctype MCM 7/2000 */
int C_native_machine_ctype = 0;

/* Machine register type string MCM 7/2000 */
char C_native_machine_ctype_str[32];
char C_native_machine_ctype_unsigned_str[32];

/* Monotonically increasing number used to create unique ids */
int C_unique_id = 0;

/* File that will be written to hold external variable declarations. */
/* 08/05/02 REK This variable is now initialized in L_gen_code. */
/*char *C_extern_file_name = "./_EM_extern.h";*/
char C_extern_file_name[MAXPATHLEN];

/* File that will be written to hold structure declarations. */
/* 08/05/02 REK This variable is now initialized in L_gen_code. */
/*char *C_struct_file_name = "./_EM_struct.h";*/
char C_struct_file_name[MAXPATHLEN];

/* File that will be written to hold ancillary includes. */
/* 08/05/02 REK This variable is now initialized in L_gen_code. */
/*char *C_include_file_name = "./_EM_include.h";*/
char C_include_file_name[MAXPATHLEN];

/* If 1, inserts trace system code and specified instrumentation */
int C_insert_probes = 1;

/* Start function ids from 1000 to simplify interpreting the trace */
int C_trace_func_id = 1000;

/* For each function, start numbering jsr calls from 1 */
int C_trace_jsr_id = 1;

/* Warn only once about potential problems detected */
int C_warned_about_promoted_preds = 0;

/* Flag for custom mode, to support mem obj profiling. */
int C_custom_profiling = 0;

/* 
 * Parameters controlling instrumentation 
 */

/* Set C_probe_for to "nothing", "custom", "profiling", or "simulation" 
 * "nothing"-> no instrumentation (sets C_insert_probes to 0)
 * "custom"-> no presets, uses parameter settings directly
 * "profiling" -> uses presets for lcode profiling 
 * "simulation" -> uses presets for lcode simulation (still uses some parms)
 * "memtrace" -> 
 */
char *C_probe_for = "nothing";

/* If set to 1, the probe code is predicated on the operation's pred[0] */
int C_predicate_probe_code = 0;

/* If set to 1, trace control flow (functions and cb trace) */
int C_trace_control_flow = 0;

/* If set to 1 and C_trace_control_flow is 1, trace empty cb ids */
int C_trace_empty_cbs = 0;

/* If set to 1, trace load and store memory addresses */
int C_trace_mem_addrs = 0;

/* If set to 1, trace MASKED_SEG_FAULT/NO_SEG_FAULT for non-trapping loads */
int C_trace_masked_load_faults = 0;

/* If set to 1, trace src[1] of jump_rg's (the hashing jump condition) */
int C_trace_jump_rg_src1 = 0;

/* If set to 1, trace predicate's value at each use */
int C_trace_pred_uses = 0;

/* If set to 1, trace predicate's value at its def(s) */
int C_trace_pred_defs = 0;

/* If set to 1, trace pred[1]'s value (if exists) at each use */
int C_trace_promoted_preds = 0;

/* If set to 1, adds L_TRACE_BRTHRU to trace if predicated jump falls-thru */
int C_trace_pred_jump_fall_thru = 0;

/* If set to 1, adds additional headers to facilitate parsing of trace
 * even if benchmark assembly is not available (doubles trace size) 
 */
int C_trace_extra_headers = 0;

/* If set to 1, emit op->id of every operation executed into trace */
int C_trace_op_ids = 0;

/* If set to 1, emit fn's id & op->id of every operation executed into trace */
int C_trace_enhanced_op_ids = 0;

/* If set to 1, emit dest reg values into trace (much larger trace) */
int C_trace_dest_reg_values = 0;

/* If set to 1, emit src reg values into trace (much larger trace) */
int C_trace_src_reg_values = 0;

/* If set to 1, emit src literal values into trace (much larger trace) */
int C_trace_src_lit_values = 0;

int C_trace_objects = 0;

/* Generate only the .c files for the inputs lcode files
 */
int C_cfile_only = 0;

/*
 * Other Lemulate parameters 
 */

/* If set to 1, output Ansi-C compatible code.  Otherwise, output 
 * output K&R-C compatible code.
 */
int C_ansi_c_mode = 1;

/* Allow verification and generation of info files for host_info,
 * preprocess_info, and impact_info attributes to be turned off 
 */
int C_generate_info_files = 1;

/* Allow use of register arrays instead of individual register variables.
 * Using arrays slows down emulation but facilitates some user extensions.
 */
int C_use_register_arrays = 0;


/* File containing basic and user type alignment and size information.
 * Generated by gen_CtoP.
 */
char *C_layout_database_name = "./host_layout_info.md";

/* Maps strings to ids so that all the identical strings in
 * a function are represented by exactly one string.  Most compilers
 * combine strings in this way (including all IMPACT backends)
 * but not all C compilers do this (so it is non-portable for Lemulate
 * to assume it will be done), so Lemulate must do this explicitly.  -JCG 2/00
 */
STRING_Symbol_Table *C_string_map = NULL;

/* Prototypes */
extern void C_dclptr_to_C_string (char *dest_buf, L_Dcltr * dclptr,
				  char *incoming_buf);
extern void C_add_data_mem_usage (L_Data * data, Mem_Usage * mem_usage);


/* Reuse emulation support */
int C_insert_reuse_emulation = 0;



/***************************************************************************\
  Check host_info to make sure consistent in all the functions emulated.  
  Write out each attribute field into separate files,
  host_info.platform, host_info.compiler, and host_info.compiler_version. 
****************************************************************************/
void
C_check_host_info (L_Func * fn)
{
  static char *expected_platform = NULL;
  static char *expected_compiler = NULL;
  static char *expected_compiler_version = NULL;
  static char *set_using = NULL;
  static int initialized_expected_values = 0;
  char *platform, *compiler, *compiler_version;
  int inconsistent;
  L_Attr *host_info_attr;

  /* Allow turning off the these checks */
  if (!C_generate_info_files)
    return;

  /* Find host_info attribute (if any) */
  host_info_attr = L_find_attr (fn->attr, "host_info");

  /* If present, make sure in proper form */
  if (host_info_attr != NULL)
    {
      if ((host_info_attr->max_field < 3) ||
	  (host_info_attr->field[0] == NULL) ||
	  !L_is_string (host_info_attr->field[0]) ||
	  (host_info_attr->field[1] == NULL) ||
	  !L_is_string (host_info_attr->field[1]) ||
	  (host_info_attr->field[2] == NULL) ||
	  !L_is_string (host_info_attr->field[2]))
	{
	  fprintf (stderr, "\n");
	  L_print_attr (stderr, host_info_attr);
	  fprintf (stderr, "\n");
	  L_punt ("Function %s: host_info attr not in proper form!",
		  fn->name);
	}
      /* Grab pointers to strings in host_info.  Must not modify
       * these values!
       */
      platform = host_info_attr->field[0]->value.s;
      compiler = host_info_attr->field[1]->value.s;
      compiler_version = host_info_attr->field[2]->value.s;
    }
  else
    {
      /* Initialize to empty strings. */
      platform = "";
      compiler = "";
      compiler_version = "";
    }

  /* If first function, initialize expected values */
  if (!initialized_expected_values)
    {
      /* Strdup expected info, so can use across functions */
      expected_platform = strdup (platform);
      expected_compiler = strdup (compiler);
      expected_compiler_version = strdup (compiler_version);
      set_using = strdup (fn->name);
      initialized_expected_values = 1;

      /* Write out info to files for gen_probed_lcode script's use */
      C_write_brand ("host_info.platform", expected_platform, "");
      C_write_brand ("host_info.compiler", expected_compiler, "");
      C_write_brand ("host_info.compiler_version",
		     expected_compiler_version, "");
    }

  /* Make sure got what we expected! If not, flag and write new
   * values out to the brand files to signal gen_probed_lcode!
   */
  inconsistent = 0;
  if (strcmp (platform, expected_platform) != 0)
    {
      C_write_brand ("host_info.platform", platform, expected_platform);
      inconsistent = 1;
    }

  if (strcmp (compiler, expected_compiler) != 0)
    {
      C_write_brand ("host_info.compiler", compiler, expected_compiler);
      inconsistent = 1;
    }

  if (strcmp (compiler_version, expected_compiler_version) != 0)
    {
      C_write_brand ("host_info.compiler_version", compiler_version,
		     expected_compiler_version);
      inconsistent = 1;
    }

  if (inconsistent)
    {
      fprintf (stderr,
	       "\nLemulate warning: Inconsistent function host_info "
	       "attributes!\n");
      fprintf (stderr,
	       "  %s:\n"
	       "    platform: %s\n"
	       "    compiler: %s\n"
	       "    compiler version: %s\n",
	       set_using, expected_platform, expected_compiler,
	       expected_compiler_version);
      fprintf (stderr,
	       "  %s:\n"
	       "    platform: %s\n"
	       "    compiler: %s\n"
	       "    compiler version: %s\n",
	       fn->name, platform, compiler, compiler_version);

      /* Get new settings */
      free (expected_platform);
      free (expected_compiler);
      free (expected_compiler_version);
      expected_platform = strdup (platform);
      expected_compiler = strdup (compiler);
      expected_compiler_version = strdup (compiler_version);
    }
}

/***************************************************************************\
  Check preprocess_info to make sure consistent in all the functions
  emulated.  Write out each attribute field into separate files,
  preprocess_info.mode, preprocess_info.invocation, and
  preprocess_info.date.
****************************************************************************/
void
C_check_preprocess_info (L_Func * fn)
{
  static char *expected_mode = NULL;
  static char *expected_invocation = NULL;
  static char *expected_extra_options = NULL;
  static char *expected_date = NULL;
  static char *set_using = NULL;
  static int initialized_expected_values = 0;
  char *mode, *invocation, *extra_options, *date;
  int inconsistent;
  L_Attr *preprocess_info_attr;

  /* Allow turning off the these checks */
  if (!C_generate_info_files)
    return;

  /* Find preprocess_info attribute (if any) */
  preprocess_info_attr = L_find_attr (fn->attr, "preprocess_info");

  /* If present, make sure in proper form 
   * Simply ignore 2.31 and earlier formats for this attribute. -6/99
   */
  if ((preprocess_info_attr != NULL) &&
      (preprocess_info_attr->max_field >= 4))
    {
      if ((preprocess_info_attr->max_field < 4) ||
	  (preprocess_info_attr->field[0] == NULL) ||
	  !L_is_string (preprocess_info_attr->field[0]) ||
	  (preprocess_info_attr->field[1] == NULL) ||
	  !L_is_string (preprocess_info_attr->field[1]) ||
	  (preprocess_info_attr->field[2] == NULL) ||
	  !L_is_string (preprocess_info_attr->field[2]) ||
	  (preprocess_info_attr->field[3] == NULL) ||
	  !L_is_string (preprocess_info_attr->field[3]))
	{
	  fprintf (stderr, "\n");
	  L_print_attr (stderr, preprocess_info_attr);
	  fprintf (stderr, "\n");
	  L_punt ("Function %s: preprocess_info attr not in proper form!",
		  fn->name);
	}
      /* Grab pointers to strings in preprocess_info.  Must not modify
       * these values!
       */
      mode = preprocess_info_attr->field[0]->value.s;
      invocation = preprocess_info_attr->field[1]->value.s;
      extra_options = preprocess_info_attr->field[2]->value.s;
      date = preprocess_info_attr->field[3]->value.s;
    }
  else
    {
      /* Initialize to empty strings. */
      mode = "";
      invocation = "";
      extra_options = "";
      date = "";
    }

  /* If first function, initialize expected values */
  if (!initialized_expected_values)
    {
      /* Strdup expected info, so can use across functions */
      expected_mode = strdup (mode);
      expected_invocation = strdup (invocation);
      expected_extra_options = strdup (extra_options);
      expected_date = strdup (date);
      set_using = strdup (fn->name);
      initialized_expected_values = 1;

      /* Write out info to files for gen_probed_lcode script's use */
      C_write_brand ("preprocess_info.mode", expected_mode, "");
      C_write_brand ("preprocess_info.invocation", expected_invocation, "");
      C_write_brand ("preprocess_info.extra_options",
		     expected_extra_options, "");
      C_write_brand ("preprocess_info.date", expected_date, "");
    }

  /* Make sure got what we expected! If not, flag and write new
   * values out to the brand files to signal gen_probed_lcode!
   */
  inconsistent = 0;
  if (strcmp (mode, expected_mode) != 0)
    {
      C_write_brand ("preprocess_info.mode", mode, expected_mode);
      inconsistent = 1;
    }

  if (strcmp (invocation, expected_invocation) != 0)
    {
      C_write_brand ("preprocess_info.invocation", invocation,
		     expected_invocation);
      inconsistent = 1;
    }

  if (strcmp (extra_options, expected_extra_options) != 0)
    {
      C_write_brand ("preprocess_info.extra_options", extra_options,
		     expected_extra_options);
      inconsistent = 1;
    }

  if (strcmp (date, expected_date) != 0)
    {
      C_write_brand ("preprocess_info.date", date, expected_date);
      inconsistent = 1;
    }

  if (inconsistent)
    {
      fprintf (stderr,
	       "\nLemulate warning: Inconsistent function preprocess_info "
	       "attributes!\n");
      fprintf (stderr,
	       "  %s:\n"
	       "                   mode: %s\n"
	       "             invocation: %s\n"
	       "          extra options: %s\n"
	       "    gen_CtoP started on: %s\n",
	       set_using, expected_mode, expected_invocation,
	       expected_extra_options, expected_date);
      fprintf (stderr,
	       "  %s:\n"
	       "                   mode: %s\n"
	       "             invocation: %s\n"
	       "          extra options: %s\n"
	       "    gen_CtoP started on: %s\n",
	       fn->name, mode, invocation, extra_options, date);

      /* Get new settings */
      free (expected_mode);
      free (expected_invocation);
      free (expected_extra_options);
      free (expected_date);
      expected_mode = strdup (mode);
      expected_invocation = strdup (invocation);
      expected_extra_options = strdup (extra_options);
      expected_date = strdup (date);
    }
}


/***************************************************************************\
  Check impact_info to make sure consistent in all the functions emulated.
  Write out each attribute field into separate files,
  impact_info.version, impact_info.type, impact_info.date, and 
  impact_info.lcode. 
****************************************************************************/
void
C_check_impact_info (L_Func * fn)
{
  static char *expected_version = NULL;
  static char *expected_type = NULL;
  static char *expected_date = NULL;
  static char *expected_lcode = NULL;
  static char *set_using = NULL;
  static int initialized_expected_values = 0;
  char *version, *type, *date, *lcode;
  int inconsistent;
  L_Attr *impact_info_attr;

  /* Allow turning off the these checks */
  if (!C_generate_info_files)
    return;

  /* Find impact_info attribute (if any) */
  impact_info_attr = L_find_attr (fn->attr, "impact_info");

  /* If present, make sure in proper form */
  if (impact_info_attr != NULL)
    {
      if ((impact_info_attr->max_field < 4) ||
	  (impact_info_attr->field[0] == NULL) ||
	  !L_is_string (impact_info_attr->field[0]) ||
	  (impact_info_attr->field[1] == NULL) ||
	  !L_is_string (impact_info_attr->field[1]) ||
	  (impact_info_attr->field[2] == NULL) ||
	  !L_is_string (impact_info_attr->field[2]) ||
	  (impact_info_attr->field[3] == NULL) ||
	  !L_is_string (impact_info_attr->field[3]))
	{
	  fprintf (stderr, "\n");
	  L_print_attr (stderr, impact_info_attr);
	  fprintf (stderr, "\n");
	  L_punt ("Function %s: impact_info attr not in proper form!",
		  fn->name);
	}
      /* Grab pointers to strings in impact_info.  Must not modify
       * these values!
       */
      version = impact_info_attr->field[0]->value.s;
      type = impact_info_attr->field[1]->value.s;
      date = impact_info_attr->field[2]->value.s;
      lcode = impact_info_attr->field[3]->value.s;
    }
  else
    {
      /* Initialize to empty strings. */
      version = "";
      type = "";
      date = "";
      lcode = "";
    }

  /* If first function, initialize expected values */
  if (!initialized_expected_values)
    {
      /* Strdup expected info, so can use across functions */
      expected_version = strdup (version);
      expected_type = strdup (type);
      expected_date = strdup (date);
      expected_lcode = strdup (lcode);
      set_using = strdup (fn->name);
      initialized_expected_values = 1;

      /* Write out info to files for gen_probed_lcode script's use */
      C_write_brand ("impact_info.version", expected_version, "");
      C_write_brand ("impact_info.type", expected_type, "");
      C_write_brand ("impact_info.date", expected_date, "");
      C_write_brand ("impact_info.lcode", expected_lcode, "");
    }

  /* Make sure got what we expected! If not, flag and write new
   * values out to the brand files to signal gen_probed_lcode!
   */
  inconsistent = 0;
  if (strcmp (version, expected_version) != 0)
    {
      C_write_brand ("impact_info.version", version, expected_version);
      inconsistent = 1;
    }

  if (strcmp (type, expected_type) != 0)
    {
      C_write_brand ("impact_info.type", type, expected_type);
      inconsistent = 1;
    }

  if (strcmp (date, expected_date) != 0)
    {
      C_write_brand ("impact_info.date", date, expected_date);
      inconsistent = 1;
    }

  if (strcmp (lcode, expected_lcode) != 0)
    {
      C_write_brand ("impact_info.lcode", lcode, expected_lcode);
      inconsistent = 1;
    }

  if (inconsistent)
    {
      fprintf (stderr,
	       "\nLemulate warning: Inconsistent function impact_info "
	       "attributes!\n");
      fprintf (stderr,
	       "  %s:\n"
	       "          version: %s\n"
	       "             type: %s\n"
	       "             date: %s\n"
	       "    lcode version: %s\n",
	       set_using, expected_version, expected_type,
	       expected_date, expected_lcode);
      fprintf (stderr,
	       "  %s:\n"
	       "          version: %s\n"
	       "             type: %s\n"
	       "             date: %s\n"
	       "    lcode version: %s\n",
	       fn->name, version, type, date, lcode);

      /* Get new settings */
      free (expected_version);
      free (expected_type);
      free (expected_date);
      free (expected_lcode);
      expected_version = strdup (version);
      expected_type = strdup (type);
      expected_date = strdup (date);
      expected_lcode = strdup (lcode);
    }
}

/***************************************************************************\
  Check host_info, preprocess_info, and impact_info to make sure
  consistent in all the functions emulated.  Write out each attribute
  field into separate files (i.e., host_info.platform,
  host_info.compiler, impact_info.version, preprocess_info.mode, etc).
 
  This allows warnings about preprocessing on one platform and
  emulating on other, etc. to be printed.  These problems are
  extremely hard to track down otherwise!
****************************************************************************/
void
C_check_info_attributes (L_Func * fn)
{
  C_check_host_info (fn);
  C_check_preprocess_info (fn);
  C_check_impact_info (fn);
}

/* Emit the entire control block in C code */
void
C_emit_cb (FILE * out, L_Func * fn, L_Cb * cb)
{
  L_Oper *op;
  int op_count;

  /* Emit cb label for use as branch targets and for documentation */
  fprintf (out, "\n%scb_%i: /* %s cb %i */\n", C_prefix, cb->id,
	   C_true_name (fn->name), cb->id);

  /* Trace cb id, if tracing control flow and not the first cb in
   * the function.
   */
  if (C_insert_probes && C_trace_control_flow && (fn->first_cb != cb))
    {
      /* If cb is not empty or if we are tracing empty cb ids, emit probe */
      if ((cb->first_op != NULL) || C_trace_empty_cbs)
	{
	  /* Pass NULL as op, since this probe should never be predicated */
	  C_emit_put_trace_int (out, fn, NULL, NULL, -cb->id);
	}
    }

  /* Count operations in the cb */
  op_count = 0;

  /* Emit all the operations in the control block */
  for (op = cb->first_op; op != NULL; op = op->next_op)
    {
      /* At the beginning of the cb, and after every 127 operations,
       * put in check for a full trace buffer.
       */
      if (C_insert_probes && ((op_count & 127) == 0))
	{
	  fprintf (out, "%s_EM_flush_trace_if_necessary();\n", C_indent);
	}

      /* Emit the emulation (and instrumentation) code for this operation */
      C_emit_op (out, fn, op);

      /* Update cb's operation count */
      op_count++;
    }
}

static int
C_get_last_parm_index (L_Func * fn)
{
  char return_type_buf[TYPE_BUF_SIZE];
  char caller_parm_type_buf[TYPE_BUF_SIZE];
  char parm_type_buf[TYPE_BUF_SIZE];
  char *parse_ptr;
  int parm_index;

  L_get_call_info (fn, NULL, fn->attr, return_type_buf,
		   caller_parm_type_buf, sizeof (return_type_buf));

  parse_ptr = caller_parm_type_buf;
  parm_index = -1;
  while (*parse_ptr != 0)
    {
      /* Get the next parameter type */
      L_get_next_param_type (parm_type_buf, &parse_ptr);
      
      /* If vararg, we want the last parameter */
      if (strcmp (parm_type_buf, "vararg") == 0)
	break;

      parm_index++;
    }

  if (parm_index == -1)
    L_punt ("C_get_last_parm_index called on a function "
	    "with no parameters");

  return parm_index;
}


/* Emit stack space declaration and setup:
 *
 * The stack is assumed to grow down and there are several
 * alias for the stack pointer, none which point to
 * the "real" stack pointer.
 *
 * During code generation, all these aliases are converted into
 * references to SP and their offsets adjusted appropriately.
 * SP is updated just twice, at function entry and exit, all
 * other accesses are just ofsets (no pushes or pops!)
 *
 * For clarity, we will set all of the aliases appropriately and
 * not mess with the offsets!
 *
 * IMPACT Architecture: The stack grows toward lower addresses
 * 
 * JWS:  Uhhh... not so sure about this
 *  (new model has positive offsets from IP)
 *                   *====================*    Caller's stack frame
 *   e.g.: $IP+20->  *   incoming parms   *  |
 * $IP->
 * $LV->    max addr *====================* ---
 *   e.g.: $LV-56->  *     local vars     *  |
 *                   *====================*  
 *                   *     alloc space    *  |
 *                   *====================*    Callee's stack frame
 *   e.g.: $SP+40->  *     swap space     *  |
 * $OP & $SP->       *====================*
 *   e.g.: $OP-20->  *   outgoing parms   *  |
 *          min addr *====================* ---
 *
 * IA-64 Architecture: The stack grows toward lower addresses
 * 
 *                   *====================*    Caller's stack frame
 *   e.g.: $IP+20->  *   incoming parms   *  |
 * $IP->    max addr ======================  | 
 *                   *   scratch space    *  |  16 bytes
 *                   ====================== ---
 *   e.g.: $LV+56->  *     local vars     *  |
 * $LV->             *====================*  
 *                   *     alloc space    *  |
 *                   *====================*    Callee's stack frame
 *   e.g.: $SP+40->  *     swap space     *  |
 *?$SP->             *====================*
 *   e.g.: $OP+20->  *   outgoing parms   *  |
 * $OP      min addr *====================* ---
 * 
 *
 * If the function returns structure, a return space is allocated
 * just before the incoming parms space.  Declare a (prefix)return_space,
 * when necessary, to emulate this.
 */
void
C_emit_stack_setup (FILE * out, FILE *extern_out, L_Func * fn, 
		    Mem_Usage * mem_usage)
{
  int stack_size;
  int adjusted_memory_stack = 0;
  int scratch_space = 0;
  char return_type_buf[TYPE_BUF_SIZE];
  char name_buf[TYPE_BUF_SIZE];
  char decl_buf[TYPE_BUF_SIZE];

  /* Make all sizes a multiple of of 8 (increase them until multple of 8) so
   * that is easy to keep the stack double aligned (on a 8-byte boundary).
   */
  C_make_multiple_of_eight (&mem_usage->incoming_parm_size);
  C_make_multiple_of_eight (&mem_usage->local_var_size);
  C_make_multiple_of_eight (&mem_usage->reg_swap_size);
  C_make_multiple_of_eight (&mem_usage->outgoing_parm_size);

  /* Calculate the stack size needed by all the stack spaces
   * (allow forced alignment to 8-byte boundary)
   */
  stack_size = mem_usage->incoming_parm_size +
    mem_usage->local_var_size +
    mem_usage->reg_swap_size + mem_usage->outgoing_parm_size;

  /* If not using register arrays... */
  if (!C_use_register_arrays)
    {
      /* Allocate Lcode stack pointers (SP & aliases) */
      fprintf (out,
	       "\n"
	       "%s/* Lcode stack pointers (SP & aliases) */\n"
	       "%s%s ", C_indent, C_indent, C_native_machine_ctype_str);
      C_emit_int_mac (out, L_MAC_SP);
      fprintf (out, ", ");
      C_emit_int_mac (out, L_MAC_IP);
      fprintf (out, ", ");
      C_emit_int_mac (out, L_MAC_OP);
      fprintf (out, ", ");
      C_emit_int_mac (out, L_MAC_LV);
      fprintf (out, ";\n");
    }

  /* Get the function's return type */
  L_get_call_info (fn, NULL, fn->attr, return_type_buf, NULL,
		   sizeof (return_type_buf));

  /* Emit return_space setup if function returns a structure */
  if (L_convert_type_to_ctype (return_type_buf) == CTYPE_STRUCT)
    {
      /* Create return space name */
      sprintf (name_buf, "%sreturn_space", C_prefix);

      /* Create declaration for this return space using return type */
      L_convert_type_to_C_format (decl_buf, return_type_buf, name_buf);

      /* Emit declaration and comment */
      fprintf (out,
	       "\n%s/* Allocate Lcode return space on 'C' stack */\n",
	       C_indent);

      fprintf (out, "%s%s;\n", C_indent, decl_buf);
    }


  if ((M_arch == M_IMPACT) || (M_arch == M_PLAYDOH))
    {
      fprintf(out,
	      "%s/* incoming_parm_size       %d */\n"
	      "%s/* local_var_size           %d */\n"
	      "%s/* swap_size                %d */\n"
	      "%s/* alloc_size               %d */\n"
	      "%s/* out_parm_size            %d */\n"
	      "%s",
	      C_indent, mem_usage->incoming_parm_size,
	      C_indent, mem_usage->local_var_size,
	      C_indent, mem_usage->reg_swap_size,
	      C_indent, mem_usage->alloc_size,
	      C_indent, mem_usage->outgoing_parm_size, C_indent);
      
      if (L_find_attr (fn->attr, "adjusted_memory_stack"))
	{
	  adjusted_memory_stack = 1;
	  scratch_space = 16;
	}
      
      stack_size += scratch_space;

      /* KVM : Add an extra 8 bytes. Return value is stored in IP+0 and IP+4,
       * and IP is SP, which is just &_EM_stack[16]. So adding 8 more bytes to _EM_stack
       * is necessary for correct emulation.
       */
      /* Allocate Lcode stack on C stack */
      fprintf (out,
	       "\n"
	       "%s/* Allocate Lcode stack on 'C' stack */\n"
	       "%schar %sstack[%i];\n",
	       C_indent, C_indent, C_prefix, stack_size + 8);

      /* Set up stack pointers according to stack frame layout 
       * specified in comment at top of this function 
       */
      fprintf (out,
	       "\n"
	       "%s/* Move SP to just after OP space and force SP "
	       "to be double aligned */\n" "%s", C_indent, C_indent);
      C_emit_int_mac (out, L_MAC_SP);
      
      fprintf (out, " = ((%s)&%sstack[%i]) & ~(7);\n",
	       C_native_machine_ctype_str, C_prefix,
	       mem_usage->outgoing_parm_size + scratch_space);  

      fprintf (out, "\n"
	       "%s/* Set up rest of SP aliases relative to SP*/\n", C_indent);

      fprintf (out, "%s", C_indent);
      C_emit_int_mac (out, L_MAC_OP);
      fprintf (out, " = ");
      C_emit_int_mac (out, L_MAC_SP);
      fprintf (out, " - %d;\n", mem_usage->outgoing_parm_size + scratch_space);

      fprintf (out, "%s", C_indent);
      C_emit_int_mac (out, L_MAC_LV);
      fprintf (out, " = ");
      C_emit_int_mac (out, L_MAC_SP);
      fprintf (out, " + %i;\n",
	       mem_usage->local_var_size + mem_usage->reg_swap_size +
	       mem_usage->alloc_size);

      {
	L_Attr *va_attr;

	if (!(va_attr = L_find_attr (fn->attr, "VARARG")))
	  {
	    fprintf (out, "%s", C_indent);
	    C_emit_int_mac (out, L_MAC_IP);
	    fprintf (out, " = ");
	    C_emit_int_mac (out, L_MAC_SP);
	    fprintf (out, " + %i;\n",
		     mem_usage->local_var_size + mem_usage->reg_swap_size +
		     mem_usage->alloc_size);
	  }
	else
	  {
	    int va_indx = va_attr->field[0]->value.i;
	    fprintf (out, "%s", C_indent);
	    C_emit_int_mac (out, L_MAC_IP);
	    fprintf (out, " = ");
	    fprintf (out, "(long)(&_EM_p0);\n");
	  }
      }

      if (adjusted_memory_stack)
	{ 
	  /* Move stack pointer back to pre-adjusted location 
	   */
	  fprintf (out,
		   "\n"
		   "%s" 
		   "/*Move stack pointer back to pre-adjusted location*/\n" 
		   "%s", C_indent, C_indent);
	  C_emit_int_mac (out, L_MAC_SP);
	  fprintf(out, " += %d;\n", mem_usage->local_var_size + mem_usage->reg_swap_size);
	}
    }
  else if ((M_arch == M_WIMS) || (M_arch == M_ARM))
    {
      fprintf(out,
	      "%s/* incoming_parm_size       %d */\n"
	      "%s/* local_var_size           %d */\n"
	      "%s/* swap_size                %d */\n"
	      "%s/* alloc_size               %d */\n"
	      "%s/* out_parm_size            %d */\n"
	      "%s",
	      C_indent, mem_usage->incoming_parm_size,
	      C_indent, mem_usage->local_var_size,
	      C_indent, mem_usage->reg_swap_size,
	      C_indent, mem_usage->alloc_size,
	      C_indent, mem_usage->outgoing_parm_size, C_indent);
      
      if (L_find_attr (fn->attr, "adjusted_memory_stack"))
	{
	  adjusted_memory_stack = 1;
	  scratch_space = 16;
	}
      
      // Amir: I am not sure about this, I should check it again(this can be smaller).
      stack_size += scratch_space + 32 + 16;

      /* Allocate Lcode stack on C stack */
      fprintf (out,
	       "\n"
	       "%s/* Allocate Lcode stack on 'C' stack */\n"
	       "%schar %sstack[%i];\n",
	       C_indent, C_indent, C_prefix, stack_size);

      /* Set up stack pointers according to stack frame layout 
       * specified in comment at top of this function 
       */
      fprintf (out,
	       "\n"
	       "%s/* Move SP to just after OP space and force SP "
	       "to be double aligned */\n" "%s", C_indent, C_indent);
      C_emit_int_mac (out, L_MAC_SP);
      
      fprintf (out, " = ((%s)&%sstack[%i]) & ~(7);\n",
	       C_native_machine_ctype_str, C_prefix,
	       mem_usage->outgoing_parm_size + scratch_space + 32);  

      fprintf (out, "\n"
	       "%s/* Set up rest of SP aliases relative to SP*/\n", C_indent);

      fprintf (out, "%s", C_indent);
      C_emit_int_mac (out, L_MAC_OP);
      fprintf (out, " = ");
      C_emit_int_mac (out, L_MAC_SP);
      fprintf (out, " - %d;\n", mem_usage->outgoing_parm_size + scratch_space);

      fprintf (out, "%s", C_indent);
      C_emit_int_mac (out, L_MAC_LV);
      fprintf (out, " = ");
      C_emit_int_mac (out, L_MAC_SP);
      fprintf (out, " + %i;\n",
	       mem_usage->alloc_size + mem_usage->reg_swap_size);

      {
	L_Attr *va_attr;

	if (!(va_attr = L_find_attr (fn->attr, "VARARG")))
	  {
	    fprintf (out, "%s", C_indent);
	    C_emit_int_mac (out, L_MAC_IP);
	    fprintf (out, " = ");
	    C_emit_int_mac (out, L_MAC_SP);
	    fprintf (out, " + %i;\n",
		     mem_usage->local_var_size + mem_usage->reg_swap_size +
		     mem_usage->alloc_size + 16);
	  }
	else
	  {
	    int va_indx = va_attr->field[0]->value.i;
	    fprintf (out, "%s", C_indent);
	    C_emit_int_mac (out, L_MAC_IP);
	    fprintf (out, " = ");
	    fprintf (out, "(long)(&_EM_p0) + 16;\n");
	  }
      }

      if (adjusted_memory_stack)
	{ 
	  /* Move stack pointer back to pre-adjusted location 
	   */
	  fprintf (out,
		   "\n"
		   "%s" 
		   "/*Move stack pointer back to pre-adjusted location*/\n" 
		   "%s", C_indent, C_indent);
	  C_emit_int_mac (out, L_MAC_SP);
	  fprintf(out, " += %d;\n", mem_usage->local_var_size + mem_usage->reg_swap_size);
	}
    }
  else if (M_arch == M_TAHOE)
    {
      if ((L_find_attr (fn->attr, "VARARG")))
	{
	  static int vararg_incl_printed = 0;

	  if (extern_out && !vararg_incl_printed)
	    {
	      fprintf (extern_out, "#include<varargs.h>\n");
	      vararg_incl_printed = 1;
	    }
	  fprintf (out, "\n%sva_list ap;\n", C_indent);
	}

      /* Allocate Lcode stack on C stack */
      fprintf (out,
	       "\n"
	       "%s/* Allocate Lcode stack on 'C' stack */\n"
	       "%schar %sstack[%i];\n",
	       C_indent, C_indent, C_prefix, stack_size + 16 + 64);

      /* Set up stack pointers according to stack frame layout 
       * specified in comment at top of this function 
       */
      fprintf (out,
	       "\n"
	       "%s/* Move SP to just after OP space and force SP "
	       "to be double aligned */\n", C_indent);

      fprintf (out,
	       "%s/* incoming_parm_size       %d */\n"
	       "%s/* local_var_size           %d */\n"
	       "%s/* swap_size                %d */\n"
	       "%s/* alloc_size               %d */\n"
	       "%s/* out_parm_size(+ scratch) %d */\n"
	       "%s/* Lemulate reg overlay     64 */\n"
	       "%s",
	       C_indent, mem_usage->incoming_parm_size,
	       C_indent, mem_usage->local_var_size,
	       C_indent, mem_usage->reg_swap_size,
	       C_indent, mem_usage->alloc_size,
	       C_indent, mem_usage->outgoing_parm_size, C_indent, C_indent);

      C_emit_int_mac (out, L_MAC_SP);

      fprintf (out, " = ((%s)&%sstack[%i]) & ~(15);\n",
	       C_native_machine_ctype_str,
	       C_prefix, mem_usage->outgoing_parm_size + 16 + 64);

      fprintf (out, "\n"
	       "%s/* Set up rest of SP aliases relative to SP*/\n", C_indent);

      /* OP points top of stack */
      fprintf (out, "%s", C_indent);
      C_emit_int_mac (out, L_MAC_OP);
      fprintf (out, " = ((%s)&%sstack[64]) & ~(15);\n",
	       C_native_machine_ctype_str, C_prefix);

      fprintf (out, "%s", C_indent);
      C_emit_int_mac (out, L_MAC_LV);
      fprintf (out, " = ");
      C_emit_int_mac (out, L_MAC_SP);
      fprintf (out, " + %i;\n",
	       mem_usage->reg_swap_size + mem_usage->alloc_size);

      fprintf (out, "%s", C_indent);

      /* 
       * Adjust IP for IA-64.  For varrags functions,
       * IP will be in a different "virtual" location
       * based on how many of the vararg parameters must be
       * spilled to the stack.
       * JWS 20010129
       */

      {
	L_Attr *va_attr;
	int va_indx, ip_ofst;

	ip_ofst = mem_usage->local_var_size + mem_usage->reg_swap_size +
	  mem_usage->alloc_size + mem_usage->outgoing_parm_size;

	if ((va_attr = L_find_attr (fn->attr, "VARARG")))
	  {
	    va_indx = va_attr->field[0]->value.i;

	    if (va_indx < 2)
	      ip_ofst += 64;
	    else if (va_indx < 4)
	      ip_ofst += 48;
	    else if (va_indx < 6)
	      ip_ofst += 32;
	    else
	      ip_ofst += 16;

	    fprintf (out, "va_start(ap);\n%s", C_indent);
	    C_emit_int_mac (out, L_MAC_IP);
	    fprintf (out, " = (ulonglong)ap + %d;\n",
		     (6 - va_indx) << 3);
	  }
	else
	  {
	    /* Normal overflow args */
	    ip_ofst += 0;
	    C_emit_int_mac (out, L_MAC_IP);
	    fprintf (out, " = ((%s)&%sstack[%i]) & ~(15);\n",
		     C_native_machine_ctype_str,
		     C_prefix, ip_ofst + 16 + 64);
	  }
      }
    }
  else
    L_punt ("C_emit_stack_setup: Unsupported architecture %d\n", M_arch);

  return;
}

/* Emit incoming parameter patchup code (sets up parameter
 * registers in values in IP space (i.e., the stack))
 */
void
C_emit_incoming_parm_patchup (FILE * out, L_Func * fn)
{
  char cast_buf[TYPE_BUF_SIZE];
  char return_type_buf[TYPE_BUF_SIZE];
  char all_parm_type_buf[TYPE_BUF_SIZE];
  char parm_type_buf[TYPE_BUF_SIZE];
  char *parse_ptr;
  char *cast_ptr = NULL;
  int param_index, param_reg_index, pr;
  L_Cb *cb;
  L_Oper *op;
  L_Operand *ret_reg, *parm_reg = NULL;
  L_Attr *tr_attr;
  L_Attr *tmo_attr;
  L_Attr *tmso_attr;
  L_Attr *trse_attr;
  int num_thru_reg, num_thru_mem, max_copied_struct, thru_reg = 0, parm_type;
  int mem_offset = 0, struct_offset, tmp_offset;
  int ret_type;
  int struct_start_param = 0, struct_end_param = 0;
  int max_reg_struct, use_trse;
  int num_llong_thru_reg = 0;

  /* Get the return type and parameter specifiers from the function 
   * attribute.  Use smallest buffers for these, since buf size is checked.
   */
  L_get_call_info (fn, NULL, fn->attr, return_type_buf, all_parm_type_buf,
		   sizeof (return_type_buf));

  /* Get tr attribute and determine number of parameters thru register */
  tr_attr = L_find_attr (fn->attr, "tr");
  if (tr_attr != NULL)
    {
      num_thru_reg = tr_attr->max_field;
    }
  else
    {
      num_thru_reg = 0;
    }

  /* Get tmo attribute and determine number of parameters thru memory */
  tmo_attr = L_find_attr (fn->attr, "tmo");
  if (tmo_attr != NULL)
    {
      num_thru_mem = tmo_attr->max_field;
    }
  else
    {
      num_thru_mem = 0;
    }

  /* Get tmso attribute (location in incoming parameter space
   * where the "copies of structures" are expected by Lcode)
   */
  tmso_attr = L_find_attr (fn->attr, "tmso");
  if (tmso_attr != NULL)
    {
      max_copied_struct = tmso_attr->max_field;
    }
  else
    {
      max_copied_struct = 0;
    }


#ifdef IT64BIT
  /* Get the trse attribute (for each struct two fields specify the
   * starting and ending param used by the struct)
   */
  trse_attr = L_find_attr (fn->attr, "trse");
  if (trse_attr != NULL)
    {
      max_reg_struct = trse_attr->max_field / 2;
    }
  else
    {
      max_reg_struct = 0;
    }
#endif

  /* Print out comment on what we are doing */
  fprintf (out,
	   "\n%s/* Setting up Lcode parameter space from "
	   "incoming parameters */\n", C_indent);

  /* Go through and handle each parameter (parsing all_parm_type_buf) */
  parse_ptr = all_parm_type_buf;
  param_index = 1;
  param_reg_index = 1;
  use_trse = 0;
  while (*parse_ptr != 0)
    {
      /* Get the next parameter type */
      L_get_next_param_type (parm_type_buf, &parse_ptr);

      /* If vararg, no more parameters need to be set up for this func */
      if (strcmp (parm_type_buf, "vararg") == 0)
	{
	  break;
	}
      
      /* Is this parameter thru register ? */
      if (param_reg_index <= num_thru_reg + num_llong_thru_reg)
	{
	  /* Yes, thru register */
	  thru_reg = 1;

	  /* Make sure the operand is really there and a macro */
	  if (!L_is_macro (tr_attr->field[param_reg_index - 1]))
	    {
	      L_print_attr (stderr, tr_attr);
	      fprintf (stderr, "\n");
	      L_punt ("C_emit_incoming_parm_patchup: "
		      "Invalid tr_attr field for parm %i!", param_reg_index);
	    }
	  parm_reg = tr_attr->field[param_reg_index - 1];
	}

      /* Otherwise, is this parameter thru memory ? */
      else if ((param_reg_index - num_thru_reg - num_llong_thru_reg) <= num_thru_mem)
	{
	  /* Yes, thru memory */
	  thru_reg = 0;

	  /* Make sure the operand is really there and a int constant */
	  if (!L_is_int_constant
	      (tmo_attr->field[param_reg_index - num_thru_reg - num_llong_thru_reg - 1]))
	    {
	      L_print_attr (stderr, tmo_attr);
	      fprintf (stderr, "\n");
	      L_punt ("C_emit_incoming_parm_patchup: "
		      "Invalid tmo_attr field for parm %i!", param_index);
	    }
	  mem_offset =
	    ITicast (tmo_attr->
		     field[param_reg_index - num_thru_reg - num_llong_thru_reg - 1]->value.i);
	}

      /* Otherwise, punt */
      else
	{
	  if (tr_attr != NULL)
	    {
	      L_print_attr (stderr, tr_attr);
	      fprintf (stderr, "\n");
	    }
	  else
	    {
	      fprintf (stderr, "No tr_attr found!\n");
	    }
	  if (tmo_attr != NULL)
	    {
	      L_print_attr (stderr, tmo_attr);
	      fprintf (stderr, "\n");
	    }
	  else
	    {
	      fprintf (stderr, "No tmo_attr found!\n");
	    }
	  L_punt ("C_emit_incoming_parm_patchup: func '%s'\n"
		  "parameter %i overruns \n"
		  "num_thru_reg (%i) and num_thru_mem (%i)!\n"
		  "Based on 'tr' and 'tmo' attributes!",
		  fn->name, param_index, num_thru_reg, num_thru_mem);
	}

      /* Handle thru-register case */
      if (thru_reg)
	{

	  /* Print opening indent */
	  fprintf (out, "%s", C_indent);

	  /* Convert type into Lcode ctype (or CTYPE_STRUCT, if required). */
	  parm_type = L_convert_type_to_ctype (parm_type_buf);

          if(parm_type == L_CTYPE_LLONG) {
            num_thru_reg--;
            num_llong_thru_reg++;
          }

	  /* Handle each CTYPE appropriately, include '=' and 
	   * cast of parameter to this type!
	   */
	  switch (parm_type)
	    {
	    case L_CTYPE_INT:
	      C_emit_operand (out, fn, parm_reg);
	      fprintf (out, " = ((int) %sp%i);\n", C_prefix,
		       (param_index - 1));
	      break;

	    case L_CTYPE_LLONG:
              fprintf(out, "l_emul_llong_buf = (union l_emul_llong_struct *)malloc(sizeof(union l_emul_llong_struct));\n");
              fprintf(out, "%sl_emul_llong_buf[0].x", C_indent);
	      fprintf (out, " = ((long long) %sp%i);\n", C_prefix, (param_index - 1));
              fprintf(out, "%s", C_indent);
              C_emit_int_mac(out, L_MAC_P0 + param_reg_index - 1);
              fprintf(out, " = l_emul_llong_buf[0].parts.lo;\n");
              fprintf(out, "%s", C_indent);
              C_emit_int_mac(out, L_MAC_P0 + param_reg_index);
              fprintf(out, " = l_emul_llong_buf[0].parts.hi;\n");
              fprintf(out, "%sfree(l_emul_llong_buf);\n", C_indent);
	      break;

	    case L_CTYPE_FLOAT:
	      C_emit_operand (out, fn, parm_reg);
	      fprintf (out, " = ((float) %sp%i);\n", C_prefix,
		       (param_index - 1));
	      break;

	    case L_CTYPE_DOUBLE:
	      C_emit_operand (out, fn, parm_reg);
	      fprintf (out, " = ((double) %sp%i);\n", C_prefix,
		       (param_index - 1));
	      break;

	      /* Copy structure to expected location on IP stack,
	       * and then point the register parameter at this location.
	       * Uses tmso to determine expected location on IP stack.
	       */
	    case CTYPE_STRUCT:
	      /* Make sure have struct offset in tmso attribute */
	      if ((param_index > max_copied_struct) ||
		  (!L_is_int_constant (tmso_attr->field[param_index - 1])))
		{
		  if (tmso_attr != NULL)
		    {
		      L_print_attr (stderr, tmso_attr);
		      fprintf (stderr, "\n");
		    }
		  L_punt ("C_emit_incoming_parm_patchup: "
			  "Invalid tmso_attr field for parm %i!",
			  param_index);
		}

	      /* Get struct offset from tmso attribute */
	      struct_offset =
		ITicast (tmso_attr->field[param_index - 1]->value.i);

#ifdef IT64BIT
	      if (!trse_attr ||
		  !L_is_int_constant (trse_attr->field[(param_index - 1) * 2])
		  ||
		  !L_is_int_constant (trse_attr->field
				      [(param_index - 1) * 2 + 1]))
		{
		  struct_start_param = 1;
		  struct_end_param = 0;
		  use_trse = 0;
		}
	      else
		{
		  use_trse = 1;
		  struct_start_param =
		    trse_attr->field[(param_index - 1) * 2]->value.i;
		  struct_end_param =
		    trse_attr->field[(param_index - 1) * 2 + 1]->value.i;
		}
#endif

	      /*
	       * First, set parameter register to stack location
	       * where copy of structure is expected.
	       */

#ifdef IT64BIT
	      if (!use_trse)
		{
#endif
		  fprintf (out, "%s", C_indent);
		  C_emit_operand (out, fn, parm_reg);
		  fprintf (out, " = ");
		  C_emit_int_mac (out, L_MAC_IP);
		  fprintf (out, " + %i;\n", struct_offset);
#ifdef IT64BIT
		}
#endif
	      /* 
	       * Second, then copy the structure to that expected
	       * location (using parm register just set).
	       */
	      /* Convert parm type to formatted string using C conventions,
	       * Passing "*" as the parameter name makes it appropriate 
	       * for a cast to a pointer to parm type. -ITI/JCG 4/99
	       */
	      L_convert_type_to_C_format (cast_buf, parm_type_buf, "*");

#ifdef IT64BIT
	      if (!use_trse)
		{
#endif
		  fprintf (out, "%s*((%s)", C_indent, cast_buf);
		  C_emit_operand (out, fn, parm_reg);
		  fprintf (out, ") = %sp%i;\n", C_prefix, (param_index - 1));

#ifdef IT64BIT
		}
	      else
		{
		  /* Struct actually passed through the parm registers */
		  /* To mimic this, First copy struct in stack */
		  fprintf (out, "%s*((%s)(", C_indent, cast_buf);
		  C_emit_int_mac (out, L_MAC_IP);
		  fprintf (out, " + %i", struct_offset);
		  fprintf (out, ")) = %sp%i;\n", C_prefix, (param_index - 1));

		  /* This read values out into parm regs */
		  tmp_offset = struct_offset;
                  for (pr = struct_start_param; pr <= struct_end_param; pr++)
                  {
                    fprintf (out, "%s", C_indent);
                    C_emit_int_mac (out, L_MAC_P0 + pr);
                    /* Amir: size of operands is different
                       in arm. Therefore I changed longlong to long*/
                    if (M_arch == M_ARM)
                      fprintf (out, " = *((long*)(");
                    else
                      fprintf (out, " = *((longlong*)(");
                    C_emit_int_mac (out, L_MAC_IP);
                    fprintf (out, " + %i));\n", tmp_offset);
                    /* Amir: size of operands is different
                       in arm. Therefore I changed 8 to 4*/
                    if (M_arch == M_ARM)
                      tmp_offset += 4;
                    else
                      tmp_offset += 8;
                  }
		}
#endif

	      break;

	    case L_CTYPE_VOID:
	      L_punt ("C_emit_incoming_parm_patchup: "
		      "parm %i VOID unexpected!", param_index);
	      break;


	    default:
	      L_punt ("C_emit_incoming_parm_patchup: "
		      "parm %i, unknown parm type %i!",
		      param_index, parm_type);
	    }
	}

      /* Otherwise, handle thru memory case */
      else
	{

	  /* Convert type into Lcode ctype (or CTYPE_STRUCT, if required). */
	  parm_type = L_convert_type_to_ctype (parm_type_buf);

	  /* Print Lcode cast, handle each CTYPE appropriately */
	  switch (parm_type)
	    {
	    case L_CTYPE_INT:
	      cast_ptr = "int";
	      break;
	    case L_CTYPE_LLONG:
	      cast_ptr = "long long";
	      break;
	    case L_CTYPE_FLOAT:
	      cast_ptr = "float";
	      break;

	    case L_CTYPE_DOUBLE:
	      cast_ptr = "double";
	      break;

	    case L_CTYPE_VOID:
	      L_punt ("C_emit_incoming_parm_patchup: "
		      "parm %i VOID unexpected!", param_index);
	      break;

	    case CTYPE_STRUCT:
	      /* Handled below, set cast_ptr to NULL as sanity check */
	      cast_ptr = NULL;
	      break;

	    default:
	      L_punt ("C_emit_incoming_parm_patchup: "
		      "parm i, unknown parm type %i!",
		      param_index, parm_type);
	    }

	  /* Handle normal (non-structure copy) case */
	  if (parm_type != CTYPE_STRUCT)
	    {
	      /* Print out dereference, cast, and start address expression */
	      fprintf (out, "%s*((%s *)(", C_indent, cast_ptr);

	      /* Print out IP parameter space pointer */
	      C_emit_int_mac (out, L_MAC_IP);

	      /* Add offset into IP space and rest of assignment */
	      fprintf (out, " + %i)) = ((%s) %sp%i);\n", mem_offset,
		       cast_ptr, C_prefix, (param_index - 1));
	    }

	  /* Handle structure copy case */
	  else
	    {
	      /* Convert parm type to formatted string using C conventions,
	       * Passing "" as the parameter name makes it appropriate 
	       * for a cast
	       */
	      L_convert_type_to_C_format (cast_buf, parm_type_buf, "");

	      /* If don't have struct offset in tmso attribute, then
	       * struct was passed directly thru memory (no indirection).
	       * Handle both cases!
	       */
	      if ((param_index > max_copied_struct) ||
		  (!L_is_int_constant (tmso_attr->field[param_index - 1])))
		{
		  /* 
		   * Directly thru memory case!
		   *
		   * Copy structure directly to parameter space,
		   */
		  /* Print out dereference, cast, and start address expr */
		  fprintf (out, "%s*((%s *)(", C_indent, cast_buf);

		  /* Print out IP parameter space pointer */
		  C_emit_int_mac (out, L_MAC_IP);

		  /* Add offset into IP space and rest of assignment */
		  fprintf (out, " + %i)) = %sp%i;\n", mem_offset,
			   C_prefix, (param_index - 1));
		}
	      else
		{
		  /*
		   * Indirectly thru memory.  Copy structure to
		   * specified structure offset and store this
		   * address in parameter space.
		   */
		  /* Get struct offset from tmso attribute */
		  struct_offset =
		    ITicast (tmso_attr->field[param_index - 1]->value.i);

		  /*
		   * First, set parameter register to stack location
		   * where copy of structure is expected.
		   */
		  /* Print out dereference, cast, and start address expr */
		  fprintf (out, "%s*((%s *)(", C_indent,
			   C_native_machine_ctype_str);

		  /* Print out IP parameter space pointer */
		  C_emit_int_mac (out, L_MAC_IP);

		  /* Add offset into IP space and '=' */
		  fprintf (out, " + %i)) = ", mem_offset);

		  C_emit_int_mac (out, L_MAC_IP);
		  fprintf (out, " + %i;\n", struct_offset);

		  /* 
		   * Second, then copy the structure to that expected
		   * location.
		   */
		  /* Print out dereference, cast, and start address expr */
		  fprintf (out, "%s*((%s *)(", C_indent, cast_buf);

		  /* Print out IP parameter space pointer */
		  C_emit_int_mac (out, L_MAC_IP);

		  /* Add offset into IP space and rest of assignment */
		  fprintf (out, " + %i)) = %sp%i;\n", struct_offset,
			   C_prefix, (param_index - 1));
		}

	    }
	}

      /* Increment parameter id */
#ifdef IT64BIT
      if (parm_type == CTYPE_STRUCT && use_trse && thru_reg)
	{
	  /* This manages the fact that a single struct
	     param can use multiple param registers */
	  param_reg_index += (struct_end_param - struct_start_param) + 1;
	  param_index++;
	}
      else
#endif
	{
	  param_reg_index++;
	  param_index++;
          if(L_convert_type_to_ctype (parm_type_buf) == L_CTYPE_LLONG && thru_reg)
            param_reg_index++;
	}
    }

  if (param_reg_index <= num_thru_reg)
    {
      /* For some architectures, some params can go
	 through registers. Therefore, some of the varargs
	 parameters might be held in registers but not
	 be in the callinfo */
      /*printf("vararg break but %d-%d to do\n",
	param_reg_index, num_thru_reg);*/

      if (M_arch == M_TAHOE)
	{
	  int ip_ofst = 16 - 8 * (num_thru_reg - param_reg_index + 1);

	  for (;param_reg_index <= num_thru_reg; param_reg_index++)
	    {  
	      /* Print opening indent */
	      fprintf (out, "%s", C_indent);
	  
	      parm_type = tr_attr->field[param_reg_index - 1]->ctype;	  
	      parm_reg = tr_attr->field[param_reg_index - 1];

	      C_emit_operand (out, fn, parm_reg);
	      fprintf (out, "= *((longlong *)(");
	      C_emit_int_mac (out, L_MAC_IP);
	      fprintf (out, " + %i));\n", ip_ofst);

	      ip_ofst += 8;
	    }
	}
      else
	{
	  for (;param_reg_index <= num_thru_reg; param_reg_index++)
	    {  
	      /* Print opening indent */
	      fprintf (out, "%s", C_indent);
	  
	      parm_type = tr_attr->field[param_reg_index - 1]->ctype;	  
	      parm_reg = tr_attr->field[param_reg_index - 1];
	      switch (parm_type)
		{
		case L_CTYPE_INT:
		  C_emit_operand (out, fn, parm_reg);
		  fprintf (out, "= *((int *)(");
		  C_emit_int_mac (out, L_MAC_IP);
		  fprintf (out, " + %i));\n", (param_reg_index-1) * 4);
		  break;

		case L_CTYPE_LLONG:
		  C_emit_operand (out, fn, parm_reg);
		  fprintf (out, "= *((longlong *)(");
		  C_emit_int_mac (out, L_MAC_IP);
		  fprintf (out, " + %i));\n", (param_reg_index-1) * 8);
		  break;
		default:
		  L_punt("C_emit_incoming_parm_patchup: Unexpected ctype\n");
		  break;
		}
	  
	      param_index++;
	    }
	}
    }

  /* Set up Lcode return structure space, if necessary */
  ret_type = L_convert_type_to_ctype (return_type_buf);
  if (ret_type == CTYPE_STRUCT)
    {
      /* When returning a structure (non-pointer), Lcode expects
       * the buffer to write the structure into to be passed
       * into the function in the return value register.
       * 
       * Find the rts to get the return value register and 
       * point this register at the appropriately allocated structure
       * space.
       */
      ret_reg = NULL;
      for (cb = fn->first_cb; (cb != NULL) && (ret_reg == NULL);
	   cb = cb->next_cb)
	{
	  for (op = cb->first_op; op != NULL; op = op->next_op)
	    {
	      /* Have we found a rts? */
	      if ((op->proc_opc == Lop_RTS) || (op->proc_opc == Lop_RTS_FS))
		{
		  /* Yes, get tr or utr (undefined thru reg) attribute  */
		  if (((tr_attr = L_find_attr (op->attr, "tr")) == NULL) &&
		      ((tr_attr = L_find_attr (op->attr, "utr")) == NULL))
		    {
		      fprintf (stderr, "In func %s:\n", fn->name);
		      L_print_oper (stderr, op);
		      L_punt ("C_emit_incoming_parm_patchup: "
			      "Expect tr or utr attr!");
		    }

		  /* Get return register from first field */
		  ret_reg = tr_attr->field[0];

		  /* Sanity check, better be a register or macro */
		  if (!L_is_reg (ret_reg) && !L_is_macro (ret_reg))
		    {
		      L_print_oper (stderr, op);
		      L_punt ("C_emit_incoming_parm_patchup: Unexpected 'tr'"
			      " or 'utr' attr contents!");
		    }

		  /* Break out of loop */
		  break;
		}
	    }
	}

      /* Sanity check, better have found return register! */
      if (ret_reg == NULL)
	{
	  fprintf (stderr, "In func %s:\n", fn->name);
	  L_punt ("C_emit_incoming_parm_patchup: rts not found!");
	}

      /* Print out comment on what we are doing */
      fprintf (out, "\n%s/* Setting up Lcode return space */\n", C_indent);

      /* Set return register to (prefix)return_space, which we assume
       * has been declared for us earlier.
       */
      fprintf (out, "%s", C_indent);
      C_emit_operand (out, fn, ret_reg);
      fprintf (out, " = (%s)&%sreturn_space;\n", C_native_machine_ctype_str,
	       C_prefix);
    }

  fprintf (out, "\n");
}

/* Emit predicate register setup (if any required)  */
void
C_emit_pred_setup (FILE * out, Reg_Usage * fn_scope_reg_usage)
{
  /* If pred_all alias used, initialize it to zero and print
   * out warning about its "unsupported status"
   */
  if (INT_find_symbol (fn_scope_reg_usage->pred_macs_used, L_MAC_PRED_ALL)
      != NULL)
    {
      fprintf (out,
	       "\n"
	       "%s/* Loads/Stores of `pred_all` alias are treated "
	       "as compiler directives*/\n"
	       "%s/* The use of local pred registers makes real "
	       "loads/stores unnecessary.*/\n"
	       "%s", C_indent, C_indent, C_indent);
      C_emit_pred_mac (out, L_MAC_PRED_ALL);
      fprintf (out,
	       " = 0;  /* Set to 0 to prevent random values in trace. */\n");
    }

}

/* Emit the entire function in C code */
void
C_emit_fn (FILE * out, FILE * extern_out, L_Func * fn, Mem_Usage * mem_usage)
{
  L_Cb *cb;
  Reg_Usage *fn_scope_reg_usage;

  /* Lemulate is a Lcode emulator, it cannot handle Mcode.  Make sure
   * the function does not have an 'lhppa_ph1' (Hppa Mcode) attribute.
   */
  if (L_find_attr (fn->attr, "lhppa_ph1") != NULL)
    {
      L_punt ("\n\n"
	      "  Lemulate only supports Lcode!\n"
	      "  Lemulate does not support Lhppa mcode!\n"
	      "\n"
	      "  Use 'gen_probed_hppa' or 'gen_profiled_hppa' instead!\n");
    }

  /* Lemulate is a Lcode emulator, it cannot handle Mcode.  Make sure
   * the function does not have an 'lhpl_pd_ph1' (HPL PD Mcode) attribute.
   */
  if (L_find_attr (fn->attr, "lhpl_pd_ph1") != NULL)
    {
      L_punt ("\n\n"
	      "  Lemulate only supports Lcode!\n"
	      "  Lemulate does not support Lhpl_pd mcode!\n"
	      "\n"
	      "  Use 'gen_probed_lcode' or 'gen_profiled_lcode' on\n"
	      "  Lcode (e.g., .O, .S, .HS) before running\n"
	      "  Lhpl_pd (e.g. .O_tr, .S_tr, .HS_tr) instead!\n");
    }

  /* Check and update info attribute files (if necessary) */
  C_check_info_attributes (fn);

  /* Scan function to build function-scope memory usage info and
   * to allow update of file-scope and program-scope memory usage
   */
  C_add_fn_mem_usage (fn, mem_usage);

  /* Create register usage structure to hold function scope info */
  fn_scope_reg_usage = C_new_reg_usage ("fn_scope");

  /* Scan function to build fn_scope_reg_usage about register usage */
  C_add_fn_reg_usage (fn, fn_scope_reg_usage, NULL);

  /* Emit function header */
  C_emit_fn_declaration (out, fn, mem_usage);

  /* Emit opening '{' */
  fprintf (out, "{\n");

  /* Emit all the function-scope register declarations */
  C_emit_reg_decls (out, fn_scope_reg_usage, C_indent);

  /* Create a mapping table for all strings in the function
   * so that we can create unique pointers for each string.
   * Use global so C_emit_operand can use string mapping. -JCG 2/00
   */
  C_string_map = C_build_string_map (fn, 1);

  /* Emit string mapping declarations */
  C_emit_string_decls (out, C_string_map);

  /* Emit stack space declaration and setup */
  C_emit_stack_setup (out, extern_out, fn, mem_usage);

  /* Emit pred setup */
  C_emit_pred_setup (out, fn_scope_reg_usage);

  /* Emit incoming parameter patchup code (sets up parameter
   * registers in values in IP space (i.e., the stack))
   */
  C_emit_incoming_parm_patchup (out, fn);

  /* Emit trace system setup code, if necessary */
  if (C_insert_probes)
    {
      C_emit_fn_trace_system_setup (out);
    }

  /* If tracing control-flow, trace function entry */
  if (C_trace_control_flow && C_insert_probes)
    {
      /* Pass NULL as op, since this probe should never be predicated */
      C_emit_put_trace_int (out, fn, NULL, "L_TRACE_FN", C_trace_func_id);
    }

  if (C_trace_objects)
    {
      /* Dump stack allocation statement */
      fprintf (out, "   _EM_put_trace(L_TRACE_ASYNCH);\n");
      fprintf (out, "   _EM_put_trace3(L_TRACE_OBJ_STAK,"
	       "%sstack,%d);\n", C_prefix,
	       mem_usage->incoming_parm_size +
	       mem_usage->local_var_size +
	       mem_usage->reg_swap_size + mem_usage->outgoing_parm_size + 8);
    }

  /* SER: Code for HCH MICRO '04: spit out stack memory range */
  if (C_insert_probes && C_custom_profiling && C_trace_mem_addrs)
    {
      fprintf (out, "    _EM_put_trace2(%sstack, %d);\n", C_prefix,
	       mem_usage->incoming_parm_size + mem_usage->local_var_size +
	       mem_usage->reg_swap_size + mem_usage->outgoing_parm_size + 8);
    }

  /* Reset jsr function id counter (for control flow tracing */
  C_trace_jsr_id = 1;


  /* Emit all the cbs in the function */
  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      C_emit_cb (out, fn, cb);
    }

  fprintf (out, "} /* End %s */\n", C_true_name (fn->name));

  /* Delete string mapping table, no free routine needed for ids -JCG 2/00 */
  STRING_delete_symbol_table (C_string_map, NULL);

  /* Set C_string_map to NULL so C_emit_operand will work correctly
   * for data sections.
   */
  C_string_map = NULL;

  /* Free function scope register usage info */
  C_delete_reg_usage (fn_scope_reg_usage);

  /* Increment func id for next function, must not change until finished
   * with all of the C_emit_cb calls
   */
  C_trace_func_id++;
}

/* This routine verifies the base type size and alignment assumptions.
 * Used as a early warning system that the host compiler is not
 * compatible with the assumption being made.
 *
 * Note, if assumed_align is set to -1, only the type size is checked. 
 */
void
C_verify_type_assumptions (char *type_name, int assumed_size,
			   int assumed_align)
{
  int host_size, host_align;

  /* If assumed_align is -1, check only type size */
  if (assumed_align == -1)
    {
      /* Get the host's size and align information for this type */
      host_size = C_read_database_i ("_HT__base_types", type_name, "size");

      /* Verify they are what we expect */
      if (host_size != assumed_size)
	{
	  fprintf (stderr,
		   "\n"
		   "Lemulate's host-compiler assumptions "
		   "violated for '%s'!\n"
		   "\n"
		   "     Assumed size (in bits): %i\n"
		   "      Actual size (in bits): %i\n",
		   type_name, assumed_size, host_size);

	  fprintf (stderr,
		   "\n"
		   "Recommend tweaking host-compiler flags to see "
		   "if it can be\n"
		   "made to match assumptions (i.e., 32-bit mode).\n\n");

	  L_punt ("C_verify_type_assumptions: exiting, "
		  "assumptions violated.");
	}

    }

  /* Otherwise, test both size and alignment */
  else
    {
      /* Get the host's size and align information for this type */
      host_size = C_read_database_i ("_HT__base_types", type_name, "size");
      host_align = C_read_database_i ("_HT__base_types", type_name, "align");

      /* Verify they are what we expect */
      if ((host_size != assumed_size) || (host_align != assumed_align))
	{
	  fprintf (stderr,
		   "\n"
		   "Lemulate's host-compiler assumptions "
		   "violated for '%s'!\n"
		   "\n"
		   "     Assumed size (in bits): %i\n"
		   "      Actual size (in bits): %i\n"
		   "\n"
		   "Assumed alignment (in bits): %i\n"
		   " Actual alignment (in bits): %i\n",
		   type_name, assumed_size, host_size,
		   assumed_align, host_align);

	  fprintf (stderr,
		   "\n"
		   "Recommend tweaking host-compiler flags to see "
		   "if it can be\n"
		   "made to match assumptions (i.e., 32-bit mode).\n\n");

	  L_punt ("C_verify_type_assumptions: exiting, "
		  "assumptions violated.");
	}
    }
  return;
}


/* 08/05/02 REK This function now initializes C_extern_file_name,
 *              C_struct_file_name, and C_include_file_name with absolute
 *              paths.
 */
void
L_gen_code (Parm_Macro_List * command_line_macro_list)
{
  FILE *extern_out = NULL;
  FILE *struct_out = NULL;
  FILE *include_out = NULL;
  FILE *file_list_in = NULL;
  FILE *file_out = NULL;
  FILE *layout_in = NULL;
  FILE *header_out = NULL;
  FILE *file_init_out = NULL;
  Mem_Usage *mem_usage;
  L_Data *data;
  L_Func *fn;
  int token_type;
  int fn_global;
  Mbuf *input_name_mbuf, *output_name_mbuf, *header_name_mbuf;
  /* 08/05/02 REK A buffer to hold the current directory */
  char cwd[MAXPATHLEN];

  if (!getcwd(cwd, MAXPATHLEN))
  {
      /* REK */
      fprintf(stderr, "tools/Lemulate/l_emul.c:%d Could not get current directory.\n", __LINE__);
  }

  /* 08/05/02 REK Initialize the header paths. */
  strncpy (C_extern_file_name, cwd, MAXPATHLEN);
  strncpy (C_struct_file_name, cwd, MAXPATHLEN);
  strncpy (C_include_file_name, cwd, MAXPATHLEN);
  strcat (C_extern_file_name, "/_EM_extern.h");
  strcat (C_struct_file_name, "/_EM_struct.h");
  strcat (C_include_file_name, "/_EM_include.h");

  /* Load parameters for Lemulate */
  L_load_parameters (L_parm_file, command_line_macro_list,
		     "(Lemulate", L_read_parm_Lemulate);

  /* Load the host layout database for this benchmark */
  if ((layout_in = fopen (C_layout_database_name, "r")) == NULL)
    {
	/* REK */
	printf("tools/Lemulate/l_emul.c:%d Could not open file %s\n", __LINE__, C_layout_database_name);
      L_punt ("Lemulate: Unable to open '%s' for reading!\n",
	      C_layout_database_name);
    }

  /* Read in the layout database */
  C_layout_database = MD_read_md (layout_in, C_layout_database_name);

  /* Done with file, close it */
  fclose (layout_in);

  /* Make sure the host compiler matches type size assumptions 
   * this Lcode to C emulator assumes.  It these checks fail for your 
   * host compiler, I recommend seeing if your host compiler can be 
   * put into '32-bit' mode.  If not, the Lemulate and the IMPACT 
   * compiler needs to be enhanced to handle your host-compiler's 
   * configuration (which will probably be non-straighforward and 
   * is definately not recommended).
   * 
   * Most of the data-layout assumptions are exercised by 
   * the 'type_test' benchmark -ITI/JCG 3/99
   */
  C_verify_type_assumptions ("char", 8, -1);
  C_verify_type_assumptions ("short", 16, -1);
  C_verify_type_assumptions ("int", 32, -1);
#if IT64BIT
  C_verify_type_assumptions ("longlong", 64, -1);
#else
  C_verify_type_assumptions ("long", 32, -1);
#endif
  if (M_native_int_register_ctype () == L_CTYPE_INT)
    C_verify_type_assumptions ("void *", 32, -1);
  else
    C_verify_type_assumptions ("void *", 64, -1);

  /* I believe Lemulate can handle different float sizes but IMPACT
   * cannot (as of 3/99).  So do sanity check here.
   */
  C_verify_type_assumptions ("float", 32, -1);
  C_verify_type_assumptions ("double", 64, -1);

  /* Set machine register ctype MCM 7/2000 */
  C_native_machine_ctype = M_native_int_register_ctype ();
  if (C_native_machine_ctype == L_CTYPE_INT)
    {
      strcpy (C_native_machine_ctype_str, "int");
      strcpy (C_native_machine_ctype_unsigned_str, "unsigned int");
    }
  else if (C_native_machine_ctype == L_CTYPE_LLONG)
    {
      strcpy (C_native_machine_ctype_str, "longlong");
      strcpy (C_native_machine_ctype_unsigned_str, "ulonglong");
    }
  else
    L_punt ("gen_code: Unsupported native machine register size ctype %d.\n",
	    C_native_machine_ctype);

  /* Verify that Lcode has at least 2 dest, 2 pred, and 3 source
   * operands specified.
   */
  if (L_max_dest_operand < 2)
    {
      L_punt ("Lemulate expects L_max_dest_operand (%i) >= 2!",
	      L_max_dest_operand);
    }
  if (L_max_pred_operand < 2)
    {
      L_punt ("Lemulate expects L_max_pred_operand (%i) >= 2!",
	      L_max_pred_operand);
    }
  if (L_max_src_operand < 3)
    {
      L_punt ("Lemulate expects L_max_src_operand (%i) >= 3!",
	      L_max_src_operand);
    }

#if 0
  /* Make sure we can open the file_list (if not stdin) */
  if (strcmp (L_input_file, "stdin") == 0)
    {
      file_list_in = stdin;
    }
  else if ((file_list_in = fopen (L_input_file, "r")) == NULL)
    {
      L_punt ("Unable to open file_list '%s'!", L_input_file);
    }
#endif

  /* Create Mbuf's for holding file names */
  input_name_mbuf = create_Mbuf ();
  output_name_mbuf = create_Mbuf ();
  header_name_mbuf = create_Mbuf ();

  /* Create structure for tracking program-wide and function
   * memory usage (includes stack, parms thru memory, etc).
   */
  mem_usage = C_new_mem_usage ("main_mem_usage");

  if (!C_cfile_only)
    {
      /* Open header file for extern variable declarations */
      if ((extern_out = fopen (C_extern_file_name, "w")) == NULL)
	{
	  L_punt ("Unable to open '%s' for writing!", C_extern_file_name);
	}

      /* Open header file for structure declarations */
      if ((struct_out = fopen (C_struct_file_name, "w")) == NULL)
	{
	  L_punt ("Unable to open '%s' for writing!", C_struct_file_name);
	}
      
      /* Open header file for ancillary include files */
      if ((include_out = fopen (C_include_file_name, "w")) == NULL)
	{
	    L_punt ("Unable to open '%s' for writing!", C_include_file_name);
	}

      /* Add header to struct header file */
      fprintf (struct_out,
	       "/* Structure definitions (including those created by "
	       "Lemulate, if any) */\n" "\n");
      
      /* Add 64-bit typedef to extern file */
      C_emit_typedefs (struct_out);

      /* Add header to extern file which includes the struct header file */
      fprintf (extern_out, "#include \"%s\"\n" "\n", C_struct_file_name);


      fprintf (extern_out, "/* Externs for program data */\n");
      
      fprintf (include_out, "/* Ancillary include files */\n");


      /****************************************************************
       * Read in all files once to build up the prototypes
       ****************************************************************/
      if (strcmp (L_input_file, "stdin") == 0)
	{
	  L_punt ("Lemulate does not work from stdin. Make a file list\n");
	}
      
      if ((file_list_in = fopen (L_input_file, "r")) == NULL)
	{
	  L_punt ("Unable to open file_list '%s'!", L_input_file);
	}
      
      L_init_call_info ();
      while (C_get_next_input_name (file_list_in,
				    input_name_mbuf,
				    output_name_mbuf, header_name_mbuf))
	{
	  /* Open the input file in input_name_mbuf */
	  L_open_input_file (input_name_mbuf->buf);
	  
	  while (L_get_input () != L_INPUT_EOF)
	    {
	      if (L_token_type == L_INPUT_FUNCTION)
		{
		  L_define_fn_name (L_fn->name);
		  L_collect_call_info (L_fn);
		  L_delete_func (L_fn);
		}
	      else
		{
		  /* Do nothing with data */
		  L_delete_data (L_data);
		}
	    }
	  
	  /* Close the input file */
	  L_close_input_file (input_name_mbuf->buf);
	}
      fclose (file_list_in);

      /* Open file in which actual main goes that 
	 calls emulated main */
      /* 11/19/02 REK Changing data_init.c to _EM_data_init.c */
      if ((file_init_out = fopen ("_EM_data_init.c", "w")) == NULL)
	{
	  L_punt ("Unable to open output file '%s'!", "_EM_data_init.c");
	}
    }
  else 
    {
      /* Open header file for extern variable declarations */
      if ((extern_out = fopen ("Trash", "w")) == NULL)
	{
	  L_punt ("Unable to open '%s' for writing!", C_extern_file_name);
	}

      /* Open header file for structure declarations */
      struct_out = extern_out;
      include_out = extern_out;
      header_out = extern_out;
    }
  
  /* Reopen for next phase */
  if ((file_list_in = fopen (L_input_file, "r")) == NULL)
    {
      L_punt ("Unable to open file_list '%s'!", L_input_file);
    }
  
  /****************************************************************
   * Now process the functions individually
   ****************************************************************/
  
  /* While there are input files to process, emit emulation code */
  while (C_get_next_input_name (file_list_in,
				input_name_mbuf,
				output_name_mbuf, header_name_mbuf))
    {
      if (!C_cfile_only)
	{
	  /* Remove any file-scope information collected for the last file */
	  C_init_file_scope_mem_usage (mem_usage);
	}
      
      /* Open the input file in input_name_mbuf */
      L_open_input_file (input_name_mbuf->buf);

      /* Open the output file for this input file */
      if ((file_out = fopen (output_name_mbuf->buf, "w")) == NULL)
	{
	  L_punt ("Unable to open output file '%s'!", output_name_mbuf->buf);
	}

      if (!C_cfile_only)
	{
	  /* Open the file-specific header file for this input file */
	  if ((header_out = fopen (header_name_mbuf->buf, "w")) == NULL)
	    {
	      L_punt ("Unable to open output file '%s'!", output_name_mbuf->buf);
	    }
	  
	  /* Add comment to file-specific header file */
	  fprintf (header_out,
		   "/* Include extern variable defs, struct defs, and "
		   "Lemulate macros */\n");
	  fprintf (header_out, "#include \"%s\"\n", C_extern_file_name);
	}

      /* Add include the file-specific header to head of 
       * every C file generated 
       */
      fprintf (file_out,
	       "/* Include global externs and "
	       "file-specific function prototypes */\n");
      fprintf (file_out, "#include \"%s\"\n", header_name_mbuf->buf);


      /* Process all data and functions within the file.
       * Use wrapper C_get_input() instead of L_get_input(), so
       * C_peek_input() can be used in C_emit_reserve_data().
       * Pass NULL to C_get_input() to indicate read from input stream.
       */
      while ((token_type = C_get_input (NULL, mem_usage)) != L_INPUT_EOF)
	{
	  if (token_type == L_INPUT_FUNCTION)
	    {
	      /* Get function from global variable for clarity.
	       * Also, printing out jump tables will destroy
	       * contents of C_fn.
	       */
	      fn = C_fn;

	      /* Defining function name required by Lcode */
	      L_define_fn_name (fn->name);

#if 0
	      /* Build up call_info database using the info in this func */
	      L_collect_call_info (fn);
#endif

	      /* First, emit code for any hashing jump tables for this 
	       * function.  Done here since really data and needs
	       * all the parameters C_emit_data needs.
	       *
	       * Note: This destroys the contents of C_fn, C_data, etc.
	       *       This also destroys mem_usage state for the
	       *       function name, so we must preserve whatever
	       *       we care about! 
	       */
	      if (fn->jump_tbls != NULL)
		{
		  /* Save static/global state for function */
		  fn_global = C_is_label_global (mem_usage,
						 C_true_name (fn->name));

		  if (L_jump_tables_have_changes (fn))
		    L_regenerate_all_jump_tables (fn);

		  C_emit_fn_jump_tables (file_out, extern_out,
					 struct_out, fn, mem_usage);

		  /* If function was global, restore state for label */
		  if (fn_global)
		    {
		      C_make_label_global (mem_usage, C_true_name (fn->name));
		    }
		}

	      /* Emit function in C */
	      C_emit_fn (file_out, extern_out, fn, mem_usage);

	      /* Free the lcode, finished with it.  Function *required*
	       * to be also pointed to by L_fn!
	       */
	      L_fn = fn;
	      L_delete_func (fn);
	      L_fn = NULL;
	    }
	  else
	    {
	      /* Get the 'data' from C_data since C_emit_data may
	       * destroy the value in C_data.
	       */
	      data = C_data;

	      /* Emit data in C (last arg NULL to indicate data is
	       * from input stream) 
	       */
	      C_emit_data (file_out, extern_out, struct_out,
			   data, mem_usage, NULL);

	      /* Free the data, finished with it */
	      L_delete_data (data);
	    }
	}

      if (!C_cfile_only)
	{
	  /* Deduce new global scope labels from those locally referenced
	   * but not locally defined.
	   */
	  C_deduce_program_scope_labels (mem_usage);
	  
	  /* Use libproto library to deduce "valid" prototypes for every
	   * called function in this file.  Pass the table of function (code)
	   * labels used, so that prototypes for them can also be deduced,
	   * if necessary.
	   *
	   * Turn off warnings about varargs deductions about unknown
	   * varargs functions and use the info on known varargs functions 
	   * (like printf()) to make the prototypes look better.  
	   * Both flags are optional.  The code should work either way!
	   */
	  L_deduce_prototypes (mem_usage->file_scope_code_labels_used,
			       DP_SILENT | DP_USE_KNOWN_VARARGS_INFO);
	  
	  /* Print out these prototypes to the file-specific header file */
	  fprintf (header_out,
		   "\n"
		   "/* Deduced prototypes (may not detect varargs or proper "
		   "types from usage) */\n");
	  
	  /* For those prototypes where the parameter types are guesses
	   * (no actual function source), use K&R-C style prototypes
	   * (no parameter types listed).
	   */
	  L_print_C_prototypes (header_out, C_ansi_c_mode, 1 /*0 */ );
	  
#if 0
	  /* Free the call info data created */
	  L_delete_call_info ();
#endif
	  
	  /* Add non-trapping load prototypes to file-specific header */
	  C_emit_non_trapping_load_prototypes (header_out);
	  
	  /* If just finished processing the last input file, 
	   * add any data initialization code we need to create to the 
	   * end of this file.
	   */
	  if (C_is_at_EOF (file_list_in))
	    {
	      /* Print out 'fake' externs for data labels that
	       * were not defined in the program.
	       */
	      C_emit_undefined_label_extern (extern_out, mem_usage);
	      
	      /* Emit function that calls all the data initialization 
	       * functions (if necessary).
	       */
	      C_emit_data_init_function (file_init_out, extern_out, mem_usage);
	      
	      /* Close the init and calling of main file */
	      fclose(file_init_out);
	      file_init_out = NULL;
	    }

	  /* Close the header file */
	  fclose (header_out);
	}

      /* Close the output file */
      fclose (file_out);

      /* Close the input file */
      L_close_input_file (L_input_file);
    }

  L_delete_call_info ();

  if (!C_cfile_only)
    {
      /* If inserting probes, insert trace system prototypes and macros */
      if (C_insert_probes)
	{
	  C_emit_trace_system_prototypes (extern_out);
	}


      if (M_arch == M_TAHOE)
	fprintf (extern_out, "#include<alloca.h>\n");

      /* This call is not necessary if detailed data types have been
       * passed down in Lcode (now the default case).  Provide as
       * a safety net since code already works and the structures
       * can be printed out from the layout database, if necessary. -ITI/JCG 4/99
       */
      C_emit_database_struct_union_defs (struct_out, mem_usage);
      
#ifdef INTRINSICS
      /* Call the Intrinsic exit hook to do what it needs to the 
       * include files.
       */
      Lem_Intrinsic_IncludeHook (include_out);
#endif
      
      /* Close the extern, struct, and include header files */
      fclose (extern_out);
      fclose (struct_out);
      fclose (include_out);
    }

  /* Free mem_usage tracking structure */
  C_delete_mem_usage (mem_usage);

  /* Free the file name mbufs */
  free_Mbuf (input_name_mbuf);
  free_Mbuf (output_name_mbuf);
  free_Mbuf (header_name_mbuf);
}
