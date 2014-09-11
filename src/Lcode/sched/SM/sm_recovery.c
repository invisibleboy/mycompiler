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
 *      File:   sm_recovery.c
 *      Author: Erik Nystrom
 *      Creation Date:  March 2001
\*****************************************************************************/
/* 09/25/02 REK Updating to use function from ltahoe_kapi and ltahoe_op_query
 *              instead of from Tmdes. */

/* 20030304 SZU
 * SMH reconciliation
 * Include recovery code only if RC_CODE defined for now
 */
#if defined(RC_CODE)

#include <config.h>
/* 20030304 SZU */
//#include "smh.h"
#include "sm.h"

/* #include <machine/tmdes_instr.h> */
#include <Lcode/ltahoe_op_query.h>

#ifdef HAMM
#include "sm_hamm_interface.h"
#endif

#if 1
#define RC_DEBUG_INFO
#endif

int RC_print_debug_info = 0;

static List master_dep_list = NULL;
/* This list is temporary until check optis
    prevent the case where one check dominates
    another, making a check unnecessary. Currently,
    the case is sometimes detected and the check
    deleted. In these cases, chk_mark must be
    updated for ops in this list */
static List master_has_chk_mark_list = NULL;

time_t g_start_time;

#define START_TIME   g_start_time = time(NULL);

#define PRINT_DELTA(s) printf("%15s: %d\n",(s),(int)(time(NULL) - g_start_time)); \
                       g_start_time = time(NULL);


/*
 * Misc functions
 */

/*##########################################################
 * Search through a list of operands and delete any matches
 ##########################################################*/
List RC_oprd_remove_from_list (List oprd_list, L_Operand * oprd)
{
  L_Operand *cur_oprd = NULL;

  List_start (oprd_list);
  while ((cur_oprd = (L_Operand *) List_next (oprd_list)))
    {
      if (L_same_operand (oprd, cur_oprd))
        {
          oprd_list = List_delete_current (oprd_list);
        }
    }

  return oprd_list;
}


/*##########################################################
 * Search through a list of operands and see if there is a match
 ##########################################################*/
int
RC_oprd_in_list (List oprd_list, L_Operand * oprd)
{
  int found = 0;
  L_Operand *cur_oprd = NULL;

  for (List_start (oprd_list);
       (cur_oprd = ((L_Operand *) List_next (oprd_list)));)
    {
      if (L_same_operand (oprd, cur_oprd))
        {
          found = 1;
          break;
        }
    }

  return found;
}


/*##########################################################
 * Search through "src" attr fields and see if there 
 *   is a matching operand
 ##########################################################*/
int
RC_check_preserves_oprd (L_Oper * chk_op, L_Operand * oprd)
{
  L_Attr *attr = NULL;
  int cnt;

  if (chk_op->opc != Lop_CHECK)
    L_punt ("RC_check_preserves_oprd: oper is not a check");

  attr = L_find_attr (chk_op->attr, "src");
  if (!attr)
    L_punt ("RC_check_preserves_oprd: src attr not found");

  for (cnt = 0; cnt < attr->max_field; cnt++)
    {
      if (L_same_operand (attr->field[cnt], oprd))
        {
          return 1;
        }
    }

  return 0;
}


/*##########################################################
 * Given a cb add in the fall through flow 
 ##########################################################*/
void 
RC_add_fallthru_for_cb(L_Cb *cb)
{
  L_Cb   *nxt              = NULL;
  L_Oper *op               = NULL;
  L_Oper *last_branch      = NULL;
  L_Flow *last_branch_flow = NULL;
  L_Flow *last_flow        = NULL;

  if (!cb->next_cb)
    return;
  nxt = cb->next_cb;
  
  for (op=cb->last_op; op; op=op->prev_op)
    {
      if (!L_is_control_oper(op))
	continue;

      if (!last_branch)
	last_branch = op;

      if (L_uncond_branch(op))
	return;
      /* I didn't break here incase there is
	 unreachable code in the cb */
    }

  if (last_branch)
    {
      last_branch_flow = L_find_flow_for_branch(cb, last_branch);
      last_flow = last_branch_flow->next_flow;
      if (last_flow)
	{
	  /* Sanity check */
	  if (last_flow->dst_cb != nxt)
	    L_punt("RC_add_fallthru_for_cb: fall thru does "
		   "not match nxt cb\n");
	  return;
	}
    }

  /* Fall thru needed */
  last_flow = L_new_flow (0, cb, nxt, 0.0);
  cb->dest_flow = L_concat_flow (cb->dest_flow, last_flow);
  last_flow = L_new_flow (0, cb, nxt, 0.0);
  nxt->src_flow = L_concat_flow (nxt->src_flow, last_flow);  
}

void
RC_delete_flow_for_op(L_Cb * cb, L_Oper * op)
{
  L_Flow *dst_flow = NULL;
  L_Flow *src_flow = NULL;
  
  dst_flow = L_find_flow_for_branch(cb, op);
  if (!dst_flow)
    L_punt("RC_delete_flow_for_op: can not find dst_flow\n");
  src_flow = L_find_matching_flow (dst_flow->dst_cb->src_flow, dst_flow);
  if (!src_flow)
    L_punt("RC_delete_flow_for_op: can not find src_flow\n");
  
  dst_flow->dst_cb->src_flow = 
    L_delete_flow(dst_flow->dst_cb->src_flow, src_flow);

  cb->dest_flow = 
    L_delete_flow(cb->dest_flow, dst_flow);
}

/*##########################################################
 * Given an op in a cb, add in necessary flows
 ##########################################################*/
void
RC_add_flow_for_op (L_Cb * cb, L_Oper * op)
{
  L_Flow *after_flow = NULL;
  L_Flow *flow = NULL;
  L_Oper *cop = NULL;
  L_Cb *dest_cb = NULL;

  if (!L_is_control_oper (op))
    return;

  for (cop = op->prev_op; (cop && !L_is_control_oper (cop));
       cop = cop->prev_op);

  if (cop)
    after_flow = L_find_flow_for_branch (cb, cop);
  else
    after_flow = NULL;

  dest_cb = L_find_branch_dest (op);

  flow = L_new_flow (1, cb, dest_cb, 0.0);
  cb->dest_flow = L_insert_flow_after (cb->dest_flow, after_flow, flow);

  flow = L_new_flow (1, cb, dest_cb, 0.0);
  dest_cb->src_flow = L_concat_flow (dest_cb->src_flow, flow);

  if (L_dest_flow_out_of_order (cb))
    L_punt ("RC_add_flow_for_op: flows incorrect for orig cb \n");
}


/*##########################################################
 * Move an existing dest flow to a new cb after a given flow
 ##########################################################*/
void
RC_move_dest_flow_after (L_Cb * from_cb, L_Flow * dst_flow,
                         L_Cb * to_cb, L_Flow * to_after_flow)
{
  L_Flow *src_flow = NULL;

  if (!dst_flow)
    L_punt ("RC_move_dest_flow: can not find dst_flow");

  src_flow = L_find_matching_flow (dst_flow->dst_cb->src_flow, dst_flow);
  if (!src_flow)
    L_punt ("RC_move_dest_flow: can not find src_flow");

  /* Move dest flow */
  from_cb->dest_flow = L_remove_flow (from_cb->dest_flow, dst_flow);
  dst_flow->src_cb = to_cb;
  to_cb->dest_flow = L_insert_flow_after (to_cb->dest_flow,
                                          to_after_flow, dst_flow);

  /* Change src of src flow */
  src_flow->src_cb = to_cb;
}


/*##########################################################
 * Move an op and flows to a new cb after an given op
 ##########################################################*/
L_Flow *
RC_move_op_after (L_Cb * from_cb, L_Oper * op,
                  L_Cb * to_cb, L_Oper * to_after_op)
{
  L_Oper *cop = NULL;
  L_Flow *dst_flow = NULL;
  L_Flow *nxt_flow = NULL;
  L_Flow *to_after_flow = NULL;

  if (L_is_control_oper (op))
    {
      /* Get matching flows for op
       */
      dst_flow = L_find_flow_for_branch (from_cb, op);
      if (dst_flow)
        nxt_flow = dst_flow->next_flow;

      /* Find last control op before(and including) to_after_op
       */
      for (cop = to_after_op; (cop && !L_is_control_oper (cop));
           cop = cop->prev_op);

      if (cop)
        to_after_flow = L_find_flow_for_branch (to_cb, cop);
      else
        to_after_flow = NULL;

      RC_move_dest_flow_after (from_cb, dst_flow, to_cb, to_after_flow);
    }

  /* Move the oper itself
   */
  L_remove_oper (from_cb, op);
  L_insert_oper_after (to_cb, to_after_op, op);

  return nxt_flow;
}

L_Flow *
RC_find_flow_after_last_control (L_Cb * cb)
{
  L_Flow *flow;
  L_Oper *op;

  flow = cb->dest_flow;
  for (op = cb->first_op; op; op = op->next_op)
    {
      if (L_is_control_oper (op))
        flow = flow->next_flow;
    }

  return flow;
}


/*##########################################################
 * Split a cb into two cbs after a given op
 ##########################################################*/
L_Cb *
RC_split_cb_after (L_Func * fn, L_Cb * cb, L_Oper * op)
{
  L_Cb *new_cb = NULL;
  L_Flow *flow = NULL;
  L_Flow *nxt_flow = NULL;
  L_Oper *next_op = NULL;

  /* Create a new cb
   */
  new_cb = L_new_cb (++fn->max_cb_id);
#if 0
  new_cb->attr = L_copy_attr (cb->attr);
#endif
  new_cb->attr = NULL;
  new_cb->flags = cb->flags;
  new_cb->weight = cb->weight;

  if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK_NO_FALLTHRU))
    {
      cb->flags = L_CLR_BIT_FLAG (cb->flags, L_CB_HYPERBLOCK_NO_FALLTHRU);
      /*L_warn ("Cleared L_CB_HYPERBLOCK_NO_FALLTHRU, cb %d\n", cb->id);*/
    }

  /* Move ops and flows from orig cb to new cb
   */
  L_insert_cb_after (fn, cb, new_cb);
  for (; op; op = next_op)
    {
      next_op = op->next_op;
      RC_move_op_after (cb, op, new_cb, new_cb->last_op);
    }


  /* There may be a flow left after moving 
   * all of the opers. This must be a fallthru.
   */
  nxt_flow = RC_find_flow_after_last_control (cb);
  if (nxt_flow)
    {
      if (nxt_flow->next_flow)
        L_punt
          ("RC_split_cb_after: nxt_flow is expected to be the last flow");

      if (new_cb->dest_flow)
        flow = L_find_last_flow (new_cb->dest_flow);
      else
        flow = NULL;

      RC_move_dest_flow_after (cb, nxt_flow, new_cb, flow);
    }


  /* Add fallthru flow: to orig cb dest_flow */
  flow = L_new_flow (0, cb, new_cb, 0.0);
  cb->dest_flow = L_concat_flow (cb->dest_flow, flow);

  /* Add fallthru flow: to new_cb src flow */
  flow = L_new_flow (0, cb, new_cb, 0.0);
  new_cb->src_flow = L_concat_flow (new_cb->src_flow, flow);


  if (L_dest_flow_out_of_order (cb))
    L_punt ("RC_split_cb_after: flows incorrect for orig cb \n");

  if (L_dest_flow_out_of_order (new_cb))
    L_punt ("RC_split_cb_after: flows incorrect for new cb \n");

  return new_cb;
}

/*##########################################################
  ##########################################################*/
int
RC_join_cbs (L_Func * fn, L_Cb * cb, L_Cb * com_cb)
{
  L_Flow *flow = NULL;
  L_Flow *nxt_flow = NULL;
  L_Oper *next_op = NULL;
  L_Oper *op = NULL;

  /* Is it safe to join these cbs
   */
  if (L_num_src_cb (com_cb) != 1)
    return 0;

  cb->flags |= com_cb->flags;

  /* Delete flow from cb to com_cb
   */
  flow = RC_find_flow_after_last_control (cb);
  if (flow)
    {
      cb->dest_flow = L_delete_flow (cb->dest_flow, flow);
    }
  else
    L_punt ("RC_join_cbs: com_cb not a fallthru to cb");

  /* Move ops and flows from orig cb to new cb
   */
  for (op = com_cb->first_op; op; op = next_op)
    {
      next_op = op->next_op;
      RC_move_op_after (com_cb, op, cb, cb->last_op);
    }

  /* There may be a flow left after moving 
   * all of the opers. This must be a fallthru.
   */
  nxt_flow = RC_find_flow_after_last_control (com_cb);
  if (nxt_flow)
    {
      if (nxt_flow->next_flow)
        L_punt ("RC_join_cbs: nxt_flow is expected to be the last flow");

      if (cb->dest_flow)
        flow = L_find_last_flow (cb->dest_flow);
      else
        flow = NULL;

      RC_move_dest_flow_after (com_cb, nxt_flow, cb, flow);
    }

  L_delete_cb (fn, com_cb);

  if (L_dest_flow_out_of_order (cb))
    L_punt ("RC_join_cbs: flows incorrect for cb \n");

  return 1;
}


/*##########################################################
 *   Makes a copy of an op for recovery code
 ##########################################################*/
L_Oper *
RC_copy_op (L_Oper * op, List * oprd_list)
{
  L_Oper *new_op;
  L_Attr *attr;
  int i;

  L_fn->max_oper_id++;
  new_op = L_new_oper (L_fn->max_oper_id);
  new_op->opc = op->opc;
  new_op->proc_opc = op->proc_opc;
  new_op->opcode = op->opcode;
  new_op->flags = op->flags;
  new_op->parent_op = op->parent_op;
  new_op->weight = op->weight;
  new_op->ext = NULL;
#if 0
  new_op->attr = NULL;
#endif
  new_op->attr = L_copy_attr (op->attr);

  attr = L_new_attr ("rc", 1);
  new_op->attr = L_concat_attr (new_op->attr, attr);
  L_set_int_attr_field (attr, 0, op->id);

#if 0
  if ((attr=L_find_attr(op->attr,"SPECID")))
    {
      new_op->attr = L_concat_attr (new_op->attr, L_copy_attr(attr));
    }
#endif

  for (i = 0; i < L_MAX_CMPLTR; i++)
    {
      new_op->com[i] = op->com[i];
    }

  /* Copy the operands */
  for (i = 0; i < L_max_dest_operand; i++)
    {
      if (op->dest[i] != NULL)
        {
          new_op->dest[i] = L_copy_operand (op->dest[i]);

          if (oprd_list &&
              !L_is_constant (op->dest[i]) &&
              !RC_oprd_in_list (*oprd_list, op->dest[i]))
            *oprd_list = List_insert_last (*oprd_list, op->dest[i]);
        }
    }

  for (i = 0; i < L_max_src_operand; i++)
    {
      if (op->src[i] != NULL)
        {
          new_op->src[i] = L_copy_operand (op->src[i]);

          if (oprd_list &&
              !L_is_constant (op->src[i]) &&
              !RC_oprd_in_list (*oprd_list, op->src[i]))
            *oprd_list = List_insert_last (*oprd_list, op->src[i]);
        }
    }

  for (i = 0; i < L_max_pred_operand; i++)
    {
      if (op->pred[i] != NULL)
        {
          new_op->pred[i] = L_copy_operand (op->pred[i]);

          if (oprd_list &&
              !L_is_constant (op->pred[i]) &&
              !RC_oprd_in_list (*oprd_list, op->pred[i]))
            *oprd_list = List_insert_last (*oprd_list, op->pred[i]);
        }
    }

  return (new_op);
}


/*##########################################################
 *  Deletes oper in presence of RC dataflow arcs
 ##########################################################*/
void
RC_delete_oper(L_Cb *cb, L_Oper *op)
{
  RC_dep_info  *dep    = NULL;
  L_Oper       *cur_op = NULL;

  List_start(master_dep_list);
  while ((dep = (RC_dep_info*)List_next(master_dep_list)))
    {
      if (dep->to_op == op ||
	  dep->from_op == op)
	{
	  dep->to_op = NULL;
	  dep->from_op = NULL;     
	}
    }

  List_start(master_has_chk_mark_list);
  while ((cur_op = (L_Oper*)List_next(master_has_chk_mark_list)))
    {
      if (RC_FLOW_INFO (cur_op)->chk_mark == op)
	{
	  RC_FLOW_INFO (cur_op)->chk_mark = NULL;
	}
    }

  RC_delete_flow_info (op);
  L_delete_oper(cb, op);
}


/*##########################################################
 *  Dump entire function to <normal name>.<ext>
 ##########################################################*/
void
RC_dump_lcode (L_Func * fn, char *ext)
{
  FILE *tmpfile = NULL;
  char name[50];

  strcpy (name, L_input_file);
  strcat (name, ext);
  tmpfile = fopen (name, "w");
  L_print_func (tmpfile, fn);
  fclose (tmpfile);
}


/*##########################################################
 *  Dump cb to <normal name>.<ext>
 ##########################################################*/
void
RC_dump_lcode_cb (L_Func * fn, L_Cb * cb, char *ext)
{
  FILE *tmpfile = NULL;
  char name[50];

  strcpy (name, L_input_file);
  strcat (name, ext);
  tmpfile = fopen (name, "a");
  L_print_cb (tmpfile, fn, cb);
  fclose (tmpfile);
}


/*##########################################################
 *  Delete all checks from function
 ##########################################################*/
void
RC_delete_all_checks (L_Func * fn)
{
  L_Oper *next_op = NULL;
  L_Oper *op = NULL;
  L_Cb *cb = NULL;

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      for (op = cb->first_op; op; op = next_op)
        {
          next_op = op->next_op;
          if (op->opc == Lop_CHECK)
            L_delete_oper (cb, op);
        }
    }
}


/*##########################################################
 *  Insert dummy defines before any op not having a dest 
 ##########################################################*/
void
RC_insert_antidep_defines (L_Func * fn)
{
  L_Oper *op = NULL;
  L_Oper *new_op = NULL;
  L_Cb *cb = NULL;
  L_Attr *attr = NULL;

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      for (op = cb->first_op; op; op = op->next_op)
        {
          if (op && !op->dest[0])
            {
              new_op = L_create_new_op (Lop_DEFINE);
              new_op->dest[0] = L_new_register_operand (++L_fn->max_reg_id,
                                                        L_CTYPE_LLONG, 0);
              L_insert_oper_before (cb, op, new_op);
              attr = L_new_attr ("rcanti", 0);
              new_op->attr = L_concat_attr (new_op->attr, attr);
            }
        }
    }
}


/*##########################################################
 *  Delete dummy defines 
 ##########################################################*/
void
RC_delete_antidep_defines (L_Func * fn)
{
  L_Oper *next_op = NULL;
  L_Oper *op = NULL;
  L_Cb *cb = NULL;

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      for (op = cb->first_op; op; op = next_op)
        {
          next_op = op->next_op;
          if (op->opc == Lop_DEFINE && L_find_attr (op->attr, "rcanti"))
            L_delete_oper (cb, op);
        }
    }
}





/*
 * Recovery code info management
 */

/*##########################################################
 *   Create a new flow info structure
 ##########################################################*/
RC_flow_info *
RC_new_flow_info ()
{
  RC_flow_info *tmp = calloc (1, sizeof (RC_flow_info));

  return tmp;
}


/*##########################################################
 *   Delete flow info structure from an oper
 ##########################################################*/
void
RC_delete_flow_info (L_Oper * op)
{
  RC_flow_info *tmp = RC_FLOW_INFO (op);
  int i = 0;

  if (!tmp)
    return;

  tmp->cb = NULL;
  tmp->def_op_list = List_reset_free_ptrs (tmp->def_op_list, RC_free_dep);
  tmp->use_op_list = List_reset_free_ptrs (tmp->use_op_list, RC_free_dep);
  tmp->anti_def_op_list =
    List_reset_free_ptrs (tmp->anti_def_op_list, RC_free_dep);
  tmp->anti_use_op_list =
    List_reset_free_ptrs (tmp->anti_use_op_list, RC_free_dep);
  tmp->chk_mark = NULL;
  
  /* <list>[i] past chk_num should be NULL */
  for (i = 0; i < tmp->chk_num; i++)
    {
      tmp->chk_ops[i] = NULL;
    }

  op->ext = NULL;
  free (tmp);
}


/*##########################################################
 *   Create a flow info structure for every op
 ##########################################################*/
void
RC_init (L_Func * fn)
{
  L_Cb *cb = NULL;
  L_Oper *op = NULL;
  L_Attr *tr_attr = NULL;

#ifdef RC_DEBUG_INFO
  printf ("---- Init  ----\n");
#endif

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      for (op = cb->first_op; op; op = op->next_op)
        {
          if (op->opc == Lop_CHECK)
            {
              tr_attr = L_find_attr (op->attr, "tr");
              op->attr = L_delete_attr (op->attr, tr_attr);
            }

          op->ext = RC_new_flow_info ();
          RC_FLOW_INFO (op)->cb = cb;
        }
    }
}


/*##########################################################
 *   Delete all flow info structures
 ##########################################################*/
void
RC_cleanup (L_Func * fn)
{
  L_Cb *cb = NULL;
  L_Oper *op = NULL;

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      for (op = cb->first_op; op; op = op->next_op)
        {
          RC_delete_flow_info (op);
        }
    }

  master_dep_list = List_reset(master_dep_list);
  master_has_chk_mark_list = List_reset(master_has_chk_mark_list);
}


/*##########################################################
 *  Gather stats on recovery code
 ###########################################################*/
void
RC_gather_stats (L_Func * fn, List lds_list)
{
  L_Cb *cb = NULL;
  L_Oper *op = NULL;
  int i = 0;
  int op_cnt[rc_last];
  int cb_cnt[rc_last];
  int chk_cnt[rc_last];
  int nop_cnt[rc_last];
  rc_kind kind;

  printf ("------- Gathering Stats -------\n");
  for (i = rc_notrc; i < rc_last; i++)
    {
      op_cnt[i] = 0;
      cb_cnt[i] = 0;
      chk_cnt[i] = 0;
      nop_cnt[i] = 0;
    }

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      if (L_find_attr (cb->attr, "rc"))
        kind = rc_isrc;
      else
        kind = rc_notrc;

      cb_cnt[kind]++;
      cb_cnt[rc_total]++;

      for (op = cb->first_op; op; op = op->next_op)
        {
          if (op->opc == Lop_DEFINE)
            continue;

          op_cnt[kind]++;
          op_cnt[rc_total]++;
          if (op->opc == Lop_CHECK)
            {
              chk_cnt[kind]++;
              chk_cnt[rc_total]++;
            }
          else if (op->opc == Lop_NO_OP)
            {
              nop_cnt[kind]++;
              nop_cnt[rc_total]++;
            }
        }
    }

  printf ("\n");
  printf ("Ops           NonRC[%6d:%0.2f] RC [%6d:%0.2f]   Total[%6d]\n",
          op_cnt[rc_notrc], (double) op_cnt[rc_notrc] / op_cnt[rc_total],
          op_cnt[rc_isrc], (double) op_cnt[rc_isrc] / op_cnt[rc_total],
          op_cnt[rc_total]);
  printf ("Cbs           NonRC[%6d:%0.2f] RC [%6d:%0.2f]   Total[%6d]\n",
          cb_cnt[rc_notrc], (double) cb_cnt[rc_notrc] / cb_cnt[rc_total],
          cb_cnt[rc_isrc], (double) cb_cnt[rc_isrc] / cb_cnt[rc_total],
          cb_cnt[rc_total]);
  printf ("Chks          NonRC[%6d:%0.2f] RC [%6d:%0.2f]   Total[%6d]\n",
          chk_cnt[rc_notrc], (double) chk_cnt[rc_notrc] / chk_cnt[rc_total],
          chk_cnt[rc_isrc], (double) chk_cnt[rc_isrc] / chk_cnt[rc_total],
          chk_cnt[rc_total]);
  printf
    ("Chks vs Ops   NonRC[%0.2f]        RC [%0.2f]          Total[%0.2f]\n",
     (double) chk_cnt[rc_notrc] / op_cnt[rc_notrc],
     (double) chk_cnt[rc_isrc] / op_cnt[rc_isrc],
     (double) chk_cnt[rc_total] / op_cnt[rc_total]);
  printf ("Nops          NonRC[%6d:%0.2f] RC [%6d:%0.2f]   Total[%6d]\n",
          nop_cnt[rc_notrc], (double) nop_cnt[rc_notrc] / nop_cnt[rc_total],
          nop_cnt[rc_isrc], (double) nop_cnt[rc_isrc] / nop_cnt[rc_total],
          nop_cnt[rc_total]);
  printf
    ("Nops vs Ops   NonRC[%0.2f]        RC [%0.2f]          Total[%0.2f]\n",
     (double) nop_cnt[rc_notrc] / op_cnt[rc_notrc],
     (double) nop_cnt[rc_isrc] / op_cnt[rc_isrc],
     (double) nop_cnt[rc_total] / op_cnt[rc_total]);
  printf ("\n");
}







/*
 * Dataflow interface
 */

/*##########################################################
 ##########################################################*/
void
RC_free_dep (void *info)
{
  RC_dep_info *tmp = (RC_dep_info *) info;

  if (tmp)
    free (tmp);
}

/*##########################################################
 ##########################################################*/
RC_dep_info *
RC_new_dep (L_Oper * from_op, L_Oper * to_op, L_Operand * oprd)
{
  RC_dep_info *tmp = (RC_dep_info *) malloc (sizeof (RC_dep_info));

  tmp->from_op = from_op;
  tmp->to_op = to_op;
  tmp->oprd = oprd;

  master_dep_list = List_insert_last(master_dep_list, tmp);

  return tmp;
}

/*##########################################################
 *  Connect two ops via flow dep
 ##########################################################*/
void
RC_add_flow_dep (L_Oper * def_op, L_Oper * use_op, L_Operand * oprd)
{
  RC_flow_info *def_op_info = NULL;
  RC_flow_info *use_op_info = NULL;
  RC_dep_info *dep = NULL;

  if (!def_op || !use_op)
    L_punt ("RC_add_flow_dep: def_op or use_op is NULL");
  if (!oprd)
    L_punt ("RC_add_flow_dep: oprd is NULL");

  def_op_info = RC_FLOW_INFO (def_op);
  use_op_info = RC_FLOW_INFO (use_op);

  if (!def_op_info || !use_op_info)
    L_punt ("RC_add_flow_dep: def_op or use_op missing flow_info");

  /* Add use_op as use of def_op's dest */
  dep = RC_new_dep (def_op, use_op, oprd);
  def_op_info->use_op_list = List_insert_last (def_op_info->use_op_list, dep);

  /* Add def_op as definer of use_op's src */
  dep = RC_new_dep (def_op, use_op, oprd);
  use_op_info->def_op_list = List_insert_last (use_op_info->def_op_list, dep);
}


/*##########################################################
 *  Connect two ops via anti dep
 ##########################################################*/
void
RC_add_anti_dep (L_Oper * def_op, L_Oper * use_op, L_Operand * oprd)
{
  RC_flow_info *def_op_info = NULL;
  RC_flow_info *use_op_info = NULL;
  RC_dep_info *dep = NULL;

  if (!def_op || !use_op)
    L_punt ("RC_add_anti_dep: def_op or use_op is NULL");

  def_op_info = RC_FLOW_INFO (def_op);
  use_op_info = RC_FLOW_INFO (use_op);

  if (!def_op_info || !use_op_info)
    L_punt ("RC_add_anti_dep: def_op or use_op missing flow_info");

  /* Add use_op as use of def_op's dest */
  dep = RC_new_dep (def_op, use_op, oprd);
  def_op_info->anti_use_op_list =
    List_insert_last (def_op_info->anti_use_op_list, dep);

  /* Add def_op as definer of use_op's src */
  dep = RC_new_dep (def_op, use_op, oprd);
  use_op_info->anti_def_op_list =
    List_insert_last (use_op_info->anti_def_op_list, dep);

  /*
     printf("Antidep from op %d -> op %d\n",
     def_op->id, use_op->id);

     printf("Op %d ",use_op->id);
     L_print_operand(stdout, use_op->src[s], 1);
     printf(" reaches Op %d ",def_op->id);
     L_print_operand(stdout, def_op->dest[d], 1);
     printf(" (seen via ");
     L_print_operand(stdout, use_dest, 1);
     printf(" )\n ");      
   */
}


/*##########################################################
 *
 ##########################################################*/
void
RC_reset_dataflow (L_Func * fn)
{
  L_Cb *cb = NULL;
  L_Oper *op = NULL;

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      for (op = cb->first_op; op; op = op->next_op)
        {
          if (!op->ext)
            op->ext = RC_new_flow_info ();

          if (RC_FLOW_INFO (op)->def_op_list)
            RC_FLOW_INFO (op)->def_op_list =
              List_reset_free_ptrs (RC_FLOW_INFO (op)->def_op_list,
                                    RC_free_dep);

          if (RC_FLOW_INFO (op)->use_op_list)
            RC_FLOW_INFO (op)->use_op_list =
              List_reset_free_ptrs (RC_FLOW_INFO (op)->use_op_list,
                                    RC_free_dep);

          if (RC_FLOW_INFO (op)->anti_def_op_list)
            RC_FLOW_INFO (op)->anti_def_op_list =
              List_reset_free_ptrs (RC_FLOW_INFO (op)->anti_def_op_list,
                                    RC_free_dep);

          if (RC_FLOW_INFO (op)->anti_use_op_list)
            RC_FLOW_INFO (op)->anti_use_op_list =
              List_reset_free_ptrs (RC_FLOW_INFO (op)->anti_use_op_list,
                                    RC_free_dep);
        }
    }
  
  master_dep_list = List_reset(master_dep_list);
}


/*##########################################################
 * Determine if any anit-deps exist between two ops
 ##########################################################*/

int
RC_anti_dep_skip (L_Oper * op)
{
  if (op->opc == Lop_DEFINE ||
      op->opc == Lop_PROLOGUE ||
      op->opc == Lop_EPILOGUE ||
      op->opc == Lop_ALLOC ||
      op->opc == Lop_NO_OP ||
      op->opc == Lop_CHECK ||
      L_subroutine_return_opcode (op) || L_spill_code (op))
    return 1;

  return 0;
}

void
RC_generate_anti_deps_for_ops (L_Oper * use_op, L_Oper * def_op)
{
  /* FORM
     use_op:  opA r1 = r2 , ...
     def_op:  opB r2 = ...
     if def(r1) in opA reaches opB then opB is killing r2 
     that existed for opA's use (anti-dependence)
   */

  L_Oper *use_dest_op = NULL;
  L_Operand *use_dest = NULL;
  int s = 0;
  int d = 0;

  if (!use_op || !def_op)
    L_punt ("RC_generate_anti_deps_for_ops: an op is null\n");

  /*
   * For recovery code, special defines are added before
   *  any ops that have no dests 
   */
  use_dest_op = use_op;
  use_dest = use_op->dest[0];
  if (!use_dest)
    {
      if (!use_op->prev_op)
        {
          L_print_oper (stdout, use_op);
          L_punt
            ("RC_generate_anti_deps_for_ops: no op before destless op\n");
        }
      use_dest_op = use_op->prev_op;
      use_dest = use_op->prev_op->dest[0];
      if (!use_dest)
        {
          L_print_oper (stdout, use_op);
          L_punt
            ("RC_generate_anti_deps_for_ops: op before destless op has "
             "no dest\n");
        }
    }


  /*
   *  Look for anti-deps in recovery code
   */

#if 0
  /* Skip anti-dep on postinc (handled elsewhere) */
  if ((use_op == def_op) &&
      (L_preincrement_load_opcode (use_op) ||
       L_postincrement_load_opcode (use_op)))
    return;
#endif

  /* Does op even reach */
  if (!L_in_oper_RIN_set (def_op, use_dest_op, use_dest))
    {
      /* Does not reach, skip if:
         - use_ops are different
       */
      if (use_op != def_op)
        return;
    }


  /* Skip repeats */
  if (List_member (RC_FLOW_INFO (def_op)->anti_use_op_list, use_op))
    return;

  for (s = 0; s < L_max_src_operand; s++)
    {
      if (!use_op->src[s])
        continue;

      for (d = 0; d < L_max_dest_operand; d++)
        {
          if (!def_op->dest[d])
            continue;

          if (L_same_operand (use_op->src[s], def_op->dest[d]))
            {
              RC_add_anti_dep (def_op, use_op, use_op->src[s]);

              /* Found one dep, so leave */
              return;
            }
        }
    }

  return;
}



/*##########################################################
 *  Fill in all flow dependences into flow info structures
 *   this includes spill->fill deps
 ##########################################################*/
void
RC_generate_global_flow_deps (L_Func * fn)
{
  L_Cb *cb = NULL;
  L_Oper *op = NULL;
  L_Oper *dep_op = NULL;
  int num_uses = 0;
  int i = 0;
  int u = 0;
  int *buffer = NULL;
  RC_flow_info *flow_info = NULL;
  Set uses = NULL;
#if 0
  Set tmp_uses = NULL;
  int offset = 0;
  List oplist = NULL;
  L_Oper *spill_op = NULL;
  L_Oper *fill_op = NULL;
#endif
  L_Attr *attr = NULL;

#if 0
  HashTable ldhash = NULL;
  HashTable sthash = NULL;
  ldhash = HashTable_create (256);
  sthash = HashTable_create (256);
#endif

  /***************************************************
   *  FLOW DEPENDENCES
   ***************************************************/
  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      for (op = cb->first_op; op; op = op->next_op)
        {
#if 0
          /* 
           * SPILLS AND FILLS
           */
          if (L_spill_code (op))
            {
              L_spill_offset (op, &offset);
              if (L_general_load_opcode (op))
                {
                  /* Add to list */
                  oplist = (List) HashTable_find_or_null (ldhash, offset);
                  if (!oplist)
                    HashTable_insert (ldhash, offset, oplist);
                  oplist = List_insert_last (oplist, op);
                  HashTable_update (ldhash, offset, oplist);

                  /* Draw deps from all spills to that offset */
                  oplist = (List) HashTable_find_or_null (sthash, offset);
                  for (List_start (oplist); (spill_op = List_next (oplist));)
                    {
                      RC_add_flow_dep (spill_op, op);
                    }
                }
              else if (L_general_store_opcode (op))
                {
                  /* Add to list */
                  oplist = (List) HashTable_find_or_null (sthash, offset);
                  if (!oplist)
                    HashTable_insert (sthash, offset, oplist);
                  oplist = List_insert_last (oplist, op);
                  HashTable_update (sthash, offset, oplist);

                  /* Draw deps to all fills from that offset */
                  oplist = (List) HashTable_find_or_null (ldhash, offset);
                  for (List_start (oplist); (fill_op = List_next (oplist));)
                    {
                      RC_add_flow_dep (op, fill_op);
                    }
                }
            }
#endif

          /* 
           * NORMAL REACH DEF : Get all uses of dests of oper 
           */
          uses = NULL;
          for (i = 0; i < L_max_dest_operand; i++)
            {
              if (!op->dest[i])
                continue;

              uses = L_get_oper_ROUT_using_opers (op, op->dest[i]);

              flow_info = RC_FLOW_INFO (op);
              num_uses = Set_size (uses);
              if (num_uses <= 0)
                continue;

              buffer = (int *) Lcode_malloc (sizeof (int) * num_uses);
              Set_2array (uses, buffer);
              for (u = 0; u < num_uses; u++)
                {
                  dep_op = L_oper_hash_tbl_find_oper (fn->oper_hash_tbl,
                                                      buffer[u]);
#if 0
                  if (!dep_op)
                    L_punt ("generate_global_flow_deps: invalid opid %d\n",
                            buffer[i]);
#else
		  if (!dep_op)
		    continue;
#endif

                  RC_add_flow_dep (op, dep_op, op->dest[i]);
                }
              Lcode_free (buffer);
              uses = Set_dispose (uses);
            }

          if (L_subroutine_call_opcode (op))
            {
              uses = NULL;
              attr = L_find_attr (op->attr, "ret");
              for (i = 0; i < attr->max_field; i++)
                {
                  if (!attr->field[i])
                    continue;

                  uses = L_get_oper_ROUT_using_opers (op, attr->field[i]);

                  flow_info = RC_FLOW_INFO (op);
                  num_uses = Set_size (uses);
                  if (num_uses <= 0)
                    continue;

                  buffer = (int *) Lcode_malloc (sizeof (int) * num_uses);
                  Set_2array (uses, buffer);
                  for (u = 0; u < num_uses; u++)
                    {
                      dep_op = L_oper_hash_tbl_find_oper (fn->oper_hash_tbl,
                                                          buffer[u]);
                      if (!dep_op)
                        L_punt
                          ("generate_global_flow_deps: invalid opid %d\n",
                           buffer[i]);

                      RC_add_flow_dep (op, dep_op, attr->field[i]);
                    }
                  Lcode_free (buffer);
                  uses = Set_dispose (uses);
                }
            }


        }                       /*op */
    }                           /*cb */

#if 0
  HashTable_free_func (ldhash, (void (*)(void *)) List_reset);
  HashTable_free_func (sthash, (void (*)(void *)) List_reset);
#endif
}


void
RC_generate_global_anti_deps (L_Func * fn)
{
  L_Cb *cb1 = NULL;
  L_Oper *op1 = NULL;
  L_Cb *cb2 = NULL;
  L_Oper *op2 = NULL;

  /***************************************************
   *  ANTI DEPENDENCES
   ***************************************************/
  for (cb1 = fn->first_cb; cb1; cb1 = cb1->next_cb)
    {
      for (op1 = cb1->first_op; op1; op1 = op1->next_op)
        {
          if (RC_anti_dep_skip (op1))
            continue;
          for (cb2 = fn->first_cb; cb2; cb2 = cb2->next_cb)
            {
              for (op2 = cb2->first_op; op2; op2 = op2->next_op)
                {
                  if (RC_anti_dep_skip (op2))
                    continue;
                  RC_generate_anti_deps_for_ops (op1, op2);
                }
            }
        }
    }
}

void
RC_add_def_using(L_Oper *using_op, L_Oper *to_op, L_Operand *to_oprd)
{
  PF_OPERAND   *pf_operand = NULL;
  RC_flow_info *using_info = NULL;
  RC_flow_info *to_info    = NULL;
  Set           RIN        = NULL;
  int          *buffer     = NULL;
  int           num_rin, i, pf_oprd_id;

  if (!(using_info = RC_FLOW_INFO(using_op)))
    L_punt("RC_add_def_using: using_op has no flow info\n");
  
  if (!(to_info = RC_FLOW_INFO(to_op)))
    {
      to_op->ext = RC_new_flow_info ();
      to_info = RC_FLOW_INFO(to_op);
      to_info->cb = using_info->cb;      
    }

  pf_oprd_id = L_REG_MAC_INDEX (to_oprd);
  
  RIN = L_get_oper_RIN_set (using_op);
  num_rin = Set_size (RIN);
  if (num_rin <= 0)
    L_punt("RC_add_def_using: using_op has no uses");

  buffer = (int *) Lcode_malloc (sizeof (int) * num_rin);
  Set_2array (RIN, buffer);
  for (i = 0; i < num_rin; i++)
    {
      pf_operand =
	(PF_OPERAND *) HashTable_find_or_null (PF_default_flow->hash_pf_operand,
					       buffer[i]);
      if (!pf_operand)
	L_punt("RC_add_def_using: could not find pf_operand for pf_operand id %d\n",
	       buffer[i]);

      if (pf_operand->reg == pf_oprd_id)
	{
	  printf("Adding dependence: op%d --> op%d\n",
		 pf_operand->pf_oper->oper->id, to_op->id);
	  RC_add_flow_dep (pf_operand->pf_oper->oper, to_op, to_oprd);
	}
    }
  Lcode_free (buffer);
}



/*
 * Recovery code builders
 */





void
RC_InitCbInfo(L_Func *fn)
{
  L_Cb * cb = NULL;

  for (cb=fn->first_cb; cb; cb=cb->next_cb)
    {
      if (cb->ext)
	L_punt("RC_InitCbInfo: not null\n");
      cb->ext = (RC_cb_info*)calloc(sizeof(RC_cb_info),1);
    }
}

void
RC_FreeCbInfo(L_Func *fn)
{
  L_Cb       *cb = NULL;
  RC_cb_info *cb_info = NULL;

  for (cb=fn->first_cb; cb; cb=cb->next_cb)
    {
      cb_info = RC_CB_INFO(cb);
      if (!cb_info)
	continue;
      Set_dispose(cb_info->valid_cbs);
      List_reset(cb_info->chk_history);

      free(cb->ext);
      cb->ext = NULL;
    }
}

Set 
RC_GetValidCbs(L_Cb *cb)
{
  return (RC_CB_INFO(cb)->valid_cbs);
}

void
RC_PrintValidCbs(L_Func *fn, List cb_list)
{
  L_Cb  *cb = NULL;
  Set    s  = NULL;

  for (cb=fn->first_cb; cb; cb=cb->next_cb)
    {
      s = (RC_CB_INFO(cb)->valid_cbs);
      if (List_member(cb_list,cb))
	{
	  printf("CB %4d: ",cb->id);
	  Set_print(stdout,"",s);
	}
    }
}

void 
RC_AddValidCb(L_Cb *cb, L_Cb *vcb)
{
  RC_cb_info *cb_info = NULL;
  
  cb_info = RC_CB_INFO(cb);  
  cb_info->valid_cbs = Set_add(cb_info->valid_cbs,
			       vcb->id);
}

void 
RC_UnionValidCb(L_Cb *cb, L_Cb *vcb)
{
  RC_cb_info *cb_info = NULL;
  RC_cb_info *vcb_info = NULL;

  cb_info = RC_CB_INFO(cb);  
  vcb_info = RC_CB_INFO(vcb);  
  
  
  cb_info->valid_cbs = Set_union_acc(cb_info->valid_cbs,
				     vcb_info->valid_cbs);
}

int
RC_IsValidControl(L_Cb *from_cb, L_Oper *from_op,
		  L_Cb *to_cb, L_Oper *to_op)
{
  if (to_cb != from_cb)
    {
      Set s = (RC_CB_INFO(from_cb)->valid_cbs);

      /* This is an inter-cb check
       */
      if (Set_in(s, to_cb->id))
	return 1;

      return 0;
    }
  else
    {
      L_Oper *op = NULL;
      /* This is an intra-cb check
       */
      for (op=from_op; op; op=op->next_op)
	{
	  if (op == to_op)
	    return 1;
	}
      for (op=from_op; op; op=op->prev_op)
	{
	  if (op == to_op)
	    return 0;
	}
      L_punt("RC_IsValidControl: Error with to_op/from_op\n");
    }

  return 0;
}

/*##########################################################
 *  Recursive helper for RC_cbs_reachable_from
 ##########################################################*/

/* These defines just make everything more readable 
 * INPROGRESS  : L_CB_RESERVED_TEMP1 : Cb is currently in path
 * FINISHED    : L_CB_RESERVED_TEMP2 : Cb never leads to a check
 * REACHES     : L_CB_RESERVED_TEMP3 : Cb reaches an included check
 */

#define RC_CB_INPROGRESS L_CB_RESERVED_TEMP1
#define RC_CB_FINISHED   L_CB_RESERVED_TEMP2
#define RC_CB_REACHES    L_CB_RESERVED_TEMP3
#define RC_CB_VISITED    L_CB_RESERVED_TEMP4

#define RC_RETURN_FINISHED   1
#define RC_RETURN_INPROGRESS 2
#define RC_RETURN_REACHES    3
#define RC_RETURN_COVERED    4

/* cb       : what cb to search from
 * start_op : op in cb from which to start (NULL = all ops)
 * vop_set  : valid op set, the maximal set of ops for RC 
 *            for included checks
 * ctrl_list: maximal set of required control ops for RC
 */

static void
RC_cbs_reachable_mark_ignore(L_Func *fn, L_Cb *lds_cb, L_Oper *lds_op, List chk_op_list)
{
  L_Flow *flow = NULL;
  L_Oper *op = NULL;
  L_Oper *chk_op = NULL;
  L_Cb   *cb = NULL;
  int change, block_lds_cb;

  /* Do any of the checks preceed the lds it's cb */
  block_lds_cb = 1;
  for (List_start(chk_op_list); (chk_op=(L_Oper*)List_next(chk_op_list));)
    {
      cb = L_oper_hash_tbl_find_cb (fn->oper_hash_tbl, chk_op->id);
      if (cb != lds_cb)
	continue;
      for (op=lds_op; op; op=op->prev_op)
	{
	  if (op == chk_op)
	    {
	      printf("Can't block\n");
	      block_lds_cb = 0;
	      break;
	    }
	}
    }  

  /* Mark everything as not reaching except check node(s) */
  for (cb=fn->last_cb; cb; cb=cb->prev_cb)
    {
      cb->flags = L_SET_BIT_FLAG (cb->flags, RC_CB_FINISHED);
    }
  for (List_start(chk_op_list); (chk_op=(L_Oper*)List_next(chk_op_list));)
    {
      cb = L_oper_hash_tbl_find_cb (fn->oper_hash_tbl, chk_op->id);
      if (block_lds_cb && cb == lds_cb)
	continue;
      cb->flags = L_CLR_BIT_FLAG (cb->flags, RC_CB_FINISHED);
    }

  /* Iterate through cbs marking those that can be lead 
   * to check cb
   */
  do {
    change = 0;
    for (cb=fn->last_cb; cb; cb=cb->prev_cb)
      {
	if (L_EXTRACT_BIT_VAL(cb->flags, RC_CB_FINISHED))
	  continue;
	/* Mark all sources are reaching */
	for (flow=cb->src_flow; flow; flow=flow->next_flow)
	  {
	    if (L_EXTRACT_BIT_VAL(flow->src_cb->flags, RC_CB_FINISHED))
	      {
		if (block_lds_cb && flow->src_cb == lds_cb)
		  continue;
		flow->src_cb->flags = L_CLR_BIT_FLAG (flow->src_cb->flags, 
						      RC_CB_FINISHED);
		change = 1;
	      }
	  }
      }
  } while (change);

  /* Always clear this at the end */
  lds_cb->flags = L_CLR_BIT_FLAG (lds_cb->flags, RC_CB_FINISHED);
}


static void
RC_cbs_reachable_process_path(L_Oper *chk_op, List *pre_chk_list, 
			      List *path_list, Set *vop_set)
{
  L_Oper *op           = NULL;
  static int count = 0;

  /* pre_chk_list: checks before and NOT INCLUDING closing check
   * path_list   : all ops leading up to and INCLUDING closing check
   */
  count++;
  /*printf("Processed path #%d\n",count);*/

  /* Go through every op in the path_list and add any op
   *  NOT subsummed by a previous check to the vop_set
   */
  List_start(*path_list);
  while ((op=(L_Oper*)List_next(*path_list)))
    {
      if (!PG_collective_subsumption(*pre_chk_list, op) &&
	  !PG_disjoint_predicates_ops (chk_op, op))
	{
	  *vop_set = Set_add(*vop_set, op->id);
	  /*printf(" [%d]",op->id);*/
	}
      /*
	else
	printf(" %d",op->id);
      */
    }
  /*printf("\n");*/
}

void
RC_print_list(List in_list, char *str, int id)
{
  L_Oper *op           = NULL;
  L_Cb   *cb = NULL;
  int cbid;

  cbid = -1;
  printf("%21s :",str);
  List_start(in_list);
  while ((op=(L_Oper*)List_next(in_list)))
    {
      cb = L_oper_hash_tbl_find_cb (L_fn->oper_hash_tbl, op->id);
      if (cbid != cb->id)
	{
	  printf(" %d",cb->id);
	  cbid = cb->id;
	}
    }
  printf("(%d) \n",id);
}

static int
RC_cbs_reachable_from_rec2 (L_Cb *cb, L_Oper *start_op, L_Flow *start_flow,
			    L_Oper *lds_op, List chk_op_list, List exclude_chk_op_list, 
			    List *path_list, List *pre_chk_list,
			    Set *vop_set, List *ctrl_list,
			    int level, int oic)
{
  L_Flow *flow         = NULL;
  L_Oper *op           = NULL;
  L_Oper *chk_op       = NULL;
  int reaches, i, inc_chk;
  int chk_ops_added, path_ops_added;
  int exclude, include;
  
  
  if (L_EXTRACT_BIT_VAL (cb->flags, RC_CB_FINISHED))
    {
#if DEBUG_REC
      RC_print_list(*path_list, "RETURN_FINISHED", cb->id);
#endif
      return RC_RETURN_FINISHED;
    }
  
  if (L_EXTRACT_BIT_VAL (cb->flags, RC_CB_INPROGRESS))
    {
#if DEBUG_REC
      RC_print_list(*path_list, "RETURN_INPROGRESS", cb->id);
#endif
      return RC_RETURN_INPROGRESS;
    }  
  if (level != 1)
    cb->flags = L_SET_BIT_FLAG (cb->flags, RC_CB_INPROGRESS);


  if (L_EXTRACT_BIT_VAL (cb->flags, RC_CB_VISITED))
    {
      List   chk_history = (RC_CB_INFO(cb)->chk_history);
      int    same = 1;

      /* Been to this cb before. Is the check history the same?
       */
      if (List_size(chk_history) == List_size(*pre_chk_list))
	{
	  List_start(*pre_chk_list);
	  while ((chk_op=(L_Oper*)List_next(*pre_chk_list)))
	    {
	      if (!List_member(chk_history,chk_op))
		{
		  same = 0;
		  break;
		}
	    }
	  if (same)
	    {
#if DEBUG_REC
	      RC_print_list(*path_list, "RETURN_SAME", cb->id);
#endif
	      return (RC_CB_INFO(cb)->val_history);
	    }
	}
    }
  if (level != 1)
    cb->flags = L_SET_BIT_FLAG (cb->flags, RC_CB_VISITED);
  

  /* This is a key loop that looks for checks and performs
   *   the recursion. 
   ********************************************************/
  reaches = 0;
  path_ops_added = 0;
  chk_ops_added = 0;
  inc_chk = 0;
  flow = start_flow;  
  op = start_op;
  while(op || flow)
    {
      /* Have we looped back around and ran into spec load
       */
      if ((level != 1) && (op == lds_op))
	{
	  /* ## PATH END: No need to go any further */
#if DEBUG_REC
	  printf("LEVEL cb%d op%d\n",cb->id,op->id);
#endif
	  break;
	}
      
      /* Add ops to current path
       */
      if (op)
	{
	  *path_list = List_insert_last(*path_list, op);
	  path_ops_added++;
	}

      /* Examine checks
       */
      if (op && L_check_opcode(op))
	{
	  include = List_member(chk_op_list, op);
	  exclude = List_member(exclude_chk_op_list, op);

	  if (include || exclude)
	    {	    
	      /* Found a relevant check */
	      if (!PG_collective_subsumption(*pre_chk_list, op))
		{
		  /* The check covers a non-redundant predicate
		   */
		  if (include)
		    {
		      /* Path to included check found */
		      RC_AddValidCb(cb, cb);
		      reaches = 1;

		      /* At this point a path to an included check is found
		       * Look through the path for potential ops
		       */
		      RC_cbs_reachable_process_path(op, pre_chk_list, path_list, vop_set);
		      inc_chk++;
#if DEBUG_REC
		      printf("OIC %d INC %d\n",oic, inc_chk);
#endif
		    }

		  /* A new check has been encountered
		   */
		  *pre_chk_list = List_insert_last(*pre_chk_list, op);
		  chk_ops_added++;

		  if (PG_collectively_exhaustive_predicates_ops(*pre_chk_list) ||
		      ((oic - inc_chk) == 0))
		    {
		      /* ## PATH END: No need to go any further */
#if DEBUG_REC
		      printf("EXHAUST cb%d op%d\n",cb->id,op->id);
#endif
		      break;
		    }
		}
	    }
	}

      /* Follow the taken paths of branches or the fallthru path
       */
      if ((op == NULL) || 
	  (L_general_branch_opcode(op) &&
	   !PG_collective_subsumption(*pre_chk_list, op)))
	{
	  switch(RC_cbs_reachable_from_rec2(flow->dst_cb, flow->dst_cb->first_op, 
					    flow->dst_cb->dest_flow,
					    lds_op, chk_op_list, exclude_chk_op_list, 
					    path_list, pre_chk_list,
					    vop_set, ctrl_list, 
					    level+1, (oic - inc_chk)))
	    {
	    case RC_RETURN_FINISHED:
	      /* ## NEXT BR: Path can never reach included check */
	      break;
	    case RC_RETURN_INPROGRESS:
	      /* ## NEXT BR: Path loops back */
	      break;
	    case RC_RETURN_COVERED:
	      /* ## NEXT BR: Path contains only redundant, included checks */
	      break;
	    case RC_RETURN_REACHES:
	      RC_AddValidCb(cb, cb);
	      RC_UnionValidCb(cb, flow->dst_cb);
	      if (op && !List_member(*ctrl_list, op))
		*ctrl_list = List_insert_last(*ctrl_list, op); 
	      reaches = 1;
	      /* ## NEXT BR: Path reaches an included check */
	      break;
	    default:
	      L_punt("RC_cbs_reachable_from_rec: invalid return value");
	    }
	}
      
      /* Shift flow along */
      if ((op == NULL) || 
	  L_is_control_oper(op))
	{
	  flow = flow->next_flow;
	}      

      /* Get next op */
      if (op)
	{
	  op=op->next_op;
	}
    }
  /* LOOP END ****************************************************/
  

  /* Clear inprogress flag 
   */
  cb->flags = L_CLR_BIT_FLAG (cb->flags, RC_CB_INPROGRESS);

  /* Remove ops from current path
   */
  for (i=0; i<path_ops_added; i++)
    {
      List_last(*path_list);
      *path_list = List_delete_current(*path_list);
    }

  /* Remove ops added to pre_chk_list
   */
  for (i=0; i<chk_ops_added; i++)
    {
      List_last(*pre_chk_list);
      *pre_chk_list = List_delete_current(*pre_chk_list);
    }

  /* Erase old list 
   */
  List_reset(RC_CB_INFO(cb)->chk_history);
  RC_CB_INFO(cb)->chk_history = NULL;
  /* Copy pre_chk_list into new list 
   */
  List_start(*pre_chk_list);
  while ((chk_op=(L_Oper*)List_next(*pre_chk_list)))
    {
      RC_CB_INFO(cb)->chk_history = 
	List_insert_last(RC_CB_INFO(cb)->chk_history, chk_op);
    }
    
  /* Was there a useful check reached along some path 
   */
  if (reaches)
    {
      cb->flags = L_SET_BIT_FLAG (cb->flags, RC_CB_REACHES);
#if DEBUG_REC
      RC_print_list(*path_list, "RETURN_REACHES", cb->id);
#endif
      RC_CB_INFO(cb)->val_history = RC_RETURN_REACHES;
      return RC_RETURN_REACHES;
    }
  
  /* No checks or only excluded/redundant checks encountered
   */
#if DEBUG_REC
  RC_print_list(*path_list, "RETURN_COVERED", cb->id);
#endif
  RC_CB_INFO(cb)->val_history = RC_RETURN_COVERED;
  return RC_RETURN_COVERED;
}



/*##########################################################
 *  Given a start_cb+op, find the set of cbs that form a path 
 *    from start_cb+op to check_cb without going back through 
 *    start_cb or check_cb.
 ##########################################################*/

void
RC_cbs_reachable_from2 (L_Func * fn, 
			L_Cb * start_cb, L_Oper * start_op,
			List chk_op_list, List exclude_chk_op_list, 
			Set *vop_set, List *ctrl_list, List *cb_list)
{
  List    pre_chk_list        = NULL;
  List    path_list           = NULL;
  L_Cb   *cb                  = NULL;
  L_Cb   *rc_cb               = NULL;
  L_Oper *op                  = NULL;
  L_Flow *start_flow          = NULL;

#ifdef RC_DEBUG_INFO
  printf ("Step ## 1 ##\n");
  printf ("Load %4d:%4d -> \n",
	  start_cb->id, start_op->id);
  List_start(chk_op_list);
  while((op=List_next(chk_op_list)))
    {
      L_Cb *cb = L_oper_hash_tbl_find_cb (fn->oper_hash_tbl, op->id);
      printf ("     %4s:%4s -> Check op%4d cb%4d\n",
	      "", "", op->id, cb->id); 
    }
#endif
  
  if (List_size(chk_op_list) == 0)
    L_punt("RC_cbs_reachable_from: no check ops provided\n");
  if (!start_op)
    L_punt("RC_cbs_reachable_from: start_op can't be NULL\n");

  RC_InitCbInfo(fn);
  pre_chk_list = NULL;
  path_list = NULL;

  /* Clear flags */
  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      cb->flags = L_CLR_BIT_FLAG (cb->flags, (RC_CB_INPROGRESS | 
					      RC_CB_FINISHED |
					      RC_CB_REACHES |
					      RC_CB_VISITED));
    }
  RC_cbs_reachable_mark_ignore(fn, start_cb, start_op, chk_op_list);
  
  /* Find starting flow */
  start_flow = start_cb->dest_flow;
  for (op=start_cb->first_op; (op && (op!=start_op)); op=op->next_op)
    {
      if (L_is_control_oper (op))
	start_flow = start_flow->next_flow;
    }
  if (op != start_op)
    L_punt ("RC_cbs_reachable_from2: start_op not found\n");
  
  /* Run the search */
  RC_cbs_reachable_from_rec2 (start_cb, start_op, start_flow,
			      start_op, chk_op_list, exclude_chk_op_list, 
			      &path_list, &pre_chk_list,
			      vop_set, ctrl_list, 1, 
			      List_size(chk_op_list));
  
  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      /* Is the cb a reachable one */
      if (L_EXTRACT_BIT_VAL (cb->flags, RC_CB_REACHES))
	{
	  *cb_list = 
	    List_insert_last (*cb_list, cb);
	}
    }
  
  /* Clear flags */
  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      cb->flags = L_CLR_BIT_FLAG (cb->flags, (RC_CB_INPROGRESS | 
					      RC_CB_FINISHED |
					      RC_CB_REACHES));
    }
  

#ifdef RC_DEBUG_INFO
  Set_print(stdout,"VOPS ",*vop_set);

  printf("    %4s:%4s : CTRL OPS = ","","");
  for (List_start (*ctrl_list); (op = (L_Oper *) List_next (*ctrl_list));)
    {
      printf ("%d ", op->id);
    }
  printf ("\n");

  printf("    %4s:%4s : CBS = ","","");
  for (List_start (*cb_list); (rc_cb = (L_Cb *) List_next (*cb_list));)
    {
      printf ("%d ", rc_cb->id);
    }
  printf ("\n");

  RC_PrintValidCbs(fn, *cb_list);
#endif

}





/*##########################################################
 ##########################################################*/
void
RC_recombine (L_Func * fn)
{
  L_Cb *cb = NULL;
  int merged = 0;

#ifdef RC_DEBUG_INFO
  printf ("---- Recombination of RC  ----\n");
#endif

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      /* 
       * Do not process into RC cbs
       ****************************/
      if (L_find_attr (cb->attr, "rc"))
        continue;

      if (!cb->first_op)
        continue;

      if (LT_is_template_op (cb->first_op))
        L_punt ("RC_recombine: recombination of bundled code unsupported");


      merged = 1;
      while (merged && cb->last_op->opc == Lop_CHECK)
        {
          if (!cb->next_cb)
            L_punt ("RC_recombine: cb with check should have succ");

          merged = RC_join_cbs (fn, cb, cb->next_cb);
        }
    }

#if 0
  PG_setup_pred_graph (fn);
  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      if (!L_find_attr(cb->attr,"rc"))
	L_check_cb(fn, cb);
    }
#endif
}


/*##########################################################
 ##########################################################*/
L_Cb* 
RC_find_rc_end (L_Cb *rc_cb)
{
  L_Cb   *cb = NULL;
  L_Oper *op = NULL;
  L_Attr *attr = NULL;
  int rc_id = 0;

  if (!rc_cb)
    L_punt("RC_find_rc_end: rc_cb is NULL\n");

  attr = L_find_attr(rc_cb->attr,"rc");
  rc_id = attr->field[1]->value.i;

  /* Look forward for the end cb
   */
  for (cb=rc_cb; cb; cb=cb->next_cb)
    {
      for (op=cb->first_op; op; op=op->next_op)
	{
	  if (op->opc != Lop_DEFINE)
	    continue;

	  if ((attr=L_find_attr(op->attr,"rcend")))
	    {
	      if (attr->field[0]->value.i == rc_id)
		return cb;
	      goto RC_find_rc_end_1;
	    }
	}
    }
  
  /* Look backward for the end cb
   */  
 RC_find_rc_end_1:
  for (cb=rc_cb; cb; cb=cb->prev_cb)
    {
      for (op=cb->first_op; op; op=op->next_op)
	{
	  if (op->opc != Lop_DEFINE)
	    continue;
	  
	  if ((attr=L_find_attr(op->attr,"rcend")))
	    {
	      if (attr->field[0]->value.i == rc_id)
		return cb;
	      goto RC_find_rc_end_2;
	    }
	}
    }

 RC_find_rc_end_2:
  L_punt("RC_find_rc_end: could not find rcend for rc_cb %d\n",
	 rc_cb->id);
  return NULL;
}


/*##########################################################
 * Makes checks always end a cb, takes into account bundles
 ##########################################################*/
void
RC_split_around_checks (L_Func * fn, int sched)
{
  L_Cb   *cb          = NULL;
  L_Cb   *rc_cb       = NULL;
  L_Cb   *end_cb      = NULL;
  L_Oper *op          = NULL;
  L_Oper *chk_op      = NULL;
  L_Oper *tl          = NULL;
  L_Oper *new_op      = NULL;
  int     mask        = 0;
  int     are_bundles = 0;

#ifdef RC_DEBUG_INFO
  printf ("---- Split of RC cbs ----\n");
#endif

  /***************************************************
   * Move ops after orig_check bundle into separate cbs
   *   all op->id must stay the same
   * Cbs can only be separated at template boundries
   *   so some compensation code may need to be added
   *   to the recovery code before the rc return jump
   ***************************************************/
  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      /* 
       * Do not process into RC cbs
       ****************************/
      if (L_find_attr (cb->attr, "rc"))
        continue;

      if (!cb->first_op)
        continue;

      op = cb->first_op;
      are_bundles = 0;
      while (op->opc == Lop_DEFINE)
	{
	  if (LT_is_template_op (op))
	    {
	      are_bundles = 1;
	      break;
	    }
	  op = op->next_op;
	}

      /* 
       * Look for a check 
       ****************************/
      for (op = cb->first_op; op; op = op->next_op)
        {
          /* Look for a check */
          if (op->opc != Lop_CHECK)
            continue;

          break;
        }

      /* Was a check found or 
       * just the end of a cb?
       ****************************/
      if (!op)
        {
          continue;
        }
      chk_op = op;

      /*
       * Bundling issues
       ****************************/
      if (are_bundles)
        {
          /*
           * Find template info for op and make sure
           * last op of cb has stop bit at the end
           ****************************/
          tl = op;
          while (tl && !LT_is_template_op (tl))
            tl = tl->prev_op;
          if (!tl)
            L_punt ("RC_split_around_checks: no template found\n");
          mask = LT_get_stop_bit_mask (tl);
          if ((mask & S_AFTER_3RD) == 0)
            {
              LT_set_stop_bit_mask (tl, mask | S_AFTER_3RD);
            }

          /* Move until the end of the template is found
           *  but keep initial position
           ****************************/
          while (op && !LT_is_template_op (op))
	    op = op->next_op;
        }
      else
        {
          /* Go to op after check */
          op = op->next_op;
        }

      /* Now split the cb after op if not
       *  already at the end of the cb
       ****************************/
      if (op)
        {
          RC_split_cb_after (fn, cb, op);
        }


      /*
       * More Bundling issues
       ****************************/
      if (are_bundles)
        {
          /*  Go back through the template and look for checks
           *   add on any ops that follow the check within the template
           ****************************/
          for (op = chk_op; (op && !LT_is_template_op (op)); op = op->next_op)
            {
              if (op->opc != Lop_CHECK)
                continue;

              if (!op->src[1] || !L_is_cb (op->src[1]))
                L_punt
                  ("RC_split_around_checks: check oper does not "
                   "have a src[1] or invalid\n");

              rc_cb = op->src[1]->value.cb;
	      end_cb = RC_find_rc_end(rc_cb);
	      
	      /* Make sure last bundle has stop bit
	       */
	      tl = end_cb->last_op;
	      while (tl && !LT_is_template_op (tl))
		tl = tl->prev_op;
	      if (tl)
		{
		  mask = LT_get_stop_bit_mask (tl);
		  if ((mask & S_AFTER_3RD) == 0)
		    {
		      LT_set_stop_bit_mask (tl, mask | S_AFTER_3RD);
		    }
		}

              /* Add all ops in template following check 
               * to end of RC
               ********************/
              for (chk_op = op->next_op;
                   (chk_op && !LT_is_template_op (chk_op));
                   chk_op = chk_op->next_op)
                {
                  new_op = RC_copy_op (chk_op, NULL);

                  L_insert_oper_after (end_cb, end_cb->last_op, new_op);
                  RC_add_flow_for_op (end_cb, new_op);
                  if (SMH_kapi_knobs_ptr && sched)
                    SMH_bundle_single (SMH_kapi_knobs_ptr, end_cb, new_op);
                }
            }
        }

    }                           /*cbs */

#if 0  
  PG_setup_pred_graph (fn);
  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      if (!L_find_attr(cb->attr,"rc"))
	L_check_cb(fn, cb);
    }
#endif
}


/*##########################################################
 ##########################################################*/
void
RC_remove_rc_return_jumps (L_Func * fn)
{
  L_Cb   *cb = NULL;
  L_Cb   *target_cb = NULL;
  L_Cb   *end_cb = NULL;
  L_Oper *op = NULL;

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      /* 
       * Do not process into RC cbs
       ****************************/
      if (L_find_attr (cb->attr, "rc"))
        continue;

      for (op = cb->first_op; op; op = op->next_op)
        {
          if (op->opc != Lop_CHECK)
            continue;

          if (!op->src[1] || !L_is_cb (op->src[1]))
            L_punt ("RC_remove_rc_return_jumps: src[1] missing or invalid\n");

          target_cb = op->src[1]->value.cb;
	  end_cb = RC_find_rc_end(target_cb);

          if (!L_find_attr (end_cb->last_op->attr, "rcjmp"))
            continue;

          L_delete_complete_oper (end_cb, end_cb->last_op);
        }
    }
}


/*##########################################################
 * Add a jump at the end of each RC block 
 ##########################################################*/
void
RC_add_rc_return_jumps (L_Func * fn, int sched)
{
  L_Cb *cb = NULL;
  L_Cb *target_cb = NULL;
  L_Cb *end_cb = NULL;
  L_Oper *op = NULL;
  L_Oper *new_op = NULL;
  L_Attr *attr = NULL;

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      /* 
       * Do not process into RC cbs
       ****************************/
      if (L_find_attr (cb->attr, "rc"))
        continue;

      for (op = cb->first_op; op; op = op->next_op)
        {
          if (op->opc != Lop_CHECK)
            continue;

          if (!op->src[1] || !L_is_cb (op->src[1]))
            L_punt ("RC_add_rc_return_jumps: src[1] missing or invalid\n");

          target_cb = op->src[1]->value.cb;

          if (!cb->next_cb)
            L_punt ("RC_add_rc_return_jumps: no cb to jump to\n");

	  end_cb = RC_find_rc_end(target_cb);

          /* Don't add an uncond jump after a pre-existing one
           */
          if (L_uncond_branch (end_cb->last_op))
            continue;

          new_op = L_create_new_op (Lop_JUMP);
          new_op->src[0] = L_new_cb_operand (cb->next_cb);
          attr = L_new_attr ("rcjmp", 0);
          new_op->attr = L_concat_attr (new_op->attr, attr);

          L_insert_oper_after (end_cb, end_cb->last_op, new_op);
          RC_add_flow_for_op (end_cb, new_op);
          S_machine_jump (new_op);

          if (SMH_kapi_knobs_ptr && sched)
            SMH_bundle_single (SMH_kapi_knobs_ptr, end_cb, new_op);
        }
    }
}


void
RC_add_sink_to_rc_blocks (L_Func * fn)
{
  L_Cb *cb = NULL;
  L_Cb *target_cb = NULL;
  L_Cb *end_cb = NULL;
  L_Oper *op = NULL;
  L_Oper *new_op = NULL;
#if 0
  L_Attr *oprd_attr = NULL;
  int i = 0;
#endif

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      /* 
       * Do not process into RC cbs
       ****************************/
      if (L_find_attr (cb->attr, "rc"))
        continue;

      for (op = cb->first_op; op; op = op->next_op)
        {
          if (op->opc != Lop_CHECK)
            continue;

          if (!op->src[1] || !L_is_cb (op->src[1]))
            L_punt ("RC_add_sink_to_rc_blocks: src[1] missing or invalid\n");

          target_cb = op->src[1]->value.cb;
	  end_cb = RC_find_rc_end(target_cb);

          if (L_subroutine_return_opcode (end_cb->last_op) ||
              L_uncond_branch (target_cb->last_op))
            continue;

#if 0
          oprd_attr = L_find_attr (end_cb->attr, "rclast");
          if (!oprd_attr)
            L_punt ("RC_add_sink_to_rc_blocks: attr rclast not found");

          for (i = 0; i < oprd_attr->max_field; i++)
            {
              new_op = L_create_new_op (Lop_DEFINE);
              new_op->src[0] = L_copy_operand (oprd_attr->field[i]);
              L_insert_oper_after (end_cb, end_cb->last_op, new_op);
              attr = L_new_attr ("rcdef", 0);
              new_op->attr = L_concat_attr (new_op->attr, attr);
            }
#endif

          new_op = L_create_new_op (Lop_RTS);
          S_machine_rts (new_op);
          L_insert_oper_after (end_cb, end_cb->last_op, new_op);
        }
    }
}


void
RC_remove_sink_from_rc_blocks (L_Func * fn, int sched)
{
  L_Cb *cb = NULL;
  L_Cb *target_cb = NULL;
  L_Cb *end_cb = NULL;
  L_Oper *op = NULL;
  L_Oper *rc_op = NULL;
  L_Oper *next_op = NULL;
  L_Oper *last_tl = NULL;

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      /* 
       * Do not process into RC cbs
       ****************************/
      if (L_find_attr (cb->attr, "rc"))
        continue;

      for (op = cb->first_op; op; op = op->next_op)
        {
          if (op->opc != Lop_CHECK)
            continue;

          if (!op->src[1] || !L_is_cb (op->src[1]))
            L_punt
              ("RC_remove_sink_from_rc_blocks: src[1] missing or invalid\n");

          target_cb = op->src[1]->value.cb;
	  end_cb = RC_find_rc_end(target_cb);

          if (!L_subroutine_return_opcode (end_cb->last_op))
            continue;

          /* Delete defines and look for last template */
          last_tl = NULL;
          for (rc_op = end_cb->first_op; rc_op; rc_op = next_op)
            {
              next_op = rc_op->next_op;
              if (L_find_attr (rc_op->attr, "rcdef"))
                L_delete_oper (end_cb, rc_op);
              else if (LT_is_template_op (rc_op))
                last_tl = rc_op;
            }

          if (!last_tl)
            L_punt ("RC_remove_sink_from_rc_blocks: no templates");

          /* Delete no_ops */
          for (rc_op = last_tl; rc_op; rc_op = next_op)
            {
              next_op = rc_op->next_op;
              if (rc_op->opc == Lop_NO_OP)
                L_delete_oper (end_cb, rc_op);
              else if (L_subroutine_return_opcode (rc_op))
                L_delete_oper (end_cb, rc_op);
            }

          /* Delete template define */
          rc_op = last_tl->next_op;
          L_delete_oper (end_cb, last_tl);
          last_tl = NULL;

          while (rc_op)
            {
              next_op = rc_op->next_op;
              if (SMH_kapi_knobs_ptr && sched)
                SMH_bundle_single (SMH_kapi_knobs_ptr, end_cb, rc_op);
              rc_op = next_op;
            }
        }
    }
}


/*##########################################################
 * Determines if op dependent on cur_op is valid for
 *  adding to the recovery code.
 ###########################################################*/
int
RC_valid_dependence (L_Cb * cur_cb, L_Oper * cur_op,
                     L_Cb * dep_cb, L_Oper * dep_op,
                     L_Cb * lds_cb, L_Oper * lds_op,
                     L_Cb * chk_cb, L_Oper * chk_op)
{
  L_Oper *op = NULL;

  /* We now have a dep_op and must screen out a cases
   * 1) dep_op and cur_op are in lds_cb
   *      - dep_op must FOLLOW cur_op or else 
   *        flow is from a back edge through lds
   * 2) dep_op is in lds_cb but cur_op is not
   *      - NEVER include these because it must be
   *        from a backedge into lds_cb
   * 3) dep_op is in chk_cb
   *      - dep_op must PRECEED chk_op
   */

  /* Case 1) */
  if (dep_cb == lds_cb && cur_cb == lds_cb)
    {
      for (op = cur_op; op; op = op->next_op)
        {
	  if (op != cur_op && op == lds_op)
	    {
	      /* Ran into lds, dep not valid */
	      return 0;
	    }
          if (op == dep_op)
            break;
        }
      if (!op)
        {
          /*printf("C1: Excluding op %d\n",dep_op->id); */
          return 0;
        }
    }

  /* Case 2) */
  if (dep_cb == lds_cb && cur_cb != lds_cb)
    {
      /* printf("C2: Excluding op %d\n",dep_op->id); */
      return 0;
    }

  /* Case 3) */
  if (dep_cb == chk_cb)
    {
      for (op = chk_op; op; op = op->prev_op)
        {
          if (op == dep_op)
            break;
        }
      if (!op)
        {
          /*printf("C3: Excluding op %d\n",dep_op->id); */
          return 0;
        }
    }

  /*
   *  The source op should not be after the check
   *   or before the load either
   */
  /* Case 4) */
  if (cur_cb == chk_cb)
    {
      for (op = chk_op->next_op; op; op = op->next_op)
        {
          if (op == cur_op)
	    {
	      printf("NEW EXCLUSION #2\n");
	      return 0;
	    }
        }     
    }
  /* Case 5) */
  if (cur_cb == lds_cb)
    {
      for (op = lds_op->prev_op; op; op = op->prev_op)
        {
          if (op == cur_op)
	    {
	      printf("NEW EXCLUSION #3\n");
	      return 0;
	    }
        }     
    }

  return 1;
}

/*##########################################################
 *
 *##########################################################*/
void
RC_delete_samecb_checks(L_Func * fn)
{
  L_Cb *cb = NULL;
  L_Oper *op1 = NULL;
  L_Oper *op2 = NULL;
  L_Oper *next_op = NULL;
  L_Attr *attr = NULL;
  int id1, id2;

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      for (op1 = cb->first_op; op1; op1 = op1->next_op)
	{
	  if (op1->opc != Lop_CHECK)
	    continue;
	  if (!(attr = L_find_attr (op1->attr, "SPECID")))
	    L_punt ("load_and_checks: Speculative loads must "
		    "have SPECID, op%d\n",
		    op1->id); 
	  id1 = attr->field[0]->value.i;

	  for (op2 = op1->next_op; op2; op2 = next_op)
	    {
	      next_op = op2->next_op;
	      if (op2->opc != Lop_CHECK)
		continue;
	      if (!(attr = L_find_attr (op2->attr, "SPECID")))
		L_punt ("load_and_checks: Speculative loads must "
			"have SPECID, op%d\n",
			op2->id);
	      id2 = attr->field[0]->value.i;
	      
	      if (id1 == id2)
		{
		  /*
		   * If first one is not an obvious superset then
		   *  make a call to PG_superset
		   */
		  if (!op1->pred[0] ||
		      PG_superset_predicate_ops(op1, op2))
		    {
		      printf("EASY: Redundant check found cb %d op %d\n",
			     cb->id, op2->id);
		      L_delete_oper(cb,op2);
		    }
		}
	    }	  
	}
    }
}

/*##########################################################
 *  Finds and pairs up speculative loads and checks
 *  (annotates using RC_INFO structure)
 ###########################################################*/
void
RC_copy_oper_deps(L_Oper *from_op, L_Oper *to_op)
{
  RC_flow_info *from_info = NULL;
  RC_flow_info *to_info   = NULL;
  RC_dep_info  *dep       = NULL;
  /*L_Oper       *dep_op    = NULL;*/

  if (!(from_info = RC_FLOW_INFO(from_op)))
    L_punt("RC_copy_oper_deps: from_op has missing flow info");

  if (!(to_info = RC_FLOW_INFO(to_op)))
    {
      to_op->ext = RC_new_flow_info ();
      to_info = RC_FLOW_INFO(to_op);
      to_info->cb = from_info->cb;      
    }

  for (List_start (from_info->def_op_list);
       (dep = (RC_dep_info *) List_next (from_info->def_op_list));)
    { 
      RC_add_flow_dep(dep->from_op, to_op, dep->oprd);
    }
}


List RC_find_spec_load_and_checks (L_Func * fn)
{
  L_Cb *cb = NULL;
  L_Oper *op = NULL;
  L_Oper *chk_op = NULL;
  L_Oper *lds_op = NULL;
  List check_list = NULL;
  List lds_list = NULL;
  int ctrl_spec = 0;
  int data_spec = 0;
  int fatal_error = 0;
  
  /***************************************************
   * Build a list of all of the checks
   * Build list of all speculative loads
   ***************************************************/
  fprintf (stderr, "---- Find Checks and Spec loads ----\n");
  fprintf (stderr, "Fixing : ");
  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      for (op = cb->first_op; op; op = op->next_op)
        {
          if (op->opc == Lop_CHECK)
            {
              /* 
               * Gather Check opers and verify assumptions
               */
              if (!op->src[0])
                {
                  L_print_oper (stderr, op);
                  L_punt
                    ("RC_find_spec_load_and_checks: check has no src[0]");
                }

              check_list = List_insert_last (check_list, op);
            }
          else if (L_general_load_opcode (op))
            {
              /* 
               * Gather Speculative Load Opers and verify assumptions
               */

              /* Ignore fills and safe label loads
               */
              if (L_spill_code (op))
                continue;

              ctrl_spec = L_EXTRACT_BIT_VAL (op->flags, L_OPER_MASK_PE);
	      
              data_spec = L_EXTRACT_BIT_VAL (op->flags, L_OPER_DATA_SPECULATIVE);
	      
              if (data_spec)
                {
                  L_print_oper (stderr, op);
                  L_punt
                    ("generate_recovery_code: Data Speculative loads "
                     "not tested");
                }

              if (ctrl_spec)
                {
                  lds_list = List_insert_last (lds_list, op);
                }
            }
        }                       /*op */
    }                           /*cb */
  fprintf (stderr, "\n");

  fprintf (stderr, "Num Speculative Loads = %d\n", List_size (lds_list));
  fprintf (stderr, "Num Spec Load Checks  = %d\n", List_size (check_list));


  /**********************************************************
   *  Match up Spec Loads and Checks
   *  Only keep checks where load dest reaches check
   **********************************************************/

#ifdef RC_DEBUG_INFO
  printf ("---- Pairing up Loads and Checks ----\n");
#endif
  for (List_start (check_list); (chk_op = (L_Oper *) List_next (check_list));)
    {
      chk_op->flags = L_CLR_BIT_FLAG (chk_op->flags, L_OPER_RESERVED_TEMP1);
    }

  for (List_start (lds_list); (lds_op = (L_Oper *) List_next (lds_list));)
    {
      RC_flow_info *info     = RC_FLOW_INFO (lds_op);
      L_Attr       *lds_attr = NULL;
      L_Attr       *chk_attr = NULL;
      L_Cb         *chk_cb   = NULL;
      L_Oper       *new_op   = NULL;
      int           c, change_specid;
      int           max_chk;

      if (!(lds_attr = L_find_attr (lds_op->attr, "SPECID")))
        L_punt ("load_and_checks: Speculative loads must have SPECID, op%d\n",
                lds_op->id);
      
      max_chk = 0;
      for (List_start (check_list);
	   (chk_op = (L_Oper *) List_next (check_list));)
        {
          if (!(chk_attr = L_find_attr (chk_op->attr, "SPECID")))
            L_punt ("load_and_checks: Checks must have SPECID, op%d\n",
                    chk_op->id);
          if (lds_attr->field[0]->value.i == chk_attr->field[0]->value.i)
	    max_chk++;
	}
      info->chk_ops = (L_Oper **)calloc(sizeof(L_Oper*),(max_chk+1));
      
      info->chk_num = 0;
      change_specid = 0;
      for (List_start (check_list);
           (chk_op = (L_Oper *) List_next (check_list));)
        {
	  chk_attr = L_find_attr (chk_op->attr, "SPECID");
          if (lds_attr->field[0]->value.i == chk_attr->field[0]->value.i)
            {
	      if (info->chk_num >= max_chk)
		L_punt("load_and_checks: max num chk exceeded\n");
	      
	      /* Def by load reaches check 
	       */
	      info->chk_ops[info->chk_num] = chk_op;
	      info->chk_num++;

	      if (L_EXTRACT_BIT_VAL (chk_op->flags, L_OPER_RESERVED_TEMP1))
		{
		  /* Multiple loads have the same SPECID, copies of checks
		   *  will be needed and a new SPECID assigned
		   */
		  change_specid = 1;
		}

              chk_op->flags =
                L_SET_BIT_FLAG (chk_op->flags, L_OPER_RESERVED_TEMP1);
            }
        }
      if (!info->chk_num)
        {
          fatal_error++;
          fprintf (stderr,
                   "load_and_checks: no checks found for spec load %d\n",
                   lds_op->id);
        }
      if (change_specid)
	{
	  printf("Changing lds %d from SPECID "ITintmaxformat" to %d\n",
		 lds_op->id, lds_attr->field[0]->value.i, fn->max_spec_id);
	  lds_attr->field[0]->value.i = fn->max_spec_id;
	  for (c=0; c<info->chk_num; c++)
	    {
	      chk_op = info->chk_ops[c];
	      chk_cb = L_oper_hash_tbl_find_cb (fn->oper_hash_tbl, chk_op->id);
	      new_op = L_copy_operation(chk_op);
	      chk_attr = L_find_attr (new_op->attr, "SPECID");
	      chk_attr->field[0]->value.i = fn->max_spec_id;
	      RC_copy_oper_deps(chk_op, new_op);
	      L_insert_oper_after(chk_cb,chk_op,new_op);
	      info->chk_ops[c] = new_op;
	      printf("\tAdding check %d\n",new_op->id);
	    }
	  fn->max_spec_id++;
	}
    }

  DB_spit_func(fn,"fn_1");

  for (List_start (check_list); (chk_op = (L_Oper *) List_next (check_list));)
    {
      if (!L_EXTRACT_BIT_VAL (chk_op->flags, L_OPER_RESERVED_TEMP1))
        {
          printf
            ("load_and_checks: no load found for check %d (deleting it)\n",
             chk_op->id);
          cb = L_oper_hash_tbl_find_cb (fn->oper_hash_tbl, chk_op->id);
          RC_delete_oper (cb, chk_op);
        }
    }

  if (fatal_error)
    L_punt ("%d Errors encountered\n", fatal_error);

  check_list = List_reset (check_list);

  return lds_list;
}


/*##########################################################
 * Has a non-spec load been speculated about a check
  ##########################################################*/
List
RC_is_load_spec_above_check (L_Func *fn, List lds_list, L_Oper *cur_op, L_Oper *chk_op)
{
  RC_flow_info *new_info = NULL;
  L_Oper *op = NULL;
  L_Attr *specid_attr = NULL;

  /* Look for Non-spill, Non-spec load 
   */
  if (L_general_load_opcode (cur_op) &&
      !L_spill_code (cur_op) && RC_FLOW_INFO (cur_op)->chk_num == 0)
    {
      new_info = RC_FLOW_INFO (cur_op);

      /* This is a non-speculative load that is a part
         of recovery code. This needs to be made speculative
         and a check added */
      if (!new_info->chk_mark)
        {
          /* New check should follow check for current 
           * spec load 
           */
          new_info->chk_mark = chk_op;
	  master_has_chk_mark_list = 
	    List_insert_last(master_has_chk_mark_list, cur_op);

	  lds_list = List_insert_last (lds_list, cur_op);
	  fprintf (stderr, "Found new spec load op%d check follows op%d\n",
		   cur_op->id, new_info->chk_mark->id);

	  L_mark_oper_speculative (cur_op);
	  specid_attr = L_new_attr ("SPECID", 1);
	  L_set_int_attr_field (specid_attr, 0, fn->max_spec_id++);
	  cur_op->attr = L_concat_attr (cur_op->attr, specid_attr);
        }
      else
        {
#if 0
	  L_punt("RC_is_load_spec_above_check: unimplemented");
#else
	  /* THIS IS NOT CORRECT FOR MULTI-CB RC
	     BECAUSE "LATER" IS AMBIGUOUS
	     NEED A MARKER FOR EACH CB */
          /* Is this check later that previous mark 
           */
          for (op = new_info->chk_mark; op; op = op->next_op)
            {
              if (op == chk_op)
                {
                  new_info->chk_mark = chk_op;
                  break;
                }
              if (L_general_branch_opcode (op))
                break;
            }
#endif
	  fprintf (stderr, "Found new chk mark for spec load op%d following op%d\n",
		   cur_op->id, new_info->chk_mark->id);
        }
    }

  return lds_list;
}


/*##########################################################
 * Make a load that should have been spec. speculative
  ##########################################################*/
void
RC_make_load_speculative (L_Func * fn, L_Oper * lds_op)
{
  RC_flow_info *info  = RC_FLOW_INFO (lds_op);
  L_Oper *new_chk_op  = NULL;
  L_Cb   *mark_cb     = NULL;
  L_Attr *specid_attr = NULL;

  /* Update load */
  L_mark_oper_speculative (lds_op);
  specid_attr = L_find_attr(lds_op->attr, "SPECID");
  if (!specid_attr)
    L_punt("RC_make_load_speculative: SPECID not found\n");

  /* Create and insert check */
  new_chk_op = L_create_new_op (Lop_CHECK);
  new_chk_op->flags |= L_OPER_CHECK;
  new_chk_op->attr = L_concat_attr (new_chk_op->attr,
                                    L_copy_attr (specid_attr));
  new_chk_op->src[0] = L_copy_operand (lds_op->dest[0]);
  if (lds_op->pred[0])
    new_chk_op->pred[0] = L_copy_operand (lds_op->pred[0]);
  mark_cb = L_oper_hash_tbl_find_cb(fn->oper_hash_tbl, info->chk_mark->id);
  L_insert_oper_after (mark_cb, info->chk_mark, new_chk_op);

  /* Update recovery code data structure */
  if (info->chk_num != 0)
    L_punt ("RC_make_load_speculative: Something is horribly wrong\n");
  info->chk_num = 1;
  info->chk_ops = (L_Oper **)malloc(sizeof(L_Oper*) * 2);
  info->chk_ops[0] = new_chk_op;

  new_chk_op->ext = RC_new_flow_info ();
  RC_FLOW_INFO (new_chk_op)->cb = info->cb;
  RC_add_flow_dep (lds_op, new_chk_op, new_chk_op->src[0]);

  if (L_EXTRACT_BIT_VAL (lds_op->flags, L_OPER_SAFE_PEI))
    {
      L_warn ("RC_make_load_speculative: Making ld %d masked and not safe\n",
              lds_op->id);
      lds_op->flags = L_CLR_BIT_FLAG (lds_op->flags, L_OPER_SAFE_PEI);
      lds_op->flags = L_SET_BIT_FLAG (lds_op->flags, L_OPER_MASK_PE);
    }
  if (!L_EXTRACT_BIT_VAL (lds_op->flags, L_OPER_MASK_PE))
    L_punt ("RC_make_load_speculative: load %d not masked\n", lds_op->id);
}


/*##########################################################
 * Determine what ops are required in RC
  ##########################################################*/
void
RC_get_ops_for_rc (L_Func *fn, List * op_list, List *dep_ctrl_list,
		   List * rc_list, List * lds_list,
                   List * last_oprd_list, Set vop_set, List cb_list,
                   L_Cb * lds_cb, L_Oper * lds_op,
                   L_Cb * chk_cb, L_Oper * chk_op)
{
  L_Cb *cur_cb = NULL;
  L_Oper *cur_op = NULL;
  RC_flow_info *cur_info = NULL;
  RC_dep_info *dep = NULL;
  L_Oper *dep_op = NULL;
  L_Cb *dep_cb = NULL;
  L_Operand *oprd = NULL;
  int has_dep = 0;

#ifdef RC_DEBUG_INFO
  printf ("Step ## 2.2 ##\n");
#endif

  List_start (*op_list);
  while ((cur_op = (L_Oper *) List_next (*op_list)))
    {
      /* Get next op from op list and Del it from list */
      *op_list = List_delete_current (*op_list);
      cur_info = RC_FLOW_INFO (cur_op);
      cur_cb = cur_info->cb;

      *rc_list = List_insert_last (*rc_list, cur_op);

      *lds_list = RC_is_load_spec_above_check (fn, *lds_list, cur_op, chk_op);

      /* Get ops flow dependent on current op
       */
      has_dep = 0;
      for (List_start (cur_info->use_op_list);
           (dep = (RC_dep_info *) List_next (cur_info->use_op_list));)
        { 
	  /* Has dep been deleted? */
	  if ((!dep->to_op) || (!dep->from_op))
	    continue;

          dep_op = dep->to_op;
          dep_cb = RC_FLOW_INFO (dep_op)->cb;

          if (cur_op == lds_op && L_same_operand (dep->oprd, lds_op->dest[1]))
            continue;

	  /* Is op in the maximal op set
	   */
	  if (!Set_in (vop_set, dep_op->id))
	    continue;
	  
	  /* Is it valid to go from cur_cb/op to dep cb/op 
	   */
	  if (!RC_IsValidControl(cur_cb, cur_op,dep_cb, dep_op))
	    {
	      printf("Excluding op %d\n",dep_op->id);
	      continue;
	    }

	  /* Keep track of control ops */
	  if (L_general_branch_opcode(dep_op) &&
	      !List_member(*dep_ctrl_list, dep_op))
	    *dep_ctrl_list = List_insert_last (*dep_ctrl_list, dep_op);
	    
          /* Has op already been seen */
          if (List_member (*rc_list, dep_op))
            continue;
          if (List_member (*op_list, dep_op))
            continue;
	  
#if 0
          if (!RC_valid_dependence (cur_cb, cur_op,
                                    dep_cb, dep_op,
                                    lds_cb, lds_op, chk_cb, chk_op))
	    {
	      L_punt("VOPSET OVERRIDDEN BY VALDEP op %d -> op %d\n",
		     cur_op->id, dep_op->id);
	    }
#endif
  
          if (dep_op->dest[0])
            has_dep = 1;
          *op_list = List_insert_last (*op_list, dep_op);
        }

      /* When it is time to change what source
       * to check, don't allow predicate dests to 
       * qualify as a dep because predicates don't have
       * NAT bits to check.
      */
      if ((!has_dep) && cur_op->dest[0] &&
          (!RC_oprd_in_list (*last_oprd_list, cur_op->dest[0])))
        *last_oprd_list = List_insert_last (*last_oprd_list, cur_op->dest[0]);
    }

#ifdef RC_DEBUG_INFO
  printf ("     %4s:%4s          %4s:%4s OPS =  ", "", "", "", "");
  List_start (*rc_list);
  while ((cur_op = (L_Oper *) List_next (*rc_list)))
    {
      if (L_is_control_oper (cur_op))
        printf ("B");
      printf ("%d ", cur_op->id);
    }
  printf ("\n");

  printf ("     %4s:%4s          %4s:%4s LAST =  ", "", "", "", "");
  List_start (*last_oprd_list);
  while ((oprd = (L_Operand *) List_next (*last_oprd_list)))
    {
      L_print_operand (stdout, oprd, 0);
    }
  printf ("\n");
#endif

  if (!List_size (*last_oprd_list))
    L_punt ("RC_get_ops_for_rc: nothing defined within RC?");
}


/*##########################################################
 *    If a wired and type predicate(s) is 
 *    in rc_list, all ops in vop_set that potentially
 *    write the pred must be in RC and the pred must
 *    be saved b/f load and restored in RC
 ##########################################################*/
void
RC_add_in_wired_preds(List *rc_list, Set vop_set, 
		      List cb_list, List *conf_list)
{
  L_Oper *op        = NULL;
  L_Cb   *cb        = NULL;
  List    oprd_list = NULL;
  int     d;

  

  /* Find all wired-and predicates in rc_list
   */ 
  List_start(*rc_list);
  while((op=(L_Oper*)List_next(*rc_list)))
    {
      for (d=0; d<=L_max_dest_operand; d++)
	{
	  if (!L_is_pred_register(op->dest[d]))
	    continue;

	  if (L_and_ptype(op->dest[d]->ptype) ||
	      L_or_ptype(op->dest[d]->ptype))
	    {
	      printf("Adding WIRED-AND operand from op %d",op->id);
	      L_print_operand(stdout, op->dest[d], 1);
	      printf("\n");
	      
	      if (RC_oprd_in_list(oprd_list, op->dest[d]))
		continue;
	      oprd_list = List_insert_last(oprd_list, op->dest[d]);
	      *conf_list = List_insert_last(*conf_list, op->dest[d]);	      
	    }
#if 0
	  if (L_or_ptype(op->dest[d]->ptype))
	    {
	      printf("Adding WIRED-OR operand from op %d",op->id);
	      L_print_operand(stdout, op->dest[d], 1);
	      printf("\n");

	      if (RC_oprd_in_list(*conf_list, op->dest[d]))
		continue;
	      *conf_list = List_insert_last(*conf_list, op->dest[d]);	      
	    }
#endif
#if 0
	  {
	    PG_Pred_SSA *pred_ssa = NULL;
	    int         *pred_buf = NULL;
	    int          size;
	    
	    size = Set_size (op->dest[d]->value.pred.ssa->composition);
	    if (size == 1)
	      {
		pred_buf = (int *) Lcode_malloc (sizeof (int) * size);
		Set_2array(op->dest[d]->value.pred.ssa->composition, 
			   pred_buf);

		pred_ssa = (PG_Pred_SSA *)HashTable_find(PG_pred_graph->hash_pgPredSSA, 
							 pred_buf[0]);
		printf("COMPOSITION (%d)\n", pred_ssa->oper->id);

		Lcode_free(pred_buf);
	      }
	    else if (size != 0)
	      {
		Set_print(stdout,"COMPOSITION",
			  op->dest[d]->value.pred.ssa->composition);
		L_punt("not zero\n");
	      }
	  }
#endif
	}      
    }
  
  /*
   * Find all other ops that modify these preds
   */
  List_start(cb_list);
  while((cb=(L_Cb*)List_next(cb_list)))
    {
      for (op=cb->first_op; op; op=op->next_op)
	{
	  if (!Set_in(vop_set, op->id))
	    continue;
	  if (List_member(*rc_list, op))
	    continue;
	  
	  for (d=0; d<=L_max_dest_operand; d++)
	    {
	      if (!L_is_pred_register(op->dest[d]))
		continue;
	      if (!RC_oprd_in_list(oprd_list, op->dest[d]))
		continue;

	      /* Add this op to recovery code
	       */
	      printf("WIRED-AND USE/DEF: Adding op %d\n",
		     op->id);
	      *rc_list = List_insert_last(*rc_list, op);
	    }
	}
    }

  List_reset(oprd_list);
}


/*##########################################################
 * Determine what regs must be preserved given ops in RC
  ##########################################################*/
void
RC_get_prsv_for_rc (List rc_list, List * psrv_list)
{
  L_Cb *cur_cb = NULL;
  L_Oper *cur_op = NULL;
  RC_flow_info *cur_info = NULL;
  RC_dep_info *dep = NULL;
  L_Oper *dep_op = NULL;
  L_Cb *dep_cb = NULL;
  L_Operand *oprd = NULL;

#ifdef RC_DEBUG_INFO
  printf ("Step ## 3.1 ##\n");
#endif

  List_start (rc_list);
  while ((cur_op = (L_Oper *) List_next (rc_list)))
    {
      /* Get next op from op list and Del it from list */
      cur_info = RC_FLOW_INFO (cur_op);
      cur_cb = cur_info->cb;

      for (List_start (cur_info->def_op_list);
           (dep = (RC_dep_info *) List_next (cur_info->def_op_list));)
        {
	  /* Has dep been deleted? */
	  if ((!dep->to_op) || (!dep->from_op))
	    continue;

          dep_op = dep->from_op;
          dep_cb = RC_FLOW_INFO (dep_op)->cb;

          if (List_member (rc_list, dep_op))
            continue;
          if (RC_oprd_in_list (*psrv_list, dep->oprd))
            continue;

          *psrv_list = List_insert_last (*psrv_list, dep->oprd);
        }
    }

#ifdef RC_DEBUG_INFO
  printf ("     %4s:%4s          %4s:%4s PSRV =  ", "", "", "", "");
  List_start (*psrv_list);
  while ((oprd = (L_Operand *) List_next (*psrv_list)))
    {
      L_print_operand (stdout, oprd, 0);
    }
  printf ("\n");
#endif
}


/*##########################################################
 * Contruct the actual recovery code cbs
  ##########################################################*/

int
RC_get_SPECID(L_Oper *op)
{
  L_Attr *attr = NULL;
  
  attr = L_find_attr(op->attr, "SPECID");
  
  return attr->field[0]->value.i;
}

void 
RC_build_rc_add_oper_after(L_Cb *rc_cb, L_Oper *rc_op, 
			   L_Oper *add_op, L_Oper *chk_op)
{
  /* Add the add_op to the rc_cb */
  L_insert_oper_after (rc_cb, rc_op, add_op);
  RC_add_flow_for_op (rc_cb, add_op);	      
}

void
RC_build_rc_cbs (L_Func * fn, List cb_list, List rc_list,
                 List * new_cb_list, List * new_op_list,
                 L_Cb ** start_cb, L_Cb ** end_cb,
                 /*List * conf_list, List * svrt_list,*/
                 L_Cb * lds_cb, L_Oper * lds_op,
                 L_Cb * chk_cb, L_Oper * chk_op)
{
  L_Cb *new_cb = NULL;
  L_Cb *rc_cb = NULL;
  L_Oper *new_op = NULL;
  L_Oper *cur_op = NULL;
  L_Attr *attr = NULL;
  RC_flow_info *info = NULL;
  HashTable map = NULL;
  static int rc_counter = 0;

#ifdef RC_DEBUG_INFO
  printf ("Step ##4##\n");
#endif
  rc_counter ++;
  map = HashTable_create (256);
  for (rc_cb=fn->first_cb; rc_cb; rc_cb=rc_cb->next_cb)
    {
      if (!List_member (cb_list, rc_cb))
	continue;
      
      new_cb = L_create_cb (0.0);
      L_insert_cb_after (fn, fn->last_cb, new_cb);
      attr = L_new_attr ("rc", 2);
      new_cb->attr = L_concat_attr (new_cb->attr, attr);
      L_set_int_attr_field (attr, 0, rc_cb->id);
      L_set_int_attr_field (attr, 1, rc_counter);

      *new_cb_list = List_insert_last (*new_cb_list, new_cb);
      HashTable_insert (map, rc_cb->id, new_cb);
     
      /* Keep track of rc cb where lds copy lives */
      if (rc_cb == lds_cb)
        {
          if (*start_cb)
            {
              L_print_cb (stdout, fn, *start_cb);
              L_punt ("RC_build_rc_cbs: over writing rc start_cb\n");
            }
          *start_cb = new_cb;
          /* Start loop at lds in its cb
           */
          cur_op = lds_op;
        }
      else
        cur_op = rc_cb->first_op;

      for (; cur_op; cur_op = cur_op->next_op)
        {
          if (!List_member (rc_list, cur_op))
            continue;

          /* Perform some error checking
           */
          if (L_subroutine_call_opcode (cur_op) ||
              L_subroutine_return_opcode (cur_op) ||
              L_general_store_opcode (cur_op))
	    {
	      if (cur_op->pred[0])
		L_warn
		  ("##!! RC_build_rc_cbs: JSR, RTS, and STORES should not be "
		   "in recovery code !!##");
	      else
		L_punt
		  ("##!! RC_build_rc_cbs: JSR, RTS, and STORES should not be "
		   "in recovery code !!##");		
	    }

          if (cur_op->opc == Lop_CHECK)
            {
              if (cur_op == chk_op)
                {
		  if (*end_cb)
		    {
		      L_print_cb (stdout, fn, *end_cb);
		      L_punt ("RC_build_rc_cbs: over writing rc end_cb\n");
		    }
		  
                  *end_cb = new_cb;
		  new_op = L_create_new_op (Lop_DEFINE);
		  L_insert_oper_before (new_cb, new_cb->first_op, new_op);
		  attr = L_new_attr ("rcend", 1);
		  new_op->attr = L_concat_attr (new_op->attr, attr);
		  L_set_int_attr_field (attr, 0, rc_counter);

		  /* No point looking past a check in it's cb */
		  break;
                }
              else
                {
                  /* A check other than the one the recovery code is
                   *  being built for already covers paths through this
                   *  cb
                   */
		  if (RC_get_SPECID(cur_op) == RC_get_SPECID(chk_op))
		    {
		      /* In predicated code, a check can exist along another
		       *   checks path. It's predicate must be FALSE if we 
		       *   made it to this check. Replace with pred-clear
		       */
#if 0
		      new_op = L_create_new_op (Lop_CMP);
		      new_op->dest[0] = L_copy_operand(cur_op->pred[0]);
		      new_op->dest[0]->ptype = L_PTYPE_COND_F;
		      new_op->com[0] = L_CTYPE_INT;
		      new_op->com[1] = Lcmp_COM_EQ;
		      new_op->src[0] = L_new_int_operand (0, L_CTYPE_INT);
		      new_op->src[1] = L_new_int_operand (0, L_CTYPE_INT);
		      L_insert_oper_after (new_cb, new_cb->last_op, new_op);		      
#endif
		    }
		  else
		    {
#if 0
		    L_punt ("RC_build_rc_cbs: encountered check of differing SPECID: %d != %d\n",
			    RC_get_SPECID(cur_op), RC_get_SPECID(chk_op));
#endif
		    }
		  continue;
                }
	      L_punt("should not get here\n");
            }

          /* 
	   * Add all general Recovery Code Opers 
           */
	  info = RC_FLOW_INFO(cur_op);
	  List_start(info->add_before);
	  while ((new_op=(L_Oper*)List_next(info->add_before)))
	    {
	      RC_build_rc_add_oper_after(new_cb, new_cb->last_op, 
					 new_op, chk_op);
	    }
	  info->add_before = List_reset(info->add_before);

	  if (!info->skip_op)
	    {
	      new_op = RC_copy_op (cur_op, NULL);
	      *new_op_list = List_insert_last (*new_op_list, new_op);
	      RC_build_rc_add_oper_after(new_cb, new_cb->last_op, 
					 new_op, chk_op);
	      
	      if (cur_op == lds_op)
		{
		  /* Copy of spec load in RC is no longer speculative */
		  new_op->flags = L_CLR_BIT_FLAG (new_op->flags,
						  (L_OPER_MASK_PE |
						   L_OPER_SPECULATIVE));
		}
	    }
	  info->skip_op = 0;

	  List_start(info->add_after);
	  while ((new_op=(L_Oper*)List_next(info->add_after)))
	    {
	      RC_build_rc_add_oper_after(new_cb, new_cb->last_op, 
					 new_op, chk_op);
	    }
	  info->add_after = List_reset(info->add_after);
        }                       /*rc_ops */
    }                           /*rc_cb */


  /* Now go through new_cb_list and update all
     of the jump target using the mapping */
  List_start(*new_cb_list);
  while((rc_cb = (L_Cb*)List_next(*new_cb_list)))
    {
      L_Cb    *new_target_cb = NULL;
      int   id,s;

      /* The end_cb never needs a fallthru */
      if (rc_cb != *end_cb)
	RC_add_fallthru_for_cb(rc_cb);

      for (cur_op=rc_cb->first_op; cur_op; cur_op=cur_op->next_op)
        {
	  if (!L_is_control_oper(cur_op))
	    continue;

	  for (s = 0; s < L_max_src_operand; s++)
	    {
	      if (!cur_op->src[s] || !L_is_cb(cur_op->src[s]))
		continue;

	      id = cur_op->src[s]->value.cb->id;
	      new_target_cb = (L_Cb*)HashTable_find_or_null(map,id);
	      
	      /* If NULL, branch condition is actually speculative
	       *  and target is not part of RC
	       */
	      if (!new_target_cb)
		continue;

	      RC_delete_flow_for_op (rc_cb, cur_op);
	      cur_op->src[s]->value.cb = new_target_cb;
	      RC_add_flow_for_op (rc_cb, cur_op);
	    }
	}

      /* There can be cases where the last jump is predicated and
	 the fallthru is impossible due to the fact that RC was
	 reached. In this case remove the predicate. */
      if ((rc_cb != *end_cb) && 
	  !rc_cb->next_cb)
	{
	  if (!L_is_control_oper(rc_cb->last_op))
	    L_punt("RC_build_rc_cbs: last op is not a jump\n");
	  if (rc_cb->last_op->pred[0])
	    {
	      printf("Unpredicating jump in RC\n");
	      L_delete_operand(rc_cb->last_op->pred[0]);
	    }
	  rc_cb->last_op->pred[0] = NULL;
	  if (rc_cb->last_op->pred[1])
	    L_delete_operand(rc_cb->last_op->pred[1]);	  
	  rc_cb->last_op->pred[1] = NULL;
	}
    }

  if (!*end_cb)
    {
      L_warn ("RC_build_rc_cbs: check is totally redundant");
    }
}


/*##########################################################
 * Add the target cbs to each check
 ##########################################################*/
void
RC_add_check_targets (L_Cb * start_cb, L_Cb * end_cb,
                      List last_oprd_list, L_Cb * chk_cb, L_Oper * chk_op)
{
  L_Attr *attr = NULL;
  L_Operand *oprd = NULL;
  int i = 0;
  L_Oper *new_op = NULL;

#ifdef RC_DEBUG_INFO
  printf ("Step ##6##\n");
#endif

  if (chk_op->src[1])
    {
      L_punt ("RC_add_check_targets: check %d maps to multiple rc cbs",
              chk_op->id);
    }
  chk_op->src[1] = L_new_cb_operand (start_cb);
  RC_add_flow_for_op (chk_cb, chk_op);

  /*
    attr = L_new_attr ("rcend", 1);
    L_set_attr_field (attr, 0, L_new_cb_operand (end_cb));
    start_cb->attr = L_concat_attr (start_cb->attr, attr);
  */

  /*attr = L_new_attr("rclast",1); */
#if 0
  List_start (last_oprd_list);
  for (i = 0; (oprd = (L_Operand *) List_next (last_oprd_list)); i++)
    {
      new_op = L_create_new_op (Lop_DEFINE);
      new_op->src[0] = L_copy_operand (oprd);
      L_insert_oper_after (end_cb, end_cb->last_op, new_op);

      attr = L_new_attr ("rcdef", 0);
      new_op->attr = L_concat_attr (new_op->attr, attr);
      /*L_set_attr_field(attr, i,  L_copy_operand(oprd)); */
    }
#endif
  /*start_cb->attr = L_concat_attr(end_cb->attr, attr); */

  attr = L_new_attr ("src", 0);
  chk_op->attr = L_concat_attr (chk_op->attr, attr);
}

void
RC_delete_redundant_checks(L_Func * fn, List lds_list)
{
  L_Oper *lds_op = NULL;
  
  for (List_start (lds_list); (lds_op = (L_Oper *) List_next (lds_list));)
    {
      RC_flow_info *info          = RC_FLOW_INFO (lds_op);
      L_Oper *chk_op              = NULL;
      L_Cb   *chk_cb              = NULL;
      Set     vop_set             = NULL;
      List    ctrl_list           = NULL;
      List    cb_list             = NULL;
      List    check_op_list       = NULL;
      int c, c2;

      /* Build a list of chk_ops and cbs
       */

      for (c = 0; c < info->chk_num; c++)
	{
          chk_op = info->chk_ops[c];
	  check_op_list = List_insert_last(check_op_list, chk_op);
	}
      
      /* Determine which cbs with checks are unreachable
       */
      RC_cbs_reachable_from2 (fn, info->cb, lds_op,
			      check_op_list, NULL, 
			      &vop_set, &ctrl_list, &cb_list);

      /* Delete any of these checks within unreachable cb
       */
      for (c = 0; c < info->chk_num; c++)
	{
          chk_op = info->chk_ops[c];
          chk_cb = L_oper_hash_tbl_find_cb (fn->oper_hash_tbl, chk_op->id);
	  if (List_member(cb_list, chk_cb))
	    continue;

	  /* Shift the array */
	  info->chk_num --;
	  for (c2 = c; c2 < info->chk_num; c2++)
	    info->chk_ops[c2] = info->chk_ops[c2+1];
	  /* Change current loc */
	  c--;
	 
	  printf("RED_CHECK: Deleteing check op%d cb%d\n",
		 chk_op->id, chk_cb->id);
	  /* Delete the check */
	  RC_delete_oper (chk_cb, chk_op);
	}
      
      /* Free up all of the lists
       */
      RC_FreeCbInfo(fn);
      vop_set = Set_dispose(vop_set);
      cb_list = List_reset (cb_list);
      ctrl_list = List_reset (ctrl_list);
      check_op_list = List_reset (check_op_list);
    }
}

/*##########################################################
 * Find ops that are control speculative due to a predicated 
 * jump
 ##########################################################*/
void
RC_add_ctrl_dep_ops(L_Func *fn, List *rc_list, List *conf_list,
		    List dep_ctrl_list, Set vop_set,
		    L_Cb * lds_cb, L_Oper * lds_op,
		    L_Cb * chk_cb, L_Oper * chk_op)
{
  L_Oper *ctrl_op = NULL;
  L_Cb   *ctrl_cb = NULL;
  L_Oper *cur_op = NULL;
  List   add_list = NULL;
  List   pred_list = NULL;
  int d;
  int lds_id;

  if (List_size(dep_ctrl_list) == 0)
    return;

  lds_id = RC_get_SPECID(lds_op);
  List_start(dep_ctrl_list);
  while((ctrl_op=(L_Oper*)List_next(dep_ctrl_list)))
    {
      ctrl_cb = L_oper_hash_tbl_find_cb (fn->oper_hash_tbl, ctrl_op->id);
      printf("Spec branch op %d: ",ctrl_op->id);
      if (ctrl_cb != chk_cb)
	{
	  /* Hopefully, this only occurs when the dep. jump is
	     actually followed */
	  printf("chk_op %d in different cb\n", chk_op->id);
	  continue;
	}

      for (cur_op=ctrl_op->next_op; cur_op; cur_op=cur_op->next_op)
	{
	  /* Is a valid op? */
	  if (!Set_in(vop_set, cur_op->id))
	    continue;
	  if (cur_op->opc != Lop_CHECK)
	    continue;
	  if (RC_get_SPECID(cur_op) != lds_id)
	    continue;
	  if (cur_op == chk_op)
	    break;
	  pred_list = List_insert_last(pred_list, cur_op);
	}

      if (PG_collective_subsumption(pred_list, ctrl_op))
	{
	  printf("jmp_op %d covered by preceeding checks\n", ctrl_op->id);
	  pred_list = List_reset(pred_list);
	  continue;
	}
     
      /* Start at the jmp and go to check
       */
      for (cur_op=ctrl_op->next_op; cur_op; cur_op=cur_op->next_op)
	{
	  /* Is a valid op? */
	  if (!Set_in(vop_set, cur_op->id))
	    continue;
	  /* Done? */
	  if (cur_op == chk_op)
	    break;
	  /* Is this op already added or in RC */
	  if (List_member(*rc_list, cur_op) ||
	      List_member(add_list, cur_op))
	    continue;
	  
	  printf(" %d",cur_op->id);
	  add_list = List_insert_last(add_list, cur_op);
	  if (L_general_store_opcode(cur_op) ||
	      L_subroutine_call_opcode(cur_op) ||
	      L_subroutine_return_opcode(cur_op))
	    L_punt("\nop %d is STORE/RTS/JSR\n",cur_op->id);
	}
      printf("\n");
      
      if (!cur_op)
	L_punt("Same cb but not found\n");		  
      
      pred_list = List_reset(pred_list);
    }
  

  /* Add ops to rc_list and 
     add their dests to the conf_list */
  List_start(add_list);
  while((cur_op=(L_Oper*)List_next(add_list)))
    {
      *rc_list = List_insert_last(*rc_list, cur_op);
      for (d=0; d<L_max_dest_operand; d++)
	{
	  if (!cur_op->dest[d])
			continue;
	  *conf_list = List_insert_last(*conf_list, cur_op->dest[d]);
	}
    }
  add_list = List_reset(add_list);
}

/*##########################################################
 * Save_restore Helpers
 ##########################################################*/
#define RC_SVRT_AFTER   1
#define RC_SVRT_BEFORE  2
#define RC_SVRT_SKIP    3
#define RC_SVRT_NO_SKIP 4

List
RC_get_dests_for_op_list (List op_list)
{
  L_Oper    *cur_op = NULL;
  List       dest_list = NULL;
  int        i;

  List_start (op_list);
  while ((cur_op = (L_Oper *) List_next (op_list)))
    {
      for (i=0; i<L_max_dest_operand; i++)
	{
	  if (!cur_op->dest[i])
	    continue;

	  if (RC_oprd_in_list (dest_list, cur_op->dest[i]))
	    continue;
	  
	  dest_list = List_insert_last (dest_list, cur_op->dest[i]);
        }
    }

  return dest_list;
}

List
RC_get_srcs_for_op_list (List op_list)
{
  L_Oper    *cur_op = NULL;
  List       src_list = NULL;
  int        i;

  List_start (op_list);
  while ((cur_op = (L_Oper *) List_next (op_list)))
    {
      for (i=0; i<L_max_src_operand; i++)
	{
	  if (!cur_op->src[i])
	    continue;
	  if (!L_is_reg(cur_op->src[i]) &&
	      !L_is_macro(cur_op->src[i]))
	    continue;

	  if (RC_oprd_in_list (src_list, cur_op->src[i]))
	    continue;
	  
	  src_list = List_insert_last (src_list, cur_op->src[i]);
        }
      if (cur_op->pred[0] &&
	  !RC_oprd_in_list (src_list, cur_op->pred[0]))
	{
	  src_list = List_insert_last (src_list, cur_op->pred[0]);
	}
    }

  return src_list;
}

int
RC_oprd_def_in_rc_block(L_Cb *rc_cb, L_Operand *oprd)
{
  L_Cb      *start_cb = NULL;
  L_Cb      *cb       = NULL;
  L_Oper    *op       = NULL;
  L_Attr    *attr     = NULL;
  int d, id;

  /* Find first cb in rc block */
  attr = L_find_attr(rc_cb->attr, "rc");
  if (!attr)
    L_punt("RC_oprd_def_in_rc_block: cb%d does not have rc attr",rc_cb->id);
  id = attr->field[1]->value.i;
  for (cb=rc_cb; cb; cb=cb->prev_cb)
    {
      attr = L_find_attr(cb->attr, "rc");
      if (!attr)
	break;
      if (id != attr->field[1]->value.i)
	break;
    }
  start_cb = cb->next_cb;
  
  /* Look through all op dests for oprd */
  for (cb=start_cb; cb; cb=cb->next_cb)
    {
      attr = L_find_attr(cb->attr, "rc");
      if (!attr)
	break;     
      if (id != attr->field[1]->value.i)
	break;
      for (op=cb->first_op; op; op=op->next_op)
	{
	  for (d=0; d<L_max_dest_operand; d++)
	    {
	      if (!op->dest[d])
		continue;
	      if (L_same_operand(op->dest[d],oprd))
		return 1;
	    }
	}
    }

  return 0;
}

void
RC_insert_svrt(L_Func *fn, L_Operand *oprd, 
	       int pos, L_Oper *ref_op, int skip)
{
  L_Operand    *tmpreg     = NULL;
  L_Oper       *save_op    = NULL;
  L_Oper       *restore_op = NULL;
  L_Cb         *ref_cb     = NULL;
  L_Cb         *cb         = NULL;
  L_Oper       *op         = NULL;
  RC_flow_info *info       = NULL;
  L_Attr       *attr       = NULL;
  Set           def_set    = NULL;
  int           save_lop, i;

  ref_cb = (RC_FLOW_INFO(ref_op))->cb;
  
  /* Get valid temp register */
  tmpreg = L_new_register_operand (++fn->max_reg_id, 
				   oprd->ctype, oprd->ptype);

  /* Determine appropriate save operation */
  switch(oprd->ctype)
    {
    case L_CTYPE_PREDICATE:
      save_lop = Lop_CMP;
      break;
    case L_CTYPE_FLOAT:
      save_lop = Lop_MOV_F;
      break;
    case L_CTYPE_DOUBLE:
      save_lop = Lop_MOV_F2;
      break;
    default:
      save_lop = Lop_MOV;
      break;
    }
  
  /* Create save operation */
  if (save_lop == Lop_CMP)
    {
      save_op = L_create_new_op (save_lop);
      save_op->flags = 0;	  
      save_op->com[0] = L_CTYPE_INT;
      save_op->com[1] = Lcmp_COM_EQ;
      save_op->pred[0] = L_copy_operand (oprd);
      save_op->pred[0]->ptype = L_PTYPE_NULL;
      save_op->src[0] = L_new_macro_operand (TAHOE_MAC_ZERO, L_CTYPE_LLONG, 0);
      save_op->src[1] = L_new_macro_operand (TAHOE_MAC_ZERO, L_CTYPE_LLONG, 0);
      save_op->dest[0] = L_copy_operand (tmpreg);
      save_op->dest[0]->ptype =  L_PTYPE_UNCOND_T;
    }
  else
    {
      save_op = L_create_new_op (save_lop);
      save_op->flags = 0;
      for (i = 0; i < L_max_pred_operand; i++)
 	save_op->pred[i] = L_copy_operand (ref_op->pred[i]);
      save_op->src[0] = L_copy_operand (oprd);
      save_op->dest[0] = L_copy_operand (tmpreg);
    }
  save_op->attr = L_new_attr("save",0);
  save_op->ext = RC_new_flow_info();
  info = RC_FLOW_INFO(save_op);
  info->cb = ref_cb;

  /* Create restore operation */
  if (save_lop == Lop_CMP)
    {
      restore_op = L_create_new_op (save_lop);
      restore_op->flags = 0;	  
      restore_op->com[0] = L_CTYPE_INT;
      restore_op->com[1] = Lcmp_COM_EQ;
      restore_op->pred[0] = L_copy_operand (tmpreg);
      restore_op->pred[0]->ptype = L_PTYPE_NULL;
      restore_op->src[0] = L_new_macro_operand (TAHOE_MAC_ZERO, L_CTYPE_LLONG, 0);
      restore_op->src[1] = L_new_macro_operand (TAHOE_MAC_ZERO, L_CTYPE_LLONG, 0);
      restore_op->dest[0] = L_copy_operand (oprd);
      restore_op->dest[0]->ptype =  L_PTYPE_UNCOND_T;
    }
  else
    {
      restore_op = L_create_new_op (save_lop);
      restore_op->flags = 0;
      for (i = 0; i < L_max_pred_operand; i++)
	restore_op->pred[i] = L_copy_operand (ref_op->pred[i]);
      restore_op->src[0] = L_copy_operand (tmpreg);
      restore_op->dest[0] = L_copy_operand (oprd);
    }
  restore_op->attr = L_new_attr("restore",0);
  
  /* Put in the save and restore */
  if (pos == RC_SVRT_BEFORE)
    {
      info = RC_FLOW_INFO(ref_op);
      L_insert_oper_before (ref_cb, ref_op, save_op);
      info->add_before = List_insert_last(info->add_before,
					  restore_op);
      /* save_op has all of the appropriate defs added
       * based on the RIN to the ref_op
       */
      /*RC_copy_oper_deps(ref_op, save_op);*/
      RC_add_def_using(ref_op, save_op, oprd);
      
      def_set = Set_add(def_set, ref_op->id); 
    }
  else
    {
      info = RC_FLOW_INFO(ref_op);
      L_insert_oper_after (ref_cb, ref_op, save_op);
      info->add_after = List_insert_last(info->add_after,
					 restore_op);
      /* save_op should be added as a use
       * of ref_op's dest oprd 
       */
      RC_add_flow_dep(ref_op, save_op, oprd);

      /* Form a set of definining opid's */
      def_set = Set_add(def_set, ref_op->id);
    }
  
  /* FIXUP previous RC blocks
   * save_op should be added after every copy
   * of a defining op in recovery code
   */
  for (cb=fn->first_cb; cb; cb=cb->next_cb)
    {
      if (!(attr=L_find_attr(cb->attr,"rc")))
	continue;
      for (op=cb->first_op; op; op=op->next_op)
	{
	  if (!(attr=L_find_attr(op->attr,"rc")))
	    continue;
	  if (!Set_in(def_set, attr->field[0]->value.i))
	    continue;
	  
	  /* Save before real and phantom ops for the BEFORE case,
	   *   but only for the real case for the AFTER case 
	   */
	  if (op->opc != Lop_DEFINE)
	    {
	      /* Real op */
	      if (pos == RC_SVRT_BEFORE)
		L_insert_oper_before (cb, op, RC_copy_op(save_op, NULL));
	      else
		L_insert_oper_after (cb, op, RC_copy_op(save_op, NULL));		
	    }
	  else if (pos == RC_SVRT_BEFORE)
	    {
	      /*Phantom load: insert save ONLY if this recovery block
	       *  defines the oprd to be saved somewhere 
	       */
	      if (RC_oprd_def_in_rc_block(cb, oprd))
		L_insert_oper_before (cb, op, RC_copy_op(save_op, NULL));
	    }
	}
    }     
  

  /* Set skip flag if requested */
  if (skip == RC_SVRT_SKIP)
    {
      info = RC_FLOW_INFO(ref_op);
      info->skip_op = 1;
    }
  
  Set_dispose(def_set);
  L_delete_operand(tmpreg);
}

/*##########################################################
 * Save_restore
 ##########################################################*/
void
RC_add_save_restores(L_Func *fn, List *rc_list, Set vop_set,
		     L_Cb *lds_cb, L_Oper *lds_op,
		     L_Cb *chk_cb, L_Oper *chk_op)
{
  List          dest_list     = NULL;
  List          src_list      = NULL;
  List          insert_list   = NULL;
  List          src_ovrt_list = NULL;
  L_Operand    *oprd          = NULL;
  L_Oper       *op            = NULL;
  RC_flow_info *info          = NULL;
  RC_flow_info *cur_info      = NULL;
  L_Oper       *cur_op        = NULL;
  L_Cb         *cur_cb        = NULL;
  L_Oper       *dep_op        = NULL;
  L_Cb         *dep_cb        = NULL;
  RC_dep_info  *dep           = NULL;
  int          *buf           = NULL;
  int           i, d, num;
  
  /********************************************************
   * Build a list of sources and dests
   */
  src_list = RC_get_srcs_for_op_list (*rc_list);
  dest_list = RC_get_dests_for_op_list (*rc_list);

  /********************************************************
   * Process all SRC oprds
   */
  List_start (src_list);
  while ((oprd = (L_Operand *) List_next (src_list)))
    {
      printf("SRC ");
      L_print_operand(stdout, oprd, 1);
      printf(" : ");
      
      List_start (*rc_list);
      while ((cur_op = (L_Oper *) List_next (*rc_list)))
	{
	  /* Process all defs for the op */
	  cur_info = RC_FLOW_INFO (cur_op);
	  cur_cb = cur_info->cb;
	  for (List_start (cur_info->def_op_list);
	       (dep = (RC_dep_info *) List_next (cur_info->def_op_list));)
	    {
	      /* Has dep been deleted? */
	      if ((!dep->to_op) || (!dep->from_op))
		continue;
	      /* Is dep for the same operand? */
	      if (!L_same_operand(dep->oprd,oprd))
		continue;
	      dep_op = dep->from_op;
	      dep_cb = RC_FLOW_INFO (dep_op)->cb;
	      
	      /* Is the defining op in rc_list */
	      if (List_member (*rc_list, dep_op))
		continue;
	      
	      if (Set_in (vop_set, dep_op->id))
		{
		  if (RC_IsValidControl(dep_cb, dep_op, cur_cb, cur_op))
		    {
		      /*************************************** 
		       * (CASE 1) Def op is in vop_set and can 
		       *          reach cur_op 
		       * -> SVRT oprd after def_op
		       */
		      printf("C1-op%d ",dep_op->id);
		      
		      /* Do insertion of SVRT */			  
		      RC_insert_svrt(fn, oprd, RC_SVRT_AFTER, 
				     dep_op, RC_SVRT_SKIP);
		      if (!List_member(insert_list, dep_op))
			insert_list = List_insert_last(insert_list, dep_op);
		    }
		}
	      else
		{
		  /**************************************
		   * (CASE 2) def op is not in vop_set
		   *  -> SVRT oprd before spec load
		   */
		  printf("C2-op%d ",dep_op->id);
		  
		  /* Do insertion of SVRT */
		  RC_insert_svrt(fn, oprd, RC_SVRT_BEFORE, 
				 lds_op, RC_SVRT_NO_SKIP);
		  if (!List_member(src_ovrt_list, oprd))
		    src_ovrt_list = List_insert_last(src_ovrt_list, oprd);
		}
	    }
	}
      printf("\n");
    }

  /********************************************************
   * Process all DEST oprds
   */
  List_start (dest_list);
  while ((oprd = (L_Operand *) List_next (dest_list)))
    {
      printf("DEST ");
      L_print_operand(stdout, oprd, 1);
      printf(" : ");
      
      /**************************************
       * (CASE 4) 
       *  -> Always SVRT b/f load
       */
      printf("C4 ");
      
      /* Do insertion of SVRT */
      RC_insert_svrt(fn, oprd, RC_SVRT_BEFORE,
		     lds_op, RC_SVRT_NO_SKIP);
    }
  printf("\n");
  

  /* Process all defs in vop_set 
   */
  num = Set_size(vop_set);
  if (!num)
    L_punt("RC_add_save_restores: vops empty\n");
  buf = (int *) Lcode_malloc (sizeof (int) * num);
  Set_2array (vop_set, buf);
  for (i = 0; i < num; i++)
    {
      dep_op = L_oper_hash_tbl_find_oper (fn->oper_hash_tbl, 
					  buf[i]);
      dep_cb = RC_FLOW_INFO (dep_op)->cb;
      
      /* Is the defining op in rc_list */
      if (List_member (*rc_list, dep_op))
	continue;
      
      for (d=0; d<L_max_dest_operand; d++)
	{
	  if (!dep_op->dest[d])
	    continue;
	  if (!RC_oprd_in_list (dest_list, dep_op->dest[d]) &&
	      !RC_oprd_in_list (src_ovrt_list, dep_op->dest[d]))
	    continue;
	  
	  /**************************************
	   * (CASE 3) Re-def op is in vop_set, is not in
	   *          rc_list
	   * -> SVRT dest oprd after re-def op
	   */
	  printf("C3-op%d ",dep_op->id);
	  
	  /* Do insertion of SVRT */
	  RC_insert_svrt(fn, dep_op->dest[d], RC_SVRT_AFTER,
			 dep_op, RC_SVRT_SKIP);
	  if (!List_member(insert_list, dep_op))
	    insert_list = List_insert_last(insert_list, dep_op);
	}
    }
  Lcode_free (buf);



  /* Added ops to rc_list that need restores inserted
   */
  List_start(insert_list);
  while ((op=(L_Oper*)List_next(insert_list)))
    {
      if (!List_member(*rc_list, op))
	*rc_list = List_insert_last(*rc_list, op);
    }


  /* Insert load place holders for future save insertion
   */
  num = Set_size(vop_set);
  if (!num)
    L_punt("RC_add_save_restores: vops empty\n");
  buf = (int *) Lcode_malloc (sizeof (int) * num);
  Set_2array (vop_set, buf);
  for (i = 0; i < num; i++)
    {
      L_Oper *new_op = NULL;
      L_Attr *attr = NULL;
      op = L_oper_hash_tbl_find_oper (fn->oper_hash_tbl, 
				      buf[i]);
      info = RC_FLOW_INFO(op);
      if (!L_general_load_opcode(op))
	continue;
      if (List_member (*rc_list, op) &&
	  info->skip_op == 0)
	continue;
      /* The load in question is not, itself, in
       * recovery code. Create a marker.
       */
      new_op = L_create_new_op (Lop_DEFINE);
      attr = L_new_attr ("rc", 1);
      L_set_int_attr_field(attr, 0, op->id);
      new_op->attr = L_concat_attr (new_op->attr, attr);
      info->add_after = List_insert_last(info->add_after,
					 new_op);

      /* Now, if not a member of rc_list */
      if (!List_member (*rc_list, op))
	{
	  *rc_list = List_insert_last(*rc_list, op);	
	  info->skip_op = 1;
	}
    }
  Lcode_free (buf);

  src_list = List_reset(src_list);
  dest_list = List_reset(dest_list);  
  insert_list = List_reset(insert_list);
  src_ovrt_list = List_reset(src_ovrt_list);
}


/*##########################################################
 * Find opers that compose the recovery code
 ##########################################################*/
void
RC_create_recovery_code (L_Func * fn, List * lds_list)
{
  L_Oper *lds_op = NULL;

#ifdef RC_DEBUG_INFO
  printf ("---- Recovery code gen ----\n");
#endif

  for (List_start (*lds_list); (lds_op = (L_Oper *) List_next (*lds_list));)
    {
      RC_flow_info *info = RC_FLOW_INFO (lds_op);
      List op_list = NULL;
      List rc_list = NULL;
      List cb_list = NULL;
      List new_op_list = NULL;
      List new_cb_list = NULL;
      List oprd_list = NULL;
      List psrv_list = NULL;
      List svrt_list = NULL;
      List conf_list = NULL;
      List dest_list = NULL;
      List last_oprd_list = NULL;
      List dep_ctrl_list = NULL;
      L_Cb *start_cb = NULL;
      L_Cb *end_cb = NULL;
      L_Oper *chk_op = NULL;
      L_Cb *chk_cb = NULL;
      List exclude_check_op_list = NULL;
      List check_op_list = NULL;
      Set  vop_set = NULL;
      int c /*,change*/;

      if (info->chk_mark)
        RC_make_load_speculative (fn, lds_op);

      for (c = 0; c < info->chk_num; c++)
	{
	  exclude_check_op_list = 
	    List_insert_last(exclude_check_op_list, info->chk_ops[c]);
	}

      /* For each check that matches this load 
       */
      for (c = 0; c < info->chk_num; c++)
        {
          /* Clear lists */
          rc_list = List_reset (rc_list);
          cb_list = List_reset (cb_list);
          op_list = List_reset (op_list);
          new_op_list = List_reset (new_op_list);
          new_cb_list = List_reset (new_cb_list);
          oprd_list = List_reset (oprd_list);
          psrv_list = List_reset (psrv_list);
          svrt_list = List_reset (svrt_list);
          conf_list = List_reset (conf_list);
	  dest_list = List_reset (dest_list);
          last_oprd_list = List_reset (last_oprd_list);
	  dep_ctrl_list = List_reset(dep_ctrl_list);
	  vop_set = Set_dispose(vop_set);
          start_cb = NULL;
          end_cb = NULL;
	  
          chk_op = info->chk_ops[c];
          chk_cb = L_oper_hash_tbl_find_cb (fn->oper_hash_tbl, chk_op->id);

          /***********************************************
           * STEP 1 : Determine which cbs form the recovery code
           ***********************************************/
	  check_op_list = List_insert_last(check_op_list, chk_op);
	  exclude_check_op_list = List_remove(exclude_check_op_list, 
					      chk_op);

	  RC_cbs_reachable_from2 (fn, info->cb, lds_op,
				  check_op_list, 
				  exclude_check_op_list, 
				  &vop_set, &op_list, &cb_list);
	  
	  check_op_list = List_reset(check_op_list);
	  exclude_check_op_list = List_insert_last(exclude_check_op_list, 
						   chk_op);
	  if (List_size(cb_list) == 0)
	    L_punt("RC_create_recovery_code: Check %d "
		   "should not be redundant\n",
		   chk_op->id);
	  

          /***********************************************
	   * (STEP 2.1 has be absorbed into STEP 1)
           * STEP 2.2 : Find ops flow dependent on the load
           *            rc_list = all RC ops
           ***********************************************/

          /* Add spec load to op list and find all flow dep
           *  ops for RC
           */
          op_list = List_insert_last (op_list, lds_op);

          RC_get_ops_for_rc (fn, &op_list, &dep_ctrl_list,
			     &rc_list, lds_list,
                             &last_oprd_list, vop_set, cb_list,
                             info->cb, lds_op, chk_cb, chk_op);

	  /* Is the check in the op_list? If not delete check
	   * and go to next check (two loads mapping to the same
	   * check can lead to this case
	   */
	  if (!List_member(rc_list,chk_op))
	    {
	      printf("!!! Check %d is unreachable through dependences, deleteing it\n",
		     chk_op->id);
	      
	      /* Shift the array */
	      {
		int c2 = 0;
		info->chk_num --;
		for (c2 = c; c2 < info->chk_num; c2++)
		  info->chk_ops[c2] = info->chk_ops[c2+1];
		/* Change current loc */
		c--;
	      }
	      RC_delete_oper (chk_cb, chk_op);
	      RC_FreeCbInfo(fn);
	      continue;
	    }


	  /***********************************************
	   * STEP 2.3 : If a wired and type predicate(s) is 
	   *    in rc_list, all ops in vop_set that potentially
	   *    write the pred must be in RC and the pred must
	   *    be saved b/f load and restored in RC
	   ***********************************************/

	  RC_add_in_wired_preds(&rc_list, vop_set, cb_list,
				&conf_list);
	  

	  /***********************************************
	   * STEP 2.4 : Are there any ops that are control
	   *    speculative due to a preceeding spec 
	   *    jump 
	   ***********************************************/

	  RC_add_ctrl_dep_ops(fn, &rc_list, &conf_list,
			      dep_ctrl_list, vop_set,
			      info->cb, lds_op,
			      chk_cb, chk_op);
	  
	  
	  /***********************************************
           * STEP 3 : All of the ops that must be re-executed
	   *    should now be in rc_list. Go through an insert
	   *    the _potentially_ required saves and create
	   *    the _potentially_ required restores to make the
	   *    re-execution legal. This will add ops to the
	   *    rc_list, but they will all have the info->subs
	   *    field set
           ***********************************************/

	  RC_add_save_restores(fn, &rc_list, vop_set,
			       info->cb, lds_op,
			       chk_cb, chk_op);

#if 0
          /***********************************************
           * STEP 3 : prsv_list = src oprd to preserve until check
           *          conf_list = oprds conflict with preserve
           *          scrt_list = oprds are simply anti-dep
           ***********************************************/
	  
          /* This iterates because adding ops can create new
           *   preservations, anti-deps, and more ops
           */
          do
            {
	      change = 0;

              RC_get_prsv_for_rc (rc_list, &psrv_list);

              change |= RC_get_anti_ops_for_rc (cb_list, vop_set,
						&rc_list, psrv_list,
						&conf_list, &svrt_list,
						info->cb, lds_op,
						chk_cb, chk_op);

	      RC_get_dests_for_rc (rc_list, &dest_list);
	      
	      change |= RC_get_def_ops_for_rc (cb_list, dest_list,
					       &rc_list,
					       info->cb, lds_op,
					       chk_cb, chk_op);
            }
          while (change);
#endif


          /************************************************
           * STEP 4 : Build the recovery code cbs 
           ************************************************/
          RC_build_rc_cbs (fn, cb_list, rc_list,
                           &new_cb_list, &new_op_list,
                           &start_cb, &end_cb, /*&conf_list, &svrt_list,*/
                           info->cb, lds_op, chk_cb, chk_op);

          if (!start_cb)
            L_punt ("RC_create_recovery_code: start_cb not found");
          if (!end_cb)
            {
              /* This signals that this check is unneeded */
	      L_punt("RC_create_recovery_code: Check %d "
		     "should not be unneeded\n",
		     chk_op->id);
            }


          /************************************************
           * STEP 5 : Add in any moves for conf, svrt to
           *           both RC and orig code
           ************************************************/
#if 0
          RC_fix_anti_deps (conf_list, svrt_list, &psrv_list,
                            start_cb, end_cb,
                            info->cb, lds_op, chk_cb, chk_op);

#endif

          /************************************************
           * STEP 6 : Add target to check and sink defines
	   *           to RC
           ************************************************/
          RC_add_check_targets (start_cb, end_cb,
                                last_oprd_list, chk_cb, chk_op);


          /************************************************
           * STEP 7 : Special jump opti for RC 
           ************************************************/
#if 1
          RC_jump_opti (fn, &new_cb_list, end_cb);
#endif
          /************************************************
           * STEP 8 : add preserves to check
           ************************************************/
#if 0
          {
            L_Operand *oprd = NULL;
            L_Attr *attr = NULL;
            int tr_count = 0;

            attr = L_find_attr (chk_op->attr, "src");
            if (!attr)
              {
                attr = L_new_attr ("src", 1);
                chk_op->attr = L_concat_attr (chk_op->attr, attr);
              }

            tr_count = 0;
            for (List_start (info->psrv_list[c]);
                 (oprd = (L_Operand *) List_next (info->psrv_list[c]));)
              {
                L_set_attr_field (attr, tr_count, L_copy_operand (oprd));
                tr_count++;
              }
          }
#endif

          /************************************************/
	  RC_FreeCbInfo(fn);
        }                       /*chk */

      /* Clear lists */
      exclude_check_op_list = List_reset(exclude_check_op_list);
      rc_list = List_reset (rc_list);
      op_list = List_reset (op_list);
      new_op_list = List_reset (new_op_list);
      new_cb_list = List_reset (new_cb_list);
      oprd_list = List_reset (oprd_list);
      psrv_list = List_reset (psrv_list);
      svrt_list = List_reset (svrt_list);
      conf_list = List_reset (conf_list);
      dest_list = List_reset (dest_list);
      last_oprd_list = List_reset (last_oprd_list);
      dep_ctrl_list = List_reset(dep_ctrl_list);
      vop_set = Set_dispose(vop_set);
      start_cb = NULL;
      end_cb = NULL;
    }                           /*lds */
}



int RC_cb_is_empty(L_Cb *cb)
{
  L_Oper *op = NULL;

  for (op=cb->first_op; op; op=op->next_op)
    {
      if (L_is_control_oper(op))
	continue;
      return 0;
    }
  return 1;
}

int RC_cb_has_single_target(L_Cb *cb)
{
  L_Flow *flow = NULL;
  L_Cb   *cb_target = NULL;

  if (!cb->dest_flow)
    return 0;

  for (flow=cb->dest_flow; flow; flow=flow->next_flow)
    {
      if (!cb_target)
	cb_target = flow->dst_cb;
      if (cb_target != flow->dst_cb)
	return 0;
    }
  return 1;
}

void RC_extract_cb(L_Func *fn, L_Cb *cb, L_Cb *end_cb)
{
  L_Oper *op = NULL;
  L_Oper *next_op = NULL;
  L_Cb   *cb_target = NULL;
  L_Flow *flow = NULL;
  int     s;

  /* Redirect all sources of cb to it's
   *  dest cb_target
   */
  cb_target = cb->dest_flow->dst_cb;
  printf("Extracting cb %d, new dest cb %d\n",
	 cb->id, cb_target->id);

  /* If cb is last one, make sure previous cb
   *  is ended by an uncond jump
   */
  if (!cb->next_cb && cb->prev_cb &&
      !L_uncond_branch_opcode(cb->prev_cb->last_op) &&
      cb->prev_cb != end_cb)
    {
      L_Oper *new_op;
      new_op = L_create_new_op (Lop_JUMP);
      new_op->src[0] = L_new_cb_operand (cb_target);
      L_insert_oper_after (cb->prev_cb, cb->prev_cb->last_op, new_op);
      RC_add_flow_for_op (cb->prev_cb, new_op);
      S_machine_jump (new_op);
    }
  
  /* Change all flows of src cbs and of their branches
   */
  while((flow=cb->src_flow))
    {
      /* This should delete the flows */
      printf("... redirecting cb %d\n", 
	     flow->src_cb->id);
      
      L_change_dest_cb (flow->src_cb, cb, cb_target);
      for (op=flow->src_cb->first_op; op; op=op->next_op)
	{
	  if (!L_is_control_oper(op))
	    continue;
	  for (s=0; s<L_max_src_operand; s++)
	    {
	      if (L_is_cb(op->src[s]) &&
		  op->src[s]->value.cb == cb)
		op->src[s]->value.cb = cb_target;
	    }
	}
    }

  /* Delete all ops in the cb, all flows,
   *  and the cb itself
   */
  for (op=cb->first_op; op; op=next_op)
    {
      next_op = op->next_op;
      L_delete_complete_oper (cb, op);
    }
  if (cb->dest_flow)
    {
      /* Delete fallthru */
      cb_target = cb->dest_flow->dst_cb;
      flow = L_find_matching_flow(cb_target->src_flow, cb->dest_flow);
      cb_target->src_flow = L_delete_flow(cb_target->src_flow, flow);
      if (cb->dest_flow->next_flow)
	L_punt("RC_extract_cb: too many fallthrus\n");
    }
  L_delete_cb(fn, cb);
}


void RC_remove_self_loop(L_Cb *cb)
{
  L_Flow *flow = NULL;
  L_Flow *next_flow = NULL;
  
  for (flow=cb->dest_flow; flow; flow=next_flow)
    {
      next_flow = flow->next_flow;
      if (flow->dst_cb == cb)
	cb->dest_flow = L_delete_flow(cb->dest_flow, flow);
    }
}

void RC_jump_opti (L_Func *fn, List *cb_list, L_Cb *end_cb)
{
  int change;
  L_Cb *cb = NULL;

  do 
    {
      change = 0;

      List_start(*cb_list);
      while((cb=(L_Cb*)List_next(*cb_list)))
	{
	  if (RC_cb_is_empty(cb))
	    {
	      RC_remove_self_loop(cb);
	      if (RC_cb_has_single_target(cb))
		{
		  *cb_list = List_delete_current(*cb_list);
		  RC_extract_cb(fn, cb, end_cb);
		  change = 1;
		}
	    }
	}
    } while (change);
}


List
RC_rebuild_rc_cb_list(L_Cb *rc_cb)
{
  L_Cb      *start_cb = NULL;
  L_Cb      *cb       = NULL;
  L_Attr    *attr     = NULL;
  List       cb_list  = NULL;
  int id;

  /* Find first cb in rc block */
  attr = L_find_attr(rc_cb->attr, "rc");
  if (!attr)
    L_punt("RC_oprd_def_in_rc_block: cb%d does not have rc attr",rc_cb->id);
  id = attr->field[1]->value.i;
  for (cb=rc_cb; cb; cb=cb->prev_cb)
    {
      attr = L_find_attr(cb->attr, "rc");
      if (!attr)
	break;
      if (id != attr->field[1]->value.i)
	break;
    }
  start_cb = cb->next_cb;
  
  /* Look through all op dests for oprd */
  for (cb=start_cb; cb; cb=cb->next_cb)
    {
      attr = L_find_attr(cb->attr, "rc");
      if (!attr)
	break;     
      if (id != attr->field[1]->value.i)
	break;
      cb_list = List_insert_last(cb_list, cb);
    }  

  return cb_list;
}

void
RC_opti_predicates(L_Func *fn, List lds_list)
{
  L_Oper *lds_op = NULL;

  PG_setup_pred_graph (fn);
  for (List_start (lds_list); (lds_op = (L_Oper *) List_next (lds_list));)
    {
      RC_flow_info *info = RC_FLOW_INFO (lds_op);
      List cb_list = NULL;
      L_Oper *chk_op = NULL;
      L_Oper *op = NULL;
      L_Cb *cb = NULL;
      int c;

      for (c = 0; c < info->chk_num; c++)
	{
	  chk_op = info->chk_ops[c];

	  /* Get a list of all cbs for this RC block 
	   */
	  cb_list = RC_rebuild_rc_cb_list(chk_op->src[1]->value.cb);

	  /* Process all ops in cb list */
	  for (List_start(cb_list);(cb=(L_Cb*)List_next(cb_list));)
	    {
	      for (op=cb->first_op; op; op=op->next_op)
		{
		  if (op->pred[0] && 
		      PG_superset_predicate_ops (op, chk_op))
		    {
		      printf("Deleted a useless predicate op%d\n",
			     op->id);
		      L_delete_operand(op->pred[0]);
		      op->pred[0] = NULL;
		      if (op->pred[1])
			{
			  L_delete_operand(op->pred[1]);
			  op->pred[1] = NULL;      
			}
		    }	      
		}
	    }
	  
	  cb_list = List_reset(cb_list);
	} /*checks*/
    }
 
}


/*
 * Master functions
 */



/*##########################################################
 * Read in parameters
 ##########################################################*/
void
L_read_parm_RC (Parm_Parse_Info * ppi)
{
  L_read_parm_b (ppi, "rc_print_debug_info",
                 &RC_print_debug_info);
}


/*##########################################################
 *  This is a master recovery code generation function
 *  - Generates ALL recovery code aside from some ops
 *    due to bundling issues
 *  DO JUST BEFORE REGISTER ALLOCATION AND AFTER PREPASS
 ##########################################################*/
void
RC_generate_recovery_code (L_Func * fn)
{
  List lds_list = NULL;

  printf ("##### START: RC_generate_recovery_code #####\n");
  if (RC_print_debug_info)
    {
      RC_dump_lcode (fn, ".rcg_pre");
    }

  /********************************************************
   * Get rid of some obviously redundant checks
   ********************************************************/
  RC_delete_samecb_checks(fn);

  /********************************************************
   * Run Global Data Flow and Build a Web of flow deps
   * Additional Info placed off of op->ext
   ********************************************************/
  RC_init (fn);
  RC_insert_antidep_defines (fn);
  L_do_flow_analysis (fn, REACHING_DEFINITION);
  RC_generate_global_flow_deps (fn);
  RC_generate_global_anti_deps (fn);
  RC_delete_antidep_defines (fn);
  
  /********************************************************
   *  Finds and pairs up speculative loads and checks
   ********************************************************/
  lds_list = RC_find_spec_load_and_checks (fn);

  
  /********************************************************
   * This gets rids of less obviously redundant checks
   * (DOES NOT REMOVE THE OBVIOUS ONES)
   ********************************************************/
  RC_delete_redundant_checks(fn, lds_list);

  /*******************************************************
   * Find opers that compose the recovery code
   * This is a big 
   *******************************************************/
  RC_create_recovery_code (fn, &lds_list);
  if (RC_print_debug_info)
    {
      RC_dump_lcode (fn, ".rcg_cre");
    }

  /******************************************************
   * For recovery code, convert checks into appropriate jumps,
   * create new cbs as needed for checks, add check targets,
   * add in post-check, template ops to RC, schedule RC
   * ? Fill/Spill code needed?
   ******************************************************/
  RC_split_around_checks (fn, 0);
  RC_add_rc_return_jumps (fn, 0);

  /* Do an opti on all RC cbs to eliminate some predicates
   */
  RC_opti_predicates(fn, lds_list);

  /******************************************************
   * Free anything created by RC_init
   ******************************************************/
  RC_cleanup (fn);

  {
    L_Cb *cb;
    PG_setup_pred_graph (fn);
    for (cb = fn->first_cb; cb; cb = cb->next_cb)
      {
	if (!L_find_attr(cb->attr,"rc"))
	  L_check_cb(fn, cb);
      }
  }

  printf ("##### DONE: RC_generate_recovery_code #####\n");
  if (RC_print_debug_info)
    {
      RC_dump_lcode (fn, ".rcg_post");
    }
  fflush(stdout);
}



/*##########################################################
 * Register allocation is done with cbs split after checks 
 *  to allow a correct view of checks as branches and to allow
 *  RC to have jumps to after each check
 * Recombine cbs that were split to allow post-sched to 
 *  be more effective.
 * DO AFTER REGISTER ALLOCATION AND BEFORE POSTPASS
 ##########################################################*/
void
RC_recombine_cbs (L_Func * fn)
{
  if (RC_print_debug_info)
    {
      RC_dump_lcode (fn, ".rcg_reg");
    }
  
#if 1
  /* EMN temp don't recombine */
  return;
#endif

  RC_remove_rc_return_jumps (fn);
  RC_recombine (fn);
  RC_add_sink_to_rc_blocks (fn);

  {
    L_Cb *cb;
    PG_setup_pred_graph (fn);
    for (cb = fn->first_cb; cb; cb = cb->next_cb)
      {
	if (!L_find_attr(cb->attr,"rc"))
	  L_check_cb(fn, cb);
      }
  }

  if (RC_print_debug_info)
    {
      RC_dump_lcode (fn, ".rcg_com");
    }
}


/*##########################################################
 * This is the final step.
 * Post pass has been performed and the cbs need to be split
 *  again following checks to allow RC to branch back to
 *  ops following each check (it adds in the return jumps)
 * DO AFTER POSTPASS (ALAP)
 ##########################################################*/
void RC_error_check_recovery_code (L_Func * fn);

void
RC_fix_recovery_code_bundles (L_Func * fn)
{
#if 1
  /* EMN temp don't recombine */
  if (RC_print_debug_info)
    {
      RC_dump_lcode (fn, ".rcg_final");
      RC_error_check_recovery_code (fn);
    }
  return;
#endif

  if (RC_print_debug_info)
    {
      RC_dump_lcode (fn, ".rcg_sched");
    }
  
  RC_remove_sink_from_rc_blocks (fn, 1);
  RC_split_around_checks (fn, 1);
  RC_add_rc_return_jumps (fn, 1);

  {
    L_Cb *cb;
    PG_setup_pred_graph (fn);
    for (cb = fn->first_cb; cb; cb = cb->next_cb)
      {
	if (!L_find_attr(cb->attr,"rc"))
	  L_check_cb(fn, cb);
      }
  }
  
  if (RC_print_debug_info)
    {
      RC_dump_lcode (fn, ".rcg_final");
      RC_error_check_recovery_code (fn);
    }
}









/*##########################################################
  ERROR CHECKING
  ERROR CHECKING
  ERROR CHECKING
  ERROR CHECKING
 ##########################################################*/


/*##########################################################
 ##########################################################*/
List
RC_compute_slice(L_Oper *op)
{
  List          work_list  = NULL;
  List          slice_list = NULL;
  L_Oper       *cur_op     = NULL;
  L_Oper       *dep_op     = NULL;
  RC_flow_info *cur_info   = NULL;
  RC_dep_info  *dep        = NULL;
  
  work_list = List_insert_last (work_list, op);
  List_start (work_list);
  while ((cur_op = (L_Oper *) List_next (work_list)))
    {
      cur_info = RC_FLOW_INFO (cur_op);

      /* Get next op from work list and del it from list */
      work_list = List_delete_current (work_list);
      slice_list = List_insert_last (slice_list, cur_op);

      /* Get ops flow dependent on current op
       */
      for (List_start (cur_info->use_op_list);
           (dep = (RC_dep_info *) List_next (cur_info->use_op_list));)
        { 
	  /* Has dep been deleted? */
	  if ((!dep->to_op) || (!dep->from_op))
	    continue;

          dep_op = dep->to_op;

          /* Has op already been seen */
          if (List_member (work_list, dep_op))
            continue;
          if (List_member (slice_list, dep_op))
            continue;
	  
          work_list = List_insert_last (work_list, dep_op);
        }
    }

  return slice_list;
}

/*##########################################################
 ##########################################################*/
int
RC_compute_dep_path_rec(L_Oper *from_op, L_Oper *to_op, List *path_list)
{
  L_Oper       *dep_op    = NULL;
  RC_flow_info *from_info = NULL;
  RC_dep_info  *dep       = NULL;
  
  from_info = RC_FLOW_INFO (from_op);
  
  /* Is op in progress? */
  if (L_EXTRACT_BIT_VAL (from_op->flags, L_OPER_RESERVED_TEMP1))
    return 0;
  /* Is op impossible */
  if (L_EXTRACT_BIT_VAL (from_op->flags, L_OPER_RESERVED_TEMP2))
    return 0;
  from_op->flags = L_SET_BIT_FLAG (from_op->flags, L_OPER_RESERVED_TEMP1);
  
  /* Search once to see if any of the ops are the to_op */
  for (List_start (from_info->use_op_list);
       (dep = (RC_dep_info *) List_next (from_info->use_op_list));)
    { 
      /* Has dep been deleted? */
      if ((!dep->to_op) || (!dep->from_op))
	continue;
      
      dep_op = dep->to_op;
      if (dep_op == to_op)
	{
	  *path_list = List_insert_last(*path_list, dep_op);
	  return 1;
	}
    }

  /* Search a second time recusively */
  for (List_start (from_info->use_op_list);
       (dep = (RC_dep_info *) List_next (from_info->use_op_list));)
    { 
      /* Has dep been deleted? */
      if ((!dep->to_op) || (!dep->from_op))
	continue;
      
      dep_op = dep->to_op;
      if (RC_compute_dep_path_rec(dep_op, to_op, path_list))
	{
	  *path_list = List_insert_last(*path_list, dep_op);
	  return 1;
	}
    }

  /* This op never leads to to_op */
  from_op->flags = L_SET_BIT_FLAG (from_op->flags, L_OPER_RESERVED_TEMP2);
  /* Op no longer in progress */
  from_op->flags = L_CLR_BIT_FLAG (from_op->flags, L_OPER_RESERVED_TEMP1);
  return 0;
}

List
RC_compute_dep_path(L_Func *fn, L_Oper *from_op, L_Oper *to_op)
{
  List path_list = NULL;
  L_Cb   *cb = NULL;
  L_Oper *op = NULL;

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      for (op=cb->first_op; op; op=op->next_op)
	{
	  op->flags = L_CLR_BIT_FLAG (op->flags, (L_OPER_RESERVED_TEMP1 |
						  L_OPER_RESERVED_TEMP2));
	}
    }

  if (RC_compute_dep_path_rec(from_op, to_op, &path_list))
    {
      path_list = List_insert_last(path_list, from_op);
    }

  return path_list;
}


/*##########################################################
 ##########################################################*/
int
RC_new_find_lds_and_chk(L_Func *fn, List *lds_list, List reg_list)
{
  L_Oper *op = NULL;
  L_Cb   *cb = NULL;
  RC_flow_info *lds_info   = NULL;
  L_Oper *lds_op = NULL;
  L_Oper *chk_op = NULL;
  L_Attr *chk_attr = NULL; 
  L_Attr *lds_attr = NULL; 
  List chk_list = NULL;
  int errors = 0;

  for (cb=fn->first_cb; cb; cb=cb->next_cb)
    {
      for (op=cb->first_op; op; op=op->next_op)
	{
	  if (L_general_load_opcode (op) &&
	      L_EXTRACT_BIT_VAL (op->flags, L_OPER_MASK_PE))
	    {
	      *lds_list = List_insert_last(*lds_list, op);
	    }
	  else if (op->opc == Lop_CHECK)
	    {
	      chk_list = List_insert_last(chk_list, op);
	    }

	}
    }
  
  for (List_start (*lds_list); (lds_op=(L_Oper*)List_next(*lds_list));)
    {
      lds_info = RC_FLOW_INFO (lds_op);
      if (!(lds_attr = L_find_attr (lds_op->attr, "SPECID")))
        L_punt ("RC_new_find_lds_and_chk: Speculative loads must have SPECID, op%d\n",
                lds_op->id);
      
      for (List_start(chk_list); (chk_op=(L_Oper*)List_next(chk_list));)
	{
          if (!(chk_attr = L_find_attr (chk_op->attr, "SPECID")))
            L_punt ("RC_new_find_lds_and_chk: Checks must have SPECID, op%d\n",
                    chk_op->id);
	  if (lds_attr->field[0]->value.i != chk_attr->field[0]->value.i)
	    continue;
	  
	  lds_info->chk_list = List_insert_last(lds_info->chk_list, chk_op);
	}

      if (List_size(lds_info->chk_list) == 0)
	{
	  errors++;
	  printf("ERROR: RC_new_find_lds_and_chk: no checks for load [%d]\n",
		 lds_op->id);
	}
#if 0
      printf("(Initial Checks) lds_op %d: ",lds_op->id);
      for (List_start(lds_info->chk_list);(chk_op=(L_Oper*)List_next(lds_info->chk_list));)
	{
	  printf(" %d",chk_op->id);
	}
      printf("\n");
#endif
    }

  List_reset(chk_list);
  
  return errors;
}


/*##########################################################
 ##########################################################*/
Set
RC_get_NAT_reaching_oper(PRED_FLOW * pred_flow, L_Func *fn,
			 L_Oper *oper, L_Operand *oprd)
{
  L_Oper *op = NULL;
  L_Cb   *cb = NULL;
  PF_OPERAND *pf_operand;
  PF_OPER *pf_oper;
  Set RIN;
  Set NAT_reach;
  int defining_pf_operand_id;

  NAT_reach = NULL;
  pf_oper =
    (PF_OPER *) HashTable_find_or_null (pred_flow->hash_oper_pfoper,
                                        oper->id);
  if (!pf_oper)
    L_punt ("RC_get_NAT_reaching_oper: pf_oper %d not in hash\n", oper->id);

  defining_pf_operand_id = -1;
  PF_FOREACH_OPERAND(pf_operand, pf_oper->dest)
    if (pf_operand->operand == oprd)
      {
        defining_pf_operand_id = pf_operand->id;
        break;
      }
  if (defining_pf_operand_id == -1)
    L_punt ("RC_get_NAT_reaching_oper: no pf_operand found for destination operand");
  
  /* Generate new sources/dests */
  for (cb=fn->first_cb; cb; cb=cb->next_cb)
    {
      for (op=cb->first_op; op; op=op->next_op)
	{
	  pf_oper =
	    (PF_OPER *) HashTable_find_or_null (pred_flow->hash_oper_pfoper, op->id);
	  if (!pf_oper)
	    L_punt ("RC_get_NAT_reaching_oper: pf_oper %d not in hash\n", op->id);
	  RIN = pf_oper->info->r_in;

	  if (!Set_in (RIN, defining_pf_operand_id))
	    continue;
	  NAT_reach = Set_add(NAT_reach, op->id);
	}
    }

  return NAT_reach;
}


/*##########################################################
 ##########################################################*/
void RC_add_NAT_define(L_Func *fn, L_Oper *before_op, L_Operand *pred, 
		       L_Operand *oprd, char *str, int before)
{
  L_Attr    *attr = NULL;
  L_Oper    *new_op  = NULL;
  L_Cb      *before_cb  = NULL;

  before_cb = L_oper_hash_tbl_find_cb (fn->oper_hash_tbl, before_op->id);
  
  new_op = L_create_new_op (Lop_DEFINE);
  new_op->dest[0] = L_copy_operand(oprd);
  if (pred)
    new_op->pred[0] = L_copy_operand(pred);
  new_op->attr = NULL;
  attr = L_new_attr (str, 0);
  new_op->attr = L_concat_attr (new_op->attr, attr);

  if (before)
    L_insert_oper_before (before_cb, before_op, new_op);  
  else
    L_insert_oper_after (before_cb, before_op, new_op);     
}


/*##########################################################
 ##########################################################*/
int
RC_generate_NAT_reach(L_Func *fn, List lds_list)
{
  L_Oper     *op  = NULL;
  L_Oper     *next_op  = NULL;
  L_Cb       *cb  = NULL;
  L_Cb       *dest_cb  = NULL;
  L_Oper *lds_op  = NULL;
  L_Oper *chk_op  = NULL;
  L_Operand *oprd = NULL;
  RC_flow_info *info   = NULL;
  int id_adjust = 0;
  int max_id;
  int errors = 0;

  /* Add in all destinations for NATs on lds_ops and checks 
   */
  max_id = fn->max_reg_id+1;
  for (List_start (lds_list); (lds_op = (L_Oper *) List_next (lds_list));)
    {
      info = RC_FLOW_INFO (lds_op);
      id_adjust ++;
      oprd = L_new_register_operand ((id_adjust + max_id),
				     L_CTYPE_LLONG, L_PTYPE_NULL);
      
      /* Create define for NAT reg */
      RC_add_NAT_define(fn, lds_op, lds_op->pred[0], 
			oprd, "NATflow", 1);

      for (List_start (info->chk_list); (chk_op=(L_Oper *)List_next(info->chk_list));)
	{
	  if (!L_is_cb(chk_op->src[1]))
	    L_punt("RC_generate_NAT_reach: chk has no cb in src[0] op%d\n",chk_op->id);
	  dest_cb = chk_op->src[1]->value.cb;
	  RC_add_NAT_define(fn, chk_op, chk_op->pred[0], 
			    oprd, "NATflow", 0);
	  RC_add_NAT_define(fn, dest_cb->first_op, NULL, 
			    oprd, "NATflow", 1);
	}

      L_delete_operand(oprd);
    }
  
  /* Run data flow 
   */
  L_do_flow_analysis (fn, REACHING_DEFINITION);

  /* Remove checks from list that are not NAT reachable 
   */
  for (List_start (lds_list); (lds_op = (L_Oper *) List_next (lds_list));)
    {
      RC_flow_info *info          = RC_FLOW_INFO (lds_op);
      Set           lds_NAT_reach = NULL;

      /* Get NAT reach for lds */
      if (!lds_op->prev_op || 
	  !L_find_attr(lds_op->prev_op->attr,"NATflow"))
	L_punt("RC_error_check_recovery_code: ld NAT define not found op%d\n",lds_op->id);
      
      lds_NAT_reach = RC_get_NAT_reaching_oper(PF_default_flow, fn, 
					       lds_op->prev_op, lds_op->prev_op->dest[0]);
      
      for (List_start (info->chk_list); (chk_op=(L_Oper *)List_next(info->chk_list));)
	{
	  if (Set_in(lds_NAT_reach, chk_op->id))
	    continue;
	  info->chk_list = List_delete_current(info->chk_list);

	  errors++;
	  printf("ERROR: RC_generate_NAT_reach: NAT from lds %d does not reach chk %d\n",
		 lds_op->id, chk_op->id);
	}

      lds_NAT_reach = Set_dispose(lds_NAT_reach);
    }

 
  /* Remove temp regs 
   */
  for (cb=fn->first_cb; cb; cb=cb->next_cb)
    {
      for (op=cb->first_op; op; op=next_op)
	{
	  next_op = op->next_op;
	  if (op->opc != Lop_DEFINE)
	    continue;
	  if (!L_find_attr(op->attr,"NATflow"))
	    continue;
	  L_delete_oper(cb,op);
	}
    }

  return errors;
}


/*##########################################################
 ##########################################################*/
List
RC_generate_reg_list(L_Func *fn)
{
  L_Oper       *op = NULL;
  L_Cb         *cb = NULL;
  List          regs = NULL;
  int d;

  /*  Form a list of all registers used
   */
  for (cb=fn->first_cb; cb; cb=cb->next_cb)
    {
      for (op=cb->first_op; op; op=op->next_op)
	{
	  for (d=0; d<L_max_dest_operand; d++)
	    {
	      if (!op->dest[d])
		continue;
	      if (!L_is_reg(op->dest[d]) && !L_is_macro(op->dest[d]))
		continue;
	      if (L_is_macro(op->dest[d]) && 
		  op->dest[d]->value.mac > L_MAC_P64)
		continue;
	      if (!RC_oprd_in_list (regs, op->dest[d]))
		regs = List_insert_last(regs, op->dest[d]);
	    }
	}
    }

  return regs;
}
 

/*##########################################################
 ##########################################################*/
void
RC_insert_defines_for_list( List reg_list, L_Func *fn, L_Oper *ref_op,  
			    L_Operand *pred, char *str, int before)
{
  L_Operand *oprd   = NULL;
  L_Operand *last_define   = NULL;
 
  for (List_start (reg_list); (oprd = (L_Operand*) List_next (reg_list));)
    {
      if (L_same_operand(oprd, pred))
	last_define = oprd;
    }

#if 0
  if (!before)
    {
      RC_add_NAT_define(fn, ref_op, pred, last_define, str,  before);
    }
#endif

  for (List_start (reg_list); (oprd = (L_Operand*) List_next (reg_list));)
    {
      /* if (!L_same_operand(oprd, pred)) */
      RC_add_NAT_define(fn, ref_op, NULL, oprd, str,  before);
    }
  
#if 0
  if (before)
    {
      RC_add_NAT_define(fn, ref_op, pred, last_define, str,  before);
    }
#endif
}

void
RC_delete_defines_labeled(L_Func *fn, L_Oper *ref_op, char *str, int before)
{
  L_Oper       *op = NULL;
  L_Cb         *cb = NULL;
  L_Oper       *next_op = NULL;
 
  cb = L_oper_hash_tbl_find_cb (fn->oper_hash_tbl, ref_op->id);
  for (op=ref_op; op; op=next_op)
    {
      if (before)
	next_op = op->prev_op;
      else
	next_op = op->next_op;
	
      if (!op->opc == Lop_DEFINE ||
	  !L_find_attr(op->attr,str))
	continue;
      
      L_delete_oper(cb, op);
    }
}


/*##########################################################
 ##########################################################*/
void
RC_insert_dep_breaks (L_Func *fn, L_Oper *lds_op, List reg_list)
{
  RC_flow_info *lds_info = RC_FLOW_INFO (lds_op);
  L_Oper       *chk_op = NULL;

  /* Insert defines before lds_op
   */
#if 0
  RC_insert_defines_for_list(reg_list, fn, lds_op, 
			     lds_op->pred[0], "depbreak", 1);
#endif

  /* Insert defines following checks
   */  
  for (List_start (lds_info->chk_list); (chk_op=(L_Oper *)List_next(lds_info->chk_list));)
    {
      RC_insert_defines_for_list(reg_list, fn, chk_op, 
				 chk_op->pred[0], "depbreak", 0);      
      RC_insert_defines_for_list(reg_list, fn, chk_op->src[1]->value.cb->first_op, 
				 NULL, "depbreak", 1);
    }
}

void
RC_remove_dep_breaks (L_Func *fn, L_Oper *lds_op)
{
  RC_flow_info *lds_info = RC_FLOW_INFO (lds_op);
  L_Oper       *chk_op = NULL;
  
  /* Delete defines before lds_op
   */
  RC_delete_defines_labeled(fn, lds_op, "depbreak", 1);

  /* Delete defines following checks
   */  
  for (List_start (lds_info->chk_list); (chk_op=(L_Oper *)List_next(lds_info->chk_list));)
    {
      RC_delete_defines_labeled(fn, chk_op, "depbreak", 0);
      RC_delete_defines_labeled(fn, chk_op->src[1]->value.cb->first_op, "depbreak", 0);
    }
}


/*##########################################################
 ##########################################################*/
int
RC_find_op_in_rc(L_Oper *chk_op, L_Oper *find_op)
{
  L_Cb *rc_cb = NULL;
  L_Cb *rc_end_cb = NULL;
  L_Cb *cur_cb = NULL;
  L_Oper *op = NULL;
  L_Attr *attr = NULL;
  int found;
  int id1, id2;

  rc_cb = chk_op->src[1]->value.cb;
  rc_end_cb = RC_find_rc_end (rc_cb);

  /* Search down */
  found = 0;
  id2 = find_op->id;
  attr = L_find_attr(find_op->attr,"rc");
  if (attr)
    id2 = attr->field[0]->value.i;
  for (cur_cb=rc_cb; cur_cb; cur_cb=cur_cb->next_cb)
    {
      for (op=cur_cb->first_op; !found && op; op=op->next_op)
	{
	  id1 = op->id;
	  attr = L_find_attr(op->attr,"rc");
	  if (attr)
	    id1 = attr->field[0]->value.i;
	  if (id1 == id2)
	    continue;
	  found = 1;
	}
      if (cur_cb == rc_end_cb)
	{
	  if (!found)
	    return 0;
	  return 1;
	}
    }

  /* Search up */
  found = 0;
  for (cur_cb=rc_cb; cur_cb; cur_cb=cur_cb->prev_cb)
    {
      for (op=cur_cb->first_op; !found && op; op=op->next_op)
	{
	  id1 = op->id;
	  attr = L_find_attr(op->attr,"rc");
	  if (attr)
	    id1 = attr->field[0]->value.i;
	  if (id1 == id2)
	    continue;
	  found = 1;
	}
      if (cur_cb == rc_end_cb)
	{
	  if (!found)
	    return 0;
	  return 1;
	}
    }
	
  L_punt("RC_new_generate_rc_ops: Could not find path "
	 "between cb%d and cb%d\n",
	 rc_cb->id, rc_end_cb->id);
  return 0;
}


/*##########################################################
 ##########################################################*/
int
RC_new_generate_rc_ops(L_Func *fn, List lds_list, List reg_list)
{
  L_Oper *lds_op = NULL;
  List    slice_list = NULL;
  L_Oper *slice_op = NULL;
  int errors = 0;
  int cnt, chk_cnt;
  RC_flow_info *lds_info = NULL;

  cnt = 1;
  chk_cnt = List_size(lds_list);
  for (List_start (lds_list); (lds_op = (L_Oper *) List_next (lds_list));)
    {
      printf("%4d.%-4d ",cnt++,chk_cnt);
      lds_info = RC_FLOW_INFO (lds_op);
      RC_reset_dataflow (fn);

      /* The following should result in normal DU chains except
	 that, all flow deps are broken at the current lds
	 and its checks. 
      */
      RC_insert_dep_breaks (fn, lds_op, reg_list);
#if 0
      PG_setup_pred_graph (fn);
#endif
      L_do_flow_analysis (fn, REACHING_DEFINITION);
#if 1
      RC_dump_lcode (fn, ".rcg_verify");
#endif
      RC_remove_dep_breaks (fn, lds_op);
      RC_generate_global_flow_deps (fn);

      /* Get the lds RC op slice */
      slice_list = RC_compute_slice(lds_op);

      /* Verify individual ops */
      printf("lds %5d [i %3d][c %3d]: ",
	     lds_op->id,
	     RC_get_SPECID(lds_op),
	     List_size(lds_info->chk_list));
      List_start (slice_list);
      while ((slice_op = (L_Oper *) List_next (slice_list)))
	{
	  printf(" %d",slice_op->id);

	  if (slice_op->opc == Lop_CHECK)
	    printf("C");
	  else if (L_is_control_oper(slice_op))
	    printf("B");
	  else if (L_general_load_opcode(slice_op))
	    printf("L");
	  else if (L_general_store_opcode(slice_op))
	    printf("S");
	  else if (L_subroutine_call_opcode(slice_op))
	    printf("J");
	  else if (L_subroutine_return_opcode(slice_op))
	    printf("R");	  
	}
      printf("\n");

      List_start (slice_list);
      while ((slice_op = (L_Oper *) List_next (slice_list)))
	{
	  if (!L_general_store_opcode(slice_op) &&
	      !L_subroutine_call_opcode(slice_op) &&
	      !L_subroutine_return_opcode(slice_op) &&
	      slice_op->opc != Lop_CHECK)
	    {
	      L_Oper *chk_op = NULL;
	      int found;
	      /* Make sure op is in RC */
	      /* For the moment, this is not "complete" because if
		 there are multiple checks this op  might need to 
		 be in one, some, or all of the RC blocks */
		  
	      List_start(lds_info->chk_list);
	      found = 0;
	      while((chk_op = List_next(lds_info->chk_list)))
		{
		  if (!RC_find_op_in_rc(chk_op, slice_op))
		    continue;
		  found = 1;
		  break;
		}
	      if (!found)
		{
		  errors++;
		  printf("ERROR:  RC_new_generate_rc_ops: dep op %d "
			 "not found in any RC for ld %d\n",
			 slice_op->id, lds_op->id);
		}
	    }

	  if (slice_op->opc == Lop_CHECK)
	    {
	      int     lds_id, chk_id;
	      List    path_list = NULL;
	      L_Oper *path_op = NULL;
	      /* See if Check is someone elses */
	      lds_id = RC_get_SPECID(lds_op);
	      chk_id = RC_get_SPECID(slice_op);
	      if (lds_id != chk_id)
		{
		  errors++;
		  printf("ERROR:  RC_new_generate_rc_ops: chk %d SPECID %d "
			 "reached by lds %d SPECID %d\n",
			 slice_op->id, chk_id,
			 lds_op->id, lds_id);

		  if (!(path_list = RC_compute_dep_path(fn, lds_op, slice_op)))
		    L_punt("RC_new_generate_rc_ops: no path found between ops\n");
		  
		  printf("     : Dep Path =");
		  for (List_start(path_list);(path_op=(L_Oper*)List_next(path_list));)
		    {
		      printf(" %d",path_op->id);
		    }
		  printf("\n");
		  List_reset(path_list);
		}
	    }

	  if (L_general_load_opcode(slice_op))
	    {
	      /* Make sure load is speculative */
	      if (!L_EXTRACT_BIT_VAL (slice_op->flags, L_OPER_MASK_PE))
		{
		  errors++;
		  printf("ERROR: RC_new_generate_rc_ops: ld [%d] should be speculative\n",
			 slice_op->id);
		}
	    }

#if 0
	  if (L_general_branch_opcode(slice_op))
	    {
	      errors++;
	      printf("ERROR: RC_new_generate_rc_ops: illegal speculative control ops [%d]\n",
		     slice_op->id);	      
	    }
#endif

	  if (L_general_store_opcode(slice_op) ||
	      L_subroutine_call_opcode(slice_op) ||
	      L_subroutine_return_opcode(slice_op))
	    {
	      List   path_list = NULL;
	      L_Oper *path_op = NULL;
	      /* Error (a NAT leak), provide info */
	      errors++;
	      printf("ERROR: RC_new_generate_rc_ops: NAT leak to STORE/JSR/RTS op [%d]\n",
		     slice_op->id);

	      if (!(path_list = RC_compute_dep_path(fn, lds_op, slice_op)))
		  L_punt("RC_new_generate_rc_ops: no path found between ops\n");

	      printf("     : Dep Path =");
	      for (List_start(path_list);(path_op=(L_Oper*)List_next(path_list));)
		{
		  printf(" %d",path_op->id);
		}
	      printf("\n");
	      List_reset(path_list);
	    }
	}

      List_reset(slice_list);
      /*L_punt("\n");*/
    }

  return errors;
}


/*##########################################################
 ##########################################################*/

/*
 * make sure SPECIDs are not in recovery code.
 *
 */

void
RC_error_check_recovery_code (L_Func * fn)
{
  List    lds_list = NULL;
  List    reg_list = NULL;
  int     errors = 0;

  printf("ERROR CHECKING: START\n");

  /*
   * Setup the pred graph
   */
  PG_setup_pred_graph (fn);

  /*
   * Init internal structures
   */
  RC_init (fn);
  reg_list = RC_generate_reg_list(fn);

  /*
   * Get the basic lds/chk lists
   */
  START_TIME;
  errors += RC_new_find_lds_and_chk(fn, &lds_list, reg_list);
  
  /*
   * Do NAT dataflow
   */
  errors += RC_generate_NAT_reach(fn, lds_list);

  /* 
   * Determine final set of checks/ops
   */
  errors += RC_new_generate_rc_ops(fn, lds_list, reg_list);
  PRINT_DELTA("ERROR Time");      
  if (errors != 0)
    {
      printf("[%d] Errors detected! ",errors);
      if (errors < 5)
	printf("... not too bad");
      else if (errors < 10)
	printf("... some work needed");
      else if (errors < 100)
	printf("... oh my");
      else if (errors < 1000)
	printf("... pray for the good lord's guidance");
      else if (errors < 10000)
	printf("... pray for direct divine intervention");
      else
	printf("... this code is from satan");
      printf("\n");
    }

  /*
   * Delete internal structures
   */
  List_reset(reg_list);
  RC_cleanup(fn);

  printf("ERROR CHECKING: DONE\n");
  fflush(stdout);
}






















/*##########################################################
 ##########################################################*/
/* DEAD, ARCHIVED CODE */
/*##########################################################
 ##########################################################*/


#if 0
/*##########################################################
 * Fix up any previously detected anti-dep issues
  ##########################################################*/
void
RC_fix_anti_deps (List conf_list, List svrt_list, List * psrv_list,
                  L_Cb * start_cb, L_Cb * end_cb,
                  L_Cb * lds_cb, L_Oper * lds_op,
                  L_Cb * chk_cb, L_Oper * chk_op)
{
  L_Operand *tmpreg = NULL;
  L_Operand *oprd   = NULL;
  L_Oper    *new_op = NULL;
  int        lop    = 0;
  int        i      = 0;

#ifdef RC_DEBUG_INFO
  printf ("Step ##5##\n");
#endif

  List_start (conf_list);
  while ((oprd = (L_Operand *) List_next (conf_list)))
    {
      tmpreg = L_new_register_operand (++L_fn->max_reg_id, oprd->ctype, oprd->ptype);
      switch(oprd->ctype)
	{
	case L_CTYPE_PREDICATE:
	  lop = Lop_CMP;
	  break;
	case L_CTYPE_FLOAT:
	  lop = Lop_MOV_F;
	  break;
	case L_CTYPE_DOUBLE:
	  lop = Lop_MOV_F2;
	  break;
	default:
	  lop = Lop_MOV;
	  break;
	}

      /* Put save in orig code
       */
      if (lop == Lop_CMP)
	{
	  /* Saving a predicate
	   */
	  new_op = L_create_new_op (lop);
	  new_op->flags = 0;	  
	  new_op->com[0] = L_CTYPE_INT;
	  new_op->com[1] = Lcmp_COM_EQ;

	  new_op->pred[0] = L_copy_operand (oprd);
	  new_op->pred[0]->ptype = L_PTYPE_NULL;
	  new_op->src[0] = L_new_macro_operand (TAHOE_MAC_ZERO, L_CTYPE_LLONG, 0);
	  new_op->src[1] = L_new_macro_operand (TAHOE_MAC_ZERO, L_CTYPE_LLONG, 0);
	  new_op->dest[0] = tmpreg;
	  new_op->dest[0]->ptype =  L_PTYPE_UNCOND_T;

	  /*
	   * Add op as a use in dep_info 
	   */
	  RC_copy_oper_deps(lds_op, new_op);
	  /*RC_add_def_using(lds_op, new_op, new_op->pred[0]);*/
	}
      else
	{
	  new_op = L_create_new_op (lop);
	  new_op->flags = 0;
	  for (i = 0; i < L_max_pred_operand; i++)
	    new_op->pred[i] = L_copy_operand (lds_op->pred[i]);
	  new_op->src[0] = L_copy_operand (oprd);
	  new_op->dest[0] = tmpreg;

	  /*
	   * Add op as a use in dep_info 
	   */
	  RC_copy_oper_deps(lds_op, new_op);
 	  /*RC_add_def_using(lds_op, new_op, new_op->src[0]);*/
	}
      L_insert_oper_before (lds_cb, lds_op, new_op);


      /* Also put save before any copies of lds_op pre-existing
       *  in recovery code
       */
      {
	L_Cb   *cb;
	L_Oper *op;
	L_Attr *attr;

	for (cb=L_fn->first_cb;cb;cb=cb->next_cb)
	  {
	    if (cb == start_cb)
	      break;
	    if (!(attr=L_find_attr(cb->attr,"rc")))
	      continue;
	    if (attr->field[0]->value.i != lds_cb->id)
	      continue;
	    for (op=cb->first_op; op; op=op->next_op)
	      {
		if (!(attr=L_find_attr(op->attr,"rc")))
		  continue;
		if (attr->field[0]->value.i != lds_op->id)
		  continue;
		
		new_op = RC_copy_op(new_op, NULL);
		L_insert_oper_before (cb, op, new_op);
		printf("Inserting copy of move op%d before op%d\n",
		       new_op->id,op->id);
		break;
	      }
	  }
      }      

      /* Put restore in RC 
       */
      if (lop == Lop_CMP)
	{
	  /* Restoring a predicate
	   */
	  new_op = L_create_new_op (lop);
	  new_op->flags = 0;	  
	  new_op->com[0] = L_CTYPE_INT;
	  new_op->com[1] = Lcmp_COM_EQ;

	  new_op->pred[0] = L_copy_operand (tmpreg);
	  new_op->pred[0]->ptype = L_PTYPE_NULL;
	  new_op->src[0] = L_new_macro_operand (TAHOE_MAC_ZERO, L_CTYPE_LLONG, 0);
	  new_op->src[1] = L_new_macro_operand (TAHOE_MAC_ZERO, L_CTYPE_LLONG, 0);
	  new_op->dest[0] = L_copy_operand (oprd);
	  new_op->dest[0]->ptype =  L_PTYPE_UNCOND_T;
	}
      else
	{
	  new_op = L_create_new_op (lop);
	  new_op->flags = 0;
	  for (i = 0; i < L_max_pred_operand; i++)
	    new_op->pred[i] = L_copy_operand (lds_op->pred[i]);
	  new_op->src[0] = L_copy_operand (tmpreg);
	  new_op->dest[0] = L_copy_operand (oprd);
	}
      L_insert_oper_before (start_cb, start_cb->first_op, new_op);

      *psrv_list = List_insert_last (*psrv_list, tmpreg);
    }

  List_start (svrt_list);
  while ((oprd = (L_Operand *) List_next (svrt_list)))
    {
      tmpreg = L_new_register_operand (++L_fn->max_reg_id, oprd->ctype, oprd->ptype);
      switch(oprd->ctype)
	{
	case L_CTYPE_FLOAT:
	  lop = Lop_MOV_F;
	  break;
	case L_CTYPE_DOUBLE:
	  lop = Lop_MOV_F2;
	  break;
	default:
	  lop = Lop_MOV;
	  break;
	}

      /* Put save at start of RC
       */
      new_op = L_create_new_op (lop);
      for (i = 0; i < L_max_pred_operand; i++)
        new_op->pred[i] = L_copy_operand (lds_op->pred[i]);
      new_op->flags = 0;
      new_op->src[0] = L_copy_operand (oprd);
      new_op->dest[0] = tmpreg;
      L_insert_oper_before (start_cb, start_cb->first_op, new_op);

      /* Put restore at end of RC 
       */
      new_op = L_create_new_op (lop);
      for (i = 0; i < L_max_pred_operand; i++)
        new_op->pred[i] = L_copy_operand (lds_op->pred[i]);
      new_op->flags = 0;
      new_op->src[0] = L_copy_operand (tmpreg);
      new_op->dest[0] = L_copy_operand (oprd);
      L_insert_oper_after (end_cb, end_cb->last_op, new_op);
    }
}
#endif


#if 0
/*##########################################################
 * Look at anti-deps and determine what needs to be fixed
  ##########################################################*/
int
RC_get_anti_ops_for_rc (List cb_list, Set vop_set,
                        List * rc_list, List psrv_list,
                        List * conf_list, List * svrt_list,
                        L_Cb * lds_cb, L_Oper * lds_op,
                        L_Cb * chk_cb, L_Oper * chk_op)
{
  L_Cb *cur_cb = NULL;
  L_Oper *cur_op = NULL;
  RC_flow_info *cur_info = NULL;
  RC_dep_info *dep = NULL;
  RC_dep_info *dep2 = NULL;
  L_Oper *dep_op = NULL;
  L_Cb *dep_cb = NULL;
  L_Operand *oprd = NULL;
  L_Operand *anti_oprd = NULL;
  List op_list = NULL;
  List op2_list = NULL;
  int change = 0;

#ifdef RC_DEBUG_INFO
  printf ("Step ## 3.2 ##\n");
#endif

  change = 0;
  List_start (*rc_list);
  while ((cur_op = (L_Oper *) List_next (*rc_list)))
    {
      cur_info = RC_FLOW_INFO (cur_op);
      cur_cb = cur_info->cb;

      for (List_start (cur_info->anti_def_op_list);
           (dep = (RC_dep_info *) List_next (cur_info->anti_def_op_list));)
        {
	  /* Has dep been deleted? */
	  if ((!dep->to_op) || (!dep->from_op))
	    continue;

          dep_op = dep->from_op;
          dep_cb = RC_FLOW_INFO (dep_op)->cb;

          if (!List_member (cb_list, dep_cb))
            continue;

	  /* Is op in the maximal op set
	   */
	  if (!Set_in (vop_set, dep_op->id))
	    {
	      printf("#1 Excluding dep_op %d, cur_op %d\n",
		     dep_op->id, cur_op->id);
	      continue;
	    }
	  
	  /* Is it valid to go from cur_cb/op to dep cb/op 
	   */
	  if (!RC_IsValidControl(cur_cb, cur_op, dep_cb, dep_op))
	    {
	      printf("#2: Excluding dep_op %d, cur_op %d\n",
		     dep_op->id, cur_op->id);
	      continue;
	    }

          /* Anti dep found 
           */
	  anti_oprd = dep->oprd;
          if (RC_oprd_in_list (psrv_list, dep->oprd))
            {
              /* save oprd value before lds AND
               * restore oprd to old value before RC
               */
              if (!RC_oprd_in_list (*conf_list, dep->oprd))
                *conf_list = List_insert_last (*conf_list, dep->oprd);

              /* Add op to RC */
              if (!List_member (op_list, dep_op) &&
		  !List_member (*rc_list, dep_op))
		{
		  if (L_general_load_opcode(dep_op))
		    L_punt("(1) A load op %d\n",dep_op->id);
		  op_list = List_insert_last (op_list, dep_op);
		  *rc_list = List_insert_last (*rc_list, dep_op);
		  change = 1;
		}
            }
          else if (!List_member(*rc_list, dep_op) ||
		   List_member(op_list, dep_op))
            {
              /* save and restore oprd around RC, but
	       * only if the value written can not
	       * contain the NAT
               */
              if (!RC_oprd_in_list (*svrt_list, dep->oprd))
                *svrt_list = List_insert_last (*svrt_list, dep->oprd);
            }
	  else
	    printf("NO SAVE/RESTORE FOR op%d, COULD BE NAT\n",dep_op->id);

	  
	  /* Given that we have found an op that redefines a src
	   *  of this op. Can we find a non-RC flow dep?
	   * We've found:
	   *    ld.s
	   *    op5      = r2, r3
	   *    op6   r2 =
	   * Is there:
	   *    ld.s
	   *  ->op4   r2 = 
	   *    op5      = r2, r3
	   *    op6   r2 =
	   */
	  for (List_start (cur_info->def_op_list);
	       (dep2 = (RC_dep_info *) List_next (cur_info->def_op_list));)
	    {
	      /* Has dep been deleted? */
	      if ((!dep2->to_op) || (!dep2->from_op))
		continue;
	      
	      dep_op = dep2->from_op;
	      dep_cb = RC_FLOW_INFO (dep_op)->cb;
	      
	      /* Is this dep on the same oprd */
	      if (!L_same_operand(dep2->oprd, anti_oprd))
		continue;

	      /* Not already added or in RC */
	      if (List_member (*rc_list, dep_op))
		continue;
	      
	      /* Is op in the maximal op set
	       */
	      if (!Set_in (vop_set, dep_op->id))
		{
		  printf("#3 Excluding dep_op %d, cur_op %d\n",
			 dep_op->id, cur_op->id);
		  continue;
		}
	  
	      /* Is it valid to go from cur_cb/op to dep cb/op 
	       */
	      if (!RC_IsValidControl(dep_cb, dep_op, cur_cb, cur_op))
		{
		  printf("#4: Excluding dep_op %d, cur_op %d\n",
			 dep_op->id, cur_op->id);
		  continue;
		}
	      
	      if (L_general_load_opcode(dep_op))
		L_punt("(2) A load op %d\n",dep_op->id);
	      op2_list = List_insert_last (op2_list, dep_op);
	      *rc_list = List_insert_last (*rc_list, dep_op);
	      change = 1;
	    }
        }
    }
  

#ifdef RC_DEBUG_INFO
  printf ("     %4s:%4s          %4s:%4s ANTI =  ", "", "", "", "");
  List_start (op_list);
  while ((cur_op = (L_Oper *) List_next (op_list)))
    {
      printf ("%d ", cur_op->id);
    }
  printf ("\n");
  printf ("     %4s:%4s          %4s:%4s OUTP =  ", "", "", "", "");
  List_start (op2_list);
  while ((cur_op = (L_Oper *) List_next (op2_list)))
    {
      printf ("%d ", cur_op->id);
    }
  printf ("\n");
  printf ("     %4s:%4s          %4s:%4s CONF =  ", "", "", "", "");
  List_start (*conf_list);
  while ((oprd = (L_Operand *) List_next (*conf_list)))
    {
      L_print_operand (stdout, oprd, 0);
    }
  printf ("\n");
  printf ("     %4s:%4s          %4s:%4s SVRT =  ", "", "", "", "");
  List_start (*svrt_list);
  while ((oprd = (L_Operand *) List_next (*svrt_list)))
    {
      L_print_operand (stdout, oprd, 0);
    }
  printf ("\n");
#endif

  List_reset(op_list);
  List_reset(op2_list);
  return change;
}



/*##########################################################
  ##########################################################*/
void
RC_get_dests_for_rc (List rc_list, List * dest_list)
{
  L_Oper    *cur_op = NULL;
  L_Operand *oprd   = NULL;
  int        i;
  
#ifdef RC_DEBUG_INFO
  printf ("Step ## 3.3 ##\n");
#endif

  List_start (rc_list);
  while ((cur_op = (L_Oper *) List_next (rc_list)))
    {
      for (i=0; i<L_max_dest_operand; i++)
	{
	  if (!cur_op->dest[i])
	    continue;

	  if (RC_oprd_in_list (*dest_list, cur_op->dest[i]))
	    continue;
	  
	  *dest_list = List_insert_last (*dest_list, cur_op->dest[i]);
        }
    }

#ifdef RC_DEBUG_INFO
  printf ("     %4s:%4s          %4s:%4s DEST =  ", "", "", "", "");
  List_start (*dest_list);
  while ((oprd = (L_Operand *) List_next (*dest_list)))
    {
      L_print_operand (stdout, oprd, 0);
    }
  printf ("\n");
#endif
}


/*##########################################################
 * Look at incoming defs and determine if there are 
 *   psuedo-"output" dependence issues
 * More precisely, if an opA (not dependent on the lds)
 *   feeds an opB (which is dependent on the load), and opA
 *   is in the speculative region, and opA kills a dest of the
 *   current RC ops then opA needs to be added to rc ops. 
  ##########################################################*/
int
RC_get_def_ops_for_rc (List cb_list, List dest_list,
		       List * rc_list,
		       L_Cb * lds_cb, L_Oper * lds_op,
		       L_Cb * chk_cb, L_Oper * chk_op)
{
  L_Cb *cur_cb = NULL;
  L_Oper *cur_op = NULL;
  RC_flow_info *cur_info = NULL;
  RC_dep_info *dep = NULL;
  L_Oper *dep_op = NULL;
  L_Cb *dep_cb = NULL;
  List op_list = NULL;
  int change = 0;

  return 0;

#ifdef RC_DEBUG_INFO
  printf ("Step ## 3.4 ##\n");
#endif

  change = 0;
  List_start (*rc_list);
  while ((cur_op = (L_Oper *) List_next (*rc_list)))
    {
      cur_info = RC_FLOW_INFO (cur_op);
      cur_cb = cur_info->cb;

      for (List_start (cur_info->def_op_list);
           (dep = (RC_dep_info *) List_next (cur_info->def_op_list));)
        {
	  /* Has dep been deleted? */
	  if ((!dep->to_op) || (!dep->from_op))
	    continue;

          dep_op = dep->from_op;
          dep_cb = RC_FLOW_INFO (dep_op)->cb;

	  /* Not already added or in RC */
	  if (List_member (op_list, dep_op))
	    continue;
          if (List_member (*rc_list, dep_op))
            continue;

	  /* Is this op within RC? */
          if (!List_member (cb_list, dep_cb))
            continue;

	  /* The ordering of depand cur is reversed from
	     get_rc_ops because we are looking backward to
	     def and not forward to use */
          if (!RC_valid_dependence (dep_cb, dep_op,
				    cur_cb, cur_op,
                                    lds_cb, lds_op, chk_cb, chk_op))
            continue;

	  /* Is dest oprd redefined in RC */
	  if (!RC_oprd_in_list (dest_list, dep->oprd))
	    continue;
	  
	  op_list = List_insert_last (op_list, dep_op);
	  *rc_list = List_insert_last (*rc_list, dep_op);
	  change = 1;
        }
    }

#ifdef RC_DEBUG_INFO
  printf ("     %4s:%4s          %4s:%4s OUTP =  ", "", "", "", "");
  List_start (op_list);
  while ((cur_op = (L_Oper *) List_next (op_list)))
    {
      printf ("%d ", cur_op->id);
    }
  printf ("\n");
#endif
  
  List_reset(op_list);
  return change;
}
#endif


#if 0
/*##########################################################
 *  Recursive helper for RC_cbs_reachable_from
 ##########################################################*/

/* These defines just make everything more readable 
 * INPROGRESS  : L_CB_RESERVED_TEMP1 : Cb is currently in path
 * FINISHED    : L_CB_RESERVED_TEMP2 : Cb never leads to a check
 * REACHES     : L_CB_RESERVED_TEMP3 : Cb reaches an included check
 */

#define RC_CB_INPROGRESS L_CB_RESERVED_TEMP1
#define RC_CB_FINISHED   L_CB_RESERVED_TEMP2
#define RC_CB_REACHES    L_CB_RESERVED_TEMP3

#define RC_RETURN_FINISHED   1
#define RC_RETURN_INPROGRESS 2
#define RC_RETURN_REACHES    3
#define RC_RETURN_COVERED    4

/* cb       : what cb to search from
 * start_op : op in cb from which to start (NULL = all ops)
 * vop_set  : valid op set, the maximal set of ops for RC 
 *            for included checks
 * ctrl_list: maximal set of required control ops for RC
 */

/*
  # Pre elimination of "FINISHED" cbs
  - i.e. ones that can never reach the check
  - There might be a bug related to eliminating
    cbs during the DFS search. This is because a
    check might not be reachable along one path,
    but is along another. Pre-elimination would
    remove those cbs whose flows never lead to the
    check. After that, every path must be evaluated.
  # Backwards use of subsumption
  - this will eliminate ops that are covered by following
    checks/jmps
*/

void
RC_cbs_reachable_mark_ignore(L_Func *fn, List chk_op_list)
{
  L_Flow *flow = NULL;
  L_Oper *op = NULL;
  L_Cb   *cb = NULL;
  int change;
  
  /* Mark everything as not reaching except check node(s) */
  for (cb=fn->last_cb; cb; cb=cb->prev_cb)
    {
      cb->flags = L_SET_BIT_FLAG (cb->flags, RC_CB_FINISHED);
    }
  for (List_start(chk_op_list); (op=(L_Oper*)List_next(chk_op_list));)
    {
      cb = L_oper_hash_tbl_find_cb (fn->oper_hash_tbl, op->id);
      cb->flags = L_CLR_BIT_FLAG (cb->flags, RC_CB_FINISHED);
    }

  /* Iterate through cbs marking those that can be lead 
   * to check cb
   */
  do {
    change = 0;
    for (cb=fn->last_cb; cb; cb=cb->prev_cb)
      {
	if (L_EXTRACT_BIT_VAL(cb->flags, RC_CB_FINISHED))
	  continue;
	/* Mark all sources are reaching */
	for (flow=cb->src_flow; flow; flow=flow->next_flow)
	  {
	    if (L_EXTRACT_BIT_VAL(flow->src_cb->flags, RC_CB_FINISHED))
	      {
		flow->src_cb->flags = L_CLR_BIT_FLAG (flow->src_cb->flags, 
						      RC_CB_FINISHED);
		change = 1;
	      }
	  }
      }
  } while (change);
}

static int
RC_cbs_reachable_from_rec2 (L_Cb * cb, L_Oper *start_op, L_Oper *lds_op,
			    List chk_op_list, List exclude_chk_op_list, 
			    List* pre_chk_list, List* pre_jump_list,
			    Set* vop_set, List* ctrl_list,
			    List* com_chk_list, int *com_empty,
			    int level)
{
  L_Flow *flow         = NULL;
  L_Flow *start_flow   = NULL;
  L_Oper *op           = NULL;
  L_Oper *last_chk_op  = NULL;
  List    added_list   = NULL;
  /*List    tmp_list     = NULL;*/
  Set     tmp_op_set   = NULL;
  int reaches, finished, changed, update;
  int exclude, include;

#if 0
  {
    int i;
    for (i=0;i<level;i++)
      printf("  ");
    printf("%d->\n",cb->id);
    
    if (L_EXTRACT_BIT_VAL (cb->flags, RC_CB_FINISHED) ||
	L_EXTRACT_BIT_VAL (cb->flags, RC_CB_INPROGRESS))
      {
	for (i=0;i<level;i++)
	  printf("  ");
	printf("%d<-",cb->id);
	if (finished)
	  printf("(stop)\n");
      }
  }
#endif

  if (L_EXTRACT_BIT_VAL (cb->flags, RC_CB_FINISHED))
    {
      return RC_RETURN_FINISHED;
    }
  
  if (L_EXTRACT_BIT_VAL (cb->flags, RC_CB_INPROGRESS))
    {
      return RC_RETURN_INPROGRESS;
    }  
  if (level != 1)
    cb->flags = L_SET_BIT_FLAG (cb->flags, RC_CB_INPROGRESS);
  

  /* Use a starting op, find starting flow
   */
  start_flow = cb->dest_flow;
  for (op=cb->first_op; (op && (op!=start_op)); op=op->next_op)
    {
      if (L_is_control_oper (op))
	start_flow = start_flow->next_flow;
    }
  if (op != start_op)
    L_punt ("RC_cbs_reachable_from: start_op not found\n");


  /* This is a key loop that looks for checks and performs
   *   the recursion. 
   */
  last_chk_op = op;
  reaches = 0;
  changed = 0;
  update = 1;
  finished = 1;
  flow = start_flow;
  for (op=start_op; op; op=op->next_op)
    {
      if (level != 1 && op == lds_op)
	break;

      if (L_check_opcode(op))
	{
	  include = List_member(chk_op_list, op);
	  exclude = List_member(exclude_chk_op_list, op);

	  if (include || exclude)
	    {	    
	      finished = 0;
	      
	      if (!PG_collective_subsumption(*pre_chk_list, op))
		{
		  /* The check covers a non-redundant predicate
		   */
		  tmp_op_set = RC_add_reachable_op(tmp_op_set, *pre_chk_list, 
						   last_chk_op, op);
		  last_chk_op = op;
		  
		  if (include)
		    {
		      RC_AddValidCb(cb, cb);
		      /* Add ops from reachable_op to current op
		       *   while exluding any ops whose predicates are
		       *   a subsumed by preceeding checks
		       */
		      *vop_set = Set_union_acc(*vop_set, tmp_op_set);
		      tmp_op_set = Set_dispose(tmp_op_set);
		      /*reachable_op = op;*/
		      reaches = 1;
		    }

		  /* A NEW CHECK HAS BEEN ENCOUNTERED ALONG PATH
		   */
		  *pre_chk_list = List_insert_last(*pre_chk_list, op);
		  added_list = List_insert_last(added_list, op);
		  changed = 1;
		  
		  if (PG_collectively_exhaustive_predicates_ops(*pre_chk_list))
		    {
		      break;
		    }
		}
	    }
	}
      
      if (L_is_control_oper(op))
	{
	  /* Only recurse along normal branch flows
	   *  and only along ones not covered by checks already
	   */
	  if (!L_check_opcode(op))
	    {
	      if (!PG_collective_subsumption(*pre_chk_list, op))
		{
		  switch(RC_cbs_reachable_from_rec2(flow->dst_cb, flow->dst_cb->first_op, lds_op,
						    chk_op_list, exclude_chk_op_list, 
						    pre_chk_list, pre_jump_list,
						    vop_set, ctrl_list, 
						    com_chk_list, com_empty,
						    level+1))
		    {
		    case RC_RETURN_FINISHED:
		      break;
		    case RC_RETURN_INPROGRESS:
		      break;
		    case RC_RETURN_COVERED:
		      finished = 0;
		      break;
		    case RC_RETURN_REACHES:
		      RC_AddValidCb(cb, cb);
		      RC_UnionValidCb(cb, flow->dst_cb);
		      if (!List_member(*ctrl_list, op))
			*ctrl_list = List_insert_last(*ctrl_list, op); 
		      
		      tmp_op_set = RC_add_reachable_op(tmp_op_set, *pre_chk_list, 
						       last_chk_op, op);
		      *vop_set = Set_union_acc(*vop_set, tmp_op_set);
		      tmp_op_set = Set_dispose(tmp_op_set);
		      last_chk_op = op;
		      
		      /*reachable_op = op;*/
		      reaches = 1;
		      finished = 0;
		      update = 0;
		      break;
		    default:
		      L_punt("RC_cbs_reachable_from_rec: invalid return value");
		    }

		  /* A NEW JUMP HAS BEEN ENCOUNTERED ALONG PATH
		   */
		  *pre_jump_list = List_insert_last(*pre_jump_list, op);
		  added_list = List_insert_last(added_list, op);
		  changed = 1;
		}
	      else
		printf("--> Skipping path to from cb %d to cb %d\n",
		       cb->id, flow->dst_cb->id);
	    }
	  flow = flow->next_flow;
	}
      
#if 0
      /* Has EVERY case has been covered:
       *  either a check fired or a jump was taken
       */
      tmp_list = RC_union_lists(*pre_chk_list, *pre_jump_list);
      if (changed && 
	  PG_collectively_exhaustive_predicates_ops(tmp_list))
	{
	  /*
	    if (!PG_collectively_exhaustive_predicates_ops(*pre_chk_list))
	    printf("--> Early stop in cb %d\n", cb->id);
	  */
	  List_reset(tmp_list);
	  break;
	}
      if (PG_collectively_exhaustive_predicates_ops(*pre_chk_list))
	L_punt("PG COL HERE\n");

      changed = 0;
      List_reset(tmp_list);      
#endif
    }


  if (!op && flow)
    {
      /* Check the fall through path
       */
      switch(RC_cbs_reachable_from_rec2(flow->dst_cb, flow->dst_cb->first_op, lds_op,
					chk_op_list, exclude_chk_op_list, 
					pre_chk_list, pre_jump_list,
					vop_set, ctrl_list, 
					com_chk_list, com_empty,
					level+1))
	{
	case RC_RETURN_FINISHED:
	  break;
	case RC_RETURN_INPROGRESS:
	  break;
	case RC_RETURN_COVERED:
	  finished = 0;
	  break;
	case RC_RETURN_REACHES:
	  RC_AddValidCb(cb, cb);
	  RC_UnionValidCb(cb, flow->dst_cb);

	  tmp_op_set = RC_add_reachable_op(tmp_op_set, *pre_chk_list, 
					   last_chk_op,  cb->last_op);
	  *vop_set = Set_union_acc(*vop_set, tmp_op_set);
	  
	  /*reachable_op = cb->last_op;*/
	  reaches = 1;
	  finished = 0;
	  break;
	default:
	  L_punt("RC_cbs_reachable_from_rec: invalid return value");
	}
    }
  tmp_op_set = Set_dispose(tmp_op_set);

  if (update)
    {
      if (*com_empty)
	{
	  List_start(*pre_chk_list);
	  while ((op=List_next(*pre_chk_list)))
	    {
	      printf(" %d",op->id);
	      *com_chk_list = List_insert_last(*com_chk_list, op);
	    }
	  printf("\n");
	  *com_empty = 0;
	}
      else
	{
	  List_start(*com_chk_list);
	  while ((op=List_next(*com_chk_list)))
	    {
	      if (List_member(*pre_chk_list, op))
		continue;
	      printf(" -%d",op->id);
	      *com_chk_list = List_delete_current(*com_chk_list);
	      printf("\n");
	    }
	}
    }

  /* Clear inprogress flag and remove ops 
   *   added to pre_chk_list and pre_jump_list
   */
  cb->flags = L_CLR_BIT_FLAG (cb->flags, RC_CB_INPROGRESS);
  List_start(*pre_chk_list);
  while ((op=List_next(*pre_chk_list)))
    {
      if (List_member(added_list, op))
	*pre_chk_list = List_delete_current(*pre_chk_list);
    }
  List_start(*pre_jump_list);
  while ((op=List_next(*pre_jump_list)))
    {
      if (List_member(added_list, op))
	*pre_jump_list = List_delete_current(*pre_jump_list);
    }
  List_reset(added_list);

#if 0
  {
    int i;
    for (i=0;i<level;i++)
      printf("  ");
    printf("%d<-",cb->id);
    if (finished)
      printf("F");
    else if (reaches)
      printf("R");
    else
      printf("C");
    printf("\n");
  }
#endif

  if (finished)
    {
      /* No check was encountered along any path
       */
#if 1
      cb->flags = L_SET_BIT_FLAG (cb->flags, RC_CB_FINISHED);
#endif
      return RC_RETURN_FINISHED;
    }

  if (reaches)
    {
      /* A useful check can be reached along some path 
       */
      cb->flags = L_SET_BIT_FLAG (cb->flags, RC_CB_REACHES);

      /* Add ops from this cb to valid op list
       */
      /*
	for (op=start_op; op!=reachable_op->next_op; op=op->next_op)
	{
	*vop_set = Set_add(*vop_set, op->id);
	}
      */
      
      return RC_RETURN_REACHES;
    }
  
  /* Only excluded or redundant checks encountered
   */
  return RC_RETURN_COVERED;
}

/*##########################################################
 *  Given a start_cb+op, find the set of cbs that form a path 
 *    from start_cb+op to check_cb without going back through 
 *    start_cb or check_cb.
 ##########################################################*/

void
RC_cbs_reachable_from2 (L_Func * fn, 
		       L_Cb * start_cb, L_Oper * start_op,
		       List chk_op_list, List exclude_chk_op_list, 
		       Set *vop_set, List *ctrl_list, List *cb_list)
{
  List    pre_chk_list        = NULL;
  List    pre_jump_list       = NULL;
  L_Cb   *cb                  = NULL;
  L_Cb   *rc_cb               = NULL;
  L_Oper *op                  = NULL;
  List    com_chk_list = NULL;
  int com_empty;

#ifdef RC_DEBUG_INFO
  printf ("Step ## 1 ##\n");
  printf ("Load %4d:%4d -> \n",
	  start_cb->id, start_op->id);
  List_start(chk_op_list);
  while((op=List_next(chk_op_list)))
    {
      L_Cb *cb = L_oper_hash_tbl_find_cb (fn->oper_hash_tbl, op->id);
      printf ("     %4s:%4s -> Check op%4d cb%4d\n",
	      "", "", op->id, cb->id); 
    }
#endif
  
  if (List_size(chk_op_list) == 0)
    L_punt("RC_cbs_reachable_from: no check ops provided\n");
  if (!start_op)
    L_punt("RC_cbs_reachable_from: start_op can't be NULL\n");

  RC_InitCbInfo(fn);
  pre_chk_list = NULL;

  /* Clear flags */
  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      cb->flags = L_CLR_BIT_FLAG (cb->flags, (RC_CB_INPROGRESS | 
					      RC_CB_FINISHED |
					      RC_CB_REACHES));
    }
  RC_cbs_reachable_mark_ignore(fn, chk_op_list);
  
  /* Run the search */
  com_empty = 1;
  com_chk_list = NULL;
  RC_cbs_reachable_from_rec2 (start_cb, start_op, start_op,
			      chk_op_list, exclude_chk_op_list, 
			      &pre_chk_list, &pre_jump_list,
			      vop_set, ctrl_list, 
			      &com_chk_list, &com_empty,
			      1);
 
  printf("COM CHECK :");
  List_start(com_chk_list);
  while ((op=List_next(com_chk_list)))
    {
      printf(" %d",op->id);
    }
  printf("\n");
 
  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      /* Is the cb a reachable one */
      if (L_EXTRACT_BIT_VAL (cb->flags, RC_CB_REACHES))
	{
	  *cb_list = 
	    List_insert_last (*cb_list, cb);
	}
    }


  /* Clear flags */
  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      cb->flags = L_CLR_BIT_FLAG (cb->flags, (RC_CB_INPROGRESS | 
					      RC_CB_FINISHED |
					      RC_CB_REACHES));
    }
  

#ifdef RC_DEBUG_INFO
  Set_print(stdout,"VOPS ",*vop_set);

  printf("    %4s:%4s : CTRL OPS = ","","");
  for (List_start (*ctrl_list); (op = (L_Oper *) List_next (*ctrl_list));)
    {
      printf ("%d ", op->id);
    }
  printf ("\n");

  printf("    %4s:%4s : CBS = ","","");
  for (List_start (*cb_list); (rc_cb = (L_Cb *) List_next (*cb_list));)
    {
      printf ("%d ", rc_cb->id);
    }
  printf ("\n");

  RC_PrintValidCbs(fn, *cb_list);
#endif

}

#endif


/*##########################################################
 * Determine what control ops are required to make RC
  ##########################################################*/
#if 0
void
RC_get_ctrl_for_rc (L_Func *fn, List * op_list, Set vops,
                    L_Cb * lds_cb, L_Oper * lds_op)
{
  L_Cb   *tg_cb  = NULL;
  L_Oper *cur_op = NULL;
  L_Oper *rc_op  = NULL;
  int    *buf    = NULL;
  int     num, i;

#ifdef RC_DEBUG_INFO
  printf ("Step ## 2.1 ##\n");
#endif

  
  num = Set_size(vops);
  if (!num)
    L_punt("RC_get_ctrl_for_rc: vops empty\n");
  buf = (int *) Lcode_malloc (sizeof (int) * num);
  Set_2array (vops, buf);
  
  for (i = 0; i < num; i++)
    {
      cur_op = L_oper_hash_tbl_find_oper (fn->oper_hash_tbl, buf[i]);
      if (!L_is_control_oper (cur_op))
	continue;
      tg_cb = L_branch_dest (cur_op);
      
      tg_cb->first_op
    }

  Lcode_free (buf);

#if 0
  List_start (cb_list);
  while ((rc_cb = (L_Cb *) List_next (cb_list)))
    {
      if (rc_cb == lds_cb)
        cur_op = lds_op;
      else
        cur_op = rc_cb->first_op;

      for (; cur_op; cur_op = cur_op->next_op)
        {
          if (!L_is_control_oper (cur_op))
            continue;

          /* Only consider branches to other RC cbs other 
           *  than the cb containing the lds
           */
          if (List_member (cb_list, L_branch_dest (cur_op)) &&
              L_branch_dest (cur_op) != lds_cb)
            {
              /* Branch target is part of recovery */
              *op_list = List_insert_last (*op_list, cur_op);
            }
        }
    }
#endif

#ifdef RC_DEBUG_INFO
  printf ("     %4s:%4s          %4s:%4s OPS =  ", "", "", "", "");
  List_start (*op_list);
  while ((rc_op = (L_Oper *) List_next (*op_list)))
    {
      if (L_is_control_oper (rc_op))
        printf ("B");
      printf ("%d ", rc_op->id);
    }
  printf ("\n");
#endif
}
#endif



#if 0
if (L_preincrement_load_opcode (cur_op))
  {
    L_punt ("preinc not handled\n");
  }
else if (L_postincrement_load_opcode (cur_op))
  {
    /* Put in subtract before load */
    new_op = L_create_new_op (Lop_SUB);
    new_op->dest[0] = L_copy_operand (cur_op->dest[1]);
    new_op->src[0] = L_copy_operand (cur_op->src[0]);
    new_op->src[1] = L_copy_operand (cur_op->src[1]);
    if (cur_op->pred[0])
      new_op->pred[0] = L_copy_operand (cur_op->pred[0]);
    L_insert_oper_before (new_cb, new_cb->last_op, new_op);
  }
#endif



#if 0
/*
 * Recovery code Scheduling
 */

/*##########################################################
 *  Schedule Recovery Code
 ###########################################################*/

#define FULLY_SCHEDULE_RC 0

void
RC_schedule_recovery_code (L_Func * fn, List lds_list)
{
  L_Oper *lds_op = NULL;
  int cb_flags = 0;

#ifdef RC_DEBUG_INFO
  printf ("------- Scheduling -------\n");
  RC_dump_lcode (fn, ".rcg_presched");
#endif

  /******************************************************
   * Setup scheduling of cbs
   ******************************************************/
  if (!SMH_kapi_knobs_ptr)
    {
      SMH_kapi_knobs_ptr =
        KAPI_Initialize2 (NULL, SMH_kapi_knobs_file, "Impact");
      KAPI_ia64_Initialize (SMH_kapi_knobs_ptr);
    }

#if FULLY_SCHEDULE_RC
  cb_flags = SM_DHASY;
  cb_flags |= SM_POSTPASS;

  L_partial_dead_code_removal (fn);
  L_demote_branches (fn);
  L_do_flow_analysis (fn, DOMINATOR_CB | LIVE_VARIABLE | SUPPRESS_PG);

#ifdef HAMM
  if (SM_use_hamm)
    HaMM_Initialize (hamm_MODE_REGION, SMH_kapi_knobs_ptr);
#endif
#endif


  /******************************************************
   * Schedule the RC cbs
   ******************************************************/
  for (List_start (lds_list); (lds_op = (L_Oper *) List_next (lds_list));)
    {
      RC_flow_info *info = RC_FLOW_INFO (lds_op);
      L_Cb *rc_cb = NULL;
      L_Oper *rc_op = NULL;
      L_Oper *next_op = NULL;
      int c = 0;

      for (c = 0; c < info->chk_num; c++)
        {
          List_start (info->rc_new_cbs[c]);
          while (rc_cb = (L_Cb *) List_next (info->rc_new_cbs[c]))
            {
#if FULLY_SCHEDULE_RC
              SM_Cb *sm_cb = NULL;

              sm_cb = SM_new_cb (lmdes, cb, cb_flags);
              SM_schedule_cb (sm_cb);
              SM_commit_cb (sm_cb);
              SM_delete_cb (sm_cb);
#else
              for (rc_op = rc_cb->first_op; rc_op; rc_op = next_op)
                {
                  next_op = rc_op->next_op;
                  SMH_bundle_single (SMH_kapi_knobs_ptr, rc_cb, rc_op);
                }
#endif
            }
        }
    }

#if FULLY_SCHEDULE_RC
#ifdef HAMM
  if (SM_use_hamm)
    HaMM_Finalize (hamm_MODE_REGION);
#endif
#endif
}
#endif



#if 0

/*
 * Recovery code register preservation
 */

/*##########################################################
 * Create Source, Sink, with special defs to allow dataflow
 ##########################################################*/
void
RC_setup_preservation_analysis (L_Func * fn, List lds_list)
{
  L_Oper *lds_op = NULL;

#ifdef RC_DEBUG_INFO
  printf ("---- Preservation setup ----\n");
#endif
  for (List_start (lds_list); (lds_op = (L_Oper *) List_next (lds_list));)
    {
      RC_flow_info *info = RC_FLOW_INFO (lds_op);
      int c = 0;

      /* For each check that matches this load 
       */
      for (c = 0; c < info->chk_num; c++)
        {
          /******************************************************************
           * RC should be self contained. Add in :
           * - rc-source cb : to lds' cb, contains defs to every reg in RC
           * - rc-sink cb   : to chks's cb, contains uses of every reg in RC
           ******************************************************************/
          L_Operand *oprd = NULL;
          L_Oper *new_op = NULL;
          L_Flow *flow = NULL;
          L_Attr *attr = NULL;
          L_Cb *rc_lds_cb = info->rc_lds_cb[c];
          L_Cb *rc_chk_cb = info->rc_chk_cb[c];
          L_Cb *rc_src_cb = NULL;
          L_Cb *rc_sink_cb = NULL;


          /* 
           * Create the RC-Source Cb
           ***************************/
          rc_src_cb = L_create_cb (0.0);
          L_insert_cb_before (fn, rc_lds_cb, rc_src_cb);

          List_start (info->oprd_list[c]);
          while (oprd = (L_Operand *) List_next (info->oprd_list[c]))
            {
              new_op = L_create_new_op (Lop_DEFINE);
              new_op->dest[0] = L_copy_operand (oprd);
              L_insert_oper_after (rc_src_cb, rc_src_cb->last_op, new_op);
              attr = L_new_attr ("rcdef", 0);
              new_op->attr = L_concat_attr (new_op->attr, attr);
            }
          /*
             new_op = L_create_new_op(Lop_JUMP);
             new_op->src[0] = L_new_cb_operand(rc_lds_cb);          
             L_insert_oper_after(rc_src_cb, rc_src_cb->last_op, new_op);
           */
          /* Dest cb */
          flow = L_new_flow (0, rc_src_cb, rc_lds_cb, 0.0);
          rc_lds_cb->src_flow = L_concat_flow (rc_lds_cb->src_flow, flow);

          /* Src cb */
          flow = L_new_flow (0, rc_src_cb, rc_lds_cb, 0.0);
          rc_src_cb->dest_flow = L_concat_flow (rc_src_cb->dest_flow, flow);
          info->rc_src_cb[c] = rc_src_cb;


          /* 
           * Create the RC-Sink Cb
           ***************************/
          rc_sink_cb = L_create_cb (0.0);
          L_insert_cb_after (fn, rc_chk_cb, rc_sink_cb);

          List_start (info->oprd_list[c]);
          while (oprd = (L_Operand *) List_next (info->oprd_list[c]))
            {
              new_op = L_create_new_op (Lop_DEFINE);
              new_op->src[0] = L_copy_operand (oprd);
              L_insert_oper_after (rc_sink_cb, rc_sink_cb->last_op, new_op);
              attr = L_new_attr ("rcdef", 0);
              new_op->attr = L_concat_attr (new_op->attr, attr);
            }
          /*
             new_op = L_create_new_op(Lop_JUMP);
             new_op->src[0] = L_new_cb_operand(rc_sink_cb);
             L_insert_oper_after(rc_chk_cb, rc_chk_cb->last_op, new_op);
           */
          new_op = L_create_new_op (Lop_RTS);
          L_insert_oper_after (rc_sink_cb, rc_sink_cb->last_op, new_op);

          /* Dest cb */
          flow = L_new_flow (0, rc_chk_cb, rc_sink_cb, 0.0);
          rc_sink_cb->src_flow = L_concat_flow (rc_sink_cb->src_flow, flow);

          /* Src cb */
          flow = L_new_flow (0, rc_chk_cb, rc_sink_cb, 0.0);
          rc_lds_cb->dest_flow = L_concat_flow (rc_lds_cb->dest_flow, flow);
          info->rc_sink_cb[c] = rc_sink_cb;

        }                       /*chk */
    }                           /*lds */
}


/*##########################################################
 *  Determine what regs must be preserved until check
 ##########################################################*/

void
RC_find_preserved_regs (L_Func * fn, List lds_list)
{
  L_Oper *lds_op = NULL;

#ifdef RC_DEBUG_INFO
  printf ("---- Reg Presevation ----\n");
#endif
  for (List_start (lds_list); (lds_op = (L_Oper *) List_next (lds_list));)
    {
      RC_flow_info *info = RC_FLOW_INFO (lds_op);
      int c;
      int i;

      /* For each check that matches this load 
       */
      for (c = 0; c < info->chk_num; c++)
        {
          L_Cb *rc_src_cb = info->rc_src_cb[c];
          L_Cb *rc_sink_cb = info->rc_sink_cb[c];
          L_Oper *src_op = NULL;
          L_Oper *dep_op = NULL;
          RC_flow_info *src_info = NULL;
          int found = 0;

          /* any def in rc-src that does NOT reach rc-sink is SAFE 
           * Exception: If spec load is a post/pre incr it's source
           *            should be saved even though it won't reach
           */
          for (src_op = rc_src_cb->first_op; src_op; src_op = src_op->next_op)
            {
              /* Only inserted defines matter */
              if (src_op->opc != Lop_DEFINE)
                continue;
              src_info = RC_FLOW_INFO (src_op);

              /* If no use is in sink, SAFE */
              found = 0;
              for (List_start (src_info->use_op_list);
                   (dep_op = (L_Oper *) List_next (src_info->use_op_list));)
                {
                  if (dep_op->opc == Lop_DEFINE &&
                      (L_oper_hash_tbl_find_cb (fn->oper_hash_tbl, dep_op->id)
                       == rc_sink_cb))
                    {
                      found = 1;
                      break;
                    }
                }

              if (found)
                info->psrv_list[c] = List_insert_last (info->psrv_list[c],
                                                       src_op->dest[0]);
            }
#if 0
          for (i = 0; i < L_max_src_operand; i++)
            {
              if (lds_op->src[i] &&
                  (L_is_reg (lds_op->src[i]) || L_is_macro (lds_op->src[i]))
                  && (!RC_oprd_in_list (info->psrv_list[c], lds_op->src[i])))
                {
                  info->psrv_list[c] = List_insert_last (info->psrv_list[c],
                                                         lds_op->src[i]);
                }
            }
#endif
        }                       /*chk */
    }                           /*lds */
}


/*##########################################################
 *  Add in uses after checks to preserve regs
 ##########################################################*/

void
RC_add_preserved_regs_to_check (L_Func * fn, List lds_list)
{
  L_Oper *lds_op = NULL;

  for (List_start (lds_list); (lds_op = (L_Oper *) List_next (lds_list));)
    {
      RC_flow_info *info = RC_FLOW_INFO (lds_op);
      int c;

      /* For each check that matches this load 
       */
      for (c = 0; c < info->chk_num; c++)
        {
          L_Oper *chk_op = info->chk_ops[c];
          L_Operand *oprd = NULL;
          L_Attr *attr = NULL;
          L_Attr *tr_attr = NULL;
          int tr_count = 0;

          /*
           * Add in uses to preserve regs (Using Lop_DEFINE)
           */
#ifdef RC_DEBUG_INFO
          printf ("Load %d -> Check %d = ", lds_op->id, chk_op->id);
#endif
          attr = L_find_attr (chk_op->attr, "src");
          if (!attr)
            {
              attr = L_new_attr ("src", 1);
              chk_op->attr = L_concat_attr (chk_op->attr, attr);
            }
#if 0
          tr_attr = L_find_attr (chk_op->attr, "tr");
          if (!tr_attr)
            {
              tr_attr = L_new_attr ("tr", 1);
              chk_op->attr = L_concat_attr (chk_op->attr, tr_attr);
            }
#endif
          tr_count = 0;
          for (List_start (info->psrv_list[c]);
               (oprd = (L_Operand *) List_next (info->psrv_list[c]));)
            {
#ifdef RC_DEBUG_INFO
              L_print_operand (stdout, oprd, 0);
#endif
              L_set_attr_field (attr, tr_count, L_copy_operand (oprd));
#if 0
              L_set_attr_field (tr_attr, tr_count, L_copy_operand (oprd));
#endif
              tr_count++;
            }
#ifdef RC_DEBUG_INFO
          printf ("\n");
#endif
        }                       /*chk */
    }                           /*lds */
}


/*##########################################################
 *  Verify that required registers were preserved on check
 ##########################################################*/

void
RC_verify_preserved_regs_on_check (L_Func * fn, List lds_list)
{
  L_Oper *lds_op = NULL;

  for (List_start (lds_list); (lds_op = (L_Oper *) List_next (lds_list));)
    {
      RC_flow_info *info = RC_FLOW_INFO (lds_op);
      int c;

      /* For each check that matches this load 
       */
      for (c = 0; c < info->chk_num; c++)
        {
          L_Oper *chk_op = info->chk_ops[c];
          L_Operand *oprd = NULL;

          /*
           *  Verify what needed to be preserved was preserved
           */
#ifdef RC_DEBUG_INFO
          printf ("Load %d -> Check %d = ", lds_op->id, chk_op->id);
#endif
          for (List_start (info->psrv_list[c]);
               (oprd = (L_Operand *) List_next (info->psrv_list[c]));)
            {
#ifdef RC_DEBUG_INFO
              L_print_operand (stdout, oprd, 0);
#endif
              if (!RC_check_preserves_oprd (chk_op, oprd))
                {
                  printf ("\nLoad %d -> Check %d = ", lds_op->id, chk_op->id);
                  L_print_operand (stdout, oprd, 0);
                  printf (" should have been preserved.\n");
                  L_punt ("RC_find_and_preserve_regs: error");
                }
#ifdef RC_DEBUG_INFO
              else
                {
                  printf ("* ");
                }
#endif
            }
#ifdef RC_DEBUG_INFO
          printf ("\n");
#endif
        }                       /*chk */
    }                           /*lds */
}


/*##########################################################
 *  Remove rc-srcs, rc-sinks, and all recovery code
 ##########################################################*/

void
RC_cleanup_preservation (L_Func * fn, List lds_list, int check_only)
{
  L_Oper *lds_op = NULL;

  for (List_start (lds_list); (lds_op = (L_Oper *) List_next (lds_list));)
    {
      RC_flow_info *info = RC_FLOW_INFO (lds_op);
      int c;

      /* For each check that matches this load 
       */
      for (c = 0; c < info->chk_num; c++)
        {
          L_Cb *rc_src_cb = info->rc_src_cb[c];
          L_Cb *rc_sink_cb = info->rc_sink_cb[c];
          L_Cb *rc_chk_cb = info->rc_chk_cb[c];
          L_Oper *op = NULL;
          L_Oper *next_op = NULL;
          L_Cb *rc_cb = NULL;
          L_Flow *flow = NULL;

          if (!check_only)
            {
              /*  Delete all recovery code cb/ops
               */
              List_start (info->rc_new_cbs[c]);
              while (rc_cb = (L_Cb *) List_next (info->rc_new_cbs[c]))
                {
                  L_delete_cb (fn, rc_cb);
                }
              info->rc_new_cbs[c] = List_reset (info->rc_new_cbs[c]);
            }
          else
            {
              /*  Delete all recovery code defines 
               */
              List_start (info->rc_new_cbs[c]);
              while (rc_cb = (L_Cb *) List_next (info->rc_new_cbs[c]))
                {
                  for (op = rc_cb->first_op; op; op = next_op)
                    {
                      next_op = op->next_op;
                      if (L_find_attr (op->attr, "rcdef"))
                        L_delete_oper (rc_cb, op);
                    }
                }

              /* Delete only flow to sink 
               */
              flow = L_find_last_flow (info->rc_chk_cb[c]->dest_flow);
              info->rc_chk_cb[c]->dest_flow =
                L_delete_flow (info->rc_chk_cb[c]->dest_flow, flow);
            }

          /* Delete rc-source cb/ops and rc-sink cb/ops
           */
          L_delete_cb (fn, rc_src_cb);
          info->rc_src_cb[c] = NULL;
          L_delete_cb (fn, rc_sink_cb);
          info->rc_sink_cb[c] = NULL;

        }                       /*chk */
    }                           /*lds */
}

#endif
#endif
