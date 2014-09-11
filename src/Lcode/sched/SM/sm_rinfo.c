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
 *      File:   sm_rinfo.c
 *      Author: John C. Gyllenhaal, Wen-mei Hwu
 *      Creation Date:  July 1996
\*****************************************************************************/

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sm.h"
#include <Lcode/l_main.h>
#include <library/l_alloc_new.h>
#include <library/heap.h>

/* Define the number of typical conflicts that occur with
 * a register action (other than self).  Want this to be as small
 * as possible but still capture most of the conflicts.  Always
 * make a multiple of 2 for efficiency.
 */
#define TYPICAL_NUM_CONFLICTS 2

L_Alloc_Pool *SM_Reg_Action_pool = NULL;
L_Alloc_Pool *SM_Reg_Info_pool = NULL;
L_Alloc_Pool *SM_Reg_Action_Conflict_pool = NULL;

/* Externs of other pools used to rinfo table */
extern L_Alloc_Pool *SM_Dep_pool;

/* Internal prototypes */
extern void SM_update_implicit_branch_actions (SM_Cb * sm_cb,
                                               SM_Reg_Info * new_rinfo);


static int
SM_type_from_operand (L_Operand * operand)
{
  int reg_type = 0;

  /* Get register id (based on operand type) */
  if (L_is_reg (operand))
    reg_type = SM_REGISTER_TYPE;
  else if (L_is_macro (operand))
    reg_type = SM_MACRO_TYPE;
  else if (L_is_reserved (operand))
    reg_type = SM_EXT_ACTION_TYPE;
  else
    L_punt ("SM_reg5_from_operand: Unexpected operand type.");

  if (L_is_ctype_dbl (operand))
    {
      reg_type |= SM_CTYPE_DOUBLE;
    }
  return reg_type;
}

static int
SM_id_from_operand (L_Operand * operand)
{
  int reg_id = 0;

  /* Get register id (based on operand type) */
  if (L_is_reg (operand))
    reg_id = operand->value.r;
  else if (L_is_macro (operand))
    reg_id = operand->value.mac;
  else if (L_is_reserved (operand))
    reg_id = operand->value.r;
  else
    L_punt ("SM_id_from_operand: Unexpected operand type.");

  return reg_id;
}


/* Doubles the rinfo hash array size */
void
SM_resize_rinfo_hash_array (SM_Cb * sm_cb)
{
  SM_Reg_Info **new_hash, *rinfo, *hash_head;
  unsigned int new_hash_size;
  unsigned int new_hash_mask, new_hash_index;
  int i;

  /* Double the size of the hash array */
  new_hash_size = sm_cb->rinfo_hash_size << 1;

  /* Allocate new hash array */
  new_hash = (SM_Reg_Info **) malloc (new_hash_size * sizeof (SM_Reg_Info *));
  if (new_hash == NULL)
    {
      L_punt ("SM_resize_rinfo_hash_array: Out of memory, new size %i.",
              new_hash_size);
    }

  /* Initialize new hash table */
  for (i = 0; i < new_hash_size; i++)
    new_hash[i] = NULL;

  /* Get the hash mask for the new hash table */
  /* AND mask, works only for power of 2 */
  new_hash_mask = new_hash_size - 1;    

  /* Go through all the existing reg_infos and add to the new hash table.
   * Can totally disreguard old hash links.
   */
  for (rinfo = sm_cb->first_rinfo; rinfo != NULL; rinfo = rinfo->next_rinfo)
    {
      /* Get the index into the new hash table for this reg info */
      new_hash_index = rinfo->id & new_hash_mask;

      /* Add reg info to head of rinfo hash's linked list */
      hash_head = new_hash[new_hash_index];
      rinfo->next_hash = hash_head;
      rinfo->prev_hash = NULL;
      if (hash_head != NULL)
        hash_head->prev_hash = rinfo;
      new_hash[new_hash_index] = rinfo;
    }

  /* Free the old hash table */
  free (sm_cb->rinfo_hash);

  /* Initialize sm_cb fields for new hash table */
  sm_cb->rinfo_hash = new_hash;
  sm_cb->rinfo_hash_size = new_hash_size;
  sm_cb->rinfo_hash_mask = new_hash_mask;
  /* Resize when count at 75% of new_hash_size */
  sm_cb->rinfo_resize_size = new_hash_size - (new_hash_size >> 2);
}

/* Add reg info structure to sm_cb for this reg_operand.
 * Dynamically increases hash array when necessary.
 * Returns pointer to new rinfo.
 */

#define MAX_CONFLICTS 129

SM_Reg_Info *
SM_add_reg_info (SM_Cb * sm_cb, L_Operand * reg_operand)
{
  SM_Reg_Info *rinfo, *hash_head, *check_rinfo, *tail_rinfo;
  SM_Reg_Info **conflict_array = NULL,
    *conflict_rinfo = NULL, **conflict_ptr = NULL;
  L_Operand *conflicts[MAX_CONFLICTS];
  int num_conflicts;
  unsigned int hash_index;
  int rinfo_count;
  int i, self_conflict_found;

  /* Increase hash table size if necessary before adding new rinfo.
   * This will change the rinfo_hash_mask if the table is resized!
   */
  rinfo_count = sm_cb->rinfo_count;
  if (rinfo_count >= sm_cb->rinfo_resize_size)
    SM_resize_rinfo_hash_array (sm_cb);

  /* Allocate a new reg_info structure (pool initialized in SM_new_cb) */
  rinfo = (SM_Reg_Info *) L_alloc (SM_Reg_Info_pool);

  /* Initialize rinfo identifier fields */
  rinfo->id = SM_id_from_operand (reg_operand);
  rinfo->type = SM_type_from_operand (reg_operand);
  rinfo->sm_cb = sm_cb;

  /* Initialize the lcode operand structure for lcode queries */
  rinfo->operand = L_copy_operand (reg_operand);

  /* Initialize rinfo flags */
  rinfo->flags = 0;

  /* Set SM_FRAGILE_MACRO if this operand is a fragile macro */
  if ((rinfo->type & SM_MACRO_TYPE) && L_is_fragile_macro (reg_operand))
    {
      rinfo->flags |= SM_FRAGILE_MACRO;
    }

  /* Initialize register action list fields */
  rinfo->first_complete = NULL;
  rinfo->last_complete = NULL;
  rinfo->first_actual = NULL;
  rinfo->last_actual = NULL;
  rinfo->first_def = NULL;
  rinfo->last_def = NULL;

  /* Get tail rinfo for ease of use */
  tail_rinfo = sm_cb->last_rinfo;

  /* Add rinfo to tail of sm_cb's linked list of reg infos */
  rinfo->next_rinfo = NULL;
  rinfo->prev_rinfo = tail_rinfo;

  if (tail_rinfo == NULL)
    sm_cb->first_rinfo = rinfo;
  else
    tail_rinfo->next_rinfo = rinfo;

  sm_cb->last_rinfo = rinfo;

  /* Get index into hash table to use for this value */
  hash_index = rinfo->id & sm_cb->rinfo_hash_mask;

  /* Get head symbol in currently linked list for ease of use */
  hash_head = sm_cb->rinfo_hash[hash_index];

  /* Sanity check (may want to ifdef out later).

   * Check that this rinfo is not already in the symbol table.
   * Punt if it is, since this can cause a major debugging nightmare.
   */
  for (check_rinfo = hash_head; check_rinfo != NULL;
       check_rinfo = check_rinfo->next_hash)
    {
      /* If already in table, punt */
      if (L_same_operand (check_rinfo->operand, rinfo->operand))
        {
          L_punt ("SM_Cb for cb %i: reg_info (%i, %i) already in table!",
                  sm_cb->lcode_cb->id, rinfo->id, rinfo->type);
        }
    }

  /* Add rinfo to head of linked list */
  rinfo->next_hash = hash_head;
  rinfo->prev_hash = NULL;
  if (hash_head != NULL)
    hash_head->prev_hash = rinfo;
  sm_cb->rinfo_hash[hash_index] = rinfo;

  /* Update sm_cb's rinfo count */
  sm_cb->rinfo_count++;


  /* Now that rinfo added, build reg conflict array for rinfo.
   * This may cause other rinfos to be built (which then will probably
   * look up this rinfo) so this MUST be done last to prevent infinite loop!
   *
   * Fake the call for the special mem, ctrl, sync, and vliw operands
   * I have added.
   */
  if (!(rinfo->type & SM_EXT_ACTION_TYPE))
    {
      /* 20030227 SZU
       * SMH reconciliation
       * SMH has 64 hard coded. MAX_CONFLICT defined as 129
       */
#if 0
      num_conflicts = sm_cb->conflicting_operands (reg_operand, conflicts,
                                                   64, sm_cb->prepass_sched);
#else
      num_conflicts = sm_cb->conflicting_operands (reg_operand, conflicts,
                                                   MAX_CONFLICTS,
						   sm_cb->prepass_sched);
#endif
    }
  else
    {
      num_conflicts = 1;
      conflicts[0] = L_copy_operand (reg_operand);
    }

  /* Make sure if there is only one conflict, that it conflicts with 
   * itself!
   */
  if (num_conflicts == 1)
    {
      /* Is this not a self conflict? */
      if (SM_verify_reg_conflicts &&
          !L_same_operand (conflicts[0], reg_operand))
        {
          fprintf (stderr, "Func %s cb %i:\n", sm_cb->lcode_fn->name,
                   sm_cb->lcode_cb->id);
          fprintf (stderr, "For operand: ");
          L_print_operand (stderr, reg_operand, 1);
          fprintf (stderr, "\nConflict array:\n");
          for (i = 0; i < num_conflicts; i++)
            {
              fprintf (stderr, " %i: ", i);
              L_print_operand (stderr, conflicts[i], 1);
              fprintf (stderr, "\n");
            }
          fprintf (stderr, "\n");
          L_punt ("SM_add_reg_info: Register should conflict with self!");
        }

      rinfo->reg_conflict = NULL;
      rinfo->num_conflicts = 0;
    }
  /* If conflicts with registers other than self, build conflict array.
   * Otherwise make NULL.
   */
  else if (num_conflicts != 0)
    {
      conflict_array = (SM_Reg_Info **) malloc (num_conflicts *
                                                sizeof (SM_Reg_Info *));
      if (conflict_array == NULL)
        L_punt ("SM_add_reg_info: Out of memory");

      /* Use pointer to assign elements */
      conflict_ptr = &conflict_array[0];

      /* Have not found self conflict yet (may not be first) */
      self_conflict_found = 0;

      /* Point conflict array at reg info structure for the conflicting
       * operand.
       */
      for (i = 0; i < num_conflicts; i++)
        {
          /* Is this the self conflict? */
          if (L_same_operand (conflicts[i], reg_operand))
            {
              /* Yes, better not be two of them */
              if (SM_verify_reg_conflicts && self_conflict_found == 1)
                {
                  fprintf (stderr, "Func %s cb %i:\n",
                           sm_cb->lcode_fn->name, sm_cb->lcode_cb->id);
                  fprintf (stderr, "For operand: ");
                  L_print_operand (stderr, reg_operand, 1);
                  fprintf (stderr, "\nConflict array:\n");
                  for (i = 0; i < num_conflicts; i++)
                    {
                      fprintf (stderr, " %i: ", i);
                      L_print_operand (stderr, conflicts[i], 1);
                      fprintf (stderr, "\n");
                    }
                  L_punt ("SM_add_reg_info: Two self conflicts!");
                }

              /* Mark that we found the self conflict */
              self_conflict_found = 1;

              /* Go to next conflict */
              continue;
            }

          /* If rinfo doesn't already exists for conflict, add it */
          conflict_rinfo = SM_find_reg_info (sm_cb, conflicts[i]);
          if (conflict_rinfo == NULL)
            conflict_rinfo = SM_add_reg_info (sm_cb, conflicts[i]);

          /* Add to conflict array and advance pointer */
          *conflict_ptr = conflict_rinfo;
          conflict_ptr++;
        }

      /* Better have found self conflict! */
      if (SM_verify_reg_conflicts && self_conflict_found != 1)
        {
          fprintf (stderr, "Func %s cb %i:\n",
                   sm_cb->lcode_fn->name, sm_cb->lcode_cb->id);
          fprintf (stderr, "For operand: ");
          L_print_operand (stderr, reg_operand, 1);
          fprintf (stderr, "\nConflict array:\n");
          for (i = 0; i < num_conflicts; i++)
            {
              fprintf (stderr, " %i: ", i);
              L_print_operand (stderr, conflicts[i], 1);
              fprintf (stderr, "\n");
            }
          L_punt ("SM_add_reg_info: Reg expected to conflict with itself!");
        }

      /* Null terminate array */
      if (conflict_ptr)
        *conflict_ptr = NULL;

      rinfo->reg_conflict = conflict_array;
      rinfo->num_conflicts = num_conflicts - 1;
    }
  else
    {
      rinfo->reg_conflict = NULL;
      rinfo->num_conflicts = 0;
    }

#if 0
  /* Debug, testing out conflict handling.  For all registers to have
   * a conflict with the register id generated by flipping bit 0.
   */
  if ((rinfo->num_conflicts == 0) && (rinfo->type & SM_REGISTER_TYPE))
    {
      L_Operand *conflict_operand;

      /* Set conflict_operand to be the same operand with id bit 0 flipped */
      conflict_operand = L_copy_operand (reg_operand);
      conflict_operand->value.r = conflict_operand->value.r ^ 1;

      conflict_array = (SM_Reg_Info **) malloc (2 * sizeof (SM_Reg_Info *));
      if (conflict_array == NULL)
        L_punt ("SM_add_reg_info: Out of memory");

      /* Find or add this operand to rinfo list */
      conflict_rinfo = SM_find_reg_info (sm_cb, &conflict_operand);
      if (conflict_rinfo == NULL)
        conflict_rinfo = SM_add_reg_info (sm_cb, &conflict_operand);

      /* Place in conflict array */
      conflict_array[0] = conflict_rinfo;
      conflict_array[1] = NULL;

      /* Set fields appropriately */
      rinfo->reg_conflict = conflict_array;
      rinfo->num_conflicts = 1;
    }
#endif

  /* Update the branch operations implicit operands to include
   * this new reg_operand (if necessary).
   *
   * Do not need to update for extended action types.
   */
  if (!(rinfo->type & SM_EXT_ACTION_TYPE))
    {
      SM_update_implicit_branch_actions (sm_cb, rinfo);
    }

  /* Delete all conflicts */
  for (i = 0; i < num_conflicts; i++)
    {
      L_delete_operand (conflicts[i]);
    }

  if (L_is_variable (reg_operand) &&
      (SM_use_fake_dataflow_info ||
       L_in_cb_IN_set (sm_cb->lcode_cb, reg_operand)))
    rinfo->flags |= SM_REG_LIVE_IN;

  rinfo->ext = NULL;

  /* Return rinfo added */
  return (rinfo);
}

/* Returns reg info for reg_operand or NULL if not found */
SM_Reg_Info *
SM_find_reg_info (SM_Cb * sm_cb, L_Operand * reg_operand)
{
  SM_Reg_Info *rinfo;
  int reg_id;
  unsigned int hash_index;

  reg_id = SM_id_from_operand (reg_operand);

  /* Get the index into the hash table based on register id */
  hash_index = reg_id & sm_cb->rinfo_hash_mask;

  /* Search the linked list for matching register */
  for (rinfo = sm_cb->rinfo_hash[hash_index]; rinfo != NULL;
       rinfo = rinfo->next_hash)
    {
      /* Compare all relavant fields to find match */
      if (L_same_operand (rinfo->operand, reg_operand))
        {
          return (rinfo);
        }
    }

  /* Not found, return NULL */
  return (NULL);
}


/* Places an existing action into the proper place in action list,
 * sort by sm_op's serial number and for the same serial number,
 * places uses before defs.
 * 
 * We want the explicit operands to take precedence over implicit operands
 * (in order to draw dependences properly if an operand is both explicit
 * and implicit).  So place explicit uses before implicit uses, and
 * place explicit defs after implicit defs.
 *
 * Tuned for inserting actions from the beginning of the cb to the end
 * (starts from the end of the action list) but works pretty well for
 * any order of insertion.
 * 
 * NOTE: Action must not already be in the action list!  
 *       Designed For use only by SM_insert_reg_action() and
 *       SM_reposition_reg_action()!
 */
void
SM_place_reg_action (SM_Reg_Action * action)
{
  SM_Reg_Action *after_this_action, *existing_action;
  SM_Reg_Info *rinfo;
  SM_Oper *sm_op;
  unsigned int flags;
  unsigned int serial_number, existing_serial_number;

  /* Get the action's flags, rinfo, and sm_op for ease of use */
  flags = action->flags;
  rinfo = action->rinfo;
  sm_op = action->sm_op;

  /* Get this operation's serial number */
  serial_number = sm_op->serial_number;

  /* Search from the end of rinfo's complete list to find the place
   * where we should place this action (sets after_this_action).
   * NULL indicates should place at beginning of list.
   *
   * Want to place in proper order according to sm_op's serial numbers.
   * For the same op, want to place all the uses before the defs.
   * (assumes for now all operations are either def or use, but not both).
   */
  for (existing_action = rinfo->last_complete; existing_action != NULL;
       existing_action = existing_action->prev_complete)
    {
      /* Get the serial number for this action */
      existing_serial_number = existing_action->sm_op->serial_number;

      /* Place after an operation with a smaller serial number */
      if (existing_serial_number < serial_number)
        break;

      if (existing_serial_number == serial_number)
        {
          /* If action is an explicit def, place at end of existing 
           * actions with same serial number.
           * 
           * If action in an implicit def, place before any explicit
           * def actions with the same serial number.
           */
          if (flags & SM_DEF_ACTION)
            {
              /* If action is explicit, place at end of existing actions */
              if (flags & SM_EXPLICIT_ACTION)
                {
                  break;
                }
              else
                {
                  /* If action is implicit, place after uses and
                   * any implicit defs.
                   */
                  if ((existing_action->flags & SM_USE_ACTION) ||
                      (existing_action->flags & SM_IMPLICIT_ACTION))
                    break;
                }
            }

          /* If new action is a explicit use, place after exising explicits
           * uses (but before implicit uses).
           */
          else
            {
              if (flags & SM_EXPLICIT_ACTION)
                {
                  /* If action is explicit, place after explicit uses */
                  if ((existing_action->flags & SM_USE_ACTION) &&
                      (existing_action->flags & SM_EXPLICIT_ACTION))
                    break;
                }
              else
                {
                  /* If action is implicit, place after any use */
                  if (existing_action->flags & SM_USE_ACTION)
                    break;
                }
            }
        }
    }

  /* Want to place new action after this action */
  after_this_action = existing_action;

  /* Place action in complete list after 'after_this_action' (if not NULL) */
  if (after_this_action != NULL)
    {
      /* Place action in complete list after 'after_this_action' */
      action->prev_complete = after_this_action;
      action->next_complete = after_this_action->next_complete;
      if (after_this_action->next_complete != NULL)
        after_this_action->next_complete->prev_complete = action;
      else
        rinfo->last_complete = action;
      after_this_action->next_complete = action;

      /* Update next_actual and prev_actual.  This update is 
       * independent of whether or not this action is an actual action.
       */
      action->next_actual = after_this_action->next_actual;

      /* Point to 'after_this_action' if an actual action */
      if ((after_this_action->flags & SM_ACTUAL_ACTION))
        action->prev_actual = after_this_action;

      /* Otherwise, will have same prev_actual as 'after_this_action' */
      else
        action->prev_actual = after_this_action->prev_actual;

      /* Update next_def and next_def.  This update is independent of
       * whether or not this action is an def action.
       */
      action->next_def = after_this_action->next_def;

      /* Point to 'after_this_action' if a actual def action */
      if ((after_this_action->flags & SM_DEF_ACTION) &&
          (after_this_action->flags & SM_ACTUAL_ACTION))
        action->prev_def = after_this_action;

      /* Otherwise, will have same prev_def as 'after_this_action' */
      else
        action->prev_def = after_this_action->prev_def;

      /* The code for updating the action and def lists are handled below! */
    }

  /* Otherwise, place action at beginning of rinfo complete action list */
  else
    {
      /* Place at beginning of rinfo complete list */
      action->next_complete = rinfo->first_complete;
      action->prev_complete = NULL;
      if (rinfo->first_complete != NULL)
        rinfo->first_complete->prev_complete = action;
      else
        rinfo->last_complete = action;
      rinfo->first_complete = action;

      /* Update next_actual and prev_actual */
      action->next_actual = rinfo->first_actual;
      action->prev_actual = NULL;

      /* Update next_def and prev_def */
      action->next_def = rinfo->first_def;
      action->prev_def = NULL;

      /* The code for updating the action and def lists are handled below! */
    }

  /* If an actual action,  place in actual list and update surrounding
   * pointers.
   */
  if (flags & SM_ACTUAL_ACTION)
    {
      /* Update the action linked list pointers */
      if (action->prev_actual != NULL)
        action->prev_actual->next_actual = action;
      else
        rinfo->first_actual = action;

      if (action->next_actual != NULL)
        action->next_actual->prev_actual = action;
      else
        rinfo->last_actual = action;

      /* Fix existing conflicting actions before this one.
       * Want them to point to this action with their next_actual pointer.
       * (Stop when hit an actual action.)
       */
      for (existing_action = action->prev_complete;
           ((existing_action != NULL) &&
            !(existing_action->flags & SM_ACTUAL_ACTION));
           existing_action = existing_action->prev_complete)
        {
          existing_action->next_actual = action;
        }

      /* Fix existing conflicting actions after this one.
       * Want them to point to this action with their prev_actual pointer.
       * (Stop when hit an actual action.)
       */
      for (existing_action = action->next_complete;
           ((existing_action != NULL) &&
            !(existing_action->flags & SM_ACTUAL_ACTION));
           existing_action = existing_action->next_complete)
        {
          existing_action->prev_actual = action;
        }


      /* If actual def, place in def list and update surrounding pointers */
      if (flags & SM_DEF_ACTION)
        {
          /* Update the def linked list pointer */
          if (action->prev_def != NULL)
            action->prev_def->next_def = action;
          else
            rinfo->first_def = action;

          if (action->next_def != NULL)
            action->next_def->prev_def = action;
          else
            rinfo->last_def = action;

          /* Fix all existing use actions (conflicting and actual use 
           * actions) and conflicting def actions before this one.
           * Want them to point to this def with their next_def pointer.
           * (Stop when hit an actual def action)
           */
          for (existing_action = action->prev_complete;
               ((existing_action != NULL) &&
                !((existing_action->flags & SM_ACTUAL_ACTION) &&
                  (existing_action->flags & SM_DEF_ACTION)));
               existing_action = existing_action->prev_complete)
            {
              existing_action->next_def = action;
            }

          /* Fix all existing use actions (conflicting and actual use 
           * actions) and conflicting def actions after this one.
           * Want them to point to this def with their prev_def pointer.
           * (Stop when hit an actual def action)
           */
          for (existing_action = action->next_complete;
               ((existing_action != NULL) &&
                !((existing_action->flags & SM_ACTUAL_ACTION) &&
                  (existing_action->flags & SM_DEF_ACTION)));
               existing_action = existing_action->next_complete)
            {
              existing_action->prev_def = action;
            }
        }
    }

}

/* Creates and initializes a new action, and places it into the
 * rinfo's action lists.  See SM_place_action() for details on how
 * actions are placed.
 *
 * Called by SM_add_reg_action, which handles the register conflict
 * aspects of register actions.
 *
 * Returns the action inserted.
 */
SM_Reg_Action *
SM_insert_reg_action (SM_Reg_Info * rinfo, SM_Oper * sm_op,
                      int operand_type, int operand_number,
                      L_Operand * reg_operand, unsigned int flags)
{
  SM_Reg_Action *action;
  SM_Compatible_Alt *compatible_alt;
  Mdes_Latency *latency;
  int reg_ptype;
  int index;
  int min_early, max_early, min_late, max_late;
  int early_use_time, late_use_time;


  /* Mark action as an implicit use, if placing an normal register action
   * in a sync location.  Otherwise, mark as an explicit use.
   * This works for our only implicit actions (live out of branches,
   * fragile macros for jsrs).  Will need to be modified if other
   * implicit actions are used in the future.
   */
  if ((!(rinfo->type & SM_EXT_ACTION_TYPE)) &&
      ((operand_type == MDES_SYNC_IN) || (operand_type == MDES_SYNC_OUT)))
    {
      /* Fragile macros implicitly defined by a JSR should be considered
       * transparent.  They should not kill other definitions (and
       * therefore cannot dominate or post-dominate another action).
       * -JCG 7/8/98
       */
      if (operand_type == MDES_SYNC_OUT)
        {
          flags |= SM_IMPLICIT_ACTION | SM_TRANSPARENT_ACTION;
        }

      /* Implicit uses are also not guarenteed to be actually used, 
       * but I don't see the benefit from marking also them as transparent.
       */
      else
        {
          flags |= SM_IMPLICIT_ACTION;
        }
    }
  else
    {
      flags |= SM_EXPLICIT_ACTION;
    }

  /* Create new reg action */
  action = (SM_Reg_Action *) L_alloc (SM_Reg_Action_pool);

  /* Initialize fields */
  action->sm_op = sm_op;
  action->operand_type = (unsigned char) operand_type;
  action->operand_number = (unsigned char) operand_number;
  index = operand_index (operand_type, operand_number);
  action->index = (unsigned short) index;

  /* This action starts out as not in any queues */
  action->first_queue = NULL;

  action->add_lat = 0;

  /* Initialize dependence management fields (only used by actual actions) */
  if (flags & SM_ACTUAL_ACTION)
    {
      /* Dependence lists are initially empty */
      action->first_dep_in = NULL;
      action->first_dep_out = NULL;

      /* Scan the compatable alts to get the min/max early use times */
      compatible_alt = sm_op->first_compatible_alt;

      /* initialize mins and maxs with first alt.
       * For now, get same early and late use times.
       * Also for now, the normal_version and the silent_version (if any) has
       * the same latencys, so don't need to check.
       */
      latency = compatible_alt->normal_version->latency;
      early_use_time = latency->operand_latency[index];
      late_use_time = latency->operand_latency[index];
      min_early = early_use_time;
      max_early = early_use_time;
      min_late = late_use_time;
      max_late = late_use_time;

      /* Scan the rest of alts.  Taking appropriate mins and maxs */
      for (compatible_alt = compatible_alt->next_compatible_alt;
           compatible_alt != NULL;
           compatible_alt = compatible_alt->next_compatible_alt)
        {
          /* For now, get same early and late use times.
           * Also for now, the normal_version and the silent_version 
           * (if any) has the same latencys, so don't need to check.
           */
          latency = compatible_alt->normal_version->latency;
          early_use_time = latency->operand_latency[index];
          late_use_time = latency->operand_latency[index];

          /* Update min/max early times */
          if (early_use_time < min_early)
            min_early = early_use_time;
          if (early_use_time > max_early)
            max_early = early_use_time;

          /* Update min/max late times */
          if (late_use_time < min_late)
            min_late = late_use_time;
          if (late_use_time < max_late)
            max_late = late_use_time;
        }

      /* Set the action's min/max times */
      action->min_early_use_time = min_early;
      action->max_early_use_time = max_early;

      action->min_late_use_time = min_late;
      action->max_late_use_time = max_late;

      if ((min_early != max_early) || (min_late != max_late))
        {
          /* Mark action flag in flags variable */
          flags |= SM_VARIABLE_ACTION;
          sm_op->flags |= SM_OP_VARIABLE_ACTIONS;
        }

      /* The actual times are not know until operaiton is scheduled.
       * Set to initial times that should cause large scheduling problems if
       * used before they are reinitialized.
       *
       * If the operation is already scheduled (can happen during
       * transformations, use actual use times.
       */
      if (sm_op->flags & SM_OP_SCHEDULED)
        {
          /* For now, get the same early and late use times */
          latency = sm_op->alt_chosen->normal_version->latency;
          action->actual_early_use_time = latency->operand_latency[index];
          action->actual_late_use_time = latency->operand_latency[index];
        }
      else
        {
          action->actual_early_use_time = -10000;
          action->actual_late_use_time = 10000;
        }

#if 0
      /* Debug, give every flow a 2 cycle latency */
      action->min_early_use_time = 0;
      action->max_early_use_time = 0;
      action->min_late_use_time = 2;
      action->max_late_use_time = 2;
#endif

    }

  /* Otherwise, this is a conflicting action and these fields will not be
   * used.  Initialize fields to values that should should make 
   * invalid uses obvious.
   */
  else
    {
      /* Initialize to bad values to cause bus error if try to use!  */
      action->first_dep_in = (SM_Dep *) - 1;
      action->first_dep_out = (SM_Dep *) - 1;

      /* Initialize use times to cause large dependence distances to
       * be drawn if used.
       */
      action->min_early_use_time = -10000;
      action->max_early_use_time = -10000;
      action->min_late_use_time = 10000;
      action->max_late_use_time = 10000;
      action->actual_early_use_time = -10000;
      action->actual_late_use_time = 10000;
    }



  /* Set def/use flags */
  if ((operand_type == MDES_DEST) || (operand_type == MDES_SYNC_OUT))
    {
      flags |= SM_DEF_ACTION;

      /* For predicate defs, set predicate def type */
      if (reg_operand->ctype == L_CTYPE_PREDICATE)
        {
          reg_ptype = reg_operand->ptype;

          if (L_is_update_predicate_ptype (reg_ptype))
            flags |= SM_PRED_TRANS_DEF;
          if (L_is_uncond_predicate_ptype (reg_ptype))
            flags |= SM_PRED_UNCOND_DEF;
        }
    }
  else
    {
      flags |= SM_USE_ACTION;

      /* For predicate uses, set predicate use type */
      if (reg_operand->ctype == L_CTYPE_PREDICATE)
        {
          /* Currently pred[0] is the only predicate that is unconditionally
           * used (it is used whether or not the instruction is pred 
           * squashed). 
           *
           * All other uses of predicate registers (in loads/stores) are
           * conditional.
           */
          if (operand_type == MDES_PRED)
            flags |= SM_PRED_UNCOND_USE;
          else
            flags |= SM_PRED_COND_USE;
        }
    }

  /* Set action flags based on above and passed in flags */
  action->flags = flags;

  /* Set reg info this action is for */
  action->rinfo = rinfo;

  /* Place action in the proper place */
  SM_place_reg_action (action);

  /* Return the action inserted */
  return (action);
}

/* Removes an action from appropriate lists in the rinfo.
 * Used by SM_delete_reg_action just before freeing the action
 * and SM_reposition_reg_action just before calling SM_place_reg_action().
 */
void
SM_unplace_reg_action (SM_Reg_Action * action)
{
  SM_Reg_Action *existing_action;

  /* If an actual def, remove it from the def list */
  if ((action->flags & SM_ACTUAL_ACTION) && (action->flags & SM_DEF_ACTION))
    {
      /* Fix existing use actions (conflicting and actual) before 
       * this one and conflicting def actions before this one.
       * 
       * Stop when hit an actual def action.
       */
      for (existing_action = action->prev_complete;
           ((existing_action != NULL) &&
            !((existing_action->flags & SM_ACTUAL_ACTION) &&
              (existing_action->flags & SM_DEF_ACTION)));
           existing_action = existing_action->prev_complete)
        {
          existing_action->next_def = action->next_def;
        }

      /* Fix existing use actions (conflicting and actual) after 
       * this one and conflicting def actions after this one.
       * 
       * Stop when hit an actual def action.
       */
      for (existing_action = action->next_complete;
           ((existing_action != NULL) &&
            !((existing_action->flags & SM_ACTUAL_ACTION) &&
              (existing_action->flags & SM_DEF_ACTION)));
           existing_action = existing_action->next_complete)
        {
          existing_action->prev_def = action->prev_def;
        }

      /* Remove actual def from def list */
      if (action->prev_def != NULL)
        action->prev_def->next_def = action->next_def;
      else
        action->rinfo->first_def = action->next_def;

      if (action->next_def != NULL)
        action->next_def->prev_def = action->prev_def;
      else
        action->rinfo->last_def = action->prev_def;
    }

  /* If an actual action, remove from actual list */
  if (action->flags & SM_ACTUAL_ACTION)
    {
      /* Fix existing conflicting actions before this one 
       * until hit another actual action.
       */
      for (existing_action = action->prev_complete;
           ((existing_action != NULL) &&
            (existing_action->flags & SM_CONFLICTING_ACTION));
           existing_action = existing_action->prev_complete)
        {
          existing_action->next_actual = action->next_actual;
        }

      /* Fix existing conflicting actions after this one
       * until hit another actual action.
       */
      for (existing_action = action->next_complete;
           ((existing_action != NULL) &&
            (existing_action->flags & SM_CONFLICTING_ACTION));
           existing_action = existing_action->next_complete)
        {
          existing_action->prev_actual = action->prev_actual;
        }

      /* Remove actual action from actual list */
      if (action->prev_actual != NULL)
        action->prev_actual->next_actual = action->next_actual;
      else
        action->rinfo->first_actual = action->next_actual;

      if (action->next_actual != NULL)
        action->next_actual->prev_actual = action->prev_actual;
      else
        action->rinfo->last_actual = action->prev_actual;
    }


  /* Remove action from complete list */
  if (action->prev_complete != NULL)
    action->prev_complete->next_complete = action->next_complete;
  else
    action->rinfo->first_complete = action->next_complete;

  if (action->next_complete != NULL)
    action->next_complete->prev_complete = action->prev_complete;
  else
    action->rinfo->last_complete = action->prev_complete;

  /* Done, removed from all lists */
}

/* Deletes the reg action including all conflicting actions and
 * all dependences into or out of this action.
 */
void
SM_delete_reg_action (SM_Reg_Action * action)
{
  SM_Oper *sm_op;
  SM_Dep *dep_in, *next_dep_in, *dep_out, *next_dep_out;
  SM_Reg_Action **conflict_ptr, *conflict;
  SM_Reg_Action **conflict_array;
  int num_conflicts;

  /* Punt if this action is in any queues  */
  if (action->first_queue != NULL)
    L_punt ("SM_delete_reg_action: Action still in queues!");

  /* Get the sm_op for this action for ease of use */
  sm_op = action->sm_op;

  /* Delete all incoming and outgoing dependences into this action */
  for (dep_in = action->first_dep_in; dep_in != NULL; dep_in = next_dep_in)
    {
      /* Get the next dep_in before deleting this one */
      next_dep_in = dep_in->next_dep_in;

      /* Delete this dependence */
      SM_delete_dep (dep_in);
    }
  for (dep_out = action->first_dep_out; dep_out != NULL;
       dep_out = next_dep_out)
    {
      /* Get the next dep_out before deleting this one */
      next_dep_out = dep_out->next_dep_out;

      /* Delete this dependence */
      SM_delete_dep (dep_out);
    }

  /* Remove from operand action list */
  if (action->prev_op_action != NULL)
    action->prev_op_action->next_op_action = action->next_op_action;
  else
    sm_op->first_op_action = action->next_op_action;

  if (action->next_op_action != NULL)
    action->next_op_action->prev_op_action = action->prev_op_action;
  else
    sm_op->last_op_action = action->prev_op_action;

  /* Get the conflict array and number of conflicts before deleting
   * register action.
   */
  conflict_array = action->conflict;
  num_conflicts = action->rinfo->num_conflicts;

  /* Need to free this action and all of it's conflicts */
  for (conflict_ptr = conflict_array;
       (conflict = *conflict_ptr) != NULL; conflict_ptr++)
    {
      /* Remove conflict from all rinfo lists */
      SM_unplace_reg_action (conflict);

      /* Free the action */
      L_free (SM_Reg_Action_pool, conflict);
    }

  /* Free the conflict array.
   * Free to conflict pool if number of conflicts of typical size,
   * otherwise use free().
   */
  if (num_conflicts <= TYPICAL_NUM_CONFLICTS)
    {
      L_free (SM_Reg_Action_Conflict_pool, conflict_array);
    }
  else
    {
      free (conflict_array);
    }
}

/* This function reposition's a reg_action in it's rinfo list without
 * changing anything else (I.e., none of the dependences are changed!). 
 * This is currently called only by SM_move_oper_after().
 */
void
SM_reposition_reg_action (SM_Reg_Action * action)
{
  SM_Reg_Action **conflict_ptr, *conflict;
  SM_Reg_Action **conflict_array;

  /* Get the conflict array for this action */
  conflict_array = action->conflict;

  /* For each conflicting action, unplace and then place action
   * so that it occurs in the proper location
   */
  for (conflict_ptr = conflict_array;
       (conflict = *conflict_ptr) != NULL; conflict_ptr++)
    {
      /* Remove conflict from all rinfo lists */
      SM_unplace_reg_action (conflict);

      /* Place the conflict action back in all the appropriate rinfo lists 
       * in the proper place 
       */
      SM_place_reg_action (conflict);
    }
}

void
SM_update_implicit_branch_actions (SM_Cb * sm_cb, SM_Reg_Info * new_rinfo)
{
  SM_Reg_Action *ctrl_action, *new_def, *new_use;
  L_Operand *new_operand;
  SM_Oper *ctrl_sm_op;
  L_Cb *lcode_cb;
  unsigned int mdes_flags;

  /* Only update for real operands (not mem, ctrl, sync, etc.) */
  if (new_rinfo->type & SM_EXT_ACTION_TYPE)
    return;

  /* Get lcode cb for ease of use */
  lcode_cb = sm_cb->lcode_cb;

  /* Get the operand for this new_rinfo for ease of use */
  new_operand = new_rinfo->operand;

  /* For every branch in the cb (so far), update its inplicit operands
   * (if necessary).
   */
  for (ctrl_action = sm_cb->ctrl_rinfo->first_def; ctrl_action != NULL;
       ctrl_action = ctrl_action->next_def)
    {
      /* Get the mdes flags for the branch this ctrl_action belongs to */
      ctrl_sm_op = ctrl_action->sm_op;
      mdes_flags = ctrl_sm_op->mdes_flags;

      /* For jsr's, add new fragile operands to its implicit srcs and dests
       * queues.
       */
      if (mdes_flags & OP_FLAG_JSR)
        {
          if (new_rinfo->flags & SM_FRAGILE_MACRO)
            {
              new_use = SM_add_reg_action (ctrl_sm_op,
                                           MDES_SYNC_IN,
                                           SM_CTRL_ACTION_INDEX, new_operand);

              new_def = SM_add_reg_action (ctrl_sm_op,
                                           MDES_SYNC_OUT,
                                           SM_CTRL_ACTION_INDEX, new_operand);

              /* Add this new implicit use and def to the implicit
               * action queues for this branch.
               */
	      if (new_use)
		SM_enqueue_action_before (ctrl_sm_op->implicit_srcs,
					  new_use, NULL);
	      if (new_def)
		SM_enqueue_action_before (ctrl_sm_op->implicit_dests,
					  new_def, NULL);
            }
        }

      /* For conditional branches and jumps, if the operand is in its
       * liveout, add to its implicit src queue.
       */
      else if (mdes_flags & (OP_FLAG_CBR | OP_FLAG_JMP))
        {
	  if (!L_find_attr (ctrl_sm_op->lcode_op->attr, "PRIC"))
            {
              /* Only add operands in liveout of branch.
               * (Assume liveout, if using fake dataflow info -JCG 6/99)
               */
	      if (SM_use_fake_dataflow_info ||
		  ((!(sm_cb->flags & SM_MODULO) || 
		    (sm_cb->last_serial_op != ctrl_sm_op)) &&
		   L_in_oper_OUT_set (lcode_cb, ctrl_sm_op->lcode_op, 
				      new_operand, TAKEN_PATH)) ||
		  ((sm_cb->flags & SM_MODULO) &&
		   (sm_cb->last_serial_op == ctrl_sm_op) &&
		   L_cond_branch (ctrl_sm_op->lcode_op) &&
		   L_in_oper_OUT_set (lcode_cb, ctrl_sm_op->lcode_op,
				      new_operand, FALL_THRU_PATH)))
                {
                  new_use = SM_add_reg_action (ctrl_sm_op,
                                               MDES_SYNC_IN,
                                               SM_CTRL_ACTION_INDEX,
                                               new_operand);

                  /* Add this new implicit use to the implicit src
                   * action queue for this branch.
                   */
                  if (new_use)
		    SM_enqueue_action_before (ctrl_sm_op->implicit_srcs,
					      new_use, NULL);
                }
            }
        }
    }
  return;
}

/* Adds a reg action for the reg_operand into the approriate place in
 * the rinfo action lists.  Also inserts appropriate 'conflict' actions
 * into conflicting rinfo action lists.
 *
 * Calls SM_insert_reg_action to insert each individual action required.
 *
 * Returns the actual action added.
 */
SM_Reg_Action *
SM_add_reg_action (SM_Oper * sm_op, int operand_type,
                   int operand_number, L_Operand * reg_operand)
{
  SM_Cb *sm_cb;
  SM_Reg_Info *rinfo, **reg_conflict;
  SM_Reg_Action **conflict, *actual_action, *conflicting_action;
  int num_conflicts, i;

  /* Get sm_cb for ease of use */
  sm_cb = sm_op->sm_cb;

  if (L_is_macro(reg_operand) && !M_dataflow_macro(reg_operand->value.mac))
    return NULL;

  /* Find reg info for this register, create if necessary */
  if ((rinfo = SM_find_reg_info (sm_cb, reg_operand)) == NULL)
    {
      rinfo = SM_add_reg_info (sm_cb, reg_operand);
    }

  /* Get reg_info register conflict info */
  reg_conflict = rinfo->reg_conflict;
  num_conflicts = rinfo->num_conflicts;

  /* Allocate action conflict array.  Use alloc pool when have
   * typical number of conflicts, malloc for larger conflict arrays.
   */
  if (num_conflicts <= TYPICAL_NUM_CONFLICTS)
    {
      conflict = (SM_Reg_Action **) L_alloc (SM_Reg_Action_Conflict_pool);
    }
  else
    {
      conflict = (SM_Reg_Action **) malloc ((num_conflicts + 2) *
                                            sizeof (SM_Reg_Action *));
      if (conflict == NULL)
        L_punt ("SM_add_reg_action: Out of memory");
    }

  /* Insert action for actual register */
  actual_action = SM_insert_reg_action (rinfo, sm_op, operand_type,
                                        operand_number, reg_operand,
                                        SM_ACTUAL_ACTION);

  /* Point actual action at conflict array, and conflict array at action */
  actual_action->conflict = conflict;
  conflict[0] = actual_action;

  /* Insert conflicting actions for conflicting registers (if any) */
  for (i = 0; i < num_conflicts; i++)
    {
      conflicting_action = SM_insert_reg_action (reg_conflict[i], sm_op,
                                                 operand_type, operand_number,
                                                 reg_operand,
                                                 SM_CONFLICTING_ACTION);

      /* Point conflicting action at conflict array, and conflict array 
       * at action 
       */
      conflicting_action->conflict = conflict;
      conflict[i + 1] = conflicting_action;
    }

  /* Terminate conflict list with NULL */
  conflict[num_conflicts + 1] = NULL;


  /* Add actual action to end of operation's op action list.
   * This allows easy scanning of all the register action for 
   * an sm_op.
   */
  actual_action->prev_op_action = sm_op->last_op_action;
  actual_action->next_op_action = NULL;
  if (sm_op->last_op_action != NULL)
    sm_op->last_op_action->next_op_action = actual_action;
  else
    sm_op->first_op_action = actual_action;
  sm_op->last_op_action = actual_action;


  /* Return the actual reg action added */
  return (actual_action);
}


/* Inserts all the appropriate reg actions (in the proper place) for
 * this sm_op.
 */
void
SM_add_reg_actions_for_op (SM_Oper * sm_op)
{
  SM_Cb *sm_cb;
  L_Attr *attr;
  L_Cb *lcode_cb;
  L_Oper *lcode_op;
  SM_Reg_Info *rinfo;
  SM_Reg_Action *implicit_src, *implicit_dest;
  unsigned int mdes_flags;
  int i;

  sm_cb = sm_op->sm_cb;

  /* Get the lcode operation in order to access operands */
  lcode_op = sm_op->lcode_op;

#if 0
  /* Debug */
  printf ("  Op %i:\n", lcode_op->id);
#endif

  /* Get the mdes flags for ease of use */
  mdes_flags = sm_op->mdes_flags;

  /* Add use actions before def actions to rinfo table */
  for (i = 0; i < L_max_src_operand; i++)
    {
      if (SM_is_reg (lcode_op->src[i]))
	sm_op->src[i] = SM_add_reg_action (sm_op, MDES_SRC, i,
					   lcode_op->src[i]);
    }

  if ((attr = L_find_attr (lcode_op->attr, "src")))
    {
      for (i = 0; i < attr->max_field; i++)
        {
          L_Operand *src;
          if (!(src = attr->field[i]))
            continue;

          implicit_src = SM_add_reg_action (sm_op, MDES_SYNC_IN,
                                            SM_CTRL_ACTION_INDEX, src);
	  if (implicit_src)
	    SM_enqueue_action_before (sm_op->implicit_srcs, implicit_src, 
				      NULL);
        }
    }

  /* Only the first predicate is actual an operand.
   * This second predicate is used with promotion to indicate what
   * the original predicate is.  It is for informational purposes
   * only and an reg action should not be created for it!
   */
  if (L_max_pred_operand > 0)
    {
      if (SM_is_reg (lcode_op->pred[0]))
	sm_op->pred[0] = SM_add_reg_action (sm_op, MDES_PRED, 0,
					    lcode_op->pred[0]);
    }

  /*
   * All operations have use actions for the cntl and sync special operands
   */
  sm_op->ext_src[SM_CTRL_ACTION_INDEX] =
    SM_add_reg_action (sm_op, MDES_SYNC_IN, SM_CTRL_ACTION_INDEX,
                       SM_CTRL_ACTION_OPERAND);

  sm_op->ext_src[SM_SYNC_ACTION_INDEX] =
    SM_add_reg_action (sm_op, MDES_SYNC_IN, SM_SYNC_ACTION_INDEX,
                       SM_SYNC_ACTION_OPERAND);

  /* Memory loads are modeled as using the memory operand. 
   * As an optimization, don't model that jsrs can read memory (in addition
   * to writing memory).  Just model as writing to memory like a store,
   * since that gives all the dependences we currently can detect.
   */
  if (mdes_flags & (OP_FLAG_LOAD | OP_FLAG_CHK))
    {
      sm_op->ext_src[SM_MEM_ACTION_INDEX] =
        SM_add_reg_action (sm_op, MDES_SYNC_IN, SM_MEM_ACTION_INDEX,
                           SM_MEM_ACTION_OPERAND);
    }


  /* 
   * Need figure out when VLIW uses should be inserted!
   */

  for (i = 0; i < L_max_dest_operand; i++)
    {
      if (SM_is_reg (lcode_op->dest[i]))
	{
	  sm_op->dest[i] = SM_add_reg_action (sm_op, MDES_DEST, i,
					      lcode_op->dest[i]);
	  {
	    L_Attr *attr;
	    L_Operand *opd;

	    if ((attr = L_find_attr (lcode_op->attr, "ADDLAT")) &&
		(attr->max_field > i) && (opd = attr->field[i]) &&
		L_is_int_constant (opd))
	      {
		L_warn ("Adding lat %d to dest %d of op %d",
			(int) opd->value.i, i, lcode_op->id);
		sm_op->dest[i]->add_lat += opd->value.i;
	      }
	  }
	}
    }


  /* Memory stores are modeled as defining the memory operand.
   * Also model jsr's as possibly modifying memory, so they also
   * define memory operands.  
   */
  if ((mdes_flags & OP_FLAG_STORE) || (mdes_flags & OP_FLAG_JSR))
    {
      sm_op->ext_dest[SM_MEM_ACTION_INDEX] =
        SM_add_reg_action (sm_op, MDES_SYNC_OUT, SM_MEM_ACTION_INDEX,
                           SM_MEM_ACTION_OPERAND);
    }

  /* All Branches are modeled as defining the cntl operand */
  if (mdes_flags & (OP_FLAG_CBR | OP_FLAG_JMP | OP_FLAG_RTS | OP_FLAG_JSR
                    | OP_FLAG_CHK))
    {
      sm_op->ext_dest[SM_CTRL_ACTION_INDEX] =
        SM_add_reg_action (sm_op, MDES_SYNC_OUT, SM_CTRL_ACTION_INDEX,
                           SM_CTRL_ACTION_OPERAND);
    }

  /* HACK to keep allocs from being in the same cycles as br.ret */
  if (lcode_op->opc == Lop_ALLOC)
    {
      sm_op->ext_dest[SM_CTRL_ACTION_INDEX] =
        SM_add_reg_action (sm_op, MDES_SYNC_OUT, SM_CTRL_ACTION_INDEX,
                           SM_CTRL_ACTION_OPERAND);
    }

  /* All ops marked with OP_FLAG_SYNC are sync operations plus any
   * lcode operations marked with L_OPER_SYNC (namely setjmp and longjmp)
   */
  if ((mdes_flags & OP_FLAG_SYNC) || (lcode_op->flags & L_OPER_SYNC) ||
      L_check_branch_opcode (lcode_op))
    {
      sm_op->ext_dest[SM_SYNC_ACTION_INDEX] =
        SM_add_reg_action (sm_op, MDES_SYNC_OUT, SM_SYNC_ACTION_INDEX,
                           SM_SYNC_ACTION_OPERAND);
    }

  /* 
   * Need to figure out when to define VLIW actions!
   */

  /*
   * Build implicit defs/uses for the various branches 
   */

  /*
   * For checks, make preserved regs implicit sources
   */

  /* For JSR's, make all known fragile macros implicit srcs and dests */
  if (mdes_flags & OP_FLAG_JSR)
    {
      for (rinfo = sm_cb->first_rinfo; rinfo != NULL;
           rinfo = rinfo->next_rinfo)
        {
          /* Only add fragile macros to implicit srcs and dests */
          if (!(rinfo->flags & SM_FRAGILE_MACRO))
            continue;

          /* Create implicit src and dest for this operand */
          implicit_src = SM_add_reg_action (sm_op, MDES_SYNC_IN,
                                            SM_CTRL_ACTION_INDEX,
                                            rinfo->operand);

          implicit_dest = SM_add_reg_action (sm_op, MDES_SYNC_OUT,
                                             SM_CTRL_ACTION_INDEX,
                                             rinfo->operand);

          /* Place these implicit action in the sm_op's implicit src
           * and dest queues.
           */
	  if (implicit_src)
	    SM_enqueue_action_before (sm_op->implicit_srcs, implicit_src, 
				      NULL);
	  if (implicit_dest)
	    SM_enqueue_action_before (sm_op->implicit_dests, implicit_dest,
				      NULL);
        }
    }

  /* For conditional branches and jumps, make live out registers 
   * implicit srcs 
   */
  if (mdes_flags & (OP_FLAG_CBR | OP_FLAG_JMP) ||
      ((mdes_flags & OP_FLAG_CHK) && L_is_control_oper(lcode_op)))
    {
      /* Get lcode cb for ease of use */
      lcode_cb = sm_cb->lcode_cb;

      if (!L_find_attr (lcode_op->attr, "PRIC"))
	{
	  for (rinfo = sm_cb->first_rinfo; rinfo != NULL;
	       rinfo = rinfo->next_rinfo)
	    {
	      /* Only look at real operands (not mem, ctrl, sync, etc.) */
	      if (rinfo->type & SM_EXT_ACTION_TYPE)
		continue;

	      /* Only add operands in liveout of branch 
	       * (Assume liveout, if using fake dataflow info -JCG 6/99)
	       */
	      if (SM_use_fake_dataflow_info ||
		  ((!(sm_cb->flags & SM_MODULO) || 
		    (sm_cb->last_serial_op != sm_op)) &&
		   L_in_oper_OUT_set (lcode_cb, lcode_op, rinfo->operand,
				      TAKEN_PATH)) ||
		  ((sm_cb->flags & SM_MODULO) &&
		   (sm_cb->last_serial_op == sm_op) &&
		   L_cond_branch (lcode_op) &&
		   L_in_oper_OUT_set (lcode_cb, lcode_op, rinfo->operand,
				      FALL_THRU_PATH)))
		{
		  implicit_src = SM_add_reg_action (sm_op, MDES_SYNC_IN,
						    SM_CTRL_ACTION_INDEX,
						    rinfo->operand);
		  if (implicit_src)
		    SM_enqueue_action_before (sm_op->implicit_srcs,
					      implicit_src, NULL);
		}
	    }
	}
    }
  return;
}

/* Builds reg info table of all the sm_cb's register actions.
 * (Uses techniques taken from dynamic_symbol library.)
 */
void
SM_build_reg_info_table (SM_Cb * sm_cb)
{
  SM_Oper *sm_op;
  SM_Reg_Info **hash;
  unsigned int hash_size;
  int i;

  /* Start with a hash size of 32.
   * (Smaller sizes don't work as well with the hashing algorithm)
   */
  hash_size = 32;

  /* Increase until bigger than the number of operations in sm_cb */
  while (hash_size < sm_cb->op_count)
    hash_size = hash_size << 1;

  /* From profiling, doubling the hash size improves performance */
  hash_size = hash_size << 1;

  /* Create pools for all the SM_Reg_Info structures if necessary */
  if (SM_Reg_Info_pool == NULL)
    {
      SM_Reg_Info_pool = L_create_alloc_pool ("SM_Reg_Info",
                                              sizeof (SM_Reg_Info), 32);

      SM_Reg_Action_pool = L_create_alloc_pool ("SM_Reg_Action",
                                                sizeof (SM_Reg_Action), 64);

      /* Use this conflict pool when a "typical" number of conflicts occur.
       * Use malloc for the large cases.
       */
      SM_Reg_Action_Conflict_pool =
        L_create_alloc_pool ("SM_Reg_Action_Conflict",
                             (TYPICAL_NUM_CONFLICTS + 2) *
                             sizeof (SM_Reg_Action *), 64);
    }

  /* Allocate array for hash */
  hash = (SM_Reg_Info **) malloc (hash_size * sizeof (SM_Reg_Info *));
  if (hash == NULL)
    L_punt ("SM_new_reg_info_table: Out of memory");

  /* Initialize hash table */
  for (i = 0; i < hash_size; i++)
    hash[i] = NULL;

  /* Initialize fields */
  sm_cb->rinfo_hash = hash;
  sm_cb->rinfo_hash_size = hash_size;
  sm_cb->rinfo_hash_mask = hash_size - 1;
  /* Resize when count at 75% of hash_size */
  sm_cb->rinfo_resize_size = hash_size - (hash_size >> 2);
  sm_cb->first_rinfo = NULL;
  sm_cb->last_rinfo = NULL;
  sm_cb->rinfo_count = 0;

  /* Add rinfo fields for the extended operands and cache them
   * in the sm_cb structure.
   */
  sm_cb->mem_rinfo = SM_add_reg_info (sm_cb, SM_MEM_ACTION_OPERAND);
  sm_cb->ctrl_rinfo = SM_add_reg_info (sm_cb, SM_CTRL_ACTION_OPERAND);
  sm_cb->sync_rinfo = SM_add_reg_info (sm_cb, SM_SYNC_ACTION_OPERAND);
  sm_cb->vliw_rinfo = SM_add_reg_info (sm_cb, SM_VLIW_ACTION_OPERAND);

  /* Build rinfo table for every operand of every operation in sm_cb */
  for (sm_op = sm_cb->first_serial_op; sm_op != NULL;
       sm_op = sm_op->next_serial_op)
    {
#if 0
      /* For testing purposes, add actions for odd numbered ops first */
      if ((sm_op->lcode_op->id & 1) == 0)
        continue;
#endif

      SM_add_reg_actions_for_op (sm_op);
    }


#if 0
  /* Build rinfo table for every operand of every operation in sm_cb */
  for (sm_op = sm_cb->first_serial_op; sm_op != NULL;
       sm_op = sm_op->next_serial_op)
    {
      /* For testing purposes, then add actions for even numbered ops */
      if ((sm_op->lcode_op->id & 1) == 1)
        continue;

      SM_add_reg_actions_for_op (sm_op);
    }
#endif

}

void
SM_print_reg_action (FILE * out, SM_Reg_Action * action)
{
  if (action->flags & SM_CONFLICTING_ACTION)
    fprintf (out, "   [");
  else
    fprintf (out, "    ");

  if (action->flags & SM_DEF_ACTION)
    fprintf (out, "DEF ");
  if (action->flags & SM_USE_ACTION)
    fprintf (out, "USE ");
  if (action->flags & (SM_PRED_UNCOND_DEF))
    fprintf (out, "UNCOND ");
  if (action->flags & (SM_PRED_TRANS_DEF))
    fprintf (out, "TRANS   ");

  fprintf (out, "Op %-3i ", action->sm_op->lcode_op->id);

  if (action->operand_type == MDES_DEST)
    fprintf (out, "dest[");
  else if (action->operand_type == MDES_SRC)
    fprintf (out, " src[");
  else if (action->operand_type == MDES_PRED)
    fprintf (out, "pred[");
  else if (action->operand_type == MDES_SYNC_OUT)
    fprintf (out, "ext_dest[");
  else if (action->operand_type == MDES_SYNC_IN)
    fprintf (out, " ext_src[");
  else
    L_punt ("SM_print_reg_action: unknown operand type '%i'!",
            action->operand_type);

  fprintf (out, "%i] ", action->operand_number);

  if (action->prev_actual != NULL)
    {
      fprintf (out, " (Op %-3i <-act)",
               action->prev_actual->sm_op->lcode_op->id);
    }
  else
    fprintf (out, " (NULL   <-act)");

  if (action->next_actual != NULL)
    {
      fprintf (out, " (act-> Op %-3i)",
               action->next_actual->sm_op->lcode_op->id);
    }
  else
    fprintf (out, " (act-> NULL  )");

  fprintf (out, " ");

  if (action->prev_def != NULL)
    {
      fprintf (out, " (Op %-3i <-def)",
               action->prev_def->sm_op->lcode_op->id);
    }
  else
    fprintf (out, " (NULL   <-def)");

  if (action->next_def != NULL)
    {
      fprintf (out, " (def-> Op %-3i)",
               action->next_def->sm_op->lcode_op->id);
    }
  else
    fprintf (out, " (def-> NULL  )");

  if (action->flags & SM_IMPLICIT_ACTION)
    fprintf (out, " <IMPLICIT>");

  if (action->flags & SM_CONFLICTING_ACTION)
    fprintf (out, "]");


  fprintf (out, "\n");
}

void
SM_print_reg_info_operand (FILE * out, SM_Reg_Info * rinfo)
{
  char buf[100];

  /* Print out the rinfo we are dealing with */
  if (rinfo->type & SM_REGISTER_TYPE)
    {
      sprintf (buf, "(r %d %s)", rinfo->id,
               L_ctype_name (rinfo->operand->ctype));
    }
  else if (rinfo->type & SM_MACRO_TYPE)
    {
      sprintf (buf, "(mac %s %s)", L_macro_name (rinfo->id),
               L_ctype_name (rinfo->operand->ctype));
    }
  else if (rinfo->type & SM_EXT_ACTION_TYPE)
    {
      switch (rinfo->id)
        {
        case SM_MEM_ACTION_INDEX:
          sprintf (buf, "(MEM)");
          break;

        case SM_CTRL_ACTION_INDEX:
          sprintf (buf, "(CTRL)");
          break;

        case SM_SYNC_ACTION_INDEX:
          sprintf (buf, "(SYNC)");
          break;

        case SM_VLIW_ACTION_INDEX:
          sprintf (buf, "(VLIW)");
          break;

        default:
          L_punt ("SM_print_reg_info_operand: Unknown special register id %i",
                  rinfo->id);
        }
    }
  else
    L_punt ("SM_print_reg_info: Unknown reg type %i!\n", rinfo->type);
  fprintf (out, "%-20s ", buf);
}

void
SM_print_reg_info (FILE * out, SM_Reg_Info * rinfo)
{
  SM_Reg_Action *action;
  int i;

  /* Print out operand for this rinfo */
  fprintf (out, "  ");
  SM_print_reg_info_operand (out, rinfo);

  /* Print out id, type */
  fprintf (out, " id %-3i type %i", rinfo->id, rinfo->type);

  if (rinfo->flags & SM_FRAGILE_MACRO)
    fprintf (out, " (FRAGILE)");
  fprintf (out, ":\n");

  /* Print out conflicts, if any */
  if (rinfo->num_conflicts > 0)
    {
      fprintf (out, "    Conflicts: ");
      for (i = 0; i < rinfo->num_conflicts; i++)
        SM_print_reg_info_operand (out, rinfo->reg_conflict[i]);
      fprintf (out, "\n");
    }

  /* Print out the actions for this rinfo */
  for (action = rinfo->first_complete; action != NULL;
       action = action->next_complete)
    {
#if 0
      /* Debug, skip conflicts for now */
      if (action->flags & SM_CONFLICTING_ACTION)
        continue;
#endif

      SM_print_reg_action (out, action);
    }

  fprintf (out, "\n");
}

void
SM_print_reg_info_table (FILE * out, SM_Cb * sm_cb)
{
  SM_Reg_Info *rinfo;

  fprintf (out, "Reg action table for cb %i:\n", sm_cb->lcode_cb->id);

  for (rinfo = sm_cb->first_rinfo; rinfo != NULL; rinfo = rinfo->next_rinfo)
    {
      SM_print_reg_info (out, rinfo);
    }
  fprintf (out, "\n");
}

void
SM_print_sorted_reg_info_table (FILE * out, SM_Cb * sm_cb)
{
  SM_Reg_Info *rinfo;
  Heap *heap;
  int key;

  fprintf (out, "Sorted Reg action table for cb %i:\n", sm_cb->lcode_cb->id);

  /* Create heap */
  heap = Heap_Create (HEAP_MIN);

  /* Insert all rinfo's into heap based on operand's id,type */
  for (rinfo = sm_cb->first_rinfo; rinfo != NULL; rinfo = rinfo->next_rinfo)
    {
      /* Make int key for this rinfo */
      key = ((rinfo->id << MAX_SM_TYPE_POWER) + rinfo->type);

      /* Insert into hash table based on this key */
      Heap_Insert (heap, (void *) rinfo, (double) key);
    }

  /* Pull off the rinfo in sorted order */
  while ((rinfo = (SM_Reg_Info *) Heap_ExtractTop (heap)) != NULL)
    {
      SM_print_reg_info (out, rinfo);
    }

  fprintf (out, "\n");

  /* Dispose of heap, should be empty, no free routine needed */
  heap = Heap_Dispose (heap, NULL);
}

void
SM_free_reg_info_table (SM_Cb * sm_cb)
{
  SM_Reg_Info *rinfo, *next_rinfo;
  SM_Reg_Action *action, *next_action;
  SM_Dep *dep_out, *next_dep_out;

  /* Delete every reg info for cb */
  for (rinfo = sm_cb->first_rinfo; rinfo != NULL; rinfo = next_rinfo)
    {
      /* Get the next rinfo before deleting this rinfo */
      next_rinfo = rinfo->next_rinfo;

      /* Free conflict array, if any */
      if (rinfo->reg_conflict != NULL)
        free (rinfo->reg_conflict);

      /* Delete every reg action for this rinfo */
      for (action = rinfo->first_complete; action != NULL;
           action = next_action)
        {
          /* Get the next action before deleting this one */
          next_action = action->next_complete;

          /* Free every dependence out of this action if an actual action */
          if (action->flags & SM_ACTUAL_ACTION)
            {
              for (dep_out = action->first_dep_out; dep_out != NULL;
                   dep_out = next_dep_out)
                {
                  /* Get the next dep_out before deleting this one */
                  next_dep_out = dep_out->next_dep_out;

                  L_free (SM_Dep_pool, dep_out);
                }

              /* Free the reg conflict array for this action 
               * Only free if actual action, since share array with
               * conflict actions.
               * 
               * Free to conflict pool if number of conflicts of
               * typical size, otherwise use free().
               */
              if (rinfo->num_conflicts <= TYPICAL_NUM_CONFLICTS)
                {
                  L_free (SM_Reg_Action_Conflict_pool, action->conflict);
                }
              else
                {
                  free (action->conflict);
                }
            }


          /* Free this action */
          L_free (SM_Reg_Action_pool, action);
        }

      /* Free this rinfo */
      L_delete_operand (rinfo->operand);
      L_free (SM_Reg_Info_pool, rinfo);

    }

  /* Free the hash array */
  free (sm_cb->rinfo_hash);
}
