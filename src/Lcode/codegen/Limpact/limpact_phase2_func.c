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
/*************************************************************************\
 *
 *  File:  limpact_phase2_func.c
 *
 *  Description:
 *    Machine specific annotation, optimization and interfaces to 
 *    scheduler/register allocator  for IMPACT architecture.
 *
 *  Creation Date :  June 1993
 *
 *  Author:  Scott A. Mahlke, Roger A. Bringmann, Richard E. Hank, 
 *           John C. Gyllenhaal, Wen-mei Hwu
 *
 *
\************************************************************************/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "limpact_main.h"
#include <Lcode/l_opti.h>
#include <Lcode/l_pred_opti.h>

void
O_perform_init (L_Func * fn)
{
}

/* Do spill optimizations on hyperblock code.  
 * The register allocator cannot do these directly. -JCG 10/16/95
 */
void
O_hyperblock_spill_opti (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *op, *next_op;
  int loads_removed, stores_removed;
  int offset = 0;
  INT_Symbol_Table *offset_table;

  /* Process only functions with hyperblocks */
  if (!L_EXTRACT_BIT_VAL (fn->flags, L_FUNC_HYPERBLOCK))
    return;

  /* Perform dataflow analysis on function with hyperblocks */
  L_do_flow_analysis (fn, LIVE_VARIABLE | DOMINATOR_CB | REACHING_DEFINITION);

  loads_removed = 0;

  /* Build the predicate graph */
  PG_setup_pred_graph (fn);

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      /* Only to memory copy propagation on hyperblocks
       * (Register allocator has already done the other ones)
       */
      if (!L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK))
        continue;

      /* Remove redundant spill loads in hyperblock */
      loads_removed += L_local_memory_copy_propagation (cb);
    }

  /* Return now if no loads removed */
  if (loads_removed == 0)
    return;

  if (Limpact_print_hb_spill_opti_stats)
    {
      printf ("%i redundant spill loads removed from hyperblocks in %s.\n",
              loads_removed, fn->name);
    }

  /* Find and remove redundant spill stores */
  offset_table = INT_new_symbol_table ("offset_table", 64);

  /* Find all the spill locations loaded from */
  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (op = cb->first_op; op != NULL; op = op->next_op)
        {
          /* Process only spill loads */
          if (!L_EXTRACT_BIT_VAL (op->flags, L_OPER_SPILL_CODE) ||
              !L_general_load_opcode (op))
            continue;

          /* Get the offset (int in src[0] or src[1]) */
          if (L_is_int_constant (op->src[0]))
            offset = op->src[0]->value.i;
          else if (L_is_int_constant (op->src[1]))
            offset = op->src[1]->value.i;
          else
            L_punt ("%s op %i: spill load offset not found!",
                    fn->name, op->id);

          /* Add to offset table */
          if (INT_find_symbol (offset_table, offset) == NULL)
            INT_add_symbol (offset_table, offset, (void *) NULL);
        }
    }

  stores_removed = 0;
  /* Remove spill stores with no corresponding spill loads */
  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (op = cb->first_op; op != NULL; op = next_op)
        {
          /* Get next op before deleting anything */
          next_op = op->next_op;

          /* Process only spill stores */
          if (!L_EXTRACT_BIT_VAL (op->flags, L_OPER_SPILL_CODE) ||
              !L_general_store_opcode (op))
            continue;

          /* Get the offset (int in src[0] or src[1]) */
          if (L_is_int_constant (op->src[0]))
            offset = op->src[0]->value.i;
          else if (L_is_int_constant (op->src[1]))
            offset = op->src[1]->value.i;
          else
            L_punt ("%s op %i: spill store offset not found!",
                    fn->name, op->id);

          /* If not in offset table, remove */
          if (INT_find_symbol (offset_table, offset) == NULL)
            {
              L_delete_oper (cb, op);
              stores_removed++;
            }
        }
    }
  if ((stores_removed != 0) && Limpact_print_hb_spill_opti_stats)
    {
      printf ("%i redundant spill stores removed from hyperblocks in %s.\n",
              stores_removed, fn->name);
    }
  return;
}

/*
 *      Currently only prepass, reg alloc, postpass scheduling.  Other normal
 *      stuff done during code generation is omitted from IMPACT code generator
 */
void
O_process_func (L_Func * fn, Parm_Macro_List * command_line_macro_list)
{
  int spill_space;

  if (L_debug_messages)
    fprintf (stderr, "Optimizing %s\n", fn->name);

  /*
   * Perform any necessary function level initialization
   */

  O_perform_init (fn);

  /*
   * Perform machine level code optimizations.
   *
   * Here is where we will perform:
   * 1) common subexpression ellimination.
   * 2) limited copy propogation (only R-R, R-M, M-R, M-M)
   * 3) dead code removal (unused operations, src[0]=dest[0])
   *
   */

  if (L_do_machine_opt)
    {
      L_split_pred_defines (fn);
      PG_setup_pred_graph (fn);
      if (L_lightweight_pred_opti (fn))
	{
	  L_split_pred_defines (fn);
	  PG_setup_pred_graph (fn);     /* Required by L_same_def_reachs */
	}
      L_combine_pred_defines (fn);
      PG_setup_pred_graph (fn);

      Mopti_perform_optimizations (fn, command_line_macro_list);
    }

  /*
   * Perform pre-regalloc peep-hole optimization
   */

  if (L_do_peephole_opt)
    {
    }

  /*
   * Software Pipelining
   */

  if (L_do_software_pipelining)
    {
      if (L_debug_messages)
        fprintf (stderr, "  Software pipelining...");

      Lpipe_software_pipeline (fn);

      if (L_debug_messages)
        fprintf (stderr, "  done\n");
    }

  /*
   * Pre-pass code scheduling:
   *
   */
  if (L_do_prepass_sched)
    {
      if (L_debug_messages)
        fprintf (stderr, "  Pre-pass code scheduling...");

      if (M_arch != M_IMPACT)
        L_punt ("O_process_func: illegal target architecture");

      SM_schedule_fn (fn, lmdes, 1);

      if (L_debug_messages)
        fprintf (stderr, "done\n");

      if (Limpact_print_prepass_schedule)
        L_print_func (prepass_out_file, L_fn);
    }

  /*
   * Perform register allocation
   *
   * global information available after register allocation
   *
   * spill_space_required
   * number_of_registers
   */

  if (L_do_register_allocation)
    {
      if (L_debug_messages)
        fprintf (stderr, "  Register Allocation\n"
                 "  ======================================\n");

      O_register_allocation (fn, command_line_macro_list, &spill_space);

      if (L_debug_messages)
        fprintf (stderr, "  --------------------------------------\n");

      if (Limpact_do_hb_spill_opti &&
          L_EXTRACT_BIT_VAL (fn->flags, L_FUNC_HYPERBLOCK))
        O_hyperblock_spill_opti (fn);
    }

  /*
   * Reset L_CB_SOFTPIPE flag for software pipelined loops that contain 
   * spill code so that postpass scheduling can schedule the spill code 
   */

  if (L_do_software_pipelining)
    {
      if (L_debug_messages)
        fprintf (stderr, "  Checking for spill code in pipelined loops...");

      Lpipe_mark_loops_with_spills (fn);

      if (L_debug_messages)
        fprintf (stderr, "done\n");
    }
  
  /*
   * Perform post "register allocation" code annotation.
   *
   * Certain commands such as prologue and epilogue can not be annotated
   * until all of the register characteristics and memory requirements
   * are known.
   *
   */

  if (L_do_register_allocation && Limpact_adjust_memory_stack)
    {
      if (L_debug_messages)
        fprintf (stderr, "  Performing postpass memory stack adjustment...");
      O_postpass_adjust_memory_stack (fn, spill_space);

      if (L_debug_messages)
	fprintf (stderr, "done\n");
    }

  if (L_do_postpass_code_annotation)
    {
    }

  /*
   * Perform peep-hole optimization
   *
   * This is where we will perform peephole optimization
   */

  if (L_do_peephole_opt)
    {
      O_insert_pred_block_ops (fn);
    }

  /*
   * Perform post-pass instruction scheduling and filling of delay slots.
   */
  if (L_do_postpass_sched)
    {
      if (L_debug_messages)
        fprintf (stderr, "  Post-pass code scheduling...");

      if (M_arch != M_IMPACT)
        L_punt ("O_process_func: illegal target architecture");

      SM_schedule_fn (fn, lmdes, 0);

      O_mask_promoted_pei (fn);

      if (L_debug_messages)
        fprintf (stderr, "done\n");
    }

  L_remove_empty_cbs (fn);

  return;
}

/*
 * Global initializations
 */
void
O_init (Parm_Macro_List * command_line_macro_list)
{
  O_register_init ();

  /* Perform mdes initialization (must be .lmdes2 file for list 
   * scheduling) -JCG 6/99
   */
  L_init_lmdes2 (L_lmdes_file_name, L_max_pred_operand, L_max_dest_operand,
                 L_max_src_operand, 4 /* Support up to 4 sync operands */ );

  /* Sanity check, can only use schedule manager if using lmdes2 file */
  if (lmdes->mdes2 == NULL)
    L_punt ("Limpact: Only supports .lmdes2 files now.\n"
            "Cannot use '%s'\n", lmdes->file_name);

  /* Read in SM parameters */
  SM_init (command_line_macro_list);

  if (L_do_software_pipelining)
    Lpipe_init (command_line_macro_list);
  return;
}

/*
 * Global initializations
 */
void
O_cleanup ()
{
  if (L_do_software_pipelining)
    Lpipe_cleanup ();
  return;
}

void
O_mask_promoted_pei (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *oper;

  /* Loop through every oper of the function */
  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
        {
          if (!L_is_pei (oper))
            continue;
          /*
           * For each promoted PEI, set the <M> flag and clear the <F> flag
           */
          if (L_EXTRACT_BIT_VAL (oper->flags, L_OPER_PROMOTED))
            {
              oper->flags = L_SET_BIT_FLAG (oper->flags, L_OPER_MASK_PE);
              oper->flags = L_CLR_BIT_FLAG (oper->flags, L_OPER_SAFE_PEI);
            }
        }
    }
}
