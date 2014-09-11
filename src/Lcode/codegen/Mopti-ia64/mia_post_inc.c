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
/**************************************************************************\
 *
 *  File: post_increment.c
 *
 *  Description:
 *     Convert ld and add or st and add pairs into a postincrement instruction.
 *
 *  Authors: Bob McGowan
 *
 *
\************************************************************************/
/* 09/17/02 REK Updating to use the new tahoeops and completers scheme. */

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "mia_internal.h"
#include <Lcode/l_softpipe.h>

/* #define DEBUG_POST_INC */

/* #define DEBUG */

static void Mia_cb_post_increment (L_Cb * cb, int conservative,
				   int ld, int st);

/****************************************************************************
 *
 * routine: Convert_to_post_increment_ops()
 * purpose: This is the top level call to the post increment optimization.
 *          Only easily recognizable post increments are done.  A ld/st is
 *          matched with an add which is either in the same basic block or
 *          atleast in the same cb.  If the add move above a branch the
 *          address register cannot be live out the taken path of the branch.
 *          No attempt is made to merge multiple adds.
 * input: fn - function to process.
 * output:
 * returns:
 * modified: 1/10/97 - Bob McGowan - created
 * note:
 *-------------------------------------------------------------------------*/

void
Mia_post_increment_conversion (L_Func * fn, int ld, int st)
{
  L_Cb *cb;

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    if (!L_EXTRACT_BIT_VAL (cb->flags, L_CB_SOFTPIPE))
      Mia_cb_post_increment (cb, 0, ld, st);

  return;
}

/****************************************************************************
 *
 * routine: Conservative_post_increment_conversion()
 * purpose: This is the top level call to the conservative post increment 
 *          optimization.  This optimization is run at the end of phase 1
 *          and is designed to make post increment optimization available 
 *          for modulo scheduled loops and for more accurate scheduling without
 *          inhibiting scheduling opportunities for loads and stores.
 * input: fn - function to process.
 * output:
 * returns:
 * modified: 9/6/97 - Dan Lavery - created
 * note:
 *-------------------------------------------------------------------------*/

void
Mia_softpipe_post_increment_conversion (L_Func * fn, int ld, int st)
{
  L_Cb *cb;

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_SOFTPIPE))
      Mia_cb_post_increment (cb, 1, ld, st);

  return;
}

/****************************************************************************
 *
 * routine: Convert_mem_op_to_post_inc()
 * purpose: Convert the given memory operation into a post increment version.
 *          Use the operand for the post-inc value or register.
 * input: mem_oper - either a load or store
 *        inc_opernad - the register or constant to use as the increment.
 * output:
 * returns:
 * modified: 1/10/97 - Bob McGowan - created
 * note: A copy of the inc_operand is NOT made.  The memory oper will own
 *       the given operand after this routine.
 *       The address operand is copied to the destination operands.
 *-------------------------------------------------------------------------*/

static void
Convert_mem_op_to_post_inc (L_Oper * mem_oper, L_Operand * inc_operand)
{
  switch (mem_oper->proc_opc)
    {
    case TAHOEop_LD1:
      L_change_opcode (mem_oper, Lop_LD_POST_C);
      /* L_change_opcode clobbers proc_opc, so it needs to be reset. */
      /* mem_oper->proc_opc = TAHOEop_LD1_POST; */
      mem_oper->proc_opc = TAHOEop_LD1;
      mem_oper->src[1] = inc_operand;
      mem_oper->dest[1] = L_copy_operand (mem_oper->src[0]);
      break;

    case TAHOEop_LD2:
      L_change_opcode (mem_oper, Lop_LD_POST_C2);
      /* L_change_opcode clobbers proc_opc, so it needs to be reset. */
      /* mem_oper->proc_opc = TAHOEop_LD2_POST; */
      mem_oper->proc_opc = TAHOEop_LD2;
      mem_oper->src[1] = inc_operand;
      mem_oper->dest[1] = L_copy_operand (mem_oper->src[0]);
      break;

    case TAHOEop_LD4:
      L_change_opcode (mem_oper, Lop_LD_POST_I);
      /* L_change_opcode clobbers proc_opc, so it needs to be reset. */
      /* mem_oper->proc_opc = TAHOEop_LD4_POST; */
      mem_oper->proc_opc = TAHOEop_LD4;
      mem_oper->src[1] = inc_operand;
      mem_oper->dest[1] = L_copy_operand (mem_oper->src[0]);
      break;

    case TAHOEop_LD8:
      L_change_opcode (mem_oper, Lop_LD_POST_Q);
      /* L_change_opcode clobbers proc_opc, so it needs to be reset. */
      /* mem_oper->proc_opc = TAHOEop_LD8_POST; */
      mem_oper->proc_opc = TAHOEop_LD8;
      mem_oper->src[1] = inc_operand;
      mem_oper->dest[1] = L_copy_operand (mem_oper->src[0]);
      break;

    case TAHOEop_LD8_FILL:
      L_change_opcode (mem_oper, Lop_LD_POST_Q);
      /* L_change_opcode clobbers proc_opc, so it needs to be reset. */
      /* mem_oper->proc_opc = TAHOEop_LD8_FILL_POST; */
      mem_oper->proc_opc = TAHOEop_LD8_FILL;
      mem_oper->opcode = L_opcode_name (mem_oper->opc);
      mem_oper->src[1] = inc_operand;
      mem_oper->dest[1] = L_copy_operand (mem_oper->src[0]);
      break;

    case TAHOEop_LDFS:
      L_change_opcode (mem_oper, Lop_LD_POST_F);
      /* L_change_opcode clobbers proc_opc, so it needs to be reset. */
      /* mem_oper->proc_opc = TAHOEop_LDF_S_POST; */
      mem_oper->proc_opc = TAHOEop_LDFS;
      mem_oper->src[1] = inc_operand;
      mem_oper->dest[1] = L_copy_operand (mem_oper->src[0]);
      break;

    case TAHOEop_LDFD:
      L_change_opcode (mem_oper, Lop_LD_POST_F2);
      /* L_change_opcode clobbers proc_opc, so it needs to be reset. */
      /* mem_oper->proc_opc = TAHOEop_LDF_D_POST; */
      mem_oper->proc_opc = TAHOEop_LDFD;
      mem_oper->src[1] = inc_operand;
      mem_oper->dest[1] = L_copy_operand (mem_oper->src[0]);
      break;

    case TAHOEop_LDF_FILL:
      L_change_opcode (mem_oper, Lop_LD_POST_F2);
      /* L_change_opcode clobbers proc_opc, so it needs to be reset. */
      /* mem_oper->proc_opc = TAHOEop_LDF_FILL_POST; */
      mem_oper->proc_opc = TAHOEop_LDF_FILL;
      mem_oper->src[1] = inc_operand;
      mem_oper->dest[1] = L_copy_operand (mem_oper->src[0]);
      break;

    case TAHOEop_ST1:
      L_change_opcode (mem_oper, Lop_ST_POST_C);
      /* L_change_opcode clobbers proc_opc, so it needs to be reset. */
      /* mem_oper->proc_opc = TAHOEop_ST1_POST; */
      mem_oper->proc_opc = TAHOEop_ST1;
      mem_oper->src[2] = inc_operand;
      mem_oper->dest[0] = L_copy_operand (mem_oper->src[0]);
      break;

    case TAHOEop_ST2:
      L_change_opcode (mem_oper, Lop_ST_POST_C2);
      /* L_change_opcode clobbers proc_opc, so it needs to be reset. */
      /* mem_oper->proc_opc = TAHOEop_ST2_POST; */
      mem_oper->proc_opc = TAHOEop_ST2;
      mem_oper->src[2] = inc_operand;
      mem_oper->dest[0] = L_copy_operand (mem_oper->src[0]);
      break;

    case TAHOEop_ST4:
      L_change_opcode (mem_oper, Lop_ST_POST_I);
      /* L_change_opcode clobbers proc_opc, so it needs to be reset. */
      /* mem_oper->proc_opc = TAHOEop_ST4_POST; */
      mem_oper->proc_opc = TAHOEop_ST4;
      mem_oper->src[2] = inc_operand;
      mem_oper->dest[0] = L_copy_operand (mem_oper->src[0]);
      break;

    case TAHOEop_ST8:
      L_change_opcode (mem_oper, Lop_ST_POST_Q);
      /* L_change_opcode clobbers proc_opc, so it needs to be reset. */
      /* mem_oper->proc_opc = TAHOEop_ST8_POST; */
      mem_oper->proc_opc = TAHOEop_ST8;
      mem_oper->src[2] = inc_operand;
      mem_oper->dest[0] = L_copy_operand (mem_oper->src[0]);
      break;

    case TAHOEop_ST8_SPILL:
      L_change_opcode (mem_oper, Lop_ST_POST_I);
      /* L_change_opcode clobbers proc_opc, so it needs to be reset. */
      /* mem_oper->proc_opc = TAHOEop_ST8_SPILL_POST; */
      mem_oper->proc_opc = TAHOEop_ST8_SPILL;
      mem_oper->src[2] = inc_operand;
      mem_oper->dest[0] = L_copy_operand (mem_oper->src[0]);
      break;

    case TAHOEop_STFS:
      L_change_opcode (mem_oper, Lop_ST_POST_F);
      /* L_change_opcode clobbers proc_opc, so it needs to be reset. */
      /* mem_oper->proc_opc = TAHOEop_STF_S_POST; */
      mem_oper->proc_opc = TAHOEop_STFS;
      mem_oper->src[2] = inc_operand;
      mem_oper->dest[0] = L_copy_operand (mem_oper->src[0]);
      break;

    case TAHOEop_STFD:
      L_change_opcode (mem_oper, Lop_ST_POST_F2);
      /* L_change_opcode clobbers proc_opc, so it needs to be reset. */
      /* mem_oper->proc_opc = TAHOEop_STF_D_POST; */
      mem_oper->proc_opc = TAHOEop_STFD;
      mem_oper->src[2] = inc_operand;
      mem_oper->dest[0] = L_copy_operand (mem_oper->src[0]);
      break;

    case TAHOEop_STF_SPILL:
      L_change_opcode (mem_oper, Lop_ST_POST_F2);
      /* L_change_opcode clobbers proc_opc, so it needs to be reset. */
      /* mem_oper->proc_opc = TAHOEop_STF_SPILL_POST; */
      mem_oper->proc_opc = TAHOEop_STF_SPILL;
      mem_oper->src[2] = inc_operand;
      mem_oper->dest[0] = L_copy_operand (mem_oper->src[0]);
      break;

    default:
      L_punt ("M_convert_mem_oper_to_post_incr: Unknown tahoeop "
	      "oper:%d  tahoeop:%d\n", mem_oper->id, mem_oper->proc_opc);
    }				/* switch */
}				/* Convert_mem_op_to_post_inc */


/****************************************************************************
  routine: L_live_out_inhibits_scheduling()
  purpose: Return 0 if the given operand is not live out
           anywhere in the given cb, with some exceptions.  The operand may
           be live out of the target of a branch that has the given cb as its
           destination as long as that branch is not above the given add oper.

  input: cb - block to process.
         operand - operand being checked
         add - a branch to the given cb must not be above this oper
  output:
  returns: 0 if not live out as described above.  1 otherwise.
  modified: 9/6/97 - Dan Lavery - created
  note:
 ***************************************************************************/


static int
L_live_out_inhibits_scheduling (L_Cb * cb, L_Operand * operand, L_Oper * add)
{
  L_Oper *oper;
  L_Oper *last;
  L_Cb *target;
  int past_add = 0;

  for (oper = cb->first_op; oper; oper = oper->next_op)
    {
      if (oper == add)
	past_add = 1;

      if (L_general_branch_opcode (oper))
	{
	  target = L_find_branch_dest (oper);
	  if (((target != cb) || !past_add) &&
	      L_in_oper_OUT_set (cb, oper, operand, TAKEN_PATH))
	    return 1;
	}
    }

  last = cb->last_op;

  if (!L_general_branch_opcode (last) ||
      L_cond_branch_opcode (last) ||
      (L_uncond_branch_opcode (last) && L_is_predicated (last) &&
       !L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK_NO_FALLTHRU)))
    {
      if (cb->next_cb && L_in_cb_IN_set (cb->next_cb, operand))
	return 1;
    }

  return 0;
}

/****************************************************************************
 *
 * routine: Mia_cb_post_increment ()
 * purpose: Finds ld/add or st/add pairs and convert them into a post increment
 *          memory instruction.  This is a very limited routine.  It only
 *          operates on pairs that are within the same basic block.
 *          This is run before scheduling, and we don't want to inhibit
 *          scheduling of the memory instructions.  Therefore the address
 *          must not be live out of any exit from the cb.  Modulo scheduling
 *          cannot rename the address register in a post increment load or
 *          store.  Therefore there memory instruction must be only one use of 
 *          the address. 
 *
 * input: cb - block to process.
 * output:
 * returns:
 * modified: 9/6/97 - Dan Lavery - created from Bob's Simple_post_increment
 * note: Looks for the pattern:
 *          (p) ld|st   rW = [rX]
 *          (p) add     rX = rY|C,rX
 *       and replaces it with
 *          (p) ld|st   rW = [rX],rY|C
 *       rY|C means either a register or a constant.
 *       The predicates must be the same.
 *       Do not run copy propagation after post-increment conversion. ????
 *       Data flow analysis needs to be run before this to tell the live out
 *       registers on each branch.
 *
 * 08/28/03 REK The increment operand for a store must be an IMM9 constant.
 *              Only loads can use a register.
 *-------------------------------------------------------------------------*/

static void
Mia_cb_post_increment (L_Cb * cb, int conservative, int ld, int st)
{
  L_Oper *opA;
  L_Oper *opB;
  L_Operand *address;

  for (opA = cb->first_op; opA; opA = opA->next_op)
    {
      if (!(ld && L_general_load_opcode (opA)) &&
	  !(st && L_store_opcode (opA)))
	continue;

      address = opA->src[0];

#ifdef DEBUG
      fprintf (stderr, "cb %d, ld/st op %d, address in ", cb->id, opA->id);
      L_print_operand (stderr, address, 0);
      fprintf (stderr, "\n");
#endif

      for (opB = opA->next_op; opB; opB = opB->next_op)
	{
	  if (L_general_branch_opcode (opB) &&
	      L_in_oper_OUT_set (cb, opB, address, TAKEN_PATH))
	    break;

	  if (!L_int_add_opcode (opB) ||
	      !L_same_operand (address, opB->dest[0]))
	    continue;

#ifdef DEBUG
	  fprintf (stderr, "  add with matching dest reg\n");
	  if (L_same_operand (address, opB->src[1]) &&
	      L_same_operand (opA->pred[0], opB->pred[1]))
	    {
	      fprintf (stderr, "  add x = ?,x\n");
	      if (L_no_defs_between (address, opA, opB))
		fprintf (stderr, "  No address defs between\n");
	      if (L_no_uses_between (address, opA, opB))
		fprintf (stderr, "  No address uses between\n");
	    }
#endif

	  if (!L_same_operand (address, opB->src[1]) ||
	      !PG_equivalent_predicates_ops (opA, opB) ||
	      !L_no_defs_between (address, opA, opB) ||
	      !L_no_uses_between (address, opA, opB))
	    break;

	  if (L_is_int_constant (opB->src[0]))
	    {
	      if (!SIMM_9 (opB->src[0]->value.i))
		break;
	    }
	  else if (!L_is_variable (opB->src[0]) ||
		   !L_no_defs_between (opB->src[0], opA, opB) ||
		   (st && L_store_opcode (opA)))
	    {
	      break;
	    }

	  if (!L_no_danger ((L_is_macro (address) ||
			     L_is_macro (opB->src[0])), 0, 0, opA, opB))
	    break;

	  if (conservative)
	    {
	      if (!L_no_other_def_use_in_cb (cb, address, opA, opB, NULL))
		break;

	      if (L_live_out_inhibits_scheduling (cb, address, opB))
		break;
	    }

	  /* pattern matched. */
#ifdef DEBUG_POST_INC
	  fprintf (stderr, "mem %d, add %d -> Post increment\n",
		   opA->id, opB->id);
#endif
	  Convert_mem_op_to_post_inc (opA, L_copy_operand (opB->src[0]));
	  L_delete_oper (cb, opB);
	  break;
	}
    }
  return;
}
