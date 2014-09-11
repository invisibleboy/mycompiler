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
/*****************************************************************************
 * phase2_reg.h                                                              *
 * ------------------------------------------------------------------------- *
 * Register allocation                                                       *
 *                                                                           *
 * AUTHORS: D.A. Connors, J. Pierce, J.W. Sias                               *
 *****************************************************************************/
/* 09/12/02 REK Updating file to use the new opcode map and completers
 *              scheme.
 */

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_mii.h>
#include <Lcode/l_softpipe.h>
#include "ltahoe_main.h"
#include "ltahoe_completers.h"
#include "phase2_func.h"
#include "phase1_func.h"
#include <Lcode/r_regalloc.h>
#include "phase2_reg.h"
#include "ltahoe_op_query.h"

#define LOAD_COST       2.0
#define STORE_COST      2.0

/* Globals used by phase2_func and phase2_reg */

/* Arrays of callee registers which must be saved to memory */
int *callee_int_array = NULL;
int *callee_flt_array = NULL;
int *callee_pred_array = NULL;
int *callee_btr_array = NULL;

/* Numbers of callee registers which must be saved to memory */
static int stacked_callee_int_num;
int callee_int_num;
int callee_flt_num;
int callee_pred_num;
int callee_btr_num;

/* Globals used only by phase2_reg */

static Set callee_int_set;
static Set caller_int_set;
static Set callee_int_macro_set;
static Set caller_int_macro_set;
static Set callee_float_set;
static Set caller_float_set;
static Set caller_float_macro_set;
static Set callee_double_set;
static Set caller_double_set;
static Set caller_double_macro_set;
static Set callee_predicate_set;
static Set caller_predicate_set;
static Set callee_branch_set;
static Set caller_branch_set;

#define NUM_CALLER_INT_REG         (18)
#define NUM_CALLEE_INT_STATIC_REG  (4)

#define NUM_CALLER_FLOAT_REG       (98)
#define NUM_CALLEE_FLOAT_REG       (20)
#define NUM_CALLER_FLOAT_MAC       (8)
#define NUM_CALLER_DOUBLE_REG      NUM_CALLER_FLOAT_REG
#define NUM_CALLEE_DOUBLE_REG      NUM_CALLEE_FLOAT_REG
#define NUM_CALLER_DOUBLE_MAC      NUM_CALLER_FLOAT_MAC

#define NUM_CALLER_PRED_REG    (10)
#define NUM_CALLEE_PRED_REG    (53)
#define NUM_CALLER_BRANCH_REG  (2)
#define NUM_CALLEE_BRANCH_REG  (5)

static int caller_int_reg_map[NUM_CALLER_INT_REG] = {
  INT_REG_BASE + 14, INT_REG_BASE + 15, INT_REG_BASE + 16, INT_REG_BASE + 17,
  INT_REG_BASE + 18, INT_REG_BASE + 19, INT_REG_BASE + 20, INT_REG_BASE + 21,
  INT_REG_BASE + 22, INT_REG_BASE + 23, INT_REG_BASE + 24, INT_REG_BASE + 25,
  INT_REG_BASE + 26, INT_REG_BASE + 27, INT_REG_BASE + 28, INT_REG_BASE + 29,
  INT_REG_BASE + 30, INT_REG_BASE + 31
};


#ifdef USE_STATIC_CALLEE
/* There are a few static callee save registers but these are not utilized at
   this time.  It may not be profitable.  The code to make these work is
   not in place. */
static int callee_int_reg_map[NUM_CALLEE_INT_STATIC_REG
			      + NUM_INT_STACKED_REG] = {
  INT_REG_BASE + 4, INT_REG_BASE + 5, INT_REG_BASE + 6, INT_REG_BASE + 7
};

#else
#undef NUM_CALLEE_INT_STATIC_REG
#define NUM_CALLEE_INT_STATIC_REG 0
static int callee_int_reg_map[NUM_INT_STACKED_REG];
#endif

/* All stacked could be used as callee save.  The actual number for
 * each function varies depending on how many input and output
 * registers are used.  Only the static register numbers are listed
 * here, the rest are filled in later. 
 */

static int callee_int_reg_map[NUM_INT_STACKED_REG];

static int callee_int_macro_map[MAX_INT_INPUT_REGS];
static int caller_int_macro_map[MAX_INT_OUTPUT_REGS + MAX_INT_RETURN_REGS];

/*
 *             Floating point register mappings
 *
 */

static int caller_float_reg_map[NUM_CALLER_FLOAT_REG] = {
  FLOAT_REG_BASE + 6, FLOAT_REG_BASE + 7, FLOAT_REG_BASE + 32,
  FLOAT_REG_BASE + 33, FLOAT_REG_BASE + 34, FLOAT_REG_BASE + 35,
  FLOAT_REG_BASE + 36, FLOAT_REG_BASE + 37, FLOAT_REG_BASE + 38,
  FLOAT_REG_BASE + 39, FLOAT_REG_BASE + 40, FLOAT_REG_BASE + 41,
  FLOAT_REG_BASE + 42, FLOAT_REG_BASE + 43, FLOAT_REG_BASE + 44,
  FLOAT_REG_BASE + 45, FLOAT_REG_BASE + 46, FLOAT_REG_BASE + 47,
  FLOAT_REG_BASE + 48, FLOAT_REG_BASE + 49, FLOAT_REG_BASE + 50,
  FLOAT_REG_BASE + 51, FLOAT_REG_BASE + 52, FLOAT_REG_BASE + 53,
  FLOAT_REG_BASE + 54, FLOAT_REG_BASE + 55, FLOAT_REG_BASE + 56,
  FLOAT_REG_BASE + 57, FLOAT_REG_BASE + 58, FLOAT_REG_BASE + 59,
  FLOAT_REG_BASE + 60, FLOAT_REG_BASE + 61, FLOAT_REG_BASE + 62,
  FLOAT_REG_BASE + 63, FLOAT_REG_BASE + 64, FLOAT_REG_BASE + 65,
  FLOAT_REG_BASE + 66, FLOAT_REG_BASE + 67, FLOAT_REG_BASE + 68,
  FLOAT_REG_BASE + 69, FLOAT_REG_BASE + 70, FLOAT_REG_BASE + 71,
  FLOAT_REG_BASE + 72, FLOAT_REG_BASE + 73, FLOAT_REG_BASE + 74,
  FLOAT_REG_BASE + 75, FLOAT_REG_BASE + 76, FLOAT_REG_BASE + 77,
  FLOAT_REG_BASE + 78, FLOAT_REG_BASE + 79, FLOAT_REG_BASE + 80,
  FLOAT_REG_BASE + 81, FLOAT_REG_BASE + 82, FLOAT_REG_BASE + 83,
  FLOAT_REG_BASE + 84, FLOAT_REG_BASE + 85, FLOAT_REG_BASE + 86,
  FLOAT_REG_BASE + 87, FLOAT_REG_BASE + 88, FLOAT_REG_BASE + 89,
  FLOAT_REG_BASE + 90, FLOAT_REG_BASE + 91, FLOAT_REG_BASE + 92,
  FLOAT_REG_BASE + 93, FLOAT_REG_BASE + 94, FLOAT_REG_BASE + 95,
  FLOAT_REG_BASE + 96, FLOAT_REG_BASE + 97, FLOAT_REG_BASE + 98,
  FLOAT_REG_BASE + 99, FLOAT_REG_BASE + 100, FLOAT_REG_BASE + 101,
  FLOAT_REG_BASE + 102, FLOAT_REG_BASE + 103, FLOAT_REG_BASE + 104,
  FLOAT_REG_BASE + 105, FLOAT_REG_BASE + 106, FLOAT_REG_BASE + 107,
  FLOAT_REG_BASE + 108, FLOAT_REG_BASE + 109, FLOAT_REG_BASE + 110,
  FLOAT_REG_BASE + 111, FLOAT_REG_BASE + 112, FLOAT_REG_BASE + 113,
  FLOAT_REG_BASE + 114, FLOAT_REG_BASE + 115, FLOAT_REG_BASE + 116,
  FLOAT_REG_BASE + 117, FLOAT_REG_BASE + 118, FLOAT_REG_BASE + 119,
  FLOAT_REG_BASE + 120, FLOAT_REG_BASE + 121, FLOAT_REG_BASE + 122,
  FLOAT_REG_BASE + 123, FLOAT_REG_BASE + 124, FLOAT_REG_BASE + 125,
  FLOAT_REG_BASE + 126, FLOAT_REG_BASE + 127
};

static int callee_float_reg_map[NUM_CALLEE_FLOAT_REG] = {
  FLOAT_REG_BASE + 2, FLOAT_REG_BASE + 3, FLOAT_REG_BASE + 4,
  FLOAT_REG_BASE + 5,
  FLOAT_REG_BASE + 16, FLOAT_REG_BASE + 17, FLOAT_REG_BASE + 18,
  FLOAT_REG_BASE + 19,
  FLOAT_REG_BASE + 20, FLOAT_REG_BASE + 21, FLOAT_REG_BASE + 22,
  FLOAT_REG_BASE + 23,
  FLOAT_REG_BASE + 24, FLOAT_REG_BASE + 25, FLOAT_REG_BASE + 26,
  FLOAT_REG_BASE + 27,
  FLOAT_REG_BASE + 28, FLOAT_REG_BASE + 29, FLOAT_REG_BASE + 30,
  FLOAT_REG_BASE + 31
};

static int caller_float_macro_map[NUM_CALLER_FLOAT_MAC] = {
  L_MAC_P20, L_MAC_P21, L_MAC_P22, L_MAC_P23,
  L_MAC_P24, L_MAC_P25, L_MAC_P26, L_MAC_P27
};

static int caller_double_reg_map[NUM_CALLER_DOUBLE_REG] = {
  FLOAT_REG_BASE + 6, FLOAT_REG_BASE + 7, FLOAT_REG_BASE + 32,
  FLOAT_REG_BASE + 33, FLOAT_REG_BASE + 34, FLOAT_REG_BASE + 35,
  FLOAT_REG_BASE + 36, FLOAT_REG_BASE + 37, FLOAT_REG_BASE + 38,
  FLOAT_REG_BASE + 39, FLOAT_REG_BASE + 40, FLOAT_REG_BASE + 41,
  FLOAT_REG_BASE + 42, FLOAT_REG_BASE + 43, FLOAT_REG_BASE + 44,
  FLOAT_REG_BASE + 45, FLOAT_REG_BASE + 46, FLOAT_REG_BASE + 47,
  FLOAT_REG_BASE + 48, FLOAT_REG_BASE + 49, FLOAT_REG_BASE + 50,
  FLOAT_REG_BASE + 51, FLOAT_REG_BASE + 52, FLOAT_REG_BASE + 53,
  FLOAT_REG_BASE + 54, FLOAT_REG_BASE + 55, FLOAT_REG_BASE + 56,
  FLOAT_REG_BASE + 57, FLOAT_REG_BASE + 58, FLOAT_REG_BASE + 59,
  FLOAT_REG_BASE + 60, FLOAT_REG_BASE + 61, FLOAT_REG_BASE + 62,
  FLOAT_REG_BASE + 63, FLOAT_REG_BASE + 64, FLOAT_REG_BASE + 65,
  FLOAT_REG_BASE + 66, FLOAT_REG_BASE + 67, FLOAT_REG_BASE + 68,
  FLOAT_REG_BASE + 69, FLOAT_REG_BASE + 70, FLOAT_REG_BASE + 71,
  FLOAT_REG_BASE + 72, FLOAT_REG_BASE + 73, FLOAT_REG_BASE + 74,
  FLOAT_REG_BASE + 75, FLOAT_REG_BASE + 76, FLOAT_REG_BASE + 77,
  FLOAT_REG_BASE + 78, FLOAT_REG_BASE + 79, FLOAT_REG_BASE + 80,
  FLOAT_REG_BASE + 81, FLOAT_REG_BASE + 82, FLOAT_REG_BASE + 83,
  FLOAT_REG_BASE + 84, FLOAT_REG_BASE + 85, FLOAT_REG_BASE + 86,
  FLOAT_REG_BASE + 87, FLOAT_REG_BASE + 88, FLOAT_REG_BASE + 89,
  FLOAT_REG_BASE + 90, FLOAT_REG_BASE + 91, FLOAT_REG_BASE + 92,
  FLOAT_REG_BASE + 93, FLOAT_REG_BASE + 94, FLOAT_REG_BASE + 95,
  FLOAT_REG_BASE + 96, FLOAT_REG_BASE + 97, FLOAT_REG_BASE + 98,
  FLOAT_REG_BASE + 99, FLOAT_REG_BASE + 100, FLOAT_REG_BASE + 101,
  FLOAT_REG_BASE + 102, FLOAT_REG_BASE + 103, FLOAT_REG_BASE + 104,
  FLOAT_REG_BASE + 105, FLOAT_REG_BASE + 106, FLOAT_REG_BASE + 107,
  FLOAT_REG_BASE + 108, FLOAT_REG_BASE + 109, FLOAT_REG_BASE + 110,
  FLOAT_REG_BASE + 111, FLOAT_REG_BASE + 112, FLOAT_REG_BASE + 113,
  FLOAT_REG_BASE + 114, FLOAT_REG_BASE + 115, FLOAT_REG_BASE + 116,
  FLOAT_REG_BASE + 117, FLOAT_REG_BASE + 118, FLOAT_REG_BASE + 119,
  FLOAT_REG_BASE + 120, FLOAT_REG_BASE + 121, FLOAT_REG_BASE + 122,
  FLOAT_REG_BASE + 123, FLOAT_REG_BASE + 124, FLOAT_REG_BASE + 125,
  FLOAT_REG_BASE + 126, FLOAT_REG_BASE + 127
};

static int callee_double_reg_map[NUM_CALLEE_DOUBLE_REG] = {
  FLOAT_REG_BASE + 2, FLOAT_REG_BASE + 3, FLOAT_REG_BASE + 4,
  FLOAT_REG_BASE + 5,
  FLOAT_REG_BASE + 16, FLOAT_REG_BASE + 17, FLOAT_REG_BASE + 18,
  FLOAT_REG_BASE + 19,
  FLOAT_REG_BASE + 20, FLOAT_REG_BASE + 21, FLOAT_REG_BASE + 22,
  FLOAT_REG_BASE + 23,
  FLOAT_REG_BASE + 24, FLOAT_REG_BASE + 25, FLOAT_REG_BASE + 26,
  FLOAT_REG_BASE + 27,
  FLOAT_REG_BASE + 28, FLOAT_REG_BASE + 29, FLOAT_REG_BASE + 30,
  FLOAT_REG_BASE + 31
};

static int caller_double_macro_map[NUM_CALLER_DOUBLE_MAC] = {
  L_MAC_P20, L_MAC_P21, L_MAC_P22, L_MAC_P23,
  L_MAC_P24, L_MAC_P25, L_MAC_P26, L_MAC_P27
};

static int caller_predicate_reg_map[NUM_CALLER_PRED_REG] = {
  PRED_REG_BASE + 6, PRED_REG_BASE + 7, PRED_REG_BASE + 8,
  PRED_REG_BASE + 9, PRED_REG_BASE + 10, PRED_REG_BASE + 11,
  PRED_REG_BASE + 12, PRED_REG_BASE + 13, PRED_REG_BASE + 14,
  PRED_REG_BASE + 15
};

static int callee_predicate_reg_map[NUM_CALLEE_PRED_REG] = {
  PRED_REG_BASE + 1, PRED_REG_BASE + 2, PRED_REG_BASE + 3,
  PRED_REG_BASE + 4, PRED_REG_BASE + 5,

  PRED_REG_BASE + 16, PRED_REG_BASE + 17, PRED_REG_BASE + 18,
  PRED_REG_BASE + 19, PRED_REG_BASE + 20, PRED_REG_BASE + 21,
  PRED_REG_BASE + 22, PRED_REG_BASE + 23, PRED_REG_BASE + 24,
  PRED_REG_BASE + 25, PRED_REG_BASE + 26, PRED_REG_BASE + 27,
  PRED_REG_BASE + 28, PRED_REG_BASE + 29, PRED_REG_BASE + 30,
  PRED_REG_BASE + 31, PRED_REG_BASE + 32, PRED_REG_BASE + 33,
  PRED_REG_BASE + 34, PRED_REG_BASE + 35, PRED_REG_BASE + 36,
  PRED_REG_BASE + 37, PRED_REG_BASE + 38, PRED_REG_BASE + 39,
  PRED_REG_BASE + 40, PRED_REG_BASE + 41, PRED_REG_BASE + 42,
  PRED_REG_BASE + 43, PRED_REG_BASE + 44, PRED_REG_BASE + 45,
  PRED_REG_BASE + 46, PRED_REG_BASE + 47, PRED_REG_BASE + 48,
  PRED_REG_BASE + 49, PRED_REG_BASE + 50, PRED_REG_BASE + 51,
  PRED_REG_BASE + 52, PRED_REG_BASE + 53, PRED_REG_BASE + 54,
  PRED_REG_BASE + 55, PRED_REG_BASE + 56, PRED_REG_BASE + 57,
  PRED_REG_BASE + 58, PRED_REG_BASE + 59, PRED_REG_BASE + 60,
  PRED_REG_BASE + 61, PRED_REG_BASE + 62, PRED_REG_BASE + 63
};

static int caller_branch_reg_map[NUM_CALLER_BRANCH_REG] = {
  BRANCH_REG_BASE + 6, BRANCH_REG_BASE + 7
};

static int callee_branch_reg_map[NUM_CALLEE_BRANCH_REG] = {
  BRANCH_REG_BASE + 1, BRANCH_REG_BASE + 2, BRANCH_REG_BASE + 3,
  BRANCH_REG_BASE + 4, BRANCH_REG_BASE + 5
};

/*****************************************************************************/
/*****************************************************************************/

/****************************************************************************
 *
 * routine: O_is_caller_save_predicate()
 * purpose: Determine if the given op is a caller save predicate.
 * input: pred - predicate operand.
 * output: 
 * returns: 1 if caller save, 0 otherwise.
 * modified: Bob McGowan - 5/97 - created
 * note:
 *-------------------------------------------------------------------------*/

int
O_is_caller_save_predicate (L_Operand * pred)
{
  int pred_id;

  for (pred_id = 0; pred_id < NUM_CALLER_PRED_REG; pred_id++)
    if (pred->value.r == caller_predicate_reg_map[pred_id])
      return (1);
  return (0);
}				/* O_is_caller_save_predicate */

/* Updates reg stack information not known in phase 1 */

void
O_update_alloc_operands (L_Oper * oper, int num_special)
{
  int rot_int_num, null;

  R_get_rot_regs (L_fn, &null, &rot_int_num, &null, &null,
		  &null, &null, &null, &null);

  L_delete_operand (oper->src[1]);
  L_delete_operand (oper->src[3]);

  oper->src[1] = L_new_gen_int_operand (stacked_callee_int_num + num_special);
  oper->src[3] = L_new_gen_int_operand (rot_int_num);
}				/* O_update_alloc_operands */

#if 0
/* Hack to fixup predicate attributes for spill code inserted SAM 10-94 */
static void
L_fix_vpred_attrs (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *oper, *ptr;
  L_Attr *attr, *new_attr;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      if (!L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK))
	continue;
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
	{
	  if (!L_EXTRACT_BIT_VAL (oper->flags, L_OPER_SPILL_CODE))
	    continue;
	  if (oper->pred[0] == NULL)
	    continue;

	  if (L_store_opcode (oper))
	    {
	      /* search up first */
	      for (ptr = oper->prev_op; ptr != NULL; ptr = ptr->prev_op)
		{
		  if (L_EXTRACT_BIT_VAL (ptr->flags, L_OPER_SPILL_CODE))
		    continue;
		  if (L_same_operand (oper->pred[0], ptr->pred[0]))
		    break;
		}		/* for ptr */
	      if (ptr == NULL)
		{		/* try down */
		  for (ptr = oper->next_op; ptr != NULL; ptr = ptr->next_op)
		    {
		      if (L_EXTRACT_BIT_VAL (ptr->flags, L_OPER_SPILL_CODE))
			continue;
		      if (L_same_operand (oper->pred[0], ptr->pred[0]))
			break;
		    }		/* for ptr */
		}		/* if */
	      if (ptr == NULL)
		L_punt ("L_fix_vpred_attrs: no match found for op %d",
			oper->id);

	      attr = L_find_attr (ptr->attr, L_VPRED_PRD_ATTR_NAME);
	      if (attr == NULL)
		L_punt ("L_fix_vpred_attrs: no vpred attr found");
	      new_attr = L_copy_attr_element (attr);
	      oper->attr = L_concat_attr (oper->attr, new_attr);
	    }			/* if */
	  else if (L_load_opcode (oper) || L_int_add_opcode (oper))
	    {
	      /* look down first */
	      for (ptr = oper->next_op; ptr != NULL; ptr = ptr->next_op)
		{
		  if (L_EXTRACT_BIT_VAL (ptr->flags, L_OPER_SPILL_CODE))
		    continue;
		  if (L_same_operand (oper->pred[0], ptr->pred[0]))
		    break;
		}		/* for ptr */
	      if (ptr == NULL)
		{		/* try up */
		  for (ptr = oper->prev_op; ptr != NULL; ptr = ptr->prev_op)
		    {
		      if (L_EXTRACT_BIT_VAL (ptr->flags, L_OPER_SPILL_CODE))
			continue;
		      if (L_same_operand (oper->pred[0], ptr->pred[0]))
			break;
		    }		/* for ptr */
		}		/* if */
	      if (ptr == NULL)
		L_punt ("L_fix_vpred_attrs: no match found for op %d",
			oper->id);

	      attr = L_find_attr (ptr->attr, L_VPRED_PRD_ATTR_NAME);
	      if (attr == NULL)
		L_punt ("L_fix_vpred_attrs: no vpred attr found");
	      new_attr = L_copy_attr_element (attr);
	      oper->attr = L_concat_attr (oper->attr, new_attr);
	    }			/* else if */
	  else
	    {
	      L_punt ("L_fix_vpred_attrs: illegal spill oper");
	    }			/* else */
	}			/* for oper */
    }				/* for cb */
}				/* L_fix_vpred_attrs */
#endif

/******************************************************************************\
 *
 * Functions which provide penalties for use of callee and caller save 
 * registers.
 *
\******************************************************************************/

/****************************************************************************
 *
 * routine: R_callee_cost()
 * purpose: Specify the cost of using a callee save register.
 * input: ctype - This is the register type: L_CTYPE_PREDICATE,
 *                L_CTYPE_INT, L_CTYPE_FLOAT, L_CTYPE_DOUBLE, L_CTYPE_BTR
 *        leaf - 1 if the function using the register is a leaf i.e. it
 *               does not have a function call.
 *        callee_allocated - 
 * output: 
 * returns:
 * modified:
 * note:
 *-------------------------------------------------------------------------*/

double
R_callee_cost (int ctype, int leaf, int callee_allocated)
{
  double cost = 0.0;

  if (L_is_ctype_integer_direct ((unsigned char) ctype))
    {
      if (leaf)
	{
	  cost = 0.001;
	  /* should be small enough but must be greater than zero in order to
	     use caller reg for leaf func - pwang */
	}			/* if */
      else
	{
	  cost = 0.02 * (LOAD_COST + STORE_COST);
	  /* Changed from 0.00 -- 20010207 JWS */
	  /* assume that 5% of stack registers force the RSE to do a spill */
	}			/* else */
    }				/* if */
  else
    {
      switch (ctype)
	{
	case L_CTYPE_FLOAT:
	case L_CTYPE_DOUBLE:
	  cost = 0.5 * (LOAD_COST + STORE_COST);
	  break;
	case L_CTYPE_PREDICATE:
	  cost = 0.0;
	  break;
	case L_CTYPE_BTR:
	case L_CTYPE_POINTER:
	  cost = 0.5 * (LOAD_COST + STORE_COST);
	  break;
	default:
	  L_punt ("R_callee_cost: invalid ctype of %d", ctype);
	}			/* switch */
    }				/* else */
  return (cost);
}				/* R_callee_cost */

/****************************************************************************
 *
 * routine: R_caller_cost()
 * purpose: Specify the cost of using a caller save register.
 * input: ctype - This is the register type: L_CTYPE_PREDICATE,
 *                L_CTYPE_INT, L_CTYPE_FLOAT, L_CTYPE_DOUBLE, L_CTYPE_BTR
 *        leaf - 1 if the function using the register is a leaf i.e. it
 *               does not have a function call.  
 * output: 
 * returns:
 * modified:
 * note:
 *-------------------------------------------------------------------------*/

double
R_caller_cost (int ctype, int leaf)
{
  double cost = 0.0;

  if (leaf)
    return (0.0);

  if (L_is_ctype_integer_direct ((unsigned char) ctype))
    {
      cost = LOAD_COST + STORE_COST;
    }				/* if */
  else
    {
      switch (ctype)
	{
	case L_CTYPE_FLOAT:
	case L_CTYPE_DOUBLE:
	  cost = LOAD_COST + STORE_COST;
	  break;
	case L_CTYPE_PREDICATE:
	  cost = 1.0;	/* This needs to be greater than callee save value */
	  break;
	case L_CTYPE_BTR:
	case L_CTYPE_POINTER:
	  cost = LOAD_COST + STORE_COST;
	  break;
	default:
	  L_punt ("R_caller_cost: invalid ctype of %d", ctype);
	}			/* switch */
    }				/* else */
  return (cost);
}				/* R_caller_cost */


double
R_spill_load_cost (int ctype)
{
  double cost = 0.0;

  if (L_is_ctype_integer_direct ((unsigned char) ctype))
    {
      cost = LOAD_COST;
    }				/* if */
  else
    {
      switch (ctype)
	{
	case L_CTYPE_FLOAT:
	case L_CTYPE_DOUBLE:
	  cost = LOAD_COST + 0.5;
	  break;
	case L_CTYPE_PREDICATE:
	  cost = LOAD_COST;
	  break;
	case L_CTYPE_BTR:
	case L_CTYPE_POINTER:
	  cost = LOAD_COST + 1.0;	/* Load then move to branch reg */
	  break;
	default:
	  L_punt ("R_spill_load_cost: invalid ctype of %d", ctype);
	}			/* switch */
    }				/* else */

  return cost;
}				/* R_spill_load_cost */

double
R_spill_store_cost (int ctype)
{
  double cost = 0.0;
  int stores_per_cycle;

  /* 20031119 SZU */
  /* currently assumes uniform model */
  stores_per_cycle = 0;

  if (L_is_ctype_integer_direct ((unsigned char) ctype))
    {
      cost = STORE_COST;
    }				/* if */
  else
    {
      switch (ctype)
	{
	case L_CTYPE_FLOAT:
	case L_CTYPE_DOUBLE:
	  cost = STORE_COST + 0.5;
	  break;
	case L_CTYPE_PREDICATE:
	  cost = STORE_COST;
	  break;
	case L_CTYPE_BTR:
	case L_CTYPE_POINTER:
	  cost = STORE_COST + 1.0;	/* Move to int reg then store */
	  break;
	default:
	  L_punt ("R_spill_store_cost: invalid ctype of %d", ctype);
	}			/* switch */
    }				/* else */
  return cost;
}				/* R_spill_store_cost */

static void
O_append_op (L_Oper **op_seq, L_Oper *new_op)
{
  L_Oper **pnxt = op_seq;

  while (*pnxt)
    pnxt = &((*pnxt)->next_op);
  
  *pnxt = new_op;
}

static void
O_prepend_op (L_Oper **op_seq, L_Oper *new_op)
{
  L_Oper *nxt = *op_seq;
  L_Oper **pnxt = &(new_op->next_op);

  *op_seq = new_op;

  while (*pnxt)
    pnxt = &((*pnxt)->next_op);
  
  *pnxt = nxt;
}


/****************************************************************************
 *
 * routine: O_address_add()
 * purpose: 
 * input: stack_pointer - stack pointer to add to such as L_MAC_SP, L_MAC_FP
 *        offset - offset from the stack pointer
 *        pred - array of predicates
 *        type_flag - type of the spilling:
 *                     R_JSR_SAVE_CODE - spilling of caller save regs around
 *                                       jsr
 *                     R_SPILL_CODE - spill to get more regs
 * output:
 * returns:
 * modified: 9/12/02 REK Updating to use new TAHOEops.
 * note: Creates op of the form:  add TEMP_REG = int #, STACK_REG
 *-------------------------------------------------------------------------*/

static L_Oper *
O_address_add (int stack_pointer, int offset, L_Operand ** pred,
	       int type_flag, int operand_ptype, L_Oper **oper_seq)
{
  int i;
  L_Oper *add_oper = NULL, *addl_oper = NULL;
  L_Attr *attr;

  if (SIMM_14(offset))
    {
      add_oper = L_create_new_op (Lop_ADD);
      add_oper->proc_opc = TAHOEop_ADD;
      add_oper->src[0] = L_new_gen_int_operand (offset);
      add_oper->src[1] = L_new_macro_operand (stack_pointer, L_CTYPE_LLONG,
					      L_PTYPE_NULL);
      add_oper->dest[0] = Ltahoe_IMAC (TMPREG1);
      O_append_op (oper_seq, add_oper);
    }
  else if (SIMM_22(offset))
    {
      add_oper = L_create_new_op (Lop_ADD);
      add_oper->proc_opc = TAHOEop_ADD;
      add_oper->src[0] = L_new_gen_int_operand (0);
      add_oper->src[1] = L_new_macro_operand (stack_pointer, L_CTYPE_LLONG,
					      L_PTYPE_NULL);
      add_oper->dest[0] = Ltahoe_IMAC (TMPREG1);
      O_append_op (oper_seq, add_oper);

      addl_oper = L_create_new_op (Lop_ADD);
      addl_oper->proc_opc = TAHOEop_ADDL;
      addl_oper->src[0] = L_new_gen_int_operand (offset);
      addl_oper->src[1] = Ltahoe_IMAC (TMPREG1);
      addl_oper->dest[0] = Ltahoe_IMAC (TMPREG1);
      O_append_op (oper_seq, addl_oper);
    }
  else
    {
      L_punt ("O_address_add: offset > SIMM_22");
    }

  /* flag the add as being inserted by the register allocator, so the spill
   * offset can be later updated 
   */
  attr = L_new_attr ("regalloc1", 1);
  L_set_int_attr_field (attr, 0, type_flag);
  add_oper->attr = L_concat_attr (add_oper->attr, attr);
  if (addl_oper)
    {
      attr = L_new_attr ("regalloc1", 1);
      L_set_int_attr_field (attr, 0, type_flag);
      addl_oper->attr = L_concat_attr (addl_oper->attr, attr);
    }

  /* Set the predicates */
  if (pred)
    {
      for (i = 0; i < L_max_pred_operand; i++)
	{
	  add_oper->pred[i] = L_copy_operand (pred[i]);
	  if (addl_oper)
	    addl_oper->pred[i] = L_copy_operand (pred[i]);
	}

      /* JWS -- red flag: what's going on here? */

      if ((operand_ptype == L_PTYPE_UNCOND_T) ||
	  (operand_ptype == L_PTYPE_UNCOND_F))
	{
	  L_delete_operand (add_oper->pred[0]);
	  add_oper->pred[0] = NULL;
	  if (addl_oper)
	    {
	      L_delete_operand (addl_oper->pred[0]);
	      addl_oper->pred[0] = NULL;
	    }
	}			/* if */
    }				/* if */

  add_oper->flags = L_SET_BIT_FLAG (add_oper->flags,
				    L_OPER_SPILL_CODE | L_OPER_SAFE_PEI);

  if (addl_oper)
    addl_oper->flags = L_SET_BIT_FLAG (addl_oper->flags,
				      L_OPER_SPILL_CODE | L_OPER_SAFE_PEI);

  return (add_oper);
}				/* O_address_add */


/****************************************************************************
 *
 * routine: O_fill_reg()
 * purpose: Generate a sequence of instructions to fill a register.
 * input: reg - physical register id of the operand being spilled.
 *        type - type of the operand (ex: L_OPERAND_REGISTER)
 *        operand - The operand which is being spilled.
 *        fill_offset - memory stack offset generated by register allocator.
 *        pred - The predicate on the oper that we are placing the fill code
 *               after.
 *        type_flag - type of the spilling:
 *                     R_JSR_SAVE_CODE - spilling of caller save regs around jsr
 *                     R_SPILL_CODE - spill to get more regs
 * output:
 * returns: A pointer to the first instruction in the sequence.
 * modified: 12/3/96 - Bob McGowan
 *                     Bug fix: The predicate on a jsr was being used to guard
 *                     the spill and fill code.  However, if the jsr predicate
 *                     is a caller save predicate, then this predicate has
 *                     an undefined value upon return.  So the fill code cannot
 *                     be gaurded by this predicate.  Hence, neither can the
 *                     spill code.
 *           9/12/02 - Robert Kidd
 *                     Updating function to use new opcode map.
 * note:
 *-------------------------------------------------------------------------*/

L_Oper *
O_fill_reg (int reg, int type, L_Operand * operand, int fill_offset,
	    L_Operand ** pred_array, int type_flag)
{
  int i, base_mac = L_MAC_SP;
  L_Oper *add_oper, *load_oper = NULL, *mov_oper = NULL, 
    *oper_seq = NULL;

  /* Look at comments in O_spill_reg */

  if (L_is_ctype_integer (operand))
    {
      /* Generate the load op */
      load_oper = L_create_new_op (Lop_LD_Q);
      if (!Ltahoe_clobber_unats)
	{
	  load_oper->proc_opc = TAHOEop_LD8_FILL;
	  load_oper->src[1] = Ltahoe_IMAC (UNAT);
	}			/* if */
      else
	{
	  load_oper->proc_opc = TAHOEop_LD8;
	}			/* else */
    }				/* if */
  else
    {
      switch (operand->ctype)
	{
	case L_CTYPE_FLOAT:
	case L_CTYPE_DOUBLE:
	  load_oper = L_create_new_op (Lop_LD_F2);
	  load_oper->proc_opc = TAHOEop_LDF_FILL;
	  base_mac = L_MAC_FP;
	  break;

	case L_CTYPE_PREDICATE:
	  load_oper = L_create_new_op (Lop_LD_C);
	  load_oper->proc_opc = TAHOEop_LD1;
	  load_oper->dest[0] = Ltahoe_IMAC (TMPREG2);
	  mov_oper = L_create_new_op (Lop_CMP);
	  mov_oper->proc_opc = TAHOEop_CMP;
	  TC_SET_CMP_OP (mov_oper->completers, TC_CMP_OP_EQ);
	  mov_oper->src[1] = Ltahoe_IMAC (TMPREG2);
	  mov_oper->src[0] = L_new_int_operand (1, L_CTYPE_INT);
	  mov_oper->com[0] = L_CTYPE_INT;
	  mov_oper->com[1] = Lcmp_COM_EQ;
	  base_mac = TAHOE_MAC_PSPILL;
	  break;

	case L_CTYPE_BTR:
	  load_oper = L_create_new_op (Lop_LD_Q);
	  load_oper->proc_opc = TAHOEop_LD8;
	  load_oper->dest[0] = Ltahoe_IMAC (TMPREG2);
	  mov_oper = L_create_new_op (Lop_MOV);
	  mov_oper->proc_opc = TAHOEop_MOV_TOBR;
	  mov_oper->src[0] = Ltahoe_IMAC (TMPREG2);
	  break;

	default:
	  L_punt ("O_fill_reg: unsupported register type", operand->ctype);
	}			/* switch */
    }				/* else */

  add_oper = O_address_add (base_mac, fill_offset, pred_array,
			    type_flag, operand->ptype, &oper_seq);

  if (pred_array && pred_array[0]
      && (type_flag == R_JSR_SAVE_CODE)
      && O_is_caller_save_predicate (pred_array[0]))
    pred_array = NULL;

  /* Generate the load op */
  load_oper->src[0] = L_copy_operand (add_oper->dest[0]);

  if (mov_oper)
    {
      int dest_ptype;
      if (operand->ctype == L_CTYPE_PREDICATE)
	dest_ptype = L_PTYPE_COND_T;
      else
	dest_ptype = L_PTYPE_NULL;

      if (L_is_reg_direct ((char) type))
	mov_oper->dest[0] = L_new_register_operand (reg, operand->ctype,
						    dest_ptype);
      else
	mov_oper->dest[0] = L_new_macro_operand (reg, operand->ctype,
						 L_PTYPE_NULL);

      mov_oper->flags = L_SET_BIT_FLAG (mov_oper->flags, L_OPER_SPILL_CODE);
    }				/* if */
  else
    {
      if (L_is_reg_direct ((char) type))
	load_oper->dest[0] = L_new_register_operand (reg, operand->ctype,
						     L_PTYPE_NULL);
      else
	load_oper->dest[0] = L_new_macro_operand (reg, operand->ctype,
						  L_PTYPE_NULL);
    }				/* else */

  Ltahoe_annotate_stack_ref (load_oper, add_oper->src[1]->value.mac,
			     fill_offset, 1);

  /* Add the stack oper flag and attribute for memory disambiguation.
   * This is needed since there are no sync arcs on spill/fill code.
   */
  load_oper->flags = L_SET_BIT_FLAG (load_oper->flags,
				     L_OPER_SPILL_CODE | L_OPER_SAFE_PEI);

  /* Change the predicates */
  if (pred_array)
    {
      for (i = 0; i < L_max_pred_operand; i++)
	load_oper->pred[i] = L_copy_operand (pred_array[i]);

      /* JWS -- red flag... what's going on here? */

      if ((operand->ptype == L_PTYPE_UNCOND_T) ||
	  (operand->ptype == L_PTYPE_UNCOND_F))
	{
	  L_delete_operand (load_oper->pred[0]);
	  load_oper->pred[0] = NULL;
	}			/* if */
    }				/* if */

  O_append_op (&oper_seq, load_oper);

  if (mov_oper)
    O_append_op (&oper_seq, mov_oper);

  return (oper_seq);
}				/* O_fill_reg */


/****************************************************************************
 *
 * routine: O_spill_reg()
 * purpose: Generate a sequence of instructions to spill a register.
 * input: reg - physical register id of the operand being spilled.
 *        type - type of the operand (ex: L_OPERAND_REGISTER)
 *        operand - The operand which is being spilled.
 *        spill_offset - memory stack offset generated by register allocator.
 *        pred - The predicate on the oper that we are placing the spill code
 *               before.
 *        type_flag - type of the spilling:
 *                     R_JSR_SAVE_CODE - spilling of caller save regs around jsr
 *                     R_SPILL_CODE - spill to get more regs
 * output:
 * returns: A pointer to the first instruction in the sequence.
 * modified: 12/3/96 - Bob McGowan
 *                     Bug fix: The predicate on a jsr was being used to gaurd
 *                     the spill and fill code.  However, if the jsr predicate
 *                     is a caller save predicate, then this predicate has
 *                     an undefined value upon return.  So the fill code cannot
 *                     be gaurded by this predicate.  Hence, neither can the
 *                     spill code.
 *           9/12/02 - Robert Kidd
 *                     Updating to use new completer map.
 * note:
 *-------------------------------------------------------------------------*/

L_Oper *
O_spill_reg (int reg, int type, L_Operand * operand, int spill_offset,
	     L_Operand ** pred_array, int type_flag)
{
  int i, base_mac = L_MAC_SP, align = 0;
  L_Oper *mov_oper = NULL, *add_oper, *store_oper = NULL, *clr_oper = NULL, 
    *oper_seq = NULL;

  /* The spill offset is memory stack offset generated by register allocator.
     Sizes are hardwired into R_insert_jsr_spill_fill_code in r_regspill.c
     and R_spill_loc in r_regalloc.c */

  if (L_is_ctype_integer (operand))
    {
      if (spill_offset % 8)
	L_punt ("Warning: int spill not aligned %d\n", spill_offset);

      /* Generate the store op */
      store_oper = L_create_new_op (Lop_ST_Q);
      store_oper->proc_opc = TAHOEop_ST8_SPILL;
      store_oper->dest[0] = Ltahoe_IMAC (UNAT);
      align = 8;
    }				/* if */
  else
    {
      switch (operand->ctype)
	{
	case L_CTYPE_FLOAT:
	case L_CTYPE_DOUBLE:
	  store_oper = L_create_new_op (Lop_ST_F2);
	  store_oper->proc_opc = TAHOEop_STF_SPILL;
	  base_mac = L_MAC_FP;
	  align = 16;
	  break;

	case L_CTYPE_PREDICATE:
	  /* Uncond clear */
	  clr_oper = L_create_new_op (Lop_MOV);
	  clr_oper->proc_opc = TAHOEop_MOVI;
	  clr_oper->src[0] = L_new_int_operand (0, L_CTYPE_CHAR);
	  clr_oper->dest[0] = Ltahoe_IMAC (TMPREG2);

	  /* Cond set */
	  mov_oper = L_create_new_op (Lop_MOV);
	  mov_oper->proc_opc = TAHOEop_MOVI;
	  mov_oper->pred[0] = L_new_register_operand (reg, operand->ctype,
						      operand->ptype);
	  mov_oper->src[0] = L_new_int_operand (1, L_CTYPE_CHAR);
	  mov_oper->dest[0] = Ltahoe_IMAC (TMPREG2);

	  /* Store + add */
	  store_oper = L_create_new_op (Lop_ST_C);
	  store_oper->proc_opc = TAHOEop_ST1;

	  L_warn ("O_spill_reg: spilling pr (op %d)", store_oper->id);
	  base_mac = TAHOE_MAC_PSPILL;
	  align = 1;
	  break;

	case L_CTYPE_BTR:
	  /* move to a temp reg and spill as an integer */

	  mov_oper = L_create_new_op (Lop_MOV);
	  mov_oper->proc_opc = TAHOEop_MOV_FRBR;
	  mov_oper->src[0] = L_new_register_operand (reg, operand->ctype,
						     L_PTYPE_NULL);
	  mov_oper->dest[0] = Ltahoe_IMAC (TMPREG2);
	  store_oper = L_create_new_op (Lop_ST_Q);
	  store_oper->proc_opc = TAHOEop_ST8;
	  L_warn ("O_spill_reg: spilling btr (op %d)", store_oper->id);
	  align = 8;
	  break;

	default:
	  L_punt ("O_spill_reg: unsupported register type", operand->ctype);
	}			/* switch */
    }				/* else */

  if (spill_offset % align)
    L_punt ("O_spill_reg: unaligned spill (op %d)", store_oper->id);

  add_oper = O_address_add (base_mac, spill_offset, pred_array,
			    type_flag, operand->ptype, &oper_seq);

  if ((type_flag == R_JSR_SAVE_CODE)
      && pred_array && pred_array[0]
      && O_is_caller_save_predicate (pred_array[0]))
    pred_array = NULL;

  store_oper->src[0] = L_copy_operand (add_oper->dest[0]);

  if (mov_oper)
    store_oper->src[1] = Ltahoe_IMAC (TMPREG2);
  else if (type == L_OPERAND_REGISTER)
    store_oper->src[1] = L_new_register_operand (reg, operand->ctype,
						 L_PTYPE_NULL);
  else
    store_oper->src[1] = L_new_macro_operand (reg, operand->ctype,
					      L_PTYPE_NULL);

  /* Add the stack oper flag and attribute for memory disambiguation.
   * This is needed since there are no sync arcs on spill/fill code. 
   */
  store_oper->flags = L_SET_BIT_FLAG (store_oper->flags,
				      L_OPER_STACK_REFERENCE |
				      L_OPER_SPILL_CODE | L_OPER_SAFE_PEI);

  Ltahoe_annotate_stack_ref (store_oper, add_oper->src[1]->value.mac,
			     spill_offset, 1);

  /* Change the predicates */
  if (pred_array)
    {
      for (i = 0; i < L_max_pred_operand; i++)
	store_oper->pred[i] = L_copy_operand (pred_array[i]);

      /* JWS red flag -- what's going on here? */
      if ((operand->ptype == L_PTYPE_UNCOND_T) ||
	  (operand->ptype == L_PTYPE_UNCOND_F))
	{
	  /* First store: low store */
	  L_delete_operand (store_oper->pred[0]);
	  store_oper->pred[0] = NULL;
	}			/* if */
    }				/* if */

  O_append_op (&oper_seq, store_oper);

  if (!mov_oper)
    return oper_seq;

  O_prepend_op (&oper_seq, mov_oper);
  mov_oper->flags = L_SET_BIT_FLAG (mov_oper->flags, L_OPER_SPILL_CODE);

  if (!clr_oper)
    return oper_seq;

  O_prepend_op (&oper_seq, clr_oper);
  clr_oper->flags = L_SET_BIT_FLAG (clr_oper->flags, L_OPER_SPILL_CODE);

  return oper_seq;
}				/* O_spill_reg */


/****************************************************************************
 *
 * routine:
 * purpose: 
 * input:
 * output:
 * returns:
 * modified:
 * note:
 *-------------------------------------------------------------------------*/

L_Oper *
O_move_reg (int dest_reg, int src_reg, int reg_type)
{
  int lop = 0, tahoeop = 0;
  L_Oper *mov_oper;

  switch (reg_type)
    {
    case L_CTYPE_INT:
    case L_CTYPE_LLONG:
      lop = Lop_MOV;
      tahoeop = TAHOEop_MOV_GR;
      break;
    case L_CTYPE_FLOAT:
      lop = Lop_MOV_F;
      tahoeop = TAHOEop_MOV_FR;
      break;
    case L_CTYPE_DOUBLE:
      lop = Lop_MOV_F2;
      tahoeop = TAHOEop_MOV_FR;
      break;
    case L_CTYPE_BTR:
    default:
      L_punt ("O_move_reg: unsupported register type", reg_type);
    }				/* switch */
  mov_oper = L_create_new_op (lop);
  mov_oper->proc_opc = tahoeop;
  mov_oper->dest[0] = L_new_register_operand (dest_reg, reg_type, 0);
  mov_oper->src[0] = L_new_register_operand (src_reg, reg_type, 0);

  return mov_oper;
}				/* O_move_reg */

L_Oper *
O_jump_oper (int opc, L_Cb * dest_cb)
{
  L_Oper *new_oper = L_create_new_op (opc);
  new_oper->src[0] = L_new_cb_operand (dest_cb);

  L_punt ("O_jump_oper was called.  This is a Very Bad Thing (TM).");
  return (new_oper);
}				/* O_jump_oper */


/****************************************************************************
 *
 * routine:
 * purpose: 
 * input:
 * output:
 * returns:
 * modified:
 * note:
 *-------------------------------------------------------------------------*/

R_Physical_Bank *
O_locate_rot_reg_bank (L_Func * fn, R_Reg * vreg)
{
  int num_reg_stack_inputs, num_reg_stack_locals,
    num_reg_stack_outputs, num_reg_stack_rots;
  R_Physical_Bank *bank;
  int offset;
  int limit;

  int nth_rot_reg = vreg->nth_rot_reg;

  if (nth_rot_reg < 0)
    L_punt ("O_locate_rot_reg_bank: vreg->nth_rot_reg < 0!");

  L_get_reg_stack_info (fn, &num_reg_stack_inputs, &num_reg_stack_locals,
			&num_reg_stack_outputs, &num_reg_stack_rots);

  switch (vreg->type)
    {
    case R_PREDICATE:
      bank = (R_bank + R_CALLEE + R_PREDICATE);
      vreg->rclass = R_CALLEE;
      vreg->type = R_PREDICATE;

      break;

    case R_INT:
      offset = 0;
      /* The first rotating registers are located in the stacked inputs. */
      bank = (R_bank + R_MACRO_CALLEE + R_INT);
      vreg->rclass = R_MACRO_CALLEE;
      vreg->type = R_INT;

      limit = bank->num_rot_reg;
      if (nth_rot_reg >= limit)
	{
	  nth_rot_reg -= bank->num_rot_reg;

	  bank = (R_bank + R_CALLEE + R_INT);
	  vreg->rclass = R_CALLEE;
	  vreg->type = R_INT;

	  limit = bank->num_rot_reg;

	  if (nth_rot_reg >= limit)
	    L_punt
	      ("O_locate_rot_reg_bank: unable to locate appropriate R_INT bank.");
	}			/* if */

      break;

    case R_FLOAT:
      bank = (R_bank + R_CALLER + R_FLOAT);
      vreg->rclass = R_CALLER;
      vreg->type = R_FLOAT;

      break;

    case R_DOUBLE:
      bank = (R_bank + R_CALLER + R_DOUBLE);
      vreg->rclass = R_CALLER;
      vreg->type = R_DOUBLE;

      break;

    default:
      bank = NULL;
    }				/* switch */

  vreg->nth_rot_reg = nth_rot_reg;

  return bank;
}				/* O_locate_rot_reg_bank */


/****************************************************************************
 *
 * routine: O_register_init()
 * purpose: Define the registers that can be used by the register allocator.
 * input:
 * output:
 * returns:
 * modified:
 * note: R_define_physical_bank will point the given set pointer to a
 *       place in memory.  These sets will change, when register allocation
 *       is done.  So it appears these sets change even though they are not
 *       explicity used.
 *-------------------------------------------------------------------------*/

void
O_register_init ()
{
  /*
   * Define the EM register banks to the register allocator.
   * The EM shares the same bank of registers for float and double.
   * R_define_physical_bank( class, type, num_reg, reg_size, overlap,
   *                         *reg_map, used set )
   */

  /*
   * INT Bank -  Caller save only.
   * Caller save are constant, so they are defined here.  Callee save
   * change for each function so they are dynamically created.  Look in
   * O_register_allocation.
   */

  R_define_physical_bank (R_CALLER,
			  R_INT,
			  NUM_CALLER_INT_REG,
			  1,
			  R_OVERLAP_INT, caller_int_reg_map, &caller_int_set);


  /* FLOAT BANK */

  R_define_physical_bank_with_rot (R_CALLER,
				   R_FLOAT,
				   NUM_CALLER_FLOAT_REG,
				   1,
				   R_OVERLAP_FLOAT | R_OVERLAP_DOUBLE,
				   caller_float_reg_map, &caller_float_set,
				   96, 2, 96);

  R_define_physical_bank (R_CALLEE,
			  R_FLOAT,
			  NUM_CALLEE_FLOAT_REG,
			  1,
			  R_OVERLAP_FLOAT | R_OVERLAP_DOUBLE,
			  callee_float_reg_map, &callee_float_set);

  R_define_physical_bank (R_MACRO_CALLER,
			  R_FLOAT,
			  NUM_CALLER_FLOAT_MAC,
			  1,
			  R_OVERLAP_FLOAT | R_OVERLAP_DOUBLE,
			  caller_float_macro_map, &caller_float_macro_set);

  /* DOUBLE BANK */

  R_define_physical_bank_with_rot (R_CALLER,
				   R_DOUBLE,
				   NUM_CALLER_DOUBLE_REG,
				   1,
				   R_OVERLAP_FLOAT | R_OVERLAP_DOUBLE,
				   caller_double_reg_map, &caller_double_set,
				   96, 2, 96);

  R_define_physical_bank (R_CALLEE,
			  R_DOUBLE,
			  NUM_CALLEE_DOUBLE_REG,
			  1,
			  R_OVERLAP_FLOAT | R_OVERLAP_DOUBLE,
			  callee_double_reg_map, &callee_double_set);

  R_define_physical_bank (R_MACRO_CALLER,
			  R_DOUBLE,
			  NUM_CALLER_DOUBLE_MAC,
			  1,
			  R_OVERLAP_FLOAT | R_OVERLAP_DOUBLE,
			  caller_double_macro_map, &caller_double_macro_set);


  /* PREDICATE BANK */
  R_define_physical_bank (R_CALLER,
			  R_PREDICATE,
			  NUM_CALLER_PRED_REG,
			  1,
			  R_OVERLAP_PREDICATE,
			  caller_predicate_reg_map, &caller_predicate_set);

  R_define_physical_bank_with_rot (R_CALLEE,
				   R_PREDICATE,
				   NUM_CALLEE_PRED_REG,
				   1,
				   R_OVERLAP_PREDICATE,
				   callee_predicate_reg_map,
				   &callee_predicate_set, 48, 5, 48);

  /* BRANCH BANK */
  R_define_physical_bank (R_CALLER,
			  R_BTR,
			  NUM_CALLER_BRANCH_REG,
			  1,
			  R_OVERLAP_BTR,
			  caller_branch_reg_map, &caller_branch_set);

  R_define_physical_bank (R_CALLEE,
			  R_BTR,
			  NUM_CALLEE_BRANCH_REG,
			  1,
			  R_OVERLAP_BTR,
			  callee_branch_reg_map, &callee_branch_set);
  return;
}				/* O_register_init */


/****************************************************************************
 *
 * routine: O_register_cleanup()
 * purpose: This is the register allocation destructor which cleans up stuff
 *          after the register alloction set and arrays have been used.
 *          This should not be called until callee registers have been spilled
 *          to the stack.
 * input:
 * output:
 * returns:
 * modified: Bob McGowan - 4/97
 * note:
 *-------------------------------------------------------------------------*/

void
O_register_cleanup ()
{
  if (callee_int_array != NULL)
    free (callee_int_array);
  if (callee_flt_array != NULL)
    free (callee_flt_array);
  if (callee_btr_array != NULL)
    free (callee_btr_array);
  return;
}				/* O_register_cleanup */


/****************************************************************************
 *
 * routine: O_register_allocation()
 * purpose: Do register allocation.
 * input:
 * output:
 * returns:
 * modified: Bob McGowan - 9/17/96
 *           Changed the way int callee save registers are handled.
 *           John Sias     2/7/01
 *           Added new register banks to scavenge more registers
  void R_define_physical_bank_with_rot(int rclass, int type, int num_reg, 
				     int reg_size, int overlap, 
				     int *reg_array, Set *used,
				     int num_rot_reg, int first_rot_reg,
				     int num_rot_reg_alloc)
 *-------------------------------------------------------------------------*/

void
O_register_allocation (L_Func * fn,
		       Parm_Macro_List * command_line_macro_list,
		       int *int_swap_space_size, int *fp_swap_space_size,
		       int *pred_swap_space_size)
{
  L_Oper *oper;
  L_Cb *cb;
  Set callee_tmp_flt_set;
  int avail, reg;
  int num_reg_stack_inputs, num_reg_stack_outputs;
  int num_reg_stack_locals, num_reg_stack_rots;
  int num_avail_callee_regs, first_callee_reg;
  int mask;
  int need_psp;

  if (L_find_attr (fn->attr, "ALLOCA"))
    need_psp = 1;
  else
    need_psp = 0;

  /* Set up variable register banks (constant banks are set up in
   * O_register_init()
   */

  L_get_reg_stack_info (fn, &num_reg_stack_inputs, &num_reg_stack_locals,
			&num_reg_stack_outputs, &num_reg_stack_rots);

  if (num_reg_stack_locals != 0)
    L_punt ("O_register_allocation: stacked locals should be zero at outset");

  /* Preserved integer registers
   * ----------------------------------------------------------------------
   * Input registers (stacked, automatic)
   * ----------------------------------------------------------------------
   * All the stack registers are callee save, but we must reserve some regs
   * for the input variables, the output variables, and special stack
   * regs (such as space to spill the predicates).  There are also a few
   * static callee regs. 
   */

  num_avail_callee_regs = NUM_INT_STACKED_REG -
    (num_reg_stack_inputs + num_reg_stack_outputs + NUM_SPECIAL_INT_REG
     + need_psp) + NUM_CALLEE_INT_STATIC_REG;

  first_callee_reg = MIN_INT_STACKED_REGISTER_ID + num_reg_stack_inputs;

  for (avail = NUM_CALLEE_INT_STATIC_REG, reg = first_callee_reg;
       avail < num_avail_callee_regs; avail++, reg++)
    callee_int_reg_map[avail] = reg;

  R_define_physical_bank_with_rot (R_CALLEE,
				   R_INT,
				   num_avail_callee_regs,
				   1,
				   R_OVERLAP_INT, callee_int_reg_map,
				   &callee_int_set,
				   num_avail_callee_regs, 0, 1);

  /* Scavengable preserved int macro registers
   * ----------------------------------------------------------------------
   * Input registers (stacked, automatic)
   * ----------------------------------------------------------------------
   * Allocable when not used in their special fashion.  For Tahoe, the
   * input registers can be reused and are callee saved.  
   */

  for (avail = 0, reg = L_MAC_P0;
       avail < num_reg_stack_inputs; avail++, reg++)
    callee_int_macro_map[avail] = reg;

  R_define_physical_bank_with_rot (R_MACRO_CALLEE,
				   R_INT,
				   num_reg_stack_inputs,
				   1,
				   R_OVERLAP_INT,
				   callee_int_macro_map,
				   &callee_int_macro_set,
				   num_reg_stack_inputs, 0, 1);

  /* Scavengable scratch int macro registers
   * ----------------------------------------------------------------------
   * Output registers (stacked, automatic)
   * Int return registers
   * ----------------------------------------------------------------------
   * Allocatable when not used in their special fashion.  For Tahoe,
   * the output registers and the return register can be reused and
   * are caller saved.  
   */

  for (avail = 0, reg = L_MAC_P8;
       avail < num_reg_stack_outputs; avail++, reg++)
    caller_int_macro_map[avail] = reg;

  for (reg = L_MAC_P16; reg < L_MAC_P16 + MAX_INT_RETURN_REGS; avail++, reg++)
    caller_int_macro_map[avail] = reg;

  R_define_physical_bank (R_MACRO_CALLER,
			  R_INT,
			  num_reg_stack_outputs + MAX_INT_RETURN_REGS,
			  1,
			  R_OVERLAP_INT,
			  caller_int_macro_map, &caller_int_macro_set);


  /* REH 4/19/95  - Make sure that the function mask pe flag
   *                is set to avoid stupid errors!
   */
  mask = 0;
  for (cb = fn->first_cb; cb && !mask; cb = cb->next_cb)
    {
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
	{
	  if (oper->flags & L_OPER_MASK_PE)
	    {
	      fn->flags = L_SET_BIT_FLAG (fn->flags, L_FUNC_MASK_PE);
	      mask = 1;
	      break;
	    }			/* if */
	}			/* for oper */
    }				/* for cb */

  /* Reset the register usage sets */

  caller_int_set = Set_dispose (caller_int_set);
  callee_int_set = Set_dispose (callee_int_set);
  callee_int_macro_set = Set_dispose (callee_int_macro_set);
  caller_int_macro_set = Set_dispose (caller_int_macro_set);
  callee_float_set = Set_dispose (callee_float_set);
  caller_float_set = Set_dispose (caller_float_set);
  caller_float_macro_set = Set_dispose (caller_float_macro_set);
  caller_double_set = Set_dispose (caller_double_set);
  callee_double_set = Set_dispose (callee_double_set);
  caller_double_macro_set = Set_dispose (caller_double_macro_set);
  caller_predicate_set = Set_dispose (caller_predicate_set);
  callee_predicate_set = Set_dispose (callee_predicate_set);
  caller_branch_set = Set_dispose (caller_branch_set);
  callee_branch_set = Set_dispose (callee_branch_set);

  /*
   * Perform register allocation
   * ----------------------------------------------------------------------
   */

  if (L_do_software_pipelining)
    /* MCM - Clear the way for software pipeliner use of the in regs. */
    Lpipe_move_int_parm_regs (fn);

  R_register_allocation_sep (fn, command_line_macro_list,
			     int_swap_space_size,
			     fp_swap_space_size, pred_swap_space_size);

  {
    /*
     * Look for r4...r7 and add to callee save set if found.
     * The register allocator may use these to spill predicates around
     * JSRs.
     */

    L_Cb *cb;
    L_Oper *oper;
    L_Operand *operand;
    int i;

    for (cb = fn->first_cb; cb; cb = cb->next_cb)
      {
	for (oper = cb->first_op; oper; oper = oper->next_op)
	  {
	    for (i = 0; i < L_max_dest_operand; i++)
	      {
		operand = oper->dest[i];
		if (!operand || !L_is_register (operand))
		  continue;
		if ((operand->value.r >= 4) && (operand->value.r <= 7))
		  {
		    callee_int_set = Set_add (callee_int_set,
					      operand->value.r);
		  }		/* if */
	      }			/* for i */
	  }			/* for oper */
      }				/* for cb */
  }

  /*
   * Make arrays of callee registers for store / load generation
   * ----------------------------------------------------------------------
   */

  /* INTEGER */

  stacked_callee_int_num = Set_size (callee_int_set);
  if (stacked_callee_int_num)
    {
      int i;
      callee_int_array = (int *) malloc (sizeof (int) *
					 stacked_callee_int_num);
      Set_2array (callee_int_set, callee_int_array);
      /* Some callee integer registers are saved by the register stack */
      callee_int_num = 0;
      for (i = 0; i < stacked_callee_int_num; i++)
	if (callee_int_array[i] < (INT_REG_BASE + 32))
	  callee_int_array[callee_int_num++] = callee_int_array[i];
      stacked_callee_int_num -= callee_int_num;
    }				/* if */
  else
    {
      callee_int_array = NULL;
      callee_int_num = 0;
    }				/* else */

  if (callee_int_num)
    L_warn ("O_register_allocation: using static callee registers");

  /* FLOAT */

  callee_tmp_flt_set = Set_union (callee_float_set, callee_double_set);
  callee_flt_num = Set_size (callee_tmp_flt_set);
  if (callee_flt_num)
    {
      callee_flt_array = (int *) malloc (sizeof (int) * callee_flt_num);
      Set_2array (callee_tmp_flt_set, callee_flt_array);
    }				/* if */
  else
    {
      callee_flt_array = NULL;
    }				/* else */

  /* PREDICATE */

  /* For some reason, p0 is included in the callee set -- remove it) */
  callee_predicate_set = Set_delete (callee_predicate_set, 0);
  callee_pred_num = Set_size (callee_predicate_set);
  /* callee predicate block is saved on register stack */
  callee_pred_array = NULL;

  /* BRANCH */

  callee_btr_num = Set_size (callee_branch_set);
  if (callee_btr_num != 0)
    {
      callee_btr_array = (int *) malloc (sizeof (int) * callee_btr_num);
      callee_btr_num = Set_2array (callee_branch_set, callee_btr_array);
    }				/* if */
  else
    {
      callee_btr_array = NULL;
    }				/* else */

  caller_int_set = Set_dispose (caller_int_set);
  callee_int_set = Set_dispose (callee_int_set);
  callee_int_macro_set = Set_dispose (callee_int_macro_set);
  caller_int_macro_set = Set_dispose (caller_int_macro_set);
  caller_float_set = Set_dispose (caller_float_set);
  callee_float_set = Set_dispose (callee_float_set);
  caller_float_macro_set = Set_dispose (caller_float_macro_set);
  caller_double_set = Set_dispose (caller_double_set);
  callee_double_set = Set_dispose (callee_double_set);
  caller_double_macro_set = Set_dispose (caller_double_macro_set);
  caller_predicate_set = Set_dispose (caller_predicate_set);
  callee_predicate_set = Set_dispose (callee_predicate_set);
  caller_branch_set = Set_dispose (caller_branch_set);
  callee_branch_set = Set_dispose (callee_branch_set);
  callee_tmp_flt_set = Set_dispose (callee_tmp_flt_set);
}				/* O_register_allocation */
