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
 *      File:   sm_opti.c
 *      Author: John C. Gyllenhaal, Wen-mei Hwu
 *      Creation Date:  January 1997
\*****************************************************************************/
/* 12/03/02 REK Taking out the lhppa requirement for distribution. */

#include <config.h>
#include <stdio.h>
#include "sm.h"
#include <library/l_alloc_new.h>
#include <library/l_parms.h>
#include <library/dynamic_symbol.h>
#include <library/heap.h>
#include <Lcode/l_main.h>
#include <machine/m_hppa.h>
#if 0
#ifndef OPENIMPACT_DISTRIBUTION
#include <Lcode/lhppa_main.h>
#endif
#endif

extern void SM_print_expr_with_copy (FILE * out, SM_Trans * trans);
extern int opti_oper_lower_bound;
extern int opti_oper_upper_bound;
extern int verbose_optimization;
extern int do_expr_without_copy_only;
extern int use_classic_renaming_heuristics;
/* 20030226 SZU
 * SMH reconciliation
 */
extern L_Alloc_Pool *SM_Oper_pool;

L_Alloc_Pool *SM_Trans_pool = NULL;

/* Prints lcode op for a sm_op with no attributes at all */
void
SM_print_minimal_lcode_op (FILE * out, SM_Oper * sm_op)
{
  L_Attr *attr_list;
  ITintmax ext;

  /* Get ext if any */
  ext = SM_get_ext (sm_op);

  /* Remove existing attribute list */
  attr_list = sm_op->lcode_op->attr;
  sm_op->lcode_op->attr = NULL;

  /* If have ext, add ext attribute temporarily */
  if (ext != 0)
    {
      sm_op->lcode_op->attr = L_new_attr ("pext", 1);
      L_set_int_attr_field (sm_op->lcode_op->attr, 0, ext);
    }

  /* Print out the oper */
  L_print_oper (out, sm_op->lcode_op);

  /* Delete an ext attribute, plus any popc attributes L_print_oper added */
  L_delete_all_attr (sm_op->lcode_op->attr);

  /* Restore original attribute list */
  sm_op->lcode_op->attr = attr_list;
}

SM_Trans *
SM_new_trans (int type, SM_Oper * target_sm_op, int flags)
{
  SM_Trans *sm_trans;

  /* Create alloc pool if necessary */
  if (SM_Trans_pool == NULL)
    {
      SM_Trans_pool = L_create_alloc_pool ("SM_Trans", sizeof (SM_Trans), 16);
    }

  /* Alloc a new trans structure */
  sm_trans = (SM_Trans *) L_alloc (SM_Trans_pool);

  /* Initialize structure's fields */

  /* Save the cb and op id for the target_sm_op.  This allows info
   * to be printed about the trans even if the target_sm_op has
   * been deleted.
   */
  sm_trans->cb_id = target_sm_op->sm_cb->lcode_cb->id;
  sm_trans->op_id = target_sm_op->lcode_op->id;
  sm_trans->type = type;
  sm_trans->target_sm_op = target_sm_op;
  sm_trans->target_index = -1;
  sm_trans->other_index = -1;
  sm_trans->flags = flags;

  sm_trans->def_sm_op = NULL;
  sm_trans->def2_sm_op = NULL;

  sm_trans->orig_src[0] = NULL;
  sm_trans->orig_src[1] = NULL;
  sm_trans->orig_opc = -1;
  sm_trans->orig_proc_opc = -1;
  sm_trans->orig_ext = -1;
  sm_trans->orig_prev_serial_op = NULL;

  sm_trans->orig_def_src[0] = NULL;
  sm_trans->orig_def_src[1] = NULL;
  sm_trans->orig_def_opc = -1;
  sm_trans->orig_def_proc_opc = -1;
  sm_trans->orig_def_ext = -1;
  sm_trans->orig_def_prev_serial_op = NULL;

  sm_trans->deleted_lcode_op = NULL;

  sm_trans->new_sm_op = NULL;
  sm_trans->renaming_sm_op = NULL;
  sm_trans->first_queue = NULL;

  /* Return the newly created structure */
  return (sm_trans);
}

void
SM_delete_trans (SM_Trans * trans)
{
  if (trans->orig_src[0] != NULL)
    L_delete_operand (trans->orig_src[0]);
  if (trans->orig_src[1] != NULL)
    L_delete_operand (trans->orig_src[1]);

  if (trans->orig_def_src[0] != NULL)
    L_delete_operand (trans->orig_def_src[0]);
  if (trans->orig_def_src[1] != NULL)
    L_delete_operand (trans->orig_def_src[1]);

  /* Delete any "deleted" lcode opers that make it here */
  if (trans->deleted_lcode_op != NULL)
    L_delete_oper (NULL, trans->deleted_lcode_op);

  /* Just free it for now */
  L_free (SM_Trans_pool, trans);
}

void
SM_print_trans (FILE * out, SM_Trans * sm_trans)
{
  fprintf (out, "  Cb %i op %i: ", sm_trans->cb_id, sm_trans->op_id);
  switch (sm_trans->type)
    {
    case RENAMING_WITH_COPY:
      fprintf (out, "RENAMING_WITH_COPY");
      break;

    case EXPR_WITH_COPY:
      fprintf (out, "EXPR_WITH_COPY");
      break;

    default:
      fprintf (out, "(Unknown)");
    }
  fprintf (out, "\n");
}

/* 20030226 SZU
 * SMH reconciliation
 */
int
SM_CRC_defs_before_or_selected(SM_Oper *sm_op, Set bef_set, List sel_list)
{
  SM_Dep *dep_in;
  SM_Reg_Action *src_action;
  SM_Oper *dep_op;
  int i;

  for (i=0; i < L_max_src_operand; i++)
    {
      src_action = sm_op->src[i];
      if (!src_action)
	continue;
      for (dep_in = src_action->first_dep_in; dep_in != NULL;
	   dep_in = dep_in->next_dep_in)
	{
	  if (dep_in->ignore &&
	      !(dep_in->flags & (SM_FLOW_DEP)))
	    continue;
	  
	  dep_op = dep_in->from_action->sm_op;
	  if (!Set_in(bef_set, dep_op->lcode_op->id) &&
	      !List_member(sel_list, dep_op))
	    {
	      /*printf("Flow dep from op%d\n",dep_op->lcode_op->id);*/
	      return 0;
	    }
	}
    }
  for (i=0; i < 1; i++)
    {
      src_action = sm_op->pred[i];
      if (!src_action)
	continue;
      for (dep_in = src_action->first_dep_in; dep_in != NULL;
	   dep_in = dep_in->next_dep_in)
	{
	  if (dep_in->ignore ||
	      !(dep_in->flags & (SM_FLOW_DEP)))
	    continue;
	  
	  dep_op = dep_in->from_action->sm_op;
	  if (!Set_in(bef_set, dep_op->lcode_op->id) &&
	      !List_member(sel_list, dep_op))
	    {
	      /*printf("Flow dep from op%d\n",dep_op->lcode_op->id);*/
	      return 0;
	    }
	}
    }
  for (i=0; i < L_max_dest_operand; i++)
    {
      src_action = sm_op->dest[i];
      if (!src_action)
	continue;
      for (dep_in = src_action->first_dep_in; dep_in != NULL;
	   dep_in = dep_in->next_dep_in)
	{
	  if (dep_in->ignore &&
	      !(dep_in->flags & (SM_ANTI_DEP)))
	    continue;
	  
	  dep_op = dep_in->from_action->sm_op;
	  if (!Set_in(bef_set, dep_op->lcode_op->id) &&
	      !List_member(sel_list, dep_op))
	    {
	      /*printf("Anti dep from op%d\n",dep_op->lcode_op->id);*/
	      return 0;
	    }
	}
    }
  return 1;
}

int
SM_CRC_long_lat_between(SM_Oper *sm_op, SM_Oper* dep_op)
{
  L_Oper *op = NULL;
  
  for (op=sm_op->lcode_op; op; op=op->next_op)
    {
      /*
	if (L_is_control_oper(op))
	return 1;
      */
      if (L_flt_load_opcode(dep_op->lcode_op) ||
	  L_dbl_load_opcode(dep_op->lcode_op))
	return 1;
      if (op == dep_op->lcode_op)
	return 0;
    }
   
  /* If dep_op was not found, dep is around backedge
     thus "control flow" is between */
  return 1;
}

void
SM_CRC_add_valid_op(SM_Oper *sm_op, List *work_list, 
		    Set bef_set, List sel_list)
{
  SM_Dep *dep_out;
  SM_Reg_Action *dest_action;
  SM_Oper *dep_op;
  int i;

  for (i=0; i < L_max_dest_operand; i++)
    {
      dest_action = sm_op->dest[i];
      if (!dest_action)
	continue;
      for (dep_out = dest_action->first_dep_out; dep_out != NULL;
	   dep_out = dep_out->next_dep_out)
	{
	  if (dep_out->ignore ||
	      (!(dep_out->flags & SM_FLOW_DEP)))
	    continue;
	  
	  dep_op = dep_out->to_action->sm_op;
	  /* No compares, stores, or long latency loads */
	  if (L_flt_load_opcode(dep_op->lcode_op) ||
	      L_dbl_load_opcode(dep_op->lcode_op) ||
	      L_general_pred_comparison_opcode(dep_op->lcode_op) ||
	      L_general_store_opcode(dep_op->lcode_op))
	    {
	      /*printf ("NO CMP/ST/LD op%d\n",dep_op->lcode_op->id);*/
	      continue;
	    }
	  /* All operands must be "ready" or from other selected ops */
	  if (!SM_CRC_defs_before_or_selected(dep_op, 
					      bef_set, sel_list))
	    {
	      /*printf ("NO dep op%d\n",dep_op->lcode_op->id);*/
	      continue;
	    }
	  /* Don't include other predicated ops */
	  if (dep_op->lcode_op->pred[0])
	    {
	      /*printf ("NO pred op%d\n",dep_op->lcode_op->id);*/
	      continue;
	    }
	  /* Don't repeat selections */
	  if (List_member(sel_list, dep_op) ||
	      List_member(*work_list, dep_op) ||
	      dep_op == sm_op)
	    {
	      /*printf ("NO repeat op%d\n",dep_op->lcode_op->id);*/
	      continue;
	    }
#if 0
	  /* Don't include ops that follow other control flow */
	  if (SM_CRC_long_lat_between(sm_op, dep_op))
	    {
	      printf ("NO long lat bw op%d\n",dep_op->lcode_op->id);
	      continue;
	    }
#endif  
	  *work_list = List_insert_last(*work_list, dep_op);
	}      
    }  
}

SM_Trans *
SM_can_relocate_cond (SM_Oper * sm_op)
{
 SM_Dep *dep_in, *dep_out;
 SM_Reg_Action *dest_action, *src_action;
 int long_latency = 0;
 int all_moveable = 0;
 int i;
 Set bef_set = NULL;
 List sel_list = NULL;
 List work_list = NULL;
 L_Operand *guard = NULL;
 
 /*if (sm_op->lcode_op->id != 288 )
     && sm_op->lcode_op->id != 320)
     return (NULL);*/

 /* Only care about compares */
 if (!L_general_pred_comparison_opcode(sm_op->lcode_op))
   return (NULL);
 
 /* Is compare fed by a float/double load */
 long_latency = 0;
 for (i=0; i < L_max_src_operand; i++)
   {
     src_action = sm_op->src[i];
     if (!src_action)
       continue;
     for (dep_in = src_action->first_dep_in; dep_in != NULL;
	  dep_in = dep_in->next_dep_in)
       {
	 if ((!(dep_in->ignore)) &&
	     (dep_in->flags & (SM_FLOW_DEP)) &&
	     (L_flt_load_opcode(dep_in->from_action->sm_op->lcode_op) ||
	      L_dbl_load_opcode(dep_in->from_action->sm_op->lcode_op)))
	   {
	     long_latency = 1;
	     break;
	   }
       } 
     if (long_latency)
       break;
   }
 
 
 /* Is compare controlling an op 
    that is not a branch or store*/
 all_moveable = 1;
 for (i=0; i < L_max_dest_operand; i++)
   {
     dest_action = sm_op->dest[i];
     if (!dest_action)
       continue;
     for (dep_out = dest_action->first_dep_out; dep_out != NULL;
	  dep_out = dep_out->next_dep_out)
       {
	 if (dep_out->ignore ||
	     (!(dep_out->flags & SM_FLOW_DEP)))
	   continue;
	 if (L_is_control_oper(dep_out->to_action->sm_op->lcode_op) ||
	     L_general_store_opcode(dep_out->to_action->sm_op->lcode_op))
	   {
	     all_moveable = 0;
	   }
	 work_list = List_insert_last(work_list, dep_out->to_action->sm_op);
       }      
   }
 
 if (long_latency && all_moveable && 
     (List_size(work_list) == 1)) 
   {
     L_Oper *op = NULL;
     SM_Oper *cur_op = NULL;
     int old_size;
     
     for (op=sm_op->lcode_op; op; op=op->prev_op)
       {
	  bef_set = Set_add(bef_set, op->id);
       }
     List_start(work_list);
     cur_op = (SM_Oper*)List_next(work_list);
     guard = L_copy_operand(cur_op->lcode_op->pred[0]);
     if (!guard)
       L_punt("SM_CRC: guard is null\n");
     
     sel_list = NULL;
     old_size = 0;
     /* Given the seed ops generate an op list to duplicate */
     printf("Looking to relocate condition on ops:\n");
     do 
       {
	 List_start(work_list);
	 while((cur_op = (SM_Oper*)List_next(work_list)))
	   {
	     work_list = List_delete_current(work_list);
	     
	     if (!List_member(sel_list,cur_op))
	       {
		 printf("OP %d\n",cur_op->lcode_op->id);
		 sel_list = List_insert_last(sel_list, cur_op);
	       }
	      SM_CRC_add_valid_op(cur_op, &work_list, bef_set, sel_list);
	   }
	 
	 if (List_size(sel_list) > old_size)
	   {
	     for (List_start(sel_list);(cur_op=(SM_Oper*)List_next(sel_list));)
	       {
		 work_list = List_insert_last(work_list, cur_op);
	       }	      
	     old_size = List_size(sel_list);
	   }
	 else
	   break;
	 
       } while (1);
     
     List_reset(work_list);
      Set_dispose(bef_set);
   }

 if (long_latency && all_moveable)
   {
#if 0
     SM_Oper *cur_op = NULL;
#endif
     if (List_size(work_list) > 1)
       {
	 printf("Multiple seed ops(%d) not yet supported\n",List_size(work_list));
	 /*
	   List_start(work_list);
	   while((cur_op = (SM_Oper*)List_next(work_list)))
	   printf("SEED op%d\n",cur_op->lcode_op->id);
	 */
       }
     if (List_size(sel_list) <= 2)
       printf("Not performed, less than 3 ops were selected\n");
   }
 
 if (List_size(sel_list) > 2)
   {
     L_Oper *op = NULL;
     SM_Oper *cur_op = NULL;
     int old_max_reg = L_fn->max_reg_id;

     /* Delete guard off of seed op */
     {
       SM_Oper *new_op = NULL;
       L_Oper *tmp_op = NULL;

       List_start(sel_list);
       cur_op = (SM_Oper*)List_next(sel_list);
       if (!L_same_operand(guard, cur_op->lcode_op->pred[0]))
	 L_punt("SM_CRC: guard does not match\n");
       sel_list = List_delete_current(sel_list);
       
       op = L_copy_operation(cur_op->lcode_op);
       L_delete_operand(op->pred[0]);
       op->pred[0] = NULL;
       new_op = SM_insert_oper_after (cur_op->sm_cb, op, cur_op);

       /*printf("Deleting orig seed op%d\n",cur_op->lcode_op->id);*/
       tmp_op = cur_op->lcode_op;
       SM_delete_oper(cur_op);
       L_nullify_operation (tmp_op);
       sel_list = List_insert_first(sel_list, new_op);
     }

     /* Copy every op */
     for (List_start(sel_list);(cur_op=(SM_Oper*)List_next(sel_list));)
       {
	 L_Oper *tmp_op = NULL;
	 L_Attr *attr = NULL;
	 
	 /* Copy operation */
	 tmp_op = L_copy_operation(cur_op->lcode_op);
	 attr = L_new_attr ("CRC", 0);
	 tmp_op->attr = L_concat_attr (tmp_op->attr, attr);
	 cur_op->lcode_op->ext = tmp_op;	  
       }
     
     /* Rename all of the dests and appropriate srcs */
     for (List_start(sel_list);(cur_op=(SM_Oper*)List_next(sel_list));)
       {
	 L_Operand *new_reg = NULL;
	 L_Operand *old_reg = NULL;
	 L_Oper *tmp_op = NULL;
	 L_Oper *mov_op = NULL;
	 SM_Oper *iter_op = NULL;
	 L_Attr *attr = NULL;
	 SM_Action_Qentry *qentry = NULL;
	 int mov_needed;
	 int i,j;
	 
	 /*printf("Renaming Op %d dest\n",cur_op->lcode_op->id);*/
	 for (i=0; i<L_max_dest_operand; i++)
	   {
	     if (!cur_op->lcode_op->dest[i])
	       continue;
	     tmp_op = (L_Oper*)cur_op->lcode_op->ext;
	     if (tmp_op->dest[i]->value.r > old_max_reg)
	       continue;

	     /* Create the new register */
	     new_reg = L_new_register_operand(++L_fn->max_reg_id,
					      cur_op->lcode_op->dest[i]->ctype,
					      cur_op->lcode_op->dest[i]->ptype);
	     old_reg = L_copy_operand(cur_op->lcode_op->dest[i]);
	     /* Create predicated mov for last_def */
	     mov_op = L_create_new_op(Lop_MOV);
	     mov_op->proc_opc = 1035; /*TAHOEop_MOV_GR*/
	     mov_op->pred[0] = L_copy_operand(guard);
	     mov_op->src[0] = L_copy_operand(new_reg);
	     mov_op->dest[0] = L_copy_operand(old_reg);
	     attr = L_new_attr ("CRC", 0);
	     mov_op->attr = L_concat_attr (mov_op->attr, attr);

	     /* Rename the dest of this oper */
	     L_delete_operand(tmp_op->dest[i]);
	     tmp_op->dest[i] = L_copy_operand(new_reg);
	     mov_needed = 1;

	     for (iter_op=cur_op->next_serial_op; iter_op; iter_op=iter_op->next_serial_op)
	       {
		 if (!iter_op->lcode_op->ext)
		   {
		     if (mov_needed)
		       {
			 /*printf("- examining op%d: ",iter_op->lcode_op->id);*/
			 /* Is this a branch with old_reg live out */
			 if (L_is_control_oper(iter_op->lcode_op))
			   {
			     for (qentry = iter_op->implicit_srcs->first_qentry; qentry != NULL;
				  qentry = qentry->next_qentry)
			       {
				 if (!L_same_operand(qentry->action->rinfo->operand,
						     old_reg))
				   continue;
				 /*printf("- [r%d] op %d branch (mov needed)\n", 
				   old_reg->value.r, iter_op->lcode_op->id);*/
				 SM_insert_oper_after (cur_op->sm_cb, 
						       L_copy_operation(mov_op), 
						       iter_op->prev_serial_op);
				 mov_needed = 0;				 
			       }
			   }
			 /* Is this a op that has old_reg as a src */
			 for (j=0; j<L_max_src_operand; j++)
			   {
			     /*printf(" %d",j);*/
			     if (!iter_op->lcode_op->src[j] || 
				 !L_same_operand(iter_op->lcode_op->src[j], old_reg))
			       continue;
			     /*printf("- [r%d] op %d src %d (mov needed)\n", 
			       old_reg->value.r, iter_op->lcode_op->id, j);*/
			     SM_insert_oper_after (cur_op->sm_cb, 
						   L_copy_operation(mov_op), 
						   iter_op->prev_serial_op);
			     mov_needed = 0;
			   }
			 /*printf("\n");*/
		       }
		     continue;
		   }
		 tmp_op = (L_Oper*)iter_op->lcode_op->ext;
		 /* Rename dest/src of all following ops */
		 for (j=0; j<L_max_dest_operand; j++)
		   {
		     if (!tmp_op->dest[j] || 
			 !L_same_operand(tmp_op->dest[j], old_reg))
		       continue;
		     /*printf("- [r%d] op %d dest %d\n", old_reg->value.r, tmp_op->id, j);*/
		     L_delete_operand(tmp_op->dest[j]);
		     tmp_op->dest[j] = L_copy_operand(new_reg);
		     mov_needed = 1;
		    }
		 for (j=0; j<L_max_src_operand; j++)
		   {
		     if (!tmp_op->src[j] || 
			 !L_same_operand(tmp_op->src[j], old_reg))
		       continue;
		     /*printf("- [r%d] op %d src %d\n", old_reg->value.r, tmp_op->id, j);*/
		     L_delete_operand(tmp_op->src[j]);
		     tmp_op->src[j] = L_copy_operand(new_reg);
		   }		  
	       }
	     
	     if (mov_needed)
	       {
		 /* Put move at end of cb */
		 /*printf("- [r%d] last_op (mov needed)\n", 
		   old_reg->value.r);*/
		 SM_insert_oper_after (cur_op->sm_cb, 
				       L_copy_operation(mov_op), 
				       cur_op->sm_cb->last_serial_op);     
	       }
	     
	     L_delete_oper(cur_op->sm_cb->lcode_cb, mov_op);
	     L_delete_operand(new_reg);
	     L_delete_operand(old_reg);
	   }
       }
     
     /* Make SM_ops and put ops into lcode */
     for (List_start(sel_list);(cur_op=(SM_Oper*)List_next(sel_list));)
       {
	 L_Oper *tmp_op = NULL;
	 SM_Oper *new_op = NULL;

	 tmp_op = (L_Oper*)cur_op->lcode_op->ext;
	 cur_op->lcode_op->ext = NULL;
	 
	 /* Put into lcode */
	 new_op = SM_insert_oper_after (cur_op->sm_cb, tmp_op, cur_op);
       }

     /* Delete first op of non-dup path. 
      * Orig_path: assumes pred will be false
      * Dup_path: assumes pred will be true
      */
     {
       L_Oper *tmp_op = NULL;

       List_start(sel_list);
       cur_op = (SM_Oper*)List_next(sel_list);

       /*printf("Deleting new seed op%d\n",cur_op->lcode_op->id);*/
       tmp_op = cur_op->lcode_op;
       SM_delete_oper(cur_op);
       L_nullify_operation (tmp_op);
     }

     /*DB_spit_func(L_fn, "fn_1");*/
     L_delete_operand(guard);
     List_reset(sel_list);
   }
 
  return (NULL);
}

void
SM_do_relocate_cond_opti(SM_Cb * sm_cb) 
{
  SM_Oper *sm_op;

  for (sm_op = sm_cb->first_serial_op; sm_op != NULL;
       sm_op = sm_op->next_serial_op)
    {
      SM_can_relocate_cond(sm_op);
    }
}

/* Determines if the renaming with copy transformation could be performed
 * on this operation.  If it cannot, NULL is returned.  Otherwise,
 * a pointer to a new SM_Trans structure is returned with all the
 * information necessary to perform the transformation
 */
SM_Trans *
SM_can_rename_with_copy (SM_Oper * sm_op)
{
  SM_Dep *dep_in, *dep_out;
  SM_Reg_Action *dest_action;
  int explicit_use, breaks_dep;

  /* Is this operation the type of operation we want to apply 
   * renaming with copy to?  For simplicity, we will restrict
   * ourselves to int virtual registers in dest[0] for now.
   * 
   * Don't do the transformation if:
   * 1) If dest[0] is not a register.
   * 2) If dest[0] is not an int virtual register (not a macro).
   * 3) The operation is already a MOV (a different transformation is
   *    needed, namely copy propation).
   */
  dest_action = sm_op->dest[0];
  if ((dest_action == NULL) ||
      (!L_is_reg (dest_action->rinfo->operand)) ||
      (!L_is_ctype_integer (dest_action->rinfo->operand)) ||
      (sm_op->lcode_op->opc == Lop_MOV))
    {
      /* The transformation cannot be done, is of no benefit, or
       * we don't want to handle this case right now.
       */
      return (NULL);
    }

  /* If emulating the classic approach to renaming with copy,
   * the original heuristics prevented this transformation from
   * being applied if the dest register was also used as
   * a source in the operation.
   *
   * If use_classic_renaming_heuristics is set and the above
   * condition is true, do not consider this transformation for renaming.
   */
  if (use_classic_renaming_heuristics)
    {
      /* If there is a previous actual action and it is for the
       * same operation, then the dest is also used as a source
       * for this operation.  Return NULL to indicate that
       * classic heuristics would not allow this transformation.
       */
      if ((dest_action->prev_actual != NULL) &&
          (dest_action->prev_actual->sm_op == dest_action->sm_op))
        {
          return (NULL);
        }
    }

  /* Determine if there is any instruction that is dependent
   * on this operation's destination.
   * (Can do this only for flow dependences to explicit actions.
   *  Renaming will not help flow deps due to branch live-out.)
   */
  explicit_use = 0;
  for (dep_out = dest_action->first_dep_out; dep_out != NULL;
       dep_out = dep_out->next_dep_out)
    {
      /* Only look at flow dependences that are to explicit sources 
       * (cannot rename implicit, live-out register).  Also, ignore
       * dependences that are already IGNORED and SOFT deps.
       */
      /* 20030226 SZU
       * SMH reconciliation
       */
      if (!(dep_out->flags & SM_FLOW_DEP) ||
          (dep_out->to_action->flags & SM_IMPLICIT_ACTION) ||
          (dep_out->ignore) || (dep_out->flags & SM_SOFT_DEP))
        continue;
#if 0
      if (!(dep_out->flags & SM_FLOW_DEP) ||
          (dep_out->to_action->flags & SM_IMPLICIT_ACTION) ||
          (dep_out->ignore))
        continue;
#endif

      /* To reduce the scope of the problem, only consider uses of this
       * register where this is the only dependence into it.
       * (This is done efficiently by seeing if there is a dependence
       *  before or after it in the to_action's dep_in list.)
       *
       * This test will prevent this transformation where there are
       * multiple possible defs due to predication.
       * 
       * Also, cannot change an usage if it is not for the same
       * register (namely a conflicting register).
       */
      if ((dep_out->prev_dep_in != NULL) ||
          (dep_out->next_dep_in != NULL) ||
          (dep_out->to_action->rinfo != dest_action->rinfo))
        continue;

      /* If got here, there is at least one explicit use of
       * the destination that could benefit from the transformation.
       */
      explicit_use = 1;
      break;
    }

  /* 
   * If there is no explict use, there is no reason to do the transformation
   */
  if (explicit_use == 0)
    {
      return (NULL);
    }

  /* Now determine that that there is at least one anti or output
   * dependence coming into dest[0] that would be broken by
   * this transformation.
   */
  breaks_dep = 0;
  for (dep_in = dest_action->first_dep_in; dep_in != NULL;
       dep_in = dep_in->next_dep_in)
    {
      if ((!(dep_in->ignore)) &&
          (dep_in->flags & (SM_ANTI_DEP | SM_OUTPUT_DEP)))
        {
          /* There is at least one dep that will be broken by renaming */
          breaks_dep = 1;
          break;
        }
    }

  /* If there are not deps broken by this transformation, don't do it */
  if (breaks_dep == 0)
    {
      return (NULL);
    }

  /* Ok, it is a renaming with copy transformation candidate, return
   * a trans structure for it.
   */
  return (SM_new_trans (RENAMING_WITH_COPY, sm_op, 0));
}

/* Returns 1 if the register sources of from_op are still available
 * at the to_op operation.  
 */
int
SM_expr_srcs_available (SM_Oper * from_op, SM_Oper * to_op)
{
  SM_Reg_Action *next_def;
  int available;
  int i;

  /* Assume that they are available unless discover otherwise */
  available = 1;

  /* Scan src operands of from_op, and make sure that they
   * are not redefined before to_op.
   */
  for (i = 0; i <= 1; i++)
    {
      /* Non-register operands are always available */
      if (from_op->src[i] == NULL)
        continue;

      /* If there is no later def, it is available */
      if ((next_def = from_op->src[i]->next_def) == NULL)
        continue;

      /* If the next def occurs after to_op (or by to_op), it is available */
      if (next_def->sm_op->serial_number >= to_op->serial_number)
        continue;

      /* If got here it is not available */
      available = 0;
      break;
    }

  return (available);
}

int
SM_can_move_before_def (SM_Oper * def_op, SM_Oper * use_op,
                        SM_Reg_Action * src)
{
  SM_Reg_Action *action;
  SM_Dep *dep_in;
  unsigned int def_serial_number;
  int can_move;

  /* Only allow add/sub/ld_i operations to move for now */
  if ((use_op->lcode_op->proc_opc != Lop_ADD) &&
      (use_op->lcode_op->proc_opc != Lop_SUB) &&
      (use_op->lcode_op->opc != Lop_LD_I) &&
      (use_op->lcode_op->opc != Lop_LD_UC) &&
      (use_op->lcode_op->opc != Lop_LD_C) &&
      (use_op->lcode_op->opc != Lop_LD_UC2) &&
      (use_op->lcode_op->opc != Lop_LD_C2))
    return (0);

  /* Assume that it can be moved before unless find out otherwise */
  can_move = 1;

  /* Get the defining operation's serial number for ease of use */
  def_serial_number = def_op->serial_number;

  /* Scan all the incoming dependences of use_op, except for 
   * those associated with the 'src' (defined by def_op).  If
   * any of the from_op's are from after def_op, cannot move
   * above.
   */
  for (action = use_op->first_op_action; action != NULL;
       action = action->next_op_action)
    {
      /* Skip the action associated with 'src' */
      if (action == src)
        continue;

      /* Test each dep in for serial numbers >= def's serial number.
       * If this occurs, cannot move operation above def for expr_with_copy.
       */
      for (dep_in = action->first_dep_in; dep_in != NULL;
           dep_in = dep_in->next_dep_in)
        {
          /* Skip dep's marked with SM_IGNORE_DEP */
          if (dep_in->ignore)
            continue;

          if (dep_in->from_action->sm_op->serial_number >= def_serial_number)
            {
              can_move = 0;
              break;
            }
        }

      /* If we cannot move now, stop */
      if (can_move == 0)
        break;
    }

  return (can_move);
}

/* If dest[0] is renamed, can we move use_op before def_op? */
int
SM_can_move_before_def_with_renaming (SM_Oper * def_op, SM_Oper * use_op,
                                      SM_Reg_Action * src)
{
  SM_Reg_Action *action, *dest_action;
  SM_Dep *dep_in;
  unsigned int def_serial_number;
  int can_move;

  /* Only allow add/sub/ld_i operations to move for now */
  if ((use_op->lcode_op->proc_opc != Lop_ADD) &&
      (use_op->lcode_op->proc_opc != Lop_SUB) &&
      (use_op->lcode_op->opc != Lop_LD_I) &&
      (use_op->lcode_op->opc != Lop_LD_UC) &&
      (use_op->lcode_op->opc != Lop_LD_C) &&
      (use_op->lcode_op->opc != Lop_LD_UC2) &&
      (use_op->lcode_op->opc != Lop_LD_C2))
    return (0);

  /* Make sure renaming works on this type of dest operand.
   * For now, require an integer register destination.
   */
  dest_action = use_op->dest[0];
  if ((dest_action == NULL) ||
      (!L_is_reg (dest_action->rinfo->operand)) ||
      (!L_is_ctype_integer (dest_action->rinfo->operand)))
    return (0);


  /* Assume that it can be moved before unless find out otherwise */
  can_move = 1;

  /* Get the defining operation's serial number for ease of use */
  def_serial_number = def_op->serial_number;

  /* Scan all the incoming dependences of use_op, except for 
   * those associated with the 'src' (defined by def_op).  If
   * any of the from_op's are from after def_op, cannot move
   * above.
   */
  for (action = use_op->first_op_action; action != NULL;
       action = action->next_op_action)
    {
      /* Skip the action associated with 'src' */
      if (action == src)
        continue;

      /* Skip the action associated with dest[0] (assumes renaming
       * will take care of it.
       */
      if ((action->operand_type == MDES_DEST) &&
          (action->operand_number == 0))
        continue;


      /* Test each dep in for serial numbers >= def's serial number.
       * If this occurs, cannot move operation above def for expr_with_copy.
       */
      for (dep_in = action->first_dep_in; dep_in != NULL;
           dep_in = dep_in->next_dep_in)
        {
          /* Skip dep's marked with SM_IGNORE_DEP */
          if (dep_in->ignore)
            continue;

          if (dep_in->from_action->sm_op->serial_number >= def_serial_number)
            {
              can_move = 0;
              break;
            }
        }

      /* If we cannot move now, stop */
      if (can_move == 0)
        break;
    }

  return (can_move);
}

/* Returns 1 if const will fit in the int field for this operation
 * (assuming SM_make_valid_mcode changes it appropriately).
 */
int
SM_int_field_large_enough (int opc, int proc_opc, ITintmax int_val)
{
  /* Handle hppa variations */
  if ((M_arch == M_HPPA) ||
      ((M_arch == M_IMPACT) && (M_model == M_HP_PA_7100)))
    {
      /* Decide based on opc for now */
      switch (opc)
        {
        case Lop_MOV:
        case Lop_ADD:
        case Lop_LD_UC:
        case Lop_LD_C:
        case Lop_LD_UC2:
        case Lop_LD_C2:
        case Lop_LD_I:
          if (FIELD_14 (int_val))
            return (1);
          else
            return (0);

        case Lop_SUB:
          if (FIELD_11 (int_val))
            return (1);
          else
            return (0);

        case Lop_BEQ:
        case Lop_BEQ_FS:
        case Lop_BNE:
        case Lop_BNE_FS:
        case Lop_BGT:
        case Lop_BGT_FS:
        case Lop_BGE:
        case Lop_BGE_FS:
        case Lop_BLT:
        case Lop_BLT_FS:
        case Lop_BLE:
        case Lop_BLE_FS:
          if (FIELD_5 (int_val))
            return (1);
          else
            return (0);

        default:
          L_punt ("SM_int_field_large_enough: opc %i not supported.", opc);
        }

    }
  else
    {
      L_punt ("SM_int_field_large_enough: M_arch = %i not supported.",
              M_arch);
    }

  L_punt ("SM_int-field_large_enough: should not get here!");
  return (0);
}


/* Returns 1 if this operation can be in the use portion of an EBWC
 * transformation.  Otherwise returns 0.
 */
int
SM_is_valid_expr_use (SM_Oper * sm_op)
{
  L_Oper *lcode_op;
  L_Operand *operand;
  int num_srcs, i;

  /* Handle hppa variations */
  if ((M_arch == M_HPPA) ||
      ((M_arch == M_IMPACT) && (M_model == M_HP_PA_7100)))
    {
      /* Get the lcode_op for ease of use */
      lcode_op = sm_op->lcode_op;

      /* Ignore processor specific opcodes */
      if (lcode_op->flags & L_OPER_PROCESSOR_SPECIFIC)
        return (0);

      /* Determine if this is a use operation we support */
      switch (sm_op->lcode_op->opc)
        {
        case Lop_ADD:
        case Lop_SUB:

          /* For add/sub, require proc_opc to be the same as opc unless
           * proc_opc is LHPPAop_LDO on a hppa
           */
          if ((lcode_op->opc != lcode_op->proc_opc) &&
              (lcode_op->proc_opc != LHPPAop_LDO))
            {
              return (0);
            }

          /* Expect exactly two sources */
          num_srcs = 2;
          break;

        case Lop_LD_UC:
        case Lop_LD_C:
        case Lop_LD_UC2:
        case Lop_LD_C2:
        case Lop_LD_I:
          /* Expect exactly two sources */
          num_srcs = 2;
          break;

        case Lop_BEQ:
        case Lop_BEQ_FS:
        case Lop_BNE:
        case Lop_BNE_FS:

          /* >, >=, <, and <= are unsafe and seem to break compress ! */
#if 0
        case Lop_BGT:
        case Lop_BGT_FS:
        case Lop_BGE:
        case Lop_BGE_FS:
        case Lop_BLT:
        case Lop_BLT_FS:
        case Lop_BLE:
        case Lop_BLE_FS:
#endif
          /* Expect exactly three sources */
          num_srcs = 3;
          break;

        default:
          return (0);
        }

      /* Return NULL, if there are unexpected operands.  This means
       * we don't have a "normal" version of this operation.
       */
      for (i = L_max_src_operand; i >= num_srcs; i--)
        {
          if (lcode_op->src[i] != NULL)
            return (0);
        }

      /* Return NULL, if the first two src operands are not an integer or
       * a register (I.e., don't support labels, strings, etc.)
       */
      for (i = 0; i < 2; i++)
        {
          operand = lcode_op->src[i];

          /* Great if operand is a register */
          if (L_is_variable (operand))
            continue;

          /* If a integer operation, make sure it fits into what we
           * can support during a transformation.
           */
          else if (L_is_int_constant (operand))
            {
              if (!SM_int_field_large_enough (lcode_op->opc,
                                              lcode_op->proc_opc,
                                              operand->value.i))
                {
                  return (0);
                }

              /* Otherwise, Ok */
              continue;
            }

          /* If got here, we don't support this operand */
          return (0);
        }

      /* If got here must be ok */
      return (1);
    }
  else
    {
      L_punt ("SM_is_valid_expr_use: M_arch = %i not supported.", M_arch);
    }

  L_punt ("SM_is_valid_expr_use: should not get here!");
  return (0);
}
/* Returns 1 if this operation can be in the def portion of an EBWC
 * transformation.  Otherwise returns 0.
 *
 * If do_expr_without_copy_only is set, this only returns 1 if
 * this def has exactly one use (it does not need to be copied).
 */
int
SM_is_valid_expr_def (SM_Oper * sm_op)
{
  L_Oper *lcode_op;
  L_Operand *operand;
  int num_srcs, i;

  /* Handle hppa variations */
  if ((M_arch == M_HPPA) ||
      ((M_arch == M_IMPACT) && (M_model == M_HP_PA_7100)))
    {
      /* Get the lcode_op for ease of use */
      lcode_op = sm_op->lcode_op;

      /* Ignore processor specific opcodes */
      if (lcode_op->flags & L_OPER_PROCESSOR_SPECIFIC)
        return (0);

      /* Determine if this is a use operation we support */
      switch (sm_op->lcode_op->opc)
        {
        case Lop_ADD:
        case Lop_SUB:

          /* For add/sub, require proc_opc to be the same as opc unless
           * proc_opc is LHPPAop_LDO on a hppa
           */
          if ((lcode_op->opc != lcode_op->proc_opc) &&
              (lcode_op->proc_opc != LHPPAop_LDO))
            {
              return (0);
            }

          /* Expect exactly two sources */
          num_srcs = 2;
          break;

        default:
          return (0);
        }

      /* Return NULL, if there are unexpected operands.  This means
       * we don't have a "normal" version of this operation.
       */
      for (i = L_max_src_operand; i >= num_srcs; i--)
        {
          if (lcode_op->src[i] != NULL)
            return (0);
        }

      /* Return NULL, if the first two src operands are not an integer or
       * a register (I.e., don't support labels, strings, etc.)
       */
      for (i = 0; i < 2; i++)
        {
          operand = lcode_op->src[i];

          /* Great if operand is a register */
          if (L_is_variable (operand))
            continue;

          /* If a integer operation, make sure it fits into what we
           * can support during a transformation.
           */
          else if (L_is_int_constant (operand))
            {
              if (!SM_int_field_large_enough (lcode_op->opc,
                                              lcode_op->proc_opc,
                                              operand->value.i))
                {
                  return (0);
                }

              /* Otherwise, Ok */
              continue;
            }

          /* If got here, we don't support this operand */
          return (0);
        }
    }
  else
    {
      L_punt ("SM_is_valid_expr_def: M_arch = %i not supported.", M_arch);
    }

  /* If doing expr_without_copy only, only allow this def
   * if its dest[0]  has exactly one use.
   */
  if (do_expr_without_copy_only)
    {
      if (!SM_def_has_exactly_one_use (sm_op->dest[0]))
        return (0);
    }

  /* Must be ok if got here */
  return (1);
}


SM_Trans *
SM_can_expr_with_copy (SM_Oper * sm_op)
{
  SM_Reg_Action *action;
  SM_Dep *dep_in;
  SM_Oper *from_op;
  SM_Trans *trans;
  unsigned int flags;

  /* Make sure it is a valid use for an expr_with_copy */
  if (!SM_is_valid_expr_use (sm_op))
    return (NULL);

  flags = 0;

  /* Check src[0] for a register action that is defined by exactly on
   * operation, and that operation is a add or sub.
   */
  if ((action = sm_op->src[0]) != NULL)
    {
      /* Make sure there is exactly one dep_in to this source */
      dep_in = action->first_dep_in;
      if ((dep_in != NULL) && (dep_in->next_dep_in == NULL))
        {
          /* Get the operation this dep is from */
          from_op = dep_in->from_action->sm_op;

          /* Is it an valid def for expr_with_copy ? */
          if (SM_is_valid_expr_def (from_op))
            {
              /* Are from_op's sources available at sm_op? */
              if (SM_expr_srcs_available (from_op, sm_op))
                {
                  /* Yes, can transform this source */
                  flags |= SM_CAN_TRANS_SRC0;
                }

              /* If not, is it possible to move sm_op to just
               * before from_op, if expr_with_copy is done?
               */
              else if (SM_can_move_before_def (from_op, sm_op, sm_op->src[0]))
                {
                  /* Yes, can transform this source */
                  flags |= SM_CAN_TRANS_SRC0 | SM_MUST_REORDER_OPS;
                }

              /* If not, is it possible to move sm_op to just
               * before from_op if renaming with copy is
               * done on sm_op, if expr_with_copy is done?
               *
               * Prevent this option if do_expr_without_copy_only is set.
               */
              else if ((do_expr_without_copy_only == 0) &&
                       SM_can_move_before_def_with_renaming (from_op, sm_op,
                                                             sm_op->src[0]))
                {
                  /* Yes, can transform this source */
                  flags |= SM_CAN_TRANS_SRC0 | SM_MUST_REORDER_OPS |
                    SM_NEEDS_RENAMING;
                }
            }
        }
    }

  /* Check src[1] for a register action that is defined by exactly on
   * operation, and that operation is a add or sub.
   */
  if ((action = sm_op->src[1]) != NULL)
    {
      /* Make sure there is exactly one dep_in to this source */
      dep_in = action->first_dep_in;
      if ((dep_in != NULL) && (dep_in->next_dep_in == NULL))
        {
          /* Get the operation this dep is from */
          from_op = dep_in->from_action->sm_op;

          /* Is it an valid def for expr_with_copy ? */
          if (SM_is_valid_expr_def (from_op))
            {
              /* Are from_op's sources available at sm_op? */
              if (SM_expr_srcs_available (from_op, sm_op))
                {
                  /* Yes, can transform this source */
                  flags |= SM_CAN_TRANS_SRC1;
                }

              /* If not, is it possible to move sm_op to just
               * before from_op, if expr_with_copy is done?
               */
              else if (SM_can_move_before_def (from_op, sm_op, sm_op->src[1]))
                {
                  /* Yes, can transform this source */
                  flags |= SM_CAN_TRANS_SRC1 | SM_MUST_REORDER_OPS;
                }

              /* If not, is it possible to move sm_op to just
               * before from_op if renaming with copy is
               * done on sm_op, if expr_with_copy is done?
               *
               * Prevent this option if do_expr_without_copy_only is set.
               */
              else if ((do_expr_without_copy_only == 0) &&
                       SM_can_move_before_def_with_renaming (from_op, sm_op,
                                                             sm_op->src[1]))
                {
                  /* Yes, can transform this source */
                  flags |= SM_CAN_TRANS_SRC1 | SM_MUST_REORDER_OPS |
                    SM_NEEDS_RENAMING;
                }
            }
        }
    }

  /* If cannot transform at least one source, return NULL */
  if (flags == 0)
    return (NULL);

  /* Ok, it is a expr with copy transformation candidate, return
   * a trans structure for it.
   */
  trans = SM_new_trans (EXPR_WITH_COPY, sm_op, flags);

  return (trans);
}


/* Perform a trival pass of local renaming to catch those simple renaming
 * opportunities that phase1 of the codegenerator insert.
 *
 * This routine is not trying to get all the corner cases.
 * It will rename a dest only if:
 * 1) The register is an int virtual register.
 * 2) There is another def of the same register later in the cb.
 * 3) The register does not have any implicit uses.
 * 4) Only looks at dest[0] for now.
 */
void
SM_do_trivial_renaming (SM_Cb * sm_cb)
{
  SM_Oper *sm_op;
  SM_Reg_Action *dest_action;
  L_Func *lcode_fn;
  L_Operand *temp_reg;
  SM_Dep *dep_out, *next_dep_out;
  int can_rename;

  for (sm_op = sm_cb->first_serial_op; sm_op != NULL;
       sm_op = sm_op->next_serial_op)
    {
      /* Only process ops with an int virtual register for dest[0] */
      if (((dest_action = sm_op->dest[0]) == NULL) ||
          (!L_is_reg (dest_action->rinfo->operand)) ||
          (!L_is_ctype_integer (dest_action->rinfo->operand)))
        continue;

      /* Only consider renaming if there is another def in this register */
      if (dest_action->next_def == NULL)
        continue;

      /* Initially assume that we can rename this register */
      can_rename = 1;

      /* If find a reg flow dep out of this action to an implicit
       * action (usually means live out of a branch), we cannot rename it.
       */
      for (dep_out = dest_action->first_dep_out; dep_out != NULL;
           dep_out = dep_out->next_dep_out)
        {
          /* Only look at flow dependences that are not ignored */
          if (!(dep_out->flags & SM_FLOW_DEP) || (dep_out->ignore))
            continue;

          /* If flow dep is to an implicit action, we cannot rename */
          if (dep_out->to_action->flags & SM_IMPLICIT_ACTION)
            {
              can_rename = 0;
              break;
            }

          /* To reduce the scope of the problem, only consider uses of this
           * register where this is the only dependence into it.
           * (This is done efficiently by seeing if there is a dependence
           *  before or after it in the to_action's dep_in list.)
           *
           * This test will prevent this transformation where there are
           * multiple possible defs due to predication.
           *
           * Also, cannot change an usage if it is not for the same
           * register (namely a conflicting register).
           */
          if ((dep_out->prev_dep_in != NULL) ||
              (dep_out->next_dep_in != NULL) ||
              (dep_out->to_action->rinfo != dest_action->rinfo))
            {
              can_rename = 0;
              break;
            }

        }

      /* If we cannot rename, goto next op */
      if (!can_rename)
        continue;

      if (verbose_optimization)
        {
          printf ("\nPerforming TRIVAL RENAMING on %s cb %i:\n",
                  sm_cb->lcode_fn->name, sm_cb->lcode_cb->id);
          printf ("  ==>");
          SM_print_minimal_lcode_op (stdout, sm_op);

        }
      /* Create a new int temp register to rename to */
      lcode_fn = sm_cb->lcode_fn;
      /* 20030226 SZU
       * SMH reconciliation
       */
      temp_reg = L_new_register_operand (++lcode_fn->max_reg_id, L_CTYPE_LLONG,
                                         L_PTYPE_NULL);
#if 0
      temp_reg = L_new_register_operand (++lcode_fn->max_reg_id, L_CTYPE_INT,
                                         L_PTYPE_NULL);
#endif


      /* Change all uses of the original register to the new register.
       * For now, only do this if there are no other dependences into
       * that use.
       */
      for (dep_out = dest_action->first_dep_out; dep_out != NULL;
           dep_out = next_dep_out)
        {
          /* Get the next dep out before doing anything to this dep out */
          next_dep_out = dep_out->next_dep_out;

          /* Only change operands that are explicit uses of this
           * exact register, and have no other dependences into them.
           */
          if ((dep_out->flags & SM_FLOW_DEP) &&
              (dep_out->to_action->flags & SM_EXPLICIT_ACTION) &&
              (!(dep_out->ignore)) &&
              (dep_out->from_action->rinfo == dep_out->to_action->rinfo) &&
              (dep_out->to_action->first_dep_in == dep_out) &&
              (dep_out->to_action->first_dep_in->next_dep_in == NULL))
            {
              /* Sanity check, to action better be a source */
              if (dep_out->to_action->operand_type != MDES_SRC)
                L_punt ("SM_do_trivial_renaming: src operand expected!");

              if (verbose_optimization)
                {
                  printf ("  ==>");
                  SM_print_minimal_lcode_op (stdout,
                                             dep_out->to_action->sm_op);
                }

              /* Replace this source operand with the new operand in to_op
               * and in underling lcode_op.
               *
               * Makes copy of temp reg and updates all deps, etc.
               */
              SM_change_operand (dep_out->to_action->sm_op, MDES_SRC,
                                 dep_out->to_action->operand_number,
                                 temp_reg);
            }
        }

      /* Change dest of the sm_op to the temp register 
       * Must change after changing uses so can use reg flow deps
       * to find uses.
       */
      SM_change_operand (sm_op, MDES_DEST, 0, temp_reg);
    }
}

/* Performs renaming with copy on the sm_op specified in sm_trans.
 * Updates all the dependences, bounds, etc., so that this
 * transformation can be done during scheduling and/or without
 * rebuilding the dependence graph.
 */
void
SM_do_renaming_with_copy (SM_Trans * sm_trans)
{
  SM_Oper *target_sm_op, *renaming_sm_op;
  SM_Dep *dep_out, *next_dep_out;
  L_Func *lcode_fn;
  L_Operand *temp_reg;
  L_Oper *new_lcode_op, *target_lcode_op;

  /* Sanity check, sm_trans better be of proper type
   * (or be another transformation that needed renaming done).
   */
  if ((sm_trans->type != RENAMING_WITH_COPY) &&
      !(sm_trans->flags & SM_NEEDS_RENAMING))
    {
      L_punt ("SM_do_renaming_with_copy: Unexpected trans type %i!",
              sm_trans->type);
    }

  /* Get the target op for ease of use */
  target_sm_op = sm_trans->target_sm_op;

  /* Sanity check, make sure have an integer dest */
  if ((target_sm_op->dest[0] == NULL) ||
      (!L_is_reg (target_sm_op->dest[0]->rinfo->operand)) ||
      (!L_is_ctype_integer (target_sm_op->dest[0]->rinfo->operand)))
    {
      fprintf (stderr, "Error in %s cb %i:\n",
               target_sm_op->sm_cb->lcode_fn->name,
               target_sm_op->sm_cb->lcode_cb->id);
      SM_print_minimal_lcode_op (stderr, target_sm_op);
      L_punt ("SM_do_renaming_with_copy: Bad dest[0]!");
    }

  /* Create a new int temp register to hold the restuls before
   * the copy */
  lcode_fn = target_sm_op->sm_cb->lcode_fn;
  /* 20030226 SZU
   * SMH reconciliation
   * Looks like Itanium only
   */
  temp_reg = L_new_register_operand (++lcode_fn->max_reg_id, L_CTYPE_LLONG,
                                     L_PTYPE_NULL);
#if 0
  temp_reg = L_new_register_operand (++lcode_fn->max_reg_id, L_CTYPE_INT,
                                     L_PTYPE_NULL);
#endif

  /* Change all uses of the original register to the new register.
   * For now, only do this if there are no other dependences into
   * that use.
   */
  for (dep_out = target_sm_op->dest[0]->first_dep_out;
       dep_out != NULL; dep_out = next_dep_out)
    {
      /* Get the next dep out before doing anything to this dep out */
      next_dep_out = dep_out->next_dep_out;

      /* Only change operands that are explicit uses of this
       * exact register, and have no other dependences into them.
       */
      if ((dep_out->flags & SM_FLOW_DEP) &&
          (dep_out->to_action->flags & SM_EXPLICIT_ACTION) &&
          (!(dep_out->ignore)) &&
          (dep_out->from_action->rinfo == dep_out->to_action->rinfo) &&
          (dep_out->to_action->first_dep_in == dep_out) &&
          (dep_out->to_action->first_dep_in->next_dep_in == NULL))
        {
          /* Sanity check, to action better be a source */
          if (dep_out->to_action->operand_type != MDES_SRC)
            L_punt ("SM_do_renaming_with_copy: src operand expected!");

	  /* 20030226 SZU
	   * SMH reconciliation
	   */
	  if (!PG_superset_predicate_ops (target_sm_op->lcode_op, 
					  dep_out->to_action->sm_op->lcode_op))
	    continue;

          /* Replace this source operand with the new operand in to_op
           * and in underling lcode_op.
           *
           * Makes copy of temp reg and updates all deps, etc.
           */
          SM_change_operand (dep_out->to_action->sm_op, MDES_SRC,
                             dep_out->to_action->operand_number, temp_reg);
        }
    }

  /* Get the target lcode operation */
  target_lcode_op = target_sm_op->lcode_op;

  /* Create new lcode mov operation */
  new_lcode_op = L_create_new_op (Lop_MOV);

  /* 20031024 SZU
   * Final merging of SM. Need to modify move instruction by architecture.
   */
  if (M_arch == M_TAHOE)
    new_lcode_op->proc_opc = 1035; /* TAHOEop_MOV_GR */

  new_lcode_op->pred[0] = L_copy_operand (target_lcode_op->pred[0]);
  new_lcode_op->pred[1] = L_copy_operand (target_lcode_op->pred[1]);

  /* Copy the old dest to the new mov */
  new_lcode_op->dest[0] = L_copy_operand (target_lcode_op->dest[0]);

  /* Set the mov's source to the temp_reg allocated above */
  new_lcode_op->src[0] = temp_reg;

  /* Change dest of the target_sm_op to the temp register and update
   * deps, etc.
   */
  SM_change_operand (target_sm_op, MDES_DEST, 0, temp_reg);

  /* Update bounds on original operation so it can be scheduled earlier */
  SM_recalculate_lower_bound (target_sm_op);
  SM_recalculate_upper_bound (target_sm_op);

  /* Insert the move right after the original operation in both
   * the lcode cb and the sm cb.
   * Should create all the necessary dependences to and from this operation
   */
  renaming_sm_op = SM_insert_oper_after (target_sm_op->sm_cb, new_lcode_op,
                                         target_sm_op);

  /* Record the operation created by this transformation so
   * that we can undo the transformation
   */
  sm_trans->renaming_sm_op = renaming_sm_op;
}

/* Undoes renaming with copy on the sm_op specified in sm_trans.
 * Updates all the dependences, bounds, etc., so that this
 * transformation can be undone during scheduling and/or without
 * rebuilding the dependence graph.
 */
void
SM_undo_renaming_with_copy (SM_Trans * sm_trans)
{
  SM_Oper *target_sm_op;
  SM_Dep *dep_out, *next_dep_out;
  L_Operand *orig_reg;
  L_Oper *new_lcode_op;


  /* Sanity check, sm_trans better be of proper type 
   * (or be another transformation that needed renaming done)
   * and renaming_sm_op better not be NULL
   */
  if ((sm_trans->type != RENAMING_WITH_COPY) &&
      !(sm_trans->flags & SM_NEEDS_RENAMING))
    {
      L_punt ("SM_undo_renaming_with_copy: Unexpected trans type %i!",
              sm_trans->type);
    }

  if (sm_trans->renaming_sm_op == NULL)
    {
      L_punt ("SM_undo_renaming_with_copy: renaming_sm_op NULL!");
    }

  /* Get the target and new ops for ease of use */
  target_sm_op = sm_trans->target_sm_op;
  new_lcode_op = sm_trans->renaming_sm_op->lcode_op;

  /* Get the original register that was used */
  orig_reg = new_lcode_op->dest[0];

  /* Change all uses of the temp register back to the original register. */
  for (dep_out = target_sm_op->dest[0]->first_dep_out;
       dep_out != NULL; dep_out = next_dep_out)
    {
      /* Get the next dep out before doing anything to this dep out */
      next_dep_out = dep_out->next_dep_out;

      /* Sanity check, to action better be a source */
      if (dep_out->to_action->operand_type != MDES_SRC)
        L_punt ("SM_undo_renaming_with_copy: src operand expected!");

      /* Change the temp register back to the orig operand.
       * This changes both lcode and sm structures and rebuilds
       * all dependences, etc.
       */
      SM_change_operand (dep_out->to_action->sm_op, MDES_SRC,
                         dep_out->to_action->operand_number, orig_reg);
    }

  /* Delete the sm_op for the move operation before changing the
   * operand so the dependences will be drawn properly.
   */
  SM_delete_oper (sm_trans->renaming_sm_op);

  /* Must change target op's dest back to the original register 
   * right after deleting the mov operation.  Otherwise, 
   * improper dependences from earlier defs may be drawn, which
   * will screw of later optis, since there will be multiple flows
   * to the same source.
   */
  SM_change_operand (target_sm_op, MDES_DEST, 0, orig_reg);

  /* Delete the new_lcode_op after changing operand so that
   * orig_reg will not be deleted until after last use
   */
  L_delete_oper (target_sm_op->sm_cb->lcode_cb, new_lcode_op);

  /* Mark that there is no new op now */
  sm_trans->renaming_sm_op = NULL;

  /* Update bounds on original operation */
  SM_recalculate_lower_bound (target_sm_op);
  SM_recalculate_upper_bound (target_sm_op);
}

void
SM_print_expr_with_copy (FILE * out, SM_Trans * trans)
{
  SM_Oper *def_op, *target_op;
  int i;

  target_op = trans->target_sm_op;

  printf ("\nEXPR_WITH_COPY %s cb %i:\n",
          target_op->sm_cb->lcode_fn->name, target_op->sm_cb->lcode_cb->id);

  for (i = 0; i <= 1; i++)
    {
      /* Find def_op, if any */
      def_op = NULL;
      switch (i)
        {
        case 0:
          if (trans->flags & SM_CAN_TRANS_SRC0)
            def_op = target_op->src[0]->prev_def->sm_op;
          break;

        case 1:
          if (trans->flags & SM_CAN_TRANS_SRC1)
            def_op = target_op->src[1]->prev_def->sm_op;
          break;

        default:
          L_punt ("SM_print_expr_with_copy: Unsupported index %i", i);
        }

      /* If no def_op, continue */
      if (def_op == NULL)
        continue;

      printf ("  Def src[%i](%.0f): ", i, def_op->lcode_op->weight);

      if (def_op->flags & SM_OP_SCHEDULED)
        {
          printf ("%i %i,%i ", def_op->sched_cycle,
                  SM_calc_action_avail_time (def_op->src[0]),
                  SM_calc_action_avail_time (def_op->src[1]));
        }
      SM_print_minimal_lcode_op (stdout, def_op);
    }
  printf ("  Use(%.0f): ", target_op->lcode_op->weight);

  if (trans->flags & SM_MUST_REORDER_OPS)
    printf ("<REORDER> ");

  if (trans->flags & SM_NEEDS_RENAMING)
    printf ("<NEEDS RENAMING> ");

  if (target_op->flags & SM_OP_SCHEDULED)
    {
      printf ("%i %i,%i ", target_op->sched_cycle,
              SM_calc_action_avail_time (target_op->src[0]),
              SM_calc_action_avail_time (target_op->src[1]));
    }
  SM_print_minimal_lcode_op (stdout, target_op);
}

/* If ext != 0, sets pext attribute to ext.  Otherwise,
 * deletes pext attribute.
 */
void
SM_set_ext (SM_Oper * sm_op, ITintmax ext)
{
  L_Oper *lcode_op;
  L_Attr *attr;


  /* Get the lcode_op for ease of use */
  lcode_op = sm_op->lcode_op;

  /* If ext == 0, delete any existing pext atttibute */
  if (ext == 0)
    {
      /* If pext attribute exists, delete it */
      if ((attr = L_find_attr (lcode_op->attr, "pext")) != NULL)
        {
          lcode_op->attr = L_delete_attr (lcode_op->attr, attr);
        }
    }

  /* Otherwise, set pext attribute to ext */
  else
    {
      /* If pext attribute already exists, just change value */
      if ((attr = L_find_attr (lcode_op->attr, "pext")) != 0)
        attr->field[0]->value.i = ext;

      /* Otherwise, create new pext attribute */
      else
        {
          attr = L_new_attr ("pext", 1);
          L_set_int_attr_field (attr, 0, ext);
          lcode_op->attr = L_concat_attr (lcode_op->attr, attr);
        }
    }
#if 0
  printf ("%s op %i: After set, get_ext returns %i\n",
          sm_op->sm_cb->lcode_fn->name, sm_op->lcode_op->id,
          SM_get_ext (sm_op));
#endif
}

/* Returns the ext for an oper, 0 if none exists */
ITintmax
SM_get_ext (SM_Oper * sm_op)
{
  L_Attr *attr;

  if ((attr = L_find_attr (sm_op->lcode_op->attr, "pext")) != 0)
    return (attr->field[0]->value.i);
  else
    return (0);
}
/* Changes opc and proc_opc for branch operations appropriately if
 * you reverse the operand order. 
 * For example:
 *   bge r1, r2, cb1  is equivalent to ble r2, r1, cb1
 */
void
SM_reverse_branch_opcode (int *opc, int *proc_opc)
{
  /* Handle hppa variations */
  if ((M_arch == M_HPPA) ||
      ((M_arch == M_IMPACT) && (M_model == M_HP_PA_7100)))
    {
      switch (*proc_opc)
        {
        case LHPPAop_COMB_EQ_FWD:
          *opc = Lop_BEQ;
          *proc_opc = LHPPAop_COMB_EQ_FWD;
          break;

        case LHPPAop_COMIB_EQ_FWD:
          *opc = Lop_BEQ;
          *proc_opc = LHPPAop_COMIB_EQ_FWD;
          break;

        case LHPPAop_COMB_NE_FWD:
          *opc = Lop_BNE;
          *proc_opc = LHPPAop_COMB_NE_FWD;
          break;

        case LHPPAop_COMIB_NE_FWD:
          *opc = Lop_BNE;
          *proc_opc = LHPPAop_COMIB_NE_FWD;
          break;

        case LHPPAop_COMB_GT_FWD:
          *opc = Lop_BLT;
          *proc_opc = LHPPAop_COMB_LT_FWD;
          break;

        case LHPPAop_COMIB_GT_FWD:
          *opc = Lop_BLT;
          *proc_opc = LHPPAop_COMIB_LT_FWD;
          break;

        case LHPPAop_COMB_GE_FWD:
          *opc = Lop_BLE;
          *proc_opc = LHPPAop_COMB_LE_FWD;
          break;

        case LHPPAop_COMIB_GE_FWD:
          *opc = Lop_BLE;
          *proc_opc = LHPPAop_COMIB_LE_FWD;
          break;

        case LHPPAop_COMB_LT_FWD:
          *opc = Lop_BGT;
          *proc_opc = LHPPAop_COMB_GT_FWD;
          break;

        case LHPPAop_COMIB_LT_FWD:
          *opc = Lop_BGT;
          *proc_opc = LHPPAop_COMIB_GT_FWD;
          break;

        case LHPPAop_COMB_LE_FWD:
          *opc = Lop_BGE;
          *proc_opc = LHPPAop_COMB_GE_FWD;
          break;

        case LHPPAop_COMIB_LE_FWD:
          *opc = Lop_BGE;
          *proc_opc = LHPPAop_COMIB_GE_FWD;
          break;


        case LHPPAop_COMB_EQ_BWD:
          *opc = Lop_BEQ;
          *proc_opc = LHPPAop_COMB_EQ_BWD;
          break;

        case LHPPAop_COMIB_EQ_BWD:
          *opc = Lop_BEQ;
          *proc_opc = LHPPAop_COMIB_EQ_BWD;
          break;

        case LHPPAop_COMB_NE_BWD:
          *opc = Lop_BNE;
          *proc_opc = LHPPAop_COMB_NE_BWD;
          break;

        case LHPPAop_COMIB_NE_BWD:
          *opc = Lop_BNE;
          *proc_opc = LHPPAop_COMIB_NE_BWD;
          break;

        case LHPPAop_COMB_GT_BWD:
          *opc = Lop_BLT;
          *proc_opc = LHPPAop_COMB_LT_BWD;
          break;

        case LHPPAop_COMIB_GT_BWD:
          *opc = Lop_BLT;
          *proc_opc = LHPPAop_COMIB_LT_BWD;
          break;

        case LHPPAop_COMB_GE_BWD:
          *opc = Lop_BLE;
          *proc_opc = LHPPAop_COMB_LE_BWD;
          break;

        case LHPPAop_COMIB_GE_BWD:
          *opc = Lop_BLE;
          *proc_opc = LHPPAop_COMIB_LE_BWD;
          break;

        case LHPPAop_COMB_LT_BWD:
          *opc = Lop_BGT;
          *proc_opc = LHPPAop_COMB_GT_BWD;
          break;

        case LHPPAop_COMIB_LT_BWD:
          *opc = Lop_BGT;
          *proc_opc = LHPPAop_COMIB_GT_BWD;
          break;

        case LHPPAop_COMB_LE_BWD:
          *opc = Lop_BGE;
          *proc_opc = LHPPAop_COMB_GE_BWD;
          break;

        case LHPPAop_COMIB_LE_BWD:
          *opc = Lop_BGE;
          *proc_opc = LHPPAop_COMIB_GE_BWD;
          break;

        default:
          L_punt ("SM_reverse_branch_opcode: Unknown proc_opc %i\n",
                  *proc_opc);
        }
    }

  /* Don't handle the others for now */
  else
    {
      L_punt ("SM_reverse_branch_opcode: Unsupported M_arch = %i", M_arch);
    }
}

/* Manipulates the operands to make valid mcode for the various 
 * architectures.
 */
void
SM_make_valid_mcode (int *opc, int *proc_opc, L_Operand ** src0,
                     L_Operand ** src1, int *ext)
{
  L_Operand *temp_operand;

  /* By default, set ext to 0 */
  *ext = 0;

  /* Handle hppa variations */
  if ((M_arch == M_HPPA) ||
      ((M_arch == M_IMPACT) && (M_model == M_HP_PA_7100)))
    {
      switch (*opc)
        {
        case Lop_ADD:
        case Lop_LD_UC:
        case Lop_LD_C:
        case Lop_LD_UC2:
        case Lop_LD_C2:
        case Lop_LD_I:
        case Lop_ST_I:

          /* For all these, set *proc_opc to *opc */
          *proc_opc = *opc;

          /* Rearrange, if necessary, so src1 is a register */
          if (!L_is_variable (*src1))
            {
              /* If src0 is a register, swap operands */
              if (L_is_variable (*src0))
                {
                  /* Swap operands */
                  temp_operand = *src0;
                  *src0 = *src1;
                  *src1 = temp_operand;

                }
              /* Otherwise, punt for now (may want to generate mov's etc.) */
              else
                {
                  fprintf (stderr, "SM_make_valid_mcode: Unhandled case:\n");
                  fprintf (stderr, " opc = %i  proc_opc = %i: src0 = ",
                           *opc, *proc_opc);
                  L_print_operand (stderr, *src0, 1);
                  fprintf (stderr, "  src1 = ");
                  L_print_operand (stderr, *src1, 1);
                  L_punt ("Yuck, more implementation needed. :(");
                }
            }
          break;

        case Lop_BEQ:
        case Lop_BEQ_FS:
        case Lop_BNE:
        case Lop_BNE_FS:
        case Lop_BGT:
        case Lop_BGT_FS:
        case Lop_BGE:
        case Lop_BGE_FS:
        case Lop_BLT:
        case Lop_BLT_FS:
        case Lop_BLE:
        case Lop_BLE_FS:
          /* Rearrange, if necessary, so src1 is a register */
          if (!L_is_variable (*src1))
            {
              /* If src0 is a register, swap operands */
              if (L_is_variable (*src0))
                {
                  /* Swap operands */
                  temp_operand = *src0;
                  *src0 = *src1;
                  *src1 = temp_operand;

                  /* Reverse the branch opcode since we are flipping 
                   * operands. 
                   */
                  SM_reverse_branch_opcode (opc, proc_opc);
                }
              /* Otherwise, punt for now (may want to generate mov's etc.) */
              else
                {
                  fprintf (stderr, "SM_make_valid_mcode: Unhandled case:\n");
                  fprintf (stderr, " opc = %i  proc_opc = %i: src0 = ",
                           *opc, *proc_opc);
                  L_print_operand (stderr, *src0, 1);
                  fprintf (stderr, "  src1 = ");
                  L_print_operand (stderr, *src1, 1);
                  L_punt ("Yuck, more implementation needed. :(");
                }
            }
          break;

        case Lop_SUB:
          /* Set proc_opc to Lop_SUB also */
          *proc_opc = Lop_SUB;

          /* Rearrange, if necessary, so src1 is a register */
          if (!L_is_variable (*src1))
            {
              /* If src0 is a register and src1 is an integer constant,
               * swap operands, negate constant, and change to add:
               * 
               *   r4 <- r2 - 10   =>   r4 <- -10 + r2
               */
              if (L_is_variable (*src0) && L_is_int_constant (*src1))
                {
                  /* Change to add */
                  *opc = Lop_ADD;
                  *proc_opc = Lop_ADD;

                  /* Swap operands */
                  temp_operand = *src0;
                  *src0 = *src1;
                  *src1 = temp_operand;

                  /* Negate the new src0 */
                  (*src0)->value.i = -(*src0)->value.i;
                }
              /* Punt for now (may want to generate mov's etc.) */
              else
                {
                  fprintf (stderr, "SM_make_valid_mcode: Unhandled case:\n");
                  fprintf (stderr,
                           "  opc = %i proc_opc = %i (Lop_SUB): src0 = ",
                           *opc, *proc_opc);
                  L_print_operand (stderr, *src0, 1);
                  fprintf (stderr, "  src1 = ");
                  L_print_operand (stderr, *src1, 1);
                  L_punt ("Yuck, more implementation needed. :(");
                }
            }
          break;

        case Lop_MOV:
          /* For now, do nothing */
          break;

        default:
          L_punt ("SM_make_valid_mcode: Unsupported opc = %i proc_opc = %i",
                  *opc, *proc_opc);
        }

      /* Handle cases were need to change proc_opc because src[0] is
       * now a register.
       */
      if (L_is_variable (*src0))
        {
          switch (*proc_opc)
            {
            case Lop_MOV:
              /* No extension need for MOV of a register */
              *proc_opc = Lop_MOV;
              break;

              /* No extension needed for ADD with two register sources */
            case Lop_ADD:
#if 0
#ifndef OPENIMPACT_DISTRIBUTION
            case LHPPAop_LDO:
#endif
#endif
              *proc_opc = Lop_ADD;
              break;

              /* No extension needed for ADD with two register sources */
            case Lop_SUB:
              *proc_opc = Lop_SUB;
              break;

            case Lop_LD_UC:
            case Lop_LD_C:
            case Lop_LD_UC2:
            case Lop_LD_C2:
            case Lop_LD_I:
              *proc_opc = *opc;
              /* Set ext to LOAD_INDEXED, if have two register srcs */
#if 0
#ifndef OPENIMPACT_DISTRIBUTION
              *ext = LOAD_INDEXED;
#endif
#endif
              break;

#if 0
#ifndef OPENIMPACT_DISTRIBUTION
            case LHPPAop_COMIB_EQ_FWD:
            case LHPPAop_COMB_EQ_FWD:
              *proc_opc = LHPPAop_COMB_EQ_FWD;
              *ext = EXT (CBR_FORWARD_EXT, COMP_EQ, 0);
              break;

            case LHPPAop_COMIB_NE_FWD:
            case LHPPAop_COMB_NE_FWD:
              *proc_opc = LHPPAop_COMB_NE_FWD;
              *ext = EXT (CBR_FORWARD_EXT, COMP_NE, 0);
              break;

            case LHPPAop_COMIB_GT_FWD:
            case LHPPAop_COMB_GT_FWD:
              *proc_opc = LHPPAop_COMB_GT_FWD;
              *ext = EXT (CBR_FORWARD_EXT, COMP_GT, 0);
              break;

            case LHPPAop_COMIB_GE_FWD:
            case LHPPAop_COMB_GE_FWD:
              *proc_opc = LHPPAop_COMB_GE_FWD;
              *ext = EXT (CBR_FORWARD_EXT, COMP_GE, 0);
              break;

            case LHPPAop_COMIB_LT_FWD:
            case LHPPAop_COMB_LT_FWD:
              *proc_opc = LHPPAop_COMB_LT_FWD;
              *ext = EXT (CBR_FORWARD_EXT, COMP_LT, 0);
              break;

            case LHPPAop_COMIB_LE_FWD:
            case LHPPAop_COMB_LE_FWD:
              *proc_opc = LHPPAop_COMB_LE_FWD;
              *ext = EXT (CBR_FORWARD_EXT, COMP_LE, 0);
              break;


            case LHPPAop_COMIB_EQ_BWD:
            case LHPPAop_COMB_EQ_BWD:
              *proc_opc = LHPPAop_COMB_EQ_BWD;
              *ext = EXT (CBR_BACKWARD_EXT, COMP_EQ, 0);
              break;

            case LHPPAop_COMIB_NE_BWD:
            case LHPPAop_COMB_NE_BWD:
              *proc_opc = LHPPAop_COMB_NE_BWD;
              *ext = EXT (CBR_BACKWARD_EXT, COMP_NE, 0);
              break;

            case LHPPAop_COMIB_GT_BWD:
            case LHPPAop_COMB_GT_BWD:
              *proc_opc = LHPPAop_COMB_GT_BWD;
              *ext = EXT (CBR_BACKWARD_EXT, COMP_GT, 0);
              break;

            case LHPPAop_COMIB_GE_BWD:
            case LHPPAop_COMB_GE_BWD:
              *proc_opc = LHPPAop_COMB_GE_BWD;
              *ext = EXT (CBR_BACKWARD_EXT, COMP_GE, 0);
              break;

            case LHPPAop_COMIB_LT_BWD:
            case LHPPAop_COMB_LT_BWD:
              *proc_opc = LHPPAop_COMB_LT_BWD;
              *ext = EXT (CBR_BACKWARD_EXT, COMP_LT, 0);
              break;

            case LHPPAop_COMIB_LE_BWD:
            case LHPPAop_COMB_LE_BWD:
              *proc_opc = LHPPAop_COMB_LE_BWD;
              *ext = EXT (CBR_BACKWARD_EXT, COMP_LE, 0);
              break;
#endif
#endif
            default:
              L_punt
                ("SM_make_valid_mcode: Unsupported2 opc = %i proc_opc = %i",
                 *opc, *proc_opc);
            }

        }
      else
        {
          switch (*proc_opc)
            {
            case Lop_MOV:
              if (FIELD_14 ((*src0)->value.i))
                {
                  *proc_opc = Lop_MOV;
#if 0
#ifndef OPENIMPACT_DISTRIBUTION
                  *ext = MOV_LDI;
#endif
#endif
                }
              else
                {
                  L_punt ("SM_make_valid_mcode: Too big for mov!");
                }
              break;

              /* Set extension for INT in src[0] */
            case Lop_ADD:
#if 0
#ifndef OPENIMPACT_DISTRIBUTION
            case LHPPAop_LDO:
#endif
#endif
              if (FIELD_11 ((*src0)->value.i))
                {
                  *proc_opc = Lop_ADD;
#if 0
#ifndef OPENIMPACT_DISTRIBUTION
                  *ext = ADD_ADDI;
#endif
#endif
                }
              else if (FIELD_14 ((*src0)->value.i))
                {
#if 0
#ifndef OPENIMPACT_DISTRIBUTION
                  *ext = ADD_LDO;
                  *proc_opc = LHPPAop_LDO;
#endif
#endif
                }
              else
                {
                  L_punt ("SM_make_valid_mcode: Too big for add!");
                }
              break;

            case Lop_SUB:
              if (FIELD_11 ((*src0)->value.i))
                {
                  *proc_opc = Lop_SUB;
#if 0
#ifndef OPENIMPACT_DISTRIBUTION
                  *ext = SUB_SUBI;
#endif
#endif
                }
              else
                {
                  L_punt ("SM_make_valid_mcode: Too big for sub!");
                }
              break;

            case Lop_LD_UC:
            case Lop_LD_C:
            case Lop_LD_UC2:
            case Lop_LD_C2:
            case Lop_LD_I:
              /* Make sure we are dealing with an int constant */
              if (!L_is_int_constant (*src0))
                {
                  L_print_operand (stderr, *src0, 0);
                  L_punt ("SM_make_valid_mcode: Only constants supported");
                }

              /* Set ext to LOAD_SHORT or LOAD_LONG base of const width */
#if 0
#ifndef OPENIMPACT_DISTRIBUTION
              if (FIELD_5 ((*src0)->value.i))
                *ext = LOAD_SHORT;
              else if (FIELD_14 ((*src0)->value.i))
                *ext = LOAD_LONG;
              else
                {
                  L_print_operand (stderr, *src0, 0);
                  L_punt ("SM_make_valid_mcode: Too big for load!");
                }
#endif
#endif
              break;

#if 0
#ifndef OPENIMPACT_DISTRIBUTION
            case LHPPAop_COMB_EQ_FWD:
            case LHPPAop_COMIB_EQ_FWD:
              *proc_opc = LHPPAop_COMIB_EQ_FWD;
              *ext = EXT (CBR_FORWARD_EXT, COMP_EQ, CBR_IMMED_EXT);
              break;

            case LHPPAop_COMB_NE_FWD:
            case LHPPAop_COMIB_NE_FWD:
              *proc_opc = LHPPAop_COMIB_NE_FWD;
              *ext = EXT (CBR_FORWARD_EXT, COMP_NE, CBR_IMMED_EXT);
              break;

            case LHPPAop_COMB_GT_FWD:
            case LHPPAop_COMIB_GT_FWD:
              *proc_opc = LHPPAop_COMIB_GT_FWD;
              *ext = EXT (CBR_FORWARD_EXT, COMP_GT, CBR_IMMED_EXT);
              break;

            case LHPPAop_COMB_GE_FWD:
            case LHPPAop_COMIB_GE_FWD:
              *proc_opc = LHPPAop_COMIB_GE_FWD;
              *ext = EXT (CBR_FORWARD_EXT, COMP_GE, CBR_IMMED_EXT);
              break;

            case LHPPAop_COMB_LT_FWD:
            case LHPPAop_COMIB_LT_FWD:
              *proc_opc = LHPPAop_COMIB_LT_FWD;
              *ext = EXT (CBR_FORWARD_EXT, COMP_LT, CBR_IMMED_EXT);
              break;

            case LHPPAop_COMB_LE_FWD:
            case LHPPAop_COMIB_LE_FWD:
              *proc_opc = LHPPAop_COMIB_LE_FWD;
              *ext = EXT (CBR_FORWARD_EXT, COMP_LE, CBR_IMMED_EXT);
              break;


            case LHPPAop_COMB_EQ_BWD:
            case LHPPAop_COMIB_EQ_BWD:
              *proc_opc = LHPPAop_COMIB_EQ_BWD;
              *ext = EXT (CBR_BACKWARD_EXT, COMP_EQ, CBR_IMMED_EXT);
              break;

            case LHPPAop_COMB_NE_BWD:
            case LHPPAop_COMIB_NE_BWD:
              *proc_opc = LHPPAop_COMIB_NE_BWD;
              *ext = EXT (CBR_BACKWARD_EXT, COMP_NE, CBR_IMMED_EXT);
              break;

            case LHPPAop_COMB_GT_BWD:
            case LHPPAop_COMIB_GT_BWD:
              *proc_opc = LHPPAop_COMIB_GT_BWD;
              *ext = EXT (CBR_BACKWARD_EXT, COMP_GT, CBR_IMMED_EXT);
              break;

            case LHPPAop_COMB_GE_BWD:
            case LHPPAop_COMIB_GE_BWD:
              *proc_opc = LHPPAop_COMIB_GE_BWD;
              *ext = EXT (CBR_BACKWARD_EXT, COMP_GE, CBR_IMMED_EXT);
              break;

            case LHPPAop_COMB_LT_BWD:
            case LHPPAop_COMIB_LT_BWD:
              *proc_opc = LHPPAop_COMIB_LT_BWD;
              *ext = EXT (CBR_BACKWARD_EXT, COMP_LT, CBR_IMMED_EXT);
              break;

            case LHPPAop_COMB_LE_BWD:
            case LHPPAop_COMIB_LE_BWD:
              *proc_opc = LHPPAop_COMIB_LE_BWD;
              *ext = EXT (CBR_BACKWARD_EXT, COMP_LE, CBR_IMMED_EXT);
              break;
#endif
#endif
            default:
              L_punt
                ("SM_make_valid_mcode: Unsupported3 opc = %i proc_opc = %i",
                 *opc, *proc_opc);

            }

        }
    }

  /* Don't handle the others for now */
  else
    {
      L_punt ("SM_make_valid_mcode: Unsupported M_arch = %i", M_arch);
    }
}


/* Returns 1 if this dest_action has exactly one use, 0 otherwise.
 * Expects there to be at least one use inside the cb (punts otherwise).
 */
int
SM_def_has_exactly_one_use (SM_Reg_Action * def_action)
{
  SM_Dep *dep_out;
  SM_Reg_Action *def_after;
  L_Operand *operand;
  int use_count, reaches_cb_end;

  /* Sanity check, better be a def action */
  if (!(def_action->flags & SM_DEF_ACTION))
    L_punt ("SM_def_has_exactly_one_use: def action expected!");

  /* Sanity check, better be at least one use */
  if (def_action->first_dep_out == NULL)
    L_punt ("SM_def_has_exactly_one_use: at least one use in cb expected!");

  /* Make sure there is only one use in cb */
  use_count = 0;
  for (dep_out = def_action->first_dep_out; dep_out != NULL;
       dep_out = dep_out->next_dep_out)
    {
      /* Only look at implicit and explicit flow deps */
      if ((dep_out->ignore) || !(dep_out->flags & SM_FLOW_DEP))
        continue;

      /* Update count */
      use_count++;

      /* If just went past one, return 0 */
      if (use_count > 1)
        return (0);
    }

  /* Determine if the action reaches the end of the cb.
   * Assume does unless hit another definition that post_dominates
   * this definition.
   */
  reaches_cb_end = 1;
  for (def_after = def_action->next_def; def_after != NULL;
       def_after = def_after->next_def)
    {
      /* Stop scan if this def post-dominates the def_action */
      if (SM_def_post_dominates_action (def_after, def_action))
        {
          /* This def kill's def_action's lifetime */
          reaches_cb_end = 0;
          break;
        }
    }

  /* If dest's lifetime reaches the end of the cb and
   * it is live out of the cb, return 0.
   */
  operand = def_action->rinfo->operand;
  if (reaches_cb_end)
    {
      /* For now, Lbx86 assumes all operands are live out */
      if (SM_use_fake_Lbx86_flow_analysis)
        return (0);

      /* Otherwise, check dataflow */
      else if (L_in_cb_OUT_set (def_action->sm_op->sm_cb->lcode_cb, operand))
        return (0);
    }

  /* If reached here, must have exactly one use */
  return (1);
}

#define ADD_EXPR        0x00000001
#define SUB_EXPR        0x00000002

void
SM_do_expr_with_copy (SM_Trans * trans)
{
  SM_Oper *target_sm_op, *def_sm_op, *new_def_sm_op;
  SM_Reg_Action *src_action;
  SM_Cb *sm_cb;
  L_Func *lcode_fn;
  L_Operand *def_src[2], *def_dest, *target_src[2], *const_src = NULL;
  L_Operand *move_src = NULL;
  L_Operand *neg_src;
  L_Oper *target_lcode_op, *def_lcode_op, *new_def_lcode_op = NULL;
  int target_flags, def_flags;
  int L1_index, r2_index, r3_index, r4_index;
  int new_def_opc, new_def_proc_opc, new_target_opc, new_target_proc_opc;
  int new_def_ext, new_target_ext;
  ITintmax const_value = 0;
  int need_new_def, can_delete_old_def;

  /* Get the target and def ops for ease of use */
  target_sm_op = trans->target_sm_op;

  /* Use the targets source to find the def.  Need to do this
   * since other transformations may have changed the sm_op associated
   * with the def!  Assume that the previous defintion exists!
   */
  src_action = target_sm_op->src[trans->target_index];
  def_sm_op = src_action->prev_def->sm_op;

  /* For now, so can easily undo, update trans->def_sm_op */
  trans->def_sm_op = def_sm_op;

  /* If this definition has exactly one use (the target_sm_op), we
   * can just change the existing def.
   */
  if (SM_def_has_exactly_one_use (def_sm_op->dest[0]))
    {
      need_new_def = 0;

      /* Assuming don't use this old def, we can delete it */
      can_delete_old_def = 1;
    }
  /* Otherwise, flag that we need to make a new def */
  else
    {
      need_new_def = 1;

      /* Flag that cannot delete old def since it has other uses */
      can_delete_old_def = 0;

      /* Punt if do_expr_without_copy_only is set, since this should
       * not happen.
       */
      if (do_expr_without_copy_only)
        {
          L_punt ("SM_do_expr_with_copy: Unintential copy required"
                  "while emulating classic application!");
        }
    }

  /* Get sm_cb and lcode function for ease of use */
  sm_cb = target_sm_op->sm_cb;
  lcode_fn = sm_cb->lcode_fn;

  /* Initialize internal transformation flags */
  target_flags = 0;
  def_flags = 0;

  /* Determine the expression type of the target and def ops */
  switch (target_sm_op->lcode_op->opc)
    {
    case Lop_ADD:
    case Lop_LD_UC:
    case Lop_LD_C:
    case Lop_LD_UC2:
    case Lop_LD_C2:
    case Lop_LD_I:
      target_flags |= ADD_EXPR;
      break;

    case Lop_SUB:
    case Lop_BEQ:
    case Lop_BEQ_FS:
    case Lop_BNE:
    case Lop_BNE_FS:
    case Lop_BGT:
    case Lop_BGT_FS:
    case Lop_BGE:
    case Lop_BGE_FS:
    case Lop_BLT:
    case Lop_BLT_FS:
    case Lop_BLE:
    case Lop_BLE_FS:
      target_flags |= SUB_EXPR;
      break;

    default:
      fprintf (stderr,
               "SM_do_expr_with_copy: %s cb %i Unknown target opc %i\n",
               sm_cb->lcode_fn->name, sm_cb->lcode_cb->id,
               target_sm_op->lcode_op->opc);
      SM_print_minimal_lcode_op (stderr, def_sm_op);
      SM_print_minimal_lcode_op (stderr, target_sm_op);
      L_punt ("Cannot continue!");
    }

  switch (def_sm_op->lcode_op->opc)
    {
    case Lop_ADD:
      def_flags |= ADD_EXPR;
      break;

    case Lop_SUB:
      def_flags |= SUB_EXPR;
      break;

    default:
      fprintf (stderr, "SM_do_expr_with_copy: %s cb %i Unknown def opc %i\n",
               sm_cb->lcode_fn->name, sm_cb->lcode_cb->id,
               def_sm_op->lcode_op->opc);
      SM_print_minimal_lcode_op (stderr, def_sm_op);
      SM_print_minimal_lcode_op (stderr, target_sm_op);
      SM_print_cb_dependences (stderr, sm_cb);
      L_punt ("Cannot continue!");
    }

  if (verbose_optimization)
    {
      printf ("\n");
    }

  /* Determine if we need to do renaming_with_copy first */
  if (trans->flags & SM_NEEDS_RENAMING)
    {
      if (!SM_can_move_before_def (def_sm_op, target_sm_op,
                                   target_sm_op->src[trans->target_index]))
        {
          if (verbose_optimization)
            {
              printf ("RENAMING DEST ");
            }

          /* If so, do renaming with copy first */
          SM_do_renaming_with_copy (trans);

          /* Punt if do_expr_without_copy_only is set, since this should
           * not happen.
           */
          if (do_expr_without_copy_only)
            {
              L_punt ("SM_do_expr_with_copy: Unintential renaming_with_copy"
                      "required while emulating classic application!");
            }
        }
      else if (verbose_optimization)
        {
          printf ("RENAMING NOT NEEDED ");
        }
    }

  if (verbose_optimization)
    {
      if (trans->flags & SM_MUST_REORDER_OPS)
        printf ("MUST_REORDER_OPS ");

      printf ("DO_EXPR_WITH_COPY %s cb %i weight %.0f/%.0f:\n",
              target_sm_op->sm_cb->lcode_fn->name,
              target_sm_op->sm_cb->lcode_cb->id,
              def_sm_op->lcode_op->weight, target_sm_op->lcode_op->weight);

      printf ("  ");
      if (def_flags & ADD_EXPR)
        printf ("ADD ");
      if (def_flags & SUB_EXPR)
        printf ("SUB ");
      SM_print_minimal_lcode_op (stdout, def_sm_op);

      printf ("  ");
      if (target_flags & ADD_EXPR)
        printf ("ADD ");
      if (target_flags & SUB_EXPR)
        printf ("SUB ");
      SM_print_minimal_lcode_op (stdout, target_sm_op);
      printf ("    Make ");
      SM_print_action_operand (stdout, def_sm_op->src[trans->other_index]);
      printf (" used late as possible.\n");
    }


  /*
   * There are 16 possible combinations of def and target ops, and
   * def operand to make late as possible, with 9 "real" cases.
   * For clarity, will handle each of the 9 "real" cases separately,
   * and will only implement them one at a time.
   * 
   * For now, we will preserve predicates (may want to
   * do predicate promotion in some cases).
   * 
   * All expressions will be in the form of:
   * 
   * r4 <- L1 o r2  (or r2 o L1)  where o can be effectively + or -
   * r5 <- r4 o r3  (or r3 o r4)  where o can be effectively + or -
   * 
   * Get the index of L1, r2, r3, and r4 for our def and target.
   */
  L1_index = trans->other_index;
  if (L1_index == 0)
    r2_index = 1;
  else
    r2_index = 0;

  r4_index = trans->target_index;
  if (r4_index == 0)
    r3_index = 1;
  else
    r3_index = 0;

  /* Get the lcode operations for target and def for ease of use */
  def_lcode_op = def_sm_op->lcode_op;
  target_lcode_op = target_sm_op->lcode_op;

  /* All transformations will create a new temp dest register,
   * so create it now.  Because cannot trust that renaming
   * has been done, if not creating a new def, do renaming
   * on this def and the use.
   */
  /* 20030226 SZU
   * SMH reconciliation
   * Looks like Itanium only
   */
  def_dest = L_new_register_operand (++lcode_fn->max_reg_id, L_CTYPE_LLONG,
                                     L_PTYPE_NULL);
#if 0
  def_dest = L_new_register_operand (++lcode_fn->max_reg_id, L_CTYPE_INT,
                                     L_PTYPE_NULL);
#endif

  /* If don't need a new def, force renaming to catch those cases
   * renaming missed.
   */
  if (!need_new_def)
    {
      SM_change_operand (def_sm_op, MDES_DEST, 0, def_dest);
      SM_change_operand (target_sm_op, MDES_SRC, trans->target_index,
                         def_dest);
    }

  /* To allow easy freeing, initialize operand pointers */
  def_src[0] = NULL;
  def_src[1] = NULL;
  target_src[0] = NULL;
  target_src[1] = NULL;

  /* Initially flag that we have not specified this case yet */
  new_def_opc = -1;

  /* By default, use same opc and proc_opc for target */
  new_target_opc = target_lcode_op->opc;
  new_target_proc_opc = target_lcode_op->proc_opc;

  /* By default, use no ext for both target and new def opcodes */
  new_def_ext = 0;
  new_target_ext = 0;

  /*
   * ADD -> ADD Case 1 of 1 (Use L1 late as possible):
   *   r4 <- L1 + r2 (or r2 + L1)   =>    t6 <- r3 + r2
   *   r5 <- r4 + r3 (or r3 + r4)   =>    r5 <- t6 + L1
   */
  if ((def_flags & ADD_EXPR) && (target_flags & ADD_EXPR))
    {
      /* The new def will be an add operation */
      new_def_opc = Lop_ADD;

      /* Specify how operands need to be rearranged to achieve the
       * expression on the left.  Copy necessary operands.
       */
      def_src[0] = L_copy_operand (target_lcode_op->src[r3_index]);
      def_src[1] = L_copy_operand (def_lcode_op->src[r2_index]);
      target_src[0] = L_copy_operand (def_dest);
      target_src[1] = L_copy_operand (def_lcode_op->src[L1_index]);
    }

  /*
   * SUB -> ADD Case 1 of 2 (Use L1 late as possible):
   *   r4 <- L1 - r2                =>    t6 <- r3 - r2
   *   r5 <- r4 + r3 (or r3 + r4)   =>    r5 <- t6 + L1
   */
  else if ((def_flags & SUB_EXPR) && (L1_index == 0) &&
           (target_flags & ADD_EXPR))
    {
      /* The new def will be a subtract operation */
      new_def_opc = Lop_SUB;

      /* Specify how operands need to be rearranged to achieve the
       * expression on the left.  Copy necessary operands.
       */
      def_src[0] = L_copy_operand (target_lcode_op->src[r3_index]);
      def_src[1] = L_copy_operand (def_lcode_op->src[r2_index]);
      target_src[0] = L_copy_operand (def_dest);
      target_src[1] = L_copy_operand (def_lcode_op->src[L1_index]);
    }

  /*
   * SUB -> ADD Case 2 of 2 (Use L1 late as possible):
   *   r4 <- r2 - L1                =>    t6 <- r2 + r3
   *   r5 <- r4 + r3 (or r3 + r4)   =>    r5 <- t6 - L1
   * 
   * NOTE: Second operation must be an Lop_ADD, so we
   * can change it into an Lop_SUB!
   */
  else if ((def_flags & SUB_EXPR) && (L1_index == 1) &&
           (target_flags & ADD_EXPR) && (target_lcode_op->opc == Lop_ADD))
    {
      /* The new def will be a add operation */
      new_def_opc = Lop_ADD;


      /* The target operation will be changed into a SUB */
      new_target_opc = Lop_SUB;

      /* Specify how operands need to be rearranged to achieve the
       * expression on the left.  Copy necessary operands.
       */
      def_src[0] = L_copy_operand (def_lcode_op->src[r2_index]);
      def_src[1] = L_copy_operand (target_lcode_op->src[r3_index]);
      target_src[0] = L_copy_operand (def_dest);
      target_src[1] = L_copy_operand (def_lcode_op->src[L1_index]);
    }

  /*
   * ADD -> SUB Case 1 of 3 (Use L1 late as possible):
   *   r4 <- L1 + r2 (or r2 + L1)    =>    t6 <- r2 - r3
   *   r5 <- r4 - r3                 =>    r5 <- t6 + L1
   * 
   * where target_op is a Lop_SUB (see case 2 for branches).
   */
  else if ((def_flags & ADD_EXPR) && (r3_index == 1) &&
           (target_flags & SUB_EXPR)
           && (target_lcode_op->proc_opc == Lop_SUB))
    {
      /* The new def will be a subtract operation */
      new_def_opc = Lop_SUB;

      /* The target will now be an add */
      new_target_opc = Lop_ADD;

      /* Specify how operands need to be rearranged to achieve the
       * expression on the left.  Copy necessary operands.
       */
      def_src[0] = L_copy_operand (def_lcode_op->src[r2_index]);
      def_src[1] = L_copy_operand (target_lcode_op->src[r3_index]);
      target_src[0] = L_copy_operand (def_dest);
      target_src[1] = L_copy_operand (def_lcode_op->src[L1_index]);
    }

  /*
   * ADD -> SUB Case 2 of 3 (Use L1 late as possible):
   *   r4 <- L1 + r2 (or r2 + L1) 
   *   r5 <- r4 - r3              
   *
   * Can be transform into:
   *   r4 <- L1 + r2 (or r2 + L1)    =>    t6 <- r3 - r2
   *   r5 <- r3 - r4                 =>    r5 <- t6 - L1
   * 
   * where target_op is a BEQ or BNE branch (see case 1 for Lop_SUB).
   * This solution works because for these branches, we can
   * switch the operands.  This also works for BLT, BLE, etc if
   * we flip the sense of the branch.  This case then becomes case 3.
   */
  else if ((def_flags & ADD_EXPR) && (r3_index == 1) &&
           (target_flags & SUB_EXPR) &&
           (target_lcode_op->proc_opc != Lop_SUB))
    {
      /* The new def will be a subtract operation */
      new_def_opc = Lop_SUB;

      /* Reverse the branch opcode since we are flipping operands */
      SM_reverse_branch_opcode (&new_target_opc, &new_target_proc_opc);

      /* Specify how operands need to be rearranged to achieve the
       * expression on the left.  Copy necessary operands.
       */
      def_src[0] = L_copy_operand (target_lcode_op->src[r3_index]);
      def_src[1] = L_copy_operand (def_lcode_op->src[r2_index]);
      target_src[0] = L_copy_operand (def_dest);
      target_src[1] = L_copy_operand (def_lcode_op->src[L1_index]);
    }

  /*
   * ADD -> SUB Case 3 of 3 (Use L1 late as possible):
   *   r4 <- L1 + r2 (or r2 + L1)    =>    t6 <- r3 - r2
   *   r5 <- r3 - r4                 =>    r5 <- t6 - L1
   */
  else if ((def_flags & ADD_EXPR) && (r3_index == 0) &&
           (target_flags & SUB_EXPR))
    {
      /* The new def will be a subtract operation */
      new_def_opc = Lop_SUB;

      /* Specify how operands need to be rearranged to achieve the
       * expression on the left.  Copy necessary operands.
       */
      def_src[0] = L_copy_operand (target_lcode_op->src[r3_index]);
      def_src[1] = L_copy_operand (def_lcode_op->src[r2_index]);
      target_src[0] = L_copy_operand (def_dest);
      target_src[1] = L_copy_operand (def_lcode_op->src[L1_index]);
    }

  /*
   * SUB -> SUB Case 1 of 4 (Use L1 late as possible):
   *   r4 <- L1 - r2   =>    t6 <- r3 + r2
   *   r5 <- r4 - r3   =>    r5 <- L1 - t6
   */
  else if ((def_flags & SUB_EXPR) &&
           (L1_index == 0) && (r3_index == 1) && (target_flags & SUB_EXPR))
    {
      /* The new def will be an add operation */
      new_def_opc = Lop_ADD;

      /* Specify how operands need to be rearranged to achieve the
       * expression on the left.  Copy necessary operands.
       */
      def_src[0] = L_copy_operand (target_lcode_op->src[r3_index]);
      def_src[1] = L_copy_operand (def_lcode_op->src[r2_index]);
      target_src[0] = L_copy_operand (def_lcode_op->src[L1_index]);
      target_src[1] = L_copy_operand (def_dest);
    }

  /*
   * SUB -> SUB Case 2 of 4 (Use L1 late as possible):
   *   r4 <- r2 - L1   =>    t6 <- r2 - r3
   *   r5 <- r4 - r3   =>    r5 <- t6 - L1
   */
  else if ((def_flags & SUB_EXPR) &&
           (L1_index == 1) && (r3_index == 1) && (target_flags & SUB_EXPR))
    {
      /* The new def will be a subtract operation */
      new_def_opc = Lop_SUB;

      /* Specify how operands need to be rearranged to achieve the
       * expression on the left.  Copy necessary operands.
       */
      def_src[0] = L_copy_operand (def_lcode_op->src[r2_index]);
      def_src[1] = L_copy_operand (target_lcode_op->src[r3_index]);
      target_src[0] = L_copy_operand (def_dest);
      target_src[1] = L_copy_operand (def_lcode_op->src[L1_index]);
    }

  /*
   * SUB -> SUB Case 3 of 4 (Use L1 late as possible):
   *   r4 <- L1 - r2   =>    t6 <- r3 + r2
   *   r5 <- r3 - r4   =>    r5 <- t6 - L1
   */
  else if ((def_flags & SUB_EXPR) &&
           (L1_index == 0) && (r3_index == 0) && (target_flags & SUB_EXPR))
    {
      /* The new def will be an add operation */
      new_def_opc = Lop_ADD;

      /* Specify how operands need to be rearranged to achieve the
       * expression on the left.  Copy necessary operands.
       */
      def_src[0] = L_copy_operand (target_lcode_op->src[r3_index]);
      def_src[1] = L_copy_operand (def_lcode_op->src[r2_index]);
      target_src[0] = L_copy_operand (def_dest);
      target_src[1] = L_copy_operand (def_lcode_op->src[L1_index]);
    }

  /*
   * SUB -> SUB Case 4 of 4 (Use L1 late as possible):
   *   r4 <- r2 - L1   =>    t6 <- r2 - r3
   *   r5 <- r3 - r4   =>    r5 <- L1 - t6 
   */
  else if ((def_flags & SUB_EXPR) &&
           (L1_index == 1) && (r3_index == 0) && (target_flags & SUB_EXPR))
    {
      /* The new def will be a subtract operation */
      new_def_opc = Lop_SUB;

      /* Specify how operands need to be rearranged to achieve the
       * expression on the left.  Copy necessary operands.
       */
      def_src[0] = L_copy_operand (def_lcode_op->src[r2_index]);
      def_src[1] = L_copy_operand (target_lcode_op->src[r3_index]);
      target_src[0] = L_copy_operand (def_lcode_op->src[L1_index]);
      target_src[1] = L_copy_operand (def_dest);
    }

  /*
   * Can't handle:
   * SUB -> ADD Case 2 of 2 for mem ops (Use L1 late as possible):
   *   r4 <- r2 - L1                =>    t6 <- r2 + r3
   *   r5 <- r4 + r3 (or r3 + r4)   =>    r5 <- t6 - L1
   * Cannot do anything for this case...
   */
  else if ((def_flags & SUB_EXPR) && (L1_index == 1) &&
           (target_flags & ADD_EXPR) && (target_lcode_op->opc != Lop_ADD))
    {
    }

  /* I believe that all the various cases should now be explictly 
   * handled above.  Punt if get here.
   */
  else
    {
      fprintf (stderr, "\nSM_do_expr_with_copy: %s cb %i Unhandled case:\n",
               sm_cb->lcode_fn->name, sm_cb->lcode_cb->id);
      fprintf (stderr, "  ");
      if (def_flags & ADD_EXPR)
        fprintf (stderr, "ADD ");
      if (def_flags & SUB_EXPR)
        fprintf (stderr, "SUB ");
      SM_print_minimal_lcode_op (stderr, def_sm_op);

      printf ("  ");
      if (target_flags & ADD_EXPR)
        fprintf (stderr, "ADD ");
      if (target_flags & SUB_EXPR)
        fprintf (stderr, "SUB ");
      SM_print_minimal_lcode_op (stderr, target_sm_op);
      fprintf (stderr, "    Make ");
      SM_print_action_operand (stderr, def_sm_op->src[trans->other_index]);
      fprintf (stderr, " used late as possible.\n");
      L_punt ("Algorithm error, cannot continue (since I want it to work)!");
    }

  /* Detect case where rearranging has made constant too big for
   * def.  For now, just don't do transformation in this case.
   * If both are constants, that will be handled separately below.
   */
  if (new_def_opc >= 0)
    {
      /* If src[0] is an int, and src[1] is a register */
      if (L_is_int_constant (def_src[0]) && L_is_variable (def_src[1]))
        {
          if (!SM_int_field_large_enough (new_def_opc, new_def_opc,
                                          def_src[0]->value.i))
            {
              /* Flag that we cannot do trans for now */
              new_def_opc = -1;
            }
        }

      /* Otherwise, if src[1] is an int, and src[0] is a register */
      else if (L_is_int_constant (def_src[1]) && L_is_variable (def_src[0]))
        {
          if (!SM_int_field_large_enough (new_def_opc, new_def_opc,
                                          def_src[1]->value.i))
            {
              /* Flag that we cannot do trans for now */
              new_def_opc = -1;
            }
        }
      /* Otherwise, if both const, make sure result fits in move */
      else if (L_is_int_constant (def_src[0]) &&
               L_is_int_constant (def_src[1]))
        {
          /* Calc the const value based on operation type */
          if (new_def_opc == Lop_ADD)
            const_value = def_src[0]->value.i + def_src[1]->value.i;

          else if (new_def_opc == Lop_SUB)
            const_value = def_src[0]->value.i - def_src[1]->value.i;

          /* Will this fit in move's int field? */
          if (!SM_int_field_large_enough (Lop_MOV, Lop_MOV, const_value))
            {
              /* Flag that we cannot do trans for now */
              new_def_opc = -1;
            }
        }
    }

  /* If new_def_opc != -1, do the transformation with the operands 
   * specified above.
   */
  if (new_def_opc >= 0)
    {
      /* Save the target's src operands before doing anything so
       * can undo the transformation
       */
      trans->orig_src[0] = L_copy_operand (target_lcode_op->src[0]);
      trans->orig_src[1] = L_copy_operand (target_lcode_op->src[1]);

      /* If both operands are integers, just generate
       * a constant source for target, unless it doesn't fit.  Then
       * generate a move, if necessary.
       */
      if (L_is_int_constant (def_src[0]) && L_is_int_constant (def_src[1]))
        {
          /* Create the new const operand based on the operation type */
          if (new_def_opc == Lop_ADD)
            {
	      /* 20030226 SZU
	       * SMH reconciliation
	       * Looks like Itanium only
	       */
              const_src = L_new_int_operand (def_src[0]->value.i +
                                             def_src[1]->value.i,
                                             L_CTYPE_LLONG);
#if 0
              const_src = L_new_int_operand (def_src[0]->value.i +
                                             def_src[1]->value.i,
                                             L_CTYPE_INT);
#endif
            }
          else if (new_def_opc == Lop_SUB)
            {
	      /* 20030226 SZU
	       * SMH reconciliation
	       * Looks like Itanium only
	       */
              const_src = L_new_int_operand (def_src[0]->value.i -
                                             def_src[1]->value.i,
                                             L_CTYPE_LLONG);
#if 0
              const_src = L_new_int_operand (def_src[0]->value.i -
                                             def_src[1]->value.i,
                                             L_CTYPE_INT);
#endif
            }
          else
            {
              fprintf (stderr,
                       "SM_do_expr_with_copy: Unknown new_def_opc %i\n",
                       new_def_opc);
              SM_print_minimal_lcode_op (stderr, def_sm_op);
              SM_print_minimal_lcode_op (stderr, target_sm_op);
              L_punt ("Cannot continue!");
            }

          /* If this const will fit in the int field for the target_sm_op,
           * just place it in there, otherwise create a move!
           */
          if (SM_int_field_large_enough (new_target_opc, new_target_proc_opc,
                                         const_src->value.i))
            {
              /* Flag there there is no new_def_lcode_op! */
              new_def_lcode_op = NULL;

              /* Replace the def_dest with the new const_src */
              if (L_same_operand (def_dest, target_src[0]))
                {
                  L_delete_operand (target_src[0]);
                  target_src[0] = const_src;
                }
              else if (L_same_operand (def_dest, target_src[1]))
                {
                  L_delete_operand (target_src[1]);
                  target_src[1] = const_src;
                }
              else
                {
                  L_punt ("SM_do_expr_with_copy: Can't find matching src!");
                }

              /* Free the src operands, they are no longer needed */
              L_delete_operand (def_dest);
              L_delete_operand (def_src[0]);
              L_delete_operand (def_src[1]);

              /* Set the def operands to NULL so they won't be freed below */
              def_dest = NULL;
              def_src[0] = NULL;
              def_src[1] = NULL;
            }

          /* Otherwise, generate a proper move operation */
          else
            {
              new_def_opc = Lop_MOV;
              new_def_proc_opc = new_def_opc;
              new_def_ext = 0;

              /* Free the orig src operands, they are no longer needed */
              L_delete_operand (def_src[0]);
              L_delete_operand (def_src[1]);

              /* Set them to new operands */
              def_src[0] = const_src;
              def_src[1] = NULL;

              /* Make this a valid move */
              SM_make_valid_mcode (&new_def_opc, &new_def_proc_opc,
                                   &def_src[0], &def_src[1], &new_def_ext);


              /* If need new def, create it, otherwise modify existing def */
              if (need_new_def)
                {
                  /* Create a move operation to move the new lit into a reg */
                  new_def_lcode_op = L_create_new_op_using (new_def_opc,
                                                            NULL);

                  /* Set it's operands using the copies above */
                  new_def_lcode_op->dest[0] = def_dest;
                  new_def_lcode_op->src[0] = def_src[0];
                  new_def_lcode_op->src[1] = def_src[1];

                  /* Set the def operands to NULL so they won't be freed 
                   * below 
                   */
                  def_dest = NULL;
                  def_src[0] = NULL;
                  def_src[1] = NULL;

                  /* Flag that we created a new def */
                  trans->flags |= SM_DUPLICATED_DEF;
                }
              /* Otherwise, modify existing def */
              else
                {
                  /* Flag that old def is in use */
                  can_delete_old_def = 0;

                  /* Flag that don't need new def */
                  new_def_lcode_op = NULL;

                  /* For sanity sake, clear SM_DUPLICATED_DEF flag */
                  trans->flags &= ~SM_DUPLICATED_DEF;

                  /* If not reordering target, move def to right above target
                   * if not already right above target.
                   */
                  if ((!(trans->flags & SM_MUST_REORDER_OPS)) &&
                      (def_sm_op->next_serial_op != target_sm_op))
                    {
                      /* Record orig prev op so can undo */
                      trans->orig_def_prev_serial_op =
                        def_sm_op->prev_serial_op;

                      /* Move to right above target */
                      SM_move_oper_after (def_sm_op,
                                          target_sm_op->prev_serial_op);
                    }

                  /* Save the operands def before changing it, so can undo */
                  trans->orig_def_src[0] =
                    L_copy_operand (def_lcode_op->src[0]);
                  trans->orig_def_src[1] =
                    L_copy_operand (def_lcode_op->src[1]);

                  /* Change the def operands to the new operands */
                  SM_change_operand (def_sm_op, MDES_SRC, 0, def_src[0]);
                  SM_change_operand (def_sm_op, MDES_SRC, 1, def_src[1]);

                  /* Unconditionally save opc and proc_opc so can undo */
                  trans->orig_def_opc = def_lcode_op->opc;
                  trans->orig_def_proc_opc = def_lcode_op->proc_opc;

                  /* If necessary, change the opc and proc_opc */
                  if ((def_lcode_op->opc != new_def_opc) ||
                      (def_lcode_op->proc_opc != new_def_proc_opc))
                    {

                      /* Use L_change_opcode so string name is also changed */
                      L_change_opcode (def_lcode_op, new_def_opc);

                      /* Then change the proc_opc */
                      def_lcode_op->proc_opc = new_def_proc_opc;
                    }

                  /* Save the original ext (so can undo), then change to new 
                   * ext 
                   */
                  trans->orig_def_ext = SM_get_ext (def_sm_op);
                  SM_set_ext (def_sm_op, new_def_ext);
                }
            }
        }

      /* Otherwise, is the def now the equivalent to a mov operation?  
       * If so, don't generate a new def, just copy propagate the
       * register to the target op.
       */
      else if (((new_def_opc == Lop_ADD) &&
                (L_is_int_zero (def_src[0]) ||
                 L_is_int_zero (def_src[1]))) ||
               ((new_def_opc == Lop_SUB) && L_is_int_zero (def_src[1])))

        {
          /* Flag there there is no new_def_lcode_op! */
          new_def_lcode_op = NULL;

          /* Find the move source and delete both sources */
          if (L_is_variable (def_src[0]))
            {
              move_src = def_src[0];
              def_src[0] = NULL;
              L_delete_operand (def_src[1]);
              def_src[1] = NULL;
            }
          else if (L_is_variable (def_src[1]))
            {
              move_src = def_src[1];
              def_src[1] = NULL;
              L_delete_operand (def_src[0]);
              def_src[0] = NULL;
            }
          else
            L_punt ("SM_do_expr_with_copy: Can't find move src!");


          /* Replace the def_dest with the move_src */
          if (L_same_operand (def_dest, target_src[0]))
            {
              L_delete_operand (target_src[0]);
              target_src[0] = move_src;
            }
          else if (L_same_operand (def_dest, target_src[1]))
            {
              L_delete_operand (target_src[1]);
              target_src[1] = move_src;
            }
          else
            {
              L_punt ("SM_do_expr_with_copy: Can't find matching src!");
            }

          /* Free the dest operand */
          L_delete_operand (def_dest);

          /* Set the def operands to NULL so they won't be freed below */
          def_dest = NULL;
        }

      /* Otherwise, is the def now the equivalent to a neg operation?  
       * If so handle the folowoing case:
       *    r4 <- 0 - r2
       *    r5 <- L1 - r4    =>  r5 = L1 + r2.
       */
      else if ((new_def_opc == Lop_SUB) &&
               L_is_int_zero (def_src[0]) &&
               (new_target_opc == Lop_SUB) &&
               L_same_operand (def_dest, target_src[1]))

        {
          /* Flag there there is no new_def_lcode_op! */
          new_def_lcode_op = NULL;

          /* Replace the def_dest with the def_src[1] */
          L_delete_operand (target_src[1]);
          target_src[1] = def_src[1];
          def_src[1] = NULL;

          /* Change the target operation into an ADD */
          new_target_opc = Lop_ADD;
          new_target_proc_opc = Lop_ADD;

          /* Delete and set to NULL the remaining def operands */
          L_delete_operand (def_src[0]);
          def_src[0] = NULL;
          L_delete_operand (def_dest);
          def_dest = NULL;
        }

      /* Otherwise, is the def now the equivalent to a neg operation?  
       * If so handle the folowoing case:
       *    r4 <- 0 - r2
       *    r5 <- L1 + r4 (or r4 + L1)   =>  r5 = L1 - r2.
       */
      else if ((new_def_opc == Lop_SUB) &&
               L_is_int_zero (def_src[0]) && (new_target_opc == Lop_ADD))
        {
          /* Flag there there is no new_def_lcode_op! */
          new_def_lcode_op = NULL;

          /* Get the neg source */
          neg_src = def_src[1];
          def_src[1] = NULL;

          /* Replace the def_dest with the neg_src, rearrange (if necessary)
           * to make it the second operand of the SUB 
           */
          if (L_same_operand (def_dest, target_src[0]))
            {
              L_delete_operand (target_src[0]);
              target_src[0] = target_src[1];
              target_src[1] = neg_src;
            }
          else if (L_same_operand (def_dest, target_src[1]))
            {
              L_delete_operand (target_src[1]);
              target_src[1] = neg_src;
            }
          else
            {
              L_punt ("SM_do_expr_with_copy: Can't find matching src!");
            }

          /* Change the target operation into a SUB */
          new_target_opc = Lop_SUB;
          new_target_proc_opc = Lop_SUB;

          /* Delete and set to NULL the remaining def operands */
          L_delete_operand (def_src[0]);
          def_src[0] = NULL;
          L_delete_operand (def_dest);
          def_dest = NULL;
        }

      /* Otherwise, if at least operand is a register, 
       * generated the new def operation
       */
      else if (L_is_variable (def_src[0]) || L_is_variable (def_src[1]))
        {
          /* Originally set def_proc_opc to opc */
          new_def_proc_opc = new_def_opc;

          /* Unfortunately, literals, etc. must go in there proper location.
           * Rearrange the operands, if neccessary.  This also may
           * require changing the opc (especially for Lop_SUB).
           */
          SM_make_valid_mcode (&new_def_opc, &new_def_proc_opc, &def_src[0],
                               &def_src[1], &new_def_ext);

          /* If need new def, create it, otherwise modify existing def */
          if (need_new_def)
            {
              /* Create the new def operation using the old one */
              new_def_lcode_op = L_create_new_op_using (new_def_opc,
                                                        def_lcode_op);

              /* Set it's operands using the copies above */
              new_def_lcode_op->dest[0] = def_dest;
              new_def_lcode_op->src[0] = def_src[0];
              new_def_lcode_op->src[1] = def_src[1];

              /* Set the def operands to NULL so they won't be freed below */
              def_dest = NULL;
              def_src[0] = NULL;
              def_src[1] = NULL;

              /* Flag that we created a new def */
              trans->flags |= SM_DUPLICATED_DEF;
            }

          /* Otherwise, modify existing def */
          else
            {
              /* Flag that don't need new def */
              new_def_lcode_op = NULL;

              /* Flag that old def is in use */
              can_delete_old_def = 0;

              /* For sanity sake, clear SM_DUPLICATED_DEF flag */
              trans->flags &= ~SM_DUPLICATED_DEF;

              /* If not reordering target, move def to right above target
               * if not already right above target.
               *
               * Note: moving to right below target will do little good
               * since def needs to define the value!
               * (Hmm... I had this exactly BACKWARD, and I don't know
               *  why... -JCG 2/23/97).
               */
              if ((!(trans->flags & SM_MUST_REORDER_OPS)) &&
                  (def_sm_op->next_serial_op != target_sm_op))
                {
                  /* Record orig prev op so can undo */
                  trans->orig_def_prev_serial_op = def_sm_op->prev_serial_op;

                  /* Move to right above target */
                  SM_move_oper_after (def_sm_op,
                                      target_sm_op->prev_serial_op);
                }

              /* Save the operands def before changing it, so can undo */
              trans->orig_def_src[0] = L_copy_operand (def_lcode_op->src[0]);
              trans->orig_def_src[1] = L_copy_operand (def_lcode_op->src[1]);

              /* Change the def operands to the new operands */
              SM_change_operand (def_sm_op, MDES_SRC, 0, def_src[0]);
              SM_change_operand (def_sm_op, MDES_SRC, 1, def_src[1]);

              /* Unconditionally save opc and proc_opc so can undo */
              trans->orig_def_opc = def_lcode_op->opc;
              trans->orig_def_proc_opc = def_lcode_op->proc_opc;

              /* If necessary, change the opc and proc_opc */
              if ((def_lcode_op->opc != new_def_opc) ||
                  (def_lcode_op->proc_opc != new_def_proc_opc))
                {

                  /* Use L_change_opcode so string name is also changed */
                  L_change_opcode (def_lcode_op, new_def_opc);

                  /* Then change the proc_opc */
                  def_lcode_op->proc_opc = new_def_proc_opc;
                }

              /* Save the original ext (so can undo), then change to new 
               * ext 
               */
              trans->orig_def_ext = SM_get_ext (def_sm_op);
              SM_set_ext (def_sm_op, new_def_ext);
            }
        }


      /* For now, don't handle the other cases */
      else
        {
          fprintf (stderr,
                   "\nSM_do_expr_with_copy: %s cb %i Unhandled case:\n",
                   sm_cb->lcode_fn->name, sm_cb->lcode_cb->id);
          fprintf (stderr, "  ");
          if (def_flags & ADD_EXPR)
            fprintf (stderr, "ADD ");
          if (def_flags & SUB_EXPR)
            fprintf (stderr, "SUB ");
          SM_print_minimal_lcode_op (stderr, def_sm_op);

          printf ("  ");
          if (target_flags & ADD_EXPR)
            fprintf (stderr, "ADD ");
          if (target_flags & SUB_EXPR)
            fprintf (stderr, "SUB ");
          SM_print_minimal_lcode_op (stderr, target_sm_op);
          fprintf (stderr, "    Make ");
          SM_print_action_operand (stderr,
                                   def_sm_op->src[trans->other_index]);
          fprintf (stderr, " used late as possible.\n");
          L_punt ("Yuck, more implmentation needed. :(");
        }

      /* If necessary, insert a new def operation */
      if (new_def_lcode_op != NULL)
        {
          /* Insert this operation right above the target_sm_op or
           * the def_sm_op, if SM_MUST_REORDER_OPS set.
           *
           * This will build all the proper incoming dependences,
           * some of the outgoing dependences will be created by
           * the SM_change_operand()s below
           */
          /* If we are reordering ops, place new def right above old def */
          if (trans->flags & SM_MUST_REORDER_OPS)
            {
              new_def_sm_op =
                SM_insert_oper_after (sm_cb, new_def_lcode_op,
                                      def_sm_op->prev_serial_op);
            }

          /* Otherwise, place right above target op */
          else
            {
              new_def_sm_op =
                SM_insert_oper_after (sm_cb, new_def_lcode_op,
                                      target_sm_op->prev_serial_op);
            }

          /* Safe the new def pointer so can undo trans */
          trans->new_sm_op = new_def_sm_op;

          /* Set the ext appropriately for this def */
          SM_set_ext (new_def_sm_op, new_def_ext);
        }

      /* Otherwise, flag that no new def was needed */
      else
        {
          new_def_sm_op = NULL;
          trans->new_sm_op = NULL;
        }

      /* Unfortunately, literals, etc. must go in there proper location.
       * Rearrange the operands, if neccessary.  This also may
       * require changing the opc (especially for Lop_SUB).
       */
      SM_make_valid_mcode (&new_target_opc, &new_target_proc_opc,
                           &target_src[0], &target_src[1], &new_target_ext);

      /* If necessary, change opc and proc_opc of target op */
      if ((target_lcode_op->opc != new_target_opc) ||
          (target_lcode_op->proc_opc != new_target_proc_opc))
        {
          /* Get it's original opc and proc_opc so we can undo trans.
           */
          trans->orig_opc = target_lcode_op->opc;
          trans->orig_proc_opc = target_lcode_op->proc_opc;

          /* Use L_change_opcode so the string name is also changed.
           * Changes both opc and proc_opc to new_target_opc.
           */
          L_change_opcode (target_lcode_op, new_target_opc);

          /* Change proc opc to specified value */
          target_lcode_op->proc_opc = new_target_proc_opc;
        }

      /* Save the original ext (so can undo), then change to new ext */
      trans->orig_ext = SM_get_ext (target_sm_op);
      SM_set_ext (target_sm_op, new_target_ext);

      /* If we need to move the target operation before the def operation,
       * do it now.  Does not change dependences, so must be done
       * before changing the target's source operands, so that the
       * proper dependences will be drawn.
       */
      if (trans->flags & SM_MUST_REORDER_OPS)
        {

          /* Note: Place before old def only if need we needed a new 
           * definition (which will go before the target_sm_op).
           * Otherwise, we only need to move right below the existing
           * def because we are always going to to rename the def's dest[0].
           */
          if (need_new_def)
            {
              /* Get it's old serial number so we can undo it */
              trans->orig_prev_serial_op = target_sm_op->prev_serial_op;

              /* Move operation to new location just before old def */
              SM_move_oper_after (target_sm_op, def_sm_op->prev_serial_op);
            }

          /* Don't move if already in correct place */
          else if (target_sm_op->prev_serial_op != def_sm_op)
            {
              /* Get it's old serial number so we can undo it */
              trans->orig_prev_serial_op = target_sm_op->prev_serial_op;

              /* Move operation to new location just after old def.
               * Renaming will take care of any dependences
               * caused by the def's dest[0].
               */
              SM_move_oper_after (target_sm_op, def_sm_op);
            }
        }

      /* Change the target src operands to the operands specified.
       * This will draw the correct dependences.
       */
      SM_change_operand (target_sm_op, MDES_SRC, 0, target_src[0]);
      SM_change_operand (target_sm_op, MDES_SRC, 1, target_src[1]);

      /* Print new def if exists */
      if (new_def_sm_op != NULL)
        {
          if (verbose_optimization)
            {
              printf ("  ==>");
              SM_print_minimal_lcode_op (stdout, new_def_sm_op);
            }
        }
      /* Otherwise, if used existing def, use it */
      else if ((!need_new_def) && !can_delete_old_def)
        {
          if (verbose_optimization)
            {
              printf ("  ==>");
              SM_print_minimal_lcode_op (stdout, def_sm_op);
            }
        }
      /* Remove old def from cb if not need anymore.  Keep around
       * so can undo.  It will be deleted when the trans structure
       * is freed.
       */
      else if (can_delete_old_def)
        {
          if (verbose_optimization)
            {
              printf ("  DELETING: ");
              SM_print_minimal_lcode_op (stdout, def_sm_op);
            }

          /* Save lcode_op in trans structure so we can undo */
          trans->deleted_lcode_op = def_sm_op->lcode_op;

          /* Save the previous sm_op so can undo */
          trans->orig_def_prev_serial_op = def_sm_op->prev_serial_op;

          /* Flag that we deleted the def */
          trans->flags |= SM_DELETED_DEF;

          /* Remove the lcode op from the cb */
          L_remove_oper (def_sm_op->sm_cb->lcode_cb, def_sm_op->lcode_op);

          /* Delete the sm_op */
          SM_delete_oper (def_sm_op);

          /* For sanity, set to NULL */
          def_sm_op = NULL;
        }

      if (verbose_optimization)
        {
          /* Print new target_sm_op */
          printf ("  ==>");
          SM_print_minimal_lcode_op (stdout, target_sm_op);
        }

#if 0
      /* Debug */
      if (new_def_sm_op != NULL)
        {
          SM_print_oper_dependences (stdout, new_def_sm_op);
        }
      else if ((!need_new_def) && (def_sm_op != NULL))
        {
          SM_print_oper_dependences (stdout, def_sm_op);
        }
      SM_print_oper_dependences (stdout, target_sm_op);
#endif
    }

  /* Otherwise, mark transformation as unbeneficial */
  else
    {
      target_sm_op->flags |= SM_EWC_UNBENEFICIAL;

      /* Mark that we didn't do trans, so can properly undo it */
      trans->flags |= SM_CANNOT_DO_TRANS;

      if (verbose_optimization)
        {
          printf ("  left as ==>");
          SM_print_minimal_lcode_op (stdout, target_sm_op);
        }
    }

  /* Free all the copied operands if not NULL */
  if (def_dest != NULL)
    L_delete_operand (def_dest);
  if (def_src[0] != NULL)
    L_delete_operand (def_src[0]);
  if (def_src[1] != NULL)
    L_delete_operand (def_src[1]);
  if (target_src[0] != NULL)
    L_delete_operand (target_src[0]);
  if (target_src[1] != NULL)
    L_delete_operand (target_src[1]);

}

/* Returns 1 if can legally undo expr with copy, 0 otherwise */
int
SM_can_undo_expr_with_copy (SM_Trans * trans)
{
  SM_Dep *dep_out;
  SM_Cb *sm_cb;

  /* Get the sm_cb for ease of use */
  sm_cb = trans->target_sm_op->sm_cb;

  /* If created a def, can undo only if that's def is used
   * by only one operation and that operation is the target_sm_op
   */
  if (trans->new_sm_op != NULL)
    {
      dep_out = trans->new_sm_op->dest[0]->first_dep_out;

      /* If there it not a dep out or there are more than one
       * return 0, cannot undo.
       */
      if ((dep_out == NULL) || (dep_out->next_dep_out != NULL))
        return (0);

      /* If dep_out->to_action is not from the target_sm_op, 
       * return 0, cannot undo.
       */
      if (dep_out->to_action->sm_op != trans->target_sm_op)
        return (0);
    }

  /* Otherwise if modified the orig_def def, 
   * can undo only if that's def is used
   * by only one operation and that operation is the target_sm_op
   */
  else if (trans->orig_def_src[0] != NULL)
    {
      dep_out = trans->def_sm_op->dest[0]->first_dep_out;

      /* If there it not a dep out or there are more than one
       * return 0, cannot undo.
       */
      if ((dep_out == NULL) || (dep_out->next_dep_out != NULL))
        return (0);

      /* If dep_out->to_action is not from the target_sm_op, 
       * return 0, cannot undo.
       */
      if (dep_out->to_action->sm_op != trans->target_sm_op)
        return (0);
    }

  /* For now, if we haven't reordered the operations, I am pretty
   * sure the operation can be undo.  Otherwise, don't let it since
   * transformation after this one may have messed things up.
   */
  if (trans->orig_prev_serial_op == NULL)
    return (1);
  else
    return (0);
}

void
SM_undo_expr_with_copy (SM_Trans * trans)
{
  L_Oper *new_def_lcode_op, *def_lcode_op;
  SM_Oper *def_sm_op;
  SM_Cb *sm_cb;

  /* If trans could not be done in the first place, don't undo */
  if (trans->flags & SM_CANNOT_DO_TRANS)
    {
      /* If necessary, undo renaming before returning */
      if (trans->renaming_sm_op != NULL)
        {
          SM_undo_renaming_with_copy (trans);
        }

      /* Return now, don't undo */
      return;
    }

  /* Get the sm_cb for ease of use */
  sm_cb = trans->target_sm_op->sm_cb;

  /* Move the target op back to it's old location, if necessary. 
   * Do before correcting source operands, so when the operands
   * are changed, the correct dependences will be drawn.
   * Moving will not change the dependences, even if they are now
   * incorrect (they will be fixed by the SM_change_operand()s later).
   */
  if (trans->orig_prev_serial_op != NULL)
    {
      if (verbose_optimization)
        {
          printf ("  AFTER ");
          SM_print_minimal_lcode_op (stdout, trans->orig_prev_serial_op);
        }

      /* Move operation back to it's original location */
      SM_move_oper_after (trans->target_sm_op, trans->orig_prev_serial_op);

      /* Reset the prev_serial_op to prevent confusion */
      trans->orig_prev_serial_op = NULL;
    }

  if (verbose_optimization)
    {
      printf ("\n  UNDOING EXPR_WITH_COPY\n");
      if (trans->new_sm_op != NULL)
        {
          printf ("  UNDO ");
          SM_print_minimal_lcode_op (stdout, trans->new_sm_op);
        }
      printf ("  UNDO ");
      SM_print_minimal_lcode_op (stdout, trans->target_sm_op);
    }

  /* Delete the new definition (if any) */
  if (trans->new_sm_op != NULL)
    {
      /* Get the lcode operation before deleting the sm_op */
      new_def_lcode_op = trans->new_sm_op->lcode_op;

      /* Delete the sm and lcode versions of this operation */
      SM_delete_oper (trans->new_sm_op);
      L_delete_oper (sm_cb->lcode_cb, new_def_lcode_op);

      /* Set to NULL so will not free it when we delete the trans */
      trans->new_sm_op = NULL;
    }

  /* If deleted orig def, reinsert it into cb */
  else if (trans->flags & SM_DELETED_DEF)
    {
      /* Insert deleted operation back into cb,
       * This will build all the proper dependences.
       */
      def_sm_op = SM_insert_oper_after (sm_cb, trans->deleted_lcode_op,
                                        trans->orig_def_prev_serial_op);

      /* Mark that we have undeleted this oper (so won't be freed) */
      trans->deleted_lcode_op = NULL;

      /* Sanity check, undo flag */
      trans->flags &= ~SM_DELETED_DEF;

      if (verbose_optimization)
        {
          printf ("  RESTORED: ");
          SM_print_minimal_lcode_op (stdout, def_sm_op);
        }
    }
  /* Otherwise, if changed original, restore def to original state */
  else if (trans->orig_def_src[0] != NULL)
    {
      /* Get def_sm_op for ease of use */
      def_sm_op = trans->def_sm_op;

      /* Move def back to original location (if necessary) */
      if (trans->orig_def_prev_serial_op != NULL)
        SM_move_oper_after (def_sm_op, trans->orig_def_prev_serial_op);

      /* Change the def operands back to original state */
      SM_change_operand (def_sm_op, MDES_SRC, 0, trans->orig_def_src[0]);
      SM_change_operand (def_sm_op, MDES_SRC, 1, trans->orig_def_src[1]);

      def_lcode_op = def_sm_op->lcode_op;

      /* If necessary, change the opc and proc_opc back */
      if ((def_lcode_op->opc != trans->orig_def_opc) ||
          (def_lcode_op->proc_opc != trans->orig_def_proc_opc))
        {
          /* Use L_change_opcode so string name is also changed */
          L_change_opcode (def_lcode_op, trans->orig_def_opc);

          /* Then change the proc_opc */
          def_lcode_op->proc_opc = trans->orig_def_proc_opc;
        }

      /* Restore the previous ext */
      SM_set_ext (def_sm_op, trans->orig_def_ext);
    }

  /* If necessary, change back target opcode */
  if ((trans->orig_opc != -1) || (trans->orig_proc_opc != -1))
    {
      /* Sanity check, if one is not -1, both better not be -1 */
      if ((trans->orig_opc == -1) || (trans->orig_proc_opc == -1))
        L_punt ("SM_undo_expr_with_copy: bad opc or proc_opc!");

      /* Must use L_change_opcode, so that opcode string
       * will be changed!
       */
      L_change_opcode (trans->target_sm_op->lcode_op, trans->orig_opc);

      /* Change the proc_opc back to specified value. */
      trans->target_sm_op->lcode_op->proc_opc = trans->orig_proc_opc;

      /* Reset, so can redo transformation if necessary */
      trans->orig_opc = -1;
      trans->orig_proc_opc = -1;
    }

  /* Restore the previous ext */
  SM_set_ext (trans->target_sm_op, trans->orig_ext);

  /* Reset the target operands */
  SM_change_operand (trans->target_sm_op, MDES_SRC, 0, trans->orig_src[0]);
  SM_change_operand (trans->target_sm_op, MDES_SRC, 1, trans->orig_src[1]);

  /* Delete the copies of the operands so can redo trans if necessary */
  L_delete_operand (trans->orig_src[0]);
  trans->orig_src[0] = NULL;

  L_delete_operand (trans->orig_src[1]);
  trans->orig_src[1] = NULL;

  if (verbose_optimization)
    {
      printf ("  ==>  ");
      SM_print_minimal_lcode_op (stdout, trans->target_sm_op);
    }

  /* Undo renaming, if necessary */
  if (trans->renaming_sm_op != NULL)
    {
      SM_undo_renaming_with_copy (trans);
    }
}

/* Do the specified transformation, based on trans type */
void
SM_do_trans (SM_Trans * trans)
{
  switch (trans->type)
    {
    case RENAMING_WITH_COPY:
      SM_do_renaming_with_copy (trans);
      break;

    case EXPR_WITH_COPY:
      SM_do_expr_with_copy (trans);
      break;

    default:
      L_punt ("SM_do_trans: Unknown trans->type %i", trans->type);
    }
}

/* Undo the specified transformation, based on trans type */
void
SM_undo_trans (SM_Trans * trans)
{
  switch (trans->type)
    {
    case RENAMING_WITH_COPY:
      /* Mark RWC as unbenefical */
      trans->target_sm_op->flags |= SM_RWC_UNBENEFICIAL;
      SM_undo_renaming_with_copy (trans);
      break;

    case EXPR_WITH_COPY:
      /* Mark EWC as unbenefical */
      trans->target_sm_op->flags |= SM_EWC_UNBENEFICIAL;
      SM_undo_expr_with_copy (trans);
      break;

    default:
      L_punt ("SM_undo_trans: Unknown trans->type %i", trans->type);
    }
}

/* Returns 1 if the transformation can be legally undone at this point,
 * 0 otherwise.  This routine is necessary since doing a transformation
 * may make a previous transformation difficult or impossible to undo.
 *
 * NOTE: A transformation may ALWAYS be immediately undone after it
 *       is performed.  The tricky part comes when there are transformations
 *       after it that mess transformed operations.
 */
int
SM_can_undo_trans (SM_Trans * trans)
{
  switch (trans->type)
    {
    case RENAMING_WITH_COPY:
      /* I believe renaming with copy can always be undone */
      return (1);

    case EXPR_WITH_COPY:
      /* For now, cannot undo expr with copy transformations after
       * another transformation has been done
       */
      return (0 /*SM_can_undo_expr_with_copy (trans) */ );

    default:
      L_punt ("SM_can_undo_trans: Unknown trans->type %i", trans->type);
    }
  L_punt ("SM_can_undo_trans: Should not reach here!");
  return (0);
}

SM_Trans_Queue *
SM_find_potential_trans (SM_Cb * sm_cb, unsigned int allowed_trans)
{
  SM_Trans_Queue *queue;
  SM_Oper *sm_op;
  SM_Trans *trans;

  /* Create a transformation queue */
  queue = SM_new_trans_queue ("Trans found");

  /* Scan each oper, adding any valid transformations to the queue */
  for (sm_op = sm_cb->first_serial_op; sm_op != NULL;
       sm_op = sm_op->next_serial_op)
    {
      /* To facilitate debugging, scan only op's in range */
      if ((sm_op->lcode_op->id < opti_oper_lower_bound) ||
          (sm_op->lcode_op->id > opti_oper_upper_bound))
        continue;

      /* If we are allowing RWC and we have not already found it
       * to be unbeneficial for this op, see if we can perform this
       * transformation on this op.
       */
      if ((allowed_trans & RENAMING_WITH_COPY) &&
          !(sm_op->flags & SM_RWC_UNBENEFICIAL))
        {
          /* If can do renaming with copy on op, add to end of queue */
          if ((trans = SM_can_rename_with_copy (sm_op)) != NULL)
            SM_enqueue_trans_before (queue, trans, NULL);
        }

      /* If we are allowing EWC and we have not already found it
       * to be unbeneficial for this op, see if we can perform this
       * transformation on this op.
       */
      if ((allowed_trans & EXPR_WITH_COPY) &&
          !(sm_op->flags & SM_EWC_UNBENEFICIAL))
        {
          /* If can do expr balancing with copy on op, add to end of queue */
          if ((trans = SM_can_expr_with_copy (sm_op)) != NULL)
            {
              SM_enqueue_trans_before (queue, trans, NULL);
            }
        }
    }

  return (queue);
}

void
SM_delete_trans_and_queue (SM_Trans_Queue * queue)
{
  SM_Trans_Qentry *qentry;
  SM_Trans *trans;

  /* Delete every trans in queue */
  while ((qentry = queue->first_qentry) != NULL)
    {
      /* Get the trans before dequeueing it */
      trans = qentry->trans;

      /* Dequeue it from ALL queues */
      SM_dequeue_trans_from_all (trans);

      /* Delete trans */
      SM_delete_trans (trans);
    }

  /* Delete the queue */
  SM_delete_trans_queue (queue);
}

double
SM_calc_do_trans_priority (SM_Trans * trans, int max_late_time)
{
  double *prob, exit_prob, priority, add_priority = 0.0;
  SM_Oper *sm_op, **exit_op;
  SM_Cb *sm_cb;
  SM_Reg_Action *op_action, *affected_action1, *affected_action2;
  SM_Dep *dep_in;
  int *late_array, late_time, early_time;
  int exit_height, before_height, after_height, total_height;
  int min_height, max_height;
  int index, num_exits;
  int new_early_time, diff_early_time;
  int constraint;


  /* Get sm_cb and sm_op for ease of use */
  sm_op = trans->target_sm_op;
  sm_cb = sm_op->sm_cb;

  /* Assume late and early times have been calculated,
   * get for ease of use.
   */
  early_time = sm_op->early_time;
  late_array = sm_op->late_time;

  /* Currently, transformations may affect up to two action's dep_ins */
  affected_action1 = NULL;
  affected_action2 = NULL;

  /* Get the action affected by the transformation */
  switch (trans->type)
    {
    case RENAMING_WITH_COPY:
      /* Breaks dest[0] anti and output dependences */
      affected_action1 = sm_op->dest[0];
      break;

    case EXPR_WITH_COPY:
      /* Breaks the target's sources flow dependence */
      affected_action1 = sm_op->src[trans->target_index];

      /* If needs to do renaming, will also break dest[0]'s anti
       * and output dependences.  If renaming has already been done
       * (a real possibility), there just will not be any deps to break.
       */
      if (trans->flags & SM_NEEDS_RENAMING)
        {
          affected_action2 = sm_op->dest[0];
        }
      break;

    default:
      L_punt ("SM_calc_do_trans_priority: Unknown trans type %i.",
              trans->type);
    }

  /* Calculate the best-case new early time if we
   * perform the transformation on this operation.
   */
  new_early_time = 0;
  for (op_action = sm_op->first_op_action; op_action != NULL;
       op_action = op_action->next_op_action)
    {
      /* Skip the affected actions */
      if ((op_action == affected_action1) || (op_action == affected_action2))
        continue;

      for (dep_in = op_action->first_dep_in; dep_in != NULL;
           dep_in = dep_in->next_dep_in)
        {
          /* Ignore deps marked with SM_IGNORE_DEP */
          if (dep_in->ignore)
            continue;

          /* Get the constraint this dep imposes */
          constraint = dep_in->from_action->sm_op->early_time +
            dep_in->min_delay;

          /* Update early time with this constraint if necessary */
          if (new_early_time < constraint)
            new_early_time = constraint;
        }

    }

  diff_early_time = early_time - new_early_time;


  /* Get array of exit ops from cb */
  exit_op = sm_cb->exit_op;

  /* Use the exit percentage array as a probability array in calc */
  prob = sm_cb->exit_percentage;

  num_exits = sm_cb->num_exits;

  priority = 0.0;

  for (index = 0; index < num_exits; index++)
    {
      late_time = late_array[index];
      if (late_time == SM_MAX_CYCLE)
        continue;

      /* Get the exit probability.  For zero weight exits, use
       * prob of .00001 so they are taking into account somehow.
       * (This is how it is done in the old dhasy scheduler.)
       */
      if ((exit_prob = prob[index]) == 0.0)
        exit_prob = .00001;

      if (exit_op[index] == NULL)
        exit_height = max_late_time;
      else
        exit_height = exit_op[index]->late_time[index];

      before_height = early_time;
      after_height = exit_height + 1 - late_time;
      total_height = before_height + after_height;

      /* Debug */
      if (after_height < 0)
        {
          printf
            ("op %i[%i] exit_height %i late_time %i after_height %i "
             "max_late %i\n",
             sm_op->lcode_op->id, index, exit_height, late_time, after_height,
             max_late_time);
        }


      if (before_height < after_height)
        {
          min_height = before_height;
          max_height = after_height;
        }
      else
        {
          min_height = after_height;
          max_height = before_height;
        }

      /* If breaking this dep does not immediately let the operation
       * be scheduled earlier, give the operation only a small priority
       * based on the min_height that could occur after the transformation
       * if the operation could be moved to cycle 0.
       */
      /* Fould better results for compress with 4MIX linear
       * search if min_height improvement was more important than
       * total height.
       */
      if (trans->type == RENAMING_WITH_COPY)
        {
#if 0
          add_priority = ((float) diff_early_time +
                          ((float) min_height * 100.0) +
                          (float) total_height * 10000.0) * exit_prob;
#endif
#if 1
          add_priority = ((float) diff_early_time +
                          ((float) total_height * 100.0) +
                          (float) min_height * 10000.0) * exit_prob;
#endif
        }
      else if (trans->type == EXPR_WITH_COPY)
        {
          add_priority = ((float) diff_early_time +
                          ((float) total_height * 100.0) +
                          (float) after_height * 10000.0) * exit_prob;


#if 0
          /* Debug */
          {
            SM_Reg_Action *action;
            int opc;

            /* Try giving preference to expressions in the middle or
             * end of other expressions.
             */
            action = trans->def_sm_op->src[trans->other_index];
            if (action->prev_def != NULL)
              {
                opc = action->prev_def->sm_op->lcode_op->opc;

                if ((opc == Lop_ADD) || (opc == Lop_SUB))
                  {
                    /* Debug */
                    printf ("%s op %i: Increasing priority!\n",
                            sm_cb->lcode_fn->name, sm_op->lcode_op->id);


                    add_priority *= 10.0;
                  }
              }

          }
#endif

        }
      else
        {
          L_punt ("SM_calc_do_trans_priority: Unknown trans pri type %i.",
                  trans->type);
        }



#if 1
      /* Significantly reduce the priority for the operation if
       * there is no dependence height benefit.
       */
      if (diff_early_time <= 0)
        add_priority = add_priority * .001;
#endif

#if 1
      /* Try reducing expr_with_copy priority */
      if (trans->type == EXPR_WITH_COPY)
        add_priority = add_priority * .0001;
#endif


      priority += add_priority;

#if 0
      /* Debug */
      printf
        ("Op %i[%i] mv %i tot_ht %i ben %i exit %.5f add %.2f total %.2f\n",
         sm_op->lcode_op->id, index, diff_early_time, total_height,
         min_height, exit_prob, add_priority, priority);
#endif

    }
#if 0
  printf ("\n");
#endif

#if 0
  /* Debug */
  printf ("Cb %i op %i mv %i tot_ht %i ben %i total %.2f\n",
          sm_cb->lcode_cb->id, sm_op->lcode_op->id, diff_early_time,
          total_height, min_height, priority);
#endif

  return (priority);
}

void
SM_update_sched_based_expr_info (SM_Trans * trans)
{
  SM_Oper *target_sm_op, *def_sm_op;
  SM_Reg_Action *src_action;
  int src0_avail_time, src1_avail_time;
  int def_src0_avail_time, def_src1_avail_time;

  /* Initially, mark as ignore unless find good trans to do */
  trans->flags |= SM_IGNORE_TRANS;

  /* Get the target sm_op for ease of use */
  target_sm_op = trans->target_sm_op;

  /* Get when each operand if available */
  src0_avail_time = SM_calc_action_avail_time (target_sm_op->src[0]);
  src1_avail_time = SM_calc_action_avail_time (target_sm_op->src[1]);

#if 0
  /* Debug */
  printf ("%s op %i: src[0] avail %i src[1] avail %i\n",
          target_sm_op->sm_cb->lcode_fn->name, target_sm_op->lcode_op->id,
          src0_avail_time, src1_avail_time);
#endif

  /* Is src0 the problem and is it possible to transform the expression 
   * coming into src0?  If they are tied, allow either src to 
   * be transformed (if possible).
   */
  if ((src0_avail_time >= src1_avail_time) &&
      (trans->flags & SM_CAN_TRANS_SRC0))
    {
      /* Get the src action for src0 */
      src_action = target_sm_op->src[0];

      /* Error, if the previous defining action is not found! */
      if (src_action->prev_def == NULL)
        L_punt ("SM_update_sched_based_expr_info: Void prev_def!");

      /* Get the operation that defines this src */
      def_sm_op = src_action->prev_def->sm_op;

      /* Yes, get the available times for the defining operation's srcs */
      def_src0_avail_time = SM_calc_action_avail_time (def_sm_op->src[0]);
      def_src1_avail_time = SM_calc_action_avail_time (def_sm_op->src[1]);

      /* For now, point def_sm_op at the defining op.  Fix up later */
      trans->def_sm_op = def_sm_op;

      /* Compair the latest available with src1 */
      if (def_src0_avail_time > def_src1_avail_time)
        {
          /* If src1 is available earlier, we should transform the expr */
          if (src1_avail_time < def_src0_avail_time)
            {
              /* Flag that should not ignore trans */
              trans->flags &= ~SM_IGNORE_TRANS;

              /* Targetting def of src0 and want to
               * place src0 of that def as late as possible.
               */
              trans->target_index = 0;
              trans->other_index = 0;
            }

        }
      else
        {
          /* If it is, swap these two operands */
          if (src1_avail_time < def_src1_avail_time)
            {
              /* Flag that should not ignore trans */
              trans->flags &= ~SM_IGNORE_TRANS;

              /* Targetting def of src0 and want to
               * place src1 of that def as late as possible.
               */
              trans->target_index = 0;
              trans->other_index = 1;
            }
        }
    }

  /* Is src1 the problem and is it possible to transform the expression 
   * coming into src1?  If either can be transformed, and src0 is
   * transformed, do not transform src1.
   */
  if ((src1_avail_time >= src0_avail_time) &&
      (trans->flags & SM_CAN_TRANS_SRC1) && (trans->flags & SM_IGNORE_TRANS))
    {
      /* Get the src action for src1 */
      src_action = target_sm_op->src[1];

      /* Error, if the previous defining action is not found! */
      if (src_action->prev_def == NULL)
        L_punt ("SM_update_sched_based_expr_info: Void prev_def!");

      /* Get the operation that defines this src */
      def_sm_op = src_action->prev_def->sm_op;

      /* Yes, get the available times for the defining operation's srcs */
      def_src0_avail_time = SM_calc_action_avail_time (def_sm_op->src[0]);
      def_src1_avail_time = SM_calc_action_avail_time (def_sm_op->src[1]);

      /* For now, point def_sm_op at the defining op.  Fix up later */
      trans->def_sm_op = def_sm_op;

      /* Compair the latest available with src0 */
      if (def_src0_avail_time > def_src1_avail_time)
        {
          /* If src0 is available earlier, we should transform the expr */
          if (src0_avail_time < def_src0_avail_time)
            {
              /* Flag that should not ignore trans */
              trans->flags &= ~SM_IGNORE_TRANS;

              /* Targetting def of src1 and want to
               * place src0 of that def as late as possible.
               */
              trans->target_index = 1;
              trans->other_index = 0;
            }

        }
      else
        {
          /* If src0 is available earlier, we should transform the expr */
          if (src0_avail_time < def_src1_avail_time)
            {
              /* Flag that should not ignore trans */
              trans->flags &= ~SM_IGNORE_TRANS;

              /* Targetting def of src1 and want to
               * place src1 of that def as late as possible.
               */
              trans->target_index = 1;
              trans->other_index = 1;
            }
        }
    }
}

void
SM_update_sched_based_trans_info (SM_Trans_Queue * queue)
{
  SM_Trans_Qentry *qentry;
  SM_Trans *trans;
  SM_Cb *sm_cb;

  /* Do nothing if the queue is empty */
  if (queue->first_qentry == NULL)
    return;

  /* Sanity check, this cb better be scheduled */
  sm_cb = queue->first_qentry->trans->target_sm_op->sm_cb;
  if (sm_cb->num_unsched != 0)
    {
      L_punt ("SM_update_sched_based_trans_info: %s cb %i %i/%i ops unsched!",
              sm_cb->lcode_fn->name, sm_cb->lcode_cb->id, sm_cb->num_unsched,
              sm_cb->op_count);
    }

  /* For each transformation, get the relevant schedule information */
  for (qentry = queue->first_qentry; qentry != NULL;
       qentry = qentry->next_qentry)
    {
      /* Get the transformation for this qentry */
      trans = qentry->trans;

      /* For now, for every trans, get the sched cycle for the 
       * target op.
       */
      trans->target_sched_cycle = trans->target_sm_op->sched_cycle;

      /* Process base on trans type */
      switch (trans->type)
        {
          /* Do nothing with renaming_with_copy for now */
        case RENAMING_WITH_COPY:
          break;


        case EXPR_WITH_COPY:
          SM_update_sched_based_expr_info (trans);

          break;

        default:
          L_punt ("SM_update_sched_base_trans_info: unknown trans type %i",
                  trans->type);
        }
    }
}

void
SM_update_dep_based_trans_info (SM_Trans_Queue * queue)
{
  SM_Trans_Qentry *qentry;
  SM_Trans *trans;

  /* Do nothing if the queue is empty */
  if (queue->first_qentry == NULL)
    return;

#if 0
  /* Sanity check, this cb better be scheduled */
  sm_cb = queue->first_qentry->trans->target_sm_op->sm_cb;
  if (sm_cb->num_unsched != 0)
    {
      L_punt ("SM_update_dep_based_trans_info: %s cb %i %i/%i ops unsched!",
              sm_cb->lcode_fn->name, sm_cb->lcode_cb->id, sm_cb->num_unsched,
              sm_cb->op_count);
    }
#endif

  /* For each transformation, get the relevant schedule information */
  for (qentry = queue->first_qentry; qentry != NULL;
       qentry = qentry->next_qentry)
    {
      /* Get the transformation for this qentry */
      trans = qentry->trans;

      /* Process base on trans type */
      switch (trans->type)
        {
          /* Do nothing for renaming with copy */
        case RENAMING_WITH_COPY:
          break;

          /* For now, just mark expr with copy so they are ignored */
        case EXPR_WITH_COPY:
          trans->flags |= SM_IGNORE_TRANS;
          break;

        default:
          L_punt ("SM_update_dep_based_trans_info: unknown trans type %i",
                  trans->type);
        }
    }
}

/* Returns the highest priority transformation to do in queue.
 * Returns NULL if queue is empty.
 */
SM_Trans *
SM_dequeue_best_do_trans (SM_Trans_Queue * queue)
{
  SM_Cb *sm_cb;
  SM_Trans_Qentry *qentry, *max_qentry;
  SM_Trans *trans;
  double priority, max_priority;
  int max_late_time;

  /* Return NULL if there are no transformations left in the queue */
  if (queue->first_qentry == NULL)
    return (NULL);

  /* Get the sm_cb from the first qentry */
  sm_cb = queue->first_qentry->trans->target_sm_op->sm_cb;

  /* Calculate early and late times for this cb */
  max_late_time = SM_calculate_early_times (sm_cb, 0);
  SM_calculate_late_times (sm_cb, max_late_time);

  /* Initialize max priority and max qentry */
  max_priority = SM_MIN_CYCLE;
  max_qentry = NULL;

  /* Scan each trans in the queue, find the trans with the highest priority */
  for (qentry = queue->first_qentry; qentry != NULL;
       qentry = qentry->next_qentry)
    {
      /* Skip transformations marked to ignore */
      if (qentry->trans->flags & SM_IGNORE_TRANS)
        continue;

      /* Calculate the priority for this transformation */
      priority = SM_calc_do_trans_priority (qentry->trans, max_late_time);

      /* Update max priority if necessary */
      if (priority > max_priority)
        {
          max_priority = priority;
          max_qentry = qentry;
        }
    }

  /* If all the transformations are marked INGORE, return NULL */
  if (max_qentry == NULL)
    return (NULL);


  /* Get the transformation for the max_qentry and dequeue it */
  trans = max_qentry->trans;
  SM_dequeue_trans (max_qentry);

  /* Return the transformation selected */
  return (trans);
}

int
SM_calc_action_avail_time (SM_Reg_Action * action)
{
  SM_Dep *dep_in;
  int avail_time, constraint;

  /* Assume constants are available in cycle -1, to encourage transformations
   * that cause constants to be combined. */
  if (action == NULL)
    return (-1);

  /* Initialize to cycle 0 */
  avail_time = 0;

  for (dep_in = action->first_dep_in; dep_in != NULL;
       dep_in = dep_in->next_dep_in)
    {
      /* Calculate the constraint due to this operation */
      constraint = dep_in->from_action->sm_op->sched_cycle +
        dep_in->min_delay;

      if (constraint > avail_time)
        avail_time = constraint;
    }

  return (avail_time);
}
