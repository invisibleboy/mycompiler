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
 *      File: l_emul_util.c
 *      Authors: IMPACT Technologies, Inc. (Jake Janovetz)
 *      Creation Date:  June 1999
 *
 *      This file performs intrinsic emulation support for Lemulate.
 *      Intrinsic operations get converted to function calls which
 *      perform the equivalent function.  The intrinsic emulatiointrinsicn 
 *      functions are external and are linked with the emulation code.
 *
\*****************************************************************************/
/* 07/12/02 REK Adding temporary debugging output */
/* 08/05/02 REK Changing C_get_next_input_name so that it returns the header
 *              name as an absolute path.  Since the input file name may
 *              contain a directory and the header will be in the same 
 *              directory, we need to strip the directory from the header
 *              file name so that the compiler doesn't try to look in a
 *              non-existant directory.  To make sure that we can easily
 *              open the header, we simply use an absolute path.
 */

/* 10/29/02 REK Adding config.h */
#include <config.h>
/* 08/05/02 REK Including sys/param.h to get MAXPATHLEN */
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

/* Read in and process parameters */
void
L_read_parm_Lemulate (Parm_Parse_Info * ppi)
{
  L_read_parm_b (ppi, "generate_info_files", &C_generate_info_files);
  L_read_parm_b (ppi, "ansi_c_mode", &C_ansi_c_mode);
  L_read_parm_b (ppi, "use_register_arrays", &C_use_register_arrays);
  L_read_parm_s (ppi, "layout_database_name", &C_layout_database_name);

  /* Instrumentation parameters */
  L_read_parm_s (ppi, "probe_for", &C_probe_for);

  /* The following parameter's values have required values when
   * probe_for=="profiling" and probe_for=="simulation" (set below).  
   *
   * They parameter's values will only be used if probe_for=="custom"!
   */
  L_read_parm_b (ppi, "predicate_probe_code", &C_predicate_probe_code);
  L_read_parm_b (ppi, "trace_control_flow", &C_trace_control_flow);
  L_read_parm_b (ppi, "trace_empty_cbs", &C_trace_empty_cbs);
  L_read_parm_b (ppi, "trace_mem_addrs", &C_trace_mem_addrs);
  L_read_parm_b (ppi, "trace_masked_load_faults",
		 &C_trace_masked_load_faults);
  L_read_parm_b (ppi, "trace_jump_rg_src1", &C_trace_jump_rg_src1);
  L_read_parm_b (ppi, "trace_pred_uses", &C_trace_pred_uses);
  L_read_parm_b (ppi, "trace_pred_defs", &C_trace_pred_defs);
  L_read_parm_b (ppi, "trace_pred_jump_fall_thru",
		 &C_trace_pred_jump_fall_thru);
  L_read_parm_b (ppi, "trace_extra_headers", &C_trace_extra_headers);
  L_read_parm_b (ppi, "trace_op_ids", &C_trace_op_ids);
  L_read_parm_b (ppi, "trace_enhanced_op_ids", &C_trace_enhanced_op_ids);
  /* Not implemented yet, leave in infrastructure -JCG 7/99 */
  L_read_parm_b (ppi, "?trace_dest_reg_values", &C_trace_dest_reg_values);
  L_read_parm_b (ppi, "?trace_src_reg_values", &C_trace_src_reg_values);
  L_read_parm_b (ppi, "?trace_src_lit_values", &C_trace_src_lit_values);

  L_read_parm_b (ppi, "trace_objects", &C_trace_objects);

  /* Reuse emulation */
  L_read_parm_b (ppi, "?reuse_emulation", &C_insert_reuse_emulation);

  /* The following parameter's values have required values when
   * probe_for=="profiling" (set below).  
   * 
   * The parameter's values will only be used if probe_for="simulation" or 
   * probe_for=="custom"!
   */
  L_read_parm_b (ppi, "trace_promoted_preds", &C_trace_promoted_preds);

  /* Process probe_for parameter */
  if (L_pmatch (C_probe_for, "nothing"))
    {
      /* Turn off instrumentation */
      C_insert_probes = 0;
    }
  else if (L_pmatch (C_probe_for, "custom"))
    {
      /* Turn on instrumentation, don't override parm settings */
      C_insert_probes = 1;
      C_custom_profiling = 1;
    }
  else if (L_pmatch (C_probe_for, "profiling"))
    {
      /* Turn on instrumentation, profiling, override parm settings */
      C_insert_probes = 1;

      /* Turn on everything the profiler expects to be in the trace */
      C_trace_control_flow = 1;
      C_trace_empty_cbs = 1;
      C_trace_jump_rg_src1 = 1;
      C_trace_pred_jump_fall_thru = 1;

      /* Turn off everything the profiler doesn't expect in the trace */
      C_predicate_probe_code = 0;
      C_trace_mem_addrs = 0;
      C_trace_masked_load_faults = 0;
      C_trace_pred_uses = 0;
      C_trace_pred_defs = 0;
      C_trace_extra_headers = 0;
      C_trace_op_ids = 0;
      C_trace_enhanced_op_ids = 0;
      C_trace_dest_reg_values = 0;
      C_trace_src_reg_values = 0;
      C_trace_src_lit_values = 0;
    }
  else if (L_pmatch (C_probe_for, "simulation"))
    {
      /* Turn on instrumentation, simulation, override most parm settings */
      C_insert_probes = 1;

      /* Turn on everything the simulator expects to be in the trace */
      C_predicate_probe_code = 1;
      C_trace_control_flow = 1;
      C_trace_mem_addrs = 1;
      C_trace_masked_load_faults = 1;
      C_trace_pred_uses = 1;
      C_trace_pred_defs = 1;

      /* Turn off everything the simulator doesn't expect in the trace */
      C_trace_empty_cbs = 0;
      C_trace_jump_rg_src1 = 0;
      C_trace_pred_jump_fall_thru = 0;

      /* Turn off extra trace information that the simulator doesn't 
       * currently support so override parameter settings.
       * (Lsim might be enhanced in the future to allow some of these
       *  parameter settings to be used.)
       */
      C_trace_extra_headers = 0;
      C_trace_op_ids = 0;
      C_trace_enhanced_op_ids = 0;
      C_trace_dest_reg_values = 0;
      C_trace_src_reg_values = 0;
      C_trace_src_lit_values = 0;
    }
  else if (L_pmatch (C_probe_for, "memtr"))
    {
      /* Turn on instrumentation, profiling, override parm settings */
      C_insert_probes = 1;

      /* Turn on everything the profiler expects to be in the trace */
      C_trace_control_flow = 1;
      C_trace_empty_cbs = 1;
      C_trace_jump_rg_src1 = 1;
      C_trace_pred_jump_fall_thru = 1;

      /* Turn off everything the profiler doesn't expect in the trace */
      C_predicate_probe_code = 0;
      C_trace_mem_addrs = 1;
      C_trace_masked_load_faults = 0;
      C_trace_pred_uses = 0;
      C_trace_pred_defs = 0;
      C_trace_extra_headers = 0;
      C_trace_op_ids = 0;
      C_trace_enhanced_op_ids = 0;
      C_trace_dest_reg_values = 0;
      C_trace_src_reg_values = 0;
      C_trace_src_lit_values = 0;

      C_trace_objects = 1;
    }
  else
    {
      L_punt ("L_read_parm_Lemulate: Unknown 'probe_for' setting of '%s'!",
	      C_probe_for);
    }

  /* For now, warn about unimplemented parameters */
  if (C_trace_dest_reg_values || C_trace_src_reg_values ||
      C_trace_src_lit_values)
    {
      L_punt ("Value tracing not yet implemented!");
    }

  L_read_parm_b (ppi, "cfile_only", &C_cfile_only);
  if (C_cfile_only)
    {
      C_generate_info_files = 0;
    }
}

/* Returns 1 if multiple of 8, 0 otherwise */
int
C_is_multiple_of_eight (int num)
{
  if (num == (num & ~(7)))
    return (1);
  else
    return (0);

}

/* Forces value passed thru int pointer to be a mutiple of 8.
 * If already a multiple of eight, does nothing.  Otherwise,
 * rounds up to next multple of eight.
 */
void
C_make_multiple_of_eight (int *int_ptr)
{
  *int_ptr = (*int_ptr + 7) & ~(7);
}

/* Returns the description of the layout_database.  String returned must
 * not be modified, freed, etc.
 * Copied from Mspec version, which I also wrote. -ITI/JCG 3/99
 */
char *
C_layout_database_desc ()
{
  char *desc;
  MD_Section *section;
  MD_Entry *entry;

  if (C_layout_database == NULL)
    return ("(Layout database not loaded!)");

  /* Find description of this database (if present).
   * Passed as the first entry name in the _HT__info section.
   */
  desc = "(no description present)";
  if ((section = MD_find_section (C_layout_database, "_HT__info")) != NULL)
    {
      entry = MD_first_entry (section);
      if (entry != NULL)
	desc = entry->name;
    }

  return (desc);
}

/* Reads and returns the integer field at the given section_name, entry_name,
 * and field_name given in the layout_database.  Punts on any error.
 * Copied from Mspec version, which I also wrote. -ITI/JCG 3/99
 */
int
C_read_database_i (char *section_name, char *entry_name, char *field_name)
{
  char *temp_name;
  MD_Section *section;
  MD_Entry *entry;
  MD_Field_Decl *field_decl;
  MD_Field *field;
  int value;

  /* Open and read in layout database on first query */
  if (C_layout_database == NULL)
    {
      L_punt ("C_read_database_i: NULL md pointer!");
    }

  /* Give troubleshooting message if section not found */
  if ((section = MD_find_section (C_layout_database, section_name)) == NULL)
    {
      L_punt ("\n"
	      "C_read_database_i: Section '%s' not found in '%s'!\n"
	      "\n"
	      "  Database built for: %s\n"
	      "\n"
	      "  Troubleshooting tips:\n"
	      "    1) Verify that *ALL* the source was run through "
	      "gen_CtoP at the same time!\n"
	      "    2) Verify that %s was generated for this benchmark,\n"
	      "       platform, and compiler!\n",
	      section_name, C_layout_database->name,
	      C_layout_database_desc (), C_layout_database->name);
    }

  /* Find field declaration in section */
  if ((field_decl = MD_find_field_decl (section, field_name)) == NULL)
    {
      L_punt ("C_read_database_i: Field decl for '%s' not found in %s->%s!",
	      field_name, C_layout_database->name, section_name);
    }

  /* Find entry in section, strip off quotes if present */
  if (entry_name[0] == '"')
    {
      /* Make copy of entry name so we can delete last quote */
      temp_name = strdup (&entry_name[1]);
      temp_name[strlen (temp_name) - 1] = 0;
      if ((entry = MD_find_entry (section, temp_name)) == NULL)
	{
	  L_punt ("C_read_database_i: Entry '%s' not found in %s->%s!",
		  temp_name, C_layout_database->name, section_name);
	}
      free (temp_name);
    }
  /* Otherwise, use raw entry name */
  else
    {
      if ((entry = MD_find_entry (section, entry_name)) == NULL)
	{
	  L_punt ("C_read_database_i: Entry '%s' not found in %s->%s!",
		  entry_name, C_layout_database->name, section_name);
	}
    }

  /* Make sure field present for this entry */
  if ((field = MD_find_field (entry, field_decl)) == NULL)
    {
      L_punt ("C_read_database_i: Field '%s' not found in %s->%s->%s!",
	      field_name, C_layout_database->name, section_name, entry_name);
    }

  /* Get the first element of the field */
  value = MD_get_int (field, 0);

  /* Return the value */
  return (value);
}

/* Reads and returns the string field at the given section_name, entry_name,
 * and field_name given in the layout_database.  Punts on any error.
 * The returned string should not be modified or freed!
 */
char *
C_read_database_s (char *section_name, char *entry_name, char *field_name)
{
  char *temp_name;
  MD_Section *section;
  MD_Entry *entry;
  MD_Field_Decl *field_decl;
  MD_Field *field;
  char *value;

  /* Open and read in layout database on first query */
  if (C_layout_database == NULL)
    {
      L_punt ("C_read_database_s: NULL md pointer!");
    }

  /* Give troubleshooting message if section not found */
  if ((section = MD_find_section (C_layout_database, section_name)) == NULL)
    {
      L_punt ("\n"
	      "C_read_database_s: Section '%s' not found in '%s'!\n"
	      "\n"
	      "  Database built for: %s\n"
	      "\n"
	      "  Troubleshooting tips:\n"
	      "    1) Verify that *ALL* the source was run through "
	      "gen_CtoP at the same time!\n"
	      "    2) Verify that %s was generated for this benchmark,\n"
	      "       platform, and compiler!\n",
	      section_name, C_layout_database->name,
	      C_layout_database_desc (), C_layout_database->name);
    }

  /* Find field declaration in section */
  if ((field_decl = MD_find_field_decl (section, field_name)) == NULL)
    {
      L_punt ("C_read_database_s: Field decl for '%s' not found in %s->%s!",
	      field_name, C_layout_database->name, section_name);
    }

  /* Find entry in section, strip off quotes if present */
  if (entry_name[0] == '"')
    {
      /* Make copy of entry name so we can delete last quote */
      temp_name = strdup (&entry_name[1]);
      temp_name[strlen (temp_name) - 1] = 0;
      if ((entry = MD_find_entry (section, temp_name)) == NULL)
	{
	  L_punt ("C_read_database_s: Entry '%s' not found in %s->%s!",
		  temp_name, C_layout_database->name, section_name);
	}
      free (temp_name);
    }
  /* Otherwise, use raw entry name */
  else
    {
      if ((entry = MD_find_entry (section, entry_name)) == NULL)
	{
	  L_punt ("C_read_database_s: Entry '%s' not found in %s->%s!",
		  entry_name, C_layout_database->name, section_name);
	}
    }

  /* Make sure field present for this entry */
  if ((field = MD_find_field (entry, field_decl)) == NULL)
    {
      L_punt ("C_read_database_s: Field '%s' not found in %s->%s->%s!",
	      field_name, C_layout_database->name, section_name, entry_name);
    }

  /* Get the first element of the field */
  value = MD_get_string (field, 0);

  /* Return the value */
  return (value);
}

/* Wrapper for L_get_input().
 *
 * If element_ptr is NULL, works the same way as L_get_input() except
 *    updates C_fn, C_data, and C_token_type 
 *    instead of L_fn, L_data, and L_token_type.
 *
 * If element_ptr is not NULL, gets the next input in the datalist as if
 *    L_get_input() was used.  Also updates datalist.  This functionality
 *    was added to allow C_emit_data() to be used to emit hash tables.
 *
 * In addition, this allows a clean implementation of C_peek_input().
 * This also allows all code and data labels to be cataloged into 
 * mem_usage!
 */
int
C_get_input (L_Datalist_Element ** element_ptr, Mem_Usage * mem_usage)
{
  /* Have we already peeked the next input? */
  if ((C_peeked_fn != NULL) || (C_peeked_data != NULL) ||
      (C_peeked_token_type != -1))
    {
      /* Yes, use it instead of doing another L_get_input() */
      C_fn = C_peeked_fn;
      C_data = C_peeked_data;
      C_token_type = C_peeked_token_type;

      /* Flag that we have used peeked data */
      C_peeked_fn = NULL;
      C_peeked_data = NULL;
      C_peeked_token_type = -1;
    }
  else
    {
      /* Handle reading from input steam case (datalist == NULL). */
      if (element_ptr == NULL)
	{
	  /* Otherwise, read in next input using L_get_input () */
	  /* Read input into Lcode's global variables */
	  C_token_type = L_get_input ();

	  C_fn = L_fn;
	  C_data = L_data;
	}

      /* Handle reading from datalist case */
      else
	{
	  /* Goto next data element, if exists */
	  if (*element_ptr != NULL)
	    *element_ptr = (*element_ptr)->next_element;

	  /* Detect at EOF case */
	  if (*element_ptr == NULL)
	    {
	      C_token_type = L_INPUT_EOF;
	      C_fn = NULL;
	      C_data = NULL;
	    }

	  /* Otherwise, get data from element */
	  else
	    {
	      C_data = (*element_ptr)->data;
	      C_token_type = C_data->type;
	      C_fn = NULL;
	    }
	}
    }

  /* Zero out Lcode global pointers, they must not be used */
  L_fn = NULL;
  L_data = NULL;
  L_token_type = -1;

  /* In order to create the proper externs, we need to know *every*
   * code and data label used, whether by the function or as
   * part of the data initialization.  The function scan is
   * done in C_emit_fn().  Because of the complicated way the data must be
   * processed, it is cleanest to scan the data here, where it is read in.
   */
  if ((C_token_type != L_INPUT_FUNCTION) && (C_token_type != L_INPUT_EOF))
    {
      /* Scan data read to build file-scope label usage info */
      C_add_data_mem_usage (C_data, mem_usage);
    }

  return (C_token_type);
}

/* C_peek_input() works like L_get_input(), except
 * updates C_peeked_fn, C_peeked_data, and C_peeked_token_type 
 * instead of L_fn, L_data, and L_token_type.
 *
 * IT is OK to peek at the same data twice.
 */
int
C_peek_input (L_Datalist_Element ** element_ptr)
{
  /* Have we already peeked the next input? */
  if ((C_peeked_fn != NULL) || (C_peeked_data != NULL) ||
      (C_peeked_token_type != -1))
    {
      /* Yes, do nothing.  Return old value read */
      return (C_peeked_token_type);
    }

  /* Handle reading from input steam case (datalist == NULL). */
  if (element_ptr == NULL)
    {
      /* Read input into Lcode's global variables */
      C_peeked_token_type = L_get_input ();

      C_peeked_fn = L_fn;
      C_peeked_data = L_data;
    }

  /* Handle reading from datalist case */
  else
    {
      /* Goto next data element, if exists */
      if (*element_ptr != NULL)
	*element_ptr = (*element_ptr)->next_element;

      /* Detect at EOF case */
      if (*element_ptr == NULL)
	{
	  C_peeked_token_type = L_INPUT_EOF;
	  C_peeked_fn = NULL;
	  C_peeked_data = NULL;
	}

      /* Otherwise, get data from element */
      else
	{
	  C_peeked_data = (*element_ptr)->data;
	  C_peeked_token_type = C_peeked_data->type;
	  C_peeked_fn = NULL;
	}
    }

  /* Zero out Lcode global pointers, they must not be used */
  L_fn = NULL;
  L_data = NULL;
  L_token_type = -1;

  return (C_peeked_token_type);
}

/* Write "value" or "value AND old_value"to file_name.
 * Used to pass host_info to gen_probed_lcode.
 */
void
C_write_brand (char *file_name, char *value, char *old_value)
{
  FILE *out;
  int value_len;
  char value_end;

  if ((out = fopen (file_name, "wt")) == NULL)
  {
      L_punt ("Unable to open '%s' for writing!", file_name);
  }

  /* If not old_value, just print value */
  if (old_value[0] == 0)
    {
      /* Remove quotes from output */
      value_len = strlen (value);
      value_end = value[value_len - 1];
      value[value_len - 1] = 0;
      fprintf (out, "%s", &value[1]);
      value[value_len - 1] = value_end;
    }
  /* Otherwise, print out combination of the two (with quotes) */
  else
    {
      fprintf (out, "%s AND %s", value, old_value);
    }

  fclose (out);
}

/* Returns a C type (in a string) that yeilds the desired alignment
 * on the host compiler.  
 *
 * Note: Do not free the returned string!
 */
char *
C_get_align_C_type (int desired_align)
{
  int abits;
  char *data_align_type = NULL;

  /* Determine the desired alignment in bits for ease of use */
  abits = desired_align * 8;

  /* Do a brute force search for a basic C type that gives
   * the desired alignment.  This search is still relatively
   * fast, so I don't think it is  worth further optimization.
   */
  if (abits == C_read_database_i ("_HT__base_types", "char", "align"))
    {
      data_align_type = "char";
    }

  else if (abits == C_read_database_i ("_HT__base_types", "short", "align"))
    {
      data_align_type = "short";
    }

  else if (abits == C_read_database_i ("_HT__base_types", "int", "align"))
    {
      data_align_type = "int";
    }

  else if (abits == C_read_database_i ("_HT__base_types", "double", "align"))
    {
      data_align_type = "double";
    }

  else if (abits == C_read_database_i ("_HT__base_types", "long", "align"))
    {
      data_align_type = "long";
    }

  else if (abits ==
	   C_read_database_i ("_HT__base_types", "longlong", "align"))
    {
      data_align_type = "longlong";
    }

  else if (abits == C_read_database_i ("_HT__base_types", "float", "align"))
    {
      data_align_type = "float";
    }

  else if (abits == C_read_database_i ("_HT__base_types", "void *", "align"))
    {
      data_align_type = "char *";	/* Since K&R C doesn't like void * */
    }

  else
    {
      L_punt
	("C_get_align_C_type: Could not find type that yields align %i bits!",
	 abits);
    }

  return (data_align_type);
}


/* Returns 1 if 'in' is at EOF, disregarding any leading white space,
 * 0 otherwise.  Note: this will consume all leading whitespace if
 * not at EOF
 */
int
C_is_at_EOF (FILE * in)
{
  int ch;

  /* Strip off leading whitespace */
  while ((ch = getc (in)) != EOF)
    {
      /* Stop at first non-whitespace */
      if (!isspace (ch))
	break;
    }

  /* If at EOF, return 1 */
  if (ch == EOF)
    {
      return (1);
    }
  /* Otherwise, put back character and return 0 */
  else
    {
      ungetc (ch, in);
      return (0);
    }
}

/* Reads in space delimited file name from 'in', puts in the the Mbuf
 * input_name and then puts input_name + ".c" in output_name.
 *
 * The Mbuf library is used to handle dynamic resizing of the buffer.
 *
 * Returns 1 until EOF hit.
 */
/* 08/05/02 REK Changing this function so that it returns the header name as an
 *              absolute path.  Since the input file name may contain a
 *              directory and the header will be in the same directory, we need
 *              to strip the directory from the header file name so that the
 *              compiler doesn't try to look in a non-existant directory.  To
 *              make sure that we can easily open the header, we simply use an 
 *              absolute path.
 */
int
C_get_next_input_name (FILE * in, Mbuf * input_name, Mbuf * output_name,
		       Mbuf * header_name)
{
  int ch;
  /* 08/05/02 REK A buffer to hold the current directory. */
  char cwd[MAXPATHLEN];

  /* 08/05/02 REK Get the current directory. */
  if (!getcwd(cwd, MAXPATHLEN))
  {
      /* REK */
      fprintf(stderr, "tools/Lemulate/l_emul_util.c:%d Could not get current directory\n", __LINE__);
  }

  /* Clear the input_name, output_name, and header_name buffers */
  clear_Mbuf (input_name);
  clear_Mbuf (output_name);
  clear_Mbuf (header_name);

  /* Strip off leading whitespace */
  while ((ch = getc (in)) != EOF)
    {
      /* Stop at first non-whitespace */
      if (!isspace (ch))
	break;
    }

  /* If at EOF, return 0 now */
  if (ch == EOF)
    return (0);

  /* Put first character into the input_name buf */
  addc_to_Mbuf (input_name, ch);

  /* Add characters until hit EOF or next whitespace */
  while (((ch = getc (in)) != EOF) && !isspace (ch))
    addc_to_Mbuf (input_name, ch);

  /* Add ".c" to end of input name to create output name */
  /* 08/05/02 REK Changing this to use the Mbuf accessor function instead
   *              of poking around inside the struct.
   */
  /*adds_to_Mbuf (output_name, input_name->buf);*/
  adds_to_Mbuf (output_name, get_Mbuf_buf (input_name));
  adds_to_Mbuf (output_name, ".c");

  /* Add ".h" to end of input name to create header name */
  /* 08/05/02 REK Return the header as an absolute path. */
  adds_to_Mbuf (header_name, cwd);
  addc_to_Mbuf (header_name, '/');
  adds_to_Mbuf (header_name, get_Mbuf_buf (input_name));
  adds_to_Mbuf (header_name, ".h");

  /* Success, return 1 */
  return (1);
}
