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
 *      File: l_emul_emit_op.c
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

/* This function must stay in sync with L_ignore_oper in 
 * impact/src/Lcode/performance/Lencode/l_encode.c.  These operations
 * are currently treated as compiler directives and are not encoded
 * (and thus should not be instrumented).
 *
 * Note: The Lop_PRED_CLEAR and Lop_PRED_SET operations are assumed to
 * be combined into one or two masking operations that set/clear
 * up to 32 predicates at a time.  Since it is painful to
 * represent and manipulate such masking operations in Lcode, the masking 
 * operations are either assumed to be there but not modeled (slightly 
 * inaccurate but better than modeling 32 individual pred clears) or are not
 * added until just before postpass scheduling.
 */
int
C_Lencode_ignores_oper (L_Oper * op)
{
  /* Ignore the following opcodes */
  if ((op->opc == Lop_PROLOGUE) ||
      (op->opc == Lop_EPILOGUE) ||
      (op->opc == Lop_PRED_CLEAR) ||
      (op->opc == Lop_PRED_SET) || (op->opc == Lop_DEFINE))
    return (1);

  return (0);
}

/* This function replaces operands in ops which should be IP
 * relative because they are in a vararg function 
 */
void
C_fixup_vararg_ip(FILE *out, L_Oper *op)
{
  L_Attr *attr;
  if ((attr = L_find_attr(op->attr, "vararg_ip")))
    {
      L_delete_operand(op->src[0]);
      op->src[0] = L_copy_operand(attr->field[0]);
      if (attr->field[1])
	{
	  L_delete_operand(op->src[1]);
	  op->src[1] = L_copy_operand(attr->field[1]);
	}
      fprintf(out, "%s" 
	      "/* Adjusted IP relative operation in vararg function */\n", 
	      C_indent);
      C_emit_op_comment (out, op);
    } 
}

/* Emit straightforward ops with one dest and one src.
 * Will punt if these requirements are not met.
 * 
 * Generates:
 *   dest[0] = 'prefix'src[0]'postfix';
 *
 *   if (pred[0] != NULL), adds if (pred[0]) {...} around it
 *
 * 'prefix' and 'postfix' may be NULL, to specify no usage of
 *  that feature.
 */
void
C_emit_1_dest_1_src_op (FILE * out, L_Func * fn, L_Oper * op, char *prefix,
			char *postfix)
{
  /* Verify proper number of operands are present */
  C_check_operands ("C_emit_1_dest_1_src_op", op, 1, 1);

  /* Set indentation */
  fprintf (out, "%s", C_indent);

  /* Add if () {} around code, if predicated */
  if (op->pred[0] != NULL)
    {
      fprintf (out, "if (");
      C_emit_operand (out, fn, op->pred[0]);
      fprintf (out, ") {");
    }

  /* Emit 'dest[0] = ' */
  C_emit_operand (out, fn, op->dest[0]);
  fprintf (out, " = ");

  /* Add prefix, if not NULL */
  if (prefix != NULL)
    {
      fprintf (out, "%s", prefix);
    }

  /* Emit src[0] */
  C_emit_operand (out, fn, op->src[0]);

  /* Add postfix (if not NULL) */
  if (postfix != NULL)
    {
      fprintf (out, "%s", postfix);
    }

  /* Terminate statement, must go inside if */
  fprintf (out, ";");

  /* End predicated if statement (if exists) */
  if (op->pred[0] != NULL)
    {
      fprintf (out, "}");
    }

  /* Finished with this line */
  fprintf (out, "\n");
}

/* JWS 20000627 - support IA64 sign and zero extension opcodes
 */

void
C_emit_sz_ext_op (FILE * out, L_Func * fn, L_Oper * op, int pos, int sgn)
{
  /* Verify proper number of operands are present */
  C_check_operands ("C_emit_sz_ext_op", op, 1, 1);

  /* Set indentation */
  fprintf (out, "%s", C_indent);

  /* Add if () {} around code, if predicated */
  if (op->pred[0] != NULL)
    {
      fprintf (out, "if (");
      C_emit_operand (out, fn, op->pred[0]);
      fprintf (out, ") {");
    }

  /* Emit 'dest[0] = ' */
  C_emit_operand (out, fn, op->dest[0]);
  fprintf (out, " = ");

  if (sgn)
    {
      fprintf (out, "(");
      C_emit_operand (out, fn, op->src[0]);
      fprintf (out, " & ~(-1" ITintmaxsuffix " << %d)) | ((", pos + 1);
      C_emit_operand (out, fn, op->src[0]);
      fprintf (out,
	       " & (1" ITintmaxsuffix " << %d)) ? (-1" ITintmaxsuffix
	       " << %d) : 0)", pos, pos + 1);
    }
  else
    {
      fprintf (out, "(");
      C_emit_operand (out, fn, op->src[0]);
      fprintf (out, " & ~(-1" ITintmaxsuffix " << %d))", pos + 1);
    }

  /* Terminate statement, must go inside if */
  fprintf (out, ";");

  /* End predicated if statement (if exists) */
  if (op->pred[0] != NULL)
    {
      fprintf (out, "}");
    }

  /* Finished with this line */
  fprintf (out, "\n");
}

void
C_emit_extr_op (FILE * out, L_Func * fn, L_Oper * op)
{
  int sgn = 0;

  /* Verify proper number of operands are present */
  C_check_operands ("C_emit_extr_op", op, 1, 3);

  if (op->opc == Lop_EXTRACT)
    sgn = 1;
  else if (op->opc == Lop_EXTRACT_U)
    sgn = 0;
  else
    L_punt ("C_emit_extr_op: improper opc");

  /* Set indentation */
  fprintf (out, "%s", C_indent);

  /* Add if () {} around code, if predicated */
  if (op->pred[0] != NULL)
    {
      fprintf (out, "if (");
      C_emit_operand (out, fn, op->pred[0]);
      fprintf (out, ") {");
    }

  /* Emit 'dest[0] = ' */
  C_emit_operand (out, fn, op->dest[0]);
  fprintf (out, " = ");

  fprintf (out, "((");
  C_emit_operand (out, fn, op->src[0]);
  fprintf (out, " >> ");
  C_emit_operand (out, fn, op->src[1]);
  fprintf (out, ") & ((1" ITintmaxsuffix " << (");
  C_emit_operand (out, fn, op->src[2]);
  fprintf (out, ")) - 1))");

  if (sgn)
    {
      fprintf (out, " | ((");
      C_emit_operand (out, fn, op->src[0]);
      fprintf (out, " & (1" ITintmaxsuffix " << (");
      C_emit_operand (out, fn, op->src[1]);
      fprintf (out, " + ");
      C_emit_operand (out, fn, op->src[2]);
      fprintf (out, " - 1))) ? (-1" ITintmaxsuffix " << ");
      C_emit_operand (out, fn, op->src[2]);
      fprintf (out, ") : 0)");
    }

  /* Terminate statement, must go inside if */
  fprintf (out, ";");

  /* End predicated if statement (if exists) */
  if (op->pred[0] != NULL)
    {
      fprintf (out, "}");
    }

  /* Finished with this line */
  fprintf (out, "\n");
}

/* Emit abs ops with one dest and one src.
 * Will punt if these requirements are not met.
 * 
 * Generates:
 *   dest[0] = (src[0] < 0) ? -(src[0]) : src[0];
 *
 *   if (pred[0] != NULL), adds if (pred[0]) {...} around it
 *
 */
void
C_emit_abs_op (FILE * out, L_Func * fn, L_Oper * op)
{
  /* Verify proper number of operands are present */
  C_check_operands ("C_emit_abs_op", op, 1, 1);

  /* Set indentation */
  fprintf (out, "%s", C_indent);

  /* Add if () {} around code, if predicated */
  if (op->pred[0] != NULL)
    {
      fprintf (out, "if (");
      C_emit_operand (out, fn, op->pred[0]);
      fprintf (out, ") {");
    }

  /* Emit 'dest[0] = (' */
  C_emit_operand (out, fn, op->dest[0]);
  fprintf (out, " = (");

  /* Emit src[0] */
  C_emit_operand (out, fn, op->src[0]);

  /* Emit ' < 0) ? -(' */
  fprintf (out, " < 0) ? -(");

  /* Emit src[0] again */
  C_emit_operand (out, fn, op->src[0]);

  /* Emit ') : ' */
  fprintf (out, ") : ");

  /* Emit src[0] yet again */
  C_emit_operand (out, fn, op->src[0]);

  /* Terminate statement, must go inside if */
  fprintf (out, ";");

  /* End predicated if statement (if exists) */
  if (op->pred[0] != NULL)
    {
      fprintf (out, "}");
    }

  /* Finished with this line */
  fprintf (out, "\n");
}

/* Emit sqrt ops with one dest and one src.
 * Will punt if these requirements are not met.
 * 
 * Generates:
 *   dest[0] = sqrt(src[0]);
 *
 *   if (pred[0] != NULL), adds if (pred[0]) {...} around it
 *
 */
void
C_emit_sqrt_op (FILE * out, L_Func * fn, L_Oper * op)
{
  /* Verify proper number of operands are present */
  C_check_operands ("C_emit_sqrt_op", op, 1, 1);

  /* Set indentation */
  fprintf (out, "%s", C_indent);

  /* Add if () {} around code, if predicated */
  if (op->pred[0] != NULL)
    {
      fprintf (out, "if (");
      C_emit_operand (out, fn, op->pred[0]);
      fprintf (out, ") {");
    }

  /* Emit 'dest[0] = (' */
  C_emit_operand (out, fn, op->dest[0]);
  fprintf (out, " = (sqrt(");

  /* Emit src[0] */
  C_emit_operand (out, fn, op->src[0]);

  /* Terminate statement, must go inside if */
  fprintf (out, "));");

  /* End predicated if statement (if exists) */
  if (op->pred[0] != NULL)
    {
      fprintf (out, "}");
    }

  /* Finished with this line */
  fprintf (out, "\n");
}

/* Emit min/max ops with one dest and two srcs.
 * Will punt if these requirements are not met.
 * 
 * Generates:
 *   dest[0] = ((src[0] > src[1]) ? src[0] : src[1]);
 *
 *   if (pred[0] != NULL), adds if (pred[0]) {...} around it
 *
 */
void
C_emit_minmax_op (FILE * out, L_Func * fn, L_Oper * op)
{
  /* Verify proper number of operands are present */
  C_check_operands ("C_emit_minmax_op", op, 1, 2);

  /* Set indentation */
  fprintf (out, "%s", C_indent);

  /* Add if () {} around code, if predicated */
  if (op->pred[0] != NULL)
    {
      fprintf (out, "if (");
      C_emit_operand (out, fn, op->pred[0]);
      fprintf (out, ") {");
    }

  /* Emit 'dest[0] = (' */
  C_emit_operand (out, fn, op->dest[0]);
  fprintf (out, " = ((");

  /* Emit src[0] */
  C_emit_operand (out, fn, op->src[0]);

  if(op->proc_opc == Lop_MIN ||
     op->proc_opc == Lop_MIN_F ||
     op->proc_opc == Lop_MIN_F2)
    fprintf (out, " < ");
  else
    fprintf (out, " > ");

  /* Emit src[1] */
  C_emit_operand (out, fn, op->src[1]);

  fprintf (out, ") ? ");

  /* Emit src[0] */
  C_emit_operand (out, fn, op->src[0]);

  fprintf (out, " : ");

  /* Emit src[1] */
  C_emit_operand (out, fn, op->src[1]);

  /* Terminate statement, must go inside if */
  fprintf (out, ");");

  /* End predicated if statement (if exists) */
  if (op->pred[0] != NULL)
    {
      fprintf (out, "}");
    }

  /* Finished with this line */
  fprintf (out, "\n");
}

/* Emit i_f, i_f2 op with one dest and one src.
 * Will punt if these requirements are not met.
 *
 * Generates:
 *   dest[0] = (float cast) (completer cast) src[0];
 *
 *   if (pred[0] != NULL), adds if (pred[0]) {...} around it
 */
void
C_emit_float_conv_op (FILE * out, L_Func * fn, L_Oper * op)
{
  /* Verify proper number of operands are present */
  C_check_operands ("C_emit_float_conv_op", op, 1, 1);

  /* Set indentation */
  fprintf (out, "%s", C_indent);

  /* Add if () {} around code, if predicated */
  if (op->pred[0] != NULL)
    {
      fprintf (out, "if (");
      C_emit_operand (out, fn, op->pred[0]);
      fprintf (out, ") {");
    }

  /* Emit 'dest[0] = (' */
  C_emit_operand (out, fn, op->dest[0]);

  /* Emit float conversion type */
  switch (op->proc_opc)
  {
    case Lop_I_F:
      fprintf (out, " = (float)");
      break;

    case Lop_I_F2:
      fprintf (out, " = (double)");
      break;

    default:
      L_print_oper (stderr, op);
      L_punt ("C_emit_float_conv_op: Unsupported operation (above)!");
  }

  /* Emit completer cast */
  if (op->com[0])
    fprintf (out, "(%s)", C_get_compare_cast (op->com[0]));

  C_emit_operand (out, fn, op->src[0]);

  /* Terminate statement, must go inside if */
  fprintf (out, ";");

  /* End predicated if statement (if exists) */
  if (op->pred[0] != NULL)
    {
      fprintf (out, "}");
    }

  /* Finished with this line */
  fprintf (out, "\n");
}

/* Emit f_i, f2_i op with one dest and one src.
 * Will punt if these requirements are not met.
 *
 * Generates:
 *   dest[0] = (completer cast) src[0];
 *
 *   if (pred[0] != NULL), adds if (pred[0]) {...} around it
 */
void
C_emit_int_conv_op (FILE * out, L_Func * fn, L_Oper * op)
{
  /* Verify proper number of operands are present */
  C_check_operands ("C_emit_int_conv_op", op, 1, 1);

  /* Set indentation */
  fprintf (out, "%s", C_indent);

  /* Add if () {} around code, if predicated */
  if (op->pred[0] != NULL)
    {
      fprintf (out, "if (");
      C_emit_operand (out, fn, op->pred[0]);
      fprintf (out, ") {");
    }

  /* Emit 'dest[0] = (' */
  C_emit_operand (out, fn, op->dest[0]);

  /* Emit cast */
  if (op->com[0])
    fprintf (out, "= (%s)", C_get_compare_cast (op->com[0]));
  else if (C_native_machine_ctype == L_CTYPE_LLONG)
    fprintf (out, " = (longlong)");
  else if (C_native_machine_ctype == L_CTYPE_INT)
    fprintf (out, " = (int)");
  else
    L_punt ("C_emit_int_conv_op (F_I or F2_I): "
	    "Unsupported native machine reg size\n");

  C_emit_operand (out, fn, op->src[0]);

  /* Terminate statement, must go inside if */
  fprintf (out, ";");

  /* End predicated if statement (if exists) */
  if (op->pred[0] != NULL)
    {
      fprintf (out, "}");
    }

  /* Finished with this line */
  fprintf (out, "\n");
}

/* Emit straightforward ops with one dest and two srcs.
 * Will punt if these requirements are not met.
 * 
 * Generates:
 *   dest[0] = 'prefix'('cast'src[0])'operator'('cast'src[1])'postfix';
 *
 *   if (pred[0] != NULL), adds if (pred[0]) {...} around it
 *
 * 'prefix', 'cast', and 'postfix' may be NULL, to specify no usage of
 *  that feature (implicit parenthesis, if any, will be removed).
 */
void
C_emit_1_dest_2_src_op (FILE * out, L_Func * fn, L_Oper * op, char *prefix,
			char *cast, char *operator, char *postfix)
{
  /* Verify proper number of operands are present */
  C_check_operands ("C_emit_1_dest_2_src_op", op, 1, 2);

  /* Set indentation */
  fprintf (out, "%s", C_indent);

  /* Add if () {} around code, if predicated */
  if (op->pred[0] != NULL)
    {
      fprintf (out, "if (");
      C_emit_operand (out, fn, op->pred[0]);
      fprintf (out, ") {");
    }

  /* Emit 'dest[0] = ' */
  C_emit_operand (out, fn, op->dest[0]);
  fprintf (out, " = ");

  /* Add prefix, if not NULL */
  if (prefix != NULL)
    {
      fprintf (out, "%s", prefix);
    }

  /* Add cast to src[0], if not NULL */
  if (cast != NULL)
    {
      fprintf (out, "((%s)", cast);
    }

  /* Emit src[0] */
  C_emit_operand (out, fn, op->src[0]);

  /* Finish cast to src[0], if not NULL */
  if (cast != NULL)
    {
      fprintf (out, ")");
    }

  /* Emit operator between src[0] and src[1] */
  fprintf (out, "%s", operator);

  /* Add cast to src[1], if not NULL */
  if (cast != NULL)
    {
      fprintf (out, "((%s)", cast);
    }

  /* Emit src[1] */
  C_emit_operand (out, fn, op->src[1]);

  /* Finish cast to src[1], if not NULL */
  if (cast != NULL)
    {
      fprintf (out, ")");
    }

  /* Add postfix (if not NULL) */
  if (postfix != NULL)
    {
      fprintf (out, "%s", postfix);
    }

  /* Terminate statement, must go inside if */
  fprintf (out, ";");

  /* End predicated if statement (if exists) */
  if (op->pred[0] != NULL)
    {
      fprintf (out, "}");
    }

  /* Finished with this line */
  fprintf (out, "\n");
}

/* Emit lsl
 * Generates:
 *   dest[0] = (src[1] < 32) ? (regular lsl) : 0 
 *
 */
void
C_emit_1_dest_2_src_lsl (FILE * out, L_Func * fn, L_Oper * op, char *prefix,
			char *cast, char *operator, char *postfix)
{
  /* Verify proper number of operands are present */
  C_check_operands ("C_emit_1_dest_2_src_lsl", op, 1, 2);

  /* Set indentation */
  fprintf (out, "%s", C_indent);

  /* Add if () {} around code, if predicated */
  if (op->pred[0] != NULL)
    {
      fprintf (out, "if (");
      C_emit_operand (out, fn, op->pred[0]);
      fprintf (out, ") {");
    }

  /* Emit 'dest[0] = ' */
  C_emit_operand (out, fn, op->dest[0]);
  fprintf (out, " = ");

  fprintf(out, "(");
  C_emit_operand(out, fn, op->src[1]);
  fprintf(out, " < 32) ? ");

  fprintf(out, "(");

  /* Add prefix, if not NULL */
  if (prefix != NULL)
    {
      fprintf (out, "%s", prefix);
    }

  /* Add cast to src[0], if not NULL */
  if (cast != NULL)
    {
      fprintf (out, "((%s)", cast);
    }

  /* Emit src[0] */
  C_emit_operand (out, fn, op->src[0]);

  /* Finish cast to src[0], if not NULL */
  if (cast != NULL)
    {
      fprintf (out, ")");
    }

  /* Emit operator between src[0] and src[1] */
  fprintf (out, "%s", operator);

  /* Add cast to src[1], if not NULL */
  if (cast != NULL)
    {
      fprintf (out, "((%s)", cast);
    }

  /* Emit src[1] */
  C_emit_operand (out, fn, op->src[1]);

  /* Finish cast to src[1], if not NULL */
  if (cast != NULL)
    {
      fprintf (out, ")");
    }

  /* Add postfix (if not NULL) */
  if (postfix != NULL)
    {
      fprintf (out, "%s", postfix);
    }

  fprintf(out, ") : 0");
  /* Terminate statement, must go inside if */
  fprintf (out, ";");

  /* End predicated if statement (if exists) */
  if (op->pred[0] != NULL)
    {
      fprintf (out, "}");
    }

  /* Finished with this line */
  fprintf (out, "\n");
}

void
C_emit_add_op (FILE * out, L_Func * fn, L_Oper * op, char *prefix,
			char *cast, char *operator, char *postfix)
{
  /* Verify proper number of operands are present */
  C_check_operands ("C_emit_add_op", op, 1, 2);

  /* Set indentation */
  fprintf (out, "%s", C_indent);

  /* Add if () {} around code, if predicated */
  if (op->pred[0] != NULL)
    {
      fprintf (out, "if (");
      C_emit_operand (out, fn, op->pred[0]);
      fprintf (out, ") {");
    }

  /* Emit 'dest[0] = ' */
  C_emit_operand (out, fn, op->dest[0]);
  fprintf (out, " = ");

  /* Add prefix, if not NULL */
  if (prefix != NULL)
    {
      fprintf (out, "%s", prefix);
    }

  /* Add cast to src[0], if not NULL */
  if (cast != NULL)
    {
      fprintf (out, "((%s)", cast);
    }

  /* Emit src[0] */
  C_emit_operand (out, fn, op->src[0]);

  /* Finish cast to src[0], if not NULL */
  if (cast != NULL)
    {
      fprintf (out, ")");
    }

  /* Emit operator between src[0] and src[1] */
  fprintf (out, " + ");

  /* Add cast to src[1], if not NULL */
  if (cast != NULL)
    {
      fprintf (out, "((%s)", cast);
    }

  /* Emit src[1] */
  C_emit_operand (out, fn, op->src[1]);

  /* Finish cast to src[1], if not NULL */
  if (cast != NULL)
    {
      fprintf (out, ")");
    }

  /* Add postfix (if not NULL) */
  if (postfix != NULL)
    {
      fprintf (out, "%s", postfix);
    }

  /* Terminate statement, must go inside if */
  fprintf (out, ";");

  /* KVM : Emit the carry computation */
  fprintf(out, "\n%sif((unsigned)", C_indent);
  C_emit_operand(out, fn, op->dest[0]);
  fprintf(out, " < (unsigned)");
  C_emit_operand(out, fn, op->src[0]);
  fprintf(out, ") l_emul_carry_bit = 1;\n");
  fprintf(out, "%sl_emul_carry_bit = 0;\n", C_indent);

  /* End predicated if statement (if exists) */
  if (op->pred[0] != NULL)
    {
      fprintf (out, "}");
    }

  /* Finished with this line */
  fprintf (out, "\n");
}

void
C_emit_addc_op (FILE * out, L_Func * fn, L_Oper * op, char *prefix,
			char *cast, char *operator, char *postfix)
{
  /* Verify proper number of operands are present */
  C_check_operands ("C_emit_addc_op", op, 2, 3);

  /* Set indentation */
  fprintf (out, "%s", C_indent);

  /* Add if () {} around code, if predicated */
  if (op->pred[0] != NULL)
    {
      fprintf (out, "if (");
      C_emit_operand (out, fn, op->pred[0]);
      fprintf (out, ") {");
    }

  /* Emit 'dest[0] = ' */
  C_emit_operand (out, fn, op->dest[0]);
  fprintf (out, " = ");

  /* Add prefix, if not NULL */
  if (prefix != NULL)
    {
      fprintf (out, "%s", prefix);
    }

  /* Add cast to src[0], if not NULL */
  if (cast != NULL)
    {
      fprintf (out, "((%s)", cast);
    }

  /* Emit src[0] */
  C_emit_operand (out, fn, op->src[0]);

  /* Finish cast to src[0], if not NULL */
  if (cast != NULL)
    {
      fprintf (out, ")");
    }

  /* Emit operator between src[0] and src[1] */
  fprintf (out, " + ");

  /* Add cast to src[1], if not NULL */
  if (cast != NULL)
    {
      fprintf (out, "((%s)", cast);
    }

  /* Emit src[1] */
  C_emit_operand (out, fn, op->src[1]);

  /* Finish cast to src[1], if not NULL */
  if (cast != NULL)
    {
      fprintf (out, ")");
    }

  /* Emit operator between src[1] and src[2] */
  fprintf (out, " + ");

  /* Add cast to src[2], if not NULL */
  if (cast != NULL)
    {
      fprintf (out, "((%s)", cast);
    }

  /* Emit src[2] */
  C_emit_operand (out, fn, op->src[2]);

  /* Finish cast to src[2], if not NULL */
  if (cast != NULL)
    {
      fprintf (out, ")");
    }

  /* Add postfix (if not NULL) */
  if (postfix != NULL)
    {
      fprintf (out, "%s", postfix);
    }

  /* Terminate statement, must go inside if */
  fprintf (out, ";");

  /* KVM : Emit the carry computation */
  fprintf(out, "\n%s", C_indent);
  C_emit_operand(out, fn, op->dest[1]);
  fprintf(out, " = 0;\n");
  fprintf(out, "%sif((unsigned)", C_indent);
  C_emit_operand(out, fn, op->dest[0]);
  fprintf(out, " < (unsigned)");
  C_emit_operand(out, fn, op->src[0]);
  fprintf(out, ") ");
  C_emit_operand(out, fn, op->dest[1]);
  fprintf(out, " = 1;\n");

  /* End predicated if statement (if exists) */
  if (op->pred[0] != NULL)
    {
      fprintf (out, "}");
    }

  /* Finished with this line */
  fprintf (out, "\n");
}

void
C_emit_subc_op (FILE * out, L_Func * fn, L_Oper * op, char *prefix,
			char *cast, char *operator, char *postfix)
{
  /* Verify proper number of operands are present */
  C_check_operands ("C_emit_subc_op", op, 2, 3);

  /* Set indentation */
  fprintf (out, "%s", C_indent);

  /* Add if () {} around code, if predicated */
  if (op->pred[0] != NULL)
    {
      fprintf (out, "if (");
      C_emit_operand (out, fn, op->pred[0]);
      fprintf (out, ") {");
    }

  /* Emit 'dest[0] = ' */
  C_emit_operand (out, fn, op->dest[0]);
  fprintf (out, " = ");

  /* Add prefix, if not NULL */
  if (prefix != NULL)
    {
      fprintf (out, "%s", prefix);
    }

  /* Add cast to src[0], if not NULL */
  if (cast != NULL)
    {
      fprintf (out, "((%s)", cast);
    }

  /* Emit src[0] */
  C_emit_operand (out, fn, op->src[0]);

  /* Finish cast to src[0], if not NULL */
  if (cast != NULL)
    {
      fprintf (out, ")");
    }

  /* Emit operator between src[0] and src[1] */
  fprintf (out, " - ");

  /* Add cast to src[1], if not NULL */
  if (cast != NULL)
    {
      fprintf (out, "((%s)", cast);
    }

  /* Emit src[1] */
  C_emit_operand (out, fn, op->src[1]);

  /* Finish cast to src[1], if not NULL */
  if (cast != NULL)
    {
      fprintf (out, ")");
    }

  /* Emit operator between src[1] and src[2] */
  fprintf (out, " - ");

  /* Add cast to src[2], if not NULL */
  if (cast != NULL)
    {
      fprintf (out, "((%s)", cast);
    }

  /* Emit src[2] */
  C_emit_operand (out, fn, op->src[2]);

  /* Finish cast to src[2], if not NULL */
  if (cast != NULL)
    {
      fprintf (out, ")");
    }

  /* Add postfix (if not NULL) */
  if (postfix != NULL)
    {
      fprintf (out, "%s", postfix);
    }

  /* Terminate statement, must go inside if */
  fprintf (out, ";");

  /* KVM : Emit the carry computation */
  fprintf(out, "\n%s", C_indent);
  C_emit_operand(out, fn, op->dest[1]);
  fprintf(out, " = 0;\n");
  fprintf(out, "%sif((unsigned)", C_indent);
  C_emit_operand(out, fn, op->dest[0]);
  fprintf(out, " > (unsigned)");
  C_emit_operand(out, fn, op->src[0]);
  fprintf(out, ") ");
  C_emit_operand(out, fn, op->dest[1]);
  fprintf(out, " = 1;\n");

  /* End predicated if statement (if exists) */
  if (op->pred[0] != NULL)
    {
      fprintf (out, "}");
    }

  /* Finished with this line */
  fprintf (out, "\n");
}

void
C_emit_mul_wide_op (FILE * out, L_Func * fn, L_Oper * op, char *prefix,
			char *cast, char *operator, char *postfix)
{
  /* Verify proper number of operands are present */
  C_check_operands ("C_emit_mul_wide_op", op, 2, 2);

  /* Set indentation */
  fprintf (out, "%s", C_indent);

  /* Add if () {} around code, if predicated */
  if (op->pred[0] != NULL)
    {
      fprintf (out, "if (");
      C_emit_operand (out, fn, op->pred[0]);
      fprintf (out, ") {");
    }

  /* Emit 'dest[0] = ' */
  C_emit_operand (out, fn, op->dest[1]);
  fprintf (out, " = ");

  /* Add prefix, if not NULL */
  if (prefix != NULL)
    {
      fprintf (out, "%s", prefix);
    }

  /* KVM : Open parens for mask. */
  fprintf(out, "(");

  /* Add cast to src[0], if not NULL */
  if (cast != NULL)
    {
      fprintf (out, "((%s)", cast);
    }

  /* KVM : Convert to long long */
  fprintf(out, "((long long)");
  /* Emit src[0] */
  C_emit_operand (out, fn, op->src[0]);
  fprintf(out, ")");

  /* Finish cast to src[0], if not NULL */
  if (cast != NULL)
    {
      fprintf (out, ")");
    }

  /* Emit operator between src[0] and src[1] */
  fprintf (out, " * ");

  /* Add cast to src[1], if not NULL */
  if (cast != NULL)
    {
      fprintf (out, "((%s)", cast);
    }

  /* KVM : Convert to long long */
  fprintf(out, "((long long)");
  /* Emit src[1] */
  C_emit_operand (out, fn, op->src[1]);
  fprintf(out, ")");

  /* Finish cast to src[1], if not NULL */
  if (cast != NULL)
    {
      fprintf (out, ")");
    }

  /* KVM : Close parens for mask. */
  fprintf(out, ") & 0xFFFFFFFF");

  /* Add postfix (if not NULL) */
  if (postfix != NULL)
    {
      fprintf (out, "%s", postfix);
    }

  /* Terminate statement, must go inside if */
  fprintf (out, ";\n");

  fprintf (out, "%s", C_indent);

  /* Emit 'dest[1] = ' */
  C_emit_operand (out, fn, op->dest[0]);
  fprintf (out, " = ");

  /* Add prefix, if not NULL */
  if (prefix != NULL)
    {
      fprintf (out, "%s", prefix);
    }

  /* KVM : Open parens for mask. */
  fprintf(out, "((");

  /* Add cast to src[0], if not NULL */
  if (cast != NULL)
    {
      fprintf (out, "((%s)", cast);
    }

  /* KVM : Convert to long long */
  fprintf(out, "((long long)");
  /* Emit src[0] */
  C_emit_operand (out, fn, op->src[0]);
  fprintf(out, ")");

  /* Finish cast to src[0], if not NULL */
  if (cast != NULL)
    {
      fprintf (out, ")");
    }

  /* Emit operator between src[0] and src[1] */
  fprintf (out, " * ");

  /* Add cast to src[1], if not NULL */
  if (cast != NULL)
    {
      fprintf (out, "((%s)", cast);
    }

  /* KVM : Convert to long long */
  fprintf(out, "((long long)");
  /* Emit src[1] */
  C_emit_operand (out, fn, op->src[1]);
  fprintf(out, ")");

  /* Finish cast to src[1], if not NULL */
  if (cast != NULL)
    {
      fprintf (out, ")");
    }

  /* KVM : Close parens for mask. */
  fprintf(out, ") >> 32) & 0xFFFFFFFF");

  /* Add postfix (if not NULL) */
  if (postfix != NULL)
    {
      fprintf (out, "%s", postfix);
    }

  /* Terminate statement, must go inside if */
  fprintf (out, ";");

  /* End predicated if statement (if exists) */
  if (op->pred[0] != NULL)
    {
      fprintf (out, "}");
    }

  /* Finished with this line */
  fprintf (out, "\n");
}


/* Emit div ops with one dest and two srcs.
 * Will punt if these requirements are not met.
 * 
 * Generates:
 *   If normal, trapping version:
 *      dest[0] = (('cast')src[0]) / (('cast')src[1]);
 *   Otherwise:
 *      dest[0] = (src[1] != 0) ? ((('cast')src[0]) / (('cast')src[1])) : 0;
 *
 *   if (pred[0] != NULL), adds if (pred[0]) {...} around it
 *
 *  'cast' may be NULL, to specify no usage of that feature 
 *   (implicit parenthesis, if any, will be removed).
 */
void
C_emit_div_op (FILE * out, L_Func * fn, L_Oper * op, char *prefix, char *operator,
	       char *cast, char *postfix)
{
  /* Verify proper number of operands are present */
  C_check_operands ("C_emit_div_op", op, 1, 2);

  /* Set indentation */
  fprintf (out, "%s", C_indent);

  /* Add if () {} around code, if predicated */
  if (op->pred[0] != NULL)
    {
      fprintf (out, "if (");
      C_emit_operand (out, fn, op->pred[0]);
      fprintf (out, ") {");
    }


  /* Emit 'dest[0] = ' */
  C_emit_operand (out, fn, op->dest[0]);
  fprintf (out, " = ");

  /* Add prefix, if not NULL */
  if (prefix != NULL)
    {
      fprintf (out, "%s", prefix);
    }

  /* If non-trapping version, protect against divide by 0 */
  if (op->flags & L_OPER_MASK_PE)
    {
      /* Emit '(src[1] != 0) ?(' to guard execution of divide */
      fprintf (out, "(");
      C_emit_operand (out, fn, op->src[1]);
      fprintf (out, " != 0) ? (");
    }

  /* Do actual divide, if normal version or src[1] not 0 */

  /* Add cast to src[0], if not NULL */
  if (cast != NULL)
    {
      fprintf (out, "((%s)", cast);
    }

  /* Emit src[0] */
  C_emit_operand (out, fn, op->src[0]);

  /* Finish cast to src[0], if not NULL */
  if (cast != NULL)
    {
      fprintf (out, ")");
    }

  /* Emit '/' or '%' between src[0] and src[1] */
  fprintf (out, " %s ", operator);

  /* Add cast to src[1], if not NULL */
  if (cast != NULL)
    {
      fprintf (out, "((%s)", cast);
    }

  /* Emit src[1] */
  C_emit_operand (out, fn, op->src[1]);

  /* Finish cast to src[1], if not NULL */
  if (cast != NULL)
    {
      fprintf (out, ")");
    }

  /* If non-trapping version, protect against divide by 0 */
  if (op->flags & L_OPER_MASK_PE)
    {
      /* Finish guard against divide by 0 */
      fprintf (out, ") : 0");
    }

  /* Add postfix (if not NULL) */
  if (postfix != NULL)
    {
      fprintf (out, "%s", postfix);
    }

  /* Terminate statement, must go inside if */
  fprintf (out, ";");

  /* End predicated if statement (if exists) */
  if (op->pred[0] != NULL)
    {
      fprintf (out, "}");
    }

  /* Finished with this line */
  fprintf (out, "\n");
}

/* Emit straightforward ops with one dest and two srcs.
 * Will punt if these requirements are not met.
 * 
 * Generates:
 *   dest[0] = 'prefix'src[index1]'operator1'src[index2]\
 *             'operator2'src[index3]'postfix';
 *
 *   if (pred[0] != NULL), adds if (pred[0]) {...} around it
 *
 */
void
C_emit_1_dest_3_src_op (FILE * out, L_Func * fn, L_Oper * op,
			char *prefix, int index1, char *operator1,
			int index2, char *operator2, int index3,
			char *postfix)
{
  /* Verify proper number of operands are present */
  C_check_operands ("C_emit_1_dest_3_src_op", op, 1, 3);

  /* Set indentation */
  fprintf (out, "%s", C_indent);

  /* Add if () {} around code, if predicated */
  if (op->pred[0] != NULL)
    {
      fprintf (out, "if (");
      C_emit_operand (out, fn, op->pred[0]);
      fprintf (out, ") {");
    }

  /* Emit 'dest[0] = ' */
  C_emit_operand (out, fn, op->dest[0]);
  fprintf (out, " = ");

  /* Add prefix */
  fprintf (out, "%s", prefix);

  /* Emit src[index1] */
  C_emit_operand (out, fn, op->src[index1]);

  /* Emit operator1 between src[index1] and src[index2] */
  fprintf (out, "%s", operator1);

  /* Emit src[index2] */
  C_emit_operand (out, fn, op->src[index2]);

  /* Emit operator2 between src[index2] and src[index3] */
  fprintf (out, "%s", operator2);

  /* Emit src[index3] */
  C_emit_operand (out, fn, op->src[index3]);

  /* Add postfix */
  fprintf (out, "%s", postfix);

  /* Terminate statement, must go inside if */
  fprintf (out, ";");

  /* End predicated if statement (if exists) */
  if (op->pred[0] != NULL)
    {
      fprintf (out, "}");
    }

  /* Finished with this line */
  fprintf (out, "\n");
}

/* Emit load ops with one dest and two srcs.
 * Will punt if these requirements are not met.
 * 
 * If a standard, trapping load, generates:
 *   dest[0] = *('cast'(src[0]+src[1]));
 *
 * Otherwise if the non-trapping load version, generates:
 *   dest[0] = _EM_NTload_'type'('cast'(src[0]+src[1]));
 *
 *   if (pred[0] != NULL), adds if (pred[0]) {...} around it
 *
 */
void
C_emit_std_load_op (FILE * out, L_Func * fn, L_Oper * op, char *cast,
		    char *type)
{
  /* Verify proper number of operands are present */
  C_check_operands ("C_emit_std_load_op", op, 1, 2);

  /* Add instrumentation if tracing memory addresses */
  if (C_insert_probes && C_trace_mem_addrs)
    {
      C_emit_put_trace_mem_addr (out, fn, op, "?L_TRACE_READ",
				 op->src[0], op->src[1]);
    }

  /* Set indentation */
  fprintf (out, "%s", C_indent);

  /* Add if () {} around code, if predicated */
  if (op->pred[0] != NULL)
    {
      fprintf (out, "if (");
      C_emit_operand (out, fn, op->pred[0]);
      fprintf (out, ") {");
    }

  /* Emit 'dest[0] ' */
  C_emit_operand (out, fn, op->dest[0]);

  /* If normal load, emit ' = *' */
  if (!(op->flags & L_OPER_MASK_PE))
    {
      fprintf (out, " = *");
    }

  /* Otherwise, if non-trapping load, emit ' = _EM_NTload_"type"' */
  else
    {
      fprintf (out, " = _EM_NTload_%s", type);
    }

  /* Emit '(cast(' */
  fprintf (out, "(%s(", cast);

  /* Emit src[0] */
  C_emit_operand (out, fn, op->src[0]);

  /* Emit '+' */
  fprintf (out, " + ");

  /* Emit src[1] */
  C_emit_operand (out, fn, op->src[1]);

  /* Terminate statement, must go inside if */
  fprintf (out, "));");

  /* End predicated if statement (if exists) */
  if (op->pred[0] != NULL)
    {
      fprintf (out, "}");
    }

  /* Finished with this line */
  fprintf (out, "\n");

  /* If non-trapping load and tracing masked load exceptions */
  if (C_insert_probes && C_trace_masked_load_faults &&
      (op->flags & L_OPER_MASK_PE))
    {
      C_emit_put_trace_exception_state (out, fn, op);
    }
}

/* Emit post-increment load ops with two dest and three srcs.
 * Will punt if these requirements are not met.
 * 
 * If a standard, trapping load, generates:
 *   dest[0] = *('cast'(src[0]+src[1]));
 *   dest[1] = src[0] + src[2];
 *
 * Otherwise if the non-trapping load version, generates:
 *   dest[0] = _EM_NTload_'type'('cast'(src[0]+src[1]));
 *   dest[1] = src[0] + src[2];
 *
 *   if (pred[0] != NULL), adds if (pred[0]) {...} around it
 *
 */
void
C_emit_post_load_op (FILE * out, L_Func * fn, L_Oper * op, char *cast,
		    char *type)
{
  /* Verify proper number of operands are present */
  C_check_operands ("C_emit_post_load_op", op, 2, 3);

  /* Add instrumentation if tracing memory addresses */
  if (C_insert_probes && C_trace_mem_addrs)
    {
      C_emit_put_trace_mem_addr (out, fn, op, "?L_TRACE_READ",
				 op->src[0], op->src[1]);
    }

  /* Set indentation */
  fprintf (out, "%s", C_indent);

  /* Add if () {} around code, if predicated */
  if (op->pred[0] != NULL)
    {
      fprintf (out, "if (");
      C_emit_operand (out, fn, op->pred[0]);
      fprintf (out, ") {");
    }

  /* Emit 'dest[0] ' */
  C_emit_operand (out, fn, op->dest[0]);

  /* If normal load, emit ' = *' */
  if (!(op->flags & L_OPER_MASK_PE))
    {
      fprintf (out, " = *");
    }

  /* Otherwise, if non-trapping load, emit ' = _EM_NTload_"type"' */
  else
    {
      fprintf (out, " = _EM_NTload_%s", type);
    }

  /* Emit '(cast(' */
  fprintf (out, "(%s(", cast);

  /* Emit src[0] */
  C_emit_operand (out, fn, op->src[0]);

  /* Emit '+' */
  fprintf (out, " + ");

  /* Emit src[1] */
  C_emit_operand (out, fn, op->src[1]);

  /* Terminate statement, must go inside if */
  fprintf (out, "));");
  
  C_emit_operand (out, fn, op->dest[1]);
  fprintf (out, " = ");
  C_emit_operand (out, fn, op->src[0]);
  fprintf (out, " + ");
  C_emit_operand (out, fn, op->src[2]);
  fprintf (out, ";");

  /* End predicated if statement (if exists) */
  if (op->pred[0] != NULL)
    {
      fprintf (out, "}");
    }

  /* Finished with this line */
  fprintf (out, "\n");

  /* If non-trapping load and tracing masked load exceptions */
  if (C_insert_probes && C_trace_masked_load_faults &&
      (op->flags & L_OPER_MASK_PE))
    {
      C_emit_put_trace_exception_state (out, fn, op);
    }
}

/* Emit store ops with zero dests and three srcs.
 * Will punt if these requirements are not met.
 * 
 * Generates:
 *   *('cast'(src[0]+src[1])) = src[2];
 *
 *   if (pred[0] != NULL), adds if (pred[0]) {...} around it
 *
 */
void
C_emit_std_store_op (FILE * out, L_Func * fn, L_Oper * op, char *cast)
{
  /* Verify proper number of operands are present */
  C_check_operands ("C_emit_std_store_op", op, 0, 3);

  /* Add instrumentation if tracing memory addresses */
  if (C_insert_probes && C_trace_mem_addrs)
    {
      if (C_custom_profiling)
        C_emit_put_trace_mem_addr (out, fn, op, "?L_TRACE_WRITE",
				   op->src[0], op->src[1]);
      else
        C_emit_put_trace_mem_addr (out, fn, op, "L_TRACE_WRITE",
				   op->src[0], op->src[1]);
    }

  if (L_find_attr (op->attr, "VIPSPILL"))
    {
      fprintf (out, "%s/* VARARG SPILL SUPPRESSED FOR EMULATION */\n",
               C_indent);
      return;
    }

  /* Set indentation */
  fprintf (out, "%s", C_indent);

  /* Add if () {} around code, if predicated */
  if (op->pred[0] != NULL)
    {
      fprintf (out, "if (");
      C_emit_operand (out, fn, op->pred[0]);
      fprintf (out, ") {");
    }

  /* Emit '*(cast(' */
  fprintf (out, "*(%s(", cast);

  /* Emit src[0] */
  C_emit_operand (out, fn, op->src[0]);

  /* Emit '+' */
  fprintf (out, " + ");

  /* Emit src[1] */
  C_emit_operand (out, fn, op->src[1]);

  /* Emit ' = ' */
  fprintf (out, ")) = ");

  /* Emit src[2] */
  C_emit_operand (out, fn, op->src[2]);

  /* Terminate statement, must go inside if */
  fprintf (out, ";");

  /* End predicated if statement (if exists) */
  if (op->pred[0] != NULL)
    {
      fprintf (out, "}");
    }

  /* Finished with this line */
  fprintf (out, "\n");
}

/* Emit post-increment store ops with one dest and four srcs.
 * Will punt if these requirements are not met.
 * 
 * Generates:
 *   *('cast'(src[0]+src[1])) = src[2];
 *   dest[0] = src[0] + src[3];
 *
 *   if (pred[0] != NULL), adds if (pred[0]) {...} around it
 *
 */
void
C_emit_post_store_op (FILE * out, L_Func * fn, L_Oper * op, char *cast)
{
  /* Verify proper number of operands are present */
  C_check_operands ("C_emit_post_store_op", op, 1, 4);

  /* Add instrumentation if tracing memory addresses */
  if (C_insert_probes && C_trace_mem_addrs)
    {
      if (C_custom_profiling)
        C_emit_put_trace_mem_addr (out, fn, op, "?L_TRACE_WRITE",
				   op->src[0], op->src[1]);
      else
        C_emit_put_trace_mem_addr (out, fn, op, "L_TRACE_WRITE",
				   op->src[0], op->src[1]);
    }

  if (L_find_attr (op->attr, "VIPSPILL"))
    {
      fprintf (out, "%s/* VARARG SPILL SUPPRESSED FOR EMULATION */\n",
	       C_indent);
      return;
    }

  /* Set indentation */
  fprintf (out, "%s", C_indent);

  /* Add if () {} around code, if predicated */
  if (op->pred[0] != NULL)
    {
      fprintf (out, "if (");
      C_emit_operand (out, fn, op->pred[0]);
      fprintf (out, ") {");
    }

  /* Emit '*(cast(' */
  fprintf (out, "*(%s(", cast);

  /* Emit src[0] */
  C_emit_operand (out, fn, op->src[0]);

  /* Emit '+' */
  fprintf (out, " + ");

  /* Emit src[1] */
  C_emit_operand (out, fn, op->src[1]);

  /* Emit ' = ' */
  fprintf (out, ")) = ");

  /* Emit src[2] */
  C_emit_operand (out, fn, op->src[2]);

  /* Terminate statement, must go inside if */
  fprintf (out, ";");

  C_emit_operand (out, fn, op->dest[0]);
  fprintf (out, " = ");
  C_emit_operand (out, fn, op->src[0]);
  fprintf (out, " + ");
  C_emit_operand (out, fn, op->src[3]);
  fprintf (out, ";");

  /* End predicated if statement (if exists) */
  if (op->pred[0] != NULL)
    {
      fprintf (out, "}");
    }

  /* Finished with this line */
  fprintf (out, "\n");
}

/* Emit conditional branch ops with zero dests and three srcs.
 * Will punt if these requirements are not met.
 * 
 * Generates:
 *   if (('cast'src[0])'operator'('cast'src[1])) goto src[2];
 *
 *   if (pred[0] != NULL), adds if (pred[0]) {...} around it
 *
 *  'cast' may be NULL, to specify no usage of
 *  that feature (implicit parenthesis, if any, will be removed).
 */
void
C_emit_cond_br_op (FILE * out, L_Func * fn, L_Oper * op,
		   char *cast, char *operator)
{
#if 0
  /* Verify proper number of operands are present */
  C_check_operands ("C_emit_cond_br_op", op, 0, 3);
#endif

  /* Set indentation - Create local scope */
  fprintf (out, "%s", C_indent);

  fprintf (out, "{\n");

  /* Set indentation - Create local comparison condition */
  fprintf (out, "%s", C_indent);

  fprintf (out, "int cond = (");

  /* Add cast to src[0], if not NULL */
  if (cast != NULL)
    fprintf (out, "((%s)", cast);

  /* Emit src[0] */
  C_emit_operand (out, fn, op->src[0]);

  /* Finish cast to src[0], if not NULL */
  if (cast != NULL)
    fprintf (out, ")");

  /* Emit operator between src[0] and src[1] */
  fprintf (out, "%s", operator);

  /* Add cast to src[1], if not NULL */
  if (cast != NULL)
    fprintf (out, "((%s)", cast);

  /* Emit src[1] */
  C_emit_operand (out, fn, op->src[1]);

  /* Finish cast to src[1], if not NULL */
  if (cast != NULL)
    fprintf (out, ")");

  fprintf (out, ");\n");

  /* If there is an epilog counter (EC), then 
     decrement EC if the pred is 0 and the EC > 0 :
     if ( (pred[0] == 0 || !cond ) && rEC > 0 ) EC--;
     if ( !cond && rEC > 0 ) EC--; */

  if (op->src[3])
    {
      /* Set indentation */
      fprintf (out, "%s", C_indent);

      fprintf (out, "if ( ");

      if (op->pred[0])
	{
	  fprintf (out, "(");

	  C_emit_operand (out, fn, op->pred[0]);

	  fprintf (out, " == 0 || ");
	}

      fprintf (out, "!cond");

      if (op->pred[0])
	{
	  fprintf (out, ")");
	}

      fprintf (out, " && ");

      /* Emit src[3] */
      C_emit_operand (out, fn, op->src[3]);

      fprintf (out, " > 0) ");

      /* Emit src[3] */
      C_emit_operand (out, fn, op->src[3]);

      fprintf (out, "--;\n");
    }

  if (op->pred[0] != NULL)
    {
      /* Set indentation */
      fprintf (out, "%s", C_indent);
      fprintf (out, "rr_temp_i = ");
      C_emit_operand (out, fn, op->pred[0]);
      fprintf (out, ";\n");
    }

  /* Emit the code that simulates the rotation of the registers. 
     If rotating, a rotation occurs even if the pred is TRUE
     and the branch falls through. */
  if (L_EXTRACT_BIT_VAL (op->flags, L_OPER_ROTATE_REGISTERS))
    {
      /* Set indentation */
      fprintf (out, "%s", C_indent);

      fprintf (out, "{");

      C_emit_register_rotation (out, fn);

      /* Set indentation */
      fprintf (out, "%s", C_indent);

      fprintf (out, "}\n");
    }

  /* If a stage predicate register file is present, then it
     was previously shifted by emit_register_rotation, but
     assign the stage 0 predicate a 1 value :
     pSTAGE = 1; */

  if (op->dest[1])
    {
      /* Set indentation */
      fprintf (out, "%s", C_indent);

      /* Emit dest[1] */
      C_emit_operand (out, fn, op->dest[1]);

      fprintf (out, " = 1;\n");
    }

  /* If there is no stage predicate then jump :
     if ( cond ) goto cb;
     If there is a stage predicate then jump if true :
     if (pred[0] == 1 && cond ) goto cb;
     If the stage predicate is true or if the new EC >= 1, then
     jump to the target :
     if ((pred[0] == 1 && cond ) || rEC >= 1) goto cb; 
   */

  /* Set indentation */
  fprintf (out, "%s", C_indent);

  fprintf (out, "if ( ");

  if (op->src[3] != NULL)
    {
      fprintf (out, "( ");
    }

  if (op->pred[0] != NULL)
    {
      fprintf (out, " rr_temp_i == 1 && ");
    }

  fprintf (out, "cond ");

  if (op->src[3] != NULL)
    {
      fprintf (out, ") || ");

      /* Emit src[3] */
      C_emit_operand (out, fn, op->src[3]);

      fprintf (out, " >= 1 ");
    }

  fprintf (out, ") goto ");

  /* Emit src[2] as branch target label (flagged via NULL as fn) */
  C_emit_operand (out, NULL, op->src[2]);

  /* Terminate statement, must go inside pred if */
  fprintf (out, ";\n");

  /* Set indentation - Destroy local scope */
  fprintf (out, "%s", C_indent);

  fprintf (out, "}\n");

  /* Add instrumentation if tracing control flow
   * (put_trace(L_TRACE_BRTHRU) is placed after conditional branches 
   * to indicate the branch did not take.)
   */
  if (C_insert_probes && C_trace_control_flow)
    C_emit_put_trace_header (out, fn, op, "L_TRACE_BRTHRU");
}

/* Emit jump ops with zero dests and one src.
 * Will punt if these requirements are not met.
 * 
 * Generates:
 *   goto src[0];
 *
 *   if (pred[0] != NULL), adds if (pred[0]) {...} around it
 */
void
C_emit_jump_op (FILE * out, L_Func * fn, L_Oper * op)
{
#if 0
  /* MCM Disabled for now. */
  /* Verify proper number of operands are present */
  C_check_operands ("C_emit_jump_op", op, 0, 1);

#endif

  /* If there is an epilog counter (EC), then 
     decrement EC if the pred is 0 and the EC > 0 :
     if (pred[0] == 0 && rEC > 0 ) EC--;
     if (rEC > 0 ) EC--; */

  if (op->src[3])
    {
      /* Set indentation */
      fprintf (out, "%s", C_indent);

      fprintf (out, "if (");

      if (op->pred[0])
	{
	  C_emit_operand (out, fn, op->pred[0]);

	  fprintf (out, " == 0 && ");
	}

      /* Emit src[3] */
      C_emit_operand (out, fn, op->src[3]);

      fprintf (out, " > 0) ");

      /* Emit src[3] */
      C_emit_operand (out, fn, op->src[3]);

      fprintf (out, "--;\n");
    }

  if (op->pred[0] != NULL)
    {
      /* Set indentation */
      fprintf (out, "%s", C_indent);
      fprintf (out, "rr_temp_i = ");
      C_emit_operand (out, fn, op->pred[0]);
      fprintf (out, ";\n");
    }

  /* Emit the code that simulates the rotation of the registers. 
     If rotating, a rotation occurs even if the pred is TRUE
     and the branch falls through. */
  if (L_EXTRACT_BIT_VAL (op->flags, L_OPER_ROTATE_REGISTERS))
    {
      /* Set indentation */
      fprintf (out, "%s", C_indent);

      fprintf (out, "{");

      C_emit_register_rotation (out, fn);

      /* Set indentation */
      fprintf (out, "%s", C_indent);

      fprintf (out, "}\n");
    }

  /* If a stage predicate register file is present, then it
     was previously shifted by emit_register_rotation, but
     assign the stage 0 predicate a 1 value :
     pSTAGE = 1; */

  if (op->dest[1])
    {
      /* Set indentation */
      fprintf (out, "%s", C_indent);

      /* Emit dest[1] */
      C_emit_operand (out, fn, op->dest[1]);

      fprintf (out, " = 1;\n");
    }

  /* If there is no stage predicate then jump :
     goto cb;
     If there is a stage predicate then jump if true :
     if (pred[0] == 1) goto cb;
     If the stage predicate is true or if the EC >= 1, then
     jump to the target :
     if (pred[0] == 1 || rEC >= 1) goto cb; 
   */

  if (op->pred[0] == NULL)
    {
      /* Set indentation */
      fprintf (out, "%s", C_indent);

      /* Emit 'goto ' */
      fprintf (out, "goto ");

      /* Emit src[0] as branch target label (flagged via NULL as fn) */
      C_emit_operand (out, NULL, op->src[0]);

      /* Terminate statement, must go inside pred if */
      fprintf (out, ";\n");
    }
  else
    {
      if (op->src[3] == NULL)
	{
	  /* Set indentation */
	  fprintf (out, "%s", C_indent);

	  fprintf (out, "if (");

	  fprintf (out, " rr_temp_i == 1) goto ");

	  /* Emit src[0] as branch target label (flagged via NULL as fn) */
	  C_emit_operand (out, NULL, op->src[0]);

	  /* Terminate statement, must go inside pred if */
	  fprintf (out, ";\n");
	}
      else
	{
	  /* Set indentation */
	  fprintf (out, "%s", C_indent);

	  fprintf (out, "if ((");

	  fprintf (out, " rr_temp_i == 1) || (");

	  /* Emit src[3] */
	  C_emit_operand (out, fn, op->src[3]);

	  fprintf (out, " >= 1)) goto ");

	  /* Emit src[0] as branch target label (flagged via NULL as fn) */
	  C_emit_operand (out, NULL, op->src[0]);

	  /* Terminate statement, must go inside pred if */
	  fprintf (out, ";\n");
	}
    }

  /* put_trace(L_TRACE_BRTHRU) is placed after predicated jumps (if desired)
   * to indicate the branch did not take (primarily for profiling
   * where the predicate's value is not traced).
   */
  if (C_insert_probes && (op->pred[0] != NULL) && C_trace_pred_jump_fall_thru)
    C_emit_put_trace_header (out, fn, op, "L_TRACE_BRTHRU");
}

/* Emit jump_rg ops (hashing jumps) with zero dests and two srcs.
 * Will punt if these requirements are not met.
 * 
 * Generates:
 *   switch (src[0]) {
 *     case 1: goto (prefix)CB_1;
 *     case 2: goto (prefix)CB_2;
 *     ... // for every CB in function 
 *     default: abort();}
 *
 *   if (pred[0] != NULL), adds if (pred[0]) {...} around it
 *
 */
void
C_emit_jump_rg_op (FILE * out, L_Func * fn, L_Oper * op)
{
  L_Datalist_Element *jumptbl_element;
  L_Data *data;
  INT_Symbol_Table *cb_id_table;
  INT_Symbol *cb_id_symbol;
  Heap *cb_id_heap;
  char func_buf[10000];
  int cb_id;
  char *cb_label;

  /* Verify proper number of operands are present */
  C_check_operands ("C_emit_jump_rg_op", op, 0, 2);

  /* Add instrumentation if tracing jump_rg_src1 (& control flow) */
  if (C_insert_probes && C_trace_control_flow && C_trace_jump_rg_src1)
    {
      /* Write second operand of hashing jump to the trace buffer

       * If instrumenting Lhpl_pd code, the "second operand" is a btr
       * register, not the condition that the hashing jump is based on.
       * Since this info is not available, simply write out -1000 (for
       * lack of a better number), to tell Lprofile to put all the
       * weight on the "default" path. 
       */
      if (L_is_ctype_btr (op->src[1]))
	{
	  C_emit_put_trace_int (out, fn, op, "?L_TRACE_SWITCH_CASE", -1000);
	}
      /* Otherwise, put the contents src[1] into the trace */
      else
	{
	  C_emit_put_trace_int_operand (out, fn, op,
					"?L_TRACE_SWITCH_CASE", op->src[1]);
	}
    }

  /* Set indentation */
  fprintf (out, "%s", C_indent);

  /* Add if () {} around code, if predicated */
  if (op->pred[0] != NULL)
    {
      fprintf (out, "if (");
      C_emit_operand (out, fn, op->pred[0]);
      fprintf (out, ") {");
    }

  /* Emit 'switch (' */
  fprintf (out, "switch (");

  /* Emit src[0] */
  C_emit_operand (out, fn, op->src[0]);

  /* Finish first line of switch */
  fprintf (out, ") {");

  /* Make int symbol table to hold cb id's referenced */
  cb_id_table = INT_new_symbol_table ("cb_id", 0);

  /* Scan thru all the hash tables, looking for cb references */
  for (jumptbl_element = fn->jump_tbls->first_element; jumptbl_element;
       jumptbl_element = jumptbl_element->next_element)
    {
      /* Get data for ease of use */
      data = jumptbl_element->data;

      /* Look for hash table initialization data items */
      if ((data->type == L_INPUT_WI || (data->type == L_INPUT_WQ)) && 
	  (data->value->type == L_EXPR_LABEL))
	{
	  /* Sanity check, make sure will not exceed buffer */
	  if (strlen (data->value->value.l) >= sizeof (func_buf))
	    {
	      L_punt ("C_emit_fn_jump_tables: label too long (%i):\n"
		      "%s",
		      strlen (data->value->value.l), data->value->value.l);
	    }

	  /* Get cb id using Mspec call */
	  if (!M_is_cb_label (data->value->value.l, func_buf, &cb_id))
	    {
	      L_punt ("C_emit_fn_jump_tables: cb label expected!");
	    }

	  /* If id not already in table, add it */
	  if (INT_find_symbol (cb_id_table, cb_id) == NULL)
	    {
	      INT_add_symbol (cb_id_table, cb_id, data->value->value.l);
	    }
	}
    }

  /* Sanity check, better have found at least one cb id! */
  if (cb_id_table->head_symbol == NULL)
    {
      L_punt ("C_emit_jump_rg: At least one cb label expected!");
    }

  /* Sort cb_ids for readability using heap library */
  cb_id_heap = Heap_Create (HEAP_MIN);

  /* Add every cb id found to the heap */
  for (cb_id_symbol = cb_id_table->head_symbol; cb_id_symbol != NULL;
       cb_id_symbol = cb_id_symbol->next_symbol)
    {
      /* Get cb id and label for ease of use */
      cb_id = cb_id_symbol->value;
      cb_label = (char *) cb_id_symbol->data;

      /* Add cb_id to heap */
      Heap_Insert (cb_id_heap, (void *) cb_label, (double) cb_id);
    }


  /* Emit case for every cb id found in hash tables for this function */
  while ((cb_label = (char *) Heap_ExtractTop (cb_id_heap)) != ((int) NULL))
    {
      /* Get cb id again using Mspec call */
      if (!M_is_cb_label (cb_label, func_buf, &cb_id))
	{
	  L_punt ("C_emit_fn_jump_tables: cb label expected!");
	}
      fprintf (out, "\n"
	       "%s  case %s%s:  goto %scb_%i;", C_indent,
	       C_prefix, cb_label, C_prefix, cb_id);
    }

  /* Free heap, nothing to free (flagged with NULL) */
  cb_id_heap = Heap_Dispose (cb_id_heap, NULL);

  /* Sanity check, emit default case that punts */
  fprintf (out, "\n" "%s  default:  abort(); /* Sanity check */", C_indent);

  /* End switch */
  fprintf (out, "}");

  /* End predicated if statement (if exists) */
  if (op->pred[0] != NULL)
    {
      fprintf (out, "}");
    }

  /* Finished with this line */
  fprintf (out, "\n");
}

/* Emit jsr ops with zero dests and one src.
 * Will punt if these requirements are not met.
 * 
 * Generates:
 *   mac_P15_x = src[0](...);
 *
 * Uses the call_info attribute to figure out how to construct
 * the function call and what type of return value register expected.
 *
 * Expects C_emit_jsr_op to handle predication and instrumentation aspects.
 */
void
C_emit_normal_jsr_op (FILE * out, L_Func * fn, L_Oper * op)
{
  char return_type_buf[TYPE_BUF_SIZE];
  char all_parm_type_buf[TYPE_BUF_SIZE];
  char cast_buf[TYPE_BUF_SIZE];
  int ret_type;
  L_Attr *ret_attr;
  L_Operand *ret_reg;
  L_Operand *temp_macro_operand;

  /* Verify proper number of operands are present */
  C_check_operands ("C_emit_normal_jsr_op", op, 0, 1);

  /* Has the return register been specified thru a ret attribute? */
  if ((ret_attr = L_find_attr (op->attr, "ret")) != NULL)
    {
      /* Get return register from first field */
      ret_reg = ret_attr->field[0];

      /* Sanity check, better be a register or macro */
      if (!L_is_reg (ret_reg) && !L_is_macro (ret_reg))
	{
	  L_print_oper (stderr, op);
	  L_punt ("C_emit_normal_jsr_op: Unexpected 'ret' attribute "
		  "contents!");
	}
    }

  /* Otherwise, set return_reg to NULL to flag better be void */
  else
    {
      ret_reg = NULL;
    }

  /* Get the return type and parameter specifiers from the jsr call_info
   * attribute.  Use smallest buffers for these, since buf size is checked.
   */
  L_get_call_info (NULL, op, op->attr, return_type_buf, all_parm_type_buf,
		   sizeof (return_type_buf));


  /* Convert return type into Lcode ctype (or CTYPE_STRUCT, if required). */
  ret_type = L_convert_type_to_ctype (return_type_buf);

  /* Handle each CTYPE appropriately */
  switch (ret_type)
    {
    case L_CTYPE_INT:
      if (C_native_machine_ctype == L_CTYPE_INT)
	{
	  /* Expect explicit return register specifier */
	  if (ret_reg != NULL)
	    {
	      C_emit_operand (out, fn, ret_reg);
	    }
	  /* Otherwise, punt */
	  else
	    {
	      L_print_oper (stderr, op);
	      L_punt ("C_emit_normal_jsr_op: Expect int 'ret' attr!");
	    }
	  fprintf (out, " = (int) ");
	}
      else
	L_punt ("C_emit_normal_jsr_op: Unexpected int ret attr found.");
      break;

    case L_CTYPE_LLONG:
#if 0
      if (C_native_machine_ctype == L_CTYPE_LLONG)
	{
	  /* Expect explicit return register specifier */
	  if (ret_reg != NULL)
	    {
	      C_emit_operand (out, fn, ret_reg);
	    }
	  /* Otherwise, punt */
	  else
	    {
	      L_print_oper (stderr, op);
	      L_punt ("C_emit_normal_jsr_op: Expect longlong 'ret' attr!");
	    }
	  fprintf (out, " = (longlong) ");
	}
      else
	L_punt ("C_emit_normal_jsr_op: Unexpected longlong ret attr found.");
#endif
      /* Expect explicit return register specifier */
      if (ret_reg != NULL)
      {
        temp_macro_operand = L_new_macro_operand (L_MAC_OP, C_native_machine_ctype, L_PTYPE_NULL);
        fprintf(out, "*((long long *)");
        C_emit_operand (out, fn, temp_macro_operand);
        fprintf(out, ")");
        L_delete_operand(temp_macro_operand);
      }
      /* Otherwise, punt */
      else
      {
        L_print_oper (stderr, op);
        L_punt ("C_emit_normal_jsr_op: Expect longlong 'ret' attr!");
      }
      fprintf (out, " = (long long) ");

      break;

    case L_CTYPE_FLOAT:
      /* Expect explicit return register specifier */
      if (ret_reg != NULL)
	{
	  C_emit_operand (out, fn, ret_reg);
	}
      /* Otherwise, punt */
      else
	{
	  L_print_oper (stderr, op);
	  L_punt ("C_emit_normal_jsr_op: Expect float 'ret' attr!");
	}
      fprintf (out, " = (float) ");
      break;

    case L_CTYPE_DOUBLE:
      /* Expect explicit return register specifier */
      if (ret_reg != NULL)
	{
	  C_emit_operand (out, fn, ret_reg);
	}
      /* Otherwise, punt */
      else
	{
	  L_print_oper (stderr, op);
	  L_punt ("C_emit_normal_jsr_op: Expect double 'ret' attr!");
	}
      fprintf (out, " = (double) ");
      break;

    case L_CTYPE_VOID:
      /* Do nothing */
      break;

    case CTYPE_STRUCT:
      /* Sanity check, better have ret_st attribute */
      if (L_find_attr (op->attr, "ret_st") == NULL)
	{
	  L_print_oper (stderr, op);
	  L_punt ("C_emit_normal_jsr_op: Expect 'ret_st' attr!");
	}

      /* Convert type into C format for cast of parameter to
       * proper C type.
       */
      L_convert_type_to_C_format (cast_buf, return_type_buf, "");

      /* Emit derefence and cast to appropriate C type */
      fprintf (out, " *((%s*) ", cast_buf);

      /* Expect explicit return register specifier */
      if (ret_reg != NULL)
	{
	  C_emit_operand (out, fn, ret_reg);
	}
      /* Otherwise, punt */
      else
	{
	  L_print_oper (stderr, op);
	  L_punt ("C_emit_normal_jsr_op: Expect struct/union 'ret' attr!");
	}
      fprintf (out, ") = ");
      break;

    default:
      L_punt ("C_emit_normal_jsr_op: unknown return type %i!", ret_type);
    }

  /* Handle function name in label case */
  if (L_is_label (op->src[0]))
    {
      /* Emit function name (directly, since C_emit_operand does not
       * do the right thing here).
       */
      fprintf (out, "%s (", C_true_name (op->src[0]->value.l));
    }

  /* Handle function name in register case */
  else if (L_is_reg (op->src[0]) || L_is_macro (op->src[0]) ||
	   L_is_int_constant (op->src[0]))
    {
      /* Start cast around function pointer */
      fprintf (out, "(");

      /* Emit cast to function pointer of appropriate type,
       * if Ansi-C, must also include parameters in cast to
       * make sure floats get passed correctly
       */
      C_emit_func_ptr_cast (out, return_type_buf, all_parm_type_buf);

      /* Emit register that contains function address */
      C_emit_operand (out, fn, op->src[0]);

      /* Finish cast and start parameter listing */
      fprintf (out, ") (");
    }

  /* Otherwise, punt */
  else
    {
      L_print_oper (stderr, op);
      L_punt ("C_emit_normal_jsr_op: Expected src[0] type!");
    }
  C_emit_jsr_parms (out, fn, op, all_parm_type_buf, 0);

  /* Terminate statement, must go inside pred if */
  fprintf (out, ");");
}

int C_emit_longlong_setup(FILE *out, L_Func *fn, L_Oper *op)
{
  L_Attr *tr_attr;
  char return_type_buf[TYPE_BUF_SIZE];
  char all_parm_type_buf[TYPE_BUF_SIZE];
  char parm_type_buf[TYPE_BUF_SIZE];
  char *parse_ptr = all_parm_type_buf;
  int num_llong_parms = 0;
  int parm_type;
  int thru_reg_count;
  int param_index = 0;
  int mac_counter = 0;

  L_get_call_info (NULL, op, op->attr, return_type_buf, all_parm_type_buf,
                   sizeof (return_type_buf));

  tr_attr = L_find_attr (op->attr, "tr");
  if(!tr_attr) return 0;

  thru_reg_count = tr_attr->max_field;

  while(*parse_ptr != 0) {
    L_get_next_param_type (parm_type_buf, &parse_ptr);
    parm_type = L_convert_type_to_ctype (parm_type_buf);

    thru_reg_count--;
    if(parm_type == L_CTYPE_LLONG) {
      thru_reg_count--;
      num_llong_parms++;
    }
    if(thru_reg_count <= 0) break;
  }

  if(num_llong_parms == 0) {
    fprintf(out, "%sl_emul_llong_buf = 0;\n", C_indent);
    return 0;
  }

  fprintf(out, "%sl_emul_llong_buf = (union l_emul_llong_struct *)", C_indent);
  fprintf(out, "malloc(%d * sizeof(union l_emul_llong_struct));\n", num_llong_parms);

  parse_ptr = all_parm_type_buf;
  thru_reg_count = tr_attr->max_field;
  param_index = 0;
  mac_counter = 0;

  while(*parse_ptr != 0) {
    L_get_next_param_type (parm_type_buf, &parse_ptr);
    parm_type = L_convert_type_to_ctype (parm_type_buf);

    thru_reg_count--;

    if(parm_type == L_CTYPE_LLONG) {
      thru_reg_count--;
      fprintf(out, "%sl_emul_llong_buf[%d].parts.lo = ", C_indent, param_index);
      C_emit_int_mac(out, L_MAC_P0 + mac_counter);
      fprintf(out, ";\n");
      fprintf(out, "%sl_emul_llong_buf[%d].parts.hi = ", C_indent, param_index);
      C_emit_int_mac(out, L_MAC_P0 + mac_counter+1);
      fprintf(out, ";\n");
      mac_counter += 2;

      param_index++;
    }
    else
      mac_counter++;

    if(thru_reg_count <= 0) break;
  }
  return 1;
}

/* Emit jsr ops with zero dests and one src.
 * Will punt if these requirements are not met.
 *
 * Uses C_emit_normal_jsr_op() and C_emit_builtin_jsr_op() to
 * actually emit core jsr code.  
 * 
 * This function implements the predication and tracing
 * aspects of emitting a jsr.  It also picks the
 * proper function to call.
 */
void
C_emit_jsr_op (FILE * out, L_Func * fn, L_Oper * op)
{
  L_Attr *ret_attr;
  L_Attr *trse_attr, *tmso_attr;
  int fld, tmp_offset;
  int pr, struct_start_param, struct_end_param;

  L_Operand *parm_reg;
  L_Operand *ret_reg;
  L_Attr *tr_attr;
  int parm_mac_id;
  int ret_mac_id;
  int llong_params_present = 0;

  /* KVM : If the callee has long long arguments, set up some
   * variables so that the correct values can be passed to the
   * actual call.
   */

  llong_params_present = C_emit_longlong_setup(out, fn, op);

  /* For O_im code under the IMPACT arch, some int macros
   *   might be shadowing floating point macros and should be
   *  removed for emulation 
   */
  if (((M_arch == M_IMPACT) || (M_arch == M_PLAYDOH)) &&
      (tr_attr = L_find_attr (op->attr, "tr")))
    {
      int i, effective_P;
      int after_float, new_max;
      char return_type_buf[TYPE_BUF_SIZE];
      char all_parm_type_buf[TYPE_BUF_SIZE];
      char parm_type_buf[TYPE_BUF_SIZE];
      char *parse_ptr = all_parm_type_buf;
      int parm_type;
      int empty = 0;

      effective_P = 0;
      after_float = 0;

      L_get_call_info (NULL, op, op->attr, return_type_buf, all_parm_type_buf,
		       sizeof (return_type_buf));
    
      for (i=0; i<tr_attr->max_field; i++)
	{
	  /* Get the next parameter type */
	  if (*parse_ptr != 0)
	    L_get_next_param_type (parm_type_buf, &parse_ptr);
	  else
	    empty = 1;

	  if (tr_attr->field[i]->value.mac == 15)
	    {
	      /* This tr is not really an input parameter
		 and will not have a matching call info field. */
	    }
	  else if (tr_attr->field[i]->value.mac >= 4)
	    {
	      if (empty == 1)
		L_punt("C_emit_jsr_op: "
		       "Not expecting call_info to be empty. fn %s, op %d\n",
		       fn->name, op->id);

	      after_float = 1;
	      /* If float then P += 1 to count off the matching int reg,
		 if double, then P += 2 for the 2 int regs. */
	      parm_type = L_convert_type_to_ctype (parm_type_buf);
	      if (parm_type == L_CTYPE_FLOAT)
		effective_P += 1;
	      else if (parm_type == L_CTYPE_DOUBLE)
		effective_P += 2;
	      else
		L_punt("Unexpected parm_type %d.\n", parm_type);
	    }
	  else
	    {
	      if (tr_attr->field[i]->value.mac > effective_P)
		{
		  DB_print_oper(op);
		  L_punt("Not expecting gaps in P macros op%d\n",op->id);
		}
	      else if (tr_attr->field[i]->value.mac == effective_P)
		effective_P++;
	      else
		{
		  /* Out of order P macro found */
		  if (after_float)
		    break;
		}
	    }
	}
      new_max = i;
      for (; i<tr_attr->max_field; i++)
	{
	  printf("Deleteing op%d tr attr field %d\n",
		 op->id, i);
	  L_delete_attr_field(tr_attr,i);	
	}
      tr_attr->max_field = new_max;
    }

  /* In Limpact generated code jsrs may have RETADDR as a dest */
  if (((M_arch == M_IMPACT) || (M_arch == M_PLAYDOH)) &&
      op->dest[0] && L_is_macro(op->dest[0]) &&
      op->dest[0]->value.mac == L_MAC_RETADDR)
    {
      L_delete_operand(op->dest[0]);
      op->dest[0] = NULL;
    }

  /* Verify proper number of operands are present */
  C_check_operands ("C_emit_jsr_op", op, 0, 1);

  /* HANDLE SETUP FOR STRUCT PARM PASSING */
  trse_attr = L_find_attr (op->attr, "trse");
  tmso_attr = L_find_attr (op->attr, "tmso");
  if (trse_attr)
    {
      for (fld = 0; fld < trse_attr->max_field; fld += 2)
	{
	  if (L_is_int_constant (trse_attr->field[fld]) &&
	      L_is_int_constant (trse_attr->field[fld + 1]))
	    {
	      struct_start_param = trse_attr->field[fld]->value.i;
	      struct_end_param = trse_attr->field[fld + 1]->value.i;

	      /* Struct actually passed through the parm registers */
	      if (!tmso_attr)
		L_punt ("tmso_attr for thru reg not found\n");
	      if (!L_is_int_constant (tmso_attr->field[fld / 2]))
		{
		  L_print_attr (stderr, tmso_attr);
		  L_punt ("tmso field for thru reg [%d] not found\n",
			  fld / 2);
		}
	      tmp_offset = ITicast (tmso_attr->field[fld / 2]->value.i);

	      /* Copy the out going params into overlay area */
              for (pr = struct_start_param; pr <= struct_end_param; pr++)
              {
                /* Amir: size of operands is different
                   in arm. Therefore I changed longlong to long*/
                if (M_arch == M_ARM)
                  fprintf (out, "%s*((long *)( ", C_indent);
                else
                  fprintf (out, "%s*((longlong *)( ", C_indent);
                C_emit_int_mac (out, L_MAC_OP);
                fprintf (out, " + %i)) = ", tmp_offset);
                C_emit_int_mac (out, L_MAC_P0 + pr);
                fprintf (out, ";\n");
                /* Amir: size of operands is different
                   in arm. Therefore I changed 8 to 4*/
                if (M_arch == M_ARM)
                  tmp_offset += 4;
                else
                  tmp_offset += 8;
              }
            }
	}
    }
  /* HANDLE SETUP FOR STRUCT PARM PASSING */

  /* Set indentation */
  fprintf (out, "%s", C_indent);

  /* Add if () {} around code, if predicated */
  if (op->pred[0] != NULL)
    {
      fprintf (out, "if (");
      C_emit_operand (out, fn, op->pred[0]);
      fprintf (out, ") {");
    }

  /* Detect calls starting with __builtin */
  if (L_is_label (op->src[0]) && C_is_builtin_function (op->src[0]->value.l))
    {
      C_emit_builtin_jsr_op (out, fn, op);
    }
  /* Otherwise, emit the "normal" jsr */
  else
    {
      C_emit_normal_jsr_op (out, fn, op);
    }

  /* If in tracing mode, detect forks and insert code that causes
   * only the parent process to be traced (the correct decision for
   * 023.eqntott
   */
  if (C_insert_probes && L_is_label (op->src[0]) &&
      C_matches_true_name (op->src[0]->value.l, "fork"))
    {
      /* Get the return register operand from the ret attribute */
      if ((ret_attr = L_find_attr (op->attr, "ret")) == NULL)
	{
	  L_print_oper (stderr, op);
	  L_punt ("C_emit_jsr_op: ret attr expected!");
	}

      /* Emit 'if (ret == 0) _TR_IN_PARENT = 0; (comment)' */
      fprintf (out, "\n%sif (", C_indent);
      C_emit_operand (out, fn, ret_attr->field[0]);
      fprintf (out, " == 0) _TR_IN_PARENT = 0; "
	       "/* Trace only the parent process */");
    }

  /* If an exception handler was installed by the user application,
     reinstall the speculation exception handler (for SIGSEGV) at the next
     speculative load, even if the newly installed handler was not
     for SIGSEGV (be safe).  A better solution would be to install and
     remove the speculation exception handler around the speculated
     load.  Another solution would be to install the speculation 
     exception handler for SIGSEGV everywhere but remember that the 
     application installed its own handler.  Upon taking an exception,
     the speculation handler could detect if the exception was from a
     non-speculated instruction in which case it would call the remembered
     application exception handler. MCM 01/29/00 */
  if (L_is_label (op->src[0]) &&
      C_matches_true_name (op->src[0]->value.l, "signal"))
    {

      fprintf (out, "\n%s_EM_INSTALL_TRAP_HANDLER = 1; "
	       "/* Reinstall speculation signal handler */", C_indent);
    }

  /* End predicated if statement (if exists) */
  if (op->pred[0] != NULL)
    {
      fprintf (out, "}");
    }

  /* Finished with this line */
  fprintf (out, "\n");

  /* If tracing control flow, emit where we are after returning from jsr. */
  if (C_insert_probes && C_trace_control_flow)
    {
      C_emit_put_trace_post_jsr (out, fn, op,
				 C_trace_jsr_id, C_trace_func_id);
    }

  /* SER 20040520: For HCH MICRO '04 */
  if (C_insert_probes && C_trace_mem_addrs && C_custom_profiling &&
      L_is_label(op->src[0]))
    {
      if (strcmp(C_true_name (op->src[0]->value.l),"malloc") == 0)
	{
	  ret_attr = L_find_attr (op->attr, "ret");
	  ret_reg = ret_attr->field[0];
	  ret_mac_id = ret_reg->value.mac;
	  tr_attr = L_find_attr (op->attr, "tr");
	  parm_reg = tr_attr->field[0];
	  parm_mac_id = parm_reg->value.mac;
	  fprintf (out, "    _EM_put_trace2(%smac_%s_i, %smac_%s_i);\n",
		   C_prefix, C_macro_name(ret_mac_id),
		   C_prefix, C_macro_name(parm_mac_id));
	}
      else if (strcmp(C_true_name (op->src[0]->value.l),"calloc") == 0)
	{
	  L_Operand * parm_reg2;
	  int parm_mac_id2;

	  ret_attr = L_find_attr (op->attr, "ret");
	  ret_reg = ret_attr->field[0];
	  ret_mac_id = ret_reg->value.mac;
	  tr_attr = L_find_attr (op->attr, "tr");
	  parm_reg = tr_attr->field[0];
	  parm_reg2 = tr_attr->field[1];
	  parm_mac_id = parm_reg->value.mac;
	  parm_mac_id2 = parm_reg2->value.mac;
	  fprintf (out, "    _EM_put_trace2(%smac_%s_i, "
		   "%smac_%s_i * %smac_%s_i);\n",
		   C_prefix, C_macro_name(ret_mac_id),
		   C_prefix, C_macro_name(parm_mac_id),
		   C_prefix, C_macro_name(parm_mac_id2));
	}
      else if (strcmp(C_true_name (op->src[0]->value.l),"free") == 0)
	{
	  tr_attr = L_find_attr (op->attr, "tr");
	  parm_reg = tr_attr->field[0];
	  parm_mac_id = parm_reg->value.mac;
	  fprintf (out, "    _EM_put_trace(%smac_%s_i);\n",
		   C_prefix, C_macro_name(parm_mac_id));
	}
    }
      
  /* HCH: 11-02-01 */
  if (C_trace_objects)
    {
      if ((L_is_label(op->src[0])) &&
	  ((strcmp(C_true_name (op->src[0]->value.l),"malloc") == 0) ||
	  (strcmp(C_true_name (op->src[0]->value.l),"calloc") == 0) ))
	{
	  fprintf(stderr, "ALLOC found\n");
	  ret_attr = L_find_attr (op->attr, "ret");
	  ret_reg = ret_attr->field[0];
	  ret_mac_id =  ret_reg->value.mac;
	  tr_attr = L_find_attr (op->attr, "tr");
	  parm_reg = tr_attr->field[0];
	  parm_mac_id =  parm_reg->value.mac;
	  fprintf (out, "   _EM_put_trace(L_TRACE_ASYNCH);\n");
	  fprintf (out, "   _EM_put_trace3(L_TRACE_OBJ_HEAP,"
		   "%smac_%s_i,%smac_%s_i);\n", 
		   C_prefix, C_macro_name(ret_mac_id), 
		   C_prefix, C_macro_name(parm_mac_id));
	}
      if ((L_is_label(op->src[0])) &&
	  (strcmp(C_true_name (op->src[0]->value.l),"free") == 0))
	{
	  fprintf(stderr, "FREE found\n");
	  tr_attr = L_find_attr (op->attr, "tr");
	  parm_reg = tr_attr->field[0];
	  parm_mac_id =  parm_reg->value.mac;
	  fprintf (out, "   _EM_put_trace(L_TRACE_ASYNCH);\n");
	  fprintf (out, "   _EM_put_trace3(L_TRACE_OBJ_HEAP,"
		   "%smac_%s_i,0);\n", 
		   C_prefix, C_macro_name(parm_mac_id));
	}

    }

  /* KVM : Free the l_emul_llong_buf.
   */
  if(llong_params_present)
    fprintf(out, "%sif(l_emul_llong_buf) free(l_emul_llong_buf);\n", C_indent);

  /* Increment jsr_id count */
  C_trace_jsr_id++;
}


/* Emit rts ops with zero dests and zero srcs.
 * Will punt if these requirements are not met.
 * 
 * Generates:
 *   return (mac_P15_x); or return (); as appropriate.
 *
 * Uses the fn's call_info attribute to figure out what type of return value 
 *  register expected.
 *
 *   if (pred[0] != NULL), adds if (pred[0]) {...} around it
 */
void
C_emit_rts_op (FILE * out, L_Func * fn, L_Oper * op)
{
  char return_type_buf[TYPE_BUF_SIZE];
  char cast_buf[TYPE_BUF_SIZE];
  int ret_type;
  L_Attr *tr_attr;
  L_Operand *ret_reg;
  L_Operand *temp_macro_operand;

  /* First eliminate RETADDR as a source from the return */
  if (((M_arch == M_IMPACT) || (M_arch == M_PLAYDOH)) &&
      op->src[0] && L_is_macro(op->src[0]) &&
      op->src[0]->value.mac == L_MAC_RETADDR)
    {
      L_delete_operand(op->src[0]);
      op->src[0] = NULL;
    } 

  /* Verify proper number of operands are present */
  C_check_operands ("C_emit_rts_op", op, 0, 0);

  /* Set indentation */
  fprintf (out, "%s", C_indent);

  /* Add if () {} around code, if predicated */
  if (op->pred[0] != NULL)
    {
      fprintf (out, "if (");
      C_emit_operand (out, fn, op->pred[0]);
      fprintf (out, ") {");
    }

  if (C_trace_objects)
    {
      fprintf (out, "   _EM_put_trace(L_TRACE_ASYNCH);\n");
      fprintf (out, "   _EM_put_trace3(L_TRACE_OBJ_STAK,%d,%d);\n", 0, 0);
    }

  /* Start the return statement */
  fprintf (out, "return");

  /* Has the return register been specified thru a tr attribute
   * (or a utr attribute if there is a return register but
   *  return value is undefined)? */
  if (((tr_attr = L_find_attr (op->attr, "tr")) != NULL) ||
      ((tr_attr = L_find_attr (op->attr, "utr")) != NULL))
    {
      /* Get return register from first field */
      ret_reg = tr_attr->field[0];

      /* Sanity check, better be a register or macro */
      if (!L_is_reg (ret_reg) && !L_is_macro (ret_reg))
	{
	  L_print_oper (stderr, op);
	  L_punt ("C_emit_rts_op: Unexpected 'tr' or 'utr' "
		  "attribute contents!");
	}
    }

  /* Otherwise, set return_reg to NULL to flag better be void type */
  else
    {
      ret_reg = NULL;
    }

  /* Get the return type specifier from the function attribute */
  L_get_call_info (fn, op, fn->attr, return_type_buf, NULL,
		   sizeof (return_type_buf));

  /* Convert return type into Lcode ctype (or CTYPE_STRUCT, if required). */
  ret_type = L_convert_type_to_ctype (return_type_buf);

  /* Convert return type to formatted string using C conventions,
   * Passing "" as the parameter name makes it appropriate for a cast
   */
  L_convert_type_to_C_format (cast_buf, return_type_buf, "");


  /* Handle each CTYPE appropriately */
  switch (ret_type)
    {
    case L_CTYPE_INT:
      if (C_native_machine_ctype == L_CTYPE_INT)
	{
	  /* Emit cast to appropriate C type */
	  fprintf (out, " ((%s) ", cast_buf);

	  /* Expect explicit return register specifier */
	  if (ret_reg != NULL)
	    {
	      C_emit_operand (out, fn, ret_reg);
	    }
	  /* Otherwise, punt */
	  else
	    {
	      L_print_oper (stderr, op);
	      L_punt ("C_emit_rts_op: Expect int 'ret' attr!");
	    }

	  /* Emit closing ')' */
	  fprintf (out, ")");
	}
      else
	L_punt ("C_emit_rts_op: Unexpected int 'ret' attr!");

      break;

    case L_CTYPE_LLONG:
#if 0
      if (C_native_machine_ctype == L_CTYPE_LLONG)
	{
	  /* Emit cast to appropriate C type */
	  fprintf (out, " ((%s) ", cast_buf);

	  /* Expect explicit return register specifier */
	  if (ret_reg != NULL)
	    {
	      C_emit_operand (out, fn, ret_reg);
	    }
	  /* Otherwise, punt */
	  else
	    {
	      L_print_oper (stderr, op);
	      L_punt ("C_emit_rts_op: Expect longlong 'ret' attr!");
	    }

	  /* Emit closing ')' */
	  fprintf (out, ")");
	}
      else
	L_punt ("C_emit_rts_op: Unexpected longlong 'ret' attr!");
#endif
      /* Emit cast to appropriate C type */
      fprintf (out, " ((%s) ", cast_buf);

      /* Expect explicit return register specifier */
      if (ret_reg != NULL)
      {
        temp_macro_operand = L_new_macro_operand (L_MAC_IP, C_native_machine_ctype, L_PTYPE_NULL);
        fprintf(out, "(*((long long *)");
        C_emit_operand (out, fn, temp_macro_operand);
        fprintf(out, "))");
        L_delete_operand(temp_macro_operand);
      }
      /* Otherwise, punt */
      else
      {
        L_print_oper (stderr, op);
        L_punt ("C_emit_rts_op: Expect longlong 'ret' attr!");
      }

      /* Emit closing ')' */
      fprintf (out, ")");

      break;

    case L_CTYPE_FLOAT:
      /* Emit cast to appropriate C type */
      fprintf (out, " ((%s) ", cast_buf);

      /* Expect explicit return register specifier */
      if (ret_reg != NULL)
	{
	  C_emit_operand (out, fn, ret_reg);
	}
      /* Otherwise, punt */
      else
	{
	  L_print_oper (stderr, op);
	  L_punt ("C_emit_rts_op: Expect float 'ret' attr!");
	}

      /* Emit closing ')' */
      fprintf (out, ")");
      break;

    case L_CTYPE_DOUBLE:
      /* Emit cast to appropriate C type */
      fprintf (out, " ((%s) ", cast_buf);

      /* Expect explicit return register specifier */
      if (ret_reg != NULL)
	{
	  C_emit_operand (out, fn, ret_reg);
	}
      /* Otherwise, punt */
      else
	{
	  L_print_oper (stderr, op);
	  L_punt ("C_emit_rts_op: Expect double 'ret' attr!");
	}

      /* Emit closing ')' */
      fprintf (out, ")");
      break;


    case CTYPE_STRUCT:
      /* Emit derefence and cast to appropriate C type */
      fprintf (out, " *((%s*) ", cast_buf);

      /* Expect explicit return register specifier */
      if (ret_reg != NULL)
	{
	  C_emit_operand (out, fn, ret_reg);
	}
      /* Otherwise, punt */
      else
	{
	  L_print_oper (stderr, op);
	  L_punt ("C_emit_rts_op: Expect struct/union 'ret' attr!");
	}

      /* Emit closing ')' */
      fprintf (out, ")");
      break;

    case L_CTYPE_VOID:
      /* Do nothing */
      break;

    default:
      L_punt ("C_emit_rts_op: unknown return type %i!", ret_type);
    }

  /* Terminate statement, must go inside pred if */
  fprintf (out, ";");

  /* End predicated if statement (if exists) */
  if (op->pred[0] != NULL)
    {
      fprintf (out, "}");
    }

  /* Finished with this line */
  fprintf (out, "\n");
}

/* Emit pred set/clear ops with one dest and zero srcs.
 * Will punt if these requirements are not met.
 * 
 * Generates:
 *   dest[0] = 'value';
 *
 *   if (pred[0] != NULL), adds if (pred[0]) {...} around it
 *
 */
void
C_emit_pred_set_op (FILE * out, L_Func * fn, L_Oper * op, int value)
{
  /* Verify proper number of operands are present */
  C_check_operands ("C_emit_pred_set_op", op, 1, 0);

  /* These operations may not be predicated!  They are implemented
   * by a pred masking instruction that sets/clears up to 32 predicates 
   * at a time!
   */
  if (op->pred[0] != NULL)
    {
      L_print_oper (stderr, op);
      L_punt ("C_emit_pred_set_op: may not be predicated!");
    }

  /* Set indentation */
  fprintf (out, "%s", C_indent);

  /* Add if () {} around code, if predicated */
  if (op->pred[0] != NULL)
    {
      fprintf (out, "if (");
      C_emit_operand (out, fn, op->pred[0]);
      fprintf (out, ") {");
    }

  /* Emit 'dest[0] = value;' */
  C_emit_operand (out, fn, op->dest[0]);
  fprintf (out, " = %i;", value);

  /* End predicated if statement (if exists) */
  if (op->pred[0] != NULL)
    {
      fprintf (out, "}");
    }

  /* Finished with this line */
  fprintf (out, "\n");
}

/* Emit pred copy ops with one dest and one srcs.
 * Will punt if these requirements are not met.
 * 
 * Generates:
 *   dest[0] = src[0];
 *
 *   if (pred[0] != NULL), adds if (pred[0]) {...} around it
 *
 */
void
C_emit_pred_copy_op (FILE * out, L_Func * fn, L_Oper * op)
{
  /* Verify proper number of operands are present */
  C_check_operands ("C_emit_pred_copy_op", op, 1, 1);

  /* These operations may not be predicated!  They are implemented
   * by a pred masking instruction that sets/clears up to 32 predicates 
   * at a time!
   */
  if (op->pred[0] != NULL)
    {
      L_print_oper (stderr, op);
      L_punt ("C_emit_pred_copy_op: may not be predicated!");
    }

  /* Set indentation */
  fprintf (out, "%s", C_indent);

  /* Add if () {} around code, if predicated */
  if (op->pred[0] != NULL)
    {
      fprintf (out, "if (");
      C_emit_operand (out, fn, op->pred[0]);
      fprintf (out, ") {");
    }

  /* Emit 'dest[0] = src[0];' */
  C_emit_operand (out, fn, op->dest[0]);
  fprintf (out, " = ");
  C_emit_operand (out, fn, op->src[0]);
  fprintf (out, ";");

  /* End predicated if statement (if exists) */
  if (op->pred[0] != NULL)
    {
      fprintf (out, "}");
    }

  /* Finished with this line */
  fprintf (out, "\n");
}

/* Emit predicate def ops with 1 or 2 dests and 2 srcs.
 * Will punt if these requirements are not met.
 * 
 * Uses C_emit_pred_def_dest() to generate code for each dest.
 *
 * Examples:
 * 
 *   For an unconditional pred def (L_PTYPE_UNCOND_T), will generate:
 *     dest[0] = pred[0] && (('cast'src[0])'operator'('cast'src[1]));
 * 
 *   For an OR-type pred def (L_PTYPE_OR_T), will generate:
 *     if (pred[0]) {dest[0] |= ('cast'src[0])'operator'('cast'src[1]);}
 *
 *   if (pred[0] == NULL), that portion of the test will be omitted;
 *
 *  'cast' may be NULL, to specify no usage of that feature 
 *  (implicit parenthesis, if any, will be removed).
 *
 * The guard predicate is saved in _EM_guard, a local temporary,
 * which is to be consumed by the dest-generating function.  This
 * is required in case the first definition entails a self anti-dependence.
 */
void
C_emit_pred_def_op (FILE * out, L_Func * fn, L_Oper * op,
		    char *cast, char *operator)
{

  fprintf (out, "%s{ int %s_guard = ", C_indent, C_prefix);

  if (op->pred[0])
    C_emit_operand (out, fn, op->pred[0]);
  else
    fprintf (out, "1");

  fprintf (out, ";\n");

  /* Verify proper number of operands are present (may have 1 or 2 dests) */
  if (op->dest[1] == NULL)
    {
      C_check_operands ("C_emit_pred_def_op", op, 1, 2);
      C_emit_pred_def_dest (out, fn, op, op->dest[0], cast, operator);

      /* If tracing pred defs, unconditionally write out predicate value
       * (since even pred squashed defs can define pred value)
       */
      if (C_insert_probes && C_trace_pred_defs)
	{
	  C_emit_put_trace_pred_operand (out, fn, NULL,
					 "?L_TRACE_PRED_DEF", op->dest[0]);
	}
    }
  else if (op->dest[0] == NULL)
    {
      L_Operand *tmp;

      tmp = op->dest[0];
      op->dest[0] = op->dest[1];
      op->dest[1] = tmp;

      C_check_operands ("C_emit_pred_def_op", op, 1, 2);
      C_emit_pred_def_dest (out, fn, op, op->dest[0], cast, operator);

      /* If tracing pred defs, unconditionally write out predicate value
       * (since even pred squashed defs can define pred value)
       */
      if (C_insert_probes && C_trace_pred_defs)
	{
	  C_emit_put_trace_pred_operand (out, fn, NULL,
					 "?L_TRACE_PRED_DEF", op->dest[0]);
	}
    }
  else
    {
      C_check_operands ("C_emit_pred_def_op", op, 2, 2);
      C_emit_pred_def_dest (out, fn, op, op->dest[0], cast, operator);
      C_emit_pred_def_dest (out, fn, op, op->dest[1], cast, operator);

      /* If tracing pred defs, unconditionally write out predicate value
       * (since even pred squashed defs can define pred value)
       */
      if (C_insert_probes && C_trace_pred_defs)
	{
	  C_emit_put_trace_pred_operand (out, fn, NULL,
					 "?L_TRACE_PRED_DEF", op->dest[0]);
	  C_emit_put_trace_pred_operand (out, fn, NULL,
					 "?L_TRACE_PRED_DEF", op->dest[1]);
	}
    }

  fprintf (out, "%s}\n", C_indent);
}

/* Emit check op
 * JWS/HCH 19991002
 */
void
C_emit_check (FILE * out, L_Func * fn, L_Oper * op)
{
  C_emit_op_comment (out, op);

  if (!C_insert_probes)
    return;

  if (!C_trace_mem_addrs)
    return;

  /* Add if () {} around code, if predicated */
  if (op->pred[0] != NULL)
    {
      fprintf (out, "if (");
      C_emit_operand (out, fn, op->pred[0]);
      fprintf (out, ") {");
    }

  C_emit_put_trace_header (out, fn, NULL, "L_TRACE_CHECK");

  if (op->pred[0] != NULL)
    fprintf (out, "}");

  return;
}

/* Emit the Lcode operation in a C comment */
void
C_emit_op_comment (FILE * out, L_Oper * op)
{
  int i, num, flags;
  char *orig_string, *filtered_string, *ptr;

  /* Print out the operation in a comment, to aid debugging, etc.
   * Code adapted from L_print_oper, etc.
   */
  fprintf (out, "\n%s/* op %d %s ", C_indent, op->id, op->opcode);

  if (op->com[0])
    fprintf(out, "%s ", L_ctype_name (op->com[0]));
  if (op->com[1])
    fprintf(out, "%s ", L_cmp_compl_name (op->com[1]));

  /*
   *  Print flags - flags are optional
   *
   *  Remember to clear out any flags that should not print!
   */
  flags = op->flags;
  flags = L_CLR_BIT_FLAG (flags, L_OPER_HIDDEN_FLAGS);
  if (flags != 0)
    {
      fprintf (out, "<");
      if (L_EXTRACT_BIT_VAL (flags, L_OPER_CHECK))
	fprintf (out, "C");
      if (L_EXTRACT_BIT_VAL (flags, L_OPER_LABEL_REFERENCE))
	fprintf (out, "L");
      if (L_EXTRACT_BIT_VAL (flags, L_OPER_PROMOTED))
	fprintf (out, "P");
      if (L_EXTRACT_BIT_VAL (flags, L_OPER_SQUASHING))
	fprintf (out, "Q");
      if (L_EXTRACT_BIT_VAL (flags, L_OPER_DATA_SPECULATIVE))
	fprintf (out, "D");
      if (L_EXTRACT_BIT_VAL (flags, L_OPER_SPILL_CODE))
	fprintf (out, "R");
      if (L_EXTRACT_BIT_VAL (flags, L_OPER_SIDE_EFFECT_FREE))
	fprintf (out, "E");
      if (L_EXTRACT_BIT_VAL (flags, L_OPER_SPECULATIVE))
	fprintf (out, "S");
      if (L_EXTRACT_BIT_VAL (flags, L_OPER_MASK_PE))
	fprintf (out, "M");
      if (L_EXTRACT_BIT_VAL (flags, L_OPER_SAFE_PEI))
	fprintf (out, "F");
      if (L_EXTRACT_BIT_VAL (flags, L_OPER_PROBE_MARK))
	fprintf (out, "X");
      if (L_EXTRACT_BIT_VAL (flags, L_OPER_SYNC))
	fprintf (out, "Y");

      if (L_EXTRACT_BIT_VAL (flags, L_OPER_PROCESSOR_SPECIFIC))
	fprintf (out, "?");
      if (L_EXTRACT_BIT_VAL (flags, L_OPER_VOLATILE))
	fprintf (out, "V");
      fprintf (out, "> ");
    }

  /*
   *  Print predicate operands - predicates are optional
   *  Only print pred[0], since only one emulated.
   */
  if (op->pred[0] != NULL)
    {
      fprintf (out, "<");
      L_print_operand (out, op->pred[0], 1);
      fprintf (out, "> [");
    }
  else
    {
      fprintf (out, "[");
    }

  /*
   *  Print dest operands
   */
  num = L_max_dest_operand;
  for (i = L_max_dest_operand - 1; i >= 0; i--)
    {
      if (op->dest[i] != NULL)
	break;
      num--;
    }
  for (i = 0; i < num; i++)
    {
      L_print_operand (out, op->dest[i], 1);
    }
  fprintf (out, "] [");
  /*
   *  Print src operands
   */
  num = L_max_src_operand;
  for (i = L_max_src_operand - 1; i >= 0; i--)
    {
      if (op->src[i] != NULL)
	break;
      num--;
    }
  for (i = 0; i < num; i++)
    {
      /* If operand is a string, filter out comment delimiters,
       * so a legal comment will be generated 
       */
      if (L_is_string (op->src[i]))
	{
	  /* Get original string */
	  orig_string = op->src[i]->value.s;

	  /* Duplicate it for filtering */
	  if ((filtered_string = strdup (orig_string)) == NULL)
	    L_punt ("Out of memory");

	  /* Filter out comment delimiters */
	  for (ptr = filtered_string; *ptr != 0; ptr++)
	    {
	      if ((ptr[0] == '/') && (ptr[1] == '*'))
		ptr[0] = '|';

	      if ((ptr[0] == '*') && (ptr[1] == '/'))
		ptr[1] = '|';
	    }

	  /* Munge operand temporarily to use filtered version */
	  op->src[i]->value.s = filtered_string;
	  L_print_operand (out, op->src[i], 1);
	  op->src[i]->value.s = orig_string;

	  free (filtered_string);
	}

      /* Otherwise, just print it out normally */
      else
	{
	  L_print_operand (out, op->src[i], 1);
	}
    }
  fprintf (out, "]");

  fprintf (out, " */\n");
}

/* Print out op as a comment, in psuedo C form */
void
C_emit_simple_version (FILE * out, L_Func * fn, L_Oper * op)
{
  int index;

  /* Print out C in comment for now */
  fprintf (out, "%s/*", C_indent);

  if (op->pred[0] != NULL)
    {
      fprintf (out, " if(");
      C_emit_operand (out, fn, op->pred[0]);
      fprintf (out, ") {");
    }

  fprintf (out, " (");
  for (index = 0; index < L_max_dest_operand; index++)
    {
      if (op->dest[index] != NULL)
	{
	  fprintf (out, " ");
	  C_emit_operand (out, fn, op->dest[index]);
	}
    }

  fprintf (out, " ) <- (");

  for (index = 0; index < L_max_src_operand; index++)
    {
      if (op->src[index] != NULL)
	{
	  fprintf (out, " ");
	  C_emit_operand (out, fn, op->src[index]);
	}
    }

  fprintf (out, " );");

  fprintf (out, "*/\n");
}

/* Emit the operation in C code */
void
C_emit_op (FILE * out, L_Func * fn, L_Oper * op)
{
  L_Attr *opcode_attr;
  
  /* Print out Lcode operation in comment to aid debugging */
  C_emit_op_comment (out, op);
  
  /* Check to see if this op should be an IP reference in
     a vararg function and if so, fix it */
  C_fixup_vararg_ip(out, op);
  
  /* Do generic operation trace code, if inserting probes */
  if (C_insert_probes)
    {
      /* Put func and op id into trace, if desired and not an
       * operation Lencode ignores.  Emit even if operation
       * predicate squashed.
       */
      if (C_trace_enhanced_op_ids && !C_Lencode_ignores_oper (op))
	{
	  C_emit_put_trace_two_ints (out, fn, NULL, "L_TRACE_FN_OP_ID",
				     C_trace_func_id, op->id);
	}
      
      /* Put op->id into trace, if desired and not an operation 
       * Lencode ignores and not doing enhanced op ids.  Emit even if
       * operation is predicate squashed.
       */
      else if (C_trace_op_ids && !C_Lencode_ignores_oper (op))
	{
	  C_emit_put_trace_int (out, fn, NULL, "L_TRACE_OP_ID", op->id);
	}

      /* If tracing pred uses and predicated, unconditionally write 
       * out predicate value (i.e., even if predicate squashed).
       */
      if (C_trace_pred_uses && (op->pred[0] != NULL))
	{
	  C_emit_put_trace_pred_operand (out, fn, NULL, NULL, op->pred[0]);
	}

      /* If tracing promoted predicates and the predicate has been promoted,
       * unconditionally write out pred[1]'s value.
       *
       * NOTE: This only produces valid values on unscheduled and
       *       unregister allocated code!  Garbage values will
       *       be produced otherwise (op may be moved above original 
       *       pred def and the register allocator does not allocate 
       *       pred[1]).  However, pred[1] is sometimes used for other
       *       reasons (but not in IMPACT 2.2!) so don't punt.
       */
      if (C_trace_promoted_preds && (op->pred[1] != NULL))
	{
	  C_emit_put_trace_pred_operand (out, fn, NULL,
					 "?L_TRACE_PROMOTED_PRED",
					 op->pred[1]);

	  /* Warn but don't punt if users are tracing pred[1] with
	   * register allocated and scheduled code.  pred[1] sometimes
	   * used for other reasons (and presumably the user knows
	   * what they are doing)...
	   */
	  if ((fn->flags & (L_FUNC_REGISTER_ALLOCATED | L_FUNC_SCHEDULED)) &&
	      !C_warned_about_promoted_preds)
	    {
	      fprintf (stderr,
		       "\n"
		       "Warning: Tracing promoted predicates (pred[1]) "
		       "in scheduled or register\n"
		       "         allocated code!  This normally produces "
		       "garbage values in trace.\n"
		       "         Assuming pred[1] is being used in a "
		       "non-standard way and\n"
		       "         user knows what they are doing...\n" "\n");

	      C_warned_about_promoted_preds = 1;
	    }


	}
    }

#ifdef INTRINSICS
    /**********************************************************************
     * If we have the "I_opcode" attribute, send the opcode to 
     * the intrinsic handling function.  If C_emit_intrinsic_op outputs
     * handles the opcode, we're done.  Otherwise, we need to take care
     * of it.   -ITI/JWJ  8.16.1999
     *********************************************************************/
  if ((opcode_attr = L_find_attr (op->attr, "I_opcode")) != NULL)
    {
      if (C_emit_intrinsic_op (out, fn, op) == 0)
	{
	  return;
	}
    }
#endif

  /* Base action on proc_opc of operation */
  switch (op->proc_opc)
    {
    case Lop_PROLOGUE:
    case Lop_EPILOGUE:
    case Lop_DEFINE:
      /* Do nothing here for compiler directives */
      break;

    case Lop_NO_OP:
      /* Do nothing for NO-OP */
      break;

    case Lop_MOV:
    case Lop_MOV_F:
    case Lop_MOV_F2:
      C_emit_1_dest_1_src_op (out, fn, op, NULL, NULL);
      break;

    case Lop_ABS:
    case Lop_ABS_F:
    case Lop_ABS_F2:
      C_emit_abs_op (out, fn, op);
      break;

    case Lop_SQRT_F:
    case Lop_SQRT_F2:
      C_emit_sqrt_op (out, fn, op);
      break;

    /* SLARSEN: min / max operations */
    case Lop_MIN:
    case Lop_MAX:
    case Lop_MIN_F:
    case Lop_MAX_F:
    case Lop_MIN_F2:
    case Lop_MAX_F2:
      C_emit_minmax_op (out, fn, op);
      break;

    /* SER 20040408: adding completer support for F_I, F2_I */
    case Lop_F_I:
    case Lop_F2_I:
      C_emit_int_conv_op(out, fn, op);
      break;

    /* SER 20040406: adding completer support for I_F, I_F2 */
    case Lop_I_F:
    case Lop_I_F2:
      C_emit_float_conv_op(out, fn, op);
      break;

    case Lop_F2_F:
      C_emit_1_dest_1_src_op (out, fn, op, "(float)(", ")");
      break;

    case Lop_F_F2:
      C_emit_1_dest_1_src_op (out, fn, op, "(double)(", ")");
      break;

    case Lop_ADD_CARRY:
    case Lop_ADD_CARRY_U:
      C_emit_addc_op (out, fn, op, NULL, NULL, " + ", NULL);
      break;

    case Lop_SUB_CARRY:
    case Lop_SUB_CARRY_U:
      C_emit_subc_op (out, fn, op, NULL, NULL, " + ", NULL);
      break;

    case Lop_MUL_WIDE:
    case Lop_MUL_WIDE_U:
      C_emit_mul_wide_op (out, fn, op, NULL, NULL, " * ", NULL);
      break;

    case Lop_ADD:
      /* C_emit_1_dest_2_src_op (out, fn, op, NULL, NULL, " + ", NULL); */
      C_emit_add_op (out, fn, op, NULL, NULL, " + ", NULL);
      break;
    case Lop_ADD_F:
      C_emit_1_dest_2_src_op (out, fn, op, "(float)(", NULL, " + ", ")");
      break;
    case Lop_ADD_F2:
      C_emit_1_dest_2_src_op (out, fn, op, "(double)(", NULL, " + ", ")");
      break;

    case Lop_ADD_U:
      C_emit_1_dest_2_src_op (out, fn, op, NULL,
			      C_native_machine_ctype_unsigned_str, " + ",
			      NULL);
      break;

    case Lop_SUB:
      C_emit_1_dest_2_src_op (out, fn, op, NULL, NULL, " - ", NULL);
      break;
    case Lop_SUB_F:
      C_emit_1_dest_2_src_op (out, fn, op, "(float)(", NULL, " - ", ")");
      break;
    case Lop_SUB_F2:
      C_emit_1_dest_2_src_op (out, fn, op, "(double)(", NULL, " - ", ")");
      break;

    case Lop_SUB_U:
      C_emit_1_dest_2_src_op (out, fn, op, NULL,
			      C_native_machine_ctype_unsigned_str, " - ",
			      NULL);
      break;

#ifdef INTRINSICS
      /* ITI/JWJ  8.11.1999 */
    case Lop_L_MAC:
    case Lop_L_MSU:
    case Lop_ADD_SAT:
    case Lop_ADD_SAT_U:
    case Lop_SUB_SAT:
    case Lop_SUB_SAT_U:
    case Lop_MUL_SAT:
    case Lop_MUL_SAT_U:
    case Lop_SAT:
    case Lop_SAT_U:
      L_print_oper (stderr, op);
      L_punt ("C_emit_op: Op can only be used as an intrinsic mapping.\n"
	      "           (missing I_intrinsic attribute.)");
      break;
#endif

    case Lop_MUL:
      C_emit_1_dest_2_src_op (out, fn, op, NULL, NULL, " * ", NULL);
      break;
    case Lop_MUL_F:
      C_emit_1_dest_2_src_op (out, fn, op, "(float)(", NULL, " * ", ")");
      break;
    case Lop_MUL_F2:
      C_emit_1_dest_2_src_op (out, fn, op, "(double)(", NULL, " * ", ")");
      break;

    case Lop_MUL_U:
      C_emit_1_dest_2_src_op (out, fn, op, NULL,
			      C_native_machine_ctype_unsigned_str, " * ",
			      NULL);
      break;


    case Lop_MUL_ADD:
      C_emit_1_dest_3_src_op (out, fn, op, "(", 0, " * ", 1, ") + ", 2, "");
      break;
    case Lop_MUL_ADD_F:
      C_emit_1_dest_3_src_op (out, fn, op, "(float)(", 0, " * ", 1, ") + ", 2, "");
      break;
    case Lop_MUL_ADD_F2:
      C_emit_1_dest_3_src_op (out, fn, op, "(double)(", 0, " * ", 1, ") + ", 2, "");
      break;

    case Lop_MUL_ADD_U:
      {
	char one[64], two[64], three[64];
	sprintf (one, "(((%s)", C_native_machine_ctype_unsigned_str);
	sprintf (two, ") * ((%s)", C_native_machine_ctype_unsigned_str);
	sprintf (three, ")) + ((%s)", C_native_machine_ctype_unsigned_str);

	C_emit_1_dest_3_src_op (out, fn, op, one, 0, two, 1, three, 2, ")");
      }
      break;

    case Lop_MUL_SUB:
      C_emit_1_dest_3_src_op (out, fn, op, "(", 0, " * ", 1, ") - ", 2, "");
      break;
    case Lop_MUL_SUB_F:
      C_emit_1_dest_3_src_op (out, fn, op, "(float)(", 0, " * ", 1, ") - ", 2, "");
      break;
    case Lop_MUL_SUB_F2:
      C_emit_1_dest_3_src_op (out, fn, op, "(double)(", 0, " * ", 1, ") - ", 2, "");
      break;

    case Lop_MUL_SUB_U:
      {
	char one[64], two[64], three[64];
	sprintf (one, "(((%s)", C_native_machine_ctype_unsigned_str);
	sprintf (two, ") * ((%s)", C_native_machine_ctype_unsigned_str);
	sprintf (three, ")) - ((%s)", C_native_machine_ctype_unsigned_str);

	C_emit_1_dest_3_src_op (out, fn, op, one, 0, two, 1, three, 2, ")");
      }
      break;

    case Lop_MUL_SUB_REV:
      C_emit_1_dest_3_src_op (out, fn, op, "", 2, " - (", 0, " * ", 1, ")");
      break;
    case Lop_MUL_SUB_REV_F:
      C_emit_1_dest_3_src_op (out, fn, op, "", 2, " - (float)(", 0, " * ", 1, ")");
      break;
    case Lop_MUL_SUB_REV_F2:
      C_emit_1_dest_3_src_op (out, fn, op, "", 2, " - (double)(", 0, " * ", 1, ")");
      break;

    case Lop_MUL_SUB_REV_U:
      {
	char one[64], two[64], three[64];
	sprintf (one, "(((%s)", C_native_machine_ctype_unsigned_str);
	sprintf (two, ") - (((%s)", C_native_machine_ctype_unsigned_str);
	sprintf (three, ") * ((%s)", C_native_machine_ctype_unsigned_str);

	C_emit_1_dest_3_src_op (out, fn, op, one, 2, two, 0, three, 1, "))");
      }
      break;


    case Lop_DIV:
      {
	char *cstr = op->com[0] ? C_get_compare_cast (op->com[0]) : NULL;
	C_emit_div_op (out, fn, op, NULL, " / ", cstr, NULL);
      }
      break;
    case Lop_DIV_F:
      C_emit_div_op (out, fn, op, "(float)(", " / ", NULL, ")");
      break;
    case Lop_DIV_F2:
      C_emit_div_op (out, fn, op, "(double)(", " / ", NULL, ")");
      break;

    case Lop_DIV_U:
      {
	char *cstr = op->com[0] ? C_get_compare_cast (op->com[0]) :
	  C_native_machine_ctype_unsigned_str;
	C_emit_div_op (out, fn, op, NULL, " / ", cstr, NULL);
      }
      break;

    case Lop_REM:
      {
	char *cstr = op->com[0] ? C_get_compare_cast (op->com[0]) : 
	  NULL;
	C_emit_div_op (out, fn, op, NULL, "%", cstr, NULL);
      }
      break;

    case Lop_REM_U:
      {
	char *cstr = op->com[0] ? C_get_compare_cast (op->com[0]) :
	  C_native_machine_ctype_unsigned_str;
	C_emit_div_op (out, fn, op, NULL, "%", cstr, NULL);
      }
      break;

    case Lop_LSL:
      C_emit_1_dest_2_src_lsl (out, fn, op, NULL, NULL, " << ", NULL);
      break;

    case Lop_ASR:		/* top bit is sign bit and will not change */
      C_emit_1_dest_2_src_op (out, fn, op, NULL, NULL, " >> ", NULL);
      break;

    case Lop_LSR:		/* "(Unsigned)" makes top bit 0 
				   on right shift */
      C_emit_1_dest_2_src_op (out, fn, op, NULL,
			      C_native_machine_ctype_unsigned_str, " >> ",
			      NULL);
      break;

    case Lop_OR:
      C_emit_1_dest_2_src_op (out, fn, op, NULL, NULL, " | ", NULL);
      break;

    case Lop_AND:
      C_emit_1_dest_2_src_op (out, fn, op, NULL, NULL, " & ", NULL);
      break;

    case Lop_XOR:
      C_emit_1_dest_2_src_op (out, fn, op, NULL, NULL, " ^ ", NULL);
      break;

    case Lop_NOR:
      C_emit_1_dest_2_src_op (out, fn, op, "~(", NULL, " | ", ")");
      break;

    case Lop_NAND:
      C_emit_1_dest_2_src_op (out, fn, op, "~(", NULL, " & ", ")");
      break;

    case Lop_NXOR:
      C_emit_1_dest_2_src_op (out, fn, op, "~(", NULL, " ^ ", ")");
      break;

    case Lop_RCMP:
    case Lop_RCMP_F:
      {
	char *ustr = NULL;
	char *com_op = NULL;

	ustr = C_get_compare_cast (op->com[0]);
	com_op = C_get_compare_token (op->com[1]);

	C_emit_1_dest_2_src_op (out, fn, op, NULL, ustr, com_op, NULL);
      }
      break;

    case Lop_LD_UC:
      C_emit_std_load_op (out, fn, op, "(unsigned char *)", "uchar");
      break;

    case Lop_LD_C:
      C_emit_std_load_op (out, fn, op, "(char *)", "char");
      break;

    case Lop_LD_UC2:
      C_emit_std_load_op (out, fn, op, "(unsigned short *)", "ushort");
      break;

    case Lop_LD_C2:
      C_emit_std_load_op (out, fn, op, "(short *)", "short");
      break;

    case Lop_LD_UI:
      C_emit_std_load_op (out, fn, op, "(unsigned int *)", "uint");
      break;

    case Lop_LD_I:
      C_emit_std_load_op (out, fn, op, "(int *)", "int");
      break;

    case Lop_LD_Q:
      C_emit_std_load_op (out, fn, op, "(long long *)", "longlong");
      break;

    case Lop_LD_F:
      C_emit_std_load_op (out, fn, op, "(float *)", "float");
      break;

    case Lop_LD_F2:
      C_emit_std_load_op (out, fn, op, "(double *)", "double");
      break;

    case Lop_LD_POST_UC:
      C_emit_post_load_op (out, fn, op, "(unsigned char *)", "uchar");
      break;

    case Lop_LD_POST_C:
      C_emit_post_load_op (out, fn, op, "(char *)", "char");
      break;

    case Lop_LD_POST_UC2:
      C_emit_post_load_op (out, fn, op, "(unsigned short *)", "ushort");
      break;

    case Lop_LD_POST_C2:
      C_emit_post_load_op (out, fn, op, "(short *)", "short");
      break;

    case Lop_LD_POST_UI:
      C_emit_post_load_op (out, fn, op, "(unsigned int *)", "uint");
      break;

    case Lop_LD_POST_I:
      C_emit_post_load_op (out, fn, op, "(int *)", "int");
      break;

    case Lop_LD_POST_Q:
      C_emit_post_load_op (out, fn, op, "(long long *)", "longlong");
      break;

    case Lop_LD_POST_F:
      C_emit_post_load_op (out, fn, op, "(float *)", "float");
      break;

    case Lop_LD_POST_F2:
      C_emit_post_load_op (out, fn, op, "(double *)", "double");
      break;

    case Lop_ST_C:
      C_emit_std_store_op (out, fn, op, "(char *)");
      break;

    case Lop_ST_C2:
      C_emit_std_store_op (out, fn, op, "(short *)");
      break;

    case Lop_ST_I:
      C_emit_std_store_op (out, fn, op, "(int *)");
      break;

    case Lop_ST_Q:
      C_emit_std_store_op (out, fn, op, "(long long *)");
      break;

    case Lop_ST_F:
      C_emit_std_store_op (out, fn, op, "(float *)");
      break;

    case Lop_ST_F2:
      C_emit_std_store_op (out, fn, op, "(double *)");
      break;

    case Lop_ST_POST_C:
      C_emit_post_store_op (out, fn, op, "(char *)");
      break;

    case Lop_ST_POST_C2:
      C_emit_post_store_op (out, fn, op, "(short *)");
      break;

    case Lop_ST_POST_I:
      C_emit_post_store_op (out, fn, op, "(int *)");
      break;

    case Lop_ST_POST_Q:
      C_emit_post_store_op (out, fn, op, "(long long *)");
      break;

    case Lop_ST_POST_F:
      C_emit_post_store_op (out, fn, op, "(float *)");
      break;

    case Lop_ST_POST_F2:
      C_emit_post_store_op (out, fn, op, "(double *)");
      break;

    case Lop_BR:
    case Lop_BR_F:
      {
	char *ustr = NULL;
	char *com_op = NULL;

	ustr = C_get_compare_cast (op->com[0]);
	com_op = C_get_compare_token (op->com[1]);
	C_emit_cond_br_op (out, fn, op, ustr, com_op);
      }
      break;

    case Lop_JUMP:
    case Lop_JUMP_FS:
      C_emit_jump_op (out, fn, op);
      break;

    case Lop_JUMP_RG:
    case Lop_JUMP_RG_FS:
      C_emit_jump_rg_op (out, fn, op);
      break;


    case Lop_JSR:
    case Lop_JSR_FS:
      C_emit_jsr_op (out, fn, op);
      break;

    case Lop_RTS:
    case Lop_RTS_FS:
      C_emit_rts_op (out, fn, op);
      break;

#ifdef INTRINSICS
    case Lop_INTRINSIC:
      C_emit_intrinsic_op (out, fn, op);
      break;
#endif

    case Lop_PRED_LD:
    case Lop_PRED_LD_BLK:
      /* Treat as integer loads */
      if (C_native_machine_ctype == L_CTYPE_INT)
	C_emit_std_load_op (out, fn, op, "(int *)", "int");
      else if (C_native_machine_ctype == L_CTYPE_LLONG)
	C_emit_std_load_op (out, fn, op, "(longlong *)", "int");
      else
	L_punt ("C_emit_op (PRED_LD or PRED_LD_BLK): "
		"Unsupported native machine reg size\n");
      break;

    case Lop_PRED_ST:
    case Lop_PRED_ST_BLK:
      /* Treat as integer stores */
      if (C_native_machine_ctype == L_CTYPE_INT)
	C_emit_std_store_op (out, fn, op, "(int *)");
      else if (C_native_machine_ctype == L_CTYPE_LLONG)
	C_emit_std_store_op (out, fn, op, "(longlong *)");
      else
	L_punt ("C_emit_op (PRED_ST or PRED_ST_BLK): "
		"Unsupported native machine reg size\n");
      break;

    case Lop_PRED_CLEAR:
      C_emit_pred_set_op (out, fn, op, 0);
      break;

    case Lop_PRED_SET:
      C_emit_pred_set_op (out, fn, op, 1);
      break;

    case Lop_PRED_COPY:
      C_emit_pred_copy_op (out, fn, op);
      break;

    case Lop_CMP:
    case Lop_CMP_F:
      {
	char *ustr = NULL;
	char *com_op = NULL;
	ustr = C_get_compare_cast (op->com[0]);
	com_op = C_get_compare_token (op->com[1]);
	C_emit_pred_def_op (out, fn, op, ustr, com_op);
      }
      break;

    case Lop_CHECK:
      C_emit_check (out, fn, op);
      break;

    case Lop_ALLOC:
    case Lop_PBR:
    case Lop_PRED_MERGE:
    case Lop_PRED_AND:
    case Lop_PRED_COMPL:
      /* Print out simple comment in partial C form */
      C_emit_simple_version (out, fn, op);

      /* Debug */
      L_print_oper (stderr, op);
      L_punt ("C_emit_op: Not implemented yet (above)!");
      break;

      /*
       * JWS 20000413 IA-64 opcodes
       */

    case Lop_EXTRACT_C:
    case Lop_SXT_C:
      C_emit_sz_ext_op (out, fn, op, 7, 1);
      break;
    case Lop_EXTRACT_C2:
    case Lop_SXT_C2:
      C_emit_sz_ext_op (out, fn, op, 15, 1);
      break;
    case Lop_SXT_I:
      C_emit_sz_ext_op (out, fn, op, 31, 1);
      break;
    case Lop_ZXT_C:
      C_emit_sz_ext_op (out, fn, op, 7, 0);
      break;
    case Lop_ZXT_C2:
      C_emit_sz_ext_op (out, fn, op, 15, 0);
      break;
    case Lop_ZXT_I:
      C_emit_sz_ext_op (out, fn, op, 31, 0);
      break;
    case Lop_EXTRACT:
    case Lop_EXTRACT_U:
      C_emit_extr_op (out, fn, op);
      break;
    case Lop_LSLADD:
    case Lop_LD_UC_CHK:
    case Lop_LD_C_CHK:
    case Lop_LD_UC2_CHK:
    case Lop_LD_C2_CHK:
    case Lop_LD_I_CHK:
    case Lop_LD_UI_CHK:
    case Lop_LD_Q_CHK:
    case Lop_LD_F_CHK:
    case Lop_LD_F2_CHK:
    case Lop_CHECK_ALAT:
      L_punt ("Lemulate: Unhandled IA-64 opcode %d", op->proc_opc);
      break;


      /* The following operations are not currently generated by 
       * IMPACT public release version 2.0 and therefore their 
       * implementation specs cannot be determined or tested.  
       * If you need them, feel free to implement them however you 
       * think they should work (since your intuition is as good as mine).
       * -ITI(JCG) 3/99
       */
    case Lop_PREF_LD:
    case Lop_BR_UNCOND:
    case Lop_BR_CONDT:
    case Lop_BR_CONDF:
    case Lop_BR_LINK:
    //case Lop_MAX:
    //case Lop_MIN:
    case Lop_REV:
    case Lop_BIT_POS:
    case Lop_RCP_F2:
    case Lop_RCP_F:
    case Lop_FETCH_AND_ADD:
    case Lop_FETCH_AND_OR:
    case Lop_FETCH_AND_AND:
    case Lop_FETCH_AND_ST:
    case Lop_FETCH_AND_COND_ST:
    case Lop_ADVANCE:
    case Lop_AWAIT:
    case Lop_MUTEX_B:
    case Lop_MUTEX_E:
    case Lop_CO_PROC:
    case Lop_CONFIRM:
    case Lop_EXPAND:
    case Lop_JSR_ND:
    case Lop_CMOV:
    case Lop_CMOV_COM:
    case Lop_CMOV_F:
    case Lop_CMOV_COM_F:
    case Lop_CMOV_F2:
    case Lop_CMOV_COM_F2:
    case Lop_SELECT:
    case Lop_SELECT_F:
    case Lop_SELECT_F2:
    case Lop_MEM_COPY:
    case Lop_MEM_COPY_BACK:
    case Lop_MEM_COPY_CHECK:
    case Lop_MEM_COPY_RESET:
    case Lop_MEM_COPY_SETUP:
    case Lop_MEM_COPY_TAG:
    case Lop_SIM_DIR:
    case Lop_BOUNDARY:
    case Lop_REMAP:
    case Lop_BIT_EXTRACT:
    case Lop_BIT_DEPOSIT:
    case Lop_LD_PRE_UC:
    case Lop_LD_PRE_C:
    case Lop_LD_PRE_UC2:
    case Lop_LD_PRE_C2:
    case Lop_LD_PRE_UI:
    case Lop_LD_PRE_I:
    case Lop_LD_PRE_Q:
    case Lop_LD_PRE_F:
    case Lop_LD_PRE_F2:
    case Lop_ST_PRE_C:
    case Lop_ST_PRE_C2:
    case Lop_ST_PRE_I:
    case Lop_ST_PRE_Q:
    case Lop_ST_PRE_F:
    case Lop_ST_PRE_F2:
    case Lop_OR_NOT:
    case Lop_AND_NOT:
    case Lop_OR_COMPL:
    case Lop_AND_COMPL:
    default:
      L_print_oper (stderr, op);
      L_punt ("C_emit_op: Unsupported operation (above)!");
    }

#if 0
  {
    int i;
    switch (op->proc_opc)
      {
      case Lop_PROLOGUE:
      case Lop_EPILOGUE:
      case Lop_DEFINE:
      case Lop_NO_OP:
	break;
      default:
	if (L_EXTRACT_BIT_VAL (op->flags, L_OPER_SPILL_CODE))
	  break;
	if (/*L_cond_branch_opcode(op) || */
	    L_general_store_opcode(op))
	  {
	    /*    PIPE_send_L_Operand(0, 9, Lop, 54, 0, _EM_r_13_i, 0.0, 0.0);*/
	    fprintf(out, "PIPE_send_L_Operand(%d, %d, %d, %d, %d, ",
		    0, op->id, op->opc, 
		    L_CTYPE_INT, L_PTYPE_NULL);
	    fprintf(out, "0, 0.0, 0.0); \n");
	  }
	else
	  {
	    for (i=0; i<2; i++)
	      {
		if (op->dest[i] &&
		    (L_is_register(op->dest[i]) ||
		     L_is_macro(op->dest[i])))
		  {
		    /*    PIPE_send_L_Operand(0, 9, Lop, 54, 0, _EM_r_13_i, 0.0, 0.0);*/
		    fprintf(out, "PIPE_send_L_Operand(%d, %d, %d, %d, %d, ",
			    0, op->id, op->opc, 
			    op->dest[i]->ctype, op->dest[i]->ptype);
		    C_emit_operand (out, fn, op->dest[i]);
		    fprintf(out, ",");
		    C_emit_operand (out, fn, op->dest[i]);
		    fprintf(out, ",");
		    C_emit_operand (out, fn, op->dest[i]);
		    fprintf(out, ");");
		  }
		else
		  {
		  }
	      }
	  }
	break;
      }
  }
#endif
}
