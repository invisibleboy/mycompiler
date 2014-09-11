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
 *  File:  phase1_preload.c
 *
 *  Description:
 *    Moves constant loading out of loops
 *
 *  Authors:  Sabrina Hwu and Rick Hank, University of Illinois
 *  Modified for Intel EM - Jim Pierce
 *
 *
\************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "mia_internal.h"

#undef DEBUG_CB_WEIGHT
#undef DEBUG_PRELOAD

#define ERR	stdout
/*=========================================================================*/

/*
 *	Also consider static weight that is
 *	determined by loop analysis.
 */
#undef CONSIDER_STATIC_ESTIMATION

/*
 *	We consider preloading from 1st level loops first.
 *	If they are too big, or not important enough,
 *	then consider preload higher level loops.
 *	1. if a loop is too large, then the life time
 *		of the preloaded constants would be too long,
 *		and due to the size, we expect many live registers.
 *	2. if a loop is not important enough, we don't want to
 *		add live-ranges, that may impede preloading for
 *		more important loops that are nested.
 */

#define MAX_LOOP_NESTING		10

#define MIN_LOOP_PRELOAD_WEIGHT		50.0
#define MIN_LOOP_ITER_FOR_PRELOAD	2.0
#define MAX_PRELOAD_CONST_PER_LOOP	16
#define MAX_LOOP_SIZE_WITH_NESTED_LOOPS	16
#define MAX_LOOP_SIZE_FOR_PRELOAD	32	/* basic blocks */

/*
 *	We use two criteria to decide whether or not
 *	a constant should be preloaded.
 *	1. is it important enough.
 *	2. how many can we preload.
 */

double Lconst_min_preload_weight = MIN_LOOP_PRELOAD_WEIGHT;
int Lconst_max_preload_const_per_loop = MAX_PRELOAD_CONST_PER_LOOP;
int Lconst_max_loop_size_with_nested_loops = MAX_LOOP_SIZE_WITH_NESTED_LOOPS;
int Lconst_max_loop_size_for_preload = MAX_LOOP_SIZE_FOR_PRELOAD;

static int
need_preload (L_Oper * oper, L_Operand * operand, int src_num)
{
  int val;

  switch (L_operand_case_type (operand))
    {
    case L_OPERAND_INT:
      val = operand->value.i;
      /* data operand of a store cannot be an immediate */
      if (L_store_opcode (oper))
	{
	  if (src_num == 2)
	    return (1);
	  else
	    return (0);
	}
      else if (L_cond_branch_opcode (oper))
	{
	  if (SIMM_8 (val))
	    return (0);
	  else
	    return (1);
	}
      else
	{
	  if (SIMM_14 (val))
	    return (0);
	  else
	    return (1);
	}
    case L_OPERAND_FLOAT:
      return (1);
    case L_OPERAND_DOUBLE:
      return (1);
    case L_OPERAND_STRING:
      return (1);
    case L_OPERAND_LABEL:
      if (L_subroutine_call_opcode (oper) && src_num == 0)
	return (0);
      else
	return (1);
    case L_OPERAND_CB:
      return (0);
    default:
      return (0);
    }
}

/*---------------------------------------------------------------------------*/
/* request */
#define L_MAX_REQUEST	2048
static int n_request = 0;
static L_Oper *request_oper[L_MAX_REQUEST];
static L_Operand **request_operand[L_MAX_REQUEST];

/* the operation and operand that needs preload */
static int n_preload = 0;
static L_Operand preload_constant[L_MAX_REQUEST];
static L_Operand *preload_register[L_MAX_REQUEST];
static double preload_weight[L_MAX_REQUEST];
static int preload[L_MAX_REQUEST];

static void
flush_preload_request ()
{
  n_request = 0;
  n_preload = 0;
}

static void
request_for_preload (L_Oper * oper, L_Operand ** operand)
{
  int k;
  /*
   *  add a request.
   */
  k = n_request++;
  if (k >= L_MAX_REQUEST)
    L_punt ("L_constant_generation: too many requests");
  request_oper[k] = oper;
  request_operand[k] = operand;
  /*
   *  record the constant operand.
   */
  for (k = 0; k < n_preload; k++)
    {
      if (L_same_operand (&preload_constant[k], *operand))
	{
	  preload_weight[k] += oper->weight;
	  break;
	}
    }
  if (k == n_preload)
    {				/* add an entry */
      k = n_preload++;
      preload_constant[k] = **operand;
      preload_weight[k] = oper->weight;
    }
}

/* global var n_preload */

static void
add_preload_code (L_Func * fn, L_Cb * preheader_cb)
{
  struct Element list1[L_MAX_REQUEST], list2[L_MAX_REQUEST];
  L_Operand *reg_operand;
  int i;
  /*
   *  Decide what to preload.
   */
  for (i = 0; i < n_preload; i++)
    {
      list1[i].ptr = NULL;
      list1[i].weight = preload_weight[i];
      list1[i].index = i;
    }
  merge_sort (list1, list2, n_preload);

  for (i = 0; i < n_preload; i++)
    {
      int ctype, index = list2[i].index;

      if (list2[i].weight < Lconst_min_preload_weight)
	{
	  preload[index] = 0;
	}
      else if (i >= Lconst_max_preload_const_per_loop)
	{
	  preload[index] = 0;
	}
      else
	{
	  preload[index] = 1;

	  if (L_is_ctype_int_direct (preload_constant[index].ctype))
	    ctype = L_CTYPE_LLONG;
	  else if (L_is_ctype_float_direct (preload_constant[index].ctype))
	    ctype = L_CTYPE_FLOAT;
	  else if (L_is_ctype_double_direct (preload_constant[index].ctype))
	    ctype = L_CTYPE_DOUBLE;
	  else
	    ctype = preload_constant[index].ctype;

	  reg_operand = L_new_register_operand (++(fn->max_reg_id), ctype, 0);

	  if (L_is_ctype_LOCAL_ABS_direct (ctype) ||
	      L_is_ctype_LOCAL_GP_direct (ctype) ||
	      L_is_ctype_GLOBAL_ABS_direct (ctype) ||
	      L_is_ctype_GLOBAL_GP_direct (ctype))
	    {
	      L_assign_type_int_register (reg_operand);
	    }

	  preload_register[index] = reg_operand;

	}
    }
  /*
   *  Insert preload code in the loop preheader.
   */
  for (i = 0; i < n_preload; i++)
    {
      /*
       *  generate a mov operation.
       */
      int opcode = 0, ctype;
      L_Oper *oper;

      if (!preload[i])
	continue;

#ifdef DEBUG_PRELOAD
      fprintf (ERR, "# preload ");
      L_print_operand (ERR, &preload_constant[i], 1);
      fprintf (ERR, " into ");
      L_print_operand (ERR, preload_register[i], 1);
      fprintf (ERR, "\n");
      fprintf (ERR, "preload weight = %f\n", preload_weight[i]);
#endif

      ctype = preload_constant[i].ctype;

      if (L_is_ctype_integer_direct (ctype) ||
	  L_is_ctype_LOCAL_ABS_direct (ctype) ||
	  L_is_ctype_LOCAL_GP_direct (ctype) ||
	  L_is_ctype_GLOBAL_ABS_direct (ctype) ||
	  L_is_ctype_GLOBAL_GP_direct (ctype))
	{
	  opcode = Lop_MOV;
	}
      else if (L_is_ctype_float_direct (ctype))
	{
	  opcode = Lop_MOV_F;
	}
      else if (L_is_ctype_double_direct (ctype))
	{
	  opcode = Lop_MOV_F2;
	}
      else
	{
	  L_punt ("L_constant_generation: illegal ctype");
	}

      oper = L_create_new_op (opcode);
      /* assign the preload register operand created above to */
      /* the destination of the preload instruction, this way */
      /* I don't need to copy the operand, and I don't need   */
      /* to free it                                           */
      oper->dest[0] = preload_register[i];
      /* copy the operand containing the constant to be preloaded */
      oper->src[0] = L_copy_operand (&preload_constant[i]);

      /*
       *  Insert in the loop preheader.
       */
      L_insert_oper_after (preheader_cb, preheader_cb->last_op, oper);

#ifdef DEBUG_PRELOAD
      fprintf (ERR, "# add ");
      L_print_oper (ERR, oper);
      fprintf (ERR, " to cb %d\n", preheader_cb->id);
#endif
    }
}

static void
replace_preload_operand ()
{
  int i, k;

  for (i = 0; i < n_preload; i++)
    {

      if (!preload[i])
	continue;

      for (k = 0; k < n_request; k++)
	{
	  if (L_same_operand (*request_operand[k], &preload_constant[i]))
	    {
#ifdef DEBUG_PRELOAD
	      fprintf (ERR, "# change ");
	      L_print_operand (ERR, *request_operand[k], 1);
	      fprintf (ERR, " to ");
	      L_print_operand (ERR, preload_register[i], 1);
	      fprintf (ERR, " in oper %d\n", request_oper[k]->id);
#endif
	      L_delete_operand (*request_operand[k]);
	      *request_operand[k] = L_copy_operand (preload_register[i]);
	    }
	}
    }
}

/*---------------------------------------------------------------------------*/
static void
preload_loop (L_Func * fn, L_Loop * loop)
{
  int i, j, loop_cb[MAX_LOOP_SIZE_FOR_PRELOAD + 1], n_loop_cb;

  /*
   *  Decode which constants to be preloaded.
   */
  flush_preload_request ();
  n_loop_cb = Set_2array (loop->loop_cb, loop_cb);
  for (i = 0; i < n_loop_cb; i++)
    {
      L_Oper *oper;
      L_Cb *cb = L_cb_hash_tbl_find (fn->cb_hash_tbl, loop_cb[i]);

      for (oper = cb->first_op; oper != 0; oper = oper->next_op)
	{
	  for (j = 0; j < L_max_src_operand; j++)
	    {
	      if (oper->src[j] == NULL)
		continue;

	      if (need_preload (oper, oper->src[j], j))
		{
#ifdef DEBUG_PRELOAD
		  fprintf (ERR, "# request preload of ");
		  L_print_operand (ERR, oper->src[j], 1);
		  fprintf (ERR, "\n");
#endif
		  request_for_preload (oper, &oper->src[j]);
		}
	    }
	}
    }
#ifdef DEBUG_PRELOAD
  fprintf (ERR, "# %d preload requests for loop %d\n", n_request, loop->id);
#endif
  add_preload_code (fn, loop->preheader);
  replace_preload_operand ();
}

/*---------------------------------------------------------------------------*/
/*
 *	CONSTANT GENERATION.
 */
void
Mopti_constant_generation (L_Func * fn)
{
  double ave_iter;
  int nest;
  if (fn->first_cb == NULL)
    return;

  /*
   *  Compute the weight of operations.
   *  Use both dynamic and static weights.
   */

  L_loop_detection (fn, 1);
  L_compute_oper_weight (fn, 1, 1);

  /*
   *  The entry block must not be a loop header.
   */
  if (fn->first_cb->src_flow != NULL)
    L_punt ("L_constant_generation: entry block must not be a loop header");

  /*
   *  Determine preload code for each loop structure.
   */

  for (nest = 1; nest < MAX_LOOP_NESTING; nest++)
    {
      /*
       *  Work from nesting level 1 up.
       */
      L_Loop *loop = fn->first_loop;
      for (; loop != NULL; loop = loop->next_loop)
	{
	  L_Cb *preheader_cb, *header_cb;
	  double preheader_weight, header_weight;

	  /*
	   *  Always start from the outer loop and go inward.
	   */
	  if (loop->nesting_level != nest)
	    continue;

	  /*
	   *  Must fulfill the following condition.
	   *  1. loop size < Lconst_max_loop_size_for_preload
	   *  2. loop weight > Lconst_min_preload_weight
	   *  3. if loop is a nested loop, if doesn't iterate enough, skip and
	   *          preload outer loop.
	   */
	  preheader_cb = loop->preheader;
	  header_cb = loop->header;

	  preheader_weight = preheader_cb->weight + preheader_cb->weight2;
	  header_weight = header_cb->weight + header_cb->weight2;

	  ave_iter = (header_weight - preheader_weight) / preheader_weight;


#ifdef DEBUG_PRELOAD
	  fprintf (stderr, "Loop preheader = %d  header = %d\n",
		   preheader_cb->id, header_cb->id);

	  fprintf (stderr, "preheader weight = %f %f\n",
		   preheader_cb->weight, preheader_cb->weight2);

	  fprintf (stderr, "header weight = %f %f\n",
		   header_cb->weight, header_cb->weight2);

	  fprintf (stderr, "ave_iter = %f  %f\n",
		   ave_iter, MIN_LOOP_ITER_FOR_PRELOAD);
#endif

	  if (ave_iter < MIN_LOOP_ITER_FOR_PRELOAD)
	    continue;

	  if (Set_empty (loop->nested_loops))
	    {
	      if (Set_size (loop->loop_cb) > Lconst_max_loop_size_for_preload)
		continue;
	    }
	  else
	    {
	      if (Set_size (loop->loop_cb) >
		  Lconst_max_loop_size_with_nested_loops)
		continue;
	    }

	  if (header_weight < Lconst_min_preload_weight)
	    continue;

	  /*
	   *  Preload this loop now.
	   */
	  preload_loop (fn, loop);
	}
    }
}

/*=========================================================================*/
