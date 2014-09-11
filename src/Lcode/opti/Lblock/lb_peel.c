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
 * lb_peel.c
 * ---------------------------------------------------------------------------
 * Loop peeling
 * ---------------------------------------------------------------------------
 * Generalized from loop peeling developed by D. August and S. Mahlke, 1994.
 * 20021031 J. Sias
 *****************************************************************************/

#include "lb_b_internal.h"
#include "lb_peel.h"

#undef DEBUG_PEEL

#define ERR stderr

static void LB_adjust_weight_for_peel (L_Func *fn, L_Loop *loop, 
				       double *pinvoc_wt, double *piter_wt,
				       double *precov_wt);

static void LB_attach_peel_attribute (L_Cb * cb, int iter_num, 
				      int peel_loop_id);

/* LB_peel_loop - 20021009 JWS
 * ----------------------------------------------------------------------
 * Peel the subgraph consisting of the set peel_cbs out of a loop,
 * changing all non-loopback flows from region_cbs (or from the entire
 * function if region_cbs is NULL) to point to the peeled header.
 *
 * fn          - Lcode function pointer
 * loop        - Lcode loop structure (Loop detection required)
 * peel_cbs    - Set of id's of CBs within the body of the specified loop
 *               which are to peeled.  If peel_cbs < loop->loop_cb, the
 *               remainder loop will be improper.
 * region_cbs  - Set of CBs whose outgoing arcs are to be redirected to
 *               the header of the peeled CBs.  If NULL, taken to be the
 *               universe of CBs.
 * peel_id     - For peel attribute: meaningless peel id
 * peel_num    - For peel attribute: iteration being peeled
 * peeled_cbs  - If non-NULL, pointer to a Set which will be updated to
 *               contain ids of peeled CBs (in accumulating fashion).
 * mod_cbs     - If non-NULL, pointer to a Set which will be updated to
 *               contain ids of CBs with outgoing arcs redirected to
 *               peeled header (in accumulating fashion).
 */
void
LB_peel_loop (L_Func *fn, L_Loop *loop, Set peel_cbs, Set region_cbs,
	      int peel_id, int peel_num, Set *peeled_cbs, Set *mod_cbs)
{
  int ncb, *cb_id, i, j, found_entry;
  L_Cb *ocb, *ohdr, *pcb, *phdr = NULL, *scb, *dcb, **copy_cb;
  L_Flow *ofl, *pfl, *sfl, *nfl;
  L_Oper *op;
  Set new_cbs = NULL, loop_cbs = loop->loop_cb;

  ohdr = loop->header;

  if (!(ncb = Set_size (peel_cbs)))
    return;

#ifdef DEBUG_PEEL
  DB_spit_func (fn, "PREPEEL");
  Set_print (ERR, "peel_cbs", peel_cbs);
  Set_print (ERR, "loop_cbs", loop_cbs);
  Set_print (ERR, "region_cbs", region_cbs);
#endif

  copy_cb = alloca (ncb * sizeof (L_Cb *));
  cb_id = alloca (ncb * sizeof (int));

  Set_2array (peel_cbs, cb_id);

  /* Copy block contents for peel_cbs */

  for (i = 0; i < ncb; i++)
    {
      ocb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, cb_id[i]);
      copy_cb[i] = pcb = L_create_cb (ocb->weight);
      new_cbs = Set_add (new_cbs, pcb->id);
      L_insert_cb_after (fn, fn->last_cb, pcb);
      loop->loop_cb = Set_delete (loop->loop_cb, pcb->id);
      L_copy_block_contents (ocb, pcb);

      /* If using acc_omega mode, need to delete sync info
       * from copied ops, reverting to (potentially conservative)
       * acc specs. -- JWS 20041109
       */
      if (L_func_acc_omega)
	{
	  L_Oper *op;
	  for (op = pcb->first_op; op; op = op->next_op)
	    {
	      if (op->sync_info)
		L_delete_all_sync (op, 1);
	    }
	}

      if (ocb == ohdr)
	phdr = pcb;

      LB_attach_peel_attribute (pcb, peel_num, peel_id);

      pcb->flags = L_CLR_BIT_FLAG (ocb->flags, L_CB_HYPERBLOCK_LOOP);
    }

  {
    /* Add newly created CBs to parent loops */

    L_Loop *ploop = loop;

    while ((ploop = ploop->parent_loop))
      ploop->loop_cb = Set_union_acc (ploop->loop_cb, new_cbs);
  }

  /* Redirect flows from region to new peeled header */

  for (ofl = ohdr->src_flow, found_entry = 0; ofl; ofl = nfl)
    {
      nfl = ofl->next_flow;
      
      scb = ofl->src_cb;

      if (Set_in (loop_cbs, scb->id))
	continue;

      if (region_cbs && !Set_in (region_cbs, scb->id))
	continue;

      found_entry = 1;

#if 0
      fprintf (stderr, "redirecting %d->%d to %d\n",
	       scb->id, ohdr->id, phdr->id);
#endif

      if (mod_cbs)
	*mod_cbs = Set_add (*mod_cbs, scb->id);

      sfl = L_find_matching_flow (scb->dest_flow, ofl);

      if ((op = L_find_branch_for_flow (scb, sfl)))
	{
	  L_change_branch_dest (op, ohdr, phdr);
	}
      else
	{
	  op = L_create_new_op (Lop_JUMP);
	  op->src[0] = L_new_cb_operand (phdr);
	  L_insert_oper_after (scb, scb->last_op, op);
	  ofl->cc = 1;
	  sfl->cc = 1;
	}

      ohdr->src_flow = L_remove_flow (ohdr->src_flow, ofl);
      phdr->src_flow = L_concat_flow (phdr->src_flow, ofl);

      ofl->dst_cb = phdr;
      sfl->dst_cb = phdr;
    }

  if (!found_entry)
    L_punt ("LB_peel_loop: peel will have no entry\n");

  /* Connect flows emanating from newly copied blocks */

  for (i = 0; i < ncb; i++)
    {
      ocb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, cb_id[i]);
      pcb = copy_cb[i];

      for (ofl = ocb->dest_flow; ofl; ofl = ofl->next_flow)
	{
	  dcb = ofl->dst_cb;
	  
	  pfl = L_copy_single_flow (ofl);
	  pfl->src_cb = pcb;
	  pcb->dest_flow = L_concat_flow (pcb->dest_flow, pfl);

	  op = L_find_branch_for_flow (pcb, pfl);

	  if (dcb == ohdr)
	    {
	      /* backedge flow... becomes peel exit to remainder loop.
	       * Since branch is copied from preheader loop (if it
	       * exists) it already has the correct dest cb
	       */
	      ;
	    }
	  else if (Set_in (peel_cbs, dcb->id))
	    {
	      /* flow internal to peeled cbs */
	      L_Cb *pdcb = NULL;

	      for (j = 0; j < ncb; j++)
		if (cb_id[j] == dcb->id)
		  {
		    pdcb = copy_cb[j];
		    break;
		  }

	      if (!pdcb)
		L_punt ("LB_peel_loop: mapping to copy failed");

	      if (op)
		L_change_branch_dest (op, dcb, pdcb);

	      pfl->dst_cb = pdcb;
	      dcb = pdcb;
	    }
	  else
	    {
	      /* flow from peeled code to other code */

	      if (Set_in (loop_cbs, dcb->id))
		{ 
		  /* flow from peeled code to loop cb -- this
		   * is an improper entry to the remainder loop
		   */
		}
	    }

	  sfl = L_copy_single_flow (pfl);
	  dcb->src_flow = L_concat_flow (dcb->src_flow, sfl);

	  if (!op)
	    {
	      op = L_create_new_op (Lop_JUMP);
	      op->src[0] = L_new_cb_operand (dcb);
	      L_insert_oper_after (pcb, pcb->last_op, op);
	      pfl->cc = 1;
	      sfl->cc = 1;
	    }
	}
    }

  {
    double invoc_wt, iter_wt, recov_wt, scale;

    LB_adjust_weight_for_peel (fn, loop, &invoc_wt, &iter_wt, &recov_wt);

    scale = (phdr->weight > 0.0) ? invoc_wt / phdr->weight : 1.0;

    for (i = 0; i < ncb; i++)
      {
	L_Flow *dfl, *sfl;

	pcb = copy_cb[i];
	pcb->weight *= scale;

	for (dfl = pcb->dest_flow; dfl; dfl = dfl->next_flow)
	  {
	    sfl = L_find_matching_flow (dfl->dst_cb->src_flow, dfl);
	    
	    dfl->weight *= scale;
	    sfl->weight *= scale;
	  }
      }
  }

#ifdef DEBUG_PEEL
  L_check_func (fn);

  DB_spit_func (fn, "POSTPEEL");
  Set_print (ERR, "new_cbs", new_cbs);
#endif

  if (peeled_cbs)
    *peeled_cbs = Set_union_acc (*peeled_cbs, new_cbs);

  Set_dispose (new_cbs);

  return;
}


/*
 * LB_adjust_weight_for_peel
 * ----------------------------------------------------------------------
 * Adjust block, flow, and iter profile counts in the residue loop after
 * peeling a single iteration.  Writes the number of loop invocations
 * (the profile weight of the peeled iteration) to *pinvoc_wt, and
 * the execution weight of the loop header to *piter_wt.  These results
 * can be used to scale the profile of the peeled body if desired.
 * -- Added 20021009 JWS
 * 
 * Assumes no improper arcs from peeled body into loop (since these
 * flows are likely zero or very small this may make little difference
 * in practice.)
 */
static void 
LB_adjust_weight_for_peel (L_Func *fn, L_Loop *loop, 
			   double *pinvoc_wt, double *piter_wt,
			   double *precov_wt)
{
  L_Cb *hdr, *cb;
  L_Attr *attr, *nattr;
  double invoc_wt = 0.0, iter_wt = 0.0, loop_wt, scale,
    peeled_invoc_wt = 0.0, recov_wt;
  int i, num_cb, *loop_cb, iter_prof_cnt = 0;

  hdr = loop->header;

  for (attr = hdr->attr; attr; attr = nattr)
    {
      nattr = attr->next_attr;
      
      if (!strncmp (attr->name, ITERATION_INFO_PREFIX, 
		    ITERATION_INFO_PREFIX_LENGTH))
	{
	  int iter_indx;
	  double num_iter;
	  char buf[32];

	  iter_indx = atoi (&attr->name[ITERATION_INFO_PREFIX_LENGTH]);
	  num_iter = attr->field[0]->value.f2;

	  if (iter_indx > 1)
	    {
	      sprintf (buf, ITERATION_INFO_PREFIX "%d", iter_indx - 1);
	      attr->name = L_add_string (L_string_table, buf);
	      iter_prof_cnt++;
	    }
	  else
	    {
	      peeled_invoc_wt += num_iter;
	      hdr->attr = L_delete_attr (hdr->attr, attr);
	    }

	  invoc_wt += num_iter;
	  iter_wt += num_iter * iter_indx;
	}
    }

  if ((attr = L_find_attr (hdr->attr, ITERATION_INFO_HEADER)))
    {
      if (iter_prof_cnt)
	L_set_int_attr_field (attr, 0, iter_prof_cnt);
      else
	hdr->attr = L_delete_attr (hdr->attr, attr);
    }

  loop_wt = iter_wt - invoc_wt;

  recov_wt = invoc_wt - peeled_invoc_wt;

  scale = (hdr->weight > 0.0) ? loop_wt / hdr->weight : 1.0;
  
  num_cb = Set_size (loop->loop_cb);
  loop_cb = alloca (num_cb * sizeof (int));
  Set_2array (loop->loop_cb, loop_cb);

  for (i = 0; i < num_cb; i++)
    {
      L_Flow *dfl, *sfl;

      cb = L_cb_hash_tbl_find (fn->cb_hash_tbl, loop_cb[i]);
      cb->weight *= scale;

      for (dfl = cb->dest_flow; dfl; dfl = dfl->next_flow)
	{
	  sfl = L_find_matching_flow (dfl->dst_cb->src_flow, dfl);

	  dfl->weight *= scale;
	  sfl->weight *= scale;
	}
    }

  *pinvoc_wt = invoc_wt;
  *piter_wt = iter_wt;
  *precov_wt = recov_wt;

  return;
}


static void
LB_attach_peel_attribute (L_Cb * cb, int iter_num, int peel_loop_id)
{
  L_Oper *oper;
  L_Attr *attr;

  for (oper = cb->first_op; oper; oper = oper->next_op)
    {
      attr = L_new_attr ("peel", 2);
      L_set_int_attr_field (attr, 0, iter_num);
      L_set_int_attr_field (attr, 1, peel_loop_id);
      oper->attr = L_concat_attr (oper->attr, attr);
    }
}
