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
 *  File:  l_speculate.c
 *
 *  Description:  Support for Sentinel Speculation Model
 *
 *  Creation Date :  June 1996
 *
 *  Author:  David August, Wen-mei Hwu
 *
 *  Modified: January 2001, Ronald Barnes - Rewritten to support IA64
 *                                          speculation model 
 *                                          (and add SPECID's)
 *
\*****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>

#undef DEBUG_SPEC

/*****************************************************************************\
 *
 * Routines which support Sentinel Scheduling with Recovery Blocks
 *
\*****************************************************************************/

L_Oper *
L_insert_check (L_Cb * cb, L_Oper * oper)
{
  return (L_global_insert_check_after (oper, cb, oper));
}

L_Oper *
L_insert_check_after (L_Cb * cb, L_Oper * oper, L_Oper * after)
{
  return (L_global_insert_check_after (oper, cb, after));
}

L_Oper *
L_insert_check_before (L_Cb * cb, L_Oper * oper, L_Oper * before)
{
  return (L_global_insert_check_before (oper, cb, before));
}

L_Oper *
L_global_insert_check_after (L_Oper * oper, L_Cb * cb_after, L_Oper * after)
{
  L_Oper *check_op;
  L_Attr *specid_attr;

  if (!L_generate_spec_checks)
    return (NULL);

#ifdef DEBUG_SPEC
  printf ("Adding check for oper %d\n", oper->id);
#endif /* DEBUG_SPEC */

  check_op = L_create_new_op (Lop_CHECK);
  check_op->pred[0] = L_copy_operand (oper->pred[0]);
  check_op->src[0] = L_copy_operand (oper->dest[0]);
  check_op->flags = L_SET_BIT_FLAG (check_op->flags, L_OPER_CHECK);

  if (!(specid_attr = L_find_attr (oper->attr, "SPECID")))
    {
      specid_attr = L_new_attr ("SPECID", 1);
      L_set_int_attr_field (specid_attr, 0, L_fn->max_spec_id++);
      oper->attr = L_concat_attr (oper->attr, specid_attr);
    }

  check_op->attr = L_concat_attr (check_op->attr, L_copy_attr (specid_attr));

  L_insert_oper_after (cb_after, after, check_op);

  return (check_op);
}

L_Oper *
L_global_insert_check_before (L_Oper * oper, L_Cb * cb_before,
                              L_Oper * before)
{
  L_Oper *check_op;
  L_Attr *specid_attr;

  if (!L_generate_spec_checks)
    return (NULL);

#ifdef DEBUG_SPEC
  printf ("Adding check for oper %d\n", oper->id);
#endif /* DEBUG_SPEC */

  check_op = L_create_new_op (Lop_CHECK);
  check_op->pred[0] = L_copy_operand (oper->pred[0]);
  check_op->src[0] = L_copy_operand (oper->dest[0]);
  check_op->flags = L_SET_BIT_FLAG (check_op->flags, L_OPER_CHECK);

  if (!(specid_attr = L_find_attr (oper->attr, "SPECID")))
    {
      specid_attr = L_new_attr ("SPECID", 1);
      L_set_int_attr_field (specid_attr, 0, L_fn->max_spec_id++);
      oper->attr = L_concat_attr (oper->attr, specid_attr);
    }

  check_op->attr = L_concat_attr (check_op->attr, L_copy_attr (specid_attr));

  L_insert_oper_after (cb_before, before, check_op);

  return (check_op);
}

void
L_mark_oper_speculative (L_Oper * oper)
{
  oper->flags = L_SET_BIT_FLAG (oper->flags, L_OPER_SPECULATIVE);

  if (L_mask_potential_exceptions && 
      L_is_pei (oper) &&
      !L_is_trivially_safe (oper))
    oper->flags = L_SET_BIT_FLAG (oper->flags, L_OPER_MASK_PE);

  return;
}

void
L_assign_all_checks(L_Func *fn, L_Oper *from_op, L_Oper *to_op)
{
  L_Cb   *cb = NULL;
  L_Oper *op = NULL;
  L_Attr *attr = NULL;
  int from_id, to_id;

  if (!L_generate_spec_checks)
    return;

  if(!(attr = L_find_attr(from_op->attr,"SPECID")))
    L_punt("L_assign_all_checks: from_op %d "
	   "found without SPECID\n", from_op->id);
  from_id = attr->field[0]->value.i;

  if(!(attr = L_find_attr(to_op->attr,"SPECID")))
    L_punt("L_assign_all_checks: to_op %d "
	   "found without SPECID\n", to_op->id);
  to_id = attr->field[0]->value.i;

  for (cb=fn->first_cb; cb; cb=cb->next_cb)
    {
      for (op=cb->first_op; op; op=op->next_op)
	{
	  if (op->opc != Lop_CHECK)
	    continue;
	  if(!(attr = L_find_attr(op->attr,"SPECID")))
	    L_punt("L_assign_all_checks: check_op %d "
		   "found without SPECID\n", op->id);
	  if (attr->field[0]->value.i == from_id)
	    attr->field[0]->value.i = to_id;
	}
    }
} 
