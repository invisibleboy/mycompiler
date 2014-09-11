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
 *      File: l_emul_operand.c
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
#include <math.h>
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

/* Checks to make sure the specified number of dest and src operands
 * are not NULL and that the rest are NULL.  Punts if requirements
 * are not request, using 'name' to indentify who expects these requirements.
 */
void
C_check_operands (char *name, L_Oper * op, int num_dest, int num_src)
{
  int index;

  /* Make sure expected dest operands are not NULL */
  for (index = 0; index < num_dest; index++)
    {
      if (op->dest[index] == NULL)
	{
	  L_print_oper (stderr, op);
	  L_punt ("%s: dest[%i] is unexpectedly NULL!", name, index);
	}
    }

  /* Make sure the rest of the dest operands are NULL */
  for (index = num_dest; index < L_max_dest_operand; index++)
    {
      if (op->dest[index] != NULL)
	{
	  L_print_oper (stderr, op);
	  L_punt ("%s: dest[%i] is unexpectedly not NULL!", name, index);
	}
    }

  /* Make sure expected src operands are not NULL */
  for (index = 0; index < num_src; index++)
    {
      if (op->src[index] == NULL)
	{
	  L_print_oper (stderr, op);
	  L_punt ("%s: src[%i] is unexpectedly NULL!", name, index);
	}
    }

  /* Make sure the rest of the src operands are NULL */
  for (index = num_src; index < L_max_src_operand; index++)
    {
      if (op->src[index] != NULL)
	{
	  L_print_oper (stderr, op);
	  L_punt ("%s: src[%i] is unexpectedly not NULL!", name, index);
	}
    }
  return;
}

/* Returns 1 if the label is a cb label in a jump table, 0 otherwisen.
 * Uses Mspec to handle.
 */
int
C_is_cb_label (char *label)
{
  static char name_buf[10000];
  int cb_id;

  /* Sanity check, make sure not going to overflow buffer */
  if (strlen (label) >= sizeof (name_buf))
    {
      L_punt ("C_is_cb_label: label name too big (%i bytes):\n"
	      "%s", strlen (label), label);
    }

  return (M_is_cb_label (label, name_buf, &cb_id));
}

/* Returns a pointer to the middle of 'name' where the true name starts*/
char *
C_true_name (char *name)
{
  /* Has name been prefixed with '_$fn_' */
  if ((name[0] == '_') && (name[1] == '$') &&
      (name[2] == 'f') && (name[3] == 'n') && (name[4] == '_'))
    {
      /* Yes, strip off */
      return (&name[5]);
    }

  /* Has name been prefixed with '_$_' (i.e., hash table label)?

   * Note: Should probably have use M_is_jumptbl_label(), but this Mspec
   * call requires the function name (which is not available in all cases,
   * and would be painful to get to here). Since this is unlikely
   * to chance, I am willing to risk a future gotcha to make this
   * code robust now. -ITI/JCG 3/99
   */
  if ((name[0] == '_') && (name[1] == '$') && (name[2] == '_'))
    {
      /* Yes, strip off '_$' (leave second '_' to help make unique) */
      return (&name[2]);
    }

  /* Has name been prefixed with '_' (normal case) */
  else if (name[0] == '_')
    {
      /* Yes, strip off */
      return (&name[1]);
    }

  else
    {
      L_punt ("C_true_name: No '_' prefix on '%s'", name);
      return (name);
    }
}

/* Returns 1 if the C_true_name(name) is the same as test_name */
int
C_matches_true_name (char *name, char *test_name)
{
  if (strcmp (C_true_name (name), test_name) == 0)
    return (1);
  else
    return (0);
}

/* Looks up the macro name and moves the pointer past the leading $ (if any), 
 * and returns it.  Don't free the returned pointer!
 */
char *
C_macro_name (int mac_id)
{
  char *name, *filtered_name;

  /* Look up macro name */
  name = L_macro_name (mac_id);

  /* Punt if name is or starts with '?' (or is empty) */
  if ((name[0] == '?') || (name[0] == 0))
    {
      L_punt ("C_macro_name: mac_id (%i) maps to illegal mac name '%s'!",
	      mac_id, name);
    }

  /* Filter out leading '$', if any */
  if (name[0] == '$')
    {
      filtered_name = &name[1];
    }

  /* Otherwise, leave the same */
  else
    {
      filtered_name = &name[0];
    }

  /* Make sure filtered name is not empty! */
  if (filtered_name[0] == 0)
    {
      L_punt ("C_macro_name: mac_id (%i) maps to illegal "
	      "filtered mac name '%s'!", mac_id, filtered_name);
    }

  /* Return the filtered name */
  return (filtered_name);
}

/* Emits an integer register in the C format.  Split out
 * of C_emit_operand to allow direct calling with a reg_id.
 */
void
C_emit_int_reg (FILE * out, int reg_id)
{
  if (C_use_register_arrays)
    {
      fprintf (out, "%sr_i[%d]", C_prefix, reg_id);
    }
  else
    {
      fprintf (out, "%sr_%d_i", C_prefix, reg_id);
    }
}

/* Emits a predicate register in the C format.  Split out
 * of C_emit_operand to allow direct calling with a reg_id.
 */
void
C_emit_pred_reg (FILE * out, int reg_id)
{
  if (C_use_register_arrays)
    {
      fprintf (out, "%sr_p[%d]", C_prefix, reg_id);
    }
  else
    {
      fprintf (out, "%sr_%d_p", C_prefix, reg_id);
    }
}

/* Emits a float register in the C format.  Split out
 * of C_emit_operand to allow direct calling with a reg_id.
 */
void
C_emit_float_reg (FILE * out, int reg_id)
{
  if (C_use_register_arrays)
    {
      fprintf (out, "%sr_f[%d]", C_prefix, reg_id);
    }
  else
    {
      fprintf (out, "%sr_%d_f", C_prefix, reg_id);
    }
}

/* Emits a double register in the C format.  Split out
 * of C_emit_operand to allow direct calling with a reg_id.
 */
void
C_emit_double_reg (FILE * out, int reg_id)
{
  if (C_use_register_arrays)
    {
      fprintf (out, "%sr_f2[%d]", C_prefix, reg_id);
    }
  else
    {
      fprintf (out, "%sr_%d_f2", C_prefix, reg_id);
    }
}

/* Emits an integer macro in the C format.  Split out
 * of C_emit_operand to allow direct calling with a mac_id.
 */
void
C_emit_int_mac (FILE * out, int mac_id)
{
  if (C_use_register_arrays)
    {
      fprintf (out, "%smac_i[%d]", C_prefix, mac_id);
    }
  else
    {
      fprintf (out, "%smac_%s_i", C_prefix, C_macro_name (mac_id));
    }
}

/* Emits a predicate macro in the C format.  Split out
 * of C_emit_operand to allow direct calling with a mac_id.
 */
void
C_emit_pred_mac (FILE * out, int mac_id)
{
  if (C_use_register_arrays)
    {
      fprintf (out, "%smac_p[%d]", C_prefix, mac_id);
    }
  else
    {
      fprintf (out, "%smac_%s_p", C_prefix, C_macro_name (mac_id));
    }
}

/* Emits a float macro in the C format.  Split out
 * of C_emit_operand to allow direct calling with a mac_id.
 */
void
C_emit_float_mac (FILE * out, int mac_id)
{
  if (C_use_register_arrays)
    {
      fprintf (out, "%smac_f[%d]", C_prefix, mac_id);
    }
  else
    {
      fprintf (out, "%smac_%s_f", C_prefix, C_macro_name (mac_id));
    }
}

/* Emits a double macro in the C format.  Split out
 * of C_emit_operand to allow direct calling with a mac_id.
 */
void
C_emit_double_mac (FILE * out, int mac_id)
{
  if (C_use_register_arrays)
    {
      fprintf (out, "%smac_f2[%d]", C_prefix, mac_id);
    }
  else
    {
      fprintf (out, "%smac_%s_f2", C_prefix, C_macro_name (mac_id));
    }
}

/* Emits operand in the C format.
 * Uses the eight helper function (C_emit_int_reg, etc.) above.
 *
 * The fn parameter is required so that cb labels that are not branch targets
 * (usually in a comparison of some sort) can be properly fixed up.  
 * I am assuming these cb labels will always reference cb's that are in
 * the function's hash table (and therefore will be #defined by 
 * C_emit_fn_jump_tables()).  The fn pointer should be NULL only when 
 * printing branch targets, which will then print out the 'C' branch target
 * cb label. -ITI/JCG 4/99
 *
 */
void
C_emit_operand (FILE * out, L_Func * fn, L_Operand * operand)
{
  int string_id;

  /* Make sure passed something, NULL probably is an error */
  if (operand == NULL)
    L_punt ("C_print_operand: NULL operand passed!");

  switch (operand->type)
    {
    case L_OPERAND_CB:
      /* If fn is NULL, print out branch target label */
      if (fn == NULL)
	{
	  fprintf (out, "%scb_%i", C_prefix, operand->value.cb->id);
	}
      /* Otherwise, print out hash table 'label' which should be
       * #defined to a cb id by C_emit_fn_jump_tables().
       */
      else
	{
	  fprintf (out, "%scb%i_%s", C_prefix, operand->value.cb->id,
		   C_true_name (fn->name));
	}
      break;

    case L_OPERAND_IMMED:
      if (L_is_ctype_integer (operand))
	{
	  if (M_native_int_register_ctype () == L_CTYPE_INT)
	    fprintf (out, "((int)" ITintmaxformat ITintmaxsuffix ")",
		     operand->value.i);
	  else
	    fprintf (out, "(" ITintmaxformat ITintmaxsuffix ")",
		     operand->value.i);
	}
      else if (L_is_ctype_flt (operand))
	{
	  float val = operand->value.f;
	  if (isnan (val))
	    fprintf (out, "(float) (0.0/0.0)");
	  else if (isinf (val) > 0)
	    fprintf (out, "(float) (1.0/0.0)");
	  else if (isinf (val) < 0)
	    fprintf (out, "(float) (-1.0/0.0)");
	  else
	    fprintf (out, "(float) %1.8e", val);
	}
      else if (L_is_ctype_dbl (operand))
	{
	  double val = operand->value.f2;

	  if (isnan (val))
	    fprintf (out, "(double) (0.0/0.0)");
	  else if (isinf (val) > 0)
	    fprintf (out, "(double) (1.0/0.0)");
	  else if (isinf (val) < 0)
	    fprintf (out, "(double) (-1.0/0.0)");
	  else
	    fprintf (out, "(double) %1.16e", val);
	}
      else
	{
	  L_print_operand (stderr, operand, 1);
	  L_punt ("C_emit_operand: Above IMMED operand unhandled!");
	}
      break;

    case L_OPERAND_STRING:
      /* Use string directly if there is not a string mapping table,
       * otherwise use "string" pointer to generate exactly one
       * address for identical duplicate strings. -JCG 2/00
       */
      if (C_string_map == NULL)
	{
	  fprintf (out, "((%s)%s)", C_native_machine_ctype_str,
		   operand->value.s);
	}
      else
	{
	  /* Get string id from string mapping table */
#if LP64_ARCHITECTURE
	  string_id = (int)((long) STRING_find_symbol_data (C_string_map,
							    operand->value.s));
#else
	  string_id = (int) STRING_find_symbol_data (C_string_map,
						     operand->value.s);
#endif

	  /* If <= 0, string unexpectly not found! */
	  if (string_id <= 0)
	    {
	      L_punt ("C_emit_operand: String mapping not found for '%s'!\n",
		      operand->value.s);
	    }

	  /* Print out mapped version of string operand */
	  fprintf (out, "((%s)%sString_%i)", C_native_machine_ctype_str,
		   C_prefix, string_id);
	}
      break;

    case L_OPERAND_MACRO:
      /* Make sure we know how to handle this type of macro */

      if ((M_arch == M_IMPACT) || (M_arch == M_PLAYDOH) || (M_arch == M_ARM))
	{
	  switch (operand->value.mac)
	    {
	      /* List of all macro's that we know how to emit properly
	       * in C (i.e., set up properly so their use will work!)
	       * List must also be updated in C_add_fn_mem_usage.
	       */
	    case L_MAC_P0:	/* Parameter register */
	    case L_MAC_P1:	/* Parameter register */
	    case L_MAC_P2:	/* Parameter register */
	    case L_MAC_P3:	/* Parameter register */
	    case L_MAC_P5:	/* Float parameter register */
	    case L_MAC_P6:	/* Float parameter register */
	    case L_MAC_P7:	/* Float parameter register */
	    case L_MAC_P8:	/* Float parameter register */
	    case L_MAC_P9:	/* Double parameter register */
	    case L_MAC_P10:	/* Double parameter register */
	    case L_MAC_P11:	/* Double parameter register */
	    case L_MAC_P12:	/* Double parameter register */

	    case L_MAC_P4:	/* Float return value register */
	    case L_MAC_P15:	/* Return value register */

	    case L_MAC_SP:	/* Stack ptr */
	    case L_MAC_IP:	/* Stack ptr alias for incoming param space */
	    case L_MAC_OP:	/* Stack ptr alias for outgoing param space */
	    case L_MAC_LV:	/* Stack ptr alias for local var space      */

            case L_MAC_RETADDR: /* Return address register */

	    case L_MAC_PRED_ALL:	/* Alias for all predicate registers */
	      break;

	    default:
	      L_print_operand (stderr, operand, 1);
	      fprintf (stderr, "\n");
	      L_punt
		("C_emit_operand: Above MACRO operand unhandled/unknown!");
	    }
	}
      else if (M_arch == M_TAHOE)
	{
	  switch (operand->value.mac)
	    {
	      /* List of all macro's that we know how to emit properly
	       * in C (i.e., set up properly so their use will work!)
	       * List must also be updated in C_add_fn_mem_usage.
	       */
	    case L_MAC_P0:	/* Incoming Parameter register */
	    case L_MAC_P1:	/* Incoming Parameter register */
	    case L_MAC_P2:	/* Incoming Parameter register */
	    case L_MAC_P3:	/* Incoming Parameter register */
	    case L_MAC_P4:	/* Incoming Parameter register */
	    case L_MAC_P5:	/* Incoming Parameter register */
	    case L_MAC_P6:	/* Incoming Parameter register */
	    case L_MAC_P7:	/* Incoming Parameter register */

	    case L_MAC_P8:	/* Outgoing Parameter register */
	    case L_MAC_P9:	/* Outgoing Parameter register */
	    case L_MAC_P10:	/* Outgoing Parameter register */
	    case L_MAC_P11:	/* Outgoing Parameter register */
	    case L_MAC_P12:	/* Outgoing Parameter register */
	    case L_MAC_P13:	/* Outgoing Parameter register */
	    case L_MAC_P14:	/* Outgoing Parameter register */
	    case L_MAC_P15:	/* Outgoing Parameter register */

	    case L_MAC_P16:	/* Return value register */
	    case L_MAC_P17:	/* Return value register */
	    case L_MAC_P18:	/* Return value register */
	    case L_MAC_P19:	/* Return value register */

	    case L_MAC_P20:	/* Float parameter register */
	    case L_MAC_P21:	/* Float parameter register */
	    case L_MAC_P22:	/* Float parameter register */
	    case L_MAC_P23:	/* Float parameter register */
	    case L_MAC_P24:	/* Float parameter register */
	    case L_MAC_P25:	/* Float parameter register */
	    case L_MAC_P26:	/* Float parameter register */
	    case L_MAC_P27:	/* Float parameter register */

	    case L_MAC_P28:	/* Float parameter register */
	    case L_MAC_P29:	/* Float parameter register */
	    case L_MAC_P30:	/* Float parameter register */
	    case L_MAC_P31:	/* Float parameter register */
	    case L_MAC_P32:	/* Float parameter register */
	    case L_MAC_P33:	/* Float parameter register */
	    case L_MAC_P34:	/* Float parameter register */
	    case L_MAC_P35:	/* Float parameter register */

	    case L_MAC_SP:	/* Stack ptr */
	    case L_MAC_IP:	/* Stack ptr alias for incoming param space */
	    case L_MAC_OP:	/* Stack ptr alias for outgoing param space */
	    case L_MAC_LV:	/* Stack ptr alias for local var space */

	    case L_MAC_PRED_ALL:	/* Alias for all predicate registers */
	      break;

	    default:
	      L_print_operand (stderr, operand, 1);
	      fprintf (stderr, "\n");
	      L_punt
		("C_emit_operand: Above MACRO operand unhandled/unknown!");
	    }
	}
      else
	L_punt ("C_emit_operand: Unsupported machine type %d\n", M_arch);

      if (L_is_ctype_integer (operand))
	{
	  C_emit_int_mac (out, operand->value.mac);
	}
      else if (L_is_ctype_predicate (operand))
	{
	  C_emit_pred_mac (out, operand->value.mac);
	}
      else if (L_is_ctype_flt (operand))
	{
	  C_emit_float_mac (out, operand->value.mac);
	}
      else if (L_is_ctype_dbl (operand))
	{
	  C_emit_double_mac (out, operand->value.mac);
	}
      else
	{
	  L_print_operand (stderr, operand, 1);
	  fprintf (stderr, "\n");
	  L_punt ("C_emit_operand: Above MACRO operand unhandled!");
	}
      break;

    case L_OPERAND_REGISTER:
      if (L_is_ctype_integer (operand))
	{
	  C_emit_int_reg (out, operand->value.r);
	}
      else if (L_is_ctype_predicate (operand))
	{
	  C_emit_pred_reg (out, operand->value.r);
	}
      else if (L_is_ctype_flt (operand))
	{
	  C_emit_float_reg (out, operand->value.r);
	}
      else if (L_is_ctype_dbl (operand))
	{
	  C_emit_double_reg (out, operand->value.r);
	}
      else
	{
	  L_print_operand (stderr, operand, 1);
	  L_punt ("C_emit_operand: Above REGISTER operand unhandled!");
	}
      break;

    case L_OPERAND_LABEL:
      /* If cb label in hash table, add C_prefix to name and
       * don't add &, since will be #defined to a constant by
       * C_emit_fn_jump_tables().
       */
      if (C_is_cb_label (operand->value.l))
	{
	  fprintf (out, "((%s)%s%s)", C_native_machine_ctype_str,
		   C_prefix, operand->value.l);
	}
      /* Otherwise, get address of true name */
      else
	{
	  fprintf (out, "((%s)&%s)", C_native_machine_ctype_str,
		   C_true_name (operand->value.l));
	}
      break;

    case L_OPERAND_RREGISTER:
      fprintf (stderr, "\n");
      L_print_operand (stderr, operand, 1);
      L_punt ("C_print_operand: L_OPERAND_RREGSITER not supported!");
      break;

    case L_OPERAND_EVR:
      fprintf (stderr, "\n");
      L_print_operand (stderr, operand, 1);
      L_punt ("C_print_operand: L_OPERAND_EVR not supported!");
      break;

    default:
      fprintf (stderr, "\n");
      L_print_operand (stderr, operand, 1);
      L_punt ("C_print_operand: Unknown operand type %i not supported!",
	      operand->type);
    }
}

/* Emit instructions that rotate the rotating registers.
 * 
 * Generates:
 * 
 */
void
C_emit_register_rotation (FILE * out, L_Func * fn)
{
  L_Attr *rr_attr = NULL;
  int int_base = 0, int_num = 0, flt_base = 0, flt_num = 0, dbl_base = 0;
  int dbl_num = 0, pred_base = 0, pred_num = 0;
  int loop;

  fprintf (out, "\n");

  /* Find the attribute that contains the rotating register numbers. */
  rr_attr = L_find_attr (fn->attr, "rr");

  /* Obtain the ranges of the rotating registers, or return the empty
     set of there are none for this function. */
  if (rr_attr != NULL)
    {
      int_base = rr_attr->field[0]->value.i;
      int_num = rr_attr->field[1]->value.i;
      flt_base = rr_attr->field[2]->value.i;
      flt_num = rr_attr->field[3]->value.i;
      dbl_base = rr_attr->field[4]->value.i;
      dbl_num = rr_attr->field[5]->value.i;
      pred_base = rr_attr->field[6]->value.i;
      pred_num = rr_attr->field[7]->value.i;
    }
  else
    {
      L_punt ("C_emit_register_rotation: cb marked for rotation, "
	      "but function has no rotating registers.");
    }

  /* Rotate the last int register into a temporary location. */
  fprintf (out,
	   "%s%sint int_tmp; float flt_tmp; double dbl_tmp; int pred_tmp;\n",
	   C_indent, C_indent);

  if (int_num > 0)
    {
      /* Rotate the last int register into a temporary location. */
      fprintf (out, "%s%sint_tmp = ", C_indent, C_indent);
      C_emit_int_reg (out, int_base + (int_num - 1));
      fprintf (out, ";\n");

      /* Loop over int registers to be rotated. */
      for (loop = (int_base + int_num - 2); loop >= int_base; loop--)
	{
	  fprintf (out, "%s%s", C_indent, C_indent);
	  C_emit_int_reg (out, loop + 1);
	  fprintf (out, " = ");
	  C_emit_int_reg (out, loop);
	  fprintf (out, ";\n");
	}

      /* Rotate the last int register (stored in the temporary location
         into the first register. */
      fprintf (out, "%s%s", C_indent, C_indent);
      C_emit_int_reg (out, int_base);
      fprintf (out, " = int_tmp;\n");
    }

  if (flt_num > 0)
    {
      /* Rotate the last flt register into a temporary location. */
      fprintf (out, "%s%sflt_tmp = ", C_indent, C_indent);
      C_emit_float_reg (out, flt_base + (flt_num - 1));
      fprintf (out, ";\n");

      /* Loop over flt registers to be rotated. */
      for (loop = (flt_base + flt_num - 2); loop >= flt_base; loop--)
	{
	  fprintf (out, "%s%s", C_indent, C_indent);
	  C_emit_float_reg (out, loop + 1);
	  fprintf (out, " = ");
	  C_emit_float_reg (out, loop);
	  fprintf (out, ";\n");
	}

      /* Rotate the last flt register (stored in the temporary location
         into the first register. */
      fprintf (out, "%s%s", C_indent, C_indent);
      C_emit_float_reg (out, flt_base);
      fprintf (out, " = flt_tmp;\n");
    }

  if (dbl_num > 0)
    {
      /* Rotate the last dbl register into a temporary location. 
         Assumes doubles are size 2.  MCM */
      fprintf (out, "%s%sdbl_tmp = ", C_indent, C_indent);
      C_emit_double_reg (out, dbl_base + (2 * (dbl_num - 1)));
      fprintf (out, ";\n");

      /* Loop over dbl registers to be rotated. */
      for (loop = (dbl_base + (2 * (dbl_num - 2))); loop >= dbl_base;
	   loop -= 2)
	{
	  fprintf (out, "%s%s", C_indent, C_indent);
	  C_emit_double_reg (out, (loop + 2));
	  fprintf (out, " = ");
	  C_emit_double_reg (out, loop);
	  fprintf (out, ";\n");
	}

      /* Rotate the last dbl register (stored in the temporary location
         into the first register. */
      fprintf (out, "%s%s", C_indent, C_indent);
      C_emit_double_reg (out, dbl_base);
      fprintf (out, " = dbl_tmp;\n");
    }

  if (pred_num > 0)
    {
      /* Rotate the last pred register into a temporary location. */
      fprintf (out, "%s%spred_tmp = ", C_indent, C_indent);
      C_emit_pred_reg (out, pred_base + (pred_num - 1));
      fprintf (out, ";\n");

      /* Loop over pred registers to be rotated. */
      for (loop = (pred_base + pred_num - 2); loop >= pred_base; loop--)
	{
	  fprintf (out, "%s%s", C_indent, C_indent);
	  C_emit_pred_reg (out, loop + 1);
	  fprintf (out, " = ");
	  C_emit_pred_reg (out, loop);
	  fprintf (out, ";\n");
	}

      /* Rotate the last pred register (stored in the temporary location
         into the first register. */
      fprintf (out, "%s%s", C_indent, C_indent);
      C_emit_pred_reg (out, pred_base);
      fprintf (out, " = pred_tmp;\n");
    }

  return;
}

/* Emit cast to function pointer of appropriate type,
 * if Ansi-C, must also include parameters in cast to
 * make sure floats get passed correctly.
 *
 * Also make sure having a return type of a function pointer also works.
 */
void
C_emit_func_ptr_cast (FILE * out, char *return_type_buf,
		      char *all_parm_type_buf)
{
  char formatted_buf[TYPE_BUF_SIZE];
  char raw_buf[TYPE_BUF_SIZE];
  char main_buf[TYPE_BUF_SIZE];
  char *parse_ptr;
  int index;

  /* Print core (*) function pointer and later parameters to formatted_buf, 
   * return type will be wrapped around everything (necessary for returning
   * function pointers) at the end.
   */
  sprintf (formatted_buf, "(*)(");

  /* Print out each parameter, if in emitting Ansi-C */
  parse_ptr = all_parm_type_buf;
  if (C_ansi_c_mode)
    {
      index = 1;
      while (*parse_ptr != 0)
	{
	  /* Get the next parameter type */
	  L_get_next_param_type (raw_buf, &parse_ptr);

	  /* Convert type to formatted string using C conventions,
	   * using empty name as the 'parameter'
	   */
	  L_convert_type_to_C_format (main_buf, raw_buf, "");

	  /* Add to end of formatted string */
	  strcat (formatted_buf, main_buf);

	  /* Increment parameter id */
	  index++;

	  /* Add comma if not at end */
	  if (*parse_ptr != 0)
	    strcat (formatted_buf, ", ");
	}
    }

  /* Add closing ) */
  strcat (formatted_buf, ")");

  /* Convert return type to formatted string using C conventions,
   * using function name + parms as the 'parameter'
   *
   * Copy formatted_buf into main_buf because buffers must not
   * overlap and I want the result in formatted_buf.
   */
  strcpy (main_buf, formatted_buf);
  L_convert_type_to_C_format (formatted_buf, return_type_buf, main_buf);

  /* Emit as cast */
  fprintf (out, "(%s)", formatted_buf);
}


/*
 * Emit the parameter list for a jsr op.
 */
void
C_emit_jsr_parms (FILE * out, L_Func * fn, L_Oper * op,
		  char all_parm_type_buf[], int first_index)
{
  char parm_type_buf[TYPE_BUF_SIZE];
  char cast_buf[TYPE_BUF_SIZE];
  char cast_ptr_buf[TYPE_BUF_SIZE];
  L_Attr *tr_attr, *tmo_attr, *tmso_attr, *trse_attr;
  L_Operand *attr_field, *parm_reg = NULL;
  int num_thru_reg, num_thru_mem, max_copied_struct;
  int parm_type, mem_offset = 0, thru_reg = 0;
  int param_index, param_reg_index;
  int max_reg_struct, use_trse;
  int struct_start_param = 0, struct_end_param = 0;
  char *parse_ptr;
  int num_llong_thru_reg = 0;

  /* Get tr attribute and determine number of parameters thru register */
  tr_attr = L_find_attr (op->attr, "tr");
  if (tr_attr != NULL)
    {
      num_thru_reg = tr_attr->max_field;
    }
  else
    {
      num_thru_reg = 0;
    }

  /* Get tmo attribute and determine number of parameters thru memory */
  tmo_attr = L_find_attr (op->attr, "tmo");
  if (tmo_attr != NULL)
    {
      num_thru_mem = tmo_attr->max_field;
    }
  else
    {
      num_thru_mem = 0;
    }
  /* Get tmso attribute (location in outgoing parameter space
   * where the "copies of structures" are expected by Lcode)
   */
  tmso_attr = L_find_attr (op->attr, "tmso");
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
  trse_attr = L_find_attr (op->attr, "trse");
  if (trse_attr != NULL)
    max_reg_struct = trse_attr->max_field / 2;
  else
    max_reg_struct = 0;
#endif

  /* Go through and handle each parameter */

  parse_ptr = all_parm_type_buf;
  param_index = first_index + 1;
  param_reg_index = first_index + 1;
  use_trse = 0;
  while (*parse_ptr != 0)
    {
      /* Get the next parameter type */
      L_get_next_param_type (parm_type_buf, &parse_ptr);

      if ((param_reg_index <= num_thru_reg) &&
	  L_is_macro (tr_attr->field[param_reg_index - 1]) &&
	  (tr_attr->field[param_reg_index - 1]->value.mac ==
	   (L_MAC_P0 + M_structure_pointer (0))))
	{
	  /* This parameter is the pointer to space allocated for
	     return-by-value structure.  Such returns are handled
	     by standard return-by-value in the emulated C code, 
	     so do not pass this paramater to the callee.

	     index points to the parm index in the parm string 
	     (eg. "int%int%float") for where these is NO entry
	     for the structure return pointer.  This index actually
	     refers to the next parm.

	     num_thru_reg was calculated by counting the number
	     of fields in the tr attribute, for which there IS an
	     entry for this return parameter.  We can ignore this
	     tr parm by reducing the number of tr parms examined,
	     ASSUMING that this return parameter is the LAST
	     tr parm in the list.
	     MCM 20000902
	   */
	  num_thru_reg--;
	}

      /* Is this parameter thru register ? */
      if (param_reg_index <= num_thru_reg + num_llong_thru_reg)
	{
	  /* Yes, thru register */
	  thru_reg = 1;

	  /* Make sure the operand is really there and a macro */
	  if (!L_is_macro (tr_attr->field[param_reg_index - 1]))
	    {
	      L_print_oper (stderr, op);
	      L_punt
		("C_emit_normal_jsr_op: Invalid tr_attr field for parm %i!",
		 param_reg_index);
	    }
	  parm_reg = tr_attr->field[param_reg_index - 1];
	}

      /* Otherwise, is this parameter thru memory ? */
      else if ((param_reg_index - num_thru_reg - num_llong_thru_reg) <= num_thru_mem)
	{
	  /* Yes, thru memory */
	  thru_reg = 0;

	  /* If structure thru memory, use tmso attribute to
	   * determine memory offset. 
	   */
	  if ((param_index <= max_copied_struct) &&
	      (L_is_int_constant (tmso_attr->field[param_index - 1])))
	    {
	      mem_offset =
		ITicast (tmso_attr->field[param_index - 1]->value.i);
	    }

	  /* Otherwise, get address from tmo attribute */
	  else
	    {
	      /* Make sure the operand is really there and a int constant */
	      attr_field =
		tmo_attr->field[param_reg_index - num_thru_reg - num_llong_thru_reg - 1];
	      if (!L_is_int_constant (attr_field))
		{
		  L_print_oper (stderr, op);
		  L_punt ("C_emit_normal_jsr_op: Invalid tmo_attr field "
			  "for parm %i!", param_index);
		}
	      mem_offset = ITicast (attr_field->value.i);
	    }
	}

      /* Otherwise, punt */
      else
	{
	  L_print_oper (stderr, op);
	  L_punt ("C_emit_normal_jsr_op: parameter %i,%i overruns \n"
		  "num_thru_reg (%i) and num_thru_mem (%i)!",
		  param_index, param_reg_index, num_thru_reg, num_thru_mem);
	}

      /* Handle thru-register case */
      if (thru_reg)
	{
	  /* Convert type into Lcode ctype (or CTYPE_STRUCT, if required). */
	  parm_type = L_convert_type_to_ctype (parm_type_buf);

          if(parm_type == L_CTYPE_LLONG) {
            num_thru_reg--;
            num_llong_thru_reg++;
          }

	  /* Handle each CTYPE appropriately */
	  switch (parm_type)
	    {
	    case L_CTYPE_INT:
	    case L_CTYPE_FLOAT:
	    case L_CTYPE_DOUBLE:
	      /* Convert type into C format for cast of parameter to
	       * proper C type.
	       */
	      L_convert_type_to_C_format (cast_buf, parm_type_buf, "");

	      /* Print out first part of cast */
	      fprintf (out, "((%s) ", cast_buf);

	      /* Print parm register with value */
	      C_emit_operand (out, fn, parm_reg);

	      /* Print out closing ')' of cast */
	      fprintf (out, ")");
	      break;
            case L_CTYPE_LLONG:
              fprintf(out, "l_emul_llong_buf[%d].x", num_llong_thru_reg - 1);
              break;
	    case CTYPE_STRUCT:
	      /* Convert type to C format for cast of parameter to 
	       * proper C type.  Make pointer to this type by passing
	       * '*' as parameter name.  This makes pointers to 
	       * function pointers work properly -ITI/JCG 4/99
	       */
	      L_convert_type_to_C_format (cast_ptr_buf, parm_type_buf, "*");

#ifdef IT64BIT
	      if (!trse_attr ||
		  !L_is_int_constant (trse_attr->field[(param_index - 1) * 2])
		  ||
		  !L_is_int_constant (trse_attr->field
				      [(param_index - 1) * 2 + 1]))
		{
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

#ifdef IT64BIT
	      if (!use_trse)
		{
#endif
		  /* Address of structure we are passing by value (copying)
		   * is in the parameter.  Dereference this pointer to
		   * pass actual structure contents.
		   */
		  /* Print out deference and first part of cast */
		  fprintf (out, "*((%s) ", cast_ptr_buf);

		  /* Print out parm register with addr */
		  C_emit_operand (out, fn, parm_reg);

		  /* Print out closing ')' of cast */
		  fprintf (out, ")");
#ifdef IT64BIT
		}
	      else
		{
		  /* Struct actually passed through the parm registers */
		  if ((param_index > max_copied_struct) ||
		      (!L_is_int_constant
		       (tmso_attr->field[param_index - 1])))
		    L_punt ("tmso for thru reg not found\n");
		  mem_offset =
		    ITicast (tmso_attr->field[param_index - 1]->value.i);
		  
		  /* Print out deference and first part of cast */
		  fprintf (out, "*((%s)( ", cast_ptr_buf);

		  /* Print out parm register with addr */
		  C_emit_int_mac (out, L_MAC_OP);
		  fprintf (out, " + %i", mem_offset);

		  /* Print out closing ')' of cast */
		  fprintf (out, "))");
		}
#endif
	      break;

	    case L_CTYPE_VOID:
	      L_punt ("C_emit_normal_jsr_op: parm %i VOID unexpected!",
		      param_index);
	      break;

	    default:
	      L_punt ("C_emit_normal_jsr_op: parm %i, unknown parm type %i!",
		      param_index, parm_type);
	    }
	}

      /* Otherwise, handle thru memory case */
      else
	{
	  /* Convert type into Lcode ctype (or CTYPE_STRUCT, if required). */
	  parm_type = L_convert_type_to_ctype (parm_type_buf);

	  /* Because char, short, etc. are in some rare cases not
	   * already promoted to int, we need to force these subtypes
	   * to int for the load from memory, then cast it to
	   * the appropriate subtype. -ITI/JCG 4/99
	   */

	  if (C_native_machine_ctype == L_CTYPE_INT)
	    {
	      if (parm_type == L_CTYPE_INT)
		{
		  /* Convert type into C format for cast of parameter to
		   * proper C type.
		   */
		  L_convert_type_to_C_format (cast_buf, parm_type_buf, "");

		  /* If actually an int, don't cast to int twice */
		  if (strcmp (cast_buf, "int") == 0)
		    {
		      /* Print out dereference and first part of cast */
		      fprintf (out, "*((int *)(");
		    }
		  /* Otherwise, load as int and cast to appropriate type */
		  else
		    {
		      /* Print out cast and first part of int dereference */
		      fprintf (out, "(%s)*((int *)(", cast_buf);
		    }
		}
	      /* Otherwise, cast as pointer to the appropriate type and
	       * dereference it.
	       */
	      else
		{
		  /* Convert type to C format for cast of parameter to 
		   * proper C type.  Make pointer to this type by passing
		   * '*' as parameter name.  This makes pointers to 
		   * function pointers work properly -ITI/JCG 4/99
		   *
		   * Note: This will not work for int types (as previously
		   *       thought) since sometimes short, etc. are not
		   *       automatically promoted to ints. -ITI/JCG 4/99
		   */
		  L_convert_type_to_C_format (cast_ptr_buf, parm_type_buf,
					      "*");

		  /* Print out dereference and first part of cast */
		  fprintf (out, "*((%s)(", cast_ptr_buf);
		}

	      /* Print out OP parameter space pointer */
	      C_emit_int_mac (out, L_MAC_OP);

	      /* Add offset into OP space */
	      fprintf (out, " + %i))", mem_offset);
	    }
	  else if (C_native_machine_ctype == L_CTYPE_LLONG)
	    {
	      if (parm_type == L_CTYPE_LLONG)
		{
		  /* Convert type into C format for cast of parameter to
		   * proper C type.
		   */
		  L_convert_type_to_C_format (cast_buf, parm_type_buf, "");

		  /* If actually an longlong, don't cast to longlong twice */
		  if (strcmp (cast_buf, "longlong") == 0)
		    {
		      /* Print out dereference and first part of cast */
		      fprintf (out, "*((longlong *)(");
		    }
		  /* Otherwise, load as llong and cast to appropriate type */
		  else
		    {
		      /* Print out cast and first part of int dereference */
		      fprintf (out, "(%s)*((longlong *)(", cast_buf);
		    }
		}
	      /* Otherwise, cast as pointer to the appropriate type and
	       * dereference it.
	       */
	      else
		{
		  /* Convert type to C format for cast of parameter to 
		   * proper C type.  Make pointer to this type by passing
		   * '*' as parameter name.  This makes pointers to 
		   * function pointers work properly -ITI/JCG 4/99
		   *
		   * Note: This will not work for int types (as previously
		   *       thought) since sometimes short, etc. are not
		   *       automatically promoted to ints. -ITI/JCG 4/99
		   */
		  L_convert_type_to_C_format (cast_ptr_buf, parm_type_buf,
					      "*");

		  /* Print out dereference and first part of cast */
		  fprintf (out, "*((%s)(", cast_ptr_buf);
		}

	      /* Print out OP parameter space pointer */
	      C_emit_int_mac (out, L_MAC_OP);

	      /* Add offset into OP space */
	      fprintf (out, " + %i))", mem_offset);
	    }
	  else
	    L_punt ("C_emit_jsr_parms: Unsupported machine register size.\n");

	}

#ifdef IT64BIT
      if (parm_type == CTYPE_STRUCT && thru_reg && use_trse)
	{
	  /* This manages the fact that a single struct
	     param can use multiple param registers */
	  param_reg_index += (struct_end_param - struct_start_param) + 1;
	  /*param_reg_index = (struct_end_param - 8)+1; */
	  param_index++;
	}
      else
#endif
	{
	  /* Increment parameter id */
	  param_index++;
	  param_reg_index++;
          if(L_convert_type_to_ctype (parm_type_buf) == L_CTYPE_LLONG && thru_reg)
            param_reg_index++;
	}

      /* Add comma if not at end */
      if (*parse_ptr != 0)
	fprintf (out, ", ");
    }
}


/* Emits C code for an unconditional pred def for a particular dest register.
 *
 * The code emitted is of the form:
 *   dest[0] = pred[0] && 'modifier'(('cast'src[0])'operator'('cast'src[1]));
 * 
 *   if (pred[0] == NULL), that portion of the test will be omitted;
 *
 *  'cast' and 'modifier' may be NULL, to specify no usage of that feature 
 *  (implicit parenthesis, if any, will be removed).

 */
void
C_emit_uncond_pred_def_dest (FILE * out, L_Func * fn, L_Oper * op,
			     L_Operand * dest, char *modifier,
			     char *cast, char *operator)
{
  /* Set indentation ' */
  fprintf (out, "%s", C_indent);

  /* Emit the dest register */
  C_emit_operand (out, fn, dest);

  /* Emit ' = ' */
  fprintf (out, " = %s_guard && (", C_prefix);

  /* Add modifier, if have modifier */
  if (modifier != NULL)
    fprintf (out, "%s(", modifier);

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

  /* Add modifier, if have modifier */
  if (modifier != NULL)
    fprintf (out, ")");

  /* Finish adding the predicate condition */
  fprintf (out, ")");

  /* Finished with this line */
  fprintf (out, ";\n");
}

/* Emits C code for an conditional pred def for a particular dest register.
 *
 * The code emitted is of the form:
 *   if (pred[0]) {dest[0]'assign'(('cast'src[0])'operator'('cast'src[1]));}
 * 
 *   if (pred[0] == NULL), that portion of the test will be omitted;
 *
 *  'cast' may be NULL, to specify no usage of that feature 
 *  (implicit parenthesis, if any, will be removed).

 */
void
C_emit_cond_pred_def_dest (FILE * out, L_Func * fn, L_Oper * op,
			   L_Operand * dest, char *assign,
			   char *cast, char *operator)
{
  /* Set indentation ' */
  fprintf (out, "%sif (%s_guard) { ", C_indent, C_prefix);

  /* Emit the dest register */
  C_emit_operand (out, fn, dest);

  /* Emit the assignment operator */
  fprintf (out, "%s", assign);

  /* Emit '(' around condition expression */
  fprintf (out, "(");

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

  /* Terminate statement, must go inside pred if */
  fprintf (out, ");}\n");

}

/* Emits C code for a squashing-and pred def for a particular dest register.
 *
 * The code emitted is of the form:
 *   dest[0] = pred[0] && dest[0] && 
 *             'modifier'(('cast'src[0])'operator'('cast'src[1]));
 * 
 *   if (pred[0] == NULL), that portion of the test will be omitted;
 *
 *  'cast' and 'modifier' may be NULL, to specify no usage of that feature 
 *  (implicit parenthesis, if any, will be removed).

 */
void
C_emit_sand_pred_def_dest (FILE * out, L_Func * fn, L_Oper * op,
			   L_Operand * dest, char *modifier,
			   char *cast, char *operator)
{
  /* Set indentation ' */
  fprintf (out, "%s", C_indent);

  /* Emit the dest register */
  C_emit_operand (out, fn, dest);

  /* Emit ' = ' */
  fprintf (out, " = %s_guard && ", C_prefix);

  C_emit_operand (out, fn, dest);

  fprintf (out, " && (");

  /* Add modifier, if have modifier */
  if (modifier != NULL)
    {
      fprintf (out, "%s(", modifier);
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

  /* Add modifier, if have modifier */
  if (modifier != NULL)
    {
      fprintf (out, ")");
    }

  fprintf (out, ")");

  /* Finished with this line */
  fprintf (out, ";\n");
}

/* Emit predicate def for a specific destination.  Uses
 * C_emit_uncond_pred_def_dest() and C_emit_cond_pred_def_dest() 
 * to emit actual code.  See them for examples.
 */

/*
 * Requires caller to have set up _EM_guard, containing the value of the
 * operation's guard predicate.
 */
void
C_emit_pred_def_dest (FILE * out, L_Func * fn, L_Oper * op, L_Operand * dest,
		      char *cast, char *operator)
{
  /* Determine if we should negate the test and whether the
   * predicate is set even if the test (or predicate) is false
   */
  switch (dest->ptype)
    {
    case L_PTYPE_UNCOND_T:
      C_emit_uncond_pred_def_dest (out, fn, op, dest, NULL, cast, operator);
      break;

    case L_PTYPE_UNCOND_F:
      C_emit_uncond_pred_def_dest (out, fn, op, dest, "!", cast, operator);
      break;

    case L_PTYPE_OR_T:
      C_emit_cond_pred_def_dest (out, fn, op, dest, " |= ", cast, operator);
      break;

    case L_PTYPE_OR_F:
      C_emit_cond_pred_def_dest (out, fn, op, dest, " |= !", cast, operator);
      break;

    case L_PTYPE_AND_T:
      C_emit_cond_pred_def_dest (out, fn, op, dest, " &= ", cast, operator);
      break;

    case L_PTYPE_AND_F:
      C_emit_cond_pred_def_dest (out, fn, op, dest, " &= !", cast, operator);
      break;

    case L_PTYPE_COND_T:
      C_emit_cond_pred_def_dest (out, fn, op, dest, " = ", cast, operator);
      break;

    case L_PTYPE_COND_F:
      C_emit_cond_pred_def_dest (out, fn, op, dest, " = !", cast, operator);
      break;

    case L_PTYPE_SAND_T:
      C_emit_sand_pred_def_dest (out, fn, op, dest, NULL, cast, operator);
      break;

    case L_PTYPE_SAND_F:
      C_emit_sand_pred_def_dest (out, fn, op, dest, "!", cast, operator);
      break;

    default:
      L_print_operand (stderr, dest, 1);
      L_punt ("C_emit_pred_def_dest: Unknown predicate dest type %i\n",
	      dest->ptype);
    }
}

char *
C_get_compare_token (ITuint8 com)
{
  switch (com)
    {
    case Lcmp_COM_EQ:
      return " == ";
    case Lcmp_COM_NE:
      return " != ";
    case Lcmp_COM_GT:
      return " > ";
    case Lcmp_COM_LE:
      return " <= ";
    case Lcmp_COM_GE:
      return " >= ";
    case Lcmp_COM_LT:
      return " < ";
    default:
      L_punt ("C_get_compare_token: bad compare type");
    }
  return NULL;
}


char *
C_get_compare_cast (ITuint8 ctype)
{
  switch (ctype)
    {
    case L_CTYPE_CHAR:
      return "char";
    case L_CTYPE_UCHAR:
      return "unsigned char";
    case L_CTYPE_SHORT:
      return "short";
    case L_CTYPE_USHORT:
      return "unsigned short";
    case L_CTYPE_INT:
      return "int";
    case L_CTYPE_UINT:
      return "unsigned int";
    case L_CTYPE_LONG:
      return "long";
    case L_CTYPE_ULONG:
      return "unsigned long";
    case L_CTYPE_LLONG:
      return "long long";
    case L_CTYPE_ULLONG:
      return "unsigned long long";

    case L_CTYPE_LLLONG:
    case L_CTYPE_ULLLONG:
      L_punt ("C_get_compare_cast: LLLONG not yet implemented %d", ctype);
      return 0;
    case L_CTYPE_POINTER:
      return C_native_machine_ctype_str;
    case L_CTYPE_VOID:
      L_punt ("C_get_compare_cast: VOID not yet implemented %d", ctype);
      return 0;
    case L_CTYPE_FLOAT:
      return "float";
    case L_CTYPE_DOUBLE:
      return "double";
    case L_CTYPE_PREDICATE:
      L_punt ("C_get_compare_cast: PREDICATE not yet implemented %d", ctype);
      return 0;
    case L_CTYPE_CONTROL:
      L_punt ("C_get_compare_cast: CONTROL not yet implemented %d", ctype);
      return 0;
    case L_CTYPE_BTR:
    case L_CTYPE_LOCAL_ABS:
    case L_CTYPE_LOCAL_GP:
    case L_CTYPE_GLOBAL_ABS:
    case L_CTYPE_GLOBAL_GP:
      return C_native_machine_ctype_str;
    default:
      L_punt ("C_get_compare_cast: Illegal type %d", ctype);
      return 0;
    }
}
