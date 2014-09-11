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
 *
 *  File: phase2_sync.c
 *
 *  Description: Properly handle sync opers and calls.  These ops are marked
 *               with the L_OPER_SYNC (Y) flag.   Currently, the sync flag is
 *               only placed on the setjmp and longjump calls.
 *               Setjmp does not save the stacked registers, so no stacked
 *               registers can be live accross a setjmp.  There are only 4
 *               static callee save registers and Impact does not properly
 *               handle or use them.  As always, caller save registers must
 *               saved around a call.
 *               Currently, Impact reuses the swap space that spill/fill
 *               instructions use.  This means that the swap space will not
 *               be the same after a longjump as when the setjmp was executed.
 *               The spill and fill code around a setjmp must be handled
 *               specially so that the swap space is not later clobbered.  This
 *               can be done by treating the setjmp swap space as part of the
 *               local variable space.  This code spills and fills all of the
 *               live registers around a op that is marked as a sync.  The Lop
 *               is Lop_JSR_ND (ND = non destructive) so the register allocator
 *               will not spill/fill around the jsr.
 *
 *               This has nothing to do with sync arcs.
 *
 *  Authors: Bob McGowan
 *
 *****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "ltahoe_main.h"
#include <library/set.h>
#include <Lcode/r_regalloc.h>
#include "phase2_reg.h"
#include "phase1_func.h"

#undef DEBUG
#undef DEBUG1

#define IS_INT_MACRO_TO_SPILL(id) (((id) >= (L_MAC_P0 + M_TAHOE_IN_INT_BASE)) && \
                                   ((id) < (L_MAC_P0 +  M_TAHOE_IN_INT_BASE + \
					    M_TAHOE_MAX_FNVAR_INT_REG)))

#define IS_FP_MACRO_TO_SPILL(id)  (((id) >= (L_MAC_P0 + M_TAHOE_FLT_BASE)) && \
                                   ((id) < (L_MAC_P0 + M_TAHOE_FLT_BASE + \
                                            M_TAHOE_MAX_FNVAR_FLT_REG)))

/* prototypes */
static int O_spill_fill_around_sync (L_Func * fn, L_Cb * cb, L_Oper * op,
				     int *int_spill_start,
				     int *int_spill_end);
static void O_insert_spill_sequence (L_Cb * cb, L_Oper * before_op,
				     L_Oper * op_list);
static void O_insert_fill_sequence (L_Cb * cb, L_Oper * after_op,
				    L_Oper * op_list);

/****************************************************************************
 *
 * routine: O_handle_sync_opers()
 * purpose: Properly handle sync opers and calls by spilling and filling
 *          all live regisiters to the local variable space.
 * input: fn - function to check and fix if needed.
 * output:
 * returns: The size of the stack used in bytes to store integers.
 * modified: Bob McGowan - 4/97 - created
 * note: Live variable analysis needs to be up to date so that we can determine
 *       which registers need to be spilled and filled.  This is called in
 *       in this routine if we find a sync.
 *       Do not apply to all items that L_sync_opcode specifies.
 *       This should work if there are multiple setjmp in the same function,
 *       but it will only make good use of the stack when there is only one.
 *-------------------------------------------------------------------------*/

int
O_handle_sync_opers (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *op;
  L_Oper *next_op;
  int first_start = 0;
  int int_spill_start = 0;
  int int_spill_end = 0;	/* actually end + 8 */
  int found = 0, spilled = 0;

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      for (op = cb->first_op; op; op = next_op)
	{
	  next_op = op->next_op;
	  if (L_EXTRACT_BIT_VAL (op->flags, L_OPER_SYNC) &&
	      L_op_in_synchronization_func_table (op))
	    {
#if 1
	      fprintf (stderr, "Found sync op %d in cb %d\n", op->id, cb->id);
#endif
	      if (!found)
		{
		  L_do_flow_analysis (fn, LIVE_VARIABLE);
		  found = 1;
		}

	      if (O_spill_fill_around_sync (fn, cb, op, &int_spill_start,
					    &int_spill_end))
		{
		  if (!spilled)
		    {
		      first_start = int_spill_start;
		      spilled = 1;
		    }
		}
	    }
	}
    }

  if (spilled)
    return (int_spill_end - first_start);
  else
    return 0;
}


/****************************************************************************
 *
 * routine: O_spill_fill_around_sync()
 * purpose: Insert spill fill code around a sync oper.  The registers are spilled
 *          to the local variable space instead of the swap space since the swap
 *          space is reused.
 * input: op - spill and fill around this sync oper.
 * output: int_spill_start - the offset from LV where the first integer was
 *                           spilled, or 0 if no integers were spilled.
 *         int_spill_end - 8 past the offset from LV where the last integer was
 *                         spilled, or 0 if no integers were spilled.
 * returns: 1 if integers have been spilled, 0 otherwise.
 * modified: Bob McGowan - 4/97 - created
 * note: The floating point registers are spilled first and then the integer
 *       registers.  This is done so that the integer registers are closer to
 *       the integer registers in swap space.  This makes management of the
 *       unats register easier and more efficient.
 *-------------------------------------------------------------------------*/

static int
O_spill_fill_around_sync (L_Func * fn, L_Cb * cb, L_Oper * op,
			  int *int_spill_start, int *int_spill_end)
{
  L_Oper *op_list;
  L_Operand *spill_reg;
  Set live_variables;
  Set live_regs;
  int *live_reg_ids;
  int num_live_regs;
  Set live_macros;
  int *live_macro_ids;
  int num_live_macros;
  int variable;
  int int_offset, fp_offset;

  int_offset = *int_spill_start;

  fp_offset = fn->s_local;

  live_variables = L_get_oper_IN_set (op);

  /* registers */
  live_regs = L_unmap_reg_set (live_variables);
  num_live_regs = Set_size (live_regs);
  live_reg_ids = calloc (num_live_regs, sizeof (int));
  Set_2array (live_regs, live_reg_ids);

  /* macros */
  live_macros = L_unmap_macro_set (live_variables);
  num_live_macros = Set_size (live_macros);
  live_macro_ids = calloc (num_live_macros, sizeof (int));
  Set_2array (live_macros, live_macro_ids);

#ifdef DEBUG
#  ifdef DEBUG1
  L_print_cb (stderr, fn, cb);
  Set_print (stderr, "Live OUT set", live_variables);
  Set_print (stderr, "Registers", live_regs);
  Set_print (stderr, "Macros", live_macros);
#  endif
  fprintf (stderr, "fp setjmp spill/fill ops: ");
#endif

  /*
   * Spill the floating point registers and macros into
   * newly-allocated local variable space
   */

  /* first spill the floating point registers */
  /* create an operand, the id of it will be altered later. */
  spill_reg = L_new_register_operand (0, L_CTYPE_DOUBLE, L_PTYPE_NULL);

  for (variable = 0; variable < num_live_regs; variable++)
    {
      if (IS_FP_REGISTER (live_reg_ids[variable]))
	{
	  spill_reg->value.r = live_reg_ids[variable];
	  op_list = O_spill_reg (live_reg_ids[variable], L_OPERAND_REGISTER,
				 spill_reg, fp_offset, NULL, R_JSR_SAVE_CODE);
	  O_insert_spill_sequence (cb, op, op_list);
	  op_list = O_fill_reg (live_reg_ids[variable], L_OPERAND_REGISTER,
				spill_reg, fp_offset, NULL, R_JSR_SAVE_CODE);
	  O_insert_fill_sequence (cb, op, op_list);
	  fp_offset += 16;
	}
    }
#ifdef DEBUG
  fprintf (stderr, "\n");
  fprintf (stderr, "fp macro setjmp spill/fill ops: ");
#endif

  /* spill the floating point macros */
  /* change the operand to a macro operand. */
  L_assign_type_float_macro (spill_reg);
  for (variable = 0; variable < num_live_macros; variable++)
    {
      if (IS_FP_MACRO_TO_SPILL (live_macro_ids[variable]))
	{
	  spill_reg->value.r = live_macro_ids[variable];
	  op_list = O_spill_reg (live_macro_ids[variable], L_OPERAND_MACRO,
				 spill_reg, fp_offset, NULL, R_JSR_SAVE_CODE);
	  O_insert_spill_sequence (cb, op, op_list);
	  op_list = O_fill_reg (live_macro_ids[variable], L_OPERAND_MACRO,
				spill_reg, fp_offset, NULL, R_JSR_SAVE_CODE);
	  O_insert_fill_sequence (cb, op, op_list);
	  fp_offset += 16;
	}
    }

#ifdef DEBUG
  fprintf (stderr, "\n");
  fprintf (stderr, "int setjmp spill/fill ops: ");
#endif

  /*
   * Spill the integer registers and macros into sync space
   */

  L_assign_type_int_register (spill_reg);
  for (variable = 0; variable < num_live_regs; variable++)
    {
      if (IS_INT_REGISTER (live_reg_ids[variable]))
	{
	  int_offset += 8;
	  spill_reg->value.r = live_reg_ids[variable];
	  op_list = O_spill_reg (live_reg_ids[variable], L_OPERAND_REGISTER,
				 spill_reg, -int_offset,
				 NULL, R_JSR_SAVE_CODE);
	  O_insert_spill_sequence (cb, op, op_list);
	  op_list = O_fill_reg (live_reg_ids[variable], L_OPERAND_REGISTER,
				spill_reg, -int_offset,
				NULL, R_JSR_SAVE_CODE);
	  O_insert_fill_sequence (cb, op, op_list);
	}
    }

#ifdef DEBUG
  fprintf (stderr, "\n");
  fprintf (stderr, "int macro setjmp spill/fill ops: ");
#endif

  L_assign_type_int_macro (spill_reg);
  for (variable = 0; variable < num_live_macros; variable++)
    {
      if (IS_INT_MACRO_TO_SPILL (live_macro_ids[variable]))
	{
	  int_offset += 8;
	  spill_reg->value.r = live_macro_ids[variable];
	  op_list = O_spill_reg (live_macro_ids[variable], L_OPERAND_MACRO,
				 spill_reg, -int_offset, NULL,
				 R_JSR_SAVE_CODE);
	  O_insert_spill_sequence (cb, op, op_list);
	  op_list = O_fill_reg (live_macro_ids[variable], L_OPERAND_MACRO,
				spill_reg, -int_offset, NULL,
				R_JSR_SAVE_CODE);
	  O_insert_fill_sequence (cb, op, op_list);
	}
    }
#ifdef DEBUG
  fprintf (stderr, "\n");
#endif

  L_delete_operand (spill_reg);

  /*
   * Put fp regs into local variable space.
   */

  L_update_local_space_size (fn, fp_offset);

  /*
   * Put int regs into sync space -- must be managed by the UNAT
   */

  if (*int_spill_start == int_offset)
    {
      /* No integers spilled */
      *int_spill_end = *int_spill_start;
      return (0);
    }
  else
    {
      *int_spill_end = int_offset;
      return (1);
    }
}


/****************************************************************************
 *
 * routine: O_insert_spill_sequence()
 * purpose: Insert the sequence of ops in op_list before the given op.
 *          While inserting the code, replace all references to SP or FP with LV.
 * input: cb - the cb which owns the op.
 *        before_op - insert the sequence before this op.
 *        op_list - the group of ops to insert before the given op.
 * output:
 * returns:
 * modified: Bob McGowan - 4/97 - created
 * note: Searching for and replacing SP and FP may not be efficient but it
 *       should reduce bugs and maintainance.
 *-------------------------------------------------------------------------*/

static void
O_insert_spill_sequence (L_Cb * cb, L_Oper * before_op, L_Oper * op_list)
{
  L_Oper *op;
  L_Oper *next_op;
  int operand;

  for (op = op_list; op; op = next_op)
    {
      next_op = op->next_op;

      /* Check the op to see if it uses SP */
      for (operand = 0;
	   (operand < L_max_dest_operand) && op->dest[operand]; operand++)
	{
	  if (L_is_macro (op->dest[operand]) &&
	      ((op->dest[operand]->value.r == L_MAC_SP) ||
	       (op->dest[operand]->value.r == L_MAC_FP)))
	    {
	      op->dest[operand]->value.r = L_MAC_LV;
	    }
	}
      for (operand = 0;
	   (operand < L_max_src_operand) && op->src[operand]; operand++)
	{
	  if (L_is_macro (op->src[operand]) &&
	      ((op->src[operand]->value.r == L_MAC_SP) ||
	       (op->src[operand]->value.r == L_MAC_FP)))
	    {
	      op->src[operand]->value.r = L_MAC_LV;
	    }
	}
#ifdef DEBUG
      fprintf (stderr, "%d ", op->id);
#endif
      L_insert_oper_before (cb, before_op, op);
    }
}


/****************************************************************************
 *
 * routine: O_insert_fill_sequence()
 * purpose: Insert the sequence of ops in op_list after the given op.
 *          While inserting the code, replace all references to SP or FP with LV.
 *          This is very similar to O_insert_spill_sequence()
 * input: cb - the cb which owns the op.
 *        after_op - insert the sequence after this op.
 *        op_list - the group of ops to insert before the given op.
 * output:
 * returns:
 * modified: Bob McGowan - 4/97 - created
 * note: Searching for and replacing SP and FP may not be efficient but it
 *       should reduce bugs and maintainance.
 *-------------------------------------------------------------------------*/

static void
O_insert_fill_sequence (L_Cb * cb, L_Oper * after_op, L_Oper * op_list)
{
  L_Oper *op;
  L_Oper *next_op;
  L_Oper *prev_op;
  int operand;

  prev_op = after_op;
  for (op = op_list; op; op = next_op)
    {
      next_op = op->next_op;

      /* Check the op to see if it uses SP */
      for (operand = 0;
	   (operand < L_max_dest_operand) && op->dest[operand]; operand++)
	{
	  if (L_is_macro (op->dest[operand]) &&
	      ((op->dest[operand]->value.r == L_MAC_SP) ||
	       (op->dest[operand]->value.r == L_MAC_FP)))
	    {
	      op->dest[operand]->value.r = L_MAC_LV;
	    }
	}
      for (operand = 0;
	   (operand < L_max_src_operand) && op->src[operand]; operand++)
	{
	  if (L_is_macro (op->src[operand]) &&
	      ((op->src[operand]->value.r == L_MAC_SP) ||
	       (op->src[operand]->value.r == L_MAC_FP)))
	    {
	      op->src[operand]->value.r = L_MAC_LV;
	    }
	}
#ifdef DEBUG
      fprintf (stderr, "%d ", op->id);
#endif
      L_insert_oper_after (cb, prev_op, op);
      prev_op = op;
    }
}
