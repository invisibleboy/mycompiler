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
/*****************************************************************************
 * phase1_func.c                                                             *
 * ------------------------------------------------------------------------- *
 * Lcode -> Mcode annotation                                                 *
 *                                                                           *
 * AUTHORS: D.A. Connors, J. Pierce, J.W. Sias                               *
 *****************************************************************************/
/* 09/11/02 REK Updating this file to use the new TAHOEops and opcode map.
 *              Modifying the following functions: Ltahoe_maintain_UNAT,
 *              Ltahoe_spill_reg_params, L_annotate_prologue,
 *              L_annotate_epilogue, Ltahoe_int_constant_load,
 *              L_annotate_extend, L_annotate_int_logic,
 *              L_annotate_int_move, L_annotate_int_add, L_annotate_int_sub,
 *              L_annotate_abs, L_annotate_extract_oper,
 *              L_annotate_deposit_oper, Ltahoe_annotate_shift,
 *              L_annotate_alloca, L_annotate_jsr,
 *              Ltahoe_pred_init, L_eff_addr_calc,
 *              L_annotate_ld, L_annotate_st, L_annotate_float_divide,
 *              L_annotate_double_divide, L_annotate_float_mul_oper,
 *              L_annotate_double_mul_oper, L_annotate_add_sub_oper,
 *              L_annotate_double_abs, L_annotate_float_mul_add_sub_oper,
 *              L_annotate_double_max_min_oper, L_annotate_float_move,
 *              L_annotate_double_move, L_annotate_float_oper,
 *              L_annotate_double_oper, L_annotate_int_to_float,
 *              L_annotate_int_to_double, L_annotate_float_to_int,
 *              L_annotate_double_to_int,
 *              L_annotate_float_to_double_conversion,
 *              L_annotate_double_to_float_conversion,
 *              L_annotate_nop, L_convert_to_tahoe_oper, L_cleanup_after_mopti.
 */

/* 10/29/02 REK Adding config.h */
#include <config.h>

#include "ltahoe_main.h"
#include "ltahoe_op_query.h"
#include "ltahoe_completers.h"
#include <Lcode/l_promotion.h>
#include <Lcode/mia_opti.h>
#include <Lcode/l_opti_functions.h>
#include <Lcode/l_softpipe.h>
#include <Lcode/l_loop_prep.h>
#include <Lcode/sm.h>
#include <Lcode/l_opti.h>
#include "phase1_bitopt.h"
#include "phase1_func.h"
#include "phase1_param.h"
#include "phase1_varargs.h"
#include "phase1_opgen.h"
#include "phase2_func.h"
#include "phase2_reg.h"

extern void Ltahoe_customize_lcode_compares (L_Func * fn);
static int Ltahoe_lcode_peephole (L_Func * fn);
extern int Ltahoe_reduce (L_Func *fn);
extern void LB_hb_pred_merging (L_Func * fn);

static L_Operand *pfs_save_operand;
static L_Operand *retaddr_save_operand;

#define Ltahoe_sign_extend(o,s,d,z) Ltahoe_extend(1,o,s,d,z)
#define Ltahoe_zero_extend(o,s,d,z) Ltahoe_extend(0,o,s,d,z)

/* Ltahoe_extend
 * ---------------------------------------------------------------------------
 * Make a sign/zero (sz=1/0) extension oper, using oper and with
 * copies of specified src and dest.  Extend from a size bytes value.
 */
L_Oper *
Ltahoe_extend (int sz,
	       L_Oper * oper, L_Operand * src, L_Operand * dest, int size)
{
  L_Oper *sxt_oper;
  int lop, tahoeop;

  switch (size)
    {
    case 1:
      lop = sz ? Lop_SXT_C : Lop_ZXT_C;
      tahoeop = sz ? TAHOEop_SXT1 : TAHOEop_ZXT1;
      break;
    case 2:
      lop = sz ? Lop_SXT_C2 : Lop_ZXT_C2;
      tahoeop = sz ? TAHOEop_SXT2 : TAHOEop_ZXT2;
      break;
    case 4:
      lop = sz ? Lop_SXT_I : Lop_ZXT_I;
      tahoeop = sz ? TAHOEop_SXT4 : TAHOEop_ZXT4;
      break;
    default:
      L_punt ("Ltahoe_extend: Unknown s/z extend: op %d", oper->id);
      return NULL;
    }				/* switch */

  sxt_oper = L_create_new_op_using (lop, oper);
  sxt_oper->proc_opc = tahoeop;

  sxt_oper->flags = L_CLR_BIT_FLAG (sxt_oper->flags,
				    L_OPER_MASK_PE | L_OPER_LABEL_REFERENCE);

  Ltahoe_copy_or_new (sxt_oper->src[0], src);
  Ltahoe_copy_or_new (sxt_oper->dest[0], dest);

  return (sxt_oper);
}				/* Ltahoe_extend */


static int
Ltahoe_should_swap_operands (L_Operand * operand1, L_Operand * operand2)
{
  /* if src1 = reg/mac and  src2 != reg/mac then swap */

  if (L_is_variable (operand1) && !L_is_variable (operand2))
    return (TRUE);

  /* if src1 = label and  src2 = int  then swap  */
  if (L_is_label (operand1) && L_is_int_constant (operand2))
    return (TRUE);

  /* if src1 int > src2 int then swap */
  if (L_is_int_constant (operand1) && L_is_int_constant (operand2) &&
      (operand1->value.i > operand2->value.i))
    return (TRUE);

  return (FALSE);
}				/* Ltahoe_should_swap_operands */


/* Stacked register support
 * ---------------------------------------------------------------------------
 * Manage the stacked registers using fn attrs and the alloc instruction
 */
static int
L_get_num_reg_stack_input_regs (L_Func * fn)
{
  L_Attr *attr;
  int number_inputs = 0;

  if ((attr = L_find_attr (fn->attr, "ip")))
    number_inputs = L_get_int_attr_field (attr, 0);
  else
    L_punt ("L_get_num_reg_stack_input_regs: Missing ip attr");

  return (number_inputs);
}				/* L_get_num_reg_stack_input_regs */

/* JWS BUG: should not report output regs for builtin functions */
static int
L_get_num_reg_stack_output_regs (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *oper;
  L_Attr *attr;
  int number, max_number = 0;

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      for (oper = cb->first_op; oper; oper = oper->next_op)
	{
	  if (!(L_subroutine_call_opcode (oper)))
	    continue;

	  if (L_is_label (oper->src[0]) &&
	      (!strncmp (oper->src[0]->value.l, "___builtin_", 11) ||
	       !strncmp (oper->src[0]->value.l, "_$fn___builtin_", 15)))
	    continue;

	  if (!(attr = L_find_attr (oper->attr, "op")))
	    L_punt ("L_get_num_reg_stack_output_regs: "
		    "missing required \"op\" attribute");

	  number = L_get_int_attr_field (attr, 0);
	  if (number > max_number)
	    max_number = number;
	}			/* for oper */
    }				/* for cb */
  return (max_number);
}				/* L_get_num_reg_stack_output_regs */


/****************************************************************************
 *
 * routine: L_get_reg_stack_info
 * purpose: Finds the number of stack registers
 * input: function and int pointers for each stack reg type
 * output: 
 * returns: TRUE of alloc found, FALSE if alloc not found
 * modified:
 * note:
 *-------------------------------------------------------------------------*/

int
L_get_reg_stack_info (L_Func * fn, int *inputs, int *locals, int *outputs,
		      int *rots)
{
  L_Oper *oper;
  L_Cb *cb;

  cb = fn->first_cb;
  for (oper = cb->first_op; oper; oper = oper->next_op)
    {
      if (oper->opc == Lop_ALLOC)
	{
	  if (inputs)
	    *inputs = ITicast (oper->src[0]->value.i);
	  if (locals)
	    *locals = ITicast (oper->src[1]->value.i);
	  if (outputs)
	    *outputs = ITicast (oper->src[2]->value.i);
	  if (rots)
	    *rots = ITicast (oper->src[3]->value.i);
	  return (1);
	}			/* if */
    }				/* for oper */

  if (inputs)
    *inputs = 0;
  if (locals)
    *locals = 0;
  if (outputs)
    *outputs = 0;
  if (rots)
    *rots = 0;
  return (0);
}				/* L_get_reg_stack_info */


/****************************************************************************
 *
 * routine: L_update_local_space_size()
 * purpose: Change the size of the local variable stack space.  Two items need
 *          to be updated: fn->s_local and the define oper for 
 *          L_MAC_LOCAL_SIZE.
 * input: fn - function to alter.
 *        new_value - the new size of the local variables on the stack.
 * output: 
 * returns:
 * modified: Bob McGowan - 4/97 - update fn->s_local too.
 * note:
 *-------------------------------------------------------------------------*/

void
L_update_local_space_size (L_Func * fn, int new_value)
{
  L_Oper *oper;

  fn->s_local = new_value;
  for (oper = fn->first_cb->first_op; oper; oper = oper->next_op)
    {
      if ((oper->opc == Lop_DEFINE) &&
	  (L_is_macro (oper->dest[0])) &&
	  (oper->dest[0]->value.mac == L_MAC_LOCAL_SIZE))
	{
	  oper->src[0]->value.i = (ITintmax) new_value;
	  return;
	}			/* if */
    }				/* for oper */
  L_punt ("L_update_local_space_size:  local space not defined in %s\n",
	  fn->name);
}				/* L_update_local_space_size */


/* Determines if function is a leaf (has no callees), and marks it accordingly.
 */

static int
L_leaf_func (L_Func * fn)
{
  L_Oper *oper;
  L_Cb *cb;
  int leaf;

  leaf = TRUE;
  for (cb = fn->first_cb; cb && leaf; cb = cb->next_cb)
    {
      for (oper = cb->first_op; oper; oper = oper->next_op)
	{
	  if ((oper->opc == Lop_JSR) ||
	      (oper->opc == Lop_JSR_FS) || (oper->opc == Lop_JSR_ND))
	    {
	      if (L_is_label (oper->src[0]))
		{
		  if (!strcmp (oper->src[0]->value.l, "_$fn_alloca") ||
		      !strcmp (oper->src[0]->value.l, "_$fn_abs"))
		    continue;
		}		/* if */

	      leaf = FALSE;
	      break;
	    }			/* if */
	}			/* for oper */
    }				/* for cb */

  if (leaf == TRUE)
    fn->flags = L_SET_BIT_FLAG (fn->flags, L_FUNC_LEAF);

  if ((leaf == FALSE) && L_EXTRACT_BIT_VAL (fn->flags, L_FUNC_LEAF))
    L_punt ("L_leaf_func: leaf flag set but there is a jsr");
  else if ((leaf == TRUE) && !L_EXTRACT_BIT_VAL (fn->flags, L_FUNC_LEAF))
    L_punt ("L_leaf_func: leaf flag not set and no jsr in fucntion");

  return (leaf);
}				/* L_leaf_func */


/*****************************************************************************\
 *
 * PROLOGUE/EPILOGUE ANNOTATION
 *
\*****************************************************************************/


static int
L_scan_for_IMPACT_alloc (L_Func * fn)
{
  int size_of_alloc, remainder;
  L_Oper *oper;
  L_Cb *cb;

  size_of_alloc = 0;

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    for (oper = cb->first_op; oper; oper = oper->next_op)
      if (oper->opc == Lop_ALLOC)
	{
	  size_of_alloc += ITicast (oper->src[0]->value.i);

	  if ((remainder = size_of_alloc % 8) != 0)
	    size_of_alloc += remainder;
	}			/* if */
  return size_of_alloc;
}				/* L_scan_for_IMPACT_alloc */


/*! \brief Fixes references to unallocated output registers
 *
 * \param fn
 *  the function to process.
 * \param num_output_regs
 *  the number of output registers allocated for \a fn.
 *
 * After builtin functions are annotated, things that were once jsrs are
 * no longer.  This means the number of output registers allocated for
 * a function may be reduced.  However, the builtin annotation may
 * still refer to these output registers.  This function finds references
 * to unallocated output registers and changed these to general registers.
 */
static void
L_fix_unallocated_output_regs (L_Func * fn, int num_output_regs)
{
  L_Cb *cb;
  L_Oper *oper;
  L_Operand *operand;
  /* An array of general register operands that will replace the unallocated
   * output register. */
  L_Operand *gen_regs[MAX_INT_OUTPUT_REGS] = {0};
  ITint32 unalloc_start, unalloc_end;
  int i;

  /* If all output registers are allocated, this function has nothing to do. */
  if (num_output_regs == MAX_INT_OUTPUT_REGS)
    return;

  unalloc_start = L_MAC_P8 + num_output_regs;
  unalloc_end = L_MAC_P15;

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    for (oper = cb->first_op; oper; oper = oper->next_op)
      {
	for (i = 0; i < L_max_dest_operand; i++)
	  if ((operand = oper->dest[i]) && \
	      operand->type == L_OPERAND_MACRO && \
	      operand->value.mac >= unalloc_start && \
	      operand->value.mac <= unalloc_end)
	    {
	      int offset = operand->value.mac - L_MAC_P8;
	      
	      if (gen_regs[offset] == NULL)
		gen_regs[offset] = L_new_register_operand (++fn->max_reg_id,
							   operand->ctype, 0);

	      L_delete_operand (operand);
	      oper->dest[i] = L_copy_operand (gen_regs[offset]);
	    }
	
	for (i = 0; i < L_max_src_operand; i++)
	  if ((operand = oper->src[i]) && operand->type == L_OPERAND_MACRO && \
	      operand->value.mac >= unalloc_start && \
	      operand->value.mac <= unalloc_end)
	    {
	      int offset = operand->value.mac - L_MAC_P8;

	      if (gen_regs[offset] == NULL)
		gen_regs[offset] = L_new_register_operand (++fn->max_reg_id,
							   operand->ctype, 0);

	      L_delete_operand (operand);
	      oper->src[i] = L_copy_operand (gen_regs[offset]);
	    }
      }

  for (i = 0; i < MAX_INT_OUTPUT_REGS; i++)
    if (gen_regs[i])
      L_delete_operand (gen_regs[i]);

  return;
}

static void
L_update_alloc (L_Func * fn)
{
  L_Oper *oper;
  int num_output_regs = 0;

  for (oper = fn->first_cb->first_op; oper; oper = oper->next_op)
    if (oper->opc == Lop_ALLOC)
      {
	oper->src[0]->value.i =
	  (ITintmax) L_get_num_reg_stack_input_regs (fn);
	oper->src[1]->value.i = (ITintmax) 0;
	num_output_regs = L_get_num_reg_stack_output_regs (fn);
	oper->src[2]->value.i = (ITintmax) num_output_regs;
	  (ITintmax) L_get_num_reg_stack_output_regs (fn);
	oper->src[3]->value.i = (ITintmax) 0;
	break;
      }				/* if */

  /* 11/03/04 REK It is possible that we have annotated builtin functions
   *              by this point.  These would have used output registers
   *              when they appeared as jsrs, and probably still do.
   *              Because there is no longer a jsr, they would not be counted
   *              as needing an output register, so the annotation may refer
   *              to an output register that is not allocated.  We need
   *              to make a pass through the Lcode to change these unallocated
   *              output registers to general registers. */
  L_fix_unallocated_output_regs (fn, num_output_regs);

  return;
}				/* L_update_alloc */


void
Ltahoe_scan_prologue_defines (L_Func * fn, int *leaf, int *alloc_size)
{
  L_Oper *oper;
  int leaf_found = FALSE, mem_alloc_found = FALSE;

  for (oper = fn->first_cb->first_op; oper; oper = oper->next_op)
    {
      if ((oper->opc == Lop_DEFINE) &&
	  (oper->dest[0]->type == L_OPERAND_MACRO))
	{
	  switch (oper->dest[0]->value.mac)
	    {
	    case TAHOE_MAC_LEAF:
	      *leaf = ITicast (oper->src[0]->value.i);
	      leaf_found = TRUE;
	      break;
	    case TAHOE_MAC_MEM_ALLOC:
	      *alloc_size = ITicast (oper->src[0]->value.i);
	      mem_alloc_found = TRUE;
	      break;
	    }			/* switch */
	}			/* if */
      if (oper->opc == Lop_PROLOGUE)
	{
	  if (!leaf_found)
	    fprintf (stderr, "Leaf define not found\n");
	  if (!mem_alloc_found)
	    fprintf (stderr, "Memory alloc define not found\n");
	  return;
	}			/* if */
    }				/* for oper */
  return;
}				/* Ltahoe_scan_prologue_defines */


#if 0

enum
{ SAVE_UNAT = 0, RESTORE_UNAT };

/* 09/11/02 REK Modifying function to use new TAHOEops. */
static void
Ltahoe_maintain_UNAT (L_Cb * cb, L_Oper * jsr_oper, int action)
{
  static L_Operand *UNAT_save_reg = NULL;
  L_Oper *mov_oper;
  L_Attr *attr;

  if (action == SAVE_UNAT)
    {
      UNAT_save_reg = Ltahoe_new_int_reg ();
      mov_oper = L_create_new_op (Lop_MOV);
      mov_oper->proc_opc = TAHOEop_MOV_FRAR_M;
      mov_oper->src[0] = Ltahoe_IMAC (UNAT);
      mov_oper->dest[0] = UNAT_save_reg;
    }				/* if */
  else if (action == RESTORE_UNAT)
    {
      if (!UNAT_save_reg)
	L_punt ("L_maintain_UNAT: UNAT not saved");

      mov_oper = L_create_new_op (Lop_MOV);
      mov_oper->proc_opc = TAHOEop_MOV_TOAR_M;
      mov_oper->src[0] = L_copy_operand (UNAT_save_reg);
      mov_oper->dest[0] = Ltahoe_IMAC (UNAT);
    }				/* else if */
  else
    {
      L_punt ("L_maintain_UNAT: unknown action %d", action);
      return;
    }				/* else */

  attr = L_new_attr ("vararg-UNAT", 0);
  mov_oper->attr = L_concat_attr (mov_oper->attr, attr);

  L_insert_oper_before (cb, jsr_oper, mov_oper);
  return;
}				/* Ltahoe_update_alloc */
#endif

/* 09/11/02 REK Modifying function to support the new TAHOEops. */
static void
L_annotate_prologue (L_Func * fn, L_Cb * cb, L_Oper * oper)
{
  L_Oper *new_oper, *alloc_oper, *mov_oper;
  int leaf;

  leaf = L_EXTRACT_BIT_VAL (fn->flags, L_FUNC_LEAF);

  /* Record if function is leaf */
  new_oper = L_create_new_op_using (Lop_DEFINE, oper);
  new_oper->proc_opc = TAHOEop_NON_INSTR;
  new_oper->dest[0] = Ltahoe_IMAC (LEAF);
  new_oper->src[0] = L_new_gen_int_operand (leaf);
  L_insert_oper_before (cb, oper, new_oper);

  /* Record the amount of bytes which must be allocated */
  new_oper = L_create_new_op_using (Lop_DEFINE, oper);
  new_oper->proc_opc = TAHOEop_NON_INSTR;
  new_oper->dest[0] = Ltahoe_IMAC (MEM_ALLOC);
  new_oper->src[0] = L_new_gen_int_operand (L_scan_for_IMPACT_alloc (fn));
  L_insert_oper_before (cb, oper, new_oper);

  /* Return address macro register */
  new_oper = L_create_new_op_using (Lop_DEFINE, oper);
  new_oper->proc_opc = TAHOEop_NON_INSTR;
  new_oper->dest[0] = Ltahoe_BMAC (RETADDR);
  L_insert_oper_before (cb, oper, new_oper);

  new_oper = L_create_new_op_using (Lop_DEFINE, oper);
  new_oper->proc_opc = TAHOEop_NON_INSTR;
  new_oper->dest[0] = Ltahoe_IMAC (GP);
  L_insert_oper_before (cb, oper, new_oper);

  /*  Mark prologue as non-Tahoe instruction  */
  oper->proc_opc = TAHOEop_NON_INSTR;
  L_insert_oper_before (cb, oper, L_copy_parent_oper (oper));

  /* Tahoe alloc instruction */
  alloc_oper = L_create_new_op_using (Lop_ALLOC, oper);
  alloc_oper->proc_opc = TAHOEop_ALLOC;
  alloc_oper->dest[0] = Ltahoe_new_int_reg ();
  pfs_save_operand = alloc_oper->dest[0];

  alloc_oper->src[0] = L_new_gen_int_operand (-1);
  alloc_oper->src[1] = L_new_gen_int_operand (-1);
  alloc_oper->src[2] = L_new_gen_int_operand (-1);
  alloc_oper->src[3] = L_new_gen_int_operand (-1);
  L_insert_oper_before (cb, oper, alloc_oper);

  if (!leaf)
    {
      /* save the Return Pointer if this is a non-leaf function */
      mov_oper = L_create_new_op (Lop_MOV);
      mov_oper->proc_opc = TAHOEop_MOV_FRBR;
      mov_oper->src[0] = Ltahoe_BMAC (RETADDR);
      mov_oper->dest[0] = Ltahoe_new_int_reg ();
      retaddr_save_operand = mov_oper->dest[0];
      L_insert_oper_before (cb, oper, mov_oper);
    }				/* if */
  return;
}				/* L_annotate_prologue */


/* 09/11/02 REK Modifying this function to support the new TAHOEops. */
static void
L_annotate_epilogue (L_Func * fn, L_Cb * cb, L_Oper * oper)
{
  L_Oper *new_oper, *def_oper;
  int leaf;

  leaf = L_EXTRACT_BIT_VAL (fn->flags, L_FUNC_LEAF);

  /* restore the Return Pointer if this is a non-leaf function */
  if (!leaf)
    {
      new_oper = L_create_new_op (Lop_MOV);
      new_oper->proc_opc = TAHOEop_MOV_TOBR;
      new_oper->completers |= TC_MOV_TOBR_RET;
      new_oper->src[0] = L_copy_operand (retaddr_save_operand);
      new_oper->dest[0] = Ltahoe_BMAC (RETADDR);
      L_insert_oper_before (cb, oper, new_oper);

      /* restore the pfs */
      new_oper = L_create_new_op (Lop_MOV);
      new_oper->proc_opc = TAHOEop_MOV_TOAR_I;
      new_oper->src[0] = L_copy_operand (pfs_save_operand);
      new_oper->dest[0] = Ltahoe_IMAC (AR_PFS);
      L_insert_oper_before (cb, oper, new_oper);
    }				/* if */

  /*  Mark epilogue as non-Tahoe instruction  */

  oper->proc_opc = TAHOEop_NON_INSTR;
  new_oper = L_copy_parent_oper (oper);
  L_insert_oper_before (cb, oper, new_oper);

  if (!leaf)
    {
      def_oper = L_create_new_op (Lop_DEFINE);
      def_oper->proc_opc = TAHOEop_NON_INSTR;
      def_oper->src[0] = Ltahoe_IMAC (AR_PFS);
      L_insert_oper_after (cb, new_oper, def_oper);
    }				/* if */

  def_oper = L_create_new_op (Lop_DEFINE);
  def_oper->proc_opc = TAHOEop_NON_INSTR;
  def_oper->src[0] = Ltahoe_IMAC (UNAT);
  L_insert_oper_after (cb, new_oper, def_oper);

  def_oper = L_create_new_op (Lop_DEFINE);
  def_oper->proc_opc = TAHOEop_NON_INSTR;
  def_oper->src[0] = Ltahoe_IMAC (GP);
  L_insert_oper_after (cb, new_oper, def_oper);

  return;
}				/* L_annotate_epilogue */


/*****************************************************************************\
 *
 * Floating Point Constants
 *
\*****************************************************************************/

static L_Operand *
L_float_constant_immed (L_Cb * cb, L_Oper * oper, L_Operand * src_operand)
{
  L_Oper *new_oper1, *new_oper2;
  int op, ctype, proc_opc;

  if (L_is_zero (src_operand))
    return (Ltahoe_FMAC (FZERO));
  else if (L_is_one (src_operand))
    return (Ltahoe_FMAC (FONE));

  new_oper1 = L_create_new_op_using (Lop_MOV, oper);
  new_oper1->proc_opc = TAHOEop_MOVL;
  new_oper1->src[0] = L_copy_operand (src_operand);

  new_oper1->dest[0] = Ltahoe_new_int_reg ();
  L_insert_oper_before (cb, oper, new_oper1);

  op = Lop_I_F;
  proc_opc = TAHOEop_SETF_S;
  ctype = L_CTYPE_DOUBLE;

  new_oper2 = L_create_new_op_using (op, oper);
  new_oper2->proc_opc = proc_opc;
  new_oper2->src[0] = L_copy_operand (new_oper1->dest[0]);
  new_oper2->dest[0] = Ltahoe_new_reg (ctype);

  L_insert_oper_before (cb, oper, new_oper2);

  return (L_copy_operand (new_oper2->dest[0]));
}				/* L_float_constant_immed */


static L_Operand *
L_double_constant_immed (L_Cb * cb, L_Oper * oper, L_Operand * src_operand)
{
  L_Oper *new_oper1, *new_oper2;
  int op, ctype, proc_opc;

  if (L_is_zero (src_operand))
    return (Ltahoe_DMAC (FZERO));
  else if (L_is_one (src_operand))
    return (Ltahoe_DMAC (FONE));

  new_oper1 = L_create_new_op_using (Lop_MOV, oper);
  new_oper1->proc_opc = TAHOEop_MOVL;
  new_oper1->src[0] = L_copy_operand (src_operand);

  new_oper1->dest[0] = Ltahoe_new_int_reg ();
  L_insert_oper_before (cb, oper, new_oper1);

  op = Lop_I_F2;
  proc_opc = TAHOEop_SETF_D;
  ctype = L_CTYPE_DOUBLE;

  new_oper2 = L_create_new_op_using (op, oper);
  new_oper2->proc_opc = proc_opc;
  new_oper2->src[0] = L_copy_operand (new_oper1->dest[0]);
  new_oper2->dest[0] = Ltahoe_new_reg (ctype);

  L_insert_oper_before (cb, oper, new_oper2);

  return (L_copy_operand (new_oper2->dest[0]));
}				/* L_double_constant_immed */


int
L_convert_to_depz (ITintmax constant, ITintmax * pnum, int *ppos, int *plen)
{
  /* 02/23/01 JDM - rewritten to handle ITintmax instead of int. If
   * literal is only 8 bits wide, can use a dep.z operation, instead
   * of expensive 64-bit move
   *
   * Return values: 1 - can use depz
   *                0 - cannot use depz
   */

  ITint64 min_constant;
  int lzero, len;

  min_constant = constant;

  /* Count least-significant zeros */

  if (constant != 0)
    {
      for (lzero = 0; !(min_constant & 0x01); lzero++)
	min_constant >>= 1;
    }				/* if */
  else
    {
      lzero = 0;
    }				/* else */

  if (min_constant > 127)
    {
      int hzero;
      ITintmax tmp = constant;

      /* Count high-order zero bits */

      for (hzero = 0; tmp > 0; hzero++)
	tmp <<= 1;

      /* Sign extend constant at minimum width */

      min_constant = tmp >> (hzero + lzero);

      len = 64 - lzero - hzero;
    }				/* if */
  else if (min_constant >= 0)
    {
      len = 8;
    }				/* else if */
  else
    {
      len = 64 - lzero;
    }				/* else */

  if ((min_constant >= -128) && (min_constant <= 127))
    {
      *pnum = min_constant;
      *ppos = lzero;
      *plen = len;
      return 1;
    }				/* if */

  return 0;
}				/* L_convert_to_depz */


/*
 * Ltahoe_int_constant_load -- JWS 20001130
 * ----------------------------------------------------------------------------
 * Inserts operations prior to oper in cb to load an int constant into dest.
 * Both "src" and "dest" are copied.
 */
/* 09/11/02 REK Modifying this function to support the new TAHOEops. */
L_Oper *
Ltahoe_int_constant_load (L_Cb * cb, L_Oper * oper,
			  L_Operand * src, L_Operand * dest)
{
  L_Oper *new_oper;
  ITintmax n;
  int pos, len;

  if (!L_is_int_constant (src))
    L_punt ("Ltahoe_int_constant_load: tried to load non-int-constant");

  if (src->value.i == 0)
    {
      /* Case of ZERO-value being moved to register */
      new_oper = L_create_new_op_using (Lop_MOV, oper);
      new_oper->proc_opc = TAHOEop_MOV_GR;
      new_oper->src[0] = Ltahoe_IMAC (ZERO);
    }				/* if */
  else if (SIMM_22 (src->value.i))
    {
      new_oper = L_create_new_op_using (Lop_MOV, oper);
      new_oper->proc_opc = TAHOEop_MOVI;
      new_oper->src[0] = L_copy_operand (src);
    }				/* else if */
  else if (L_convert_to_depz (src->value.i, &n, &pos, &len))
    {
      /* Case of literal fitting in requirements of zero and deposit  */
      /* Saves some, because otherwise need to use a whole 64-bit I-word */
      new_oper = L_create_new_op_using (Lop_LSL, oper);
      new_oper->proc_opc = TAHOEop_DEP_Z;
      new_oper->src[0] = L_new_gen_int_operand (n);
      new_oper->src[1] = L_new_gen_int_operand (pos);
      new_oper->src[2] = L_new_gen_int_operand (len);
    }				/* else if */
  else
    {
      /* If all other ways to "move" can't be used, then use
         expensive move */
      new_oper = L_create_new_op_using (Lop_MOV, oper);
      new_oper->proc_opc = TAHOEop_MOVL;
      new_oper->src[0] = L_copy_operand (src);
    }				/* else */

  CLEAR_FLAGS (new_oper);
  Ltahoe_copy_or_new (new_oper->dest[0], dest);
  L_insert_oper_before (cb, oper, new_oper);
  return (new_oper);
}				/* Ltahoe_int_constant_load */


/*
 * Ltahoe_label_load -- JWS 20001130
 * ----------------------------------------------------------------------------
 * Inserts operations prior to oper in cb to load a label into dest.
 * Both "src" and "dest" are copied.
 */
L_Oper *
Ltahoe_label_load (L_Cb * cb, L_Oper * oper, L_Operand * src,
		   L_Operand * dest)
{
  L_Oper *new_oper, *new_oper2;
  L_Operand *inter;

  if (L_is_label (src))
    {
      /* Check if the operation has been marked as a potential
	 operation to use gp relative addressing */

      if (Ltahoe_use_gp_rel_addressing)
	{
	  if (L_is_LOCAL_GP_label (src))
	    {
	      /* Access to OWN SHORT data space
	       * ----------------------------------------------
	       * addl        dest = @gprel(label#), gp ;;
	       */
	      new_oper = L_create_new_op_using (Lop_ADD, oper);
	      new_oper->proc_opc = TAHOEop_ADDL;
	      new_oper->src[0] = L_copy_operand (src);
	      new_oper->src[1] = Ltahoe_IMAC (GP);
	      /* 02/17/03 REK We can clear the volatile flag from the new add
	       *              oper.  The actual ld/st will still be volatile.
	       */
	      new_oper->flags = L_CLR_BIT_FLAG (new_oper->flags,
						L_OPER_VOLATILE);
	    }			/* if */
	  else if (L_is_GLOBAL_GP_label (src))
	    {
	      /* Access to OWN LONG data space
	       * ----------------------------------------------
	       * movl        t1 = @gprel(label#) ;;
	       * add         dest = t1, gp ;;
	       */
	      new_oper2 = L_create_new_op_using (Lop_MOV, oper);
	      new_oper2->proc_opc = TAHOEop_MOVL;
	      new_oper2->src[0] = L_copy_operand (src);
	      /* 02/17/03 REK We can clear the volatile flag from the new mov
	       *              oper.  The actual ld/st will still be volatile.
	       */
	      new_oper2->flags = L_CLR_BIT_FLAG (new_oper2->flags,
						 L_OPER_VOLATILE);
	      L_assign_type_LOCAL_GP_label (new_oper2->src[0]);
	      inter = Ltahoe_new_int_reg ();
	      new_oper2->dest[0] = inter;
	      L_insert_oper_before (cb, oper, new_oper2);
	      new_oper = L_create_new_op_using (Lop_ADD, oper);
	      new_oper->proc_opc = TAHOEop_ADD;
	      new_oper->src[0] = L_copy_operand (inter);
	      new_oper->src[1] = Ltahoe_IMAC (GP);
	      /* 02/17/03 REK We can clear the volatile flag from the new add
	       *              oper.  The actual ld/st will still be volatile.
	       */
	      new_oper->flags = L_CLR_BIT_FLAG (new_oper->flags,
						L_OPER_VOLATILE);
	    }			/* else if */
	  else
	    {
	      /* Access to other load module via linkage table
	       * ----------------------------------------------
	       * addl        t1 = @ltoff(label#), gp ;;
	       * ld8         dest = [t1] ;;
	       */
	      L_Operand *inter;
	      L_Attr *label_attr;

	      new_oper2 = L_create_new_op_using (Lop_ADD, oper);
	      new_oper2->proc_opc = TAHOEop_ADDL;
	      new_oper2->src[0] = L_copy_operand (src);
	      /* 02/17/03 REK We can clear the volatile flag from the new add
	       *              oper.  The actual ld/st will still be volatile.
	       */
	      new_oper2->flags = L_CLR_BIT_FLAG (new_oper2->flags,
						 L_OPER_VOLATILE);
	      L_assign_type_GLOBAL_GP_label (new_oper2->src[0]);
	      new_oper2->src[1] = Ltahoe_IMAC (GP);
	      inter = Ltahoe_new_int_reg ();
	      new_oper2->dest[0] = inter;
	      L_insert_oper_before (cb, oper, new_oper2);
	      new_oper = L_create_new_op_using (Lop_LD_Q, oper);
	      new_oper->proc_opc = TAHOEop_LD8;
	      new_oper->src[0] = L_copy_operand (inter);

	      if ((label_attr = L_find_attr (new_oper->attr, "label")))
		L_assign_type_GLOBAL_GP_label (label_attr->field[0]);
	    }			/* else */
	}				/* if */
      else
	{
	  /* Flat label access
	   * ----------------------------------------------
	   * movl        dest = label#
	   */
	  new_oper = L_create_new_op_using (Lop_MOV, oper);
	  new_oper->proc_opc = TAHOEop_MOVL;
	  new_oper->src[0] = L_copy_operand (src);
	  /* 02/17/03 REK We can clear the volatile flag from the new mov
	   *              oper.  The actual ld/st will still be volatile. */
	  new_oper->flags = L_CLR_BIT_FLAG (new_oper->flags, L_OPER_VOLATILE);
	}				/* else */
    }
  else if (L_is_cb (src))
    {
      new_oper = L_create_new_op_using (Lop_MOV, oper);
      new_oper->proc_opc = TAHOEop_MOVL;
      new_oper->src[0] = L_copy_operand (src);
    }
  else
    {
      L_punt ("Ltahoe_label_load: unhandled operand on op %d", oper->id);
      return NULL;
    }

  CLEAR_FLAGS (new_oper);
  Ltahoe_copy_or_new (new_oper->dest[0], dest);
  L_insert_oper_before (cb, oper, new_oper);

  return (new_oper);
}				/* Ltahoe_label_load */

/*
 * L_convert_compares_to_predicates (L_Func * fn)
 * ----------------------------------------------------------------------
 * <p3> rcmp i eq r13 = r28, r30 -> <p3>  cmp.i.eq p1_ut,p2_uf = r28, r30
 *                                  <p1>  mov      r13 = 1
 *                                  <p2>  mov      r13 = 0
 */
static void
L_convert_compares_to_predicates (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *op, *next_op;
  L_Oper *mov0_oper, *mov1_oper, *mov_oper, *cmp_oper;
  L_Operand *src;
  L_Attr *attr;
  int i, dest_is_src;

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      for (op = cb->first_op; op; op = next_op)
	{
	  next_op = op->next_op;

	  if (!L_general_comparison_opcode (op))
	    continue;

	  if (L_is_int_constant (op->src[0])
	      && L_is_int_constant (op->src[1]))
	    fprintf (stderr,
		     "Warning:Compare op %i has two const operands\n",
		     op->id);

	  cmp_oper = L_create_new_op_using (op->opc, op);
	  L_change_to_cmp_op (cmp_oper);
	  L_copy_compare (cmp_oper, op);

	  /* 09/16/02 REK Changing this to use the new completers field. */
	  /* cmp_oper->proc_opc = M_tahoe_cmp_proc_opc (cmp_oper->com[0], */
	  /*                                            cmp_oper->com[1], */
	  /*                                            UNC_CMP_TYPE); */
	  cmp_oper->proc_opc = LT_tahoe_cmp_proc_opc (cmp_oper->com[0],
						      cmp_oper->com[1]);
	  cmp_oper->completers = LT_tahoe_cmp_completer (cmp_oper->com[0],
							 cmp_oper->com[1],
							 TC_CMP_TYPE_UNC);

	  for (i = 0; i < L_max_src_operand; i++)
	    if ((src = op->src[i]))
	      cmp_oper->src[i] = L_copy_operand (src);

	  cmp_oper->dest[0] = Ltahoe_new_pred_reg (L_PTYPE_UNCOND_T);
	  cmp_oper->dest[1] = Ltahoe_true_pred (L_PTYPE_UNCOND_F);

	  if (L_is_src_operand (op->dest[0], op))
	    dest_is_src = TRUE;
	  else
	    dest_is_src = FALSE;

	  mov0_oper = L_create_new_op_using (Lop_MOV, op);
	  mov0_oper->src[0] = Ltahoe_IMAC (ZERO);

	  if (dest_is_src)
	    mov0_oper->dest[0] = Ltahoe_new_int_reg ();
	  else
	    mov0_oper->dest[0] = L_copy_operand (op->dest[0]);

	  mov1_oper = L_create_new_op (Lop_MOV);
	  mov1_oper->src[0] = L_new_gen_int_operand (1);
	  mov1_oper->dest[0] = L_copy_operand (mov0_oper->dest[0]);
	  mov1_oper->pred[0] = L_copy_operand (cmp_oper->dest[0]);
	  mov1_oper->pred[0]->ptype = L_PTYPE_NULL;

	  /* Add attribute so that compare optimizations can find these
	     instructions easier */
	  attr = L_new_attr ("compare_lop", 1);
	  L_set_int_attr_field (attr, 0, op->id);
	  mov0_oper->attr = L_concat_attr (mov0_oper->attr, attr);
	  cmp_oper->attr = L_concat_attr (cmp_oper->attr, L_copy_attr (attr));
	  mov1_oper->attr = L_concat_attr (mov1_oper->attr,
					   L_copy_attr (attr));

	  L_insert_oper_before (cb, op, mov0_oper);
	  L_insert_oper_before (cb, op, cmp_oper);
	  L_insert_oper_before (cb, op, mov1_oper);

	  if (dest_is_src)
	    {
	      mov_oper = L_create_new_op_using (Lop_MOV, op);
	      mov_oper->src[0] = L_copy_operand (mov0_oper->dest[0]);
	      mov_oper->dest[0] = L_copy_operand (op->dest[0]);
	      mov_oper->attr =
		L_concat_attr (mov_oper->attr, L_copy_attr (attr));
	      L_insert_oper_before (cb, op, mov_oper);
	    }			/* if */

	  cb->flags = L_SET_BIT_FLAG (cb->flags, L_CB_HYPERBLOCK);
	  fn->flags = L_SET_BIT_FLAG (fn->flags, L_FUNC_HYPERBLOCK);

	  /* All done, delete the old op */
	  L_delete_oper (cb, op);
	}			/* for op */
    }				/* for cb */
}				/* L_convert_compares_to_predicates */

/* 09/11/02 REK Modifying function to use the new TAHOEops. */
static void
L_annotate_extend (L_Cb * cb, L_Oper * oper)
{
  L_Oper *new_oper;
  ITintmax n;
  int pos, len;
  L_Operand *new_operand;

  if (oper->src[1])
    L_punt ("L_annotate_extend: non-null second operand: oper %d\n",
	    oper->id);

  if (!(L_is_variable (oper->dest[0])))
    L_punt ("L_annotate_extend: non-variable dest: oper %d\n", oper->id);

  /* Replace sxt with constant src with a move */
  if (L_is_int_constant (oper->src[0]))
    {
      if (oper->src[0]->value.i == 0)
	{
	  /* Case of ZERO-value being moved to register */
	  new_oper = L_create_new_op_using (Lop_MOV, oper);
	  new_oper->proc_opc = TAHOEop_MOV_GR;
	  new_oper->src[0] = Ltahoe_IMAC (ZERO);
	  new_oper->dest[0] = L_copy_operand (oper->dest[0]);
	  L_insert_oper_before (cb, oper, new_oper);
	  return;
	}			/* if */

      /* 01/13/03 REK L_opcode_ctype is supposed to return the type of the
       *              destination of the oper, not the source.  For historical
       *              reasons, it returns the type of the source for sign and
       *              zero extend operations. */
      new_operand =
	L_copy_immed_operand (L_CTYPE_LLONG, L_opcode_ctype (oper),
			      oper->src[0]);
      if (SIMM_22 (new_operand->value.i))
	{
	  new_oper = L_create_new_op_using (Lop_MOV, oper);
	  new_oper->proc_opc = TAHOEop_MOVI;
	  new_oper->src[0] = new_operand;
	  new_oper->dest[0] = L_copy_operand (oper->dest[0]);
	  L_insert_oper_before (cb, oper, new_oper);
	  return;
	}			/* if */

      if (L_convert_to_depz (new_operand->value.i, &n, &pos, &len))
	{
	  /* Case of literal fitting in requirements of zero and deposit  */
	  /* Saves some, because otherwise need to use a whole 64-bit */
	  /* I-word */
	  L_delete_operand (new_operand);
	  new_oper = L_create_new_op_using (Lop_BIT_DEPOSIT, oper);
	  new_oper->proc_opc = TAHOEop_DEP_Z;
	  new_oper->src[0] = L_new_gen_int_operand (n);
	  new_oper->src[1] = L_new_gen_int_operand (pos);
	  new_oper->src[2] = L_new_gen_int_operand (len);
	  new_oper->dest[0] = L_copy_operand (oper->dest[0]);
	  L_insert_oper_before (cb, oper, new_oper);
	  return;
	}			/* if */

      /* If all other ways to "move" can't be used, 
         then use expensive move */
      new_oper = L_create_new_op_using (Lop_MOV, oper);
      new_oper->proc_opc = TAHOEop_MOVL;
      new_oper->src[0] = new_operand;
      new_oper->dest[0] = L_copy_operand (oper->dest[0]);
      L_insert_oper_before (cb, oper, new_oper);
      return;
    }				/* if */

  if (!(L_is_variable (oper->src[0])))
    L_punt ("L_annotate_extend: non-variable src or dest: oper %d\n",
	    oper->id);

  new_oper = L_create_new_op_using (oper->opc, oper);
  switch (oper->opc)
    {
    case Lop_EXTRACT_C:
    case Lop_SXT_C:
      new_oper->proc_opc = TAHOEop_SXT1;
      break;

    case Lop_EXTRACT_C2:
    case Lop_SXT_C2:
      new_oper->proc_opc = TAHOEop_SXT2;
      break;

    case Lop_SXT_I:
      new_oper->proc_opc = TAHOEop_SXT4;
      break;

    case Lop_ZXT_C:
      new_oper->proc_opc = TAHOEop_ZXT1;
      break;

    case Lop_ZXT_C2:
      new_oper->proc_opc = TAHOEop_ZXT2;
      break;

    case Lop_ZXT_I:
      new_oper->proc_opc = TAHOEop_ZXT4;
      break;

    default:
      L_punt ("L_annotate_extend: Unknown lopcode: oper %d\n", oper->id);
      break;
    }				/* switch */

  new_oper->src[0] = L_copy_operand (oper->src[0]);
  new_oper->dest[0] = L_copy_operand (oper->dest[0]);
  L_insert_oper_before (cb, oper, new_oper);
}				/* L_annotate_extend */

static int pwr_n_minus_1 (ITintmax i);

/*
 * L_annotate_int_logic
 * ----------------------------------------------------------------------
 * AND, NAND, OR, NOR, XOR, NXOR, AND_COMPL, OR_COMPL
 *
 * (qp) and   r1 = (imm8|r2), r3               // r1 = r2 & r3
 * (qp) or    r1 = (imm8|r2), r3               // r1 = r2 | r3
 * (qp) xor   r1 = (imm8|r2), r3               // r1 = r2 ^ r3
 * (qp) andcm r1 = (imm8|r2), r3               // r1 = r2 & ~r3
 */
/* 09/11/02 REK Modifying function to use new TAHOEops. */
static void
L_annotate_int_logic (L_Cb * cb, L_Oper * oper)
{
  L_Oper *logic_oper, *extend_oper;
  L_Operand *logic_src0, *logic_src1 = NULL;
  int logic_tahoeop = TAHOEop_NON_INSTR;
  int con0, con1, swap;
  char negated_oper;

  con0 = L_is_int_constant (oper->src[0]);
  con1 = L_is_int_constant (oper->src[1]);

  if (con0 && con1)
    {
      ITintmax imval;

      imval = L_evaluate_int_arithmetic (oper);
      L_convert_to_move (oper, L_copy_operand (oper->dest[0]),
			 L_new_gen_int_operand (imval));
      L_annotate_int_move (cb, oper);
      return;
    }				/* if */

  logic_src0 = oper->src[0];
  logic_src1 = oper->src[1];

  negated_oper = FALSE;
  swap = FALSE;

  switch (oper->opc)
    {
      /* commutative cases */

    case Lop_NAND:
      negated_oper = TRUE;
    case Lop_AND:
      logic_tahoeop = TAHOEop_AND;
      if (con1)
	swap = TRUE;
      break;

    case Lop_NOR:
      negated_oper = TRUE;
    case Lop_OR:
      logic_tahoeop = TAHOEop_OR;
      if (con1)
	swap = TRUE;
      break;

    case Lop_NXOR:
      negated_oper = TRUE;
    case Lop_XOR:
      logic_tahoeop = TAHOEop_XOR;
      if (con1)
	swap = TRUE;
      break;

      /* non-commutative cases */

    case Lop_OR_COMPL:
      /* Apply DeMorgan's */

      negated_oper = TRUE;
      swap = TRUE;
    case Lop_AND_COMPL:
      logic_tahoeop = TAHOEop_ANDCM;
      break;

    default:
      L_punt ("Unknown logic operation  oper:%d\n", oper->id);
    }				/* switch */

  if (swap)
    {
      logic_src1 = oper->src[0];
      logic_src0 = oper->src[1];
      con0 = L_is_int_constant (logic_src0);
      con1 = L_is_int_constant (logic_src1);
    }				/* if */

  /* At most one operand is a constant.  For commutative operations,
   * the potentially constant operation is logic_src0
   */

  /* Special case where AND r1 = CONST,r1 is used to clear bits above 
   * 1 or 2 bytes.  zero extend rather than a load constant and add 
   */

  if (con0)
    {
      if (oper->opc == Lop_AND)
	{
	  ITintmax val = logic_src0->value.i;
	  int ones;

	  if (val == LLCONST (255))
	    {
	      extend_oper =
		Ltahoe_zero_extend (oper, logic_src1, oper->dest[0], 1);
	      L_insert_oper_before (cb, oper, extend_oper);
	      return;
	    }			/* if */
	  else if (val == LLCONST (65535))
	    {
	      extend_oper =
		Ltahoe_zero_extend (oper, logic_src1, oper->dest[0], 2);
	      L_insert_oper_before (cb, oper, extend_oper);
	      return;
	    }			/* else if */
	  else if (!SIMM_8 (val) && (ones = pwr_n_minus_1 (val)))
	    {
	      logic_oper = L_create_new_op_using (Lop_EXTRACT_U, oper);
	      logic_oper->proc_opc = TAHOEop_EXTR_U;
	      logic_oper->dest[0] = L_copy_operand (oper->dest[0]);
	      logic_oper->src[0] = L_copy_operand (logic_src1);
	      logic_oper->src[1] = L_new_gen_int_operand (0);
	      logic_oper->src[2] = L_new_gen_int_operand (ones);
	      L_insert_oper_before (cb, oper, logic_oper);
	      return;
	    }			/* else if */
	}			/* if */

      if (!SIMM_8 (logic_src0->value.i))
	{
	  L_Operand *new_opd = Ltahoe_new_int_reg ();
	  Ltahoe_int_constant_load (cb, oper, logic_src0, new_opd);
	  logic_src0 = new_opd;
	  con0 = 0;
	}			/* if */
      else
	{
	  logic_src0 = L_copy_operand (logic_src0);
	}			/* else */
    }				/* if */
  else
    {
      logic_src0 = L_copy_operand (logic_src0);
    }				/* else */

  if (con1)
    {
      L_Operand *new_opd = Ltahoe_new_int_reg ();
      Ltahoe_int_constant_load (cb, oper, logic_src1, new_opd);
      logic_src1 = new_opd;
    }				/* if */
  else
    {
      logic_src1 = L_copy_operand (logic_src1);
    }				/* else */

  logic_oper = L_create_new_op_using (oper->opc, oper);
  logic_oper->proc_opc = logic_tahoeop;
  L_insert_oper_before (cb, oper, logic_oper);

  logic_oper->src[0] = logic_src0;
  logic_oper->src[1] = logic_src1;

  if (negated_oper)
    {
      L_Oper *comp_oper;

      fprintf (stderr, "Haven't checked negated logic  oper:%d\n", oper->id);
      logic_oper->dest[0] = Ltahoe_new_int_reg ();
      comp_oper = L_create_new_op_using (Lop_AND_COMPL, oper);
      comp_oper->proc_opc = TAHOEop_ANDCM;
      L_insert_oper_before (cb, oper, comp_oper);

      comp_oper->src[0] = L_new_gen_int_operand (-1);
      comp_oper->src[1] = L_copy_operand (logic_oper->dest[0]);
      comp_oper->dest[0] = L_copy_operand (oper->dest[0]);
    }				/* if */
  else
    {
      logic_oper->dest[0] = L_copy_operand (oper->dest[0]);
    }				/* else */

  return;
}				/* L_annotate_int_logic */

/*****************************************************************************\
 *
 * Integer Annotation functions
 *
\*****************************************************************************/
/* 09/11/02 REK Modifying this function so it uses new TAHOEops. */
void
L_annotate_int_move (L_Cb * cb, L_Oper * oper)
{
  ITintmax n;
  int pos, len;
  L_Oper *new_oper;
  L_Operand *src, *dest;
  int proc_opc;

  src = oper->src[0];
  dest = oper->dest[0];

  /* Only need to check if predicate is source register  */
  /* since only one operation moves a predicate register */
  if (L_is_predicate_reg (src))
    {
      fprintf (stderr, "Check out pred move in phase1 oper %d:\n", oper->id);
      new_oper = L_copy_parent_oper (oper);
      /* Move predicate register to general register */
      new_oper->proc_opc = TAHOEop_MOV_FRPR;
      L_insert_oper_before (cb, oper, new_oper);
      return;
    }				/* if */

  /* If writing into an application register, use the
     correct proc_opc. These may be generated in Lsoftpipe. */
  if (L_is_macro (dest) && TAHOE_APPLICATION_MACRO (dest))
    {
      if (TAHOE_I_APPLICATION_MACRO (dest))
	proc_opc = TAHOEop_MOV_TOAR_I;
      else
	proc_opc = TAHOEop_MOV_TOAR_M;

      /* src can only be a general purpose register or
         an 8-bit immediate. */
      if ((L_is_register (src) && L_is_ctype_integer (src))
	  || (L_is_macro (src) && src->value.mac <= L_MAC_LAST)
	  || (L_is_macro (src) && src->value.mac == TAHOE_MAC_ZERO)
	  || (L_is_int_constant (src) && SIMM_8 (src->value.i)))
	{
	  new_oper = L_create_new_op_using (Lop_MOV, oper);
	  new_oper->proc_opc = proc_opc;
	  new_oper->src[0] = L_copy_operand (src);
	  new_oper->dest[0] = L_copy_operand (dest);
	  L_insert_oper_before (cb, oper, new_oper);
	}			/* if */
      else
	L_punt
	  ("L_annotate_int_move: unexpected move src into app register %d.",
	   oper->id);
      return;
    }				/* if */

  switch (src->type)
    {
    case L_OPERAND_IMMED:
      if (L_is_ctype_integer (src))
	{
	  if (src->value.i == 0)
	    {
	      /* Case of ZERO-value being moved to register */
	      new_oper = L_create_new_op_using (Lop_MOV, oper);
	      new_oper->proc_opc = TAHOEop_MOV_GR;
	      new_oper->src[0] = Ltahoe_IMAC (ZERO);
	      new_oper->dest[0] = L_copy_operand (oper->dest[0]);
	      L_insert_oper_before (cb, oper, new_oper);
	    }			/* if */
	  else if (SIMM_22 (src->value.i))
	    {
	      new_oper = L_create_new_op_using (Lop_MOV, oper);
	      new_oper->proc_opc = TAHOEop_MOVI;
	      new_oper->src[0] = L_copy_operand (src);
	      new_oper->dest[0] = L_copy_operand (oper->dest[0]);
	      L_insert_oper_before (cb, oper, new_oper);
	    }			/* else if */
	  else if (L_convert_to_depz (src->value.i, &n, &pos, &len))
	    {
	      /* Case of literal fitting in requirements of zero and
	       * deposit.
	       * Saves some, because otherwise need to use a whole 
	       * 64-bit I-word */
	      new_oper = L_create_new_op_using (Lop_LSL, oper);
	      new_oper->proc_opc = TAHOEop_DEP_Z;
	      new_oper->src[0] = L_new_gen_int_operand (n);
	      new_oper->src[1] = L_new_gen_int_operand (pos);
	      new_oper->src[2] = L_new_gen_int_operand (len);
	      new_oper->dest[0] = L_copy_operand (oper->dest[0]);
	      L_insert_oper_before (cb, oper, new_oper);
	    }			/* else if */
	  else
	    {
	      /* If all other ways to "move" can't be used, 
	         then use expensive move */
	      new_oper = L_create_new_op_using (Lop_MOV, oper);
	      new_oper->proc_opc = TAHOEop_MOVL;
	      new_oper->src[0] = L_copy_operand (src);
	      new_oper->dest[0] = L_copy_operand (oper->dest[0]);
	      L_insert_oper_before (cb, oper, new_oper);
	    }			/* else */
	}			/* if */
      else
	{
	  L_punt ("L_annotate_int_move: non-int constant %d\n", oper->id);
	}			/* else */
      break;

    case L_OPERAND_CB:
      new_oper = L_create_new_op_using (Lop_MOV, oper);
      new_oper->proc_opc = TAHOEop_MOVL;
      new_oper->src[0] = L_copy_operand (src);
      new_oper->dest[0] = L_copy_operand (oper->dest[0]);;
      L_insert_oper_before (cb, oper, new_oper);
      break;

    case L_OPERAND_LABEL:
      if (!strncmp (src->value.l, "hash_", 5))
	{
	  /* Special case for hash tables */
	  char buf[50];
	  sprintf (buf, "_$%s_%s", L_fn->name, src->value.l);
	  new_oper = L_create_new_op_using (Lop_MOV, oper);
	  new_oper->src[0] = L_new_label_operand (buf, L_CTYPE_GLOBAL_ABS);
	  new_oper->proc_opc = TAHOEop_MOVL;
	  new_oper->dest[0] = L_copy_operand (oper->dest[0]);
	  CLEAR_FLAGS (new_oper);
	  L_insert_oper_before (cb, oper, new_oper);
	}			/* if */
      else
	{
	  Ltahoe_label_load (cb, oper, src, oper->dest[0]);
	}			/* else */
      break;

    case L_OPERAND_REGISTER:
    case L_OPERAND_MACRO:
      if (!L_is_ctype_float(src))
	{
	  new_oper = L_create_new_op_using (Lop_MOV, oper);
	  if ((src->type == L_OPERAND_MACRO) &&
	      (src->value.mac == TAHOE_PRED_BLK_REG))
	    new_oper->proc_opc = TAHOEop_MOV_FRPR;
	  else
	    new_oper->proc_opc = TAHOEop_MOV_GR;
	}
      else 
	{
	  new_oper = L_create_new_op_using (Lop_F_I, oper);
	  if (L_is_ctype_dbl (src))
	    new_oper->proc_opc = TAHOEop_GETF_D;
	  else
	    new_oper->proc_opc = TAHOEop_GETF_S;
	}

      new_oper->src[0] = L_copy_operand (src);
      new_oper->dest[0] = L_copy_operand (oper->dest[0]);
      L_insert_oper_before (cb, oper, new_oper);

      break;

    default:
      L_punt ("L_annotate_int_mov: error");
    }				/* switch */
}				/* L_annotate_int_move */


/* 09/11/02 REK Modifying function so it uses new TAHOEops. */
static void
L_annotate_int_add (L_Cb * cb, L_Oper * oper)
{
  int lop, tahoeop;
  L_Oper *new_oper;
  L_Operand *src0, *src1, *new_src0, *new_src1;
  L_Operand *const_operand;

  src0 = oper->src[0];
  src1 = oper->src[1];

  if (Ltahoe_should_swap_operands (src0, src1))
    {
      src0 = oper->src[1];
      src1 = oper->src[0];
    }				/* if */

  switch (src0->type)
    {
    case L_OPERAND_IMMED:

      if (!L_is_ctype_integer (src0))
	{
	  L_punt ("L_annotate_int_add: non-int immed, oper  %d\n", oper->id);
	  return;
	}			/* if */

      switch (src1->type)
	{
	case L_OPERAND_IMMED:	/* src0 and src1 are int */
	  if (!L_is_ctype_integer (src1))
	    {
	      L_punt ("L_annotate_int_add: non-int immed, oper  %d\n",
		      oper->id);
	      return;
	    }			/* if */

	  const_operand = L_new_gen_int_operand (src0->value.i +
						 src1->value.i);
	  Ltahoe_int_constant_load (cb, oper, const_operand, oper->dest[0]);
	  L_delete_operand (const_operand);
	  return;

	case L_OPERAND_REGISTER:
	case L_OPERAND_MACRO:
	  new_src1 = L_copy_operand (src1);
	  if (SIMM_14 (src0->value.i))
	    {
	      lop = Lop_ADD;
	      tahoeop = TAHOEop_ADDS;
	      new_src0 = L_new_gen_int_operand (src0->value.i);
	    }			/* if */
	  else
	    {
	      new_src0 = Ltahoe_new_int_reg ();
	      Ltahoe_int_constant_load (cb, oper, src0, new_src0);
	      lop = Lop_ADD;
	      tahoeop = TAHOEop_ADD;
	    }			/* else */
	  break;

	case L_OPERAND_LABEL:
	  new_src1 = Ltahoe_new_int_reg ();
	  Ltahoe_label_load (cb, oper, src1, new_src1);
	  if (SIMM_14 (src0->value.i))
	    {
	      lop = Lop_ADD;
	      tahoeop = TAHOEop_ADDS;
	      new_src0 = L_new_gen_int_operand (src0->value.i);
	    }			/* if */
	  else
	    {
	      new_src0 = Ltahoe_new_int_reg ();
	      Ltahoe_int_constant_load (cb, oper, src0, new_src0);
	      lop = Lop_ADD;
	      tahoeop = TAHOEop_ADD;
	    }			/* else */
	  break;

	default:
	  L_punt ("Unexpected operand src1 type: %d\n in oper %d",
		  src0->type, oper->id);
	  return;
	}			/* switch */
      break;

    case L_OPERAND_REGISTER:
    case L_OPERAND_MACRO:
      new_src0 = L_copy_operand (src0);
      switch (src1->type)
	{
	case L_OPERAND_REGISTER:
	case L_OPERAND_MACRO:
	  lop = Lop_ADD;
	  tahoeop = TAHOEop_ADD;
	  new_src1 = L_copy_operand (src1);
	  break;
	case L_OPERAND_LABEL:	/* src0 is reg/mac and src1 is label */
	  new_src1 = Ltahoe_new_int_reg ();
	  Ltahoe_label_load (cb, oper, src1, new_src1);
	  lop = Lop_ADD;
	  tahoeop = TAHOEop_ADD;
	  break;
	default:
	  L_punt ("Unexpected operand src1 type: %d\n in oper %d",
		  src0->type, oper->id);
	  return;
	}			/* switch */
      break;

    case L_OPERAND_LABEL:
      new_src0 = Ltahoe_new_int_reg ();
      Ltahoe_label_load (cb, oper, src0, new_src0);
      switch (src1->type)
	{
	case L_OPERAND_REGISTER:
	case L_OPERAND_MACRO:
	  lop = Lop_ADD;
	  tahoeop = TAHOEop_ADD;
	  new_src1 = L_copy_operand (src1);
	  break;
	case L_OPERAND_LABEL:
	  new_src1 = Ltahoe_new_int_reg ();
	  Ltahoe_label_load (cb, oper, src1, new_src1);
	  lop = Lop_ADD;
	  tahoeop = TAHOEop_ADD;
	  break;
	default:
	  L_punt ("Unexpected operand src1 type: %d\n in oper %d",
		  src0->type, oper->id);
	  return;
	}			/* switch */
      break;

    default:
      L_punt ("Unexpected operand src0 type: %d\n in oper %d",
	      src0->type, oper->id);
      return;
    }				/* switch */

  /* new_sources have already been copied */

  new_oper = L_create_new_op_using (lop, oper);
  new_oper->proc_opc = tahoeop;
  new_oper->src[0] = new_src0;
  new_oper->src[1] = new_src1;
  new_oper->dest[0] = L_copy_operand (oper->dest[0]);

  L_insert_oper_before (cb, oper, new_oper);

  return;
}				/* L_annotate_int_add */


/* 09/11/02 REK Modifying this function so it uses the new TAHOEops. */
void
L_annotate_int_sub (L_Cb * cb, L_Oper * oper)
{
  int lop, tahoeop;
  L_Oper *new_oper;
  L_Operand *src0, *src1, *new_src0, *new_src1;
  L_Operand *const_operand;

  src0 = oper->src[0];
  src1 = oper->src[1];

  switch (src0->type)
    {
    case L_OPERAND_IMMED:
      if (!L_is_ctype_integer (src0))
	{
	  L_punt ("L_annotate_int_add: non-int immed, oper  %d\n", oper->id);
	  return;
	}			/* if */

      switch (src1->type)
	{
	case L_OPERAND_IMMED:
	  if (!L_is_ctype_integer (src1))
	    {
	      L_punt ("L_annotate_int_add: non-int immed, oper  %d\n",
		      oper->id);
	      return;
	    }			/* if */
	  const_operand =
	    L_new_gen_int_operand (src0->value.i - src1->value.i);
	  Ltahoe_int_constant_load (cb, oper, const_operand, oper->dest[0]);
	  L_delete_operand (const_operand);
	  return;

	case L_OPERAND_REGISTER:
	case L_OPERAND_MACRO:
	  new_src1 = L_copy_operand (src1);
	  if (SIMM_8 (src0->value.i))
	    {
	      lop = Lop_SUB;
	      tahoeop = TAHOEop_SUB;
	      new_src0 = L_copy_operand (src0);
	    }			/* if */
	  else
	    {
	      new_src0 = Ltahoe_new_int_reg ();
	      Ltahoe_int_constant_load (cb, oper, src0, new_src0);
	      lop = Lop_SUB;
	      tahoeop = TAHOEop_SUB;
	    }			/* else */
	  break;
	case L_OPERAND_LABEL:
	  new_src1 = Ltahoe_new_int_reg ();
	  Ltahoe_label_load (cb, oper, src1, new_src1);
	  if (SIMM_8 (src0->value.i))
	    {
	      lop = Lop_SUB;
	      tahoeop = TAHOEop_SUB;
	      new_src0 = L_copy_operand (oper->src[0]);
	    }			/* if */
	  else
	    {
	      new_src0 = Ltahoe_new_int_reg ();
	      Ltahoe_int_constant_load (cb, oper, src0, new_src0);
	      lop = Lop_SUB;
	      tahoeop = TAHOEop_SUB;
	    }			/* else */
	  break;
	default:
	  L_punt ("Unexpected operand src1 type: %d\n in oper %d", src0->type,
		  oper->id);
	  return;
	}			/* switch */
      break;

    case L_OPERAND_REGISTER:
    case L_OPERAND_MACRO:
      new_src0 = L_copy_operand (src0);

      switch (src1->type)
	{
	case L_OPERAND_IMMED:
	  if (!L_is_ctype_integer (src1))
	    {
	      L_punt ("L_annotate_int_add: non-int immed, oper  %d\n",
		      oper->id);
	      return;
	    }			/* if */

	  if (SIMM_14 (-src1->value.i))
	    {
	      lop = Lop_ADD;
	      tahoeop = TAHOEop_ADDS;
	      new_src1 = new_src0;	/* Sources must be int,reg */
	      new_src0 = L_new_gen_int_operand (-src1->value.i);
	    }			/* if */
	  else
	    {
	      new_src1 = Ltahoe_new_int_reg ();
	      Ltahoe_int_constant_load (cb, oper, src1, new_src1);
	      lop = Lop_SUB;
	      tahoeop = TAHOEop_SUB;
	    }			/* else */
	  break;

	case L_OPERAND_REGISTER:
	case L_OPERAND_MACRO:
	  lop = Lop_SUB;
	  tahoeop = TAHOEop_SUB;
	  new_src1 = L_copy_operand (src1);
	  break;

	case L_OPERAND_LABEL:	/* src0 is reg/mac and src1 is label */
	  new_src1 = Ltahoe_new_int_reg ();
	  Ltahoe_label_load (cb, oper, src1, new_src1);
	  lop = Lop_SUB;
	  tahoeop = TAHOEop_SUB;
	  break;
	default:
	  L_punt ("Unexpected operand src1 type: %d\n in oper %d", src0->type,
		  oper->id);
	  return;
	}			/* switch */
      break;

    case L_OPERAND_LABEL:
      new_src0 = Ltahoe_new_int_reg ();
      Ltahoe_label_load (cb, oper, src0, new_src0);

      switch (src1->type)
	{
	case L_OPERAND_IMMED:	/* src0 is label and src1 is int */

	  if (!L_is_ctype_integer (src1))
	    {
	      L_punt ("L_annotate_int_add: non-int immed, oper  %d\n",
		      oper->id);
	      return;
	    }			/* if */

	  if (SIMM_14 (-src1->value.i))
	    {
	      lop = Lop_ADD;
	      tahoeop = TAHOEop_ADDS;
	      new_src1 = new_src0;	/* Sources must be int,reg */
	      new_src0 = L_new_gen_int_operand (-src1->value.i);
	    }			/* if */
	  else
	    {
	      new_src1 = Ltahoe_new_int_reg ();
	      Ltahoe_int_constant_load (cb, oper, src1, new_src1);
	      lop = Lop_SUB;
	      tahoeop = TAHOEop_SUB;
	    }			/* else */
	  break;

	case L_OPERAND_REGISTER:	/* src0 is label and src1 is reg/mac */
	case L_OPERAND_MACRO:
	  lop = Lop_SUB;
	  tahoeop = TAHOEop_SUB;
	  new_src1 = L_copy_operand (src1);
	  break;

	case L_OPERAND_LABEL:	/* src0 is label and src1 is label */
	  new_src1 = Ltahoe_new_int_reg ();
	  Ltahoe_label_load (cb, oper, src1, new_src1);
	  lop = Lop_SUB;
	  tahoeop = TAHOEop_SUB;
	  break;

	default:
	  L_punt ("Unexpected operand src1 type: %d\n in oper %d", src0->type,
		  oper->id);
	  return;
	}			/* switch */
      break;

    default:
      L_punt ("Unexpected operand src0 type: %d\n in oper %d", src0->type,
	      oper->id);
      return;
    }				/* switch */

  /* new_sources have already been copied */

  new_oper = L_create_new_op_using (lop, oper);
  new_oper->proc_opc = tahoeop;
  new_oper->src[0] = new_src0;
  new_oper->src[1] = new_src1;
  new_oper->dest[0] = L_copy_operand (oper->dest[0]);
  L_insert_oper_before (cb, oper, new_oper);
  return;
}				/* L_annotate_int_sub */


/*
 * L_annotate_abs(L_Cb *, L_Oper *)
 * ----------------------------------------------------------------------
 * (pg) r2 = Lop_ABS r1          (pg) mov         r2 = r1
 *                               (pg) cmp4.lt.unc p1,p0 = r1, r0
 *                               (p1) sub         r2 = r0, r1
 */
/* 09/11/02 REK Modifying function so it uses new TAHOEops. */
static void
L_annotate_abs (L_Cb * cb, L_Oper * oper)
{
  L_Oper *new_oper0, *new_oper1, *new_oper2;

  L_Operand *pred = oper->pred[0], *src = oper->src[0], *dest = oper->dest[0];

  new_oper0 = L_create_new_op (Lop_MOV);
  new_oper0->proc_opc = TAHOEop_MOV_GR;
  new_oper0->pred[0] = pred ? L_copy_operand (pred) : NULL;
  new_oper0->dest[0] = L_copy_operand (dest);
  new_oper0->src[0] = L_copy_operand (src);

  new_oper1 = L_create_new_op (Lop_CMP);
  new_oper1->proc_opc = TAHOEop_CMP;
  new_oper1->completers |= TC_CMP_4;
  TC_SET_CMP_OP (new_oper1->completers, TC_CMP_OP_LT);
  TC_SET_CMP_TYPE (new_oper1->completers, TC_CMP_TYPE_UNC);
  L_set_compare (new_oper1, L_CTYPE_INT, Lcmp_COM_LT);
  new_oper1->pred[0] = pred ? L_copy_operand (pred) : NULL;
  new_oper1->dest[0] = Ltahoe_new_pred_reg (L_PTYPE_UNCOND_T);
  new_oper1->dest[1] = Ltahoe_true_pred (L_PTYPE_UNCOND_F);
  new_oper1->src[0] = L_copy_operand (src);
  new_oper1->src[1] = Ltahoe_IMAC (ZERO);

  new_oper2 = L_create_new_op (Lop_SUB);
  new_oper2->proc_opc = TAHOEop_SUB;
  new_oper2->pred[0] = pred ? L_copy_operand (pred) : NULL;
  new_oper2->src[0] = Ltahoe_IMAC (ZERO);
  new_oper2->src[1] = L_copy_operand (src);
  new_oper2->dest[0] = L_copy_operand (dest);
  new_oper2->pred[0] = L_copy_operand (new_oper1->dest[0]);
  L_assign_ptype_null (new_oper2->pred[0]);

  L_insert_oper_before (cb, oper, new_oper0);
  L_insert_oper_before (cb, oper, new_oper1);
  L_insert_oper_before (cb, oper, new_oper2);
}				/* L_annotate_abs */

static void
L_annotate_shladd (L_Cb * cb, L_Oper * oper)
{
  ITintmax int_val;

  /* For now, this is a special case for Vulcanized code. */

  if (!L_is_register (oper->dest[0]) && !L_is_macro (oper->dest[0]))
    {
      L_print_oper (stderr, oper);
      L_punt
	("L_annotate_shladd: First dest expected to be register or macro\n");
    }				/* if */
  if (!L_is_register (oper->src[0]) && !L_is_macro (oper->src[0]))
    {
      L_print_oper (stderr, oper);
      L_punt
	("L_annotate_shladd: First source expected to be register or macro\n");
    }				/* if */
  if (!L_is_int_constant (oper->src[1]))
    {
      L_print_oper (stderr, oper);
      L_punt
	("L_annotate_shladd: Second source expected to be int constant\n");
    }				/* if */
  if (!L_is_register (oper->src[2]) && !L_is_macro (oper->src[2]))
    {
      L_print_oper (stderr, oper);
      L_punt
	("L_annotate_shladd: Third source expected to be register or macro\n");
    }				/* if */

  int_val = oper->src[1]->value.i;
  if ((int_val >= 0) && (int_val <= 4))
    {
      L_Oper *shladd_oper;
      shladd_oper = L_create_new_op_using (Lop_LSLADD, oper);
      shladd_oper->proc_opc = TAHOEop_SHLADD;
      shladd_oper->dest[0] = L_copy_operand (oper->dest[0]);
      shladd_oper->src[0] = L_copy_operand (oper->src[0]);
      shladd_oper->src[1] = L_copy_operand (oper->src[1]);
      shladd_oper->src[2] = L_copy_operand (oper->src[2]);
      L_insert_oper_before (cb, oper, shladd_oper);
    }				/* if */
  else
    {
      L_print_oper (stderr, oper);
      L_punt ("L_annotate_shladd: Shift amount must be 1, 2, 3, or 4.\n");
    }				/* else */

  return;
}				/* L_annotate_shladd */


/* 09/11/02 REK Modifying function to use new TAHOEops. */
static void
L_annotate_extract_oper (L_Cb * cb, L_Oper * oper)
{
  ITintmax int_pos, int_len;

  /* For now, this is a special case for Vulcanized code. */

  if (!L_is_register (oper->dest[0]) && !L_is_macro (oper->dest[0]))
    {
      L_print_oper (stderr, oper);
      L_punt
	("L_annotate_extract: First dest expected to be register or macro\n");
    }				/* if */
  if (!L_is_register (oper->src[0]) && !L_is_macro (oper->src[0]))
    {
      L_print_oper (stderr, oper);
      L_punt
	("L_annotate_extract: First source expected to be register or macro\n");
    }				/* if */
  if (!L_is_int_constant (oper->src[1]))
    {
      L_print_oper (stderr, oper);
      L_punt
	("L_annotate_extract: Second source expected to be int constant\n");
    }				/* if */
  if (!L_is_int_constant (oper->src[2]))
    {
      L_print_oper (stderr, oper);
      L_punt
	("L_annotate_extract: Third source expected to be int constant\n");
    }				/* if */

  int_pos = oper->src[1]->value.i;
  int_len = oper->src[2]->value.i;

  if ((int_pos >= 0) && (int_pos <= 63) && (int_len >= 1) && (int_len <= 64))
    {
      L_Oper *extr_oper;
      if (oper->opc == Lop_EXTRACT_U)
	{
	  extr_oper = L_create_new_op_using (Lop_EXTRACT_U, oper);
	  extr_oper->proc_opc = TAHOEop_EXTR_U;
	}			/* if */
      else
	{
	  extr_oper = L_create_new_op_using (Lop_EXTRACT, oper);
	  extr_oper->proc_opc = TAHOEop_EXTR;
	}			/* else */
      extr_oper->dest[0] = L_copy_operand (oper->dest[0]);
      extr_oper->src[0] = L_copy_operand (oper->src[0]);
      extr_oper->src[1] = L_copy_operand (oper->src[1]);
      extr_oper->src[2] = L_copy_operand (oper->src[2]);
      L_insert_oper_before (cb, oper, extr_oper);
    }				/* if */
  else
    {
      L_print_oper (stderr, oper);
      L_punt ("L_annotate_extract: 0 <= pos <= 63, 1 <= len <= 64.\n");
    }				/* else */

  return;
}				/* L_annotate_extract_oper */


/* 09/11/02 REK Modifying function to support new TAHOEops. */
static void
L_annotate_deposit_oper (L_Cb * cb, L_Oper * oper)
{
  ITintmax int_pos, int_len, int_imm = 0;
  int imm_form, zero_form;
  L_Oper *depo_oper;

  if (!L_is_variable (oper->dest[0]))
    {
      L_print_oper (stderr, oper);
      L_punt ("L_annotate_deposit: "
	      "First dest expected to be register or macro\n");
    }				/* if */

  if (L_is_variable (oper->src[0]))
    {
      imm_form = 0;
    }				/* if */
  else if (L_is_int_constant (oper->src[0]))
    {
      imm_form = 1;
      int_imm = oper->src[0]->value.i;
    }				/* else if */
  else
    {
      L_print_oper (stderr, oper);
      L_punt ("L_annotate_deposit_oper: first src not reg or imm");
      return;
    }				/* else */

  if (L_is_variable (oper->src[1]))
    {
      zero_form = 0;

      if (!L_is_int_constant (oper->src[2]) ||
	  !L_is_int_constant (oper->src[3]))
	{
	  L_print_oper (stderr, oper);
	  L_punt ("L_annotate_deposit_oper: third or fourth src not imm");
	}			/* if */

      int_pos = oper->src[2]->value.i;
      int_len = oper->src[3]->value.i;

      if (!UIMM_4 (int_len - 1))
	{
	  L_print_oper (stderr, oper);
	  L_punt ("L_annotate_deposit_oper: len limited to 1..16");
	}			/* if */

      if (imm_form && ((int_imm < 0) || (int_imm > 1)))
	{
	  L_print_oper (stderr, oper);
	  L_punt ("L_annotate_deposit_oper: imm not imm1");
	}			/* if */
    }				/* if */
  else if (L_is_int_constant (oper->src[1]))
    {
      zero_form = 1;

      if (!L_is_int_constant (oper->src[2]))
	{
	  L_print_oper (stderr, oper);
	  L_punt ("L_annotate_deposit_oper: third src not imm");
	}			/* if */

      int_pos = oper->src[1]->value.i;
      int_len = oper->src[2]->value.i;

      if (!UIMM_6 (int_len - 1))
	{
	  L_print_oper (stderr, oper);
	  L_punt ("L_annotate_deposit_oper: len limited to 1..64");
	}			/* if */

      if (imm_form && !SIMM_8 (int_imm))
	{
	  L_print_oper (stderr, oper);
	  L_punt ("L_annotate_deposit_oper: imm not imm8");
	}			/* if */
    }				/* else if */
  else
    {
      L_print_oper (stderr, oper);
      L_punt ("L_annotate_deposit_oper: second src not reg or imm");
      return;
    }				/* else */

  if (!UIMM_6 (int_pos))
    {
      L_print_oper (stderr, oper);
      L_punt ("L_annotate_deposit_oper: pos limited to 0..63");
    }				/* if */

  depo_oper = L_create_new_op_using (Lop_DEPOSIT, oper);

  depo_oper->proc_opc = zero_form ? TAHOEop_DEP_Z : TAHOEop_DEP;

  depo_oper->dest[0] = L_copy_operand (oper->dest[0]);
  depo_oper->src[0] = L_copy_operand (oper->src[0]);
  depo_oper->src[1] = L_copy_operand (oper->src[1]);
  depo_oper->src[2] = L_copy_operand (oper->src[2]);
  if (zero_form == 0)
    depo_oper->src[3] = L_copy_operand (oper->src[3]);

  L_insert_oper_before (cb, oper, depo_oper);

  return;
}				/* L_annotate_deposit_oper */

/* Ltahoe_annotate_shift
 * ----------------------------------------------------------------------
 * Annotates Lop(s): Lop_ASR, Lop_LSR, Lop_SHL
 *
 * (1) shr   r1 = r3, count6 ==> extr   r1 = r3, count6, 64-count6    (I11)
 * (2) shr.u r1 = r3, count6 ==> extr.u r1 = r3, count6, 64-count6    (I11)
 * (3) shl   r1 = r2, count6 ==> dep.z  r1 = r2, count6, 64-count6    (I12)
 *               count6 <= 4 ==> shladd r1 = r2, count2, r0           (A2)
 * (4) shr   r1 = r3, r2                                              (I5)
 * (5) shr.u r1 = r3, r2                                              (I5)
 * (6) shl   r1 = r2, r3                                              (I7)
 */
/* 09/11/02 REK Modifying function to use new TAHOEops. */
static void
Ltahoe_annotate_shift (L_Cb * cb, L_Oper * oper)
{
  int lop, top = 0;
  L_Oper *new_oper;
  ITintmax int_val;

  lop = oper->opc;

  if (lop != Lop_ASR && lop != Lop_LSR && lop != Lop_LSL)
    L_punt ("Ltahoe_annotate_shift: unexpected opc %d", lop);

  new_oper = L_create_new_op_using (lop, oper);
  new_oper->dest[0] = L_copy_operand (oper->dest[0]);

  if (L_is_int_constant (oper->src[0]))
    {
      new_oper->src[0] = Ltahoe_new_int_reg ();
      Ltahoe_int_constant_load (cb, oper, oper->src[0], new_oper->src[0]);
    }				/* if */
  else if (L_is_variable (oper->src[0]))
    {
      new_oper->src[0] = L_copy_operand (oper->src[0]);
    }				/* else if */
  else if (oper->src[0]->type == L_OPERAND_LABEL)
    {
      L_Operand *new_src0;

      new_src0 = Ltahoe_new_int_reg ();
      Ltahoe_label_load (cb, oper, oper->src[0], new_src0);
      new_oper->src[0] = new_src0;
    }				/* else if */
  else
    {
      L_punt ("Ltahoe_annotate_shift: Bad src[0] on oper %d", oper->id);
    }				/* else */

  if (L_is_int_constant (oper->src[1]))
    {
      int_val = oper->src[1]->value.i;
      if (int_val < 0)
	L_punt ("Ltahoe_annotate_shift: non-negative shift required");
      if (UIMM_6 (int_val))
	{
	  new_oper->src[1] = L_copy_operand (oper->src[1]);
	  switch (lop)
	    {
	    case Lop_ASR:
	      lop = Lop_EXTRACT;
	      top = TAHOEop_EXTR;
	      new_oper->src[2] = L_new_gen_int_operand (64 - int_val);
	      break;
	    case Lop_LSR:
	      lop = Lop_EXTRACT_U;
	      top = TAHOEop_EXTR_U;
	      new_oper->src[2] = L_new_gen_int_operand (64 - int_val);
	      break;
	    case Lop_LSL:
	      if ((int_val >= 1) && (int_val <= 4))
		{
		  top = TAHOEop_SHLADD;
		  new_oper->src[2] = Ltahoe_IMAC (ZERO);
		}		/* if */
	      else
		{
		  lop = Lop_DEPOSIT;
		  top = TAHOEop_DEP_Z;
		  new_oper->src[2] = L_new_gen_int_operand (64 - int_val);
		}		/* else */
	      break;
	    }			/* switch */
	}			/* if */
      else
	{
	  new_oper->src[1] = Ltahoe_new_int_reg ();
	  Ltahoe_int_constant_load (cb, oper, oper->src[1], new_oper->src[1]);
	}			/* else */
    }				/* if */
  else if (L_is_variable (oper->src[1]))
    {
      new_oper->src[1] = L_copy_operand (oper->src[1]);
    }				/* else if */
  else
    L_punt ("Ltahoe_annotate_shift: Bad src[1] on oper %d", oper->id);

  if (!top)
    {
      /* No special cases applied -- default to 4, 5, or 6 */

      switch (lop)
	{
	case Lop_ASR:
	  top = TAHOEop_SHR;
	  break;
	case Lop_LSR:
	  top = TAHOEop_SHR_U;
	  break;
	case Lop_LSL:
	  top = TAHOEop_SHL;
	  break;
	}			/* switch */
    }				/* if */

  L_change_opcode (new_oper, lop);
  new_oper->proc_opc = top;

  L_insert_oper_before (cb, oper, new_oper);

  return;
}				/* Ltahoe_annotate_shift */

static void
L_annotate_check (L_Cb * cb, L_Oper * oper)
{
  L_Oper *new_oper;

  if (!L_is_variable (oper->src[0]))
    {
      L_warn ("Check without a register or macro source");
      return; 
    }
  new_oper = L_create_new_op_using (Lop_CHECK, oper);
  new_oper->proc_opc = S_machine_check (oper);
  new_oper->src[0] = L_copy_operand (oper->src[0]);
  new_oper->src[1] = L_copy_operand (oper->src[1]);
  L_insert_oper_before (cb, oper, new_oper);

  return;
}				/* L_annotate_check */

/* Builtin annotation
 * ----------------------------------------------------------------------
 */
/* 09/11/02 REK Modifying function to use new TAHOEops. */
static void
L_annotate_alloca (L_Func * fn, L_Cb * cb, L_Oper * oper, int outparam_space)
{
  L_Oper *sub_oper, *mask_oper, *add_oper;
  L_Attr *attr;

  sub_oper = L_create_new_op_using (Lop_SUB, oper);
  sub_oper->proc_opc = TAHOEop_SUB;
  sub_oper->dest[0] = L_new_macro_operand (L_MAC_P8,
					   L_CTYPE_LLONG, L_PTYPE_NULL);
  sub_oper->src[0] = L_new_macro_operand (L_MAC_OP,
					  L_CTYPE_LLONG, L_PTYPE_NULL);
  sub_oper->src[1] = L_new_macro_operand (L_MAC_P8,
					  L_CTYPE_LLONG, L_PTYPE_NULL);

  if ((attr = L_find_attr (sub_oper->attr, "tr")))
    sub_oper->attr = L_delete_attr (sub_oper->attr, attr);
  if ((attr = L_find_attr (sub_oper->attr, "ret")))
    sub_oper->attr = L_delete_attr (sub_oper->attr, attr);
  if ((attr = L_find_attr (sub_oper->attr, "param_size")))
    sub_oper->attr = L_delete_attr (sub_oper->attr, attr);
  if ((attr = L_find_attr (sub_oper->attr, "call_info")))
    sub_oper->attr = L_delete_attr (sub_oper->attr, attr);
  attr = L_new_attr ("ALLOCA", 0);
  sub_oper->attr = L_concat_attr (sub_oper->attr, attr);

  mask_oper = L_create_new_op_using (Lop_AND, sub_oper);
  mask_oper->proc_opc = TAHOEop_AND;
  mask_oper->dest[0] = L_new_macro_operand (L_MAC_OP,
					    L_CTYPE_LLONG, L_PTYPE_NULL);
  mask_oper->src[0] = L_new_gen_int_operand (-16);
  mask_oper->src[1] = L_new_macro_operand (L_MAC_P8,
					   L_CTYPE_LLONG, L_PTYPE_NULL);

  add_oper = L_create_new_op_using (Lop_ADD, sub_oper);
  add_oper->proc_opc = TAHOEop_ADDS;
  add_oper->dest[0] = L_new_macro_operand (L_MAC_P16,
					   L_CTYPE_LLONG, L_PTYPE_NULL);
  add_oper->src[0] = L_new_gen_int_operand (16 + outparam_space);
  add_oper->src[1] = L_new_macro_operand (L_MAC_OP,
					  L_CTYPE_LLONG, L_PTYPE_NULL);

  L_insert_oper_before (cb, oper, sub_oper);
  L_insert_oper_before (cb, oper, mask_oper);
  L_insert_oper_before (cb, oper, add_oper);

  if (!(attr = L_find_attr (fn->attr, "ALLOCA")))
    {
      attr = L_new_attr ("ALLOCA", 0);
      fn->attr = L_concat_attr (fn->attr, attr);
    }				/* if */

  L_set_attr_field (attr, attr->max_field,
		    L_new_gen_int_operand (sub_oper->id));

  return;
}				/* L_annotate_alloca */

/*****************************************************************************\
 * CONTROL FLOW                                                              *
 \****************************************************************************/

/* 09/11/02 REK Modifying function to use new TAHOEops. */
static void
L_annotate_jsr (L_Func * fn, L_Cb * cb, L_Oper * oper)
{
  L_Oper *jsr_oper = NULL, *add_oper;
  L_Oper *mov_oper, *ld_oper1, *ld_oper2, *dummy_oper;
  L_Oper *ap_mov_oper;
  L_Attr *attr;

  /*
   * Ltahoe intrinsics
   * ----------------------------------------------------------------------
   * alloca -- allocate variable data space in the stack
   */

  if (L_is_label (oper->src[0]))
    {
      char *target = oper->src[0]->value.l;
      if (!strcmp (target, "_$fn_alloca") ||
          !strcmp (target, "_$fn___builtin_alloca") ||
	  !strcmp (target, "___builtin_alloca"))
	{
	  L_annotate_alloca (fn, cb, oper, fn->s_param);
	  return;
	}			/* if */
      else if (!strcmp (target, "___builtin_memcpy") ||
	       !strcmp (target, "_$fn___builtin_memcpy"))
	{
	  L_delete_operand (oper->src[0]);
	  oper->src[0] = L_new_label_operand ("_$fn_memcpy",
					      L_CTYPE_GLOBAL_ABS);
	}
      else if (!strcmp (target, "_IMPACT_block_mov"))
	{
	  /* Replace an _IMPACT_block_mov with a memcpy */
	  /* For the _IMPACT_block_mov,                 */
	  /*  $P0 = dest address                        */
	  /*  $P1 = src address                         */
	  /*  $P2 = size in bytes                       */
	  /*  $P3 = alignment                           */

	  L_delete_operand (oper->src[0]);
	  oper->src[0] = L_new_label_operand ("_$fn_memcpy",
					      L_CTYPE_GLOBAL_ABS);

	  L_warn ("L_annotate_jsr: annotating an IMPACT_block_mov");
	}			/* else if */
      else if (!strcmp (target, "_$fn_abs"))
	{
	  L_Oper *abs_oper;

	  abs_oper = L_create_new_op (Lop_ABS);
	  abs_oper->pred[0] = L_copy_operand (oper->pred[0]);
	  abs_oper->src[0] = L_new_macro_operand (L_MAC_P8, L_CTYPE_LLONG,
						  L_PTYPE_NULL);
	  abs_oper->dest[0] = L_new_macro_operand (L_MAC_P16, L_CTYPE_LLONG,
						   L_PTYPE_NULL);
	  L_insert_oper_before (cb, oper, abs_oper);
	  L_annotate_abs (cb, abs_oper);
	  L_delete_oper (cb, abs_oper);
	  return;
	}
      else if (!strncmp (target, "___builtin_", 11) ||
	       !strncmp (target, "_$fn___builtin_", 15))
	{
	  L_punt ("Unimplemented builtin %s", target + 1);
	  return;
	}			/* else if */
    }				/* if */

  if (Ltahoe_add_mov_ap)
    {
      ap_mov_oper = L_create_new_op_using (Lop_MOV, oper);
      ap_mov_oper->proc_opc = TAHOEop_MOV_GR;
      ap_mov_oper->src[0] = L_new_macro_operand (L_MAC_SP, L_CTYPE_LLONG, 0);
      ap_mov_oper->dest[0] = Ltahoe_IMAC (AP);
      L_insert_oper_before (cb, oper, ap_mov_oper);
    }				/* if */

  switch (oper->src[0]->type)
    {
    case L_OPERAND_LABEL:
      /* DIRECT JSR */

      if (L_EXTRACT_BIT_VAL (oper->flags, OP_FLAG_SYNC))
	jsr_oper = L_create_new_op_using (Lop_JSR_ND, oper);
      else
	jsr_oper = L_create_new_op_using (Lop_JSR, oper);

      jsr_oper->proc_opc = TAHOEop_BR_CALL;
      jsr_oper->src[0] = L_copy_operand (oper->src[0]);
      jsr_oper->dest[0] = Ltahoe_BMAC (RETADDR);

      L_insert_oper_before (cb, oper, jsr_oper);
      break;

    case L_OPERAND_REGISTER:
    case L_OPERAND_MACRO:

      /* INDIRECT JSR */

      dummy_oper = L_create_new_op_using (Lop_NO_OP, oper);
      L_insert_oper_before (cb, oper, dummy_oper);
      if ((attr = L_find_attr (dummy_oper->attr, "tr")))
	dummy_oper->attr = L_delete_attr (dummy_oper->attr, attr);
      if ((attr = L_find_attr (dummy_oper->attr, "ret")))
	dummy_oper->attr = L_delete_attr (dummy_oper->attr, attr);
      if ((attr = L_find_attr (dummy_oper->attr, "param_size")))
	dummy_oper->attr = L_delete_attr (dummy_oper->attr, attr);

      /* add rA = 8, src0  // compute address of gp in linkage table */

      add_oper = L_create_new_op_using (Lop_ADD, dummy_oper);
      add_oper->proc_opc = TAHOEop_ADDS;
      add_oper->src[0] = L_new_gen_int_operand (8);
      add_oper->src[1] = L_copy_operand (oper->src[0]);
      add_oper->dest[0] = Ltahoe_new_int_reg ();
      L_insert_oper_before (cb, oper, add_oper);

      /* ld8 rB = [src0]   // load fn's addr from linkage table */

      ld_oper1 = L_create_new_op_using (Lop_LD_Q, dummy_oper);
      ld_oper1->proc_opc = TAHOEop_LD8;
      ld_oper1->src[0] = L_copy_operand (oper->src[0]);
      ld_oper1->dest[0] = Ltahoe_new_int_reg ();
      L_insert_oper_before (cb, oper, ld_oper1);

      /* ld8 rC = [rA]     // load fn's gp from linkage table */

      ld_oper2 = L_create_new_op_using (Lop_LD_Q, dummy_oper);
      ld_oper2->proc_opc = TAHOEop_LD8;
      ld_oper2->src[0] = L_copy_operand (add_oper->dest[0]);
      ld_oper2->dest[0] = Ltahoe_new_int_reg ();
      L_insert_oper_before (cb, oper, ld_oper2);

      /* mov bA = rA   // move fn's addr to branch register */

      mov_oper = L_create_new_op_using (Lop_MOV, dummy_oper);
      mov_oper->proc_opc = TAHOEop_MOV_TOBR;
      mov_oper->src[0] = L_copy_operand (ld_oper1->dest[0]);
      mov_oper->dest[0] = Ltahoe_new_reg (L_CTYPE_BTR);
      L_insert_oper_before (cb, oper, mov_oper);

      /* br.call b0 = bA (rC)   // call subroutine 
       * src[1] stores function's gp, which will be loaded
       * into gp in phase 2.
       */

      if (L_EXTRACT_BIT_VAL (oper->flags, OP_FLAG_SYNC))
	jsr_oper = L_create_new_op_using (Lop_JSR_ND, oper);
      else
	jsr_oper = L_create_new_op_using (Lop_JSR, oper);
      jsr_oper->proc_opc = TAHOEop_BR_CALL;
      jsr_oper->src[0] = L_copy_operand (mov_oper->dest[0]);
      jsr_oper->src[1] = L_copy_operand (ld_oper2->dest[0]);
      jsr_oper->dest[0] = Ltahoe_BMAC (RETADDR);

      L_insert_oper_before (cb, oper, jsr_oper);
      L_delete_oper (cb, dummy_oper);
      break;
    default:
      L_punt ("Unknown jsr source  oper:%d\n", oper->id);
    }				/* switch */

  if (jsr_oper && oper->sync_info)
    {
      jsr_oper->sync_info = L_copy_sync_info (oper->sync_info);
      L_add_to_child_list (oper, jsr_oper);
    }				/* if */

  if (jsr_oper && oper->acc_info)
    jsr_oper->acc_info = L_copy_mem_acc_spec_list (oper->acc_info);

  return;
}				/* L_annotate_jsr */

static void
L_annotate_jump (L_Cb * cb, L_Oper * oper)
{
  L_Oper *jump_oper;

  jump_oper = L_create_new_op_using (Lop_JUMP, oper);
  jump_oper->proc_opc = TAHOEop_BR_COND;
  jump_oper->src[0] = L_copy_operand (oper->src[0]);
  L_insert_oper_before (cb, oper, jump_oper);

  return;
}				/* L_annotate_jump */


static void
L_annotate_br_indir (L_Cb *cb, L_Oper *oper)
{
  L_Oper *br_oper, *mov_br_oper;
  L_Operand *btr;

  br_oper = L_create_new_op_using (Lop_JUMP_RG, oper);
  br_oper->proc_opc = TAHOEop_BR_COND;

  mov_br_oper = L_create_new_op (Lop_MOV);
  mov_br_oper->proc_opc = TAHOEop_MOV_TOBR;
  if (oper->pred[0])
    mov_br_oper->pred[0] = L_copy_operand (oper->pred[0]);
  mov_br_oper->src[0] = L_copy_operand (oper->src[0]);
  mov_br_oper->dest[0] = btr = Ltahoe_new_reg (L_CTYPE_BTR);
  br_oper->src[0] = L_copy_operand (btr);
  L_insert_oper_before (cb, oper, mov_br_oper);
  L_insert_oper_before (cb, oper, br_oper);
  return;
}

static void
L_annotate_rts (L_Cb * cb, L_Oper * oper)
{
  L_Oper *rts_oper;

  rts_oper = L_create_new_op_using (Lop_RTS, oper);
  rts_oper->proc_opc = TAHOEop_BR_RET;
  rts_oper->src[0] = Ltahoe_BMAC (RETADDR);
  L_insert_oper_before (cb, oper, rts_oper);

  if (oper->sync_info != NULL)
    {
      rts_oper->sync_info = L_copy_sync_info (oper->sync_info);
      L_add_to_child_list (oper, rts_oper);
    }				/* if */

  if (oper->acc_info)
    rts_oper->acc_info = L_copy_mem_acc_spec_list (oper->acc_info);

  return;
}				/* L_annotate_rts */

/*****************************************************************************\
 * PREDICATION                                                               *
 \****************************************************************************/

/*
 * Ltahoe_pred_init(L_Oper *using, L_Operand *dest0, L_Operand *dest1)
 * ------------------------------------------------------------------------
 * Generate a predicate initializing instruction.
 * dest0 is an initialization to 1; dest1 is an initialization to 0.
 * NULL dests are replaced by the Ltahoe true predicate macro
 * JWS 20010117
 */
/* 09/11/02 REK Modifying function to use new TAHOEops. */
L_Oper *
Ltahoe_pred_init (L_Oper * using, L_Operand * dest0, L_Operand * dest1)
{
  L_Oper *oper;

  if (using)
    oper = L_create_new_op_using (Lop_CMP, using);
  else
    oper = L_create_new_op (Lop_CMP);
  oper->src[0] = L_new_gen_int_operand (0);
  oper->src[1] = Ltahoe_IMAC (ZERO);
  L_set_compare (oper, L_CTYPE_INT, Lcmp_COM_EQ);
  oper->proc_opc = TAHOEop_CMP;
  oper->completers |= TC_CMP_4;
  TC_SET_CMP_OP (oper->completers, TC_CMP_OP_EQ);
  TC_SET_CMP_TYPE (oper->completers, TC_CMP_TYPE_UNC);

  oper->dest[0] = dest0 ? L_copy_operand (dest0) :
    Ltahoe_true_pred (L_PTYPE_NULL);
  oper->dest[0]->ptype = L_PTYPE_UNCOND_T;
  oper->dest[1] = dest1 ? L_copy_operand (dest1) :
    Ltahoe_true_pred (L_PTYPE_NULL);
  oper->dest[1]->ptype = L_PTYPE_UNCOND_F;

  return oper;
}

/*
 * Ltahoe_annotate_pred_init(L_Cb *cb, L_Oper *oper) (DEPRECATED)
 * ------------------------------------------------------------------------
 * Annotate a predicate initializing instruction.
 */
static void
Ltahoe_annotate_pred_init (L_Cb * cb, L_Oper * oper)
{
  L_Oper *cmp_oper = NULL;
  L_Attr *pred_attr;

  if (oper->opc == Lop_PRED_CLEAR)
    {
      cmp_oper = Ltahoe_pred_init (oper, NULL, oper->dest[0]);
      if (oper->dest[1] != NULL)
	L_punt ("Two predicate clear oper:%d\n", oper->id);
    }
  else if (oper->opc == Lop_PRED_SET)
    {
      cmp_oper = Ltahoe_pred_init (oper, oper->dest[0], NULL);
      if (oper->dest[1] != NULL)
	L_punt ("Two predicate clear oper:%d\n", oper->id);
    }
  else if (oper->opc == Lop_PRED_COPY)
    {
      cmp_oper = Ltahoe_pred_init (oper, oper->dest[0], NULL);
      if (oper->dest[1] != NULL)
	L_punt ("Two predicate copy oper:%d\n", oper->id);
    }
  else
    {
      L_punt ("Ltahoe_annotate_pred_init: Unknown lop %d  oper: %d\n",
	      oper->opc, oper->id);
    }

  pred_attr = L_new_attr ("pred_init", 0);
  if (cmp_oper)
    {
      cmp_oper->attr = L_concat_attr (cmp_oper->attr, pred_attr);
      L_insert_oper_before (cb, oper, cmp_oper);
    }

  return;
}



/* Ltahoe_annotate_cmp
 * ----------------------------------------------------------------------
 * Annotates Lop(s): Lop_CMP
 *
 * (1) cmp   p1,p2 = r2, r3      (register form)                      (A6)
 * (2) cmp   p1,p2 = imm8, r3    (imm8 form)                          (A8)
 * (3) cmp   p1,p2 = r0, r3      (parallel inequality form)           (A7)
 * (4) tbit  p1,p2 = r3, pos6                                         (I16)
 *
 * Assumes the compare is already in "pseudo-tahoe" format: a proper
 * combination of destination operands and comparison types.
 * JWS 20010117
 */

/* 09/16/02 REK Updating function to use new opcode map and completers
 *              scheme.
 */
static void
Ltahoe_annotate_cmp (L_Cb * cb, L_Oper * oper)
{
  L_Operand *src[2], *new_src[2];
  int ptype0, ptype1;
  L_Operand *dest[2];
  int ctype, com, proc_opc = 0, completer = 0;
  L_Oper *cmp_oper;

  ctype = oper->com[0];

  cmp_oper = L_create_new_op_using (Lop_CMP, oper);
  L_copy_compare (cmp_oper, oper);

  if (L_gen_tz_cmp_opcode (oper) || L_gen_tn_cmp_opcode (oper))
    {
      if (L_is_int_constant (oper->src[0]))
	{
	  L_warn ("Ltahoe_annotate_cmp: Emitting decideable tbit (op %d)",
		  oper->id);
	  new_src[0] = Ltahoe_new_int_reg ();
	  Ltahoe_int_constant_load (cb, oper, oper->src[0], new_src[0]);
	}			/* if */
      else if (L_is_variable (oper->src[0]))
	new_src[0] = L_copy_operand (oper->src[0]);
      else
	L_punt ("Ltahoe_annotate_cmp: tbit must have int/reg as src 0");

      if (L_is_int_constant (oper->src[1]) && UIMM_6 (oper->src[1]->value.i))
	new_src[1] = L_copy_operand (oper->src[1]);
      else
	L_punt ("Ltahoe_annotate_cmp: tbit must have UIMM6 as src 1");
    }				/* if */
  else
    {
      if (Ltahoe_should_swap_operands (oper->src[0], oper->src[1]))
	{
	  src[0] = oper->src[1];
	  src[1] = oper->src[0];
	  L_swap_compare (cmp_oper);
	}			/* if */
      else
	{
	  src[0] = oper->src[0];
	  src[1] = oper->src[1];
	}			/* else */

      switch (src[0]->type)
	{
	case L_OPERAND_REGISTER:
	case L_OPERAND_MACRO:
	  new_src[0] = L_copy_operand (src[0]);
	  break;
	case L_OPERAND_IMMED:
	  if (!L_is_ctype_integer (src[0]))
	    L_punt ("Ltahoe_annotate_cmp: Non-integral immed operand");
	  if (src[0]->value.i == 0)
	    {
	      new_src[0] = Ltahoe_IMAC (ZERO);
	    }			/* if */
	  else if (SIMM_8 (src[0]->value.i))
	    {
	      new_src[0] = L_copy_operand (src[0]);
	      if ((oper->com[0] == L_CTYPE_UINT) && (src[0]->value.i & 0x80))
		{
		  /* 32-bit Sign extend */
		  printf ("CMP UI to NEG: 0x" ITintmaxhexfmt " \n",
			  src[0]->value.i);
		}		/* if */
	    }			/* else if */
	  else
	    {
	      new_src[0] = Ltahoe_new_int_reg ();
	      Ltahoe_int_constant_load (cb, oper, src[0], new_src[0]);
	    }			/* else */
	  break;
	case L_OPERAND_LABEL:
	case L_OPERAND_CB:
	  new_src[0] = Ltahoe_new_int_reg ();
	  Ltahoe_label_load (cb, oper, src[0], new_src[0]);
	  break;
	default:
	  L_punt ("Unknown source type  oper:%d\n", oper->id);
	}			/* switch */

      switch (src[1]->type)
	{
	case L_OPERAND_REGISTER:
	case L_OPERAND_MACRO:
	  new_src[1] = L_copy_operand (src[1]);
	  break;
	case L_OPERAND_IMMED:
	  if (!L_is_ctype_integer (src[1]))
	    L_punt ("Ltahoe_annotate_cmp: Non-integral immed operand");
	  if (src[1]->value.i == 0)
	    {
	      new_src[1] = Ltahoe_IMAC (ZERO);
	    }			/* if */
	  else
	    {
	      new_src[1] = Ltahoe_new_int_reg ();
	      Ltahoe_int_constant_load (cb, oper, src[1], new_src[1]);
	    }			/* else */
	  break;
	case L_OPERAND_LABEL:
	case L_OPERAND_CB:
	  new_src[1] = Ltahoe_new_int_reg ();
	  Ltahoe_label_load (cb, oper, src[1], new_src[1]);
	  break;
	default:
	  L_punt ("Unknown source type  oper:%d\n", oper->id);
	}			/* switch */
    }				/* else */

  cmp_oper->src[0] = new_src[0];
  cmp_oper->src[1] = new_src[1];

  dest[0] = oper->dest[0];
  dest[1] = oper->dest[1];

  /* If one of the dests is a (mac p0), ignore it
   */
  if (L_is_macro (dest[0]) &&
      dest[0]->ctype == L_CTYPE_PREDICATE &&
      dest[0]->value.mac == TAHOE_MAC_PRED_TRUE)
    dest[0] = NULL;

  if (L_is_macro (dest[1]) &&
      dest[1]->ctype == L_CTYPE_PREDICATE &&
      dest[1]->value.mac == TAHOE_MAC_PRED_TRUE)
    dest[1] = NULL;

  if (!dest[0])
    {
      if (dest[1])
	{
	  dest[0] = dest[1];
	  dest[1] = NULL;
	}			/* if */
      else
	{
	  L_warn ("Ltahoe_annotate_cmp: Predicate define with no dests");
	  return;
	}			/* else */
    }				/* if */

  ptype0 = dest[0] ? dest[0]->ptype : L_PTYPE_NULL;
  ptype1 = dest[1] ? dest[1]->ptype : L_PTYPE_NULL;

  if (L_false_ptype (ptype0))
    {
      L_negate_compare (cmp_oper);
      ptype0 = L_opposite_ptype (ptype0);
      ptype1 = L_opposite_ptype (ptype1);
    }				/* if */

  /* Assert: ptype0 is a TRUE PTYPE dest */
  com = cmp_oper->com[1];
  ctype = cmp_oper->com[0];

  switch (ptype0)
    {
    case L_PTYPE_UNCOND_T:
      if ((ptype1 == L_PTYPE_UNCOND_F) || (ptype1 == L_PTYPE_NULL))
	{
	  proc_opc = LT_tahoe_cmp_proc_opc (ctype, com);
	  completer = LT_tahoe_cmp_completer (ctype, com, TC_CMP_TYPE_UNC);
	}			/* if */
      if (ptype1 == L_PTYPE_NULL)
	ptype1 = L_PTYPE_UNCOND_F;
      break;
    case L_PTYPE_COND_T:
      if ((ptype1 == L_PTYPE_COND_F) || (ptype1 == L_PTYPE_NULL))
	{
	  proc_opc = LT_tahoe_cmp_proc_opc (ctype, com);
	  completer = LT_tahoe_cmp_completer (ctype, com, TC_CMP_TYPE_NONE);
	}			/* if */
      if (ptype1 == L_PTYPE_NULL)
	ptype1 = L_PTYPE_COND_F;
      break;
    case L_PTYPE_OR_T:
      if ((ptype1 == L_PTYPE_OR_T) || (ptype1 == L_PTYPE_NULL))
	{
	  proc_opc = LT_tahoe_cmp_proc_opc (ctype, com);
	  completer = LT_tahoe_cmp_completer (ctype, com, TC_CMP_TYPE_OR);
	}			/* if */
      else if (ptype1 == L_PTYPE_AND_F)
	{
	  proc_opc = LT_tahoe_cmp_proc_opc (ctype, com);
	  completer =
	    LT_tahoe_cmp_completer (ctype, com, TC_CMP_TYPE_OR_ANDCM);
	}			/* else if */
      if (ptype1 == L_PTYPE_NULL)
	ptype1 = L_PTYPE_OR_T;
      break;
    case L_PTYPE_AND_T:
      if ((ptype1 == L_PTYPE_AND_T) || (ptype1 == L_PTYPE_NULL))
	{
	  proc_opc = LT_tahoe_cmp_proc_opc (ctype, com);
	  completer = LT_tahoe_cmp_completer (ctype, com, TC_CMP_TYPE_AND);
	}			/* if */
      else if (ptype1 == L_PTYPE_OR_F)
	{
	  proc_opc = LT_tahoe_cmp_proc_opc (ctype, com);
	  completer =
	    LT_tahoe_cmp_completer (ctype, com, TC_CMP_TYPE_AND_ORCM);
	}			/* else if */
      if (ptype1 == L_PTYPE_NULL)
	ptype1 = L_PTYPE_AND_T;
      break;
    default:
      L_punt ("Ltahoe_annotate_cmp: Bad combo");
    }				/* switch */

  if (!proc_opc)
    L_punt ("Ltahoe_annotate_cmp: Bad proc_opc");

  cmp_oper->proc_opc = proc_opc;
  cmp_oper->completers = completer;

  cmp_oper->dest[0] = L_copy_operand (dest[0]);
  cmp_oper->dest[0]->ptype = ptype0;

  if (dest[1])
    {
      cmp_oper->dest[1] = L_copy_operand (dest[1]);
      cmp_oper->dest[1]->ptype = ptype1;
    }				/* if */
  else
    cmp_oper->dest[1] = Ltahoe_true_pred (ptype1);

  L_insert_oper_before (cb, oper, cmp_oper);

  return;
}				/* Ltahoe_annotate_cmp */

static L_Operand *
L_gen_fp_src (L_Cb * cb, L_Oper * oper, L_Operand * src)
{
  if (!src)
    return NULL;

  if (L_is_constant (src))
    {
      if (L_is_ctype_flt (src))
	src = L_float_constant_immed (cb, oper, src);
      else if (L_is_ctype_dbl (src))
	src = L_double_constant_immed (cb, oper, src);
      else
	L_punt ("L_gen_fp_src: unknown fp constant ctype\n");
    }
  else
    {
      src = L_copy_operand (src);
    }

  return src;
}

/* Ltahoe_annotate_fcmp
 * ----------------------------------------------------------------------
 * Annotates Lop(s): Lop_CMP_F
 *
 * (1) fcmp   p1,p2 = f2, f3                                          (F4)
 *
 * Assumes the compare is already in "pseudo-tahoe" format: a proper
 * combination of destination operands and comparison types.
 * JWS 20010117
 */
/* 09/17/02 REK Updating to remove references to functions provided by
 *              Tmdes.
 */
static void
Ltahoe_annotate_fcmp (L_Cb * cb, L_Oper * oper)
{
  int i;
  L_Oper *cmp_oper = NULL;
  L_Operand *dest[2];
  int ptype0, ptype1;

  dest[0] = oper->dest[0];
  dest[1] = oper->dest[1];

  if (!dest[0])
    {
      if (dest[1])
	{
	  dest[0] = dest[1];
	  dest[1] = NULL;
	}			/* if */
      else
	{
	  L_punt ("Ltahoe_annotate_fcmp: Predicate define with no dests");
	}			/* else */
    }				/* if */

  ptype0 = dest[0]->ptype;
  ptype1 = dest[1] ? dest[1]->ptype : L_opposite_ptype (ptype0);

  cmp_oper = L_create_new_op_using (oper->opc, oper);
  L_copy_compare (cmp_oper, oper);

  if (L_false_ptype (ptype0))
    {
      L_negate_compare (cmp_oper);
      ptype0 = L_opposite_ptype (ptype0);
      ptype1 = L_opposite_ptype (ptype1);
    }				/* if */

  for (i = 0; i <= 1; i++)
    cmp_oper->src[i] = L_gen_fp_src (cb, oper, oper->src[i]);

  /* UNC_T / UNC_F ONLY SUPPORTED TYPE */

  if ((ptype0 == L_PTYPE_UNCOND_T) && (ptype1 == L_PTYPE_UNCOND_F))
    {
      cmp_oper->proc_opc = LT_tahoe_cmp_proc_opc (cmp_oper->com[0],
						  cmp_oper->com[1]);
      cmp_oper->completers = LT_tahoe_cmp_completer (cmp_oper->com[0],
						     cmp_oper->com[1],
						     TC_CMP_TYPE_UNC);
    }				/* if */
  else
    {
      L_print_oper (stderr, oper);
      L_punt ("Ltahoe_annotate_fcmp: Invalid floating point combination");
    }				/* else */

  cmp_oper->dest[0] = L_copy_operand (dest[0]);
  cmp_oper->dest[0]->ptype = ptype0;
  if (dest[1])
    {
      cmp_oper->dest[1] = L_copy_operand (dest[1]);
      cmp_oper->dest[1]->ptype = ptype1;
    }				/* if */
  else
    {
      cmp_oper->dest[1] = Ltahoe_true_pred (ptype1);
    }				/* else */
  L_insert_oper_before (cb, oper, cmp_oper);
  return;
}				/* Ltahoe_annotate_fcmp */


/*****************************************************************************/

static L_Operand *
L_eff_addr_calc (L_Cb * cb, L_Oper * oper, L_Operand * src0, L_Operand * src1)
{
  L_Oper *add_oper, *load_oper;
  L_Operand *new_src0 = NULL, *add_src0 = NULL, *add_src1 = NULL, *temp_src;
  int tahoeop = 0;
  L_Attr *attr;

  if (L_is_int_constant (src0) && L_is_int_constant (src1))
    {
      fprintf (stderr, "WARNING: Loading from a constant address  oper:%d\n",
	       oper->id);
      src0->value.i += src1->value.i;
      new_src0 = Ltahoe_new_int_reg ();
      load_oper = Ltahoe_int_constant_load (cb, oper, src0, new_src0);
      attr = L_find_attr (load_oper->attr, "param");
      load_oper->attr = L_delete_attr (load_oper->attr, attr);
      return (new_src0);
    }

  /* Switch order of operands if int is first source */
  if (L_is_int_constant (src0))
    {
      temp_src = src0;
      src0 = src1;
      src1 = temp_src;
    }

  switch (src0->type)
    {
    case L_OPERAND_REGISTER:
    case L_OPERAND_MACRO:
      new_src0 = L_copy_operand (src0);
      break;
    case L_OPERAND_LABEL:
      new_src0 = Ltahoe_new_int_reg ();
      load_oper = Ltahoe_label_load (cb, oper, src0, new_src0);
      attr = L_find_attr (load_oper->attr, "param");
      load_oper->attr = L_delete_attr (load_oper->attr, attr);
      break;
    default:
      L_punt ("Unknown load src0 type  oper:%d\n", oper->id);
    }

  if (src1 == NULL)		/* Only one register source */
    return (new_src0);
  switch (src1->type)
    {

    case L_OPERAND_IMMED:
      if (L_is_zero (src1))
	return (new_src0);
      else if (SIMM_14 (src1->value.i))
	{
	  add_src0 = L_copy_operand (src1);
	  add_src1 = new_src0;
	  tahoeop = TAHOEop_ADDS;
	}
      else
	{
	  add_src0 = new_src0;
	  add_src1 = Ltahoe_new_int_reg ();
	  load_oper = Ltahoe_int_constant_load (cb, oper, src1, add_src1);
	  attr = L_find_attr (load_oper->attr, "param");
	  load_oper->attr = L_delete_attr (load_oper->attr, attr);
	  tahoeop = TAHOEop_ADD;
	}
      break;
    case L_OPERAND_REGISTER:
    case L_OPERAND_MACRO:
      add_src0 = new_src0;
      add_src1 = L_copy_operand (src1);
      tahoeop = TAHOEop_ADD;
      break;
    case L_OPERAND_LABEL:
      add_src0 = new_src0;
      add_src1 = Ltahoe_new_int_reg ();
      load_oper = Ltahoe_label_load (cb, oper, src1, add_src1);
      attr = L_find_attr (load_oper->attr, "param");
      load_oper->attr = L_delete_attr (load_oper->attr, attr);
      tahoeop = TAHOEop_ADD;
      break;
    default:
      L_punt ("Unimplemented args types to load  oper:%d", oper->id);
    }

  add_oper = L_create_new_op_using (Lop_ADD, oper);
  add_oper->proc_opc = tahoeop;
  add_oper->src[0] = add_src0;
  add_oper->src[1] = add_src1;
  add_oper->dest[0] = Ltahoe_new_int_reg ();
  CLEAR_FLAGS (add_oper);
  if ((attr = L_find_attr (add_oper->attr, "param")))
    add_oper->attr = L_delete_attr (add_oper->attr, attr);
  if ((attr = L_find_attr (add_oper->attr, "NCSPEC")))
    add_oper->attr = L_delete_attr (add_oper->attr, attr);
  L_insert_oper_before (cb, oper, add_oper);

  return (L_copy_operand (add_oper->dest[0]));
}


/*
 * L_annotate_ld (L_Cb *cb, L_Oper *oper)
 * ----------------------------------------------------------------------
 * note: Load lops form: ld rx = [ry+rz],rw 
 *                       rx = dest[0],
 *                       ry = src[0] = base,
 *                       rz = src[1] = offset,
 *                       rw = src[2] = post increment,
 *       Load tahoe form: ld rx = [ry], rw
 *                       rx = dest[0],
 *                       ry = src[0] = base,
 *                       ry = dest[1] when post inc is used,
 *                       rw = src[1] = post increment
 */
/* 09/11/02 REK Modifying function to use new TAHOEops. */
static void
L_annotate_ld (L_Cb * cb, L_Oper * oper)
{
  int tahoeop = 0, tahoeCompleters = 0;
  L_Oper *load_oper;		/* the new load oper */
  L_Oper *extend_oper;		/* sign extend if needed */
  L_Operand *load_src;
  int extend_size = 0;
  int signed_oper = TRUE;
  int new_opc, vol;
  load_src = L_eff_addr_calc (cb, oper, oper->src[0], oper->src[1]);
  new_opc = oper->opc;

  /* The completers can be set regardless of the specific LD opcode */
  if ((L_EXTRACT_BIT_VAL (oper->flags, L_OPER_DATA_SPECULATIVE)) &&
      (L_EXTRACT_BIT_VAL (oper->flags, L_OPER_SPECULATIVE)))
    TC_SET_LD_TYPE (tahoeCompleters, TC_LD_TYPE_SA);
  else if ((!L_EXTRACT_BIT_VAL (oper->flags, L_OPER_DATA_SPECULATIVE)) &&
	   (L_EXTRACT_BIT_VAL (oper->flags, L_OPER_SPECULATIVE)))
    TC_SET_LD_TYPE (tahoeCompleters, TC_LD_TYPE_S);
  else if ((L_EXTRACT_BIT_VAL (oper->flags, L_OPER_DATA_SPECULATIVE)) &&
	   (!L_EXTRACT_BIT_VAL (oper->flags, L_OPER_SPECULATIVE)))
    TC_SET_LD_TYPE (tahoeCompleters, TC_LD_TYPE_A);

  vol = L_EXTRACT_BIT_VAL (oper->flags, L_OPER_VOLATILE);

  switch (oper->opc)
    {
    case Lop_LD_UC:
    case Lop_LD_POST_UC:
      tahoeop = !vol ? TAHOEop_LD1 : TAHOEop_LD1_ACQ;
      extend_size = 1;
      signed_oper = FALSE;
      break;

    case Lop_LD_C:
      tahoeop = !vol ? TAHOEop_LD1 : TAHOEop_LD1_ACQ;
      extend_size = 1;
      new_opc = Lop_LD_UC;
      break;

    case Lop_LD_UC2:
    case Lop_LD_POST_UC2:
      tahoeop = !vol ? TAHOEop_LD2 : TAHOEop_LD2_ACQ;
      extend_size = 2;
      signed_oper = FALSE;
      break;

    case Lop_LD_C2:
      tahoeop = !vol ? TAHOEop_LD2 : TAHOEop_LD2_ACQ;
      extend_size = 2;
      new_opc = Lop_LD_UC2;
      break;

    case Lop_LD_UI:
    case Lop_LD_POST_UI:
      tahoeop = !vol ? TAHOEop_LD4 : TAHOEop_LD4_ACQ;
      extend_size = 4;
      signed_oper = FALSE;
      break;

    case Lop_LD_I:
      tahoeop = !vol ? TAHOEop_LD4 : TAHOEop_LD4_ACQ;
      extend_size = 4;
      new_opc = Lop_LD_UI;
      break;

    case Lop_LD_Q:
      tahoeop = !vol ? TAHOEop_LD8 : TAHOEop_LD8_ACQ;
      break;

    case Lop_LD_F:
      if ((L_EXTRACT_BIT_VAL (oper->flags, L_OPER_DATA_SPECULATIVE)) &&
	  (L_EXTRACT_BIT_VAL (oper->flags, L_OPER_SPECULATIVE)))
	tahoeop = TAHOEop_LDFS_A;
      else if ((!L_EXTRACT_BIT_VAL (oper->flags, L_OPER_DATA_SPECULATIVE)) &&
	       (L_EXTRACT_BIT_VAL (oper->flags, L_OPER_SPECULATIVE)))
	tahoeop = TAHOEop_LDFS;
      else if ((L_EXTRACT_BIT_VAL (oper->flags, L_OPER_DATA_SPECULATIVE)) &&
	       (!L_EXTRACT_BIT_VAL (oper->flags, L_OPER_SPECULATIVE)))
	tahoeop = TAHOEop_LDFS_A;
      else
	tahoeop = TAHOEop_LDFS;
      break;

    case Lop_LD_F2:
      if ((L_EXTRACT_BIT_VAL (oper->flags, L_OPER_DATA_SPECULATIVE)) &&
	  (L_EXTRACT_BIT_VAL (oper->flags, L_OPER_SPECULATIVE)))
	tahoeop = TAHOEop_LDFD_A;
      else if ((!L_EXTRACT_BIT_VAL (oper->flags, L_OPER_DATA_SPECULATIVE)) &&
	       (L_EXTRACT_BIT_VAL (oper->flags, L_OPER_SPECULATIVE)))
	tahoeop = TAHOEop_LDFD;
      else if ((L_EXTRACT_BIT_VAL (oper->flags, L_OPER_DATA_SPECULATIVE)) &&
	       (!L_EXTRACT_BIT_VAL (oper->flags, L_OPER_SPECULATIVE)))
	tahoeop = TAHOEop_LDFD_A;
      else
	tahoeop = TAHOEop_LDFD;
      break;

    case Lop_LD_POST_C:
      tahoeop = !vol ? TAHOEop_LD1 : TAHOEop_LD1_ACQ;
      extend_size = 1;
      new_opc = Lop_LD_POST_UC;
      break;

    case Lop_LD_POST_C2:
      tahoeop = !vol ? TAHOEop_LD2 : TAHOEop_LD2_ACQ;
      extend_size = 2;
      new_opc = Lop_LD_POST_UC2;
      break;

    case Lop_LD_POST_I:
      tahoeop = !vol ? TAHOEop_LD4 : TAHOEop_LD4_ACQ;
      extend_size = 4;
      new_opc = Lop_LD_POST_UI;
      break;

    case Lop_LD_POST_Q:
      tahoeop = !vol ? TAHOEop_LD8 : TAHOEop_LD8_ACQ;
      break;

    case Lop_LD_POST_F:
      fprintf (stderr, "check Fp_single_post_load oper:%d\n", oper->id);
      if ((L_EXTRACT_BIT_VAL (oper->flags, L_OPER_DATA_SPECULATIVE)) &&
	  (L_EXTRACT_BIT_VAL (oper->flags, L_OPER_SPECULATIVE)))
	tahoeop = TAHOEop_LDFS_A;
      else if ((!L_EXTRACT_BIT_VAL (oper->flags, L_OPER_DATA_SPECULATIVE)) &&
	       (L_EXTRACT_BIT_VAL (oper->flags, L_OPER_SPECULATIVE)))
	tahoeop = TAHOEop_LDFS;
      else if ((L_EXTRACT_BIT_VAL (oper->flags, L_OPER_DATA_SPECULATIVE)) &&
	       (!L_EXTRACT_BIT_VAL (oper->flags, L_OPER_SPECULATIVE)))
	tahoeop = TAHOEop_LDFS_A;
      else
	tahoeop = TAHOEop_LDFS;
      break;

    case Lop_LD_POST_F2:
      fprintf (stderr, "check Fp_double_post_load oper:%d\n", oper->id);
      if ((L_EXTRACT_BIT_VAL (oper->flags, L_OPER_DATA_SPECULATIVE)) &&
	  (L_EXTRACT_BIT_VAL (oper->flags, L_OPER_SPECULATIVE)))
	tahoeop = TAHOEop_LDFD_A;
      else if ((!L_EXTRACT_BIT_VAL (oper->flags, L_OPER_DATA_SPECULATIVE)) &&
	       (L_EXTRACT_BIT_VAL (oper->flags, L_OPER_SPECULATIVE)))
	tahoeop = TAHOEop_LDFD;
      else if ((L_EXTRACT_BIT_VAL (oper->flags, L_OPER_DATA_SPECULATIVE)) &&
	       (!L_EXTRACT_BIT_VAL (oper->flags, L_OPER_SPECULATIVE)))
	tahoeop = TAHOEop_LDFD_A;
      else
	tahoeop = TAHOEop_LDFD;
      break;

    case Lop_LD_UC_CHK:
      tahoeop = TAHOEop_LD1_C;
      extend_size = 1;
      signed_oper = FALSE;
      break;

    case Lop_LD_C_CHK:
      tahoeop = TAHOEop_LD1_C;
      extend_size = 1;
      new_opc = Lop_LD_UC_CHK;
      break;

    case Lop_LD_UC2_CHK:
      tahoeop = TAHOEop_LD2_C;
      extend_size = 2;
      signed_oper = FALSE;
      break;

    case Lop_LD_C2_CHK:
      tahoeop = TAHOEop_LD2_C;
      extend_size = 2;
      new_opc = Lop_LD_UC2_CHK;
      break;

    case Lop_LD_UI_CHK:
      tahoeop = TAHOEop_LD4_C;
      extend_size = 4;
      signed_oper = FALSE;
      break;

    case Lop_LD_I_CHK:
      tahoeop = TAHOEop_LD4_C;
      extend_size = 4;
      new_opc = Lop_LD_UI_CHK;
      break;

    case Lop_LD_Q_CHK:
      tahoeop = TAHOEop_LD8_C;
      break;

    case Lop_LD_F_CHK:
      tahoeop = TAHOEop_LDFS_C;
      break;

    case Lop_LD_F2_CHK:
      tahoeop = TAHOEop_LDFD_C;
      break;

    default:
      L_punt ("L_annotate_ld: unknown load opcode for op %d", oper->id);
      break;
    }				/* switch */

  load_oper = L_create_new_op_using (new_opc, oper);
  load_oper->proc_opc = tahoeop;
  load_oper->completers = tahoeCompleters;
  load_oper->src[0] = load_src;
  if (oper->src[2])
    {				/* post_incr load */
      load_oper->dest[1] = L_copy_operand (load_oper->src[0]);
      if (L_is_int_constant (oper->src[2]))
	{
	  if (SIMM_9 (oper->src[2]->value.i))
	    {
	      load_oper->src[1] = L_copy_operand (oper->src[2]);
	    }			/* if */
	  else
	    {
	      /* This constant is too big for the post increment arg
	         on the load.  So first put the constant in a register,
	         then use the post inc with reg form of the load */
	      fprintf (stderr, "Warning: Ugly post-incr load  oper:%d\n",
		       oper->id);
	      load_oper->src[1] = Ltahoe_new_int_reg ();
	      Ltahoe_int_constant_load (cb, oper, oper->src[2],
					load_oper->src[1]);
	    }			/* else */
	}			/* if */
      else if (L_is_reg (oper->src[2]))
	{
	  load_oper->src[1] = L_copy_operand (oper->src[2]);
	}			/* else if */
      else
	{
	  L_punt ("Unknown operand type in load - oper: %d\n", oper->id);
	}			/* else */
    }				/* if */

  L_insert_oper_before (cb, oper, load_oper);

  if (oper->sync_info != NULL)
    {
      load_oper->sync_info = L_copy_sync_info (oper->sync_info);
      L_add_to_child_list (oper, load_oper);
    }				/* if */

  if (oper->acc_info)
    load_oper->acc_info = L_copy_mem_acc_spec_list (oper->acc_info);

  /* Don't need to zero-extend loads since 64 bits are always written */
  if (extend_size && signed_oper)
    {
      extend_oper =
	Ltahoe_sign_extend (oper, NULL, oper->dest[0], extend_size);
      load_oper->dest[0] = L_copy_operand (extend_oper->src[0]);
      L_insert_oper_before (cb, oper, extend_oper);
    }				/* if */
  else
    {
      load_oper->dest[0] = L_copy_operand (oper->dest[0]);
    }				/* else */
}				/* L_annotate_ld */


/*
 * L_annotate_st (L_Cb *cb, L_Oper *oper)
 * ----------------------------------------------------------------------
 * note: Store Lops form:  st [rx+ry] = rz,rw 
 *                           rx = src[0] = base,
 *                           ry = src[1] = offset,
 *                           rz = src[2] = value to store
 *                           rw = src[3] = post increment,
 *       Store tahoe form: st [rx] = rz,rw
 *                           rx = src[0] = base,
 *                           rz = src[1] = value,
 *                           rw = dest[0] when post inc is used,
 *                           rw = src[2] = post increment
 */
/* 09/11/02 REK Modifying function to use new TAHOEops. */
static void
L_annotate_st (L_Cb * cb, L_Oper * oper)
{
  int tahoeop = 0, vol;
  L_Oper *store_oper;
  L_Oper *const_oper;
  L_Oper *inc_oper;
  L_Operand *store_targ;
  L_Operand *store_src = NULL;
  L_Operand *post_inc_operand;
  store_targ = L_eff_addr_calc (cb, oper, oper->src[0], oper->src[1]);
  switch (oper->src[2]->type)
    {
    case L_OPERAND_IMMED:
      if (L_is_ctype_integer (oper->src[2]))
	{
	  if (L_is_zero (oper->src[2]))
	    {
	      store_src = Ltahoe_IMAC (ZERO);
	    }			/* if */
	  else
	    {
	      store_src = Ltahoe_new_int_reg ();
	      const_oper = Ltahoe_int_constant_load (cb, oper, oper->src[2],
						     store_src);
	    }			/* else */
	}			/* if */
      else if (L_is_ctype_flt (oper->src[2]))
	{
	  store_src = L_float_constant_immed (cb, oper, oper->src[2]);
	}			/* else if */
      else if (L_is_ctype_dbl (oper->src[2]))
	{
	  store_src = L_double_constant_immed (cb, oper, oper->src[2]);
	}			/* else if */
      else
	{
	  L_punt ("Unknown store source type  oper:%d", oper->id);
	}			/* else */
      break;

    case L_OPERAND_REGISTER:
    case L_OPERAND_MACRO:
      store_src = L_copy_operand (oper->src[2]);
      break;

    case L_OPERAND_LABEL:
      store_src = Ltahoe_new_int_reg ();
      Ltahoe_label_load (cb, oper, oper->src[2], store_src);
      break;

    default:
      L_punt ("Unknown store source type  oper:%d", oper->id);
    }				/* switch */

  vol = L_EXTRACT_BIT_VAL (oper->flags, L_OPER_VOLATILE);

  switch (oper->opc)
    {
    case Lop_ST_C:
    case Lop_ST_POST_C:
      tahoeop = !vol ? TAHOEop_ST1 : TAHOEop_ST1_REL;
      break;

    case Lop_ST_C2:
    case Lop_ST_POST_C2:
      tahoeop = !vol ? TAHOEop_ST2 : TAHOEop_ST2_REL;
      break;

    case Lop_ST_I:
    case Lop_ST_POST_I:
      tahoeop = !vol ? TAHOEop_ST4 : TAHOEop_ST4_REL;
      break;

    case Lop_ST_Q:
      if (L_find_attr (oper->attr, "VIPSPILL"))
	tahoeop = TAHOEop_ST8_SPILL;
      else if (L_EXTRACT_BIT_VAL (oper->flags, L_OPER_VOLATILE))
	tahoeop = TAHOEop_ST8_REL;
      else
	tahoeop = !vol ? TAHOEop_ST8 : TAHOEop_ST8_REL;
      break;

    case Lop_ST_F:
    case Lop_ST_POST_F:
      tahoeop = TAHOEop_STFS;
      break;

    case Lop_ST_F2:
    case Lop_ST_POST_F2:
      tahoeop = TAHOEop_STFD;
      break;

    default:
      L_punt ("L_annotate_st: unknown store opcode for op %d", oper->id);
      break;
    }				/* switch */

  store_oper = L_create_new_op_using (oper->opc, oper);
  store_oper->proc_opc = tahoeop;
  store_oper->src[0] = store_targ;
  store_oper->src[1] = store_src;	/* Already copied */

  if (L_find_attr (oper->attr, "VIPSPILL"))
    store_oper->dest[0] = Ltahoe_IMAC (UNAT);

  if (oper->sync_info != NULL)
    {
      store_oper->sync_info = L_copy_sync_info (oper->sync_info);
      L_add_to_child_list (oper, store_oper);
    }				/* if */

  if (oper->acc_info)
    store_oper->acc_info = L_copy_mem_acc_spec_list (oper->acc_info);

  L_insert_oper_before (cb, oper, store_oper);
  post_inc_operand = oper->src[3];
  if (post_inc_operand)
    {
      if (L_is_int_constant (post_inc_operand))
	{
	  if (SIMM_9 (post_inc_operand->value.i))
	    {
	      /* the base reg will be incremented, making it a dest */
	      store_oper->dest[0] = L_copy_operand (store_targ);
	      store_oper->src[2] = L_copy_operand (post_inc_operand);
	      L_print_oper (stderr, store_oper);
	    }			/* if */
	  else
	    {
	      /* The constant increment on this store is too large to fit
	         into the 9 bit field in the instruction.  Therefore we must
	         break up the store into a ST and an ADD to achieve the post
	         increment functionality */
	      fprintf (stderr, "Warning: Ugly post increment store oper %d\n",
		       oper->id);
	      inc_oper = L_create_new_op_using (Lop_ADD, store_oper);
	      inc_oper->proc_opc = TAHOEop_ADDS;
	      inc_oper->dest[0] = L_copy_operand (store_targ);
	      inc_oper->src[0] = L_copy_operand (store_targ);
	      L_insert_oper_after (cb, store_oper, inc_oper);
	      if (SIMM_14 (post_inc_operand->value.i))
		{
		  inc_oper->src[1] = L_copy_operand (post_inc_operand);
		}		/* if */
	      else
		{
		  /* The constant doesn't even fit in 15 bits.
		     Create a movl and then add it to the base reg */
		  inc_oper->src[1] = Ltahoe_new_int_reg ();
		  const_oper = Ltahoe_int_constant_load (cb, oper,
							 post_inc_operand,
							 inc_oper->src[1]);
		  inc_oper->proc_opc = TAHOEop_ADD;
		  L_print_oper (stderr, const_oper);
		}		/* else */
	      L_print_oper (stderr, store_oper);
	      L_print_oper (stderr, inc_oper);
	    }			/* else */
	}			/* if */
      else
	{
	  L_punt ("Unknown operand type in load - oper: %d\n", oper->id);
	}			/* else */
    }				/* if */
}				/* L_annotate_st */

/******************************************************************************\
 *
 * Floating point Annotation functions
 *
\******************************************************************************/

int
L_dbl_involved_opcode (L_Oper * op)
{
  if (!op)
    return 0;
  if (L_dbl_opcode (op))
    return 1;
  if ((op->opc == Lop_F_F2) || (op->opc == Lop_F2_F) ||
      (op->opc == Lop_F2_I) || (op->opc == Lop_I_F2) || (op->opc == Lop_F_I))
    return 1;
  /* Special case to get rid of double defines */
  if (op->opc == Lop_DEFINE)
    return 1;
  return 0;
}


void
L_convert_double_to_float (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *op;
  L_Attr *attr;
  int i;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (op = cb->first_op; op != NULL; op = op->next_op)
	{
	  /* Change the sources */
	  /* Change the double registers to float registers */
	  for (i = 0; i < L_max_src_operand; i++)
	    {
	      if (L_is_ctype_dbl (op->src[i]))
		{
		  if (L_is_reg (op->src[i]))
		    L_assign_type_float_register (op->src[i]);
		  if (L_is_macro (op->src[i]))
		    L_assign_type_float_macro (op->src[i]);
		}
	    }

	  /* Change the destinations */
	  /* Change the double registers to float registers */
	  for (i = 0; i < L_max_dest_operand; i++)
	    {
	      if (L_is_ctype_dbl (op->dest[i]))
		{
		  if (L_is_reg (op->dest[i]))
		    L_assign_type_float_register (op->dest[i]);
		  if (L_is_macro (op->dest[i]))
		    L_assign_type_float_macro (op->dest[i]);
		}
	    }

	  if (op->opc != Lop_JSR)
	    continue;
	  /* Change the return register macro to float */
	  attr = L_find_attr (op->attr, "ret");
	  if (attr && L_is_ctype_dbl (attr->field[0]))
	    L_assign_ctype (attr->field[0], L_CTYPE_DOUBLE);

	  /* Change the input register macros to float */
	  if ((attr = L_find_attr (op->attr, "tr")) &&
	      L_is_ctype_dbl (attr->field[0]))
	    L_assign_ctype (attr->field[0], L_CTYPE_DOUBLE);

	}
    }

}


/* 09/11/02 REK Modifying function to use new TAHOEops. */
void
L_annotate_float_divide (L_Cb * cb, L_Oper * oper)
{
  L_Oper *new_oper1, *new_oper2, *new_oper3, *new_oper4;
  L_Oper *new_oper5, *new_oper6, *new_oper7, *new_oper8;
  L_Oper *new_oper9, *mov_oper;
  L_Operand *const_operand, *dest_operand;
  L_Operand *fconst_reg_operand;
  L_Operand *fconst_reg_operand0 = NULL;
  L_Operand *fconst_reg_operand1 = NULL;
  L_Operand *fzero, *fone;
  float constant;

  if (!((L_is_ctype_flt (oper->src[0]) || L_is_ctype_dbl (oper->src[0]))
	&& (L_is_ctype_flt (oper->src[1]) || L_is_ctype_dbl (oper->src[1]))))
    L_punt ("L_annotate_float_divide: non-fp src in oper %d", oper->id);

  /* If both sources are constants, do computation at compile time */
  if (L_is_constant (oper->src[0]) && L_is_constant (oper->src[1]))
    {
      constant = (oper->src[0]->value.f) / (oper->src[1]->value.f);
      const_operand = L_new_float_operand (constant);
      fconst_reg_operand = L_float_constant_immed (cb, oper, const_operand);
      L_delete_operand (const_operand);
      mov_oper = L_create_new_op_using (Lop_MOV_F, oper);
      mov_oper->proc_opc = TAHOEop_MOV_FR;
      mov_oper->src[0] = fconst_reg_operand;
      mov_oper->dest[0] = L_copy_operand (oper->dest[0]);
      L_insert_oper_before (cb, oper, mov_oper);
      return;
    }				/* if */

  /* If src0 is constant, move to a register */
  if (L_is_constant (oper->src[0]))
    fconst_reg_operand0 = L_float_constant_immed (cb, oper, oper->src[0]);

  /* If src1 is a constant, multiply by inverse constant */
  if (L_is_constant (oper->src[1]))
    {
      constant = 1 / (oper->src[1]->value.f);	/* mbt 3/7/98 */
      const_operand = L_new_float_operand (constant);
      fconst_reg_operand1 = L_float_constant_immed (cb, oper, const_operand);
      L_delete_operand (const_operand);
      /* Do a multiply instruction */

      new_oper1 = L_create_new_op_using (Lop_MUL_ADD_F, oper);
      new_oper1->proc_opc = TAHOEop_FMA_S;

      if (fconst_reg_operand0)
	new_oper1->src[0] = fconst_reg_operand0;
      else
	new_oper1->src[0] = L_copy_operand (oper->src[0]);
      new_oper1->src[1] = fconst_reg_operand1;
      new_oper1->src[2] = Ltahoe_FMAC (FZERO);
      new_oper1->dest[0] = L_copy_operand (oper->dest[0]);
      L_insert_oper_before (cb, oper, new_oper1);
      return;
    }				/* if */

  /* Resolved to "fr =  fr / fr" case */

  fzero = Ltahoe_FMAC (FZERO);
  fone = Ltahoe_FMAC (FONE);

  if (L_same_operand (oper->dest[0], oper->src[0]) ||
      L_same_operand (oper->dest[0], oper->src[1]))
    {
      /* The srcs and dest are ALL live at the same time during
         the computation sequence. If the dest is the same
         as either of the srcs, we CANNOT overwrite the
         src with the very first frcpa.  Instead, use a temporary
         destination, and create a move at the end of the 
         sequence.  MCM 7/2001 */
      dest_operand = NULL;
    }				/* if */
  else
    {
      dest_operand = oper->dest[0];
    }				/* else */
  
  new_oper1 = Ltahoe_new_frcpa (oper->pred[0],
				dest_operand, NULL, NULL, NULL, FSF_S0, oper);
  
  if (fconst_reg_operand0)
    new_oper1->src[0] = fconst_reg_operand0;
  else
    new_oper1->src[0] = L_copy_operand (oper->src[0]);

  if (fconst_reg_operand1)
    new_oper1->src[1] = fconst_reg_operand1;
  else
    new_oper1->src[1] = L_copy_operand (oper->src[1]);

  L_insert_oper_before (cb, oper, new_oper1);

  new_oper2 = Ltahoe_new_fnma (new_oper1->dest[1],
			       NULL, new_oper1->dest[0], new_oper1->src[1],
			       fone, FSF_S1, FPC_NONE, NULL);
  L_insert_oper_before (cb, oper, new_oper2);

  new_oper3 = Ltahoe_new_fma (new_oper1->dest[1],
			      NULL, new_oper1->src[0], new_oper1->dest[0],
			      fzero, FSF_S1, FPC_NONE, NULL);
  L_insert_oper_before (cb, oper, new_oper3);

  new_oper4 = Ltahoe_new_fma (new_oper1->dest[1],
			      NULL, new_oper2->dest[0], new_oper2->dest[0],
			      fzero, FSF_S1, FPC_NONE, NULL);
  L_insert_oper_before (cb, oper, new_oper4);

  new_oper5 = Ltahoe_new_fma (new_oper1->dest[1],
			      NULL, new_oper3->dest[0], new_oper2->dest[0],
			      new_oper3->dest[0], FSF_S1, FPC_NONE, NULL);
  L_insert_oper_before (cb, oper, new_oper5);

  new_oper6 = Ltahoe_new_fma (new_oper1->dest[1],
			      NULL, new_oper4->dest[0], new_oper4->dest[0],
			      fzero, FSF_S1, FPC_NONE, NULL);
  L_insert_oper_before (cb, oper, new_oper6);

  new_oper7 = Ltahoe_new_fma (new_oper1->dest[1],
			      NULL, new_oper5->dest[0], new_oper4->dest[0],
			      new_oper5->dest[0], FSF_S1, FPC_NONE, NULL);
  L_insert_oper_before (cb, oper, new_oper7);

  new_oper8 = Ltahoe_new_fma (new_oper1->dest[1],
			      NULL, new_oper7->dest[0], new_oper6->dest[0],
			      new_oper7->dest[0], FSF_S1, FPC_D, NULL);
  L_insert_oper_before (cb, oper, new_oper8);

  new_oper9 = Ltahoe_new_fadd (new_oper1->dest[1],
			       new_oper1->dest[0], new_oper8->dest[0],
			       fzero, FSF_S0, FPC_S, NULL);
  L_insert_oper_before (cb, oper, new_oper9);

  if (!L_same_operand (new_oper1->dest[0], oper->dest[0]))
    {
      L_Oper *new_oper14;

      /* We did not use the target destination register.
         Now, move the temp dest into the target dest. */
      new_oper14 = L_create_new_op_using (Lop_MOV_F, oper);
      new_oper14->proc_opc = TAHOEop_MOV_FR;
      new_oper14->dest[0] = L_copy_operand (oper->dest[0]);
      new_oper14->src[0] = L_copy_operand (new_oper1->dest[0]);
      L_insert_oper_before (cb, oper, new_oper14);
    }				/* if */

  L_delete_operand (fzero);
  L_delete_operand (fone);

  return;
}				/* L_annotate_float_divide */


/* 09/11/02 REK Modifying function to use new TAHOEops. */
void
L_annotate_double_divide (L_Cb * cb, L_Oper * oper)
{
  L_Oper *new_oper1, *new_oper2, *new_oper3, *new_oper4;
  L_Oper *new_oper5, *new_oper6, *new_oper7, *new_oper8;
  L_Oper *new_oper9, *new_oper10, *new_oper11, *new_oper12;
  L_Oper *new_oper13, *mov_oper;
  L_Operand *const_operand;
  L_Operand *src0 = NULL;
  L_Operand *src1 = NULL;
  L_Operand *fzero, *fone;
  double constant;

  if (!((L_is_ctype_flt (oper->src[0]) || L_is_ctype_dbl (oper->src[0]))
	&& (L_is_ctype_flt (oper->src[1]) || L_is_ctype_dbl (oper->src[1]))))
    L_punt ("L_annotate_double_divide: non-fp src in oper %d", oper->id);

  /* If both sources are constants, do computation at compile time */
  if (L_is_constant (oper->src[0]) && L_is_constant (oper->src[1]))
    {
      L_Operand *fconst_reg_operand;

      constant = (oper->src[0]->value.f2) / (oper->src[1]->value.f2);
      const_operand = L_new_double_operand (constant);
      fconst_reg_operand = L_double_constant_immed (cb, oper, const_operand);
      L_delete_operand (const_operand);
      mov_oper = L_create_new_op_using (Lop_MOV_F2, oper);
      mov_oper->proc_opc = TAHOEop_MOV_FR;
      mov_oper->src[0] = fconst_reg_operand;
      mov_oper->dest[0] = L_copy_operand (oper->dest[0]);
      L_insert_oper_before (cb, oper, mov_oper);
      return;
    }				/* if */

  src0 = L_gen_fp_src (cb, oper, oper->src[0]);

  /* If src1 is a constant, multiply by inverse constant */
  if (L_is_constant (oper->src[1]))
    {
      constant = 1 / (oper->src[1]->value.f2);
      const_operand = L_new_double_operand (constant);
      src1 = L_double_constant_immed (cb, oper, const_operand);
      L_delete_operand (const_operand);
      /* Do a multiply instruction */
      new_oper1 = L_create_new_op_using (Lop_MUL_ADD_F2, oper);
      new_oper1->proc_opc = TAHOEop_FMA_D;

      new_oper1->src[0] = src0; /* src0 not freed in this case */
      new_oper1->src[1] = src1;
      new_oper1->src[2] = Ltahoe_DMAC (FZERO);

      new_oper1->dest[0] = L_copy_operand (oper->dest[0]);
      L_insert_oper_before (cb, oper, new_oper1);
      return;
    }				/* if */

  /* Now assume register =  register / register */

  fzero = Ltahoe_DMAC(FZERO);
  fone = Ltahoe_DMAC(FONE);

  if (L_same_operand (oper->dest[0], oper->src[0]) ||
      L_same_operand (oper->dest[0], oper->src[1]))
    {
      /* The srcs and dest are ALL live at the same time during
         the computation sequence. If the dest is the same
         as either of the srcs, we CANNOT overwrite the
         src with the very first frcpa.  Instead, use a temporary
         destination, and create a move at the end of the 
         sequence.  MCM 7/2001 */
      
      new_oper1 = Ltahoe_new_frcpa(oper->pred[0], NULL, NULL, src0, 
				   oper->src[1], FSF_S0, oper);
    }				/* if */
  else
    {
      new_oper1 = Ltahoe_new_frcpa(oper->pred[0], oper->dest[0], NULL, src0, 
				   oper->src[1], FSF_S0, oper);
    }				/* else */
  
  L_insert_oper_before (cb, oper, new_oper1);

  new_oper2 = Ltahoe_new_fnma (new_oper1->dest[1],
			       NULL, new_oper1->src[1], new_oper1->dest[0],
			       fone, FSF_S1, FPC_NONE, NULL);
  
  L_insert_oper_before (cb, oper, new_oper2);

  new_oper3 = Ltahoe_new_fma (new_oper1->dest[1],
			      NULL, new_oper1->dest[0], new_oper1->src[0],
			      fzero, FSF_S1, FPC_NONE, NULL);

  L_insert_oper_before (cb, oper, new_oper3);

  new_oper4 = Ltahoe_new_fma (new_oper1->dest[1],
			      NULL, new_oper2->dest[0], new_oper2->dest[0],
			      fzero, FSF_S1, FPC_NONE, NULL);

  L_insert_oper_before (cb, oper, new_oper4);

  new_oper5 = Ltahoe_new_fma (new_oper1->dest[1],
			      NULL, new_oper3->dest[0], new_oper2->dest[0],
			      new_oper3->dest[0], FSF_S1, FPC_NONE, NULL);

  L_insert_oper_before (cb, oper, new_oper5);

  new_oper6 = Ltahoe_new_fma (new_oper1->dest[1],
			      NULL, new_oper1->dest[0], new_oper2->dest[0],
			      new_oper1->dest[0], FSF_S1, FPC_NONE, NULL);

  L_insert_oper_before (cb, oper, new_oper6);

  new_oper7 = Ltahoe_new_fma (new_oper1->dest[1],
			      NULL, new_oper4->dest[0], new_oper4->dest[0],
			      fzero, FSF_S1, FPC_NONE, NULL);
  
  L_insert_oper_before (cb, oper, new_oper7);
  
  new_oper8 = Ltahoe_new_fma (new_oper1->dest[1],
			      NULL, new_oper5->dest[0], new_oper4->dest[0], 
			      new_oper5->dest[0], FSF_S1, FPC_NONE, NULL);
  
  L_insert_oper_before (cb, oper, new_oper8);

  new_oper9 =  Ltahoe_new_fma (new_oper1->dest[1],
			       NULL, new_oper6->dest[0], new_oper4->dest[0],
			       new_oper6->dest[0], FSF_S1, FPC_NONE, NULL);
  
  L_insert_oper_before (cb, oper, new_oper9);
  
  new_oper10 = Ltahoe_new_fma (new_oper1->dest[1],
			       NULL, new_oper8->dest[0], new_oper7->dest[0],
			       new_oper8->dest[0], FSF_S1, FPC_D, NULL);

  L_insert_oper_before (cb, oper, new_oper10);

  new_oper11 = Ltahoe_new_fma (new_oper1->dest[1],
			       NULL, new_oper9->dest[0], new_oper7->dest[0],
			       new_oper9->dest[0], FSF_S1, FPC_NONE, NULL);
  
  L_insert_oper_before (cb, oper, new_oper11);
  
  new_oper12 = Ltahoe_new_fnma (new_oper1->dest[1],
			       NULL, new_oper1->src[1], new_oper10->dest[0],
			       new_oper1->src[0], FSF_S1, FPC_D, NULL);

  L_insert_oper_before (cb, oper, new_oper12);

  new_oper13 = Ltahoe_new_fma (new_oper1->dest[1],
			       new_oper1->dest[0], new_oper12->dest[0], 
			       new_oper11->dest[0], new_oper10->dest[0], 
			       FSF_S0, FPC_D, NULL);

  L_insert_oper_before (cb, oper, new_oper13);

  if (!L_same_operand (new_oper1->dest[0], oper->dest[0]))
    {
      L_Oper *new_oper14;

      /* We did not use the target destination register.
         Now, move the temp dest into the target dest. */
      new_oper14 = L_create_new_op_using (Lop_MOV_F2, oper);
      new_oper14->proc_opc = TAHOEop_MOV_FR;
      new_oper14->dest[0] = L_copy_operand (oper->dest[0]);
      new_oper14->src[0] = L_copy_operand (new_oper1->dest[0]);
      L_insert_oper_before (cb, oper, new_oper14);
    }				/* if */
  
  /* Since these operands have only been copied, they still need
     to be freed */
  L_delete_operand(src0);
  L_delete_operand (fzero);
  L_delete_operand (fone);
}				/* L_annotate_double_divide */

/* 09/11/02 REK Modifying function to use new TAHOEops. */
static void
L_annotate_float_mul_oper (L_Cb * cb, L_Oper * oper, int op)
{
  L_Oper *new_oper1 = NULL, *mov_oper;
  L_Operand *fconst_reg_operand;
  L_Operand *fconst_reg_operand0 = NULL;
  L_Operand *fconst_reg_operand1 = NULL;
  L_Operand *const_operand;
  float constant;
  /* If both sources are constants, do computation at compile time */
  if (L_is_constant (oper->src[0]) && L_is_constant (oper->src[1]))
    {
      fprintf (stderr, "Check constant move  %d\n", oper->id);
      constant = (oper->src[0]->value.f * oper->src[1]->value.f);
      const_operand = L_new_float_operand (constant);
      fconst_reg_operand = L_float_constant_immed (cb, oper, const_operand);
      L_delete_operand (const_operand);
      mov_oper = L_create_new_op_using (Lop_MOV_F, oper);
      mov_oper->proc_opc = TAHOEop_MOV_FR;
      mov_oper->src[0] = fconst_reg_operand;
      mov_oper->dest[0] = L_copy_operand (oper->dest[0]);
      L_insert_oper_before (cb, oper, mov_oper);
      return;
    }				/* if */

  if (L_is_constant (oper->src[0]))
    fconst_reg_operand0 = L_float_constant_immed (cb, oper, oper->src[0]);

  if (L_is_constant (oper->src[1]))
    fconst_reg_operand1 = L_float_constant_immed (cb, oper, oper->src[1]);

  /* Assume new register =  register / register */

  if (op == Lop_MUL_F)
    {
      new_oper1 = L_create_new_op_using (Lop_MUL_ADD_F, oper);
      new_oper1->proc_opc = TAHOEop_FMA_S;
      new_oper1->src[2] = Ltahoe_FMAC (FZERO);
    }				/* if */
  else
    {
      L_punt ("L_annotate_float_mul_oper: no correct mul op, id %d",
	      oper->id);
    }				/* else */
  if (fconst_reg_operand0)
    new_oper1->src[0] = fconst_reg_operand0;
  else
    new_oper1->src[0] = L_copy_operand (oper->src[0]);
  if (fconst_reg_operand1)
    new_oper1->src[1] = fconst_reg_operand1;
  else
    new_oper1->src[1] = L_copy_operand (oper->src[1]);
  new_oper1->dest[0] = L_copy_operand (oper->dest[0]);
  L_insert_oper_before (cb, oper, new_oper1);
}				/* L_annotate_float_mul_oper */

/* 09/11/02 REK Modifying function to use new TAHOEops. */
static void
L_annotate_double_mul_oper (L_Cb * cb, L_Oper * oper, int op)
{
  L_Oper *new_oper1 = NULL, *mov_oper;
  L_Operand *fconst_reg_operand;
  L_Operand *fconst_reg_operand0 = NULL;
  L_Operand *fconst_reg_operand1 = NULL;
  L_Operand *const_operand;
  double constant;
  /* If both sources are constants, do computation at compile time */
  if (L_is_constant (oper->src[0]) && L_is_constant (oper->src[1]))
    {
      fprintf (stderr, "Check constant move  %d\n", oper->id);
      constant = (oper->src[0]->value.f2 * oper->src[1]->value.f2);
      const_operand = L_new_double_operand (constant);
      fconst_reg_operand = L_double_constant_immed (cb, oper, const_operand);
      L_delete_operand (const_operand);
      mov_oper = L_create_new_op_using (Lop_MOV_F2, oper);
      mov_oper->proc_opc = TAHOEop_MOV_FR;
      mov_oper->src[0] = fconst_reg_operand;
      mov_oper->dest[0] = L_copy_operand (oper->dest[0]);
      L_insert_oper_before (cb, oper, mov_oper);
      return;
    }				/* if */

  if (L_is_constant (oper->src[0]))
    fconst_reg_operand0 = L_double_constant_immed (cb, oper, oper->src[0]);

  if (L_is_constant (oper->src[1]))
    fconst_reg_operand1 = L_double_constant_immed (cb, oper, oper->src[1]);

  /* Assume new register =  register / register */

  if (op == Lop_MUL_F2)
    {
      new_oper1 = L_create_new_op_using (Lop_MUL_ADD_F2, oper);
      new_oper1->proc_opc = TAHOEop_FMA_D;
      new_oper1->src[2] = Ltahoe_DMAC (FZERO);
    }				/* if */
  else
    {
      L_punt ("L_annotate_double_mul_oper: no correct mul op, id %d",
	      oper->id);
    }				/* else */
 
  if (fconst_reg_operand0)
    new_oper1->src[0] = fconst_reg_operand0;
  else
    new_oper1->src[0] = L_copy_operand (oper->src[0]);
  if (fconst_reg_operand1)
    new_oper1->src[1] = fconst_reg_operand1;
  else
    new_oper1->src[1] = L_copy_operand (oper->src[1]);
  new_oper1->dest[0] = L_copy_operand (oper->dest[0]);
  L_insert_oper_before (cb, oper, new_oper1);
}				/* L_annotate_double_mul_oper */


/* 09/11/02 REK Modifying function to use new TAHOEops. */
static void
L_annotate_add_sub_oper (L_Cb * cb, L_Oper * oper)
{
  L_Oper *mul_oper = NULL;

  switch (oper->opc)
    {
    case Lop_ADD_F:
      mul_oper = L_create_new_op_using (Lop_MUL_ADD_F, oper);
      mul_oper->proc_opc = TAHOEop_FMA_S;
      mul_oper->src[1] = Ltahoe_FMAC (FONE);
      break;

    case Lop_SUB_F:
      mul_oper = L_create_new_op_using (Lop_MUL_SUB_F, oper);
      mul_oper->proc_opc = TAHOEop_FMS_S;
      mul_oper->src[1] = Ltahoe_FMAC (FONE);
      break;

    case Lop_ADD_F2:
      mul_oper = L_create_new_op_using (Lop_MUL_ADD_F2, oper);
      mul_oper->proc_opc = TAHOEop_FMA_D;
      mul_oper->src[1] = Ltahoe_DMAC (FONE);
      break;

    case Lop_SUB_F2:
      mul_oper = L_create_new_op_using (Lop_MUL_SUB_F2, oper);
      mul_oper->proc_opc = TAHOEop_FMS_D;
      mul_oper->src[1] = Ltahoe_DMAC (FONE);
      break;

    default:
      L_punt ("L_annotate_float_add_sub_oper: no correct op, id %d",
	      oper->id);
    }

  mul_oper->src[0] = L_gen_fp_src (cb, oper, oper->src[0]);
  mul_oper->src[2] = L_gen_fp_src (cb, oper, oper->src[1]);

  mul_oper->dest[0] = L_copy_operand (oper->dest[0]);
  L_insert_oper_before (cb, oper, mul_oper);
}				/* L_annotate_add_sub_oper */


/* 09/11/02 REK Modifying function to use new TAHOEops. */
static void
L_annotate_double_abs (L_Cb * cb, L_Oper * oper, int op)
{
  L_Oper *new_oper;

  new_oper = L_create_new_op_using (op, oper);
  new_oper->src[0] = L_copy_operand (oper->src[0]);
  new_oper->dest[0] = L_copy_operand (oper->dest[0]);
  new_oper->proc_opc = TAHOEop_FABS;
  L_insert_oper_before (cb, oper, new_oper);
}				/* L_annotate_double_abs */

/* 09/11/02 REK Modifying function to use new TAHOEops. */
static void
L_annotate_float_mul_add_sub_oper (L_Cb * cb, L_Oper * oper, int op)
{
  int proc_opc = 0;
  L_Oper *new_oper1;
  L_Operand *src0;
  L_Operand *src1;
  L_Operand *src2;

  src0 = L_gen_fp_src (cb, oper, oper->src[0]);
  src1 = L_gen_fp_src (cb, oper, oper->src[1]);
  src2 = L_gen_fp_src (cb, oper, oper->src[2]);

  /* Assume now register =  register / register */

  switch (op)
    {
    case Lop_MUL_ADD_F:
      proc_opc = TAHOEop_FMA_S;
      break;
    case Lop_MUL_ADD_F2:
      proc_opc = TAHOEop_FMA_D;
      break;
    case Lop_MUL_SUB_F:
      proc_opc = TAHOEop_FMS_S;
      break;
    case Lop_MUL_SUB_F2:
      proc_opc = TAHOEop_FMS_D;
      break;
    case Lop_MUL_SUB_REV_F:
      proc_opc = TAHOEop_FNMA_S;
      break;
    case Lop_MUL_SUB_REV_F2:
      proc_opc = TAHOEop_FNMA_D;
      break;
    default:
      L_punt ("L_annotate_float_mul_add_sub_oper:  error id : %d", oper->id);
    }				/* switch */

  new_oper1 = L_create_new_op_using (op, oper);
  new_oper1->src[0] = src0;
  new_oper1->src[1] = src1;
  new_oper1->src[2] = src2;
  new_oper1->dest[0] = L_copy_operand (oper->dest[0]);
  new_oper1->proc_opc = proc_opc;
 
  L_insert_oper_before (cb, oper, new_oper1);
}				/* L_annotate_float_mul_add_sub_oper */


/* 09/11/02 REK Modifying function to use new TAHOEops. */
static void
L_annotate_double_max_min_oper (L_Cb * cb, L_Oper * oper, int op)
{
  L_Oper *new_oper1;
  int proc_opc = 0;
  fprintf (stderr, "double_max_min -unchecked  oper:%d\n", oper->id);
  switch (op)
    {
    case Lop_MAX_F2:
      proc_opc = TAHOEop_FMAX;
      break;
    case Lop_MIN_F2:
      proc_opc = TAHOEop_FMIN;
      break;
    default:
      L_punt ("L_annotate_double_max_min_oper: not a good op, id %d",
	      oper->id);
    }

  new_oper1 = L_create_new_op_using (op, oper);
  new_oper1->src[0] = L_copy_operand (oper->src[0]);
  new_oper1->src[1] = L_copy_operand (oper->src[1]);
  new_oper1->dest[0] = L_copy_operand (oper->dest[0]);
  new_oper1->proc_opc = proc_opc;
  L_insert_oper_before (cb, oper, new_oper1);
}				/* L_annotate_double_max_min_oper */

/* 09/11/02 REK Modifying function to use new TAHOEops. */
static void
L_annotate_float_move (L_Cb * cb, L_Oper * oper)
{
  L_Oper *mov_oper;
  L_Operand *src;
  src = oper->src[0];
  mov_oper = L_create_new_op_using (Lop_MOV_F, oper);
  mov_oper->proc_opc = TAHOEop_MOV_FR;
  if (L_is_constant (src))
    src = L_float_constant_immed (cb, oper, src);

  mov_oper->src[0] = L_copy_operand (src);
  mov_oper->dest[0] = L_copy_operand (oper->dest[0]);

  L_insert_oper_before (cb, oper, mov_oper);
}				/* L_annotate_float_move */

/* 09/11/02 REK Modifying function to use new TAHOEops. */
static void
L_annotate_double_move (L_Cb * cb, L_Oper * oper)
{
  L_Oper *mov_oper;
  L_Operand *src;
  src = oper->src[0];
  mov_oper = L_create_new_op_using (Lop_MOV_F2, oper);
  mov_oper->proc_opc = TAHOEop_MOV_FR;
  if (L_is_constant (src))
    src = L_double_constant_immed (cb, oper, src);

  mov_oper->src[0] = L_copy_operand (src);
  mov_oper->dest[0] = L_copy_operand (oper->dest[0]);

  L_insert_oper_before (cb, oper, mov_oper);
}				/* L_annotate_double_move */

/* 09/11/02 REK Modifying function to use new TAHOEops. */
static void
L_annotate_float_oper (L_Cb * cb, L_Oper * oper, int op)
{
  L_Oper *new_oper1, *new_oper2, *new_oper3, *new_oper4;
  fprintf (stderr, "float_oper -unchecked  oper:%d\n", oper->id);
  new_oper1 = L_create_new_op_using (Lop_MUL_ADD_F, oper);
  new_oper1->src[0] = L_copy_operand (oper->src[0]);
  new_oper1->src[1] = Ltahoe_FMAC (FONE);
  new_oper1->src[2] = Ltahoe_FMAC (FZERO);
  new_oper1->dest[0] = Ltahoe_new_reg (L_CTYPE_DOUBLE);
  new_oper1->proc_opc = TAHOEop_FMA_S;
  L_insert_oper_before (cb, oper, new_oper1);

  new_oper2 = L_create_new_op_using (Lop_MUL_ADD_F, oper);
  new_oper2->src[0] = L_copy_operand (oper->src[1]);
  new_oper2->src[1] = Ltahoe_FMAC (FONE);
  new_oper2->src[2] = Ltahoe_FMAC (FZERO);
  new_oper2->dest[0] = Ltahoe_new_reg (L_CTYPE_DOUBLE);
  new_oper2->proc_opc = TAHOEop_FMA_S;
  L_insert_oper_before (cb, oper, new_oper2);

  new_oper3 = L_create_new_op_using (op, oper);
  new_oper3->src[0] = L_copy_operand (new_oper1->dest[0]);
  new_oper3->src[1] = L_copy_operand (new_oper2->dest[0]);
  new_oper3->dest[0] = L_copy_operand (new_oper1->dest[0]);
  if (op == Lop_SQRT_F)
    new_oper3->proc_opc = TAHOEop_FRSQRTA;
  else
    new_oper3->proc_opc = TAHOEop_FRCPA;
  L_insert_oper_before (cb, oper, new_oper3);

  new_oper4 = L_create_new_op_using (Lop_MUL_ADD_F, oper);
  new_oper4->src[0] = L_copy_operand (new_oper3->dest[0]);
  new_oper4->src[1] = Ltahoe_FMAC (FONE);
  new_oper4->src[2] = Ltahoe_FMAC (FZERO);
  new_oper4->dest[0] = L_copy_operand (oper->dest[0]);
  new_oper4->proc_opc = TAHOEop_FMA_S;
  L_insert_oper_before (cb, oper, new_oper4);
}				/* L_annotate_float_oper */


/* 09/11/02 REK Modifying function to use TAHOEops. */
static void
L_annotate_double_oper (L_Cb * cb, L_Oper * oper, int op)
{
  L_Oper *new_oper;
  fprintf (stderr, "double_oper -unchecked  oper:%d\n", oper->id);
  new_oper = L_create_new_op_using (op, oper);
  new_oper->src[0] = L_copy_operand (oper->src[0]);
  new_oper->src[1] = L_copy_operand (oper->src[1]);
  new_oper->dest[0] = L_copy_operand (oper->dest[0]);
  if (op == Lop_SQRT_F2)
    new_oper->proc_opc = TAHOEop_FRSQRTA;
  else
    new_oper->proc_opc = TAHOEop_FRCPA;
  L_insert_oper_before (cb, oper, new_oper);
}				/* L_annotate_double_oper */


/* 09/11/02 REK Modifying function to use new TAHOEops. */
void
L_annotate_int_to_float (L_Cb * cb, L_Oper * oper, int unsigned_conv)
{
  L_Oper *setf_oper, *conv_oper;
  L_Operand *src_operand = NULL;
  if (L_is_int_constant (oper->src[0]))
    {
      src_operand = Ltahoe_new_int_reg ();
      Ltahoe_int_constant_load (cb, oper, oper->src[0], src_operand);
    }				/* if */
  else if (L_is_variable (oper->src[0]))
    {
      src_operand = oper->src[0];
    }				/* else if */
  else
    {
      L_punt ("L_annotate_int_to_float: Unknown src type  oper:%d", oper->id);
    }				/* else */

  /* Int to fixed point */

  /*   fprintf(stderr, "Now using sig in i_f  oper:%d\n", oper->id); */
  setf_oper = L_create_new_op_using (oper->opc, oper);
  setf_oper->proc_opc = TAHOEop_SETF_SIG;

  setf_oper->src[0] = L_copy_operand (src_operand);
  setf_oper->dest[0] = Ltahoe_new_reg (L_CTYPE_DOUBLE);

  /* Fixed to float conversion */

  if (!unsigned_conv)
    {
      conv_oper = L_create_new_op_using (Lop_I_F, oper);
      conv_oper->proc_opc = TAHOEop_FCVT_XF;
      conv_oper->src[0] = L_copy_operand (setf_oper->dest[0]);
      conv_oper->dest[0] = L_copy_operand (oper->dest[0]);
    }				/* if */
  else
    {
      conv_oper = L_create_new_op_using (Lop_I_F, oper);
      conv_oper->proc_opc = TAHOEop_FMA;
      conv_oper->src[0] = L_copy_operand (setf_oper->dest[0]);
      conv_oper->src[1] = Ltahoe_FMAC (FONE);
      conv_oper->src[2] = Ltahoe_FMAC (FZERO);
      conv_oper->dest[0] = L_copy_operand (oper->dest[0]);
    }				/* else */

  L_insert_oper_before (cb, oper, setf_oper);
  L_insert_oper_before (cb, oper, conv_oper);

  /*    fprintf(stderr, "No mult done in int->flt conversion  oper:%d\n",
     oper->id); */
}				/* L_annotate_int_to_float */


/* 09/11/02 REK Modifying function to use new TAHOEops. */
void
L_annotate_int_to_double (L_Cb * cb, L_Oper * oper)
{
  L_Oper *setf_oper, *conv_oper;
  L_Operand *src_operand = NULL;

  if (L_is_int_constant (oper->src[0]))
    {
      src_operand = Ltahoe_new_int_reg ();
      Ltahoe_int_constant_load (cb, oper, oper->src[0], src_operand);
    }				/* if */
  else if (L_is_variable (oper->src[0]))
    {
      src_operand = L_copy_operand (oper->src[0]);
    }				/* else if */
  else
    {
      L_punt ("L_annotate_int_to_double: Unknown src type  oper:%d",
	      oper->id);
    }				/* else */

  /* Int to fixed point */
/*   fprintf(stderr, "Now using sig in i_f2  oper:%d\n", oper->id); */
  setf_oper = L_create_new_op_using (oper->opc, oper);
  setf_oper->proc_opc = TAHOEop_SETF_SIG;
  setf_oper->src[0] = src_operand;
  setf_oper->dest[0] = Ltahoe_new_reg (L_CTYPE_DOUBLE);

  /* Fixed to double conversion */
#if 1
  /* fprintf(stderr, "Should we be using fma instead of fcvt?  oper: %d\n", */
  /*           oper->id); */
  conv_oper = L_create_new_op_using (Lop_I_F2, oper);
  conv_oper->proc_opc = TAHOEop_FCVT_XF;
  conv_oper->src[0] = L_copy_operand (setf_oper->dest[0]);
  conv_oper->dest[0] = L_copy_operand (oper->dest[0]);
#else
  fprintf (stderr, "Now using fma instead of fcvt  oper: %d\n", oper->id);
  conv_oper = L_create_new_op_using (Lop_F_F2, oper);
  conv_oper->proc_opc = TAHOEop_FMA_D;
  conv_oper->src[0] = L_copy_operand (setf_oper->dest[0]);
  conv_oper->src[1] = Ltahoe_FMAC (FONE);
  conv_oper->src[2] = Ltahoe_FMAC (FZERO);
  conv_oper->dest[0] = L_copy_operand (oper->dest[0]);
#endif
  L_insert_oper_before (cb, oper, setf_oper);
  L_insert_oper_before (cb, oper, conv_oper);
/*    fprintf(stderr, "No mult done in int->flt conversion  oper:%d\n",
      oper->id); */
}				/* L_annotate_int_to_double */


/* 09/11/02 REK Modifying function to use new TAHOEops. */
void
L_annotate_float_to_int (L_Cb * cb, L_Oper * oper)
{
  L_Oper *new_oper1, *new_oper2;

  new_oper1 = L_create_new_op_using (oper->opc, oper);
  new_oper1->src[0] = L_copy_operand (oper->src[0]);
  new_oper1->dest[0] = Ltahoe_new_reg (L_CTYPE_DOUBLE);
  new_oper1->proc_opc = TAHOEop_FCVT_FX;
  new_oper1->completers |= TC_FCVT_TRUNC;
  new_oper2 = L_create_new_op_using (Lop_F_I, oper);
  new_oper2->src[0] = L_copy_operand (new_oper1->dest[0]);
  new_oper2->dest[0] = L_copy_operand (oper->dest[0]);
  new_oper2->proc_opc = TAHOEop_GETF_SIG;
  L_insert_oper_before (cb, oper, new_oper1);
  L_insert_oper_before (cb, oper, new_oper2);

  return;
}				/* L_annotate_float_to_int */


/* 09/11/02 REK Modifying function to use new TAHOEops. */
void
L_annotate_double_to_int (L_Cb * cb, L_Oper * oper)
{
  L_Oper *new_oper1, *new_oper2;

  new_oper1 = L_create_new_op_using (oper->opc, oper);
  new_oper1->src[0] = L_copy_operand (oper->src[0]);
  new_oper1->dest[0] = Ltahoe_new_reg (L_CTYPE_DOUBLE);
  new_oper1->proc_opc = TAHOEop_FCVT_FX;
  new_oper1->completers |= TC_FCVT_TRUNC;
  new_oper2 = L_create_new_op_using (Lop_F2_I, oper);
  new_oper2->src[0] = L_copy_operand (new_oper1->dest[0]);
  new_oper2->dest[0] = L_copy_operand (oper->dest[0]);
  new_oper2->proc_opc = TAHOEop_GETF_SIG;
  L_insert_oper_before (cb, oper, new_oper1);
  L_insert_oper_before (cb, oper, new_oper2);

  return;
}				/* L_annotate_double_to_int */


/* 09/11/02 REK Modifying function to use new TAHOEops. */
static void
L_annotate_double_to_float_conversion (L_Cb * cb, L_Oper * oper)
{
  if (L_is_constant (oper->src[0]))
    {
      L_annotate_float_move (cb, oper);
    }				/* if */
  else
    {
      L_Oper *new_oper1;

      new_oper1 = L_create_new_op_using (Lop_MUL_ADD_F, oper);
      new_oper1->proc_opc = TAHOEop_FMA_S;
      new_oper1->src[1] = Ltahoe_FMAC (FONE);
      new_oper1->src[2] = Ltahoe_FMAC (FZERO);
      new_oper1->src[0] = L_copy_operand (oper->src[0]);
      new_oper1->dest[0] = L_copy_operand (oper->dest[0]);
      L_insert_oper_before (cb, oper, new_oper1);
    }				/* else */

  return;
}				/* L_annotate_double_to_float_conversion */

static int
Ltahoe_is_int_constant_add (L_Oper * oper)
{
  if ((oper->opc != Lop_ADD) && (oper->opc != Lop_ADD_U))
    return 0;
  if (!L_is_constant (oper->src[0]) && !L_is_constant (oper->src[1]))
    return 0;
  return 1;
}


/* 09/11/02 REK Modifying function to use new TAHOEops. */
static void
L_annotate_nop (L_Cb * cb, L_Oper * oper)
{
  L_Oper *new_oper, *tmpl_oper;
  int s, tmpl, type, popc = 0;

  for (tmpl_oper = oper->prev_op, s = 0; tmpl_oper && s < 3;
       tmpl_oper = tmpl_oper->prev_op, s++)
    if (LT_is_template_op (tmpl_oper))
      break;

  if (!tmpl_oper || s > 2)
    return;

  tmpl = LT_get_template (tmpl_oper);
  type = LT_syllable_type (tmpl, s);

  switch (type)
    {
    case M_SYLL:
      popc = TAHOEop_NOP_M;
      break;
    case I_SYLL:
      popc = TAHOEop_NOP_I;
      break;
    case F_SYLL:
      popc = TAHOEop_NOP_F;
      break;
    case B_SYLL:
      popc = TAHOEop_NOP_B;
      break;
    case L_SYLL:
      popc = TAHOEop_NOP_X;
      break;
    default:
      L_punt ("Invalid NOP type is required");
    }				/* switch */

  new_oper = LT_create_nop (popc, 0xBEEF);

  L_warn ("Annotating a NOP (oper %d)", oper->id);

  L_insert_oper_before (cb, oper, new_oper);

  return;
}				/* L_annotate_nop */


/* 09/11/02 REK Modifying function to use new TAHOEops. */
static void
L_convert_to_tahoe_oper (L_Func * fn, L_Cb * cb, L_Oper * oper)
{
  int opc;

  {
    /* JWS 20000704 Stack operations are adjusted in postpass to
     * be MAC $SP relative.  Thus, all stack operands need to be
     * on addi instructions
     */
    int i;
    L_Operand *mac_opd;
    L_Oper *new_oper;

    for (i = 0; i < L_max_src_operand; i++)
      {
	if (L_is_macro (oper->src[i]) &&
	    ((oper->src[i]->value.mac == L_MAC_OP) ||
	     (oper->src[i]->value.mac == L_MAC_IP) ||
	     (oper->src[i]->value.mac == L_MAC_LV) ||
	     (oper->src[i]->value.mac == L_MAC_SP) ||
	     (oper->src[i]->value.mac == L_MAC_FP)))
	  mac_opd = oper->src[i];
	else
	  continue;

	if (Ltahoe_is_int_constant_add (oper) || oper->opc == Lop_DEFINE)
	  continue;
	else if (oper->opc == Lop_MOV)
	  {
	    /* Turn a MOV into a constant add */
	    oper->opc = Lop_ADD;
	    oper->src[1] = oper->src[0];
	    oper->src[0] = L_new_gen_int_operand (0);
	    break;
	  }			/* else if */
	else
	  {
	    /* Insert a constant add prior to the op */
	    new_oper = L_create_new_op_using (Lop_ADD, oper);
	    new_oper->src[0] = mac_opd;
	    new_oper->src[1] = L_new_gen_int_operand (0);
	    new_oper->dest[0] = Ltahoe_new_int_reg ();
	    oper->src[i] = L_copy_operand (new_oper->dest[0]);
	    L_insert_oper_before (cb, oper, new_oper);
	    L_annotate_int_add (cb, new_oper);
	    L_delete_oper (cb, new_oper);
	  }			/* else */
      }				/* for i */
  }

  opc = oper->opc;
  switch (opc)
    {
    case Lop_NO_OP:
      L_annotate_nop (cb, oper);
      break;
    case Lop_JSR:
    case Lop_JSR_FS:
      L_annotate_jsr (fn, cb, oper);
      break;
    case Lop_RTS:
    case Lop_RTS_FS:
      L_annotate_rts (cb, oper);
      break;
    case Lop_JUMP:
    case Lop_JUMP_FS:
      L_annotate_jump (cb, oper);
      break;
    case Lop_PROLOGUE:
      L_annotate_prologue (fn, cb, oper);
      break;
    case Lop_EPILOGUE:
      L_annotate_epilogue (fn, cb, oper);
      break;
    case Lop_DEFINE:
      oper->proc_opc = TAHOEop_NON_INSTR;
      L_insert_oper_before (cb, oper, L_copy_parent_oper (oper));
      break;
    case Lop_CMP:
      Ltahoe_annotate_cmp (cb, oper);
      break;
    case Lop_CMP_F:
      Ltahoe_annotate_fcmp (cb, oper);
      break;
    case Lop_PRED_CLEAR:
    case Lop_PRED_SET:
    case Lop_PRED_COPY:
      Ltahoe_annotate_pred_init (cb, oper);
      break;
    case Lop_LD_POST_UC:
    case Lop_LD_POST_C:
    case Lop_LD_POST_UC2:
    case Lop_LD_POST_C2:
    case Lop_LD_POST_F:
    case Lop_LD_POST_F2:
    case Lop_LD_POST_I:
    case Lop_LD_UC:
    case Lop_LD_C:
    case Lop_LD_UC2:
    case Lop_LD_C2:
    case Lop_LD_F:
    case Lop_LD_F2:
    case Lop_LD_I:
    case Lop_LD_UI:
    case Lop_LD_POST_UI:
    case Lop_LD_UC_CHK:
    case Lop_LD_C_CHK:
    case Lop_LD_UC2_CHK:
    case Lop_LD_C2_CHK:
    case Lop_LD_I_CHK:
    case Lop_LD_UI_CHK:
    case Lop_LD_F_CHK:
    case Lop_LD_F2_CHK:
    case Lop_LD_Q:
      L_annotate_ld (cb, oper);
      break;
    case Lop_ST_POST_C:
    case Lop_ST_POST_C2:
    case Lop_ST_POST_I:
    case Lop_ST_POST_F:
    case Lop_ST_POST_F2:
    case Lop_ST_C:
    case Lop_ST_C2:
    case Lop_ST_I:
    case Lop_ST_F:
    case Lop_ST_F2:
    case Lop_ST_Q:
      L_annotate_st (cb, oper);
      break;
    case Lop_EXTRACT_C:
    case Lop_EXTRACT_C2:
    case Lop_SXT_C:
    case Lop_ZXT_C:
    case Lop_SXT_C2:
    case Lop_ZXT_C2:
    case Lop_SXT_I:
    case Lop_ZXT_I:
      L_annotate_extend (cb, oper);
      break;
    case Lop_MOV:
      L_annotate_int_move (cb, oper);
      break;
    case Lop_ADD:
    case Lop_ADD_U:
      L_annotate_int_add (cb, oper);
      break;
    case Lop_SUB:
    case Lop_SUB_U:
      L_annotate_int_sub (cb, oper);
      break;
    case Lop_AND:
    case Lop_NAND:
    case Lop_OR:
    case Lop_NOR:
    case Lop_XOR:
    case Lop_NXOR:
    case Lop_AND_COMPL:
    case Lop_OR_COMPL:
      L_annotate_int_logic (cb, oper);
      break;
    case Lop_MUL:
    case Lop_MUL_U:
      L_annotate_int_multiply (cb, oper);
      break;
    case Lop_DIV:
    case Lop_DIV_U:
      L_annotate_int_divide (cb, oper);
      break;
    case Lop_REM:
    case Lop_REM_U:
      L_annotate_int_remainder (cb, oper);
      break;
    case Lop_MOV_F:
      L_annotate_float_move (cb, oper);
      break;
    case Lop_MOV_F2:
      L_annotate_double_move (cb, oper);
      break;
    case Lop_DIV_F:
      L_annotate_float_divide (cb, oper);
      break;
    case Lop_DIV_F2:
      L_annotate_double_divide (cb, oper);
      break;
    case Lop_MUL_ADD_F:
    case Lop_MUL_ADD_F2:
    case Lop_MUL_SUB_F:
    case Lop_MUL_SUB_F2:
    case Lop_MUL_SUB_REV_F:
    case Lop_MUL_SUB_REV_F2:
      L_annotate_float_mul_add_sub_oper (cb, oper, opc);
      break;
    case Lop_ADD_F:
    case Lop_ADD_F2:
    case Lop_SUB_F:
    case Lop_SUB_F2:
      L_annotate_add_sub_oper (cb, oper);
      break;
    case Lop_MUL_F:
      L_annotate_float_mul_oper (cb, oper, opc);
      break;
    case Lop_MUL_F2:
      L_annotate_double_mul_oper (cb, oper, opc);
      break;
    case Lop_F_F2:
      L_annotate_double_move (cb, oper);
      break;
    case Lop_F2_F:
      L_annotate_double_to_float_conversion (cb, oper);
      break;
    case Lop_F_I:
      L_annotate_float_to_int (cb, oper);
      break;
    case Lop_F2_I:
      L_annotate_double_to_int (cb, oper);
      break;
    case Lop_I_F:
      L_annotate_int_to_float (cb, oper, 0);
      break;
    case Lop_I_F2:
      L_annotate_int_to_double (cb, oper);
      break;
    case Lop_ABS:
      L_annotate_abs (cb, oper);
      break;
    case Lop_LSR:
    case Lop_ASR:
    case Lop_LSL:
      Ltahoe_annotate_shift (cb, oper);
      break;
    case Lop_LSLADD:
      L_annotate_shladd (cb, oper);
      break;
    case Lop_EXTRACT:
    case Lop_EXTRACT_U:
      L_annotate_extract_oper (cb, oper);
      break;
    case Lop_DEPOSIT:
      L_annotate_deposit_oper (cb, oper);
      break;
    case Lop_ABS_F:
      L_warn ("Treating ABS_F as ABS_F2, oper %d", oper->id);
    case Lop_ABS_F2:
      L_annotate_double_abs (cb, oper, opc);
      break;
    case Lop_MAX_F:
    case Lop_MIN_F:
      L_warn ("Treating MAX/MIN_F as MAX/MIN_F2, oper %d", oper->id);
    case Lop_MIN_F2:
    case Lop_MAX_F2:
      L_annotate_double_max_min_oper (cb, oper, opc);
      break;
    case Lop_RCP_F2:
    case Lop_SQRT_F2:
      L_annotate_double_oper (cb, oper, opc);
      break;
    case Lop_RCP_F:
    case Lop_SQRT_F:
      L_annotate_float_oper (cb, oper, opc);
      break;
    case Lop_CHECK:
      L_annotate_check (cb, oper);
      break;
    case Lop_PBR:
      L_print_oper (stderr, oper);
      L_punt ("L_convert_to_tahoe_oper: unsupported Lcode opcode");
      break;
    case Lop_ALLOC:
      L_punt ("L_convert_to_tahoe_oper: should not encounter Lop_ALLOC");
      break;
    case Lop_JUMP_RG:
    case Lop_JUMP_RG_FS:
      L_annotate_br_indir (cb, oper);
      break;
    case Lop_MUL_ADD:
    case Lop_MUL_ADD_U:
    case Lop_MUL_SUB:
    case Lop_MUL_SUB_U:
    case Lop_MUL_SUB_REV:
    case Lop_MUL_SUB_REV_U:
    case Lop_LD_PRE_UC:
    case Lop_LD_PRE_C:
    case Lop_LD_PRE_UC2:
    case Lop_LD_PRE_C2:
    case Lop_LD_PRE_F:
    case Lop_LD_PRE_F2:
    case Lop_LD_PRE_I:
    case Lop_LD_PRE_UI:
    case Lop_ST_PRE_C:
    case Lop_ST_PRE_C2:
    case Lop_ST_PRE_I:
    case Lop_ST_PRE_F:
    case Lop_ST_PRE_F2:
    case Lop_BIT_EXTRACT:
    case Lop_BIT_DEPOSIT:
    case Lop_AND_NOT:
    case Lop_OR_NOT:
    case Lop_RCMP:
    case Lop_RCMP_F:
    case Lop_MAX:
    case Lop_MIN:
    case Lop_REV:
    case Lop_BIT_POS:
    case Lop_PRED_LD:
    case Lop_PRED_ST:
    case Lop_PRED_LD_BLK:
    case Lop_PRED_ST_BLK:
    case Lop_PRED_MERGE:
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
    case Lop_PREF_LD:
    case Lop_EXPAND:
    case Lop_SIM_DIR:
    case Lop_CMOV:
    case Lop_CMOV_COM:
    case Lop_CMOV_F:
    case Lop_CMOV_COM_F:
    case Lop_CMOV_F2:
    case Lop_CMOV_COM_F2:
    case Lop_SELECT:
    case Lop_SELECT_F:
    case Lop_SELECT_F2:
    case Lop_JSR_ND:
    default:
      L_print_oper (stderr, oper);
      L_punt ("L_convert_to_tahoe_oper: unsupported Lcode opcode!");
      break;
    }				/* switch */
}				/* L_convert_to_tahoe_oper */


/* MCM Export this as a call back from Lsoftpipe. */
void
L_annotate_oper (L_Func * fn, L_Cb * cb, L_Oper * oper)
{
  L_convert_to_tahoe_oper (fn, cb, oper);
}


static void
L_mark_cb_as_hyperblock (L_Func * fn, L_Cb * cb, L_Oper * oper)
{
  if (L_cond_branch_opcode (oper))
    {
      cb->flags = L_SET_BIT_FLAG (cb->flags, L_CB_HYPERBLOCK);
      fn->flags = L_SET_BIT_FLAG (fn->flags, L_FUNC_HYPERBLOCK |
				  L_FUNC_CC_IN_PREDICATE_REGS);
    }
  else
    {
      switch (oper->opc)
	{
	case Lop_DIV:
	case Lop_DIV_U:
	case Lop_REM:
	case Lop_REM_U:
	case Lop_DIV_F:
	case Lop_DIV_F2:
	case Lop_MUL_F:
	case Lop_MUL_F2:
	case Lop_ABS:
	  cb->flags = L_SET_BIT_FLAG (cb->flags, L_CB_HYPERBLOCK);
	  fn->flags = L_SET_BIT_FLAG (fn->flags, L_FUNC_HYPERBLOCK);
	}
    }

  if (L_pred_define_opcode (oper) &&
      (!L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK) ||
       !L_EXTRACT_BIT_VAL (cb->flags, L_FUNC_HYPERBLOCK)))
    {
      cb->flags = L_SET_BIT_FLAG (cb->flags, L_CB_HYPERBLOCK);
      fn->flags = L_SET_BIT_FLAG (fn->flags, L_FUNC_HYPERBLOCK);
    }
}


static void
L_opt_flt_constants (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *oper;
  int i;

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      for (oper = cb->first_op; oper; oper = oper->next_op)
	{
	  for (i = 0; i < L_max_src_operand; i++)
	    {
	      if (L_is_flt_constant (oper->src[i]))
		{
		  if (oper->src[i]->value.f == 0.0)
		    {
		      L_delete_operand (oper->src[i]);
		      oper->src[i] = Ltahoe_FMAC (FZERO);
		    }
		  else if (oper->src[i]->value.f == 1.0)
		    {
		      L_delete_operand (oper->src[i]);
		      oper->src[i] = Ltahoe_FMAC (FONE);
		    }
		}
	      if (L_is_dbl_constant (oper->src[i]))
		{
		  if (oper->src[i]->value.f2 == 0.0)
		    {
		      L_delete_operand (oper->src[i]);
		      oper->src[i] = Ltahoe_DMAC (FZERO);
		    }
		  else if (oper->src[i]->value.f2 == 1.0)
		    {
		      L_delete_operand (oper->src[i]);
		      oper->src[i] = Ltahoe_DMAC (FONE);
		    }
		}
	    }
	}
    }
}


/* 09/11/02 REK Modifying function to use new TAHOEops. */
void
L_cleanup_after_mopti (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *oper, *next_oper;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (oper = cb->first_op; oper; oper = next_oper)
	{
	  next_oper = oper->next_op;
	  if (oper->proc_opc == oper->opc)
	    {
	      L_convert_to_tahoe_oper (fn, cb, oper);
	      L_delete_oper (cb, oper);
	      continue;
	    }			/* if */

	  /* Mopti will sometimes replace constants with registers.  Must
	     then change register src movl to general mov */

	  if (oper->proc_opc == TAHOEop_MOVL &&
	      (L_is_variable (oper->src[0])
	       || L_is_int_constant (oper->src[0])))
	    {
	      L_convert_to_tahoe_oper (fn, cb, oper);
	      L_delete_oper (cb, oper);
	      continue;
	    }			/* if */
	}			/* for oper */
    }				/* for cb */
}				/* L_cleanup_after_mopti */

/*****************************************************************************\
 *
 * L_process_func - annotate an Lcode function
 *
 * To ensure correct ordering of new Mopers within an Lcode function
 * we will annotate the Lcode oper into a list of Mcode opers [1 or more
 * Mcode opers].  This Mcode list of opers will be inserted on at a time
 * into the control block of the Lcode oper just before it.
 *
\*****************************************************************************/

void
L_process_func (L_Func * fn, Parm_Macro_List * command_line_macro_list)
{
  L_Oper *oper;
  L_Cb *cb;

  LTD ("FUNCTION %s START PHASE 1", fn->name);

  L_check_func (fn);

  /* Lcode preliminaries
   * ----------------------------------------------------------------------
   */

  if (L_do_software_pipelining)
    {
      LTD ("Preparing loops for software pipelining (SWP)");
      Lpipe_softpipe_loop_prep (fn);
    }

  if (!L_do_recovery_code)
    {
      LTD ("Deleting checks (non-RC)");
      /* 20031119 SZU
       * Recovery code all under RC_CODE for now
       */
#if defined(RC_CODE)
      RC_delete_all_checks (fn);
#endif
    }

  if (L_do_machine_opt && (Ltahoe_machine_opt_mask & 0x1))
    {
      if (Ltahoe_do_lcode_peephole)
	{
	  LTD ("Lcode peephole optimization");
	  Ltahoe_lcode_peephole (fn);
	}

      if (Mopti_constant_preloading)
	{
	  LTD ("Constant preloading");
	  L_do_flow_analysis (fn, DOMINATOR);
	  Mopti_constant_generation (fn);
	}

      if (Ltahoe_input_param_subst)
	{
	  LTD ("Input parameter substitution");
	  Ltahoe_ip_subst (fn);
	}

      if (Ltahoe_prologue_merge)
	{
	  LTD ("Branch target expansion");
	  Mopti_branch_target_expansion (fn);
	}
    }

  {
    L_Inner_Loop *il;

    L_inner_loop_detection (fn, 1);

    for (il = fn->first_inner_loop; il; il = il->next_inner_loop)
      {
	cb = il->cb;

	if (L_is_counted_inner_loop (il))
	  printf ("cb %d is counted inner loop\n", cb->id);

	/* Mark loopback branch to be converted
	 * to cloop
	 */
      }
  }

  LTD ("Ltahoe-izing Lcode");

  L_opt_flt_constants (fn);

  /* Conversion to predication, IA-64 style, while still in Lcode.
   * ----------------------------------------------------------------------
   * JWS 20010117
   */

  /* Convert compare register operations into corresponding predicate
   * operations 
   */

  L_convert_compares_to_predicates (fn);

  /* Convert conditional branches into a compare - unconditional jump
   * pair 
   */

  L_branch_split_func (fn);

  if (L_do_machine_opt && (Ltahoe_machine_opt_mask & 0x2))
    {
      /* Aid formation of and-chains by promoting intervening
       * predicated operations
       */
      L_do_flow_analysis (fn, LIVE_VARIABLE|REACHING_DEFINITION);
      L_predicate_promotion (fn, 0);
      LB_hb_pred_merging (fn);
    }

  /* Transform pred compares in Lcode form to pseudo-tahoe format.  
   */

  Ltahoe_customize_lcode_compares (fn);

  /* Set the function leaf flag if the function makes no calls 
   */

  L_leaf_func (fn);

  /* Lop -> TAHOEop Conversion
   * ----------------------------------------------------------------------
   * To maintain sync arcs, we must re-link all of the child opers
   * together after annotation.  To facilitate this, we maintain an
   * array of child pointers, indexed by parent id.
   */

  L_init_child_list (fn);

  L_mcode_init_function ();

  LTD ("Machine code annotation");

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      oper = cb->first_op;
      while (oper)
	{
	  L_mark_cb_as_hyperblock (fn, cb, oper);
	  L_convert_to_tahoe_oper (fn, cb, oper);
	  oper = L_convert_to_parent (cb, oper);
	}
    }

  /* Walks through fn and points sync arcs to other children, not parents */
  L_relink_child_sync_arcs (fn);
  L_deinit_child_list (fn);

  /* Parameter passing adjustment
   * ----------------------------------------------------------------------
   * Floating point parameters must be passed on the stack and in
   * floating point registers.  This differs from the Lcode specification.
   */

  Ltahoe_adjust_fp_parameter_passing (fn);

  /* Machine level code optimizations
   * ----------------------------------------------------------------------
   */

  LTD ("Unreachable code removal");

  PG_setup_pred_graph (fn);
  PG_pred_dead_code_removal (fn);
  L_delete_unreachable_blocks (fn);

  if (L_do_machine_opt && (Ltahoe_machine_opt_mask & 0x4))
    {
      LTD ("Bit trace optimization");

      if (Ltahoe_bitopt)
	L_optimize_bit_trace (fn);

      LTD ("Predicate promotion and merging");

      L_do_flow_analysis (fn, LIVE_VARIABLE|REACHING_DEFINITION);
      L_predicate_promotion (fn, 0);
      LB_hb_pred_merging (fn);

      LTD ("Mopti optimizations");

      if (Ltahoe_do_repeated_mopti)
	Mopti_perform_optimizations_tahoe (fn);

      if (Ltahoe_do_redux)
	Ltahoe_reduce (fn);

      if (Mopti_shift_add_merge)
	Mopti_shladd (fn);

      L_cleanup_after_mopti (fn);
    }

  if (Ltahoe_tag_loads)
    { 
      L_tag_load(fn);
    }

  /* Phase 1 cleanup
   * ----------------------------------------------------------------------
   */

  LTD ("Phase 1 cleanup");

  L_remove_empty_cbs (fn);
  L_set_hb_no_fallthru_flag (fn);

  L_partial_dead_code_removal (fn);
  L_demote_branches (fn);

  L_check_func (fn);

  /* This must be run after L_adjust_fp_parameter_passing.  It
     reflects the additional stack registers possibly needed by fp
     parameter passing.  */

  L_update_alloc (fn);

  L_check_func (fn);

  LTD ("FUNCTION %s END PHASE 1", fn->name);
  return;
}


/*
 * Phase 1 Global initializations */
void
L_init (Parm_Macro_List * command_line_macro_list)
{
  /* software pipeliner */

  if (L_do_software_pipelining)
    Lpipe_loop_prep_init (command_line_macro_list);
}

void
L_end ()
{
  /* software pipeliner */

  if (L_do_software_pipelining)
    Lpipe_loop_prep_end ();
}

/* JWS 20000907 */

void
Ltahoe_annotate_stack_ref (L_Oper * op, int mac, int offset, int spill)
{
  L_Attr *stack_attr;

  /* Add the stack oper flag and attribute for memory disambiguation.
     This is needed since there are no sync arcs on these stores. */

  op->flags = L_SET_BIT_FLAG (op->flags, L_OPER_STACK_REFERENCE);

  if (!(stack_attr = L_find_attr (op->attr, "stack")))
    {
      stack_attr = L_new_attr ("stack", 2);
      op->attr = L_concat_attr (op->attr, stack_attr);

      /* base */
      stack_attr->field[0] = L_new_macro_operand (mac, L_CTYPE_LLONG,
						  L_PTYPE_NULL);
      /* offset */
      stack_attr->field[1] = L_new_gen_int_operand (offset);
    }
  else
    {
      stack_attr->field[0]->value.mac = mac;
      stack_attr->field[1]->value.i = offset;
    }

  if (spill)
    {
      op->flags = L_SET_BIT_FLAG (op->flags, L_OPER_SPILL_CODE);

      if (!L_store_opcode (op))
	op->flags = L_SET_BIT_FLAG (op->flags, L_OPER_SAFE_PEI);

      if (!(stack_attr = L_find_attr (op->attr, "offset")))
	{
	  stack_attr = L_new_attr ("offset", 1);
	  op->attr = L_concat_attr (op->attr, stack_attr);

	  stack_attr->field[0] = L_new_gen_int_operand (offset);
	}
      else
	{
	  stack_attr->field[0]->value.i = offset;
	}
    }
  return;
}

/* JWS 20010805 */

static int
pwr_n_minus_1 (ITintmax i)
{
  int ones = 0;
  ITuintmax u = (ITuintmax) i;

  if (u == 0)
    return 1;

  if (!(u & 1))
    return 0;

  while ((u != 0) && (u & 1))
    {
      u >>= 1;
      ones++;
    }

  if (u != 0)
    return 0;
  else
    return ones;
}

static int
Ltahoe_mask_oper (L_Oper * oper, int *pmask_width, int *preg_indx)
{
  int reg_indx = 0, mask_width = 0;

  switch (oper->opc)
    {
    case Lop_AND:
      {
	ITintmax and_mask;
	if ((L_is_int_constant (oper->src[0])) &&
	    (L_is_register (oper->src[1])))
	  {
	    and_mask = oper->src[0]->value.i;
	    reg_indx = 1;
	  }
	else if ((L_is_int_constant (oper->src[1])) &&
		 (L_is_register (oper->src[0])))
	  {
	    and_mask = oper->src[1]->value.i;
	    reg_indx = 0;
	  }
	else
	  break;

	mask_width = pwr_n_minus_1 (and_mask);
      }
      break;
    case Lop_ZXT_C:
      mask_width = 8;
      break;
    case Lop_ZXT_C2:
      mask_width = 16;
      break;
    case Lop_ZXT_I:
      mask_width = 32;
      break;
    default:
      break;
    }

  if (mask_width)
    {
      if (preg_indx)
	*preg_indx = reg_indx;

      if (pmask_width)
	*pmask_width = mask_width;

      return 1;
    }
  else
    {
      return 0;
    }
}

#define DEBUG_PEEPHOLE 0

static int
Ltahoe_lcode_peephole (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *opA, *opB;
  int change = 0;
  int macro_flag, load_flag, store_flag;

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
	{
	  int mask_width, reg_indx, shift_value;
	  L_Operand *preshift_operand;

	  /* Pattern I: lsr-mask --> extr.u
	   */

	  if (L_is_opcode (Lop_LSR, opA) &&
	      (L_is_int_constant (opA->src[1])) &&
	      (L_is_register (opA->src[0])))
	    {
	      shift_value = ITicast (opA->src[1]->value.i);

	      preshift_operand = !L_same_operand (opA->dest[0], opA->src[0]) ?
		opA->src[0] : NULL;

	      for (opB = opA->next_op; opB != NULL; opB = opB->next_op)
		{
		  if (!PG_intersecting_predicates_ops (opA, opB))
		    continue;
		  if (Ltahoe_mask_oper (opB, &mask_width, &reg_indx) &&
		      L_same_operand (opA->dest[0], opB->src[reg_indx]) &&
		      PG_superset_predicate_ops (opA, opB))
		    {
		      macro_flag = L_is_fragile_macro (opA->src[0]);
		      load_flag = 0;
		      store_flag = 0;
		      if (!L_no_danger (macro_flag, load_flag, store_flag,
					opA, opB))
			break;
#if DEBUG_PEEPHOLE
		      fprintf (stderr, ">>> PEEPHOLE MATCH I:\n");
		      L_print_oper (stderr, opA);
		      L_print_oper (stderr, opB);
		      fprintf (stderr, "===\n");
#endif

		      if (!preshift_operand)
			{
			  L_Oper *mov_op;
			  /* lsl is self-antidependent -- need to save src!
			   * (this could actually make code worse)  
			   */

			  mov_op = L_create_new_op_using (Lop_MOV, opA);
			  L_insert_oper_before (cb, opA, mov_op);
			  mov_op->src[0] = L_copy_operand (opA->src[0]);
			  mov_op->dest[0] = Ltahoe_new_int_reg ();
			  preshift_operand = mov_op->dest[0];
			}

		      L_change_opcode (opB, Lop_EXTRACT_U);
		      L_delete_operand (opB->src[0]);
		      opB->src[0] = L_copy_operand (preshift_operand);
		      L_delete_operand (opB->src[1]);
		      opB->src[1] = L_new_gen_int_operand (shift_value);
		      opB->src[2] = L_new_gen_int_operand (mask_width);

#if DEBUG_PEEPHOLE
		      L_print_oper (stderr, opB);
		      fprintf (stderr, "<<<\n");
#endif

		      change += 1;
		    }
		  if (L_is_dest_operand (opA->dest[0], opB))
		    break;
		}
	    }

	  /* Pattern II: mask-lsl --> dep.u
	   */

	  else if (Ltahoe_mask_oper (opA, &mask_width, &reg_indx))
	    {
	      preshift_operand = !L_same_operand (opA->dest[0],
						  opA->src[reg_indx]) ?
		opA->src[reg_indx] : NULL;

	      for (opB = opA->next_op; opB != NULL; opB = opB->next_op)
		{
		  if (!PG_intersecting_predicates_ops (opA, opB))
		    continue;
		  if (L_is_opcode (Lop_LSL, opB) &&
		      L_same_operand (opA->dest[0], opB->src[0]) &&
		      PG_superset_predicate_ops (opA, opB) &&
		      L_is_int_constant (opB->src[1]))
		    {
		      int mask_width_b;
		      shift_value = ITicast (opB->src[1]->value.i);

		      macro_flag = L_is_fragile_macro (opA->src[0]);
		      load_flag = 0;
		      store_flag = 0;
		      if (!L_no_danger (macro_flag, load_flag, store_flag,
					opA, opB))
			break;

#if DEBUG_PEEPHOLE
		      fprintf (stderr, ">>> PEEPHOLE MATCH II:\n");
		      L_print_oper (stderr, opA);
		      L_print_oper (stderr, opB);
		      fprintf (stderr, "===\n");
#endif
		      if (!preshift_operand)
			{
			  L_Oper *mov_op;
			  /* lsl is self-antidependent -- need to save src!
			   * (this could actually make code worse)  
			   */

			  mov_op = L_create_new_op_using (Lop_MOV, opA);
			  L_insert_oper_before (cb, opA, mov_op);
			  mov_op->src[0] = L_copy_operand (opA->src[0]);
			  mov_op->dest[0] = Ltahoe_new_int_reg ();
			  preshift_operand = mov_op->dest[0];
			}

		      if (shift_value + mask_width > 64)
			mask_width_b = 64 - shift_value;
		      else
			mask_width_b = mask_width;

		      L_change_opcode (opB, Lop_DEPOSIT);
		      L_delete_operand (opB->src[0]);
		      opB->src[0] = L_copy_operand (preshift_operand);
		      L_delete_operand (opB->src[1]);
		      opB->src[1] = L_new_gen_int_operand (shift_value);
		      opB->src[2] = L_new_gen_int_operand (mask_width_b);
#if DEBUG_PEEPHOLE
		      L_print_oper (stderr, opB);
		      fprintf (stderr, "<<<\n");
#endif
		      change += 1;
		    }
		  if (L_is_dest_operand (opA->src[0], opB))
		    preshift_operand = 0;
		  if (L_is_dest_operand (opA->dest[0], opB))
		    break;
		}

	      if (L_is_opcode (Lop_AND, opA))
		{
#if DEBUG_PEEPHOLE
		  fprintf (stderr, ">>> PEEPHOLE MATCH III:\n");
		  L_print_oper (stderr, opA);
		  fprintf (stderr, "===\n");
#endif
		  L_change_opcode (opA, Lop_EXTRACT_U);

		  if (reg_indx == 1)
		    {
		      L_delete_operand (opA->src[0]);
		      opA->src[0] = opA->src[1];
		    }
		  else
		    {
		      L_delete_operand (opA->src[1]);
		    }
		  opA->src[1] = L_new_gen_int_operand (0);
		  opA->src[2] = L_new_gen_int_operand (mask_width);

#if DEBUG_PEEPHOLE
		  L_print_oper (stderr, opA);
		  fprintf (stderr, "<<<\n");
#endif
		}
	    }
	}
    }
  return change;
}
