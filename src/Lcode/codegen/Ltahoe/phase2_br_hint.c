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
 *  File:  phase2_br_hint.c
 *
 *  Description:
 *    Inserts branch and prefetch hints on br and mov-to-br instructions.
 *    Inserts brp instructions for IP-relative branches.
 *
 *  Authors:  Jim Pierce, Mark Tozer
 *  Modified: Mark Tozer - 06/25/97 - rewrote to do pf and br hints
 *            Kevin Crozier - 06/17/98 - added bundle counting for prefetch
 *
\**************************************************************************/
/* 09/12/02 REK Updating file to support the new opcode map. */

#undef HINT_DEBUG2		/* some debug details */
#undef HINT_DEBUG		/* all the details you can shake a stick at */

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "ltahoe_main.h"
#include "ltahoe_op_query.h"
#include "ltahoe_completers.h"
#include <library/dynamic_symbol.h>
#include "phase1_func.h"
#include "phase2_icache.h"
#include "phase2_br_hint.h"

/* how many cb's to go up to find a place 
   to insert the hint (-1N = infinite) */
#define RECURSION_LEVEL -1

#define TAR_WEIGHT_THRESHOLD 1000 /* min weight for a tar on a predicated br */
#define TAC_WEIGHT_THRESHOLD 100  /* min weight for a tac on a predicated br */
#define MIN_HINTED_PATH_WEIGHT 10 /* min weight to apply any hints */

/* use a tar hint above this prob */
#define TAR_PROB_THRESHOLD 0.9
/* use a tac hint above this prob */
#define TAC_PROB_THRESHOLD 0.5

/* Max distance (in bundles) between hint and branch */
/* 2^8 (bit 9 is a sign bit) */
/* include some headroom for expansion... */
#define MAX_BUNDLES_BETWEEN_HINT_AND_BR 250
#define MAX_INSERTION_DISTANCE 240

#define MAX_EXPANDS_PER_CB 1
#define MAX_INSERTS_PER_CB 3
#define MAX_TARS_ON_PATH 4

#define BRP_MANY_HINT_MIN_SIZE  4
#define BRP_CPREFETCH_MIN_SIZE  8
#define BRANCH_BUNDLE_COUNT_BIAS 0.80
#define PREFETCH_FE_CYCLES 12

#define PF_MANY_HINT_MIN_SIZE 24

/**********************************************/
/*     STRUCTURES                             */
/**********************************************/

typedef struct _cb_info
{
  int bundles_in_cb;
  int num_expanded;
  int num_inserted;
}
BH_CB_Info;

typedef struct _bh_hint_path
{
  struct _bh_hint_path *next_cb;
  L_Cb *cb;
}
BH_hint_path;

/* If you modify this struct, you must modify BH_copy_stats function */
typedef struct _bh_path_info
{
  /* dynamic path info */
  BH_hint_path *path_tail;
  BH_hint_path *path;

  int num_br_crossed;
  int num_calls_crossed;
  int num_cbs_on_path;
  int fe_cycles_away;
  int brps_on_path;
  int tars_on_path;

  int expanded_bundle;

  /* static path info */
  int direction;		/* is the brp first statically */
  int static_distance;


}
BH_path_info;

typedef struct _bh_hints_list
{
  struct _bh_hints_list *next_hint;

  L_Oper *hint_oper;
  L_Cb *hint_cb;
  BH_path_info path_info;

}
BH_hints_list;


typedef struct _bh_br_hint
{
  struct _bh_br_hint *next_br;
  struct _bh_br_hint *prev_br;

  int br_type;			/* type of branch */
  int ipwh;			/* whether hint (either ipwh or mwh) */
  int ph;			/* sequential prefetch hint */
  int pvec;			/* prefetch vector hint */
  int ih;			/* importance hint */
  int spec_ih;			/* importance hint used if too many tars on path */
  L_Oper *br_oper;		/* branch instr oper */
  L_Cb *br_cb;			/* cb containing branch instr */
  float prob;			/* Probability of branch */
  float path_weight;		/* Importance of hint - execution counts */

  L_Operand *target;		/* Target of branch - will be Cb type */
  L_Oper *br_bundle;		/* template oper preceding branch */
  L_Operand *label;		/* label to be inserted before branch bundle */
  L_Operand *num_bundles;	/* Number of bundles for counted prefetch */

  struct _bh_br_hint *advanced_hint;
  BH_hints_list *list;
  BH_hints_list *list_tail;
  int tar_hinted;		/* any tar hints inserted for the branch? */
  int num_inserted;

  int paths_success;
  int paths_tried;
  int cross_same_call;

}
BH_Br_hint;


#define L_is_br_indirect(oper) ((oper->src[0]->ctype == L_CTYPE_BTR))

/*****************************************************************/
/*   Global variables                                            */
/*****************************************************************/

static BH_Br_hint **sorted_hints;
static BH_Br_hint *list_br_hints = 0;
static BH_Br_hint *existing_br_hints = 0;
static int br_hint_cnt = 0;
static int advanced_br_hint_cnt = 0;
static int existing_br_hint_cnt = 0;
static Set cbs_looked_at_set = NULL;

/* statistics */
static int num_branches = 0;
static int hightakebranches = 0;
static int lowweighttossed = 0;
static int calls = 0;
static int conds = 0;
static int returns = 0;
static int total_attempted = 0;
static int total_inserted = 0;
static int stat_max_recursion = 0;
static int stat_cb_too_far = 0;
static int stat_merged_path = 0;
static int stat_tac_hints = 0;
static int stat_tar_hints = 0;
static int stat_max_expand = 0;
static int stat_max_inserted = 0;

/*****************************************/
/* 0   can't do anything without expanding */
/* 1   miI -> miB */
/* 2   mXi -> miB, move 3rd to 2nd */
/* 3   mmI -> mmB */
/* 4   */
/* 5   mMI -> MIB, move 2,3 to 1,2 */
/* 6   mfI -> mfB */
/* 8   no change needed */
/* 9   do nothing now */
/* 20  illegal template */
/*****************************************/

static int hint_fit_table[][8] = {
  /*  density:        0   1   2   3   4   5   6   7  */
  /*  ---------------------------------------------- */
  /* MII        */ {1, 2, 1, 0, 1, 2, 1, 0},
  /* MI|I       */ {0, 0, 0, 0, 0, 0, 0, 0},
  /* MLX        */ {0, 0, 0, 0, 0, 0, 0, 0},
  /* RES        */ {20, 20, 20, 20, 20, 20, 20, 20},
  /* MMI        */ {3, 2, 3, 5, 3, 2, 3, 0},
  /* M|MI       */ {0, 0, 0, 0, 0, 0, 0, 0},
  /* MFI        */ {6, 2, 6, 0, 6, 2, 6, 0},
  /* RES        */ {20, 20, 20, 20, 20, 20, 20, 20},
  /* MIB        */ {8, 9, 8, 0, 8, 9, 8, 0},
  /* MBB        */ {1, 9, 8, 9, 8, 9, 8, 0},
  /* RES        */ {20, 20, 20, 20, 20, 20, 20, 20},
  /* BBB        */ {8, 9, 8, 9, 8, 9, 8, 0},
  /* MMB        */ {8, 9, 8, 9, 8, 9, 8, 0},
  /* RES        */ {20, 20, 20, 20, 20, 20, 20, 20},
  /* MFB        */ {8, 9, 8, 0, 8, 9, 8, 0},
  /* RES        */ {20, 20, 20, 20, 20, 20, 20, 20}
};

static int pvec_table[3][3] = {
  /*  second br:  DC           NT           TK         */
  /* DC */ {PVEC_DC_DC, PVEC_DC_NT, PVEC_DC_DC},
  /* NT */ {PVEC_NT_DC, PVEC_NT_NT, PVEC_NT_TK},
  /* TK */ {PVEC_TK_DC, PVEC_TK_NT, PVEC_TK_TK}
};

/*****************************************************************/
/* internal prototypes */

static void BH_process_branches (L_Func * fn);
static BH_Br_hint *BH_find_hint_instr (L_Oper * oper, L_Oper * label_oper);
static void BH_add_to_existing_hint_list (L_Cb * cb, L_Oper * oper);
static void BH_new_br_hint (char *func_name, L_Cb * cb, L_Oper * br_oper,
			    int num_bundles, int br_type, int ipwh, int ph,
			    int pvec, int ih, float prob, float path_weight);
static void BH_delete_list_br_hints (BH_Br_hint * hint_list);
static BH_Br_hint **BH_prioritize_branch_hints ();
int BH_hint_compare (BH_Br_hint ** hint1, BH_Br_hint ** hint2);
static char *BH_hint_str (BH_Br_hint * br_hint);
static void BH_print_hint_suggestions (BH_Br_hint ** hint_array, char *title);

static void free_CB_info (void *ptr);
static int BH_cb_info (int operation, L_Cb * cb1, L_Cb * cb2);
static int BH_static_distance (L_Cb * hint_cb, L_Oper * hint_oper,
			       L_Cb * br_cb, L_Oper * br_oper,
			       int *direction);
static int BH_insert_hint_in_paths (L_Cb * cb, L_Oper * oper,
				    BH_Br_hint * br_hint,
				    BH_path_info * path_stats,
				    int min_fe_cycles, int max_recursion,
				    int indent, int test);
static int BH_insert_hint_in_block (L_Cb * cb, L_Oper * oper,
				    BH_Br_hint * br_hint,
				    BH_path_info * path_stats,
				    int cycles_to_go, int indent, int test);
static int BH_insert_at_best_location (L_Cb * cb, L_Oper * top_tmpl,
				       L_Oper * bottom_tmpl,
				       BH_Br_hint * br_hint,
				       BH_path_info * path_stats, int test);
static int BH_test_insert_hint_in_bundle (L_Cb * cb, L_Oper * bundle,
					  BH_Br_hint * br_hint,
					  BH_path_info * path_stats);
static int BH_insert_hint_in_bundle (L_Cb * cb, L_Oper * bundle,
				     BH_Br_hint * br_hint,
				     BH_path_info * path_stats);
static void BH_insert_branch_label (BH_Br_hint * br_hint);
static int BH_bundles_from_top (L_Oper * oper);
static void BH_add_mov2br_instr_hints (L_Oper * oper, int mwh, int ph,
				       int pvec, int ih);
static void BH_add_br_instr_hints (L_Oper * oper, int bwh, int ph, int dh,
				   float prob, float path_weight);
static void BH_add_cb_densities (L_Cb * cb);
static void BH_add_densities (L_Func * fn);
static void BH_fix_flow_ccs (L_Func * fn);
static void BH_add_instr_hints_only (L_Func * fn);
static void BH_specify_hint_on_mov2br (L_Cb * cb, L_Oper * oper);
static void BH_specify_hint_on_br (L_Cb * cb, L_Oper * oper,
				   L_Flow ** flow, double *path_weight);
static void BH_suggest_hint_instr (char *fn_name, L_Cb * cb, L_Oper * oper,
				   L_Flow * flow, double path_weight);
static void BH_insert_branch_hints (L_Func * fn, BH_Br_hint * hints[]);
static void BH_insert_advanced_hint (BH_Br_hint * hint);
static void BH_print_stats (BH_Br_hint ** hints);
static void BH_copy_path_info (BH_path_info * dest, BH_path_info * src);
static void BH_add_to_hint_list (BH_Br_hint * br_hint, BH_path_info * stats,
				 L_Oper * hint_oper, L_Cb * hint_cb);
static void BH_add_cb_to_path (L_Cb * cb, BH_path_info * path_info);
static int BH_ok_to_expand (L_Cb * cb, L_Oper * oper, BH_Br_hint * br_hint);
static int BH_ok_to_insert (L_Cb * cb, BH_Br_hint * br_hint);
static void BH_insert_update (L_Cb * cb, L_Oper * oper, BH_Br_hint * br_hint,
			      int expanded);
static int BH_is_on_static_path (L_Cb * cb, L_Oper * oper,
				 L_Cb * hint_cb, L_Oper * hint_oper,
				 L_Cb * br_cb, L_Oper * br_oper);
static int BH_is_on_dynamic_path (L_Cb * cb, BH_hints_list * hint_list);

#ifdef HINT_DEBUG
static void BH_check_all_distances ();
#endif

#ifdef HINT_DEBUG2
static void BH_print_hint (int type, int cb_id, BH_Br_hint * br_hint);
static void BH_print_br_stats (BH_Br_hint * br_hint);
#endif

#if 0
static int BH_available_slot (L_Oper * oper);
#endif

/*****************************************************************/

/****************************************************************************
 *
 * routine: O_count_bundles_before_branch_func
 * purpose:   counts the number of bundles up to a highly biased branch
 * input:  fn - function to count bundles for
 * output: 
 * returns:
 * modified:
 *         
 * note:
 *-------------------------------------------------------------------------*/

#define DATAFILE "datafile"
void
O_count_bundles_before_branch_func (L_Func * fn)
{
  int num_bundles;
  L_Attr *attr;
  FILE *data;

  num_bundles =
    O_count_bundles_before_branch (fn->first_cb, fn->first_cb->first_op);

  attr = L_new_attr ("bundles", 1);
  L_set_int_attr_field (attr, 0, num_bundles);
  fn->attr = L_concat_attr (fn->attr, attr);

  data = fopen (DATAFILE, "a");
  fprintf (data, "%s %d\n", fn->name, num_bundles);
  fclose (data);
}

/****************************************************************************
 *
 * routine: O_count_bundles_before_branch
 * purpose:   counts the number of bundles up to a highly biased branch
 * input:  starting_cb - cb to start counting bundles from
 * output: 
 * returns:
 * modified:
 *         
 * note:
 *-------------------------------------------------------------------------*/

int
O_count_bundles_before_branch (L_Cb * starting_cb, L_Oper * starting_oper)
{
  int num_bundles = 0;
  L_Cb *cb, *next_cb;
  L_Oper *op, *next_op;
  L_Flow *flow;

  op = starting_oper;
  for (cb = starting_cb; cb != NULL; cb = next_cb)
    {
      next_cb = cb->next_cb;
      /* if op comes in NULL, assume the first op in the cb */
      op = op ? op : cb->first_op;
      while (op)
	{
	  next_op = op->next_op;
	  if (LT_is_cond_br (op))
	    {
	      flow = L_find_flow_for_branch (cb, op);
	      if ((flow->weight / cb->weight) > BRANCH_BUNDLE_COUNT_BIAS)
		{
		  next_op = NULL;
		  next_cb = NULL;
		}
	    }
	  else if (LT_is_call_br (op) || LT_is_ret_br (op))
	    {
	      next_op = NULL;
	      next_cb = NULL;
	    }
	  else if (LT_is_template_op (op))
	    {
	      num_bundles++;
	    }
	  op = next_op;
	}
    }

  return num_bundles;
}

/****************************************************************************
 *
 * routine: BH_num_bundles_in_call  
 * purpose:   counts the number of bundles up to a highly biased branch
 * input:  br_call - call to function that is currently being hinted
 * output: 
 * returns: int - number of bundles before major branch in called fn
 * modified:
 *         
 * note: Uses a file that is output at the end phase 2 in Ltahoe
 *-------------------------------------------------------------------------*/

int
BH_num_bundles_in_call (L_Oper * br_call)
{
  char *fn_name;
  char current[1024];
  char curr_line[1024], *ptr;
  int num_bundles;

  FILE *data;

  /* For register indirect branches (e.g. function ptrs),
     jump label is undefined, so don't look for it */
  if (br_call->src[0]->type == L_OPERAND_REGISTER)
    return 4;

  current[0] = '\0';

  fn_name = br_call->src[0]->value.l + 4;	/* lose the _$fn part */

  data = fopen (DATAFILE, "r");
  while ((strcmp (fn_name, current) != 0) && (data != NULL))
    {
      ptr = fgets (curr_line, 1024, data);
      if (ptr == NULL)
	{
	  num_bundles = 0;
	  break;
	}
      sscanf (curr_line, "%s %d", current, &num_bundles);
    }

  if (!data)
    {
#if 0
      L_warn ("BH_num_bundles_in_call: Function %s not in datafile\n",
	      fn_name);
#endif
      num_bundles = 4;
    }
  return num_bundles;
}

/****************************************************************************
 *
 * routine: BH_num_bundles_in_branch
 * purpose:   counts the number of bundles up to a highly biased branch
 * input:  br_cond - branch operation to be hinted
 * output: 
 * returns: int - number of bundles before major branch in called fn
 * modified:
 *         
 * note: 
 *-------------------------------------------------------------------------*/

int
BH_num_bundles_in_branch (L_Oper * br_cond)
{
  return (O_count_bundles_before_branch (br_cond->src[0]->value.cb,
					 br_cond->src[0]->value.cb->
					 first_op));
}


/****************************************************************************
 *
 * routine: O_insert_branch_hints
 * purpose:   insert brp instructions and hints on br and mov2br instrs
 * input:  fn - function to add hints to
 * output: 
 * returns:
 * modified:
 *         
 * note:
 *-------------------------------------------------------------------------*/

void
O_insert_branch_hints (L_Func * fn)
{
  /* initialize */
  BH_add_densities (fn);
  BH_fix_flow_ccs (fn);
  BH_cb_info (INITIALIZE, fn->first_cb, NULL);
  L_do_flow_analysis (fn, DOMINATOR);	/* for finding backedges */

  /* identify */
  BH_process_branches (fn);

#ifdef HINT_DEBUG
  if (Ltahoe_print_hint_info)
    BH_print_hint_suggestions (NULL, "Unsorted Hint Suggestions");
#endif

  /* prioritize */
  sorted_hints = BH_prioritize_branch_hints ();

  if (sorted_hints)
    {
      if (Ltahoe_print_hint_info)
	BH_print_hint_suggestions (sorted_hints, "Sorted Hint Suggestions");

      /* insert */
      BH_insert_branch_hints (fn, sorted_hints);

#ifdef HINT_DEBUG
      DB_spit_func (fn, "hinted_func");
#endif

      if (Ltahoe_print_hint_info)
	BH_print_stats (sorted_hints);

      /* de-initialize */
      free (sorted_hints);
    }
  BH_cb_info (CLEANUP, NULL, NULL);
  BH_delete_list_br_hints (list_br_hints);
  BH_delete_list_br_hints (existing_br_hints);
}


/****************************************************************************
 *
 * routine: O_insert_br_instr_hints_only
 * purpose: insert hints only on br and mov2br instructions
 * input: fn - function to add hints to
 * output: 
 * returns:
 * modified:
 *         
 * note:
 *-------------------------------------------------------------------------*/

void
O_insert_br_instr_hints_only (L_Func * fn)
{
  BH_fix_flow_ccs (fn);
  BH_cb_info (INITIALIZE, fn->first_cb, NULL);
  L_do_flow_analysis (fn, DOMINATOR);	/* for finding backedges */

  BH_add_instr_hints_only (fn);

  BH_cb_info (CLEANUP, NULL, NULL);
}



/*****************************************************************/


/****************************************************************************
 *
 * routine: BH_process_branches()
 * purpose:  Find branches and add hints to/for them
 * input: fn - function to process
 * output: 
 * returns:
 * modified: 
 *
 * note:
 *-------------------------------------------------------------------------*/

static void
BH_process_branches (L_Func * fn)
{
  L_Flow *flow, *br_flow;
  L_Cb *cb;
  L_Oper *oper, *template_oper, *label_oper = NULL;
  double path_weight;
  int label_valid = 0;
  int slot = 0;

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      int bbb_context = 0;

      path_weight = cb->weight;
      flow = cb->dest_flow;
      label_valid = 0;

      template_oper = NULL;

      for (oper = cb->first_op; oper; oper = oper->next_op)
	{
	  if (LT_is_label_op (oper))
	    {
	      label_oper = oper;
	      label_valid = 2;
	      continue;
	    }
	  else if (LT_is_template_op (oper))
	    {
	      template_oper = oper;
	      slot = 0;
	      if (label_valid > 0) /* invalidate label after current bundle */
		label_valid--;

	      bbb_context = (LT_get_template (oper) == BBB);
	      continue;
	    }
	  else if (LT_is_brp (oper))
	    {
	      BH_add_to_existing_hint_list (cb, oper);
	    }
	  else if (LT_is_mov_to_br (oper))
	    {
	      BH_specify_hint_on_mov2br (cb, oper);
	    }
	  else if (LT_is_call_br (oper))
	    {
	      num_branches++;
	      calls++;
	      if ((slot == 2) &&
		  (!label_valid || !BH_find_hint_instr (oper, label_oper)))
		BH_suggest_hint_instr (fn->name, cb, oper, flow, path_weight);

	      BH_specify_hint_on_br (cb, oper, &flow, &path_weight);
	    }
	  else if (LT_is_indir_br (oper))
	    {
	      TC_SET_BR_WTHR (oper->completers, TC_BR_WTHR_DPTK);
	      if (oper->next_op)
		L_warn ("BH_process_branches: op follows an indir br");
	      break;
	    }
	  else if (LT_is_cond_br (oper))
	    {
	      num_branches++;
	      conds++;

	      br_flow = L_find_flow_for_branch (cb, oper);
	      if ((!Ltahoe_mckinley_hints && (slot == 2)) ||
		  (Ltahoe_mckinley_hints &&
		   ((br_flow->weight / cb->weight) >
		    BRANCH_BUNDLE_COUNT_BIAS)))
		{
		  hightakebranches++;
		  if (((oper->src[0]->value.cb == cb) ||
		       (L_in_cb_DOM_set (cb, oper->src[0]->value.cb->id))) ||
		      (Ltahoe_aggressive_hints &&
		       (!label_valid ||
			!BH_find_hint_instr (oper, label_oper))))
		    BH_suggest_hint_instr (fn->name, cb, oper, flow,
					   path_weight);
		}

	      BH_specify_hint_on_br (cb, oper, &flow, &path_weight);

	      if (bbb_context)
		{
		  int bwh;

		  bwh = TC_GET_BR_WTHR (oper->completers);

		  switch (bwh)
		    {
		    case TC_BR_WTHR_SPTK:
		      TC_SET_BR_WTHR (oper->completers, TC_BR_WTHR_DPTK);
		      break;
		    case TC_BR_WTHR_SPNT:
		      TC_SET_BR_WTHR (oper->completers, TC_BR_WTHR_DPNT);
		      break;
		    default:
		      break;
		    }
		}
	    }
	  else if (LT_is_ret_br (oper))
	    {
	      num_branches++;
	      returns++;
	      BH_specify_hint_on_br (cb, oper, &flow, &path_weight);
	    }

	  slot++;
	}
    }
  return;
}


/****************************************************************
 *
 * routine: BH_find_hint_instr()
 * purpose: find an existing hint instruction in the list
 * input:  oper - br_oper looking for
 *         label_oper - label for this branch bundle
 * output: 
 * returns: Pointer to existing br_hint struct (0 if not found)
 * modified:
 *
 * note:
 *----------------------------------------------------------------*/

static BH_Br_hint *
BH_find_hint_instr (L_Oper * oper, L_Oper * label_oper)
{
  BH_Br_hint *br_list;

  br_list = existing_br_hints;

  while (br_list != NULL)
    {
      if ((!strcmp (br_list->label->value.l, label_oper->src[0]->value.l)) ||
	  (br_list->br_oper == oper))
	return (br_list);

      br_list = br_list->next_br;
    }

  return NULL;
}


/****************************************************************************
 *
 * routine: BH_add_to_existing_hint_list
 * purpose: Add to the existing hint list
 * input: cb - cb with the br hint
 *        oper - br hint oper
 * output: 
 * returns:
 * modified:
 *         
 * note:  Used the full BH_Br_hint structure in case we want to do
 *        something with these hints/brs in the future.
 *-------------------------------------------------------------------------*/

static void
BH_add_to_existing_hint_list (L_Cb * cb, L_Oper * oper)
{
  BH_Br_hint *br_hint, *br_list;

  existing_br_hint_cnt++;

  br_hint = (BH_Br_hint *) calloc (1, sizeof (BH_Br_hint));

  br_hint->label =
    L_new_label_operand (oper->src[0]->value.l, L_CTYPE_GLOBAL_ABS);

  br_list = existing_br_hints;

  br_hint->next_br = NULL;

  if (br_list == NULL)
    {
      br_hint->prev_br = NULL;
      br_list = br_hint;
      existing_br_hints = br_hint;
    }
  else
    {
      while (br_list->next_br != NULL)
	br_list = br_list->next_br;
      br_list->next_br = br_hint;
      br_hint->prev_br = br_list;
    }

}

/****************************************************************************
 *
 * routine: BH_new_advanced_hint
 * purpose: Setup a br_hint structure for later insertion
 * input: func_name - function name
 *        cb - cb of the branch
 *        br_oper - branch oper
 *        br_type - branch type (cond/call/ret)
 *        ipwh - whether hint
 *        ph - prefetch hint
 *        pvec - prefetch cancellation vector
 *        ih - importance hint
 *        prob - br taken probability
 *        path_weight - br taken edge weight
 * output: 
 * returns:
 * modified:
 *         
 * note:
 *-------------------------------------------------------------------------*/

static BH_Br_hint *
BH_new_advanced_hint (char *func_name, L_Cb * cb, L_Oper * br_oper,
		      int num_bundles, int br_type, int ipwh, int ph,
		      int pvec, int ih, float prob, float path_weight)
{

  BH_Br_hint *br_hint;
  char *label;

  if (L_is_br_indirect (br_oper))
    {
#ifdef HINT_DEBUG
      fprintf (stderr, "Indirect branch %d not hinted\n", br_oper->id);
#endif
      return NULL;
    }

  advanced_br_hint_cnt++;

  label = (char *) calloc (50, sizeof (char));
  strcpy (label, func_name);
  strcat (label, "adv_hint_");
  sprintf (label + strlen (label), "%d", advanced_br_hint_cnt);


  br_hint = (BH_Br_hint *) calloc (1, sizeof (BH_Br_hint));
  br_hint->br_type = br_type;
  br_hint->ipwh = ipwh;
  br_hint->ph = ph;
  br_hint->pvec = pvec;
  br_hint->ih = ih;
  br_hint->spec_ih = ih;
  br_hint->br_oper = br_oper;
  br_hint->br_cb = cb;
  br_hint->prob = prob;
  br_hint->path_weight = path_weight;

  br_hint->target = L_copy_operand (br_oper->src[0]);
  br_hint->br_bundle = NULL;
  br_hint->label = L_new_label_operand (label, L_CTYPE_GLOBAL_ABS);
  if (Ltahoe_use_counted_prefetch_hints)
    br_hint->num_bundles = L_new_int_operand (num_bundles, L_CTYPE_INT);

  br_hint->tar_hinted = 0;
  br_hint->num_inserted = 0;

  br_hint->paths_success = 0;
  br_hint->paths_tried = 0;
  br_hint->cross_same_call = 0;

  return br_hint;
}

/****************************************************************************
 *
 * routine: BH_new_br_hint
 * purpose: Setup a br_hint structure for later insertion
 * input: func_name - function name
 *        cb - cb of the branch
 *        br_oper - branch oper
 *        br_type - branch type (cond/call/ret)
 *        ipwh - whether hint
 *        ph - prefetch hint
 *        pvec - prefetch cancellation vector
 *        ih - importance hint
 *        prob - br taken probability
 *        path_weight - br taken edge weight
 * output: 
 * returns:
 * modified:
 *         
 * note:
 *-------------------------------------------------------------------------*/

static void
BH_new_br_hint (char *func_name, L_Cb * cb, L_Oper * br_oper,
		int num_bundles, int br_type, int ipwh, int ph,
		int pvec, int ih, float prob, float path_weight)
{
  BH_Br_hint *br_hint, *br_list;
  char *label;

  if (L_is_br_indirect (br_oper))
    {
#ifdef HINT_DEBUG
      fprintf (stderr, "Indirect branch %d not hinted\n", br_oper->id);
#endif
      return;
    }

  br_hint_cnt++;

  label = (char *) calloc (50, sizeof (char));
  strcpy (label, func_name);
  strcat (label, "_hint_");
  sprintf (label + strlen (label), "%d", br_hint_cnt);


  br_hint = (BH_Br_hint *) calloc (1, sizeof (BH_Br_hint));
  br_hint->br_type = br_type;
  br_hint->ipwh = ipwh;
  br_hint->ph = ph;
  br_hint->pvec = pvec;
  br_hint->ih = ih;
  br_hint->spec_ih = ih;
  br_hint->br_oper = br_oper;
  br_hint->br_cb = cb;
  br_hint->prob = prob;
  br_hint->path_weight = path_weight;

  br_hint->target = L_copy_operand (br_oper->src[0]);
  br_hint->br_bundle = NULL;
  br_hint->label = L_new_label_operand (label, L_CTYPE_GLOBAL_ABS);
  if (Ltahoe_use_counted_prefetch_hints)
    br_hint->num_bundles = L_new_int_operand (num_bundles, L_CTYPE_INT);

  br_hint->tar_hinted = 0;
  br_hint->num_inserted = 0;

  br_hint->paths_success = 0;
  br_hint->paths_tried = 0;
  br_hint->cross_same_call = 0;

  if (Ltahoe_mckinley_hints && Ltahoe_advanced_prefetch &&
      !LT_is_call_br (br_oper) && !LT_is_ret_br (br_oper) && (prob < 1.0))
    {
#ifdef HINT_DEBUG
      fprintf (stderr, "Making an advanced hint for op %d\n", br_oper->id);
#endif
      num_bundles = O_count_bundles_before_branch (cb, br_oper->next_op);
      br_hint->advanced_hint = BH_new_advanced_hint (func_name, cb, br_oper,
						     num_bundles, BR_ADV,
						     IPWH_EXIT, PH_MANY,
						     PVEC_DC_DC, IH_NONE, 1.0,
						     path_weight);
    }
  else
    {
      br_hint->advanced_hint = NULL;
    }

  if (!(br_list = list_br_hints))
    {
      br_hint->next_br = NULL;
      br_hint->prev_br = NULL;
      br_list = br_hint;
      list_br_hints = br_hint;
    }
  else
    {
      br_hint->next_br = NULL;
      while (br_list->next_br != NULL)
	br_list = br_list->next_br;
      br_list->next_br = br_hint;
      br_hint->prev_br = br_list;
    }
}


/****************************************************************************
 *
 * routine: BH_delete_list_br_hints
 * purpose: Free hint list
 * input:  list - list to free
 * output: list = 0
 * returns:
 * modified:
 *         
 * note:
 *-------------------------------------------------------------------------*/

static void
BH_delete_list_br_hints (BH_Br_hint * list)
{
  BH_Br_hint *next_br, *br_hint;
  BH_hints_list *hint_list, *next_hint;
  BH_hint_path *hint_path, *next_path;

  br_hint = list;

  while (br_hint)
    {
      next_br = br_hint->next_br;

      L_delete_operand (br_hint->label);
      L_delete_operand (br_hint->target);
      if (Ltahoe_use_counted_prefetch_hints)
	L_delete_operand (br_hint->num_bundles);

      hint_list = br_hint->list;
      while (hint_list)
	{
	  next_hint = hint_list->next_hint;

	  hint_path = hint_list->path_info.path;
	  while (hint_path)
	    {
	      next_path = hint_path->next_cb;
	      free (hint_path);
	      hint_path = next_path;
	    }

	  free (hint_list);
	  hint_list = next_hint;
	}

      free (br_hint);

      br_hint = next_br;
    }

  return;
}


/**************************************************************
 *
 * routine: BH_hint_compare
 * purpose: compare two hints for qsort
 * input:   hint1, hint2 - two hints to compare
 * output:
 * returns: -1 - hint1 is more important
 *           0 - hint1 = hint2 in priority
 *           1 - hint2 is more important
 *          
 * modified:
 *         
 * note:
 *-------------------------------------------------------------------------*/

int
BH_hint_compare (BH_Br_hint ** hint1, BH_Br_hint ** hint2)
{
  if (!*hint1)
    {
      if (*hint2)
	return (1);
    }
  else if (!*hint2)
    {
      return (-1);
    }
  else
    {
      if (!Ltahoe_mckinley_hints)
	{
	  if (((*hint1)->ih == IH_IMP) && ((*hint2)->ih == IH_NONE))
	    return (-1);
	  else if (((*hint2)->ih == IH_IMP) && ((*hint1)->ih == IH_NONE))
	    return (1);
	}

      if ((*hint1)->path_weight < (*hint2)->path_weight)
	return (1);
      else if ((*hint2)->path_weight < (*hint1)->path_weight)
	return (-1);

      if ((*hint1)->prob < (*hint2)->prob)
	return (1);
      else if ((*hint2)->prob < (*hint1)->prob)
	return (-1);
    }

  return (0);

}


/**************************************************************
 *
 * routine: BH_prioritize_branch_hints
 * purpose: prioritize hints before sorting
 * input:
 * output:
 * returns: Pointer to sorted list
 * modified:
 *         
 * note:
 *-------------------------------------------------------------------------*/

static BH_Br_hint **
BH_prioritize_branch_hints ()
{
  BH_Br_hint *br_list, **hint_array = 0;
  int cnt;

  br_list = list_br_hints;

  if ((br_list != NULL) && (br_hint_cnt > 0))
    {
      hint_array =
	(BH_Br_hint **) calloc (br_hint_cnt, sizeof (BH_Br_hint *));

      for (cnt = 0; cnt < br_hint_cnt; cnt++)
	{
	  if (!br_list)
	    continue;
	  hint_array[cnt] = br_list;
	  br_list = br_list->next_br;
	}

      qsort (hint_array, br_hint_cnt, sizeof (BH_Br_hint *),
	     (int (*)(const void *, const void *)) (BH_hint_compare));
    }

  return (hint_array);
}


/****************************************************************************
 *
 * routine: BH_hint_str
 * purpose: convert importance hint to string
 * input: br_hint - br hint struct
 * output:
 * returns: pointer to string
 * modified:
 *         
 * note:
 *-------------------------------------------------------------------------*/

static char *
BH_hint_str (BH_Br_hint * br_hint)
{

  switch (br_hint->ih)
    {
    case IH_IMP:
      return ("TAR");
    case IH_NONE:
      return ("TAC");
    default:
      return ("Unknown type");
    }
}


/****************************************************************************
 *
 * routine: BH_print_hint
 * purpose: print debug messages
 * input: type - type of message
 *        cb_id - cb working in
 *        br_hint - br_hint working on
 * output: 
 * returns:
 * modified:
 *         
 * note:
 *-------------------------------------------------------------------------*/

#if 0
static void
BH_print_hint (int type, int cb_id, BH_Br_hint * br_hint)
{
  switch (type)
    {
    case INSERTED_HINT:
      if (br_hint->br_type == BR_COND)
	{
	  fprintf (stderr, "  Inserted brp%d in cb %d for cond br %d "
		   "from cb %d (prob: %3.0f%%, w: %6.0f - %s)\n",
		   br_hint->list_tail->hint_oper->id,
		   cb_id, br_hint->br_oper->id, br_hint->br_cb->id,
		   br_hint->prob * 100, br_hint->path_weight,
		   BH_hint_str (br_hint));
	}
      else
	{
	  fprintf (stderr, "  Inserted brp%d in cb %d for call br %d "
		   "from cb %d (w: %6.0f - %s)\n",
		   br_hint->list_tail->hint_oper->id,
		   cb_id, br_hint->br_oper->id, br_hint->br_cb->id,
		   br_hint->path_weight, BH_hint_str (br_hint));
	}
      break;

    case MISS_IN_CB:
      if (br_hint->br_type == BR_COND)
	{
	  fprintf (stderr,
		   "  Unable to find place to insert brp in cb %d for cond br %d "
		   "from cb %d (prob: %3.0f%%, w: %6.0f - %s)\n", cb_id,
		   br_hint->br_oper->id, br_hint->br_cb->id,
		   br_hint->prob * 100, br_hint->path_weight,
		   BH_hint_str (br_hint));
	}
      else
	{
	  fprintf (stderr,
		   "  Unable to find place to insert brp in cb %d for call br %d "
		   "from cb %d (w: %6.0f - %s)\n", cb_id,
		   br_hint->br_oper->id, br_hint->br_cb->id,
		   br_hint->path_weight, BH_hint_str (br_hint));
	}
      break;

    case INSERT_FAILED:
      fprintf (stderr,
	       "   --> Failed attempt to insert brp for op %d from cb %d\n",
	       br_hint->br_oper->id, cb_id);
      break;

    default:
      fprintf (stderr, "BH_print_hint: unknown message\n");
      break;
    }
}
#endif

/****************************************************************************
 *
 * routine: BH_print_hint_suggestions
 * purpose: print hint suggestions 
 * input:
 * output: 
 * returns:
 * modified:
 *         
 * note:
 *-------------------------------------------------------------------------*/

static void
BH_print_hint_suggestions (BH_Br_hint ** hint_array, char *title)
{

  BH_Br_hint *br_hint = NULL;
  int cnt;

  if (!hint_array)
    {
      br_hint = list_br_hints;

      if (br_hint == NULL)
	fprintf (stderr, "\n** No branch hints suggested **\n");
    }

  if (title)
    fprintf (stderr, "\n** %s **\n", title);
  else
    fprintf (stderr, "\n** Hint Suggestions **\n");

  for (cnt = 0; cnt < br_hint_cnt; cnt++)
    {

      if (hint_array)
	{
	  br_hint = hint_array[cnt];
	}
      else if (!br_hint)
	{
	  L_punt
	    ("BH_print_hint_suggestions: br_hint_list is longer than br_hint_cnt\n");
	}

      if (!br_hint->label)
	fprintf (stderr,
		 " ----- label zero pointer read ----------------------\n");

      if (!br_hint->target)
	fprintf (stderr,
		 " ----- target zero pointer read ---------------------\n");

      if (!br_hint->br_oper)
	fprintf (stderr,
		 " ----- br_oper zero pointer read --------------------\n");

      if (LT_is_call_br (br_hint->br_oper))
	{
	  fprintf (stderr,
		   "Call br op%-3d (%s) p:%3.0f%% w:%7.0f  targ: %s  label: %s\n",
		   br_hint->br_oper->id, BH_hint_str (br_hint),
		   100 * br_hint->prob, br_hint->path_weight,
		   br_hint->target->value.l, br_hint->label->value.l);
	}
      else
	{
	  fprintf (stderr,
		   "Cond br op%-3d (%s) p:%3.0f%% w:%7.0f  targ:(cb %d) label: %s\n",
		   br_hint->br_oper->id, BH_hint_str (br_hint),
		   100 * br_hint->prob, br_hint->path_weight,
		   br_hint->target->value.cb->id, br_hint->label->value.l);
	}

      if (!hint_array)
	{
	  br_hint = br_hint->next_br;
	}

    }

}



/****************************************************************************
 *
 * routine: BH_available_slot
 * purpose: determine if a B slot is available 
 * input: bundle - bundle to look at
 * output: 
 * returns: 1=open slot, 0=no slot
 * modified:
 *         
 * note: (capital letter in template type means a NOP)
 *-------------------------------------------------------------------------*/
#if 0
static int
BH_available_slot (L_Oper * bundle)
{
  if (!LT_is_template_op (bundle))
    return (0);

  switch (hint_fit_table[LT_get_template (bundle)][LT_get_density (bundle)])
    {
    case 0:
      return (0);

    case 1:			/* 1   miI -> miB */
    case 2:			/* 2   mXi -> miB, move 3rd to 2nd */
    case 3:			/* 3   mmI -> mmB */
    case 5:			/* 5   mMi -> miB, move 2,3 to 1,2 */
    case 6:			/* 6   mfI -> mfB */
    case 8:			/* 8   xxB -> xxB */
      return (1);


    case 9:			/* 9   mIb -> mbB, move 3rd branch to 2nd */
      /* is this good to do?  probably not */
      return (0);

    case 20:
      L_punt ("Using reserved template  bundle:%d\n", bundle->id);

    default:
      L_punt ("Unknown put_hint bundle:%d\n", bundle->id);
    }

  return (0);
}
#endif


/****************************************************************************
 *
 * routine: BH_cb_info
 * purpose: find cb bundle information
 * input: cb1, cb2 - cb's to work on
          operation - function to perform on cb's
 *          BUNDLES_IN_CB - returns number of bundles in cb
 *          INITIALIZE    - returns 1=initialized, 0=failed
 *          BETWEEN       - number of bundles between heads of two cb's
 *          UPDATE        - update bundle count for cb1
 *          ADD_INSERTED  - add to inserted count for cb1
 *          ADD_EXPANDED  - add to expanded count for cb1
 *          NUM_EXPANDED  - return expanded count for cb1
 *          NUM_INSERTED  - return inserted count for cb1
 * output: 
 * returns: result
 * modified:
 *         
 * note:
 *-------------------------------------------------------------------------*/

static void
free_CB_info (void *ptr)
{
  free ((BH_CB_Info *) ptr);
}

static int
BH_cb_info (int operation, L_Cb * cb1, L_Cb * cb2)
{
  L_Cb *cb;
  L_Oper *oper;
  int bundle_count;
  void free_info (void *ptr);
  static int cb_init_done = FALSE;
  static HashTable cb_info = NULL;
  BH_CB_Info *info = NULL;

  switch (operation)
    {
    case BUNDLES_IN_CB:
      if (cb_init_done)
	info = (BH_CB_Info *) HashTable_find (cb_info, cb1->id);
      else
	L_punt ("BH_cb_info: BUNDLES_IN_CB - Bundle counts not initialized");

      return (info->bundles_in_cb);

    case INITIALIZE:
      if (!cb_info)
	cb_info = HashTable_create (512);
      else
	HashTable_reset_func (cb_info, free_CB_info);

      for (cb = cb1; cb; cb = cb->next_cb)
	{
	  bundle_count = 0;
	  for (oper = cb->first_op; oper; oper = oper->next_op)
	    if (LT_is_template_op (oper))
	      bundle_count++;

	  bundle_count++;	/* to account for later padding of cb's */

	  if (!(info = (BH_CB_Info *) malloc (sizeof (BH_CB_Info))))
	    L_punt ("BH_cb_info: Couldn't alloc CB_info entry");
	  info->bundles_in_cb = bundle_count;
	  info->num_expanded = 0;
	  info->num_inserted = 0;
	  HashTable_insert (cb_info, cb->id, info);
	}
      cb_init_done = TRUE;
      return (cb_init_done);

    case CLEANUP:
      /* Free the hash table and cb info entries */
      HashTable_free_func (cb_info, free_CB_info);
      cb_info = NULL;
      return 1;

    case UPDATE:
      if (cb_init_done)
	{
	  bundle_count = 0;
	  for (oper = cb1->first_op; oper; oper = oper->next_op)
	    if (LT_is_template_op (oper))
	      bundle_count++;

	  bundle_count++;
	  info = (BH_CB_Info *) HashTable_find (cb_info, cb1->id);
	  info->bundles_in_cb = bundle_count;

	  return (bundle_count);
	}
      else
	L_punt ("BH_cb_info: UPDATE - Bundle counts not initialized");

    case ADD_EXPANDED:
      if (cb_init_done)
	{
	  info = (BH_CB_Info *) HashTable_find (cb_info, cb1->id);
	  info->num_expanded++;
	  return (info->num_expanded);
	}
      else
	L_punt ("BH_cb_info: NUM_EXPANDED - Bundle counts not initialized");

    case ADD_INSERTED:
      if (cb_init_done)
	{
	  info = (BH_CB_Info *) HashTable_find (cb_info, cb1->id);
	  info->num_inserted++;
	  return (info->num_inserted);
	}
      else
	L_punt ("BH_cb_info: NUM_INSERTED - Bundle counts not initialized");

    case NUM_EXPANDED:
      if (cb_init_done)
	{
	  info = (BH_CB_Info *) HashTable_find (cb_info, cb1->id);
	  return (info->num_expanded);
	}
      else
	L_punt ("BH_cb_info: NUM_EXPANDED - Bundle counts not initialized");

    case NUM_INSERTED:
      if (cb_init_done)
	{
	  info = (BH_CB_Info *) HashTable_find (cb_info, cb1->id);
	  return (info->num_inserted);
	}
      else
	L_punt ("BH_cb_info: NUM_INSERTED - Bundle counts not initialized");

    case BETWEEN:
      if (!cb_init_done)
	L_punt ("BH_cb_info: BETWEEN - Bundle counts not initialized");

      bundle_count = 0;
      for (cb = cb1; cb; cb = cb->next_cb)
	{
	  info = (BH_CB_Info *) HashTable_find (cb_info, cb->id);
	  bundle_count += info->bundles_in_cb;
	  if (cb1->id == cb2->id)
	    return (bundle_count);
	}

      /* Cbs must have been in different order */
      bundle_count = 0;
      for (cb = cb2; cb; cb = cb->next_cb)
	{
	  info = (BH_CB_Info *) HashTable_find (cb_info, cb->id);
	  bundle_count += info->bundles_in_cb;
	  if (cb2->id == cb1->id)
	    return (bundle_count);
	}
      break;

    default:
      L_punt ("BH_cb_info: Unknown command\n");
      break;

    }
  return 0;
}



/****************************************************************************
 *
 * routine: BH_static_distance
 * purpose: check the static distance between a hint and the branch
 * input:  hint_cb - cb containing the hint
 *         hint_oper - hint oper
 *         br_cb - cb containing the branch
 *         br_oper - branch oper being hinted
 *         *direction - path direction, returns 1 if brp is first statically
 *                                              0 if br is first statically
 * output: 
 * returns:  distance in bundles
 * modified:
 *         
 * note:
 *-------------------------------------------------------------------------*/

static int
BH_static_distance (L_Cb * hint_cb, L_Oper * hint_oper,
		    L_Cb * br_cb, L_Oper * br_oper, int *direction)
{

  int bundle_cnt = 0;
  L_Cb *cb, *first_cb, *second_cb;
  L_Oper *oper, *first_oper = NULL, *second_oper = NULL;

  if (!hint_cb || !br_cb)
    L_punt ("BH_static_distance: cb oper is NULL!!\n");

  if (hint_cb == br_cb)
    {
      /* in same cb, so find which oper is first */
      first_cb = br_cb;
      second_cb = br_cb;

      oper = br_cb->first_op;
      while (oper)
	{
	  if (oper == hint_oper)
	    {
	      first_oper = hint_oper;
	      second_oper = br_oper;
	      *direction = BRP_TO_BR;	/* brp is first */
	      break;
	    }
	  else if (oper == br_oper)
	    {
	      first_oper = br_oper;
	      second_oper = hint_oper;
	      *direction = BR_TO_BRP;	/* br is first */
	      break;
	    }
	  oper = oper->next_op;
	}

      if (!oper)
	L_punt ("BH_static_distance: Opers aren't in the cb specified!\n");
    }
  else
    {
      /* Find which cb is first statically */
      first_cb = hint_cb;
      while (first_cb)
	{
	  if (first_cb == br_cb)
	    break;
	  first_cb = first_cb->next_cb;
	}

      if (first_cb)
	{			/* hint_cb -> br_cb */
	  first_cb = hint_cb;
	  first_oper = hint_oper;
	  second_cb = br_cb;
	  second_oper = br_oper;
	  *direction = BRP_TO_BR;	/* brp is first */
	}
      else
	{			/* br_cb -> hint_cb */
	  first_cb = br_cb;
	  first_oper = br_oper;
	  second_cb = hint_cb;
	  second_oper = hint_oper;
	  *direction = BR_TO_BRP;	/* br is first */
	}
    }

  /* Walk from first to second cb */
  for (cb = first_cb; cb; cb = cb->next_cb)
    {
      if (cb == first_cb)
	{
	  bundle_cnt++;		/* for padding */
	  for (oper = first_oper; oper; oper = oper->next_op)
	    {
	      if (LT_is_template_op (oper))
		bundle_cnt++;

	      if (oper == second_oper)
		break;
	    }
	  if (oper)
	    break;
	}
      else if (cb == second_cb)
	{
	  bundle_cnt++;		/* for padding */
	  for (oper = cb->first_op; oper; oper = oper->next_op)
	    {
	      if (LT_is_template_op (oper))
		bundle_cnt++;

	      if (oper == second_oper)
		break;
	    }
	  if (oper)
	    break;
	}
      else
	{
	  bundle_cnt += BH_cb_info (BUNDLES_IN_CB, cb, 0);
	}
    }

  return (bundle_cnt);
}



/****************************************************************************
 *
 * routine: BH_insert_hint_in_paths
 * purpose: insert hint in this cb or every path to this cb
 * input:   cb - cb to start in
 *          starting_oper - oper to start inserting above
 *          br_hint - branch hint structure
 *          path_stats - cumul
 *          min_fe_cycles - fe_cycles to go up before inserting
 *          max_recursion - max cb's to go up paths to try to insert
 *          indent - debug message printing indentation for recursion
 * output:
 * returns: >1 - inserted
 *          -1 - not inserted
 * modified:
 * note:
 *-------------------------------------------------------------------------*/


static int
BH_insert_hint_in_paths (L_Cb * cb, L_Oper * starting_oper,
			 BH_Br_hint * br_hint,
			 BH_path_info * path_stats, int min_fe_cycles,
			 int max_recursion, int indent, int test)
{
  L_Flow *flow;
  L_Cb *pred_cb;
  BH_path_info stats = { NULL };
  int cycles_left, result = 0;

  BH_add_cb_to_path (cb, path_stats);

  cycles_left = BH_insert_hint_in_block (cb, starting_oper, br_hint,
					 path_stats, min_fe_cycles, indent,
					 test);

  if ((max_recursion != 0) && (cycles_left != 0))
    {
      if (max_recursion > 0)
	max_recursion--;

      if (!cb->src_flow)	/* no predecessors, so mark failure */
	br_hint->paths_tried++;

      for (flow = cb->src_flow; flow != NULL; flow = flow->next_flow)
	{
	  pred_cb = flow->src_cb;

	  BH_copy_path_info (&stats, path_stats);

#ifdef HINT_DEBUG
	  fprintf (stderr, "   %*cgoing from cb%d to cb%d for %d cycles\n",
		   2 * indent, ' ', cb->id, flow->src_cb->id, cycles_left);
#endif

	  if (!Set_in (cbs_looked_at_set, pred_cb->id))
	    {
	      cbs_looked_at_set = Set_add (cbs_looked_at_set, pred_cb->id);
	      result |=
		BH_insert_hint_in_paths (pred_cb, pred_cb->last_op, br_hint,
					 &stats, cycles_left, max_recursion,
					 indent + 1, test);
	    }
	  else
	    {
#ifdef HINT_DEBUG
	      fprintf (stderr, "   %*c- Hit this cb before (cb%d)\n",
		       2 * indent, ' ', cb->id);
#endif
	      stat_merged_path++;
	    }
	}
    }
  else if (cycles_left == 0)
    {
      result = 1;		/* success */
      br_hint->paths_tried++;
      br_hint->paths_success++;
      total_attempted++;
    }
  else if (cycles_left < 0)
    {
      result = -1;		/* failure */
      br_hint->paths_tried++;
      total_attempted++;
    }
  else if (cycles_left > 0)
    {
      result = -1;		/* failure */
      br_hint->paths_tried++;
      stat_max_recursion++;
      total_attempted++;
#ifdef HINT_DEBUG
      fprintf (stderr, "   %*c- Hit the maximum cb recursion level (cb%d)\n",
	       2 * indent, ' ', cb->id);
#endif
    }

  /* >1 - inserted */
  /* -1 - not inserted */
  return (result);
}


/****************************************************************************
 *
 * routine: BH_add_to_hint_list()
 * purpose: add hint to list for br
 * input:   br_hint - branch info
 *          stats - cumulative path stats
 *          hint_oper - new hint oper
 *          hint_cb - cb of new hint
 * output:
 * returns:
 * modified:  Adds hint to br_hint list
 *                         
 * note:
 *-------------------------------------------------------------------------*/

static void
BH_add_to_hint_list (BH_Br_hint * br_hint, BH_path_info * stats,
		     L_Oper * hint_oper, L_Cb * hint_cb)
{
  if (br_hint->list)
    {
      br_hint->list_tail->next_hint =
	(BH_hints_list *) calloc (1, sizeof (BH_hints_list));
      br_hint->list_tail = br_hint->list_tail->next_hint;
    }
  else
    {
      br_hint->list = (BH_hints_list *) calloc (1, sizeof (BH_hints_list));
      br_hint->list_tail = br_hint->list;
    }

  BH_copy_path_info (&(br_hint->list_tail->path_info), stats);
  br_hint->list_tail->hint_oper = hint_oper;
  br_hint->list_tail->hint_cb = hint_cb;
}


/****************************************************************************
 *
 * routine: BH_add_cb_to_path()
 * purpose: add cb to path list
 * input:   cb - cb to add to path
 *          path_info - path structure
 * output: 
 * returns:
 * modified: Adds cb to path
 *                         
 * note:
 *-------------------------------------------------------------------------*/

static void
BH_add_cb_to_path (L_Cb * cb, BH_path_info * path_info)
{

  if (path_info->path)
    {
      path_info->path_tail->next_cb =
	(BH_hint_path *) calloc (1, sizeof (BH_hint_path));
      path_info->path_tail = path_info->path_tail->next_cb;
    }
  else
    {
      path_info->path = (BH_hint_path *) calloc (1, sizeof (BH_hint_path));
      path_info->path_tail = path_info->path;
    }

  path_info->path_tail->cb = cb;

}


/****************************************************************************
 *
 * routine: BH_copy_path_info()
 * purpose: copy stat structure
 * input:   dest - dest struct of copy
 *          src  - src struct of copy
 * output: 
 * returns:
 * modified: 
 *                         
 * note:  THIS MUST BE UPDATED WHEN BH_path_info STRUCTURE CHANGES
 *-------------------------------------------------------------------------*/

static void
BH_copy_path_info (BH_path_info * dest, BH_path_info * src)
{
  BH_hint_path *dest_path, *src_path;

  if (!src)
    {
      L_punt ("BH_copy_path_info: src is NULL!!\n");
      return;
    }

  if (!dest)
    {
      L_punt ("BH_copy_path_info: dest is NULL!!\n");
      return;
    }

  dest->num_br_crossed = src->num_br_crossed;
  dest->num_calls_crossed = src->num_calls_crossed;
  dest->num_cbs_on_path = src->num_cbs_on_path;
  dest->fe_cycles_away = src->fe_cycles_away;
  dest->brps_on_path = src->brps_on_path;
  dest->tars_on_path = src->tars_on_path;

  dest->expanded_bundle = src->expanded_bundle;

  dest->direction = src->direction;
  dest->static_distance = src->static_distance;

  if (src->path)
    {
      dest->path = (BH_hint_path *) calloc (1, sizeof (BH_hint_path));
      dest->path_tail = dest->path;

      dest->path->cb = src->path->cb;

      dest_path = dest->path;
      src_path = src->path->next_cb;

      while (src_path)
	{

	  dest_path->next_cb =
	    (BH_hint_path *) calloc (1, sizeof (BH_hint_path));
	  dest_path = dest_path->next_cb;

	  dest_path->cb = src_path->cb;

	  src_path = src_path->next_cb;
	}

      dest->path_tail = dest_path;

    }
  else
    {
      dest->path = 0;
    }
  return;
}


/****************************************************************************
 *
 * routine: BH_ok_to_expand()
 * purpose: determine if it's ok to expand a bundle
 * input:   cb - cb to expand
 *          oper - template to expand
 *          br_hint - br to expand for
 * output:  1 - ok to expand
 *          0 - not ok to expand
 * returns:
 * modified: JEP 5/2/98 Added parm so that no expanding is allowed
 
 *                         
 * note: A bundle will not be exanded if
                 - the dont_expand_for_hints parm is on
                 - expanding will hurt a current hint/path
 *-------------------------------------------------------------------------*/

static int
BH_ok_to_expand (L_Cb * cb, L_Oper * oper, BH_Br_hint * br_hint)
{
  BH_Br_hint *br_list;
  BH_hints_list *hint_list;
  int on_path;

#if 0
  if (BH_cb_info (NUM_EXPANDED, cb, NULL) >= MAX_EXPANDS_PER_CB)
    return (0);
#endif

  if (Ltahoe_dont_expand_for_hints)
    return (0);

  br_list = list_br_hints;

  while (br_list)
    {
      hint_list = br_list->list;

      while (hint_list)
	{
	  if (hint_list->path_info.direction == BRP_TO_BR)
	    {
	      /* brp -> br */
	      on_path =
		BH_is_on_static_path (cb, oper, hint_list->hint_cb,
				      hint_list->hint_oper,
				      br_list->br_cb, br_list->br_oper);
	    }
	  else if (hint_list->path_info.direction == BR_TO_BRP)
	    {
	      /* br -> brp */
	      on_path =
		BH_is_on_static_path (cb, oper, br_list->br_cb,
				      br_list->br_oper,
				      hint_list->hint_cb,
				      hint_list->hint_oper);
	    }
	  else
	    {
	      fprintf (stderr,
		       "BH_ok_to_expand: Unknown path direction warning!\n");
	      on_path = 1;	/* conservative? */
	    }

	  if (on_path &&
	      (hint_list->path_info.static_distance >=
	       MAX_BUNDLES_BETWEEN_HINT_AND_BR))
	    return (0);		/* only need to hit one that it hurts */

	  hint_list = hint_list->next_hint;
	}

      br_list = br_list->next_br;
    }

  return (1);
}


/****************************************************************************
 *
 * routine: BH_ok_to_insert()
 * purpose: determine if it's ok to insert a brp (no interference)
 * input:   cb - cb to insert into
 *          br_hint - br info
 * output: 
 * returns: 1 - ok to insert
 *          0 - inserting will hurt a hint/path
 * modified: 
 *                         
 * note:
 *-------------------------------------------------------------------------*/

static int
BH_ok_to_insert (L_Cb * cb, BH_Br_hint * br_hint)
{
  BH_Br_hint *br_list;
  BH_hints_list *hint_list;
  L_Attr *ih_attr;


  /* do we need to look at brps_on_path? */

  if (BH_cb_info (NUM_INSERTED, cb, 0) >= MAX_INSERTS_PER_CB)
    return (0);

  /* We're not a tar, so won't interfere */
  /* should check for max # brp's on path? */
  if (br_hint->ih != IH_IMP)
    return (1);

  br_hint->spec_ih = br_hint->ih;
  br_list = list_br_hints;

  while (br_list)
    {
      /* only check TAR interference right now */
      if (br_list->ih != IH_IMP)
	{
	  br_list = br_list->next_br;
	  continue;
	}

      hint_list = br_list->list;
      while (hint_list)
	{
	  /* only check TAR interference right now */
	  ih_attr = L_find_attr (hint_list->hint_oper->attr, "ih");
	  /* JWS -- continue if no ih_attr */
	  if (!ih_attr || (L_get_int_attr_field (ih_attr, 0) != IH_IMP))
	    {
	      hint_list = hint_list->next_hint;
	      continue;
	    }

	  if (BH_is_on_dynamic_path (cb, hint_list) &&
	      (hint_list->path_info.tars_on_path >= MAX_TARS_ON_PATH))
	    {
	      /* either change to a TAC hint or return(0) */
	      br_hint->spec_ih = IH_NONE;	/* change to tac */
	      return (1);	/* only need to hit one that it hurts */
	    }
	  hint_list = hint_list->next_hint;
	}
      br_list = br_list->next_br;
    }
  return (1);
}


/****************************************************************************
 *
 * routine: BH_check_all_distances()
 * purpose: brute force check of static distances
 * input:
 * output:
 * returns:
 * modified: 
 *                         
 * note:
 *-------------------------------------------------------------------------*/
#ifdef HINT_DEBUG
static void
BH_check_all_distances ()
{
  BH_Br_hint *br_list;
  BH_hints_list *hint_list;
  int dist, direction;

  br_list = list_br_hints;

  while (br_list)
    {
      hint_list = br_list->list;
      while (hint_list)
	{
	  dist = BH_static_distance (hint_list->hint_cb,
				     hint_list->hint_oper,
				     br_list->br_cb,
				     br_list->br_oper, &direction);

	  if (dist != hint_list->path_info.static_distance)
	    {
	      fprintf (stderr,
		       "BH_check_all_distances: DISTANCE MISMATCH pbr%d -> "
		       "br%d == %d bundles (info says %d)\n",
		       hint_list->hint_oper->id, br_list->br_oper->id, dist,
		       hint_list->path_info.static_distance);
	    }

	  if (dist >= MAX_INSERTION_DISTANCE)
	    {
	      fprintf (stderr,
		       "BH_check_all_distances: HINT TOO FAR pbr%d -> "
		       "br%d == %d bundles (info says %d)\n",
		       hint_list->hint_oper->id, br_list->br_oper->id, dist,
		       hint_list->path_info.static_distance);
	    }
	  hint_list = hint_list->next_hint;
	}

      br_list = br_list->next_br;
    }
  return;
}
#endif



/****************************************************************************
 *
 * routine: BH_is_on_static_path()
 * purpose: determine if bundle is on the static path
 * input:   exp_cb - cb to expand
 *          exp_oper - template to expand
 *          first_cb - first cb in path (statically)
 *          first_oper - first oper in path (statically)
 *          second_cb - second cb in path (statically)
 *          second_oper - second oper in path (statically)
 * output: 
 * returns: 1 - exp_oper is on path from first_oper to second_oper
 *          0 - exp_oper not on path
 * modified: 
 *                         
 * note:
 *-------------------------------------------------------------------------*/

static int
BH_is_on_static_path (L_Cb * exp_cb, L_Oper * exp_oper,
		      L_Cb * first_cb, L_Oper * first_oper,
		      L_Cb * second_cb, L_Oper * second_oper)
{
  L_Oper *oper, *tmpl_oper;
  L_Cb *cb;

  /* back up to template op */
  tmpl_oper = exp_oper;
  while (!LT_is_template_op (tmpl_oper))
    {
      tmpl_oper = tmpl_oper->prev_op;
      if (!tmpl_oper)
	L_punt
	  ("BH_is_on_static_path: no template op found for bundle %d!!\n",
	   exp_oper->id);
    }

  /* Walk from first to second cb */
  for (cb = first_cb; cb; cb = cb->next_cb)
    {
      if (cb == first_cb)
	{
	  if (cb == exp_cb)
	    {
	      for (oper = first_oper; oper; oper = oper->next_op)
		{
		  if (oper == tmpl_oper)
		    return (1);
		  else if (oper == second_oper)
		    break;
		}
	      return (0);
	    }
	  else if (cb == second_cb)
	    {
	      return (0);
	    }
	}
      else if (cb == second_cb)
	{
	  if (cb == exp_cb)
	    {
	      for (oper = cb->first_op; oper; oper = oper->next_op)
		{
		  if (oper == tmpl_oper)
		    return (1);
		  else if (oper == second_oper)
		    break;
		}
	      return (0);
	    }
	  else
	    {
	      return (0);
	    }
	}
      else
	{
	  if (cb == exp_cb)
	    return (1);
	}
    }

  /* fall thru means we never hit the second cb or expanding cb */
  return (0);
}




/****************************************************************************
 *
 * routine: BH_is_on_dynamic_path()
 * purpose: determine if bundle is on the dynamic path
 * input:   cb - cb to check for
 *          hint_list - hint info (contains path to check)
 * output: 
 * returns: 1 - cb is on the hint's path
 *          0 - cb is not on the path
 * modified: 
 *                         
 * note:
 *-------------------------------------------------------------------------*/

static int
BH_is_on_dynamic_path (L_Cb * cb, BH_hints_list * hint_list)
{
  BH_hint_path *hint_cb;

  hint_cb = hint_list->path_info.path;
  while (hint_cb)
    {
      if (hint_cb->cb == cb)
	return (1);

      hint_cb = hint_cb->next_cb;
    }
  return (0);
}


/****************************************************************************
 *
 * routine: BH_insert_update()
 * purpose: update all paths for this insertion (distance, tars on path, etc)
 * input:   cb - cb inserted into
 *          oper - oper inserted
 *          br_hint - br inserted for
 *          expanded - number of bundles expanded
 * output: 
 * returns:
 * modified:
 *                         
 * note:
 *-------------------------------------------------------------------------*/

static void
BH_insert_update (L_Cb * cb, L_Oper * oper,
		  BH_Br_hint * br_hint, int expanded)
{
  BH_Br_hint *br_list;
  BH_hints_list *hint_list;
  int tar_inc = 0;
  int on_path;

  if (br_hint->spec_ih == IH_IMP)
    tar_inc = 1;

  br_list = list_br_hints;

  while (br_list)
    {
      hint_list = br_list->list;
      while (hint_list)
	{			/* foreach brp on the br */
	  if (hint_list->path_info.direction == BRP_TO_BR)
	    {
	      /* brp -> br */
	      on_path =
		BH_is_on_static_path (cb, oper, hint_list->hint_cb,
				      hint_list->hint_oper, br_list->br_cb,
				      br_list->br_oper);
	    }
	  else if (hint_list->path_info.direction == BR_TO_BRP)
	    {
	      /* br -> brp */
	      on_path =
		BH_is_on_static_path (cb, oper, br_list->br_cb,
				      br_list->br_oper, hint_list->hint_cb,
				      hint_list->hint_oper);
	    }
	  else
	    {
	      fprintf (stderr,
		       "BH_insert_update: Unknown path direction warning!\n");
	      on_path = 1;	/* conservative */
	    }

	  if (on_path)
	    hint_list->path_info.static_distance += expanded;

	  if (BH_is_on_dynamic_path (cb, hint_list))
	    {
	      hint_list->path_info.brps_on_path++;
	      hint_list->path_info.tars_on_path += tar_inc;
	    }

	  hint_list = hint_list->next_hint;
	}
      br_list = br_list->next_br;
    }
  return;
}


/****************************************************************************
 *
 * routine: BH_insert_hint_in_block
 * purpose: insert a hint in this cb
 * input:   cb - cb to insert in
 *          starting_oper - oper to go up from
 *          br_hint - hint info structure
 *          cycles_to_go - fe_cycles left to go up
 *          indent - for recursive debug stmts 
 * output: 
 * returns: Number of fe_cycles left to cover in distance between hint
 *          and branch.
 *          0  = inserted it
 *          -1 = crossed minimum cycles, but failed to insert
 *          N  = still N cycles below the minimum
 * modified:
 * note:
 *-------------------------------------------------------------------------*/

static int
BH_insert_hint_in_block (L_Cb * cb, L_Oper * starting_oper,
			 BH_Br_hint * br_hint,
			 BH_path_info * path_stats,
			 int cycles_to_go, int indent, int test)
{
  int num_bundles, fe_cycles, result, inserted, direction, 
    pvec_new_bundle, untouchable_cb, first_tmpl = 1, pvec[2];
  L_Oper *oper, *top_tmpl = 0, *bottom_tmpl = 0;
  L_Attr *ih_attr;

  pvec[0] = pvec[1] = -1;

  if ((BH_static_distance (cb, cb->first_op, br_hint->br_cb, br_hint->br_oper,
			   &direction) >= MAX_INSERTION_DISTANCE) ||
      (BH_static_distance (cb, cb->last_op, br_hint->br_cb, br_hint->br_oper,
			   &direction) >= MAX_INSERTION_DISTANCE))
    {
#ifdef HINT_DEBUG
      fprintf (stderr, "   %*c- Hit a cb that's too far away (cb%d)\n",
	       2 * indent, ' ', cb->id);
#endif
      stat_cb_too_far++;
      untouchable_cb = 1;
    }
  else if (!BH_ok_to_insert (cb, br_hint))
    {
      /* maybe change from a TAR to a TAC and insert? */
#ifdef HINT_DEBUG
      fprintf (stderr, "   %*c- not ok to insert in cb%d\n",
	       2 * indent, ' ', cb->id);
#endif
      stat_max_inserted++;
      untouchable_cb = 1;
    }
  else
    {
      untouchable_cb = 0;
    }

  /* mbt - change this to be conservative? */
  /* Is the last bundle of the cb the first half of a cache line?    */
  /* Need to know this to count fe_cycles correctly, assuming starts */
  /* of cb's are always cache aligned... */
  num_bundles = BH_bundles_from_top (starting_oper);

  path_stats->num_cbs_on_path++;

  /* track branches for pvec, don't count br we're hinting */
  if (starting_oper == br_hint->br_oper)
    {
      oper = starting_oper->prev_op;
      pvec_new_bundle = 0;
    }
  else
    {
      oper = starting_oper;
      pvec_new_bundle = 1;
    }

  fe_cycles = 0;

  while (oper)
    {
      /* find the highest spot we can insert it in this cb */

      if (LT_is_template_op (oper))
	{
	  pvec_new_bundle = 1;
	  if ((fe_cycles == (-1)) || (fe_cycles >= cycles_to_go))
	    {
	      /* we're beyond the minimum fe cycles */

	      if (first_tmpl)
		{
		  bottom_tmpl = oper;
		  first_tmpl = 0;
		}
	      top_tmpl = oper;

	    }

	  /* do this second so fe_cycles is still correct above */
	  if (num_bundles & 0x01)
	    {
	      fe_cycles++;
	      path_stats->fe_cycles_away++;
	    }
	  num_bundles++;
	}
      else if (LT_is_brp (oper))
	{
	  path_stats->brps_on_path++;

	  ih_attr = L_find_attr (oper->attr, "ih");
	  if (ih_attr && L_get_int_attr_field (ih_attr, 0) == IH_IMP)
	    path_stats->tars_on_path++;
	}
      else if (LT_is_cond_br (oper))
	{
	  /* pvec only does NT edges */
	  if (pvec_new_bundle)
	    {
	      pvec[1] = pvec[0];
	      pvec[0] = 0;
	      pvec_new_bundle = 0;
	      path_stats->num_br_crossed++;
	    }
	}
      else if (LT_is_call_br (oper))
	{
	  /* change to a tac if cross a call. (stop instead?) */
	  br_hint->spec_ih = IH_NONE;

	  if (pvec_new_bundle)
	    {
	      /* calls come back, so set as don't-care */
	      pvec[1] = pvec[0];
	      pvec[0] = -1;
	      pvec_new_bundle = 0;
	      path_stats->num_calls_crossed++;
	    }

	  if (L_same_operand (br_hint->br_oper->src[0], oper->src[0]))
	    {
	      /* another call to same func, so it will do the prefetching */
	      /* should continue for branch prediction, though? mbt */
	      /* check predication? */
#ifdef HINT_DEBUG
	      fprintf (stderr,
		       "   %*c Stop at a call to the same func(op%d)\n",
		       2 * indent, ' ', oper->id);
#endif
	      br_hint->cross_same_call++;
	      return (0);
	    }

	  if (untouchable_cb ||
	      ((cycles_to_go != (-1)) &&
	       (fe_cycles != (-1)) && (fe_cycles < cycles_to_go)))
	    {
	      /* not to the minimum fe cycles yet, keep going */

	      if (L_is_predicated (oper))
		{
		  path_stats->fe_cycles_away++;
		  fe_cycles++;
		}
	      else
		{
		  fe_cycles = (-1);
		  path_stats->fe_cycles_away = (-1);
		}
	    }
	  else
	    {
	      break;
	    }
	}

      oper = oper->prev_op;
    }


  /* problem:  if the minimum bundle is one cycle below the top,
     it only gets one chance to insert it and then dies.
     Should allow it to go up predecessors in this case!
   */


  if ((cycles_to_go != (-1)) &&
      (fe_cycles != (-1)) && (fe_cycles < cycles_to_go))
    {
      result = (cycles_to_go - fe_cycles);
    }
  else if (!untouchable_cb)
    {
      br_hint->pvec = pvec_table[1 + pvec[0]][1 + pvec[1]];
      inserted = BH_insert_at_best_location (cb, top_tmpl, bottom_tmpl,
					     br_hint, path_stats, test);
      result = (inserted == 1) ? 0 : -1;
    }

  else
    {
      result = -1;
    }

  /* 0  = inserted it */
  /* -1 = crossed minimum cycles, but failed */
  /* N  = still N cycles below the minimum */
  return (result);
}


/****************************************************************************
 *
 * routine: BH_insert_at_best_location
 * purpose: insert at the best location found in the cb
 * input:   cb - cb inserting in
 *          top_tmpl - highest bundle you can insert it
 *          bottom_tmpl - lowest bundle you can insert it
 *          br_hint - branch hint info
 *          path_stats - cumulative path stats
 * output: 
 * returns: -1 = failed to find a location
 *           0 = found a spot but failed to insert
 *           1 = found a spot and inserted
 * modified:
 *         
 * note:
 *-------------------------------------------------------------------------*/

static int
BH_insert_at_best_location (L_Cb * cb, L_Oper * top_tmpl,
			    L_Oper * bottom_tmpl, BH_Br_hint * br_hint,
			    BH_path_info * path_stats, int test)
{
  L_Oper *oper = top_tmpl;
  L_Oper *empty_slot;
  int inserted = (-1);
  int result = (-1);

  while (oper)
    {
      if (!LT_is_template_op (oper))
	{
	  oper = oper->next_op;
	  continue;
	}

      if (test)
	return (BH_test_insert_hint_in_bundle
		(cb, oper, br_hint, path_stats));

      /* try to insert without expansion */
      inserted = BH_insert_hint_in_bundle (cb, oper, br_hint, path_stats);

      if (!inserted)
	{
	  /* couldn't, so try to expand and insert */
	  if (BH_ok_to_expand (cb, oper, br_hint))
	    {
	      if ((empty_slot = Ltahoe_pad_bundle (cb, oper)))
		{
		  BH_cb_info (UPDATE, cb, NULL);
		  BH_cb_info (ADD_EXPANDED, cb, NULL);
		  BH_insert_update (cb, oper, br_hint, 1);
		  BH_add_cb_densities (cb);

#ifdef HINT_DEBUG
		  fprintf (stderr, "   --> expanded bundle %d in cb %d\n",
			   empty_slot->id, cb->id);
		  BH_check_all_distances ();
#endif

		  path_stats->expanded_bundle++;
		  inserted = BH_insert_hint_in_bundle (cb, empty_slot,
						       br_hint, path_stats);

		  if (inserted)
		    {
		      result = 1;
		      break;
		    }
		  else
		    {
		      result = 0;
		    }
		}
	      else
		{
		  result = -1;
#ifdef HINT_DEBUG
		  fprintf (stderr,
			   "   Couldn't expand bundle%d in cb%d for br%d\n",
			   oper->id, cb->id, br_hint->br_oper->id);
#endif
		}
	    }

	  else
	    {
	      result = -1;
	      stat_max_expand++;
#ifdef HINT_DEBUG
	      fprintf (stderr,
		       "   Not ok to expand bundle%d in cb %d for br%d\n",
		       oper->id, cb->id, br_hint->br_oper->id);
#endif
	    }
	}

      else
	{
	  result = 1;
	  BH_insert_update (cb, oper, br_hint, 0);
	  break;
	}

      if (oper == bottom_tmpl)
	{
#ifdef HINT_DEBUG
	  fprintf (stderr, "   Hit the bottom template %d\n", oper->id);
#endif

	  inserted = -1;
	  break;
	}
      else
	{
	  oper = oper->next_op;
	}
    }

  if (!oper)
    inserted = -1;

  if (inserted > 0)
    BH_cb_info (ADD_INSERTED, cb, NULL);

#ifdef HINT_DEBUG2
  if (Ltahoe_print_hint_info)
    {
      if (inserted > 0)
	BH_print_hint (INSERTED_HINT, cb->id, br_hint);
      else
	BH_print_hint (INSERT_FAILED, cb->id, br_hint);
    }
#endif

  /* -1 = failed to find a location */
  /* 0 = found a spot but failed to insert */
  /* 1 = found a spot and inserted */

  return (result);
}


/****************************************************************************
 *
 * routine: BH_test_insert_hint_in_bundle
 * purpose: insert hint in a bundle (without expanding)
 * input:   cb - cb inserting into
 *          bundle - bundle to insert into
 *          br_hint - branch hint info
 *          stats - cumulative path stats
 * output: 
 * returns: -1 = failed to insert
 *           1 = inserted
 * modified:
 *         
 * note:
 *-------------------------------------------------------------------------*/

static int
BH_test_insert_hint_in_bundle (L_Cb * cb, L_Oper * bundle,
			       BH_Br_hint * br_hint, BH_path_info * stats)
{
  L_Oper *tmpl_oper = NULL;

  if (bundle)
    tmpl_oper = bundle;
  else
    L_punt ("BH_insert_hint_in_bundle: bundle oper is NULL!!\n");

  while (!LT_is_template_op (tmpl_oper))
    {
      tmpl_oper = tmpl_oper->prev_op;
      if (!tmpl_oper)
	L_punt
	  ("BH_insert_hint_in_bundle: no template op found for bundle %d!!\n",
	   bundle->id);
    }

  switch (hint_fit_table[LT_get_template (tmpl_oper)]
	  [LT_get_density (tmpl_oper)])
    {

    case 0:
    case 9:
      return (0);

    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
      return (1);

    default:
      return (0);

    }
}

/****************************************************************************
 *
 * routine: BH_insert_hint_in_bundle
 * purpose: insert hint in a bundle (without expanding)
 * input:   cb - cb inserting into
 *          bundle - bundle to insert into
 *          br_hint - branch hint info
 *          stats - cumulative path stats
 * output: 
 * returns: -1 = failed to insert
 *           1 = inserted
 * modified: 9/12/02 REK Updating to use new TAHOEops/completer scheme.
 *         
 * note:
 *-------------------------------------------------------------------------*/

static int
BH_insert_hint_in_bundle (L_Cb * cb, L_Oper * bundle,
			  BH_Br_hint * br_hint, BH_path_info * stats)
{
  L_Oper *tmpl_oper = NULL;
  L_Oper *first_oper, *second_oper = NULL, *hint_oper;
  L_Attr *attr;
  int result, direction;

  if (bundle)
    tmpl_oper = bundle;
  else
    L_punt ("BH_insert_hint_in_bundle: bundle oper is NULL!!\n");

  while (!LT_is_template_op (tmpl_oper))
    {
      tmpl_oper = tmpl_oper->prev_op;
      if (!tmpl_oper)
	L_punt
	  ("BH_insert_hint_in_bundle: no template op found for bundle %d!!\n",
	   bundle->id);
    }				/* while */

  switch (hint_fit_table[LT_get_template (tmpl_oper)]
	  [LT_get_density (tmpl_oper)])
    {
    case 0:
      return (0);

    case 1:			/* 1   miI -> miB */
      LT_set_template (tmpl_oper, MIB);
      LT_set_density (tmpl_oper, LT_get_density (tmpl_oper) | 1);
      first_oper = tmpl_oper->next_op;
      second_oper = first_oper->next_op;
      L_delete_oper (cb, second_oper->next_op);
      break;

    case 2:			/* 2   mXi -> miB, move 3rd to 2nd */
      LT_set_template (tmpl_oper, MIB);
      LT_set_density (tmpl_oper, LT_get_density (tmpl_oper) | 3);
      first_oper = tmpl_oper->next_op;
      L_delete_oper (cb, first_oper->next_op);
      second_oper = first_oper->next_op;
      break;

    case 3:			/* 3   mmI -> mmB */
      LT_set_template (tmpl_oper, MMB);
      LT_set_density (tmpl_oper, LT_get_density (tmpl_oper) | 1);
      first_oper = tmpl_oper->next_op;
      second_oper = first_oper->next_op;
      L_delete_oper (cb, second_oper->next_op);
      break;

    case 5:			/* 5   Mmi -> miB, move 2,3 to 1,2 */
      LT_set_template (tmpl_oper, MIB);
      LT_set_density (tmpl_oper, 7);
      L_delete_oper (cb, tmpl_oper->next_op);
      second_oper = tmpl_oper->next_op->next_op;
      break;

    case 6:			/* 6   mfI -> mfB */
      LT_set_template (tmpl_oper, MFB);
      LT_set_density (tmpl_oper, LT_get_density (tmpl_oper) | 1);
      first_oper = tmpl_oper->next_op;
      second_oper = first_oper->next_op;
      L_delete_oper (cb, second_oper->next_op);
      break;

    case 8:			/* 8  xxB -> xxB  No change needed */
      LT_set_density (tmpl_oper, (LT_get_density (tmpl_oper) | 1));
      first_oper = tmpl_oper->next_op;
      second_oper = first_oper->next_op;
      L_delete_oper (cb, second_oper->next_op);
      break;

    case 9:			/* 9   mIb -> mbB, move 3rd branch to 2nd */
      /* Do nothing now, not a good option */
      return (0);

    case 20:
      L_punt ("Using reserved template  oper:%d\n", tmpl_oper->id);

    default:
      L_punt ("Unknown put_hint oper:%d\n", tmpl_oper->id);
    }				/* switch */

  hint_oper = L_create_new_op (Lop_PBR);
  hint_oper->proc_opc = TAHOEop_BRP;
  hint_oper->src[0] = L_copy_operand (br_hint->target);
  if ((Ltahoe_use_counted_prefetch_hints) && (br_hint->ipwh == IPWH_EXIT))
    /* count -- number of bundles to prefetch */
    hint_oper->src[1] = L_copy_operand (br_hint->num_bundles);
  else
    /* tag -- branch instruction which the brp applies */
    hint_oper->src[1] = L_copy_operand (br_hint->label);

#if 0
  /* an idea that doesn't work well as is... */
  if (br_hint->br_cb != cb)
    {
      br_hint->ih = IH_NONE;
    }				/* if */
#endif

  switch (br_hint->ipwh)
    {
    case IPWH_SPTK:
      TC_SET_BR_WTHR (hint_oper->completers, TC_BR_WTHR_SPTK);
      break;

    case IPWH_DPTK:
      TC_SET_BR_WTHR (hint_oper->completers, TC_BR_WTHR_DPTK);
      break;

    case IPWH_LOOP:
      hint_oper->completers |= TC_BRP_LOOP;
      break;

    case IPWH_EXIT:
      hint_oper->completers |= TC_BRP_EXIT;
      break;

    default:
      L_punt ("BH_insert_hint_in_bundle: Unknown value in br_hint->ipwh (%d)",
	      br_hint->ipwh);
      break;
    }				/* switch */

  if (br_hint->ph == PH_MANY)
    hint_oper->completers |= TC_BRP_MANY;

  switch (br_hint->pvec)
    {
    case PVEC_DC_NT:
      TC_SET_BR_HNT1 (hint_oper->completers, TC_BR_HNT_DC);
      TC_SET_BR_HNT2 (hint_oper->completers, TC_BR_HNT_NT);
      break;

    case PVEC_TK_DC:
      TC_SET_BR_HNT1 (hint_oper->completers, TC_BR_HNT_TK);
      TC_SET_BR_HNT2 (hint_oper->completers, TC_BR_HNT_DC);
      break;

    case PVEC_TK_TK:
      TC_SET_BR_HNT1 (hint_oper->completers, TC_BR_HNT_TK);
      TC_SET_BR_HNT2 (hint_oper->completers, TC_BR_HNT_TK);
      break;

    case PVEC_TK_NT:
      TC_SET_BR_HNT1 (hint_oper->completers, TC_BR_HNT_TK);
      TC_SET_BR_HNT2 (hint_oper->completers, TC_BR_HNT_NT);
      break;

    case PVEC_NT_DC:
      TC_SET_BR_HNT1 (hint_oper->completers, TC_BR_HNT_NT);
      TC_SET_BR_HNT2 (hint_oper->completers, TC_BR_HNT_DC);
      break;

    case PVEC_NT_TK:
      TC_SET_BR_HNT1 (hint_oper->completers, TC_BR_HNT_NT);
      TC_SET_BR_HNT2 (hint_oper->completers, TC_BR_HNT_TK);
      break;

    case PVEC_NT_NT:
      TC_SET_BR_HNT1 (hint_oper->completers, TC_BR_HNT_NT);
      TC_SET_BR_HNT2 (hint_oper->completers, TC_BR_HNT_NT);
      break;

    default:
      TC_SET_BR_HNT1 (hint_oper->completers, TC_BR_HNT_NONE);
      TC_SET_BR_HNT2 (hint_oper->completers, TC_BR_HNT_NONE);
      break;
    }				/* switch */

  if (br_hint->spec_ih != IH_NONE)
    {
      hint_oper->completers |= TC_BRP_IMP;

      stat_tar_hints++;
      br_hint->tar_hinted++;
    }				/* if */
  else
    {
      stat_tac_hints++;
    }				/* else */

  attr = L_new_attr ("hint_id", 1);
  L_set_int_attr_field (attr, 0, hint_oper->id);
  (br_hint->br_oper)->attr = L_concat_attr ((br_hint->br_oper)->attr, attr);

  L_insert_oper_after (cb, second_oper, hint_oper);

  stats->static_distance = BH_static_distance (cb, hint_oper,
					       br_hint->br_cb,
					       br_hint->br_oper, &direction);
  stats->direction = direction;
  if (stats->static_distance >= MAX_BUNDLES_BETWEEN_HINT_AND_BR)
    {
      stat_cb_too_far++;
      result = -1;
    }				/* if */
  else
    {
      BH_add_to_hint_list (br_hint, stats, hint_oper, cb);
      result = 1;
    }				/* else */

  total_inserted++;
  br_hint->num_inserted++;

  return (result);
}				/* BH_insert_hint_in_bundle */


/****************************************************************************
 *
 * routine: BH_insert_branch_label
 * purpose: insert a label on the branch bundle
 * input:   br_hint - branch hint info
 * output: 
 * returns:
 * modified:
 *         
 * note:
 *-------------------------------------------------------------------------*/

static void
BH_insert_branch_label (BH_Br_hint * br_hint)
{
  L_Oper *label_oper, *bundle_oper;

  label_oper = L_create_new_op (Lop_DEFINE);
  label_oper->proc_opc = TAHOEop_NON_INSTR;
  label_oper->dest[0] =
    L_new_macro_operand (TAHOE_MAC_LABEL, L_CTYPE_LLONG, 0);
  label_oper->src[0] = L_copy_operand (br_hint->label);

  bundle_oper = br_hint->br_oper->prev_op;
  while ((bundle_oper = bundle_oper->prev_op))
    {
      if (LT_is_template_op (bundle_oper))
	{
	  br_hint->br_bundle = bundle_oper;
	  break;
	}
    }
  if (!bundle_oper)
    L_punt ("BH_insert_branch_label: couldn't find a template\n");

  L_insert_oper_before (br_hint->br_cb, br_hint->br_bundle, label_oper);
}



/****************************************************************************
 *
 * routine: BH_bundles_from_top
 * purpose: find the number of bundles from top of cb
 * input: bottom_oper - oper to start at
 * output: 
 * returns: number of bundles including one containing bottom_oper
 * modified:
 *         
 * note:
 *-------------------------------------------------------------------------*/

static int
BH_bundles_from_top (L_Oper * bottom_oper)
{
  L_Oper *oper;
  int num_bundles = 0;

  oper = bottom_oper;

  while (oper)
    {
      if (LT_is_template_op (oper))
	num_bundles++;

      oper = oper->prev_op;
    }

  return (num_bundles);
}



/****************************************************************************
 *
 * routine: BH_add_mov2br_instr_hints
 * purpose: add hints to mov2br instruction
 * input:   oper - mov2br oper
 *          mwh - whether hint
 *          ph - prefetch hint
 *          pvec - pref. cancellation vector
 *          ih - importance hint
 * output: 
 * returns:
 * modified: 9/12/02 REK Updating to use new opcode map/completers scheme
 *         
 * note:
 *-------------------------------------------------------------------------*/

static void
BH_add_mov2br_instr_hints (L_Oper * oper, int mwh, int ph, int pvec, int ih)
{
  switch (mwh)
    {
    case MWH_SPTK:
      TC_SET_BR_WTHR (oper->completers, TC_BR_WTHR_SPTK);
      break;

    case MWH_DPTK:
      TC_SET_BR_WTHR (oper->completers, TC_BR_WTHR_DPTK);
      break;

    default:
      break;
    }				/* switch */

  if (ph == PH_MANY)
    oper->completers |= TC_MOV_TOBR_MANY;

  switch (pvec)
    {
    case PVEC_DC_NT:
      TC_SET_BR_HNT1 (oper->completers, TC_BR_HNT_DC);
      TC_SET_BR_HNT2 (oper->completers, TC_BR_HNT_NT);
      break;

    case PVEC_TK_DC:
      TC_SET_BR_HNT1 (oper->completers, TC_BR_HNT_TK);
      TC_SET_BR_HNT2 (oper->completers, TC_BR_HNT_DC);
      break;

    case PVEC_TK_TK:
      TC_SET_BR_HNT1 (oper->completers, TC_BR_HNT_TK);
      TC_SET_BR_HNT2 (oper->completers, TC_BR_HNT_TK);
      break;

    case PVEC_TK_NT:
      TC_SET_BR_HNT1 (oper->completers, TC_BR_HNT_TK);
      TC_SET_BR_HNT2 (oper->completers, TC_BR_HNT_NT);
      break;

    case PVEC_NT_DC:
      TC_SET_BR_HNT1 (oper->completers, TC_BR_HNT_NT);
      TC_SET_BR_HNT2 (oper->completers, TC_BR_HNT_DC);
      break;

    case PVEC_NT_TK:
      TC_SET_BR_HNT1 (oper->completers, TC_BR_HNT_NT);
      TC_SET_BR_HNT2 (oper->completers, TC_BR_HNT_TK);
      break;

    case PVEC_NT_NT:
      TC_SET_BR_HNT1 (oper->completers, TC_BR_HNT_NT);
      TC_SET_BR_HNT2 (oper->completers, TC_BR_HNT_NT);
      break;

    default:
      TC_SET_BR_HNT1 (oper->completers, TC_BR_HNT_NONE);
      TC_SET_BR_HNT2 (oper->completers, TC_BR_HNT_NONE);
      break;
    }				/* switch */

  if (ih == IH_IMP)
    oper->completers |= TC_MOV_TOBR_IMP;

  return;
}				/* BH_add_mov2br_instr_hints */

/****************************************************************************
 *
 * routine: BH_add_br_instr_hints
 * purpose: add hints to branch instruction
 * input:   oper - br oper
 *          bwh - br whether hint
 *          ph - prefetch hint
 *          dh - dealloc hint (clr)
 *          prob - taken probability
 *          path_weight - br executed weight
 * output: 
 * returns:
 * modified: 9/12/02 REK Updating to use new opcode map/completers scheme
 *         
 * note:
 *-------------------------------------------------------------------------*/

static void
BH_add_br_instr_hints (L_Oper * oper, int bwh, int ph, int dh,
		       float prob, float path_weight)
{
  L_Attr *attr;
  ITintmax int_prob, int_weight;

  int_prob = (ITintmax) (prob * 100);
  int_weight = (ITintmax) path_weight;

  switch (bwh)
    {
    case BWH_SPNT:
      TC_SET_BR_WTHR (oper->completers, TC_BR_WTHR_SPNT);
      break;

    case BWH_SPTK:
      TC_SET_BR_WTHR (oper->completers, TC_BR_WTHR_SPTK);
      break;

    case BWH_DPNT:
      TC_SET_BR_WTHR (oper->completers, TC_BR_WTHR_DPNT);
      break;

    case BWH_DPTK:
      TC_SET_BR_WTHR (oper->completers, TC_BR_WTHR_DPTK);
      break;
    }				/* switch */

  switch (ph)
    {
    case PH_FEW:
      oper->completers |= TC_BR_FEW;
      break;

    case PH_MANY:
      oper->completers |= TC_BR_MANY;
      break;

    case PH_NONE:
    default:
      break;
    }				/* switch */

  if (dh == DH_CLR)
    oper->completers |= TC_BR_CLR;

  attr = L_new_attr ("br_info", 2);
  L_set_int_attr_field (attr, 0, int_prob);
  L_set_int_attr_field (attr, 1, int_weight);
  oper->attr = L_concat_attr (oper->attr, attr);
}				/* BH_add_br_instr_hints */



/****************************************************************************
 *
 * routine: BH_add_cb_densities
 * purpose: add densities to all bundles in a cb
 * input:   cb - cb to process
 * output: 
 * returns:
 * modified: 9/12/02 REK Updating to use new opcode map/completers scheme
 *         
 * note:
 *-------------------------------------------------------------------------*/

static void
BH_add_cb_densities (L_Cb * cb)
{
  L_Oper *oper, *template_oper;
  int count = 0, density = 0;

  template_oper = NULL;

  for (oper = cb->first_op; oper; oper = oper->next_op)
    {
      switch (oper->proc_opc)
	{
	case TAHOEop_NON_INSTR:
	  if (LT_is_template_op (oper))
	    {
	      if (template_oper)
		{		/* Save density */
		  if (LT_get_template (template_oper) == MLI)
		    density = 7;	/* Full template */

		  if (template_oper->src[2])
		    LT_set_density (template_oper, density);
		  else
		    LT_new_density (template_oper, density);
		}		/* if */
	      template_oper = oper;
	      density = 0;
	      count = 3;
	    }			/* if */
	  break;
	case TAHOEop_NOP_B:
	case TAHOEop_NOP_F:
	case TAHOEop_NOP_I:
	case TAHOEop_NOP_M:
	case TAHOEop_NOP_X:
	  count--;
	  break;
	default:
	  count--;
	  density |= (1 << count);
	}			/* switch */
    }				/* for oper */

  if (template_oper)
    {				/* Save density */
      if (LT_get_template (template_oper) == MLI)
	density = 7;		/* Full template */

      if (template_oper->src[2])
	LT_set_density (template_oper, density);
      else
	LT_new_density (template_oper, density);
    }				/* if */
}				/* BH_add_cb_densities */


/****************************************************************************
 *
 * routine: BH_add_densities
 * purpose: add densities to all bundles in a function
 * input:   fn - function to process
 * output: 
 * returns:
 * modified:
 *         
 * note:
 *-------------------------------------------------------------------------*/

static void
BH_add_densities (L_Func * fn)
{
  L_Cb *cb;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    BH_add_cb_densities (cb);
}


/****************************************************************************
 *
 * routine: BH_fix_flow_ccs
 * purpose: verify flow cc's
 * input:   fn - function to process
 * output: 
 * returns:
 * modified:
 *         
 * note:
 *-------------------------------------------------------------------------*/

static void
BH_fix_flow_ccs (L_Func * fn)
{
  L_Flow *flow;
  L_Cb *cb;
  L_Oper *oper;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      flow = cb->dest_flow;
      oper = cb->first_op;
      while (flow != NULL)
	{
	  while (oper != NULL && !(LT_is_cond_br (oper) ||
				   L_check_branch_opcode (oper)))
	    oper = oper->next_op;

	  if (!oper)
	    break;

	  if (oper->opc == Lop_JUMP_RG ||
	      oper->opc == Lop_JUMP_RG_FS)
	    return;

	  if (((flow->dst_cb)->id) == (L_find_branch_dest (oper)->id))
	    {
	      flow->cc = 1;
	    }
	  else
	    {
	      fprintf (stderr, "flow targ: %d  branch targ: %d\n",
		       (flow->dst_cb)->id, (((oper->src[0])->value.cb)->id));
	      L_punt ("BH_fix_flow_ccs: major problem in cb %d\n", cb->id);
	    }
	  flow = flow->next_flow;
	  oper = oper->next_op;
	}
      if (flow != NULL)
	{
	  flow->cc = 0;
	  if (flow->next_flow != NULL)
	    L_punt
	      ("BH_fix_flow_ccs: Unexpected flow at end of flow list in "
	       "cb %d\n", cb->id);
	}
    }
}


/****************************************************************************
 *
 * routine: BH_add_instr_hints_only()
 * purpose: add br instr hints only (no brp's)
 * input:   fn - function to process
 * output: 
 * returns:
 * modified: Bob McGowan - 5/20/97 - bug fix for jumps and returns
 * note:
 *-------------------------------------------------------------------------*/

static void
BH_add_instr_hints_only (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *oper;
  int bbb_context = 0;

  double path_weight;
  L_Flow *flow;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      path_weight = cb->weight;
      flow = cb->dest_flow;
      for (oper = cb->first_op; oper; oper = oper->next_op)
	{
	  if (LT_is_template_op (oper))
	    {
	      bbb_context = (LT_get_template (oper) == BBB);
	    }
	  else if (LT_is_mov_to_br (oper))
	    {
	      BH_specify_hint_on_mov2br (cb, oper);
	    }
	  else if (LT_is_indir_br (oper))
	    {
	      TC_SET_BR_WTHR (oper->completers, TC_BR_WTHR_DPTK);
	      if (oper->next_op)
		L_warn ("BH_add_instr_hints_only: op follows an indir br");
	      break;
	    }
	  else if (LT_is_call_br (oper) || LT_is_cond_br (oper) ||
		   LT_is_ret_br (oper))
	    {
	      if (LT_is_cond_br (oper) && !flow)
		L_punt ("BH_add_instr_hints_only: No flow for branch");

	      BH_specify_hint_on_br (cb, oper, &flow, &path_weight);

	      if (bbb_context)
		{
		  int bwh;

		  bwh = TC_GET_BR_WTHR (oper->completers);

		  switch (bwh)
		    {
		    case TC_BR_WTHR_SPTK:
		      TC_SET_BR_WTHR (oper->completers, TC_BR_WTHR_DPTK);
		      break;
		    case TC_BR_WTHR_SPNT:
		      TC_SET_BR_WTHR (oper->completers, TC_BR_WTHR_DPNT);
		      break;
		    default:
		      break;
		    }
		}
	    }
	  else if (L_check_branch_opcode (oper))
	    {
	      flow = flow->next_flow;
	    }
	}
    }
  return;
}


/****************************************************************************
 *
 * routine: BH_specify_hint_on_mov2br
 * purpose: determine how to hint a mov2br instruction
 * input:   cb - cb containing mov2br
 *          oper - mov2br oper
 * output: 
 * returns:
 * modified:
 *         
 * note:
 *-------------------------------------------------------------------------*/

static void
BH_specify_hint_on_mov2br (L_Cb * cb, L_Oper * oper)
{

  /* doesn't even look at the branch yet to add a mwh hint. */

  /* MWH_NONE != MWH_SPTK, so no prefetching is done!!! */

  BH_add_mov2br_instr_hints (oper, MWH_NONE, PH_FEW, PVEC_NONE, IH_NONE);

}




/****************************************************************************
 *
 * routine: BH_specify_hint_on_br
 * purpose: determine how to hint a br instruction
 * input:   cb - cb containing br
 *          oper - br oper
 *          flow - flow for this br
 *          path_weight - execution weight for this branch
 * output: 
 * returns: path_weight is changed to be the weight of the
 *          block after the branch.  
 *          If the branch is a conditional branch, flow is updated
 *          to the next flow.
 * modified: JEP 4/3 - changed path_weight to a pointer
 *           REK 9/12/02 Updating to use new opcode map/completers scheme.
 *         
 * note:
 *-------------------------------------------------------------------------*/

static void
BH_specify_hint_on_br (L_Cb * cb, L_Oper * oper, L_Flow ** flow,
		       double *path_weight)
{
  double prob, weight, bweight, fweight;
  int bwh, ph, dh, do_prefetch = 0, is_predicated;
  int use_prob = 0;

  bweight = *path_weight;

  is_predicated = L_is_predicated (oper);

  switch (oper->proc_opc)
    {
    case TAHOEop_BR_COND:
    case TAHOEop_BR_CLOOP:
    case TAHOEop_BR_CTOP:
    case TAHOEop_BR_CEXIT:
    case TAHOEop_BR_WTOP:
    case TAHOEop_BR_WEXIT:

      if (is_predicated)
	{
	  fweight = (*flow)->weight;

	  prob = (bweight > 0.0) && (fweight > 0.0) ?
	    (fweight / bweight) : 0.0;

	  /* If the branch's weight is leq 0.0,
	     then no valid prob could be computed. */

	  if (Ltahoe_correct_profile && (prob >= 0.0) && (prob <= 1.00) &&
	      (bweight > 0.0))
	    use_prob = 1;

	  bweight = (bweight > fweight) ? bweight - fweight : 0.0;

	  if ((*flow)->dst_cb != oper->src[0]->value.cb)
	    L_punt ("BH_specify_hint_on_br: flow / br mismatch");

	  if (prob >= 0.5)
	    {
	      bwh = (use_prob && (prob >= Ltahoe_dp_upper_prob)) ?
		BWH_SPTK : BWH_DPTK;

	      if (Ltahoe_use_many_hint_on_all_branches ||
		  ((Ltahoe_use_streaming_only ||
		    (Ltahoe_insert_branch_hints && Ltahoe_aggressive_hints &&
		     !Ltahoe_use_counted_prefetch_hints)) &&
		   (BH_num_bundles_in_branch (oper) > 
		    PF_MANY_HINT_MIN_SIZE)))
		ph = PH_MANY;
	      else
		ph = PH_NONE;
	    }			/* if */
	  else
	    {
	      bwh = (use_prob && (prob <= Ltahoe_dp_lower_prob)) ?
		BWH_SPNT : BWH_DPNT;
	      ph = PH_NONE;
	    }			/* else */
	  weight = *path_weight;
	}			/* if */
      else
	{
	  bwh = BWH_SPTK;

	  if (Ltahoe_use_many_hint_on_all_branches ||
	      ((Ltahoe_use_streaming_only ||
		(Ltahoe_insert_branch_hints && Ltahoe_aggressive_hints &&
		 !Ltahoe_use_counted_prefetch_hints)) &&
	       (BH_num_bundles_in_branch (oper) > PF_MANY_HINT_MIN_SIZE)))
	    ph = PH_MANY;
	  else
	    ph = PH_NONE;

	  prob = 1.0;
	  weight = bweight;
	  bweight = 0.0;
	}			/* else */

      /* don't prefetch if this is a backedge */

      if ((oper->src[0]->value.cb == cb) ||
	  (L_in_cb_DOM_set (cb, oper->src[0]->value.cb->id)))
	ph = PH_NONE;

      dh = DH_NONE;

      *flow = (*flow)->next_flow;
      *path_weight = bweight;

      BH_add_br_instr_hints (oper, bwh, ph, dh, prob, weight);
      break;

    case TAHOEop_BR_CALL:
      do_prefetch = (Ltahoe_use_many_hint_on_call && !is_predicated) ||
	(!Ltahoe_use_counted_prefetch_hints &&
	 (BH_num_bundles_in_call (oper) > PF_MANY_HINT_MIN_SIZE));

      dh = DH_NONE;
      ph = do_prefetch ? PH_MANY : PH_NONE;
      bwh = is_predicated ? BWH_DPTK : BWH_SPTK;

      BH_add_br_instr_hints (oper, bwh, ph, dh, 1.0, bweight);
      break;

    case TAHOEop_BR_RET:
      dh = DH_NONE;
      ph = (Ltahoe_use_many_hint_on_return && !is_predicated) ?
	PH_MANY : PH_NONE;
      bwh = is_predicated ? BWH_DPTK : BWH_SPTK;

      BH_add_br_instr_hints (oper, bwh, ph, dh, 1.0, bweight);
      break;

    default:
      L_punt ("BH_specify_hint_on_br: Tried to place a hint "
	      "on a non-branch instruction (op %d)\n", oper->id);
      break;
    }				/* switch */
  return;
}				/* BH_specify_hint_on_br */

/****************************************************************************
 *
 * routine: BH_suggest_hint_instr
 * purpose: suggest the prefered hint and add to the hint list
 * input:   fn_name - function name
 *          cb - cb containing br
 *          oper - br oper
 *          flow - flow for this br
 *          path_weight - execution weight for br
 * output: 
 * returns:
 * modified: Kevin Crozier 6/17/98 -- Adding BRP Manys and such
 *           REK 9/12/02 Updating to use new opcode map/completers scheme.
 *         
 * note:
 *-------------------------------------------------------------------------*/

static void
BH_suggest_hint_instr (char *fn_name, L_Cb * cb, L_Oper * oper,
		       L_Flow * flow, double path_weight)
{
  double prob = 0.0, weight = 0.0;
  int type = 0, ipwh = 0, ph = 0, pvec = 0, ih = 0, num_bundles = 0;
  int hint_flag = 0;

  /* Don't hint unimportant branches */
  if (path_weight <= MIN_HINTED_PATH_WEIGHT)
    {
      lowweighttossed++;
      return;
    }				/* if */

  switch (oper->proc_opc)
    {
    case TAHOEop_BR_CALL:
      if (L_is_predicated (oper))
	{
	  if (path_weight > TAR_WEIGHT_THRESHOLD)
	    {
	      type = BR_COND;
	      ipwh = IPWH_DPTK;
	      ph = PH_FEW;
	      pvec = PVEC_DC_DC;
	      ih = IH_NONE;	/* don't use tar since call prob is unknown */
	      prob = 1.0;
	      num_bundles = BH_num_bundles_in_call (oper);
	      hint_flag = 1;
	      weight = path_weight;
	    }			/* if */
	  else if (path_weight > TAC_WEIGHT_THRESHOLD)
	    {
	      type = BR_COND;
	      ipwh = IPWH_DPTK;
	      ph = PH_FEW;
	      pvec = PVEC_DC_DC;
	      ih = IH_NONE;
	      prob = 1.0;
	      num_bundles = BH_num_bundles_in_call (oper);
	      hint_flag = 1;
	      weight = path_weight;
	    }			/* else if */
	}			/* if */

      else if (path_weight > TAC_WEIGHT_THRESHOLD)
	{
	  type = BR_CALL;
	  ipwh = IPWH_SPTK;
	  ph = PH_FEW;
	  pvec = PVEC_DC_DC;
	  ih = IH_IMP;
	  prob = 1.0;
	  num_bundles = BH_num_bundles_in_call (oper);
	  hint_flag = 1;
	  weight = path_weight;
	}			/* else if */

      if (Ltahoe_use_many_hint_on_brp ||
	  (num_bundles > BRP_MANY_HINT_MIN_SIZE))
	ph = PH_MANY;

      if (Ltahoe_use_imp_hint_on_brp)
	ih = IH_IMP;

      if (Ltahoe_use_counted_prefetch_hints &&
	  (num_bundles > BRP_CPREFETCH_MIN_SIZE))
	{
	  ph = PH_NONE;
	  ipwh = IPWH_EXIT;
	  ih = IH_NONE;
	}			/* if */

      if (hint_flag)
	BH_new_br_hint (fn_name, cb, oper, num_bundles, type, ipwh, ph,
			pvec, ih, prob, weight);

      break;

    case TAHOEop_BR_COND:
    case TAHOEop_BR_CLOOP:
    case TAHOEop_BR_CTOP:
    case TAHOEop_BR_CEXIT:
    case TAHOEop_BR_WTOP:
    case TAHOEop_BR_WEXIT:
      if (L_is_predicated (oper))
	{
	  prob = (path_weight > 0.0) ? (flow->weight / path_weight) : 0.0;

	  if (Ltahoe_correct_profile &&
	      (prob >= TAR_PROB_THRESHOLD) &&
	      (path_weight > TAR_WEIGHT_THRESHOLD))
	    {
	      type = BR_COND;
	      ipwh = IPWH_DPTK;
	      ph = PH_FEW;
	      pvec = PVEC_DC_DC;
	      ih = IH_IMP;
	      num_bundles = BH_num_bundles_in_branch (oper);
	      hint_flag = 1;
	      weight = path_weight;
	    }			/* if */

	  else if ((prob >= TAC_PROB_THRESHOLD) &&
		   (path_weight > TAC_WEIGHT_THRESHOLD))
	    {
	      type = BR_COND;
	      ipwh = IPWH_DPTK;
	      ph = PH_FEW;
	      pvec = PVEC_DC_DC;
	      ih = IH_NONE;
	      num_bundles = BH_num_bundles_in_branch (oper);
	      hint_flag = 1;
	      weight = path_weight;
	    }			/* else if */

	  path_weight -= flow->weight;
	}			/* if */
      else
	{
	  type = BR_COND;
	  ipwh = IPWH_SPTK;
	  ph = PH_FEW;
	  pvec = PVEC_DC_DC;
	  ih = IH_IMP;
	  prob = 1.0;
	  num_bundles = BH_num_bundles_in_branch (oper);
	  hint_flag = 1;
	  weight = path_weight;

	  path_weight = 0;
	}			/* else */

      if (Ltahoe_use_many_hint_on_brp ||
	  (num_bundles > BRP_MANY_HINT_MIN_SIZE))
	ph = PH_MANY;

      if (Ltahoe_use_imp_hint_on_brp)
	ih = IH_IMP;

      if ((Ltahoe_use_counted_prefetch_hints) &&
	  (num_bundles > BRP_CPREFETCH_MIN_SIZE))
	{
	  ph = PH_NONE;
	  ipwh = IPWH_EXIT;
	  ih = IH_NONE;
	}			/* if */

      if (hint_flag)
	BH_new_br_hint (fn_name, cb, oper, num_bundles, type, ipwh, ph,
			pvec, ih, prob, weight);

      flow = flow->next_flow;
      break;

    case TAHOEop_BR_RET:
#ifdef HINT_DEBUG
      fprintf (stderr,
	       "BH_specify_hint_on_br: Tried to hint a return(op%d)\n",
	       oper->id);
#endif
      break;

    default:
      fprintf (stderr, "BH_specify_hint_on_br: Tried to insert a hint\n"
	       "for a non-branch instruction (op %d)\n", oper->id);
      break;

    }				/* switch */

  return;
}				/* BH_suggest_hint_instr */

/****************************************************************************
 *
 * routine: BH_split_cb
 * purpose: splits the given cb at the given oper into 2 cbs.
 * input:   cb - cb to split
 *          oper - location to split the cb at (this oper and below)
 * output: 
 * returns:
 * modified:
 *         
 * note:
 *-------------------------------------------------------------------------*/
static L_Cb *
BH_split_cb (L_Cb * cb, L_Oper * oper)
{
  L_Cb *new_cb;
  L_Flow *next_flow, *flow;
  L_Oper *next_oper;

  flow = NULL;
  new_cb = L_new_cb (++L_fn->max_cb_id);
  new_cb->weight = cb->weight;
  L_insert_cb_after (L_fn, cb, new_cb);
  while (oper)
    {
      if ((LT_is_cond_br (oper)) && (flow == NULL))
	flow = L_find_flow_for_branch (cb, oper);
      next_oper = oper->next_op;
      L_remove_oper (cb, oper);
      L_insert_oper_after (new_cb, new_cb->last_op, oper);
      oper = next_oper;
    }

  while (flow)
    {
      next_flow = flow->next_flow;
      cb->dest_flow = L_remove_flow (cb->dest_flow, flow);
      new_cb->dest_flow = L_concat_flow (new_cb->dest_flow, flow);
      L_change_src (new_cb->dest_flow, cb, new_cb);
      flow = next_flow;
    }

  /* make the fall thru to the new cb */
  flow = L_new_flow (0, cb, new_cb, cb->weight);
  cb->dest_flow = L_concat_flow (cb->dest_flow, flow);
  L_rebuild_src_flow (L_fn);

  return new_cb;
}

/****************************************************************************
 *
 * routine: BH_insert_advanced_hint
 * purpose: insert hints from list
 * input:   fn - function
 *          hints - list of hints to insert
 *          num_hints - number of hints in list
 * output: 
 * returns:
 * modified:
 *         
 * note:
 *-------------------------------------------------------------------------*/
static void
BH_insert_advanced_hint (BH_Br_hint * hint)
{
  L_Cb *cb;
  L_Oper *oper, *brp_oper = NULL;
  L_Flow *flow;
  BH_path_info *path_stats;
  int got_it, found;
  path_stats = (BH_path_info *) calloc (1, sizeof (BH_path_info));
#ifdef HINT_DEBUG
  fprintf (stderr, "Trying to insert an advanced hint...\n");
#endif
  cb = hint->br_cb;
  oper = hint->br_oper;
  flow = L_find_last_flow (cb->dest_flow);

  while (oper)
    {
      if (LT_is_template_op (oper))
	break;
      oper = oper->next_op;
    }
  if (oper != NULL)
    cb = BH_split_cb (cb, oper);
  else
    {
      if (flow->dst_cb != cb->next_cb)
	return;
      cb = cb->next_cb;
    }

  /* insert the brp in the block below the branch */
  hint->target->value.cb = cb;
  got_it = BH_insert_hint_in_block (cb, cb->last_op, hint,
				    path_stats, -1, 1, 0);
#ifdef HINT_DEBUG
  fprintf (stderr, "New cb for %d, got_it %d..\n", cb->id, got_it);
#endif
/* go find the brp we inserted and fix up a target */
  if (got_it == 0)
    {
      found = 0;
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
	{
	  if (LT_is_template_op (oper) && (found == 1))
	    break;

	  if (LT_is_brp (oper) &&
	      (oper->src[0]->value.cb == hint->target->value.cb))
	    {
	      brp_oper = oper;
	      found = 1;
	    }
	}
      if (oper != NULL)
	cb = BH_split_cb (cb, oper);
      else
	{
	  flow = L_find_last_flow (cb->dest_flow);
	  cb = flow->dst_cb;
	}
      brp_oper->src[0]->value.cb = cb;
      hint->br_cb = cb;
      hint->br_oper = cb->first_op->next_op->next_op;
      BH_insert_branch_label (hint);
    }
  free (path_stats);
}


/****************************************************************************
 *
 * routine: BH_insert_branch_hints
 * purpose: insert hints from list
 * input:   fn - function
 *          hints - list of hints to insert
 *          num_hints - number of hints in list
 * output: 
 * returns:
 * modified:
 *         
 * note:
 *-------------------------------------------------------------------------*/

static void
BH_insert_branch_hints (L_Func * fn, BH_Br_hint * hints[])
{
  BH_Br_hint *br_hint;
  int cnt;
  int success;
  int fe_cycles;
  BH_path_info *path_stats;
  L_Attr *tar_attr;

  cbs_looked_at_set = Set_dispose (cbs_looked_at_set);

  for (cnt = 0; cnt < br_hint_cnt; cnt++)
    {
      br_hint = hints[cnt];
      path_stats = (BH_path_info *) calloc (1, sizeof (BH_path_info));

      if (!br_hint)
	{
	  fprintf (stderr,
		   "\n Hit a null br_hint in the sorted array!! <-----\n");
	  break;
	}

#ifdef HINT_DEBUG2
      if (Ltahoe_print_hint_info)
	{
	  fprintf (stderr,
		   "\nTrying to insert hint for oper %d with distance %d\n",
		   br_hint->br_oper->id,
		   Ltahoe_min_fe_cycles_for_prefetch_brp);
	}
#endif

      cbs_looked_at_set = Set_add (cbs_looked_at_set, br_hint->br_cb->id);

      if (Ltahoe_insert_with_full_coverage && Ltahoe_dont_expand_for_hints)
	{
	  for (fe_cycles = PREFETCH_FE_CYCLES, success = -1;
	       (fe_cycles >= 0); fe_cycles = fe_cycles - 2)
	    {
	      br_hint->paths_tried = 0;
	      br_hint->paths_success = 0;
	      success =
		BH_insert_hint_in_paths (br_hint->br_cb, br_hint->br_oper,
					 br_hint, path_stats,
					 fe_cycles, RECURSION_LEVEL, 1, 1);
	      if ((success > 0) &&
		  (br_hint->paths_tried == br_hint->paths_success))
		break;
#ifdef HINT_DEBUG
	      else if ((success > 0)
		       && (br_hint->paths_tried != br_hint->paths_success))
		printf ("Could have inserted but didn't %d != %d\n",
			br_hint->paths_tried, br_hint->paths_success);
	      else
		printf ("No insert because of bad success %d != %d\n",
			br_hint->paths_tried, br_hint->paths_success);
#endif
	    }
	  if (fe_cycles > 0)
	    {
#ifdef HINT_DEBUG
	      printf ("Inserting %d != %d\n", br_hint->paths_tried,
		      br_hint->paths_success);
#endif
	      success =
		BH_insert_hint_in_paths (br_hint->br_cb, br_hint->br_oper,
					 br_hint, path_stats,
					 fe_cycles, RECURSION_LEVEL, 1, 0);
	    }
	}
      else if (Ltahoe_insert_with_retries)
	{
	  for (fe_cycles = PREFETCH_FE_CYCLES, success = -1;
	       (fe_cycles >= 0); fe_cycles = fe_cycles - 2)
	    {
	      success =
		BH_insert_hint_in_paths (br_hint->br_cb, br_hint->br_oper,
					 br_hint, path_stats,
					 fe_cycles, RECURSION_LEVEL, 1, 0);
	      if (success > 0)
		break;
	    }
	}
      else
	{
	  success = BH_insert_hint_in_paths (br_hint->br_cb, br_hint->br_oper,
					     br_hint, path_stats,
					     PREFETCH_FE_CYCLES,
					     RECURSION_LEVEL, 1, 0);
	}

#ifdef HINT_DEBUG
      if (success > 0)
	printf ("Inserted brp with fe_cycles = %d\n", fe_cycles);
      else
	printf ("Couldn't insert brp with fe_cycles = %d\n", fe_cycles);
#endif

      if (!success && Ltahoe_advanced_prefetch &&
	  Ltahoe_mckinley_hints && br_hint->advanced_hint)
	BH_insert_advanced_hint (br_hint->advanced_hint);

      if (br_hint->num_inserted > 0)
	{
	  BH_insert_branch_label (br_hint);
#ifdef HINT_DEBUG2
	  fprintf (stderr, "Inserted label for br %d\n",
		   br_hint->br_oper->id);
	  BH_print_br_stats (br_hint);
#endif

	  if (br_hint->tar_hinted)
	    {
	      tar_attr = L_new_attr ("TAR_HINTED", 0);
	      br_hint->br_bundle->attr =
		L_concat_attr (br_hint->br_bundle->attr, tar_attr);
#ifdef HINT_DEBUG2
	      fprintf (stderr, "Inserted tar attr for bundle %d\n",
		       br_hint->br_bundle->id);
#endif
	    }

	}

#ifdef HINT_DEBUG2
      else
	{
	  BH_print_br_stats (br_hint);
	  fprintf (stderr, "Didn't insert any hints for this br\n");
	}
#endif

      cbs_looked_at_set = Set_dispose (cbs_looked_at_set);
      free (path_stats);

    }
}

/****************************************************************************
 *
 * routine: BH_print_stats()
 * purpose: print stats
 * input:
 * output:
 * returns:
 * modified:
 *                         
 * note:
 *-------------------------------------------------------------------------*/

static void
BH_print_stats (BH_Br_hint * hints[])
{

  BH_Br_hint *br_hint;
  BH_hints_list *hint_list;
  BH_path_info total_stats = { NULL };
  int cnt, total = 0;
  int cross_same_call = 0;
  int paths_success = 0, paths_tried = 0;


  fprintf (stderr, "*********************************************\n");
  fprintf (stderr, "total number of branches:      %d\n", num_branches);
  fprintf (stderr, "calls:                         %d\n", calls);
  fprintf (stderr, "conds:                         %d\n", conds);
  fprintf (stderr, "conds taken greater than 80 percent %d\n",
	   hightakebranches);
  fprintf (stderr, "returns:                         %d\n", returns);
  fprintf (stderr, "branches tossed with low weight %d\n", lowweighttossed);
  fprintf (stderr, "num branches tried to hint:    %d\n", br_hint_cnt);
  fprintf (stderr, "num of advanced hints           %d\n",
	   advanced_br_hint_cnt);
  fprintf (stderr, "total brps inserted:           %d\n", total_inserted);
  fprintf (stderr, "     tar hints:        %d\n", stat_tar_hints);
  fprintf (stderr, "     tac hints:        %d\n", stat_tac_hints);
  fprintf (stderr, "*********************************************\n");

  if (!hints || !br_hint_cnt)
    return;


  for (cnt = 0; cnt < br_hint_cnt; cnt++)
    {
      br_hint = hints[cnt];

      cross_same_call += br_hint->cross_same_call;
      paths_success += br_hint->paths_success;
      paths_tried += br_hint->paths_tried;

      hint_list = br_hint->list;

      while (hint_list)
	{
	  total++;
	  total_stats.num_br_crossed += hint_list->path_info.num_br_crossed;
	  total_stats.num_calls_crossed +=
	    hint_list->path_info.num_calls_crossed;
	  total_stats.num_cbs_on_path += hint_list->path_info.num_cbs_on_path;
	  total_stats.static_distance += hint_list->path_info.static_distance;
	  total_stats.fe_cycles_away += hint_list->path_info.fe_cycles_away;
	  total_stats.expanded_bundle += hint_list->path_info.expanded_bundle;
	  total_stats.brps_on_path += hint_list->path_info.brps_on_path;
	  total_stats.tars_on_path += hint_list->path_info.tars_on_path;
	  hint_list = hint_list->next_hint;
	}
    }

  fprintf (stderr, "Path stats:\n");
  fprintf (stderr, "   Ratio of paths inserted to tried:  %d / %d\n",
	   paths_success, paths_tried);
  fprintf (stderr, "   Paths stopped by a call to same func:   %d\n",
	   cross_same_call);
  fprintf (stderr, "   number of merged paths:                 %d\n",
	   stat_merged_path);
  fprintf (stderr, "   number of paths that hit max_recursion: %d\n",
	   stat_max_recursion);
  fprintf (stderr, "   number of cb's hit that were too far:   %d\n",
	   stat_cb_too_far);
  fprintf (stderr, "   hit max number of expansions for a cb:  %d\n",
	   stat_max_expand);
  fprintf (stderr, "*********************************************\n");

  fprintf (stderr, "Averages per brp inserted (%d inserted):\n", total);
  fprintf (stderr, "   avg num_br_crossed:       %6.2f\n",
	   total ? (float) total_stats.num_br_crossed / (float) total : 0);
  fprintf (stderr, "   avg num_calls_crossed:    %6.2f\n",
	   total ? (float) total_stats.num_calls_crossed / (float) total : 0);
  fprintf (stderr, "   avg num_cbs_on_path:      %6.2f\n",
	   total ? (float) total_stats.num_cbs_on_path / (float) total : 0);
  fprintf (stderr, "   avg static_distance:      %6.2f\n",
	   total ? (float) total_stats.static_distance / (float) total : 0);
  fprintf (stderr, "   avg expanded_bundle:      %6.2f (%d tot)\n",
	   total ? (float) total_stats.expanded_bundle / (float) total
	   : 0, total_stats.expanded_bundle);
  fprintf (stderr, "*********************************************\n");


  fprintf (stderr, "Questionable stats:\n");
  fprintf (stderr, "   avg tars_on_path:        %6.2f\n",
	   total ? (float) total_stats.tars_on_path / (float) total : 0);
  fprintf (stderr, "   avg brps_on_path:        %6.2f\n",
	   total ? (float) total_stats.brps_on_path / (float) total : 0);
  fprintf (stderr, "   total attempted to insert:     %d\n", total_attempted);
  fprintf (stderr, "   avg fe_cycles_away:       %6.2f\n",
	   total ? (float) total_stats.fe_cycles_away / (float) total : 0);
  fprintf (stderr, "*********************************************\n");

}



/****************************************************************************
 *
 * routine: BH_print_br_stats()
 * purpose: print branch level stats
 * input:
 * output: 
 * returns:
 * modified: 
 *                         
 * note:
 *-------------------------------------------------------------------------*/

#ifdef HINT_DEBUG2
static void
BH_print_br_stats (BH_Br_hint * br_hint)
{

  fprintf (stderr, "br%d paths: %d / %d\n", br_hint->br_oper->id,
	   br_hint->paths_success, br_hint->paths_tried);
  fprintf (stderr, "paths stopped by crossing call to same func: %d\n",
	   br_hint->cross_same_call);

}
#endif
