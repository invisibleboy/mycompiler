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
 *      File: l_emul_trace.c
 *      Authors: IMPACT Technologies, Inc. (Jake Janovetz)
 *      Creation Date:  June 1999
 *
 *      This file performs intrinsic emulation support for Lemulate.
 *      Intrinsic operations get converted to function calls which
 *      perform the equivalent function.  The intrinsic emulatiointrinsicn 
 *      functions are external and are linked with the emulation code.
 *
\*****************************************************************************/

#include <config.h>
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

/* Emit code to put an header into the trace pipe.
 *
 * 'header' should be a string corresponding a #defined
 * trace token.
 *
 * If 'header' begins with a '?', it will be put into the trace only
 * if C_trace_extra_headers is true (with '?' removed).
 * 
 * Will predicate the "put_trace" code if C_predicate_probe_code == 1 and 
 * op is not NULL and op is predicated.
 * 
 * 'op' may be NULL, which flags that put_trace call should not be predicated.
 */
void
C_emit_put_trace_header (FILE * out, L_Func * fn, L_Oper * op, char *header)
{
  /* If header begins with '?', determine if header should be printed */
  if (header[0] == '?')
    {
      /* If not tracing extra headers, do nothing */
      if (!C_trace_extra_headers)
	{
	  return;
	}
      /* Otherwise, strip '?' and print header */
      else
	{
	  header = &header[1];
	}
    }

  /* Start pred wrapper if need to predicate put_trace code */
  if (C_predicate_probe_code && (op != NULL) && (op->pred[0] != NULL))
    {
      fprintf (out, "%sif (", C_indent);
      C_emit_operand (out, fn, op->pred[0]);
      fprintf (out, ") {");
    }
  /* Otherwise, just indent put_trace code */
  else
    {
      fprintf (out, "%s", C_indent);
    }

  /* Just put the header into the trace */
  fprintf (out, "_EM_put_trace (%s);", header);

  /* Finish pred wrapper if need to predicate put_trace code */
  if (C_predicate_probe_code && (op != NULL) && (op->pred[0] != NULL))
    {
      fprintf (out, "}\n");
    }
  /* Otherwise, done with this line */
  else
    {
      fprintf (out, "\n");
    }
}

/* Emit code to put L_TRACE_NO_SEG_FAULT or L_TRACE_MASKED_SEG_FAULT
 * into the trace pipe, depending on value of _TR_EXCEPTION_MASKED.
 * 
 * Will predicate the "put_trace" code if C_predicate_probe_code == 1 and 
 * op is not NULL and op is predicated.
 * 
 * 'op' may be NULL, which flags that put_trace call should not be predicated.
 */
void
C_emit_put_trace_exception_state (FILE * out, L_Func * fn, L_Oper * op)
{
  /* Start pred wrapper if need to predicate put_trace code */
  if (C_predicate_probe_code && (op != NULL) && (op->pred[0] != NULL))
    {
      fprintf (out, "%sif (", C_indent);
      C_emit_operand (out, fn, op->pred[0]);
      fprintf (out, ") {");
    }
  /* Otherwise, just indent put_trace code */
  else
    {
      fprintf (out, "%s", C_indent);
    }

  /* L_TRACE_NO_SEG_FAULT is L_TRACE_MASKED_SEG_FALUT + 1.
   * Using the subtraction allows a binary flag to be used to
   * encode the correct error state.  Lsim will punt if 
   * _TR_EXCEPTION_MASKED != 0 or 1!
   */
  fprintf (out, "_EM_put_trace (L_TRACE_NO_SEG_FAULT-_TR_EXCEPTION_MASKED);");

  /* Finish pred wrapper if need to predicate put_trace code */
  if (C_predicate_probe_code && (op != NULL) && (op->pred[0] != NULL))
    {
      fprintf (out, "}\n");
    }
  /* Otherwise, done with this line */
  else
    {
      fprintf (out, "\n");
    }
}

/* Emit code to put an optional header and an integer into the trace pipe.
 *
 * 'header' may be NULL, which will cause it to always be omitted,
 * otherwise header should be a string corresponding a #defined
 * trace token.
 *
 * If 'header' begins with a '?', it will be put into the trace only
 * if C_trace_extra_headers is true (with '?' removed).
 * 
 * Will predicate the "put_trace" code if C_predicate_probe_code == 1 and 
 * op is not NULL and op is predicated.
 * 
 * 'op' may be NULL, which flags that put_trace call should not be predicated.
 */
void
C_emit_put_trace_int (FILE * out, L_Func * fn, L_Oper * op, char *header,
		      int value)
{
  /* Start pred wrapper if need to predicate put_trace code */
  if (C_predicate_probe_code && (op != NULL) && (op->pred[0] != NULL))
    {
      fprintf (out, "%sif (", C_indent);
      C_emit_operand (out, fn, op->pred[0]);
      fprintf (out, ") {");
    }
  /* Otherwise, just indent put_trace code */
  else
    {
      fprintf (out, "%s", C_indent);
    }

  /* If header begins with '?', determine if header should be printed */
  if ((header != NULL) && (header[0] == '?'))
    {
      /* If not tracing extra headers, remove header */
      if (!C_trace_extra_headers)
	{
	  header = NULL;
	}
      /* Otherwise, strip '?' and print header */
      else
	{
	  header = &header[1];
	}
    }

  /* If have header, emit it and the int value */
  if (header != NULL)
    {
      fprintf (out, "_EM_put_trace2 (%s, %i);", header, value);
    }
  /* Otherwise, just emit the int value */
  else
    {
      fprintf (out, "_EM_put_trace (%i);", value);
    }

  /* Finish pred wrapper if need to predicate put_trace code */
  if (C_predicate_probe_code && (op != NULL) && (op->pred[0] != NULL))
    {
      fprintf (out, "}\n");
    }
  /* Otherwise, done with this line */
  else
    {
      fprintf (out, "\n");
    }
}

/* Emit code to put an optional header and two integers into the trace pipe.
 *
 * 'header' may be NULL, which will cause it to always be omitted,
 * otherwise header should be a string corresponding a #defined
 * trace token.
 *
 * If 'header' begins with a '?', it will be put into the trace only
 * if C_trace_extra_headers is true (with '?' removed).
 * 
 * Will predicate the "put_trace" code if C_predicate_probe_code == 1 and 
 * op is not NULL and op is predicated.
 * 
 * 'op' may be NULL, which flags that put_trace call should not be predicated.
 */
void
C_emit_put_trace_two_ints (FILE * out, L_Func * fn, L_Oper * op,
			   char *header, int value1, int value2)
{
  /* Start pred wrapper if need to predicate put_trace code */
  if (C_predicate_probe_code && (op != NULL) && (op->pred[0] != NULL))
    {
      fprintf (out, "%sif (", C_indent);
      C_emit_operand (out, fn, op->pred[0]);
      fprintf (out, ") {");
    }
  /* Otherwise, just indent put_trace code */
  else
    {
      fprintf (out, "%s", C_indent);
    }

  /* If header begins with '?', determine if header should be printed */
  if ((header != NULL) && (header[0] == '?'))
    {
      /* If not tracing extra headers, remove header */
      if (!C_trace_extra_headers)
	{
	  header = NULL;
	}
      /* Otherwise, strip '?' and print header */
      else
	{
	  header = &header[1];
	}
    }

  /* If have header, emit it and the two int values */
  if (header != NULL)
    {
      fprintf (out, "_EM_put_trace3 (%s, %i, %i);", header, value1, value2);
    }
  /* Otherwise, just emit the two int values */
  else
    {
      fprintf (out, "_EM_put_trace2 (%i, %i);", value1, value2);
    }

  /* Finish pred wrapper if need to predicate put_trace code */
  if (C_predicate_probe_code && (op != NULL) && (op->pred[0] != NULL))
    {
      fprintf (out, "}\n");
    }
  /* Otherwise, done with this line */
  else
    {
      fprintf (out, "\n");
    }
}

/* Emit code to put an optional header and an integer operand into 
 * the trace pipe.
 *
 * 'header' may be NULL, which will cause it to always be omitted,
 * otherwise header should be a string corresponding a #defined
 * trace token.
 * 
 * If 'header' begins with a '?', it will be put into the trace only
 * if C_trace_extra_headers is true (with '?' removed).
 * 
 * Will predicate the "put_trace" code if C_predicate_probe_code == 1 and 
 * op is not NULL and op is predicated.
 * 
 * 'op' may be NULL, which flags that put_trace call should not be predicated.
 */
void
C_emit_put_trace_int_operand (FILE * out, L_Func * fn, L_Oper * op,
			      char *header, L_Operand * operand)
{
  /* Sanity check, operand better not be NULL */
  if (operand == NULL)
    L_punt ("C_emit_put_trace_int_operand: operand is NULL!");

  /* Sanity check, operand better be an int ctype */
  if (!L_is_ctype_integer (operand))
    {
      L_print_operand (stderr, operand, 1);
      L_punt ("C_emit_put_trace_int_operand: operand not int!");
    }

  /* Start pred wrapper if need to predicate put_trace code */
  if (C_predicate_probe_code && (op != NULL) && (op->pred[0] != NULL))
    {
      fprintf (out, "%sif (", C_indent);
      C_emit_operand (out, fn, op->pred[0]);
      fprintf (out, ") {");
    }
  /* Otherwise, just indent put_trace code */
  else
    {
      fprintf (out, "%s", C_indent);
    }

  /* If header begins with '?', determine if header should be printed */
  if ((header != NULL) && (header[0] == '?'))
    {
      /* If not tracing extra headers, remove header */
      if (!C_trace_extra_headers)
	{
	  header = NULL;
	}
      /* Otherwise, strip '?' and print header */
      else
	{
	  header = &header[1];
	}
    }

  /* If have header, emit it and the int value */
  if (header != NULL)
    {
      fprintf (out, "_EM_put_trace2 (%s, ", header);
      C_emit_operand (out, fn, operand);
      fprintf (out, ");");
    }
  /* Otherwise, just emit the int value */
  else
    {
      fprintf (out, "_EM_put_trace (");
      C_emit_operand (out, fn, operand);
      fprintf (out, ");");
    }

  /* Finish pred wrapper if need to predicate put_trace code */
  if (C_predicate_probe_code && (op != NULL) && (op->pred[0] != NULL))
    {
      fprintf (out, "}\n");
    }
  /* Otherwise, done with this line */
  else
    {
      fprintf (out, "\n");
    }
}

/* Emit code to put an optional header and an predicate operand into 
 * the trace pipe.
 *
 * 'header' may be NULL, which will cause it to always be omitted,
 * otherwise header should be a string corresponding a #defined
 * trace token.
 * 
 * If 'header' begins with a '?', it will be put into the trace only
 * if C_trace_extra_headers is true (with '?' removed).
 * 
 * Will predicate the "put_trace" code if C_predicate_probe_code == 1 and 
 * op is not NULL and op is predicated.
 * 
 * 'op' may be NULL, which flags that put_trace call should not be predicated.
 */
void
C_emit_put_trace_pred_operand (FILE * out, L_Func * fn, L_Oper * op,
			       char *header, L_Operand * operand)
{
  /* Sanity check, operand better not be NULL */
  if (operand == NULL)
    L_punt ("C_emit_put_trace_pred_operand: operand is NULL!");

  /* Sanity check, operand better be an predicate ctype */
  if (!L_is_ctype_predicate (operand))
    {
      L_print_operand (stderr, operand, 1);
      L_punt ("C_emit_put_trace_pred_operand: operand not predicate!");
    }

  /* Start pred wrapper if need to predicate put_trace code */
  if (C_predicate_probe_code && (op != NULL) && (op->pred[0] != NULL))
    {
      fprintf (out, "%sif (", C_indent);
      C_emit_operand (out, fn, op->pred[0]);
      fprintf (out, ") {");
    }
  /* Otherwise, just indent put_trace code */
  else
    {
      fprintf (out, "%s", C_indent);
    }

  /* If header begins with '?', determine if header should be printed */
  if ((header != NULL) && (header[0] == '?'))
    {
      /* If not tracing extra headers, remove header */
      if (!C_trace_extra_headers)
	{
	  header = NULL;
	}
      /* Otherwise, strip '?' and print header */
      else
	{
	  header = &header[1];
	}
    }

  /* If have header, emit it and the int value */
  if (header != NULL)
    {
      fprintf (out, "_EM_put_trace2 (%s, ", header);
      C_emit_operand (out, fn, operand);
      fprintf (out, ");");
    }
  /* Otherwise, just emit the int value */
  else
    {
      fprintf (out, "_EM_put_trace (");
      C_emit_operand (out, fn, operand);
      fprintf (out, ");");
    }

  /* Finish pred wrapper if need to predicate put_trace code */
  if (C_predicate_probe_code && (op != NULL) && (op->pred[0] != NULL))
    {
      fprintf (out, "}\n");
    }
  /* Otherwise, done with this line */
  else
    {
      fprintf (out, "\n");
    }
}

/* Emit code to put the jsr_id (encoded) and the function id into
 * the trace pipe after returning from jsr.
 */
void
C_emit_put_trace_post_jsr (FILE * out, L_Func * fn, L_Oper * op, int jsr_id,
			   int func_id)
{
  /* Sanity check */
  if (op == NULL)
    L_punt ("C_emit_put_trace_post_jsr: op NULL!");

  /* Start pred wrapper if need to predicate put_trace code */
  if (C_predicate_probe_code && (op != NULL) && (op->pred[0] != NULL))
    {
      fprintf (out, "%sif (", C_indent);
      C_emit_operand (out, fn, op->pred[0]);
      fprintf (out, ") {");
    }
  /* Otherwise, just indent put_trace code */
  else
    {
      fprintf (out, "%s", C_indent);
    }

  /* Encode jsr offset by subtracting it from the JSR offset
   * which is -2048 (on 4/99).
   *
   * Add extra header, if required.
   */
  if (C_trace_extra_headers)
    {
      fprintf (out, "_EM_put_trace3 (L_TRACE_RET_FROM_JSR, "
	       "L_TRACE_JSR_OFFSET - %i, %i);", jsr_id, func_id);
    }
  /* Otherwise, don't add header */
  else
    {
      fprintf (out, "_EM_put_trace2 (L_TRACE_JSR_OFFSET - %i, %i);",
	       jsr_id, func_id);
    }

  /* Finish pred wrapper if need to predicate put_trace code */
  if (C_predicate_probe_code && (op != NULL) && (op->pred[0] != NULL))
    {
      fprintf (out, "}\n");
    }
  /* Otherwise, done with this line */
  else
    {
      fprintf (out, "\n");
    }
}

/* Emit code to put an optional header and the sum of integer operand1
 * and integer operand 2 into the trace pipe.
 *
 * 'header' may be NULL, which will cause it to always be omitted,
 * otherwise header should be a string corresponding a #defined
 * trace token.
 * 
 * If 'header' begins with a '?', it will be put into the trace only
 * if C_trace_extra_headers is true (with '?' removed).
 * 
 * Will predicate the "put_trace" code if C_predicate_probe_code == 1 and 
 * op is not NULL and op is predicated.
 * 
 * 'op' may be NULL, which flags that put_trace call should not be predicated.
 */
void
C_emit_put_trace_mem_addr (FILE * out, L_Func * fn, L_Oper * op, char *header,
			   L_Operand * operand1, L_Operand * operand2)
{
#if 0 /* Commenting out this section to see what happens -- SAL 06/22/2006 */
  /* SER: Don't profile stores that use objects, loads that def objects,
   *      or pred mem ops. */
  L_AccSpec * accspec;
  /* 10/25/04 REK Commenting out unused variable to quiet compiler warning. */
#if 0
  char *obj;
#endif

  if (L_store_opcode (op))
    {
      for (accspec = op->acc_info; accspec; accspec = accspec->next)
	if (!(accspec->is_def))
	  return;
    }
  else if (L_load_opcode (op))
    {
      for (accspec = op->acc_info; accspec; accspec = accspec->next)
	if (accspec->is_def)
	  return;
    }
  else
    return;
  /* End of modification. */
#endif

  /* Sanity check, operand1 better not be NULL */
  if (operand1 == NULL)
    L_punt ("C_emit_put_trace_mem_addr: operand1 is NULL!");

  /* Sanity check, operand1 better be an int ctype (register or literal),
   * a label, or string constant.
   */
  if (!L_is_ctype_integer (operand1) &&
      !L_is_label (operand1) && !L_is_string (operand1))
    {
      L_print_operand (stderr, operand1, 1);
      L_punt ("C_emit_put_trace_mem_addr: "
	      "operand1 not int, label or string!");
    }

  /* Sanity check, operand2 better not be NULL */
  if (operand2 == NULL)
    L_punt ("C_emit_put_trace_mem_addr: operand2 is NULL!");


  /* Sanity check, operand2 better be an int ctype (register or literal),
   * a label, or string constant.
   */
  if (!L_is_ctype_integer (operand2) &&
      !L_is_label (operand2) && !L_is_string (operand2))
    {
      L_print_operand (stderr, operand2, 1);
      L_punt ("C_emit_put_trace_mem_addr: "
	      "operand2 not int, label or string!");
    }

  /* SER: can't put pred wrappers like this for Lprofile,
   * doesn't know how to handle it! Don't do for custom profiling. */

  /* Start pred wrapper if need to predicate put_trace code */
  if (C_predicate_probe_code && (op != NULL) && (op->pred[0] != NULL)
      && !C_custom_profiling)
    {
      fprintf (out, "%sif (", C_indent);
      C_emit_operand (out, fn, op->pred[0]);
      fprintf (out, ") {");
    }
  /* Otherwise, just indent put_trace code */
  else
    {
      fprintf (out, "%s", C_indent);
    }

  /* If header begins with '?', determine if header should be printed */
  if ((header != NULL) && (header[0] == '?'))
    {
      /* If not tracing extra headers, remove header */
      if (!C_trace_extra_headers)
	{
	  header = NULL;
	}
      /* Otherwise, strip '?' and print header */
      else
	{
	  header = &header[1];
	}
    }

  /* If have header, emit it and the int sum */
  if (header != NULL)
    {
      fprintf (out, "_EM_put_trace2 (%s, ", header);
      C_emit_operand (out, fn, operand1);
      fprintf (out, " + ");
      C_emit_operand (out, fn, operand2);
      fprintf (out, ");");
    }
  /* Otherwise, just emit the int sum */
  else
    {
      fprintf (out, "_EM_put_trace (");
      C_emit_operand (out, fn, operand1);
      fprintf (out, " + ");
      C_emit_operand (out, fn, operand2);
      fprintf (out, ");");
    }

  /* SER: see comment for beginning pred wrapper. */

  /* Finish pred wrapper if need to predicate put_trace code */
  if (C_predicate_probe_code && (op != NULL) && (op->pred[0] != NULL)
      && !C_custom_profiling)
    {
      fprintf (out, "}\n");
    }
  /* Otherwise, done with this line */
  else
    {
      fprintf (out, "\n");
    }
}

/* Emit trace system intialization code (goes at beginning of every function)*/
void
C_emit_fn_trace_system_setup (FILE * out)
{
  /* Emit first part of call to initialization macro */
  fprintf (out, "%s_EM_init_trace_system_if_necessary(\n", C_indent);

  /* 
   * Set trace_flag parameters based on parameters 
   */

  if (L_pmatch (C_probe_for, "profiling"))
    {
      fprintf (out, "%s%sTF_PROBE_FOR_PROFILING |\n", C_indent, C_indent);
    }

  else if (L_pmatch (C_probe_for, "simulation"))
    {
      fprintf (out, "%s%sTF_PROBE_FOR_SIMULATION |\n", C_indent, C_indent);
    }

  else if (L_pmatch (C_probe_for, "custom"))
    { /* SER: changed for HCH MICRO '04, might affect others. */
      fprintf (out, "%s%sTF_PROBE_FOR_PROFILING |\n", C_indent, C_indent);
    }

  else if (L_pmatch (C_probe_for, "memtrace"))
    {
      fprintf (out, "%s%sTF_PROBE_FOR_MEMTRACE |\n", C_indent, C_indent);
    }

  else
    {
      L_punt ("C_emit_fn_trace_system_setup: Unexpected probe_for '%s'!",
	      C_probe_for);
    }

  if (C_predicate_probe_code)
    {
      fprintf (out, "%s%sTF_PREDICATE_PROBE_CODE |\n", C_indent, C_indent);
    }

  if (C_trace_control_flow)
    {
      fprintf (out, "%s%sTF_TRACE_CONTROL_FLOW |\n", C_indent, C_indent);
    }

  if (C_trace_empty_cbs)
    {
      fprintf (out, "%s%sTF_TRACE_EMPTY_CBS |\n", C_indent, C_indent);
    }

  if (C_trace_mem_addrs)
    {
      fprintf (out, "%s%sTF_TRACE_MEM_ADDRS |\n", C_indent, C_indent);
    }

  if (C_trace_masked_load_faults)
    {
      fprintf (out, "%s%sTF_TRACE_MASKED_LOAD_FAULTS |\n", C_indent,
	       C_indent);
    }

  if (C_trace_jump_rg_src1)
    {
      fprintf (out, "%s%sTF_TRACE_JUMP_RG_SRC1 |\n", C_indent, C_indent);
    }

  if (C_trace_pred_uses)
    {
      fprintf (out, "%s%sTF_TRACE_PRED_USES |\n", C_indent, C_indent);
    }

  if (C_trace_pred_defs)
    {
      fprintf (out, "%s%sTF_TRACE_PRED_DEFS |\n", C_indent, C_indent);
    }

  if (C_trace_promoted_preds)
    {
      fprintf (out, "%s%sTF_TRACE_PROMOTED_PREDS |\n", C_indent, C_indent);
    }

  if (C_trace_pred_jump_fall_thru)
    {
      fprintf (out, "%s%sTF_TRACE_PRED_JUMP_FALL_THRU |\n", C_indent,
	       C_indent);
    }

  if (C_trace_extra_headers)
    {
      fprintf (out, "%s%sTF_TRACE_EXTRA_HEADERS |\n", C_indent, C_indent);
    }

  if (C_trace_op_ids)
    {
      fprintf (out, "%s%sTF_TRACE_OP_IDS |\n", C_indent, C_indent);
    }

  if (C_trace_enhanced_op_ids)
    {
      fprintf (out, "%s%sTF_TRACE_ENHANCED_OP_IDS |\n", C_indent, C_indent);
    }

  if (C_trace_dest_reg_values)
    {
      fprintf (out, "%s%sTF_TRACE_DEST_REG_VALUES |\n", C_indent, C_indent);
    }

  if (C_trace_src_reg_values)
    {
      fprintf (out, "%s%sTF_TRACE_SRC_REG_VALUES |\n", C_indent, C_indent);
    }

  if (C_trace_src_lit_values)
    {
      fprintf (out, "%s%sTF_TRACE_SRC_LIT_VALUES |\n", C_indent, C_indent);
    }

  /* End with TF_FUNC_IDS. Always using function ids for Lemulate */
  fprintf (out, "%s%sTF_FUNC_IDS);\n\n", C_indent, C_indent);
}

/* Emit prototypes, externs, and macros for the trace system */
void
C_emit_trace_system_prototypes (FILE * out)
{
  fprintf (out,
	   "\n"
	   "#include <Lcode/l_trace_interface.h>\n"
	   "\n"
	   "/* Trace system prototypes and externs */\n"
	   "extern int _TR_SYSTEM_INITIALIZED;\n"
	   "extern int *_TR_MEM_PTR;\n"
	   "extern int *_TR_MEM_FLUSH_PTR;\n"
	   "extern int _TR_IN_PARENT;\n"
	   "extern int _TR_EXCEPTION_MASKED;\n"
	   "extern void _TR_INITIALIZE_TRACE_SYSTEM(/* int flags */);\n"
	   "extern void _TR_TRACE_DUMP();\n"
	   "\n"
	   "/* Trace system macros */\n"
	   "/* Only initialize trace on the first function entered */\n"
	   "#define _EM_init_trace_system_if_necessary(flags) \\\n"
	   "   if(!_TR_SYSTEM_INITIALIZED) "
	   "_TR_INITIALIZE_TRACE_SYSTEM(flags)\n"
	   "\n"
	   "#define _EM_flush_trace() _TR_DUMP_TRACE()\n"
	   "\n"
	   "/* Flush trace if buffer is more than half full */\n"
	   "#define _EM_flush_trace_if_necessary() \\\n"
	   "   if(_TR_MEM_PTR > (_TR_MEM_FLUSH_PTR)) _EM_flush_trace()\n"
	   "\n"
	   "/* Use fully buffered version of _EM_put_trace unless FLUSH_TRACE defined */\n"
	   "#ifndef FLUSH_TRACE\n"
	   "\n"
	   "/* Use inlined trace buffering code by default */\n"
	   "#define _EM_put_trace(trace_word) *(_TR_MEM_PTR++)=(trace_word)\n"
	   "\n"
	   "#else\n"
	   "\n"
	   "/* Otherwise, flush trace after every write for debugging */\n"
	   "#define _EM_put_trace(trace_word) \\\n"
	   "   *(_TR_MEM_PTR++)=(trace_word), _EM_flush_trace()\n"
	   "\n"
	   "#endif\n"
	   "\n"
	   "/* Concise way of putting two words */\n"
	   "#define _EM_put_trace2(trace_word1, trace_word2) \\\n"
	   "   _EM_put_trace(trace_word1), _EM_put_trace(trace_word2)\n"
	   "\n"
	   "/* Concise way of putting three words */\n"
	   "#define _EM_put_trace3(trace_word1, trace_word2, trace_word3) \\\n"
	   "   _EM_put_trace(trace_word1), _EM_put_trace(trace_word2), \\\n"
	   "   _EM_put_trace(trace_word3)\n" "\n");
}
