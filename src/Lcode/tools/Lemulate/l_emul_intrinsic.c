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
 *      File: l_emul_intrinsic.c
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
#include <Lcode/l_main.h>
#include <library/md.h>
#include "l_emul_intrinsic.h"
#include "l_emul_emit_operand.h"

/* indentation defined in l_emul.c */
extern char *C_indent;

/* Changes to 1 if an intrinsic is found and requires emulation code. */
static int C_intrinsic_include_required = 0;

static void
Lem_strunquote (char *dest, char *src)
{
  strncpy (dest, &src[1], 80);
  dest[strlen (dest) - 1] = '\0';
}


static char *
Lem_Intrinsic_type_to_C_type (char *name)
{
  if (!strcmp (name, "Integer"))
    return ("int");
  else if (!strcmp (name, "Long"))
    return ("long");
  else if (!strcmp (name, "Short"))
    return ("short");
  else if (!strcmp (name, "Char"))
    return ("char");
  else if (!strcmp (name, "Float"))
    return ("float");
  else if (!strcmp (name, "Double"))
    return ("double");

  L_punt ("invalid intrinsic operand type");
  return NULL;
}


/**************************************************************************
 * Lem_Intrinsic_IncludeHook
 *
 * This function is called at an appropriate time to add any include
 * files that may be necessary to emulate intrinsics.
 *************************************************************************/
void
Lem_Intrinsic_IncludeHook (FILE * include_out)
{
  if (C_intrinsic_include_required == 1)
    {
      fprintf (include_out, "#include <intrinsic.h>\n");
    }
}


/**************************************************************************
 * C_emit_intrinsic_op
 *
 * Emits emulation code for intrinsic operations.  The name of the
 * function to call for emulation is referenced in the intrinsic database.
 *
 * Returns:
 *   0 - if the opcode was handled.
 *   1 - if the opcode was not handled.
 *************************************************************************/
int
C_emit_intrinsic_op (FILE * out, L_Func * fn, L_Oper * op)
{
  MD_Entry *entry;
  MD_Field *field;
  L_Attr *opcode_attr;
  L_Operand *fn_name;
  char tmp_str[80];
  int i;
  int tmp_int;


  C_intrinsic_include_required = 1;

   /***********************************************************************
    * Fetch the I_opcode attribute and look up the opcode in the database.
    **********************************************************************/
  if ((opcode_attr = L_find_attr (op->attr, "I_opcode")) == NULL)
    {
      L_punt ("C_emit_intrinsic_op: missing opcode attribute to an\n"
	      "   intrinsic operand.  This is not valid Lcode.");
    }
  fn_name = opcode_attr->field[0];
  Lem_strunquote (tmp_str, fn_name->value.s);
  if ((entry = MD_find_entry (L_intrinsic_intrinsic_section_g, tmp_str))
      == NULL)
    {
      L_punt ("C_emit_intrinsic_op: intrinsic opcode not found in database.\n"
	      "   Be sure that the database being used is the same as the\n"
	      "   database used to generate this Lcode.");
    }

   /***********************************************************************
    * If the opcode is not Lop_INTRINSIC, then the I_opcode attribute
    * and the opcode have to match.  Otherwise, we do not handle the 
    * emulation.
    **********************************************************************/
  if (op->proc_opc != Lop_INTRINSIC)
    {
      field = MD_find_field (entry, L_intrinsic_opcode_field_decl_g);
      if (field != NULL)
	{
	  char *nativeOpcode;

	  nativeOpcode = MD_get_string (field, 0);
	  if (strcmp (nativeOpcode, L_opcode_name (op->proc_opc)))
	    {
	      return (1);
	    }
	}
      else
	{
	  L_punt
	    ("C_emit_intrinsic_op: the Lcode looks as if this opcode was\n"
	     "   originally an intrinsic, but the intrinsic database does\n"
	     "   not agree.  Make sure the intrinsic database has not\n"
	     "   changed since Lcode generation.");
	}
    }


  /* Set indentation */
  fprintf (out, "%s", C_indent);

  /* Add if () {} around code, if predicated -ITI/JCG 8/99 */
  if (op->pred[0] != NULL)
    {
      fprintf (out, "if (");
      C_emit_operand (out, fn, op->pred[0]);
      fprintf (out, ") {");
    }

  /* Output assignment */
  if (op->dest[0])
    {
      L_Operand *operand = op->dest[0];

      /********************************************************************
       * If the operand is a macro and one of our intrinsic registers,
       * then skip the destination assignment.  Otherwise, the `real'
       * destination is always first and there are no additional `real'
       * destinations.
       *******************************************************************/
      if (!L_is_intrinsic_register (operand))
	{
	  C_emit_operand (out, fn, operand);
	  fprintf (out, " = (int) ");
	}
    }
  else
    {
      fprintf (out, "(void) ");
    }

   /***********************************************************************
    * Lookup the function call name to substitute for the opcode.
    **********************************************************************/
  if (!(op->flags & L_OPER_MASK_PE))
    field = MD_find_field (entry, L_intrinsic_functioncall_field_decl_g);
  else
    {
      field =
	MD_find_field (entry, L_intrinsic_functioncallsilent_field_decl_g);

      if (!field)
	field = MD_find_field (entry, L_intrinsic_functioncall_field_decl_g);
    }
  fprintf (out, "%s(", MD_get_string (field, 0));


   /***********************************************************************
    * Output source operands.
    **********************************************************************/
  field = MD_find_field (entry, L_intrinsic_srctype_field_decl_g);
  if (field)
    {
      int first_emitted = 1;

      tmp_int = MD_num_elements (field);
      for (i = 0; i < tmp_int; i++)
	{
	  L_Operand *operand = op->src[i];

	 /*****************************************************************
          * If the operand is a macro and one of our intrinsic registers,
          * then don't output as a source.
          ****************************************************************/
	  if (L_is_intrinsic_register (operand))
	    {
	      continue;
	    }

	  if (!first_emitted)
	    {
	      fprintf (out, ", ");
	    }
	  fprintf (out, "(%s)",
		   Lem_Intrinsic_type_to_C_type (MD_get_link
						 (field, i)->name));
	  C_emit_operand (out, fn, op->src[i]);
	  first_emitted = 0;
	}
    }

  /* Terminate statement, must go inside if */
  fprintf (out, ");");

  /* End predicated if statement (if exists) -ITI/JCG 8/99 */
  if (op->pred[0] != NULL)
    {
      fprintf (out, "}");
    }

  /* Finished with this line */
  fprintf (out, "\n");

  return (0);
}
