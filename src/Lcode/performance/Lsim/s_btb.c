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
 *
 *  File:  s_btb.c
 *
 *  Description:  Functions to implement two types of BTB:
 *			1) Standard BTB with 2-bit counter
 *			2) Two-level BTB (Yeh,Patt)
 *
 *  Creation Date :  September, 1993
 *
 *  Author:  John Gyllenhaal, Dave Gallagher, Dan Connors
 *
 *  Revisions:
 *
 *      (C) Copyright 1993, John Gyllenhaal, Dave Gallagher, Dan Connors,
 *       & Wen-mei Hwu
 *      All rights granted to University of Illinois Board of Regents.
 *
\*****************************************************************************/

#include <config.h>
#include "s_main.h"

#include "s_super.h"
#include "s_vliw.h"
#include "s_profile.h"

/**************************************************************
	Function Declarations
**************************************************************/

BTB *S_create_BTB ();
int S_get_BTB_prediction ();
void S_create_return_stack ();


static void *S_create_BTB_block_data ();
char *S_BTB_create_history ();

void S_new_btb_data (BTB * btb, Scblock * block, int dir, int target,
		     int branch_addr, int control_opc, Sint * branch_sint);
void S_add_new_branch_to_BTB (BTB * btb, int branch_addr,
			      int actual_direction, int actual_target,
			      BTB_Stats * bstats, int control_opc,
			      Sint * branch_sint);

static int S_is_already_used_block ();
static int S_update_pattern ();
/* 10/22/04 REK Commenting out unused function prototypes to quiet compiler
 *              warnings. */
#if 0
static int S_has_more_ones_than_zeros ();
static int S_smart_pattern ();
#endif
int S_get_BTB_block_predict (Pnode * pnode, Sint * sint, BTB * btb,
			     Scblock * block, int *target, int branch_addr);
void S_update_BTB_block (Pnode * pnode, Sint * sint, BTB * btb,
			 Scblock * block, int actually_taken, int target,
			 int branch_addr);
void S_update_BTB_history (Pnode * pnode, Sint * sint, BTB * btb,
			   int actually_taken, int branch_addr);


/**************************************************************
	Exported Functions 
**************************************************************/

void
S_read_parm_BTB (Parm_Parse_Info * ppi)
{
/* SCM 7-21-00 */
/* S_read_parm_BTB currently reads the BTB parms into a temporary BTB */
/* structure which is later assigned to the pnode.  This is necessary */
/* because of the order in which parts of the pnode are created. */

  L_read_parm_s (ppi, "BTB_model", &S_temp_btb->model_name);
  L_read_parm_i (ppi, "BTB_size", &S_temp_btb->size);
  L_read_parm_i (ppi, "BTB_block_size", &S_temp_btb->block_size);
  L_read_parm_i (ppi, "BTB_assoc", &S_temp_btb->assoc);
  L_read_parm_i (ppi, "BTB_history_size", &S_temp_btb->history_size);

  /* DAC 9-11-95 */
  /* Added the ability to specify the starting patterns for the history */
  /* registers and the starting counter values in the history table */

  L_read_parm_s (ppi, "BTB_counter_type", &S_temp_btb->counter_type);

  L_read_parm_i (ppi, "BTB_number_history_sets",
		 &S_temp_btb->number_history_sets);

  L_read_parm_i (ppi, "BTB_index_set_at_bit", &S_temp_btb->index_set_at_bit);

  L_read_parm_i (ppi, "BTB_CBR_only", &S_temp_btb->CBR_only);


  /* SCM 7-21-00 */
  /* Added parms for nine variations on the two-level BTB - GAg, GAp, etc. */
  /* and for gshare and gselect. */

  L_read_parm_i (ppi, "BTB_number_BHT_sets", &S_temp_btb->number_BHT_sets);
  L_read_parm_i (ppi, "BTB_BHT_index_set_at_bit",
		 &S_temp_btb->BHT_index_set_at_bit);
  L_read_parm_i (ppi, "BTB_address_bits_used",
		 &S_temp_btb->address_bits_used);

  L_read_parm_s (ppi, "BTB_return_stack_type",
		 &S_temp_btb->return_stack_type);

  L_read_parm_i (ppi, "BTB_return_stack_size",
		 &S_temp_btb->return_stack_size);

  L_read_parm_b (ppi, "BTB_track_addresses", &S_temp_btb->track_addresses);

  /* Parameters for predication prediction modeling */
  L_read_parm_s (ppi, "BTB_predicate_prediction_type",
		 &S_temp_btb->predicate_prediction_type);

  L_read_parm_b (ppi, "use_predicate_predictor_pred1",
		 &S_temp_btb->predicate_predictor_pred1);

}

void
S_print_configuration_BTB (FILE * out)
{
  int pattern;			/* For the history pattern output fror 2-level BTB */
  fprintf (out, "# BTB CONFIGURATION:\n");
  fprintf (out, "\n");
  fprintf (out, "%12s BTB model.\n", S_pnode->btb->model_name);

  if ((S_pnode->btb->model == BTB_MODEL_PERFECT) ||
      (S_pnode->btb->model == BTB_MODEL_ALWAYS_WRONG))
    return;

  if (S_pnode->btb->track_addresses)
    {
      if (!S_simulation_with_profile_information)
	S_punt
	  ("S_print_configuration_BTB: tracking of addresses must simulate with profile");
    }

  fprintf (out, "%12u BTB entries.\n", S_pnode->btb->size);
  if (S_pnode->btb->model == BTB_MODEL_BTC)
    {
      fprintf (out, "%12u BTB block size.\n", S_pnode->btb->block_size);
    }

  fprintf (out, "%12u BTB associativity\n", S_pnode->btb->assoc);

  if ((S_pnode->btb->model == BTB_MODEL_TWO_LEVEL)
      || (S_pnode->btb->model == BTB_MODEL_COUNTER))
    {
      fprintf (out, "%12s BTB counter type.\n", S_pnode->btb->counter_type);

      fprintf (out, "%12s BTB return stack type.\n",
	       S_pnode->btb->return_stack_type);
      fprintf (out, "%12d BTB return stack size.\n",
	       S_pnode->btb->return_stack_size);
    }

  if (S_pnode->btb->model == BTB_MODEL_PREDICATE)
    {

      fprintf (out, "%12s BTB predicate prediction type.\n",
	       S_pnode->btb->predicate_prediction_type);
    }

  if ((S_pnode->btb->model == BTB_MODEL_GAG) ||
      (S_pnode->btb->model == BTB_MODEL_GAS) ||
      (S_pnode->btb->model == BTB_MODEL_GAP) ||
      (S_pnode->btb->model == BTB_MODEL_SAG) ||
      (S_pnode->btb->model == BTB_MODEL_SAS) ||
      (S_pnode->btb->model == BTB_MODEL_SAP) ||
      (S_pnode->btb->model == BTB_MODEL_PAG) ||
      (S_pnode->btb->model == BTB_MODEL_PAS) ||
      (S_pnode->btb->model == BTB_MODEL_PAP) ||
      (S_pnode->btb->model == BTB_MODEL_TWO_LEVEL) ||
      ((S_pnode->btb->model == BTB_MODEL_PREDICATE) &&
       (S_pnode->btb->predicate_prediction_model ==
	BTB_PREDICATE_MODEL_PEP_HISTORY)))
    {

      fprintf (out, "%12u BTB history size.\n", S_pnode->btb->history_size);
      fprintf (out, "%12d BTB history tables.\n",
	       S_pnode->btb->number_history_sets);
      fprintf (out, "%12d BTB set index bit.\n",
	       S_pnode->btb->index_set_at_bit);
      fprintf (out, "%12d BTB BHT entries.\n", S_pnode->btb->number_BHT_sets);
      fprintf (out, "%12d BTB BHT set index bit.\n",
	       S_pnode->btb->BHT_index_set_at_bit);
      pattern = 0x5555555 & ((1 << S_pnode->btb->history_size) - 1);
      fprintf (out, "%12x BTB default taken history pattern.\n", pattern);
      fprintf (out, "%12x BTB default not taken history pattern.\n", pattern);
      if (S_pnode->btb->CBR_only)
	{
	  fprintf (out,
		   "             Only conditional branches update history.\n");
	}
      else
	{
	  fprintf (out, "             All branches update history.\n");
	}
    }

  if ((S_pnode->btb->model == BTB_MODEL_GSHARE) ||
      (S_pnode->btb->model == BTB_MODEL_GSELECT))
    {
      fprintf (out, "%12u BTB history size.\n", S_pnode->btb->history_size);
      fprintf (out, "%12d BTB set index bit.\n",
	       S_pnode->btb->index_set_at_bit);
      fprintf (out, "%12d BTB address bits used.\n",
	       S_pnode->btb->address_bits_used);
      pattern = 0x5555555 & ((1 << S_pnode->btb->history_size) - 1);
      fprintf (out, "%12x BTB default taken history pattern.\n", pattern);
      fprintf (out, "%12x BTB default not taken history pattern.\n", pattern);
      if (S_pnode->btb->CBR_only)
	{
	  fprintf (out,
		   "             Only conditional branches update history.\n");
	}
      else
	{
	  fprintf (out, "             All branches update history.\n");
	}
    }


  fprintf (out, "\n");

}

BTB_Stats *
S_create_stats_btb ()
{
  BTB_Stats *stats;

  /* Create bus stats structure */
  stats = (BTB_Stats *) L_alloc (BTB_Stats_pool);

  /* Use STATS_ZERO(...) to initialize stats */
  STATS_ZERO (hits);
  STATS_ZERO (miss_pred);
  STATS_ZERO (miss_addr);
  STATS_ZERO (entries_kicked_out);
  STATS_ZERO (dynamic_cond);
  STATS_ZERO (dynamic_call);
  STATS_ZERO (dynamic_ret);
  STATS_ZERO (miss_pred_cond);
  STATS_ZERO (dynamic_uncond_pred);
  STATS_ZERO (miss_pred_uncond_pred);
  STATS_ZERO (miss_pred_call);
  STATS_ZERO (miss_pred_ret);
  stats->cond_address_node = NULL;
  stats->uncond_address_node = NULL;
  stats->uncond_pred_address_node = NULL;
  stats->call_address_node = NULL;
  stats->ret_address_node = NULL;

  /* Return the initialized structure */
  return (stats);
}

BTB_addr_node *
S_attach_address_node (BTB_addr_node * src1, BTB_addr_node * src2)
{
  BTB_addr_node *node;

  if (src1 == NULL)
    return src2;

  if (src2 == NULL)
    return src1;

  /* Otherwise append node for concatenating regions */
  for (node = src1; node->next != NULL; node = node->next)
    {
    }

  /* this assumes that regions are unique and cannot overlap */
  node->next = src2;

  return src1;

}


/* For multiple regions */
void
S_add_stats_btb (BTB_Stats * dest, BTB_Stats * src1, BTB_Stats * src2)
{

  STATS_ADD (hits);
  STATS_ADD (miss_pred);
  STATS_ADD (miss_addr);
  STATS_ADD (entries_kicked_out);
  STATS_ADD (dynamic_cond);
  STATS_ADD (dynamic_call);
  STATS_ADD (dynamic_ret);
  STATS_ADD (miss_pred_cond);
  STATS_ADD (dynamic_uncond_pred);
  STATS_ADD (miss_pred_call);
  STATS_ADD (miss_pred_uncond_pred);
  STATS_ADD (miss_pred_ret);

  dest->cond_address_node =
    S_attach_address_node (src1->cond_address_node, src2->cond_address_node);

  dest->uncond_address_node =
    S_attach_address_node (src1->uncond_address_node,
			   src2->uncond_address_node);

  dest->uncond_pred_address_node =
    S_attach_address_node (src1->uncond_pred_address_node,
			   src2->uncond_pred_address_node);

  dest->call_address_node =
    S_attach_address_node (src1->call_address_node, src2->call_address_node);

  dest->ret_address_node =
    S_attach_address_node (src1->ret_address_node, src2->ret_address_node);

}


void
S_print_stats_region_btb (FILE * out, Stats * stats,
			  char *rname, Stats * total_stats)
{
  double BTB_hit_ratio;
  double BTB_cond_hit_ratio;
  double BTB_uncond_hit_ratio;
  double BTB_call_hit_ratio;
  double BTB_ret_hit_ratio;
  double BTB_uncond_pred_hit_ratio;
  int total_predictions;
  int dynamic_uncond;
  int miss_pred_uncond;
  BTB_Stats *bstats;
  /* 10/22/04 REK Commenting out unused variable to quiet compiler warning. */
#if 0
  BTB_addr_node *node;
#endif
  int i;
  int num_cond_ops = 0;
  int num_uncond_ops = 0;
  int num_uncond_pred_ops = 0;
  int num_call_ops = 0;
  int num_ret_ops = 0;
  S_Oper *oper;
  int control_opc;
  S_Opc_Info *info;

  /* Setup Pstats calls */
  Pstats_out = out;
  Pstats_rname = rname;

  /* Get the BTB stats structure for ease of use */
  bstats = stats->btb;

  /* Want to distiguish parameter and result model lines */
  Pstats ("# BTB:");
  Pstats ("");
  Pstats ("%12s BTB simulation model.", S_pnode->btb->model_name);
  Pstats ("%12u BTB entries kicked out.", bstats->entries_kicked_out);
  total_predictions = bstats->hits + bstats->miss_pred + bstats->miss_addr;

  Pstats ("%12u BTB predictions were made.", total_predictions);

  Pstats ("%12u BTB predictions were correct.", bstats->hits);
  Pstats ("%12u BTB predictions were incorrect.",
	  bstats->miss_pred + bstats->miss_addr);
  Pstats ("%12u BTB predictions with incorrect direction.",
	  bstats->miss_pred);
  Pstats ("%12u BTB predictions with incorrect target.", bstats->miss_addr);

  Pstats ("");
  /* Begin stats on control percentages */
  dynamic_uncond = total_predictions -
    bstats->dynamic_cond -
    bstats->dynamic_call - bstats->dynamic_uncond_pred - bstats->dynamic_ret;

  miss_pred_uncond = bstats->miss_pred +
    bstats->miss_addr -
    bstats->miss_pred_cond -
    bstats->miss_pred_uncond_pred -
    bstats->miss_pred_call - bstats->miss_pred_ret;

  /* calculate the number of unique addresses */

  Pstats ("%12u BTB conditional branch predictions.", bstats->dynamic_cond);
  Pstats ("%12u BTB incorrect conditional branch predictions.",
	  bstats->miss_pred_cond);

  Pstats ("%12u BTB predicated uncond (jsr,jmp) branch predictions.",
	  bstats->dynamic_uncond_pred);
  Pstats ("%12u BTB incorrect predicated uncond branch predictions.",
	  bstats->miss_pred_uncond_pred);

  Pstats ("%12u BTB jmp predictions.", dynamic_uncond);
  Pstats ("%12u BTB incorrect jmp predictions.", miss_pred_uncond);

  Pstats ("%12u BTB call predictions.", bstats->dynamic_call);
  Pstats ("%12u BTB incorrect call predictions.", bstats->miss_pred_call);

  Pstats ("%12u BTB ret predictions.", bstats->dynamic_ret);
  Pstats ("%12u BTB incorrect ret predictions.", bstats->miss_pred_ret);

  Pstats ("");

  if (S_pnode->btb->track_addresses)
    {

      for (i = 0; i <= S_max_pc; i++)
	{
	  if (!prof_info[i].num_executed)
	    continue;

	  oper = oper_tab[i];
	  info = &opc_info_tab[oper->opc];
	  control_opc = info->opc_type;

	  switch (control_opc)
	    {
	    case CBR_OPC:
	      num_cond_ops++;
	      break;
	    case JSR_OPC:
	      num_call_ops++;
	      break;
	    case RTS_OPC:
	      num_ret_ops++;
	      break;
	    case JMP_OPC:
	      if (oper->flags & PREDICATED)
		num_uncond_pred_ops++;
	      else
		num_uncond_ops++;
	      break;
	    }
	}

      Pstats ("%12u BTB unique branches predicted.",
	      num_cond_ops + num_uncond_ops + num_call_ops + num_ret_ops +
	      num_uncond_pred_ops);

      Pstats ("%12u BTB unique conditional branches predicted.",
	      num_cond_ops);

      Pstats
	("%12u BTB unique predicated uncond (jsr,jmp) branches predicted.",
	 num_uncond_pred_ops);

      Pstats ("%12u BTB unique jmps predicted.", num_uncond_ops);

      Pstats ("%12u BTB unique calls predicted.", num_call_ops);

      Pstats ("%12u BTB unique rets predicted.", num_ret_ops);

    }

  /* Prevent divide by zero */
  if (bstats->dynamic_cond > 0)
    {
      BTB_cond_hit_ratio = 100.0
	* ((double) (bstats->dynamic_cond - bstats->miss_pred_cond) /
	   (double) (bstats->dynamic_cond));
    }
  else
    {
      BTB_cond_hit_ratio = 0.0;
    }

  if (bstats->dynamic_uncond_pred > 0)
    {
      BTB_uncond_pred_hit_ratio = 100.0
	*
	((double)
	 (bstats->dynamic_uncond_pred -
	  bstats->miss_pred_uncond_pred) /
	 (double) (bstats->dynamic_uncond_pred));
    }
  else
    {
      BTB_uncond_pred_hit_ratio = 0.0;
    }


  if (dynamic_uncond > 0)
    {
      BTB_uncond_hit_ratio = 100.0
	* ((double) (dynamic_uncond - miss_pred_uncond) /
	   (double) (dynamic_uncond));
    }
  else
    {
      BTB_uncond_hit_ratio = 0.0;
    }

  /* Prevent divide by zero */
  if (bstats->dynamic_call > 0)
    {
      BTB_call_hit_ratio = 100.0
	* ((double) (bstats->dynamic_call - bstats->miss_pred_call) /
	   (double) (bstats->dynamic_call));
    }
  else
    {
      BTB_call_hit_ratio = 0.0;
    }

  /* Prevent divide by zero */
  if (bstats->dynamic_ret > 0)
    {
      BTB_ret_hit_ratio = 100.0
	* ((double) (bstats->dynamic_ret - bstats->miss_pred_ret) /
	   (double) (bstats->dynamic_ret));
    }
  else
    {
      BTB_ret_hit_ratio = 0.0;
    }


  /* Prevent divide by zero */
  if (total_predictions > 0)
    {
      BTB_hit_ratio = 100.0 * ((double) bstats->hits /
			       (double) total_predictions);
    }
  else
    {
      BTB_hit_ratio = 0.0;
    }

  Pstats ("");
  Pstats ("# BTB SUMMARY:");
  Pstats ("");
  Pstats ("%12.2lf BTB cond hit ratio.", BTB_cond_hit_ratio);
  Pstats ("%12.2lf BTB predicated uncond (jsr,jmp) hit ratio.",
	  BTB_uncond_pred_hit_ratio);
  Pstats ("%12.2lf BTB uncond hit ratio (excludes calls&returns).",
	  BTB_uncond_hit_ratio);
  Pstats ("%12.2lf BTB call hit ratio.", BTB_call_hit_ratio);
  Pstats ("%12.2lf BTB ret hit ratio.", BTB_ret_hit_ratio);
  Pstats ("%12.2lf BTB overall hit ratio.", BTB_hit_ratio);
  Pstats ("");

}

int
S_get_addr_return_stack ()
{
  int pop_addr;

  pop_addr = S_pnode->btb->return_stack[S_pnode->btb->return_stack_pointer];
  return pop_addr;
}

int
S_pop_addr_return_stack (addr)
     int addr;
{
  int pop_addr;

  pop_addr = S_pnode->btb->return_stack[S_pnode->btb->return_stack_pointer];

  S_pnode->btb->return_stack_pointer--;
  if (S_pnode->btb->return_stack_pointer < 0)
    S_pnode->btb->return_stack_pointer = S_pnode->btb->return_stack_size - 1;

  return pop_addr;
}

void
S_push_addr_on_return_stack (addr)
     int addr;
{

  S_pnode->btb->return_stack_pointer++;

  if (S_pnode->btb->return_stack_pointer >= S_pnode->btb->return_stack_size)
    S_pnode->btb->return_stack_pointer = 0;

  S_pnode->btb->return_stack[S_pnode->btb->return_stack_pointer] = addr;
}


void
S_create_return_stack ()
{

  if (L_pmatch (S_temp_btb->return_stack_type, "none"))
    {
      S_temp_btb->stack_type = BTB_RETURN_STACK_NONE;
      S_temp_btb->return_stack_type = "None";
    }
  else if (L_pmatch (S_temp_btb->return_stack_type, "real"))
    {
      /* Allocate the approprate space */
      S_temp_btb->return_stack =
	(int *) malloc (sizeof (int) * (S_temp_btb->return_stack_size) + 1);
      S_temp_btb->stack_type = BTB_RETURN_STACK_REAL;
      S_temp_btb->return_stack_type = "Real";
    }
  else if (L_pmatch (S_temp_btb->return_stack_type, "perfect"))
    {
      S_temp_btb->stack_type = BTB_RETURN_STACK_PERFECT;
      S_temp_btb->return_stack_type = "Perfect";
    }

#if 0
  if ((S_temp_btb->stack_type != BTB_RETURN_STACK_NONE) &&
      ((S_operation_count_pred_ret > 0) || (S_operation_count_pred_call)))
    {
      S_punt
	("S_create_return_stack: Return Stack Not supported for predicated calls/rets");
    }
#endif

  /* Initialize the pointer */
  S_temp_btb->return_stack_pointer = 0;
}


BTB *
S_create_BTB (Pnode * pnode)
{
/* SCM 7-21-00 */
/* Since this must be called when creating the pnode, but parms must be read in */
/* earlier, the BTB is actually already allocated at this point through a temporary */
/* global pointer, S_temp_btb.  This just sets up the rest of the BTB. */
  int i;
  int idx;
  S_Fn *fn;
  S_Opc_Info *info;

  S_temp_btb->pnode = pnode;

  if (L_pmatch (S_temp_btb->model_name, "perfect"))
    {
      /* Reassign name to beautify simulation output */
      S_temp_btb->model_name = "Perfect";
      S_temp_btb->model = BTB_MODEL_PERFECT;
    }
  else if (L_pmatch (S_temp_btb->model_name, "always-wrong"))
    {
      /* Reassign name to beautify simulation output */
      S_temp_btb->model_name = "Always-wrong";
      S_temp_btb->model = BTB_MODEL_ALWAYS_WRONG;
    }
  else if (L_pmatch (S_temp_btb->model_name, "counter"))
    {
      /* Reassign name to beautify simulation output */
      S_temp_btb->model_name = "Counter";
      S_temp_btb->model = BTB_MODEL_COUNTER;
    }
  else if (L_pmatch (S_temp_btb->model_name, "two-level"))
    {
      /* Reassign name to beautify simulation output */
      S_temp_btb->model_name = "Two-level";
      S_temp_btb->model = BTB_MODEL_TWO_LEVEL;
    }
  else if (L_pmatch (S_temp_btb->model_name, "GAG"))
    {
      S_temp_btb->model = BTB_MODEL_GAG;
      S_temp_btb->number_BHT_sets = 1;
      S_temp_btb->number_history_sets = 1;
    }
  else if (L_pmatch (S_temp_btb->model_name, "GAS"))
    {
      S_temp_btb->model = BTB_MODEL_GAS;
      S_temp_btb->number_BHT_sets = 1;
    }
  else if (L_pmatch (S_temp_btb->model_name, "GAP"))
    {
      S_temp_btb->model = BTB_MODEL_GAP;
      S_temp_btb->number_BHT_sets = 1;
    }
  else if (L_pmatch (S_temp_btb->model_name, "SAG"))
    {
      S_temp_btb->model = BTB_MODEL_SAG;
      S_temp_btb->number_history_sets = 1;
    }
  else if (L_pmatch (S_temp_btb->model_name, "SAS"))
    {
      S_temp_btb->model = BTB_MODEL_SAS;
    }
  else if (L_pmatch (S_temp_btb->model_name, "SAP"))
    {
      S_temp_btb->model = BTB_MODEL_SAP;
    }
  else if (L_pmatch (S_temp_btb->model_name, "PAG"))
    {
      S_temp_btb->model = BTB_MODEL_PAG;
      S_temp_btb->number_history_sets = 1;
    }
  else if (L_pmatch (S_temp_btb->model_name, "PAS"))
    {
      S_temp_btb->model = BTB_MODEL_PAS;
    }
  else if (L_pmatch (S_temp_btb->model_name, "PAP"))
    {
      S_temp_btb->model = BTB_MODEL_PAP;
    }
  else if (L_pmatch (S_temp_btb->model_name, "gshare"))
    {
      S_temp_btb->model = BTB_MODEL_GSHARE;
    }
  else if (L_pmatch (S_temp_btb->model_name, "gselect"))
    {
      S_temp_btb->model = BTB_MODEL_GSELECT;
    }
  else if (L_pmatch (S_temp_btb->model_name, "static"))
    {
      /* Reassign name to beautify simulation output */
      S_temp_btb->model_name = "Static";
      S_temp_btb->model = BTB_MODEL_STATIC;
    }
  else if (L_pmatch (S_temp_btb->model_name, "btc"))
    {
      /* Reassign name to beautify simulation output */
      S_temp_btb->model_name = "BTC";
      S_temp_btb->model = BTB_MODEL_BTC;
    }
  else if ((L_pmatch (S_temp_btb->model_name, "predicate")) ||
	   (L_pmatch (S_temp_btb->model_name, "Predicate Prediction")))
    {

      S_temp_btb->model_name = "Predicate Prediction";
      S_temp_btb->model = BTB_MODEL_PREDICATE;

      /* Need to set the predicate prediction model */
      if (L_pmatch (S_temp_btb->predicate_prediction_type, "POP"))
	{
	  S_temp_btb->predicate_prediction_model = BTB_PREDICATE_MODEL_POP;
	}
      else if (L_pmatch (S_temp_btb->predicate_prediction_type, "PEP"))
	{
	  S_temp_btb->predicate_prediction_model = BTB_PREDICATE_MODEL_PEP;
	}
      else
	if (L_pmatch (S_temp_btb->predicate_prediction_type, "PEP_HISTORY"))
	{
	  S_temp_btb->predicate_prediction_model =
	    BTB_PREDICATE_MODEL_PEP_HISTORY;
	}
      else
	S_punt ("S_create_BTB: Illegal BTB predicate type '%s'",
		S_temp_btb->predicate_prediction_type);
    }
  else
    S_punt ("S_create_BTB: Illegal BTB model '%s'", S_temp_btb->model_name);

  /* Beautify the other names: stack, counter */
  if (L_pmatch (S_temp_btb->counter_type, "Automaton_A3"))
    {
      S_temp_btb->counter_type = "Automaton_A3";
      S_temp_btb->counter_model = BTB_COUNTER_MODEL_A3;
    }
  else if ((L_pmatch (S_temp_btb->counter_type, "up-down")) ||
	   (L_pmatch (S_temp_btb->counter_type, "up_down")) ||
	   (L_pmatch (S_temp_btb->counter_type, "Automaton_A2")))
    {
      S_temp_btb->counter_type = "Automaton_A2";
      S_temp_btb->counter_model = BTB_COUNTER_MODEL_A2;
    }
  else if ((L_pmatch (S_temp_btb->counter_type, "last-taken")))
    {
      S_temp_btb->counter_type = "Last-Taken";
      S_temp_btb->counter_model = BTB_COUNTER_MODEL_LT;
    }

  /* For perfect cases, do not need to allocate cache or history */
  if ((S_temp_btb->model == BTB_MODEL_PERFECT) ||
      (S_temp_btb->model == BTB_MODEL_ALWAYS_WRONG))
    {
      S_temp_btb->cache = NULL;
      S_temp_btb->history = NULL;
    }
  else
    {
      if ((S_temp_btb->size <= 0) || (S_temp_btb->assoc <= 0))
	S_punt ("S_create_BTB: Invalid BTB_size %i or BTB_assoc %i",
		S_temp_btb->size, S_temp_btb->assoc);

      /* Except for BTC, BTB_block_size must equal 1 (force to 1) */
      if (S_temp_btb->model != BTB_MODEL_BTC)
	{
	  S_temp_btb->block_size = 1;
	}
      else if (!S_is_power_of_two (S_temp_btb->block_size))
	{
	  S_punt ("S_create_BTB: BTB_block_size (%i) must be power of"
		  " two for BTC", S_temp_btb->block_size);
	}

      S_temp_btb->cache =
	S_create_cache (S_temp_btb->size, S_temp_btb->block_size,
			S_temp_btb->assoc, S_create_BTB_block_data);

      /* Two BTB models need to generate history tables */
      if ((S_temp_btb->model == BTB_MODEL_TWO_LEVEL) ||
	  (S_temp_btb->model == BTB_MODEL_GAG) ||
	  (S_temp_btb->model == BTB_MODEL_GAS) ||
	  (S_temp_btb->model == BTB_MODEL_SAG) ||
	  (S_temp_btb->model == BTB_MODEL_SAS) ||
	  (S_temp_btb->model == BTB_MODEL_PAG) ||
	  (S_temp_btb->model == BTB_MODEL_PAS) ||
	  ((S_temp_btb->model == BTB_MODEL_PREDICATE) &&
	   (S_temp_btb->predicate_prediction_model ==
	    BTB_PREDICATE_MODEL_PEP_HISTORY)))
	{
	  /* need to allocate space for the history tables */
	  S_temp_btb->history = (char **)
	    malloc (sizeof (char *) * (S_temp_btb->number_history_sets));

	  /* Check for even number of sets */

	  if (!S_is_power_of_two (S_temp_btb->number_history_sets))
	    S_punt ("S_create_BTB: BTB_number_history_sets (%i) "
		    "must be power of two", S_temp_btb->number_history_sets);

	  for (i = 0; i < S_temp_btb->number_history_sets; i++)
	    {
	      S_temp_btb->history[i] =
		S_BTB_create_history (S_temp_btb->history_size);
	    }
	}
      else if (S_temp_btb->model == BTB_MODEL_GSELECT)
	{
	  S_temp_btb->history = (char **) malloc (sizeof (char *));
	  S_temp_btb->history[0] =
	    S_BTB_create_history (S_temp_btb->history_size +
				  S_temp_btb->address_bits_used);
	}
      else if (S_temp_btb->model == BTB_MODEL_GSHARE)
	{
	  S_temp_btb->history = (char **) malloc (sizeof (char *));
	  if (S_temp_btb->history_size > S_temp_btb->address_bits_used)
	    {
	      S_temp_btb->history[0] =
		S_BTB_create_history (S_temp_btb->history_size);
	    }
	  else
	    {
	      S_temp_btb->history[0] =
		S_BTB_create_history (S_temp_btb->address_bits_used);
	    }
	}
      else
	{
	  S_temp_btb->history = NULL;
	}

      /* initialize patterns */
      S_temp_btb->max_history_pattern = (1 << S_temp_btb->history_size) - 1;
      S_temp_btb->default_not_taken_history_pattern = 0x55555555 &
	S_temp_btb->max_history_pattern;
      S_temp_btb->default_taken_history_pattern = 0x55555555 &
	S_temp_btb->max_history_pattern;

      if ((S_temp_btb->model == BTB_MODEL_GAP) ||
	  (S_temp_btb->model == BTB_MODEL_SAP) ||
	  (S_temp_btb->model == BTB_MODEL_PAP) ||
	  (S_temp_btb->model == BTB_MODEL_PAS) ||
	  (S_temp_btb->model == BTB_MODEL_PAG))
	{
	  /* SCM 7-21-00 */
	  /* For per-branch history tables, the table is stored in the S_Oper data */
	  /* Looping through all the code, history table must be declared for only branches */
	  if (!S_is_power_of_two (S_temp_btb->number_history_sets))
	    S_punt
	      ("S_create_BTB: BTB_number_history_sets (%i) must be power of two",
	       S_temp_btb->number_history_sets);
	  for (fn = head_fn; fn != NULL; fn = fn->next_fn)
	    {
	      for (idx = 0; idx < fn->op_count; idx++)
		{
		  info = &opc_info_tab[fn->op[idx].opc];
		  if ((info->opc_type == CBR_OPC) ||
		      (info->opc_type == JSR_OPC) ||
		      (info->opc_type == RTS_OPC) ||
		      (info->opc_type == JMP_OPC))
		    {
		      /* only create tables when necessary */
		      if ((S_temp_btb->model == BTB_MODEL_GAP) ||
			  (S_temp_btb->model == BTB_MODEL_SAP) ||
			  (S_temp_btb->model == BTB_MODEL_PAP))
			{
			  fn->op[idx].BTB_prediction_table =
			    S_BTB_create_history (S_temp_btb->history_size);
			}
		      /* initialize history registers to default */
		      fn->op[idx].BTB_branch_history =
			S_temp_btb->default_taken_history_pattern;
		    }
		}
	    }
	}
      /* For the not per-branch schemes, a branch history table much be used */
      if ((S_temp_btb->model == BTB_MODEL_SAG) ||
	  (S_temp_btb->model == BTB_MODEL_SAS) ||
	  (S_temp_btb->model == BTB_MODEL_SAP) ||
	  (S_temp_btb->model == BTB_MODEL_GAG) ||
	  (S_temp_btb->model == BTB_MODEL_GAS) ||
	  (S_temp_btb->model == BTB_MODEL_GAP))
	{
	  S_temp_btb->BHT =
	    (int *) malloc (sizeof (int) * (S_temp_btb->number_BHT_sets));
	  /*initialize all to default */
	  for (i = 0; i < S_temp_btb->number_BHT_sets; i++)
	    {
	      S_temp_btb->BHT[i] = S_temp_btb->default_taken_history_pattern;
	    }
	}
      if ((S_temp_btb->model == BTB_MODEL_GSELECT) ||
	  (S_temp_btb->model == BTB_MODEL_GSHARE))
	{
	  /* just one entry in BHT for global schemes */
	  S_temp_btb->BHT = (int *) malloc (sizeof (int));
	}
    }

  /* Create the function stack for the BTB */
  S_create_return_stack ();

  return (S_temp_btb);
}



/*
 * Returns -1 if fall thru, target pc otherwise (which may be 0).
 * Marks sint with MISPREDICTED if branch is mispredicted.
 */
int
S_get_BTB_prediction (Pnode * pnode, Sint * branch_sint)
{
  int predicted_taken, predicted_target;
  int branch_addr, actually_taken, actual_target;
  int off_path;
  Scblock *block;
  /* 10/22/04 REK Commenting out unused variable to quiet compiler warning. */
#if 0
  BTB_data *data;
#endif
  BTB *btb;
  BTB_Stats *bstats;
  int mispredicted, wrong_target;
  int pop_addr;
  S_Opc_Info *info;
  int control_opc;
  int squashed;

  /* Get btb stats structure for ease of use */
  bstats = branch_sint->stats->btb;

  btb = pnode->btb;
  actually_taken = branch_sint->flags & BRANCHED;
  actual_target = branch_sint->trace.target_pc;
  off_path = branch_sint->flags & OFF_PATH;
  squashed = branch_sint->flags & PRED_SQUASHED;
  branch_addr = branch_sint->oper->pc;


  /* For BTC, use address that icache uses, for now mult by 4 */
  if (btb->model == BTB_MODEL_BTC)
    branch_addr = branch_addr << 2;

  /* Get opcode info from table */
  info = &opc_info_tab[branch_sint->oper->opc];

  control_opc = info->opc_type;

  /* Update the function stack for subroutine calls  DAC 9/30/95 */
  /* For subroutine calls with a machine using function stacks   */
  if ((control_opc == JSR_OPC) && (btb->stack_type == BTB_RETURN_STACK_REAL))
    {
      if (!(branch_sint->flags & UNTRACED_JSR))
	{

	  /* Also check to see if the jsr is squashed */
	  if ((!squashed) && (!off_path))
	    {
	      if (btb->model == BTB_MODEL_BTC)
		S_push_addr_on_return_stack (branch_addr + 4);
	      else
		S_push_addr_on_return_stack (branch_addr + 1);
	    }
	}
    }

  /* Initialize prediction to fall-thru */
  predicted_taken = 0;
  predicted_target = -1;

  switch (btb->model)
    {
    case BTB_MODEL_PERFECT:
#if 0
      /* Can happen with VLIW */
      if (off_path)
	S_punt ("S_get_BTB_prediction: off path sint with perfect pred.");
#endif

      predicted_taken = actually_taken;
      predicted_target = actual_target;
      break;


    case BTB_MODEL_ALWAYS_WRONG:
      /* If on-path fall thru, predict taken to pc + 4 (picked arbitrarily),
       * otherwise use the default of predict fall-thru.
       */
      if (!off_path && !actually_taken)
	{
	  predicted_taken = 1;
	  predicted_target = branch_sint->oper->pc + 4;
	}
      break;

      /* Use BTB with different schemes */
    default:

      /* Get the BTB cache block */
      block = S_cache_find_addr (btb->cache, branch_addr);

      /* Retrieve the function stack for return from subroutine calls */
      if ((control_opc == RTS_OPC) && 
	  (btb->stack_type != BTB_RETURN_STACK_NONE))
	{

	  if (btb->stack_type == BTB_RETURN_STACK_REAL)
	    {
	      pop_addr = S_get_addr_return_stack ();
	      predicted_taken = 1;
	      predicted_target = pop_addr;

	      /* Check validity of sint for updating the return stack */
	      if ((!squashed) && (!off_path))
		pop_addr = S_pop_addr_return_stack (branch_addr);
	    }
	  else
	    {
	      /* Assume perfect stack model, always correct */
	      predicted_taken = actually_taken;
	      predicted_target = actual_target;
	    }
	}
      else			/* not an RTS type opcode */
	{
	  if (block != NULL)
	    {
	      /* BTB entry exists, get prediction */
	      predicted_taken = S_get_BTB_block_predict (pnode,
							 branch_sint,
							 btb,
							 block,
							 &predicted_target,
							 branch_addr);

	      /* Do not update BTB or stats if an off path instruction */
	      if (!off_path)
		{
		  S_update_BTB_block (pnode,
				      branch_sint,
				      btb,
				      block,
				      actually_taken,
				      actual_target, branch_addr);
		}
	    }
	  else			/* block == NULL */
	    {
	      /* Do not update BTB or stats if an off path instruction */
	      if ((!off_path) && (actually_taken))
		{
		  S_add_new_branch_to_BTB (btb, branch_addr,
					   actually_taken, actual_target,
					   bstats, control_opc, branch_sint);
		}
	    }			/* end of if (block != NULL) ... else */

	  if (!off_path)
	    {
	      /* Update history and predictors regardless of whether the branch */
	      /* is in the BTB's cache. */
	      S_update_BTB_history (pnode,
				    branch_sint,
				    btb, actually_taken, branch_addr);
	    }			/* end of if (!off_path) */
	}			/* end of ... else */
      break;
    }				/* end of switch (btb->model) */

  /* Detect misprediction, assume correct */
  mispredicted = 0;
  wrong_target = 0;

  /* If actually taken */
  if (actually_taken)
    {
      /* Mispredict if predict not taken or target wrong */
      if (!predicted_taken)
	{
	  mispredicted = 1;
	}
      else if (predicted_target != actual_target)
	{
	  mispredicted = 1;
	  wrong_target = 1;
	}
    }

  /* If actually fell-thru */
  else
    {
      /* If predict taken, then mispredicted */
      if (predicted_taken)
	mispredicted = 1;
    }


  if (!off_path)
    {
      /* Keep track of the dynamic ops  */
      switch (control_opc)
	{
	case CBR_OPC:
	  bstats->dynamic_cond++;
	  break;
	case JSR_OPC:
	  bstats->dynamic_call++;
	  break;
	case RTS_OPC:
	  bstats->dynamic_ret++;
	  break;
	default:
	  if ((branch_sint->oper->flags & PREDICATED))
	    bstats->dynamic_uncond_pred++;
	}

      /* Update stats for on path only */
      if (wrong_target)
	{
	  bstats->miss_addr++;
	  /* Count the wrong address as a miss */
	  switch (control_opc)
	    {
	    case CBR_OPC:
	      bstats->miss_pred_cond++;
	      break;
	    default:
	      if ((branch_sint->oper->flags & PREDICATED))
		{
		  bstats->miss_pred_uncond_pred++;
		}
	      else
		{
		  switch (control_opc)
		    {
		    case JSR_OPC:
		      bstats->miss_pred_call++;
		      break;
		    case RTS_OPC:
		      bstats->miss_pred_ret++;
		      break;
		    }

		}

	    }
	}
      else if (mispredicted)
	{
	  bstats->miss_pred++;

	  switch (control_opc)
	    {
	    case CBR_OPC:
	      bstats->miss_pred_cond++;
	      break;
	    default:
	      if ((branch_sint->oper->flags & PREDICATED))
		{
		  bstats->miss_pred_uncond_pred++;
		}
	      else
		{
		  switch (control_opc)
		    {
		    case JSR_OPC:
		      bstats->miss_pred_call++;
		      break;
		    case RTS_OPC:
		      bstats->miss_pred_ret++;
		      break;
		    }
		}
	    }

	}
      else
	{
	  bstats->hits++;
	}
    }


  /* Mark sint if mispredicted branch */
  if (mispredicted)
    branch_sint->flags |= MISPREDICTED;

  /* Return prediction (-1 for fall thru, otherwise target) */
  if (!predicted_taken)
    return (-1);
  else
    return (predicted_target);
}



/**************************************************************
	Static Functions 
**************************************************************/

static void *
S_create_BTB_block_data ()
{
  BTB_data *new_data;

  new_data = (BTB_data *) malloc (sizeof (BTB_data));
  new_data->target = 0;
  new_data->branch_addr = 0;

  switch (S_temp_btb->model)
    {
    case BTB_MODEL_COUNTER:
      new_data->counter = 2;
      break;

    case BTB_MODEL_TWO_LEVEL:
      new_data->counter = 0;
      break;

    case BTB_MODEL_GAG:
      new_data->counter = 0;
      break;

    case BTB_MODEL_GAS:
      new_data->counter = 0;
      break;

    case BTB_MODEL_GAP:
      new_data->counter = 0;
      break;

    case BTB_MODEL_SAG:
      new_data->counter = 0;
      break;

    case BTB_MODEL_SAS:
      new_data->counter = 0;
      break;

    case BTB_MODEL_SAP:
      new_data->counter = 0;
      break;

    case BTB_MODEL_PAG:
      new_data->counter = 0;
      break;

    case BTB_MODEL_PAS:
      new_data->counter = 0;
      break;

    case BTB_MODEL_PAP:
      new_data->counter = 0;
      break;

    case BTB_MODEL_GSHARE:
      new_data->counter = 0;
      break;

    case BTB_MODEL_GSELECT:
      new_data->counter = 0;
      break;

    case BTB_MODEL_STATIC:	/* Counter not used */
      new_data->counter = 1;
      break;

    case BTB_MODEL_BTC:
      new_data->counter = 1;
      break;

    case BTB_MODEL_PREDICATE:
      switch (S_pnode->btb->predicate_prediction_model)
	{
	case BTB_PREDICATE_MODEL_POP:
	  new_data->counter = 1;
	  new_data->counter2 = 2;
	  break;
	case BTB_PREDICATE_MODEL_PEP:
	  new_data->counter = 0;
	  new_data->counter2 = 0;
	  break;
	case BTB_PREDICATE_MODEL_PEP_HISTORY:
	  break;
	}

      break;
    default:
      S_punt ("S_create_BTB_block_data: unknown BTB model %i.",
	      S_temp_btb->model);
    }

  return ((void *) new_data);
}


char *
S_BTB_create_history (history_size)
     int history_size;
{
  char *new_history;
  int i, history_file_size;

  history_file_size = 1 << history_size;

  new_history = (char *) malloc (sizeof (char) * history_file_size);

/* SCM 7/21/00 */
/* Got rid of "smart" history table initialization */
  for (i = 0; i < history_file_size; i++)
    {
      new_history[i] = 0;
    }

  return (new_history);
}

static int
S_get_history_set_number (addr)
     int addr;
{
  int set_mask;
  int count;
  int log;
  int set;

  log = S_log_base_two (S_pnode->btb->number_history_sets);
  set_mask = 0;

  for (count = log; count > 0; count--)
    set_mask = ((set_mask << 1) | 1);

  set = ((addr >> S_pnode->btb->index_set_at_bit) & set_mask);

  return set;
}


static int
S_get_BHT_set_number (addr)
     int addr;
{
  int set_mask;
  int count;
  int log;
  int set;

  log = S_log_base_two (S_pnode->btb->number_BHT_sets);
  set_mask = 0;

  for (count = log; count > 0; count--)
    set_mask = ((set_mask << 1) | 1);

  set = ((addr >> S_pnode->btb->BHT_index_set_at_bit) & set_mask);

  return set;
}


void
S_add_new_branch_to_BTB (BTB * btb, int branch_addr, int actual_direction,
			 int actual_target, BTB_Stats * bstats,
			 int control_opc, Sint * branch_sint)
{
  Scblock *replace_block;

  replace_block = S_cache_find_LRU (btb->cache, branch_addr);

  if (S_is_already_used_block (replace_block))
    {
      bstats->entries_kicked_out++;
    }

  S_cache_change_addr (btb->cache, replace_block, branch_addr);
  S_new_btb_data (btb, replace_block, actual_direction, actual_target,
		  branch_addr, control_opc, branch_sint);
  S_cache_make_MRU (btb->cache, replace_block);
}

int
S_get_branch_predicate_prediction (Sint * branch_sint)
{
  int operand_id;

  /* Sint stores pointers to operand ids get id of pred[1] */
  if (S_pnode->btb->predicate_predictor_pred1)
    {
      operand_id = branch_sint->oper->operand[S_first_pred + 1];

      /* Use pred[0] if pred[1] not set */
      if (operand_id <= 0)
	operand_id = branch_sint->oper->operand[S_first_pred];
    }
  else
    operand_id = branch_sint->oper->operand[S_first_pred];

  /* If still 0, have unpredicated oper */
  if (operand_id <= 0)
    {
      fprintf (stderr, "%s cb %i: ", branch_sint->oper->cb->fn->name,
	       branch_sint->oper->cb->lcode_id);
      S_print_sint (stderr, branch_sint);
      S_punt
	("S_get_branch_predicate_prediction: Called for non-predicated sint\n");
    }

  return (operand_id);
}


/* This should be called during the fetch cycle of branch instruction */
int
S_get_predicate_regfile_value (Pnode * pnode, int reg_index)
{
  int value = 0;
  Superscalar *super;
  VLIW *vliw;

  /* Should never get an zero reg_index or bigger than reg file size */

  if ((reg_index == 0) || (reg_index > S_max_register_operand))
    {
      S_punt ("S_get_predicate_regfile_value: Invalid reg_index %i!",
	      reg_index);
    }

  if (S_processor_model == PROCESSOR_MODEL_SUPERSCALAR)
    {
      super = (Superscalar *) pnode->processor_v;
      value = super->reg_file[reg_index].accessible_value;
    }
  else if (S_processor_model == PROCESSOR_MODEL_VLIW)
    {

      vliw = (VLIW *) pnode->processor_v;
      value = vliw->reg_file[reg_index].accessible_value;
    }
  else
    {
      S_punt
	("S_get_predicate_regfile_value: processor model %s not supported",
	 S_processor_model_name);
    }

#if 0
  if (pnode->btb->cur_sint->flags & BRANCHED)
    return (1);
  else
    return (0);
#endif

  /* Take whatever is in the predicate register file, not in pipe */
  return (value);
}


int
S_get_BTB_block_predict (Pnode * pnode, Sint * branch_sint, BTB * btb,
			 Scblock * block, int *target, int branch_addr)
{
  BTB_data *data;
  int pattern = 0, count;
  int set;
  int entry_type;
  /* 10/22/04 REK Commenting out unused variables to quiet compiler
   *              warnings. */
#if 0
  Superscalar *super;
  VLIW *vliw;
  int def_cb_id, def_fetch_cycle;
  S_Oper *def_oper;
  Sint *def_sint;
#endif
  int predicate_value;
  int addr_mask;
  int temp_history;

  data = (BTB_data *) block->data;

  *target = data->target;	/* all BTB types get the target */

  if (btb->model == BTB_MODEL_COUNTER)
    return (data->counter >= 2);

  else if (btb->model == BTB_MODEL_TWO_LEVEL)
    {
      entry_type = data->entry_type;

      if (entry_type == CBR_OPC)
	{
	  pattern = data->counter;
	  set = S_get_history_set_number (branch_addr);
	  count = btb->history[set][pattern];
	  return (count >= 2);
	}
      else
	{			/* Unconditional type */
	  /* Check to see if this is a predicated instructions */
	  /* If so, then it is also in the history table */
	  /* this means JSRs, RETs, UNCONDs */
	  if ((branch_sint->oper->flags & PREDICATED))
	    {
	      pattern = data->counter;
	      set = S_get_history_set_number (branch_addr);
	      count = btb->history[set][pattern];
	      return (count >= 2);
	    }
	  /* Should always return taken with the target */
	  return 2;
	}
    }
  else if ((btb->model == BTB_MODEL_GAG) ||
	   (btb->model == BTB_MODEL_GAS) ||
	   (btb->model == BTB_MODEL_GAP) ||
	   (btb->model == BTB_MODEL_SAG) ||
	   (btb->model == BTB_MODEL_SAS) ||
	   (btb->model == BTB_MODEL_SAP) ||
	   (btb->model == BTB_MODEL_PAG) ||
	   (btb->model == BTB_MODEL_PAS) || (btb->model == BTB_MODEL_PAP))
    {
      entry_type = data->entry_type;

      if ((entry_type == CBR_OPC) || (branch_sint->oper->flags & PREDICATED))
	{
	  /* get pattern, as appropriate from the BHT or the S_Oper struct */
	  if ((btb->model == BTB_MODEL_PAG) ||
	      (btb->model == BTB_MODEL_PAS) || (btb->model == BTB_MODEL_PAP))
	    {
	      pattern = branch_sint->oper->BTB_branch_history;
	    }
	  else
	    {
	      pattern = btb->BHT[S_get_BHT_set_number (branch_addr)];
	    }
	  /* use pattern to index into the appropriate table, depending on scheme */
	  if ((btb->model == BTB_MODEL_GAP) ||
	      (btb->model == BTB_MODEL_SAP) || (btb->model == BTB_MODEL_PAP))
	    {
	      count = branch_sint->oper->BTB_prediction_table[pattern];
	    }
	  else
	    {
	      set = S_get_history_set_number (branch_addr);
	      count = btb->history[set][pattern];
	    }
	  return (count >= 2);
	}
      return 2;
    }
  else if (btb->model == BTB_MODEL_GSHARE)
    {
      /* xor subset of addr bits with global history pattern (in BHT[0]) */
      addr_mask = 0;
      for (count = btb->address_bits_used; count > 0; count--)
	{
	  addr_mask = ((addr_mask << 1) | 1);
	}
      pattern = (branch_addr >> btb->index_set_at_bit) & addr_mask;
      temp_history = btb->BHT[0];
      if (btb->history_size < btb->address_bits_used)
	{
	  temp_history =
	    temp_history << (btb->address_bits_used - btb->history_size);
	}
      pattern = pattern ^ temp_history;
      count = btb->history[0][pattern];
      return (count >= 2);
    }
  else if (btb->model == BTB_MODEL_GSELECT)
    {
      /* concat subset of addr bits with history pattern (in BHT[0]) */
      addr_mask = 0;
      for (count = btb->address_bits_used; count > 0; count--)
	{
	  addr_mask = ((addr_mask << 1) | 1);
	}
      temp_history = btb->BHT[0];
      pattern = (branch_addr >> btb->index_set_at_bit) & addr_mask;
      pattern = (pattern << btb->history_size) + temp_history;
      count = btb->history[0][pattern];
      return (count >= 2);
    }
  else if (btb->model == BTB_MODEL_STATIC)
    {
      if (branch_sint->oper->flags & BRANCH_PREDICTED_TAKEN)
	return (1);
      else if (branch_sint->oper->flags & BRANCH_PREDICTED_FALLTHRU)
	return (0);
      else
	S_punt ("%s op %i: branch has no static prediction.",
		branch_sint->fn->name, branch_sint->oper->lcode_id);
    }
  else if (btb->model == BTB_MODEL_BTC)
    {
      /* Use prediction for only the branch at the branch_addr */
      if (data->branch_addr == branch_addr)
	return (data->counter);
      else
	return (0);
    }

  else if (btb->model == BTB_MODEL_PREDICATE)
    {
      /* Need to get predicate at time of fetch */

      if (branch_sint->oper->flags & PREDICATED)
	predicate_value =
	  S_get_predicate_regfile_value (pnode, data->pred_predictor);
      else
	{

	  /* Must be a unpredicated jump,return, or jsr */
	  return (1);

	}


#if 0
      if (S_processor_model == PROCESSOR_MODEL_SUPERSCALAR)
	{
	  super = (Superscalar *) pnode->processor_v;
	  def_sint = super->reg_file[data->pred_predictor].last_def_fetched;
	  def_cb_id = super->reg_file[data->pred_predictor].value_cb_id;
	  def_fetch_cycle =
	    super->reg_file[data->pred_predictor].value_fetch_cycle;
	  def_oper = super->reg_file[data->pred_predictor].value_oper;
	}
      else if (S_processor_model == PROCESSOR_MODEL_VLIW)
	{
	  vliw = (VLIW *) pnode->processor_v;
	  def_sint = vliw->reg_file[data->pred_predictor].last_def_fetched;
	  def_cb_id = vliw->reg_file[data->pred_predictor].value_cb_id;
	  def_fetch_cycle =
	    vliw->reg_file[data->pred_predictor].value_fetch_cycle;
	  def_oper = vliw->reg_file[data->pred_predictor].value_oper;
	}
      else
	S_punt ("s_btb.c: processor model %s not supported",
		S_processor_model_name);

#endif

      /* Find the predicate value */
      switch (btb->predicate_prediction_model)
	{

	case BTB_PREDICATE_MODEL_POP:
	  /* Return the predicate value as a prediction */
	  return (predicate_value);
	  break;

	case BTB_PREDICATE_MODEL_PEP:
	  /* Locate counter */

#if 0
	  printf ("branch %d pred %d  counter %d  counter2 %d\n",
		  branch_sint->oper->lcode_id, predicate_value, data->counter,
		  data->counter2);
#endif

	  if (predicate_value == 0)
	    {
	      return (data->counter > 1);
	    }
	  else if (predicate_value == 1)
	    {
	      return (data->counter2 > 1);
	    }
	  else
	    S_punt ("S_get_BTB_block_predicate: predicate value is not 0,1");

	  break;

	case BTB_PREDICATE_MODEL_PEP_HISTORY:
	  /* Access the history pattern */
	  if (predicate_value == 0)
	    {
	      pattern = data->counter;
	    }
	  else if (predicate_value == 1)
	    {
	      pattern = data->counter2;
	    }
	  else
	    S_punt ("S_get_BTB_block_predicate: predicate value is not 0,1");

	  /* Access the history tables */
	  if (btb->number_history_sets == 1)
	    {
	      count = btb->history[0][pattern];
	      return (count >= 2);
	    }
	  else
	    {
	      set = S_get_history_set_number (branch_addr);
	      count = btb->history[set][pattern];
	      return (count >= 2);
	    }
	  break;

	}

    }
  else
    {
      S_punt ("S_get_BTB_block_predict: Unexpectd BTB model");
    }

  /* Should never get here */
  exit (1);
}




int
S_update_BTB_counter (int actually_taken, int current_counter)
{
  /* return the new counter value */

  if (S_pnode->btb->counter_model == BTB_COUNTER_MODEL_A3)
    {
      if (actually_taken)
	{
	  if (current_counter == 0)
	    return 1;
	  else
	    return 3;
	}
      else
	{
	  if (current_counter == 3)
	    return 2;
	  else
	    return 0;
	}

    }
  else if (S_pnode->btb->counter_model == BTB_COUNTER_MODEL_A2)
    {

      if (actually_taken)
	{
	  if (current_counter == 3)
	    return 3;
	  else
	    return (current_counter + 1);
	}
      else
	{
	  if (current_counter == 0)
	    return 0;
	  else
	    return (current_counter - 1);
	}

    }
  else if (S_pnode->btb->counter_model == BTB_COUNTER_MODEL_LT)
    {

      if (actually_taken)
	return 2;
      else
	return 1;
    }

  return 0;
}


void
S_update_BTB_history (Pnode * pnode, Sint * branch_sint, BTB * btb,
		      int actually_taken, int branch_addr)
{
  int temp_history;
  int addr_mask;
  int counter;
  int count;
  int pattern;
  int set_number;
  S_Opc_Info *info;

  if (btb->CBR_only)
    {
      info = &opc_info_tab[branch_sint->oper->opc];

      if (info->opc_type != CBR_OPC)
	{
	  return;
	}
    }

  if ((btb->model == BTB_MODEL_GAG) ||
      (btb->model == BTB_MODEL_GAS) ||
      (btb->model == BTB_MODEL_GAP) ||
      (btb->model == BTB_MODEL_SAG) ||
      (btb->model == BTB_MODEL_SAS) ||
      (btb->model == BTB_MODEL_SAP) ||
      (btb->model == BTB_MODEL_PAG) ||
      (btb->model == BTB_MODEL_PAS) || (btb->model == BTB_MODEL_PAP))
    {
      /* get pattern from appropriate place, and update it */
      if ((btb->model == BTB_MODEL_PAG) ||
	  (btb->model == BTB_MODEL_PAS) || (btb->model == BTB_MODEL_PAP))
	{
	  pattern = branch_sint->oper->BTB_branch_history;
	  branch_sint->oper->BTB_branch_history =
	    S_update_pattern (pattern, actually_taken);
	}
      else
	{
	  pattern = btb->BHT[S_get_BHT_set_number (branch_addr)];
	  btb->BHT[S_get_BHT_set_number (branch_addr)] =
	    S_update_pattern (pattern, actually_taken);
	}
      /* update prediction, indexing with pattern into appropriate table */
      if ((btb->model == BTB_MODEL_GAP) ||
	  (btb->model == BTB_MODEL_SAP) || (btb->model == BTB_MODEL_PAP))
	{
	  counter = branch_sint->oper->BTB_prediction_table[pattern];
	  branch_sint->oper->BTB_prediction_table[pattern] =
	    S_update_BTB_counter (actually_taken, counter);
	}
      else
	{
	  set_number = S_get_history_set_number (branch_addr);
	  counter = btb->history[set_number][pattern];
	  btb->history[set_number][pattern] =
	    S_update_BTB_counter (actually_taken, counter);
	}
    }
  else if (btb->model == BTB_MODEL_GSELECT)
    {
      /* concat subset of addr bits with history pattern (in BHT[0]) */
      addr_mask = 0;
      for (count = btb->address_bits_used; count > 0; count--)
	{
	  addr_mask = ((addr_mask << 1) | 1);
	}
      pattern = (branch_addr >> btb->index_set_at_bit) & addr_mask;
      pattern = (pattern << btb->history_size) + btb->BHT[0];
      /* update history */
      btb->BHT[0] = S_update_pattern (btb->BHT[0], actually_taken);
      /* update prediction */
      btb->history[0][pattern] =
	S_update_BTB_counter (actually_taken, btb->history[0][pattern]);
    }
  else if (btb->model == BTB_MODEL_GSHARE)
    {
      /* xor subset of addr bits with history pattern (in BHT[0]) */
      addr_mask = 0;
      for (count = btb->address_bits_used; count > 0; count--)
	{
	  addr_mask = ((addr_mask << 1) | 1);
	}
      pattern = (branch_addr >> btb->index_set_at_bit) & addr_mask;
      temp_history = btb->BHT[0];
      if (btb->history_size < btb->address_bits_used)
	{
	  temp_history =
	    temp_history << (btb->address_bits_used - btb->history_size);
	}
      pattern = pattern ^ temp_history;
      /* update history */
      btb->BHT[0] = S_update_pattern (btb->BHT[0], actually_taken);
      /* update prediction */
      btb->history[0][pattern] =
	S_update_BTB_counter (actually_taken, btb->history[0][pattern]);
    }
}

void
S_update_BTB_block (Pnode * pnode, Sint * branch_sint, BTB * btb,
		    Scblock * block, int actually_taken, int target,
		    int branch_addr)
{
  BTB_data *data;
  int pattern = 0;
  int set_number;
  int counter;
  int predicate_value;
  /* 10/22/04 REK Commenting out unused variable to quiet compiler warning. */
#if 0
  int count;
#endif

  data = (BTB_data *) block->data;

  if (actually_taken)
    {
      data->target = target;
    }

  if (btb->model == BTB_MODEL_COUNTER)
    {

      data->counter = S_update_BTB_counter (actually_taken, data->counter);

      S_cache_make_MRU (btb->cache, block);
    }

  else if (btb->model == BTB_MODEL_TWO_LEVEL)
    {
      if (data->entry_type != CBR_OPC)
	{
	  /* Handle unconditional case by simply updating cache */
	  /* block tracking of MRU, using a combines BTB, where */
	  /* only the conditional BTB entries need to update counters */

	  if (!(branch_sint->oper->flags & PREDICATED))
	    {
	      S_cache_make_MRU (btb->cache, block);
	      return;
	    }

	}

      /* history actually holds the 2-bit counter, and the counter */
      /* data structure holds the history pattern */
      pattern = data->counter;
      data->counter = S_update_pattern (data->counter, actually_taken);

      set_number = S_get_history_set_number (branch_addr);

      counter = btb->history[set_number][pattern];
      btb->history[set_number][pattern] =
	S_update_BTB_counter (actually_taken, counter);

      S_cache_make_MRU (btb->cache, block);
    }
  else if ((btb->model == BTB_MODEL_GAG) ||
	   (btb->model == BTB_MODEL_GAS) ||
	   (btb->model == BTB_MODEL_GAP) ||
	   (btb->model == BTB_MODEL_SAG) ||
	   (btb->model == BTB_MODEL_SAS) ||
	   (btb->model == BTB_MODEL_SAP) ||
	   (btb->model == BTB_MODEL_PAG) ||
	   (btb->model == BTB_MODEL_PAS) || (btb->model == BTB_MODEL_PAP))
    {
      S_cache_make_MRU (btb->cache, block);
    }
  else if (btb->model == BTB_MODEL_GSELECT)
    {
      S_cache_make_MRU (btb->cache, block);
    }
  else if (btb->model == BTB_MODEL_GSHARE)
    {
      S_cache_make_MRU (btb->cache, block);
    }
  else if (btb->model == BTB_MODEL_PREDICATE)
    {


      /* Always update the btb register index, in case of aliasing */

      S_cache_make_MRU (btb->cache, block);

      if (branch_sint->oper->flags & PREDICATED)
	{
	  /* Get the predicate value */
	  predicate_value =
	    S_get_predicate_regfile_value (pnode, data->pred_predictor);

	  data->pred_predictor =
	    S_get_branch_predicate_prediction (branch_sint);
	}
      else
	return;

      switch (btb->predicate_prediction_model)
	{

	case BTB_PREDICATE_MODEL_PEP:

	  if (predicate_value == 0)
	    {
	      data->counter =
		S_update_BTB_counter (actually_taken, data->counter);
	    }
	  else if (predicate_value == 1)
	    {
	      data->counter2 =
		S_update_BTB_counter (actually_taken, data->counter2);
	    }
	  else
	    S_punt ("S_update_BTB_block: illegal predicate value");

	  break;

	case BTB_PREDICATE_MODEL_PEP_HISTORY:

	  /* History actually holds the 2-bit counter, and the counter */
	  /* data structure holds the history pattern */

	  if (predicate_value == 0)
	    {
	      pattern = data->counter;
	      data->counter =
		S_update_pattern (data->counter, actually_taken);
	    }
	  else if (predicate_value == 1)
	    {
	      pattern = data->counter2;
	      data->counter2 =
		S_update_pattern (data->counter2, actually_taken);
	    }
	  else
	    S_punt ("S_update_BTB_block: illegal predicate value");

	  set_number = 0;
	  if (btb->number_history_sets != 1)
	    {
	      set_number = S_get_history_set_number (branch_addr);
	    }

	  counter = btb->history[set_number][pattern];
	  btb->history[set_number][pattern] =
	    S_update_BTB_counter (actually_taken, counter);

	  break;

	}

    }
  else if (btb->model == BTB_MODEL_STATIC)
    {
      /* Make MRU only for predicted taken branches */
      if (branch_sint->flags & BRANCH_PREDICTED_TAKEN)
	S_cache_make_MRU (btb->cache, block);
    }
  else if (btb->model == BTB_MODEL_BTC)
    {
      /* taken branch, make this where the cache block goes */
      if (actually_taken)
	{
	  data->counter = 1;
	  data->branch_addr = branch_addr;
	}
      /* otherwise, only update counter if prediction is for this branch */
      else
	{
	  if (data->branch_addr == branch_addr)
	    data->counter = 0;
	}
      S_cache_make_MRU (btb->cache, block);
    }
  else
    {
      S_punt ("S_update_BTB_block: Unexpectd BTB model");
    }
}



void
S_new_btb_data (BTB * btb, Scblock * block, int dir, int target,
		int branch_addr, int control_opc, Sint * branch_sint)
{
  BTB_data *data;
  int pattern;

  data = (BTB_data *) block->data;

  data->target = target;

  if (btb->model == BTB_MODEL_COUNTER)
    {
      if (dir)
	data->counter = 3;
      else
	data->counter = 0;
    }
  else if (btb->model == BTB_MODEL_TWO_LEVEL)
    {

      data->entry_type = control_opc;

      if (dir)
	pattern = S_update_pattern (btb->default_taken_history_pattern, dir);
      else
	pattern =
	  S_update_pattern (btb->default_not_taken_history_pattern, dir);
      data->counter = pattern;
    }
  /* Don't change conter for two-level variations - since multiple branches will */
  /* access the same counter (under certain schemes) the counter cannot be */
  /* initialized when adding a new branch. */
  else if (btb->model == BTB_MODEL_GAG)
    {
      data->entry_type = control_opc;
    }
  else if (btb->model == BTB_MODEL_GAS)
    {
      data->entry_type = control_opc;
    }
  else if (btb->model == BTB_MODEL_GAP)
    {
      data->entry_type = control_opc;
    }
  else if (btb->model == BTB_MODEL_SAG)
    {
      data->entry_type = control_opc;
    }
  else if (btb->model == BTB_MODEL_SAS)
    {
      data->entry_type = control_opc;
    }
  else if (btb->model == BTB_MODEL_SAP)
    {
      data->entry_type = control_opc;
    }
  else if (btb->model == BTB_MODEL_PAG)
    {
      data->entry_type = control_opc;
    }
  else if (btb->model == BTB_MODEL_PAS)
    {
      data->entry_type = control_opc;
    }
  else if (btb->model == BTB_MODEL_PAP)
    {
      data->entry_type = control_opc;
    }
  else if (btb->model == BTB_MODEL_GSHARE)
    {
      data->entry_type = control_opc;
    }
  else if (btb->model == BTB_MODEL_GSELECT)
    {
      data->entry_type = control_opc;
    }
  else if (btb->model == BTB_MODEL_BTC)
    {
      if (dir)
	data->counter = 1;
      else
	data->counter = 0;
      data->branch_addr = branch_addr;
    }
  else if (btb->model == BTB_MODEL_STATIC)
    {
      data->counter = 1;	/* Really not used */
    }
  else if (btb->model == BTB_MODEL_PREDICATE)
    {

      /* assign the predicate prediction register for all predicated branches */
      if ((control_opc == CBR_OPC) || (branch_sint->oper->flags & PREDICATED))
	data->pred_predictor =
	  S_get_branch_predicate_prediction (branch_sint);

      switch (btb->predicate_prediction_model)
	{
	case BTB_PREDICATE_MODEL_PEP:
	  data->counter = 1;
	  data->counter2 = 2;
	  break;
	case BTB_PREDICATE_MODEL_PEP_HISTORY:
	  data->counter = btb->pure_not_taken_history_pattern;
	  data->counter2 = btb->pure_taken_history_pattern;
	  break;
	}

    }
  else
    S_punt ("S_new_btb_data:  Unexpectd BTB model");
}



static int
S_is_already_used_block (block)
     Scblock *block;
{
  return (block->hash_next != (Scblock *) - 1);
}



static int
S_update_pattern (pattern, dir)
     int pattern, dir;
{
  int new_pattern;

  new_pattern = ((pattern << 1) + dir) & S_pnode->btb->max_history_pattern;

  return (new_pattern);
}
