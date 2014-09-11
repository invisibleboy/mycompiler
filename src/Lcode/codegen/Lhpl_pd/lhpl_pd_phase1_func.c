/***************************************************************************\
 *
 *  File:  lhpl_pd_phase1_func.c
 *
 *  Description:
 *    Annotate Lcode to machine specific Lcode for the HPL_PD architecture
 *
 *  Creation Date : August 1993
 *
 *  Author:  Scott A. Mahlke
 *
 *  Modified by Sadun Anik : March 1994 (HP Labs)
 *
\************************************************************************/
#include "lhpl_pd_main.h"
#include <Lcode/l_opti_functions.h>

L_Cb *mcb=NULL;


static int function_return_address_holder ;

#define CLEAR_PROBE_MARK_FLAG(oper)  {(oper)->flags = L_CLR_BIT_FLAG((oper)->flags,\
							(L_OPER_PROBE_MARK)); }

/*
 * Operand swapping not necessary, just convenient to put things
 * in a normalized representation with literals always in the 2nd
 * source if possible.
 */
static int L_should_swap_operands(L_Operand *operand1, L_Operand *operand2)
{
    /* if reg/mac and !reg/mac place reg/mac in src2 */
    if ((L_is_reg(operand2) ||
	L_is_macro(operand2)) &&
	!(L_is_reg(operand1) ||
	  L_is_macro(operand1)))
	return (1);
    /* if int and string/label place the int in src2 */
    if ( (L_is_int_constant(operand1) &&
	  ((L_is_label(operand2)) ||
	   (L_is_string(operand2)))))
	return(1);
    /* place the largest integer in src2 */
    else if (L_is_int_constant(operand1) &&
	     L_is_int_constant(operand2) &&
	     operand1->value.i > operand2->value.i )
	return(1);
    else
	return(0);
}


/*===============================================================================*/
/*
 *	Fix hyperblocks in Lcode
 *      Inserts pred clear/sets which are missing
 *      Changes CMP EQ i0 i0 to Pred Sets
 */
/*===============================================================================*/
static void Lhpl_pd_fixup_hyperblocks(L_Func *fn) 
{
    L_Cb *cb;
    L_Oper *oper, *new_oper;
    Set defined_operands = NULL;
    Set cmp_or_operands = NULL;
    Set cmp_and_operands = NULL;

    for (cb=fn->first_cb; cb!=NULL; cb=cb->next_cb) {
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
	{
	  /* Add to defined_operands map if a pred_clear or pred_set */
	  if( L_initializing_pred_define_opcode (oper)) {
	    L_Operand *dst = oper->dest[0];
	    defined_operands = Set_add(defined_operands, dst->value.r);	    
	  }	  
	  
	  /* For any cmp's who's dest pred hasn't been defined, add a define*/
	  if(L_general_pred_comparison_opcode (oper)) {
	    int i;
	    for (i = 0; i < L_max_dest_operand; i++)
	      {
		L_Operand *dst = oper->dest[i];
		if(!dst) continue;
		if(Set_in(defined_operands, dst->value.r)) continue;

		/* Add a PRED_CLEAR for or-type preds*/
		if(L_or_ptype(dst->ptype)) {
		  new_oper = L_create_new_op(Lop_PRED_CLEAR);
		  new_oper->dest[0] = L_copy_operand(dst);
		  new_oper->dest[0]->ptype = L_PTYPE_NULL;
		  new_oper->pred[0] = L_copy_operand(oper->pred[i]);
		  L_insert_oper_before(cb, cb->first_op, new_oper);
		  defined_operands = Set_add(defined_operands, dst->value.r);	    		    
		}
		/* Add a PRED_SET for and-type preds*/
		else if(L_and_ptype(dst->ptype)) {
		  new_oper = L_create_new_op(Lop_PRED_SET);
		  new_oper->dest[0] = L_copy_operand(dst);
		  new_oper->dest[0]->ptype = L_PTYPE_NULL;
		  new_oper->pred[0] = L_copy_operand(oper->pred[i]);
		  L_insert_oper_before(cb, cb->first_op, new_oper);
		  defined_operands = Set_add(defined_operands, dst->value.r);	    		    
		}
		else
		  defined_operands = Set_add(defined_operands, dst->value.r);		  
	      }
	  }

	  /* Change any (cmp eq i0 i0) ops to pred_sets */	  	  
	  /*	    
	  if(L_gen_eq_cmp_opcode(oper) && 
	     oper->src[0]->type == L_OPERAND_IMMED && 
	     oper->src[1]->type == L_OPERAND_IMMED &&
	     oper->src[0]->value.i == 0 && 
	     oper->src[1]->value.i == 0 && 
	     oper->dest[0]->ptype == L_PTYPE_UNCOND_T) {	    
	      L_Operand *dst = L_copy_operand(oper->dest[0]);	      
	      dst->ptype = L_PTYPE_NULL;
	      L_Operand *pred = L_copy_operand(oper->pred[0]);	      
	      
	      L_nullify_operation(oper);
	      L_change_opcode(oper, Lop_PRED_SET);
	      oper->com[0] = 0;
	      oper->com[1] = 0;
	      oper->dest[0] = dst;	      
	      oper->pred[0] = pred;
	      fprintf(stderr, "Replaced PRED_SET for op: %d\n", oper->id);
	      defined_operands = Set_add(defined_operands, dst->value.r);
	  }
	  */	    	  
	}      
      defined_operands = Set_dispose(defined_operands);
      cmp_or_operands = Set_dispose(cmp_or_operands);
      cmp_and_operands = Set_dispose(cmp_and_operands);           
    }
}

/*===============================================================================*/
/*
 *	Build sync arcs from Lcode memory disambiguation
 */
/*===============================================================================*/
void L_build_sync_arcs(L_Func *fn)
{
    L_Attr *attr_dep_pragmas, *attr_jsr_dep_pragmas, *attr_new_jsr_dep_pragmas;

    /* Check if any sync arcs are already present in the code */

    attr_dep_pragmas = L_find_attr(fn->attr, "DEP_PRAGMAS");
    attr_jsr_dep_pragmas = L_find_attr(fn->attr, "JSR_DEP_PRAGMAS");
    attr_new_jsr_dep_pragmas = L_find_attr(fn->attr, "NEW_JSR_DEP_PRAGMAS");

    if(!(attr_dep_pragmas || attr_jsr_dep_pragmas || attr_new_jsr_dep_pragmas)) {
       L_build_sync_arcs_from_lcode_disamb(fn,
			L_SYNC_TYPE_LS_GLOBAL|L_SYNC_TYPE_JSR_GLOBAL, 1);
    }
}

/*===============================================================================*/
/*
 *	Breaking up basic block loops that have a jump for the ft path
 */
/*===============================================================================*/

static void L_breakup_single_bb_loops(L_Func *fn)
{
    L_Cb *cb, *new_cb, *target_cb;
    L_Flow *flow, *jump_flow, *ft_flow=NULL;
    L_Oper *jump, *cbr;
    int has_ft;
    double weight;

    for (cb=fn->first_cb; cb!=NULL; cb=cb->next_cb) {
	/* detect a single block loop which has a jump after the backedge
	   instead of a fallthru path */
	jump = cb->last_op;
	if (! L_uncond_branch_opcode(jump))
	    continue;
	cbr = jump->prev_op;
	if (! L_cond_branch_opcode(cbr))
	    continue;
	if (L_find_branch_dest(cbr)!=cb)
	    continue;
	if (! L_no_br_between(cb->first_op, cbr))
	    continue;

	/* Pattern detected, break up the cb into 2 */

	jump_flow = L_find_flow_for_branch(cb, jump);
	weight = jump_flow->weight;
	if (jump_flow->next_flow == NULL) {
	    has_ft = 0;
	}
	else {
	    has_ft = 1;
	    ft_flow = jump_flow->next_flow;
	    weight += ft_flow->weight;
	    if (ft_flow->next_flow != NULL)
		L_punt("L_breakup_single_bb_loops: cb %d has corrupt dest flows",
				cb->id);
	}

	/* create the new cb */
	new_cb = L_create_cb(weight);
	L_insert_cb_after(fn, cb, new_cb);
	cb->flags = L_CLR_BIT_FLAG(cb->flags, L_CB_HYPERBLOCK_NO_FALLTHRU);
	if (has_ft) {
	    if (! L_is_predicated(jump))
		L_punt("L_breakup_single_bb_loops: cb %d has ft path that cannot occur",
				cb->id);
	    new_cb->flags = L_SET_BIT_FLAG(new_cb->flags, L_CB_HYPERBLOCK);
	}

	/* move jump from cb to new_cb */
	L_remove_oper(cb, jump);
	L_insert_oper_after(new_cb, NULL, jump);

	/* Remove jump_flow and ft_flow from cb */
	cb->dest_flow = L_remove_flow(cb->dest_flow, jump_flow);
	if (has_ft)
	    cb->dest_flow = L_remove_flow(cb->dest_flow, ft_flow);

	/* Add fallthru flow from cb to new_cb */
	flow = L_new_flow(0, cb, new_cb, weight);
	cb->dest_flow = L_concat_flow(cb->dest_flow, flow);

	/* Adjust src flow for target of jump_flow */
	target_cb = jump_flow->dst_cb;
	flow = L_find_matching_flow(target_cb->src_flow, jump_flow);
	flow->cc = 1;
	flow->src_cb = new_cb;

	/* Adjust src flow for the target of ft_flow */
	if (has_ft) {
	    target_cb = ft_flow->dst_cb;
	    flow = L_find_matching_flow(target_cb->src_flow, ft_flow);
	    flow->cc = 0;
	    flow->src_cb = new_cb;
	}

	/* Setup dest/src flow arcs for new_cb */
	flow = L_new_flow(0, cb, new_cb, weight);
	new_cb->src_flow = L_concat_flow(new_cb->src_flow, flow);
	jump_flow->src_cb = new_cb;
	jump_flow->cc = 1;
	new_cb->dest_flow = L_concat_flow(new_cb->dest_flow, jump_flow);
	if (has_ft) {
	    ft_flow->src_cb = new_cb;
	    ft_flow->cc = 0;
	    new_cb->dest_flow = L_concat_flow(new_cb->dest_flow, ft_flow);
	}
    }
}

static void Lplaydoh_breakup_cb(L_Func * fn, L_Cb * cb, Set *new_cb_set)
{
  L_Oper *oper, *branch, *ptr;
  L_Flow *flow, *new_flow;
  L_Cb *new_cb;

  for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
    {
      if (!L_general_branch_opcode(oper))
        continue;
      branch = oper;
      flow = cb->dest_flow;
#if 0
	SAM- In Elcor, only 1 branch per BB, so this special case not allowed,
	although in Lcode this is fine.
      if (L_uncond_branch_opcode(branch->next_op))
        {
          branch = oper->next_op;
          flow = flow->next_flow;
        }
#endif
      /* end condition */
      if (branch->next_op == NULL)
        return;

      if (L_uncond_branch_opcode(oper))
        L_punt("Lhyper_breakup_cb: something is wrong here, jump in middle of cb");

      /* create new cb */
      new_cb = L_create_cb(branch->next_op->weight);
      L_insert_cb_after(fn, cb, new_cb);
      *new_cb_set = Set_add(*new_cb_set, new_cb->id);

      /* breakup the oper 2-way linked list */
      new_cb->first_op = branch->next_op;
      new_cb->last_op = cb->last_op;
      cb->last_op = branch;
      branch->next_op->prev_op = NULL;
      branch->next_op = NULL;

      /* Need to update the oper hash table with these changes!!- SAM 8-96 */
      for (ptr=new_cb->first_op; ptr!=NULL; ptr=ptr->next_op) {
        L_oper_hash_tbl_update_cb(fn->oper_hash_tbl, ptr->id, new_cb);
      }

      /* breakup the flow 1-way linked list */
      new_cb->dest_flow = flow->next_flow;
      flow->next_flow->prev_flow = NULL;
      flow->next_flow = NULL;

      /* create new fallthru flow for cb */
      new_flow = L_new_flow(0, cb, new_cb, new_cb->weight);
      cb->dest_flow = L_concat_flow(cb->dest_flow, new_flow);

      /* change src cb of all dest flows of new_cb */
      L_change_src(new_cb->dest_flow, cb, new_cb);

      Lplaydoh_breakup_cb(fn, new_cb, new_cb_set);

    }
}

static void Lplaydoh_convert_to_strict_basic_block_code (L_Func * fn)
{
  int flag, i, *buf, num_cb, is_sb_hb;
  L_Cb *cb, *next_cb;
  L_Oper *oper, *next_op;
  Set new_cb_set;
  L_Attr *attr;

  flag = 0;
  L_compute_oper_weight(fn, 0, 1);

  for (cb = fn->first_cb; cb != NULL; cb = next_cb)
    {
      next_cb = cb->next_cb;
      for (oper = cb->first_op; oper != NULL; oper = next_op)
        {
          next_op = oper->next_op;
          if (!L_general_branch_opcode(oper))
            continue;

          /* single branch in block is ok */
          if (oper->next_op == NULL)
            break;

          /* need to break up cb */
#ifdef DEBUG_BREAKUP
          fprintf(ERR, "Breakup cb %d\n", cb->id);
#endif
          if (!flag)
            {
              flag = 1;
              L_clear_src_flow(fn);
            }

          is_sb_hb = 0;
	  if (L_EXTRACT_BIT_VAL(cb->flags, L_CB_SUPERBLOCK) ||
	      L_EXTRACT_BIT_VAL(cb->flags, L_CB_HYPERBLOCK)) {
            is_sb_hb = 1;
            cb->flags = L_CLR_BIT_FLAG(cb->flags, L_CB_SUPERBLOCK);
	  }

	  new_cb_set = NULL;
          Lplaydoh_breakup_cb(fn, cb, &new_cb_set);

	  if (new_cb_set==NULL)
            L_punt("Lplaydoh_convert_to_strict_basic_block_code: no new cbs made!");

          /* install attribute */
          if (is_sb_hb) {
	    num_cb = Set_size(new_cb_set);
	    buf = (int *) Lcode_malloc(sizeof(int)*num_cb);
	    Set_2array(new_cb_set, buf);
	    attr = L_new_attr("IMPACT_reg", 1);
	    L_set_int_attr_field(attr, 0, cb->id);
	    cb->attr = L_concat_attr(cb->attr, attr);
	    for (i=0; i<num_cb; i++) {
              L_set_int_attr_field(attr, i+1, buf[i]);
            }
	    Lcode_free(buf);
          }
          Set_dispose(new_cb_set);
        }
    }

  if (flag)
    L_rebuild_src_flow(fn);
}


/*===============================================================================*/
/*
 *	Normalize flow arcs for jump rgs (mainly a debugging assist)
 */
/*===============================================================================*/

void L_normalize_flows_for_jrg(L_Cb *cb, L_Oper *jrg)
{
    int i, j, num_jrg_flow, found;
    L_Flow *flow, *ptr, *next_flow, **buf, *prev_flow, *tmp, *fallthru_flow;

    if (! L_register_branch_opcode(jrg))
	L_punt("L_normalize_flows_for_jrg: op %d is not a jrg", jrg->id);

    /*
     *	Break down the dest flows into 2 groups [start..prev_flow] = non_jrg flows
     *	and [flow..end] = jrg flows
     */
    flow = L_find_flow_for_branch(cb, jrg);
    prev_flow = NULL;
    found = 0;
    num_jrg_flow = 0;
    for (ptr=cb->dest_flow; ptr!=NULL; ptr=ptr->next_flow) {
	if (ptr==flow)
	    found = 1;
	if (! found)
	    prev_flow = ptr;
	else
	    num_jrg_flow++;
    }

    /* Disconnect jrg flows and insert them into the buf array */
    if (prev_flow != NULL)
	prev_flow->next_flow = NULL;
    else
	cb->dest_flow = NULL;

    buf = (L_Flow **) Lcode_malloc(sizeof(L_Flow *)*num_jrg_flow);
    i = 0;
    for (ptr=flow; ptr!=NULL; ptr=next_flow) {
	next_flow = ptr->next_flow;
	buf[i++] = ptr;
        ptr->prev_flow = NULL;
        ptr->next_flow = NULL;
    }

    /* if a fallthru flow exists, pull it out of buf */
    fallthru_flow = NULL;
    if (buf[num_jrg_flow-1]->cc == L_JUMPTBL_DEFAULT_CC) {
	fallthru_flow = buf[num_jrg_flow-1];
	num_jrg_flow--;
    }
    
    /* Sort flows using 2 keys: 1. highest to lowest weight, 2. lowest to
       highest cc */
    for (i=0; i<num_jrg_flow; i++) {
        for (j=i; j<num_jrg_flow; j++) {
            if (buf[j]->weight > buf[i]->weight) {
                tmp = buf[i];
                buf[i] = buf[j];
                buf[j] = tmp;
            }
	    else if ((buf[j]->weight == buf[i]->weight) &&
	             (buf[j]->cc < buf[i]->cc)) {
                tmp = buf[i];
                buf[i] = buf[j];
                buf[j] = tmp;
	    }
        }
    }

    for (i=0; i<num_jrg_flow; i++) {
        cb->dest_flow = L_concat_flow(cb->dest_flow, buf[i]);
    }

    if (fallthru_flow != NULL)
	cb->dest_flow = L_concat_flow(cb->dest_flow, fallthru_flow);

    Lcode_free(buf);
}

/*
 *	Currently only jrg's have a normalization function
 */
void L_normalize_flows(L_Func *fn)
{
    L_Cb *cb;
    L_Oper *oper;

    for (cb=fn->first_cb; cb!=NULL; cb=cb->next_cb) {
	for (oper=cb->first_op; oper!=NULL; oper=oper->next_op) {
	    if (L_register_branch_opcode(oper))
		L_normalize_flows_for_jrg(cb, oper);
	}
    }
}

/*===============================================================================*/
/*
 *	Epilog block insertion
 */
/*===============================================================================*/

void Lhpl_pd_insert_epilog_block(L_Func *fn)
{
  L_Cb *cb, *new_cb;
  L_Oper *new_op;
  char found = 0;

  /* First figure out if the function already has one, we can search through
     all the cbs for the L_CB_EPILOGUE flag */
  for (cb=fn->first_cb; cb!=NULL; cb=cb->next_cb) {
    if (L_EXTRACT_BIT_VAL(cb->flags, L_CB_EPILOGUE)) {
      found = 1;
      break;
    }
  }
  if (found)	/* Don't do anything if CB already has epilog block */
    return;

  /* So, now we have a function that doesn't have an epilog block.  This means
     Impact deleted it as it was unreachable.  So, we will just stick in a bogus
     epilog block at the end of the procedure that is unreachable.  But, this
     will satisfy elcor analysis that wants this block. */

  new_cb = L_create_cb(0.0);
  new_cb->flags = L_SET_BIT_FLAG(new_cb->flags, L_CB_EPILOGUE);
  L_insert_cb_after (fn, fn->last_cb, new_cb);

  /* Create epilog and rts instructions */
  new_op = L_create_new_op(Lop_EPILOGUE);
  L_insert_oper_after (new_cb, new_cb->last_op, new_op);
  new_op = L_create_new_op(Lop_RTS);
  L_insert_oper_after (new_cb, new_cb->last_op, new_op);

}


/*===============================================================================*/
/*
 *	Clearing unnecessary flags/attributes
 */
/*===============================================================================*/

static void L_remove_unnec_flags_from_pbr(L_Oper *pbr)
{
    pbr->flags = L_CLR_BIT_FLAG(pbr->flags, L_OPER_SIDE_EFFECT_FREE |
					    L_OPER_SYNC);
}


static void L_remove_unnec_attr_from_pbr(L_Oper *pbr)
{
    L_Attr *attr;

    attr = L_find_attr(pbr->attr, "tr");
    if (attr)
	pbr->attr = L_delete_attr(pbr->attr, attr);
    attr = L_find_attr(pbr->attr, "tm");
    if (attr)
	pbr->attr = L_delete_attr(pbr->attr, attr);
    attr = L_find_attr(pbr->attr, "ret");
    if (attr)
	pbr->attr = L_delete_attr(pbr->attr, attr);
    attr = L_find_attr(pbr->attr, "ret_ptr_st");
    if (attr)
	pbr->attr = L_delete_attr(pbr->attr, attr);
    attr = L_find_attr(pbr->attr, "param_size");
    if (attr)
	pbr->attr = L_delete_attr(pbr->attr, attr);
    attr = L_find_attr(pbr->attr, "NL_inner");
    if (attr)
	pbr->attr = L_delete_attr(pbr->attr, attr);
    attr = L_find_attr(pbr->attr, "NL_outer");
    if (attr)
	pbr->attr = L_delete_attr(pbr->attr, attr);
    attr = L_find_attr(pbr->attr, "NL_stln");
    if (attr)
	pbr->attr = L_delete_attr(pbr->attr, attr);
    attr = L_find_attr(pbr->attr, "LB_inner");
    if (attr)
	pbr->attr = L_delete_attr(pbr->attr, attr);
    attr = L_find_attr(pbr->attr, "LE_inner");
    if (attr)
	pbr->attr = L_delete_attr(pbr->attr, attr);
    attr = L_find_attr(pbr->attr, "LB_outer");
    if (attr)
	pbr->attr = L_delete_attr(pbr->attr, attr);
    attr = L_find_attr(pbr->attr, "LE_outer");
    if (attr)
	pbr->attr = L_delete_attr(pbr->attr, attr);
    attr = L_find_attr(pbr->attr, "ptr");
    if (attr)
	pbr->attr = L_delete_attr(pbr->attr, attr);
    attr = L_find_attr(pbr->attr, "loop_nest");
    if (attr)
	pbr->attr = L_delete_attr(pbr->attr, attr);
    attr = L_find_attr(pbr->attr, L_JUMPTBL_OP_ATTR);
    if (attr)
	pbr->attr = L_delete_attr(pbr->attr, attr);

}

static void L_remove_unnec_flags_from_addr_calc(L_Oper *addr)
{
    addr->flags = L_CLR_BIT_FLAG(addr->flags,
					L_OPER_LABEL_REFERENCE|
					L_OPER_SAFE_PEI|
					L_OPER_MASK_PE|
					L_OPER_DATA_SPECULATIVE);
}

static void L_remove_unnec_attr_from_addr_calc(L_Oper *addr)
{
    L_Attr *attr;

    attr = L_find_attr(addr->attr, "label");
    if (attr)
        addr->attr = L_delete_attr(addr->attr, attr);
    attr = L_find_attr(addr->attr, "param");
    if (attr)
        addr->attr = L_delete_attr(addr->attr, attr);
    attr = L_find_attr(addr->attr, "dmr");
    if (attr)
        addr->attr = L_delete_attr(addr->attr, attr);
    attr = L_find_attr(addr->attr, "dmc");
    if (attr)
        addr->attr = L_delete_attr(addr->attr, attr);
    attr = L_find_attr(addr->attr, "drr");
    if (attr)
        addr->attr = L_delete_attr(addr->attr, attr);
    attr = L_find_attr(addr->attr, "srr");
    if (attr)
        addr->attr = L_delete_attr(addr->attr, attr);
    attr = L_find_attr(addr->attr, "trr");
    if (attr)
        addr->attr = L_delete_attr(addr->attr, attr);
    attr = L_find_attr(addr->attr, "tm");
    if (attr)
        addr->attr = L_delete_attr(addr->attr, attr);
    attr = L_find_attr(addr->attr, "exec_freq");
    if (attr)
        addr->attr = L_delete_attr(addr->attr, attr);
    while ((attr = L_find_attr_prefix(addr->attr, "dep", 0)) != NULL) {
        addr->attr = L_delete_attr(addr->attr, attr);
    }
}

/*===============================================================================*/
/*
 *	Instruction annotation
 */
/*===============================================================================*/


/* KVM */
static int L_pred_compare_for_branch(L_Oper *oper)
{
    if (oper==NULL)
	L_punt("L_compare_for_branch: oper is NULL");

    if(L_int_beq_branch_opcode(oper)) {
      return Lop_CMP;
    }
    else if(L_int_bne_branch_opcode(oper)) {
      return Lop_CMP;
    }
    else if(L_int_bge_branch_opcode(oper)) {
      if(L_unsigned_int_cond_branch_opcode(oper)) 
        return Lop_CMP;
      else
        return Lop_CMP;
    }
    else if(L_int_bgt_branch_opcode(oper)) {
      if(L_unsigned_int_cond_branch_opcode(oper)) 
        return Lop_CMP;
      else
        return Lop_CMP;
    }
    else if(L_int_ble_branch_opcode(oper)) {
      if(L_unsigned_int_cond_branch_opcode(oper)) 
        return Lop_CMP;
      else
        return Lop_CMP;
    }
    else if(L_int_blt_branch_opcode(oper)) {
      if(L_unsigned_int_cond_branch_opcode(oper)) 
        return Lop_CMP;
      else
        return Lop_CMP;
    }
    else if(L_flt_beq_branch_opcode(oper)) {
      return Lop_CMP_F;
    }
    else if(L_flt_bne_branch_opcode(oper)) {
      return Lop_CMP_F;
    }
    else if(L_flt_bge_branch_opcode(oper)) {
      return Lop_CMP_F;
    }
    else if(L_flt_bgt_branch_opcode(oper)) {
      return Lop_CMP_F;
    }
    else if(L_flt_ble_branch_opcode(oper)) {
      return Lop_CMP_F;
    }
    else if(L_flt_blt_branch_opcode(oper)) {
      return Lop_CMP_F;
    }
    else if(L_dbl_beq_branch_opcode(oper)) {
      return Lop_CMP_F;
    }
    else if(L_dbl_bne_branch_opcode(oper)) {
      return Lop_CMP_F;
    }
    else if(L_dbl_bge_branch_opcode(oper)) {
      return Lop_CMP_F;
    }
    else if(L_dbl_bgt_branch_opcode(oper)) {
      return Lop_CMP_F;
    }
    else if(L_dbl_ble_branch_opcode(oper)) {
      return Lop_CMP_F;
    }
    else if(L_dbl_blt_branch_opcode(oper)) {
      return Lop_CMP_F;
    }
    else {
      L_punt("L_pred_compare_for_branch: illegal opcode");
      return (0);
    }
}

static void L_annotate_prologue(L_Oper *oper)
{
    L_Oper *new_oper, *new_def ;
    
    new_def = L_create_new_op_using(Lop_DEFINE,oper) ;
    CLEAR_PROBE_MARK_FLAG(new_def);
    new_def->dest[0] = L_new_macro_operand(PLAYDOH_MAC_RETADDR, L_CTYPE_BTR, L_PTYPE_NULL) ;
    L_insert_oper_before(mcb,oper,new_def) ;
 
    L_insert_oper_before(mcb,oper,L_copy_parent_oper(oper));
    new_oper = L_create_new_op_using(Lop_MOV,oper) ;
    CLEAR_PROBE_MARK_FLAG(new_oper);
    new_oper->src[0] = L_new_macro_operand(PLAYDOH_MAC_RETADDR, L_CTYPE_BTR, L_PTYPE_NULL) ;
    new_oper->dest[0] = L_new_register_operand(++L_fn->max_reg_id,L_CTYPE_BTR, L_PTYPE_NULL);
    function_return_address_holder = L_fn->max_reg_id ;
    
    L_insert_oper_before(mcb, oper, new_oper);
}

static void L_annotate_jsr(L_Oper *oper)
{
    L_Oper *new_oper1, *new_oper2;
    
    /* create PBR */
    new_oper1 = L_create_new_op_using(Lop_PBR, oper);
    CLEAR_PROBE_MARK_FLAG(new_oper1);

    L_remove_unnec_flags_from_pbr(new_oper1);
    L_remove_unnec_attr_from_pbr(new_oper1);

    new_oper1->dest[0] = L_new_register_operand(++L_fn->max_reg_id, L_CTYPE_BTR,
							L_PTYPE_NULL);
    new_oper1->src[0] = L_copy_operand(oper->src[0]);
    new_oper1->src[1] = L_new_gen_int_operand(L_PREDICT_TAKEN);    
    new_oper1->proc_opc = PLAYDOHop_PBRR;
    
    new_oper2 = L_create_new_op_using(Lop_JSR,oper) ;
    new_oper2->proc_opc = PLAYDOHop_BRL;
    new_oper2->src[0] = L_copy_operand(oper->src[0]);
    new_oper2->src[1] = L_copy_operand(new_oper1->dest[0]) ;
    new_oper2->dest[0] = L_new_macro_operand(PLAYDOH_MAC_RETADDR,L_CTYPE_BTR, L_PTYPE_NULL);

#if 1
    /* copy sync arcs - SAM 8-95 */
    if (oper->sync_info != NULL) {
	new_oper2->sync_info = L_copy_sync_info(oper->sync_info);
        L_add_to_child_list (oper, new_oper2);
    }
#endif

    if(oper->acc_info)
      new_oper2->acc_info = L_copy_mem_acc_spec_list (oper->acc_info);
    
    L_insert_oper_before(mcb,oper,new_oper1);
    L_insert_oper_before(mcb,oper,new_oper2);
}

static void L_annotate_rts(L_Oper *oper)
{
    L_Oper *new_oper1, *new_oper2;
    
    /* create PBR */
    new_oper1 = L_create_new_op_using(Lop_PBR, oper);
    CLEAR_PROBE_MARK_FLAG(new_oper1);

    L_remove_unnec_flags_from_pbr(new_oper1);
    L_remove_unnec_attr_from_pbr(new_oper1);

    new_oper1->dest[0] = L_new_macro_operand(PLAYDOH_MAC_RETADDR, L_CTYPE_BTR, L_PTYPE_NULL);
    new_oper1->src[0] = L_new_register_operand(function_return_address_holder, 
						L_CTYPE_BTR, L_PTYPE_NULL);
    new_oper1->src[1] = L_new_gen_int_operand(L_PREDICT_TAKEN);    
    new_oper1->proc_opc = PLAYDOHop_PBRA;
    
    new_oper2 = L_create_new_op_using(Lop_RTS, oper);
    new_oper2->src[0] = L_new_macro_operand(PLAYDOH_MAC_RETADDR, L_CTYPE_BTR, L_PTYPE_NULL);
    
    if ( oper->prev_op == NULL || oper->prev_op->opc != Lop_EPILOGUE )  {
	L_punt("L_annotate_rts: Oper before rts is supposed to be Lop_EPILOGUE\n");
    }
    L_insert_oper_before(mcb,oper->prev_op,new_oper1);
    L_insert_oper_before(mcb,oper,new_oper2);
}

static void L_annotate_jump(L_Oper *oper, int opc)
{
    L_Oper *new_oper1, *new_oper2;

    /* create PBR */
    new_oper1 = L_create_new_op_using(Lop_PBR, oper);
    CLEAR_PROBE_MARK_FLAG(new_oper1);

    L_remove_unnec_flags_from_pbr(new_oper1);
    L_remove_unnec_attr_from_pbr(new_oper1);

    new_oper1->dest[0] = L_new_register_operand(++L_fn->max_reg_id, L_CTYPE_BTR,
							L_PTYPE_NULL);
    new_oper1->src[0] = L_copy_operand(oper->src[0]);
    new_oper1->src[1] = L_new_gen_int_operand(L_PREDICT_TAKEN);
    new_oper1->proc_opc = PLAYDOHop_PBRR;
    
    /* create branch */
    if (L_register_branch_opcode(oper))
        new_oper2 = L_create_new_op_using(Lop_JUMP_RG, oper);
    else
        new_oper2 = L_create_new_op_using(Lop_JUMP, oper);
    new_oper2->proc_opc = PLAYDOHop_BRU;
    new_oper2->src[0] = L_copy_operand(oper->src[0]);
    new_oper2->src[1] = L_copy_operand(new_oper1->dest[0]);

    /* insert new ops */
    L_insert_oper_before(mcb, oper, new_oper1);
    L_insert_oper_before(mcb, oper, new_oper2);
}

static void L_annotate_cond_br(L_Oper *oper)
{
    L_Oper *new_oper1, *new_oper2, *new_oper3;

    /* create PBR */
    new_oper1 = L_create_new_op_using(Lop_PBR, oper);
    new_oper1->com[0] = 0; /* Clear spurious completers */
    new_oper1->com[1] = 0;

    CLEAR_PROBE_MARK_FLAG(new_oper1);

    L_remove_unnec_flags_from_pbr(new_oper1);
    L_remove_unnec_attr_from_pbr(new_oper1);

    new_oper1->dest[0] = L_new_register_operand(++L_fn->max_reg_id, L_CTYPE_BTR,
							L_PTYPE_NULL);
    new_oper1->src[0] = L_copy_operand(oper->src[2]);
    /* if (L_fs_cond_branch_opcode(oper)) KVM */
    new_oper1->src[1] = L_new_gen_int_operand(L_PREDICT_NOTTAKEN);
    new_oper1->proc_opc = PLAYDOHop_PBRR;
    
    /* create compare */
    new_oper2 = L_create_new_op_using(L_pred_compare_for_branch(oper), oper);
    CLEAR_PROBE_MARK_FLAG(new_oper2);

    L_remove_unnec_flags_from_pbr(new_oper2);
    L_remove_unnec_attr_from_pbr(new_oper2);

    new_oper2->dest[0] = L_new_register_operand(++L_fn->max_reg_id,
						L_CTYPE_PREDICATE, L_PTYPE_UNCOND_T);
    
    new_oper2->src[0] = L_copy_operand(oper->src[0]);
    new_oper2->src[1] = L_copy_operand(oper->src[1]);
    new_oper2->com[0] = oper->com[0];
    new_oper2->com[1] = oper->com[1];

    /* create branch */
    new_oper3 = L_create_new_op_using(Lop_BR, oper);
    new_oper3->com[0] = L_CTYPE_INT;
    new_oper3->com[1] = Lcmp_COM_EQ;
    new_oper3->proc_opc = PLAYDOHop_BRCT;
    new_oper3->src[0] = L_copy_operand(new_oper2->dest[0]);
    new_oper3->src[0]->ptype = L_PTYPE_NULL;
    new_oper3->src[1] = L_new_gen_int_operand(1);
    new_oper3->src[2] = L_copy_operand(oper->src[2]);
    new_oper3->src[3] = L_copy_operand(new_oper1->dest[0]);

    /* insert new ops */
    L_insert_oper_before(mcb, oper, new_oper1);
    L_insert_oper_before(mcb, oper, new_oper2);
    L_insert_oper_before(mcb, oper, new_oper3);

}

static void L_annotate_pbr(L_Oper *oper)
{
    L_Oper *new_oper1;

    new_oper1 = L_copy_parent_oper(oper);
    new_oper1->proc_opc = PLAYDOHop_PBRR;
    L_insert_oper_before(mcb, oper, new_oper1);
}

static void L_annotate_int_move(L_Oper *oper)
{
    L_Oper *new_oper1;

    new_oper1 = L_copy_parent_oper(oper);

    if (L_is_register(oper->src[0]) &&
	L_is_ctype_predicate(oper->src[0])) {
	new_oper1->proc_opc = PLAYDOHop_MOVEPG;
    }
    
    L_insert_oper_before(mcb, oper, new_oper1);
}

static L_Oper* L_annotate_base_disp_ld(L_Oper *oper)
{
    int load_opc, load_popc, load_base_popc, chs_src, chs_dest;
    L_Oper *new_oper2;
    L_Attr *attr;

    /*
     *	Determine the opc and base proc_opc
     */
    switch (oper->opc)  {
	case Lop_LD_UC:
	    load_opc = Lop_LD_UC;
            load_base_popc = PLAYDOHop_LG_B_V1_V1;
            break;
	case Lop_LD_C:
	  if(Lplaydoh_use_sign_ext) {
	    load_opc = Lop_LD_C;
	    load_base_popc = PLAYDOHop_LGX_B_V1_V1;
          }
          else {
            load_opc = Lop_LD_UC;
	    load_base_popc = PLAYDOHop_LG_B_V1_V1;
          }
	    break;
	case Lop_LD_UC2:
            load_opc = Lop_LD_UC2;
	    load_base_popc = PLAYDOHop_LG_H_V1_V1;
            break;
	case Lop_LD_C2:
	  if(Lplaydoh_use_sign_ext) {
	    load_opc = Lop_LD_C2;
	    load_base_popc = PLAYDOHop_LGX_H_V1_V1;
	  }
          else {
            load_opc = Lop_LD_UC2;
	    load_base_popc = PLAYDOHop_LG_H_V1_V1;
          }
	    break;
	case Lop_LD_I:
	    load_opc = Lop_LD_I;
	    load_base_popc = PLAYDOHop_LG_W_V1_V1;
	    break;
	case Lop_LD_F:
	    load_opc = Lop_LD_F;
	    load_base_popc = PLAYDOHop_FLG_S_V1_V1;
	    break;
	case Lop_LD_F2:
	    load_opc = Lop_LD_F2;
	    load_base_popc = PLAYDOHop_FLG_D_V1_V1;
	    break;
        /* Just punt for now */
	case Lop_LD_POST_UC:
	case Lop_LD_POST_C:
	case Lop_LD_POST_UC2:
	case Lop_LD_POST_C2:
	case Lop_LD_POST_I:
	case Lop_LD_POST_F:
	case Lop_LD_POST_F2:
	default:
	    L_punt("L_annotate_ld: unknown load opcode for op %d", oper->id);
	    break;
    }

    /*
     *	Now determine the real proc_opc based on the src and dest target
     *	cache specifiers (if the exist, (c1,c1) is the default if nothing
     *	is specified)
     */
    chs_src = LPLAYDOH_CACHE_SPECIFIER_C1;
    chs_dest = LPLAYDOH_CACHE_SPECIFIER_C1;
    attr=L_find_attr(oper->attr, LPLAYDOH_CACHE_SPECIFIER_NAME);
    if (attr!=NULL) {
	chs_src = attr->field[1]->value.i;
	chs_dest = attr->field[0]->value.i;
    }
    /* update the proc_opc using some simple arithmetic, note this assumes
       sequentially numbering of the load opcodes is maintained */
    load_popc = load_base_popc + (4*chs_src) + chs_dest;
    
    new_oper2 = L_create_new_op_using(load_opc,oper);
    new_oper2->proc_opc = load_popc;
    new_oper2->src[0] = L_copy_operand(oper->src[0]);
    new_oper2->src[1] = L_copy_operand(oper->src[1]);
    new_oper2->src[2] = L_copy_operand(oper->src[2]);
    new_oper2->dest[0] = L_copy_operand(oper->dest[0]);
    new_oper2->dest[1] = L_copy_operand(oper->dest[1]);

    return new_oper2;
}

static L_Oper* L_annotate_plain_ld(L_Oper *oper)
{
    int load_opc, load_popc, load_base_popc, chs_src, chs_dest;
    L_Oper *new_oper1=NULL, *new_oper2=NULL;
    L_Operand *load_src=NULL;
    L_Attr *attr=NULL;

    int vector = 0;

    if (L_is_int_zero(oper->src[1]) &&
	(L_is_register(oper->src[0]) || L_is_macro(oper->src[0]))) {
	load_src = oper->src[0];
    } 
    
    else {
        /* create address calculation */
	new_oper1 = L_create_new_op_using(Lop_ADD,oper) ;
	CLEAR_PROBE_MARK_FLAG(new_oper1);

	L_remove_unnec_flags_from_addr_calc(new_oper1);
	L_remove_unnec_attr_from_addr_calc(new_oper1);

	new_oper1->dest[0] = L_new_register_operand(++L_fn->max_reg_id, L_CTYPE_INT, 
						    L_PTYPE_NULL);
	new_oper1->src[0] = L_copy_operand(oper->src[0]) ;
	new_oper1->src[1] = L_copy_operand(oper->src[1]) ;
	load_src = new_oper1->dest[0];

	L_insert_oper_before(mcb,oper,new_oper1);
    }

    /*
     *	Determine the opc and base proc_opc
     */
    switch (oper->opc)  {
	case Lop_LD_UC:
            load_opc = Lop_LD_UC;
	    load_base_popc = PLAYDOHop_L_B_V1_V1;
	    break;
	case Lop_LD_C:
	  if(Lplaydoh_use_sign_ext) {
	    load_opc = Lop_LD_C;
	    load_base_popc = PLAYDOHop_LX_B_V1_V1;
          }
          else {
            load_opc = Lop_LD_UC;
	    load_base_popc = PLAYDOHop_L_B_V1_V1;
          }
	    break;
	case Lop_LD_UC2:
            load_opc = Lop_LD_UC2;
	    load_base_popc = PLAYDOHop_L_H_V1_V1;
	    break;
	case Lop_LD_C2:
	  if(Lplaydoh_use_sign_ext) {
	    load_opc = Lop_LD_C2;
	    load_base_popc = PLAYDOHop_LX_H_V1_V1;
          }
          else {
            load_opc = Lop_LD_UC2;
	    load_base_popc = PLAYDOHop_L_H_V1_V1;
          }
	    break;
	case Lop_LD_I:
	    load_opc = Lop_LD_I;
	    load_base_popc = PLAYDOHop_L_W_V1_V1;
	    break;
	case Lop_LD_F:
	    load_opc = Lop_LD_F;
	    load_base_popc = PLAYDOHop_FL_S_V1_V1;
	    break;
	case Lop_LD_F2:
	    load_opc = Lop_LD_F2;
	    load_base_popc = PLAYDOHop_FL_D_V1_V1;
	    break;
	case Lop_LD_POST_UC:
	case Lop_LD_POST_C:
	    load_opc = Lop_LD_POST_UC;
	    load_base_popc = PLAYDOHop_LI_B_V1_V1;
	    break;
	case Lop_LD_POST_UC2:
	case Lop_LD_POST_C2:
	    load_opc = Lop_LD_POST_UC2;
	    load_base_popc = PLAYDOHop_LI_H_V1_V1;
	    break;
	case Lop_LD_POST_I:
	    load_opc = Lop_LD_POST_I;
	    load_base_popc = PLAYDOHop_LI_W_V1_V1;
	    break;
	case Lop_LD_POST_F:
	    load_opc = Lop_LD_POST_F;
	    load_base_popc = PLAYDOHop_FLI_S_V1_V1;
	    break;
	case Lop_LD_POST_F2:
	    load_opc = Lop_LD_POST_F2;
	    load_base_popc = PLAYDOHop_FLI_D_V1_V1;
	    break;

	// SLARSEN: Handle vector loads
	case Lop_VLD_UC:
	case Lop_VLD_C:
	    load_opc = Lop_VLD_C;
	    load_base_popc = PLAYDOHop_VL_B_C1_C1;
	    vector = 1;
	    break;
	case Lop_VLDE_UC:
	case Lop_VLDE_C:
	    load_opc = Lop_VLDE_C;
	    load_base_popc = PLAYDOHop_VLE_B_C1_C1;
	    vector = 1;
	    break;
	case Lop_VLD_UC2:
	case Lop_VLD_C2:
	    load_opc = Lop_VLD_C2;
	    load_base_popc = PLAYDOHop_VL_H_C1_C1;
	    vector = 1;
	    break;
	case Lop_VLDE_UC2:
	case Lop_VLDE_C2:
	    load_opc = Lop_VLDE_C2;
	    load_base_popc = PLAYDOHop_VLE_H_C1_C1;
	    vector = 1;
	    break;
	case Lop_VLD_I:
	    load_opc = Lop_VLD_I;
	    load_base_popc = PLAYDOHop_VL_W_C1_C1;
	    vector = 1;
	    break;
	case Lop_VLDE_I:
	    load_opc = Lop_VLDE_I;
	    load_base_popc = PLAYDOHop_VLE_W_C1_C1;
	    vector = 1;
	    break;
	case Lop_VLD_F:
	    load_opc = Lop_VLD_F;
	    load_base_popc = PLAYDOHop_VFL_S_C1_C1;
	    vector = 1;
	    break;
	case Lop_VLDE_F:
	    load_opc = Lop_VLDE_F;
	    load_base_popc = PLAYDOHop_VFLE_S_C1_C1;
	    vector = 1;
	    break;
	case Lop_VLD_F2:
	    load_opc = Lop_VLD_F2;
	    load_base_popc = PLAYDOHop_VFL_D_C1_C1;
	    vector = 1;
	    break;
	case Lop_VLDE_F2:
	    load_opc = Lop_VLDE_F2;
	    load_base_popc = PLAYDOHop_VFLE_D_C1_C1;
	    vector = 1;
	    break;

	default:
	    L_punt("L_annotate_ld: unknown load opcode for op %d", oper->id);
	    break;
    }

    /*
     *	Now determine the real proc_opc based on the src and dest target
     *	cache specifiers (if the exist, (c1,c1) is the default if nothing
     *	is specified)
     */
    chs_src = LPLAYDOH_CACHE_SPECIFIER_C1;
    chs_dest = LPLAYDOH_CACHE_SPECIFIER_C1;
    attr=L_find_attr(oper->attr, LPLAYDOH_CACHE_SPECIFIER_NAME);
    if (attr!=NULL) {
	chs_src = attr->field[1]->value.i;
	chs_dest = attr->field[0]->value.i;
    }
    /* update the proc_opc using some simple arithmetic, note this assumes
       sequentially numbering of the load opcodes is maintained */
    if (vector) load_popc = load_base_popc;
    else load_popc = load_base_popc + (4*chs_src) + chs_dest;
    
    new_oper2 = L_create_new_op_using(load_opc,oper);
    new_oper2->proc_opc = load_popc;
    new_oper2->src[0] = L_copy_operand(load_src);
    new_oper2->src[1] = L_new_gen_int_operand(0);
    new_oper2->src[2] = L_copy_operand(oper->src[2]);
    new_oper2->dest[0] = L_copy_operand(oper->dest[0]);
    new_oper2->dest[1] = L_copy_operand(oper->dest[1]);

    return new_oper2;
}

static void L_annotate_ld(L_Oper *oper)
{
    L_Oper *new_oper2=NULL, *new_oper3=NULL;

    if (L_should_swap_operands(oper->src[0],oper->src[1])) {
	L_Operand *tmpop ;

	tmpop = oper->src[0] ;
	oper->src[0] = oper->src[1] ;
	oper->src[1] = tmpop ;
    }

    if(Lplaydoh_use_base_disp) {
      if (!L_is_int_zero(oper->src[1]))
       new_oper2 = L_annotate_base_disp_ld(oper);
      else 
       new_oper2 = L_annotate_plain_ld(oper);
    }
    else {
     new_oper2 = L_annotate_plain_ld(oper);
    } 

#if 1
    /* copy sync arcs - SAM 8-95 */
    if (oper->sync_info != NULL) {
	new_oper2->sync_info = L_copy_sync_info (oper->sync_info);
	L_add_to_child_list(oper, new_oper2);
    }
#endif
    if(oper->acc_info)
      new_oper2->acc_info = L_copy_mem_acc_spec_list (oper->acc_info);
    
    L_insert_oper_before(mcb,oper,new_oper2);
    
    /* If we are not generating the LX/LGX opcodes, then insert
       these explicit sign ext operations.
     */
    if(!Lplaydoh_use_sign_ext) {
      /* Handle sign extension if necessary */
      switch ( oper->opc )  {
	case Lop_LD_C:
	case Lop_LD_POST_C:
	    new_oper3 = L_create_new_op_using(Lop_SXT_C,oper);
	    CLEAR_PROBE_MARK_FLAG(new_oper3);
	    break;
	case Lop_LD_C2:
	case Lop_LD_POST_C2:
	    new_oper3 = L_create_new_op_using(Lop_SXT_C2,oper);
	    CLEAR_PROBE_MARK_FLAG(new_oper3);
	    break;

        // SLARSEN: Handle sign extension for vector loads
        case Lop_VLD_C:
	    new_oper3 = L_create_new_op_using(Lop_VEXTRACT_C,oper);
	    CLEAR_PROBE_MARK_FLAG(new_oper3);
	    new_oper3->proc_opc = PLAYDOHop_VEXTRSB;
	    break;
	case Lop_VLD_C2:
	    new_oper3 = L_create_new_op_using(Lop_VEXTRACT_C2,oper);
	    CLEAR_PROBE_MARK_FLAG(new_oper3);
	    new_oper3->proc_opc = PLAYDOHop_VEXTRSH;
	    break;
        case Lop_VLDE_C:
        case Lop_VLDE_C2:
	    fprintf(stderr, "** FIX ME **\n");
	    L_punt("Unsupported vector opcode.");
    }
    if ( new_oper3 )  {
	new_oper3->src[0] = L_copy_operand(new_oper2->dest[0]);
	new_oper3->dest[0] = L_copy_operand(oper->dest[0]);
	L_insert_oper_before(mcb,oper,new_oper3);
      }
    }
}

static void L_annotate_preinc_ld(L_Oper *oper)
{
    L_Oper *new_oper ;

    new_oper = L_create_new_op_using(Lop_ADD,oper) ;
    CLEAR_PROBE_MARK_FLAG(new_oper);

    L_remove_unnec_flags_from_addr_calc(new_oper);
    L_remove_unnec_attr_from_addr_calc(new_oper);

    new_oper->dest[0] = L_copy_operand(oper->dest[1]) ;
    new_oper->src[0] = L_copy_operand(oper->src[0]) ;
    new_oper->src[1] = L_copy_operand(oper->src[2]) ;

    L_insert_oper_before(mcb,oper,new_oper);
    
    if ( oper->opc == Lop_LD_PRE_C )
	oper->opc = Lop_LD_C;
    else if ( oper->opc == Lop_LD_PRE_C2 )
	oper->opc = Lop_LD_C2;
    
    L_annotate_ld(oper); 
}

static L_Oper* L_annotate_base_disp_st(L_Oper *oper)
{
    int store_popc, store_base_popc, chs_dest;
    L_Oper *new_oper2 ;
    L_Attr *attr;

    /*
     *	Determine the base proc_opc
     */
    store_base_popc = 0;

    switch (oper->opc)  {
	case Lop_ST_C:
	    store_base_popc = PLAYDOHop_SG_B_V1;
	    break;
	case Lop_ST_C2:
	    store_base_popc = PLAYDOHop_SG_H_V1;
	    break;
	case Lop_ST_I:
	    store_base_popc = PLAYDOHop_SG_W_V1;
	    break;
	case Lop_ST_F:
	    store_base_popc = PLAYDOHop_FSG_S_V1;
	    break;
	case Lop_ST_F2:
	    store_base_popc = PLAYDOHop_FSG_D_V1;
	    break;
        /* Just punt for now */
	case Lop_ST_POST_C:
	case Lop_ST_POST_C2:
	case Lop_ST_POST_I:
	case Lop_ST_POST_F:
	case Lop_ST_POST_F2:
	default:
	    L_punt("L_annotate_st: unknown store opcode for op %d", oper->id);
	    break;
    }

    /*
     *	Now determine the real proc_opc based on the src and dest target
     *	cache specifiers (if the exist, (c1,c1) is the default if nothing
     *	is specified)
     */
    chs_dest = LPLAYDOH_CACHE_SPECIFIER_C1;
    attr=L_find_attr(oper->attr, LPLAYDOH_CACHE_SPECIFIER_NAME);
    if (attr!=NULL) {
	chs_dest = attr->field[0]->value.i;
    }
    /* update the proc_opc using some simple arithmetic, note this assumes
       sequentially numbering of the load opcodes is maintained */
    store_popc = store_base_popc + chs_dest;


    /* create store operation */
    new_oper2 = L_create_new_op_using(oper->opc,oper) ;
    new_oper2->proc_opc = store_popc;
    new_oper2->src[0] = L_copy_operand(oper->src[0]);
    new_oper2->src[1] = L_copy_operand(oper->src[1]);
    new_oper2->src[3] = L_copy_operand(oper->src[3]);
    new_oper2->dest[0] = L_copy_operand(oper->dest[0]);

    return new_oper2;
}
static L_Oper* L_annotate_plain_st(L_Oper *oper)
{
    int store_popc, store_base_popc, chs_dest;
    L_Oper *new_oper1=NULL, *new_oper2=NULL ;
    L_Operand *st_src=NULL;
    L_Attr *attr=NULL;

    int vector = 0;

    if (L_is_int_zero(oper->src[1]) &&
	(L_is_register(oper->src[0]) || L_is_macro(oper->src[0]))) {
	st_src = oper->src[0];
    } 
    else {
	/* create address calculation */
	new_oper1 = L_create_new_op_using(Lop_ADD,oper) ;
	CLEAR_PROBE_MARK_FLAG(new_oper1);

        L_remove_unnec_flags_from_addr_calc(new_oper1);
        L_remove_unnec_attr_from_addr_calc(new_oper1);

	new_oper1->dest[0] = L_new_register_operand(++L_fn->max_reg_id, L_CTYPE_INT, L_PTYPE_NULL);
	new_oper1->src[0] = L_copy_operand(oper->src[0]) ;
	new_oper1->src[1] = L_copy_operand(oper->src[1]) ;
	st_src = new_oper1->dest[0];

	L_insert_oper_before(mcb,oper,new_oper1);
    }

    /*
     *	Determine the base proc_opc
     */
    switch (oper->opc)  {
	case Lop_ST_C:
	    store_base_popc = PLAYDOHop_S_B_V1;
	    break;
	case Lop_ST_C2:
	    store_base_popc = PLAYDOHop_S_H_V1;
	    break;
	case Lop_ST_I:
	    store_base_popc = PLAYDOHop_S_W_V1;
	    break;
	case Lop_ST_F:
	    store_base_popc = PLAYDOHop_FS_S_V1;
	    break;
	case Lop_ST_F2:
	    store_base_popc = PLAYDOHop_FS_D_V1;
	    break;
	case Lop_ST_POST_C:
	    store_base_popc = PLAYDOHop_SI_B_V1;
	    break;
	case Lop_ST_POST_C2:
	    store_base_popc = PLAYDOHop_SI_H_V1;
	    break;
	case Lop_ST_POST_I:
	    store_base_popc = PLAYDOHop_SI_W_V1;
	    break;
	case Lop_ST_POST_F:
	    store_base_popc = PLAYDOHop_FSI_S_V1;
	    break;
	case Lop_ST_POST_F2:
	    store_base_popc = PLAYDOHop_FSI_D_V1;
	    break;

	// SLARSEN: Handle vector stores
	case Lop_VST_C:
	    store_base_popc = PLAYDOHop_VS_B_C1;
	    vector = 1;
	    break;
	case Lop_VSTE_C:
	    store_base_popc = PLAYDOHop_VSE_B_C1;
	    vector = 1;
	    break;
	case Lop_VST_C2:
	    store_base_popc = PLAYDOHop_VS_H_C1;
	    vector = 1;
	    break;
	case Lop_VSTE_C2:
	    store_base_popc = PLAYDOHop_VSE_H_C1;
	    vector = 1;
	    break;
	case Lop_VST_I:
	    store_base_popc = PLAYDOHop_VS_W_C1;
	    vector = 1;
	    break;
	case Lop_VSTE_I:
	    store_base_popc = PLAYDOHop_VSE_W_C1;
	    vector = 1;
	    break;
	case Lop_VST_F:
	    store_base_popc = PLAYDOHop_VFS_S_C1;
	    vector = 1;
	    break;
	case Lop_VSTE_F:
	    store_base_popc = PLAYDOHop_VFSE_S_C1;
	    vector = 1;
	    break;
	case Lop_VST_F2:
	    store_base_popc = PLAYDOHop_VFS_D_C1;
	    vector = 1;
	    break;
	case Lop_VSTE_F2:
	    store_base_popc = PLAYDOHop_VFSE_D_C1;
	    vector = 1;
	    break;

	default:
	    L_punt("L_annotate_st: unknown store opcode for op %d", oper->id);
	    break;
    }

    /*
     *	Now determine the real proc_opc based on the src and dest target
     *	cache specifiers (if the exist, (c1,c1) is the default if nothing
     *	is specified)
     */
    chs_dest = LPLAYDOH_CACHE_SPECIFIER_C1;
    attr=L_find_attr(oper->attr, LPLAYDOH_CACHE_SPECIFIER_NAME);
    if (attr!=NULL) {
	chs_dest = attr->field[0]->value.i;
    }
    /* update the proc_opc using some simple arithmetic, note this assumes
       sequentially numbering of the load opcodes is maintained */
    if (vector) store_popc = store_base_popc;
    else store_popc = store_base_popc + chs_dest;

    /* create store operation */
    new_oper2 = L_create_new_op_using(oper->opc,oper) ;
    new_oper2->proc_opc = store_popc;
    new_oper2->src[0] = L_copy_operand(st_src);
    new_oper2->src[1] = L_new_gen_int_operand(0);
    new_oper2->src[3] = L_copy_operand(oper->src[3]);
    new_oper2->dest[0] = L_copy_operand(oper->dest[0]);

    return new_oper2;
}

static void L_annotate_st(L_Oper *oper)
{
    L_Oper *new_oper2=NULL ;

    if (L_should_swap_operands(oper->src[0],oper->src[1])) {
	L_Operand *tmpop ;

	tmpop = oper->src[0] ;
	oper->src[0] = oper->src[1] ;
	oper->src[1] = tmpop ;
    }

    if(Lplaydoh_use_base_disp) {
      if(!L_is_int_zero(oper->src[1]))
        new_oper2 = L_annotate_base_disp_st(oper);
      else
        new_oper2 = L_annotate_plain_st(oper);
    }
    else {
      new_oper2 = L_annotate_plain_st(oper);
    }

#if 1
    /* copy the sync arcs - SAM 8-95 */
    if (oper->sync_info!=NULL) {
	new_oper2->sync_info = L_copy_sync_info (oper->sync_info);
	L_add_to_child_list(oper, new_oper2);
    }
#endif
       
    new_oper2->src[2] = L_copy_operand(oper->src[2]);
    if(oper->acc_info)
      new_oper2->acc_info = L_copy_mem_acc_spec_list (oper->acc_info);

    L_insert_oper_before(mcb,oper,new_oper2);
}

static void L_annotate_preinc_st(L_Oper *oper)
{
    L_Oper *new_oper ;

    new_oper = L_create_new_op_using(Lop_ADD,oper) ;
    CLEAR_PROBE_MARK_FLAG(new_oper);

    L_remove_unnec_flags_from_addr_calc(new_oper);
    L_remove_unnec_attr_from_addr_calc(new_oper);

    new_oper->dest[0] = L_copy_operand(new_oper->dest[0]) ;
    new_oper->src[0] = L_copy_operand(oper->src[0]) ;
    new_oper->src[1] = L_copy_operand(oper->src[3]) ;
    
    L_insert_oper_before(mcb,oper,new_oper);
    
    L_annotate_st(oper) ;
}

static void L_annotate_extract(L_Oper *oper, int opc)
{
    L_Oper *new_oper1;

    new_oper1 = L_copy_parent_oper(oper);

    // SLARSEN: Handle vector extracts
    if (opc==Lop_VEXTRACT_C)
        new_oper1->proc_opc = PLAYDOHop_VEXTRSB;
    else if (opc==Lop_VEXTRACT_C2)
        new_oper1->proc_opc = PLAYDOHop_VEXTRSH;

    else if (opc==Lop_EXTRACT_C)
        new_oper1->proc_opc = PLAYDOHop_EXTRSB;
    else
        new_oper1->proc_opc = PLAYDOHop_EXTRSH;

    L_insert_oper_before(mcb,oper,new_oper1);
}

static void L_annotate_or_and_not(L_Oper *oper, int opc)
{
    L_Oper *new_oper1, *new_oper2 ;

    if (opc==Lop_OR_NOT)
	new_oper1 = L_create_new_op_using(Lop_OR_COMPL, oper);
    else
	new_oper1 = L_create_new_op_using(Lop_AND_COMPL, oper);
    new_oper1->src[0] = L_copy_operand(oper->src[0]) ;
    new_oper1->src[1] = L_copy_operand(oper->src[1]) ;
    new_oper1->dest[0] = L_new_register_operand(++L_fn->max_reg_id, L_CTYPE_INT,
						L_PTYPE_NULL);

    new_oper2 = L_create_new_op_using(Lop_NE, oper);
    CLEAR_PROBE_MARK_FLAG(new_oper2);
    new_oper2->dest[0] = L_copy_operand(oper->dest[0]);
    new_oper2->src[0] = L_copy_operand(new_oper1->dest[0]);
    new_oper2->src[1] = L_new_gen_int_operand(0);

    L_insert_oper_before(mcb,oper,new_oper1);
    L_insert_oper_before(mcb,oper,new_oper2);    
}

/******************************************************************************\
 *
 * Floating point Annotation functions
 *
\******************************************************************************/

static void L_annotate_float_move(L_Oper *oper)
{
    L_Oper *new_oper;
   
    new_oper = L_copy_parent_oper(oper);

    L_insert_oper_before(mcb,oper,new_oper);
}

static void L_annotate_float_oper(L_Oper *oper, int op)
{
    int i;
    L_Oper *new_oper, *new_oper1;
    
    new_oper = L_create_new_op_using(op,oper);
    
    for ( i = 0; i < L_max_src_operand; i++ )  {
	L_Operand *src;
	if ( (src = oper->src[i]) == NULL )  continue;

#if 0
        // The code below does not appear to work.  L_float_constant_load
        // doesn't seem to exist anymore.  -KF 1/2007

	/* SLARSEN: vector/scalar transfer ops have an integer source,
	   so make sure this is actually a float operand */
	if ( !L_is_reg(src) && !L_is_macro(src) && 
	     (L_is_ctype_flt(src) || L_is_ctype_dbl(src)) ) {
	    if ( src->value.f2 == 0.0 )  {
		new_oper->src[i] = L_new_macro_operand(PLAYDOH_MAC_FZERO,L_return_old_ctype(src),
						       L_PTYPE_NULL);
	    }
	    else if ( (L_is_ctype_flt(src) && src->value.f == 1.0) ||
		      (L_is_ctype_dbl(src) && src->value.f2 == 1.0) )  {
		new_oper->src[i] = L_new_macro_operand(PLAYDOH_MAC_FONE,L_return_old_ctype(src),
						       L_PTYPE_NULL);
	    }
	    else  {
                new_oper1 = L_float_constant_load(src,oper);
		L_insert_oper_before(mcb,oper,new_oper1);
		new_oper->src[i] = L_copy_operand(new_oper1->dest[0]);
	    }
	}
	else
#endif
	    new_oper->src[i] = L_copy_operand(src);
    }
    
    for ( i = 0; i < L_max_dest_operand; i++ )  {
	L_Operand *dest;
	if ( (dest = oper->dest[i]) == NULL ) continue;
	new_oper->dest[i] = L_copy_operand(dest);
    }
    
    L_insert_oper_before(mcb, oper, new_oper);
}

 

static void L_annotate_float_cond_branch(L_Oper *oper, int op)
{
    int i;
    L_Oper *new_oper1, *new_oper2, *new_oper3;

    /* create PBR */
    new_oper1 = L_create_new_op_using(Lop_PBR, oper);
    new_oper1->com[0] = 0; /* Clear spurious completers */
    new_oper1->com[1] = 0;

    CLEAR_PROBE_MARK_FLAG(new_oper1);

    L_remove_unnec_flags_from_pbr(new_oper1);
    L_remove_unnec_attr_from_pbr(new_oper1);

    new_oper1->dest[0] = L_new_register_operand(++L_fn->max_reg_id, L_CTYPE_BTR,
							L_PTYPE_NULL);
    new_oper1->src[0] = L_copy_operand(oper->src[2]);
    /* if (L_fs_cond_branch_opcode(oper)) KVM*/
    new_oper1->src[1] = L_new_gen_int_operand(L_PREDICT_NOTTAKEN);
    new_oper1->proc_opc = PLAYDOHop_PBRR;
    
    /* create compare */
    new_oper2 = L_create_new_op_using(L_pred_compare_for_branch(oper), oper);
    CLEAR_PROBE_MARK_FLAG(new_oper2);

    L_remove_unnec_flags_from_pbr(new_oper2);
    L_remove_unnec_attr_from_pbr(new_oper2);

    new_oper2->dest[0] = L_new_register_operand(++L_fn->max_reg_id,
						L_CTYPE_PREDICATE, L_PTYPE_UNCOND_T);
    
    for ( i = 0; i < 2; i++ )  {
	L_Operand *src;
	if ( (src = oper->src[i]) == NULL )  continue;
	new_oper2->src[i] = L_copy_operand(src);
    }
    new_oper2->com[0] = oper->com[0];
    new_oper2->com[1] = oper->com[1];

    /* create branch */
    new_oper3 = L_create_new_op_using(Lop_BR, oper);
    new_oper3->com[0] = L_CTYPE_INT;
    new_oper3->com[1] = Lcmp_COM_EQ;
    new_oper3->proc_opc = PLAYDOHop_BRCT;
    new_oper3->src[0] = L_copy_operand(new_oper2->dest[0]);
    new_oper3->src[0]->ptype = L_PTYPE_NULL;
    new_oper3->src[1] = L_new_gen_int_operand(1);
    new_oper3->src[2] = L_copy_operand(oper->src[2]);
    new_oper3->src[3] = L_copy_operand(new_oper1->dest[0]);

    /* insert new ops */
    L_insert_oper_before(mcb, oper, new_oper1);
    L_insert_oper_before(mcb, oper, new_oper2);
    L_insert_oper_before(mcb, oper, new_oper3);
}

static void L_annotate_float_compare(L_Oper *oper, int op)
{
    int i;
    L_Oper *new_oper;

    new_oper = L_create_new_op_using(op,oper);

    for ( i = 0; i < L_max_src_operand; i++ )  {
	L_Operand *src;
	if ( (src = oper->src[i]) == NULL )  continue;
	new_oper->src[i] = L_copy_operand(src);
    }
   
    for (i=0; i<L_max_dest_operand; i++) { 
	L_Operand *dest;
	if ( (dest = oper->dest[i]) == NULL)
	    continue;
        new_oper->dest[i] = L_copy_operand(dest);
    }
    /* KVM : Jun 17th 2003 */
    for (i=0; i<2; i++) {
      new_oper->com[i] = oper->com[i];
    }

    L_insert_oper_before(mcb,oper,new_oper);
}

static void L_annotate_float_conversion(L_Oper *oper, int op)
{
    L_Oper *new_oper;
    
    new_oper = L_create_new_op_using(op,oper);

    new_oper->src[0] = L_copy_operand(oper->src[0]);
    new_oper->dest[0] = L_copy_operand(oper->dest[0]);
    L_insert_oper_before(mcb,oper,new_oper);
}

void L_annotate_oper(L_Func *fn, L_Cb *cb, L_Oper *oper)
{
    int op = oper->opc;

    if (L_opc_vestigial(op)) {
      L_punt("L_annotate_oper: Illegal vestigial opcode: %s", oper->opcode);
    } 
    switch(op) {

  	case Lop_NO_OP:
	    L_insert_oper_before(mcb,oper,L_copy_parent_oper(oper));
	    break;

        case Lop_JSR: 
	    L_annotate_jsr(oper);
	    break;

        case Lop_RTS:
	    L_annotate_rts(oper);
	    break;

        case Lop_JUMP: 
        case Lop_JUMP_FS: 
	case Lop_JUMP_RG:
	    L_annotate_jump(oper, op);
	    break;

  	case Lop_PROLOGUE:
	    L_annotate_prologue(oper);
	    break;

  	case Lop_EPILOGUE: 
	case Lop_DEFINE: 
  	case Lop_ALLOC: 
	    L_insert_oper_before(mcb,oper,L_copy_parent_oper(oper));
            break;

  	case Lop_BR: /*KVM */
	    L_annotate_cond_br(oper);
	    break;

  	case Lop_BR_F: /* KVM */
	    L_annotate_float_cond_branch(oper,op);
	    break;

        case Lop_PBR:
	    L_annotate_pbr(oper);
	    break;

  	case Lop_MOV: 
	    L_annotate_int_move(oper);
	    break;

	case Lop_LD_PRE_UC: 
	case Lop_LD_PRE_C: 
	case Lop_LD_PRE_UC2: 
	case Lop_LD_PRE_C2: 
	case Lop_LD_PRE_I:
	case Lop_LD_PRE_Q:
	case Lop_LD_PRE_F: 
	case Lop_LD_PRE_F2: 
	    L_annotate_preinc_ld(oper);
	    break;

	case Lop_LD_POST_UC: 
	case Lop_LD_POST_C: 
	case Lop_LD_POST_UC2: 
	case Lop_LD_POST_C2: 
	case Lop_LD_POST_I:
	case Lop_LD_POST_Q:
	case Lop_LD_POST_F: 
	case Lop_LD_POST_F2: 
  	case Lop_LD_UC:
	case Lop_LD_C:
	case Lop_LD_UC2: 
	case Lop_LD_C2:
	case Lop_LD_I: 
	case Lop_LD_Q: 
  	case Lop_LD_F:
	case Lop_LD_F2:
	    L_annotate_ld(oper);
	    break;

	case Lop_ST_PRE_C: 
	case Lop_ST_PRE_C2: 
	case Lop_ST_PRE_I:
	case Lop_ST_PRE_Q:
	case Lop_ST_PRE_F: 
	case Lop_ST_PRE_F2:
	    L_annotate_preinc_st(oper);
	    break;

	case Lop_ST_POST_C: 
	case Lop_ST_POST_C2: 
	case Lop_ST_POST_I:
	case Lop_ST_POST_Q:
	case Lop_ST_POST_F: 
	case Lop_ST_POST_F2: 
  	case Lop_ST_C:
	case Lop_ST_C2:
	case Lop_ST_I:
	case Lop_ST_Q:
  	case Lop_ST_F:
	case Lop_ST_F2: 
	    L_annotate_st(oper);
	    break;

	case Lop_EXTRACT_C:
	case Lop_EXTRACT_C2:
	    L_annotate_extract(oper, op);
	    break;

  	case Lop_ADD: 
	case Lop_ADD_U: 
	case Lop_SUB: 
	case Lop_SUB_U: 
	case Lop_MUL:
	case Lop_MUL_U: 
        case Lop_REM:
	case Lop_REM_U:
	case Lop_DIV:
	case Lop_DIV_U:
        case Lop_ABS:
  	case Lop_LSL:
	case Lop_LSR:
	case Lop_ASR: 
	    L_insert_oper_before(mcb,oper,L_copy_parent_oper(oper));
	    break;

	case Lop_MUL_ADD:
	case Lop_MUL_ADD_U:
	    L_insert_oper_before(mcb,oper,L_copy_parent_oper(oper));
	    // L_annotate_int_muladd(oper, op);
	    break;

	case Lop_MUL_SUB:
        case Lop_MUL_SUB_U:
	case Lop_MUL_SUB_REV:
        case Lop_MUL_SUB_REV_U:
	    L_insert_oper_before(mcb,oper,L_copy_parent_oper(oper));
	    // L_annotate_int_mulsub(oper, op);
	    break;

	case Lop_MAX:
	case Lop_MIN:
	    L_insert_oper_before(mcb,oper,L_copy_parent_oper(oper));
	    break;

	case Lop_REV:
	case Lop_BIT_POS:
    	    L_print_oper(stderr, oper);
	    L_punt("L_annotate_oper: unsupported Lcode opcode");
	    break;

  	case Lop_OR:
	case Lop_NOR:
	case Lop_OR_COMPL:
	case Lop_AND:
	case Lop_NAND: 
	case Lop_AND_COMPL:
  	case Lop_XOR:
	case Lop_NXOR:
	    L_insert_oper_before(mcb,oper,L_copy_parent_oper(oper));
	    break;

	case Lop_AND_NOT:
	case Lop_OR_NOT:
	    L_annotate_or_and_not(oper, op);
	    break;

	case Lop_RCMP:
	    L_insert_oper_before(mcb,oper,L_copy_parent_oper(oper));
	    break;

  	case Lop_MOV_F:
  	case Lop_MOV_F2: 
	    L_annotate_float_move(oper);
	    break;

  	case Lop_ADD_F:
	case Lop_SUB_F: 
	case Lop_MUL_F:
	case Lop_DIV_F:
	case Lop_RCP_F:
	case Lop_ABS_F:
	case Lop_MUL_ADD_F:
	case Lop_MUL_SUB_F:
	case Lop_MUL_SUB_REV_F:
	case Lop_SQRT_F:
	case Lop_MAX_F:
	case Lop_MIN_F:
	case Lop_ADD_F2: 
	case Lop_SUB_F2:
	case Lop_MUL_F2: 
        case Lop_DIV_F2:
        case Lop_RCP_F2:
        case Lop_ABS_F2:
	case Lop_MUL_ADD_F2:
	case Lop_MUL_SUB_F2:
	case Lop_MUL_SUB_REV_F2:
	case Lop_SQRT_F2:
	case Lop_MAX_F2:
	case Lop_MIN_F2:
	    L_annotate_float_oper(oper,op);
	    break;

	case Lop_RCMP_F:
	    L_annotate_float_compare(oper,oper->opc);
	    break;

	case Lop_F2_F:
	case Lop_F_F2: 
  	case Lop_F2_I: 
	case Lop_F_I:
	    L_annotate_float_conversion(oper,op);
	    break;

  	case Lop_I_F2: 
	case Lop_I_F:
	    L_insert_oper_before(mcb,oper,L_copy_parent_oper(oper));
	    break;

	case Lop_PRED_CLEAR:
	case Lop_PRED_SET:
	    L_insert_oper_before(mcb,oper,L_copy_parent_oper(oper));
	    break;

	case Lop_CMP:
	    L_insert_oper_before(mcb,oper,L_copy_parent_oper(oper));
	    break;

	case Lop_CMP_F:
	    L_annotate_float_compare(oper,op);
	    break;

	case Lop_PRED_LD:
	case Lop_PRED_ST:
	case Lop_PRED_LD_BLK:
	case Lop_PRED_ST_BLK:
	case Lop_PRED_MERGE:
    	    L_print_oper(stderr, oper);
	    L_punt("L_annotate_oper: unsupported Lcode opcode!");
	    break;

	case Lop_FETCH_AND_ADD:
	case Lop_FETCH_AND_OR:
	case Lop_FETCH_AND_AND:
	case Lop_FETCH_AND_ST:
	case Lop_FETCH_AND_COND_ST:
	case Lop_ADVANCE:
	case Lop_AWAIT:
	case Lop_MUTEX_B:
	case Lop_MUTEX_E:
	case Lop_CO_PROC:
    	    L_print_oper(stderr, oper);
	    L_punt("L_annotate_oper: unsupported Lcode opcode!");
	    break;

	case Lop_CHECK:
	case Lop_CONFIRM:
	case Lop_PREF_LD:
	case Lop_JSR_ND:
	case Lop_EXPAND:
	case Lop_SIM_DIR:
    	    L_print_oper(stderr, oper);
	    L_punt("L_annotate_oper: unsupported Lcode opcode!");
	    break;

	/* Partial predicate support */
	case Lop_CMOV:
	case Lop_CMOV_COM:
	case Lop_CMOV_F:
	case Lop_CMOV_COM_F:
	case Lop_CMOV_F2:
	case Lop_CMOV_COM_F2:
	case Lop_SELECT:
	case Lop_SELECT_F:
	case Lop_SELECT_F2:
    	    L_print_oper(stderr, oper);
	    L_punt("L_annotate_oper: unsupported Lcode opcode!");
	    break;
  
        case Lop_SXT_C: case Lop_SXT_C2: case Lop_SXT_I:
        case Lop_ZXT_C: case Lop_ZXT_C2: case Lop_ZXT_I:
            L_insert_oper_before(mcb,oper,L_copy_parent_oper(oper));
            break;

        case Lop_EXTRACT:
        case Lop_EXTRACT_U:
            L_insert_oper_before(mcb,oper,L_copy_parent_oper(oper));
            break;

        case Lop_MUL_WIDE:
        case Lop_ADD_CARRY:
        case Lop_ADD_CARRY_U:
        case Lop_SUB_CARRY_U:
        case Lop_SUB_CARRY:
        case Lop_MUL_WIDE_U:
            L_insert_oper_before(mcb, oper, L_copy_parent_oper(oper));
            break;

        /* SLARSEN: Handle vector instructions */
        case Lop_VADD:
        case Lop_VADD_U:
        case Lop_VSUB:
        case Lop_VSUB_U:
        case Lop_VMUL:
        case Lop_VMUL_U:
        case Lop_VDIV:
        case Lop_VDIV_U:
        case Lop_VREM:
        case Lop_VREM_U:
        case Lop_VMIN:
        case Lop_VMAX:
        case Lop_VOR:
        case Lop_VAND:
        case Lop_VPERM:
	    L_insert_oper_before(mcb,oper,L_copy_parent_oper(oper));
	    break;
        case Lop_VADD_F:
        case Lop_VSUB_F:
        case Lop_VMUL_F:
        case Lop_VDIV_F:
        case Lop_VMAX_F:
        case Lop_VMIN_F:
        case Lop_VABS_F:
        case Lop_VPERM_F:
        case Lop_VSQRT_F:
        case Lop_VADD_F2:
        case Lop_VSUB_F2:
        case Lop_VMUL_F2:
        case Lop_VDIV_F2:
        case Lop_VMAX_F2:
        case Lop_VMIN_F2:
        case Lop_VABS_F2:
        case Lop_VSQRT_F2:
        case Lop_VPERM_F2:
	    L_annotate_float_oper(oper,op);
	    break;
	case Lop_VF2_VF:
	case Lop_VF_VF2: 
  	case Lop_VF2_VI: 
	case Lop_VF_VI:
	    L_annotate_float_conversion(oper,op);
	    break;
        case Lop_VF_F:
        case Lop_F_VF:
        case Lop_VF2_F2:
        case Lop_F2_VF2:
        case Lop_VSPLAT_F:
        case Lop_VSPLAT_F2:
	    L_annotate_float_oper(oper,op);
	    break;
  	case Lop_VI_VF2: 
	case Lop_VI_VF:
        case Lop_VI_I:
        case Lop_I_VI:
        case Lop_VSPLAT:
	    L_insert_oper_before(mcb,oper,L_copy_parent_oper(oper));
	    break;
  	case Lop_VMOV: 
	    L_annotate_int_move(oper);
	    break;
  	case Lop_VMOV_F:
  	case Lop_VMOV_F2: 
	    L_annotate_float_move(oper);
	    break;
	case Lop_VLD_UC:
	case Lop_VLD_C:
	case Lop_VLD_UC2:
	case Lop_VLD_C2:
	case Lop_VLD_I:
  	case Lop_VLD_F:
	case Lop_VLD_F2:
	case Lop_VLDE_UC:
	case Lop_VLDE_C:
	case Lop_VLDE_UC2:
	case Lop_VLDE_C2:
	case Lop_VLDE_I:
  	case Lop_VLDE_F:
	case Lop_VLDE_F2:
	    L_annotate_ld(oper);
	    break;
  	case Lop_VST_C:
	case Lop_VST_C2:
	case Lop_VST_I:
  	case Lop_VST_F:
	case Lop_VST_F2: 
  	case Lop_VSTE_C:
	case Lop_VSTE_C2:
	case Lop_VSTE_I:
  	case Lop_VSTE_F:
	case Lop_VSTE_F2: 
	    L_annotate_st(oper);
	    break;
	case Lop_VEXTRACT_C:
	case Lop_VEXTRACT_C2:
	    L_annotate_extract(oper, op);
	    break;

  	default:
    	    L_print_oper(stderr, oper);
    	    L_punt("annotate_oper: unrecognized opcode");
	    break;
    }
}

/*===============================================================================*/
/*
 *	Preserving Lcode ids in the Mcode.
 */
/*===============================================================================*/

int max_lcode_oper_id;

#define L_OPER_LCODE_ID_SAVED   L_OPER_RESERVED_TEMP1

void L_preserve_lcode_ids(L_Func *fn)
{
    L_Cb *cb;
    L_Oper *oper;

    for ( cb = fn->first_cb; cb != NULL ; cb = cb->next_cb )  {
        for ( oper = cb->first_op; oper != NULL; oper = oper->next_op )  {
            if ( L_EXTRACT_BIT_VAL(oper->flags,L_OPER_PROBE_MARK) ) {
                /* This operation should retain the id of the */
                /* parent Lcode operation                     */
                if ( !L_EXTRACT_BIT_VAL(oper->parent_op->flags,
                                                L_OPER_LCODE_ID_SAVED) )  {
                    oper->id = oper->parent_op->id;
                    oper->parent_op->flags = L_SET_BIT_FLAG(oper->parent_op->flags,
                                                        L_OPER_LCODE_ID_SAVED);
                }
                else {
                    /* Two opers from the same parent are marked for */
                    /* probing.  This is an erroneous condition!     */
                    fprintf(stdout,"Child Oper\n");
                    L_print_oper(stdout,oper);
                    fprintf(stdout,"of Parent Oper\n");
                    L_print_oper(stdout,oper->parent_op);
                    fprintf(stdout,"is the second to want the parent's id\n");
                    exit(-1);
                }
            }
            else  {
                /* This operation should be given a new id */
                oper->id = ++max_lcode_oper_id;
            }
        }
    }
    fn->max_oper_id = max_lcode_oper_id;

    L_oper_hash_tbl_rebuild(fn);

    return;
}

void L_clear_all_probe_mark_flags(L_Func *fn)
{
   L_Cb *cb;
    L_Oper *oper;

    for (cb=fn->first_cb; cb!=NULL; cb=cb->next_cb) {
        for (oper=cb->first_op; oper!=NULL; oper=oper->next_op) {
            oper->flags = L_CLR_BIT_FLAG(oper->flags, L_OPER_PROBE_MARK);
	}
    }
}

L_Oper *L_find_pbr_for_branch(L_Cb *cb, L_Oper *br)
{
    L_Oper *op;
    L_Operand *btr_reg = br->src[3]; /* the branch target */

    for (op=br->prev_op; op!=NULL; op=op->prev_op) {
	if (! L_is_opcode(Lop_PBR, op))
	    continue;
	if (L_same_operand(op->dest[0], btr_reg))
	    return op;
    }
    return NULL;
}

void L_set_pbr_static_predict(L_Func *fn)
{
    L_Cb *cb;
    L_Flow *flow;
    L_Oper *oper;

    L_Oper *pbr_op;
    double weight, taken_weight;

    for (cb=fn->first_cb; cb!=NULL; cb=cb->next_cb) {

        weight = cb->weight;
        flow = cb->dest_flow;
        for (oper=cb->first_op; oper!=NULL; oper=oper->next_op) {
            if (! (L_general_branch_opcode(oper) ||
                   L_subroutine_call_opcode(oper) ||
                   L_subroutine_return_opcode(oper)))
                continue;

            if (L_cond_branch(oper)) {
                taken_weight = flow->weight;
                flow = flow->next_flow;
                weight -= taken_weight;
            }
            else {
                taken_weight = weight;
                if ( ( /*L_uncond_branch_opcode(oper) || */
                     L_register_branch_opcode(oper)) &&
                     (oper!=cb->last_op))
                    L_punt("L_jump_branch_prediction: jrg or jmp must be last op");
            }

	    pbr_op = L_find_pbr_for_branch(cb, oper);
	    if (pbr_op != NULL) {
                /* mark likely taken pbr */
                if ((taken_weight>=weight) && (taken_weight>L_min_fs_weight))
		   pbr_op->src[1] = L_new_gen_int_operand(L_PREDICT_TAKEN);
                /* NOT likely taken */
                else
		   pbr_op->src[1] = L_new_gen_int_operand(L_PREDICT_NOTTAKEN);
            }

        }
    }
}


/******************************************************************************\
 *
 * L_process_func - annotate an Lcode function
 *
 * To ensure correct ordering of new Mopers within an Lcode function
 * we will annotate the Lcode oper into a list of Mcode opers [1 or more
 * Mcode opers].  This Mcode list of opers will be inserted on at a time
 * into the control block of the Lcode oper just before it.
 *
\******************************************************************************/
void L_process_func(L_Func *fn)
{
  /* initializations */
  L_Oper *oper;
  L_Cb *cb;

  if (L_do_software_pipelining) {
      Lpipe_softpipe_loop_prep(fn);
  }
  
  if (L_debug_messages)
    fprintf(stderr, "Annotating %s\n", fn->name);

  /*
   *	Before actually generating HPL_PD operations, some preprocessing
   *	of the Lcode file is needed to get it ready for downstream
   *    processing by Elcor.  These reflect minor incompatiblities
   *	between the 2 compiler systems.
   */

  /* Get rid of post increment ld/sts since we only emulate the PA
	versions of these not the most general form */
  if (!Lplaydoh_retain_post_inc) {
    L_breakup_pre_post_inc_ops(fn);
    L_unmark_all_pre_post_increments(fn);
  }

  /* Jump through register fixup */
  if (L_func_has_jump_table_info(fn))
      L_make_all_jump_tables_unique(fn);
  L_normalize_flows(fn);

  /* Having fallthru jumps in single bb loops gives elcor problems, so split
     those jumps out of the bb or hbs */
  if (Lplaydoh_breakup_single_bb_loops)
    L_breakup_single_bb_loops(fn);

  /* If we are doing region formation in Elcor, make input only have bbs!! */
  if (Lplaydoh_convert_to_strict_bb_code)
    Lplaydoh_convert_to_strict_basic_block_code(fn);

  /* Pass FP args thru the int param passing regs as well as the FP regs
     This is to support varargs functions, see lhpl_pd_phase1_varargs.c */
  Lhpl_pd_adjust_fp_parameter_passing(fn);

  /* Insert epilog block (if not there already), Elcor requires an epilog
     block, even if it is unreachable. */
  Lhpl_pd_insert_epilog_block(fn);

  /* Need to fix:
     1) O-type cmps w/o a pred_clear and A-type cmps w/o a pred_set
     2) cmp eq i0 i0 --> pred_set
     added mchu 5/04 */     
  Lhpl_pd_fixup_hyperblocks(fn);

  /* 
   *	Build sync arcs from Lcode memory dependence information
   */
  if (Lplaydoh_build_sync_arcs) {
    /* KVM : Don't try to add sync arcs if they've already been added
     */
    if (!L_find_attr(fn->attr, "DEP_PRAGMAS") ||
	!L_find_attr(fn->attr, "JSR_DEP_PRAGMAS"))
      L_build_sync_arcs(fn);
  }

  max_lcode_oper_id = fn->max_oper_id;

  /*
   * To maintain sync arcs, we must re-link all of the child opers
   * together after annotation.  To facilitate this, we maintain an
   * array of child pointers, indexed by parent id.
   */

  L_create_child_list(fn);

  /*
   * Initialize appropriate function variables to ensure correct
   * memory cleanup.  This is only needed when you are going to
   * create new parent Lcode operations which only occurs in phase 1
   * so far.
   */
  L_mcode_init_function();


  /* Loop through all of the control blocks within the function */
  for (cb=fn->first_cb; cb!=NULL; cb=cb->next_cb)
  {
    /*
     * Perform Lcode to Lcode transformations to simplify code annotation
     * process.  Some of these also relate to performance improvements
     * (ie strength reduction).
     */
    mcb = cb;

    /* Loop through all of the Lcode opers within a control block */
    oper = cb->first_op;
    while (oper != NULL)
    {

      oper->flags = L_SET_BIT_FLAG(oper->flags,L_OPER_PROBE_MARK);

      /* Annotate the current Lcode oper */
      L_annotate_oper(fn, cb, oper);

      /* Convert the Lcode oper to a parent */
      oper = L_convert_to_parent (mcb, oper);
    }
  }

  /* L_print_func(stdout, fn); */
  if (Lplaydoh_preserve_lcode_ids)
	L_preserve_lcode_ids(fn);

  L_clear_all_probe_mark_flags(fn);

  /* Walks through fn and points all sync arcs to other children, not parents */
  L_relink_child_sync_arcs (fn);
  L_delete_child_list();

  /* Set the static prediction bit for all pbrs */
  L_set_pbr_static_predict(fn);

  /* Mark the function as having branch compare conditions in preds */
  fn->flags = L_SET_BIT_FLAG(fn->flags, L_FUNC_CC_IN_PREDICATE_REGS);

}

/*
 * Global initializations
 */
void L_init(Parm_Macro_List *command_line_macro_list)
{
    if (L_do_software_pipelining) {
        Lpipe_loop_prep_init(command_line_macro_list);
    }
}
