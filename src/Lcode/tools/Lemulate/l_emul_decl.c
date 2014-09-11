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
 *      File: l_emul_decl.c
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

/* Creates and initializes a Reg_Usage structure and returns
 * it to the caller.
 */
Reg_Usage *
C_new_reg_usage (char *name)
{
  Reg_Usage *reg_usage;

  /* Malloc the reg_usage structure */
  if ((reg_usage = (Reg_Usage *) malloc (sizeof (Reg_Usage))) == NULL)
    L_punt ("C_new_reg_usage: Out of memory");

  /* Set the structure name */
  if ((reg_usage->name = strdup (name)) == NULL)
    L_punt ("C_new_reg_usage: Out of memory");

  /* Create each integer symbol table with the default size */
  reg_usage->pred_regs_used = INT_new_symbol_table ("pred_regs_used", 0);
  reg_usage->int_regs_used = INT_new_symbol_table ("int_regs_used", 0);
  reg_usage->float_regs_used = INT_new_symbol_table ("float_regs_used", 0);
  reg_usage->double_regs_used = INT_new_symbol_table ("double_regs_used", 0);
  reg_usage->pred_macs_used = INT_new_symbol_table ("pred_macs_used", 0);
  reg_usage->int_macs_used = INT_new_symbol_table ("int_macs_used", 0);
  reg_usage->float_macs_used = INT_new_symbol_table ("float_macs_used", 0);
  reg_usage->double_macs_used = INT_new_symbol_table ("double_macs_used", 0);

  /* Return the newly created structure */
  return (reg_usage);
}

/* Frees the passed Reg_Usage structure  */
void
C_delete_reg_usage (Reg_Usage * reg_usage)
{
  /* Free structure name */
  free (reg_usage->name);

  /* Delete all the symbol tables (no data, so pass NULL for free routine) */
  INT_delete_symbol_table (reg_usage->pred_regs_used, NULL);
  INT_delete_symbol_table (reg_usage->int_regs_used, NULL);
  INT_delete_symbol_table (reg_usage->float_regs_used, NULL);
  INT_delete_symbol_table (reg_usage->double_regs_used, NULL);
  INT_delete_symbol_table (reg_usage->pred_macs_used, NULL);
  INT_delete_symbol_table (reg_usage->int_macs_used, NULL);
  INT_delete_symbol_table (reg_usage->float_macs_used, NULL);
  INT_delete_symbol_table (reg_usage->double_macs_used, NULL);

  /* Free the overall structure */
  free (reg_usage);
}

/* Adds register usage (if not already present) to the proper table based on 
 * operand type.  Will ignore operands that are not a normal register or
 * macro register.
 */
void
C_add_reg_usage (Reg_Usage * reg_usage, L_Operand * operand)
{
  INT_Symbol_Table *table;
  int reg_id;

  /* Select table and method of getting reg_id based on operand passed */
  reg_id = -1;
  table = NULL;

  /* Is it a normal register? */
  if (L_is_register (operand))
    {
      /* Use register number as register id */
      reg_id = operand->value.r;

      /* Pick appropriate table based on operand type */
      if (L_is_ctype_integer (operand))
	{
	  table = reg_usage->int_regs_used;
	}
      else if (L_is_ctype_predicate (operand))
	{
	  table = reg_usage->pred_regs_used;
	}
      else if (L_is_ctype_flt (operand))
	{
	  table = reg_usage->float_regs_used;
	}
      else if (L_is_ctype_dbl (operand))
	{
	  table = reg_usage->double_regs_used;
	}
      else
	{
	  L_print_operand (stderr, operand, 1);
	  L_punt ("C_add_reg_usage: Above REGISTER operand unhandled!");
	}
    }

  /* Otherwise, is it a macro register? */
  else if (L_is_macro (operand))
    {
      /* Use macro id as register id */
      reg_id = operand->value.mac;

      /* Don't add stack & aliases, handled in C_emit_stack_setup (), 
       * unless using register arrays -ITI/JCG 4/99.
       */
      if (((reg_id == L_MAC_SP) ||
	   (reg_id == L_MAC_IP) ||
	   (reg_id == L_MAC_OP) ||
	   (reg_id == L_MAC_LV)) && (!C_use_register_arrays))
	{
	  return;
	}

      /* Pick appropriate table based on operand type */
      if (L_is_ctype_integer (operand))
	{
	  table = reg_usage->int_macs_used;
	}
      else if (L_is_ctype_predicate (operand))
	{
	  table = reg_usage->pred_macs_used;
	}
      else if (L_is_ctype_flt (operand))
	{
	  table = reg_usage->float_macs_used;
	}
      else if (L_is_ctype_dbl (operand))
	{
	  table = reg_usage->double_macs_used;
	}
      else
	{
	  L_print_operand (stderr, operand, 1);
	  L_punt ("C_add_reg_usage: Above REGISTER operand unhandled!");
	}
    }

  /* Otherwise return without doing anything, not register */
  else
    {
      return;
    }

  /* Add reg_id if not already in the table */
  if (INT_find_symbol (table, reg_id) == NULL)
    {
      INT_add_symbol (table, reg_id, NULL);
    }
}

/* Adds register usage (if not already present) to the proper table based on 
 * operand type.  Will ignore operands that are not a normal register or
 * macro register.
 */
void
C_add_reg_id_usage (Reg_Usage * reg_usage, int reg_id, int ctype)
{
  INT_Symbol_Table *table;

  /* Select table and method of getting reg_id based on operand passed */
  table = NULL;

  /* Pick appropriate table based on operand type */
  if (L_is_ctype_int_direct (ctype))
    {
      table = reg_usage->int_regs_used;
    }
  else if (L_is_ctype_predicate_direct (ctype))
    {
      table = reg_usage->pred_regs_used;
    }
  else if (L_is_ctype_float_direct (ctype))
    {
      table = reg_usage->float_regs_used;
    }
  else if (L_is_ctype_double_direct (ctype))
    {
      table = reg_usage->double_regs_used;
    }
  else
    {
      L_punt ("C_add_reg_id_usage: "
	      "register operand %d of ctype 0x%X unhandled!", reg_id, ctype);
    }

  /* Add reg_id if not already in the table */
  if (INT_find_symbol (table, reg_id) == NULL)
    {
      INT_add_symbol (table, reg_id, NULL);
    }
}

/* Scans all the operands in the function, and adds all registers found
 * to the fn_scope_reg_usage tables.  If global_reg_usage is not NULL, 
 * adds register found to it also.
 */
void
C_add_fn_reg_usage (L_Func * fn, Reg_Usage * fn_scope_reg_usage,
		    Reg_Usage * global_reg_usage)
{
  L_Cb *cb;
  L_Oper *op;
  L_Attr *tr_attr, *ret_attr, *rr_attr;
  L_Operand *ret_reg, *stack_reg;
  char return_type_buf[TYPE_BUF_SIZE];
  int index, proc_opc;
  int ctype;
  int int_base, int_num, flt_base, flt_num,
    dbl_base, dbl_num, pred_base, pred_num;
  int loop;

  /* If using register arrays, stack setup code requires 
   * that SP, IP, OP, and LV be defined here so proper
   * array space can be allocated. 
   */
  if (C_use_register_arrays)
    {
      stack_reg = L_new_macro_operand (L_MAC_SP, C_native_machine_ctype, 
				       L_PTYPE_NULL);
      C_add_reg_usage (fn_scope_reg_usage, stack_reg);
      if (global_reg_usage)
	C_add_reg_usage (global_reg_usage, stack_reg);
      L_delete_operand (stack_reg);

      stack_reg = L_new_macro_operand (L_MAC_IP, C_native_machine_ctype, 
				       L_PTYPE_NULL);
      C_add_reg_usage (fn_scope_reg_usage, stack_reg);
      if (global_reg_usage)
	C_add_reg_usage (global_reg_usage, stack_reg);
      L_delete_operand (stack_reg);

      stack_reg = L_new_macro_operand (L_MAC_OP, C_native_machine_ctype, 
				       L_PTYPE_NULL);
      C_add_reg_usage (fn_scope_reg_usage, stack_reg);
      if (global_reg_usage)
	C_add_reg_usage (global_reg_usage, stack_reg);
      L_delete_operand (stack_reg);

      stack_reg = L_new_macro_operand (L_MAC_LV, C_native_machine_ctype, 
				       L_PTYPE_NULL);
      C_add_reg_usage (fn_scope_reg_usage, stack_reg);
      if (global_reg_usage)
	C_add_reg_usage (global_reg_usage, stack_reg);
      L_delete_operand (stack_reg);
    }

  /* Add incoming parameters passed thru registers.  Not
   * all of these parameters may actually be used by the
   * program, but the parameter fixup code will define them.
   */
  if ((tr_attr = L_find_attr (fn->attr, "tr")) != NULL)
    {
      /* Go thru every parameter thru register specified */
      for (index = 0; index < tr_attr->max_field; index++)
	{
	  /* Sanity check */
	  if (tr_attr->field[index] == NULL)
	    {
	      fprintf (stderr, "In function %s", fn->name);
	      L_print_attr (stderr, tr_attr);
	      L_punt ("C_add_fn_reg_usage: field[%i] NULL!", index);
	    }
	  C_add_reg_usage (fn_scope_reg_usage, tr_attr->field[index]);
	  if (global_reg_usage != NULL)
	    {
	      C_add_reg_usage (global_reg_usage, tr_attr->field[index]);
	    }
	}
    }

  /* For every cb in the function */
  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      /* For every op in the cb */
      for (op = cb->first_op; op != NULL; op = op->next_op)
	{
	  /* Ignore compiler directives */
	  proc_opc = op->proc_opc;
	  if ((proc_opc == Lop_DEFINE) ||
	      (proc_opc == Lop_PROLOGUE) || (proc_opc == Lop_EPILOGUE))
	    continue;

	  /* Handle explicit/implicit return register for function calls */
	  if ((proc_opc == Lop_JSR) || (proc_opc == Lop_JSR_FS))
	    {
	      /* Has the return register been specified thru a ret attr? */
	      if ((ret_attr = L_find_attr (op->attr, "ret")) != NULL)
		{
		  /* Get return register from first field */
		  ret_reg = ret_attr->field[0];

		  /* Sanity check, better be a register or macro */
		  if (!L_is_reg (ret_reg) && !L_is_macro (ret_reg))
		    {
		      L_print_oper (stderr, op);
		      L_punt ("C_add_fn_reg_usage: Unexpected 'ret' attr "
			      "contents!");
		    }
		  C_add_reg_usage (fn_scope_reg_usage, ret_reg);
		  if (global_reg_usage != NULL)
		    {
		      C_add_reg_usage (global_reg_usage, ret_reg);
		    }
		}
	      /* Otherwise, better not be expecting a return value! */
	      else
		{
		  L_get_call_info (fn, op, op->attr, return_type_buf, NULL,
				   sizeof (return_type_buf));

		  /* Convert type string into type */
		  ctype = L_convert_type_to_ctype (return_type_buf);

		  /* Better be void type! */
		  if (ctype != L_CTYPE_VOID)
		    {
		      fprintf (stderr, "In function %s:\n", fn->name);
		      L_print_oper (stderr, op);
		      L_punt ("C_add_fn_reg_usage: Expect 'ret' attr!");
		    }
		}
	    }

	  /* Handle explicit/implicit return register for the function */
	  if ((proc_opc == Lop_RTS) || (proc_opc == Lop_RTS_FS))
	    {
	      /* Has the return register been specified thru a tr 
	       * or a utr attr? */
	      if (((tr_attr = L_find_attr (op->attr, "tr")) != NULL) ||
		  ((tr_attr = L_find_attr (op->attr, "utr")) != NULL))
		{
		  /* Get return register from first field */
		  ret_reg = tr_attr->field[0];

		  /* Sanity check, better be a register or macro */
		  if (!L_is_reg (ret_reg) && !L_is_macro (ret_reg))
		    {
		      L_print_oper (stderr, op);
		      L_punt ("C_add_fn_reg_usage: Unexpected 'tr' or 'utr' "
			      "attr contents!");
		    }
		  C_add_reg_usage (fn_scope_reg_usage, ret_reg);
		  if (global_reg_usage != NULL)
		    {
		      C_add_reg_usage (global_reg_usage, ret_reg);
		    }
		}
	      /* Otherwise, use implicit return register for function */
	      else
		{
		  L_get_call_info (fn, NULL, fn->attr, return_type_buf, NULL,
				   sizeof (return_type_buf));

		  /* Convert type string into type */
		  ctype = L_convert_type_to_ctype (return_type_buf);

		  /* Better be void type! */
		  if (ctype != L_CTYPE_VOID)
		    {
		      fprintf (stderr, "In function %s:\n", fn->name);
		      L_print_oper (stderr, op);
		      L_punt ("C_add_fn_reg_usage: Expect 'tr' attr!");
		    }
		}
	    }

	  /* Add pred[0], if not NULL */
	  if (op->pred[0] != NULL)
	    {
	      C_add_reg_usage (fn_scope_reg_usage, op->pred[0]);
	      if (global_reg_usage)
		C_add_reg_usage (global_reg_usage, op->pred[0]);
	    }


	  /* If tracing promoted predicates, add pred[1], if not NULL */
	  if (C_trace_promoted_preds && (op->pred[1] != NULL))
	    {
	      C_add_reg_usage (fn_scope_reg_usage, op->pred[1]);
	      if (global_reg_usage)
		C_add_reg_usage (global_reg_usage, op->pred[1]);
	    }

	  /* Scan every dest in op */
	  for (index = 0; index < L_max_dest_operand; index++)
	    {
	      /* Add dest[index], if not NULL */
	      if (op->dest[index] != NULL)
		{
		  C_add_reg_usage (fn_scope_reg_usage, op->dest[index]);
		  if (global_reg_usage)
		    C_add_reg_usage (global_reg_usage, op->dest[index]);
		}
	    }

	  /* Scan every src in op */
	  for (index = 0; index < L_max_src_operand; index++)
	    {
	      /* Add src[index], if not NULL */
	      if (op->src[index] != NULL)
		{
		  C_add_reg_usage (fn_scope_reg_usage, op->src[index]);
		  if (global_reg_usage)
		    C_add_reg_usage (global_reg_usage, op->src[index]);
		}
	    }
	}
    }

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

      for (loop = int_base; loop < int_base + int_num; loop++)
	{
	  C_add_reg_id_usage (fn_scope_reg_usage, loop,
			      C_native_machine_ctype);
	  /*
	     fprintf(stderr,"C_add_fn_reg_usage: alloced int %d\n", loop);
	   */
	  if (global_reg_usage)
	    C_add_reg_id_usage (global_reg_usage, loop,
				C_native_machine_ctype);
	}
      for (loop = flt_base; loop < flt_base + flt_num; loop++)
	{
	  C_add_reg_id_usage (fn_scope_reg_usage, loop, L_CTYPE_FLOAT);
	  if (global_reg_usage)
	    C_add_reg_id_usage (global_reg_usage, loop, L_CTYPE_FLOAT);
	}
      for (loop = dbl_base; loop < dbl_base + dbl_num; loop++)
	{
	  C_add_reg_id_usage (fn_scope_reg_usage, loop, L_CTYPE_DOUBLE);
	  if (global_reg_usage)
	    C_add_reg_id_usage (global_reg_usage, loop, L_CTYPE_DOUBLE);
	}
      for (loop = pred_base; loop < pred_base + pred_num; loop++)
	{
	  C_add_reg_id_usage (fn_scope_reg_usage, loop, L_CTYPE_PREDICATE);
	  if (global_reg_usage)
	    C_add_reg_id_usage (global_reg_usage, loop, L_CTYPE_PREDICATE);
	}
    }
}

/* Creates and initializes a MemUsage structure and returns
 * it to the caller.
 */
Mem_Usage *
C_new_mem_usage (char *name)
{
  Mem_Usage *mem_usage;

  /* Malloc the mem_usage structure */
  if ((mem_usage = (Mem_Usage *) malloc (sizeof (Mem_Usage))) == NULL)
    L_punt ("C_new_mem_usage: Out of memory");

  /* Set the structure name */
  if ((mem_usage->name = strdup (name)) == NULL)
    L_punt ("C_new_mem_usage: Out of memory");

  /* Initialize IMPACT data state */
  mem_usage->data_label_name = NULL;
  mem_usage->data_label_global = 0;
  mem_usage->data_label_align = -1;
  mem_usage->data_label_element_size = -1;
  mem_usage->data_label_decl[0] = 0;
  mem_usage->data_label_cast[0] = 0;

  mem_usage->incoming_parm_size = 0;
  mem_usage->outgoing_parm_size = 0;
  mem_usage->reg_swap_size = 0;
  mem_usage->local_var_size = 0;
  mem_usage->alloc_size = 0;

  /* Create each string symbol table with the default size */
  mem_usage->file_scope_data_labels_defined =
    STRING_new_symbol_table ("file_scope_data_labels_defined", 0);
  mem_usage->file_scope_data_labels_used =
    STRING_new_symbol_table ("file_scope_data_labels_used", 0);
  mem_usage->file_scope_code_labels_defined =
    STRING_new_symbol_table ("file_scope_code_labels_defined", 0);
  mem_usage->file_scope_code_labels_used =
    STRING_new_symbol_table ("file_scope_code_labels_used", 0);

  mem_usage->program_scope_data_labels_defined =
    STRING_new_symbol_table ("program_scope_data_labels_defined", 0);
  mem_usage->program_scope_data_labels_used =
    STRING_new_symbol_table ("program_scope_data_labels_used", 0);
  mem_usage->program_scope_code_labels_defined =
    STRING_new_symbol_table ("program_scope_code_labels_defined", 0);
  mem_usage->program_scope_code_labels_used =
    STRING_new_symbol_table ("program_scope_code_labels_used", 0);

  mem_usage->struct_names_defined =
    STRING_new_symbol_table ("struct_names_defined", 0);
  mem_usage->union_names_created =
    STRING_new_symbol_table ("union_names_created", 0);
  mem_usage->init_routines_created =
    STRING_new_symbol_table ("init_routines_created", 0);
  /* Return the newly created structure */
  return (mem_usage);
}

/* Initializes program_scope tables (empties them) */
void
C_init_file_scope_mem_usage (Mem_Usage * mem_usage)
{
  STRING_Symbol *symbol, *next_symbol;

  /* 
   * Delete all file scope information 
   */

  for (symbol = mem_usage->file_scope_data_labels_defined->head_symbol;
       symbol != NULL; symbol = next_symbol)
    {
      /* Get next symbol before deleting this one */
      next_symbol = symbol->next_symbol;
      STRING_delete_symbol (symbol, NULL);
    }

  for (symbol = mem_usage->file_scope_data_labels_used->head_symbol;
       symbol != NULL; symbol = next_symbol)
    {
      /* Get next symbol before deleting this one */
      next_symbol = symbol->next_symbol;
      STRING_delete_symbol (symbol, NULL);
    }

  for (symbol = mem_usage->file_scope_code_labels_defined->head_symbol;
       symbol != NULL; symbol = next_symbol)
    {
      /* Get next symbol before deleting this one */
      next_symbol = symbol->next_symbol;
      STRING_delete_symbol (symbol, NULL);
    }

  for (symbol = mem_usage->file_scope_code_labels_used->head_symbol;
       symbol != NULL; symbol = next_symbol)
    {
      /* Get next symbol before deleting this one */
      next_symbol = symbol->next_symbol;
      STRING_delete_symbol (symbol, NULL);
    }
}

/* Frees the passed Mem_Usage structure  */
void
C_delete_mem_usage (Mem_Usage * mem_usage)
{
  /* Free structure name */
  free (mem_usage->name);

  /* Free data_label string, if exists */
  if (mem_usage->data_label_name != NULL)
    free (mem_usage->data_label_name);

  /* Delete all the symbol tables (no data, so pass NULL for free routine) */
  STRING_delete_symbol_table (mem_usage->file_scope_data_labels_defined,
			      NULL);
  STRING_delete_symbol_table (mem_usage->file_scope_data_labels_used, NULL);
  STRING_delete_symbol_table (mem_usage->file_scope_code_labels_defined,
			      NULL);
  STRING_delete_symbol_table (mem_usage->file_scope_code_labels_used, NULL);

  STRING_delete_symbol_table (mem_usage->program_scope_data_labels_defined,
			      NULL);
  STRING_delete_symbol_table (mem_usage->program_scope_data_labels_used,
			      NULL);
  STRING_delete_symbol_table (mem_usage->program_scope_code_labels_defined,
			      NULL);
  STRING_delete_symbol_table (mem_usage->program_scope_code_labels_used,
			      NULL);

  STRING_delete_symbol_table (mem_usage->struct_names_defined, NULL);
  STRING_delete_symbol_table (mem_usage->union_names_created, NULL);
  STRING_delete_symbol_table (mem_usage->init_routines_created, NULL);

  /* Free the overall structure */
  free (mem_usage);
}

/* If have new data label, reset info for this label.  Only one
 * label needs to be kept track of at a time, due to IMPACT's data
 * format.
 */
void
C_reset_data_label_if_necessary (Mem_Usage * mem_usage, char *label)
{
  /* Reset existing info if have new label */
  if ((mem_usage->data_label_name == NULL) ||
      (strcmp (label, mem_usage->data_label_name) != 0))
    {
      /* Free old name, if any */
      if (mem_usage->data_label_name != NULL)
	free (mem_usage->data_label_name);

      /* Get copy of new name */
      if ((mem_usage->data_label_name = strdup (label)) == NULL)
	L_punt ("C_reset_data_label_if_necessary: Out of memory");

      mem_usage->data_label_global = 0;
      mem_usage->data_label_align = -1;
      mem_usage->data_label_element_size = -1;
      mem_usage->data_label_decl[0] = 0;
      mem_usage->data_label_cast[0] = 0;
    }
}

/* Returns a pointer to the last data label that was specified.
 * May return NULL.
 */
char *
C_get_label_name (Mem_Usage * mem_usage)
{
  return (mem_usage->data_label_name);
}


/* Records global specifier info for label.  */
void
C_make_label_global (Mem_Usage * mem_usage, char *label)
{
  /* Reset info if new data label */
  C_reset_data_label_if_necessary (mem_usage, label);

  /* Set global setting */
  mem_usage->data_label_global = 1;
}

/* Is this label global? */
int
C_is_label_global (Mem_Usage * mem_usage, char *label)
{
  /* Reset info if new data label */
  C_reset_data_label_if_necessary (mem_usage, label);

  /* Return global setting */
  return (mem_usage->data_label_global);
}

/* Derives and records the label declaration and cast strings */
void
C_set_label_decl_cast (Mem_Usage * mem_usage, char *label, L_Type * h_type)
{
  char prefix_buf[TYPE_BUF_SIZE];
  char temp_decl_buf[TYPE_BUF_SIZE], temp_cast_buf[TYPE_BUF_SIZE];
  unsigned int basic_type;

  /* Reset info if new data label */
  C_reset_data_label_if_necessary (mem_usage, label);

  /* Sanity check */
  if (h_type == NULL)
    {
      L_punt ("C_set_label_decl_cast: h_type NULL!");
    }

  /* Initialize the prefix buf */
  prefix_buf[0] = 0;

  /* Get bit field describing the basic type */
  basic_type = h_type->type;

  /* Add them to prefix_buf in a reasonable C order */
  if (basic_type & L_DATA_STATIC)
    strcat (prefix_buf, "static ");
  if (basic_type & L_DATA_EXTERN)
    strcat (prefix_buf, "extern ");
  if (basic_type & L_DATA_CONST)
    strcat (prefix_buf, "const ");
  if (basic_type & L_DATA_VOLATILE)
    strcat (prefix_buf, "volatile ");
#if 0
  /* I don't think these apply, just ignore them for now -ITI/JCG 4/99 */
  if (basic_type & L_DATA_NOALIAS)
    strcat (prefix_buf, "noalias ");
  if (basic_type & L_DATA_REGISTER)
    strcat (prefix_buf, "register ");
  if (basic_type & L_DATA_AUTO)
    strcat (prefix_buf, "auto ");
  if (basic_type & L_DATA_GLOBAL)
    strcat (prefix_buf, "global ");
  if (basic_type & L_DATA_PARAMETER)
    strcat (prefix_buf, "parameter ");
  if (basic_type & L_DATA_SIGNED)	/* Default, so omit for clarity */
    strcat (prefix_buf, "signed ");
#endif
  if (basic_type & L_DATA_UNSIGNED)
    strcat (prefix_buf, "unsigned ");
  if (basic_type & L_DATA_VOID)
    strcat (prefix_buf, "void ");
  if (basic_type & L_DATA_CHAR)
    strcat (prefix_buf, "char ");
  if (basic_type & L_DATA_SHORT)
    strcat (prefix_buf, "short ");
  if (basic_type & L_DATA_INT)
    strcat (prefix_buf, "int ");
  if (basic_type & L_DATA_LONG)
    strcat (prefix_buf, "long ");
  if (basic_type & L_DATA_LONGLONG)
    strcat (prefix_buf, "long long ");
  if (basic_type & L_DATA_FLOAT)
    strcat (prefix_buf, "float ");
  if (basic_type & L_DATA_DOUBLE)
    strcat (prefix_buf, "double ");
  if (basic_type & L_DATA_STRUCT)
    {
      strcat (prefix_buf, "struct ");
      strcat (prefix_buf, h_type->struct_name);
      strcat (prefix_buf, " ");
    }
  if (basic_type & L_DATA_UNION)
    {
      strcat (prefix_buf, "union ");
      strcat (prefix_buf, h_type->struct_name);
      strcat (prefix_buf, " ");
    }
  if (basic_type & L_DATA_ENUM)
    {
      strcat (prefix_buf, "enum ");
      strcat (prefix_buf, h_type->struct_name);
      strcat (prefix_buf, " ");
    }

  /* Create the rest of the declaration from the type info (pointers, 
   * arrays, etc.) 
   */
  C_dclptr_to_C_string (temp_decl_buf, h_type->dcltr, label);

  /* Create the label declaration from the prefix and the type info */
  sprintf (mem_usage->data_label_decl, "%s%s", prefix_buf, temp_decl_buf);

  /* Create the rest of the cast from the type info (pointers, 
   * arrays, etc.) by passing "" as the label.
   */
  C_dclptr_to_C_string (temp_cast_buf, h_type->dcltr, "");

  /* Create the cast for this label from the prefix and the type info */
  sprintf (mem_usage->data_label_cast, "%s%s", prefix_buf, temp_cast_buf);

}

/* Returns NULL if no label decl recorded, otherwise returns
 * a pointer to the decl buffer.  Must not free this buffer,
 * and must be used before any other data is processed (since
 * buffer contents will be destroyed).
 */
char *
C_get_label_decl (Mem_Usage * mem_usage, char *label)
{
  /* Reset info if new data label */
  C_reset_data_label_if_necessary (mem_usage, label);

  /* Return NULL if no decl present */
  if (mem_usage->data_label_decl[0] == 0)
    {
      return (NULL);
    }
  /* Otherwise, return decl pointer */
  else
    {
      return (mem_usage->data_label_decl);
    }
}

/* Returns NULL if no label cast recorded, otherwise returns
 * a pointer to the cast buffer.  Must not free this buffer,
 * and must be used before any other data is processed (since
 * buffer contents will be destroyed).
 */
char *
C_get_label_cast (Mem_Usage * mem_usage, char *label)
{
  /* Reset info if new data label */
  C_reset_data_label_if_necessary (mem_usage, label);

  /* Return NULL if no cast present */
  if (mem_usage->data_label_cast[0] == 0)
    {
      return (NULL);
    }
  /* Otherwise, return cast pointer */
  else
    {
      return (mem_usage->data_label_cast);
    }
}

/* Records alignment specifier info for label.  */
void
C_set_label_alignment (Mem_Usage * mem_usage, char *label, int align)
{
  /* Reset info if new data label */
  C_reset_data_label_if_necessary (mem_usage, label);

  /* Set alignment setting */
  mem_usage->data_label_align = align;
}

/* Returns alignment specifier info for label, or -1 if none.  */
int
C_get_label_alignment (Mem_Usage * mem_usage, char *label)
{
  /* Reset info if new data label */
  C_reset_data_label_if_necessary (mem_usage, label);

  /* Return alignment setting */
  return (mem_usage->data_label_align);
}

/* Records element size specifier info for label.  */
void
C_set_label_element_size (Mem_Usage * mem_usage, char *label, int size)
{
  /* Reset info if new data label */
  C_reset_data_label_if_necessary (mem_usage, label);

  /* Set element_size setting */
  mem_usage->data_label_element_size = size;
}

/* Returns element size specifier info for label, or -1 if none.  */
int
C_get_label_element_size (Mem_Usage * mem_usage, char *label)
{
  /* Reset info if new data label */
  C_reset_data_label_if_necessary (mem_usage, label);

  /* Return element_size setting */
  return (mem_usage->data_label_element_size);
}

/* Adds memory usage (if not already present) to the specified table.
 */
void
C_add_mem_usage (STRING_Symbol_Table * table, char *label)
{
  /* Add filtered_label if not already in the table */
  if (!STRING_find_symbol (table, label))
    STRING_add_symbol (table, label, NULL);
}

void
C_add_fn_mem_usage (L_Func * fn, Mem_Usage * mem_usage)
{
  L_Cb *cb;
  L_Oper *op;
  int index;
  char *label;
  int i, max_offset, raw_offset, offset = 0, size;
  L_Attr *tmo_attr, *tmso_attr, *tro_attr, *trsof_attr, *tms_attr;
  L_Attr *param_size_attr;

#ifdef IT64BIT
  int min_offset;
#endif

  /* Initialize stack space counters for this function */
  mem_usage->incoming_parm_size = 0;
  mem_usage->outgoing_parm_size = 0;
  mem_usage->reg_swap_size = 0;
  mem_usage->local_var_size = 0;
  mem_usage->alloc_size = 0;

  /* Parameter passing code uses incoming parameter offsets
   * from both tmo_attr and tmso_attributes.  Set 
   * mem_usage->incoming_parm_size to the maximum of these
   * offsets, since this information does not show up
   * in the defines for structure copies.
   */
  max_offset = 0;
#ifdef IT64BIT
  min_offset = 0;
#endif

  /* Scan function's tmo attribute, if present */
  if ((tmo_attr = L_find_attr (fn->attr, "tmo")) != NULL)
    {
      for (i = 0; i < tmo_attr->max_field; i++)
	{
	  /* Better be an int constant */
	  if (!L_is_int_constant (tmo_attr->field[i]))
	    {
	      fprintf (stderr, "In func '%s':\n", fn->name);
	      L_print_attr (stderr, tmo_attr);
	      L_punt ("C_add_fn_mem_usage: Int const expected for field %i!",
		      i);
	    }

	  /* Get offset */
	  raw_offset = ITicast (tmo_attr->field[i]->value.i);

	  offset = raw_offset;

	  /* Expected to be positive */
	  if (raw_offset < 0)
	    {
	      fprintf (stderr, "In func '%s':\n", fn->name);
	      L_print_attr (stderr, tmo_attr);
	      L_punt ("C_add_fn_mem_usage: field[%i] neg int(%i)!", i,
		      raw_offset);
	    }

	  /* Because of allocation direction, this starting offset
	     does not imply size as it does in IMPACT arch so
	     it must be read from another macro. */
	  if (!(tms_attr = L_find_attr (fn->attr, "tms")))
	    {
	      fprintf (stderr, "In func '%s':\n", fn->name);
	      L_punt ("C_add_fn_mem_usage: tms field expected!");
	    }
	  if (!L_is_int_constant (tms_attr->field[i]))
	    {
	      if (M_arch == M_TAHOE)
		offset += 8;
	      else
		offset += 4;
	    }
	  else
	    {
	      /* Structure */
	      offset += ITicast (tms_attr->field[i]->value.i);
	    }
	  
	  if (offset > max_offset)
	    max_offset = offset;
	}
    }

  /* Scan function's tro attribute, if present */
  if ((tro_attr = L_find_attr (fn->attr, "tro")) != NULL)
    {
      for (i = 0; i < tro_attr->max_field; i++)
	{
	  /* Better be an int constant */
	  if (!L_is_int_constant (tro_attr->field[i]))
	    {
	      /* Actually, when structs are passed this
	         field will be left empty */
	      /*
	         fprintf (stderr, "In func '%s':\n", fn->name);
	         L_print_attr (stderr, tro_attr);
	         L_punt ("C_add_fn_mem_usage: "
	         "Int const expected for field %i!",
	         i);
	       */
	      continue;
	    }

	  /* Get offset */
	  raw_offset = ITicast (tro_attr->field[i]->value.i);

	  if (M_arch == M_IMPACT || M_arch == M_PLAYDOH)
	    {
	      offset = raw_offset + 4;
	    }
	  // CHANGES.JB
	  else if (M_arch == M_TAHOE || M_arch == M_ARM)
	    {

	      /* MCM IA64 tro's are 0 */
	      /* Expected to be positive */

	      offset = raw_offset;

	      if (raw_offset < 0)
		{
		  fprintf (stderr, "In func '%s':\n", fn->name);
		  L_print_attr (stderr, tro_attr);
		  L_punt ("C_add_fn_mem_usage: field[%i] neg int(%i)!", i,
			  raw_offset);
		}
	    }
	  else
	    L_punt ("C_add_fn_mem_usage: Unsupported architecture: %d",
		    M_arch);

	  if (offset > max_offset)
	    max_offset = offset;
	}
    }

  /* Scan function's tmso attribute, if present */
  if ((tmso_attr = L_find_attr (fn->attr, "tmso")) != NULL)
    {
      for (i = 0; i < tmso_attr->max_field; i++)
	{
	  /* Skip NULL fields */
	  if (tmso_attr->field[i] == NULL)
	    continue;

	  /* Make sure int, if present */
	  if (!L_is_int_constant (tmso_attr->field[i]))
	    {
	      fprintf (stderr, "In func '%s':\n", fn->name);
	      L_print_attr (stderr, tmso_attr);
	      L_punt ("C_add_fn_mem_usage: Int const expected for field %i!",
		      i);
	    }

	  /* Get offset */
	  raw_offset = ITicast (tmso_attr->field[i]->value.i);

	  offset = raw_offset;

	  /* Expected to be positive */
	  if (raw_offset < 0)
	    {
	      fprintf (stderr, "In func '%s':\n", fn->name);
	      L_print_attr (stderr, tmso_attr);
	      L_warn ("C_add_fn_mem_usage: field[%i] neg int(%i)!", i,
		      raw_offset);
	    }

          if (M_arch == M_TAHOE)
             offset += 8;
          else
             offset += 4;

	  if (offset > max_offset)
	    max_offset = offset;
#ifdef IT64BIT
	  else if (offset < 0 && offset < min_offset)
	    min_offset = offset;
#endif
	}
    }

#ifdef IT64BIT
  if ((trsof_attr = L_find_attr (fn->attr, "trsof")) != NULL)
    {
      for (i = 0; i < trsof_attr->max_field; i++)
	{
	  /* Better be an int constant */
	  if (!L_is_int_constant (trsof_attr->field[i]))
	    continue;

	  /* Get offset */
	  raw_offset = ITicast (trsof_attr->field[i]->value.i);

	  if (M_arch == M_IMPACT || M_arch == M_PLAYDOH)
	    {
	      L_punt ("C_add_fn_mem_usage: "
		      "trsof attr unexpected in IMPACT arch\n");
	    }
	  else if (M_arch == M_TAHOE || M_arch == M_ARM)
	    {
	      offset = raw_offset;
	    }
	  else
	    L_punt ("C_add_fn_mem_usage: Unsupported architecture: %d",
		    M_arch);

	  if (offset > max_offset)
	    max_offset = offset;
	  else if (offset < 0 && offset < min_offset)
	    min_offset = offset;
	}
    }
#endif

  /* ASSIGNMENT OF MEM_USAGE */
  /* Update incoming_parm_size, if found information */
  if (max_offset > mem_usage->incoming_parm_size)
    mem_usage->incoming_parm_size = max_offset;



  /* Scan every cb in the function */
  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      /* Scan every op in the cb */
      for (op = cb->first_op; op != NULL; op = op->next_op)
	{
	  /* Scan every define (compiler-directive) */
	  if (op->proc_opc == Lop_DEFINE)
	    {
	      /* Expect dest[0] to be a macro register; 
	         otherwise, just ignore the define. */
	      if (L_is_macro (op->dest[0]))
		{
		  /* Handle each macro type appropriately */
		  switch (op->dest[0]->value.mac)
		    {
		    case L_MAC_P0:
		    case L_MAC_P1:
		    case L_MAC_P2:
		    case L_MAC_P3:
		    case L_MAC_P4:
		    case L_MAC_P5:
		    case L_MAC_P6:
		    case L_MAC_P7:
		    case L_MAC_P8:
		    case L_MAC_P15:

		      /* Support for IA-64 */
		    case L_MAC_P9:
		    case L_MAC_P10:
		    case L_MAC_P11:
		    case L_MAC_P12:
		    case L_MAC_P13:
		    case L_MAC_P14:
		    case L_MAC_P16:
		    case L_MAC_P17:
		    case L_MAC_P18:
		    case L_MAC_P19:
		    case L_MAC_P20:
		    case L_MAC_P21:
		    case L_MAC_P22:
		    case L_MAC_P23:
		    case L_MAC_P24:
		    case L_MAC_P25:
		    case L_MAC_P26:
		    case L_MAC_P27:

		    case L_MAC_OP:
		    case L_MAC_IP:
		    case L_MAC_LV:

		      /* Don't need to do anything for parameter registers */
		      break;

		    case L_MAC_RET_TYPE:
		      /* We use call_info attribute instead of this */
		      break;

		    case L_MAC_RETADDR:
                      break; 

                    case L_MAC_TM_TYPE:
		      /* Indicates incoming parameters are passed thru
		       * the stack.  Get the max offset used.
		       */

		      /* Make sure src[0] is MAC_IP! */
		      if ((!L_is_macro (op->src[0])) ||
			  (op->src[0]->value.mac != L_MAC_IP))
			{
			  fprintf (stderr, "In func '%s':\n", fn->name);
			  L_print_oper (stderr, op);
			  L_punt ("C_add_fn_mem_usage: " "src[0] not IP!");
			}

		      /* Make sure src[1] is positive int offset */
		      if ((!L_is_int_constant (op->src[1])) ||
			  (op->src[1]->value.i < 0))
			{
			  fprintf (stderr, "In func '%s':\n", fn->name);
			  L_print_oper (stderr, op);
			  L_punt ("C_add_fn_mem_usage: "
				  "src[1] neg int!");
			}

		      /* Get the parameter's raw offset */
		      raw_offset = ITicast (op->src[1]->value.i);

		      /* Negating this offset, gives the minimum size for
		       * the incoming parameter space.
		       */
		      size = -raw_offset;

		      /* ASSIGNMENT OF MEM_USAGE */
		      /* Update incoming parameter space size, if necessary */
		      if (mem_usage->incoming_parm_size < size)
			mem_usage->incoming_parm_size = size;
		      break;

		    case L_MAC_LOCAL_SIZE:
		      /* Make sure src[0] is non-negative int size */
		      if ((!L_is_int_constant (op->src[0])) ||
			  (op->src[0]->value.i < 0))
			{
			  fprintf (stderr, "In func '%s':\n", fn->name);
			  L_print_oper (stderr, op);
			  L_punt ("C_add_fn_mem_usage: "
				  "src[0] not non-neg int!");
			}

		      /* Make sure local space not already set! */
		      if (mem_usage->local_var_size != 0)
			{
			  fprintf (stderr, "In func '%s':\n", fn->name);
			  L_print_oper (stderr, op);
			  L_punt ("C_add_fn_mem_usage: "
				  "local already %i!",
				  mem_usage->local_var_size);
			}

		      /* ASSIGNMENT OF MEM_USAGE */
		      /* Set the local var size */
		      mem_usage->local_var_size =
			ITicast (op->src[0]->value.i);
		      break;

		    case L_MAC_SWAP_SIZE:
		      /* Make sure src[0] is non-negative int size */
		      if ((!L_is_int_constant (op->src[0])) ||
			  (op->src[0]->value.i < 0))
			{
			  fprintf (stderr, "In func '%s':\n", fn->name);
			  L_print_oper (stderr, op);
			  L_punt ("C_add_fn_mem_usage: "
				  "src[0] not non-neg int!");
			}

		      /* Make sure swap space not already set! */
		      if (mem_usage->reg_swap_size != 0)
			{
			  fprintf (stderr, "In func '%s':\n", fn->name);
			  L_print_oper (stderr, op);
			  L_punt ("C_add_fn_mem_usage: "
				  "swap already %i!",
				  mem_usage->reg_swap_size);
			}

		      /* ASSIGNMENT OF MEM_USAGE */
		      /* Set the reg swap size */
		      mem_usage->reg_swap_size =
			ITicast (op->src[0]->value.i);
		      break;

		    case L_MAC_PARAM_SIZE:
		      /* Make sure src[0] is non-negative int size */
		      if ((!L_is_int_constant (op->src[0])) ||
			  (op->src[0]->value.i < 0))
			{
			  fprintf (stderr, "In func '%s':\n", fn->name);
			  L_print_oper (stderr, op);
			  L_punt ("C_add_fn_mem_usage: "
				  "src[0] not non-neg int!");
			}

		      /* Make sure outgoing parm space not already set! */
		      if (mem_usage->outgoing_parm_size != 0)
			{
			  fprintf (stderr, "In func '%s':\n", fn->name);
			  L_print_oper (stderr, op);
			  L_punt ("C_add_fn_mem_usage: "
				  "outgoing_parm already %i!",
				  mem_usage->outgoing_parm_size);
			}

		      /* ASSIGNMENT OF MEM_USAGE */
		      /* Set the outgoing parm size */
		      mem_usage->outgoing_parm_size =
			ITicast (op->src[0]->value.i);

		      if (M_arch == M_TAHOE)
			{
			  /* IA64 includes 16 byte scratch space */
			  mem_usage->outgoing_parm_size += 16;
			  /* To handle potential func passing by value */
			  mem_usage->outgoing_parm_size += 64;
			}
		      break;

		      /* Make sure we know and handle each type of compiler
		       * directive properly!
		       */
		    default:
		      fprintf (stderr, "In func '%s':\n", fn->name);
		      L_print_oper (stderr, op);
		      L_warn ("C_add_fn_mem_usage: "
			      "I don't know how to handle this define!");

		    }
		}
	    }

	  /* Scan every source operand (labels may not appear
	   * in dest or pred operands)
	   */
	  for (index = 0; index < L_max_src_operand; index++)
	    {
	      /* Is this source operand a label? */
	      if ((op->src[index] != NULL) && L_is_label (op->src[index]))
		{
		  /* Get the label for ease of use */
		  label = op->src[index]->value.l;

		  /* If JSR op or if begins with _$fn_ assume
		   * it is a code label 
		   */
		  if ((op->proc_opc == Lop_JSR) ||
		      (op->proc_opc == Lop_JSR_FS) ||
		      ((label[0] == '_') &&
		       (label[1] == '$') &&
		       (label[2] == 'f') &&
		       (label[3] == 'n') && (label[4] == '_')))
		    {
		      /* ASSIGNMENT OF MEM_USAGE */
		      /* Assume file scope until find out otherwise */
		      C_add_mem_usage (mem_usage->file_scope_code_labels_used,
				       C_true_name (label));
		    }
		  /* Otherwise, assume it is a data label */
		  else
		    {
		      /* ASSIGNMENT OF MEM_USAGE */
		      /* Assume file scope until find out otherwise */
		      C_add_mem_usage (mem_usage->file_scope_data_labels_used,
				       C_true_name (label));
		    }
		}
	    }
	}
    }


  /* ASSIGNMENT OF MEM_USAGE */
  /* This should be sufficient by itself for computing the incoming_parm_size, 
   * but for compatibility with older lcode the other methods are left in use
   */
  if ((param_size_attr = L_find_attr (fn->attr, "formal_param_size")) != NULL)
    {
      ITintmax stat_size = param_size_attr->field[0]->value.i;

      if (M_arch == M_TAHOE)
	stat_size += 16; /* Must include scratch space size */

      if (stat_size < mem_usage->incoming_parm_size)
	L_warn ("Prepared smaller than computed incoing parm size "
		"(function %s) (%d < %d)",
		fn->name, (int) stat_size, 
		(int) mem_usage->incoming_parm_size);	
      else if (stat_size > mem_usage->incoming_parm_size)
	mem_usage->incoming_parm_size = stat_size;
    }


#if 0
  printf ("Func '%s'\n", fn->name);
  printf ("IP     size %d\n", mem_usage->incoming_parm_size);
#ifdef IT64BIT
  if (min_offset < 0)
    printf ("  IP uses %d of LV \n", -1 * min_offset);
#endif
  printf ("LV     size %d\n", mem_usage->local_var_size);
  printf ("OP     size %d\n", mem_usage->outgoing_parm_size);
  printf ("Swap   size %d\n", mem_usage->reg_swap_size);
  printf ("Alloc  size %d\n", mem_usage->alloc_size);
#endif
}

/* Scan data read to build file-scope label usage info.
 * See C_emit_data() for description of each data type.
 * Only need to scan for label usage.  The C_emit_data() routines
 * will record the definition of each label as they are defined.
 */
void
C_add_data_mem_usage (L_Data * data, Mem_Usage * mem_usage)
{
  /* Scan each type of data appropriately */
  switch (data->type)
    {
      /* The following data type do not use any labels directly */
    case L_INPUT_ALIGN:
    case L_INPUT_GLOBAL:
    case L_INPUT_MS:
    case L_INPUT_ELEMENT_SIZE:
    case L_INPUT_DEF_STRUCT:
    case L_INPUT_DEF_UNION:
    case L_INPUT_DEF_ENUM:
    case L_INPUT_FIELD:
    case L_INPUT_ENUMERATOR:
    case L_INPUT_RESERVE:
      break;

      /* Initialization data for arrays and structures. */
    case L_INPUT_WB:
    case L_INPUT_WW:
    case L_INPUT_WI:
    case L_INPUT_WQ:
    case L_INPUT_WF:
    case L_INPUT_WF2:
    case L_INPUT_WS:
      /* Variable declarations with optional initialization. */
    case L_INPUT_BYTE:
    case L_INPUT_WORD:
    case L_INPUT_LONG:
    case L_INPUT_LONGLONG:
    case L_INPUT_FLOAT:
    case L_INPUT_DOUBLE:
      /* If has initialization value, only need to scan label 
       * expressions that are not cb labels (jump table cb labels 
       * are handled with #defines).
       */
      if (data->value)
	{
	  char *label = NULL;

	  if (data->value->type == L_EXPR_LABEL)
	    {
	      label = data->value->value.l;
	    }
	  else if (data->value->type == L_EXPR_ADD &&
		   data->value->A->type == L_EXPR_LABEL)
	    {
	      label = data->value->A->value.l;
	    }

	  if (label && !C_is_cb_label (label))
	    {
	      /* Function labels have _$fn_ prepended to them */
	      if ((label[0] == '_') &&
		  (label[1] == '$') &&
		  (label[2] == 'f') && (label[3] == 'n') && 
		  (label[4] == '_'))
		{
		  /* Assume file scope until find out otherwise */
		  C_add_mem_usage (mem_usage->file_scope_code_labels_used,
				   C_true_name (label));
		}
	      /* Otherwise, assume it is a data label */
	      else
		{
		  /* Assume file scope until find out otherwise */
		  C_add_mem_usage (mem_usage->file_scope_data_labels_used,
				   C_true_name (label));
		}
	    }
	}
      break;

    default:
      L_print_data (stderr, data);
      L_punt ("C_add_data_mem_usage: Unknown type %i!\n", data->type);
      break;
    }

}

/* Deduce program scope labels from those referenced within this file
 * but not defined within this file.
 */
int
C_deduce_program_scope_labels (Mem_Usage * mem_usage)
{
  STRING_Symbol *symbol;
  char *label;

  /* Scan all the data labels used within this file.  For those that 
   * were not defined in this file, add to program-scope labels.
   */
  for (symbol = mem_usage->file_scope_data_labels_used->head_symbol;
       symbol != NULL; symbol = symbol->next_symbol)
    {
      /* Get label for ease of use */
      label = symbol->name;

      /* If label was not defined in this file, make program scope */
      if (STRING_find_symbol (mem_usage->file_scope_data_labels_defined,
			      label) == NULL)
	{
	  C_add_mem_usage (mem_usage->program_scope_data_labels_used, label);
	}
    }

  /* Scan all the code labels used within this file.  For those that 
   * were not defined in this file, add to program-scope labels.
   */
  for (symbol = mem_usage->file_scope_code_labels_used->head_symbol;
       symbol != NULL; symbol = symbol->next_symbol)
    {
      /* Get label for ease of use */
      label = symbol->name;

      /* If label was not defined in this file, make program scope */
      if (STRING_find_symbol (mem_usage->file_scope_code_labels_defined,
			      label) == NULL)
	{
	  C_add_mem_usage (mem_usage->program_scope_code_labels_used, label);
	}
    }
  return 0;
}

/* For the data labels that have not been defined by the
 * code that has been seen, emit minimal externs.  
 * These externs may not be right but will hopefully allow the linker to 
 * find the proper data externally.
 *
 * May be required to enhance Lcode, etc. to provide extra info
 * to make this less of a hack :) -ITI (JCG) 3/99
 */
void
C_emit_undefined_label_extern (FILE * extern_out, Mem_Usage * mem_usage)
{
  STRING_Symbol *symbol;
  char *label;
  int needs_header;

  /* Print out header on first extern */
  needs_header = 1;

  /* Scan all the used data labels.  For those that were not
   * internally defined, print out 'fake' externs.
   */
  for (symbol = mem_usage->program_scope_data_labels_used->head_symbol;
       symbol != NULL; symbol = symbol->next_symbol)
    {
      /* Get label for ease of use */
      label = symbol->name;

      /* Skip labels that were defined */
      if (STRING_find_symbol (mem_usage->program_scope_data_labels_defined,
			      label) != NULL)
	continue;

      /* Emit header, if first fake extern */
      if (needs_header)
	{
	  fprintf (extern_out,
		   "\n"
		   "/* Externs for referenced but undefined variables "
		   "(bogus type used) */\n");
	  needs_header = 0;
	}

      /* Print out these externs as char *, for lack of better idea.
       * The C generated from Lcode should only be using the address anyway
       */
      fprintf (extern_out, "extern char *%s;\n", label);
    }
}

void
C_emit_reg_array_decl (FILE * out, INT_Symbol_Table * table,
		       char *line_prefix, char *var_prefix)
{
  INT_Symbol *symbol;
  int min_id, max_id;

  /* If the table is empty, do nothing */
  if (table->head_symbol == NULL)
    return;

  /* Initialize min/max id */
  min_id = table->head_symbol->value;
  max_id = min_id;

  /* Go through table to determine min and max id */
  for (symbol = table->head_symbol; symbol != NULL;
       symbol = symbol->next_symbol)
    {
      if (symbol->value > max_id)
	max_id = symbol->value;

      if (symbol->value < min_id)
	min_id = symbol->value;
    }

  /* Sanity check, don't allow negative or huge ids */
  if (min_id < 0)
    {
      L_punt ("C_emit_reg_array_decl: negative ids (%i) not currently "
	      "supported!", min_id);
    }

  if (max_id > 1000000)
    {
      L_punt ("C_emit_reg_array_decl: huge ids (%i) not currently "
	      "supported!", max_id);
    }

  /* Declare array (max_id + 1, to make [max_id] valid) */
  fprintf (out, "%s %s[%i];\n", line_prefix, var_prefix, (max_id + 1));
}

void
C_emit_specific_reg_decls (FILE * out, INT_Symbol_Table * table,
			   char *line_prefix, char *var_prefix,
			   char *var_postfix, int is_macro)
{
  INT_Symbol *symbol;
  int cur_width, max_width, name_len, line_prefix_len;
  char name_buf[256];

  /* Aim for 78 character max_width */
  max_width = 78;

  /* If the table is empty, do nothing */
  if (table->head_symbol == NULL)
    return;

  /* Initially, have not print anything */
  cur_width = 0;

  /* Get line_prefix length for ease of use */
  line_prefix_len = strlen (line_prefix);

  /* Go through table, declaring each operand */
  for (symbol = table->head_symbol; symbol != NULL;
       symbol = symbol->next_symbol)
    {
      /* Create variable name */
      if (!is_macro)
	{
	  sprintf (name_buf, "%s%i%s", var_prefix, symbol->value,
		   var_postfix);
	}
      else
	{
	  sprintf (name_buf, "%s%s%s", var_prefix,
		   C_macro_name (symbol->value), var_postfix);
	}

      /* Get the length of the new name */
      name_len = strlen (name_buf);

      /* Do special processing if first item, or will exceed max_width */
      if ((cur_width == 0) || ((cur_width + name_len + 3) > max_width))
	{
	  /* Terminate this line if printing with exceed max_width */
	  if (cur_width != 0)
	    {
	      fprintf (out, ";\n");
	      cur_width = 0;
	    }

	  /* Print out start of new line */
	  fprintf (out, "%s%s", line_prefix, name_buf);
	  cur_width = line_prefix_len + name_len;
	}
      /* Otherwise, add var_name to existing list */
      else
	{
	  fprintf (out, ", %s", name_buf);
	  cur_width += name_len + 2;
	}
    }

  /* Terminate list, if necesary */
  if (cur_width != 0)
    {
      fprintf (out, ";\n");
      cur_width = 0;
    }
}

void
C_emit_reg_decls (FILE * out, Reg_Usage * reg_usage, char *passed_line_prefix)
{
  char line_prefix[256], var_prefix[256], var_postfix[256];
  char *comment_prefix, *ptr;

  /* Dup passed_line_prefix */
  if ((comment_prefix = strdup (passed_line_prefix)) == NULL)
    L_punt ("C_emit_reg_decls: Out of memory!");

  /* and truncate at first non-space, to derive comment prefix */
  for (ptr = comment_prefix; *ptr != 0; ptr++)
    {
      if (*ptr != ' ')
	{
	  *ptr = 0;
	  break;
	}
    }

  /* Emit pred normal register declaration, if any */
  if (reg_usage->pred_regs_used->head_symbol != NULL)
    {
      fprintf (out, "\n%s/* Function-scope pred register declarations*/\n",
	       comment_prefix);
      sprintf (line_prefix, "%s%s ", passed_line_prefix,
	       C_native_machine_ctype_str);
      if (C_use_register_arrays)
	{
	  sprintf (var_prefix, "%sr_p", C_prefix);
	  C_emit_reg_array_decl (out, reg_usage->pred_regs_used,
				 line_prefix, var_prefix);

	}
      else
	{
	  sprintf (var_prefix, "%sr_", C_prefix);
	  sprintf (var_postfix, "_p");
	  C_emit_specific_reg_decls (out, reg_usage->pred_regs_used,
				     line_prefix, var_prefix, var_postfix, 0);
	}
    }

  /* Emit pred macro register declaration, if any */
  if (reg_usage->pred_macs_used->head_symbol != NULL)
    {
      fprintf (out, "\n%s/* Function-scope pred macro declarations*/\n",
	       comment_prefix);
      sprintf (line_prefix, "%s%s ", passed_line_prefix,
	       C_native_machine_ctype_str);
      if (C_use_register_arrays)
	{
	  sprintf (var_prefix, "%smac_p", C_prefix);
	  C_emit_reg_array_decl (out, reg_usage->pred_macs_used,
				 line_prefix, var_prefix);
	}
      else
	{
	  sprintf (var_prefix, "%smac_", C_prefix);
	  sprintf (var_postfix, "_p");
	  C_emit_specific_reg_decls (out, reg_usage->pred_macs_used,
				     line_prefix, var_prefix, var_postfix, 1);
	}
    }

  /* Emit int normal register declaration, if any */
  if (reg_usage->int_regs_used->head_symbol != NULL)
    {
      fprintf (out, "\n%s/* Function-scope int register declarations*/\n",
	       comment_prefix);
      sprintf (line_prefix, "%s%s ", passed_line_prefix,
	       C_native_machine_ctype_str);
      if (C_use_register_arrays)
	{
	  sprintf (var_prefix, "%sr_i", C_prefix);
	  C_emit_reg_array_decl (out, reg_usage->int_regs_used,
				 line_prefix, var_prefix);

	}
      else
	{
	  sprintf (var_prefix, "%sr_", C_prefix);
	  sprintf (var_postfix, "_i");
	  C_emit_specific_reg_decls (out, reg_usage->int_regs_used,
				     line_prefix, var_prefix, var_postfix, 0);
	}
    }

  /* Emit int macro register declaration, if any */
  if (reg_usage->int_macs_used->head_symbol != NULL)
    {
      fprintf (out, "\n%s/* Function-scope macro register declarations*/\n",
	       comment_prefix);
      sprintf (line_prefix, "%s%s ", passed_line_prefix,
	       C_native_machine_ctype_str);
      if (C_use_register_arrays)
	{
	  sprintf (var_prefix, "%smac_i", C_prefix);
	  C_emit_reg_array_decl (out, reg_usage->int_macs_used,
				 line_prefix, var_prefix);
	}
      else
	{
	  sprintf (var_prefix, "%smac_", C_prefix);
	  sprintf (var_postfix, "_i");
	  C_emit_specific_reg_decls (out, reg_usage->int_macs_used,
				     line_prefix, var_prefix, var_postfix, 1);
	}
    }

  /* Emit float normal register declaration, if any */
  if (reg_usage->float_regs_used->head_symbol != NULL)
    {
      fprintf (out, "\n%s/* Function-scope float register declarations*/\n",
	       comment_prefix);
      sprintf (line_prefix, "%sfloat ", passed_line_prefix);
      if (C_use_register_arrays)
	{
	  sprintf (var_prefix, "%sr_f", C_prefix);
	  C_emit_reg_array_decl (out, reg_usage->float_regs_used,
				 line_prefix, var_prefix);

	}
      else
	{
	  sprintf (var_prefix, "%sr_", C_prefix);
	  sprintf (var_postfix, "_f");
	  C_emit_specific_reg_decls (out, reg_usage->float_regs_used,
				     line_prefix, var_prefix, var_postfix, 0);
	}
    }

  /* Emit float macro register declaration, if any */
  if (reg_usage->float_macs_used->head_symbol != NULL)
    {
      fprintf (out, "\n%s/* Function-scope macro register declarations*/\n",
	       comment_prefix);
      sprintf (line_prefix, "%sfloat ", passed_line_prefix);
      if (C_use_register_arrays)
	{
	  sprintf (var_prefix, "%smac_f", C_prefix);
	  C_emit_reg_array_decl (out, reg_usage->float_macs_used,
				 line_prefix, var_prefix);
	}
      else
	{
	  sprintf (var_prefix, "%smac_", C_prefix);
	  sprintf (var_postfix, "_f");
	  C_emit_specific_reg_decls (out, reg_usage->float_macs_used,
				     line_prefix, var_prefix, var_postfix, 1);
	}
    }

  /* Emit double normal register declaration, if any */
  if (reg_usage->double_regs_used->head_symbol != NULL)
    {
      fprintf (out, "\n%s/* Function-scope double register declarations*/\n",
	       comment_prefix);
      sprintf (line_prefix, "%sdouble ", passed_line_prefix);
      if (C_use_register_arrays)
	{
	  sprintf (var_prefix, "%sr_f2", C_prefix);
	  C_emit_reg_array_decl (out, reg_usage->double_regs_used,
				 line_prefix, var_prefix);
	}
      else
	{
	  sprintf (var_prefix, "%sr_", C_prefix);
	  sprintf (var_postfix, "_f2");
	  C_emit_specific_reg_decls (out, reg_usage->double_regs_used,
				     line_prefix, var_prefix, var_postfix, 0);
	}
    }

  /* Emit double macro register declaration, if any */
  if (reg_usage->double_macs_used->head_symbol != NULL)
    {
      fprintf (out, "\n%s/* Function-scope macro register declarations*/\n",
	       comment_prefix);
      sprintf (line_prefix, "%sdouble ", passed_line_prefix);
      if (C_use_register_arrays)
	{
	  sprintf (var_prefix, "%smac_f2", C_prefix);
	  C_emit_reg_array_decl (out, reg_usage->double_macs_used,
				 line_prefix, var_prefix);
	}
      else
	{
	  sprintf (var_prefix, "%smac_", C_prefix);
	  sprintf (var_postfix, "_f2");
	  C_emit_specific_reg_decls (out, reg_usage->double_macs_used,
				     line_prefix, var_prefix, var_postfix, 1);
	}
    }

  fprintf (out, "\tint rr_temp_i;\n");

  /* KVM : Emit the register for carry bit.
   */
  fprintf(out, "\tint l_emul_carry_bit;\n");

  /* KVM : Emit the union to pass long longs as parms.
   */
  fprintf(out, "\tunion l_emul_llong_struct { long long x; struct {int lo, hi;} parts;} *l_emul_llong_buf;\n");
  /* Free allocated comment prefix */
  free (comment_prefix);
}

/* Emit the C function declaration */
void
C_emit_fn_declaration (FILE * out, L_Func * fn, Mem_Usage * mem_usage)
{
  char return_type_buf[TYPE_BUF_SIZE], all_parm_type_buf[TYPE_BUF_SIZE];
  char raw_buf[TYPE_BUF_SIZE], name_buf[TYPE_BUF_SIZE];
  /* Make these next buffers extra large since will need to hold
   * extra formatting, etc.
   */
  char formatted_buf[TYPE_BUF_SIZE + 10000];
  char krc_buf[TYPE_BUF_SIZE + 10000];
  char main_buf[TYPE_BUF_SIZE + 10000];
  char *parse_ptr;
  int old_style_param;
  int append_gcc_ellipsis;
  int index;

  /* Get the return type and parameter specifiers from the function 
   * attribute.  Use smallest buffers for these, since buf size is checked.
   */
  L_get_call_info (fn, NULL, fn->attr, return_type_buf, all_parm_type_buf,
		   sizeof (return_type_buf));

  /* If the original source was compiled as Ansi-C but used old style
   * parameter declarations (usually for old-style varargs), the
   * function will be marked with a attribute "old_style_param".
   * If the attribute is present, print out in K&R-c style so
   * old-style varargs will work in Ansi-C mode.
   */

  old_style_param = 
    (L_find_attr (fn->attr, "old_style_param") != NULL);

  /* If gcc is the host compiler and the source used old-style varargs,
   * the frontend removed the gcc added '...' after the last varargs
   * decl (i.e. long va_list; ...) and added the attribute 
   * "append_gcc_ellipsis".  If the attribute is present, we need to add 
   * this ... back so gcc will handle the stack fixup properly for varargs.
   */

  append_gcc_ellipsis = 
    (L_find_attr (fn->attr, "append_gcc_ellipsis") != NULL);

  /* Print function name and later parameters to formatted_buf, return 
   * type will be wrapped around everything (necessary for returning 
   * function pointers) at the end.
   */
  if (!C_matches_true_name (fn->name, "main"))
    sprintf (formatted_buf, "%s (", C_true_name (fn->name));
  else
    sprintf (formatted_buf, "%sreal_main (", C_prefix);

  /* Start parsing the parm_type_buf */
  parse_ptr = all_parm_type_buf;

  /* Initialize krc_buf (will hold krc style parm declarators) */
  krc_buf[0] = 0;

  /* Handle each parameter type specifier */
  index = 1;
  while (*parse_ptr != 0)
    {
      /* Get the next parameter type */
      L_get_next_param_type (raw_buf, &parse_ptr);

      /* Make up a name for the parameter */
      sprintf (name_buf, "%sp%i", C_prefix, index - 1);

      if (append_gcc_ellipsis && (*parse_ptr == '\0') && 
	  (M_arch == M_TAHOE))
	{
	  strcpy (raw_buf, "longlong");
	}

      /* Convert type to formatted string using C conventions,
       * using name_bud as the 'parameter'
       */
      L_convert_type_to_C_format (main_buf, raw_buf, name_buf);

      /* For Ansi-C, everything goes on first line */
      if (C_ansi_c_mode && !old_style_param)
	{
	  /* Add to end of formatted string */
	  strcat (formatted_buf, main_buf);
	}

      /* For K&R-C, put parm name on first line and real declarations
       * afterward (krc_buf).
       */
      else
	{
	  /* Add parm name only to formatted string */
	  strcat (formatted_buf, name_buf);

	  /* Add real declaration to end of krc_buf, with indent */
	  strcat (krc_buf, "  ");
	  strcat (krc_buf, main_buf);

	  /* If flagged that we need to add '...' after last parm and
	   * it is the last parm, add the ... before the newline.
	   */
	  if (append_gcc_ellipsis && (*parse_ptr == 0))
	    {
	      /* Add '; ...\n' */
	      strcat (krc_buf, "; ...\n");
	    }
	  else
	    {
	      /* Add ';\n' */
	      strcat (krc_buf, ";\n");
	    }
	}

      /* Increment parameter id */
      index++;

      /* Add comma if not at end */
      if (*parse_ptr != 0)
	strcat (formatted_buf, ", ");
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
  printf("%s\n", main_buf);
  L_convert_type_to_C_format (formatted_buf, return_type_buf, main_buf);
  printf("%s\n", return_type_buf);
  /* Add blank line before function declaration */
  fprintf (out, "\n");

  /* Emit comment to allow easy searching for function start */
  fprintf (out, "/* Begin %s */\n", C_true_name (fn->name));

  {
    L_Attr *attr;
    if ((attr = L_find_attr (fn->attr, "Cattr")))
      {
	int i;
	L_Operand *opd;
	for (i = 0; i < attr->max_field; i++)
	  {
	    if ((opd = attr->field[i]) && L_is_string (opd))
	      {
		char *val, *buf = strdup (opd->value.s);
		/* 10/25/04 REK Commenting out unused variable to quiet
		 *              compiler warning. */
#if 0
		char attr_buf[128];
#endif
		if (buf[0] == '\"')
		  {
		    val = buf + 1;
		    val[strlen(val) -1] = '\0';
		  }
		else
		  {
		    val = buf;
		  }
		fprintf (out, "__attribute__ ((%s)) ", val);
		free (buf);
	      }
	  }
      }
  }

  /* If function is not globally visable, print out 'static'. */
  if (!C_is_label_global (mem_usage, C_true_name (fn->name)))
    {
      fprintf (out, "static ");

      /* Add this function to file_scope_code_labels_defined */
      C_add_mem_usage (mem_usage->file_scope_code_labels_defined,
		       C_true_name (fn->name));
    }
  else
    {
      /* Add this function to program_scope_code_labels_defined */
      C_add_mem_usage (mem_usage->program_scope_code_labels_defined,
		       C_true_name (fn->name));
    }

  /* Print out function declaration (parm names only for K&R-c) */
  fprintf (out, "%s\n", formatted_buf);

  /* If K&R C, print out real type declarations for each parameter */
  if (!C_ansi_c_mode || old_style_param)
    {
      fprintf (out, "%s", krc_buf);
    }
}

/* Build string mapping table for the function and return pointer
 * to the table.  Start numbering at start_id.  -JCG 2/00
 */
STRING_Symbol_Table *
C_build_string_map (L_Func * fn, int start_id)
{
  L_Cb *cb;
  L_Oper *op;
  int index, id;
  char *string;
  STRING_Symbol_Table *string_map;

  /* Sanity check, start_id must be > 0 (so can use 0 as error
   * code and negative numbers do not work in variable names)
   */
  if (start_id <= 0)
    {
      L_punt ("C_build_string_map: start_id (%i) must be > 0!", start_id);
    }

  /* Create string mapping table */
  string_map = STRING_new_symbol_table ("string_map", 0);

  /* Start numbering strings at start_id */
  id = start_id;

  /* Scan every cb in the function */
  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      /* Scan every op in the cb */
      for (op = cb->first_op; op != NULL; op = op->next_op)
	{
	  /* Scan every source operand (strings may not appear
	   * in dest or pred operations
	   */
	  for (index = 0; index < L_max_src_operand; index++)
	    {
	      /* Is this source operand a string? */
	      if ((op->src[index] != NULL) && L_is_string (op->src[index]))
		{
		  /* Get the string for ease of use */
		  string = op->src[index]->value.s;

		  /* Add string to string mapping if not already present */
		  if (STRING_find_symbol (string_map, string) == NULL)
		    {
		      /* Use void * to store int mapping id value */
#if LP64_ARCHITECTURE
		      STRING_add_symbol (string_map, string,
					 (void *)((long)id));
#else
		      STRING_add_symbol (string_map, string, (void *) id);
#endif
		      id++;
		    }
		}
	    }
	}
    }

  return (string_map);
}

/* Emit string declarations for the function.  The IMPACT optimizer assumes
 * that all duplicate string literials in the function will end up
 * to the same memory location.  Since only some host compilers generate
 * code consistent with this assumption, do the duplicate string
 * compaction manually. -JCG 2/00
 */
void
C_emit_string_decls (FILE * out, STRING_Symbol_Table * string_map)
{
  STRING_Symbol *symbol;
  char name_buf[256];

  /* If the string map table is empty, do nothing */
  if (string_map->head_symbol == NULL)
    return;

  /* Print out what these declarations are for */
  fprintf (out, "\n%s/* String literals declared in the function */\n",
	   C_indent);

  /* Go through table, declaring each operand */
  for (symbol = string_map->head_symbol; symbol != NULL;
       symbol = symbol->next_symbol)
    {
      /* Create string pointer name from 'id' stored in symbol data pointer */
#if LP64_ARCHITECTURE
      sprintf (name_buf, "%sString_%li", C_prefix, (long) symbol->data);
#else
      sprintf (name_buf, "%sString_%i", C_prefix, (int) symbol->data);
#endif

      /* Create exactly one declaration for each unique string */
      fprintf (out, "%sstatic char *%s = %s;\n", C_indent,
	       name_buf, symbol->name);
    }
}


/* This helper function finds a unique union name
 * and declares it (in extern_out) on first use so that it enforces the 
 * size and alignment requirements.
 *
 * For example, if the data charateristics are align 4, but size 6, emit:
 *
 * union {char d[6]; int a;} union_name;
 *
 * This name will be reused for all other calls with the 
 * same size and alignment requirements.
 * 
 * Returns the union name to use, do not free or modify this name!
 */
char *
C_generate_union_name (FILE * extern_out, int data_size, int data_align,
		       Mem_Usage * mem_usage)
{
  char name_buf[1000];
  char *data_align_type;
  int index, name_found;
  STRING_Symbol *name_symbol;

  /* Sanity check, don't expec C_layout_database to be NULL */
  if (C_layout_database == NULL)
    {
      L_punt ("C_generate_union_name: C_layout_database == NULL!");
    }

  /* Keep creating union names until find one that
   * was not a user-defined type.
   */
  name_found = 0;
  for (index = 0; (index < 100) && !name_found; index++)
    {
      /* Create union name from size and alignment info,
       * if first time thru loop.
       */
      if (index == 0)
	{
	  sprintf (name_buf, "s%i_a%i", data_size, data_align);
	}
      /* Otherwise, tack _(index) on end of desired name */
      else
	{
	  sprintf (name_buf, "s%i_a%i_%i", data_size, data_align, index);
	}

      /* If name not in layout database (i.e., user type), 
       * we can safely use it.
       */
      if (MD_find_section (C_layout_database, name_buf) == NULL)
	{
	  name_found = 1;
	}
    }

  /* Make sure found name! */
  if (!name_found)
    {
      L_punt ("C_generate_union_name: Unable to generate unique name\n"
	      "Last try:'%s'", name_buf);
    }

  /* Determine if we have previously declared this union name */
  name_symbol = STRING_find_symbol (mem_usage->union_names_created, name_buf);

  /* If we have not, declare it to extern_out and add to table */
  if (name_symbol == NULL)
    {
      /* Add structure name to table, no data necessary */
      name_symbol = STRING_add_symbol (mem_usage->union_names_created,
				       name_buf, NULL);

      /* Get the C type that yields this alignment */
      data_align_type = C_get_align_C_type (data_align);

      /* Declare structure in extern_out */
      fprintf (extern_out, "union %s {char s[%i]; %s a;};\n",
	       name_buf, data_size, data_align_type);
    }

  /* Return pointer to name stored in symbol table.
   * Do not free or modify this name.
   */
  return (name_symbol->name);
}


/* Convert the the data field/var dclptr into C format.
 * Do this recursively and piece by piece, since the ordering
 * of the types makes a non-recursive routine difficult to write.
 *
 * In the following field decl, this will be initially passed a dclptr pointer
 * to (a (i 10) with an incoming_buf of "j" (the int return type
 * is handled by the caller):
 *
 *   (field j (int) (a (i 10)) (p) (f) (p) (a (i 9)))
 *
 * After all the recursion, dest_buf will contain:
 *   (*(*j[10])())[9];
 * 
 * Based on the same code I wrote to parse call_info strings in
 * Lbuild_prototype_info/l_build_prototype_info.c (L_subtype_to_C_string) -JCG
 *
 * NOTE: This functions parameters MUST NOT overlap (be the same buffer)!
 */
void
C_dclptr_to_C_string (char *dest_buf, L_Dcltr * dclptr, char *incoming_buf)
{
  char sub_buf[TYPE_BUF_SIZE];

  /* Terminate dest_buf initially */
  dest_buf[0] = 0;

  /* If nothing, just copy the incoming buf to the outgoing buf */
  if (dclptr == NULL)
    {
      strcat (dest_buf, incoming_buf);
    }
  /* Handle an array type */
  else if (dclptr->method == L_D_ARRY)
    {
      /* If have array with no size info, wrap (* ) around whatever
       * has already been parsed or the field/var name
       */
      if (dclptr->index == NULL)
	{
	  sprintf (sub_buf, "(*%s)", incoming_buf);
	}

      /* Otherwise, add [size] to end */
      else
	{
	  sprintf (sub_buf, "%s[%i]", incoming_buf,
		   C_get_data_int (dclptr->index));
	}

      /* Convert the rest of the dclptr recursively */
      C_dclptr_to_C_string (dest_buf, dclptr->next, sub_buf);
    }

  /* Handle an pointer followed by an array type. */
  else if ((dclptr->method == L_D_PTR) &&
	   (dclptr->next != NULL) && (dclptr->next->method == L_D_ARRY))
    {
      /* Wrap (* ) around whatever has already been parsed
       * or the field/var name.
       */
      sprintf (sub_buf, "(*%s)", incoming_buf);

      /* Convert the rest of the dclptr recursively */
      C_dclptr_to_C_string (dest_buf, dclptr->next, sub_buf);
    }

  /* Handle function pointers (consume the optional leading pointer decl,
   * if present)
   */
  else if ((dclptr->method == L_D_FUNC) ||
	   ((dclptr->method == L_D_PTR) &&
	    (dclptr->next != NULL) && (dclptr->next->method == L_D_FUNC)))
    {
      /* Consume optional leading pointer decl */
      if (dclptr->method == L_D_PTR)
	dclptr = dclptr->next;

      /* Wrap field/var name (or previously parsed type) in function
       * pointer specification
       */
      sprintf (sub_buf, "(*%s)()", incoming_buf);

      /* Convert the rest of the dclptr recursively */
      C_dclptr_to_C_string (dest_buf, dclptr->next, sub_buf);
    }

  /* Handle normal pointers */
  else if (dclptr->method == L_D_PTR)
    {
      /* Add * to front of incoming_buf */
      sprintf (sub_buf, "*%s", incoming_buf);

      /* Convert the rest of the dclptr recursively */
      C_dclptr_to_C_string (dest_buf, dclptr->next, sub_buf);
    }

  /* Otherwise, punt since I don't know what this is */
  else
    {
      L_punt ("C_dclptr_to_C_string: unexected modifier '%i' at '%s'",
	      dclptr->method, incoming_buf);
    }
}

/* Emits a field declaration for a struct or union based on the
 * info in 'data'.  This is basically an encoding of Pcode's
 * type info into a format originally intended for source-level debugging.
 *
 * Uses the recursive C_dclptr_to_C_string () to translate:
 *   (field j (int) (a (i 10)) (p) (f) (p) (a (i 9)))
 * into:
 *   int (*(*j[10])())[9];
 * 
 * See C_dclptr_to_C_string () for details.
 */
void
C_emit_field_decl (FILE * out, L_Data * data)
{
  char temp_buf[TYPE_BUF_SIZE];
  unsigned int field_type;

  /* Sanity check, better be field declaration */
  if (data->type != L_INPUT_FIELD)
    {
      L_print_data (stderr, data);
      L_punt ("C_emit_field_decl: field type expected!");
    }

  /* Sanity check, better have a field name */
  if (data->address == NULL)
    {
      L_print_data (stderr, data);
      L_punt ("C_emit_field_decl: address expected!");
    }

  /* Sanity check, I believe the h_type better not be NULL */
  if (data->h_type == NULL)
    {
      L_print_data (stderr, data);
      L_punt ("C_emit_field_decl: h_type != NULL expected!");
    }

  /* Print out indent */
  fprintf (out, "%s", C_indent);

  /* Get bit field descriping field type */
  field_type = data->h_type->type;

  /* Print them out in a reasonable C order */
  if (field_type & L_DATA_STATIC)
    fprintf (out, "static ");
  if (field_type & L_DATA_EXTERN)
    fprintf (out, "extern ");
  if (field_type & L_DATA_CONST)
    fprintf (out, "const ");
  if (field_type & L_DATA_VOLATILE)
    fprintf (out, "volatile ");
#if 0
  /* I don't think these apply, just ignore them for now -ITI/JCG 4/99 */
  if (field_type & L_DATA_NOALIAS)
    fprintf (out, "noalias ");
  if (field_type & L_DATA_REGISTER)
    fprintf (out, "register ");
  if (field_type & L_DATA_AUTO)
    fprintf (out, "auto ");
  if (field_type & L_DATA_GLOBAL)
    fprintf (out, "global ");
  if (field_type & L_DATA_PARAMETER)
    fprintf (out, "parameter ");
  if (field_type & L_DATA_SIGNED)	/* Default, so omit for clarity */
    fprintf (out, "signed ");
#endif
  if (field_type & L_DATA_UNSIGNED)
    fprintf (out, "unsigned ");
  if (field_type & L_DATA_VOID)
    fprintf (out, "void ");
  if (field_type & L_DATA_CHAR)
    fprintf (out, "char ");
  if (field_type & L_DATA_SHORT)
    fprintf (out, "short ");
  if (field_type & L_DATA_INT)
    fprintf (out, "int ");
  if (field_type & L_DATA_LONG)
    fprintf (out, "long ");
  if (field_type & L_DATA_LONGLONG)
    fprintf (out, "long long ");
  if (field_type & L_DATA_FLOAT)
    fprintf (out, "float ");
  if (field_type & L_DATA_DOUBLE)
    fprintf (out, "double ");
  if (field_type & L_DATA_STRUCT)
    fprintf (out, "struct %s ", data->h_type->struct_name);
  if (field_type & L_DATA_UNION)
    fprintf (out, "union %s ", data->h_type->struct_name);
  if (field_type & L_DATA_ENUM)
    fprintf (out, "enum %s ", data->h_type->struct_name);

  /* Handle the rest of the type info (pointers, arrays, etc.).
   * Use raw label because the data->address does not get '_' prepended.
   */
  C_dclptr_to_C_string (temp_buf, data->h_type->dcltr,
			C_get_raw_data_label (data->address));

  /* Print out appropriately modified field name */
  fprintf (out, "%s", temp_buf);

  /* Add bit field specifier, if present */
  if (data->value != NULL)
    {
      fprintf (out, ":%i", C_get_data_int (data->value));
    }

#if IT64BIT
  if (strcmp ("__jmpbuf", C_get_raw_data_label (data->address)) == 0)
    {
      fprintf (out, " __attribute__ ((aligned (16))) ");
    }
#endif

  /* Finished with field declaration */
  fprintf (out, ";\n");
}

/* Print out struct/union definition in the form:
 * struct name
 * {
 *    type field1;
 *    ...
 *    type fieldN;
 * };
 *
 * This uses the source-level debug information passed down to Lcode
 * to regenerate the struct/union exactly.  Expects all the
 * fields for the struct/union to follow the def_data.
 * Uses C_emit_field_decl () to print out each field declaration;
 */
int
C_emit_def_struct_union (FILE * out, L_Data * def_data,
			 Mem_Usage * mem_usage, int is_union,
			 L_Datalist_Element ** jump_tbl_data)
{
  int token_type;
  L_Data *field_data;

  /* Print out 'union' or 'struct' */
  if (is_union)
    {
      fprintf (out, "union ");
    }
  else
    {
      fprintf (out, "struct ");
    }

  /* Print out name of union or struct and opening '{' 
   * Use raw label because the def_data->address does not get '_' prepended.
   */
  fprintf (out, "%s", C_get_raw_data_label (def_data->address));

  if (C_peek_input (jump_tbl_data) != L_INPUT_FIELD)
    {
      fprintf (out, ";\n\n");
      return 0;
    }
  else
    {
      fprintf (out, "\n{\n");
    }

  /* Print out struct/union fields as long as have field decls */
  while (C_peek_input (jump_tbl_data) == L_INPUT_FIELD)
    {
      /* Get this field (instead of just peeking at it) */
      token_type = C_get_input (jump_tbl_data, mem_usage);

      /* Get initializer data */
      field_data = C_data;

      /* Print out the field declaration */
      C_emit_field_decl (out, field_data);

      /* Free the data we just read in (if jump_tbl_data == NULL).
       * Otherwise, this data (from a jump table) will be freed when
       * the function is freed.
       */
      if (jump_tbl_data == NULL)
	{
	  L_delete_data (field_data);
	}
    }

  /* Finish declaration */
  fprintf (out, "};\n" "\n");
  return 1;
}

/* Print out enum definition in the form:
 * enum name
 * {
 *    enumerator1=value1,
 *    ...
 *    enumeratorN=valueN
 * };
 *
 * Enums are automatically removed by both the EDG front end and Hcode
 * flattening, so you normally will not see them unless you are doing
 * something unusual.  This function is primarily for documentation
 * purposes.
 */
int
C_emit_def_enum (FILE * out, L_Data * def_data,
		 Mem_Usage * mem_usage, L_Datalist_Element ** jump_tbl_data)
{
  int token_type;
  L_Data *enumerator_data;

  /* Print out name of enum and opening '{' 
   * Use raw label because the def_data->address does not get '_' prepended.
   */
  fprintf (out, "enum %s\n" "{\n", C_get_raw_data_label (def_data->address));

  /* Print out enumerators as long as have enumerator decls */
  while (C_peek_input (jump_tbl_data) == L_INPUT_ENUMERATOR)
    {

      /* Get this enumerator (instead of just peeking at it) */
      token_type = C_get_input (jump_tbl_data, mem_usage);

      /* Get initializer data */
      enumerator_data = C_data;

      /* Print out the enumerator declaration, expect
       * the name to be in address (use raw label) and the value
       * to be in value (as an int).
       */
      fprintf (out, "%s%s=%i", C_indent,
	       C_get_raw_data_label (enumerator_data->address),
	       C_get_data_int (enumerator_data->value));

      /* Free the data we just read in (if jump_tbl_data == NULL).
       * Otherwise, this data (from a jump table) will be freed when
       * the function is freed.
       */
      if (jump_tbl_data == NULL)
	{
	  L_delete_data (enumerator_data);
	}

      /* If not last enumerator, print out comma before newline */
      if (C_peek_input (jump_tbl_data) == L_INPUT_ENUMERATOR)
	{
	  fprintf (out, ",");
	}

      /* Goto next line */
      fprintf (out, "\n");
    }

  /* Finish declaration */
  fprintf (out, "};\n" "\n");
  return 1;
}


/* Emits prototypes for non-trapping load support */
void
C_emit_non_trapping_load_prototypes (FILE * out)
{
  if (C_ansi_c_mode)
    {
      fprintf (out,
	       "\n"
	       "/* Non-trapping load emulation prototypes */\n"
	       "extern char _EM_NTload_char (char *ptr);\n"
	       "extern unsigned char _EM_NTload_uchar (unsigned char *ptr);\n"
	       "extern short _EM_NTload_short (short *ptr);\n"
	       "extern unsigned short _EM_NTload_ushort (unsigned short *ptr);\n"
	       "extern int _EM_NTload_int (int *ptr);\n"
	       "extern unsigned int _EM_NTload_uint (unsigned int *ptr);\n"
	       "extern long _EM_NTload_long (long *ptr);\n"
	       "extern longlong _EM_NTload_longlong (longlong *ptr);\n"
	       "extern float _EM_NTload_float (float *ptr);\n"
	       "extern double _EM_NTload_double (double *ptr);\n"
	       "extern int _EM_INSTALL_TRAP_HANDLER;\n" "\n");

      if (M_arch == M_TAHOE)
	fprintf (out, "extern unsigned int _EM_NTload_uint (unsigned int *ptr);\n");
      else
	fprintf (out, "#define _EM_NTload_uint _EM_NTload_int\n");
    }
  else
    {
      fprintf (out,
	       "\n"
	       "/* Non-trapping load emulation prototypes */\n"
	       "extern char _EM_NTload_char ();\n"
	       "extern unsigned char _EM_NTload_uchar ();\n"
	       "extern short _EM_NTload_short ();\n"
	       "extern unsigned short _EM_NTload_ushort ();\n"
	       "extern int _EM_NTload_int ();\n"
	       "extern long _EM_NTload_long ();\n"
	       "extern longlong _EM_NTload_longlong ();\n"
	       "extern float _EM_NTload_float ();\n"
	       "extern double _EM_NTload_double ();\n"
	       "extern int _EM_INSTALL_TRAP_HANDLER;\n" "\n");

      if (M_arch == M_TAHOE)
	fprintf (out, "extern unsigned int _EM_NTload_uint ();\n");
      else
	fprintf (out, "#define _EM_NTload_uint _EM_NTload_int\n");

    }
}

/* Emits typedefs for 64-bit support */
void
C_emit_typedefs (FILE * out)
{

  fprintf (out,
	   "\n"
	   "#ifdef _WIN32\n"
	   "typedef __int64 longlong;\n"
           "typedef unsigned long long __uint64;\n"
	   "typedef __uint64 ulonglong;\n"
	   "#else\n"
	   "typedef long long longlong;\n"
	   "typedef unsigned long long ulonglong;\n" 
           "#endif\n" "\n");
}

/* Emits the actual def for a struct or union using the layout info database. 
 * See C_emit_database_struct_union_defs() for details.
 */
void
C_emit_database_struct_union_def (FILE * struct_out, char *struct_name,
				  int is_union)
{
  MD_Section *section;
  MD_Entry *entry;
  char *field_name, *decl;

  /* Get the struct or union section of host layout info database */
  if ((section = MD_find_section (C_layout_database, struct_name)) == NULL)
    {
      L_punt ("C_emit_database_struct_union_def: section '%s' "
	      "unexpected not found!", struct_name);
    }

  /* 
   * Start the declaration 
   */

  /* Print out 'union' or 'struct' */
  if (is_union && struct_out)
    {
      fprintf (struct_out, "union ");
    }
  else
    {
      fprintf (struct_out, "struct ");
    }

  /* Print out name of union or struct and opening '{' */
  if (struct_out)
    fprintf (struct_out, "%s\n" "{\n", struct_name);


  /* Declare every field in the structure/union, using the decl field */
  for (entry = MD_first_entry (section); entry != NULL;
       entry = MD_next_entry (entry))
    {
      /* Get field name for ease of use */
      field_name = entry->name;

      /* Get the field declaration (must not free decl string!) */
      decl = C_read_database_s (struct_name, field_name, "decl");

      /* Print out the field declaration, indented */
      if (struct_out)
	fprintf (struct_out, "%s%s;\n", C_indent, decl);
    }

  /* Finish declaration */
  if (struct_out)
    fprintf (struct_out, "};\n" "\n");
}

/* Emit the actual definitions for all the structures and unions found
 * in the program, if they have not already been emitted using the
 * information found in Lcode (now the default cases -ITI/JCG 4/99).  
 * 
 * In most cases, this routine will emit nothing at all.  It
 * is purely to handle the case where the users has turned off
 * the parameter emit_data_type_info in HtoL for some reason.
 * Uses information provided by the layout_info database file 
 * generated by gen_CtoP.
 *
 * Note: Although using abstract definitions for structures
 *  (ignoring the actual fields, but getting the size and alignment
 *  right) works, it causes compatiblity problems with mixing
 *  IMPACT compiled code and natively compiled code.  The problem
 *  that caused the switch is that HP's cc passes by value the
 *  structure struct S1 {char c; double d} differently than
 *  struct S1 {union {char s[16]; double a}}, even though the
 *  size and alignment information is the same.
 */
void
C_emit_database_struct_union_defs (FILE * struct_out, Mem_Usage * mem_usage)
{
  MD_Section *section;
  MD_Entry *entry;
  char *name;
  int is_union;

  /* Get user types section of host layout info database */
  section = MD_find_section (C_layout_database, "_HT__user_types");

  /* If doesn't exist, there are no user types */
  if (section == NULL)
    return;

  /* Print out every structure or union type defined by the program */
  for (entry = MD_first_entry (section); entry != NULL;
       entry = MD_next_entry (entry))
    {
      /* Get name of the structure or union for ease of use */
      name = entry->name;

      /* Do nothing if this structure or union already been defined using
       * Lcode data type info. -ITI/JCG 4/99
       */
      if (STRING_find_symbol (mem_usage->struct_names_defined, name) != NULL)
	continue;

      /* Get whether it is a union or not. */
      is_union = C_read_database_i ("_HT__user_types", name, "union");

      /* Print out struct/union definition */
      C_emit_database_struct_union_def (struct_out, name, is_union);
    }
}
