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
 *  File: icache_pad.c
 *
 *  Description: Cache align the top of loops and branch targets by padding
 *               bundles with nops.
 *               This should be called in phase 2, after both prepass and
 *               postpass scheduling (and compaction).  
 *
 *  Authors: Bob McGowan - 5/19/96
 *  Modified: Bob McGowan - 9/97 - Branches with TAR hints should not go
 *                                 in the same cache line as another branch.
 *                                 If this would happen, the other branch
 *                                 would be incorrectly predicted by the TAR.
 *                                 New code was added to put TAR hinted
 *                                 branches in a seperate cache line.
 *
\*****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "ltahoe_main.h"
#include "ltahoe_op_query.h"
#include "phase2_icache.h"

#undef DEBUG

#define NOP_PAD_VALUE 1
#define TAR_HINT_ATTR "TAR_HINTED"

#define IS_NOT_CACHE_ALIGNED(bundle_cnt) ((bundle_cnt) & 0x1)
#define SECOND_BUNDLE_IN_CACHE_LINE(bundle_cnt) ((~(bundle_cnt)) & 0x1)

/****************************************************************************
 *
 * routine: Ltahoe_pad_bundle()
 * purpose: This routine will expand one bundle into two bundles without
 *          creating a template or 6-wide issue stall.  It will also maintain
 *          the same ports for each of the syllables.
 * input: cb - block which owns the given bundle.
 *        bundle - pointer to the template define on the bundle which should
 *                 be expanded.
 * output:
 * returns: A pointer to a B syllable with a NOP.
 *          If no empty B syllable is created in the process of padding,
 *            but one could be created for a hint by converting a bundle type,
 *            the return value will point to the template define that needs to
 *            be converted.  Note that the return will not point to a nop.b in
 *            the original bundle if one existed. 
 *          If the bundle could not be expanded, then NULL is returned.
 * modified:
 * note: Capital letters in the notation signify nops.
 *-------------------------------------------------------------------------*/

L_Oper *
Ltahoe_pad_bundle (L_Cb * cb, L_Oper * bundle)
{
  L_Oper *op, *op2, *nop, *bnop, *tmpl;
  int template;
  int stopbit_mask;

  template = LT_get_template (bundle);
  stopbit_mask = LT_get_stop_bit_mask (bundle);

  /* the first instruction in the bundle */
  op = bundle->next_op;

  if (stopbit_mask & S_AFTER_3RD)
    {
      /* stop bit at the end of the bundle */
      switch (template)
	{
	case MSMI:
	  /* m;mi; => mIB; miB; */
	  LT_set_template (bundle, MIB);
	  LT_set_stop_bit_mask (bundle, S_AFTER_3RD /* at end */ );
	  op = op->next_op;	/* second original m */
	  nop = LT_create_nop (TAHOEop_NOP_I, NOP_PAD_VALUE);
	  L_insert_oper_before (cb, op, nop);
	  nop = LT_create_nop (TAHOEop_NOP_B, NOP_PAD_VALUE);
	  L_insert_oper_before (cb, op, nop);
	  tmpl = LT_create_template_op (MIB, S_AFTER_3RD /* at end */ );
	  L_insert_oper_before (cb, op, tmpl);
	  nop = LT_create_nop (TAHOEop_NOP_B, NOP_PAD_VALUE);
	  op = op->next_op;	/* original i */
	  L_insert_oper_after (cb, op, nop);
	  return (nop);

	case MISI:
	  /* mi;i; => miB; MiB; */
	  LT_set_template (bundle, MIB);
	  LT_set_stop_bit_mask (bundle, S_AFTER_3RD /* at end */ );
	  op = op->next_op->next_op;	/* 3rd original instr */
	  nop = LT_create_nop (TAHOEop_NOP_B, NOP_PAD_VALUE);
	  L_insert_oper_before (cb, op, nop);
	  tmpl = LT_create_template_op (MIB, S_AFTER_3RD /* at end */ );
	  L_insert_oper_before (cb, op, tmpl);
	  nop = LT_create_nop (TAHOEop_NOP_M, NOP_PAD_VALUE);
	  L_insert_oper_before (cb, op, nop);
	  nop = LT_create_nop (TAHOEop_NOP_B, NOP_PAD_VALUE);
	  L_insert_oper_after (cb, op, nop);
	  return (op);

	default:
	  /* mii mli mmi mfi mib mbb bbb mmb mfb */
	  /* find the previous template to see if the previous template
	     ended in a stop bit */
	  for (tmpl = bundle->prev_op;
	       tmpl && !LT_is_template_op (tmpl); tmpl = tmpl->prev_op);
	  if (!tmpl || (LT_get_stop_bit_mask (tmpl) & S_AFTER_3RD))
	    {
	      /* ; xxx ; or beginning of cb.  If it is the beginning of a
	         cb we know that whatever is in the above cb ends with
	         a stop bit */
	      if (LT_syllable_type (template, 2) == B_SYLL)
		{
		  switch (template)
		    {
		    case MMB:
		      /* mmb; => mFI mIb; */
		      LT_set_template (bundle, MFI);
		      LT_set_stop_bit_mask (bundle, NO_S_BIT);
		      op = op->next_op;	/* second original m */
		      nop = LT_create_nop (TAHOEop_NOP_F, NOP_PAD_VALUE);
		      L_insert_oper_before (cb, op, nop);
		      nop = LT_create_nop (TAHOEop_NOP_I, NOP_PAD_VALUE);
		      L_insert_oper_before (cb, op, nop);
		      tmpl = LT_create_template_op (MIB, S_AFTER_3RD);
		      L_insert_oper_before (cb, op, tmpl);
		      nop = LT_create_nop (TAHOEop_NOP_I, NOP_PAD_VALUE);
		      L_insert_oper_after (cb, op, nop);
		      return (bundle);

		    case MIB:
		      /* mib; => mFi MIb; */
		      LT_set_template (bundle, MFI);
		      LT_set_stop_bit_mask (bundle, NO_S_BIT);
		      op = op->next_op;	/* original i */
		      nop = LT_create_nop (TAHOEop_NOP_F, NOP_PAD_VALUE);
		      L_insert_oper_before (cb, op, nop);
		      tmpl = LT_create_template_op (MIB, S_AFTER_3RD);
		      L_insert_oper_after (cb, op, tmpl);
		      nop = LT_create_nop (TAHOEop_NOP_M, NOP_PAD_VALUE);
		      L_insert_oper_after (cb, tmpl, nop);
		      op = LT_create_nop (TAHOEop_NOP_I, NOP_PAD_VALUE);
		      L_insert_oper_after (cb, nop, op);	/* notice nop &
								   op are 
								   swapped */
		      return (bundle);

		    case MBB:
		      /* mbb; => mFI Mbb; */
		      LT_set_template (bundle, MFI);
		      LT_set_stop_bit_mask (bundle, NO_S_BIT);
		      op = op->next_op;	/* first original b */
		      nop = LT_create_nop (TAHOEop_NOP_F, NOP_PAD_VALUE);
		      L_insert_oper_before (cb, op, nop);
		      nop = LT_create_nop (TAHOEop_NOP_I, NOP_PAD_VALUE);
		      L_insert_oper_before (cb, op, nop);
		      tmpl = LT_create_template_op (MBB, S_AFTER_3RD);
		      L_insert_oper_before (cb, op, tmpl);
		      nop = LT_create_nop (TAHOEop_NOP_M, NOP_PAD_VALUE);
		      L_insert_oper_before (cb, op, nop);
		      return (bundle);

		    case BBB:
		      /* bbb; => MFI bbb; */
		      tmpl = LT_create_template_op (MFI, NO_S_BIT);
		      L_insert_oper_before (cb, bundle, tmpl);
		      nop = LT_create_nop (TAHOEop_NOP_M, NOP_PAD_VALUE);
		      L_insert_oper_before (cb, bundle, nop);
		      nop = LT_create_nop (TAHOEop_NOP_F, NOP_PAD_VALUE);
		      L_insert_oper_before (cb, bundle, nop);
		      nop = LT_create_nop (TAHOEop_NOP_I, NOP_PAD_VALUE);
		      L_insert_oper_before (cb, bundle, nop);
		      return (tmpl);

		    case MFB:
		      /* mfb; => mfI MIb; */
		      LT_set_template (bundle, MFI);
		      LT_set_stop_bit_mask (bundle, NO_S_BIT);
		      op = op->next_op->next_op;	/* original b */
		      nop = LT_create_nop (TAHOEop_NOP_I, NOP_PAD_VALUE);
		      L_insert_oper_before (cb, op, nop);
		      tmpl = LT_create_template_op (MIB, S_AFTER_3RD);
		      L_insert_oper_before (cb, op, tmpl);
		      nop = LT_create_nop (TAHOEop_NOP_M, NOP_PAD_VALUE);
		      L_insert_oper_before (cb, op, nop);
		      nop = LT_create_nop (TAHOEop_NOP_I, NOP_PAD_VALUE);
		      L_insert_oper_before (cb, op, nop);
		      return (tmpl);

		    default:
		      L_punt ("pad_bundle: Unknown template type\n");
		    }		/* switch */
		}		/* if */
	      else
		{
		  /* mii; mli; mmi; mfi; => xxx BBB; */
		  LT_set_stop_bit_mask (bundle, NO_S_BIT);
		  if (template == MLI)
		    op = op->next_op;	/* 2nd original instr = l */
		  else
		    op = op->next_op->next_op;	/* 3rd original instr */
		  tmpl = LT_create_template_op (BBB, S_AFTER_3RD);
		  L_insert_oper_after (cb, op, tmpl);
		  op = LT_create_nop (TAHOEop_NOP_B, NOP_PAD_VALUE);
		  L_insert_oper_after (cb, tmpl, op);
		  /* now put the other 2 in front of this B */
		  nop = LT_create_nop (TAHOEop_NOP_B, NOP_PAD_VALUE);
		  L_insert_oper_before (cb, op, nop);
		  nop = LT_create_nop (TAHOEop_NOP_B, NOP_PAD_VALUE);
		  L_insert_oper_before (cb, op, nop);
		  return (op);
		}		/* else */
	    }			/* if */
	  else
	    {
	      /* m;mi xxx; or mi;i xxx; or xxx xxx; */
	      /* Unable to pad this bundle */
	      return (NULL);
	    }			/* else */
	}			/* switch */
    }				/* if */
  else
    {
      /* no stop bit end of bundle */
      if (template == MSMI)
	{
	  /* m;mi => mFB; mFi */

	  LT_set_template (bundle, MFB);
	  LT_set_stop_bit_mask (bundle, S_AFTER_3RD);
	  op = op->next_op;	/* second original m */
	  nop = LT_create_nop (TAHOEop_NOP_F, NOP_PAD_VALUE);
	  L_insert_oper_before (cb, op, nop);
	  bnop = LT_create_nop (TAHOEop_NOP_B, NOP_PAD_VALUE);
	  L_insert_oper_before (cb, op, bnop);
	  tmpl = LT_create_template_op (MFI, NO_S_BIT);
	  L_insert_oper_before (cb, op, tmpl);
	  nop = LT_create_nop (TAHOEop_NOP_F, NOP_PAD_VALUE);
	  L_insert_oper_after (cb, op, nop);
	  return (bnop);
	}			/* if */
      else
	{
	  if (template == MISI)
	    {
	      int template2;

	      /* find the next template, and determine if the second
	         syllable is an M */

	      for (tmpl = op->next_op; tmpl && !LT_is_template_op (tmpl);
		   tmpl = tmpl->next_op);
	      if (!tmpl)
		L_punt ("pad_bundle mi;i (no stop at end) "
			"is last bundle in cb");

	      template2 = LT_get_template (tmpl);
	      if (template2 == MMI || template2 == MMB)
		{
		  /* mi;i mmx; */
		  /* We know that the second template has a stop bit
		     in it since Merced cannot execute from more than
		     2 bundles. */
		  /* Second syllable in the following template is a M
		     as in mmi or mmb.  These take much more work to
		     expand. */
		  op = op->next_op->next_op;	/* 3rd instr in mi;i */
		  if (op->proc_opc == TAHOEop_NOP_I)
		    {
		      /* mi;I mmi; => miB; mFI miB;
		         mi;I mmb; => miB; mFI mIb; */
		      bnop = op;

		      LT_set_template (bundle, MIB);
		      LT_set_stop_bit_mask (bundle, S_AFTER_3RD);
		      bnop->proc_opc = TAHOEop_NOP_B;

		      LT_set_template (tmpl, MFI);
		      LT_set_stop_bit_mask (tmpl, NO_S_BIT);

		      op = tmpl->next_op->next_op;	/* second original m */
		      nop = LT_create_nop (TAHOEop_NOP_F, NOP_PAD_VALUE);
		      L_insert_oper_before (cb, op, nop);
		      nop = LT_create_nop (TAHOEop_NOP_I, NOP_PAD_VALUE);
		      L_insert_oper_before (cb, op, nop);
		      tmpl = LT_create_template_op (MIB, S_AFTER_3RD);
		      L_insert_oper_before (cb, op, tmpl);

		      if (template2 == MMI)
			{
			  nop = LT_create_nop (TAHOEop_NOP_B, NOP_PAD_VALUE);
			  op = op->next_op;
			  L_insert_oper_after (cb, op, nop);
			}	/* if */
		      else
			{
			  nop = LT_create_nop (TAHOEop_NOP_I, NOP_PAD_VALUE);
			  L_insert_oper_after (cb, op, nop);
			}	/* else */

		      return (bnop);
		    }		/* if */
		  else
		    {
		      op2 = tmpl->next_op;	/* 1st instr in 2nd bundle */
		      if ((op2->proc_opc == TAHOEop_NOP_M) ||
			  (((op2->dest[0] == NULL) ||
			    (!L_is_src_operand (op2->dest[0], op) &&
			     !L_same_operand (op2->dest[0], op->pred[0]))) &&
			   ((op2->dest[1] == NULL) ||
			    (!L_is_src_operand (op2->dest[1], op) &&
			     !L_same_operand (op2->dest[1], op->pred[0])))))
			{
			  /* mi;i Mmi; => miB; MFi miB;
			     mi;i Mmb; => miB; MFi mIb;
			     op points to last i in mi;i
			     op2 points to the M in the second bundle 
			     which is a nop.

			     mi;i mmi; => miB; mFi miB; 
			     mi;i mmb; => miB; mFi mIb;
			     op points to last i in mi;i
			     op2 points to the first m in second bundle which
			     is not a nop but can be swapped with the i at 
			     op */
			  LT_set_template (bundle, MIB);	/* 1st bundle */
			  LT_set_stop_bit_mask (bundle, S_AFTER_3RD);
			  bnop = LT_create_nop (TAHOEop_NOP_B, NOP_PAD_VALUE);
			  L_insert_oper_before (cb, op, bnop);
			  /* first new b */

			  /* take the nop.m and move it before the i */
			  L_remove_oper (cb, op2);	/* original nop.m */
			  L_insert_oper_before (cb, op, op2);
			  nop = LT_create_nop (TAHOEop_NOP_F, NOP_PAD_VALUE);
			  L_insert_oper_before (cb, op, nop);

			  LT_set_template (tmpl, MIB);
			  /* this will become the 3rd */
			  LT_set_stop_bit_mask (tmpl, S_AFTER_3RD);
			  /* make op point to last syllable in
			     original 2nd bundle, this is either an i
			     or a b */
			  op = tmpl->next_op->next_op;
			  if (template2 == MMI)
			    {
			      nop = LT_create_nop (TAHOEop_NOP_B,
						   NOP_PAD_VALUE);
			      L_insert_oper_after (cb, op, nop);
			    }	/* if */
			  else
			    {
			      nop = LT_create_nop (TAHOEop_NOP_I,
						   NOP_PAD_VALUE);
			      L_insert_oper_before (cb, op, nop);
			    }	/* else */

			  /* create template for new 2nd bundle. */
			  tmpl = LT_create_template_op (MFI, NO_S_BIT);
			  L_insert_oper_before (cb, op2, tmpl);

			  return (bnop);
			}	/* if */
		      else
			{
			  return (NULL);
			}	/* else */
		    }		/* else */
		}		/* if */
	      else
		{
		  /* mi;i x(;)yx => miB; MFi x(;)yx, where y != m */

		  LT_set_template (bundle, MIB);
		  LT_set_stop_bit_mask (bundle, S_AFTER_3RD);
		  op = op->next_op->next_op;	/* 3rd original instr */
		  bnop = LT_create_nop (TAHOEop_NOP_B, NOP_PAD_VALUE);
		  L_insert_oper_before (cb, op, bnop);
		  tmpl = LT_create_template_op (MFI, NO_S_BIT);
		  L_insert_oper_before (cb, op, tmpl);
		  nop = LT_create_nop (TAHOEop_NOP_M, NOP_PAD_VALUE);
		  L_insert_oper_before (cb, op, nop);
		  nop = LT_create_nop (TAHOEop_NOP_F, NOP_PAD_VALUE);
		  L_insert_oper_before (cb, op, nop);
		  return (bnop);
		}		/* else */
	    }			/* if */
	  else
	    {
	      /* mii mli mmi mfi mib mbb mmb mfb with no stop at end */
	      return (NULL);
	    }			/* else */
	}			/* else */
    }				/* else */
  L_punt ("pad_bundle: Unknown template encountered");
  return (NULL);
}				/* Ltahoe_pad_bundle */


static int
Ltahoe_pad_cb_for_cache_alignment (L_Cb * cb)
{
  L_Oper *first_bundle, *op;

  do
    {
      /* find the first bundle template */
      for (first_bundle = cb->first_op;
	   first_bundle && !LT_is_template_op (first_bundle);
	   first_bundle = first_bundle->next_op);

      /* Start at the last bundle and walk up the ops until we find a
         spot to pad.  Do not pad the first bundle. */
      for (op = cb->last_op; op != first_bundle; op = op->prev_op)
	if (LT_is_template_op (op) && Ltahoe_pad_bundle (cb, op))
	  return (1);	/* able to pad */

      /* op == first_bundle, This means that we did not find a place
         to pad.  Go up and try to pad the previous cb only if cb is
         self aligned.  Self aligned means that there is a stop bit in
         or at the end of the first bundle. */
    }
  while ((!first_bundle ||
	  (LT_get_stop_bit_mask (first_bundle) != NO_S_BIT))
	 && (cb = cb->prev_cb));
  return (0);
}				/* Ltahoe_pad_cb_for_cache_alignment */


int
Check_and_align_for_TAR (L_Cb * cb, L_Oper * template, int bundle_count)
{
  L_Oper *op;

  if (SECOND_BUNDLE_IN_CACHE_LINE (bundle_count) &&
      L_find_attr (template->attr, TAR_HINT_ATTR))
    {
      /* A branch with a TAR hint exists in the current bundle.
         This branch is in the second of 2 bundles in a cache line.
         A conflict will occur if there is also a branch in the first
         bundle in the cache line.
         If there is a branch in the previous bundle we need to pad this
         bundle to move it to the next cache line. */
      for (op = template->prev_op;
	   op && !LT_is_template_op (op); op = op->prev_op)
	{
	  if (L_general_branch_opcode (op) || L_subroutine_call_opcode (op))
	    {
	      /* A branch has been found.  Pad the second bundle. */
	      if (Ltahoe_pad_bundle (cb, template))
		return (1);
	      else
		return (0);
	    }			/* if */
	}			/* for op */
      return (0);
    }				/* if */
  else
    {
      return (0);
    }				/* else */
}				/* Check_and_align_for_TAR */


void
Ltahoe_cache_align_cbs (L_Func * fn, double threshold)
{
  L_Cb *cb, *pcb;
  L_Oper *template;
  int bundle_count = 0, do_tar;

  if (!(cb = fn->first_cb))
    return;

  do_tar = (M_model == M_IPF_ITANIUM);

  for (cb = fn->first_cb, pcb = NULL; cb; pcb = cb, cb = cb->next_cb)
    {
      /* skip empty blocks */
      if (!cb->first_op)
	continue;

#ifdef DEBUG
      fprintf (stderr, "  %d bundles before cb %d\n", bundle_count, cb->id);
#endif

      /* Find the first template */
      for (template = cb->first_op;
	   template && !LT_is_template_op (template);
	   template = template->next_op);

      if (!template)
	continue;

      if (pcb)
	{
	  L_Flow *ft_flow;
	  double ft_weight;

	  /* fallthrough flows don't count towards decision weight,
	   * since the straight-line path needn't be cache-aligned 
	   */

	  if (pcb->dest_flow &&
	      (ft_flow = L_find_last_flow (pcb->dest_flow)) &&
	      (ft_flow->dst_cb == cb))
	    ft_weight = ft_flow->weight;
	  else
	    ft_weight = 0.0;

	  if (IS_NOT_CACHE_ALIGNED (bundle_count) && 
	      ((cb->weight - ft_weight) >= threshold) &&
	      (LT_get_stop_bit_mask (template) == NO_S_BIT))
	    {
	      /* This cb is not cache aligned and the first bundle does not
		 have a stop bit in it or at the end of it.
		 A stall will occur if we try to execute two bundles in a
		 cycle after a taken branch and the two bundles are not in
		 the same cache line */
#ifdef DEBUG
	      fprintf (stderr, "  Need to pad cb %d\n", cb->id);
#endif
	      if (Ltahoe_pad_cb_for_cache_alignment (pcb))
		{
		  /* Added a block to the previous cb to align this block. */
		  bundle_count++;
		}
	      else
		{
		  L_Attr *not_aligned;

		  /* not able to align this cb */
		  not_aligned = L_new_attr ("NOT_ALIGNED", 0);
		  cb->attr = L_concat_attr (not_aligned, cb->attr);
		}
	    }
	}

      do
	{
	  /* template points to a template oper */
	  
	  bundle_count++;

	  if (do_tar && Check_and_align_for_TAR (cb, template, bundle_count))
	    bundle_count++;

	  for (template = template->next_op;
	       template && !LT_is_template_op (template); 
	       template = template->next_op);
	}
      while (template);
    }
}				/* Ltahoe_cache_align_cbs */
