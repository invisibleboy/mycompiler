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
 *  File: mia_shladd.c
 *
 *  Description:
 *      Convert a shladd with R0 and an add to a shladd instruction.
 *
 *  Authors:
 *
\*****************************************************************************/
/* 09/17/02 REK Updating file to use function from libtahoeop instead of
 *              Tmdes. */

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "mia_internal.h"

#undef DEBUG_SHLADD_MERGE
#undef DEBUG_SHLADDP4

/* function prototypes */
static int M_local_shift_add_merge (L_Cb * cb);
static int M_global_shift_add_merge (L_Cb * cbA, L_Cb * cbB);

/* dataflow with reaching_defs must have been done */

static int
M_does_operand_contain_label_addr (L_Oper * oper,
				   L_Operand * operand, int *reaching_df_done)
{
  L_Oper *def_oper;
  Set def_set;
  int num_defs, *def_buf;
  int i, value = FALSE;

  if (!L_is_macro (operand))
    {
      def_set = L_get_reaching_defs (operand, oper, reaching_df_done);

      if (def_set)
	{
	  value = TRUE;
	  num_defs = Set_size (def_set);
	  def_buf = (int *) Lcode_malloc (sizeof (int) * (num_defs + 1));
	  (void) Set_2array (def_set, def_buf);

	  for (i = 0; i < num_defs; i++)
	    {
	      def_oper = L_oper_hash_tbl_find_oper (L_fn->oper_hash_tbl,
						    def_buf[i]);
	      if (def_oper->proc_opc != TAHOEop_MOVL)
		{
		  value = FALSE;
		  break;
		}		/* if */
	    }			/* for i */
	  Lcode_free (def_buf);
	}			/* if */
    }				/* if */
  return (value);
}				/* M_does_operand_contain_label_addr */


/****************************************************************************
 *
 * routine: M_local_shladdp4
 * purpose: compbine sxt and shladd into shladdp4
 * input: cb - block to process
 * output:
 * returns:  number of changes
 * modified: Created JEP 1/98
 *           
 * note:  Changes
 
             sxt      r1 = r2
             shladd   r3 = r1,n,rb

     into 

             sxt      r1 = r2
             shladdp4 r3 = r2,n,rb

No opers are removed.  The proc_opc is changed from shladd to shladdp4.
Some attempt is made to make sure that rb is a base register and the
upper bits can be swizzled.  

 *-------------------------------------------------------------------------*/

#if 0

int
M_local_shladdp4 (L_Cb * cb, int *reaching_df_done)
{
  int change = 0;
  L_Oper *opA, *opB;

  /*   fprintf(stderr, "***********   In cb %d *************\n", cb->id); */

  for (opA = cb->first_op; opA; opA = opA->next_op)
    {
      if (opA->proc_opc != TAHOEop_SXT4)
	continue;

      for (opB = opA->next_op; opB; opB = opB->next_op)
	{
	  int macro_flag;

	  if (!(opB->proc_opc == TAHOEop_SHLADD))
	    continue;

	  if (!L_same_operand (opA->dest[0], opB->src[0]))
	    continue;

	  /* Conservatively, need to make sure that base is positive
	     and swizzleable */

	  if (!M_does_operand_contain_label_addr (opB, opB->src[2],
						  reaching_df_done))
	    continue;

	  if (!PG_superset_predicate_ops (opA, opB))
	    continue;

	  if (!L_no_defs_between (opA->dest[0], opA, opB))
	    break;

	  if (!L_same_def_reachs (opA->src[0], opA, opB))
	    continue;

	  macro_flag = (L_is_fragile_macro (opA->dest[0]) ||
			L_is_fragile_macro (opA->src[0]));

	  if (!L_no_danger
	      (macro_flag, 0 /*load_flag */ , 0 /* store_flag */ ,
	       opA, opB))
	    break;

	  /* 
	   *  Replace Pattern
	   */

#ifdef DEBUG_SHLADDP4
	  fprintf (stderr, "Local shladdp4: op%d -> op%d, (cb %d)\n",
		   opA->id, opB->id, cb->id);
#endif

	  opB->proc_opc = TAHOEop_SHLADDP4;
	  L_delete_operand (opB->src[0]);
	  opB->src[0] = L_copy_operand (opA->src[0]);
	  change += 1;
	}			/* for opB */
    }				/* for opA */
  return change;
}				/* M_local_shladdp4 */

#endif

void
Mopti_shladd (L_Func * fn)
{
  L_Cb *cb1;
  L_Cb *cb2;

  L_alloc_danger_ext = L_create_alloc_pool ("L_Danger_Ext",
					    sizeof (struct L_Danger_Ext), 64);

  MOD ("Flow analysis DOM AD AE LV due to shladd");
  L_do_flow_analysis (fn, DOMINATOR_CB | AVAILABLE_DEFINITION |
		      AVAILABLE_EXPRESSION | LIVE_VARIABLE);

  for (cb1 = fn->first_cb; cb1 != NULL; cb1 = cb1->next_cb)
    while (M_local_shift_add_merge (cb1));

  L_compute_danger_info (fn);
  MOD ("Flow analysis DOM AD AE LV due to shladd");
  L_do_flow_analysis (fn, DOMINATOR_CB | AVAILABLE_DEFINITION |
		      AVAILABLE_EXPRESSION | LIVE_VARIABLE);

  for (cb1 = fn->first_cb; cb1 != NULL; cb1 = cb1->next_cb)
    {
      for (cb2 = fn->first_cb; cb2 != NULL; cb2 = cb2->next_cb)
	{
	  if (cb1->id == cb2->id)
	    continue;
	  while (M_global_shift_add_merge (cb1, cb2));
	}			/* for cb2 */
    }				/* for cb1 */
  L_delete_all_danger_ext (fn);

  L_free_alloc_pool (L_alloc_danger_ext);
  L_alloc_danger_ext = NULL;
}				/* Mopti_shladd */

static int
M_local_shift_add_merge (L_Cb * cb)
{
  int change = 0;
  L_Oper *opA, *opB, *new_oper;
  L_Oper *next;

  for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
    {
      if ((opA->proc_opc != TAHOEop_SHLADD) ||
	  !LT_is_R0_operand (opA->src[2]) || L_has_unsafe_macro_operand (opA))
	continue;

      for (opB = opA->next_op; opB; opB = next)
	{
	  int macro_flag;

	  next = opB->next_op;
	  /* 
	   *  Match Pattern
	   */
	  if (!(opB->proc_opc == TAHOEop_ADD))
	    continue;
	  if (!L_is_variable (opB->src[0]))
	    continue;
	  if (!L_is_variable (opB->src[1]))
	    continue;
	  if (!((L_same_operand (opA->dest[0], opB->src[0]) &&
		 !L_same_operand (opA->dest[0], opB->src[1])) ||
		(L_same_operand (opA->dest[0], opB->src[1]) &&
		 !L_same_operand (opA->dest[0], opB->src[0]))))
	    continue;
	  if (!PG_superset_predicate_ops (opA, opB))
	    continue;
	  if (!L_no_defs_between (opA->dest[0], opA, opB))
	    break;
	  if (!L_same_def_reachs (opA->src[0], opA, opB))
	    continue;
	  macro_flag = (L_is_fragile_macro (opA->dest[0]) ||
			L_is_fragile_macro (opA->src[0]));
	  if (!L_no_danger
	      (macro_flag, 0 /*load_flag */ , 0 /* store_flag */ ,
	       opA, opB))
	    break;

	  /* 
	   *  Replace Pattern
	   */
#ifdef DEBUG_SHLADD_MERGE
	  fprintf (stderr, "Local shift-add merge: op%d -> op%d, (cb %d)\n",
		   opA->id, opB->id, cb->id);
#endif
	  new_oper = L_create_new_op_using (Lop_LSL, opB);
	  new_oper->proc_opc = TAHOEop_SHLADD;
	  if (L_same_operand (opA->dest[0], opB->src[0]))
	    new_oper->src[2] = L_copy_operand (opB->src[1]);
	  else
	    new_oper->src[2] = L_copy_operand (opB->src[0]);

	  new_oper->src[0] = L_copy_operand (opA->src[0]);
	  new_oper->src[1] = L_copy_operand (opA->src[1]);
	  new_oper->dest[0] = L_copy_operand (opB->dest[0]);
	  L_insert_oper_after (cb, opB, new_oper);
	  L_nullify_operation (opB);
	  next = new_oper->next_op;
	  change += 1;
	}			/* for opB */
    }				/* for opA */
  return change;
}				/* M_local_shift_add_merge */

static int
M_global_shift_add_merge (L_Cb * cbA, L_Cb * cbB)
{
  int change = 0;
  L_Oper *opA, *opB, *new_oper;
  L_Oper *next;

  for (opA = cbA->first_op; opA != NULL; opA = opA->next_op)
    {
      for (opB = cbB->first_op; opB != NULL; opB = next)
	{
	  int macro_flag, load_flag, store_flag;
	  next = opB->next_op;

	  /* 
	   *  Match Pattern
	   */
	  if (!(opA->proc_opc == TAHOEop_SHLADD))
	    break;
	  if (!(LT_is_R0_operand (opA->src[2])))
	    break;
	  if (L_has_unsafe_macro_operand (opA))
	    break;
	  if (!(opB->proc_opc == TAHOEop_ADD))
	    continue;
	  if (!(L_is_register (opB->src[0]) || L_is_macro (opB->src[0])))
	    continue;
	  if (!(L_is_register (opB->src[1]) || L_is_macro (opB->src[1])))
	    continue;
	  if (!((L_same_operand (opA->dest[0], opB->src[0]) &&
		 !L_same_operand (opA->dest[0], opB->src[1])) ||
		(L_same_operand (opA->dest[0], opB->src[1]) &&
		 !L_same_operand (opA->dest[0], opB->src[0]))))
	    continue;
	  if (!PG_superset_predicate_ops (opA, opB))
	    continue;

	  if (!L_global_no_defs_between (opA->dest[0], cbA, opA, cbB, opB))
	    break;

	  if (!L_global_same_def_reachs (opA->src[0], cbA, opA, cbB, opB))
	    continue;
	  macro_flag = (L_is_fragile_macro (opA->dest[0]) ||
			L_is_fragile_macro (opA->src[0]));
	  load_flag = 0;
	  store_flag = 0;
	  if (!L_global_no_danger
	      (macro_flag, load_flag, store_flag, cbA, opA, cbB, opB))
	    break;
	  /* 
	   *  Replace Pattern
	   */
#ifdef DEBUG_SHLADD_MERGE
	  fprintf (stderr,
		   "Global shift-add merge: op%d (cb %d) -> op%d (cb %d)\n",
		   opA->id, cbA->id, opB->id, cbB->id);
#endif
	  new_oper = L_create_new_op_using (Lop_LSL, opB);
	  new_oper->proc_opc = TAHOEop_SHLADD;

	  if (L_same_operand (opA->dest[0], opB->src[0]))
	    new_oper->src[2] = L_copy_operand (opB->src[1]);
	  else
	    new_oper->src[2] = L_copy_operand (opB->src[0]);

	  new_oper->src[0] = L_copy_operand (opA->src[0]);
	  new_oper->src[1] = L_copy_operand (opA->src[1]);
	  new_oper->dest[0] = L_copy_operand (opB->dest[0]);
	  L_insert_oper_after (cbB, opB, new_oper);
	  L_nullify_operation (opB);
	  next = new_oper->next_op;
	  change += 1;
	}			/* for opB */
    }				/* for opA */
  return change;
}				/* M_global_shift_add_merge */
