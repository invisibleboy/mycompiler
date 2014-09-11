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
 *      File :          l_loop_opti.c
 *      Description :   loop optimization
 *      Info Needed :   Dominator info, live var info
 *      Creation Date : July, 1990
 *      Author :        Scott Mahlke, Pohua Chang.
 *
 *      Addition :      Add in real support for generating post inc ld/st's,
 *                      April 1994.
 *
 *      (C) Copyright 1990, Pohua Chang, Scott Mahlke.
 *      All rights granted to University of Illinois Board of Regents.
 *==========================================================================*/
/* 02/07/03 REK Modifying L_loop_invariant_code_removal, 
 *              L_loop_global_var_migration,
 *              L_loop_global_var_migration_by_sync,
 *              L_loop_induction_strength_reduction, L_loop_induction_reinit,
 *              L_loop_induction_reassociation, 
 *              L_loop_post_increment_conversion, to not touch opers marked
 *              volatile.
 */

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "l_opti.h"
#include <library/dynamic_symbol.h>
#include <library/i_list.h>

#define BUFSIZE 1024

#define ERR     stderr

#define LOOP_USE_WEIGHT_METRIC
#define LOOP_USE_CUTSET_METRIC
#define LOOP_CUTSET_RATIO 0.5
#undef DEBUG_LOOP_METRICS

#undef DEBUG_FIND_IND_VAR
#undef DEBUG_LOOP_SIMPLIFY_LOOP_BR
#undef DEBUG_LOOP_INVAR_CODE_REM
#undef DEBUG_LOOP_IND_STR_RED
#undef DEBUG_LOOP_IND_ELIM
#undef DEBUG_LOOP_IND_REASSOC
#undef DEBUG_LOOP_IND_REINIT
#undef DEBUG_LOOP_POST_INC_CONVERSION
#undef DEBUG_LOOP_GLOBAL_VAR_MIG

static L_Oper *
L_insert_alias_compensation_store (L_Cb * old_cb,
				   L_Oper * store_oper,
				   L_Oper * alias_oper,
				   int st_opc,
				   L_Operand * st_s1,
				   L_Operand * st_s2,
				   L_Operand * st_s3,
				   L_Attr * attr, int flags,
				   int safe, L_Oper * cond_set);

static void
L_insert_alias_compensation_load (L_Cb * cb, L_Oper * old_oper,
				  L_Oper * alias_oper, int ld_opc,
				  L_Operand * ld_dest,
				  L_Operand * ld_s1,
				  L_Operand * ld_s2,
				  L_Attr * attr, int flags,
				  int safe);

static L_Oper *
L_insert_load_ops_for_global_var_mig (L_Loop *, L_Oper *, int,
				      L_Operand *, L_Operand *,
				      L_Operand *, L_Attr *,
				      int, int, L_Cb *); 

static void 
L_insert_store_ops_for_global_var_mig (L_Loop *, int *, int,
				       L_Oper *, int, L_Operand *,
				       L_Operand *, L_Operand *,
				       L_Attr *, int, int,
				       L_Oper **pred_set);


/*
 * Loop invariant code motion (LICM)
 * ----------------------------------------------------------------------
 * Assumes loop has preheader.
 */

int
L_loop_invariant_code_removal (L_Loop * loop)
{
  int change, i, num_cb, *loop_cb = NULL, num_exit_cb, *exit_cb = NULL,
    suppress_flag = 0;
  L_Cb *cb;
  L_Oper *op, *next_op;
  change = suppress_flag = 0;

  /*
   * Use cost function to determine if should move ops to preheader.
   * This is necessary because some machines have few registers so
   * if make too many vars live in loop will get negative perf.
   */
  if (!L_cost_effective_to_move_ops_to_preheader (loop))
    return 0;

#if defined(LOOP_USE_CUTSET_METRIC)
  {
    int cnt_livein, num_regs;

    cnt_livein = Set_size (L_get_cb_IN_set(loop->header));
    num_regs = M_num_registers (L_native_machine_ctype);
    if (cnt_livein > LOOP_CUTSET_RATIO * num_regs)
      {
#if defined(DEBUG_LOOP_METRICS) || defined(DEBUG_LOOP_INVAR_CODE_REM)
	fprintf (ERR, ">LICM> suppressed for loop header cb %d"
		 "(cutset %d/%d)\n",
		 loop->header->id, cnt_livein, num_regs);
#endif
	suppress_flag = 1;
      }
  }
#endif

  if ((num_cb = Set_size (loop->loop_cb)))
    {
      loop_cb = (int *) alloca (sizeof (int) * num_cb);
      Set_2array (loop->loop_cb, loop_cb);
    }

  /* setup exit_cb array */
  if ((num_exit_cb = Set_size (loop->exit_cb)))
    {
      exit_cb = (int *) alloca (sizeof (int) * num_exit_cb);
      Set_2array (loop->exit_cb, exit_cb);
    }

  for (i = 0; i < num_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);

      if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_BOUNDARY))
        continue;

#if defined(LOOP_USE_WEIGHT_METRIC)
      if (cb->weight < loop->preheader->weight)
	{
#if defined(DEBUG_LOOP_METRICS) || defined(DEBUG_LOOP_INVAR_CODE_REM)
	  fprintf (ERR, ">LICM> Under [hdr %d,phd %d] suppressed "
		   "for cb %d (weight)\n", loop->header->id,
		   loop->preheader->id, cb->id);
#endif
	  continue;
	}
#endif

      for (op = cb->first_op; op; op = next_op)
        {
          int safe, macro_flag, load_flag, store_flag;

          next_op = op->next_op;

	  /* 02/07/03 REK Adding a check to make sure we don't touch a
	   *              volatile oper. */
	  if (L_EXTRACT_BIT_VAL (op->flags, L_OPER_VOLATILE))
	    continue;

          /*
           *  match pattern
           */
          if (!L_can_move_opcode (op))
            continue;
	  if (suppress_flag && !L_store_opcode (op))
	    continue;
          if (!L_loop_invariant_operands (loop, loop_cb, num_cb, op))
            continue;
          if (!L_unique_def_in_loop (loop, loop_cb, num_cb, op))
            continue;
          if (!L_all_uses_in_loop_from (loop, loop_cb, num_cb, cb, op))
            continue;
          if (!L_def_reachs_all_out_cb_of_loop (loop, exit_cb, 
						num_exit_cb, cb, op))
            continue;
          safe = L_safe_to_move_out_of_loop (loop, exit_cb, num_exit_cb, 
					     cb, op);
          if (!safe && !L_non_excepting_ops)
            continue;
          macro_flag = L_has_fragile_macro_operand (op);
          load_flag = L_general_load_opcode (op);
          store_flag = L_general_store_opcode (op);
          if (!(L_no_danger_in_loop (loop, loop_cb, num_cb, macro_flag,
                                     load_flag, store_flag)))
            continue;
          if ((load_flag | store_flag) &&
              !L_no_memory_conflicts_in_loop (loop, loop_cb, num_cb, 
					      op, NULL))
            continue;

          /*
           *  replace pattern
           */
#ifdef DEBUG_LOOP_INVAR_CODE_REM
          fprintf (ERR, ">LICM> APPLIED op %d (cb %d) -> (cb %d) : %lf\n",
                   op->id, cb->id, loop->preheader->id, op->weight);
          if (cb->region != loop->header->region)
            Lopti_inter_region_loop_inv_wgt += op->weight;
#endif
          if (L_mask_potential_exceptions && !safe &&
	      L_general_load_opcode (op) &&
              !L_EXTRACT_BIT_VAL (op->flags, L_OPER_MASK_PE))
	    L_insert_check_after (cb, op, op);

          L_move_op_to_end_of_block (cb, loop->preheader, op);

          if (L_mask_potential_exceptions && !safe)
            L_mark_oper_speculative (op);

          change++;
        }
    }

  return change;
}

/*
 * Loop global variable migration (LGVM)
 * ----------------------------------------------------------------------
 * Assumes loop has preheader.
 */

int
L_loop_global_var_migration (L_Loop * loop)
{
  int change = 0, i, j, k, num_cb, *loop_cb = NULL, num_out_cb, *out_cb = NULL,
    num_exit_cb, *exit_cb = NULL, n_read, n_write, new_load_opc,
    new_store_opc, suppress_flag = 0;
  int num_be_cb, *be_cb = NULL;
  double ld_cost, st_cost, total;
  L_Cb *cb;
  L_Oper *op, *ptr, *store_op, *new_load, *pred_set = NULL;
  L_Operand *new_reg, *s1, *s2;

  /*
   * Use cost function to determine if should move ops to preheader.
   * This is necessary because some machines have few registers so
   * if make too many vars live in loop will get negative perf.
   */
  if (!L_cost_effective_to_move_ops_to_preheader (loop))
    return 0;


#if defined(LOOP_USE_CUTSET_METRIC)
  {
    int cnt_livein, num_regs;
    cnt_livein = Set_size (L_get_cb_IN_set(loop->header));
    num_regs = M_num_registers (L_native_machine_ctype);
    if (cnt_livein > LOOP_CUTSET_RATIO * num_regs)
      {
#if defined(DEBUG_LOOP_METRICS) || defined (DEBUG_LOOP_IND_STR_RED)
	fprintf (ERR, ">LGVM> SUPPRESSED CUTSET (cutset %d/%d)\n",
		 cnt_livein, num_regs);
#endif
	suppress_flag = 1;
      }
  }
#endif

  /*  
   * REH 12-4-95, if backedge is a boundary cb, don't perform
   *  global var migration on this loop
   */

  if ((num_be_cb = Set_size (loop->back_edge_cb)))
    {
      be_cb = (int *) alloca (sizeof (int) * num_be_cb);
      Set_2array (loop->back_edge_cb, be_cb);
      for (i = 0; i < num_be_cb; i++)
        {
          cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, be_cb[i]);
          if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_BOUNDARY))
	    return (0);
        }
    }

  if ((num_cb = Set_size (loop->loop_cb)))
    {
      loop_cb = (int *) alloca (sizeof (int) * num_cb);
      Set_2array (loop->loop_cb, loop_cb);
    }

  if ((num_exit_cb = Set_size (loop->exit_cb)))
    {
      exit_cb = (int *) alloca (sizeof (int) * num_exit_cb);
      Set_2array (loop->exit_cb, exit_cb);
    }

  if ((num_out_cb = Set_size (loop->out_cb)))
    {
      out_cb = (int *) Lcode_malloc (sizeof (int) * num_out_cb);
      Set_2array (loop->out_cb, out_cb);
    }

  for (i = 0; i < num_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);

      for (op = cb->first_op; op; op = op->next_op)
        {
          int safe, macro_flag, load_flag, store_flag;
          /*
           *  Match pattern
           */

          if (!((L_load_opcode (op) && !suppress_flag)|| L_store_opcode (op)))
            continue;
	  /* 02/07/03 REK Adding a check to make sure we don't touch a
	   *              volatile oper. */
	  if (L_EXTRACT_BIT_VAL (op->flags, L_OPER_VOLATILE))
	    continue;
          if (!(L_is_loop_inv_operand (loop, loop_cb, num_cb, op->src[0])))
            continue;
          if (!(L_is_loop_inv_operand (loop, loop_cb, num_cb, op->src[1])))
            continue;
          safe =
            L_safe_to_move_out_of_loop (loop, exit_cb, num_exit_cb, cb, op);
          if (!L_non_excepting_ops && !safe)
            continue;
          macro_flag = L_has_fragile_macro_operand (op);
          load_flag = 1;
          store_flag = 1;
          if (!(L_no_danger_in_loop (loop, loop_cb, num_cb, macro_flag,
                                     load_flag, store_flag)))
            continue;
          if (!(L_unique_memory_location (loop, loop_cb, num_cb, op, &n_read,
                                          &n_write, &store_op)))
            continue;

          /* 
           * See if this whole thing makes sense based upon execution freq.
           */

          s1 = op->src[0];
          s2 = op->src[1];

          st_cost = 0.0;
          ld_cost = 0.0;
          for (j = 0; j < num_cb; j++)
            {
              L_Cb *cb2;

              cb2 = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[j]);

              for (ptr = cb2->first_op; ptr; ptr = ptr->next_op)
                {
                  if (L_load_opcode (ptr))
                    {
                      if (!(L_same_operand (s1, ptr->src[0])))
                        continue;
                      if (!(L_same_operand (s2, ptr->src[1])))
                        continue;
                      ld_cost += ptr->weight;

                      if (cb2->region != loop->header->region)
                        Lopti_inter_region_gvm_wgt += ptr->weight;
                    }
                  else if (L_store_opcode (ptr))
                    {
                      if (!(L_same_operand (s1, ptr->src[0])))
                        continue;
                      if (!(L_same_operand (s2, ptr->src[1])))
                        continue;
                      st_cost += ptr->weight;

                      if (cb2->region != loop->header->region)
                        Lopti_inter_region_gvm_wgt += ptr->weight;
                    }
                }
            }

          total = ld_cost + st_cost;

          for (k = 0; k < num_out_cb; k++)
            {
              L_Cb *cb2 = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, out_cb[k]);
              total -= cb2->weight;
            }

          total -= loop->preheader->weight;

#if defined(LOOP_USE_WEIGHT_METRIC)
          if (total < 0.0)
            {
#if defined(DEBUG_LOOP_METRICS)
              fprintf (ERR, ">LGVM> suppressed for cb %d op %d (weight)\n",
		       cb->id, op->id);
#endif
              continue;
            }
#endif
          /*
           *  Replace pattern
           */
#ifdef DEBUG_LOOP_GLOBAL_VAR_MIG
          fprintf (ERR, ">LGVM> APPLIED op%d (cb %d) : %lf\n",
                   op->id, cb->id, total);
#endif

          s1 = L_copy_operand (s1);
          s2 = L_copy_operand (s2);

          new_load_opc = L_corresponding_load (op);
          new_store_opc = L_corresponding_store (op);
          new_reg = L_new_register_operand (++L_fn->max_reg_id,
                                            L_opcode_ctype2 (op),
                                            L_PTYPE_NULL);
          new_load =
            L_insert_load_ops_for_global_var_mig (loop, op, new_load_opc,
                                                  new_reg, op->src[0],
                                                  op->src[1], op->attr,
                                                  op->flags, safe, cb);

          if (n_write > 0)
	    {
	      if (pred_set)
		{
		  L_delete_oper (NULL, pred_set);
		  pred_set = NULL;
		}
	      L_insert_store_ops_for_global_var_mig (loop, out_cb, num_out_cb,
						     store_op, new_store_opc,
						     op->src[0], op->src[1],
						     new_reg, op->attr,
						     op->flags, safe,
						     &pred_set);
	    }

          for (j = 0; j < num_cb; j++)
            {
              L_Cb *cb2 = NULL;

              cb2 = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[j]);

              for (ptr = cb2->first_op; ptr != NULL; ptr = ptr->next_op)
                {
		  /* 02/07/03 REK Adding a check to make sure we don't touch a
		   *              volatile oper. */
		  if (L_EXTRACT_BIT_VAL (ptr->flags, L_OPER_VOLATILE))
		    continue;

                  if (L_load_opcode (ptr))
                    {
                      if (!(L_same_operand (s1, ptr->src[0])))
                        continue;
                      if (!(L_same_operand (s2, ptr->src[1])))
                        continue;

		      if (ptr->opc != new_load_opc)
			L_warn("L_loop_global_var_migration: loads of "
			       "different sizes detected.  This is very bad. "
			       "See RDB.");
			
                      if (!safe && L_mask_potential_exceptions &&
                          !(L_EXTRACT_BIT_VAL (ptr->flags, L_OPER_MASK_PE)))
                        {
                          L_insert_check_before (cb2, new_load, ptr);
                        }
                      L_convert_to_extended_move (ptr,
                                                  L_copy_operand (ptr->
                                                                  dest[0]),
                                                  L_copy_operand (new_reg));
                    }
                  else if (L_store_opcode (ptr))
                    {
		      L_Oper *pred_set_op;
                      if (!(L_same_operand (s1, ptr->src[0])))
                        continue;
                      if (!(L_same_operand (s2, ptr->src[1])))
                        continue;

		      if (ptr->opc != new_store_opc)
			L_warn("L_loop_global_var_migration: stores of "
			       "different sizes detected.  This is very bad. "
			       "See RDB.");

		      if (pred_set)
			{
			  if (Lopti_store_migration_mode == 
			      L_STORE_MIGRATION_FULL_PRED)
			    cb2->flags = L_SET_BIT_FLAG (cb2->flags, 
							 L_CB_HYPERBLOCK);
			  pred_set_op = L_copy_operation(pred_set);
			  pred_set_op->pred[0] = L_copy_operand(ptr->pred[0]);
			  L_insert_oper_after(cb2,ptr, pred_set_op);
			}
                      L_convert_to_move (ptr, L_copy_operand (new_reg),
                                         L_copy_operand (ptr->src[2]));
                    }
                }
            }
          /* Need to resetup out_cb array, since global var mig changes this */

          /* REH 12/95 - If num_out_cb is 0, then array was not malloc'd */
          if (num_out_cb > 0)
            Lcode_free (out_cb);
          num_out_cb = Set_size (loop->out_cb);
          if (num_out_cb > 0)
            {
              out_cb = (int *) Lcode_malloc (sizeof (int) * num_out_cb);
              Set_2array (loop->out_cb, out_cb);
            }
          /* dispose of temporary operands */
          L_delete_operand (new_reg);
          L_delete_operand (s1);
          L_delete_operand (s2);
          STAT_COUNT ("L_loop_global_var_mig", 1, cb);
          change++;
        }
    }

  if (out_cb)
    Lcode_free (out_cb);
  if (pred_set)
    L_delete_oper (NULL, pred_set);

  return change;
}

/* 
 * Invariant address access (IAA) data structures
 * ----------------------------------------------------------------------
 */

/* BCC - data structure for global variable migration - 1/31/99 */

typedef struct _invariant_address_access {
  List store_mem_op;
  List load_mem_op;
  List alias_store_mem_op;   /* stores potentially writing variable */
  List alias_load_mem_op;    /* loads potentially reading variable */
  List alias_store_jsr;      /* JSRs potentially writing variable */
  List alias_load_jsr;       /* JSRs potentially reading variable */
  int safe;
  int is_block_move;
  double weight;
  double benefit;
} invariant_address_access;

struct _operand_node;

typedef struct _operand_pair {
  int opc;
  ITuint8 com[L_MAX_CMPLTR];
  struct _operand_node *operand1, *operand2;
  struct _operand_pair *next;
} operand_pair;

typedef struct _operand_node {
  L_Operand *operand;
  int DFS_search_flag;
  struct _operand_pair *def;
  int poison; /* Set to 1 if operand is redefined in the loop */
} operand_node;

static L_Alloc_Pool *Invariant_Address_Access_Pool;
static L_Alloc_Pool *Operand_Node_Pool;
static L_Alloc_Pool *Operand_Pair_Pool;

static STRING_Symbol_Table *operand_node_tbl;


static invariant_address_access *
L_new_invariant_address_access ()
{
  invariant_address_access *new;

  if (!Invariant_Address_Access_Pool)
    Invariant_Address_Access_Pool =
      L_create_alloc_pool ("Invariant_Address_Access_Pool",
                           sizeof (invariant_address_access), 2048);

  new = L_alloc (Invariant_Address_Access_Pool);
  new->store_mem_op = NULL;
  new->load_mem_op = NULL;
  new->alias_store_mem_op = NULL;
  new->alias_load_mem_op = NULL;
  new->alias_store_jsr = NULL;
  new->alias_load_jsr = NULL;
  new->safe = 0;
  new->is_block_move = 0;
  new->weight = 0.0;
  new->benefit = 0.0;
  return new;
}

static void
L_free_invariant_address_access (void *vp)
{
  invariant_address_access *p;

  p = vp;
  p->store_mem_op = List_reset (p->store_mem_op);
  p->load_mem_op = List_reset (p->load_mem_op);
  p->alias_store_mem_op = List_reset (p->alias_store_mem_op);
  p->alias_load_mem_op = List_reset (p->alias_load_mem_op);
  p->alias_store_jsr = List_reset (p->alias_store_jsr);
  p->alias_load_jsr = List_reset (p->alias_load_jsr);
  L_free (Invariant_Address_Access_Pool, p);
}

static void
L_generate_operand_name (L_Operand * oper, char *oper_name)
{
  int cnt;

  switch (oper->type)
    {
    case L_OPERAND_VOID:
      cnt = snprintf (oper_name, BUFSIZE, "void");
      break;
    case L_OPERAND_CB:
      cnt = snprintf (oper_name, BUFSIZE, "cb%d", oper->value.cb->id);
      break;
    case L_OPERAND_IMMED:
      cnt = snprintf (oper_name, BUFSIZE, "i" ITintmaxformat, oper->value.i);
      break;
    case L_OPERAND_STRING:
      cnt = snprintf (oper_name, BUFSIZE, "s%s", oper->value.s);
      break;
    case L_OPERAND_LABEL:
      cnt = snprintf (oper_name, BUFSIZE, "l%s", oper->value.l);
      break;
    case L_OPERAND_MACRO:
      cnt = snprintf (oper_name, BUFSIZE, "mac%d", oper->value.mac);
      break;
    case L_OPERAND_REGISTER:
      cnt = snprintf (oper_name, BUFSIZE, "r%d", oper->value.r);
      break;
    case L_OPERAND_RREGISTER:
      cnt = snprintf (oper_name, BUFSIZE, "rr%d", oper->value.rr);
      break;
    case L_OPERAND_EVR:
      cnt = snprintf (oper_name, BUFSIZE, "evr%d", oper->value.evr.num);
      break;
    default:
      cnt = -1;
      L_punt ("L_generate_operand_name: see Ben");
    }

  if (cnt < 0)
    L_punt ("L_generate_operand_name: buffer size exceeded");

  return;
}

static operand_node *
L_new_operand_node (L_Oper * oper, L_Operand * op, Set loop_ops)
{
  operand_node *new;
  Set def_set;

  if (!Operand_Node_Pool)
    Operand_Node_Pool = L_create_alloc_pool ("Operand_Node_Pool",
                                             sizeof (operand_node), 2048);

  new = L_alloc (Operand_Node_Pool);
  new->poison = 0;

  if (L_is_variable (op))
    {
      def_set = L_get_oper_RIN_defining_opers (oper, op);
      if (!Set_intersect_empty (def_set, loop_ops) &&
          (Set_size (def_set) != 1))
        new->poison = 1;
      Set_dispose (def_set);
    }

  new->operand = L_copy_operand (op);
  new->DFS_search_flag = 0;
  new->def = NULL;
  return new;
}

static operand_node *
L_find_ast_node (L_Oper *op, L_Operand *opd, Set loop_ops)
{
  char buffer[BUFSIZE];
  STRING_Symbol *symbol;
  operand_node *node;

  if (!opd)
    return NULL;

  L_generate_operand_name (opd, buffer);

  if ((symbol = STRING_find_symbol (operand_node_tbl,
				    buffer)))
    {
      node = symbol->data;
    }
  else
    {
      node = L_new_operand_node (op, opd, loop_ops);
      STRING_add_symbol (operand_node_tbl, buffer, node);
    }

  return node;
}			

static operand_pair *
L_new_operand_pair (L_Oper *op, Set loop_ops)
{
  operand_pair *new;
  int i;

  if (!Operand_Pair_Pool)
    Operand_Pair_Pool = L_create_alloc_pool ("Operand_Pair_Pool",
                                             sizeof (operand_pair), 2048);

  new = L_alloc (Operand_Pair_Pool);

  new->opc = op->opc;
  for (i = 0; i < L_MAX_CMPLTR; i++)
    new->com[i] = op->com[i];
  new->next = NULL;

  new->operand1 = op->src[0] ? 
    L_find_ast_node (op, op->src[0], loop_ops) : NULL;

  new->operand2 = op->src[1] ? 
    L_find_ast_node (op, op->src[1], loop_ops) : NULL;

  return new;
}

static void
L_delete_operand_pair (operand_pair * p)
{
  if (p->next)
    L_delete_operand_pair (p->next);
  L_free (Operand_Pair_Pool, p);
}

static void
L_delete_operand_node (operand_node * p)
{
  if (p)
    {
      L_delete_operand (p->operand);
      if (p->def)
	L_delete_operand_pair (p->def);
      L_free (Operand_Node_Pool, p);
    }
}

static int
L_def_already_found (operand_node * op_node, L_Oper * op)
{
  operand_pair *op_pair;

  for (op_pair = op_node->def; op_pair; op_pair = op_pair->next)
    {
      if (op_pair->opc != op->opc)
        continue;
      if (L_same_operand (op_pair->operand1 ? op_pair->operand1->operand : NULL,
                          op->src[0]) &&
          L_same_operand (op_pair->operand2 ? op_pair->operand2->operand : NULL,
                          op->src[1]))
        return 1;
    }
  return 0;
}

static void
L_build_ast_for_reg_in_loop (L_Loop * loop, int *loop_cb, int num_cb)
{
  L_Cb *cb;
  L_Oper *op;
  int i, j;
  STRING_Symbol *symbol;
  char buffer[BUFSIZE];
  int symbol_count;
  Set loop_ops = NULL;
  operand_node *op_node;
  operand_pair *op_pair;

  operand_node_tbl = STRING_new_symbol_table ("operand_node_tbl", 32);

  /* Build the loop ops set */

  for (i = 0; i < num_cb; i++)
    {
      if (!(cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i])))
        L_punt ("L_build_ast_for_reg_in_loop: cb is corrupt");

      for (op = cb->first_op; op; op = op->next_op)
	loop_ops = Set_add (loop_ops, op->id);
    }

  /* Seed the database with a trivial record for each register
   * sourced in a load or store in the loop
   */

  for (i = 0; i < num_cb; i++)
    {
      if (!(cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i])))
        L_punt ("L_build_ast_for_reg_in_loop: cb is corrupt");

      for (op = cb->first_op; op; op = op->next_op)
	if (L_load_opcode (op) || L_store_opcode (op))
	  {
	    for (j = 0; j < 2; j++)
	      {
		L_generate_operand_name (op->src[j], buffer);
		if (!(symbol = STRING_find_symbol (operand_node_tbl, buffer)))
		  {
		    op_node = L_new_operand_node (op, op->src[j], loop_ops);
		    STRING_add_symbol (operand_node_tbl, buffer, op_node);
		  }
	      }
	  }
    }

  do
    {
      symbol_count = operand_node_tbl->symbol_count;
      for (i = 0; i < num_cb; i++)
        {
          if (!(cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i])))
            L_punt ("L_build_ast_for_reg_in_loop: cb is corrupt");

          for (op = cb->first_op; op; op = op->next_op)
	    if (op->dest[0] && L_is_variable (op->dest[0]))
	      {
		L_generate_operand_name (op->dest[0], buffer);

		if (!(symbol = STRING_find_symbol (operand_node_tbl, buffer)))
		  continue;
		
		op_node = symbol->data;
		
		if (L_def_already_found (op_node, op))
		  continue;

		op_pair = L_new_operand_pair (op, loop_ops);

		op_pair->next = op_node->def;
		op_node->def = op_pair;
	      }
	}
    }
  while (symbol_count != operand_node_tbl->symbol_count);

  Set_dispose (loop_ops);

  return;
}

static void
L_delete_ast_for_reg_in_loop ()
{
  STRING_Symbol *symbol;
  operand_node *op_node;

  for (symbol = operand_node_tbl->head_symbol;
       symbol; symbol = symbol->next_symbol)
    {
      op_node = symbol->data;
      L_delete_operand_node (op_node);
    }
  STRING_delete_symbol_table (operand_node_tbl, 0);
}


static int
L_find_oper_acc_name (L_Operand *operand, char *acc_name)
{
  char operand_name[BUFSIZE], operand1_name[BUFSIZE], operand2_name[BUFSIZE];
  char tentative_acc_name[BUFSIZE];
  STRING_Symbol *symbol;
  operand_node *op_node;
  operand_pair *op_pair;

  acc_name[0] = 0;

  /* MCM 7/2000 Return registers must be protected.  There may
     be a better way to do this, but it works for now and is
     better than the old hard-coded P15. */

  if (operand->type == L_OPERAND_MACRO)
    {
      if ((operand->ctype == L_CTYPE_INT || operand->ctype == L_CTYPE_LLONG)
          && operand->value.mac == M_return_register (M_TYPE_INT, 0))
	return 0;

      if ((operand->ctype == L_CTYPE_FLOAT) &&
          operand->value.mac == M_return_register (M_TYPE_FLOAT, 0))
	return 0;

      if ((operand->ctype == L_CTYPE_DOUBLE) &&
          operand->value.mac == M_return_register (M_TYPE_DOUBLE, 0))
	return 0;
    }

  L_generate_operand_name (operand, operand_name);

  if (!(symbol = STRING_find_symbol (operand_node_tbl, operand_name)))
    L_punt ("L_find_oper_acc_name: operand not connected to def graph");

  op_node = symbol->data;

  if (op_node->DFS_search_flag)
    {
      op_node->DFS_search_flag = 0;
      return 0;
    }

  op_node->DFS_search_flag = 1;

  if (!op_node->def)
    {
      strcpy (acc_name, operand_name);
      op_node->DFS_search_flag = 0;
      return (acc_name[0] != 0);
    }
  tentative_acc_name[0] = 0;

  for (op_pair = op_node->def; op_pair; op_pair = op_pair->next)
    {
      /* more than one level of loads */
      if (op_pair->opc >= Lop_LD_UC && op_pair->opc <= Lop_LD_F2)
        {
          op_node->DFS_search_flag = 0;
          return 0;
        }

      if (op_pair->operand1)
        {
	  if (op_pair->operand1->poison ||
	      !L_find_oper_acc_name (op_pair->operand1->operand, operand1_name)) 
	    {
	      op_node->DFS_search_flag = 0;
	      return 0;
	    }
        }
      else
        {
          operand1_name[0] = '\0';
        }

      if (op_pair->operand2)
        {
	  if (op_pair->operand2->poison ||
	      !L_find_oper_acc_name (op_pair->operand2->operand, operand2_name)) 
	    {
	      op_node->DFS_search_flag = 0;
	      return 0;
	    }
        }
      else
        {
          operand2_name[0] = '\0';
        }

      if (operand1_name[0] != '\0' &&
          (operand2_name[0] == '\0' || !strcmp (operand2_name, "i0")))
        strcpy (acc_name, operand1_name);
      else if (operand1_name[0] == '\0' && operand2_name[0] != '\0')
        strcpy (acc_name, operand2_name);
      else
        {
	  int cnt;
          cnt = snprintf (acc_name, BUFSIZE, "(%s %s %s)", operand1_name,
			  L_opcode_name (op_pair->opc), operand2_name);
	  if (cnt < 0)
	    L_punt ("L_find_oper_acc_name: buffer size exceeded");
        }

      /* more than one definition is found */

      if (tentative_acc_name[0] && strcmp (tentative_acc_name, acc_name))
        {
          op_node->DFS_search_flag = 0;
          return 0;
        }
      else
	{
	  strcpy (tentative_acc_name, acc_name);
	}
    }

  /* unique definition is found */
  op_node->DFS_search_flag = 0;
  return (acc_name[0] != 0);
}

static L_Operand *
L_initialize_addr_register (L_Loop * loop, operand_node * op_node)
{
  L_Operand *operand, *dest, *src0, *src1;
  L_Oper *new_op;
  operand_pair *op_pair;
  L_Cb *preheader;
  int i;

  if (!op_node)
    return 0;

  operand = op_node->operand;
  if (op_node->def)
    {
      op_pair = op_node->def;
      src0 = L_initialize_addr_register (loop, op_pair->operand1);
      src1 = L_initialize_addr_register (loop, op_pair->operand2);
      dest = L_new_register_operand (++L_fn->max_reg_id,
                                     L_return_old_ctype (operand),
                                     operand->ptype);
      new_op = L_create_new_op (op_pair->opc);
      new_op->src[0] = L_copy_operand (src0);
      new_op->src[1] = L_copy_operand (src1);
      new_op->dest[0] = L_copy_operand (dest);
      for (i = 0; i < L_MAX_CMPLTR; i++)
	new_op->com[i] = op_pair->com[i];

      preheader = loop->preheader;
      L_insert_oper_after (preheader, preheader->last_op, new_op);
      L_delete_operand (src0);
      L_delete_operand (src1);
    }
  else
    {
      dest = L_copy_operand (operand);
    }

  return dest;
}

/*
 * Assumes op is a load or store
 */

static int
L_gen_oper_acc_name (L_Oper *op, char *buf)
{
  char opd1_acc_name[BUFSIZE], opd2_acc_name[BUFSIZE];

  buf[0] = '\0';

  if (!L_load_opcode (op) && !L_store_opcode (op))
    return 0;

  if (!L_find_oper_acc_name (op->src[0], opd1_acc_name) ||
      !L_find_oper_acc_name (op->src[1], opd2_acc_name))
    return 0;

  /* Accommodate [base + offset] loads */

  if (opd1_acc_name[0] == '\0' || opd2_acc_name[0] == '\0')
    {
      return 0;
    }
  else if (!strcmp (opd2_acc_name, "i0"))
    {
      strcpy (buf, opd1_acc_name);
    }
  else if (!strcmp (opd1_acc_name, "i0"))
    {
      strcpy (buf, opd2_acc_name);
    }
  else
    {
      int cnt;
      cnt = snprintf (buf, BUFSIZE, "(%s %s %s)",
		      opd1_acc_name, L_opcode_name (Lop_ADD),
		      opd2_acc_name);

      if (cnt < 0)
	L_punt ("L_gen_oper_acc_name: buffer size exceeded");
    }

  return (buf[0] != '\0');
}

#ifdef DEBUG_LOOP_GLOBAL_VAR_MIG
static void
L_print_iaa (FILE *fp, char *name, L_Loop *loop, invariant_address_access *iaa)
{
  L_Oper *op;

  fprintf (fp, ">LGVMS> iaa \"%s\" lphd %d ops:", name, loop->header->id);
  List_start (iaa->load_mem_op);
  while ((op = (L_Oper *)List_next(iaa->load_mem_op)))
    fprintf (fp, " L%d", op->id);
  List_start (iaa->store_mem_op);
  while ((op = (L_Oper *)List_next(iaa->store_mem_op)))
    fprintf (fp, " S%d", op->id);
  List_start (iaa->alias_load_mem_op);
  while ((op = (L_Oper *)List_next(iaa->alias_load_mem_op)))
    fprintf (fp, " AL%d", op->id);
  List_start (iaa->alias_store_mem_op);
  while ((op = (L_Oper *)List_next(iaa->alias_store_mem_op)))
    fprintf (fp, " AS%d", op->id);
  List_start (iaa->alias_load_jsr);
  while ((op = (L_Oper *)List_next(iaa->alias_load_jsr)))
    fprintf (fp, " ALJ%d", op->id);
  List_start (iaa->alias_store_jsr);
  while ((op = (L_Oper *)List_next(iaa->alias_store_jsr)))
    fprintf (fp, " ASJ%d", op->id);
  fprintf (fp, "\n");

  return;
}

static void
L_print_op_list (List l)
{
  L_Oper *op;

  if (!l)
    return;

  fprintf (stderr, "OPS( ");

  List_start (l);
  while ((op = (L_Oper *)List_next (l)))
    {
      fprintf (stderr, "%d ", op->id);
    }
  fprintf (stderr, ")\n");
  return;
}
#endif

/*****************************************************************************\ 

    BCC - promote a memory location or not in the presence of jsr's - 2/14/99 
  +-------+-------+-------------+-------------+-----------+---------+--------+
  | w/ ld | w/ st | ld/jsr sync | st/jsr sync | jsr's act | promote | add st |
  +-------+-------+-------------+-------------+-----------+---------+--------+
  |   1   |   0   |      0      |      0      |    r/     |    1    |    0   |
  +-------+-------+-------------+-------------+-----------+---------+--------+
  |   1   |   0   |      1      |      0      |   r?/w    |    0    |   N/A  |
  +-------+-------+-------------+-------------+-----------+---------+--------+
  |   1   |   1   |      0      |      0      |     /     |    1    |    0   |
  +-------+-------+-------------+-------------+-----------+---------+--------+
  |   1   |   1   |      0      |      1      |    r/     |    1    |    1   |
  +-------+-------+-------------+-------------+-----------+---------+--------+
  |   1   |   1   |      1      |      1      |   r?/w    |    0    |   N/A  |
  +-------+-------+-------------+-------------+-----------+---------+--------+

    BCC - promote a memory location in the presence of aliasd ld's  - 2/14/99 
  +-------+-------+------------+------------+---------+
  | w/ ld | w/ st | ld/ld sync | st/ld sync | promote |
  +-------+-------+------------+------------+---------+
  |   1   |   0   |      0     |      0     |    1    |
  +-------+-------+------------+------------+---------+
  |   1   |   1   |      0     |      0     |    1    |
  +-------+-------+------------+------------+---------+
  |   1   |   1   |      0     |      1     |    0    |
  +-------+-------+------------+------------+---------+

    BCC - promote a memory location in the presence of aliasd st's  - 2/14/99 
  +-------+-------+------------+------------+---------+
  | w/ ld | w/ st | ld/st sync | st/st sync | promote |
  +-------+-------+------------+------------+---------+
  |   1   |   0   |      0     |      0     |    1    |
  +-------+-------+------------+------------+---------+
  |   1   |   0   |      1     |      0     |    0    |
  +-------+-------+------------+------------+---------+
  |   1   |   1   |      0     |      0     |    1    |
  +-------+-------+------------+------------+---------+
  |   1   |   1   |      1     |      1     |    0    |
  +-------+-------+------------+------------+---------+

\*****************************************************************************/

/* Assumes loop has preheader */

/*
 * Loop global variable migration by sync arcs (LGVMS)
 * ----------------------------------------------------------------------
 * Assumes loop has preheader.
 */

int
L_loop_global_var_migration_by_sync (L_Loop *loop)
{
  int change = 0, i, num_cb, *loop_cb = NULL, num_out_cb, *out_cb = NULL,
    num_exit_cb, *exit_cb = NULL, new_load_opc, new_store_opc,
    suppress_flag = 0;
  int num_be_cb, *be_cb = NULL;
  L_Cb *cb;
  L_Oper *op, *load_op, *store_op, *alias_op, *new_load, *pred_set = NULL;
  L_Operand *new_reg, *new_addr_reg[2];
  char acc_name1[BUFSIZE], acc_name2[BUFSIZE];
  STRING_Symbol_Table *invariant_address_access_tbl;
  STRING_Symbol *symbol;
  invariant_address_access *iaa;
  int dep_flags;
  double ph_weight;

#ifdef DEBUG_LOOP_GLOBAL_VAR_MIG
  L_Attr *acc_name_attr;
#endif

  ph_weight = loop->preheader->weight;

  /*
   * Use cost function to determine if should move ops to preheader.
   * This is necessary because some machines have few registers so
   * if make too many vars live in loop will get negative perf.
   */
  if (!L_cost_effective_to_move_ops_to_preheader (loop))
    return 0;

#if defined(LOOP_USE_CUTSET_METRIC)
  {
    int cnt_livein, num_regs;
    cnt_livein = Set_size (L_get_cb_IN_set(loop->header));
    num_regs = M_num_registers (L_native_machine_ctype);
    if (cnt_livein > LOOP_CUTSET_RATIO * num_regs)
      {
#if defined(DEBUG_LOOP_METRICS) || defined (DEBUG_LOOP_IND_STR_RED)
	fprintf (ERR, ">LGVMS> SUPPRESSED CUTSET (cutset %d/%d), no iaa info."
		 "\n", cnt_livein, num_regs);
#endif
	suppress_flag = 1;
      }
  }
#endif

  dep_flags = SET_INNER_CARRIED (0) | SET_NONLOOP_CARRIED (0);
  invariant_address_access_tbl =
    STRING_new_symbol_table ("invariant_address_access_tbl", 16);

  /*  
   * REH 12-4-95, if backedge is a boundary cb, don't perform
   *  global var migration on this loop
   */

  if ((num_be_cb = Set_size (loop->back_edge_cb)))
    {
      be_cb = (int *) alloca (sizeof (int) * num_be_cb);
      Set_2array (loop->back_edge_cb, be_cb);
      for (i = 0; i < num_be_cb; i++)
        {
          cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, be_cb[i]);
          if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_BOUNDARY))
	    return (0);
        }
    }

  if ((num_cb = Set_size (loop->loop_cb)))
    {
      loop_cb = (int *) alloca (sizeof (int) * num_cb);
      Set_2array (loop->loop_cb, loop_cb);
    }

  if ((num_exit_cb = Set_size (loop->exit_cb)))
    {
      exit_cb = (int *) alloca (sizeof (int) * num_exit_cb);
      Set_2array (loop->exit_cb, exit_cb);
    }

  if ((num_out_cb = Set_size (loop->out_cb)))
    {
      out_cb = (int *) Lcode_malloc (sizeof (int) * num_out_cb);
      Set_2array (loop->out_cb, out_cb);
    }

  /* 1. Build symbolic expressions for each loop-invariant
   *    access location 
   */

  L_build_ast_for_reg_in_loop (loop, loop_cb, num_cb);

  /* group mem ops based on acc names */
  for (i = 0; i < num_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);

      for (op = cb->first_op; op; op = op->next_op)
        {
          if (!((L_load_opcode (op) && !suppress_flag) || L_store_opcode (op)))
            continue;
	  /* 02/07/03 REK Adding a check to make sure we don't touch a
	   *              volatile oper. */
	  if (L_EXTRACT_BIT_VAL (op->flags, L_OPER_VOLATILE))
	    continue;

	  if (!L_gen_oper_acc_name (op, acc_name1))
	    continue;

          if (!(symbol = STRING_find_symbol (invariant_address_access_tbl,
					     acc_name1)))
            {
              iaa = L_new_invariant_address_access ();
              symbol = STRING_add_symbol (invariant_address_access_tbl,
                                          acc_name1, 0);
              symbol->data = iaa;
            }
	  else
	    {
	      iaa = symbol->data;
	    }

          iaa->safe |= L_safe_to_move_out_of_loop (loop, exit_cb, 
						   num_exit_cb, cb, op);

          if (L_load_opcode (op))
	    iaa->load_mem_op = List_insert_last (iaa->load_mem_op, op);
          else
	    iaa->store_mem_op = List_insert_last (iaa->store_mem_op, op);

	  iaa->benefit += op->weight;

	  iaa->is_block_move |= L_find_attr (op->attr, "block_move") ? 1 : 0;
        }
    }

  /* 2. Detect aliasing accesses for each loop-invariant
   *    access location 
   */

  for (symbol = invariant_address_access_tbl->head_symbol;
       symbol; symbol = symbol->next_symbol)
    {
      iaa = symbol->data;

      /* no load, no promotion */
      if (!iaa->load_mem_op)
        continue;

      iaa->benefit -= ph_weight;

      /* no promotion for block moves */
      if (iaa->is_block_move)
        continue;

      load_op = List_first (iaa->load_mem_op);
      L_gen_oper_acc_name (load_op, acc_name1);

      store_op = List_first (iaa->store_mem_op);

      if (store_op)
	iaa->benefit -= ph_weight;

      for (i = 0; i < num_cb; i++)
        {
          cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);

          for (op = cb->first_op; op; op = op->next_op)
            {
              if (L_subroutine_call_opcode (op))
                {
		  if (store_op)
		    {
		      if (!L_depinfo_indep_mem_ops (store_op, op, dep_flags))
			{
			  iaa->alias_load_jsr =
			    List_insert_last (iaa->alias_load_jsr, op);
			  iaa->benefit -= op->weight;


			  if (!L_depinfo_indep_mem_ops (load_op, op, dep_flags))
			    {
			      iaa->alias_store_jsr =
				List_insert_last (iaa->alias_store_jsr, op);
			      iaa->benefit -= op->weight;
			    }
			}
		    }
		  else
		    {
		      if (!L_depinfo_indep_mem_ops (load_op, op, dep_flags))
			{
			  iaa->alias_store_jsr =
			    List_insert_last (iaa->alias_store_jsr, op);
			  iaa->benefit -= op->weight;
			}
		    }
                }
              else if ((L_load_opcode (op) || L_store_opcode (op)) &&
		       !L_EXTRACT_BIT_VAL (op->flags, L_OPER_VOLATILE))
                {
                  /* generate the access name */

		  L_gen_oper_acc_name (op, acc_name2);

                  /* memory access with the same loop invariant address */
                  if (!strcmp (acc_name1, acc_name2))
                    continue;

                  if (L_load_opcode (op))
		    {
		      if (store_op &&
			  !L_depinfo_indep_mem_ops (store_op, op, dep_flags))
			{
			  iaa->alias_load_mem_op =
			    List_insert_last (iaa->alias_load_mem_op, op);
			  iaa->benefit -= op->weight;
			}
		    }
                  else if (L_store_opcode (op))
		    {
                      if (!L_depinfo_indep_mem_ops (load_op, op, dep_flags))
			{
			  iaa->alias_store_mem_op =
			    List_insert_last (iaa->alias_store_mem_op, op);
			  iaa->benefit -= op->weight;
			}
		    }
		}
            }
        }
    }

  /* 3. Heuristically select locations to promote to a register for the
   *    duration of the loop
   */

  for (symbol = invariant_address_access_tbl->head_symbol;
       symbol; symbol = symbol->next_symbol)
    {
      iaa = symbol->data;

      if (!iaa->load_mem_op)
	{
	  /* No load to promote */
#if defined (DEBUG_LOOP_GLOBAL_VAR_MIG)
	  fprintf (ERR, ">LGVMS> SUPPRESSED NOLOAD "
		   "(benefit %0.0f)\n", iaa->benefit);
	  L_print_iaa (ERR, symbol->name, loop, iaa);
#endif
	  continue;
	}
      if (iaa->is_block_move)
	{
	  /* Block move promotion is prohibited */
#if defined (DEBUG_LOOP_GLOBAL_VAR_MIG)
	  fprintf (ERR, ">LGVMS> SUPPRESSED BLOCK MOV "
		   "(benefit %0.0f)\n", iaa->benefit);
	  L_print_iaa (ERR, symbol->name, loop, iaa);
#endif
	  continue;
	}
      if (iaa->alias_store_mem_op)
	{
	  /* IAA potentially modified by aliased operations */
#if defined (DEBUG_LOOP_GLOBAL_VAR_MIG)
	  fprintf (ERR, ">LGVMS> SUPPRESSED W-ALIAS "
		   "(benefit %0.0f)\n", iaa->benefit);
	  L_print_op_list (iaa->alias_store_mem_op);
	  L_print_iaa (ERR, symbol->name, loop, iaa);
#endif
	  continue;
	}
      if (iaa->alias_store_jsr
#if 0
	  && (!iaa->store_mem_op || !iaa->load_mem_op)
#endif
	  )
	{
	  /* IAA potentially modified by aliased operations */
#if defined (DEBUG_LOOP_GLOBAL_VAR_MIG)
	  fprintf (ERR, ">LGVMS> SUPPRESSED JSR W-ALIAS "
		   "(benefit %0.0f)\n", iaa->benefit);
	  L_print_iaa (ERR, symbol->name, loop, iaa);
#endif
	  continue;
	}
      if (iaa->store_mem_op && iaa->alias_load_mem_op)
	{
	  /* IAA potentially read by aliased operations */
#if defined (DEBUG_LOOP_GLOBAL_VAR_MIG)
	  fprintf (ERR, ">LGVMS> SUPPRESSED R-ALIAS "
		   "(benefit %0.0f)\n", iaa->benefit);
	  L_print_iaa (ERR, symbol->name, loop, iaa);
#endif
	  continue;
	}
#if defined(LOOP_USE_WEIGHT_METRIC)
      if (iaa->benefit <= 0.0)
	{
	  /* Judged unbeneficial */
#if defined(DEBUG_LOOP_METRICS) || defined (DEBUG_LOOP_GLOBAL_VAR_MIG)
	  fprintf (ERR, ">LGVMS> SUPPRESSED BENEFIT "
		   "(benefit %0.0f)\n", iaa->benefit);
#if defined (DEBUG_LOOP_GLOBAL_VAR_MIG)
	  L_print_iaa (ERR, symbol->name, loop, iaa);
#endif
#endif
	  continue;
	}
#endif

      if ((load_op = List_first (iaa->load_mem_op)) &&
	  !L_non_excepting_ops && !L_is_trivially_safe(load_op))
	{
#if defined (DEBUG_LOOP_GLOBAL_VAR_MIG)
	  fprintf (ERR, ">LGVMS> SUPPRESSED NOSPEC (benefit %0.0f)\n",
		   iaa->benefit);
	  L_print_iaa (ERR, symbol->name, loop, iaa);
#endif
	  continue;
	}

      /*
       * load_op is the first aliasing load in the list.  This could be
       * the load from a block move which doesn't contain any sync arcs.
       * To make sure that the proper sync arcs are copied to the new
       * loads, the list is traversed until the load contains sync arcs.
       * Also need to check that we don't infinite loop due to an op
       * being repeated.
       * This is a hack until sync arcs are generated for the loads 
       * in the block move.  CJS 05/01/01
       */

      List_start (iaa->load_mem_op);
      load_op = (L_Oper *) List_next (iaa->load_mem_op);
      while (!load_op->sync_info)
	{
	  L_Oper *next_op = (L_Oper *)List_next (iaa->load_mem_op);
	  
	  /*
	   * The case does exist where an op is followed by itself,
	   * neither having sync_info.  I avoid this by checking the id.
	   */

	  if(!next_op || (next_op == load_op))
	    break;

	  load_op = next_op;
	}

      /* now it is safe to promote this location to register */

      new_load_opc = L_corresponding_load (load_op);
      new_store_opc = L_corresponding_store (load_op);
      new_reg = L_new_register_operand (++L_fn->max_reg_id,
					L_opcode_ctype2 (load_op),
					L_PTYPE_NULL);
      for (i = 0; i < 2; i++)
        {
	  char acc_name[BUFSIZE];
	  STRING_Symbol *opd_symbol;
          L_generate_operand_name (load_op->src[i], acc_name);
          opd_symbol = STRING_find_symbol (operand_node_tbl, acc_name);
          new_addr_reg[i] = L_initialize_addr_register (loop, 
							opd_symbol->data);
        }

#ifdef DEBUG_LOOP_GLOBAL_VAR_MIG
      acc_name_attr = L_find_attr (load_op->attr, "ACC_NAME");
      fprintf (ERR, ">LGVMS> APPLIED promoted access to %s "
	       "(%s, benefit %0.0f) to reg %d in loop %d\n", symbol->name,
               acc_name_attr ? acc_name_attr->field[0]->value.l : "N/A",
               iaa->benefit, new_reg->value.r, loop->id);

      L_print_iaa (ERR, symbol->name, loop, iaa);
#endif

      cb = L_oper_hash_tbl_find_cb (L_fn->oper_hash_tbl, load_op->id);

      /* insert a load in the prologue */

      new_load =
        L_insert_load_ops_for_global_var_mig (loop, load_op, new_load_opc,
                                              new_reg, new_addr_reg[0],
                                              new_addr_reg[1], load_op->attr,
                                              load_op->flags, iaa->safe, cb);

      /* insert store ops in out cbs, store ops before JSRs with aliasing
       * reads.
       */

      if ((store_op = List_first (iaa->store_mem_op)))
        {
	  if (pred_set)
	    {
	      L_delete_oper (NULL, pred_set);
	      pred_set = NULL;
	    }
          L_insert_store_ops_for_global_var_mig (loop, out_cb, num_out_cb,
                                                 store_op, new_store_opc,
                                                 new_addr_reg[0],
                                                 new_addr_reg[1], new_reg,
                                                 store_op->attr,
                                                 store_op->flags, iaa->safe,
						 &pred_set);
        }

      List_start (iaa->alias_store_mem_op);
      while ((alias_op = (L_Oper *)List_next (iaa->alias_store_mem_op)))
	{
	  cb = L_oper_hash_tbl_find_cb (L_fn->oper_hash_tbl, alias_op->id);
	  L_insert_alias_compensation_load (cb, load_op,
					    alias_op, new_load_opc, new_reg,
					    new_addr_reg[0], new_addr_reg[1], 
					    load_op->attr, load_op->flags, 
					    iaa->safe);
	}

      List_start (iaa->alias_store_jsr);
      while ((alias_op = (L_Oper *)List_next (iaa->alias_store_jsr)))
	{
	  cb = L_oper_hash_tbl_find_cb (L_fn->oper_hash_tbl, alias_op->id);
	  L_insert_alias_compensation_store (cb, store_op,
					     alias_op, new_store_opc, 
					     new_addr_reg[0], new_addr_reg[1],
					     new_reg, store_op->attr,
					     store_op->flags, 1, pred_set);
	  L_insert_alias_compensation_load (cb, load_op,
					    alias_op, new_load_opc, new_reg,
					    new_addr_reg[0], new_addr_reg[1], 
					    load_op->attr, load_op->flags, 
					    iaa->safe);
	}

      List_start (iaa->alias_load_jsr);
      while ((alias_op = (L_Oper *)List_next (iaa->alias_load_jsr)))
	{
	  cb = L_oper_hash_tbl_find_cb (L_fn->oper_hash_tbl, alias_op->id);
	  L_insert_alias_compensation_store (cb, store_op,
					     alias_op, new_store_opc, 
					     new_addr_reg[0], new_addr_reg[1],
					     new_reg, store_op->attr,
					     store_op->flags, 1, pred_set);
	}

      List_start (iaa->load_mem_op);
      while ((load_op = (L_Oper *)List_next (iaa->load_mem_op)))
        {
	  if (load_op->opc != new_load_opc)
	    L_warn("L_loop_loop_invariant_location_migration_by_sync_arcs: "
		   "loads of different sizes detected.  This is very bad. "
		   "See RDB.");

          if (!L_is_trivially_safe(new_load) && L_mask_potential_exceptions &&
              !(L_EXTRACT_BIT_VAL (load_op->flags, L_OPER_MASK_PE)))
            {
              cb = L_oper_hash_tbl_find_cb (L_fn->oper_hash_tbl, load_op->id);
              L_insert_check_before (cb, new_load, load_op);
            }
          L_convert_to_extended_move (load_op,
                                      L_copy_operand (load_op->dest[0]),
                                      L_copy_operand (new_reg));
          STAT_COUNT("L_loop_inv_mig_sync_arcs1", 1, cb);
          change++;
        }

      List_start (iaa->store_mem_op);
      while ((store_op = (L_Oper *)List_next (iaa->store_mem_op)))
        {
	  L_Cb *store_cb = NULL;
	  L_Oper *pred_set_op;
	  store_cb = L_oper_hash_tbl_find_cb(L_fn->oper_hash_tbl, 
					     store_op->id);
	  
	  if (store_op->opc != new_store_opc)
	    L_warn("L_loop_loop_invariant_location_migration_by_sync_arcs: "
		   "stores of different sizes detected.  This is very bad. "
		   "See RDB.");

	  if (pred_set)
	    {
	      if (Lopti_store_migration_mode == L_STORE_MIGRATION_FULL_PRED)
		store_cb->flags = L_SET_BIT_FLAG (store_cb->flags, 
						  L_CB_HYPERBLOCK);

	      pred_set_op = L_copy_operation(pred_set);
	      pred_set_op->pred[0] = L_copy_operand(store_op->pred[0]);
	      L_insert_oper_after(store_cb,store_op, pred_set_op);
	    }
          L_convert_to_move (store_op, L_copy_operand (new_reg),
                             L_copy_operand (store_op->src[2]));
          STAT_COUNT("L_loop_inv_mig_sync_arcs2", 1, store_cb);
	  change++;
        }

      /* REH 12/95 - If num_out_cb is 0, then array was not malloc'd */
      if (num_out_cb > 0)
        Lcode_free (out_cb);

      if ((num_out_cb = Set_size (loop->out_cb)))
        {
          out_cb = (int *) Lcode_malloc (sizeof (int) * num_out_cb);
          Set_2array (loop->out_cb, out_cb);
        }

      /* dispose of temporary operands */
      L_delete_operand (new_reg);
      L_delete_operand (new_addr_reg[0]);
      L_delete_operand (new_addr_reg[1]);
      STAT_COUNT("L_loop_inv_mig_sync_arcs3", 1, NULL);
      change++;
    }

  STRING_delete_symbol_table (invariant_address_access_tbl,
                              L_free_invariant_address_access);

  if (out_cb)
    Lcode_free (out_cb);
  if (pred_set)
    L_delete_oper (NULL, pred_set);

  L_delete_ast_for_reg_in_loop ();  
  return change;
}


static L_Oper *
L_insert_load_ops_for_global_var_mig (L_Loop * loop, L_Oper * old_oper,
				      int ld_opc, L_Operand * ld_dest,
				      L_Operand * ld_s1, L_Operand * ld_s2,
				      L_Attr * attr, int flags, int safe,
				      L_Cb * cb)
{
  L_Oper *new_op;
  L_Cb *preheader;
  L_Sync_Info *new_sync_info;

  new_op = L_create_new_op (ld_opc);
  new_op->dest[0] = L_copy_operand (ld_dest);
  new_op->src[0] = L_copy_operand (ld_s1);
  new_op->src[1] = L_copy_operand (ld_s2);
  /* attr fields */
  new_op->attr = L_copy_attr (attr);
  /* flag field */
  new_op->flags = flags;
  if (!L_is_trivially_safe(new_op) && 
      L_mask_potential_exceptions)
    L_mark_oper_speculative (new_op);

  preheader = loop->preheader;

  /* DMG -sync arcs */
  if (old_oper->sync_info != NULL)
    {
      new_sync_info = L_copy_sync_info (old_oper->sync_info);
      new_op->sync_info = new_sync_info;
      L_insert_all_syncs_in_dep_opers (new_op);
      L_adjust_syncs_for_movement_out_of_loop (new_op, preheader);
    }

  if (old_oper->acc_info)
    new_op->acc_info = L_copy_mem_acc_spec_list_as_use (old_oper->acc_info);

  L_insert_oper_after (preheader, preheader->last_op, new_op);

  return (new_op);
}


static void
L_insert_store_ops_for_global_var_mig (L_Loop * loop, int *out_cb,
				       int num_out_cb, L_Oper * old_oper,
				       int st_opc, L_Operand * st_s1,
				       L_Operand * st_s2, L_Operand * st_s3,
				       L_Attr * attr, int flags, int safe,
				       L_Oper ** cond_set)
{
  int i;
  L_Oper *new_op, *new_jump_op, *op, *new_move_op;
  L_Cb *out, *src_cb, *new_cb, *old_cb, *cond_cb = NULL;
  L_Flow *flow, *f, *new_flow, *next_flow;
  L_Sync_Info *new_sync_info;
  L_Operand *old_st_s3, *new_move_src, *new_move_dst;

  old_cb = L_oper_hash_tbl_find_cb (L_fn->oper_hash_tbl, old_oper->id);

  /*
   * Creates a predicate for selecting to perform the store
   */
  *cond_set = NULL;
  switch (Lopti_store_migration_mode)
    {
    case L_STORE_MIGRATION_FULL_PRED:
      *cond_set = L_setup_conditional_op_with_pred (loop->preheader);
      L_fn->flags = L_SET_BIT_FLAG (L_fn->flags, L_FUNC_HYPERBLOCK);
      break;
    case L_STORE_MIGRATION_NO_PRED:
      *cond_set = L_setup_conditional_op_with_cbs (loop->preheader);
      break;
    case L_STORE_MIGRATION_NO_COND:
      break;
    default:
      L_punt ("L_insert_store_ops_for_global_var_mig: unknown mode\n");
    }

  for (i = 0; i < num_out_cb; i++)
    {
      out = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, out_cb[i]);

      /* create new store op */
      new_op = L_create_new_op (st_opc);
      new_op->src[0] = L_copy_operand (st_s1);
      new_op->src[1] = L_copy_operand (st_s2);
      new_op->src[2] = L_copy_operand (st_s3);
      /* attr fields */
      new_op->attr = L_copy_attr (attr);
      /* flag field */
      new_op->flags = flags;

      /* DMG -sync arcs */
      if (old_oper->sync_info != NULL)
	{
	  new_sync_info = L_copy_sync_info (old_oper->sync_info);
	  new_op->sync_info = new_sync_info;
	  L_insert_all_syncs_in_dep_opers (new_op);
	}

      if (old_oper->acc_info)
	new_op->acc_info = L_copy_mem_acc_spec_list_as_def (old_oper->acc_info);

      if (L_all_predecessor_cb_in_loop (loop, out))
	{
	  switch (Lopti_store_migration_mode)
	    {
	    case L_STORE_MIGRATION_FULL_PRED:
	      cond_cb =
		L_create_conditional_op_with_pred_in_cb (*cond_set,
							 new_op,
							 out, out->first_op);
	      break;
	    case L_STORE_MIGRATION_NO_PRED:
	      cond_cb =
		L_create_conditional_op_with_cbs_at_cb (L_fn, *cond_set,
							new_op,
							out, out->first_op);
	      break;
	    case L_STORE_MIGRATION_NO_COND:
	      L_insert_oper_before (out, out->first_op, new_op);
	      cond_cb = out;
	      break;
	    default:
	      L_punt
		("L_insert_store_ops_for_global_var_mig: unknown mode\n");
	    }

	  /* DMG -sync arcs */
	  if (old_oper->sync_info != NULL)
	    L_adjust_syncs_for_movement_out_of_loop (new_op, out);

	  /* REH 10-95                                               */
	  /* insert a move operation to place the new store value    */
	  /* into the register holding the old store value.          */
	  /* Subsequent optimization passes will determine whether   */
	  /* the move is necessary.                                  */
	  old_st_s3 = old_oper->src[2];
	  if ((L_is_register (old_st_s3) || L_is_macro (old_st_s3)))
	    {
	      int available = 1;
	      for (flow = out->src_flow; flow != NULL; flow = flow->next_flow)
		{
		  if (!L_in_cb_EIN_set (flow->src_cb, old_oper))
		    {
		      available = 0;
		      break;
		    }
		}
	      if (available)
		{
		  new_move_dst = L_copy_operand (old_st_s3);
		  new_move_src = L_copy_operand (st_s3);

		  new_move_op = L_create_move (new_move_dst, new_move_src);
		  L_insert_oper_after (cond_cb, new_op, new_move_op);
		}
	    }
	}
      else
	{
	  new_cb = L_create_cb (out->weight);
	  L_insert_cb_after (L_fn, L_fn->last_cb, new_cb);
	  for (flow = out->src_flow; flow != NULL; flow = next_flow)
	    {
	      next_flow = flow->next_flow;
	      if (!Set_in (loop->loop_cb, flow->src_cb->id))
		{
		  new_cb->weight -= flow->weight;
		  continue;
		}
	      src_cb = flow->src_cb;
	      f = L_find_flow (src_cb->dest_flow, flow->cc, src_cb, out);
	      op = L_find_branch_for_flow (src_cb, f);
	      if (op != NULL)
		{
		  L_change_branch_dest (op, out, new_cb);
		}
	      else
		{
		  /* this must be fallthru path or something wrong */
		  if (src_cb != out->prev_cb)
		    L_punt
		      ("L_insert_store_ops_for_global_var_mig: "
		       "illegal control structure");
		  /* new_cb wont be fallthru, so insert explicit jump to it */
		  if (src_cb->weight > L_min_fs_weight)
		    new_jump_op = L_create_new_op (Lop_JUMP_FS);
		  else
		    new_jump_op = L_create_new_op (Lop_JUMP);
		  new_jump_op->src[0] = L_new_cb_operand (new_cb);
		  L_insert_oper_after (src_cb, src_cb->last_op, new_jump_op);
		}
	      /* update flow arcs */
	      f->dst_cb = new_cb;
	      out->src_flow = L_delete_flow (out->src_flow, flow);
	      new_flow = L_new_flow (f->cc, src_cb, new_cb, f->weight);
	      new_cb->src_flow = L_concat_flow (new_cb->src_flow, new_flow);
	    }

	  old_cb =
	    L_oper_hash_tbl_find_cb (L_fn->oper_hash_tbl, old_oper->id);
	  /* only partly update loop structure for now */
	  if (old_cb->deepest_loop != NULL)
	    new_cb->deepest_loop = old_cb->deepest_loop->parent_loop;
	  /* DMG - sync info */
	  if (old_oper->sync_info != NULL)
	    L_adjust_syncs_for_movement_out_of_loop (new_op, new_cb);

	  /* insert jump into new_cb to out */
	  if (new_cb->weight > L_min_fs_weight)
	    new_jump_op = L_create_new_op (Lop_JUMP_FS);
	  else
	    new_jump_op = L_create_new_op (Lop_JUMP);
	  new_jump_op->src[0] = L_new_cb_operand (out);
	  L_insert_oper_after (new_cb, new_cb->last_op, new_jump_op);
	  /* add correponding flow for jump */
	  new_flow = L_new_flow (1, new_cb, out, new_cb->weight);
	  new_cb->dest_flow = L_concat_flow (new_cb->dest_flow, new_flow);
	  new_flow = L_new_flow (1, new_cb, out, new_cb->weight);
	  out->src_flow = L_concat_flow (out->src_flow, new_flow);
	  /* update loop structure to reflect new out cb */
	  loop->out_cb = Set_delete (loop->out_cb, out->id);
	  loop->out_cb = Set_add (loop->out_cb, new_cb->id);

	  out = new_cb;

	  switch (Lopti_store_migration_mode)
	    {
	    case L_STORE_MIGRATION_FULL_PRED:
	      cond_cb =
		L_create_conditional_op_with_pred_in_cb (*cond_set,
							 new_op,
							 new_cb,
							 new_cb->first_op);
	      break;
	    case L_STORE_MIGRATION_NO_PRED:
	      cond_cb =
		L_create_conditional_op_with_cbs_at_cb (L_fn, *cond_set,
							new_op,
							new_cb,
							new_cb->first_op);
	      break;
	    case L_STORE_MIGRATION_NO_COND:
	      L_insert_oper_before (new_cb, new_cb->first_op, new_op);
	      cond_cb = new_cb;
	      break;
	    default:
	      L_punt
		("L_insert_store_ops_for_global_var_mig: unknown mode\n");
	    }
	}
    }
  return;
}


static L_Oper *
L_insert_alias_compensation_store (L_Cb * old_cb,
				   L_Oper * store_oper,
				   L_Oper * alias_oper,
				   int st_opc,
				   L_Operand * st_s1,
				   L_Operand * st_s2,
				   L_Operand * st_s3,
				   L_Attr * attr, int flags,
				   int safe, L_Oper * cond_set)
{
  L_Oper *new_op = NULL;
  L_Sync_Info *new_sync_info = NULL;
  L_Cb *cond_cb = NULL;

  /* create new store op */
  new_op = L_create_new_op (st_opc);
  new_op->src[0] = L_copy_operand (st_s1);
  new_op->src[1] = L_copy_operand (st_s2);
  new_op->src[2] = L_copy_operand (st_s3);
  /* attr fields */
  new_op->attr = L_copy_attr (attr);
  /* flag field */
  new_op->flags = flags;

#if 0
  printf ("JSR: %d\n", alias_oper->id);
#endif
  switch (Lopti_store_migration_mode)
    {
    case L_STORE_MIGRATION_FULL_PRED:
      cond_cb =
	L_create_conditional_op_with_pred_in_cb (cond_set,
						 new_op, old_cb, alias_oper);
      break;
    case L_STORE_MIGRATION_NO_PRED:
      cond_cb =
	L_create_conditional_op_with_cbs_at_cb (L_fn, cond_set,
						new_op, old_cb, alias_oper);
      break;
    case L_STORE_MIGRATION_NO_COND:
      L_insert_oper_before (old_cb, alias_oper, new_op);
      cond_cb = old_cb;
      break;
    default:
      L_punt ("L_insert_alias_compensation_store: unknown mode\n");
    }

  if (store_oper->sync_info)
    {
      new_sync_info = L_copy_sync_info (store_oper->sync_info);
      new_op->sync_info = new_sync_info;
      L_insert_all_syncs_in_dep_opers (new_op);

      L_add_sync_between_opers (new_op, alias_oper);
    }

  if (store_oper->acc_info)
    new_op->acc_info = L_copy_mem_acc_spec_list_as_def (store_oper->acc_info);


  return new_op;
}


static void
L_insert_alias_compensation_load (L_Cb * cb, L_Oper * old_oper,
				  L_Oper * alias_oper, int ld_opc,
				  L_Operand * ld_dest,
				  L_Operand * ld_s1,
				  L_Operand * ld_s2,
				  L_Attr * attr, int flags,
				  int safe)
{
  L_Oper *new_op;
  L_Sync_Info *new_sync_info;

  new_op = L_create_new_op (ld_opc);
  new_op->dest[0] = L_copy_operand (ld_dest);
  new_op->src[0] = L_copy_operand (ld_s1);
  new_op->src[1] = L_copy_operand (ld_s2);
  /* attr fields */
  new_op->attr = L_copy_attr (attr);
  /* flag field */
  new_op->flags = flags;
  if (!safe && L_mask_potential_exceptions)
    L_mark_oper_speculative (new_op);

  /* DMG -sync arcs */
  if (old_oper->sync_info != NULL)
    {
      new_sync_info = L_copy_sync_info (old_oper->sync_info);
      new_op->sync_info = new_sync_info;
      L_insert_all_syncs_in_dep_opers (new_op);
    }

  if (old_oper->acc_info)
    new_op->acc_info = L_copy_mem_acc_spec_list_as_use (old_oper->acc_info);

  L_insert_oper_after (cb, alias_oper, new_op);
  return;
}


/*
 * Simplify back branch
 * ----------------------------------------------------------------------
 * Convert blt, ble, bgt, bge loop back branches to beq or bne.
 */
int
L_loop_simplify_back_branch (L_Loop * loop)
{
  int change, i, num_cb, *loop_cb = NULL, num_exit_cb, *exit_cb = NULL;
  L_Cb *cb;
  L_Oper *last;
  change = 0;

  /* if arch can handle all types of branches with 1 instruction, this
     optimization is not necessary */
  if (M_oper_supported_in_arch (Lop_BLT) &&
      M_oper_supported_in_arch (Lop_BLE) &&
      M_oper_supported_in_arch (Lop_BGT) &&
      M_oper_supported_in_arch (Lop_BGE))
    return 0;

  if ((num_cb = Set_size (loop->loop_cb)))
    {
      loop_cb = (int *) alloca (sizeof (int) * num_cb);
      Set_2array (loop->loop_cb, loop_cb);
    }

  if ((num_exit_cb = Set_size (loop->exit_cb)))
    {
      exit_cb = (int *) alloca (sizeof (int) * num_exit_cb);
      Set_2array (loop->exit_cb, exit_cb);
    }

  L_find_all_ind_info (loop, loop_cb, num_cb);

  for (i = 0; i < num_exit_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, exit_cb[i]);
      last = cb->last_op;
      if (L_uncond_branch_opcode (last) &&
          L_cond_branch_opcode (last->prev_op))
        last = last->prev_op;
      /*
       *      match pattern no. 1
       */
      if (!(L_int_bgt_branch_opcode (last) ||
            L_int_bge_branch_opcode (last) ||
            L_int_blt_branch_opcode (last) || 
	    L_int_ble_branch_opcode (last)))
        continue;
      if (!(L_basic_induction_var (loop, last->src[0])))
        continue;
      if (!(L_is_numeric_constant (last->src[1])))
        continue;
      if (!(L_num_constant_increment_of_ind (last->src[0], loop->ind_info)))
        continue;
      if (!(L_num_constant_init_val_of_ind (last->src[0], loop->ind_info)))
        continue;
      if (!(L_can_simplify_loop_branch (loop, last, loop->ind_info)))
        continue;
      if (!(L_ind_var_will_reach_limit (loop, last, loop->ind_info)))
        continue;
      /*
       *      replace pattern 1
       */
#ifdef DEBUG_LOOP_SIMPLIFY_LOOP_BR
      fprintf (ERR, "Apply loop br simplification1 to %d (cb %d)\n",
               last->id, cb->id);
#endif
      L_simplify_loop_branch1 (loop, last);
      STAT_COUNT ("L_loop_simplify_back_branch1", 1, cb);
      change++;
    }
  for (i = 0; i < num_exit_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, exit_cb[i]);
      last = cb->last_op;
      if (L_uncond_branch_opcode (last) &&
          L_cond_branch_opcode (last->prev_op))
        last = last->prev_op;
      /*
       *      match pattern 2
       */
      if (!(L_int_bgt_branch_opcode (last) ||
            L_int_bge_branch_opcode (last) ||
            L_int_blt_branch_opcode (last) || L_int_ble_branch_opcode (last)))
        continue;
      if (!(L_basic_induction_var (loop, last->src[0])))
        continue;
      if (!(L_int_one_increment_of_ind (last->src[0], loop->ind_info) ||
            L_int_neg_one_increment_of_ind (last->src[0], loop->ind_info)))
        continue;
      if (!(L_is_loop_inv_operand (loop, loop_cb, num_cb, last->src[1])))
        continue;
      if (!(L_can_simplify_loop_branch (loop, last, loop->ind_info)))
        continue;
      if (!(L_ind_var_will_reach_limit (loop, last, loop->ind_info)))
        continue;
      /*
       *      replace pattern 2
       */
#ifdef DEBUG_LOOP_SIMPLIFY_LOOP_BR
      fprintf (ERR, "Apply loop br simplification2 to %d (cb %d)\n",
               last->id, cb->id);
#endif
      L_simplify_loop_branch2 (loop, last);
      STAT_COUNT ("L_loop_simplify_back_branch2", 1, cb);
      change++;
    }

  return change;
}

/*
 * Loop induction strength reduction (LISR)
 * ----------------------------------------------------------------------
 */
int
L_loop_induction_strength_reduction (L_Loop * loop)
{
  int change, i, num_cb, *loop_cb = NULL, num_out_cb, *out_cb = NULL,
    num_exit_cb, *exit_cb = NULL;
  L_Cb *cb, *preheader;
  L_Oper *op, *next, *new_op;
  L_Operand *new_reg;
  change = 0;

  if ((num_cb = Set_size (loop->loop_cb)))
    {
      loop_cb = (int *) alloca (sizeof (int) * num_cb);
      Set_2array (loop->loop_cb, loop_cb);
    }

  if ((num_out_cb = Set_size (loop->out_cb)))
    {
      out_cb = (int *) alloca (sizeof (int) * num_out_cb);
      Set_2array (loop->out_cb, out_cb);
    }

  if ((num_exit_cb = Set_size (loop->exit_cb)))
    {
      exit_cb = (int *) alloca (sizeof (int) * num_exit_cb);
      Set_2array (loop->exit_cb, exit_cb);
    }

  L_find_all_ind_info (loop, loop_cb, num_cb);

#if defined(LOOP_USE_CUTSET_METRIC)
  {
    int cnt_livein, num_regs;
    cnt_livein = Set_size (L_get_cb_IN_set(loop->header));
    num_regs = M_num_registers (L_native_machine_ctype);
    if (cnt_livein > LOOP_CUTSET_RATIO * num_regs)
      {
#if defined(DEBUG_LOOP_METRICS) || defined (DEBUG_LOOP_IND_STR_RED)
	fprintf (ERR, ">LISR> SUPPRESSED CUTSET (cutset %d/%d)\n",
		 cnt_livein, num_regs);
#endif
	return 0;
      }
  }
#endif

  for (i = 0; i < num_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);

      for (op = cb->first_op; op != NULL; op = next)
        {
          int macro_flag, load_flag, store_flag;
          next = op->next_op;
          /*
           *  match pattern
           */
	  /* 02/07/03 REK Adding a check to make sure we don't touch a
	   *              volatile oper. */
	  if (L_EXTRACT_BIT_VAL (op->flags, L_OPER_VOLATILE))
	    continue;
          if (!(L_is_str_reducible_opcode (op)))
            continue;
          if (!(L_basic_induction_var (loop, op->src[0])))
            continue;
          if (L_same_operand (op->dest[0], op->src[0]))
            continue;
          if (!(L_is_register (op->dest[0])))
            continue;
          if (!(L_is_loop_inv_operand (loop, loop_cb, num_cb, op->src[1])))
            continue;
          macro_flag = (L_is_fragile_macro (op->dest[0]) ||
                        L_is_fragile_macro (op->src[0]) ||
                        L_is_fragile_macro (op->src[1]));
          load_flag = 0;
          store_flag = 0;
          if (!(L_no_danger_in_loop (loop, loop_cb, num_cb, macro_flag,
                                     load_flag, store_flag)))
            continue;
          if (!(L_useful_str_red (loop, op, op->src[0])))
            continue;

	  /* SER: Don't reduce strength if there are a lot of 
	   * live registers. LISR replaces the old instruction with an 
	   * add/move and a new operand. */

          /* The following checks make sure that loop optis don't "ping-pong"
           * without doing anything useful, by seeing if further optimizations
           * are possible.
           */
          if (!L_all_uses_in_loop_from (loop, loop_cb, num_cb, cb, op))
            continue;
          if (!L_unique_def_in_loop (loop, loop_cb, num_cb, op))
            continue;
          if (!L_can_modify_dep_branches_in_loop (loop, loop_cb, num_cb,
                                                  op, cb))
            continue;
          if (!L_def_reachs_all_out_cb_of_loop (loop, exit_cb, num_exit_cb,
                                                cb, op))
            continue;

          /*
           *  replace pattern
           */
          preheader = loop->preheader;
          new_reg = L_new_register_operand (++L_fn->max_reg_id,
                                            L_opcode_ctype (op),
                                            L_PTYPE_NULL);
          /* loop->basic_ind_var =
            Set_add (loop->basic_ind_var, new_reg->value.r); */
          new_op = L_create_new_op (op->opc);
          new_op->dest[0] = new_reg;
          new_op->src[0] = L_copy_operand (op->src[0]);
          new_op->src[1] = L_copy_operand (op->src[1]);
          L_insert_oper_after (preheader, preheader->last_op, new_op);
          L_insert_strength_reduced_op_into_loop (loop, loop_cb, num_cb,
                                                  op, new_reg);
          L_convert_to_move (op, L_copy_operand (op->dest[0]),
                             L_copy_operand (new_reg));
#ifdef DEBUG_LOOP_IND_STR_RED
          fprintf (ERR, "Apply loop ind str red to %d (r%d -> r%d) (cb %d)\n",
                   op->id, op->dest[0]->value.r, new_reg->value.r, cb->id);
#endif
          change++;
        }
    }

  return change;
}

static int L_loop_induction_elimination0 (L_Loop * loop);
static int L_loop_induction_elimination1 (L_Loop * loop);
static int L_loop_induction_elimination2 (L_Loop * loop);
static int L_loop_induction_elimination3 (L_Loop * loop);
static int L_loop_induction_elimination4 (L_Loop * loop);
static int L_loop_induction_reinit (L_Loop * loop);
static int L_loop_induction_reassociation (L_Loop * loop);

/*
 * Loop induction elimination 
 * ----------------------------------------------------------------------
 */

int
L_loop_induction_elimination (L_Loop * loop)
{
  int change = 0, c1, c2, c3, c4;

  /* simple ind eliminations */
  change += L_loop_induction_elimination0 (loop);
  change += L_loop_induction_elimination1 (loop);
  change += L_loop_induction_elimination2 (loop);

  /* create post increment ld/st's */
  if (Lopti_do_post_inc_conv)
    {
      c1 = c2 = c3 = c4 = 0;
      c1 = L_loop_induction_reinit (loop);
      c2 = L_loop_induction_reassociation (loop);
      if (c2 > 0)
	c3 = L_loop_induction_reinit (loop);

      if ((c1 + c2 + c3) > 0)
	L_do_flow_analysis (L_fn,
			    DOMINATOR_CB | LIVE_VARIABLE |
			    AVAILABLE_EXPRESSION);

      c4 = L_loop_post_increment_conversion (loop);
      change += (c1 + c2 + c3 + c4);
    }

  /* complex ind eliminations, beware these cost a lot in loop preheaders */
  change += L_loop_induction_elimination3 (loop);
  change += L_loop_induction_elimination4 (loop);

  return (change);
}


/*
 * Loop induction elimination (level 0)
 * ----------------------------------------------------------------------
 * Remove dead induction variables.
 */

static int
L_loop_induction_elimination0 (L_Loop * loop)
{
  int change = 0, i, num_cb, *loop_cb = NULL, num_out_cb, *out_cb = NULL,
    num_basic_ind_var, *basic_ind_var = NULL;
  L_Operand *operand = NULL;

  if ((num_cb = Set_size (loop->loop_cb)))
    {
      loop_cb = (int *) alloca (sizeof (int) * num_cb);
      Set_2array (loop->loop_cb, loop_cb);
    }

  if ((num_out_cb = Set_size (loop->out_cb)))
    {
      out_cb = (int *) alloca (sizeof (int) * num_out_cb);
      Set_2array (loop->out_cb, out_cb);
    }

  if ((num_basic_ind_var = Set_size (loop->basic_ind_var)))
    {
      basic_ind_var = (int *) alloca (sizeof (int) * num_basic_ind_var);
      Set_2array (loop->basic_ind_var, basic_ind_var);
    }

  L_find_all_ind_info (loop, loop_cb, num_cb);

  operand = L_new_register_operand (0, L_native_machine_ctype, L_PTYPE_NULL);

  for (i = 0; i < num_basic_ind_var; i++)
    {
      operand->value.r = basic_ind_var[i];

      /*
       *      Match pattern
       */

      if (!L_basic_induction_var (loop, operand) ||
	  !L_not_live_in_out_cb (out_cb, num_out_cb, operand) ||
	  !L_no_uses_of_ind (loop, loop_cb, num_cb, operand))
        continue;

      /*
       *      Replace pattern
       */
#ifdef DEBUG_LOOP_IND_ELIM
      fprintf (ERR, "Apply loop ind elim 0 to ");
      L_print_operand (ERR, operand, 0);
      fprintf (ERR, "(loop %d)\n", loop->id);
#endif

      L_delete_all_basic_ind_var_op_from_loop (loop, loop_cb, num_cb,
                                               operand);

      /* Update ind var info */
      loop->basic_ind_var =
        Set_delete (loop->basic_ind_var, operand->value.r);
      L_invalidate_ind_var (operand, loop->ind_info);

      STAT_COUNT ("L_loop_induction_elimination0", 1, NULL);
      change++;
    }

  L_delete_operand (operand);

  return change;
}

/*
 * Loop induction elimination (level 1)
 * ----------------------------------------------------------------------
 * Combine pairs of induction variables having identical increments
 * and initial values.
 */

static int
L_loop_induction_elimination1 (L_Loop * loop)
{
  int change = 0, i, j, num_cb, *loop_cb = NULL, num_out_cb, *out_cb = NULL,
    num_basic_ind_var, *basic_ind_var = NULL;
  L_Operand *operand1 = NULL, *operand2 = NULL;

  if ((num_cb = Set_size (loop->loop_cb)))
    {
      loop_cb = (int *) alloca (sizeof (int) * num_cb);
      Set_2array (loop->loop_cb, loop_cb);
    }

  if ((num_out_cb = Set_size (loop->out_cb)))
    {
      out_cb = (int *) alloca (sizeof (int) * num_out_cb);
      Set_2array (loop->out_cb, out_cb);
    }

  if ((num_basic_ind_var = Set_size (loop->basic_ind_var)))
    {
      basic_ind_var = (int *) alloca (sizeof (int) * num_basic_ind_var);
      Set_2array (loop->basic_ind_var, basic_ind_var);
    }

  L_find_all_ind_info (loop, loop_cb, num_cb);

  operand1 = L_new_register_operand (0,
				     L_native_machine_ctype,
				     L_PTYPE_NULL);

  operand2 = L_new_register_operand (0,
				     L_native_machine_ctype,
				     L_PTYPE_NULL);

  for (i = 0; i < num_basic_ind_var; i++)
    {
      operand1->value.r = basic_ind_var[i];

      if (!L_basic_induction_var (loop, operand1))
	continue;

      for (j = 0; j < num_basic_ind_var; j++)
        {
          if (i == j)
            continue;

	  operand2->value.r = basic_ind_var[j];

          /*
           *  match pattern
           *
           *    First 2 rules needed because remove elements from basic_ind_var
           *    sets but do not modify the arrays
           */

          if (!L_basic_induction_var (loop, operand2))
            continue;

          if (!L_basic_ind_var_in_same_family (loop, loop_cb, num_cb,
                                                operand1, operand2))
            continue;
          if (!L_not_live_in_out_cb (out_cb, num_out_cb, operand1))
	    break;
          if (!L_same_ind_increment (operand1, operand2, loop->ind_info))
            continue;
          if (!L_same_ind_initial_val(loop, operand1, 
				      operand2, loop->ind_info))
            continue;
          if (!L_no_uses_of_between_first_and_last_defs(loop, loop_cb, 
							 num_cb, operand1, 
							 operand2))
            continue;
          /*
           *  replace pattern
           */
#ifdef DEBUG_LOOP_IND_ELIM
          fprintf (ERR, "Apply loop ind elim 1 to ");
          L_print_operand (ERR, operand1, 0);
          fprintf (ERR, " replace with ");
          L_print_operand (ERR, operand2, 0);
          fprintf (ERR, "(loop %d)\n", loop->id);
#endif

          L_unmark_all_pre_post_increments_for_ind (loop, loop_cb, num_cb,
                                                    operand1);
          L_unmark_all_pre_post_increments_for_ind (loop, loop_cb, num_cb,
                                                    operand2);
          L_induction_elim_1 (loop, loop_cb, num_cb, operand1, operand2);

          /* Update ind var info */
          loop->basic_ind_var =
            Set_delete (loop->basic_ind_var, operand1->value.r);
          L_invalidate_ind_var (operand1, loop->ind_info);

          STAT_COUNT ("L_loop_induction_elimination1", 1, NULL);
          change++;
	  break;
        }
    }

  L_delete_operand (operand1);
  L_delete_operand (operand2);

  return change;
}

/*
 * Loop induction elimination (level 2)
 * ----------------------------------------------------------------------
 * Combine pairs of induction variables having identical increments
 * and initial values with a constant offset.
 */

static int
L_loop_induction_elimination2 (L_Loop * loop)
{
  int change = 0, i, j, num_cb, *loop_cb = NULL, num_out_cb, *out_cb = NULL,
    num_basic_ind_var, *basic_ind_var = NULL;
  L_Operand *operand1 = NULL, *operand2 = NULL;

  if ((num_cb = Set_size (loop->loop_cb)))
    {
      loop_cb = (int *) alloca (sizeof (int) * num_cb);
      Set_2array (loop->loop_cb, loop_cb);
    }

  if ((num_out_cb = Set_size (loop->out_cb)))
    {
      out_cb = (int *) alloca (sizeof (int) * num_out_cb);
      Set_2array (loop->out_cb, out_cb);
    }

  if ((num_basic_ind_var = Set_size (loop->basic_ind_var)))
    {
      basic_ind_var = (int *) alloca (sizeof (int) * num_basic_ind_var);
      Set_2array (loop->basic_ind_var, basic_ind_var);
    }

  L_find_all_ind_info (loop, loop_cb, num_cb);
  L_simplify_combs_of_ind_vars (loop, loop_cb, num_cb);

  operand1 = L_new_register_operand (0,
				     L_native_machine_ctype,
				     L_PTYPE_NULL);
  operand2 = L_new_register_operand (0,
				     L_native_machine_ctype,
				     L_PTYPE_NULL);

  for (i = 0; i < num_basic_ind_var; i++)
    {
      operand1->value.r = basic_ind_var[i];

      if (!L_basic_induction_var (loop, operand1))
	continue;

      for (j = 0; j < num_basic_ind_var; j++)
        {
          if (i == j)
            continue;

	  operand2->value.r = basic_ind_var[j];

          /*
           *  match pattern
           */

          if (!L_basic_induction_var (loop, operand2))
            continue;
          if (!L_not_live_in_out_cb (out_cb, num_out_cb, operand1))
	    break;
          if (!L_basic_ind_var_in_same_family (loop, loop_cb, num_cb,
                                                operand1, operand2))
            continue;
          if (!L_same_ind_increment (operand1, operand2, loop->ind_info))
            continue;
          if (!L_ind_constant_offset_initial_val (loop->preheader, operand1,
                                                   operand2, NULL, NULL,
                                                   loop->ind_info))
            continue;

          if (!L_all_uses_of_ind_can_change_offset (loop, loop_cb, num_cb,
                                                     out_cb, num_out_cb,
                                                     operand1, operand2))
            continue;
          if (!L_no_uses_of_between_first_and_last_defs
		(loop, loop_cb, num_cb, operand1, operand2))
            continue;
          /*
           *  replace pattern
           */
#ifdef DEBUG_LOOP_IND_ELIM
          fprintf (ERR, "Apply loop ind elim 2: replace  ");
          L_print_operand (ERR, operand1, 0);
          fprintf (ERR, " with ");
          L_print_operand (ERR, operand2, 0);
          fprintf (ERR, "(loop %d)\n", loop->id);
#endif

          L_unmark_all_pre_post_increments_for_ind (loop, loop_cb, num_cb,
                                                    operand1);
          L_unmark_all_pre_post_increments_for_ind (loop, loop_cb, num_cb,
                                                    operand2);
          L_induction_elim_2 (loop, loop_cb, num_cb, operand1, operand2);

          /* Update ind var info */
          loop->basic_ind_var =
            Set_delete (loop->basic_ind_var, operand1->value.r);
          L_invalidate_ind_var (operand1, loop->ind_info);
	  
          STAT_COUNT ("L_loop_induction_elimination2", 1, NULL);
	  change++;
	  break;
        }
    }

  L_delete_operand (operand1);
  L_delete_operand (operand2);

  return change;
}

/*
 * Loop induction elimination (level 3)
 * ----------------------------------------------------------------------
 * Combine pairs of induction variables having identical increments,
 * with no restriction on initial values.
 */

static int
L_loop_induction_elimination3 (L_Loop * loop)
{
  int change = 0, i, j, num_cb, *loop_cb = NULL, num_out_cb, *out_cb = NULL,
    num_basic_ind_var, *basic_ind_var = NULL;
  L_Operand *operand1 = NULL, *operand2 = NULL;

  if (!Lopti_do_complex_ind_elim)
    return 0;

  if ((num_cb = Set_size (loop->loop_cb)))
    {
      loop_cb = (int *) alloca (sizeof (int) * num_cb);
      Set_2array (loop->loop_cb, loop_cb);
    }

  /* check cost effectiveness of this opti */
  if (!L_cost_effective_for_ind_complex_elim (loop, loop_cb, num_cb))
    return 0;

  if ((num_out_cb = Set_size (loop->out_cb)))
    {
      out_cb = (int *) alloca (sizeof (int) * num_out_cb);
      Set_2array (loop->out_cb, out_cb);
    }

  if ((num_basic_ind_var = Set_size (loop->basic_ind_var)))
    {
      basic_ind_var = (int *) alloca (sizeof (int) * num_basic_ind_var);
      Set_2array (loop->basic_ind_var, basic_ind_var);
    }

  L_find_all_ind_info (loop, loop_cb, num_cb);

  operand1 = L_new_register_operand (0,
				     L_native_machine_ctype,
				     L_PTYPE_NULL);
  operand2 = L_new_register_operand (0,
				     L_native_machine_ctype,
				     L_PTYPE_NULL);

  for (i = 0; i < num_basic_ind_var; i++)
    {
      operand1->value.r = basic_ind_var[i];

      if (!(L_basic_induction_var (loop, operand1)))
	continue;

      for (j = 0; j < num_basic_ind_var; j++)
        {
          if (i == j)
            continue;

	  operand2->value.r = basic_ind_var[j];

          /*
           *  match pattern
           */

          if (!(L_basic_induction_var (loop, operand2)))
            continue;
          if (!(L_not_live_in_out_cb (out_cb, num_out_cb, operand1)))
            continue;
          if (!(L_basic_ind_var_in_same_family (loop, loop_cb, num_cb,
                                                operand1, operand2)))
            continue;
          if (!(L_same_ind_increment (operand1, operand2, loop->ind_info)))
            continue;
          if (!(L_ind_only_used_with_loop_inv_operands (loop, loop_cb, num_cb,
                                                        operand1)))
            continue;
          if (!(L_all_uses_of_ind_can_be_modified1 (loop, loop_cb, num_cb,
                                                    operand1)))
            continue;
          if (!
              (L_no_uses_of_between_first_and_last_defs
               (loop, loop_cb, num_cb, operand1, operand2)))
            continue;
          if (L_better_to_eliminate_operand2 (loop, loop_cb, num_cb, out_cb,
                                              num_out_cb, operand1, operand2))
            continue;
          if (L_ind_var_is_updated_by_pre_post_increment
              (loop, loop_cb, num_cb, operand1))
            continue;
          /* next restriction is conservative, but easier this way */
          if (L_ind_var_is_updated_by_pre_post_increment
              (loop, loop_cb, num_cb, operand2))
            continue;
          if (L_is_loop_var (loop, loop_cb, num_cb, operand1, loop->ind_info)
              && Lopti_preserve_loop_var)
            continue;

          /*
           *  replace pattern
           */
#ifdef DEBUG_LOOP_IND_ELIM
          fprintf (ERR, "Apply loop ind elim 3 to ");
          L_print_operand (ERR, operand1, 0);
          fprintf (ERR, " replace with ");
          L_print_operand (ERR, operand2, 0);
          fprintf (ERR, "(loop %d)\n", loop->id);
#endif
          L_induction_elim_3 (loop, loop_cb, num_cb, operand1, operand2);

          /* Update ind var info */
          loop->basic_ind_var =
            Set_delete (loop->basic_ind_var, operand1->value.r);
          L_invalidate_ind_var (operand1, loop->ind_info);

          STAT_COUNT ("L_loop_induction_elimination3", 1, NULL);
	  change++;
	  break;
        }
    }
  L_delete_operand (operand1);
  L_delete_operand (operand2);

  return change;
}

/*
 * Loop induction elimination (level 4)
 * ----------------------------------------------------------------------
 * Combine pairs of induction variables having different increments
 */

static int
L_loop_induction_elimination4 (L_Loop * loop)
{
  int change = 0, i, j, num_cb, *loop_cb = NULL, num_out_cb, *out_cb = NULL,
    num_basic_ind_var, *basic_ind_var = NULL;
  L_Operand *operand1 = NULL, *operand2 = NULL;

  if (!Lopti_do_complex_ind_elim)
    return 0;

  if ((num_cb = Set_size (loop->loop_cb)))
    {
      loop_cb = (int *) alloca (sizeof (int) * num_cb);
      Set_2array (loop->loop_cb, loop_cb);
    }

  /* check cost effectiveness of this opti */
  if (!L_cost_effective_for_ind_complex_elim (loop, loop_cb, num_cb))
    return 0;

  if ((num_out_cb = Set_size (loop->out_cb)))
    {
      out_cb = (int *) alloca (sizeof (int) * num_out_cb);
      Set_2array (loop->out_cb, out_cb);
    }

  if ((num_basic_ind_var = Set_size (loop->basic_ind_var)))
    {
      basic_ind_var = (int *) alloca (sizeof (int) * num_basic_ind_var);
      Set_2array (loop->basic_ind_var, basic_ind_var);
    }

  L_find_all_ind_info (loop, loop_cb, num_cb);

  operand1 = L_new_register_operand (0,
				     L_native_machine_ctype,
				     L_PTYPE_NULL);
  operand2 = L_new_register_operand (0,
				     L_native_machine_ctype,
				     L_PTYPE_NULL);

  for (i = 0; i < num_basic_ind_var; i++)
    {
      operand1->value.r = basic_ind_var[i];

      if (!L_basic_induction_var (loop, operand1))
	continue;

      for (j = 0; j < num_basic_ind_var; j++)
        {
          if (i == j)
            continue;

	  operand2->value.r = basic_ind_var[j];

          /*
           *  match pattern
           */

          if (!L_basic_induction_var (loop, operand2))
            continue;
          if (!L_not_live_in_out_cb (out_cb, num_out_cb, operand1))
            continue;
          if (!L_basic_ind_var_in_same_family (loop, loop_cb, num_cb,
					       operand1, operand2))
            continue;
          if (!L_num_constant_increment_of_ind (operand1, loop->ind_info))
            continue;
          if (!L_num_constant_increment_of_ind (operand2, loop->ind_info))
            continue;
          if (!L_ind_increment_is_multiple_of
	      (operand1, operand2, loop->ind_info))
            continue;
          if (L_same_ind_increment (operand1, operand2, loop->ind_info))
            continue;
          if (!L_ind_only_used_with_loop_inv_operands (loop, loop_cb, num_cb,
						       operand1))
            continue;
          if (!L_all_uses_of_ind_can_be_modified2 (loop, loop_cb, num_cb,
						   operand1))
            continue;
          if (!L_no_uses_of_between_first_and_last_defs
	      (loop, loop_cb, num_cb, operand1, operand2))
            continue;
          if (L_better_to_eliminate_operand2 (loop, loop_cb, num_cb, out_cb,
                                              num_out_cb, operand1, operand2))
            continue;
          if (L_ind_var_is_updated_by_pre_post_increment
              (loop, loop_cb, num_cb, operand1))
            continue;
          /* next restriction is conservative, but easier this way */
          if (L_ind_var_is_updated_by_pre_post_increment
              (loop, loop_cb, num_cb, operand2))
            continue;
          if (L_is_loop_var (loop, loop_cb, num_cb, operand1, loop->ind_info)
              && Lopti_preserve_loop_var)
            continue;
          /*
           *  replace pattern
           */
#ifdef DEBUG_LOOP_IND_ELIM
          fprintf (ERR, "Apply loop ind elim 4 to ");
          L_print_operand (ERR, operand1, 0);
          fprintf (ERR, " replace with ");
          L_print_operand (ERR, operand2, 0);
          fprintf (ERR, "(loop %d)\n", loop->id);
#endif
          L_induction_elim_4 (loop, loop_cb, num_cb, operand1, operand2);

          /* Update ind var info */
          loop->basic_ind_var =
            Set_delete (loop->basic_ind_var, operand1->value.r);
          L_invalidate_ind_var (operand1, loop->ind_info);

          STAT_COUNT ("L_loop_induction_elimination4", 1, NULL);
	  change++;
	  break;
        }
    }

  L_delete_operand (operand1);
  L_delete_operand (operand2);

  return change;
}

/*
 * Loop inductor reinitialization (LIRei)
 * ----------------------------------------------------------------------
 *
 */

static int
L_loop_induction_reinit (L_Loop * loop)
{
  int change, i, num_cb, *loop_cb = NULL, num_out_cb, *out_cb = NULL,
    num_backedge_cb, *backedge_cb = NULL;
  L_Cb *cb;
  L_Oper *op, *last_use;
  L_Operand *ind_var;

  change = 0;

  if (!Lopti_do_post_inc_conv)
    return (0);

  if ((num_cb = Set_size (loop->loop_cb)))
    {
      loop_cb = (int *) alloca (sizeof (int) * num_cb);
      Set_2array (loop->loop_cb, loop_cb);
    }

  if ((num_out_cb = Set_size (loop->out_cb)))
    {
      out_cb = (int *) alloca (sizeof (int) * num_out_cb);
      Set_2array (loop->out_cb, out_cb);
    }

  if ((num_backedge_cb = Set_size (loop->back_edge_cb)))
    {
      backedge_cb = (int *) alloca (sizeof (int) * num_backedge_cb);
      Set_2array (loop->back_edge_cb, backedge_cb);
    }

  L_find_all_ind_info (loop, loop_cb, num_cb);

  for (i = 0; i < num_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);
      for (op = cb->first_op; op != NULL; op = op->next_op)
        {
          /*
           *  match pattern
           */
	  /* 02/07/03 REK Adding a check to make sure we don't touch a
	   *              volatile oper. */
	  if (L_EXTRACT_BIT_VAL (op->flags, L_OPER_VOLATILE))
	    continue;
          if (!(Set_in (loop->basic_ind_var_op, op->id)))
            continue;
          if (!(L_int_add_opcode (op) || L_int_sub_opcode (op)))
            continue;
          if (L_marked_as_post_increment (op) ||
              L_marked_as_pre_increment (op))
            continue;
          last_use = L_find_last_use_of_ind_var (loop, loop_cb, num_cb,
                                                 backedge_cb, num_backedge_cb,
                                                 cb, op);
          if (!(L_load_opcode (last_use) || L_store_opcode (last_use)))
            continue;
          if (L_marked_as_post_increment (last_use) ||
              L_marked_as_pre_increment (last_use))
            continue;
          /* already can do post inc, no need to change anything */
          if (L_can_make_post_inc (last_use, op))
            continue;
          /* this opti just change offset, so if 0, no point */
          if (L_is_int_zero (last_use->src[0])
              || L_is_int_zero (last_use->src[1]))
            continue;
          if (!L_not_live_in_out_cb (out_cb, num_out_cb, op->dest[0]))
            continue;
          if (!L_ind_should_be_reinitialized (loop, loop_cb, num_cb, 
					      op->dest[0]))
            continue;
          /*
           *  replace pattern
           */
#ifdef DEBUG_LOOP_IND_REINIT
          fprintf (ERR, ">LIRei> Reinitialize ind var ");
          L_print_operand (ERR, op->dest[0], 0);
          fprintf (ERR, " (op %d) (loop %d)\n", op->id, loop->id);
#endif
          ind_var = L_copy_operand (op->dest[0]);
          L_reinit_induction_var (loop, loop_cb, num_cb, ind_var, last_use);
          L_delete_operand (ind_var);
	  change++;
        }
    }

  return (change);
}

/*
 * Loop induction reassociation
 * ----------------------------------------------------------------------
 * Enable more opportunities for post increment conversion.  Note this
 * opti increases the number of induction vars and therefore registers
 * so should be applied with caution!
 */
static int
L_loop_induction_reassociation (L_Loop * loop)
{
  int i, change, used_reg, avail_reg, num_new_ind_var, num_cb,
    num_backedge_cb, num_out_cb;
  int *loop_cb = NULL, *backedge_cb = NULL, *out_cb = NULL;
  L_Cb *cb;
  L_Oper *op, *last_use, *next_op;

  change = 0;

  if (!Lopti_do_post_inc_conv)
    return (0);

  if ((num_cb = Set_size (loop->loop_cb)))
    {
      loop_cb = (int *) alloca (sizeof (int) * num_cb);
      Set_2array (loop->loop_cb, loop_cb);
    }

  if ((num_out_cb = Set_size (loop->out_cb)))
    {
      out_cb = (int *) alloca (sizeof (int) * num_out_cb);
      Set_2array (loop->out_cb, out_cb);
    }

  if ((num_backedge_cb = Set_size (loop->back_edge_cb)))
    {
      backedge_cb = (int *) alloca (sizeof (int) * num_backedge_cb);
      Set_2array (loop->back_edge_cb, backedge_cb);
    }

  L_find_all_ind_info (loop, loop_cb, num_cb);

  /* Estimate if it makes sense to try reassociation (reg pressure) */
  L_estimate_num_live_regs_in_loop (loop, loop_cb, num_cb);
  used_reg = L_num_reg_used (L_native_machine_ctype);
  avail_reg = M_num_registers (L_native_machine_ctype);
  if (used_reg > avail_reg)
    {
#ifdef DEBUG_LOOP_IND_REASSOC
      fprintf (ERR, "Too many int regs used to allow Reassoc (loop %d)\n",
               loop->id);
#endif
      return (0);
    }

  for (i = 0; i < num_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);
      for (op = cb->first_op; op != NULL; op = next_op)
        {
          next_op = op->next_op;
          /*
           *  match pattern
           */
	  /* 02/07/03 REK Adding a check to make sure we don't touch a
	   *              volatile oper. */
	  if (L_EXTRACT_BIT_VAL (op->flags, L_OPER_VOLATILE))
	    continue;
          if (!(Set_in (loop->basic_ind_var_op, op->id)))
            continue;
          if (!(L_int_add_opcode (op) || L_int_sub_opcode (op)))
            continue;
          if (L_marked_as_post_increment (op) ||
              L_marked_as_pre_increment (op))
            continue;
          last_use = L_find_last_use_of_ind_var (loop, loop_cb, num_cb,
                                                 backedge_cb, num_backedge_cb,
                                                 cb, op);
          if (!(L_load_opcode (last_use) || L_store_opcode (last_use)))
            continue;
          if (L_marked_as_post_increment (last_use) ||
              L_marked_as_pre_increment (last_use))
            continue;
          /* already can do post inc, no need to change anything */
          if (L_can_make_post_inc (last_use, op))
            continue;
          /* this opti just change offset, so if 0, no point */
          if (L_is_int_zero (last_use->src[0])
              || L_is_int_zero (last_use->src[1]))
            continue;
          if (!(L_not_live_in_out_cb (out_cb, num_out_cb, op->dest[0])))
            continue;
          /* if can reinitialize it, then do that, it easier!! */
          if (L_ind_should_be_reinitialized
              (loop, loop_cb, num_cb, op->dest[0]))
            continue;
          if (!(L_ind_only_used_with_memory_ops (loop, loop_cb, num_cb,
                                                 op->dest[0])))
            continue;
          if (!(L_ind_only_used_with_loop_inv_operands (loop, loop_cb, num_cb,
                                                        op->dest[0])))
            continue;
          num_new_ind_var =
            L_num_ind_var_to_reassociate (loop, loop_cb, num_cb, op->dest[0]);
          if (num_new_ind_var <= 0)
            {
#ifdef DEBUG_LOOP_IND_REASSOC
              fprintf (ERR, "Reg %d not reassociated -- no uses to reassoc\n",
                       op->dest[0]->value.r);
#endif
              continue;
            }
          if ((num_new_ind_var + used_reg - 1) > avail_reg)
            {
#ifdef DEBUG_LOOP_IND_REASSOC
              fprintf (ERR,
                       "Reg %d not reassociated -- too many ind vars\n",
                       op->dest[0]->value.r);
#endif
              continue;
            }

          /*
           *  replace pattern
           */

#ifdef DEBUG_LOOP_IND_REASSOC
          fprintf (ERR, "Apply ind var reassociation to op%d (reg %d)\n",
                   op->id, op->dest[0]->value.r);
          fprintf (ERR, "Create %d new ind vars!!\n", num_new_ind_var);
#endif

          used_reg += (num_new_ind_var - 1);
          L_reassociate_ind_var (loop, loop_cb, num_cb, op->dest[0], last_use,
                                 num_new_ind_var);
          STAT_COUNT ("L_loop_induction_reassociation", 1, NULL);
	  change++;
        }
    }

  return (change);
}

/*
 * Loop post-increment conversion
 * ----------------------------------------------------------------------
 */

int
L_loop_post_increment_conversion (L_Loop * loop)
{
  int change, i, num_cb, *loop_cb = NULL, num_backedge_cb, *backedge_cb =
    NULL, num_out_cb, *out_cb = NULL;
  L_Cb *cb;
  L_Oper *op, *next, *last_use;
  L_Operand *base, *offset, *temp;

  change = 0;

  if (!Lopti_do_post_inc_conv)
    return (0);

  if ((num_cb = Set_size (loop->loop_cb)))
    {
      loop_cb = (int *) alloca (sizeof (int) * num_cb);
      Set_2array (loop->loop_cb, loop_cb);
    }

  if ((num_out_cb = Set_size (loop->out_cb)))
    {
      out_cb = (int *) alloca (sizeof (int) * num_out_cb);
      Set_2array (loop->out_cb, out_cb);
    }

  if ((num_backedge_cb = Set_size (loop->back_edge_cb)))
    {
      backedge_cb = (int *) alloca (sizeof (int) * num_backedge_cb);
      Set_2array (loop->back_edge_cb, backedge_cb);
    }

  L_find_all_ind_info (loop, loop_cb, num_cb);

  for (i = 0; i < num_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);
      for (op = cb->first_op; op != NULL; op = next)
        {
          next = op->next_op;
          /*
           *  match pattern
           */
	  /* 02/07/03 REK Adding a check to make sure we don't touch a
	   *              volatile oper. */
	  if (L_EXTRACT_BIT_VAL (op->flags, L_OPER_VOLATILE))
	    continue;
          if (!(Set_in (loop->basic_ind_var_op, op->id)))
            continue;
          if (!(L_int_add_opcode (op) || L_int_sub_opcode (op)))
            continue;
          if (L_marked_as_post_increment (op) ||
              L_marked_as_pre_increment (op))
            continue;
          last_use = L_find_last_use_of_ind_var (loop, loop_cb, num_cb,
                                                 backedge_cb, num_backedge_cb,
                                                 cb, op);
          if (!(L_load_opcode (last_use) || L_store_opcode (last_use)))
            continue;
          if (L_marked_as_post_increment (last_use) ||
              L_marked_as_pre_increment (last_use))
            continue;

          if (L_same_operand (op->dest[0], last_use->src[0]))
            {
              base = last_use->src[0];
              offset = last_use->src[1];
            }
          else if (L_same_operand (op->dest[0], last_use->src[1]))
            {
              base = last_use->src[1];
              offset = last_use->src[0];
            }
          else
            {
              continue;
            }
          if (!(L_not_live_in_out_cb (out_cb, num_out_cb, base)))
            continue;

          if (!(L_can_make_post_inc (last_use, op)))
            continue;
          /*
           *  replace pattern
           */

#ifdef DEBUG_LOOP_POST_INC_CONVERSION
          fprintf (ERR,
                   "Apply post increment conversion: op%d -> op%d (loop %d)\n",
                   last_use->id, op->id, loop->id);
          fprintf (ERR, "Induction var: ");
          L_print_operand (ERR, base, 0);
          fprintf (ERR, "\n");
#endif

          /* Align the operands to the proper positions!! */
          /* make sure base/offset in right place for ld/st */
          if (base != last_use->src[0])
            {
              last_use->src[0] = base;
              last_use->src[1] = offset;
            }

          /* if increment is sub, convert it to and add */
          if (L_int_sub_opcode (op) && L_is_int_constant (op->src[1]))
            {
              temp = op->src[1];
              op->src[1] = L_new_gen_int_operand (-(temp->value.i));
              L_delete_operand (temp);
              L_change_opcode (op, L_corresponding_add (op));
            }

          /* swap increment operands if necessary */
          if (L_int_add_opcode (op) &&
              L_same_operand (op->dest[0], op->src[1]))
            {
              temp = op->src[0];
              op->src[0] = op->src[1];
              op->src[1] = temp;
            }

          /* set the attribute fields indicating a post increment */
          L_mark_as_post_increment (last_use, op);

          STAT_COUNT ("L_loop_post_incremenet_conversion", 1, NULL);
          change++;
        }
    }

  return change;
}

int
L_dead_loop_removal (L_Loop * loop)     /* ADA 4/6/95 */
{
  int change = 0;

  int i, cnt, remove, *loop_cb = NULL, cb_num;
  L_Cb *cb = NULL, *cb0;
  L_Oper *op, *del;
  L_Flow *flow;

  cb_num = Set_size (loop->loop_cb);
  loop_cb = (int *) alloca (sizeof (int) * cb_num);
  Set_2array (loop->loop_cb, loop_cb);

  for (cnt = i = 0; i < cb_num; i++)
    {
      cb0 = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);

      for (op = cb0->first_op; op; op = op->next_op)
        if (op->opc != Lop_NO_OP && !L_uncond_branch (op))
          {
            cnt++;
            cb = cb0;
            break;
          }
    }

  if (cnt == 1)
    {                           /* So far we only do simple case */
      for (remove = 1, op = cb->first_op; op != NULL && remove;
           op = op->next_op)
        {
          if (!(L_store_opcode (op) || L_subroutine_call_opcode (op)))
            {
              for (i = 0; i < L_max_dest_operand; i++)
                if (L_in_oper_OUT_set
                    (cb, cb->last_op, op->dest[i], FALL_THRU_PATH))
                  {
                    remove = 0;
                    break;      /* double break */
                  }
            }
          else
            remove = 0;         /* break */
        }

      if (remove && !L_uncond_branch (cb->first_op))
        {
#if 0
          fprintf (ERR, "Dead Loop detected fn_name = <%s> ....\n",
                   L_fn->name);
          L_print_cb (ERR, NULL, cb);
#endif
          /* delete all (ops in cb) of them... */
          for (op = cb->first_op; op;)
            {
              if (L_general_branch_opcode (op) ||
		  L_check_branch_opcode (op))
                {
		  if (L_uncond_branch (op))
		    break;

		  /* conditional branch */

		  flow = L_find_flow_for_branch (cb, op);
		  cb->dest_flow = L_delete_flow (cb->dest_flow, flow);

		  cb0 = L_find_branch_dest (op);

		  cb0->src_flow =
		    L_delete_flow (cb0->src_flow,
				   L_find_flow_with_src_cb (cb0->src_flow,
							    cb));
                }
              /* everyone but unconditional branch (no predicate) */
              del = op;
              op = op->next_op;
              L_delete_oper (cb, del);
            }


          change++;

#if 0
          fprintf (ERR, "Dead Loop : done!\n");
          L_print_cb (ERR, NULL, cb);
#endif
        }

    }

  return change;
}




