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
 *
 *  File:  l_mcode.c
 *
 *  Description: Mcode specific interface routines to support easier 
 *     creation/modification of Lcode/Mcode operations.
 *
 *  Creation Date :  February 2, 1993
 *
 *  Author:  Roger A. Bringmann, Scott Mahlke, Wen-mei Hwu
 *
 *  Revisions:
 *
 *
\*****************************************************************************/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>

int L_debug_messages = 0;
int L_debug_memory_usage = 0;
int L_codegen_phase = 7;
int L_do_machine_opt = 1;
int L_do_software_pipelining = 0;
int L_do_prepass_sched = 1;
int L_do_register_allocation = 1;
int L_do_postpass_code_annotation = 1;
int L_do_peephole_opt = 1;
int L_do_postpass_sched = 1;
int L_print_mcode_phase_3 = 0;
int L_loop_unrolled = 1;
int L_do_recovery_code = 1;
int L_do_super_speculation = 0;


/*****************************************************************************\
 *
 * 
 *
\*****************************************************************************/

void
L_mcode_init_function ()
{
  if (L_fn == NULL)
    L_punt ("L_mcode_init_function: no function defined!");

  if (L_fn->n_parent_oper != 0)
    L_punt ("L_mcode_init_function: function already initialized!");

  L_fn->n_parent_oper = L_fn->n_oper;
  L_fn->n_oper = 0;
  L_fn->max_oper_id = 0;

  /* Delete all Lcode opers from the hash table */
  L_oper_hash_tbl_delete_all (L_fn->oper_hash_tbl);
}

L_Oper *
L_convert_to_parent (L_Cb * cb, L_Oper * oper)
{
  L_Oper *next;

  if (oper == NULL)
    L_punt ("L_convert_to_parent: NULL oper!");

  if (L_EXTRACT_BIT_VAL (oper->flags, L_OPER_PARENT))
    L_punt ("L_convert_to_parent: oper is already a parent!");

  /* Set parent flag */
  oper->flags = L_SET_BIT_FLAG (oper->flags, L_OPER_PARENT);

  next = oper->next_op;

  L_remove_oper (cb, oper);

  /*
     * Insert into parent linked list to ensure appropriate freeing
     * of memory when function complete
   */
  oper->next_op = L_fn->last_parent_op;
  L_fn->last_parent_op = oper;

  return (next);
}

/*****************************************************************************\
 *
 * Mcode specific attributes
 *
\*****************************************************************************/

/*
 * Search for the specified (name) attribute.  
 * Return the pointer if it exists.
 */
#if 0
L_Attr *
L_attr_defined (L_Oper * oper, char *name)
{
  L_Attr *ptr;
  for (ptr = oper->attr; ptr != 0; ptr = ptr->next_attr)
    if (!strcmp (ptr->name, name))
      return ptr;
  return 0;
}
#endif

/*
 * Return the attribute field value for the specified operation
 * 0 is returned if there is no attribute specified.
 */
void
L_get_attribute (L_Oper * oper, int *attr)
{
  L_Attr *ptr;

  if ((ptr = L_find_attr (oper->attr, "pext")) != 0)
    *attr = (int) ptr->field[0]->value.i;
  else
    *attr = 0;
}

/*
 * Set/change the Mcode attribute for machine specific attributes.
 *
 * Valid values are greater than 1.  Each code generator must define
 * what attributes to standard Lcode instructions exist.
 *
 * Example:  AMD29000 has subtract and subtract reverse.  An attribute
 * field would tell the code generator to use subtract reverse for
 * the specific opcode.
 */
void
L_set_attribute (L_Oper * oper, int attr)
{
  L_Attr *ptr;

  if ((ptr = L_find_attr (oper->attr, "pext")) != 0)
    {
      ptr->field[0]->value.i = (ITintmax) attr;
    }
  else
    {
      ptr = L_new_attr ("pext", 1);
      L_set_int_attr_field (ptr, 0, attr);
      oper->attr = L_concat_attr (oper->attr, ptr);
    }
}

/*
 * Return the preload field value for the specified mcode operation
 * 0 is returned if there is no preload specified.
 */
void
L_get_preload (L_Oper * oper, int *pre)
{
  L_Attr *ptr;

  if ((ptr = L_find_attr (oper->attr, "pre")) != 0)
    *pre = (int) ptr->field[0]->value.i;
  else
    *pre = 0;
}

/*
 * Set/change the Mcode attribute for constant preload to specified value.
 * 
 * This field specifies if this operation is a plausible candidate for
 * constant preloading.  You do not need to worry if the instruction
 * is in a loop.  Register allocation will only look at those instructions
 * that are in loops.
 *
 * The only valid value so far is L_PRELOAD defined in l_code.h
 */
void
L_set_preload (L_Oper * oper, int pre)
{
  L_Attr *ptr;

  if ((ptr = L_find_attr (oper->attr, "pre")) != 0)
    {
      ptr->field[0]->value.i = (ITintmax) pre;
    }
  else
    {
      ptr = L_new_attr ("pre", 1);
      L_set_int_attr_field (ptr, 0, pre);
      oper->attr = L_concat_attr (oper->attr, ptr);
    }
}

/*****************************************************************************\
 *
 * Routines used to indicate the type of delay slot present
 *
\*****************************************************************************/

/* This routine assumes that the instruction copied is a parent Lcode oper */
L_Oper *
L_copy_parent_oper (L_Oper * oper)
{
  L_Oper *new_op;
  L_Attr *attr;
  int i;

  L_fn->max_oper_id++;
  new_op = L_new_oper (L_fn->max_oper_id);
  new_op->opc = oper->opc;
  new_op->proc_opc = oper->proc_opc;
  new_op->opcode = oper->opcode;
  new_op->flags = oper->flags;
  new_op->parent_op = oper->parent_op;
  new_op->weight = oper->weight;

  for (i = 0; i < L_MAX_CMPLTR; i++)
    new_op->com[i] = oper->com[i];

  /* Copy the operands */
  for (i = 0; i < L_max_dest_operand; i++)
    {
      if (oper->dest[i] != NULL)
        {
          new_op->dest[i] = L_copy_operand (oper->dest[i]);
        }
    }

  for (i = 0; i < L_max_src_operand; i++)
    {
      if (oper->src[i] != NULL)
        {
          new_op->src[i] = L_copy_operand (oper->src[i]);
        }
    }

  for (i = 0; i < L_max_pred_operand; i++)
    {
      if (oper->pred[i] != NULL)
        {
          new_op->pred[i] = L_copy_operand (oper->pred[i]);
        }
    }

  /* Copy attribute field */
  attr = L_copy_attr (oper->attr);
  new_op->attr = L_concat_attr (new_op->attr, attr);

  new_op->parent_op = oper;

  return new_op;
}

void
L_read_parm_mcode (Parm_Parse_Info * ppi)
{
  L_read_parm_b (ppi, "debug_messages", &L_debug_messages);
  L_read_parm_b (ppi, "print_lcode", &L_print_parent_op);
  L_read_parm_b (ppi, "debug_memory_usage", &L_debug_memory_usage);
  L_read_parm_b (ppi, "print_mcode_phase_3", &L_print_mcode_phase_3);
  L_read_parm_i (ppi, "phase", &L_codegen_phase);
  /* Commented out by REH, 12/09/94 
     L_read_parm_i(ppi, "loop_unrolled", &L_loop_unrolled);
   */
  L_read_parm_b (ppi, "do_machine_opt", &L_do_machine_opt);
  L_read_parm_b (ppi, "do_software_pipelining", &L_do_software_pipelining);
  L_read_parm_b (ppi, "do_prepass_sched", &L_do_prepass_sched);
  L_read_parm_b (ppi, "do_register_allocation", &L_do_register_allocation);
  L_read_parm_b (ppi, "do_postpass_code_annotation",
                 &L_do_postpass_code_annotation);
  L_read_parm_b (ppi, "do_peephole_opt", &L_do_peephole_opt);
  L_read_parm_b (ppi, "do_postpass_sched", &L_do_postpass_sched);
  L_read_parm_b (ppi, "do_register_allocation", &L_do_register_allocation);
  L_read_parm_b (ppi, "do_recovery_code", &L_do_recovery_code);
  L_read_parm_b (ppi, "do_super_speculation", &L_do_super_speculation);
}
