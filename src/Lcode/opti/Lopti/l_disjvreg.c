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
 *      File :          l_disjvreg.c
 *      Description :   Rename disjoint virtual register lifetimes
 *                      have the same virtual register id
 *      Creation Date : April 5, 1994
 *      Author :        Richard Hank, Wen-mei Hwu
 *
 * Revision 1.5  95/01/10  17:20:49  17:20:49  hank (Richard E. Hank)
 * This code has been updated with the data structure
 * changes within the register allocator and is now
 * required prior to region-based allocation.
 * The code was orignially written for experiments
 * with register allocation within the AMD
 * code generator.
 *
 * Revision 1.1  95/01/10  17:16:23  17:16:23  hank (Richard E. Hank)
 * Initial revision
 *
 * 20020406 JWS Moved to Lopti for inclusion in Lsuperscalar
 * 20020908 JWS Totally re-engineered to fix live range bug in predicated code
 *
 *===========================================================================*/
/*===========================================================================*/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "l_opti.h"
#include "l_disjvreg.h"
#include <machine/m_impact.h>

#undef DEBUG_LIVE_RANGE
#undef DEBUG_RENAME
#undef DEBUG_COALESCE

/* LiveRange data structure */

typedef struct _LiveRange {
  short id;
  short vreg;
  short valid;
  short ctype;
  L_Oper *def_oper;
  Set op;
  Set def_op;
  Set ref_op;
  Set intf;
  Set composition;
  int flags;
} LiveRange;

#define RDVR_LR_NOSPLIT        0x00000001

#define RDVR_DO_RENAME         1
#define RDVR_NO_RENAME         0

L_Alloc_Pool *LiveRange_pool = NULL;

static LiveRange * L_rdvr_new_live_range (L_Oper *def_op, L_Operand *def_dest);
static LiveRange * L_rdvr_delete_live_range (LiveRange *lr);
static void L_rdvr_rename_live_range (L_Func *fn, LiveRange * lr, 
				      int vreg, int *oparray);
void L_rdvr_print_liverange (LiveRange * lr);

/* Disjoint virtual register manipulation */

static void L_rdvr_dataflow_analysis(L_Func * fn);
static void L_rdvr_create_pre_live_ranges(L_Func * fn, HashTable vhash, 
					  Set nosplit);
static void L_rdvr_annotate_interference (L_Func *fn, List *plrlist);
static int L_rdvr_generate_disjvreg (L_Func *fn, List *plrlist, 
				     int do_rename, Set nosplit);
static void L_rdvr_add_ref_op (HashTable vhash, L_Oper *oper, L_Operand *opd);
static int L_rdvr_coalesce_live_ranges (L_Func * fn, List *plrlist);
static void L_rdvr_cleanup (List *plrlist);

/*
 * L_rename_disjoint_virtual_registers
 * ----------------------------------------------------------------------
 * Rename disjoint live ranges, so that each has an unique vreg, subject
 * to the constraints imposed by the set "nosplit" (the set of vregs
 * whose live ranges, though disjoint, must not be renamed) and the
 * rules in L_rdvr_new_live_range.
 */
void
L_rename_disjoint_virtual_registers (L_Func * fn, Set nosplit)
{
  List lrlist = NULL;

  L_rdvr_dataflow_analysis (fn);

#ifdef DEBUG_RENAME
  DB_spit_func (fn, "PRE-RENAME");
#endif

  L_rdvr_generate_disjvreg (fn, &lrlist, RDVR_DO_RENAME, nosplit);

#ifdef DEBUG_RENAME
  DB_spit_func (fn, "POST-RENAME");
#endif

  L_rdvr_cleanup (&lrlist);

  return;
}


void
L_rename_coalesce_disjoint_virtual_registers (L_Func * fn, Set nosplit)
{
  List lrlist = NULL;

  L_rdvr_dataflow_analysis (fn);

  L_rdvr_generate_disjvreg (fn, &lrlist, RDVR_DO_RENAME, nosplit);

  L_rdvr_dataflow_analysis (fn);

  L_rdvr_annotate_interference (fn, &lrlist);

  L_rdvr_coalesce_live_ranges (fn, &lrlist);

  L_rdvr_cleanup (&lrlist);

  return;
}


static LiveRange *
L_rdvr_new_live_range (L_Oper *def_op, L_Operand *def_dest)
{
  LiveRange *lr;

  lr = (LiveRange *) L_alloc (LiveRange_pool);
  lr->valid = 1;
  lr->id = def_op->id;
  lr->vreg = def_dest->value.r;
  lr->ctype = L_return_old_ctype (def_dest);
  lr->def_oper = def_op;
  lr->flags = 0;

  lr->def_op = Set_add (NULL, def_op->id);
  lr->ref_op = NULL;

  lr->op = NULL;

  /* prevent splitting of pre/post ince instr.   */

  if (L_preincrement_load_opcode (def_op) ||
      L_postincrement_load_opcode (def_op) ||
      L_preincrement_store_opcode (def_op) ||
      L_postincrement_store_opcode (def_op) ||
      L_bit_deposit_opcode (def_op) ||
      (def_op->opc == Lop_DEFINE) ||
      L_find_attr (def_op->attr, "do_not_split"))
    lr->flags |= RDVR_LR_NOSPLIT;

  lr->intf = NULL;
  lr->composition = NULL;

  return lr;
}


static LiveRange *
L_rdvr_delete_live_range (LiveRange *lr)
{
  if (lr)
    {
      Set_dispose (lr->op);
      Set_dispose (lr->def_op);
      Set_dispose (lr->ref_op);
      Set_dispose (lr->intf);
      Set_dispose (lr->composition);
      L_free (LiveRange_pool, lr);
    }

  return NULL;
}


void
L_rdvr_print_liverange (LiveRange * lr)
{
  fprintf (stdout, "***\nvreg %d\n", lr->vreg);
  fprintf (stdout, "def_oper %d\n", lr->def_oper->id);
  Set_print (stdout, "def_op", lr->def_op);
  Set_print (stdout, "ref_op", lr->ref_op);
  Set_print (stdout, "op", lr->op);
  Set_print (stdout, "intf", lr->intf);
  fprintf (stdout, "flags %08X\n", lr->flags);
}


static void
L_rdvr_dataflow_analysis(L_Func * fn)
{
  L_Cb *cb;
  L_Oper *oper;
  int i, j;
  
  L_setup_dataflow_no_operands (fn);

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      for (oper = cb->first_op; oper; oper = oper->next_op)
	{
	  /* guard predicate */

	  if (oper->pred[0] && L_is_reg(oper->pred[0]))
	    L_add_src_operand_reg (oper, oper->pred[0]->value.r, FALSE, TRUE);

	  /* dest operands */

	  for (i = 0; i < L_max_dest_operand; i++)
	    {
	      L_Operand *dest;

	      if ((dest = oper->dest[i]) && L_is_reg (dest))
		{
		  int trans = FALSE, uncond = FALSE;

		  if (L_is_ctype_predicate (dest))
		    {
		      if (L_is_update_predicate_ptype (dest->ptype))
			trans = TRUE;
		      if (L_is_uncond_predicate_ptype (dest->ptype))
			uncond = TRUE;
		    }

		  L_add_dest_operand_reg (oper, dest->value.r, trans, uncond);
		}
	    }
	  
	  /* src operands */

	  for (i = 0; i < L_max_src_operand; i++)
	    {
	      L_Operand *src;

	      if ((src = oper->src[i]) && L_is_reg (src))
		L_add_src_operand_reg (oper, src->value.r, FALSE, FALSE);
	    }

	  /* EMN: Extended source support */

	  if (oper->opc == Lop_CHECK)
	    {
	      L_Attr *attr = NULL;
	      
	      /* L_rdvr_dataflow_analysis is called from other places than
		 just register allocation, so oper might not have
		 an extended source */
	      if ((attr = L_find_attr (oper->attr, "src")))
		{
		  for (j = 0; j < attr->max_field; j++)
		    {
		      L_Operand *src;
		      
		      if ((src = attr->field[j]) && L_is_reg (src))
			L_add_src_operand_reg (oper, src->value.r, FALSE, FALSE);
		    }
		}
	    }
	}
    }

  L_dataflow_analysis (LIVE_VARIABLE | REACHING_DEFINITION);

  return;
}


static int
L_rdvr_create_live_ranges(L_Func * fn, LiveRange **LiveRanges)
{
  L_Cb *cb;
  L_Oper *oper;
  int two_operand, i, cnt = 0;

  /* This prevents spitting live ranges of the form:  */
  /*          r1 <-                                   */
  /*          r1 <- r1 + r2                           */
  /*             <- r1                                */
  /* Since x86 has a 2 operand format.                */
  /* REH 3/10/95                                      */

  two_operand = ((M_arch == M_X86) ||
		 (M_arch == M_TI) ||
		 (M_arch == M_SPARC && M_model == M_IM_SPARC_MCODE) ||
		 (M_arch == M_STARCORE) ||
		 (M_arch == M_SH));

  /*
   * Create a live range for each virtual register definition
   */

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    for (oper = cb->first_op; oper; oper = oper->next_op)
      for (i = 0; i < L_max_dest_operand; i++)
	{
	  L_Operand *dest;
	  LiveRange *lr;
	  
	  if (!(dest = oper->dest[i]) || !L_is_reg (dest))
	    continue;
	  
	  LiveRanges[cnt++] = lr = L_rdvr_new_live_range (oper, dest);

	  /* Prevent src/dest splitting on two-operand architectures */
	  
	  if (two_operand)
	    lr->op = Set_add (lr->op, oper->id);
	}

  return cnt;
}


static void
L_rdvr_merge_into (LiveRange *lr1, LiveRange *lr2)
{
  lr1->op = Set_union_acc (lr1->op, lr2->op);
  lr1->def_op = Set_union_acc (lr1->def_op, lr2->def_op);
  lr1->ref_op = Set_union_acc (lr1->ref_op, lr2->ref_op);
  lr1->intf = Set_union_acc (lr1->intf, lr2->intf);
  lr1->composition = Set_union_acc (lr1->composition, lr2->composition);

  if (lr2->flags & RDVR_LR_NOSPLIT)
    lr1->flags |= RDVR_LR_NOSPLIT;

  lr2->op = Set_dispose (lr2->op);
  lr2->def_op = Set_dispose (lr2->def_op);
  lr2->ref_op = Set_dispose (lr2->ref_op);
  lr2->intf = Set_dispose (lr2->intf);
  lr2->composition = Set_dispose (lr2->composition);
  lr2->valid = 0;
  return;
}


static void
L_rdvr_rename_live_range (L_Func *fn, LiveRange * lr, int vreg, int *oparray)
{
  int old_vreg, num_op, j, k, i;

  old_vreg = lr->vreg;

  /*
   * Loop through defining operations 
   */

  num_op = Set_2array (lr->def_op, oparray);
  for (j = 0; j < num_op; j++)
    {
      L_Attr *attr;
      L_Oper *oper = L_oper_hash_tbl_find_oper (fn->oper_hash_tbl,
						oparray[j]);

      for (k = 0; k < L_max_dest_operand; k++)
	{
	  L_Operand *dest;
	  if ((dest = oper->dest[k]) && L_is_reg (dest) &&
	      (dest->value.r == old_vreg))
	    dest->value.r = vreg;
	}

      /* rename all registers in ill_reg attributes SYH 9/24/96 */
      
      attr = oper->attr;
      while ((attr = L_find_attr (attr, "ill_reg")))
	{
	  if (attr && (int) attr->field[0]->value.i == old_vreg)
	    attr->field[0]->value.i = (ITintmax) vreg;
	  attr = attr->next_attr;
	}
    }

  /* 
   * Loop through referencing operations
   */

  num_op = Set_2array (lr->ref_op, oparray);
  for (j = 0; j < num_op; j++)
    {
      L_Attr *attr;
      L_Oper *oper = L_oper_hash_tbl_find_oper (fn->oper_hash_tbl,
						oparray[j]);

      /* guard predicate */

      {
	L_Operand *pred;
	if ((pred = oper->pred[0]) && L_is_reg (pred) &&
	    pred->value.r == old_vreg)
	  pred->value.r = vreg;
      }

      /* source registers */
      
      for (k = 0; k < L_max_src_operand; k++)
	{
	  L_Operand *src;
	  if ((src = oper->src[k]) && L_is_reg (src) && 
	      (src->value.r == old_vreg))
	    src->value.r = vreg;
	}

      /* rename all registers in ill_reg attributes SYH 9/24/96 */

      attr = oper->attr;
      while ((attr = L_find_attr (attr, "ill_reg")))
	{
	  if (attr && (int) attr->field[0]->value.i == old_vreg)
	    attr->field[0]->value.i = (ITintmax) vreg;
	  attr = attr->next_attr;
	}

      /* EMN: Extended source support */

      if ((oper->opc == Lop_CHECK)&&(attr = L_find_attr (oper->attr, "src")))
	{
	  for (i = 0; i < attr->max_field; i++)
	    {
	      L_Operand *src;
	      if ((src = attr->field[i]) && L_is_reg (src) && 
		  src->value.r == old_vreg)
		src->value.r = vreg;
	    }
	}
    }
  
  lr->vreg = vreg;
  
  return;
}


/*
 * L_rdvr_annotate_interference
 * ----------------------------------------------------------------------
 * Compute an interference set for each LiveRange, containing all vreg
 * indices which conflict (are live at the point of) defining opers.
 */
static void
L_rdvr_annotate_interference (L_Func *fn, List *plrlist)
{
  int sz, bufsz = 0, *buf = NULL, j;
  List lrlist = *plrlist;
  LiveRange *lr;
  L_Oper *oper;
  L_Cb *cb;

  List_start (lrlist);

  while ((lr = (LiveRange *)List_next (lrlist)))
    {
      if (lr->composition)
	lr->composition = Set_dispose (lr->composition);

      lr->composition = Set_add (NULL, lr->vreg);

      if (lr->intf)
	lr->intf = Set_dispose (lr->intf);

      sz = Set_size (lr->def_op);
 
      if (sz == 1)
	{
	  cb = L_oper_hash_tbl_find_cb (fn->oper_hash_tbl, lr->def_oper->id);
	  lr->intf = Set_copy (L_get_oper_OUT_set (cb, lr->def_oper,
						   BOTH_PATHS));
	}
      else
	{
	  if (sz > bufsz)
	    {
	      bufsz = 2 * sz;
	      if (buf)
		free (buf);
	      buf = Lcode_malloc (bufsz * sizeof (int));
	    }

	  Set_2array (lr->def_op, buf);
	  
	  for (j = 0; j < sz; j++)
	    {
	      oper = L_oper_hash_tbl_find_oper (fn->oper_hash_tbl, buf[j]);
	      cb = L_oper_hash_tbl_find_cb (fn->oper_hash_tbl, buf[j]);
	      lr->intf = Set_union_acc (lr->intf, 
					L_get_oper_OUT_set (cb, oper,
							    BOTH_PATHS));
	    }
	}
   }

  if (buf)
    free (buf);

  return;
}


/*
 * L_rdvr_coalesce_live_ranges
 * ----------------------------------------------------------------------
 * Coalesce disjoint live ranges connected by mov operations
 */
static int
L_rdvr_coalesce_live_ranges (L_Func * fn, List *plrlist)
{
  List lrlist = *plrlist;
  int levA, levB, cnt, *oparray;
  LiveRange *lrA, *lrB;
  L_Cb *cb;
  L_Oper *op;
  Set mov_ops = NULL;

  if (!lrlist)
    return 0;

  oparray = (int *) malloc ((fn->max_oper_id + 1) * sizeof (int));

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      for (op = cb->first_op; op; op = op->next_op)
	{
	  if (!L_is_predicated (op) &&
	      L_move_opcode (op))
	    mov_ops = Set_add (mov_ops, op->id);
	}
    }

  cnt = 0;

  levA = List_register_new_ptr (lrlist);
  levB = List_register_new_ptr (lrlist);

  List_start_l (lrlist, levA);
  while ((lrA = (LiveRange *)List_next_l (lrlist, levA)))
    {
      List_start_l (lrlist, levB);

      List_copy_current_ptr (lrlist, levB, levA);
      while ((lrB = (LiveRange *)List_next_l (lrlist, levB)))
	{
	  Set isect;

	  if (lrA->vreg == lrB->vreg)
	    continue;

	  if (lrA->ctype != lrB->ctype)
	    continue;
	  
	  if (!Set_intersect_empty (lrA->intf, lrB->composition) ||
	      !Set_intersect_empty (lrB->intf, lrA->composition))
	    continue;

	  if (!Set_intersect_empty (lrA->def_op, lrB->ref_op))
	    isect = Set_intersect (lrA->def_op, lrB->ref_op);
	  else if (!Set_intersect_empty (lrA->ref_op, lrB->def_op))
	    isect = Set_intersect (lrA->ref_op, lrB->def_op);
	  else
	    continue;

	  isect = Set_intersect_acc (isect, mov_ops);

	  if (Set_size (isect))
	    {
#ifdef DEBUG_RENAME
	      fprintf (stderr, "MRG %d -> %d\n", lrB->vreg, lrA->vreg);
#endif
	      lrA->composition = Set_add (lrA->composition, lrB->vreg);

	      L_rdvr_rename_live_range (fn, lrB, lrA->vreg, 
					oparray);

	      L_rdvr_merge_into (lrA, lrB);
	      L_rdvr_delete_live_range (lrB);

	      lrlist = List_delete_current_l (lrlist, levB);

	      cnt++;
	    }

	  Set_dispose (isect);	  
	}
    }

  List_free_all_ptrs (lrlist);

  free (oparray);

  *plrlist = lrlist;

  Set_dispose (mov_ops);

  return cnt;
}


static void
L_rdvr_add_ref_op (HashTable vhash, L_Oper *oper, L_Operand *opd)
{
  List lrlist;
  LiveRange *lr;
  int id, vreg, cnt = 0;

  id = oper->id;
  vreg = opd->value.r;

  lrlist = HashTable_find_or_null(vhash, vreg);

  List_start (lrlist);
  while ((lr = (LiveRange *)List_next (lrlist)))
    {
      if (L_in_oper_RIN_set_reg (oper, lr->def_oper, vreg))
	{
	  lr->ref_op = Set_add (lr->ref_op, id);
	  cnt++;
	}
    }

  return;
}


static void
L_rdvr_create_pre_live_ranges(L_Func * fn, HashTable vhash, Set nosplit)
{
  L_Cb *cb;
  L_Oper *oper;
  int two_operand, i, j;

  /* This prevents spitting live ranges of the form:  */
  /*          r1 <-                                   */
  /*          r1 <- r1 + r2                           */
  /*             <- r1                                */
  /* Since x86 has a 2 operand format.                */
  /* REH 3/10/95                                      */

  two_operand = ((M_arch == M_X86) ||
		 (M_arch == M_TI) ||
		 (M_arch == M_SPARC && M_model == M_IM_SPARC_MCODE) ||
		 (M_arch == M_STARCORE) ||
		 (M_arch == M_SH));

  /*
   * Create a live range for each virtual register definition
   */

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    for (oper = cb->first_op; oper; oper = oper->next_op)
      for (i = 0; i < L_max_dest_operand; i++)
	{
	  L_Operand *dest;
	  LiveRange *lr;
	  List lrl;
	  int vreg;

	  if (!(dest = oper->dest[i]) || !L_is_reg (dest))
	    continue;
	  
	  lr = L_rdvr_new_live_range (oper, dest);

	  /* Prevent src/dest splitting on two-operand architectures */
	  
	  if (two_operand)
	    lr->op = Set_add (lr->op, oper->id);

	  /* Insert the new live range into the vhash */

	  vreg = dest->value.r;
	  lrl = HashTable_find_or_null (vhash, vreg);
	  lrl = List_insert_last (lrl, lr);
	  HashTable_update (vhash, vreg, lrl);

	  if (Set_in (nosplit, vreg))
	    lr->flags |= RDVR_LR_NOSPLIT;
	}

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
      {
	L_Operand *opd;

	if ((opd = oper->pred[0]) && L_is_reg (opd))
	  L_rdvr_add_ref_op (vhash, oper, opd);

	/* src operands */
		
	for (j = 0; j < L_max_src_operand; j++)
	  if ((opd = oper->src[j]) && L_is_reg (opd))
	    L_rdvr_add_ref_op (vhash, oper, opd);
		
	/* EMN: Extended source support */
		
	if (oper->opc == Lop_CHECK)
	  {
	    L_Attr *attr;

	    if ((attr = L_find_attr (oper->attr, "src")))
	      {
		for (j = 0; j < attr->max_field; j++)
		  if ((opd = attr->field[j]) && L_is_reg (opd))
		    L_rdvr_add_ref_op (vhash, oper, opd);
	      }
	  }
      }
  
  return;
}


static int
L_rdvr_generate_disjvreg (L_Func *fn, List *plrlist, int do_rename, 
			  Set nosplit)
{
  HashTable vhash;
  List lrlist = NULL, worklist;
  LiveRange *lrA, *lrB;
  int vreg;
  int *oparray;

  oparray = (int *) malloc ((fn->max_oper_id + 1) * sizeof (int));

  if (!LiveRange_pool)
    LiveRange_pool = L_create_alloc_pool ("LiveRange", sizeof (LiveRange), 
					  fn->max_oper_id / 2 + 1);

  vhash = HashTable_create (2 * fn->max_reg_id);

  /* Create a live range record for each def, hashed by vreg id */

  L_rdvr_create_pre_live_ranges(fn, vhash, nosplit);

  HashTable_start (vhash);

  while ((worklist = (List) HashTable_next (vhash)))
    {
      int rename = 0;

      do
	{
	  int old_vreg, new_vreg, merged = 0;

	  List_start (worklist);
	  lrA = (LiveRange *)List_next (worklist);
	  worklist = List_delete_current (worklist);
	  vreg = lrA->vreg;

	  while ((lrB = (LiveRange *) List_next (worklist)))
	    {
	      if (Set_intersect_empty (lrA->ref_op, lrB->ref_op) &&
		  !Set_in (nosplit, vreg) &&
		  !((lrA->flags | lrB->flags) & RDVR_LR_NOSPLIT))
		{
		  /* lrA and lrB will be split */
		  continue;
		}

	      L_rdvr_merge_into (lrA, lrB);
	      if (lrB->flags & RDVR_LR_NOSPLIT)
		lrA->flags |= RDVR_LR_NOSPLIT;
	      L_rdvr_delete_live_range (lrB);	      
	      worklist = List_delete_current (worklist);
	      merged = 1;
	    }

	  if (merged)
	    {
	      worklist = List_insert_last (worklist, lrA);
	      continue;
	    }
	  else if (rename && 
		   !(lrA->flags & RDVR_LR_NOSPLIT))
	    {
	      new_vreg = ++(fn->max_reg_id);

#ifdef DEBUG_RENAME
	      fprintf (stderr, "RENAME r%d -> r%d\n", old_vreg, new_vreg);
	      Set_print (stderr, "def_opers", lrA->def_op);
	      Set_print (stderr, "ref_opers", lrA->ref_op);
#endif

	      L_rdvr_rename_live_range (fn, lrA, new_vreg, oparray);
	    }
	  else
	    {
#ifdef DEBUG_RENAME
	      fprintf (stderr, "LR r%d\n", vreg);
	      Set_print (stderr, "def_opers", lrA->def_op);
	      Set_print (stderr, "ref_opers", lrA->ref_op);
#endif
	      old_vreg = vreg;

	      if (do_rename == RDVR_DO_RENAME)
		rename = 1;
	    }

	  lrlist = List_insert_last (lrlist, lrA);
	}
      while (List_size(worklist));

      if (worklist)
	List_reset (worklist);
    }

  HashTable_free (vhash);

  free (oparray);

  *plrlist = lrlist;

  return List_size (lrlist);
}


static void
L_rdvr_cleanup (List *plrlist)
{
  List lrlist = *plrlist;

  List_reset_free_ptrs (lrlist, (void (*)(void *))L_rdvr_delete_live_range);

  if (LiveRange_pool)
    L_free_alloc_pool (LiveRange_pool);

  LiveRange_pool = NULL;

  *plrlist = lrlist;

  return;
}


static int
L_check_for_coalescing (L_Func *fn, LiveRange *lr1, LiveRange *lr2)
{
  Set lrs;
  int i, num_op, valid = 0, *oparray;

  oparray = (int *) malloc ((fn->max_oper_id + 1) * sizeof (int));

  if (lr2 && (lr2->valid == 1))
    {
      lrs = Set_intersect (lr1->op, lr2->op);
      num_op = Set_2array (lrs, oparray);
      valid = 1;
      for (i = 0; i < num_op; i++)
	{
	  if (!Set_in (lr1->def_op, oparray[i]))
	    {
	      valid = 0;
	      break;
	    }
	}
      Set_dispose (lrs);
    }

  return valid;
}


static void
L_rdvr_compress_live_ranges (LiveRange **LiveRanges, int numLiveRange)
{
  int i, dest;

  dest = -1;
  for (i = 0; i < numLiveRange; i++)
    {
      LiveRange *lr = LiveRanges[i];

      if (!lr)
	continue;

      if (!lr->valid)
	{
	  if (dest == -1)
	    dest = i;

	  LiveRanges[i] = L_rdvr_delete_live_range (lr);
	  continue;
	}

      if (dest != -1)
	{
	  LiveRanges[dest++] = LiveRanges[i];
	  LiveRanges[i] = NULL;
	}
    }
  return;
}


static void
L_rdvr_merge_live_ranges (L_Func * fn, LiveRange **LiveRanges, 
			  int numLiveRange, Set nosplit)
{
  int i, j, change;
  
  /* 
   * Merge non-disjoint live ranges 
   */

  do
    {
      change = 0;

      for (i = 0; i < numLiveRange; i++)
	{
	  LiveRange *lr1 = LiveRanges[i];

	  if (!lr1 || !lr1->valid)
	    continue;

	  for (j = i + 1; j < numLiveRange; j++)
	    {
	      LiveRange *lr2 = LiveRanges[j];

	      if (!lr2 || !lr2->valid || (lr1->vreg != lr2->vreg))
		continue;

	      /* If the oper sets for the live ranges have no oper
	         in common and the live ranges are not rotating registers,
	         then do not merge the ranges. */

	      if (Set_intersect_empty (lr1->ref_op, lr2->ref_op) &&
		  !Set_in (nosplit, lr1->vreg))
		continue;

	      L_rdvr_merge_into (lr1, lr2);

	      change++;
	    }
	}

      L_rdvr_compress_live_ranges (LiveRanges, numLiveRange);
    }
  while (change);

  return;
}


static void
L_rdvr_compute_liveness_live_ranges(L_Func * fn, LiveRange **LiveRanges, 
				    int numLiveRange)
{
  L_Cb *cb;
  L_Oper *oper;
  int i, j;

  /*
   * Determine the set of instructions over which each definition
   * of a virtual register is live.
   */

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
      {
	int id = oper->id;
	Set v_in = L_get_oper_IN_set (oper);

	for (i = 0; i < numLiveRange; i++)
	  {
	    LiveRange *lr = LiveRanges[i];
	    int vreg = lr->vreg;

	    if (Set_in (v_in, vreg) &&
		L_in_oper_RIN_set_reg (oper, lr->def_oper, vreg))
	      {
		lr->op = Set_add (lr->op, oper->id);
		
		/* guard predicate */

		{
		  L_Operand *pred;
		  if ((pred = oper->pred[0]) && L_is_reg (pred) &&
		      (pred->value.r == vreg))
		    lr->ref_op = Set_add (lr->ref_op, id);
		}

		/* src operands */
		
		for (j = 0; j < L_max_src_operand; j++)
		  {
		    L_Operand *src;
		    if ((src = oper->src[j]) && L_is_reg (src) && 
			(src->value.r == vreg))
		      lr->ref_op = Set_add (lr->ref_op, id);
		  }
		
		/* EMN: Extended source support */
		
		if (oper->opc == Lop_CHECK)
		  {
		    L_Attr *attr;

		    if ((attr = L_find_attr (oper->attr, "src")))
		      {
			for (j = 0; j < attr->max_field; j++)
			  {
			    L_Operand *src;
			    if ((src = attr->field[j]) && L_is_reg (src) &&
				(src->value.r == vreg))
			      lr->ref_op = Set_add (lr->ref_op, id);
			  }
		      }
		  }
	      }
	  }
      }

  return;
}


static void
L_rdvr_delete_live_ranges (LiveRange **LiveRanges, int numLiveRange)
{
  int i;

  for (i = 0; i < numLiveRange; i++)
    if (LiveRanges[i])
      LiveRanges[i] = L_rdvr_delete_live_range (LiveRanges[i]);
  return;
}


void
L_coalesce_live_ranges (L_Func *fn)
{
  L_Oper *oper;
  L_Operand *dest, *src, *pred;
  L_Attr *ill_reg;
  LiveRange **LiveRanges, **vrarray, *lr1, *lr2 = NULL;
  int i, j, numLiveRange, flag, old_vreg, new_vreg, num_op, *oparray, 
    valid = 0;

  if (!LiveRange_pool)
    LiveRange_pool = L_create_alloc_pool ("LiveRange", sizeof (LiveRange), 
					  fn->max_oper_id / 2 + 1);

  L_rename_disjoint_virtual_registers (fn, NULL);

  L_rdvr_dataflow_analysis (fn);

  LiveRanges =
    (LiveRange **) calloc (((fn->max_oper_id + 1) * L_max_dest_operand),
			   sizeof (LiveRange *));

  numLiveRange = L_rdvr_create_live_ranges (fn, LiveRanges);

  L_rdvr_compute_liveness_live_ranges (fn, LiveRanges, numLiveRange);

  L_rdvr_merge_live_ranges (fn, LiveRanges, numLiveRange, NULL);

  vrarray = (LiveRange **) calloc (fn->max_reg_id + 1, sizeof (LiveRange *));

  /* Setup vrarray that will point to each virtual register's live range */
  for (i = 0; i < numLiveRange; i++)
    {
      if (LiveRanges[i] && LiveRanges[i]->valid)
	vrarray[LiveRanges[i]->vreg] = LiveRanges[i];
    }

  for (i = 0; i < numLiveRange; i++)
    {
      lr1 = LiveRanges[i];
      if (!lr1 || !lr1->valid)
	continue;

      oper = lr1->def_oper;

      /* Mspec call to determine which source operand would benefit */
      if ((flag = M_coalescing_oper (oper)) == 0)
	continue;

      /* Check src[0] */
      if ((flag & 1) && L_is_reg (oper->src[0]))
	{
	  lr2 = vrarray[oper->src[0]->value.r];

	  valid = L_check_for_coalescing (fn, lr1, lr2);

#ifdef DEBUG_COALESCE
	  printf ("comparing %d and %d for op %d\n", lr1->vreg, lr2->vreg,
		  oper->id);
	  printf ("valid = %d\n", valid);
#endif
	}

      /* Try src[1] if src[0] fails */
      if (!valid && (flag & 2) && L_is_reg (oper->src[1]))
	{
	  lr2 = vrarray[oper->src[1]->value.r];

	  valid = L_check_for_coalescing (fn, lr1, lr2);

#ifdef DEBUG_COALESCE
	  printf ("comparing %d and %d for op %d\n", lr1->vreg, lr2->vreg,
		  oper->id);
	  printf ("valid = %d\n", valid);
#endif
	}

      if (valid)
	{
	  /* Coalesce the live ranges */
#ifdef DEBUG_COALESCE
	  fprintf (stdout, "Merging Live Range %d %d\n", lr1->vreg, lr2->vreg);
	  L_rdvr_print_liverange (lr1);
	  L_rdvr_print_liverange (lr2);
#endif

	  oparray = (int *) malloc ((fn->max_oper_id + 1) * sizeof (int));

	  /* Change all defs and refs of lr1 to lr2 */
	  old_vreg = lr1->vreg;
	  new_vreg = lr2->vreg;

	  /* Loop through all defining operations */
	  num_op = Set_2array (lr1->def_op, oparray);
	  for (i = 0; i < num_op; i++)
	    {
	      oper = L_oper_hash_tbl_find_oper (fn->oper_hash_tbl, oparray[i]);

	      /* rename all registers in ill_reg attributes SYH 9/24/96 */
	      ill_reg = oper->attr;
	      while ((ill_reg = L_find_attr (ill_reg, "ill_reg")))
		{
		  if (ill_reg && (int) ill_reg->field[0]->value.i == old_vreg)
		    ill_reg->field[0]->value.i = (ITintmax) new_vreg;
		  ill_reg = ill_reg->next_attr;
		}

	      for (j = 0; j < L_max_dest_operand; j++)
		{
		  if ((dest = oper->dest[j]) && L_is_reg (dest) && 
		      dest->value.r == old_vreg)
		    dest->value.r = new_vreg;
		}
	    }

	  /* Loop through referencing operations */
	  num_op = Set_2array (lr1->ref_op, oparray);
	  for (i = 0; i < num_op; i++)
	    {
	      oper = L_oper_hash_tbl_find_oper (fn->oper_hash_tbl, oparray[i]);

	      /* guard predicate */
	      if ((pred = oper->pred[0]) &&
		  L_is_reg (pred) &&
		  pred->value.r == old_vreg)
		pred->value.r = new_vreg;

	      /* source registers */
	      for (j = 0; j < L_max_src_operand; j++)
		{
		  if ((src = oper->src[j]) &&
		      L_is_reg (src) && 
		      (src->value.r == old_vreg))
		    src->value.r = new_vreg;
		}

	      /* rename all registers in ill_reg attributes SYH 9/24/96 */
	      ill_reg = oper->attr;
	      while ((ill_reg = L_find_attr (ill_reg, "ill_reg")))
		{
		  if (ill_reg && (int) ill_reg->field[0]->value.i == old_vreg)
		    ill_reg->field[0]->value.i = (ITintmax) new_vreg;
		  ill_reg = ill_reg->next_attr;
		}
	    }

	  /* Merge lr1 into lr2 */
	  lr2->op = Set_union_acc (lr2->op, lr1->op);
	  lr2->def_op = Set_union_acc (lr2->def_op, lr1->def_op);
	  lr2->ref_op = Set_union_acc (lr2->ref_op, lr1->ref_op);
	  lr1->valid = 0;
	}
    }

  L_rdvr_delete_live_ranges (LiveRanges, numLiveRange);
  free (LiveRanges);

  if (LiveRange_pool)
    L_free_alloc_pool (LiveRange_pool);

  LiveRange_pool = NULL;
  return;
}

