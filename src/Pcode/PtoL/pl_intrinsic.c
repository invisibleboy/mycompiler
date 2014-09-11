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

#include <config.h>
#include "pl_main.h"
#include <library/md.h>

/* First macro operand to use for intrinsic register dependencies */
#define L_FIRST_INTRINSIC_MAC               L_MAC_P32

/**************************************************************************
 * Simple routine to add quotes to a string so that it can be
 * added as an Lcode attribute.
 *************************************************************************/
void
straddquotes (char *dest, char *src)
{
  dest[0] = '\"';
  strcpy (&dest[1], src);
  dest[strlen (dest) + 1] = '\0';
  dest[strlen (dest)] = '\"';
}


void
PL_Intrinsic_Shutdown ()
{
}


int
PL_Intrinsic_OperandType (char *name)
{
  if (!strcmp (name, "Integer"))
    return (M_TYPE_INT);
  else if (!strcmp (name, "Long"))
    return (M_TYPE_LONG);
  else if (!strcmp (name, "LLong"))
    return (M_TYPE_LLONG);
  else if (!strcmp (name, "Short"))
    return (M_TYPE_SHORT);
  else if (!strcmp (name, "Char"))
    return (M_TYPE_CHAR);
  else if (!strcmp (name, "Float"))
    return (M_TYPE_FLOAT);
  else if (!strcmp (name, "Double"))
    return (M_TYPE_DOUBLE);

  P_punt ("invalid intrinsic operand type");

  return -1;
}


/**************************************************************************
 * This function is called from ``gen_call_data'' to generate an 
 * intrinsic opcode if the function name matches that of a recognized
 * intrinsic.  Instead of a call, an intrinsic is inserted into the Lcode.
 * Attributes to the opcode as well as the intrinsic database contain 
 * all the information about the alteration.
 *
 * ITI/JWJ
 * June 1999
 *************************************************************************/
int
PL_intrinsic_intrinsify (char *fn_name, PL_Ret result,
			 Expr * temp_expr, _PL_Ret ret,
			 L_Cb * cb, PL_Operand dest, _PL_Operand src[],
			 int n_src)
{
  MD_Field *field;
  MD_Field *srctype_field;
  MD_Entry *entry;
  MD_Entry *tmp_entry;
  int dest_type;
  L_Oper *new_oper;
  L_Attr *new_attr;
  int i;
  char txt[40];
  int dest_present;
  int opcode_is_intrinsic;



  /***********************************************************************
   * Search the Intrinsics section for a matching function name.
   **********************************************************************/
  entry = MD_first_entry (L_intrinsic_intrinsic_section_g);
  while (entry != NULL)
    {
      field = MD_find_field (entry, L_intrinsic_functioncall_field_decl_g);

      if (!strcmp (fn_name, MD_get_string (field, 0)))
	break;

      entry = MD_next_entry (entry);
    }
  if (entry == NULL)
    return (0);

  /***********************************************************************
   * Stop now if substitution is disabled for this intrinsic.
   **********************************************************************/
  field = MD_find_field (entry, L_intrinsic_enabled_field_decl_g);
  if (MD_get_int (field, 0) == 0)
    return (0);

  for (i = 0; i < n_src; i++)
    {
      PLI_gen_data (cb, temp_expr[i], &ret);
      PLI_simplify (cb, &ret);
      src[i] = ret.op1;
    }

   /***********************************************************************
    * Determine the destination operand type.
    **********************************************************************/
  field = MD_find_field (entry, L_intrinsic_desttype_field_decl_g);
  if (field)
    {
      tmp_entry = MD_get_link (field, 0);
      dest_type = PL_Intrinsic_OperandType (tmp_entry->name);
      dest_present = 1;

      PL_new_register (dest, PL_next_reg_id (), dest_type, 0);
      result->type = PL_RET_SIMPLE;
      result->op1 = *dest;
      PL_new_int (&(result->op2), 0, 0);
    }
  else
    {
      dest_present = 0;
      result->type = PL_RET_NONE;
    }

  /***********************************************************************
   * Do some type checking.  P_punt on failure.
   **********************************************************************/
  srctype_field = MD_find_field (entry, L_intrinsic_srctype_field_decl_g);
  if (!srctype_field)
    {
      if (n_src != 0)
	P_punt ("PL_intrinsic_intrinsify: no source arguments expected");
    }
  else
    {
      if (n_src != MD_num_elements (srctype_field))
	P_punt ("PL_intrinsic_intrinsify: incorrect number of arguments");

      for (i = 0; i < n_src; i++)
	{
	  tmp_entry = MD_get_link (srctype_field, i);
	  switch (PL_Intrinsic_OperandType (tmp_entry->name))
	    {
	    case M_TYPE_INT:
	      if (((src + i)->data_type) != M_TYPE_INT)
		P_punt ("PL_intrinsic_intrinsify: incorrect src types.\n"
			"   Argument %d of %s: expected an Integer.",
			i + 1, fn_name);
	      break;
	    case M_TYPE_LONG:
	      if ((((src + i)->data_type) != M_TYPE_LONG) &&
		  (((src + i)->data_type) != M_TYPE_INT))
		P_punt ("PL_intrinsic_intrinsify: incorrect src types.\n"
			"   Argument %d of %s: expected a Long.",
			i + 1, fn_name);
	      break;
	    case M_TYPE_LLONG:
	      if ((((src + i)->data_type) != M_TYPE_LLONG) &&
		  (((src + i)->data_type) != M_TYPE_INT))
		P_punt ("PL_intrinsic_intrinsify: incorrect src types.\n"
			"   Argument %d of %s: expected a Llong.",
			i + 1, fn_name);
	      break;
	    case M_TYPE_SHORT:
	      if ((((src + i)->data_type) != M_TYPE_SHORT) &&
		  (((src + i)->data_type) != M_TYPE_INT))
		P_punt ("PL_intrinsic_intrinsify: incorrect src types.\n"
			"   Argument %d of %s: expected a Short.",
			i + 1, fn_name);
	      break;
	    case M_TYPE_CHAR:
	      if (((src + i)->data_type) != M_TYPE_CHAR)
		P_punt ("PL_intrinsic_intrinsify: incorrect src types.\n"
			"   Argument %d of %s: expected a Char.",
			i + 1, fn_name);
	      break;
	    case M_TYPE_FLOAT:
	      if (((src + i)->data_type) != M_TYPE_FLOAT)
		P_punt ("PL_intrinsic_intrinsify: incorrect src types.\n"
			"   Argument %d of %s: expected a Float.",
			i + 1, fn_name);
	      break;
	    case M_TYPE_DOUBLE:
	      if (((src + i)->data_type) != M_TYPE_DOUBLE)
		P_punt ("PL_intrinsic_intrinsify: incorrect src types.\n"
			"   Argument %d of %s: expected a Double.",
			i + 1, fn_name);
	      break;
	    }
	}
    }

  /**********************************************************************
   * All our checks have passed so we can build the intrinsic op.
   *  - if the ``opcode'' field is defined, use that.
   *  - otherwise, use opcode ``intrinsic''.
   *********************************************************************/

  field = MD_find_field (entry, L_intrinsic_opcode_field_decl_g);
  if (field != NULL)
    {
      int id;
      char *nativeOpcode;

      nativeOpcode = MD_get_string (field, 0);
      id = L_opcode_id (nativeOpcode);
      if (id == -1)
	{
	  P_punt ("PL_intrinsic_intrinsify: "
		  "Opcode \"%s\" is not a recognized\n"
		  "     Lcode opcode.", MD_get_string (field, 0));
	}
      new_oper = PL_new_loper (NULL, id);
      opcode_is_intrinsic = 0;
    }
  else
    {
      new_oper = PL_new_loper (NULL, Lop_INTRINSIC);
      opcode_is_intrinsic = 1;
    }

  /**********************************************************************
   * Create attributes to describe the intrinsic.
   *  All we keep is the opcode.  The rest will be looked up in the
   *  intrinsic database on a need-to-know basis.
   *********************************************************************/
  new_attr = L_new_attr ("I_opcode", 1);
  straddquotes (txt, entry->name);
  L_set_string_attr_field (new_attr, 0, txt);
  new_oper->attr = L_concat_attr (new_oper->attr, new_attr);

  /**********************************************************************
   * Add our sources and destinations to the instruction.
   *********************************************************************/
  if (dest_present)
    new_oper->dest[0] = PL_gen_operand (dest);
  for (i = 0; i < n_src; i++)
    new_oper->src[i] = PL_gen_operand (src + i);

  /**********************************************************************
   * If the opcode isn't an ``intrinsic'' opcode (i.e. it has been
   * mapped to a native Lcode instruction), then we're done.
   * Otherwise, we still have some work to do.
   *   - add macro registers for implicit sources/destinations.
   *********************************************************************/

  if (opcode_is_intrinsic == 0)
    {
      L_insert_oper_after (cb, cb->last_op, new_oper);
      return (1);
    }

  /**********************************************************************
   * Add macro register sources to force dependencies on intrinsic
   * registers.  These registers are never actually used in IMPACT except
   * for dependence analysis.
   *********************************************************************/
  field = MD_find_field (entry, L_intrinsic_srcdepend_field_decl_g);
  if (field != NULL)
    {
      for (i = 0; i < MD_num_elements (field); i++)
	{
	  MD_Entry *regdep_entry;
	  L_Operand *macro_operand;
	  int id;

	  regdep_entry = MD_get_link (field, i);

	  id = L_find_symbol_id (L_macro_symbol_table, regdep_entry->name);
	  macro_operand = L_new_macro_operand (id, L_CTYPE_INT, L_PTYPE_NULL);
	  new_oper->src[n_src + i] = macro_operand;
	}
    }

  /**********************************************************************
   * Add macro register destinations to force dependencies on intrinsic
   * registers.  These registers are never actually used in IMPACT except
   * for dependence analysis.
   *********************************************************************/

  field = MD_find_field (entry, L_intrinsic_destdepend_field_decl_g);
  if (field != NULL)
    {
      for (i = 0; i < MD_num_elements (field); i++)
	{
	  MD_Entry *regdep_entry;
	  L_Operand *macro_operand;
	  int id;

	  regdep_entry = MD_get_link (field, i);

	  id = L_find_symbol_id (L_macro_symbol_table, regdep_entry->name);
	  macro_operand = L_new_macro_operand (id, L_CTYPE_INT, L_PTYPE_NULL);
	  new_oper->dest[i + (dest_present != 0)] = macro_operand;
	}
    }

  L_insert_oper_after (cb, cb->last_op, new_oper);
  return (1);
}
