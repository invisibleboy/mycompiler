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
 *  File:  ltahoe_redux.c
 *
 *  Description:
 *      Performs tahoe specific code optimizations:
 *	1) and/cmp sequence to a tstb instruction
 *
 *  Creation Date :  Feb 1997
 *
 *  Author:  Mark Tozer, Bob McGowan
 *
 *  Revisions: Bob McGowan - 4/97 - added andcm optimization
 *
\*****************************************************************************/
/* 09/17/02 REK Updating to use function from libtahoeop instead of Tmdes. */

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "ltahoe_main.h"
#include <Lcode/l_opti.h>
#include <Lcode/ltahoe_op_query.h>
#include <Lcode/ltahoe_completers.h>

#define DEBUG_TBIT_REDU  0
#define DEBUG_EXTR_REDU  0
#define DEBUG_SP_REMOVAL 0

#define LTA_REDUX_MAX_ITER 1

/* prototypes */

static int Ltahoe_local_test_bit_reduction (L_Cb * cb);
static int Ltahoe_local_extract_reduction (L_Cb * cb);
static int Ltahoe_local_ldf_reduction (L_Cb * cb);
static int Ltahoe_local_deposit_reduction (L_Cb * cb);

int
Ltahoe_reduce (L_Func * fn)
{
  int i, tbit_redu = 0, extr_redu = 0, depo_redu = 0, ldf_redu = 0;
  L_Cb *cb;

  if (!L_alloc_danger_ext)
    L_alloc_danger_ext = L_create_alloc_pool ("L_Danger_Ext",
					      sizeof (struct L_Danger_Ext),
					      64);

  LTD ("Flow analysis LV due to tahoe reductions");
  L_do_flow_analysis (fn, LIVE_VARIABLE);

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      for (i = 0; i < LTA_REDUX_MAX_ITER; i++)
	{
	  int c1 = 0, c2 = 0, c3 = 0, c4 = 0;

	  if (Ltahoe_do_tbit_redux)
	    {
	      c1 = Ltahoe_local_test_bit_reduction (cb);
	      tbit_redu += c1;
	    }

	  if (Ltahoe_do_extr_redux)
	    {
	      c2 = Ltahoe_local_extract_reduction (cb);
	      extr_redu += c2;
	    }

	  if (Ltahoe_do_depo_redux)
	    {
	      c3 = Ltahoe_local_deposit_reduction (cb);
	      depo_redu += c3;
	    }

	  if (Ltahoe_do_ldf_redux)
	    {
	      c4 = Ltahoe_local_ldf_reduction (cb);
	      ldf_redu += c4;
	    }

	  if (!(c1 + c2 + c3 + c4))
	    break;
	}
    }

  L_free_alloc_pool (L_alloc_danger_ext);
  L_alloc_danger_ext = NULL;

  return (tbit_redu + extr_redu + depo_redu + ldf_redu);
}


static int
Ltahoe_local_test_bit_reduction (L_Cb * cb)
{
  L_Oper *opA, *opB, *new_oper, *next_op;
  int change;
  int bitnum;
  int new_proc_op;
  int new_completer = 0;

  /* Looking for:
   *  and  r1 = 2^n,r2
   *  cmp.eq.ctype p1,p2 = 0, r1
   */

  change = 0;

  for (opA = cb->first_op; opA; opA = opA->next_op)
    {
      L_Operand *operandAConstSrc, *operandBZeroSrc;
      L_Operand *operandAOtherSrc, *operandBOtherSrc;

      if (opA->opc == Lop_AND)
	{
	  /* find if either src is a constant */
	  if (L_is_int_constant (opA->src[0]))
	    {
	      operandAConstSrc = opA->src[0];
	      operandAOtherSrc = opA->src[1];
	    }			/* if */
	  else if (L_is_int_constant (opA->src[1]))
	    {
	      operandAConstSrc = opA->src[1];
	      operandAOtherSrc = opA->src[0];
	    }			/* else if */
	  else
	    {
	      continue;
	    }			/* else */

	  /* eight bit immediate with "and", check that only one bit is
	   * set */
	  switch ((int) operandAConstSrc->value.i)
	    {
	    case 1:
	      bitnum = 0;
	      break;

	    case 2:
	      bitnum = 1;
	      break;

	    case 4:
	      bitnum = 2;
	      break;

	    case 8:
	      bitnum = 3;
	      break;

	    case 16:
	      bitnum = 4;
	      break;

	    case 32:
	      bitnum = 5;
	      break;

	    case 64:
	      bitnum = 6;
	      break;

	    case 128:
	      bitnum = 7;
	      break;

	    default:
	      bitnum = 255;
	      break;
	    }			/* switch */

	  /* not a power of two */
	  if (bitnum == 255)
	    continue;
	}			/* if */
      else if (opA->proc_opc == TAHOEop_EXTR_U)
	{
	  operandAOtherSrc = opA->src[0];

	  /* Length of extract must be 1 bit. */
	  if (!L_is_int_constant (opA->src[1]) ||
	      !L_is_int_constant (opA->src[2]) || 
	      opA->src[2]->value.i != 1)
	    continue;

	  /* Grab the position of the bit. */
	  bitnum = opA->src[1]->value.i;
	}			/* else if */
      else
	{
	  continue;
	}			/* else */

      for (opB = opA->next_op; opB; opB = next_op)
	{
	  next_op = opB->next_op;

	  if (!L_no_danger (L_has_fragile_macro_operand (opA),
			    0, 0, opA, opB))
	    break;

	  if (!L_int_eq_cmp_opcode (opB) && !L_int_ne_cmp_opcode (opB))
	    continue;

	  if ((L_is_int_zero (opB->src[0])) || LT_is_R0_operand (opB->src[0]))
	    {
	      operandBZeroSrc = opB->src[0];
	      operandBOtherSrc = opB->src[1];
	    }			/* if */
	  else if ((L_is_int_zero (opB->src[1])) ||
		   LT_is_R0_operand (opB->src[1]))
	    {
	      operandBZeroSrc = opB->src[1];
	      operandBOtherSrc = opB->src[0];
	    }			/* else if */
	  else
	    {
	      continue;
	    }			/* else */

	  if (!L_same_operand (opA->dest[0], operandBOtherSrc))
	    {
	      continue;
	    }			/* if */

	  if (!PG_superset_predicate_ops (opA, opB))
	    continue;

	  if (!L_no_defs_between (opA->dest[0], opA, opB))
	    continue;

	  if (!L_no_defs_between (operandAOtherSrc, opA, opB))
	    continue;

	  /*  Replace 
	     cmp.eq.ctype p1,p2 = 0, r1    with
	     tbit.z.ctype p1,p2 = r2,pos6

	     or

	     cmp.ne.ctype p1,p2 = 0, r1    with
	     tbit.nz.ctype p1,p2 = r2,pos6
	   */

	  if (L_int_eq_cmp_opcode (opB))
	    {
	      new_proc_op = TAHOEop_TBIT;
	      TC_SET_CMP_OP (new_completer, TC_CMP_OP_Z);

	      new_oper = L_create_new_op_using (Lop_CMP, opB);
	      L_set_compare (new_oper, L_CTYPE_INT, Lcmp_COM_TZ);
	    }			/* if */
	  else
	    {
	      new_proc_op = TAHOEop_TBIT;
	      TC_SET_CMP_OP (new_completer, TC_CMP_OP_NZ);

	      new_oper = L_create_new_op_using (Lop_CMP, opB);
	      L_set_compare (new_oper, L_CTYPE_INT, Lcmp_COM_TN);
	    }			/* else */

	  new_oper->proc_opc = new_proc_op;
	  new_oper->completers = new_completer;
	  /* 09/20/02 REK Preserve the compare type in the new op */
	  TC_SET_CMP_TYPE (new_oper->completers,
			   TC_GET_CMP_TYPE (opB->completers));

	  new_oper->src[0] = L_copy_operand (operandAOtherSrc);
	  new_oper->src[1] = L_new_gen_int_operand (bitnum);
	  new_oper->dest[0] = L_copy_operand (opB->dest[0]);
	  new_oper->dest[1] = L_copy_operand (opB->dest[1]);

#if DEBUG_TBIT_REDU
	  fprintf (stderr, "New tbit inst: %d\n", new_oper->id);
	  L_print_oper (stderr, opA);
	  L_print_oper (stderr, opB);
#endif

	  L_insert_oper_before (cb, opB, new_oper);
	  L_delete_oper (cb, opB);

	  change++;

	}			/* for opB */
    }				/* for opA */

  return (change);
}				/* Ltahoe_local_test_bit_reduction */


#if 0
/****************************************************************************
 *
 * routine: Ltahoe_andcm_reduction()
 * purpose: Find places where andcm (and complement) can be utilized.
 *          andcm: The first source operand is logically ANDed with the 1's
 *                 complement of the second operand.
 *          Replace pattern:
 *                  MOV x = -1
 *                  XOR z = y,x
 *                  AND dest = s,z
 *          with:
 *                  ANDCM dest = s,y 
 * input:
 * output: 
 * returns:
 * modified: Bob McGowan - 4/97 - created/adapted from HP optimization
 * note:
 *-------------------------------------------------------------------------*/

static int
Ltahoe_andcm_reduction (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *oper;
  L_Oper *next_op;
  L_Oper *xor_op;
  L_Oper *and_op;
  L_Oper *new_oper;
  int ext, count = 0;

  /* Replace pattern:    MOV x,-1
     XOR  z,y,x
     AND dest,s,z

     with:    AND_COMPL dest,s,y        */

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      for (oper = cb->first_op; oper; oper = next_op)
	{
	  next_op = oper->next_op;

	  if (oper->opc != Lop_MOV)
	    continue;

	  L_get_attribute (oper, &ext);
	  if (INSTR_EXT (ext) != MOV_LDI ||
	      oper->src[0]->type != L_OPERAND_INT ||
	      oper->src[0]->value.i != -1)
	    continue;

	  xor_op = oper->next_op;
	  if (!xor_op || xor_op->opc != Lop_XOR ||
	      (!L_same_operand (xor_op->src[0], oper->dest[0]) &&
	       !L_same_operand (xor_op->src[1], oper->dest[0])))
	    continue;

	  for (and_op = xor_op->next_op; and_op; and_op = and_op->next_op)
	    {

	      if (and_op->opc != Lop_AND)
		continue;

	      if (L_same_operand (and_op->src[0], xor_op->dest[0]) ||
		  L_same_operand (and_op->src[1], xor_op->dest[0]))
		break;
	    }
	  if (!and_op)
	    continue;

	  if (D_in_oper_OUT_set
	      (cb, xor_op, oper->dest[0]->value.r, FALL_THRU_PATH)
	      || D_in_oper_OUT_set (cb, and_op, xor_op->dest[0]->value.r,
				    FALL_THRU_PATH))
	    continue;


	  if (O_is_subr_call_between (xor_op, and_op) ||
	      O_is_branch_between (cb, xor_op, and_op,
				   xor_op->dest[0]->value.r))
	    continue;

	  if (!L_no_uses_between (xor_op->dest[0], xor_op, and_op))
	    continue;

	  if (L_same_operand (xor_op->src[0], oper->dest[0]) &&
	      !L_no_defs_between (xor_op->src[1], xor_op, and_op))
	    continue;
	  else if (L_same_operand (xor_op->src[1], oper->dest[0]) &&
		   !L_no_defs_between (xor_op->src[0], xor_op, and_op))
	    continue;

	  /* create the new andcm instruction */
	  new_oper = L_create_new_op_using (Lop_AND_COMPL, and_op);

	  if (L_same_operand (xor_op->dest[0], and_op->src[0]))
	    new_oper->src[0] = L_copy_operand (and_op->src[1]);
	  else
	    new_oper->src[0] = L_copy_operand (and_op->src[0]);

	  if (L_same_operand (xor_op->src[0], oper->dest[0]))
	    new_oper->src[1] = L_copy_operand (xor_op->src[1]);
	  else
	    new_oper->src[1] = L_copy_operand (xor_op->src[0]);

	  new_oper->dest[0] = L_copy_operand (and_op->dest[0]);

	  L_insert_oper_after (cb, and_op, new_oper);

	  count += 1;

	  /* delete all original operations */
	  L_delete_oper (cb, oper);
	  L_delete_oper (cb, xor_op);
	  L_delete_oper (cb, and_op);

	  next_op = new_oper->next_op;
	}
    }
  return (count);
}

#endif


/* Ltahoe_sp_removal(fn)
 *
 * Find's uses of sp in prologs and epilogs that cancel each other and 
 * removes them!
 *
 * written by Chad Lester as his first experimental optimization
 * this routine saved a clock cycle in 'cpylist' in go.
 */

void
Ltahoe_sp_removal (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *op;
  int temp;

  struct s_remlist {
    L_Cb *cb;
    L_Oper *op;
    struct s_remlist *prev;
  } *rlist = NULL, *work;

  struct {
    int got_prolog;
    int prolog_val;
    int epilog_count;
    int other_use;
  } stats = {0, 0, 0, 0};

  int maxoutput = 36;

#define db(x) (DEBUG_SP_REMOVAL && (maxoutput-- > 0) && printf x )

  temp = db (("Chad's Optimization Code working on %s!\n", fn->name));

  L_rebuild_src_flow (fn);

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      int sp_count = 0;

      temp = db (("  Cb %i:\n", cb->id));

      for (op = cb->first_op; op; op = op->next_op)
	{
	  int i, sp_flag = 0;

	  for (i = 0; i < L_max_src_operand; i++)
	    {
	      if (!L_is_macro (op->src[i]))
		continue;
	      if (!(op->src[i]->value.mac == L_MAC_SP))
		continue;
	      sp_flag = 1;
	    }

	  for (i = 0; i < L_max_dest_operand; i++)
	    {
	      if (!L_is_macro (op->dest[i]))
		continue;
	      if (!(op->dest[i]->value.mac == L_MAC_SP))
		continue;
	      sp_flag = 1;
	    }

	  if (sp_flag)
	    {
	      struct s_remlist *work = calloc (sizeof (*work), 1);
	      int ival;
	      temp = db (("FOUND POTENTIAL INSTRUCTION:\n"));
	      if (DEBUG_SP_REMOVAL && (maxoutput-- > 0))
		L_print_oper (stdout, op);
	      work->prev = rlist;
	      work->cb = cb;
	      work->op = op;
	      rlist = work;

	      sp_count++;

	      /* do additional checking */

	      if (!L_int_add_opcode (op))
		{
		  temp = db (("NOT AND iADD INSTRUCTION!!!!\n"));
		  stats.other_use++;
		  continue;
		}
	      ival = 0;
	      for (i = 0; i < L_max_src_operand; i++)
		{
		  if (!L_is_int_constant (op->src[i]))
		    continue;
		  ival = op->src[i]->value.i;
		}
	      if (!cb->src_flow)	/* this is a prolog */
		{
		  if (stats.got_prolog)
		    {
		      temp = db (("Multiple Prologs!"));
		      stats.other_use++;
		    }
		  stats.got_prolog = 1;
		  temp = db (("In prolog\n"));
		  stats.prolog_val = ival;
		}
	      else if (!cb->dest_flow)	/* this is an epilog */
		{
		  if (ival != -stats.prolog_val)
		    {
		      temp =
			db (("Epilog val of %d does not complement prolog "
			     "val of %d", ival, stats.prolog_val));
		      stats.other_use++;
		    }
		}
	      else
		{
		  temp =
		    db (("sp instruction in neither epilog or prolog\n"));
		  stats.other_use++;
		}
	    }
	}			/* for OP */

      if (!cb->dest_flow)	/* this is an epilog */
	{
	  stats.epilog_count++;
	  if (!sp_count)
	    stats.other_use++;	/* SP wasn't fixed in the epilog! */
	}

    }				/* for CB */

  if ((stats.got_prolog) && (stats.epilog_count) && (!stats.other_use))
    {
      /* remove the useless SP instructions! */
      for (work = rlist; work != NULL; work = work->prev)
	{
	  temp = db (("Removing Instructions\n"));
	  if (DEBUG_SP_REMOVAL && (maxoutput-- > 0))
	    L_print_oper (stdout, op);
	  L_delete_oper (work->cb, work->op);
	}
    }

  work = rlist;
  while (work)
    {
      struct s_remlist *tmp = work;
      work = work->prev;
      free (tmp);
    }
}


static int
pwr_n_minus_1 (ITintmax i)
{
  int ones = 0;
  ITuintmax u = (ITuintmax) i;

  if (u == 0)
    return 1;

  if (!(u & 1))
    return 0;

  while ((u != 0) && (u & 1))
    {
      u >>= 1;
      ones++;
    }

  if (u != 0)
    return 0;
  else
    return ones;
}


/*
 * Extract reduction
 * ======================================================================
 * Optimize sequences of extr, extr.u, and, sxt, and zxt opcodes
 * Reduce single opcodes to preferred forms
 */

typedef struct _MiaExtract
{
  L_Oper *oper;
  L_Operand *mopd;
  int uns;
  unsigned int len;
  unsigned int pos;
}
MiaExtract;


static int
Ltahoe_classify_extract (L_Oper * op, MiaExtract * me)
{
  unsigned int len = 0, pos = 0;
  int uns = 0;
  L_Operand *mopd;

  switch (op->proc_opc)
    {
    case TAHOEop_EXTR_U:
      uns = 1;
    case TAHOEop_EXTR:
      mopd = op->src[0];
      pos = op->src[1]->value.i;
      len = op->src[2]->value.i;
      break;

    case TAHOEop_ZXT1:
      uns = 1;
    case TAHOEop_SXT1:
      len = 8;
      mopd = op->src[0];
      break;

    case TAHOEop_ZXT2:
      uns = 1;
    case TAHOEop_SXT2:
      len = 16;
      mopd = op->src[0];
      break;

    case TAHOEop_ZXT4:
      uns = 1;
    case TAHOEop_SXT4:
      len = 32;
      mopd = op->src[0];
      break;

    case TAHOEop_AND:
      if (!L_is_int_constant (op->src[0]) ||
	  !(len = pwr_n_minus_1 (op->src[0]->value.i)))
	return 0;

      uns = 1;
      pos = 0;
      mopd = op->src[1];
      break;

    default:
      return 0;
    }

  if (me)
    {
      me->oper = op;
      me->mopd = mopd;
      me->uns = uns;
      me->len = len;
      me->pos = pos;
    }

  return 1;
}


static int
Ltahoe_local_extract_reduction (L_Cb * cb)
{
  L_Oper *opA, *opB;
  int uns, i, count = 0;
  unsigned int len, pos;
  MiaExtract extrA, extrB;

  for (opB = cb->last_op; opB; opB = opB->prev_op)
    {
      if (Ltahoe_classify_extract (opB, &extrB))
	{
	  int new_opc = 0, new_popc = 0, popc;
	  ITintmax mask = 0;

	  for (opA = opB->prev_op; opA; opA = opA->prev_op)
	    {
	      if (!L_same_operand (opA->dest[0], extrB.mopd))
		continue;

	      if (!PG_superset_predicate_ops (opA, opB) ||
		  !Ltahoe_classify_extract (opA, &extrA) ||
		  !L_no_defs_between (opA->dest[0], opA, opB) ||
		  !L_same_def_reachs (extrA.mopd, opA, opB))
		break;

	      pos = extrA.pos + extrB.pos;
	      uns = extrB.uns;

	      if (extrA.len <= extrB.pos)
		{
		  /* B totally in A's extension region */

		  if (extrA.uns)
		    {
		      len = 0;
		    }
		  else
		    {
		      len = extrB.len;
		      pos = extrA.pos + extrA.len - 1;
		    }
		}
	      else if (extrA.len < (extrB.pos + extrB.len))
		{
		  /* B starts in A and ends in A's extension */

		  if (extrA.uns)
		    {
		      uns = 1;
		      len = extrA.len - extrB.pos;
		    }
		  else if (extrB.uns)
		    {
		      break;
		    }
		  else
		    {
		      len = extrA.len - extrB.pos;
		    }
		}
	      else
		{
		  /* B totally inside A */
		  len = extrB.len;
		}

	      if ((pos + len) > 64)
		len = 64 - pos;

	      /* Match */

#if DEBUG_EXTR_REDU
	      fprintf (stderr, ">> REDUCING\n");
	      L_print_oper (stderr, opA);
	      L_print_oper (stderr, opB);
	      fprintf (stderr, "   TO\n");
#endif

	      if (uns)
		{
		  L_change_opcode (opB, Lop_EXTRACT_U);
		  opB->proc_opc = TAHOEop_EXTR_U;
		}
	      else
		{
		  L_change_opcode (opB, Lop_EXTRACT);
		  opB->proc_opc = TAHOEop_EXTR;
		}

	      for (i = 0; i < L_max_src_operand; i++)
		{
		  L_delete_operand (opB->src[i]);
		  opB->src[i] = NULL;
		}

	      opB->src[0] = extrB.mopd = L_copy_operand (extrA.mopd);
	      opB->src[1] = L_new_gen_int_operand (pos);
	      opB->src[2] = L_new_gen_int_operand (len);

	      extrB.uns = uns;
	      extrB.pos = pos;
	      extrB.len = len;

#if DEBUG_EXTR_REDU
	      L_print_oper (stderr, opB);
#endif

	      count++;
	    }

	  new_popc = popc = opB->proc_opc;

	  if (extrB.len == 0)
	    {
	      new_popc = TAHOEop_MOVI;
	      new_opc = Lop_MOV;
	    }
	  else if (extrB.pos == 0)
	    {
	      if (extrB.len == 8)
		{
		  new_popc = extrB.uns ? TAHOEop_ZXT1 : TAHOEop_SXT1;
		  new_opc = extrB.uns ? Lop_ZXT_C : Lop_SXT_C;
		}
	      else if (extrB.len == 16)
		{
		  new_popc = extrB.uns ? TAHOEop_ZXT2 : TAHOEop_SXT2;
		  new_opc = extrB.uns ? Lop_ZXT_C2 : Lop_SXT_C2;
		}
	      else if (extrB.len == 32)
		{
		  new_popc = extrB.uns ? TAHOEop_ZXT4 : TAHOEop_SXT4;
		  new_opc = extrB.uns ? Lop_ZXT_I : Lop_SXT_I;
		}
	      else if ((extrB.len < 8) && extrB.uns)
		{
		  new_popc = TAHOEop_AND;
		  new_opc = Lop_AND;
		  mask = (1 << extrB.len) - 1;
		}
	    }

	  if (new_popc != popc)
	    {
#if DEBUG_EXTR_REDU
	      fprintf (stderr, ">> REDUCING\n");
	      L_print_oper (stderr, opB);
#endif

	      L_change_opcode (opB, new_opc);
	      opB->proc_opc = new_popc;

	      if (opB->src[1])
		{
		  L_delete_operand (opB->src[1]);
		  opB->src[1] = NULL;
		}

	      if (opB->src[2])
		{
		  L_delete_operand (opB->src[2]);
		  opB->src[2] = NULL;
		}

	      if (new_opc == Lop_AND)
		{
		  opB->src[1] = L_copy_operand (extrB.mopd);
		  L_delete_operand (opB->src[0]);
		  opB->src[0] = L_new_gen_int_operand (mask);
		}
	      else if (new_opc == Lop_MOV)
		{
		  L_delete_operand (opB->src[0]);
		  opB->src[0] = Ltahoe_IMAC (ZERO);
		}

#if DEBUG_EXTR_REDU
	      fprintf (stderr, "   TO\n");
	      L_print_oper (stderr, opB);
#endif
	    }
	}
#if 0
      /* SXT/ZXT could be clearing upper bits! */
      else if ((opB->proc_opc == TAHOEop_SHR) ||
	       (opB->proc_opc == TAHOEop_SHR_U) ||
	       (opB->proc_opc == TAHOEop_SHL))
	{
	  /* shift value is interpreted as unsigned, so
	   * sign/zero extension is superfluous
	   */

	  if (!L_is_variable (opB->src[1]))
	    continue;

	  for (opA = opB->prev_op; opA; opA = opA->prev_op)
	    {
	      if (!PG_superset_predicate_ops (opA, opB))
		continue;
	      if (!L_same_operand (opA->dest[0], opB->src[1]))
		continue;

	      opd = opA->src[0];
	      switch (opA->proc_opc)
		{
		case TAHOEop_ZXT1:
		case TAHOEop_SXT1:
		case TAHOEop_ZXT2:
		case TAHOEop_SXT2:
		case TAHOEop_ZXT4:
		case TAHOEop_SXT4:
		  break;
		default:
		  continue;
		}

	      if (!L_same_def_reachs (opA->src[0], opA, opB))
		break;

	      if (!L_no_defs_between (opA->dest[0], opA, opB))
		break;

	      /* Replace */

	      L_delete_operand (opB->src[1]);
	      opB->src[1] = L_copy_operand (opA->src[0]);
	    }
	}
#endif
    }
  return count;
}


#if 1
static int
Ltahoe_classify_deposit (L_Oper * op, MiaExtract * me)
{
  unsigned int len = 0, pos = 0;
  int uns = 1;
  L_Operand *mopd;

  switch (op->proc_opc)
    {
    case TAHOEop_DEP_Z:
      mopd = op->src[0];
      pos = op->src[1]->value.i;
      len = op->src[2]->value.i;
      break;

    case TAHOEop_SHR_U:
      if (!L_is_int_constant (op->src[1]))
	return 0;

      mopd = op->src[0];
      pos = op->src[1]->value.i;
      len = 64 - pos;
      break;

    case TAHOEop_ZXT1:
      len = 8;
      mopd = op->src[0];
      break;

    case TAHOEop_ZXT2:
      len = 16;
      mopd = op->src[0];
      break;

    case TAHOEop_ZXT4:
      len = 32;
      mopd = op->src[0];
      break;

    case TAHOEop_AND:
      if (!L_is_int_constant (op->src[0]) ||
	  !(len = pwr_n_minus_1 (op->src[0]->value.i)))
	return 0;
      mopd = op->src[1];
      break;

    default:
      return 0;
    }

  if (me)
    {
      me->oper = op;
      me->mopd = mopd;
      me->uns = uns;
      me->len = len;
      me->pos = pos;
    }

  return 1;
}


static int
Ltahoe_local_deposit_reduction (L_Cb * cb)
{
  L_Oper *opA, *opB;
  int uns = 0, i, count = 0;
  unsigned int len, pos;
  MiaExtract depA, depB;

  for (opB = cb->last_op; opB; opB = opB->prev_op)
    {
      if (Ltahoe_classify_deposit (opB, &depB))
	{
	  int new_opc = 0, new_popc = 0, popc;
	  ITintmax mask = 0;

	  for (opA = opB->prev_op; opA; opA = opA->prev_op)
	    {
	      if (!L_same_operand (opA->dest[0], depB.mopd))
		continue;

	      if (!PG_superset_predicate_ops (opA, opB) ||
		  !Ltahoe_classify_deposit (opA, &depA) ||
		  !L_no_defs_between (opA->dest[0], opA, opB) ||
		  !L_same_def_reachs (depA.mopd, opA, opB))
		break;

	      pos = depA.pos + depB.pos;

	      if (depB.len <= depA.pos)
		{
		  len = 0;
		}
	      else if (depB.len < (depA.pos + depA.len))
		{
		  len = depB.len - depA.pos;
		}
	      else
		{
		  len = depA.len - depA.pos;
		}

	      if (pos >= 64)
		{
		  len = 0;
		  pos = 0;
		}

	      /* Match */

#if DEBUG_EXTR_REDU
	      fprintf (stderr, ">> REDUCING\n");
	      L_print_oper (stderr, opA);
	      L_print_oper (stderr, opB);
	      fprintf (stderr, "   TO\n");
#endif

	      L_change_opcode (opB, Lop_DEPOSIT);
	      opB->proc_opc = TAHOEop_DEP_Z;

	      for (i = 0; i < L_max_src_operand; i++)
		{
		  L_delete_operand (opB->src[i]);
		  opB->src[i] = NULL;
		}

	      opB->src[0] = depB.mopd = L_copy_operand (depA.mopd);
	      opB->src[1] = L_new_gen_int_operand (pos);
	      opB->src[2] = L_new_gen_int_operand (len);

	      depB.uns = uns;
	      depB.pos = pos;
	      depB.len = len;

#if DEBUG_EXTR_REDU
	      L_print_oper (stderr, opB);
#endif
	      count++;
	    }

	  new_popc = popc = opB->proc_opc;

	  if (depB.len == 0)
	    {
	      new_popc = TAHOEop_MOVI;
	      new_opc = Lop_MOV;
	    }
	  else if (depB.pos == 0)
	    {
	      if (depB.len == 8)
		{
		  new_popc = depB.uns ? TAHOEop_ZXT1 : TAHOEop_SXT1;
		  new_opc = depB.uns ? Lop_ZXT_C : Lop_SXT_C;
		}
	      else if (depB.len == 16)
		{
		  new_popc = depB.uns ? TAHOEop_ZXT2 : TAHOEop_SXT2;
		  new_opc = depB.uns ? Lop_ZXT_C2 : Lop_SXT_C2;
		}
	      else if (depB.len == 32)
		{
		  new_popc = depB.uns ? TAHOEop_ZXT4 : TAHOEop_SXT4;
		  new_opc = depB.uns ? Lop_ZXT_I : Lop_SXT_I;
		}
	      else if ((depB.len < 8) && depB.uns)
		{
		  new_popc = TAHOEop_AND;
		  new_opc = Lop_AND;
		  mask = (1 << depB.len) - 1;
		}
	    }

	  if (new_popc != popc)
	    {
#if DEBUG_EXTR_REDU
	      fprintf (stderr, ">> REDUCING\n");
	      L_print_oper (stderr, opB);
#endif

	      L_change_opcode (opB, new_opc);
	      opB->proc_opc = new_popc;

	      if (opB->src[1])
		{
		  L_delete_operand (opB->src[1]);
		  opB->src[1] = NULL;
		}

	      if (opB->src[2])
		{
		  L_delete_operand (opB->src[2]);
		  opB->src[2] = NULL;
		}

	      if (new_opc == Lop_AND)
		{
		  opB->src[1] = L_copy_operand (depB.mopd);
		  L_delete_operand (opB->src[0]);
		  opB->src[0] = L_new_gen_int_operand (mask);
		}
	      else if (new_opc == Lop_MOV)
		{
		  L_delete_operand (opB->src[0]);
		  opB->src[0] = Ltahoe_IMAC (ZERO);
		}

#if DEBUG_EXTR_REDU
	      fprintf (stderr, "   TO\n");
	      L_print_oper (stderr, opB);
#endif
	    }
	}
#if 0
      /* SXT/ZXT could be clearing upper bits! */
      else if ((opB->proc_opc == TAHOEop_SHR) ||
	       (opB->proc_opc == TAHOEop_SHR_U) ||
	       (opB->proc_opc == TAHOEop_SHL))
	{
	  /* shift value is interpreted as unsigned, so
	   * sign/zero extension is superfluous
	   */

	  if (!L_is_variable (opB->src[1]))
	    continue;

	  for (opA = opB->prev_op; opA; opA = opA->prev_op)
	    {
	      if (!PG_superset_predicate_ops (opA, opB))
		continue;
	      if (!L_same_operand (opA->dest[0], opB->src[1]))
		continue;

	      opd = opA->src[0];
	      switch (opA->proc_opc)
		{
		case TAHOEop_ZXT1:
		case TAHOEop_SXT1:
		case TAHOEop_ZXT2:
		case TAHOEop_SXT2:
		case TAHOEop_ZXT4:
		case TAHOEop_SXT4:
		  break;
		default:
		  continue;
		}

	      if (!L_same_def_reachs (opA->src[0], opA, opB))
		break;

	      if (!L_no_defs_between (opA->dest[0], opA, opB))
		break;

	      /* Replace */

	      L_delete_operand (opB->src[1]);
	      opB->src[1] = L_copy_operand (opA->src[0]);
	    }
	}
#endif
    }
  return count;
}
#endif

static int
Ltahoe_local_ldf_reduction (L_Cb * cb)
{
  L_Oper *oper, *prod_oper, *cmp_oper, *ld_oper, *tbit_oper;
  int proc_opc, is_dbl, change = 0, cmpOp = 0;

  for (oper = cb->first_op; oper; oper = oper->next_op)
    {
      /*
       * Match pattern:
       * ldf f2 = ...
       * fcmp.gt.unc p2 = f3,f0
       */

      if (oper->opc != Lop_CMP_F)
	continue;

      if (!LT_is_R0_operand (oper->src[1]))
	continue;

      /* Check to see if the producer is a single or double load */

      {
	Set rd_set;
	int rd_cnt, rd_id;

	rd_set = L_get_oper_RIN_defining_opers (oper, oper->src[0]);

	rd_cnt = (Set_size (rd_set) == 1) ? Set_2array (rd_set, &rd_id) : 0;

	Set_dispose (rd_set);

	if (rd_cnt != 1)
	  continue;

	prod_oper = L_oper_hash_tbl_find_oper (L_fn->oper_hash_tbl, rd_id);
      }

      /* Find size of load (if load) */

      switch (prod_oper->proc_opc)
	{
	case TAHOEop_LDFS:
	  is_dbl = 0;
	  break;
	case TAHOEop_LDFD:
	  is_dbl = 1;
	  break;
	default:
	  continue;
	}			/* switch */

      cmp_oper = oper;

      proc_opc = cmp_oper->proc_opc;

      if (cmp_oper->dest[1])
	{
	  L_warn ("Ltahoe_convert_ldf_to_ldi: Opti missed! (1)");
	  continue;
	}			/* if */

      if (cmp_oper->dest[0]->ptype != L_PTYPE_UNCOND_T)
	{
	  L_warn ("Ltahoe_convert_ldf_to_ldi: Opti missed! (2)");
	  continue;
	}			/* if */

      fprintf (stderr, "Coverting ld_f to ld_i: op %d -> op %d\n",
	       prod_oper->id, oper->id);

      change++;

      /* First convert floating point load to an int load */

      ld_oper = L_copy_operation (prod_oper);

      L_insert_oper_before (cb, prod_oper, ld_oper);

      if (!is_dbl)
	{
	  L_change_opcode (ld_oper, Lop_LD_I);

	  /* 09/17/02 REK The speculative bit is now in the completers
	   *              field, so proc_opc only has to change to LD4. */
	  /* if (ld_oper->proc_opc == TAHOEop_LDF_S_S) */
	  /*     ld_oper->proc_opc = TAHOEop_LD4_S; */
	  /* else */
	  /*     ld_oper->proc_opc = TAHOEop_LD4; */

	  ld_oper->proc_opc = TAHOEop_LD4;
	}			/* if */
      else
	{
	  L_change_opcode (ld_oper, Lop_LD_Q);

	  /* 09/17/02 REK The speculative bit is now in the completers
	   *              field, so proc_opc only has to change to LD8. */
	  /* if (ld_oper->proc_opc == TAHOEop_LDF_D_S) */
	  /*     ld_oper->proc_opc = TAHOEop_LD8_S; */
	  /* else */
	  /*     ld_oper->proc_opc = TAHOEop_LD8; */

	  ld_oper->proc_opc = TAHOEop_LD8;
	}			/* else */

      L_delete_operand (ld_oper->dest[0]);
      ld_oper->dest[0] = Ltahoe_new_int_reg ();

      /* Finally replace the fcmp with a cmp and a tbit */

      L_change_opcode (cmp_oper, Lop_CMP);

      cmpOp = TC_GET_CMP_OP (cmp_oper->completers);

      if ((cmpOp != TC_CMP_OP_EQ) &&
	  (cmpOp != TC_CMP_OP_GE) && (cmpOp != TC_CMP_OP_LE))
	{
	  L_set_compare (cmp_oper, L_CTYPE_LLONG, Lcmp_COM_NE);
	  cmp_oper->proc_opc = TAHOEop_CMP;
	  TC_SET_CMP_OP (cmp_oper->completers, TC_CMP_OP_NEQ);
	}			/* if */
      else
	{
	  L_set_compare (cmp_oper, L_CTYPE_LLONG, Lcmp_COM_EQ);
	  cmp_oper->proc_opc = TAHOEop_CMP;
	  TC_SET_CMP_OP (cmp_oper->completers, TC_CMP_OP_EQ);
	}			/* else */

      cmp_oper->dest[0]->ptype = L_PTYPE_UNCOND_T;
      L_delete_operand (cmp_oper->src[0]);
      cmp_oper->src[0] = L_copy_operand (ld_oper->dest[0]);
      L_delete_operand (cmp_oper->src[1]);
      cmp_oper->src[1] = Ltahoe_IMAC (ZERO);

      /* Add the tbit if needed */
      if ((cmpOp == TC_CMP_OP_GT) || (cmpOp == TC_CMP_OP_GE))
	{
	  tbit_oper = L_create_new_op (Lop_CMP);
	  L_set_compare (tbit_oper, L_CTYPE_INT, Lcmp_COM_TZ);
	  tbit_oper->proc_opc = TAHOEop_TBIT;
	  TC_SET_CMP_OP (tbit_oper->completers, TC_CMP_OP_Z);
	  TC_SET_CMP_TYPE (tbit_oper->completers, TC_CMP_TYPE_AND);
	}			/* if */
      else if ((cmpOp == TC_CMP_OP_LT) || (cmpOp == TC_CMP_OP_LE))
	{
	  tbit_oper = L_create_new_op (Lop_CMP);
	  L_set_compare (tbit_oper, L_CTYPE_INT, Lcmp_COM_TN);
	  tbit_oper->proc_opc = TAHOEop_TBIT;
	  TC_SET_CMP_OP (tbit_oper->completers, TC_CMP_OP_NZ);
	  TC_SET_CMP_TYPE (tbit_oper->completers, TC_CMP_TYPE_AND);
	}			/* else if */
      else
	{
	  /* no tbit oper required */
	  continue;
	}			/* else */

      L_insert_oper_after (cb, cmp_oper, tbit_oper);

      tbit_oper->pred[0] = L_copy_operand (cmp_oper->pred[0]);

      tbit_oper->dest[0] = L_copy_operand (cmp_oper->dest[0]);
      tbit_oper->dest[0]->ptype = L_PTYPE_AND_T;

      tbit_oper->src[0] = L_copy_operand (ld_oper->dest[0]);
      tbit_oper->src[1] = L_new_gen_int_operand (is_dbl ? 63 : 31);
    }				/* for oper */
  return change;
}				/* Ltahoe_local_ldf_reduction */
