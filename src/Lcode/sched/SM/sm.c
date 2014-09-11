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
 *      File:   sm.c
 *      Author: John C. Gyllenhaal, Wen-mei Hwu
 *      Modulo Scheduling support: IMPACT Technologies (John Gyllenhaal)
 *      Creation Date:  March 1996
\*****************************************************************************/

#include <config.h>
#include <stdio.h>
#include "sm.h"
#include <library/l_alloc_new.h>
#include <library/l_parms.h>
#include <library/dynamic_symbol.h>
#include <library/heap.h>
#include <Lcode/l_main.h>
#include <machine/lmdes_interface.h>
#include <assert.h>

L_Alloc_Pool *SM_Func_pool = NULL;
L_Alloc_Pool *SM_Cb_pool = NULL;
L_Alloc_Pool *SM_Oper_pool = NULL;
L_Alloc_Pool *SM_Compatible_Alt_pool = NULL;
L_Alloc_Pool *SM_Issue_Group_pool = NULL;

/* Declare other pools so can free all of them */
extern L_Alloc_Pool *SM_Dep_pool;
extern L_Alloc_Pool *SM_Oper_Queue_pool;
extern L_Alloc_Pool *SM_Oper_Qentry_pool;
extern L_Alloc_Pool *SM_Action_Queue_pool;
extern L_Alloc_Pool *SM_Action_Qentry_pool;
extern L_Alloc_Pool *SM_Trans_Queue_pool;
extern L_Alloc_Pool *SM_Trans_Qentry_pool;
extern L_Alloc_Pool *SM_Reg_Action_pool;
extern L_Alloc_Pool *SM_Reg_Info_pool;
extern L_Alloc_Pool *SM_Reg_Action_Conflict_pool;
extern L_Alloc_Pool *SM_Trans_pool;
extern L_Alloc_Pool *SM_Priority_Queue_pool;
extern L_Alloc_Pool *SM_Priority_Qentry_pool;


/* Special operands used for mem, ctrl, sync, and vliw action operands */
L_Operand _sm_mem_action_operand;
L_Operand _sm_ctrl_action_operand;
L_Operand _sm_sync_action_operand;
L_Operand _sm_vliw_action_operand;

/* For now, phd parameter */
extern char *prof_info;
extern int suppress_lcode_output;

/* SM_schedule_fn parameters */
int SM_debug_use_sched_cb_bounds = 0;
int SM_debug_lower_sched_cb_bound = 0;
int SM_debug_upper_sched_cb_bound = 100000000;

/* General SM parameters */

/* Print dep graph whenever SM_new_cb called */
int SM_print_dependence_graph = 0;

/*
 * If set, makes sure the same dependences are specified for the
 * sm cb when the incoming or outgoing dependences are drawn.
 */
int SM_check_dependence_symmetry = 0;

int SM_verify_reg_conflicts = 1;

int SM_output_dep_distance = 0;

int SM_ignore_pred_analysis = 0;
int SM_perform_rename_with_copy = 0;
int SM_perform_relocate_cond_opti = 0;

/* JWS 20040105 */
int SM_sched_slack_loads_for_miss = 0;

/* 20031023 SZU
 * New parameter.
 * Indicates if template bundling (for IPF) should be done.
 * This allows for one SM module instead of SM and SMH.
 * Also specify if bundle compaction should be done.
 */
int SM_do_template_bundling = 0;
int SM_do_bundle_compaction = 0;

/* 20030607 SZU
 * Goes through the issue group and assigns stop bit
 * at the end of the last template.
 */
void
SM_assign_stop_bit (SM_Issue_Group * issue_group_ptr)
{
  int current_bundle, index, num_slots, slots_per_template, empty_bundle;
  int empty_issue = 1, last_bundle = 0;
  int template_index;
  SM_Template *template;

  num_slots = SM_get_num_slots (issue_group_ptr->sm_cb);
  slots_per_template = SM_get_slots_per_template (issue_group_ptr->sm_cb);

  empty_bundle = 1;
  for (index = 0; index < num_slots; index++)
    {
      /* If op present and not nop, then bundle not empty */
      if ((issue_group_ptr->slots[index]) &&
	  (!(SM_is_nop (issue_group_ptr->slots[index]))))
	{
	  empty_bundle = 0;
	  empty_issue = 0;
	}

      /* index is last slot of a bundle */
      if ((index % slots_per_template) == (slots_per_template - 1))
	{
	  current_bundle = index / slots_per_template;

	  /* 20030722 SZU
	   * If bundle is part of a compacted bundle, ie has internal stop bit,
	   * consider as non-empty.
	   */
	  if ((!empty_bundle) ||
	      (issue_group_ptr->bundles[current_bundle]->internal_stop_bit))
	    {
	      issue_group_ptr->bundles[current_bundle]->empty = 0;
	      empty_bundle = 1;
	      last_bundle = current_bundle;
	    }
	  else
	    {
	      issue_group_ptr->bundles[current_bundle]->empty = 1;
	      empty_bundle = 1;
	    }
	}
    }

  /* Assign stop bit to the last bundle in the last possible place */
  template_index = issue_group_ptr->bundles[last_bundle]->template_index; 
  template = &issue_group_ptr->sm_cb->sm_mdes->template_array[template_index];

  /* 20030718 SZU
   * New value for internal_stop_bit.
   * If internal_stop_bit == 2, that means 2nd half of bundle.
   * Therefore potentially add stop bit at end
   * 20030930 SZU
   * Do not do this for empty issues!
   */
  if (!empty_issue)
    {
      if ((issue_group_ptr->bundles[last_bundle]->internal_stop_bit == 2) ||
	  (!issue_group_ptr->bundles[last_bundle]->template_lock))
	{
	  for (index = slots_per_template - 1; index >= 0; index--)
	    {
	      if (template->stop_bits[index])
		{
		  issue_group_ptr->bundles[last_bundle]->stop = index;
		  break;
		}
	    }
	}
      /* Should have internal_stop_bit and stop set. Warn if otherwise. */
      else if ((!issue_group_ptr->bundles[last_bundle]->internal_stop_bit) ||
	       (issue_group_ptr->bundles[last_bundle]->stop < 0))
	{
	  printf ("SM_assign_stop_bit: Issue group is template locked.\n"
		  "But no internal stop bit or stop! Check!\n"
		  "cb %i issue time %i internal_stop_bit %i stop %i",
		  issue_group_ptr->sm_cb->lcode_cb->id,
		  issue_group_ptr->issue_time,
		  issue_group_ptr->bundles[last_bundle]->internal_stop_bit,
		  issue_group_ptr->bundles[last_bundle]->stop);
	}
    }
  /* If issue group is completely empty, somthing may be wrong */
  else
    {
      printf ("SM_assign_stop_bit: Completely empty issue found! Check!!\n"
	      "cb %i time %i\n", issue_group_ptr->sm_cb->lcode_cb->id,
	      issue_group_ptr->issue_time);
    }
}

/* 20030609 SZU
 * Copied from mckinley_mdes.c, from tmdes_instr.c.
 */
L_Oper *
SM_create_template_op (int template_type, int stop_bit_mask)
{
  L_Oper *new_oper;

  new_oper = L_create_new_op (Lop_DEFINE);
  new_oper->proc_opc = TAHOEop_NON_INSTR;
  new_oper->dest[0] = L_new_macro_operand (TAHOE_MAC_TEMPLATE,
					   L_CTYPE_INT, 0);
  M_new_template (new_oper, template_type);
  M_new_stop_bit_mask (new_oper, stop_bit_mask);
  return (new_oper);
}

/* Free all the pools allocated by the SM manager.
 * Primarily used to find memory leaks (for now)
 */
void
SM_free_alloc_pools ()
{
  if (SM_Func_pool != NULL)
    {
      L_free_alloc_pool (SM_Func_pool);
      SM_Func_pool = NULL;
    }

  if (SM_Cb_pool != NULL)
    {
      L_free_alloc_pool (SM_Cb_pool);
      SM_Cb_pool = NULL;
    }

  if (SM_Oper_pool != NULL)
    {
      L_free_alloc_pool (SM_Oper_pool);
      SM_Oper_pool = NULL;
    }

  /* 20021210 SZU */
  if (SM_Issue_Group_pool != NULL)
    {
      L_free_alloc_pool (SM_Issue_Group_pool);
      SM_Issue_Group_pool = NULL;
    }

  if (SM_Compatible_Alt_pool != NULL)
    {
      L_free_alloc_pool (SM_Compatible_Alt_pool);
      SM_Compatible_Alt_pool = NULL;
    }

  if (SM_Dep_pool != NULL)
    {
      L_free_alloc_pool (SM_Dep_pool);
      SM_Dep_pool = NULL;
    }

  if (SM_Oper_Queue_pool != NULL)
    {
      L_free_alloc_pool (SM_Oper_Queue_pool);
      SM_Oper_Queue_pool = NULL;
    }

  if (SM_Oper_Qentry_pool != NULL)
    {
      L_free_alloc_pool (SM_Oper_Qentry_pool);
      SM_Oper_Qentry_pool = NULL;
    }

  if (SM_Action_Queue_pool != NULL)
    {
      L_free_alloc_pool (SM_Action_Queue_pool);
      SM_Action_Queue_pool = NULL;
    }

  if (SM_Action_Qentry_pool != NULL)
    {
      L_free_alloc_pool (SM_Action_Qentry_pool);
      SM_Action_Qentry_pool = NULL;
    }

  if (SM_Trans_Queue_pool != NULL)
    {
      L_free_alloc_pool (SM_Trans_Queue_pool);
      SM_Trans_Queue_pool = NULL;
    }

  if (SM_Trans_Qentry_pool != NULL)
    {
      L_free_alloc_pool (SM_Trans_Qentry_pool);
      SM_Trans_Qentry_pool = NULL;
    }

  if (SM_Priority_Queue_pool != NULL)
    {
      L_free_alloc_pool (SM_Priority_Queue_pool);
      SM_Priority_Queue_pool = NULL;
    }

  if (SM_Priority_Qentry_pool != NULL)
    {
      L_free_alloc_pool (SM_Priority_Qentry_pool);
      SM_Priority_Qentry_pool = NULL;
    }

  if (SM_Reg_Action_pool != NULL)
    {
      L_free_alloc_pool (SM_Reg_Action_pool);
      SM_Reg_Action_pool = NULL;
    }

  if (SM_Reg_Info_pool != NULL)
    {
      L_free_alloc_pool (SM_Reg_Info_pool);
      SM_Reg_Info_pool = NULL;
    }

  if (SM_Reg_Action_Conflict_pool != NULL)
    {
      L_free_alloc_pool (SM_Reg_Action_Conflict_pool);
      SM_Reg_Action_Conflict_pool = NULL;
    }

  if (SM_Trans_pool != NULL)
    {
      L_free_alloc_pool (SM_Trans_pool);
      SM_Trans_pool = NULL;
    }
}

/* Free all the pools allocated by the SM manager.
 * Primarily used to find memory leaks (for now)
 */
void
SM_print_alloc_info (FILE * out, int verbose)
{
  if (SM_Func_pool != NULL)
    {
      L_print_alloc_info (out, SM_Func_pool, verbose);
    }

  if (SM_Cb_pool != NULL)
    {
      L_print_alloc_info (out, SM_Cb_pool, verbose);
    }

  if (SM_Oper_pool != NULL)
    {
      L_print_alloc_info (out, SM_Oper_pool, verbose);
    }

  if (SM_Compatible_Alt_pool != NULL)
    {
      L_print_alloc_info (out, SM_Compatible_Alt_pool, verbose);
    }

  if (SM_Dep_pool != NULL)
    {
      L_print_alloc_info (out, SM_Dep_pool, verbose);
    }

  if (SM_Oper_Queue_pool != NULL)
    {
      L_print_alloc_info (out, SM_Oper_Queue_pool, verbose);
    }

  if (SM_Oper_Qentry_pool != NULL)
    {
      L_print_alloc_info (out, SM_Oper_Qentry_pool, verbose);
    }

  if (SM_Action_Queue_pool != NULL)
    {
      L_print_alloc_info (out, SM_Action_Queue_pool, verbose);
    }

  if (SM_Action_Qentry_pool != NULL)
    {
      L_print_alloc_info (out, SM_Action_Qentry_pool, verbose);
    }

  if (SM_Trans_Queue_pool != NULL)
    {
      L_print_alloc_info (out, SM_Trans_Queue_pool, verbose);
    }

  if (SM_Trans_Qentry_pool != NULL)
    {
      L_print_alloc_info (out, SM_Trans_Qentry_pool, verbose);
    }

  if (SM_Reg_Action_pool != NULL)
    {
      L_print_alloc_info (out, SM_Reg_Action_pool, verbose);
    }

  if (SM_Reg_Info_pool != NULL)
    {
      L_print_alloc_info (out, SM_Reg_Info_pool, verbose);
    }

  if (SM_Reg_Action_Conflict_pool != NULL)
    {
      L_print_alloc_info (out, SM_Reg_Action_Conflict_pool, verbose);
    }

  if (SM_Trans_pool != NULL)
    {
      L_print_alloc_info (out, SM_Trans_pool, verbose);
    }
}

/* For now, use build list from mdes built list of compatable_alt (sp) list
 * Do allow an version1_mdes to be specified and do the appropriate global
 * variable manipulations to use this mdes.
 */
SM_Compatible_Alt *
SM_build_compatible_alt_list (L_Oper * lcode_op, Mdes * version1_mdes)
{
  SM_Compatible_Alt *first_compatible_alt, *last_compatible_alt;
  SM_Compatible_Alt *compatible_alt;
  Mdes_Info *mdes_info, *old_mdes_info;
  Mdes_Compatable_Alt *mdes_alt;
  Mdes *old_version1_mdes;
  Mdes_Alt *normal_alt, *silent_alt;
  int silent_versions, paired_versions;

  /* Make the passed in mdes the current lmdes */
  old_version1_mdes = lmdes;
  lmdes = version1_mdes;

  /* Initially list is empty */
  first_compatible_alt = NULL;
  last_compatible_alt = NULL;

  /* Create SM_Compatible_Alt_pool if necessary */
  if (SM_Compatible_Alt_pool == NULL)
    {
      SM_Compatible_Alt_pool =
        L_create_alloc_pool ("SM_Compatible_Alt",
                             sizeof (SM_Compatible_Alt), 128);
    }

  /* Preserve old mdes info before building for this mdes */
  old_mdes_info = lcode_op->mdes_info;
  L_build_oper_mdes_info (lcode_op);

  /* Get mdes info for ease of use */
  mdes_info = (Mdes_Info *) lcode_op->mdes_info;

  /* Do special processoing if there are silent versions of this
   * compatable alternative.
   */
  silent_versions = 0;

  /* For each compatable normal (non-silent) alt, build a 
   * new compatible alt 
   */
  for (mdes_alt = mdes_info->compatable_alts; mdes_alt != NULL;
       mdes_alt = mdes_alt->next)
    {
      if (mdes_alt->alt->alt_flags & ALT_FLAG_SILENT)
        {
          silent_versions++;
          continue;
        }

      /* Alloc a SM alt */
      compatible_alt = (SM_Compatible_Alt *) L_alloc (SM_Compatible_Alt_pool);

      /* Get the non-silent version from this mdes_alt */
      compatible_alt->normal_version = mdes_alt->alt;

      /* Initially, don't point to silent version */
      compatible_alt->silent_version = NULL;

      /* Add to end of compatible alt linked list */
      if (last_compatible_alt != NULL)
        last_compatible_alt->next_compatible_alt = compatible_alt;
      else
        first_compatible_alt = compatible_alt;
      last_compatible_alt = compatible_alt;

      compatible_alt->next_compatible_alt = NULL;
    }

  /* For now, if there are silent versions of an operation, require
   * that for every non-silent version, there is exactly one silent
   * version.  Otherwise, punt.
   * 20020831 JWS - ignore silent versions if !L_non_excepting_ops
   */
  if (L_non_excepting_ops && (silent_versions > 0))
    {
      paired_versions = 0;

      /* For each non-silent alt, find the silent version */
      for (compatible_alt = first_compatible_alt; compatible_alt != NULL;
           compatible_alt = compatible_alt->next_compatible_alt)
        {
          /* Get the normal alt for ease of use */
          normal_alt = compatible_alt->normal_version;

          /* Find the silent version for this 'normal' version */
          for (mdes_alt = mdes_info->compatable_alts; mdes_alt != NULL;
               mdes_alt = mdes_alt->next)
            {
              /* Look only at silent verions */
              if (!(mdes_alt->alt->alt_flags & ALT_FLAG_SILENT))
                continue;

              /* Get this silent version for ease of use */
              silent_alt = mdes_alt->alt;

              /* Stop if they are the same */
              if ((normal_alt->table == silent_alt->table) &&
                  (normal_alt->IO_item == silent_alt->IO_item) &&
                  (normal_alt->latency == silent_alt->latency))
                break;
            }

          /* Set silent_version field if found silent version */
          if (mdes_alt != NULL)
            {
              compatible_alt->silent_version = mdes_alt->alt;
              paired_versions++;
            }

          /* Otherwise, punt for now since assume pairing */
          else
            {
              L_punt ("SM_build_compatible_alt_list(%s op %i proc_opc %i):\n"
                      "\n    Only some 'normal' alts have silent versions!\n"
                      "    SM currently requires that all or none have"
                      " silent versions.\n"
                      "    Please modify you mdes file or enhance the SM.",
                      L_fn->name, lcode_op->id, lcode_op->proc_opc);
            }
        }

      /* Make sure there are no left over silent versions.
       * I am assuming the mdes optimizer will delete any duplicate
       * alternatives that could mess this check up.
       */
      if (silent_versions != paired_versions)
        {
          L_punt ("SM_build_compatible_alt_list(%s op %i proc_opc %i):\n"
                  "\n  Only some 'silent' alts have normal versions!\n"
                  "  SM currently requires that there be a normal version "
                  "for each silent version.\n"
                  "  Please modify your mdes or enhance the SM.",
                  L_fn->name, lcode_op->id, lcode_op->proc_opc);
        }
    }

  /* Free new mdes_info and restore original mdes_info */
  L_free_oper_mdes_info (lcode_op);
  lcode_op->mdes_info = old_mdes_info;

  /* Restore lmdes state */
  lmdes = old_version1_mdes;

  /* Return the list of compatible alts */
  return (first_compatible_alt);
}

void
SM_ignore_op (SM_Oper * sm_op, int flag)
{
  if (sm_op->flags & SM_OP_SCHEDULED)
    {
      L_punt ("SM_ignore_op: Cannot ignore scheduled op.");
    }

  if (sm_op->ignore)
    {
      sm_op->ignore |= flag;
      SM_ignore_dep_out (sm_op, flag);
      SM_ignore_dep_in (sm_op, flag);
      return;
    }
  sm_op->ignore |= flag;
  SM_ignore_dep_out (sm_op, flag);
  SM_ignore_dep_in (sm_op, flag);
  sm_op->sm_cb->num_ignored++;
  sm_op->sm_cb->num_unsched--;
  if (sm_op->dep_in_resolved_qentry != NULL)
    {
      SM_dequeue_oper (sm_op->dep_in_resolved_qentry);
      sm_op->dep_in_resolved_qentry = NULL;
    }
}

void
SM_enable_ignored_op (SM_Oper * sm_op, int flag)
{
  if (!(sm_op->ignore))
    return;

  if (sm_op->flags & SM_OP_SCHEDULED)
    {
      L_punt ("SM_enable_ignored_op: Ignored op scheduled.");
    }

  sm_op->ignore &= ~flag;
  SM_enable_ignored_dep_out (sm_op, flag);
  SM_enable_ignored_dep_in (sm_op, flag);
  if (!sm_op->ignore)
    {
      sm_op->sm_cb->num_ignored--;
      sm_op->sm_cb->num_unsched++;
      if ((sm_op->num_unresolved_hard_dep_in == 0) &&
          /* EMN (sm_op->num_unresolved_soft_dep_in == 0) && */
          (sm_op->dep_in_resolved_qentry == NULL) &&
          !(sm_op->flags & SM_OP_SCHEDULED))
        {
          sm_op->dep_in_resolved_qentry =
            SM_enqueue_oper_before (sm_op->sm_cb->dep_in_resolved,
                                    sm_op, NULL);
        }
    }
}

void
SM_init_sm_op_fields (SM_Oper * sm_op, SM_Cb * sm_cb, L_Oper * lcode_op)
{
  Mdes *version1_mdes;
  SM_Reg_Action **operand;
  int latency_count, i;
  int array_size;
  int proc_opc;


  /* Get the version1 mdes structure this sm_cb is using */
  version1_mdes = sm_cb->version1_mdes;

  /* Make sure this operation is defined in the mdes */
  proc_opc = lcode_op->proc_opc;

  if ((proc_opc > lmdes->max_opcode) ||
      (version1_mdes->op_table[proc_opc] == NULL))
    {
      fprintf (stderr,
               "\nSM_init_sm_op_fields:\n  Error processing %s op %i:\n    ",
               sm_cb->lcode_fn->name, lcode_op->id);

      L_print_oper (stderr, lcode_op);

      fprintf (stderr,
               "\n  Undefined proc_opc %i (%s) in lmdes2 file:\n"
               "    %s.\n\n",
               proc_opc, lcode_op->opcode, version1_mdes->file_name);

      L_punt ("Please define the proc_opc in the above lmdes2 file.");
    }

  /* Initialize the mdes flags */
  sm_op->mdes_flags = version1_mdes->op_table[proc_opc]->op_flags;

  /* Initialize the SM flags */
  sm_op->flags = 0;
  sm_op->ignore = 0;

  /* Build compatible alt list from version1_mdes */
  sm_op->first_compatible_alt = SM_build_compatible_alt_list (lcode_op,
                                                              version1_mdes);

  /* For now, if any compatible alt has a silent version, all must
   * have a silent version, so check only the first compatible alt
   * to see if it has a silent version.
   */
  if (sm_op->first_compatible_alt->silent_version != NULL)
    sm_op->flags |= SM_HAS_SILENT_VERSION;

  /* Allocate operand array using latency count */
  latency_count = version1_mdes->latency_count;
  operand = (SM_Reg_Action **) malloc (latency_count *
                                       sizeof (SM_Reg_Action *));
  if (operand == NULL)
    L_punt ("SM_init_sm_op_fields: Out of memory");

  /* Initialize all pointers to NULL */
  for (i = 0; i < latency_count; i++)
    operand[i] = NULL;

  /* Point operand fields at appropriate places in this array */
  sm_op->operand = operand;
  sm_op->dest = &operand[version1_mdes->offset[MDES_DEST]];
  sm_op->src = &operand[version1_mdes->offset[MDES_SRC]];
  sm_op->pred = &operand[version1_mdes->offset[MDES_PRED]];
  sm_op->ext_dest = &operand[version1_mdes->offset[MDES_SYNC_OUT]];
  sm_op->ext_src = &operand[version1_mdes->offset[MDES_SYNC_IN]];

  /* Create implicit dests queue for jsrs only (holds fragile macros used
   * in the sm_cb) 
   */
  if (sm_op->mdes_flags & OP_FLAG_JSR)
    sm_op->implicit_dests = SM_new_action_queue ();
  else
    sm_op->implicit_dests = NULL;

  /* Create implicit src queues for jsrs (holds fragile macros used in the
   * sm_cb) and branches (holds live out registers used in sm_cb).
   */
  /* 20030225 SZU
   * SMH reconciliation
   */
  if (sm_op->mdes_flags & (OP_FLAG_JSR | OP_FLAG_CBR | OP_FLAG_JMP |
                           OP_FLAG_CHK))
#if 0
  if (sm_op->mdes_flags & (OP_FLAG_JSR | OP_FLAG_CBR | OP_FLAG_JMP))
#endif
    sm_op->implicit_srcs = SM_new_action_queue ();
  else
    sm_op->implicit_srcs = NULL;

  /* Initially the no reg actions specfied for operation */
  sm_op->first_op_action = NULL;
  sm_op->last_op_action = NULL;


  /* Set here this sm_op is coming from */
  sm_op->lcode_op = lcode_op;
  sm_op->sm_cb = sm_cb;

  /* This op is currently not in any queues */
  sm_op->first_queue = NULL;

  /* Priorities have not been assigned yet */
  sm_op->priority = 0.0;

  /* Early and late times have not been built yet */
  sm_op->early_time = SM_MIN_CYCLE;
  sm_op->late_time = NULL;

  /* Get the mdes operation used to model this op */
  sm_op->mdes_op = version1_mdes->op_table[lcode_op->proc_opc];

  /* Set cycle fields to easy to debug values */
  sm_op->cycle_lower_bound = SM_MIN_CYCLE;
  sm_op->cycle_upper_bound = SM_MAX_CYCLE;
  /* 20030225 SZU
   * SMH reconciliation
   */
  sm_op->nosoft_cycle_lower_bound = SM_MIN_CYCLE;
  sm_op->nosoft_cycle_upper_bound = SM_MAX_CYCLE;
  sm_op->sched_cycle = sm_op->cycle_upper_bound;

  /* Set slot fields to easy to debug values */
  sm_op->slot_lower_bound = 0;
  sm_op->slot_upper_bound = SM_MAX_SLOT;
  /* 20030225 SZU
   * SMH reconciliation
   */
  sm_op->nosoft_slot_lower_bound = 0;
  sm_op->nosoft_slot_upper_bound = SM_MAX_SLOT;
  sm_op->sched_slot = sm_op->slot_upper_bound;

  /* Set to NULL to indicate no height bound is set */
  sm_op->dep_lower_bound = NULL;
  sm_op->dep_upper_bound = NULL;
  /* 20030225 SZU
   * SMH reconciliation
   */
  sm_op->nosoft_dep_lower_bound = NULL;
  sm_op->nosoft_dep_upper_bound = NULL;

  /* Set to NULL to indicate that this operation is not scheduled */
  sm_op->alt_chosen = NULL;

  /* Allocate option choice array able to handle the choices for any table */
  array_size = sizeof (unsigned short) * sm_cb->sm_mdes->max_num_choices;
  if ((sm_op->options_chosen = (unsigned short *) malloc (array_size)) ==
      NULL)
    L_punt ("SM_init_sm_op_fields: Out of memory");

  sm_op->num_hard_dep_in = 0;
  sm_op->num_unresolved_hard_dep_in = 0;
  sm_op->num_soft_dep_in = 0;
  sm_op->num_unresolved_soft_dep_in = 0;
  sm_op->num_ignore_dep_in = 0;
  sm_op->num_unresolved_ignore_dep_in = 0;

  sm_op->num_hard_dep_out = 0;
  sm_op->num_unresolved_hard_dep_out = 0;
  sm_op->num_soft_dep_out = 0;
  sm_op->num_unresolved_soft_dep_out = 0;
  sm_op->num_ignore_dep_out = 0;
  sm_op->num_unresolved_ignore_dep_out = 0;

  /* Initially, not in any sm managed queues */
  sm_op->dep_in_resolved_qentry = NULL;

  /* 20021210 SZU
   * Add bundling info initializer values
   */
  sm_op->issue_group = NULL;
  sm_op->syll_type = 0;
  sm_op->old_issue_time = -1;
  sm_op->qentry = NULL;
}

SM_Oper *
SM_find_sm_op (SM_Cb * sm_cb, L_Oper * l_oper)
{
  SM_Oper *sm_oper = sm_cb->first_serial_op;

  if (sm_cb == NULL)
    L_punt ("Lpipe_find_sm_oper: NULL pointer to sm_cb\n");

  while (sm_oper != NULL)
    {
      if (sm_oper->lcode_op == l_oper)
        {
          return sm_oper;
        }
      sm_oper = sm_oper->next_serial_op;
    }

  return NULL;
}

static void
SM_initialize_special_operands ()
{
  SM_MEM_ACTION_OPERAND->type = L_OPERAND_RESERVED;
  SM_MEM_ACTION_OPERAND->ctype = L_CTYPE_VOID;
  SM_MEM_ACTION_OPERAND->ptype = L_PTYPE_NULL;
  SM_MEM_ACTION_OPERAND->value.r = SM_MEM_ACTION_INDEX;

  SM_CTRL_ACTION_OPERAND->type = L_OPERAND_RESERVED;
  SM_CTRL_ACTION_OPERAND->ctype = L_CTYPE_VOID;
  SM_CTRL_ACTION_OPERAND->ptype = L_PTYPE_NULL;
  SM_CTRL_ACTION_OPERAND->value.r = SM_CTRL_ACTION_INDEX;

  SM_SYNC_ACTION_OPERAND->type = L_OPERAND_RESERVED;
  SM_SYNC_ACTION_OPERAND->ctype = L_CTYPE_VOID;
  SM_SYNC_ACTION_OPERAND->ptype = L_PTYPE_NULL;
  SM_SYNC_ACTION_OPERAND->value.r = SM_SYNC_ACTION_INDEX;

  SM_VLIW_ACTION_OPERAND->type = L_OPERAND_RESERVED;
  SM_VLIW_ACTION_OPERAND->ctype = L_CTYPE_VOID;
  SM_VLIW_ACTION_OPERAND->ptype = L_PTYPE_NULL;
  SM_VLIW_ACTION_OPERAND->value.r = SM_VLIW_ACTION_INDEX;
}


static void
SM_remove_matching_check (SM_Cb * sm_cb, SM_Oper * sm_op)
{
  SM_Oper *ck_op;
  L_Oper *lcode_op;
  L_Attr *attr;
  ITintmax id;

  lcode_op = sm_op->lcode_op;

  if (!(attr = L_find_attr (lcode_op->attr, "SPECID")))
    return;

  /* 20030220 SZU
   * SMH reconciliation
   */
  /* Don't remove promoted checks (The rule for removal is
   *   different and will be handled later)
   */
  if (L_EXTRACT_BIT_VAL (lcode_op->flags, L_OPER_PROMOTED))
    return;

  id = attr->field[0]->value.i;

  lcode_op->attr = L_delete_attr (lcode_op->attr, attr);

  for (ck_op = sm_cb->last_serial_op; ck_op != NULL;
       ck_op = ck_op->prev_serial_op)
    {
      lcode_op = ck_op->lcode_op;
      if (!(lcode_op->flags & L_OPER_CHECK))
        continue;
      attr = L_find_attr (lcode_op->attr, "SPECID");
      if (!attr || attr->field[0]->value.i != id)
        continue;
      /* 20030220 SZU
       * SMH reconciliation
       */
      L_change_opcode(lcode_op, Lop_NO_OP);
#if 0
      SM_delete_oper (ck_op);
#if 0
      printf ("Deleted: ");
      L_print_oper (stdout, lcode_op);
#endif
      L_delete_oper (sm_cb->lcode_cb, lcode_op);
#endif
      sm_cb->chk_list = List_remove (sm_cb->chk_list, lcode_op);
      break;
    }
  return;
}


static void
SM_remove_if_unused (L_Cb * cb, L_Oper * chk_op)
{
  L_Attr *attr = NULL;
  L_Oper *op = NULL;
  ITintmax id;

  if (chk_op->opc != Lop_CHECK)
    L_punt ("SM_remove_if_unused: op not a check\n");
  if (!(attr = L_find_attr (chk_op->attr, "SPECID")))
    L_punt ("SM_remove_if_unused: check not speculative\n");

  id = attr->field[0]->value.i;

  for (op = chk_op->prev_op; op; op = op->prev_op)
    {
      if (!L_general_load_opcode (op))
        continue;
      attr = L_find_attr (op->attr, "SPECID");
      if (!attr || attr->field[0]->value.i != id)
        continue;

      if (L_EXTRACT_BIT_VAL (op->flags, L_OPER_SPECULATIVE))
        return;
#if 0
      printf ("Unused: ");
      L_print_oper (stdout, chk_op);
#endif
      op->attr = L_delete_attr (op->attr, attr);
      L_delete_oper (cb, chk_op);
      return;
    }
  L_punt ("SM_remove_if_unused: load not found\n");
}


List
SM_insert_checks (L_Func * fn, L_Cb * cb)
{
  L_Oper *op = NULL;
  L_Oper *check_lop = NULL;
  L_Attr *specid_attr = NULL;
  List chk_list = NULL;

  for (op = cb->first_op; op; op = op->next_op)
    {
      if (!L_general_load_opcode (op))
        continue;
      /* 20030220 SZU
       * SMH reconciliation
       * Different conditions were in SMH.
       */
      if (L_EXTRACT_BIT_VAL (op->flags, (L_OPER_SAFE_PEI | L_OPER_MASK_PE)))
        continue;
#if 0
      if (L_EXTRACT_BIT_VAL (op->flags, (L_OPER_SPECULATIVE |
                                         L_OPER_SAFE_PEI | L_OPER_PROMOTED)))
        continue;
#endif

      if (L_find_attr (op->attr, "SPECID"))
        L_punt ("SM_insert_checks: non-spec op already has SPECID\n");

      specid_attr = L_new_attr ("SPECID", 1);
      L_set_int_attr_field (specid_attr, 0, fn->max_spec_id++);
      op->attr = L_concat_attr (op->attr, specid_attr);

      check_lop = L_create_new_op (Lop_CHECK);
      check_lop->flags |= L_OPER_CHECK;
      check_lop->attr = L_concat_attr (check_lop->attr,
                                       L_copy_attr (specid_attr));

      check_lop->src[0] = L_copy_operand (op->dest[0]);
      if (op->pred[0])
        check_lop->pred[0] = L_copy_operand (op->pred[0]);
      /* 20030220 SZU
       * SMH reconciliation
       * This was commented out in SM, and present in SMH.
       */
      check_lop->proc_opc = S_machine_check (check_lop);

      L_insert_oper_after (cb, op, check_lop);
#if 0
      printf ("Inserted: ");
      L_print_oper (stdout, check_lop);
#endif
      chk_list = List_insert_last (chk_list, check_lop);
    }

  return chk_list;
}

List
SM_delete_checks (L_Func * fn, L_Cb * cb, List chk_list)
{
  L_Oper *check_lop = NULL;

  List_start (chk_list);
  while ((check_lop = (L_Oper *) (List_next (chk_list))))
    {
#if 0
      printf ("Examined: ");
      L_print_oper (stdout, check_lop);
#endif
      SM_remove_if_unused (cb, check_lop);
    }

  List_reset (chk_list);
  return NULL;
}

/* Add cb_flags to support modulo scheduling -ITI/JCG 8/99 */
SM_Cb *
SM_new_cb (Mdes * version1_mdes, L_Cb * lcode_cb, int cb_flags)
{
  SM_Cb *sm_cb;
  SM_Oper *sm_op, *last_op;
  L_Oper *lcode_op;
  L_Flow *flow;
  int serial_number;
  SM_Oper_Queue *exit_queue;
  SM_Oper_Qentry *qentry;
  double total_taken_weight, exit_weight;
  int has_fall_thru, num_exits, index;
  Mdes *old_version1_mdes;
  unsigned int op_count;
  int use_Lbx86_flow_hack;
  int reserved_flags;

  /* Sanity check, can only use schedule manager if using lmdes2 file */
  if (version1_mdes->mdes2 == NULL)
    {
      L_punt ("SM_new_cb: Schedule manager only supports .lmdes2 files.\n"
              "Cannot use '%s'\n", version1_mdes->file_name);
    }


  /* Sanity check, make sure they didn't specify '1' or '0' for flags 
   * (calling convension changed -ITI/JCG 8/99)
   */
  if ((cb_flags == 0) || (cb_flags == 1))
    {
      L_punt ("SM_new_cb: Expects at least 'SM_PREPASS' or "
              "'SM_POSTPASS'\n" "           not '%i'!", cb_flags);
    }

  /* Sanity check, there are several flags that should not be specified
   * by the user.
   */
  reserved_flags = SM_CROSS_ITERATION | SM_ASSUME_MVE |
    SM_MODULO_RESOURCES | SM_CB_HAS_FALL_THRU | SM_CB_NO_FALL_THRU |
    SM_CB_TEST_ONLY_ONE_ALT;

  if ((cb_flags & SM_CB_RESERVED_FLAGS) != 0)
    {
      L_punt ("SM_new_cb: Reserved/internal flags specified in cb_flags!\n"
              "           See sm.h for defintion of these flags: %x",
              cb_flags & SM_CB_RESERVED_FLAGS);
    }

  /* Save original value of lmdes before setting to mdes2->version1_mdes */
  old_version1_mdes = lmdes;
  lmdes = version1_mdes;

  /* Initialize sm_cb and sm_op pools if necessary */
  if (SM_Cb_pool == NULL)
    {
      SM_Cb_pool = L_create_alloc_pool ("SM_Cb", sizeof (SM_Cb), 16);

      /* Initialize sm_op pool if necessary */
      if (SM_Oper_pool == NULL)
        SM_Oper_pool = L_create_alloc_pool ("SM_Oper", sizeof (SM_Oper), 64);

      /* Initialize special operands (mem, ctrl, sync, and vliw operands) */
      SM_initialize_special_operands ();

      /* 20020914 SZU
       * Initialize SM_Issue_Group_pool if necessary
       */
      if (SM_Issue_Group_pool == NULL)
        SM_Issue_Group_pool = L_create_alloc_pool ("SM_Issue_Group",
						   sizeof (SM_Issue_Group), 64);
    }

  /* Alloc new sm_cb structure */
  sm_cb = (SM_Cb *) L_alloc (SM_Cb_pool);

  /* Initialize fields */
  sm_cb->sm_mdes = version1_mdes->mdes2->sm_mdes;
  sm_cb->version1_mdes = version1_mdes;
  sm_cb->lcode_cb = lcode_cb;
  sm_cb->lcode_fn = L_fn;       /* For now, until sm_func is used */
  sm_cb->sm_func = NULL;        /* For now, until sm_func is used. */
  sm_cb->cb_weight = lcode_cb->weight;
  sm_cb->flags = cb_flags;      /* -ITI/JCG 8/99 */
  sm_cb->II = 0;                /* -ITI/JCG 8/99 */
  sm_cb->stages = 0;
  sm_cb->sched_cycle_offset = 0;
  sm_cb->chk_list = NULL;

  /* 20021210 SZU
   * Modified for Itanium dispersal 
   */
  sm_cb->first_issue_group = NULL;
  sm_cb->last_issue_group = NULL;

  /* 20030220 SZU
   * SMH reconciliation
   */
  /* Insert checks if requested */
  if (L_generate_spec_checks && L_do_recovery_code && 
      (cb_flags & SM_PREPASS))
    {
      DB_spit_func(L_fn, "pre_check");
      sm_cb->chk_list = SM_insert_checks (L_fn, lcode_cb);
    }

  /* Dependences with these special ignore flags are considered actual
   * dependences by the early and late time calculations but are truly
   * ignored everywhere else (like normal).
   */
  sm_cb->special_dep_ignore_flags = 0;

  /* Only create kernel queue when modulo-scheduled kernel is committed 
   * -ITI/JCG 9/99
   */
  sm_cb->kernel_queue = NULL;

  /* Create queue for sm_ops with all their dep_in resolved. */
  sm_cb->dep_in_resolved = SM_new_oper_queue ();

  /* Set fields for determining which registers conflict */
  /* Get pointer to conflicting operands function from Mspec */
  sm_cb->conflicting_operands = M_dep_conflicting_operands ();

  /* Make sure user didn't set both PREPASS and POSTPASS -ITI/JCG 8/99 */
  if ((cb_flags & SM_PREPASS) && (cb_flags & SM_POSTPASS))
    L_punt ("SM_new_cb: SM_PREPASS and SM_POSTPASS both set!");

  /* This call needs to know if register allocation has been done
   * so that dependences can be drawn properly.
   */
  if (cb_flags & SM_PREPASS)
    sm_cb->prepass_sched = 1;
  else if (cb_flags & SM_POSTPASS)
    sm_cb->prepass_sched = 0;
  else
    L_punt ("SM_new_cb: Expect SM_PREPASS or SM_POSTPASS to be set in "
	    "cb_flags!");

  /* Make sure DHASY and MODULO not both specified by user. -ITI/JCG 8/99 */
  if ((cb_flags & SM_DHASY) && (cb_flags & SM_MODULO))
    L_punt ("SM_new_cb: SM_DHASY and SM_MODULO both set!");

  /* Set up modulo scheduler, if specified */
  if (cb_flags & SM_MODULO)
    {
      /* Currently only support prepass modulo scheduling */
      if (cb_flags & SM_POSTPASS)
	L_punt ("SM_new_cb: SM_MODULO incompatible with SM_POSTPASS!");

      /* Turn on cross-iteration dependence drawning assuming that
       * MVE will be used (removes many anti and output dependences).
       * Also use modulo resource allocation.
       */
      sm_cb->flags |=
        SM_CROSS_ITERATION | SM_ASSUME_MVE | SM_MODULO_RESOURCES;
    }

  /* Expect DHASY to be specified if not SM_MODULO */
  /* 20040712SZU
   * SM_SEQUENTIAL now another option; sequential order retained
   * Can only be one kind.
   */
#if 0
  else if (!(cb_flags & SM_DHASY))
    {
      L_punt ("SM_new_cb: Expect SM_DHASY or SM_MODULO to be set in "
              "cb_flags!");
    }
#else
  else if (!(cb_flags & SM_DHASY))
    if (!(cb_flags & SM_SEQUENTIAL))
      {
	L_punt ("SM_new_cb: Expect SM_DHASY, SM_MODULO, or SM_SEQUENTIAL "
		"to be set in cb_flags!");
      }
#endif

  /* For efficiency, flag in the cb flags if the mdes parameter
   * "check_resources_for_only_one_alt" is set to yes (only can
   * be set to one if using an lmdes2 file.
   */
  if (version1_mdes->check_resources_for_only_one_alt)
    sm_cb->flags |= SM_CB_TEST_ONLY_ONE_ALT;

  /* Use queue to build exit list */
  exit_queue = SM_new_oper_queue ();

  /* Build schedule and serial order op lists for cb */
  sm_cb->first_sched_op = NULL;
  sm_cb->last_sched_op = NULL;
  sm_cb->first_serial_op = NULL;
  sm_cb->last_serial_op = NULL;
  serial_number = 1024;

  op_count = 0;

  /* Add each oper in cb to sm_cb (except IGNORE ops) */
  for (lcode_op = lcode_cb->first_op; lcode_op != NULL;
       lcode_op = lcode_op->next_op)
    {
      /* Skip IGNORE ops, they are just compiler directives */
      if (op_flag_set (lcode_op->proc_opc, OP_FLAG_IGNORE))
        continue;

      /* Alloc sm_op for this op */
      sm_op = (SM_Oper *) L_alloc (SM_Oper_pool);

      /* Initialize fields */
      SM_init_sm_op_fields (sm_op, sm_cb, lcode_op);

      /* Add to end of serial op list */
      sm_op->prev_serial_op = sm_cb->last_serial_op;
      sm_op->next_serial_op = NULL;
      if (sm_cb->last_serial_op)
        sm_cb->last_serial_op->next_serial_op = sm_op;
      else
        sm_cb->first_serial_op = sm_op;
      sm_cb->last_serial_op = sm_op;

      /* Set serial number so that serial order can be determined by
       * comparing the serial numbers of two operations..
       * Initially place 1024 apart so that at least 10 operations can be 
       * inserted between two operations without needing to redo the serial 
       *  numbers.
       * 
       * DO NOT USE SERIAL NUMBERS AS AN ID FOR THE OPERATION!  
       * THEY MAY CHANGE AT ANY TIME AND SHOULD BE USED ONLY FOR DETERMINING
       * THE SERIAL ORDER!
       */
      sm_op->serial_number = serial_number;
      serial_number += 1024;

      /* 20030909 SZU
       * Initialize new sm_op->liverange_reduced to 0.
       */
      sm_op->liverange_reduced = 0;

      op_count++;

#if 0
      /* JWS 19991002 : Generate check instructions */
      if (L_generate_spec_checks &&
          (sm_op->mdes_flags & (OP_FLAG_LOAD)) &&
          (!L_find_attr (lcode_op->attr, "SPECID")))
        {
          L_Oper *check_lop;
          SM_Oper *check_op;
          L_Attr *specid_attr;

          specid_attr = L_new_attr ("SPECID", 1);
          L_set_int_attr_field (specid_attr, 0, SM_spec_id++);
          lcode_op->attr = L_concat_attr (lcode_op->attr, specid_attr);

          check_lop = L_create_new_op (Lop_CHECK);
          check_lop->flags |= L_OPER_CHECK;

          check_lop->attr = L_concat_attr (check_lop->attr,
                                           L_copy_attr (specid_attr));

          check_lop->src[0] = L_copy_operand (lcode_op->dest[0]);
          if (lcode_op->pred[0])
            check_lop->pred[0] = L_copy_operand (lcode_op->pred[0]);

          check_op = (SM_Oper *) L_alloc (SM_Oper_pool);

          SM_init_sm_op_fields (check_op, sm_cb, check_lop);

          check_op->prev_serial_op = sm_cb->last_serial_op;
          check_op->next_serial_op = NULL;
          sm_cb->last_serial_op->next_serial_op = check_op;
          sm_cb->last_serial_op = check_op;

          check_op->serial_number = serial_number;
          serial_number += 1024;

          op_count++;
        }
#endif

      /* If this operation is a non-jsr branch, place it in the
       * exit queue.
       */
      if (sm_op->mdes_flags & (OP_FLAG_CBR | OP_FLAG_JMP | OP_FLAG_RTS))
	SM_enqueue_oper_before (exit_queue, sm_op, NULL);
    }

  /* Set op counts in sm_cb */
  sm_cb->op_count = op_count;
  sm_cb->num_unsched = op_count;
  sm_cb->num_ignored = 0;

  /* The cb has a fall-thru path if
   * 1) the cb is empty
   * 2) it doesn't end in an unconditional branch
   * 3) it ends in a predicated unconditional branch and the
   *    hyperblock is not marked as having no fall-thru path.
   * Otherwise, it does not have a fall-thru path.
   */
  has_fall_thru = !(last_op = sm_cb->last_serial_op) ||
    !(last_op->mdes_flags & (OP_FLAG_JMP | OP_FLAG_RTS)) ||
    (L_is_predicated (last_op->lcode_op) &&
     !(lcode_cb->flags & L_CB_HYPERBLOCK_NO_FALLTHRU));

  /* Based on if the cb has a fall-thru path,
   * set the number of exits and fall-thru flags.
   */
  if (has_fall_thru)
    {
      sm_cb->flags |= SM_CB_HAS_FALL_THRU;
      num_exits = exit_queue->num_qentries + 1;
    }
  else
    {
      sm_cb->flags |= SM_CB_NO_FALL_THRU;
      num_exits = exit_queue->num_qentries;
    }

  /* Sanity check, the cross-iteration dependence routines were
   * written assuming that the cb is a loop with one backedge
   * and that backedge is at the very end of the cb.  Incorrect
   * dependences may be drawn if these assumptions are not met,
   * so punt. -ITI/JCG 8/99
   *
   * The cross-iteration memory dependences currently require
   * correct sync arcs.  Sync arcs are currently not correctly
   * generated for parameters passed thru parameters (to JSRs).
   * Punt if this case is detected -ITI/JCG 8/99
   */
  if (sm_cb->flags & SM_CROSS_ITERATION)
    {
      /* Scan for JSRs with parameters thru memory */
      for (qentry = exit_queue->first_qentry; qentry != NULL;
           qentry = qentry->next_qentry)
        {
          /* Get branch sm_op and lcode_op for ease of use */
          sm_op = qentry->sm_op;
          lcode_op = sm_op->lcode_op;

          /* Test JSRs to see if pass parameter thru memory 
           * or returns a structure */
          if (sm_op->mdes_flags & OP_FLAG_JSR)
            {
              /* Punt if pass parms thru memory (tm attr)
               * or returns a structure thru memory (ret_st attr)
               * since sync-arcs not currently drawn for these
               * operations.  The sync-arc drawing code needs
               * to be enhanced to handle these cases if modulo
               * scheduling of general JSRs are to be handled
               * (seemed more trouble than it is worth at this point).
               * -ITI/JCG 8/99
               */

              if (L_find_attr (lcode_op->attr, "tm"))
		L_punt ("SM_new_cb (cb %i, op %i): cross-iteration "
			"dependence drawer cannot JSR with parms "
			"thru memory!", lcode_cb->id, lcode_op->id);
              else if (L_find_attr (lcode_op->attr, "ret_st"))
		L_punt ("SM_new_cb (cb %i, op %i): cross-iteration "
			"dependence drawer cannot JSR that returns "
			"structure!", lcode_cb->id, lcode_op->id);
            }
        }

      /* Find the qentry holding the backedge sm_op */
      for (qentry = exit_queue->first_qentry; qentry;
           qentry = qentry->next_qentry)
        {
          sm_op = qentry->sm_op;
          lcode_op = sm_op->lcode_op;

          if ((L_cond_branch (lcode_op) || L_uncond_branch (lcode_op)) &&
	      (L_branch_dest (lcode_op)->id == lcode_cb->id))
	    break;
        }

      /* A backedge branch is required, and it must be the final op
       * of the CB. */

      if (!qentry)
	L_punt ("SM_new_cb (cb %i): cross-iteration dependence drawer "
		"expects backedge!", lcode_cb->id);

      sm_op = qentry->sm_op;
      if (sm_op->next_serial_op)
	L_punt ("SM_new_cb (cb %i, op %i):\n"
		"  cross-iteration dependence drawer expects single "
		"backedge at end of cb!", lcode_cb->id,
		sm_op->lcode_op->id);
    }

  /* 
   * Initialize cb exit info fields 
   */
  /* Set the number of exits (size of arrays below) */
  sm_cb->num_exits = num_exits;

  /* Malloc arrays of size num_exits */
  sm_cb->exit_op = (SM_Oper **) malloc (sizeof (SM_Oper *) * num_exits);
  if (sm_cb->exit_op == NULL)
    L_punt ("SM_new_cb: Out of memory");

  sm_cb->exit_weight = (double *) malloc (sizeof (double) * num_exits);
  if (sm_cb->exit_weight == NULL)
    L_punt ("SM_new_cb: Out of memory");

  sm_cb->exit_percentage = (double *) malloc (sizeof (double) * num_exits);
  if (sm_cb->exit_percentage == NULL)
    L_punt ("SM_new_cb: Out of memory");


  /* For every branch, initialize the array elements */
  index = 0;
  total_taken_weight = 0.0;
  flow = lcode_cb->dest_flow;

  /* Lbx86 currently cannot draw all the dest flows properly, due
   * to strange register jumps.  For now, hack it so that this
   * routine can handle a NULL dest_flow for Lbx86 (for now). -JCG 3/9/98
   */
  if ((flow == NULL) && SM_make_Lbx86_assumptions)
    use_Lbx86_flow_hack = 1;
  else
    use_Lbx86_flow_hack = 0;

  for (qentry = exit_queue->first_qentry; qentry != NULL;
       qentry = qentry->next_qentry)
    {
      sm_cb->exit_op[index] = qentry->sm_op;

      /* returns don't have flows, infer exit weight */
      if (qentry->sm_op->mdes_flags & OP_FLAG_RTS)
	{
	  exit_weight = sm_cb->cb_weight - total_taken_weight;
	}
      else
        {
	  /* Otherwise get weight from flow and goto next flow */
          /* Unless making a hack for Lbx86, use the dest flows
           * to calculate exit weights.
           */

          if (!use_Lbx86_flow_hack && flow!=NULL)
            {

        	  exit_weight = flow->weight;
        	  flow = flow->next_flow;

            }

          /* Otherwise, for now assume all the weight goes to the 
           * last branch. 
           */
          else
            {


              if (qentry->next_qentry == NULL)
                exit_weight = sm_cb->cb_weight;
              else

                exit_weight = 0.0;
            }
        }

      sm_cb->exit_weight[index] = exit_weight;
      total_taken_weight += exit_weight;

      /* For zero-weight cbs, give all the but the last exit 
       * a zero exit percentage.
       */
      if (sm_cb->cb_weight > .0001)
        sm_cb->exit_percentage[index] = exit_weight / sm_cb->cb_weight;
      else
        sm_cb->exit_percentage[index] = 0.0;

      /* Goto next array element */
      index++;
    }

  /* Give the last exit all the remaining weight. 
   * Designed to handle both hashing jumps (multiple flows for the
   * last branch) and the fall-thru case.
   */
  index = num_exits - 1;

  /* Set last exit to NULL if have fall-thru, and give it
   * the remaining weight
   */
  if (has_fall_thru)
    {
      sm_cb->exit_op[index] = NULL;
      sm_cb->exit_weight[index] = sm_cb->cb_weight - total_taken_weight;
    }

  /* Otherwise, add the remaining weight to that already read
   * from the first flow.
   */
  else
    {
      sm_cb->exit_weight[index] += sm_cb->cb_weight - total_taken_weight;
    }

  /* Calculate the correct exit percentage.  For zero weight blocks,
   * give last exit all the exit percentage.
   */
  if (sm_cb->cb_weight > .0001)
    {
      sm_cb->exit_percentage[index] =
        sm_cb->exit_weight[index] / sm_cb->cb_weight;
    }
  else
    {
      sm_cb->exit_percentage[index] = 1.0;
    }

  /* Debug, mess with profile weights */
  if (!L_pmatch (prof_info, "real"))
    {
      /* Give each cb 100 weight */
      sm_cb->cb_weight = 100.0;

      /* For 'all', make exit branch equally likely */
      if (L_pmatch (prof_info, "all"))
        {
          for (index = 0; index < num_exits; index++)
            {
              sm_cb->exit_weight[index] = 100.0 / (double) num_exits;
              sm_cb->exit_percentage[index] = 100.0 / (double) num_exits;
            }
        }

      /* For 'last', make only last branch taken */
      else if (L_pmatch (prof_info, "last"))
        {
          for (index = 0; index < (num_exits - 1); index++)
            {
              sm_cb->exit_weight[index] = 0.0;
              sm_cb->exit_percentage[index] = 0.0;
            }
          sm_cb->exit_weight[num_exits - 1] = 100.0;
          sm_cb->exit_percentage[num_exits - 1] = 100.0;
        }
      /* For 'first', make only first branch taken */
      else if (L_pmatch (prof_info, "first"))
        {
          for (index = 1; index < num_exits; index++)
            {
              sm_cb->exit_weight[index] = 0.0;
              sm_cb->exit_percentage[index] = 0.0;
            }
          sm_cb->exit_weight[0] = 100.0;
          sm_cb->exit_percentage[0] = 100.0;
        }
      else
        {
          L_punt ("Unknown value for prof_info parm '%s'", prof_info);
        }

    }

  /* Free the exit queue */
  SM_delete_oper_queue (exit_queue);

  /* Initialize resource map fields */
  sm_cb->map_array = NULL;      /* Algorithm assumes set to NULL initially */

  /* Initialize register info table for cb */
  SM_build_reg_info_table (sm_cb);

#if 0
  /* Debug */
  SM_print_sorted_reg_info_table (stdout, sm_cb);
#endif

  /* Build the dependence graph for the cb */
  SM_build_cb_dependences (sm_cb);

  /* 20040712SZU
   * If SM_SEQUENTIAL, add SYNC_DEP between ops to retain sequential order.
   */
  if (cb_flags & SM_SEQUENTIAL)
    {
      SM_Oper *next_serial_op;

      for (sm_op = sm_cb->first_serial_op; sm_op != NULL;
	   sm_op = next_serial_op)
	{
	  next_serial_op = sm_op->next_serial_op;

	  if (next_serial_op)
	    SM_insert_sync_dep_betw_ops (sm_op, next_serial_op);
	}
    }

  /* Debug, now a parameter -JCG 6/99 */
  if (SM_print_dependence_graph)
    {
      SM_print_cb_dependences (stdout, sm_cb);
    }

  /* The rest of the fields don't need to be intialized here */

#if 0
  /* Debug */
  printf ("Cb %i, weight %f:\n", lcode_cb->id, sm_cb->cb_weight);
  for (index = 0; index < num_exits; index++)
    {
      printf ("%8.2f %6.2f", sm_cb->exit_weight[index],
              sm_cb->exit_percentage[index]);

      if (sm_cb->exit_op[index] == NULL)
        printf ("   Fall-thru\n");
      else
        SM_print_oper (stdout, sm_cb->exit_op[index]);
    }
  printf ("\n");
#endif

  /* Restore original value of lmdes */
  lmdes = old_version1_mdes;

  /* Return new created sm_cb structure */
  return (sm_cb);
}

/* Sets II for a cb -ITI/JCG 8/99 */
void
SM_set_cb_II (SM_Cb * sm_cb, int II)
{
  /* Sanity check, II must be >= 1 */
  if (II < 1)
    {
      L_punt ("SM_set_cb_II: II (%i) < 1!", II);
    }

  /* Sanity check, SM_MODULO_RESOURCES flag must be set */
  if ((sm_cb->flags & SM_MODULO_RESOURCES) == 0)
    {
      L_punt ("SM_set_cb_II: Modulo resources not specified for cb %i!",
              sm_cb->lcode_cb->id);
    }
  sm_cb->II = II;
}

void
SM_delete_cb (SM_Cb * sm_cb)
{
  SM_Oper *sm_op, *next_serial_op;
  SM_Compatible_Alt *compatible_alt, *next_compatible_alt;
  SM_Issue_Group *issue_group_ptr, *next_issue_group;

  if (L_generate_spec_checks && sm_cb->prepass_sched)
    {
      sm_cb->chk_list = SM_delete_checks (L_fn,
                                          sm_cb->lcode_cb, sm_cb->chk_list);
    }

  /* Delete dep_in_resolved queue before delete operations */
  SM_delete_oper_queue (sm_cb->dep_in_resolved);

  /* Delete kernel queue (if present) before deleting operations 
   * -ITI/JCG 9/99 
   */
  if (sm_cb->kernel_queue != NULL)
    {
      SM_delete_oper_queue (sm_cb->kernel_queue);
    }

  /* Delete the sm_ops in the linked list (use serial op list) */
  for (sm_op = sm_cb->first_serial_op; sm_op != NULL; sm_op = next_serial_op)
    {
      /* Get the next sm_op before deleting this one */
      next_serial_op = sm_op->next_serial_op;

      /* Free the compatable alt nodes */
      for (compatible_alt = sm_op->first_compatible_alt;
           compatible_alt != NULL; compatible_alt = next_compatible_alt)
        {
          /* Get next compatible alt before deleting this one */
          next_compatible_alt = compatible_alt->next_compatible_alt;

          L_free (SM_Compatible_Alt_pool, compatible_alt);
        }


      /* Free the operand array */
      free (sm_op->operand);

      /* Free implicit queues (if they exist) */
      if (sm_op->implicit_dests != NULL)
        SM_delete_action_queue (sm_op->implicit_dests);
      if (sm_op->implicit_srcs != NULL)
        SM_delete_action_queue (sm_op->implicit_srcs);

      /* Free the late time array (if exists) */
      if (sm_op->late_time)
        free (sm_op->late_time);

      /* Free the schedule choices array */
      free (sm_op->options_chosen);

      L_free (SM_Oper_pool, sm_op);
    }

  /* Free the map array if necessary */
  if (sm_cb->map_array != NULL)
    free (sm_cb->map_array);

  /* Free the data in the reg info table and the dependence graph */
  SM_free_reg_info_table (sm_cb);

  /* Free exit_op, exit_weight, and exit_percentage  arrays */
  free (sm_cb->exit_op);
  free (sm_cb->exit_weight);
  free (sm_cb->exit_percentage);

  for (issue_group_ptr = sm_cb->first_issue_group; issue_group_ptr != NULL;
       issue_group_ptr = next_issue_group)
    {
      next_issue_group = issue_group_ptr->next_issue_group;

      free (issue_group_ptr->slots);
      L_free (SM_Issue_Group_pool, issue_group_ptr);
    }

  /* Free the sm_cb structure */
  L_free (SM_Cb_pool, sm_cb);
}

/* Analyzes the actions' predicates (if any), and returns 1 if
 * the action are on mutually exclusive predicate control paths,
 * otherwise 0.
 *
 * Used to determine of a dependence should be drawn.
 *
 * Unpredicated code always returns 0 (not mutually exclusive).
 *
 * JWS 20000107 - WARNING! Do not compare pred operands for "sameness"
 * using L_same_operand().  This *does not work*.  *Always* reference 
 * the predicate graph.
 */
int
SM_mutually_exclusive_opers (SM_Oper * sm_op1, SM_Oper * sm_op2)
{
  L_Oper *lcode_op1, *lcode_op2;

  /* Return 0 if predicate operands are not supported by this architecture */
  if (L_max_pred_operand <= 0)
    return (0);

  lcode_op1 = sm_op1->lcode_op;
  lcode_op2 = sm_op2->lcode_op;

  /* Query predicate graph to see if there is a control path between
   * these operations.  If there is, they are not mutually exclusive,
   * so return 0.
   */
  if (PG_intersecting_predicates_ops (lcode_op1, lcode_op2))
    {
      return (0);
    }

  /* Otherwise, they are mutually exclusive, return 1 */
  else
    {
      return (1);
    }
}

L_Attr *
SM_attach_isl (SM_Oper * sm_op, int sched_cycle_offset)
{
  L_Oper *lcode_op = sm_op->lcode_op;
  L_Attr *isl_attr;
  int max_dest_index;
  int index;
  /* 20031007 SZU */
  int l_set = 0;

  /* Delete any existing isl attribute */
  if ((isl_attr = L_find_attr (lcode_op->attr, "isl")) != NULL)
    lcode_op->attr = L_delete_attr (lcode_op->attr, isl_attr);

  /* Determine the max dest index which has a register */
  for (max_dest_index = L_max_dest_operand - 1; max_dest_index >= 0;
       max_dest_index--)
    {
      if (sm_op->dest[max_dest_index])
        break;
    }

  /* Create a new isl attribute with enough room for the scheduled cycle,
   * the scheduled slot, and for every of the destinations use time.
   */
  isl_attr = L_new_attr ("isl", 2 + max_dest_index + 1);

  /* Set the scheduled cycle and slot.  Use sched_cycle_offset to
   * allow the issue time to be "normalized" (see above).
   */
  L_set_int_attr_field (isl_attr, 0, sm_op->sched_cycle + sched_cycle_offset);
  L_set_int_attr_field (isl_attr, 1, sm_op->sched_slot);

  /* Set the destination use times */
  for (index = 0; index <= max_dest_index; index++)
    {
      /* 20031007 SZU
       * Make sure a l value is set to keep Lstatic happy
       */
#if 0
      if (sm_op->dest[index])
	L_set_int_attr_field (isl_attr, 2 + index,
			      sm_op->dest[index]->actual_late_use_time);
#else
      if (sm_op->dest[index])
	{
	  L_set_int_attr_field (isl_attr, 2 + index,
				sm_op->dest[index]->actual_late_use_time);
	  l_set = 1;
	}
#endif
    }

  /* 20031007 SZU
   * Resolve pred_ld_blk having no l attr in isl problem w/ Limpact and Lstatic.
   * Give all isl zero l if no destination under SM.
   * This is to keep Lstatic happy.
   */
  if (!l_set)
    L_set_int_attr_field (isl_attr, 2, 0);

  /* Insert the attribute into the lcode op's attribute list */
  lcode_op->attr = L_concat_attr (lcode_op->attr, isl_attr);

  return isl_attr;
}

int
SM_set_sched_cycle_offset (SM_Cb *sm_cb)
{
  int ofst, lcycle, lpos, length;

  if (!sm_cb)
    L_punt ("SM_set_sched_cycle_offset: NULL sm_cb");

  /* Get the minimum schedule cycle.  It is guarenteed to be the sched
   * cycle of the first scheduled operation.  Set this offset to
   * -min_sched_cycle so that the first "issue time" will be adjusted
   * to 0 (which IMPACT tools expect).  
   */

  ofst = - sm_cb->first_sched_op->sched_cycle;

  if (sm_cb->flags & SM_MODULO_RESOURCES)
    {
      /* Ensure that the loopback branch is in the last cycle */

      if (!sm_cb->II || 
	  !sm_cb->first_sched_op || !sm_cb->last_sched_op)
	{
	  sm_cb->sched_cycle_offset = 0;
	  return -1;
	}

      lcycle = sm_cb->last_sched_op->sched_cycle;

      lpos = (lcycle + ofst) % sm_cb->II;

      if (lpos < (sm_cb->II - 1))
	ofst += sm_cb->II - 1 - lpos;

      assert (((lcycle + ofst) % sm_cb->II) == sm_cb->II - 1);

      length = sm_cb->last_sched_op->sched_cycle -
	sm_cb->first_sched_op->sched_cycle + 1;

      sm_cb->stages = (length + sm_cb->II - 1) / sm_cb->II;
    }

  sm_cb->sched_cycle_offset = ofst;

  return 0;
}

/* Commit the schedule, reordering the lcode operations, creating
 * isl attributes, and marking speculative operations.
 */
void
SM_commit_cb_template_bundling (SM_Cb * sm_cb)
{
  L_Cb *lcode_cb;
  L_Oper *lcode_op = NULL;
  SM_Oper *sm_op, *from_op, *br_op;
  L_Attr *isl_attr, *stage_attr;
  SM_Dep *dep_in;
  unsigned int serial_number, cb_flags;
  int sched_cycle, II, kernel_cycle, kernel_stage, sched_cycle_offset;
  unsigned short sched_slot;
  SM_Oper_Queue *br_queue;
  SM_Oper_Qentry *br_qentry;
  int prepass_sched, index, bundle_type = 0, stop = 0;
  SM_Issue_Group *issue_group_ptr;
  L_Oper *op = NULL;
  int max_dest_index, syll_type = 0, num_slots, current_bundle, insert = 0;
  int slots_per_template, shift_var, template_index;
  unsigned int template_mask;
  SM_Bundle *bundle;

  /* Only process cb's with operation in them. */
  if (sm_cb->first_serial_op == NULL)
    return;

  cb_flags = sm_cb->flags;
  II = sm_cb->II;

  /* Get prepass scheduling flag for ease of use -JCG 9/99 */
  /* 20021214 SZU 
   * Continuing MSM HACK. Treat modulo scheduled loops as post-pass
   * */
  if (cb_flags & SM_MODULO_RESOURCES)
    prepass_sched = 0;
  else
    prepass_sched = sm_cb->prepass_sched;

  /*
   * For now, mark operations speculative here 
   * 
   * Enhanced to handle predicated code more accurately (mark less
   * operations as speculative) by keeping track of all the branches
   * that this operation could have crossed (in br_queue) and testing their
   * predicate relationships -JCG 9/99
   */
  br_queue = SM_new_oper_queue ();

  for (sm_op = sm_cb->last_sched_op; sm_op != NULL;
       sm_op = sm_op->prev_sched_op)
    {
      /* Get this op's serial number and scheduled cycle for ease of use */
      serial_number = sm_op->serial_number;
      sched_cycle = sm_op->sched_cycle;

      /* Initially clear sm_op's speculative flag */
      sm_op->flags &= ~SM_OP_SPECULATIVE;

      /* This op is speculative if it moved above any branches.
       * Determine this by checking every branch that was scheduled
       * after it (if branch not predicate independent).
       * This only determines if made speculative by this pass
       * of scheduling.  Previous passes of scheduling may have
       * marked the lcode op but it will not show in the sm_op flags.
       * 
       * Also, if doing modulo scheduling, the operation is speculative
       * if scheduled an II or more before the backedge. -ITI/JCG 9/99
       */
      for (br_qentry = br_queue->first_qentry; br_qentry != NULL;
           br_qentry = br_qentry->next_qentry)
        {
          /* Get the branch we are considering for ease of use */
          br_op = br_qentry->sm_op;

          /* Mark as speculative if from below this branch and
           * not from a mutually exclusive execution path.
           */
          if ((serial_number > br_op->serial_number) &&
              !SM_mutually_exclusive_opers (sm_op, br_op))
            {

              sm_op->flags |= SM_OP_SPECULATIVE;
              L_mark_oper_speculative (sm_op->lcode_op);

              /* Sanity check, stores should never be marked speculative! */
              if (sm_op->mdes_flags & OP_FLAG_STORE)
                {
                  L_punt ("SM_commit_cb in %s cb %i op %i: "
                          "Speculative store!",
                          sm_cb->lcode_fn->name, sm_cb->lcode_cb->id,
                          sm_op->lcode_op->id);
                }

              /* No need to check other branches */
              break;
            }

          /* If doing modulo scheduling, mark as speculative if
           * scheduled more than II cycles before any branch (even
           * if didn't move past it) since the next kernel iteration 
           * will start execution before it is known that the iteration
           * should execute. -ITI/JCG 9/99
           */
          else if ((cb_flags & SM_MODULO_RESOURCES) &&
                   (sched_cycle < (br_op->sched_cycle - (II - 1))))
            {
              sm_op->flags |= SM_OP_SPECULATIVE;
              L_mark_oper_speculative (sm_op->lcode_op);

              /* Sanity check, stores should never be marked speculative! */
              if (sm_op->mdes_flags & OP_FLAG_STORE)
                {
                  L_punt ("SM_commit_cb in %s cb %i op %i: "
                          "Speculative store (modulo)!",
                          sm_cb->lcode_fn->name, sm_cb->lcode_cb->id,
                          sm_op->lcode_op->id);
                }

              /* No need to check other branches */
              break;
            }
        }

      /* JWS 19991002 : Generate check instructions */
      if (L_generate_spec_checks &&
          (sm_op->mdes_flags & (OP_FLAG_LOAD)) &&
          (!(sm_op->flags & (SM_OP_SPECULATIVE))) &&
          (!(sm_op->lcode_op->flags & (L_OPER_SPECULATIVE))))
        {
          /* Remove check if op is not speculative */
          SM_remove_matching_check (sm_cb, sm_op);
        }

      /* If this operation has some ignored soft dependences, 
       * determine if we need an silent version of this operation. 
       * (If any of the ignored ctrl dep_in have been violated,
       *  then we need a silent version).
       */
      if (sm_op->num_ignore_dep_in > 0)
        {
          sched_slot = sm_op->sched_slot;

          for (dep_in = sm_op->ext_src[SM_CTRL_ACTION_INDEX]->first_dep_in;
               dep_in != NULL; dep_in = dep_in->next_dep_in)
            {
              /* Only consider ignored dependences that are ignored
               * only because the operation can be speculated. -JCG 9/99
               */
              if ((dep_in->ignore & SM_CAN_SPEC_IGNORE) != SM_CAN_SPEC_IGNORE)
                continue;

              from_op = dep_in->from_action->sm_op;

              /* Handle cross-iteration soft-dependences appropriately */
              if (dep_in->omega != 0)
                {
                  /* Sanity check, omega must be 1 for cross-iteration
                   * soft dependences (assumed below). -ITI/JCG 9/99
                   */
                  if (dep_in->omega != 1)
                    {
                      L_punt ("SM_commit_cb in %s cb %i:\n"
                              "SM_CAN_SPEC_IGNORE dep with omega %i!",
                              sm_cb->lcode_fn->name, sm_cb->lcode_cb->id,
                              dep_in->omega);
                    }

                  /* For cross-iteration soft-dependences, if the
                   * operation is scheduled II or more cycles before the
                   * branch, it is speculative. -ITI/JCG 9/99.
                   */
                  if (sched_cycle <= (from_op->sched_cycle - II))
                    {
                      /* Stop, dependence found that requires 
                       *  silent version 
                       */
                      break;
                    }
                }

              /* Handle intra-iteration soft-dependences appropriately.

               * If the operation we are dependent on is scheduled 
               * after this operation, then we need a non-trapping
               * version of this operation.
               */
              else if ((sched_cycle < from_op->sched_cycle) ||
                       ((sched_cycle == from_op->sched_cycle) &&
                        (sched_slot < from_op->sched_slot)))
                {
                  /* Stop, dependence found that requires silent version */
                  break;
                }
            }

          /* If we have a dependence that requires a silent version,
           * mark as silent, and make sure marked speculative!
           */
          if (dep_in != NULL)
            {
              sm_op->flags |= SM_OP_SILENT;

              /* No longer print warning -JCG 6/99 */
#if 0
              /* Sanity check, better be speculative 
               * Warn for now, since I am rescheduling scheduled code and
               * this test doens't work in that case.
               */
              if (!(sm_op->flags & SM_OP_SPECULATIVE))
                {
                  SM_print_oper (stderr, sm_op);
                  fprintf (stderr,
                           "Warning: SILENT but NOT SPECULATIVE op!\n\n");
                }
#endif
            }
          else
            {
              sm_op->flags &= ~SM_OP_SILENT;

              /* No longer print warning -JCG 6/99 */
#if 0
              /* Sanity check, better not be speculative, well
               * can happen in predicated code.  Just warn for now!
               */
              if (sm_op->flags & SM_OP_SPECULATIVE)
                {
                  SM_print_oper (stderr, sm_op);
                  fprintf (stderr,
                           "Warning: NOT SILENT but SPECULATIVE op!\n\n");
                }
#endif
            }

        }

      /* If this op is a branch (non-JSR), add to beginning of branch queue 
       * so that earlier operations can determine if they have
       * been speculated across it. -JCG 9/99
       */
      if (sm_op->mdes_flags & (OP_FLAG_CBR | OP_FLAG_JMP | OP_FLAG_RTS))
        {
          SM_enqueue_oper_after (br_queue, sm_op, NULL);
        }
    }

  /* Free branch queue used for speculation */
  SM_delete_oper_queue (br_queue);
  br_queue = NULL;

  /* Get the lcode cb we are committing the sm_cb's schedule to */
  lcode_cb = sm_cb->lcode_cb;

  /* Remove every op in the sm_cb from the lcode cb starting from
   * the end of the cb.
   * 
   * For now, check to make sure all ops have been scheduled!
   */
  for (sm_op = sm_cb->last_serial_op; sm_op != NULL;
       sm_op = sm_op->prev_serial_op)
    {
      lcode_op = sm_op->lcode_op;

      /* Expect this operation to be marked as scheduled */
      if (!(sm_op->flags & SM_OP_SCHEDULED))
	L_punt ("SM_commit_cb in %s cb %i op %i: Unscheduled operation!",
		sm_cb->lcode_fn->name, lcode_cb->id, lcode_op->id);

      /* Remove lcode op from lcode cb */
      L_remove_oper (lcode_cb, lcode_op);
    }



  /* If SM_NORMALIZE_ISSUE cb flag is set, calculate the sched cycle offset
   * to use when writing out the "issue" part of the isl attributes.  
   */
  if (cb_flags & SM_NORMALIZE_ISSUE)
    {
      SM_set_sched_cycle_offset (sm_cb);
      sched_cycle_offset = sm_cb->sched_cycle_offset;
    }
  else
    {
      /* Otherwise, use sched cyclce offset of 0 when the actual schedule cycle
       * should be used in the isl attribute.
       */
      sched_cycle_offset = 0;
    }

  /* 20021214 SZU
   * Add code to take care of issue group for Itanium
   * 20020606 SZU
   * Alter to mdes format.
   */
  num_slots = SM_get_num_slots (sm_cb);
  slots_per_template = SM_get_slots_per_template (sm_cb);

  if (SM_do_bundle_compaction)
    SM_compact_w_internal_sbits (sm_cb);

  for (issue_group_ptr = sm_cb->first_issue_group; issue_group_ptr != NULL;
       issue_group_ptr = issue_group_ptr->next_issue_group)
    {
      /* Check which bundles are empty and assign stop bit to last bundle.
       * Only do this if not compacting w/ internal stop bits already
       */
      if (!SM_do_bundle_compaction)
	SM_assign_stop_bit (issue_group_ptr);

      for (index = 0; index < num_slots;)
	{
	  current_bundle = index / slots_per_template;

	  /* If current bundle is empty, go to next one */
	  /* 20030724 SZU
	   * Hak. 
	   * Wierd situations occuring where schedule first takes 2 bundles,
	   * but then find legal 1 bundle schedule after compaction.
	   * Insert NOP for now, maybe explore different scheduling priorities.
	   * Different scheduling priorities done in mdes.
	   */
#if 0
	  if (issue_group_ptr->bundles[current_bundle]->empty)
	    {
	      index += slots_per_template;
	      continue;
	    }
#else
	  if ((issue_group_ptr->bundles[current_bundle]->empty) &&
	      (issue_group_ptr->bundles[current_bundle]->
	       internal_stop_bit != 1))
	    {
	      index += slots_per_template;
	      continue;
	    }
#endif

	  /* If not prepass and first slot of a template */
	  /* 20030718 SZU
	   * Change condition to look for internal stop bit setting.
	   */
	  if ((!prepass_sched) &&
	      ((index % slots_per_template) == 0) &&
	      (issue_group_ptr->bundles[current_bundle]->internal_stop_bit != 2)
	      )
	    {
	      /* 20030609 SZU
	       * This is hardcoded for Itanium right now.
	       * Need to re-examine interface for creating templates.
	       */
	      switch (issue_group_ptr->bundles[current_bundle]->stop)
		{
		  case -1:
		    stop = NO_S_BIT;
		    break;
		  case 0:
		    stop = S_AFTER_1ST;
		    break;
		  case 1:
		    stop = S_AFTER_2ND;
		    break;
		  case 2:
		    stop = S_AFTER_3RD;
		    break;
		}

	      /* 20030718 SZU
	       * If the first part of a bundle with split issue,
	       * check next part to see if another stop needed
	       */
	      if (issue_group_ptr->bundles[current_bundle]->
		  internal_stop_bit == 1)
		{
		  SM_Issue_Group *nxt_issue_group;

		  /* First double check stop in this part */
		  if ((issue_group_ptr->bundles[current_bundle]->stop != 0) &&
		      (issue_group_ptr->bundles[current_bundle]->stop != 1))
		    L_punt ("SM_commit_cb: cb %i issue group %i bundle %i has "
			    "internal_stop_bit==1, but improper stop: %i\n",
			    sm_cb->lcode_cb->id, issue_group_ptr->issue_time,
			    current_bundle,
			    issue_group_ptr->bundles[current_bundle]->stop);

		  /* Get the next issue group */
		  /* 20030724 SZU
		   * Get the next non-empty issue_group.
		   * One must exist, otherwise not compacted.
		   */
		  nxt_issue_group = issue_group_ptr->next_issue_group;

		  while ((nxt_issue_group) &&
			 (nxt_issue_group->num_slots_left == 6))
		    nxt_issue_group = nxt_issue_group->next_issue_group;

		  /* Just in case... */
		  if (!nxt_issue_group)
		    L_punt ("SM_commit_cb: Compaction occurred. "
			    "1st part is time %i but no 2nd part!\n"
			    "Func %s CB %i\n",
			    issue_group_ptr->issue_time, sm_cb->lcode_fn->name,
			    sm_cb->lcode_cb->id);

		  /* Check the stop bit in the first bundle */
		  if ((nxt_issue_group->bundles[0]->stop != 2) &&
		      (nxt_issue_group->bundles[1]->empty))
		    L_punt ("SM_commit_cb: second part of split issue bundle "
			    "contains improper stop bit\n"
			    "function: %s cb: %i nxt_issue_group time %i\n",
			    sm_cb->lcode_fn->name, sm_cb->lcode_cb->id,
			    nxt_issue_group->issue_time);
		  /* If ok OR in stop bit */
		  else
		    switch (nxt_issue_group->bundles[0]->stop)
		      {
			case -1:
			  L_punt ("SM_commit_cb: Interal stop bit. "
				  "2nd part has stop == -1. Should be either "
				  "0, 1, or 2.\n");
			  break;
			case 2:
			  stop |= S_AFTER_3RD;
			  break;
		      }
		}

	      /* 20030609 SZU
	       * bundle_type also hard coded for Itanium.
	       * Need to re-examine interface for creating templates.
	       */
	      /* Convert mdes template_index to present hard code bundle_type */
	      bundle = issue_group_ptr->bundles[current_bundle];
	      switch (bundle->template_index)
		{
		  /* 20030819 SZU
		   * Switched order of templates in mdes.
		   * Correct accordingly.
		   */
		  case 0:
		    bundle_type = MLI;
		    break;
		  case 1:
		    /* MSMI, not MMI */
		    if (bundle->stop == 0)
		      bundle_type = MSMI;
		    else
		      bundle_type = MMI;
		    break;
		  case 2:
		    bundle_type = MFI;
		    break;
		  case 3:
		    bundle_type = MMF;
		    break;
		  case 4:
		    /* MISI, not MII */
		    if (bundle->stop == 1)
		      bundle_type = MISI;
		    else
		      bundle_type = MII;
		    break;
		  case 5:
		  case 10:
		    bundle_type = MIB;
		    break;
		  case 6:
		  case 11:
		  case 13:
		  case 14:
		  case 15:
		    bundle_type = MBB;
		    break;
		  case 7:
		  case 16:
		  case 17:
		  case 18:
		  case 19:
		  case 20:
		  case 21:
		  case 22:
		    bundle_type = BBB;
		    break;
		  case 8:
		    bundle_type = MMB;
		    break;
		  case 9:
		  case 12:
		    bundle_type = MFB;
		    break;
		}

	      op = SM_create_template_op (bundle_type, stop);
	      L_insert_oper_after (lcode_cb, lcode_cb->last_op, op);
	    }

	  sm_op = issue_group_ptr->slots[index];
	  /* If NULL, create and insert NOP */
	  /* 20030721 SZU
	   * Could be null but shouldn't insert NOP.
	   * This is due to compaction!
	   * Therefore check for compaction before inserting NOP.
	   * If NOP exists, then definite insert.
	   */
	  if (!sm_op)
	    {
	      /* 20030721 SZU
	       * Checking for null slots modified due to compaction.
	       * IPF specific.
	       */
	      SM_Issue_Group *prev_issue;
	      SM_Bundle *bundle_ptr, *prev_issue_bundle_ptr;

	      insert = 0;
	      bundle_ptr = issue_group_ptr->bundles[current_bundle];

	      if (!bundle_ptr->internal_stop_bit)
		{
		  insert = 1;
		}
	      else if ((bundle_ptr->internal_stop_bit == 1) &&
		       ((index % slots_per_template)<= bundle_ptr->stop))
		{
		  insert = 1;
		}
	      else if (bundle_ptr->internal_stop_bit == 2)
		{
		  /* 20030724 SZU
		   * Need to find the previous non-empty issue
		   */
		  prev_issue = issue_group_ptr->prev_issue_group;

		  while ((prev_issue) &&
			 (prev_issue->num_slots_left == 6))
		    prev_issue = prev_issue->prev_issue_group;

		  /* Just in case */
		  if (!prev_issue)
		    L_punt ("SM_commit_cb: Compaction occurred. "
			    "2nd part is time %i but no 1st part!\n"
			    "Func %s CB %i\n",
			    issue_group_ptr->issue_time, sm_cb->lcode_fn->name,
			    sm_cb->lcode_cb->id);

		  /* First check the last bundle */
		  prev_issue_bundle_ptr = prev_issue->bundles[1];

		  /* 1st part is last bundle of prev issue */
		  if (prev_issue_bundle_ptr->internal_stop_bit == 1)
		    {
		      if (index > prev_issue_bundle_ptr->stop)
			insert = 1;
		    }
		  /* 1st part is the first bundle of previous issue */
		  else 
		    {
		      prev_issue_bundle_ptr = prev_issue->bundles[0];

		      if (prev_issue_bundle_ptr->internal_stop_bit == 1)
			{
			  if (index > prev_issue_bundle_ptr->stop)
			    insert = 1;
			}
		        /* Should not be here if compaction correct!! */
		      else
			{
			  L_punt ("SM_commit_cb: compaction had bad stops!\n"
				  "Function: %s CB: %i Time: %i\n",
				  sm_cb->lcode_fn->name, lcode_cb->id,
				  issue_group_ptr->issue_time);
			}
		    }
		}

	      if ((!prepass_sched) && (insert))
		{
		  op = L_create_new_op (Lop_NO_OP);
		  op->src[0] = L_new_gen_int_operand (0);
		  L_insert_oper_after (lcode_cb,
				       lcode_cb->last_op, op);

		  /* 20030610 SZU
		   * Get the template mask from the template index.
		   * Get the syllable from the template mask.
		   * Currently hard coded for Itanium.
		   * How to get from mdes?
		   */
		  template_index = 
		    issue_group_ptr->bundles[current_bundle]->template_index;
		  template_mask = SM_get_template (sm_cb, template_index);
		  shift_var = SM_get_template_shift_var (sm_cb);
		  syll_type = 
		    (template_mask >>
		     ((slots_per_template - (index % slots_per_template) - 1) *
		      shift_var) & (C_pow2 (shift_var) - 1));

		  switch (syll_type)
		    {
		      case 0x1:
			/* M syllable */
			op->proc_opc = TAHOEop_NOP_M;
			break;
		      case 0x2:
			/* I syllable */
			op->proc_opc = TAHOEop_NOP_I;
			break;
		      case 0x4:
			/* B syllable */
			op->proc_opc = TAHOEop_NOP_B;
			break;
		      case 0x8:
			/* F syllable */
			op->proc_opc = TAHOEop_NOP_F;
			break;
		      case 0x10:
			/* L syllable */
			op->proc_opc = TAHOEop_NOP_X;
			break;
		      case 0x20:
			/* Bnop syllable */
			op->proc_opc = TAHOEop_NOP_B;
			break;
		      default:
			L_punt ("SM_commit_cb: illegal nop syllable type\n"
				"Issue time: %i Slot: %i Syllable 0x%x\n",
				issue_group_ptr->issue_time, index, syll_type);
		    }
		}
	    }
	  else
	    {
	      /* Not NULL, but rather a NOP operation */
	      if (SM_is_nop (sm_op))
		{
		  if (!prepass_sched)
		    {
		      op = L_create_new_op (Lop_NO_OP);
		      op->src[0] = L_new_gen_int_operand (0);
		      L_insert_oper_after (lcode_cb, lcode_cb->last_op, op);

		      switch (sm_op->syll_type)
			{
			  case 0x1:
			    /* M syllable */
			    op->proc_opc = TAHOEop_NOP_M;
			    break;
			  case 0x2:
			    /* I syllable */
			    op->proc_opc = TAHOEop_NOP_I;
			    break;
			  case 0x4:
			    /* B syllable */
			    op->proc_opc = TAHOEop_NOP_B;
			    break;
			  case 0x8:
			    /* F syllable */
			    op->proc_opc = TAHOEop_NOP_F;
			    break;
			  case 0x10:
			    /* L syllable */
			    op->proc_opc = TAHOEop_NOP_X;
			    break;
			  case 0x20:
			    /* Bnop syllable */
			    op->proc_opc = TAHOEop_NOP_B;
			    break;
			  default:
			    L_punt ("SM_commit_cb: illegal nop syllable type\n"
				    "Issue time: %i Slot: %i Syllable %i\n",
				    issue_group_ptr->issue_time, index,
				    sm_op->syll_type);
			}
		    }
		  SM_delete_nop (sm_op);
		}
	      /* Not a NOP */
	      else
		{
		  lcode_op = sm_op->lcode_op;

		  /* Mark this operation as speculated, if necessary */
		  if (sm_op->flags & SM_OP_SPECULATIVE)
		    lcode_op->flags |= L_OPER_SPECULATIVE;

		  /* Mark this operation as silent, if necessary */
		  if (sm_op->flags & SM_OP_SILENT)
		    lcode_op->flags |= L_OPER_MASK_PE;
		  
		  /* Mark this operation as speculated, if necessary */
		  if (sm_op->flags & SM_OP_SPECULATIVE)
		    L_mark_oper_speculative (lcode_op);
		  
		  /* Note: the above can MASK an op as well as mark
		   * it speculative.  This is now required for
		   * intrinsics, but is a hack. -- JWS
		   *
		   * Note: The above three conditions appear to be
		   * partially redundant.  Can we eliminte one or more
		   * of them?? -- MCM
		   */

		  /* Determine the max dest index which has a register */
		  for (max_dest_index = L_max_dest_operand - 1;
		       max_dest_index >= 0; max_dest_index--)
		    {
		      if (sm_op->dest[max_dest_index] != NULL)
			break;
		    }
			  
		  isl_attr = SM_attach_isl (sm_op, sched_cycle_offset);

		  if (cb_flags & SM_MODULO_RESOURCES)
		    {
		      /* Calculate the kernel cycle and stage this
		       * operation should go in.  (Use sched_cycle_offset
		       * to normalize issue time.)
		       * 
		       * For some reason, we are rewriting the isl
		       * attribute values for latency that were created
		       * by the above call to
		       * SM_attach_isl, but I am unsure why, -- MCM
		       */
		      L_set_int_attr_field (isl_attr, 0,
					    sm_op->sched_cycle +
					    sched_cycle_offset);
		      L_set_int_attr_field (isl_attr, 1, sm_op->sched_slot);
		      
		      /* Insert the attribute into the lcode op's
		       * attribute list
		       */
		      /* 20021111 SZU
		       * isl_attr already in sm_op->lcode_op->attr.
		       * Creates infinite loop.
		       */
#if 0
		      lcode_op->attr = L_concat_attr (lcode_op->attr,
						      isl_attr);
#endif

		      /* Calculate the kernel cycle and stage this
		       * operation should go in.
		       * (Use sched_cycle_offset to normalize issue time.)
		       */
		      kernel_cycle = (sm_op->sched_cycle +
				      sched_cycle_offset) % II;
		      kernel_stage = (sm_op->sched_cycle +
				      sched_cycle_offset) / II;

		      /* Wrap negative cycle times back around into
		       * the positive range.
		       * (This can happen only for unnormalized issue
		       * times.)
		       */
		      if (kernel_cycle < 0)
			kernel_cycle += II;

		      /* Update the isl attribute with the kernel cycle */
		      L_set_int_attr_field (isl_attr, 0, kernel_cycle);

		      /* Delete any existing stage attribute */
		      if ((stage_attr = L_find_attr (lcode_op->attr,
						     "stage")) != NULL)
			lcode_op->attr = L_delete_attr (lcode_op->attr,
							stage_attr);

		      /* Create stage attribute to hold kernel stage */
		      stage_attr = L_new_attr ("stage", 1);
		      L_set_int_attr_field (stage_attr, 0, kernel_stage);

		      /* Insert the attribute into the lcode op's
		       * attribute list
		       */
		      lcode_op->attr = L_concat_attr (lcode_op->attr,
						      stage_attr);

		      L_insert_oper_after (lcode_cb,
					   lcode_cb->last_op, lcode_op);
		    }

		  /* Otherwise (if not doing modulo scheduling), simply
		   * place operations in cb in schedule order.
		   */
		  else
		    {
		      /* Flag lcode io routine to place a new line
		       * before this operation if it is the first
		       * operation in this cycle (and not at the 
		       * beginning of the cb).  
		       * (Don't need to use sched_cycle_offset since
		       * on both sides.)
		       */
		      if ((sm_op->prev_sched_op != NULL) &&
			  (sm_op->sched_cycle !=
			   sm_op->prev_sched_op->sched_cycle))
			lcode_op->flags |= L_OPER_PRINT_CYCLE_DELIMITER;
		      else
			lcode_op->flags &=
			  ~L_OPER_PRINT_CYCLE_DELIMITER;

		      /* Insert op at end of cb */
		      L_insert_oper_after (lcode_cb,
					   lcode_cb->last_op, lcode_op);
		    }
		}
	    }
	  /* Advance index by num_slots operation takes.
	   * If NULL advance by one.
	   * 20030819 SZU
	   * Add in IPF specific hack.
	   * Advance index by 2 if L type NOP inserted.
	   */
	  if (sm_op)
	    index += sm_op->mdes_op->num_slots;
	  else if ((insert) && (syll_type == 0x10))
	    index += 2;
	  else
	    index++;
	}
      if (!prepass_sched)
	issue_group_ptr->full = 1;
    }

  L_reorder_dest_flows (lcode_cb);
}

/* Commit the schedule, reordering the lcode operations, creating
 * isl attributes, and marking speculative operations.
 */
void
SM_commit_cb_no_bundling (SM_Cb * sm_cb)
{
  L_Cb *lcode_cb;
  L_Oper *lcode_op = NULL;
  SM_Oper *sm_op, *from_op, *br_op;
  L_Attr *isl_attr, *stage_attr;
  SM_Dep *dep_in;
  unsigned int serial_number, cb_flags;
  int sched_cycle, II, kernel_cycle, kernel_stage;
  int last_cycle, this_cycle, sched_cycle_offset;
  /*int adjusted_branch_cycle, adjusted_branch_slot; */
  /*int backedge_cycle, min_sched_cycle;*/
  unsigned short sched_slot;
  Heap *kernel_heap = NULL;
  SM_Oper_Queue *br_queue;
  SM_Oper_Qentry *br_qentry;
  int prepass_sched;

  /* Only process cb's with operation in them. */
  if (sm_cb->first_serial_op == NULL)
    return;

  cb_flags = sm_cb->flags;
  II = sm_cb->II;

  /* Get prepass scheduling flag for ease of use -JCG 9/99 */
  prepass_sched = sm_cb->prepass_sched;

  /*
   * For now, mark operations speculative here 
   * 
   * Enhanced to handle predicated code more accurately (mark less
   * operations as speculative) by keeping track of all the branches
   * that this operation could have crossed (in br_queue) and testing their
   * predicate relationships -JCG 9/99
   */
  br_queue = SM_new_oper_queue ();

  for (sm_op = sm_cb->last_sched_op; sm_op != NULL;
       sm_op = sm_op->prev_sched_op)
    {
      /* Get this op's serial number and scheduled cycle for ease of use */
      serial_number = sm_op->serial_number;
      sched_cycle = sm_op->sched_cycle;

      /* Initially clear sm_op's speculative flag */
      sm_op->flags &= ~SM_OP_SPECULATIVE;

      /* This op is speculative if it moved above any branches.
       * Determine this by checking every branch that was scheduled
       * after it (if branch not predicate independent).
       * This only determines if made speculative by this pass
       * of scheduling.  Previous passes of scheduling may have
       * marked the lcode op but it will not show in the sm_op flags.
       * 
       * Also, if doing modulo scheduling, the operation is speculative
       * if scheduled an II or more before the backedge. -ITI/JCG 9/99
       */
      for (br_qentry = br_queue->first_qentry; br_qentry != NULL;
           br_qentry = br_qentry->next_qentry)
        {
          /* Get the branch we are considering for ease of use */
          br_op = br_qentry->sm_op;

          /* Mark as speculative if from below this branch and
           * not from a mutually exclusive execution path.
           */
          if ((serial_number > br_op->serial_number) &&
              !SM_mutually_exclusive_opers (sm_op, br_op))
            {

              sm_op->flags |= SM_OP_SPECULATIVE;
              L_mark_oper_speculative (sm_op->lcode_op);

              /* Sanity check, stores should never be marked speculative! */
              if (sm_op->mdes_flags & OP_FLAG_STORE)
                {
                  L_punt ("SM_commit_cb in %s cb %i op %i: "
                          "Speculative store!",
                          sm_cb->lcode_fn->name, sm_cb->lcode_cb->id,
                          sm_op->lcode_op->id);
                }

              /* No need to check other branches */
              break;
            }

          /* If doing modulo scheduling, mark as speculative if
           * scheduled more than II cycles before any branch (even
           * if didn't move past it) since the next kernel iteration 
           * will start execution before it is known that the iteration
           * should execute. -ITI/JCG 9/99
           */
          else if ((cb_flags & SM_MODULO_RESOURCES) &&
                   (sched_cycle < (br_op->sched_cycle - (II - 1))))
            {
              sm_op->flags |= SM_OP_SPECULATIVE;
              L_mark_oper_speculative (sm_op->lcode_op);

              /* Sanity check, stores should never be marked speculative! */
              if (sm_op->mdes_flags & OP_FLAG_STORE)
                {
                  L_punt ("SM_commit_cb in %s cb %i op %i: "
                          "Speculative store (modulo)!",
                          sm_cb->lcode_fn->name, sm_cb->lcode_cb->id,
                          sm_op->lcode_op->id);
                }

              /* No need to check other branches */
              break;
            }
        }

      /* JWS 19991002 : Generate check instructions */
      if (L_generate_spec_checks &&
          (sm_op->mdes_flags & (OP_FLAG_LOAD)) &&
          (!(sm_op->flags & (SM_OP_SPECULATIVE))) &&
          (!(sm_op->lcode_op->flags & (L_OPER_SPECULATIVE))))
        {
          /* Remove check if op is not speculative */
          SM_remove_matching_check (sm_cb, sm_op);
        }

      /* If this operation has some ignored soft dependences, 
       * determine if we need an silent version of this operation. 
       * (If any of the ignored ctrl dep_in have been violated,
       *  then we need a silent version).
       */
      if (sm_op->num_ignore_dep_in > 0)
        {
          sched_slot = sm_op->sched_slot;

          for (dep_in = sm_op->ext_src[SM_CTRL_ACTION_INDEX]->first_dep_in;
               dep_in != NULL; dep_in = dep_in->next_dep_in)
            {
              /* Only consider ignored dependences that are ignored
               * only because the operation can be speculated. -JCG 9/99
               */
              if ((dep_in->ignore & SM_CAN_SPEC_IGNORE) != SM_CAN_SPEC_IGNORE)
                continue;

              from_op = dep_in->from_action->sm_op;

              /* Handle cross-iteration soft-dependences appropriately */
              if (dep_in->omega != 0)
                {
                  /* Sanity check, omega must be 1 for cross-iteration
                   * soft dependences (assumed below). -ITI/JCG 9/99
                   */
                  if (dep_in->omega != 1)
                    {
                      L_punt ("SM_commit_cb in %s cb %i:\n"
                              "SM_CAN_SPEC_IGNORE dep with omega %i!",
                              sm_cb->lcode_fn->name, sm_cb->lcode_cb->id,
                              dep_in->omega);
                    }

                  /* For cross-iteration soft-dependences, if the
                   * operation is scheduled II or more cycles before the
                   * branch, it is speculative. -ITI/JCG 9/99.
                   */
                  if (sched_cycle <= (from_op->sched_cycle - II))
                    {
                      /* Stop, dependence found that requires 
                       *  silent version 
                       */
                      break;
                    }
                }

              /* Handle intra-iteration soft-dependences appropriately.

               * If the operation we are dependent on is scheduled 
               * after this operation, then we need a non-trapping
               * version of this operation.
               */
              else if ((sched_cycle < from_op->sched_cycle) ||
                       ((sched_cycle == from_op->sched_cycle) &&
                        (sched_slot < from_op->sched_slot)))
                {
                  /* Stop, dependence found that requires silent version */
                  break;
                }
            }

          /* If we have a dependence that requires a silent version,
           * mark as silent, and make sure marked speculative!
           */
          if (dep_in != NULL)
            {
              sm_op->flags |= SM_OP_SILENT;

              /* No longer print warning -JCG 6/99 */
#if 0
              /* Sanity check, better be speculative 
               * Warn for now, since I am rescheduling scheduled code and
               * this test doens't work in that case.
               */
              if (!(sm_op->flags & SM_OP_SPECULATIVE))
                {
                  SM_print_oper (stderr, sm_op);
                  fprintf (stderr,
                           "Warning: SILENT but NOT SPECULATIVE op!\n\n");
                }
#endif
            }
          else
            {
              sm_op->flags &= ~SM_OP_SILENT;

              /* No longer print warning -JCG 6/99 */
#if 0
              /* Sanity check, better not be speculative, well
               * can happen in predicated code.  Just warn for now!
               */
              if (sm_op->flags & SM_OP_SPECULATIVE)
                {
                  SM_print_oper (stderr, sm_op);
                  fprintf (stderr,
                           "Warning: NOT SILENT but SPECULATIVE op!\n\n");
                }
#endif
            }

        }

      /* If this op is a branch (non-JSR), add to beginning of branch queue 
       * so that earlier operations can determine if they have
       * been speculated across it. -JCG 9/99
       */
      if (sm_op->mdes_flags & (OP_FLAG_CBR | OP_FLAG_JMP | OP_FLAG_RTS))
        {
          SM_enqueue_oper_after (br_queue, sm_op, NULL);
        }
    }

  /* Free branch queue used for speculation */
  SM_delete_oper_queue (br_queue);
  br_queue = NULL;

  /* Get the lcode cb we are committing the sm_cb's schedule to */
  lcode_cb = sm_cb->lcode_cb;

  /* Remove every op in the sm_cb from the lcode cb starting from
   * the end of the cb.
   * 
   * For now, check to make sure all ops have been scheduled!
   */
  for (sm_op = sm_cb->last_serial_op; sm_op != NULL;
       sm_op = sm_op->prev_serial_op)
    {
      lcode_op = sm_op->lcode_op;

      /* Expect this operation to be marked as scheduled */
      if (!(sm_op->flags & SM_OP_SCHEDULED))
	L_punt ("SM_commit_cb in %s cb %i op %i: Unscheduled operation!",
		sm_cb->lcode_fn->name, lcode_cb->id, lcode_op->id);

      /* Remove lcode op from lcode cb */
      L_remove_oper (lcode_cb, lcode_op);
    }

  /* If SM_NORMALIZE_ISSUE cb flag is set, calculate the sched cycle offset
   * to use when writing out the "issue" part of the isl attributes.  
   */
  if (cb_flags & SM_NORMALIZE_ISSUE)
    {
      SM_set_sched_cycle_offset (sm_cb);
      sched_cycle_offset = sm_cb->sched_cycle_offset;
    }
  else
    {
      /* Otherwise, use sched cycle offset of 0 when the actual schedule cycle
       * should be used in the isl attribute.
       */
      sched_cycle_offset = 0;
    }

  /* If creating modulo scheduling kernel, create kernel heap that
   * is used to sort the kernel operations so they can be placed
   * in the cb in the expected order -ITI/JCG 8/99
   */
  if (cb_flags & SM_MODULO_RESOURCES)
    kernel_heap = Heap_Create (HEAP_MIN);

  /* Now place the ops back into the cb in the ordered scheduled */
  for (sm_op = sm_cb->first_sched_op; sm_op != NULL;
       sm_op = sm_op->next_sched_op)
    {
      lcode_op = sm_op->lcode_op;

      /* Mark this operation as speculated, if necessary */
      if ((sm_op->flags & SM_OP_SPECULATIVE) || (sm_op->flags & SM_OP_SILENT))
        L_mark_oper_speculative (lcode_op);

      /* Note: the above can MASK an op as well as mark
       * it speculative.  This is now required for
       * intrinsics, but is a hack. -- JWS
       */

      /* Mark this operation as silent, if necessary */
      if (sm_op->flags & SM_OP_SILENT)
        lcode_op->flags |= L_OPER_MASK_PE;

      isl_attr = SM_attach_isl (sm_op, sched_cycle_offset);

      /* If using modulo resources (assumed for modulo scheduling only), 
       * generate kernel order (wraps around itself), update isl attribute
       * and generate stage attribute.  -ITI/JCG 8/99
       */
      if (cb_flags & SM_MODULO_RESOURCES)
        {
          /* Calculate the kernel cycle and stage this operation should 
           * go in.  (Use sched_cycle_offset to normalize issue time.)
           */
          kernel_cycle = (sm_op->sched_cycle + sched_cycle_offset) % II;
          kernel_stage = (sm_op->sched_cycle + sched_cycle_offset) / II;

          /* Wrap negative cycle times back around into the positive range. 
           * (This can happen only for unnormalized issue times.)
           */
          if (kernel_cycle < 0)
            kernel_cycle += II;

          /* Update the isl attribute with the kernel cycle */
          L_set_int_attr_field (isl_attr, 0, kernel_cycle);

          /* Delete any existing stage attribute */
          if ((stage_attr = L_find_attr (lcode_op->attr, "stage")) != NULL)
            lcode_op->attr = L_delete_attr (lcode_op->attr, stage_attr);

          /* Create stage attribute to hold kernel stage */
          stage_attr = L_new_attr ("stage", 1);
          L_set_int_attr_field (stage_attr, 0, kernel_stage);

          /* Insert the attribute into the lcode op's attribute list */
          lcode_op->attr = L_concat_attr (lcode_op->attr, stage_attr);


          /* Insert the lcode op into a heap, indexing by the
           * kernel cycle and the slot.  
           */
          Heap_Insert (kernel_heap, (void *) sm_op,
                       (double) ((kernel_cycle << 16) + sm_op->sched_slot));
        }

      /* Otherwise (if not doing modulo scheduling), simply
       * place operations in cb in schedule order.
       */
      else
        {
          /* Flag lcode io routine to place a new line before this operation
           * if it is the first operation in this cycle (and not at the 
           * beginning of the cb).  
           * (Don't need to use sched_cycle_offset since on both sides.)
           */
          if ((sm_op->prev_sched_op != NULL) &&
              (sm_op->sched_cycle != sm_op->prev_sched_op->sched_cycle))
            lcode_op->flags |= L_OPER_PRINT_CYCLE_DELIMITER;
          else
            lcode_op->flags &= ~L_OPER_PRINT_CYCLE_DELIMITER;

          /* Insert op at end of cb */
          L_insert_oper_after (lcode_cb, lcode_cb->last_op, lcode_op);
        }
    }

  L_reorder_dest_flows (lcode_cb);

  if (cb_flags & SM_MODULO_RESOURCES)
    {
      /* Assume first operations with go in cycle 0.  If not,
       * we will get a blank line before the first cycle operations.
       */
      last_cycle = 0;

      /* Delete kernel queue (if present) -ITI/JCG 9/99 */
      if (sm_cb->kernel_queue != NULL)
        SM_delete_oper_queue (sm_cb->kernel_queue);

      /* Create kernel queue that holds sm_ops in kernel order */
      sm_cb->kernel_queue = SM_new_oper_queue ();

      /* Extract operations in kernel order and place them in the cb */
      while ((sm_op = (SM_Oper *) Heap_ExtractTop (kernel_heap)) != NULL)
        {
          /* Get the cycle this operation will fall in */
          this_cycle = (sm_op->sched_cycle + sched_cycle_offset) % II;

          /* Wrap negative cycle times back around into the positive range.
           * (This can happen only for unnormalized issue times.)
           */
          if (this_cycle < 0)
            this_cycle += II;

          /* Flag lcode io routine to place a new line before this operation
           * if it is the first operation in this cycle (and not at the 
           * beginning of the cb).
           */
          if (last_cycle != this_cycle)
            sm_op->lcode_op->flags |= L_OPER_PRINT_CYCLE_DELIMITER;
          else
            sm_op->lcode_op->flags &= ~L_OPER_PRINT_CYCLE_DELIMITER;

          /* Update last_cycle state */
          last_cycle = this_cycle;

          /* Insert op at end of cb */
          L_insert_oper_after (lcode_cb, lcode_cb->last_op, sm_op->lcode_op);

          /* Insert op at end of kernel queue */
          SM_enqueue_oper_before (sm_cb->kernel_queue, sm_op, NULL);
        }

      /* Dispose of heap, empty so no free routine needed */
      kernel_heap = Heap_Dispose (kernel_heap, NULL);
    }
}

/* 20031024 SZU
 * Depending on whether SM_do_template_bundling is set,
 * call the appropriate function.
 */
void
SM_commit_cb (SM_Cb * sm_cb)
{
  if (SM_do_template_bundling)
    {
      SM_commit_cb_template_bundling (sm_cb);
    }
  else
    {
      SM_commit_cb_no_bundling (sm_cb);
    }
}

/* This function is extracted from SM_commit_cb! */
void
SM_construct_temp_kernel_queue (SM_Cb * sm_cb)
{
  int sched_cycle_offset;
  Heap *kernel_heap;
  SM_Oper *sm_op;
  int kernel_cycle, kernel_stage, II = sm_cb->II, cb_flags = sm_cb->flags;
  L_Attr *isl_attr, *stage_attr, *cycle_attr;
  L_Oper *lcode_op;

  if (!(cb_flags & SM_NORMALIZE_ISSUE) || !(cb_flags & SM_MODULO_RESOURCES))
    return;

  SM_set_sched_cycle_offset(sm_cb);
  sched_cycle_offset = sm_cb->sched_cycle_offset;

  /* If creating modulo scheduling kernel, create kernel heap that
   * is used to sort the kernel operations so they can be placed
   * in the cb in the expected order -ITI/JCG 8/99
   */
  kernel_heap = Heap_Create (HEAP_MIN);

  /* Now place the ops back into the cb in the ordered scheduled */
  for (sm_op = sm_cb->first_sched_op; sm_op != NULL;
       sm_op = sm_op->next_sched_op)
    {
      /* Get lcode op for ease of use */
      lcode_op = sm_op->lcode_op;

      isl_attr = SM_attach_isl (sm_op, sched_cycle_offset);
      
      /* Calculate the kernel cycle and stage this operation should 
       * go in.  (Use sched_cycle_offset to normalize issue time.)
       */
      kernel_cycle = (sm_op->sched_cycle + sched_cycle_offset) % II;
      kernel_stage = (sm_op->sched_cycle + sched_cycle_offset) / II;

      /* Wrap negative cycle times back around into the positive range. 
       * (This can happen only for unnormalized issue times.)
       */
      if (kernel_cycle < 0)
        kernel_cycle += II;

      /* Update the isl attribute with the kernel cycle */
      L_set_int_attr_field (isl_attr, 0, kernel_cycle);
      
      /* Delete any existing stage attribute */
      if ((stage_attr = L_find_attr (lcode_op->attr, "stage")) != NULL)
	lcode_op->attr = L_delete_attr (lcode_op->attr, stage_attr);
      
      /* Create stage attribute to hold kernel stage */
      stage_attr = L_new_attr ("stage", 1);
      L_set_int_attr_field (stage_attr, 0, kernel_stage);
      
      /* Insert the attribute into the lcode op's attribute list */
      lcode_op->attr = L_concat_attr (lcode_op->attr, stage_attr);
      
      /* 20030220 SZU
       * SMH reconciliation
       */
      cycle_attr = L_new_attr ("cycle", 1);
      L_set_int_attr_field (cycle_attr, 0,
			    sm_op->sched_cycle + sched_cycle_offset);
      
      /* Insert the attribute into the lcode op's attribute list */
      lcode_op->attr = L_concat_attr (lcode_op->attr, cycle_attr);

      /* Insert the lcode op into a heap, indexing by the
       * kernel cycle and the slot.  
       */
      Heap_Insert (kernel_heap, (void *) sm_op,
                   (double) ((kernel_cycle << 16) + sm_op->sched_slot));
    }

  /* Delete kernel queue (if present) -ITI/JCG 9/99 */
  if (sm_cb->kernel_queue != NULL)
    SM_delete_oper_queue (sm_cb->kernel_queue);

  /* Create kernel queue that holds sm_ops in kernel order */
  sm_cb->kernel_queue = SM_new_oper_queue ();

  /* Extract operations in kernel order and place them in the cb */
  while ((sm_op = (SM_Oper *) Heap_ExtractTop (kernel_heap)) != NULL)
    {
      /* Insert op at end of kernel queue */
      SM_enqueue_oper_before (sm_cb->kernel_queue, sm_op, NULL);
    }

  /* Dispose of heap, empty so no free routine needed */
  kernel_heap = Heap_Dispose (kernel_heap, NULL);

  return;
}


void
SM_print_cb_schedule (SM_Cb * sm_cb)
{
  SM_Oper *sm_op;
  int found;
  int index;

  for (sm_op = sm_cb->first_sched_op; sm_op != NULL;
       sm_op = sm_op->next_sched_op)
    {
      fprintf (stderr,"%d ", sm_op->sched_cycle);

      SM_print_oper (stderr, sm_op);

      if (L_general_branch_opcode (sm_op->lcode_op))
        {
          found = FALSE;
          for (index = 0; index < sm_cb->num_exits; index++)
            {
              if (sm_cb->exit_op[index] == sm_op)
                {
                  fprintf (stderr,"------EXIT %f ", sm_cb->exit_weight[index]);
                  found = TRUE;
                  break;
                }
            }
          if (!found)
            L_punt ("SM_print_cb_schedule: exit_op array messed up");
        }

      fprintf (stderr,"\n");
    }
}









/* Deletes an sm_op, deleting all dependences, etc. and removing
 * it from all the lists and queues in the appropriate sm_cb.
 *
 * Initially designed for use during optimization.  Deletion of
 * branches all not currently supported.
 */
void
SM_delete_oper (SM_Oper * sm_op)
{
  SM_Cb *sm_cb;
  SM_Reg_Action *reg_action;
  SM_Action_Qentry *action_qentry;
  SM_Compatible_Alt *compatible_alt, *next_compatible_alt;
  SM_Reg_Action *def_after, *def_before;
  SM_Reg_Action *first_def_before, *last_def_before;
  SM_Reg_Action *first_def_after, *last_def_after;

  /* Get the sm_cb this op is in for ease of use */
  sm_cb = sm_op->sm_cb;

  /* For now, don't handle deletion of branches */
  if (sm_op->mdes_flags & (OP_FLAG_CBR | OP_FLAG_JMP | OP_FLAG_RTS))
    {
      L_punt ("SM_delete_oper: Deleting branches currently not supported");
    }

  /* If this operation is scheduled, unschedule it before deleting it */
  if (sm_op->flags & SM_OP_SCHEDULED)
    SM_unschedule_oper (sm_op, NULL);

  /* Delete all the explicit reg actions for the sm_op (this deletes
   * all the dependences also.)  Must be done after deleting
   * implicit actions, otherwise will get "action still in queue" errors.
   */
  while ((reg_action = sm_op->first_op_action) != NULL)
    {
      /* Delete the action from any queues it might be in
       * (for now, only the implicit_src/dest queues)
       */
      while ((action_qentry = reg_action->first_queue) != NULL)
        {
          SM_dequeue_action (action_qentry);
        }

      /* If this is a def action for a non-special register,
       * determine which adjacent defs dependences need to be rebuilt.
       */
      if ((reg_action->flags & SM_DEF_ACTION) &&
          (reg_action->rinfo->type != SM_EXT_ACTION_TYPE))
        {
          /* Find all previous definitions that deleting this def could
           * affect.
           */
          first_def_before = reg_action->prev_def;
          last_def_before = NULL;       /* Initially all defs */
          for (def_before = first_def_before; def_before != NULL;
               def_before = def_before->prev_def)
            {
              /* Stop the scan if the def_before dominates reg_action */
              if (SM_def_dominates_action (def_before, reg_action))
                {
                  /* Set last_def_before to the def action before this
                   * one, if any.
                   */
                  last_def_before = def_before->prev_def;
                  break;
                }
            }

          /* Find all post definitions that deleting this new def could
           * affect.
           */
          first_def_after = reg_action->next_def;
          last_def_after = NULL;        /* Initially all defs */
          for (def_after = first_def_after; def_after != NULL;
               def_after = def_after->next_def)
            {
              /* Stop the scan if reg_action post-dominates last_def_after */
              if (SM_def_post_dominates_action (def_after, reg_action))
                {
                  /* Set the last_def_after to the action after this
                   * one, if any.
                   */
                  last_def_after = def_after->next_def;
                  break;
                }
            }
        }
      else
        {
          /* Otherwise, no adjacent defs need to be updated */
          first_def_before = NULL;
          last_def_before = NULL;
          first_def_after = NULL;
          last_def_after = NULL;
        }

      /* Delete the reg action */
      SM_delete_reg_action (reg_action);

      /* Rebuild outgoing dependences for previous def actions,
       * if necessary
       */
      for (def_before = first_def_before; def_before != last_def_before;
           def_before = def_before->prev_def)
        {
          SM_rebuild_dest_reg_deps (def_before, SM_BUILD_DEP_OUT);
        }

      /* Rebuild incoming dependences for following def actions,
       * if necessary
       */
      for (def_after = first_def_after; def_after != last_def_after;
           def_after = def_after->next_def)
        {
          SM_rebuild_dest_reg_deps (def_after, SM_BUILD_DEP_IN);
        }
    }

  /* Delete implicit dest reg actions (if any).   All entries
   * should have been deleted above.
   */
  if (sm_op->implicit_dests != NULL)
    {
      SM_delete_action_queue (sm_op->implicit_dests);
      sm_op->implicit_dests = NULL;
    }

  /* Delete implicit src reg actions (if any).  All queue entries
   * should have been deleted above.
   */
  if (sm_op->implicit_srcs != NULL)
    {
      SM_delete_action_queue (sm_op->implicit_srcs);
      sm_op->implicit_srcs = NULL;
    }

  /* If this operation is in the dep_in_resolved queue, remove it 
   * NOTE: Must do AFTER deleting all incoming dependences, since
   *       deleting dependences will put it in the resolved queue!
   */
  if (sm_op->dep_in_resolved_qentry != NULL)
    {
      SM_dequeue_oper (sm_op->dep_in_resolved_qentry);
      sm_op->dep_in_resolved_qentry = NULL;
    }

  /* For now, punt if in any other oper queues.  In future, may
   * just want to remove it from those queues.
   */
  if (sm_op->first_queue != NULL)
    L_punt ("SM_delete_oper: op in a queue, cannot delete (currently)!");


  /* Free the compatable alt nodes */
  for (compatible_alt = sm_op->first_compatible_alt;
       compatible_alt != NULL; compatible_alt = next_compatible_alt)
    {
      /* Get next compatible alt before deleting this one */
      next_compatible_alt = compatible_alt->next_compatible_alt;

      L_free (SM_Compatible_Alt_pool, compatible_alt);
    }

  /* Free the mdes info for this lcode operation */
  L_free_oper_mdes_info (sm_op->lcode_op);

  /* Free the operand array */
  free (sm_op->operand);

  /* Free the late time array (if exists) */
  if (sm_op->late_time)
    free (sm_op->late_time);

  /* Free the schedule choices array */
  free (sm_op->options_chosen);

  /* Remove sm_op from the sm_cb's serial op linked lists */
  if (sm_op->prev_serial_op != NULL)
    sm_op->prev_serial_op->next_serial_op = sm_op->next_serial_op;
  else
    sm_cb->first_serial_op = sm_op->next_serial_op;
  if (sm_op->next_serial_op != NULL)
    sm_op->next_serial_op->prev_serial_op = sm_op->prev_serial_op;
  else
    sm_cb->last_serial_op = sm_op->prev_serial_op;

  /* Update cb's op and unsched count */
  sm_cb->op_count--;
  sm_cb->num_unsched--;

  /* Free the memory of the sm_op */
  L_free (SM_Oper_pool, sm_op);
}


/* Inserts the give lcode_op into the lcode_cb after 'after_sm_op->lcode_op'
 * and creates a matching sm_op for it and places it after after_sm_op.
 *
 * If after_sm_op is NULL, it placed at the beginning of the cb.
 *
 * Returns the new sm_op.
 */
SM_Oper *
SM_insert_oper_after (SM_Cb * sm_cb, L_Oper * lcode_op, SM_Oper * after_sm_op)
{
  SM_Oper *sm_op, *num_op;
  SM_Reg_Action *dest_action, *def_after, *def_before;
  SM_Action_Qentry *qentry;
  int i;
  Mdes *old_version1_mdes;
  unsigned int serial_number, prev_serial_number, next_serial_number;

  /* Save old lmdes value before setting to sm_cb's mdes */
  old_version1_mdes = lmdes;
  lmdes = sm_cb->version1_mdes;


  /* Sanity check, make sure not passed an IGNORE oper */
  if (op_flag_set (lcode_op->proc_opc, OP_FLAG_IGNORE))
    L_punt ("SM_insert_oper_after: passed an IGNORE oper!");

  /* For now, don't handle addition of branches */
  if (op_flag_set
      (lcode_op->proc_opc, OP_FLAG_CBR | OP_FLAG_JMP | OP_FLAG_RTS))
    {
      L_punt
        ("SM_insert_oper_after: Adding a branch is not currently supported");
    }

  /* Sanity check, lcode_op better not already be in linked list */
  if ((lcode_op->prev_op != NULL) || (lcode_op->next_op != NULL))
    {
      L_punt ("SM_insert_oper_after: lcode op %i already in a cb!",
              lcode_op->id);
    }

  /* Insert lcode op after the after_sm_op->lcode_op (if not NULL) */
  if (after_sm_op != NULL)
    {
      L_insert_oper_after (sm_cb->lcode_cb, after_sm_op->lcode_op, lcode_op);
    }
  /* Otherwise, insert at begining of cb */
  else
    {
      L_insert_oper_after (sm_cb->lcode_cb, NULL, lcode_op);
    }

  /* Alloc sm_op for this op */
  sm_op = (SM_Oper *) L_alloc (SM_Oper_pool);

  /* Initialize the fields of this op */
  SM_init_sm_op_fields (sm_op, sm_cb, lcode_op);

  /* If after_sm_op is NULL, add to beginning of op list */
  if (after_sm_op == NULL)
    {
      sm_op->prev_serial_op = NULL;
      sm_op->next_serial_op = sm_cb->first_serial_op;
      if (sm_cb->first_serial_op != NULL)
        sm_cb->first_serial_op->prev_serial_op = sm_op;
      else
        sm_cb->last_serial_op = sm_op;
      sm_cb->first_serial_op = sm_op;
    }

  /* Otherwise, add to op list after 'after_sm_op' */
  else
    {
      /* Sanity check */
      if (after_sm_op->sm_cb != sm_cb)
        L_punt ("SM_insert_oper_after: after_sm_op not from sm_cb!");

      sm_op->prev_serial_op = after_sm_op;
      sm_op->next_serial_op = after_sm_op->next_serial_op;
      if (after_sm_op->next_serial_op != NULL)
        after_sm_op->next_serial_op->prev_serial_op = sm_op;
      else
        sm_cb->last_serial_op = sm_op;
      after_sm_op->next_serial_op = sm_op;
    }

  /* Get the bounds on the serial number we can pick for this operation */
  if (after_sm_op == NULL)
    prev_serial_number = 0;
  else
    prev_serial_number = after_sm_op->serial_number;

  if (sm_op->next_serial_op == NULL)
    next_serial_number = prev_serial_number + 2048;
  else
    next_serial_number = sm_op->next_serial_op->serial_number;

  /* Pick a serial number precisely between the two bounds */
  serial_number = (next_serial_number + prev_serial_number) >> 1;

  /* Renumber all ops if this is not a valid pick */
  if ((serial_number <= prev_serial_number) ||
      (serial_number >= next_serial_number))
    {
      serial_number = 1024;
      for (num_op = sm_cb->first_serial_op; num_op != NULL;
           num_op = num_op->next_serial_op)
        {
          num_op->serial_number = serial_number;
          serial_number += 1024;
        }

    }

  /* Otherwise, use this serial number */
  else
    {
      sm_op->serial_number = serial_number;
    }

  /* Update the sm_cb's operation count */
  sm_cb->op_count++;
  sm_cb->num_unsched++;

  /* Add this operation's register actions to the reg info table */
  SM_add_reg_actions_for_op (sm_op);

  /* Build all the incoming and outgoing dependences for this operation,
   * prevent any redundant deps from being added to make opti routines
   * cleaner.
   */
  SM_build_oper_dependences (sm_cb, sm_op,
                             SM_BUILD_DEP_OUT | SM_BUILD_DEP_IN |
                             SM_PREVENT_REDUNDANT_DEPS);

  /* For each register destination action, rebuild dependences for
   * adjancent destination actions.
   */
  for (i = 0; i < L_max_dest_operand; i++)
    {
      /* Only process non-NULL dest actions */
      if ((dest_action = sm_op->dest[i]) == NULL)
        continue;

      /* For all previous definitions that this new def could affect,
       * rebuild all outgoing dependences for them.
       */
      for (def_before = dest_action->prev_def; def_before != NULL;
           def_before = def_before->prev_def)
        {
          SM_rebuild_dest_reg_deps (def_before, SM_BUILD_DEP_OUT);

          /* Stop the scan if the previous def dominates this def */
          if (SM_def_dominates_action (def_before, dest_action))
            break;
        }

      /* For all post definitions that this new def could affect,
       * rebuild all incoming dependences for them.
       */
      for (def_after = dest_action->next_def; def_after != NULL;
           def_after = def_after->next_def)
        {
          SM_rebuild_dest_reg_deps (def_after, SM_BUILD_DEP_IN);

          /* Stop the scan if dest_action post-dominates def_after */
          if (SM_def_post_dominates_action (def_after, dest_action))
            break;
        }
    }

  /* For each implicit register destination action, rebuild dependences for
   * adjancent destination actions.
   */
  if (sm_op->implicit_dests != NULL)
    {
      for (qentry = sm_op->implicit_dests->first_qentry; qentry != NULL;
           qentry = qentry->next_qentry)
        {
          /* Get this implicit def action */
          dest_action = qentry->action;

          /* For all previous definitions that this new def could affect,
           * rebuild all outgoing dependences for them.
           */
          for (def_before = dest_action->prev_def; def_before != NULL;
               def_before = def_before->prev_def)
            {
              SM_rebuild_dest_reg_deps (def_before, SM_BUILD_DEP_OUT);

              /* Stop the scan if the previous def dominates this def */
              if (SM_def_dominates_action (def_before, dest_action))
                break;
            }

          /* For all post definitions that this new def could affect,
           * rebuild all incoming dependences for them.
           */
          for (def_after = dest_action->next_def; def_after != NULL;
               def_after = def_after->next_def)
            {
              SM_rebuild_dest_reg_deps (def_after, SM_BUILD_DEP_IN);

              /* Stop the scan if dest_action post-dominates def_after */
              if (SM_def_post_dominates_action (def_after, dest_action))
                break;
            }
        }
    }


  /* Restore original mdes value */
  lmdes = old_version1_mdes;

  /* Return the newly created op */
  return (sm_op);
}

/* Moves the given sm_op after 'after_mc_op' in both the sm_cb and
 * in the lcode cb.  Assumes the move_sm_op is already in the
 * cb.  
 *
 * NOTE: Does not redraw any dependence!  Assumes that this move
 * is legal and thus the dependences are the same.  Use SM_change_operand()
 * to update register dependences if necessary.
 * 
 * For now, cannot move branches.
 *
 * If after_sm_op is NULL, it placed at the beginning of the cb.
 */
void
SM_move_oper_after (SM_Oper * move_sm_op, SM_Oper * after_sm_op)
{
  SM_Cb *sm_cb;
  SM_Oper *num_op;
  SM_Reg_Action *op_action;
  unsigned int serial_number, prev_serial_number, next_serial_number;

  /* For now, don't handle moving of branches */
  if (move_sm_op->mdes_flags & (OP_FLAG_CBR | OP_FLAG_JMP | OP_FLAG_RTS))
    {
      L_punt
        ("SM_move_oper_after: Moving a branch is not currently supported");
    }

  /* Get sm_cb for ease of use */
  sm_cb = move_sm_op->sm_cb;

  /* Sanity check, after_sm_op better be in same cb (just because
   * I don't want to handle this case for now).
   */
  if ((after_sm_op != NULL) && (sm_cb != after_sm_op->sm_cb))
    L_punt ("SM_move_oper_after: Cannot currently move to difference cb!");

  /* Sanity check, does not make sense to move operation after itself */
  if (move_sm_op == after_sm_op)
    L_punt ("SM_move_oper_after: Cannot move op after itself!");

  /* Remove the lcode_op for move_sm_op from the lcode cb */
  L_remove_oper (sm_cb->lcode_cb, move_sm_op->lcode_op);

  /* Insert lcode op after the after_sm_op->lcode_op (if not NULL) */
  if (after_sm_op != NULL)
    {
      L_insert_oper_after (sm_cb->lcode_cb, after_sm_op->lcode_op,
                           move_sm_op->lcode_op);
    }
  /* Otherwise, insert at begining of cb */
  else
    {
      L_insert_oper_after (sm_cb->lcode_cb, NULL, move_sm_op->lcode_op);
    }

  /* Remove move_sm_op from the sm_cb's serial op linked lists */
  if (move_sm_op->prev_serial_op != NULL)
    {
      move_sm_op->prev_serial_op->next_serial_op = move_sm_op->next_serial_op;
    }
  else
    {
      sm_cb->first_serial_op = move_sm_op->next_serial_op;
    }
  if (move_sm_op->next_serial_op != NULL)
    {
      move_sm_op->next_serial_op->prev_serial_op = move_sm_op->prev_serial_op;
    }
  else
    {
      sm_cb->last_serial_op = move_sm_op->prev_serial_op;
    }

  /* If after_sm_op is NULL, add to beginning of op list */
  if (after_sm_op == NULL)
    {
      move_sm_op->prev_serial_op = NULL;
      move_sm_op->next_serial_op = sm_cb->first_serial_op;
      if (sm_cb->first_serial_op != NULL)
        sm_cb->first_serial_op->prev_serial_op = move_sm_op;
      else
        sm_cb->last_serial_op = move_sm_op;
      sm_cb->first_serial_op = move_sm_op;
    }

  /* Otherwise, add to op list after 'after_sm_op' */
  else
    {
      move_sm_op->prev_serial_op = after_sm_op;
      move_sm_op->next_serial_op = after_sm_op->next_serial_op;
      if (after_sm_op->next_serial_op != NULL)
        after_sm_op->next_serial_op->prev_serial_op = move_sm_op;
      else
        sm_cb->last_serial_op = move_sm_op;
      after_sm_op->next_serial_op = move_sm_op;
    }

  /* Get the bounds on the serial number we can pick for this operation */
  if (after_sm_op == NULL)
    prev_serial_number = 0;
  else
    prev_serial_number = after_sm_op->serial_number;

  if (move_sm_op->next_serial_op == NULL)
    next_serial_number = prev_serial_number + 2048;
  else
    next_serial_number = move_sm_op->next_serial_op->serial_number;

  /* Pick a serial number precisely between the two bounds */
  serial_number = (next_serial_number + prev_serial_number) >> 1;

  /* Renumber all ops if this is not a valid pick */
  if ((serial_number <= prev_serial_number) ||
      (serial_number >= next_serial_number))
    {
      serial_number = 1024;
      for (num_op = sm_cb->first_serial_op; num_op != NULL;
           num_op = num_op->next_serial_op)
        {
          num_op->serial_number = serial_number;
          serial_number += 1024;
        }
    }

  /* Otherwise, use this serial number */
  else
    {
      move_sm_op->serial_number = serial_number;
    }

  /* For each register action, reposition it so it is in the
   * proper place with respect to move_sm_op's new serial number.
   */
  for (op_action = move_sm_op->first_op_action; op_action != NULL;
       op_action = op_action->next_op_action)
    {
      SM_reposition_reg_action (op_action);
    }

  /* Everything (except possibly dependences) should now be as if the
   * operation was originally here.
   */
}

/* Changes an operand in both the sm_op and the underlining lcode op.
 * Automatically delete/addes register actions and dependences.
 *
 * NOTE: Deletes the both existing operands (if any), and makes 
 * a copy of new_operand.  If you are going to use the old operand,
 * set it to null in the lcode_op before you call this!
 */
void
SM_change_operand (SM_Oper * sm_op, int operand_type,
                   int operand_number, L_Operand * new_operand)
{
  L_Oper *lcode_op;
  L_Operand **lcode_operand_ptr = NULL;
  SM_Reg_Action **sm_operand_ptr = NULL, *dest_action;
  SM_Reg_Action *def_after, *def_before;
  SM_Reg_Action *first_def_before, *last_def_before;
  SM_Reg_Action *first_def_after, *last_def_after;
  Mdes *old_version1_mdes;
  int *number;

  /* Get the lcode_op for ease of use */
  lcode_op = sm_op->lcode_op;

  /* Save the old lmdes value and set it to the mdes we are using */
  old_version1_mdes = lmdes;
  lmdes = sm_op->sm_cb->version1_mdes;

  /* If the operand being changed is a dest operand, the deps for
   * the surrounding def actions may need to be rebuilt.  Initially
   * set the range of def actions to rebuild to NULL.
   */
  first_def_before = NULL;
  last_def_before = NULL;
  first_def_after = NULL;
  last_def_after = NULL;

  /* Get the number of each type of operand for ease of use */
  number = lmdes->number;

  /* Get the src/dest/pred pointers depending on operand type */
  switch (operand_type)
    {
    case MDES_SRC:
      /* Sanity check, make sure operand number legal */
      if (operand_number >= number[MDES_SRC])
        L_punt ("SM_change_operand: src[%i] out of bounds!", operand_number);

      lcode_operand_ptr = &lcode_op->src[operand_number];
      sm_operand_ptr = &sm_op->src[operand_number];
      break;

    case MDES_DEST:
      /* Sanity check, make sure operand number legal */
      if (operand_number >= number[MDES_DEST])
        L_punt ("SM_change_operand: dest[%i] out of bounds!", operand_number);

      lcode_operand_ptr = &lcode_op->dest[operand_number];
      sm_operand_ptr = &sm_op->dest[operand_number];

      /* Get the dest action (if any) */
      dest_action = *sm_operand_ptr;

      if (dest_action != NULL)
        {
          /* Find all previous definitions that deleting this def could
           * affect.
           */
          first_def_before = dest_action->prev_def;
          last_def_after = NULL;        /* Initially all defs */
          for (def_before = first_def_before; def_before != NULL;
               def_before = def_before->prev_def)
            {
              /* Stop the scan if the def_before dominates dest_action */
              if (SM_def_dominates_action (def_before, dest_action))
                {
                  /* Set last_def_before to the def action before this
                   * one, if any.
                   */
                  last_def_before = def_before->prev_def;
                  break;
                }
            }

          /* Find all post definitions that deleting this new def could
           * affect.
           */
          first_def_after = dest_action->next_def;
          last_def_after = NULL;        /* Initially all defs */
          for (def_after = first_def_after; def_after != NULL;
               def_after = def_after->next_def)
            {
              /* Stop the scan if dest_action post-dominates last_def_after */
              if (SM_def_post_dominates_action (def_after, dest_action))
                {
                  /* Set the last_def_after to the action after this
                   * one, if any.
                   */
                  last_def_after = def_after->next_def;
                  break;
                }
            }
        }
      break;

    case MDES_PRED:
      /* Sanity check, make sure operand number legal */
      if (operand_number >= number[MDES_PRED])
        L_punt ("SM_change_operand: pred[%i] out of bounds!", operand_number);

      lcode_operand_ptr = &lcode_op->pred[operand_number];
      sm_operand_ptr = &sm_op->pred[operand_number];
      break;

    default:
      L_punt ("SM_change_operand: Unsupported operand type %i", operand_type);
    }

  /* Delete the old reg action (if any) (also deletes dependences) */
  if (*sm_operand_ptr != NULL)
    SM_delete_reg_action (*sm_operand_ptr);

  /* Delete the old lcode operand (if any) */
  if (*lcode_operand_ptr != NULL)
    L_delete_operand (*lcode_operand_ptr);

  /* Rebuild outgoing dependences for previous def actions, if necessary */
  for (def_before = first_def_before; def_before != last_def_before;
       def_before = def_before->prev_def)
    {
      SM_rebuild_dest_reg_deps (def_before, SM_BUILD_DEP_OUT);
    }

  /* Rebuild incoming dependences for following def actions, if necessary */
  for (def_after = first_def_after; def_after != last_def_after;
       def_after = def_after->next_def)
    {
      SM_rebuild_dest_reg_deps (def_after, SM_BUILD_DEP_IN);
    }

  /* Copy the new operand to the lcode oper */
  *lcode_operand_ptr = L_copy_operand (new_operand);

  /* If a register, add a register action unless a predicate
   * src that is not pred[0]
   */
  if (SM_is_reg (new_operand) &&
      ((operand_type != MDES_PRED) || (operand_number == 0)))
    {
      *sm_operand_ptr = SM_add_reg_action (sm_op, operand_type,
                                           operand_number, new_operand);

      /* 20030220 SZU
       * SMH initialization
       * 20030907 SZU
       * In JWS new version in SM, following if is cut out.
       * Keep for now since in SMH, shouldn't hurt.
       */
      if (*sm_operand_ptr)
	{
	  /* Build the dependences for this operand.
	   * Prevent redundant dependences from be added to 
	   * make optimizations easier to write.
	   */
	  switch (operand_type)
	    {
	    case MDES_PRED:
	    case MDES_SRC:
	      SM_build_src_reg_deps (*sm_operand_ptr,
				     SM_BUILD_DEP_OUT | SM_BUILD_DEP_IN |
				     SM_PREVENT_REDUNDANT_DEPS);
	      break;

	    case MDES_DEST:
	      /* Get the dest action */
	      dest_action = *sm_operand_ptr;


	      SM_build_dest_reg_deps (*sm_operand_ptr,
				      SM_BUILD_DEP_OUT | SM_BUILD_DEP_IN |
				      SM_PREVENT_REDUNDANT_DEPS);

	      /* For all previous definitions that this new def could affect,
	       * rebuild all outgoing dependences for them.
	       */
	      for (def_before = dest_action->prev_def; def_before != NULL;
		   def_before = def_before->prev_def)
		{
		  SM_rebuild_dest_reg_deps (def_before, SM_BUILD_DEP_OUT);

		  /* Stop the scan if the previous def dominates this def */
		  if (SM_def_dominates_action (def_before, dest_action))
		    break;
		}

	      /* For all post definitions that this new def could affect,
	       * rebuild all incoming dependences for them.
	       */
	      for (def_after = dest_action->next_def; def_after != NULL;
		   def_after = def_after->next_def)
		{
		  SM_rebuild_dest_reg_deps (def_after, SM_BUILD_DEP_IN);

		  /* Stop the scan if dest_action post-dominates def_after */
		  if (SM_def_post_dominates_action (def_after, dest_action))
		    break;
		}
	      break;

	    default:
	      L_punt ("SM_change_operand: Unsupported reg operand type %i",
		      operand_type);
	    }
	}
    }

  /* Otherwise, set the src_action to NULL to flag that it is not a
   * register.
   */
  else
    {
      *sm_operand_ptr = NULL;
    }

  /* If this operation is not scheduled, make sure it is in
   * (or not in) the dep_in_resolved queue.
   */
  if (!(sm_op->flags & SM_OP_SCHEDULED))
    {
      /* Should it be in the dep_in_resolved_queue ? */
      if ((sm_op->num_unresolved_hard_dep_in == 0) &&
          /* EMN (sm_op->num_unresolved_soft_dep_in == 0) && */
          (!sm_op->ignore))
        {
          /* Yes, add if not already in it */
          if (sm_op->dep_in_resolved_qentry == NULL)
            {
              sm_op->dep_in_resolved_qentry =
                SM_enqueue_oper_before (sm_op->sm_cb->dep_in_resolved,
                                        sm_op, NULL);
            }
        }
      else
        {
          /* No, remove it if it is */
          if (sm_op->dep_in_resolved_qentry != NULL)
            {
              SM_dequeue_oper (sm_op->dep_in_resolved_qentry);
              sm_op->dep_in_resolved_qentry = NULL;
            }
        }
    }

  /* Restore the lmdes variable to its original value */
  lmdes = old_version1_mdes;
}


/* Calculates the cycle lower bound due to the passed action.
 * If the passed action is NULL, cycle -200000000 is returned.
 * If an action's input dependencence have not been all resolved,
 * then 2000000000 is returned.
 * Otherwise, the earliest cycle an operation may go due to that
 * operation is returned.
 */
int
SM_action_cycle_lower_bound (SM_Reg_Action * action)
{
  SM_Dep *dep_in;
  int cycle_lower_bound, cycle_dep_resolved;

  if (action == NULL)
    return (SM_MIN_CYCLE);

  cycle_lower_bound = SM_MIN_CYCLE;
  for (dep_in = action->first_dep_in; dep_in != NULL;
       dep_in = dep_in->next_dep_in)
    {
      /* Ignore IGNORE deps */
      if (dep_in->ignore)
        continue;

      /* If from op is not scheduled yet, return SM_MAX_CYCLE */
      if (!(dep_in->from_action->sm_op->flags & SM_OP_SCHEDULED))
        return (SM_MAX_CYCLE);

      /* Calculate the constraint of this dependence */
      cycle_dep_resolved = dep_in->from_action->sm_op->sched_cycle +
        dep_in->min_delay;

      /* Update the lowever bound if necessary */
      if (cycle_dep_resolved > cycle_lower_bound)
        cycle_lower_bound = cycle_dep_resolved;
    }

  /* Return the min cycle the action can be scheduled in */
  return (cycle_lower_bound);
}

/* Returns the number of cycles spent in the cb (best case), based
 * on the profiling weights and the height of each exit.
 */
double
SM_calc_best_case_cycles (SM_Cb * sm_cb)
{
  int index, num_exits;
  double cycles, add_cycles, *exit_weight;
  SM_Oper **exit_op;


  /* Sanity check, all operations should be scheduled */
  if (sm_cb->num_unsched != 0)
    {
      L_punt ("SM_calc_best_case_cycles: %s cb %i, %i of %i ops unsched!",
              sm_cb->lcode_fn->name, sm_cb->lcode_cb->id,
              sm_cb->num_unsched, sm_cb->op_count);
    }

  /* Return zero cycles for empty cbs */
  if (sm_cb->last_sched_op == NULL)
    return (0.0);

  /* Get the number of exits from this cb for ease of use */
  num_exits = sm_cb->num_exits;

  /* Get exits arrays for ease of use */
  exit_op = sm_cb->exit_op;
  exit_weight = sm_cb->exit_weight;

  /* Add up the weight of each exit (multiplied by the height of the
   * exit).
   */
  cycles = 0.0;
  for (index = 0; index < num_exits; index++)
    {
      if (exit_op[index] && exit_op[index]->ignore)
        continue;

      /* For the fall-thru path, use the height of the cb */
      if (exit_op[index] == NULL)
        {
          add_cycles = exit_weight[index] *
            (sm_cb->last_sched_op->sched_cycle + 1);

        }

      /* Otherwise, use the height of the exit op */
      else
        {
          add_cycles = exit_weight[index] * (exit_op[index]->sched_cycle + 1);
        }
      cycles += add_cycles;
    }

  /* return the number of cycles spent in this cb */
  return (cycles);
}

/* 20030220 SZU
 * SMH reconciliation
 */
int
SM_softfix_promotion (SM_Dep * dep, int issue_time, int mode)
{
  SM_Oper *from_op, *to_op;

  from_op = dep->from_action->sm_op;
  to_op = dep->to_action->sm_op;

  switch (mode)
    {
    case SOFTFIX_POSSIBLE:
      if ((from_op->flags & SM_OP_SCHEDULED) &&
          (from_op->sched_cycle != issue_time))
        return 1;
      if ((to_op->flags & SM_OP_SCHEDULED) &&
          (to_op->sched_cycle != issue_time))
        return 1;
      break;
    case SOFTFIX_COMMIT:
      /* 20030910 SZU
       * Doesn't work for case of zero cycle scheduling.
       * Unsure what this case supposed to check to begin w/.
       */
#if 0
      if (((from_op->flags & SM_OP_SCHEDULED) &&
           from_op->sched_cycle == issue_time) ||
          ((to_op->flags & SM_OP_SCHEDULED) &&
           to_op->sched_cycle == issue_time))
        L_punt ("SM_softfix_promotion: commit no longer valid");
#else
#if 0
      if (((from_op->flags & SM_OP_SCHEDULED) &&
           from_op->sched_cycle == issue_time) ||
          ((to_op->flags & SM_OP_SCHEDULED) &&
           to_op->sched_cycle == issue_time))
	if (from_op->sched_cycle != to_op->sched_cycle)
	  L_punt ("SM_softfix_promotion: commit no longer valid");
#endif
#endif

      /* 20030929 SZU
       * Trying to see which ops, and the deps that are broken.
       */
#if 1
      printf ("Committing break of soft dep op%d -> op%d\n",
              from_op->lcode_op->id, to_op->lcode_op->id);
#else
      fprintf (stderr, "Committing break of soft dep op%d -> op%d\n",
	       from_op->lcode_op->id, to_op->lcode_op->id);
      fprintf (stderr, "function %s CB %d\n",
	       to_op->sm_cb->lcode_fn->name, to_op->sm_cb->lcode_cb->id);
      fprintf (stderr, "from_op:\n");
      SM_print_oper_dependences (stderr, from_op);
      fprintf (stderr, "\nto_op:\n");
      SM_print_oper_dependences (stderr, to_op);
#endif
      SM_ignore_dep (dep, dep->flags);
      return 1;
      break;
    case SOFTFIX_UNDO:
      if (to_op->sched_cycle == issue_time)
        SM_enable_ignored_dep (dep, dep->flags);
      if (from_op->sched_cycle == issue_time)
        SM_enable_ignored_dep (dep, dep->flags);
      return 1;
      break;
    }
  return 0;
}

void
SM_undo_fix_soft_dep (SM_Oper * sm_op)
{
  SM_Dep *dep_in, *dep_out;
  SM_Reg_Action *op_action;

  /* Find the dep that was violated */
  for (op_action = sm_op->first_op_action; op_action != NULL;
       op_action = op_action->next_op_action)
    {
      /* Scan the dep in constraints to build lower bound */
      for (dep_in = op_action->first_dep_in; dep_in != NULL;
           dep_in = dep_in->next_dep_in)
        {
          /* Only look at _ignored_ deps */
          if (!dep_in->ignore)
            continue;

          if ((dep_in->flags & SM_SOFTFIX_PROMOTION))
            {
              if (!SM_softfix_promotion (dep_in, sm_op->sched_cycle,
                                         SOFTFIX_UNDO))
                L_punt ("SM_undo_fix_soft_dep: Unable to undo fix");
            }
        }
      /* Scan the dep in constraints to build upper bound */
      for (dep_out = op_action->first_dep_out; dep_out != NULL;
           dep_out = dep_out->next_dep_out)
        {
          /* Only look at _ignored_ deps */
          if (!dep_out->ignore)
            continue;

          if ((dep_out->flags & SM_SOFTFIX_PROMOTION))
            {
              if (!SM_softfix_promotion (dep_out, sm_op->sched_cycle,
                                         SOFTFIX_UNDO))
                L_punt ("SM_undo_fix_soft_dep: Unable to undo fix");
            }
        }
    }
}

int
SM_fix_soft_dep (SM_Oper * sm_op, int issue_time, int mode)
{
  SM_Oper *from_op, *to_op;
  SM_Dep *dep_in, *dep_out;
  SM_Reg_Action *op_action;
  int cycle_dep_resolved;
  int found;

  found = 0;

  for (op_action = sm_op->first_op_action; op_action != NULL;
       op_action = op_action->next_op_action)
    {
      /* Scan the dep in constraints to build lower bound */
      for (dep_in = op_action->first_dep_in; dep_in != NULL;
           dep_in = dep_in->next_dep_in)
        {
          if (dep_in->ignore)
            continue;
          if (!(dep_in->flags & SM_SOFT_DEP))
            continue;
          from_op = dep_in->from_action->sm_op;

          /* If predecessor NOT scheduled, no dep has been violated (yet) */
          if (!(from_op->flags & SM_OP_SCHEDULED))
            continue;

          /* What is the lower bound required for soft dep */
          if (dep_in->omega == 0)
            {
              cycle_dep_resolved = from_op->sched_cycle + dep_in->min_delay;
            }
          else
            {
              cycle_dep_resolved = from_op->sched_cycle + dep_in->min_delay -
                (dep_in->omega * from_op->sm_cb->II);
            }

          /* Was soft dep violated? */
          if (issue_time >= cycle_dep_resolved)
            continue;

          /* 
           * Can soft dep be broken
           ************************************/
          found = 0;
          if ((dep_in->flags & SM_SOFTFIX_PROMOTION))
            {
              found = 1;
              if (!SM_softfix_promotion (dep_in, issue_time, mode))
                return 0;       /* Not fixable */
            }
#if 0
          if ((dep_in->flags & YOURFLAG))
            {
              found = 1;
              if (!SM_softfix_yourfix (dep_in, issue_time, mode))
                return 0;       /* Not fixable */
            }
#endif
          if (!found)
            return 0;           /* Not fixable */
        }
      dep_in = NULL;


      /* Scan the dep in constraints to build lower bound */
      for (dep_out = op_action->first_dep_out; dep_out != NULL;
           dep_out = dep_out->next_dep_out)
        {
          if (dep_out->ignore)
            continue;
          if (!(dep_out->flags & SM_SOFT_DEP))
            continue;
          to_op = dep_out->to_action->sm_op;

          /* If successor NOT scheduled, no dep has been violated (yet) */
          if (!(to_op->flags & SM_OP_SCHEDULED))
            continue;

          /* What is the upper bound required for soft dep */
          if (dep_out->omega == 0)
            {
              cycle_dep_resolved = to_op->sched_cycle - dep_out->min_delay;
            }
          else
            {
              cycle_dep_resolved = to_op->sched_cycle - dep_out->min_delay +
                (dep_out->omega * to_op->sm_cb->II);
            }

          /* Was soft dep violated? */
	  /* 20030910 SZU
	   * Also check the slots for zero cycle dependences.
	   */
#if 0
          if (issue_time <= cycle_dep_resolved)
            continue;
#else
	    {
	      int violate = 0;

	      if ((issue_time > cycle_dep_resolved) ||
		  ((issue_time == cycle_dep_resolved) &&
		   (to_op->sched_slot < sm_op->sched_slot)))
		violate = 1;

	      if (!violate)
		continue;
	    }
#endif

          /* 
           * Can soft dep be broken 
           ************************************/
          found = 0;
          if ((dep_out->flags & SM_SOFTFIX_PROMOTION))
            {
              found = 1;
              if (!SM_softfix_promotion (dep_out, issue_time, mode))
                return 0;       /* Not fixable */
            }
#if 0
          if ((dep_out->flags & YOURFLAG))
            {
              found = 1;
              if (!SM_softfix_yourfix (dep_out, issue_time, mode))
                return 0;       /* Not fixable */
            }
#endif
          if (!found)
            return 0;           /* Not fixable */
        }
    }
  dep_out = NULL;

  return 1;
}

/*
 * Performs DHASY scheduling on sm_cb.  
 *
 * If sm_cb already scheduled (or partially scheduled), the
 * cb will be first unscheduled, then it will be scheduled again.
 */
void
SM_schedule_cb (SM_Cb * sm_cb)
{
  /* 20030220 SZU
   * SMH reconciliation
   */
  int issue_time, reset_time;
  SM_Oper *sm_op, *sched_op, *scan_op;
  SM_Oper_Qentry *qentry;

  /* If cb is empty, return now */
  if (sm_cb->first_serial_op == NULL)
    return;

  /* If any operations are scheduled, unschedule them first */
  if (sm_cb->first_sched_op != NULL)
    SM_unschedule_cb (sm_cb);

  /* Sanity check, make sure there is at least one ready operation now */
  if (sm_cb->dep_in_resolved->first_qentry == NULL)
    {
      L_punt ("SM_schedule_cb: %i unscheduled operations but none ready!",
              sm_cb->op_count);
    }

  /* Sanity check, Does num_unscheduled count equal the number of ops */
  if ((sm_cb->num_unsched + sm_cb->num_ignored) != sm_cb->op_count)
    {
      L_punt
        ("SM_schedule_cb: Before scheduling num unsched %i + %i > # op %i!",
         sm_cb->num_unsched, sm_cb->num_ignored, sm_cb->op_count);
    }

  /* Calculate the priorities for this cb's operations */
  SM_calculate_priorities (sm_cb, 0);

  /* Perform list scheduling until all operations are scheduled
   * or have gone 10000 cycles (likely a problem somewhere).
   */
  /* 20030219 SZU
   * SMH reconciliation
   */
  reset_time = 0;
  for (issue_time = 0;
       (reset_time < 100) && (sm_cb->num_unsched > 0); issue_time++)
#if 0
  for (issue_time = 0;
       (issue_time < 10000) && (sm_cb->num_unsched > 0); issue_time++)
#endif
    {
      /* 20030219 SZU
       * SMH reconciliation
       */
      reset_time++;
#ifdef DEBUG
      printf("Time %d\n",issue_time);
#endif
      /* Reset the tested bit on the resolved operations */
      for (qentry = sm_cb->dep_in_resolved->first_qentry;
           qentry != NULL; qentry = qentry->next_qentry)
        {
          qentry->sm_op->flags &= ~SM_OP_TESTED;
        }

      /* Attempt to schedule the operations in priority order
       * until all have been tested.
       */
      while (1)
        {
          /* Find the highest priority untested operation */
          sched_op = NULL;
          for (qentry = sm_cb->dep_in_resolved->first_qentry;
               qentry != NULL; qentry = qentry->next_qentry)
            {
              scan_op = qentry->sm_op;
	      /* 20030219 SZU
	       * SMH reconciliation
	       */
#ifdef DEBUG
              printf("Scanning %d\n",scan_op->lcode_op->id);
#endif
              /* Ignore already tested ops */
              if (scan_op->flags & SM_OP_TESTED)
		{
		  /* 20030219 SZU
		   * SMH reconciliation
		   */
#ifdef DEBUG
		  printf("Continuing. Already tested (op %d).\n",
			 scan_op->lcode_op->id); 
#endif
		  continue;
		}

              /* Detect operations that cannot go in this cycle 
               * due to dependence constraints
               */
	      /* 20030219 SZU
	       * SMH reconciliation
	       */
              if (scan_op->nosoft_cycle_lower_bound > issue_time)
                {
                  /* Mark so can be ignored quicker next time */
                  scan_op->flags |= SM_OP_TESTED;
#ifdef DEBUG
		  printf("Continuing. scan_op->nosoft_cycle_lower_bound (%d) "
			 "> issue_time (%d) (op %d).\n",
			 scan_op->nosoft_cycle_lower_bound, issue_time,
			 scan_op->lcode_op->id); 
#endif
                  continue;
                }

              if (!SM_fix_soft_dep (scan_op, issue_time, SOFTFIX_POSSIBLE))
                {
                  /* Mark so can be ignored quicker next time */
                  scan_op->flags |= SM_OP_TESTED;
#ifdef DEBUG
		  printf("Continuing. Cannot fix soft dep (op %d).\n",
			 scan_op->lcode_op->id); 
#endif
                  continue;
                }
#if 0
              if (scan_op->cycle_lower_bound > issue_time)
                {
                  /* Mark so can be ignored quicker next time */
                  scan_op->flags |= SM_OP_TESTED;
                  continue;
                }
#endif

              /* Get the highest priority untested op.
               * For ties, then use op with most deps out (to match
               * dhasy_sched default configuration.)
               * If still tied, used earliest op in serial order.
               */
              if ((sched_op == NULL) ||
                  (sched_op->priority < scan_op->priority) ||
                  ((sched_op->priority == scan_op->priority) &&
                   (sched_op->num_hard_dep_out <
                    scan_op->num_hard_dep_out)) ||
                  ((sched_op->priority == scan_op->priority) &&
                   (sched_op->num_hard_dep_out ==
                    scan_op->num_hard_dep_out) &&
                   (sched_op->serial_number > scan_op->serial_number)))
                {
                  sched_op = scan_op;
                }
            }

          /* If have tested all operations, goto next cycle */
          if (sched_op == NULL)
            break;

          /* If cannot schedule operation, mark it as tested */
          if (!SM_schedule_oper (sched_op, issue_time, 0, SM_MAX_SLOT, 0))
            {
	      /* 20030219 SZU
	       * SMH reconciliation
	       */
#ifdef DEBUG
	      printf("Continuing. SM scheduling failed (op %d).\n",
		     scan_op->lcode_op->id); 
#endif
              sched_op->flags |= SM_OP_TESTED;
            }
	  /* 20030219 SZU
	   * SMH reconciliation
	   */
	  else
	    {
#ifdef DEBUG
              printf("Scheduled %d\n",sched_op->lcode_op->id);
#endif
              reset_time = 0;
            }
        }
    }

  /* At this point, the number of unscheduled operations must be zero,
   * unless they are ignored.
   */
  if (sm_cb->num_unsched != 0)
    {
      fprintf (stderr, "SM_schedule_cb unscheduled and unignored ops:\n");
      for (sm_op = sm_cb->first_serial_op; sm_op != NULL;
           sm_op = sm_op->next_serial_op)
        {
          if (!(sm_op->flags & SM_OP_SCHEDULED) && !(sm_op->ignore))
            {
              SM_print_oper_dependences (stderr, sm_op);
#ifdef LP64_ARCHITECTURE
              fprintf (stderr,
                       "  unresolved_hard_deps = %i unresolved_soft_deps = %i "
                       "resolved_qentry = %lx\n",
                       sm_op->num_unresolved_hard_dep_in,
                       sm_op->num_unresolved_soft_dep_in,
                       (unsigned long) sm_op->dep_in_resolved_qentry);
#else
              fprintf (stderr,
                       "  unresolved_hard_deps = %i unresolved_soft_deps = %i "
                       "resolved_qentry = %x\n",
                       sm_op->num_unresolved_hard_dep_in,
                       sm_op->num_unresolved_soft_dep_in,
                       (unsigned int) sm_op->dep_in_resolved_qentry);
#endif
            }
        }
      L_punt ("SM_schedule_cb: %s cb %i, %i of %i ops unscheduled!",
              sm_cb->lcode_fn->name, sm_cb->lcode_cb->id,
              sm_cb->num_unsched, sm_cb->op_count);
    }
}

static int
SM_min_late_time (SM_Oper *sm_op)
{
  int num_exits, i, min_late;

  num_exits = sm_op->sm_cb->num_exits;

  min_late = SM_MAX_CYCLE;

  for (i = 0; i < num_exits; i++)
    {
      if (sm_op->late_time[i] < min_late)
	min_late = sm_op->late_time[i];
    }
  return min_late;
}

void
SM_reduce_liveranges_cb (SM_Cb * sm_cb)
{
  SM_Oper *sm_op;

  /* If cb is empty, return now */
  if (sm_cb->last_sched_op == NULL)
    return;

  for (sm_op = sm_cb->last_sched_op; sm_op; sm_op = sm_op->prev_sched_op)
    {
      int issue_time;
      int orig = sm_op->sched_cycle;
      int late = SM_min_late_time(sm_op);

      if (sm_op->liverange_reduced)
	continue;

      if (orig < late)
	{
	  SM_unschedule_oper (sm_op, NULL);

	  for (issue_time = late; issue_time >= orig; issue_time--)
	    {
	      if (SM_schedule_oper (sm_op, issue_time, 0, SM_MAX_SLOT, 0))
		break;
	    }

	  if (issue_time < orig)
	    L_punt("reschedule failed!");
	}

      sm_op->liverange_reduced = 1;
    }

  return;
}

/*
 * Does an efficient unscheduling of the cb by reinitializing key fields
 * back to there unscheduled state.  This is much more efficient that
 * just unscheduling each operation individually if most or all of
 * the operations are scheduled.  
 *
 * This routine is optimized for most or all the operations being scheduled
 * but will work in all cases.
 */
void
SM_unschedule_cb (SM_Cb * sm_cb)
{
  SM_Oper *sm_op, *to_op;
  SM_Reg_Action *op_action, *to_action;
  SM_Dep *dep_out;
  short min_delay, max_delay, delay_offset;
  short min_late_use_time, max_late_use_time;
  int start_index, end_index, index;
  unsigned int *map_array;
  /* SZU */
  SM_Issue_Group *issue_group_ptr, *next_issue_group;
  SM_Dep_PCLat *dep_pclat;
  int from_penalty, to_penalty;
  int max_from_penalty, min_from_penalty, max_to_penalty, min_to_penalty;

  /* Return early if no operations scheduled */
  if (sm_cb->first_sched_op == NULL)
    return;

  /* Initialize all operations to be "unscheduled".
   * Need to initialized all operations (verses just scheduled ones)
   * do that the bounds, unresolved dep counts, etc get reset.
   */
  for (sm_op = sm_cb->first_serial_op; sm_op != NULL;
       sm_op = sm_op->next_serial_op)
    {
      /* Reinitialize the sm_op to its unscheduled state */

      sm_op->cycle_lower_bound = SM_MIN_CYCLE;
      sm_op->cycle_upper_bound = SM_MAX_CYCLE;
      /* 20030219 SZU
       * SMH reconciliation
       */
      sm_op->nosoft_cycle_lower_bound = SM_MIN_CYCLE;
      sm_op->nosoft_cycle_upper_bound = SM_MAX_CYCLE;
      sm_op->sched_cycle = SM_MAX_CYCLE;

      sm_op->slot_lower_bound = 0;
      sm_op->slot_upper_bound = SM_MAX_SLOT;
      /* 20030219 SZU
       * SMH reconciliation
       */
      sm_op->nosoft_slot_lower_bound = 0;
      sm_op->nosoft_slot_upper_bound = SM_MAX_SLOT;
      sm_op->sched_slot = SM_MAX_SLOT;

      sm_op->dep_lower_bound = NULL;
      sm_op->dep_upper_bound = NULL;
      /* 20030219 SZU
       * SMH reconciliation
       */
      sm_op->nosoft_dep_lower_bound = NULL;
      sm_op->nosoft_dep_upper_bound = NULL;

      sm_op->alt_chosen = NULL;

      sm_op->num_unresolved_hard_dep_in = sm_op->num_hard_dep_in;
      sm_op->num_unresolved_soft_dep_in = sm_op->num_soft_dep_in;
      sm_op->num_unresolved_ignore_dep_in = sm_op->num_ignore_dep_in;

      sm_op->num_unresolved_hard_dep_out = sm_op->num_hard_dep_out;
      sm_op->num_unresolved_soft_dep_out = sm_op->num_soft_dep_out;
      sm_op->num_unresolved_ignore_dep_out = sm_op->num_ignore_dep_out;

      /* If have no incoming hard or soft deps, place in resolved queue */
      if ((sm_op->num_unresolved_hard_dep_in == 0) &&
          /* EMN (sm_op->num_unresolved_soft_dep_in == 0) && */
          (!sm_op->ignore))
        {
          if (sm_op->dep_in_resolved_qentry == NULL)
            {
              sm_op->dep_in_resolved_qentry =
                SM_enqueue_oper_before (sm_cb->dep_in_resolved, sm_op, NULL);
            }
        }

      /* Otherwise, remove from resolved queue, if necessary */
      else
        {
          if (sm_op->dep_in_resolved_qentry != NULL)
            {
              SM_dequeue_oper (sm_op->dep_in_resolved_qentry);
              sm_op->dep_in_resolved_qentry = NULL;
            }
        }

      /* Reset any scheduling related flags */
      sm_op->flags &= ~(SM_OP_SCHEDULED | SM_OP_SPECULATIVE | SM_OP_SILENT);

      /* Reset sm_op list pointers */
      sm_op->next_sched_op = NULL;
      sm_op->prev_sched_op = NULL;
    }

  /* Reset the cb state */
  sm_cb->first_sched_op = NULL;
  sm_cb->last_sched_op = NULL;
  sm_cb->num_unsched = sm_cb->op_count - sm_cb->num_ignored;

  /* 20021210 SZU
   * Modified for Itanium dispersal 
   */
  for (issue_group_ptr = sm_cb->first_issue_group; issue_group_ptr != NULL;
       issue_group_ptr = next_issue_group)
    {
      next_issue_group = issue_group_ptr->next_issue_group;

      free (issue_group_ptr->slots);
      L_free (SM_Issue_Group_pool, issue_group_ptr);
    }
  sm_cb->first_issue_group = NULL;
  sm_cb->last_issue_group = NULL;

  /* Clear resource map, if exists */
  if ((map_array = sm_cb->map_array) != NULL)
    {
      /* Calculate bounds of array that need initializing */
      start_index = sm_cb->min_init_offset - sm_cb->map_start_offset;
      end_index = sm_cb->max_init_offset - sm_cb->map_start_offset;

      /* Do 4 copies per iteration to speed up initialization.  The
       * algorithm was set up so there is always a multiple of 4 elements
       * that have been initialized!
       */
      for (index = start_index; index <= end_index; index += 4)
        {
          map_array[index] = 0;
          map_array[index + 1] = 0;
          map_array[index + 2] = 0;
          map_array[index + 3] = 0;
        }
    }

  /* Fix up all the variable delay depencences in the cb.
   * Only need to do all the outgoing deps to get all of them.
   */
  for (sm_op = sm_cb->first_serial_op; sm_op != NULL;
       sm_op = sm_op->next_serial_op)
    {
      /* Process each register action for this sm_op */
      for (op_action = sm_op->first_op_action; op_action != NULL;
           op_action = op_action->next_op_action)
        {
          /* If op action had a range of late use times, update the 
           * mdes-based dependences out of this action.
           */
          if (op_action->min_late_use_time != op_action->max_late_use_time)
            {
              /* Get the min and max late use time for this action */
              min_late_use_time = op_action->min_late_use_time;
              max_late_use_time = op_action->max_late_use_time;

              /* Update the mdes-based dep outs with the original range
               * of late use times
               */
              for (dep_out = op_action->first_dep_out; dep_out != NULL;
                   dep_out = dep_out->next_dep_out)
                {
                  /* Only process mdes-based dep_outs */
                  if (!(dep_out->flags & SM_MDES_BASED_DELAY))
                    continue;

                  /* Get the action and operation this dependence is to */
                  to_action = dep_out->to_action;
                  to_op = to_action->sm_op;

                  /* Get the dep_out's delay offset for ease of use */
                  delay_offset = dep_out->delay_offset;

                  /* By definition, to_op is unscheduled, so recalculate
                   * the original delay bounds.
                   */

                  /* Min delay is earliest def time - latest use time */
                  min_delay = delay_offset + min_late_use_time -
                    to_action->max_early_use_time;

                  /* Max delay is latest def time - earliest use time */
                  max_delay = delay_offset + max_late_use_time -
                    to_action->min_early_use_time;

		  /* 20020808 SZU
		   * Check if the producer & consumer fit
		   * any of the special producer_consumer latencies.
		   */
		  max_from_penalty = 0;
		  min_from_penalty = 0;
		  max_to_penalty = 0;
		  min_to_penalty = 0;

		  if (op_action->sm_op->sm_cb->sm_mdes->pclat_array != NULL)
		    {
		      SM_check_prod_cons_lat_match (dep_out, op_action,
						    to_action);

		      if (dep_out->pclat_list != NULL)
			{
			  dep_pclat = dep_out->pclat_list;

			  max_from_penalty = dep_pclat->from_penalty;
			  min_from_penalty = dep_pclat->from_penalty;
			  max_to_penalty = dep_pclat->to_penalty;
			  min_to_penalty = dep_pclat->to_penalty;

			  for (dep_pclat = dep_pclat->next_dep_pclat;
			       dep_pclat != NULL;
			       dep_pclat = dep_pclat->next_dep_pclat)
			    {
			      from_penalty = dep_pclat->from_penalty;
			      to_penalty = dep_pclat->to_penalty;

			      if (from_penalty < min_from_penalty)
				min_from_penalty = from_penalty;
			      if (from_penalty > max_from_penalty)
				max_from_penalty = from_penalty;

			      if (to_penalty < min_to_penalty)
				min_to_penalty = to_penalty;
			      if (to_penalty > max_to_penalty)
				max_to_penalty = to_penalty;
			    }
#if DEBUG_PCLAT
			  printf ("PCLat found\n");
			  printf ("Producer:\n");
			  SM_print_oper (stdout, op_action->sm_op);
			  printf ("Consumer:\n");
			  SM_print_oper (stdout, to_action->sm_op);
			  printf ("max_from_penalty: %i\n", max_from_penalty);
			  printf ("min_from_penalty: %i\n", min_from_penalty);
			  printf ("max_to_penalty: %i\n", max_to_penalty);
			  printf ("min_to_penalty: %i\n", min_to_penalty);
#endif
			}
		    }

		  /* This prod_cons have penalty latency.
		   * And it applies to this set of actions (operands)
		   */
		  if (max_from_penalty && max_to_penalty)
		    {
		      min_delay += min_from_penalty;
		      max_delay += max_from_penalty;
		    }

                  /* Superscalar scheduler, to not allow negative deps */
                  if (min_delay < 0)
                    min_delay = 0;
                  if (max_delay < 0)
                    max_delay = 0;

                  /* Update the dep_out's delays */
                  dep_out->min_delay = min_delay;
                  dep_out->max_delay = max_delay;

                  /* Set or reset SM_VARIABLE_DELAY flag as necessary */
                  if (min_delay != max_delay)
                    dep_out->flags |= SM_VARIABLE_DELAY;
                  else
                    dep_out->flags &= ~SM_VARIABLE_DELAY;
                }
            }
        }
    }
}


/*
 * Performs DHASY scheduling on Lcode function.  
 * (Side effect: run LIVEOUT dataflow analysis)
 *
 * If prepass_sched set to 1, prepass scheduling done, otherwise
 * postpass scheduling done.  
 * 
 * Note: Prepass scheduling requires register allocation *not* be done!
 *       Postpass scheduling requires register allocation *to* be done!  
 *       Otherwise, subtle errors can occur due to the different mapping
 *       used by virtual and actual registers!
 *
 * New wrapper for Limpact port. -JCG 6/99
 */
void
SM_schedule_fn (L_Func * fn, Mdes * version1_mdes, int prepass_sched)
{
  SM_Cb *sm_cb;
  L_Cb *cb;
  int cb_flags;

  /* Flag the scheduling mode to use -JCG 8/99 */
  cb_flags = SM_DHASY;

  /* 20030219 SZU
   * SMH reconciliation
   */
#if 0
  if (L_do_recovery_code && prepass_sched)
    {
      RC_dump_lcode (fn, ".chk_pre");
    }
#endif

  /* Flag if doing prepass (virtual registers) or 
   * postpass (physical registers)scheduling -JCG 8/99
   */
  cb_flags |= (prepass_sched ? SM_PREPASS : SM_POSTPASS);

  /* Flow analysis for scheduling:
   * Dependence drawing requires LIVE_VARIABLE
   * L_independent_memory_ops requires DOMINATOR_CB
   */

  L_partial_dead_code_removal (L_fn);
  L_do_flow_analysis (fn, DOMINATOR_CB | LIVE_VARIABLE | SUPPRESS_PG);

  /* Go through each cb and prepass schedule it */
  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      /* To aid debugging the scheduler and/or the dependence graph,
       * allow the cbs scheduled to be limited by putting a bound
       * on the cb id's that allowed to be scheduled. -JCG 6/99
       */
      if ((SM_debug_use_sched_cb_bounds) &&
          ((cb->id > SM_debug_upper_sched_cb_bound) ||
           (cb->id < SM_debug_lower_sched_cb_bound)))
        continue;

      /* MCM Do no reschedule software pipelined kernels. */

      /* 20030219 SZU
       * SMH reconciliation
       */
      if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_SOFTPIPE) &&
	  L_find_attr (cb->attr, "kernel"))
        continue;
#if 0
      if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_SOFTPIPE))
        continue;
#endif

      /* Create a sm_cb for this cb.  This creates the rinfo table
       * and builds all the dependences for this cb.
       */
      sm_cb = SM_new_cb (version1_mdes, cb, cb_flags);

      /* 20030219 SZU
       * SMH reconciliation
       */
      if (prepass_sched && SM_perform_relocate_cond_opti)
	SM_do_relocate_cond_opti (sm_cb); /* This does _not_ schedule cb */

      /* 20031014 SZU
       * JWS says don't do renaming due to how dependences are drawn in SM
       */
      /* Schedule the cb using DHASY list scheduling */
#if 0
      if (prepass_sched && SM_perform_rename_with_copy)
	SM_do_classic_renaming_with_copy (sm_cb); /* This does schedule cb */
      else
	SM_schedule_cb (sm_cb);
#endif
      SM_schedule_cb (sm_cb);

#if 0
      if (prepass_sched)
	SM_reduce_liveranges_cb (sm_cb);
#endif

      /* Commit the schedule to the lcode cb */
      SM_commit_cb (sm_cb);

      /* Delete the sm_cb, done with it */
      SM_delete_cb (sm_cb);
    }
  /* 20030219 SZU
   * SMH reconciliation
   */
#if 0
  if (L_do_recovery_code && prepass_sched)
    {
      RC_dump_lcode (fn, ".chk_post");
    }
#endif
}

/* 20040712SZU */
/** \fn SM_bundle_fn (L_Func * fn, Mdes * version1_mdes)
 * \brief takes a function and schedules it while retaining sequential
 * order
 *
 * \param fn the function
 * \param version1_mdes mdes (machine description) file
 *
 * "Schedules" a function.
 * Used to generate assembly code while retaining the Lcode sequential
 * order. No optimizations done. No modulo scheduling.
 * Create with IA64 in mind, to be run instead of postpass scheduling.
 */
void
SM_bundle_fn (L_Func * fn, Mdes * version1_mdes)
{
  int cb_flags;
  L_Cb *cb;
  SM_Cb *sm_cb;

  /* Set scheduling to SM_SEQUENTIAL.
   */
  cb_flags = SM_POSTPASS | SM_SEQUENTIAL;

  /* Flow analysis for scheduling:
   * Dependence drawing requires LIVE_VARIABLE
   * L_independent_memory_ops requires DOMINATOR_CB
   */

  L_partial_dead_code_removal (L_fn);
  L_do_flow_analysis (fn, DOMINATOR_CB | LIVE_VARIABLE | SUPPRESS_PG);

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      /* To aid debugging the scheduler and/or the dependence graph,
       * allow the cbs scheduled to be limited by putting a bound
       * on the cb id's that allowed to be scheduled. -JCG 6/99
       */
      if ((SM_debug_use_sched_cb_bounds) &&
          ((cb->id > SM_debug_upper_sched_cb_bound) ||
           (cb->id < SM_debug_lower_sched_cb_bound)))
        continue;

      /* MCM Do no reschedule software pipelined kernels. */
      if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_SOFTPIPE) &&
	  L_find_attr (cb->attr, "kernel"))
        continue;

      /* Create a sm_cb for this cb.  This creates the rinfo table
       * and builds all the dependences for this cb.
       * Since sequential order is to be retained, add SYNC_DEP
       * dependences between ops.
       */
      sm_cb = SM_new_cb (version1_mdes, cb, cb_flags);

      SM_schedule_cb (sm_cb);

      /* Commit the schedule to the lcode cb */
      SM_commit_cb (sm_cb);

      /* Delete the sm_cb, done with it */
      SM_delete_cb (sm_cb);
    }
}

static void
L_read_parm_SM (Parm_Parse_Info * ppi)
{
  /* 20031023 SZU
   * New parameter to indicate IPF template bundling and bundle compaction.
   */
  L_read_parm_b (ppi, "do_template_bundling",
                 &SM_do_template_bundling);
  L_read_parm_b (ppi, "do_bundle_compaction",
                 &SM_do_bundle_compaction);

  L_read_parm_b (ppi, "debug_use_sched_cb_bounds",
                 &SM_debug_use_sched_cb_bounds);
  L_read_parm_i (ppi, "debug_lower_sched_cb_bound",
                 &SM_debug_lower_sched_cb_bound);
  L_read_parm_i (ppi, "debug_upper_sched_cb_bound",
                 &SM_debug_upper_sched_cb_bound);
  L_read_parm_b (ppi, "print_dependence_graph", &SM_print_dependence_graph);
  L_read_parm_b (ppi, "check_dependence_symmetry",
                 &SM_check_dependence_symmetry);
  L_read_parm_b (ppi, "verify_reg_conflicts", &SM_verify_reg_conflicts);
  L_read_parm_i (ppi, "output_dep_distance", &SM_output_dep_distance);

  L_read_parm_b (ppi, "?ignore_pred_analysis", &SM_ignore_pred_analysis);
  L_read_parm_b (ppi, "perform_rename_with_copy", 
		 &SM_perform_rename_with_copy);
  L_read_parm_b (ppi, "perform_relocate_cond_opti", 
		 &SM_perform_relocate_cond_opti);

  L_read_parm_b (ppi, "?sched_slack_loads_for_miss",
		 &SM_sched_slack_loads_for_miss);
  /* 20030311 SZU
   * ifdef on RC as well.
   */
#if defined(RC_CODE)
  L_read_parm_RC (ppi);
#endif

  /* 20030907 SZU
   * In JWS checked in version of SM.
   */
  {
    int idummy;
    char *sdummy;
    L_read_parm_b (ppi, "?use_hamm", &idummy);
    L_read_parm_s (ppi, "?kapi_knobs_file", &sdummy);
    L_read_parm_i (ppi, "?compaction_scheme", &idummy);
    L_read_parm_b (ppi, "?verify_reg_conflicts", &idummy);
    L_read_parm_b (ppi, "?rc_print_debug_info", &idummy);
  }
}

void
SM_init (Parm_Macro_List * command_line_macro_list)
{
  static int SM_initialized = 0;

  /* Read in parameters once. */
  if (!SM_initialized)
    {
      L_load_parameters (L_parm_file, command_line_macro_list,
                         "(SM", L_read_parm_SM);

      SM_initialized = 1;
    }
}
