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
/*===========================================================================
 *
 *      File :          l_pipe_rename.c
 *      Description :   Rename disjoint virtual register lifetimes
 *			have the same virtual register id within loop
 *      Creation Date : January 26, 1997
 *      Author :        Daniel Lavery, based on code by Richard Hank
 *
 *      Revision Date: Summer 1999
 *      Authors: IMPACT Technologies: Matthew Merten and John Gyllenhaal
 *
 *===========================================================================*/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <alloca.h>
#include "l_softpipe_int.h"
#include "l_pipe_rename.h"
#include "l_pipe_util.h"
#include <Lcode/r_regalloc.h>
#include <Lcode/l_opti.h>

#undef DEBUG_PIPE_RENAME

/*************************************************************************
                Structures and Static Variables
*************************************************************************/

typedef struct _LpipeLiveRange {
  short id;
  short vreg;
  short valid;
  short ctype;
  L_Oper *def_oper;
  Set op;
  Set def_op;
  Set ref_op;
  Set live_out_op;
} LpipeLiveRange;

static LpipeLiveRange **LpipeLiveRanges;
static int numLpipeLiveRange;

/*************************************************************************
                Function Definitions
*************************************************************************/

static int L_fix_exposed_pred_wrt (L_Func * fn, L_Cb * pipe_cb, int vreg);

/*************************************************************************
                Utility Function Definitions
*************************************************************************/

void
R_print_LpipeLiveRange (LpipeLiveRange * lr)
{
  fprintf (stdout, "***\nvreg		%d\n", lr->vreg);
  fprintf (stdout, "def_oper		%d\n", lr->def_oper->id);
  Set_print (stdout, "def_op", lr->def_op);
  Set_print (stdout, "ref_op", lr->ref_op);
  Set_print (stdout, "live_out_op", lr->live_out_op);
  Set_print (stdout, "op", lr->op);
}

/*************************************************************************
                Function Definitions
*************************************************************************/

static void
Lpipe_create_rename_comp_code_block (L_Func * fn, L_Cb * pipe_cb,
				     L_Oper * exit_branch)
{
  L_Flow *exit_flow;
  L_Cb *exit_cb, *comp_cb;
  L_Attr *attr;
  L_Oper *new_oper;

  exit_flow = L_find_flow_for_branch (pipe_cb, exit_branch);
  exit_cb = exit_flow->dst_cb;

  if (L_single_predecessor_cb (exit_cb))
    return;

  comp_cb = L_split_arc (fn, pipe_cb, exit_flow);

  attr = L_new_attr ("rename_comp_code", 0);
  comp_cb->attr = L_concat_attr (comp_cb->attr, attr);

  if ((new_oper = comp_cb->first_op))
    {
      L_annotate_oper (fn, comp_cb, new_oper);
      L_delete_oper (comp_cb, new_oper);
    }

  return;
}

/*
 * Lpipe_create_comp_code_blocks
 * ----------------------------------------------------------------------
 * Ensure that each of the loop's outgoing arcs terminates at a
 * single-predecessor block suitable for receiving compensation code.
 */
void
Lpipe_create_comp_code_blocks (L_Func * fn, L_Inner_Loop * loop)
{
  L_Cb *pipe_cb;
  L_Oper *oper, *loop_back_br;
  L_Flow *last_flow;

  pipe_cb = loop->cb;
  loop_back_br = loop->feedback_op;

  for (oper = pipe_cb->first_op; oper; oper = oper->next_op)
    if ((L_cond_branch (oper)) && (oper != loop_back_br))
      Lpipe_create_rename_comp_code_block (fn, pipe_cb, oper);

  /* Create one for the fallthrough path also --- since the RIN set
   * of this successor block is used to find the ROUT/ft set of the
   * loopback branch, the successor had better not have any other
   * predecessors in the loop!
   */

  if ((last_flow = L_find_last_flow (pipe_cb->dest_flow)) &&
      (last_flow->dst_cb == pipe_cb->next_cb) && 
      !L_single_predecessor_cb (pipe_cb->next_cb))
    L_split_arc (fn, pipe_cb, last_flow);

  return;
}

static void
Lpipe_rename_mac (L_Func *fn, L_Operand *mopd, L_Inner_Loop *inl)
{
  L_Cb *cb = inl->cb;
  L_Oper *op;
  L_Operand *opd, *ropd;
  L_Flow *fl;
  int mac = mopd->value.mac,  rval = ++(fn->max_reg_id), i;

  mopd = L_copy_operand (mopd);
  ropd = L_copy_operand (mopd);
  ropd->type = L_OPERAND_REGISTER;
  ropd->value.r = rval;

  for (op = cb->first_op; op; op = op->next_op)
    {
      for (i = 0; i < L_max_dest_operand; i++)
	{
	  if (!(opd = op->dest[i]) || !L_is_macro (opd) || 
	      (opd->value.mac != mac))
	    continue;

	  opd->type = L_OPERAND_REGISTER;
	  opd->value.r = rval;
	}
      for (i = 0; i < L_max_src_operand; i++)
	{
	  if (!(opd = op->src[i]) || !L_is_macro (opd) || 
	      (opd->value.mac != mac))
	    continue;

	  opd->type = L_OPERAND_REGISTER;
	  opd->value.r = rval;
	}
    }

  for (fl = cb->dest_flow; fl; fl = fl->next_flow)
    {
      if (fl->dst_cb == cb)
	continue;

      op = Lpipe_gen_mov_consuming_operands (L_copy_operand (mopd),
					     L_copy_operand (ropd));

      L_insert_oper_before (fl->dst_cb, fl->dst_cb->first_op, op);
      L_annotate_oper (fn, fl->dst_cb, op);
      L_delete_oper (fl->dst_cb, op);
    }

  op = Lpipe_gen_mov_consuming_operands (ropd, mopd);
  L_insert_oper_after (inl->preheader, inl->preheader->last_op, op);
  L_annotate_oper (fn, inl->preheader, op);
  L_delete_oper (inl->preheader, op);

  return;
}

/* 
 * Rename all macros explicitly defined in the block (to enable
 * subsequent mve/rreg gen, as necessary).  Assumes comp code CBs
 * have already been generated.
 */
extern int
Lpipe_rename_defined_macros (L_Func *fn, L_Inner_Loop *inl)
{
  int cnt, i;
  L_Cb *cb = inl->cb;
  L_Oper *op;
  L_Operand *opd;
  Set macs = NULL;

  for (op = cb->first_op; op; op = op->next_op)
    {
      for (i = 0; i < L_max_dest_operand; i++)
	{
	  if ((opd = op->dest[i]) && L_is_macro (opd) &&
	      !L_is_ctype_predicate (opd) &&
	      !Set_in (macs, opd->value.mac))
	    {
	      macs = Set_add (macs, opd->value.mac);
	      Lpipe_rename_mac (fn, opd, inl);
	    }
	}
    }

  if ((cnt = Set_size (macs)))
    {
      L_warn ("Softpipe renamed %d macros in %s() cb %d",
	      cnt, fn->name, cb->id);
      macs = Set_dispose (macs);      
    }
  return cnt;
}

/*
 * PROBLEM:
 *
 * A        r3 = r3
 * 1   <pn> r3 = r3 + 1
 * 2   ... big dep height
 * 3   <pm> jump ----------> r3 live out
 * 4   <pn> r3 = r3 + 1
 * 5   ... big dep height
 * 6   <pm> jump ----------> r3 live out
 *   
 * Fixing the exposed writes will put a move of r3 into r3 to 
 * ensure a complete write (line A).
 * However, the write to r3 in line 4 is pinned belwo the
 * jump in line 3 due to an anti dependence.
 */


static int
L_fix_exposed_pred_wrt (L_Func * fn, L_Cb * pipe_cb, int vreg)
{
  L_Oper *oper, *new_oper;
  int i;
  L_Operand *dest, *new_dest, *new_src;

  for (oper = pipe_cb->first_op; oper; oper = oper->next_op)
    {
      for (i = 0; i < L_max_dest_operand; i++)
	{
	  if (!(dest = oper->dest[i]) ||
	      !L_is_reg (dest) || 
	      (L_REG_INDEX (dest->value.r) != vreg))
	    continue;

	  if (L_is_predicated (oper))
	    {
	      new_dest = L_new_register_operand (dest->value.r, dest->ctype,
						 L_PTYPE_NULL);
	      new_src = L_copy_operand (new_dest);
	      new_oper = Lpipe_gen_mov_consuming_operands (new_dest, new_src);
	      L_insert_oper_before (pipe_cb, oper, new_oper);
	      L_annotate_oper (fn, pipe_cb, new_oper);
	      L_delete_oper (pipe_cb, new_oper);

	      if (Lpipe_debug >= 1)
		fprintf (stdout,
			 "Fixed infinite live range on r %d before op %d\n",
			 L_UNMAP_REG (vreg), oper->id);

	      return 1;
	    }
	  else
	    {
	      return 0;
	    }
	}
    }
  return 0;
}


int
Lpipe_fix_infinite_lifetimes (L_Func * fn, L_Cb * cb)
{
  Set in_set;
  int *varray, num_live_in, i, change = 0;

  in_set = L_get_cb_IN_set (cb);

  num_live_in = Set_size (in_set);
  varray = (int *) alloca (num_live_in * sizeof (int));
  Set_2array (in_set, varray);
  for (i = 0; i < num_live_in; i++)
    change += L_fix_exposed_pred_wrt (fn, cb, varray[i]);

  return change;
}


void
Lpipe_rename_disj_vregs_in_loop (L_Func * fn, L_Inner_Loop * loop)
{
  int i, j, k, arch, dest, change, cnt = 0;
  int *oparray;
  LpipeLiveRange **vrarray;
  L_Attr *ill_reg;
  L_Cb *pipe_cb, *comp_cb;
  L_Oper *loop_back_br, *ptr, *oper, *new_oper;

  pipe_cb = loop->cb;
  loop_back_br = loop->feedback_op;

  /* Requires up-to-date live variable and reaching definition */

  LpipeLiveRanges = (LpipeLiveRange **) calloc (((fn->max_oper_id + 1) *
						 L_max_dest_operand),
						sizeof (LpipeLiveRange *));

  /*
   * Create a live range for each virtual register definition in loop
   */
  arch = M_arch;
  for (oper = pipe_cb->first_op; oper; oper = oper->next_op)
    {
      for (i = 0; i < L_max_dest_operand; i++)
	{
	  L_Operand *dest = oper->dest[i];
	  LpipeLiveRange *lr;

	  if (!dest || !L_is_reg (dest) || L_is_ctype_predicate (dest))
	    continue;

	  lr = (LpipeLiveRange *) malloc (sizeof (LpipeLiveRange));
	  lr->valid = 1;
	  lr->id = oper->id;
	  lr->vreg = dest->value.r;
	  lr->ctype = dest->ctype;
	  lr->def_oper = oper;

	  /* This prevents spitting live ranges of the form:      */
	  /*              r1 <-                                   */
	  /*              r1 <- r1 + r2                           */
	  /*                 <- r1                                */
	  /* Since x86 has a 2 operand format.                    */
	  /* REH 3/10/95                                          */
	  /* Also prevent splitting of pre/post ince instr.       */
	  
	  /* TI also has 2 operand format.      SYH 9/24/96       */

	  if ((arch == M_X86) || (arch == M_TI) ||
	      L_preincrement_load_opcode (oper) ||
	      L_postincrement_load_opcode (oper) ||
	      L_preincrement_store_opcode (oper) ||
	      L_postincrement_store_opcode (oper) ||
	      L_bit_deposit_opcode (oper) ||
	      L_find_attr (oper->attr, "do_not_split"))
	    lr->op = Set_add (NULL, oper->id);
	  else
	    lr->op = NULL;

	  lr->def_op = Set_add (NULL, oper->id);
	  lr->ref_op = NULL;
	  lr->live_out_op = NULL;
	  LpipeLiveRanges[cnt++] = lr;
	}
    }
  numLpipeLiveRange = cnt;

  /* For each reaching definition IN set for each oper in loop, remove
     reaching definitions that lexically follow the oper in the loop,
     but do not reach the loop back branch.  These reaching defs must
     be live out of at least one of the early exits, and can be renamed
     in the compensation code blocks.  For each oper, find lexically later
     definitions by searching forward in cb.  Remove definition if it is in
     the IN set of oper and not in the IN set of loop back branch. */

  for (oper = pipe_cb->first_op; oper; oper = oper->next_op)
    {
      for (ptr = oper; ptr; ptr = ptr->next_op)
	{
	  for (i = 0; i < L_max_dest_operand; i++)
	    {
	      L_Operand *dest = ptr->dest[i];

	      if (!dest || !L_is_reg (dest) || L_is_ctype_predicate (dest))
		continue;

	      /* exclude defs we don't want to rename */
	      if (L_preincrement_load_opcode (ptr) ||
		  L_postincrement_load_opcode (ptr) ||
		  L_preincrement_store_opcode (ptr) ||
		  L_postincrement_store_opcode (ptr) ||
		  L_bit_deposit_opcode (ptr) ||
		  (L_find_attr (ptr->attr, "do_not_split") != NULL))
		continue;

	      if (!L_in_oper_RIN_set (loop_back_br, ptr, dest) &&
		  L_in_oper_RIN_set (oper, ptr, dest))
		L_remove_from_oper_RIN_set (oper, ptr, dest);
	    }
	}
    }

  /*
   * Determine the set of instructions over which each definition
   * of a virtual register is live.
   */
  for (oper = pipe_cb->first_op; oper; oper = oper->next_op)
    {
      int id = oper->id, is_pred_loopback;
      Set v_in = L_get_oper_IN_set (oper);
      Set v_out = NULL;

      is_pred_loopback = (oper == loop_back_br) && !oper->next_op &&
	L_is_predicated (oper) && L_has_fallthru_to_next_cb (pipe_cb);

      if (is_pred_loopback)
	v_out = L_get_oper_OUT_set (pipe_cb, oper, FALL_THRU_PATH);

      for (i = 0; i < numLpipeLiveRange; i++)
	{
	  LpipeLiveRange *lr = LpipeLiveRanges[i];
	  int vreg, vrid;

	  vreg = lr->vreg;
	  vrid = L_REG_INDEX (vreg);

	  if ((Set_in (v_in, vrid) &&
	       L_in_oper_RIN_set_reg (oper, lr->def_oper, vrid)) ||
	      (is_pred_loopback && Set_in (v_out, vrid) &&
#if 0
	       L_in_oper_ROUT_set_reg (oper, lr->def_oper, vrid, 
				       FALL_THRU_PATH)
#else
	      L_in_cb_RIN_set_reg (pipe_cb->next_cb, lr->def_oper, vrid)
#endif
	       ))
	    {
	      lr->op = Set_add (lr->op, oper->id);
	      for (j = 0; j < L_max_src_operand; j++)
		{
		  L_Operand *src = oper->src[j];
		  if (src && L_is_reg (src) && (src->value.r == vreg))
		    lr->ref_op = Set_add (lr->ref_op, id);
		}

	      /* If virtual register is live out of branch, add branch to
	         live_out_op set for live range.  Compensation code is
	         needed if this live range is renamed. */
	      if (L_cond_branch_opcode (oper) ||
		  L_uncond_branch_opcode (oper))
		{
		  int no_fallthru =
		    L_EXTRACT_BIT_VAL (pipe_cb->flags,
				       L_CB_HYPERBLOCK_NO_FALLTHRU);
		  if (oper != loop_back_br)
		    {
		      v_out = L_get_oper_OUT_set (pipe_cb, oper, 
						  TAKEN_PATH);
		      if (Set_in (v_out, vrid))
			lr->live_out_op = Set_add (lr->live_out_op, id);
		    }
		  else if (!L_uncond_branch (oper) && !no_fallthru)
		    {
		      v_out = L_get_oper_OUT_set (pipe_cb, oper,
						  FALL_THRU_PATH);
		      if (Set_in (v_out, vrid))
			lr->live_out_op = Set_add (lr->live_out_op, id);
		    }
		}
	    }
	}
    }
#ifdef DEBUG_PIPE_RENAME
  fprintf (stdout, "*\n*\n Live Ranges Before Split\n *\n");
  for (i = 0; i < numLpipeLiveRange; i++)
    R_print_LpipeLiveRange (LpipeLiveRanges[i]);
#endif

  /* 
   * Merge non-disjoint live ranges 
   */

  do
    {
      change = 0;
      for (i = 0; i < numLpipeLiveRange; i++)
	{
	  Set isect;
	  LpipeLiveRange *lr1 = LpipeLiveRanges[i];

	  if (!lr1 || !lr1->valid)
	    continue;

	  for (j = i + 1; j < numLpipeLiveRange; j++)
	    {
	      LpipeLiveRange *lr2 = LpipeLiveRanges[j];

	      if (!lr2 || !lr2->valid || (lr1->vreg != lr2->vreg))
		continue;

	      if (Set_intersect_empty (lr1->op, lr2->op))
		continue;

#if 1
	      /* This appears to prevent the merging of infinite-lifetime
	       * moves.  If the first half gets renamed, the predicated
	       * write is re-exposed.  
	       * This seems questionable at best.
	       * JWS
	       */
	      if ((isect = Set_intersect (lr1->op, lr2->op)) &&
		  Set_size (isect) == 1)
		{
		  int isect_op;
		  L_Oper *isect_oper;

		  Set_2array (isect, &isect_op);
		  isect_oper = L_oper_hash_tbl_find_oper (fn->oper_hash_tbl,
							  isect_op);
		  if (L_general_move_opcode (isect_oper) &&
		      L_same_operand (isect_oper->dest[0],
				      isect_oper->src[0]))
		    continue;
		}

	      isect = Set_dispose (isect);
#endif

#ifdef DEBUG_PIPE_RENAME
	      fprintf (stdout, "Merging Live Range %d %d\n",
		       lr1->vreg, lr2->vreg);
	      Set_print (stdout, "lr1", lr1->op);
	      Set_print (stdout, "lr2", lr2->op);
#endif
	      lr1->op = Set_union_acc (lr1->op, lr2->op);
	      lr1->def_op = Set_union_acc (lr1->def_op, lr2->def_op);
	      lr1->ref_op = Set_union_acc (lr1->ref_op, lr2->ref_op);
	      lr1->live_out_op = Set_union_acc (lr1->live_out_op,
						lr2->live_out_op);

	      lr2->valid = 0;
	      Set_dispose (lr2->op);
	      Set_dispose (lr2->def_op);
	      Set_dispose (lr2->ref_op);
	      Set_dispose (lr2->live_out_op);

	      change += 1;
	    }
	}
      dest = -1;
      for (i = 0; i < numLpipeLiveRange; i++)
	{
	  LpipeLiveRange *lr = LpipeLiveRanges[i];

	  if (!lr)
	    {
	      continue;
	    }
	  else if (!lr->valid)
	    {
	      if (dest == -1)
		dest = i;
	      Lcode_free (lr);
	      LpipeLiveRanges[i] = NULL;
	    }
	  else if (dest != -1)
	    {
	      LpipeLiveRanges[dest++] = LpipeLiveRanges[i];
	      LpipeLiveRanges[i] = NULL;
	    }
	}
    }
  while (change);

#ifdef DEBUG_PIPE_RENAME
  fprintf (stdout, "*\n*\n Live Ranges After Merging\n *\n");
  for (i = 0; i < numLpipeLiveRange; i++)
    if (LpipeLiveRanges[i])
      R_print_LpipeLiveRange (LpipeLiveRanges[i]);
#endif

  vrarray =
    (LpipeLiveRange **) calloc (fn->max_reg_id + 1,
				sizeof (LpipeLiveRange *));
  oparray = (int *) malloc ((fn->max_oper_id + 1) * sizeof (int));

#ifdef DEBUG_PIPE_RENAME
  fprintf (stdout, "Starting disj vreg renaming.\n");
#endif

  /* 
   * Rename disjoint virtual registers with the same 
   * virtual register id.
   */
  for (i = 0; i < numLpipeLiveRange; i++)
    {
      int old_vreg, vreg, num_op;
      LpipeLiveRange *temp_lr;
      LpipeLiveRange *lr = LpipeLiveRanges[i];

      if (!lr)
	break;

      /* vrarray[lr->vreg] equal to 0 indicates that a live range with name
         vreg has not been seen before.  If vrarray[lr->vreg] is not 0,
         then it is a pointer to previously processed live range with the 
         same name. */
      if (!vrarray[lr->vreg])
	{
	  vrarray[lr->vreg] = lr;
	  continue;
	}

      /* If current live range contains the loop back branch, rename the
         earlier live range.  If current live range does not contain the 
         loop back branch, rename the current live range. */
      if (Set_in (lr->op, loop_back_br->id))
	{
	  temp_lr = vrarray[lr->vreg];
	  vrarray[lr->vreg] = lr;
	  lr = temp_lr;
	}

      old_vreg = lr->vreg;
      vreg = ++(fn->max_reg_id);

      if (Lpipe_debug >= 2)
	fprintf (stdout, "Renaming %d (def op %d) -> %d\n",
		 old_vreg, lr->def_oper->id, vreg);

      /*
       * Loop through defining operations 
       */
      num_op = Set_2array (lr->def_op, oparray);
      for (j = 0; j < num_op; j++)
	{
	  L_Oper *oper = L_oper_hash_tbl_find_oper (fn->oper_hash_tbl,
						    oparray[j]);

	  /* rename all registers in ill_reg attributes SYH 9/24/96 */
	  ill_reg = oper->attr;
	  while ((ill_reg = L_find_attr (ill_reg, "ill_reg")))
	    {
	      if (ill_reg && ill_reg->field[0]->value.i == old_vreg)
		ill_reg->field[0]->value.i = vreg;
	      ill_reg = ill_reg->next_attr;
	    }

	  for (k = 0; k < L_max_dest_operand; k++)
	    {
	      L_Operand *dest;
	      if (!(dest = oper->dest[k]))
		continue;
	      if (L_is_reg (dest) && dest->value.r == old_vreg)
		{
		  dest->value.r = vreg;
#ifdef DEBUG_PIPE_RENAME
		  fprintf (stdout, "op %d: dst reg %d -> %d\n",
			   oper->id, old_vreg, vreg);
#endif
		}
	    }
	}

      /* 
       * Loop through referencing operations
       */
      num_op = Set_2array (lr->ref_op, oparray);
      for (j = 0; j < num_op; j++)
	{
	  L_Oper *oper = L_oper_hash_tbl_find_oper (fn->oper_hash_tbl,
						    oparray[j]);

	  /* rename all registers in ill_reg attributes SYH 9/24/96 */
	  ill_reg = oper->attr;
	  while ((ill_reg = L_find_attr (ill_reg, "ill_reg")))
	    {
	      if (ill_reg && ill_reg->field[0]->value.i == old_vreg)
		ill_reg->field[0]->value.i = vreg;
	      ill_reg = ill_reg->next_attr;
	    }

	  for (k = 0; k < L_max_src_operand; k++)
	    {
	      L_Operand *src;
	      if ((src = oper->src[k]) &&
		  (L_is_reg (src)) && (src->value.r == old_vreg))
		{
		  src->value.r = vreg;
#ifdef DEBUG_PIPE_RENAME
		  fprintf (stdout, "op %d: src reg %d -> %d\n",
			   oper->id, old_vreg, vreg);
#endif
		}
	    }
	}

      /*  Loop through branches for which live range is live out and add
         compensation code. */
      num_op = Set_2array (lr->live_out_op, oparray);
      for (j = 0; j < num_op; j++)
	{
	  L_Operand *src, *dest;
	  L_Oper *oper = L_oper_hash_tbl_find_oper (fn->oper_hash_tbl,
						    oparray[j]);
	  if (oper != loop_back_br)
	    {
	      comp_cb = L_find_branch_dest (oper);

	      src = L_new_register_operand (vreg, lr->ctype, L_PTYPE_NULL);
	      dest =
		L_new_register_operand (old_vreg, lr->ctype, L_PTYPE_NULL);
	      new_oper = Lpipe_gen_mov_consuming_operands (dest, src);
	      /* make sure move goes before jump at end of block */
	      L_insert_oper_before (comp_cb, comp_cb->first_op, new_oper);
	      L_annotate_oper (fn, comp_cb, new_oper);
	      L_delete_oper (comp_cb, new_oper);
	    }
	}
    }

#ifdef DEBUG_PIPE_RENAME
  fprintf (stdout, "Done with disj vreg renaming.\n");
#endif

  Lcode_free (vrarray);
  Lcode_free (oparray);

  for (i = 0; i < numLpipeLiveRange; i++)
    {
      LpipeLiveRange *lr = LpipeLiveRanges[i];

      if (!lr)
	break;

      Set_dispose (lr->op);
      Set_dispose (lr->def_op);
      Set_dispose (lr->ref_op);
      Set_dispose (lr->live_out_op);
      Lcode_free (lr);
      LpipeLiveRanges[i] = NULL;
    }

  Lcode_free (LpipeLiveRanges);

  return;
}
