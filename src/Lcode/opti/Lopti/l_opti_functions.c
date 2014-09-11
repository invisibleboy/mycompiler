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
 *      File :          l_opti_functions.c
 *      Description :   data struct manipulation functs for optimizer
 *      Creation Date : July, 1990
 *      Author :        Scott Mahlke, Pohua Chang.
 *
 *      (C) Copyright 1990, Pohua Chang, Scott Mahlke.
 *      All rights granted to University of Illinois Board of Regents.
 *==========================================================================*/
/* 02/07/03 REK Modifying L_convert_to_zero_extend_oper,
 *              L_convert_to_extended_move, L_convert_to_move,
 *              L_convert_to_move_of_zero, L_convert_to_move_of_one,
 *              L_convert_to_jump, L_reinit_induction_var, L_induction_elim_2,
 *              L_breakup_pre_post_inc_ops, L_generate_pre_post_inc_ops,
 *              to not optimize opers marked volatile.
 */
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "l_opti.h"

#define ERR stderr

/* debugging flags */
#undef DEBUG_BREAKUP_PRE_POST_INC

/*=========================================================================*/
/*
 *      Converting opers
 */
/*=========================================================================*/

/*
 * L_create_divide_operations
 * ----------------------------------------------------------------------
 * Create the following code sequence based on div_oper which takes b/c.
 *     t0 = b
 *     t1 = -1 * sgn(t0)
 *     t2 = t1 & (c-1)
 *     t3 = t0 + t2
 *     n = t3 >> log2(c)
 * If c is negative, all instances of b and c should be made negative.
 */
void
L_create_divide_operations (L_Oper * div_oper, L_Cb * cb)
{
  ITintmax c_value;
  L_Oper *new_oper, *prev_oper, *b_oper;

  /* create "t0 = b" */
  if (div_oper->src[1]->value.i > 0)
    {				/* c is a positive */
      c_value = div_oper->src[1]->value.i;
      b_oper = L_create_new_op (Lop_MOV);
      b_oper->dest[0] = L_new_register_operand (++L_fn->max_reg_id,
						L_native_machine_ctype,
						L_PTYPE_NULL);
      b_oper->src[0] = L_copy_operand (div_oper->src[0]);
    }
  else
    {				/* c is negative */
      c_value = -div_oper->src[1]->value.i;
      b_oper = L_create_new_op (Lop_SUB);
      b_oper->dest[0] = L_new_register_operand (++L_fn->max_reg_id,
						L_native_machine_ctype,
						L_PTYPE_NULL);
      b_oper->src[0] = L_new_gen_int_operand (0);
      b_oper->src[1] = L_copy_operand (div_oper->src[0]);
    }
  L_insert_oper_before (cb, div_oper, b_oper);

  /* create "t1 = -1 * sgn(t0)" */
  new_oper = L_create_new_op (Lop_ASR);
  new_oper->dest[0] = L_new_register_operand (++L_fn->max_reg_id,
					      L_native_machine_ctype,
					      L_PTYPE_NULL);
  new_oper->src[0] = L_copy_operand (b_oper->dest[0]);
  /* JWS -- this is not 64-bit compliant! */
  new_oper->src[1] = L_new_gen_int_operand (M_type_size (M_TYPE_INT) - 1);
  L_insert_oper_before (cb, div_oper, new_oper);

  /* create "t2 = t1 & (c-1)" */
  prev_oper = new_oper;
  new_oper = L_create_new_op (Lop_AND);
  new_oper->dest[0] = L_new_register_operand (++L_fn->max_reg_id,
					      L_native_machine_ctype,
					      L_PTYPE_NULL);
  new_oper->src[0] = L_copy_operand (prev_oper->dest[0]);
  new_oper->src[1] = L_new_gen_int_operand (c_value - 1);
  L_insert_oper_before (cb, div_oper, new_oper);

  /* create "t3 = t0 + t2" */
  prev_oper = new_oper;
  new_oper = L_create_new_op (Lop_ADD);
  new_oper->dest[0] = L_new_register_operand (++L_fn->max_reg_id,
					      L_native_machine_ctype,
					      L_PTYPE_NULL);
  new_oper->src[0] = L_copy_operand (b_oper->dest[0]);
  new_oper->src[1] = L_copy_operand (prev_oper->dest[0]);
  L_insert_oper_before (cb, div_oper, new_oper);

  /* create "n = t3 >> log2(c)" */
  prev_oper = new_oper;
  new_oper = L_create_new_op (Lop_ASR);
  new_oper->dest[0] = L_copy_operand (div_oper->dest[0]);
  new_oper->src[0] = L_copy_operand (prev_oper->dest[0]);
  new_oper->src[1] = L_new_gen_int_operand (C_log2 (ITicast (c_value)));
  L_insert_oper_before (cb, div_oper, new_oper);
  return;
}


/*
 * L_create_rem_operations
 * ----------------------------------------------------------------------
 * Create the following code sequence based on rem_oper which takes b%c.
 *     t1 = -1 * sgn(b)
 *     t2 = t1 & (c-1)
 *     t3 = b + t2
 *     t4 = t3 & -c
 *     rem = b - t4
 * If c is negative, all instances of c should be negated.
 */
void
L_create_rem_operations (L_Oper * rem_oper, L_Cb * cb)
{
  ITintmax c_value;
  L_Oper *new_oper, *prev_oper;

  if (rem_oper->src[1]->value.i > 0)	/* c is positive */
    c_value = rem_oper->src[1]->value.i;
  else				/* c is negative */
    c_value = -rem_oper->src[1]->value.i;

  /* create "t1 = -1 * sgn(b)" */
  new_oper = L_create_new_op (Lop_ASR);
  new_oper->dest[0] = L_new_register_operand (++L_fn->max_reg_id,
					      L_native_machine_ctype,
					      L_PTYPE_NULL);
  new_oper->src[0] = L_copy_operand (rem_oper->src[0]);
  /* JWS -- this is not 64-bit compliant! */
  new_oper->src[1] = L_new_gen_int_operand (M_type_size (M_TYPE_INT) - 1);
  L_insert_oper_before (cb, rem_oper, new_oper);

  /* create "t2 = t1 & (c-1)" */
  prev_oper = new_oper;
  new_oper = L_create_new_op (Lop_AND);
  new_oper->dest[0] = L_new_register_operand (++L_fn->max_reg_id,
					      L_native_machine_ctype,
					      L_PTYPE_NULL);
  new_oper->src[0] = L_copy_operand (prev_oper->dest[0]);
  new_oper->src[1] = L_new_gen_int_operand (c_value - 1);
  L_insert_oper_before (cb, rem_oper, new_oper);

  /* create "t3 = b + t2" */
  prev_oper = new_oper;
  new_oper = L_create_new_op (Lop_ADD);
  new_oper->dest[0] = L_new_register_operand (++L_fn->max_reg_id,
					      L_native_machine_ctype,
					      L_PTYPE_NULL);
  new_oper->src[0] = L_copy_operand (rem_oper->src[0]);
  new_oper->src[1] = L_copy_operand (prev_oper->dest[0]);
  L_insert_oper_before (cb, rem_oper, new_oper);

  /* create "t4 = t3 & -c" */
  prev_oper = new_oper;
  new_oper = L_create_new_op (Lop_AND);
  new_oper->dest[0] = L_new_register_operand (++L_fn->max_reg_id,
					      L_native_machine_ctype,
					      L_PTYPE_NULL);
  new_oper->src[0] = L_copy_operand (prev_oper->dest[0]);
  new_oper->src[1] = L_new_gen_int_operand (-c_value);
  L_insert_oper_before (cb, rem_oper, new_oper);

  /* create "rem = b - t4" */
  prev_oper = new_oper;
  new_oper = L_create_new_op (Lop_SUB);
  new_oper->dest[0] = L_copy_operand (rem_oper->dest[0]);
  new_oper->src[0] = L_copy_operand (rem_oper->src[0]);
  new_oper->src[1] = L_copy_operand (prev_oper->dest[0]);
  L_insert_oper_before (cb, rem_oper, new_oper);
  return;
}


L_Oper *
L_create_move (L_Operand * dest, L_Operand * src)
{
  int new_opc = 0;
  L_Oper *new_oper;

  if ((dest == NULL) || (src == NULL))
    L_punt ("L_create_move: dest and src cannot be NULL");

  if (!L_operand_ctype_same (dest, src))
    L_punt ("L_create_move: src and dest not same ctype");

  switch (L_operand_case_ctype (dest))
    {
    case L_CTYPE_INT:
    case L_CTYPE_LLONG:
    case L_CTYPE_POINTER:
      new_opc = Lop_MOV;
      break;
    case L_CTYPE_FLOAT:
      new_opc = Lop_MOV_F;
      break;
    case L_CTYPE_DOUBLE:
      new_opc = Lop_MOV_F2;
      break;
    default:
      L_punt ("L_create_move: illegal ctype");
      return (NULL);		/* L_punt doesn't return */
    }

  new_oper = L_create_new_op (new_opc);

  new_oper->dest[0] = dest;
  new_oper->src[0] = src;

  return (new_oper);
}


L_Oper *
L_create_move_using (L_Operand * dest, L_Operand * src, L_Oper * oper)
{
  L_Oper *new_oper;

  if ((dest == NULL) || (src == NULL))
    L_punt ("L_create_move_using: dest and src cannot be NULL");
  if (oper == NULL)
    L_punt ("L_create_move_using: oper to use cannot be NULL");

  if (L_return_old_ctype (dest) != L_return_old_ctype (src))
    L_punt ("L_create_move_using: src and dest not same ctype");

  switch (L_operand_case_ctype (dest))
    {
    case L_CTYPE_INT:
    case L_CTYPE_LLONG:
    case L_CTYPE_POINTER:
      new_oper = L_create_new_op_using (Lop_MOV, oper);
      break;
    case L_CTYPE_FLOAT:
      new_oper = L_create_new_op_using (Lop_MOV_F, oper);
      break;
    case L_CTYPE_DOUBLE:
      new_oper = L_create_new_op_using (Lop_MOV_F2, oper);
      break;
    default:
      L_punt ("L_create_move: illegal ctype");
      return (NULL);
    }

  new_oper->dest[0] = dest;
  new_oper->src[0] = src;

  return (new_oper);
}


void
L_convert_to_zero_extend_oper (L_Oper * oper, L_Operand * dest,
			       L_Operand * src)
{
  int i, opc;

  /* 02/07/03 REK Adding a check to make sure we don't touch a volatile
   *              oper. */
  if (L_EXTRACT_BIT_VAL (oper->flags, L_OPER_VOLATILE))
    L_punt ("L_convert_to_zero_extend_oper: Cannot convert volatile oper %d",
	    oper->id);

  if ((oper == NULL) || (dest == NULL) || (src == NULL))
    L_punt ("L_convert_to_zero_extend_oper: oper, dest, src may not be NULL");

  if (L_is_opcode (Lop_LD_UC, oper))
    opc = Lop_ZXT_C;
  else if (L_is_opcode (Lop_LD_UC2, oper))
    opc = Lop_ZXT_C2;
  else if (L_is_opcode (Lop_LD_UI, oper))
    opc = Lop_ZXT_I;
  else
    {
      L_punt ("L_create_zero_extend_oper_for_load: Illegal signed conflict");
      return;			/* L_punt doesn't return */
    }

  /* Free up old oper fields, leave predicates alone!! */
  for (i = 0; i < L_max_dest_operand; i++)
    {
      L_Operand *d;
      d = oper->dest[i];
      if (d == NULL)
	continue;
      L_delete_operand (d);
      oper->dest[i] = NULL;
    }
  for (i = 0; i < L_max_src_operand; i++)
    {
      L_Operand *s;
      s = oper->src[i];
      if (s == NULL)
	continue;
      L_delete_operand (s);
      oper->src[i] = NULL;
    }
  L_change_opcode (oper, opc);
  oper->src[0] = src;
  oper->dest[0] = dest;

  /* For loads/stores converted to moves, need to delete sync arcs */
  L_delete_all_sync (oper, 1);

  if (oper->acc_info)
    oper->acc_info = L_delete_mem_acc_spec_list (oper->acc_info);
}


void
L_convert_to_extended_move (L_Oper * oper, L_Operand * dest, L_Operand * src)
{
  int i, opc = 0, ctype;

  /* 02/07/03 REK Adding a check to make sure we don't touch a volatile
   *              oper. */
  if (L_EXTRACT_BIT_VAL (oper->flags, L_OPER_VOLATILE))
    L_punt ("L_convert_to_extended_move: Cannot convert volatile oper %d",
	    oper->id);

  if ((oper == NULL) || (dest == NULL) || (src == NULL))
    L_punt ("L_convert_to_extended_move: oper, dest, src may not be NULL");
  /* L_opcode_ctype returns void for store, want to return data type */
  if (L_store_opcode (oper))
    ctype = L_opcode_ctype2 (oper);
  else
    ctype = L_opcode_ctype (oper);
  switch (ctype)
    {
    case L_CTYPE_CHAR:
      opc = Lop_SXT_C;
      break;
    case L_CTYPE_UCHAR:
      opc = Lop_ZXT_C;
      break;
    case L_CTYPE_SHORT:
      opc = Lop_SXT_C2;
      break;
    case L_CTYPE_USHORT:
      opc = Lop_ZXT_C2;
      break;
    case L_CTYPE_INT:
      if (L_native_machine_ctype != L_CTYPE_INT)
	opc = Lop_SXT_I;
      else
	opc = Lop_MOV;
      break;
    case L_CTYPE_UINT:
      if (L_native_machine_ctype != L_CTYPE_INT)
	opc = Lop_ZXT_I;
      else
	opc = Lop_MOV;
      break;
    case L_CTYPE_LLONG:
    case L_CTYPE_ULLONG:
      opc = Lop_MOV;
      break;
    case L_CTYPE_FLOAT:
      opc = Lop_MOV_F;
      break;
    case L_CTYPE_DOUBLE:
      opc = Lop_MOV_F2;
      break;
    case L_CTYPE_PREDICATE:
      opc = Lop_PRED_COPY;
      break;
    default:
      L_punt ("L_convert_to_extended_move: illegal data type");
    }
  /* Free up old oper fields, leave predicates alone!! */
  for (i = 0; i < L_max_dest_operand; i++)
    {
      L_Operand *d;
      d = oper->dest[i];
      if (d == NULL)
	continue;
      L_delete_operand (d);
      oper->dest[i] = NULL;
    }
  for (i = 0; i < L_max_src_operand; i++)
    {
      L_Operand *s;
      s = oper->src[i];
      if (s == NULL)
	continue;
      L_delete_operand (s);
      oper->src[i] = NULL;
    }
  L_change_opcode (oper, opc);
  oper->dest[0] = dest;
  oper->src[0] = src;

  /* For loads/stores converted to moves, need to delete sync arcs */
  L_delete_all_sync (oper, 1);

  if (oper->acc_info)
    oper->acc_info = L_delete_mem_acc_spec_list (oper->acc_info);
}


void
L_convert_to_move (L_Oper * oper, L_Operand * dest, L_Operand * src)
{
  int i, opc, ctype;

  /* 02/07/03 REK Adding a check to make sure we don't touch a volatile
   *              oper. */
  if (L_EXTRACT_BIT_VAL (oper->flags, L_OPER_VOLATILE))
    L_punt ("L_convert_to_move: Cannot convert volatile oper %d",
	    oper->id);

  if ((oper == NULL) || (dest == NULL) || (src == NULL))
    L_punt ("L_convert_to_move: oper, dest, src may not be NULL");
  /* L_opcode_ctype returns void for store, want to return data type */
  if (L_store_opcode (oper))
    ctype = L_opcode_ctype2 (oper);
  else
    ctype = L_opcode_ctype (oper);
  switch (ctype)
    {
    case L_CTYPE_CHAR:
    case L_CTYPE_UCHAR:
    case L_CTYPE_SHORT:
    case L_CTYPE_USHORT:
    case L_CTYPE_INT:
    case L_CTYPE_UINT:
    case L_CTYPE_LLONG:
    case L_CTYPE_ULLONG:
      opc = Lop_MOV;
      break;
    case L_CTYPE_FLOAT:
      opc = Lop_MOV_F;
      break;
    case L_CTYPE_DOUBLE:
      opc = Lop_MOV_F2;
      break;
    case L_CTYPE_PREDICATE:
      opc = Lop_PRED_COPY;
      break;
    default:
      L_punt ("L_convert_to_move: illegal data type");
      return;			/* L_punt does not return */
    }
  /* Free up old oper fields, leave predicates alone!! */
  for (i = 0; i < L_max_dest_operand; i++)
    {
      L_Operand *d;
      d = oper->dest[i];
      if (d == NULL)
	continue;
      L_delete_operand (d);
      oper->dest[i] = NULL;
    }
  for (i = 0; i < L_max_src_operand; i++)
    {
      L_Operand *s;
      s = oper->src[i];
      if (s == NULL)
	continue;
      L_delete_operand (s);
      oper->src[i] = NULL;
    }
  L_change_opcode (oper, opc);
  oper->dest[0] = dest;
  oper->src[0] = src;

  /* Moves don't have completers beyond 0 */
  for (i = 1; i < L_MAX_CMPLTR; i++)
    oper->com[i] = 0;
  
  /* For loads/stores converted to moves, need to delete sync arcs */
  L_delete_all_sync (oper, 1);

  if (oper->acc_info)
    oper->acc_info = L_delete_mem_acc_spec_list (oper->acc_info);
}


void
L_convert_to_extract (L_Oper * op, L_Operand * dest, L_Operand * src, int of)
{
  int i, opc, ctype, sz = 0, sg = 0;

  /* 02/07/03 REK Adding a check to make sure we don't touch a volatile
   *              oper. */
  if (L_EXTRACT_BIT_VAL (op->flags, L_OPER_VOLATILE))
    L_punt ("L_convert_to_extract: Cannot convert volatile oper %d",
	    op->id);

  if (of < 0)
    L_punt ("L_convert_to_extract: negative offset");

  if (L_store_opcode (op))
    ctype = L_opcode_ctype2 (op);
  else
    ctype = L_opcode_ctype (op);

  switch (ctype)
    {
    case L_CTYPE_CHAR:
      sg = 1;
    case L_CTYPE_UCHAR:
      sz = 8;
      break;
    case L_CTYPE_SHORT:
      sg = 1;
    case L_CTYPE_USHORT:
      sz = 16;
      break;
    case L_CTYPE_INT:
      sg = 1;
    case L_CTYPE_UINT:
      sz = 32;
      break;
    case L_CTYPE_LLONG:
      sg = 1;
    case L_CTYPE_ULLONG:
      sz = 64;
      break;
    default:
      L_punt ("L_convert_to_extract: unhandled ctype");
    }

  opc = sg ? Lop_EXTRACT : Lop_EXTRACT_U;

  /* Free up old op fields, leave predicates alone!! */
  for (i = 0; i < L_max_dest_operand; i++)
    {
      L_Operand *d;
      if (!(d = op->dest[i]))
	continue;
      L_delete_operand (d);
      op->dest[i] = NULL;
    }
  for (i = 0; i < L_max_src_operand; i++)
    {
      L_Operand *s;
      if (!(s = op->src[i]))
	continue;
      L_delete_operand (s);
      op->src[i] = NULL;
    }

  L_change_opcode (op, opc);
  op->src[0] = src;
  op->src[1] = L_new_gen_int_operand (of << 3);
  op->src[2] = L_new_gen_int_operand (sz);
  op->dest[0] = dest;

  /* For loads/stores converted to moves, need to delete sync arcs */
  L_delete_all_sync (op, 1);

  if (op->acc_info)
    op->acc_info = L_delete_mem_acc_spec_list (op->acc_info);

  return;
}


void
L_convert_to_move_of_zero (L_Oper * oper, L_Operand * dest)
{
  L_Operand *zero;

  /* 02/07/03 REK Adding a check to make sure we don't touch a volatile
   *              oper. */
  if (L_EXTRACT_BIT_VAL (oper->flags, L_OPER_VOLATILE))
    L_punt ("L_convert_to_move_of_zero: Cannot convert volatile oper %d",
	    oper->id);

  if ((oper == NULL) || (dest == NULL))
    L_punt ("L_convert_to_move_of_zero: oper, dest may not be NULL");

  switch (L_opcode_ctype (oper))
    {
    case L_CTYPE_CHAR:
    case L_CTYPE_UCHAR:
    case L_CTYPE_SHORT:
    case L_CTYPE_USHORT:
      L_warn ("L_convert_to_move_of_zero: converting a char or a short");
    case L_CTYPE_INT:
    case L_CTYPE_UINT:
    case L_CTYPE_LLONG:
    case L_CTYPE_ULLONG:
      zero = L_new_gen_int_operand (0);
      break;
    case L_CTYPE_FLOAT:
      zero = L_new_float_operand (0.0);
      break;
    case L_CTYPE_DOUBLE:
      zero = L_new_double_operand (0.0);
      break;
    default:
      L_punt ("L_convert_to_move_of_zero: illegal data type");
      return;
    }

  L_convert_to_move (oper, dest, zero);
}


void
L_convert_to_move_of_one (L_Oper * oper, L_Operand * dest)
{
  L_Operand *one;

  /* 02/07/03 REK Adding a check to make sure we don't touch a volatile
   *              oper. */
  if (L_EXTRACT_BIT_VAL (oper->flags, L_OPER_VOLATILE))
    L_punt ("L_convert_to_move_of_one: Cannot convert volatile oper %d",
	    oper->id);

  if ((oper == NULL) || (dest == NULL))
    L_punt ("L_convert_to_move_of_one: oper, dest may not be NULL");

  switch (L_opcode_ctype (oper))
    {
    case L_CTYPE_CHAR:
    case L_CTYPE_UCHAR:
    case L_CTYPE_SHORT:
    case L_CTYPE_USHORT:
      L_warn ("L_convert_to_move_of_one: converting a char or a short");
    case L_CTYPE_INT:
    case L_CTYPE_UINT:
    case L_CTYPE_LLONG:
    case L_CTYPE_ULLONG:
      one = L_new_gen_int_operand (1);
      break;
    case L_CTYPE_FLOAT:
      one = L_new_float_operand (1.0);
      break;
    case L_CTYPE_DOUBLE:
      one = L_new_double_operand (1.0);
      break;
    default:
      L_punt ("L_convert_to_move_of_one: illegal data type");
      return;			/* L_punt does not return */
    }

  L_convert_to_move (oper, dest, one);
}


void
L_convert_to_jump (L_Oper * oper, L_Operand * target)
{
  int i, opc;

  /* 02/07/03 REK Adding a check to make sure we don't touch a volatile
   *              oper. */
  if (L_EXTRACT_BIT_VAL (oper->flags, L_OPER_VOLATILE))
    L_punt ("L_convert_to_jump: Cannot convert volatile oper %d",
	    oper->id);

  if ((oper == NULL) || (target == NULL))
    L_punt ("L_convert_to_jump: oper and target cannot be NULL");
  switch (oper->opc)
    {
    case Lop_BR:
    case Lop_BR_F:
      opc = Lop_JUMP;
      break;
    default:
      L_punt ("L_convert_to_jump: illegal opcode");
      return;			/* L_punt does not return */
    }
  oper->com[0] = 0;
  oper->com[1] = 0;

  /* Free up old oper fields */
  for (i = 0; i < L_max_dest_operand; i++)
    {
      L_Operand *d;
      d = oper->dest[i];
      if (d == NULL)
	continue;
      L_delete_operand (d);
      oper->dest[i] = NULL;
    }
  for (i = 0; i < L_max_src_operand; i++)
    {
      L_Operand *s;
      s = oper->src[i];
      if (s == NULL)
	continue;
      L_delete_operand (s);
      oper->src[i] = NULL;
    }
  L_change_opcode (oper, opc);
  oper->src[0] = target;
}

/*
 *      Note this func really messes up consistency of profile, may
 *      in future want to redistribute weights for flows that are being
 *      deleted (???)
 */

void
L_fix_cond_br (L_Cb * cb, L_Oper * oper, int cc)
{
  L_Operand *target;
  L_Oper *ptr, *next_op;
  L_Cb *target_cb;
  L_Flow *flow, *f, *f2, *next_flow;

  if (!L_cond_branch_opcode (oper))
    L_punt ("L_fix_cond_br: must be a conditional branch");

  flow = L_find_flow_for_branch (cb, oper);

  if (cc)
    {
      /* always taken => convert to a jump, remove all operations after it */

      target = L_copy_operand (oper->src[2]);
      L_convert_to_jump (oper, target);

      if (L_is_predicated (oper))
	{
	  /* delete all opers after branch */
	  f = flow->next_flow;
	  for (ptr = oper->next_op; ptr != NULL; ptr = next_op)
	    {
	      next_op = ptr->next_op;
	      if (L_general_branch_opcode (ptr) ||
		  L_check_branch_opcode (ptr))
		{
		  next_flow = f->next_flow;
		  if (PG_superset_predicate_ops (oper, ptr))
		    {
		      target_cb = f->dst_cb;
		      f2 = L_find_matching_flow (target_cb->src_flow, f);
		      target_cb->src_flow =
			L_delete_flow (target_cb->src_flow, f2);
		      cb->dest_flow = L_delete_flow (cb->dest_flow, f);
		      L_delete_oper (cb, ptr);
		    }
		  f = next_flow;
		}
	      else
		{
		  if (PG_superset_predicate_ops (oper, ptr))
		    L_delete_oper (cb, ptr);
		}
	    }
	}
      else
	{
	  /* delete all opers after branch */
	  for (ptr = oper->next_op; ptr != NULL; ptr = next_op)
	    {
	      next_op = ptr->next_op;
	      L_delete_oper (cb, ptr);
	    }
	  /* delete all flow arcs after flow, also delete corresp src arcs */
	  for (f = flow->next_flow; f != NULL; f = next_flow)
	    {
	      next_flow = f->next_flow;
	      target_cb = f->dst_cb;
	      f2 = L_find_matching_flow (target_cb->src_flow, f);
	      target_cb->src_flow = L_delete_flow (target_cb->src_flow, f2);
	      cb->dest_flow = L_delete_flow (cb->dest_flow, f);
	    }
	}
    }
  else
    {
      /* always fallthru => delete , remove associated flow arc */

      L_delete_oper (cb, oper);
      target_cb = flow->dst_cb;
      f2 = L_find_matching_flow (target_cb->src_flow, flow);
      target_cb->src_flow = L_delete_flow (target_cb->src_flow, f2);
      cb->dest_flow = L_delete_flow (cb->dest_flow, flow);
    }
}


/*=========================================================================*/
/*
 *      opcode relating functions
 */
/*=========================================================================*/

int
L_move_from_ctype (int ctype)
{
  switch (ctype)
    {
    case L_CTYPE_CHAR:
    case L_CTYPE_UCHAR:
    case L_CTYPE_SHORT:
    case L_CTYPE_USHORT:
    case L_CTYPE_INT:
    case L_CTYPE_UINT:
    case L_CTYPE_LONG:
    case L_CTYPE_ULONG:
    case L_CTYPE_LLONG:
    case L_CTYPE_ULLONG:
    case L_CTYPE_LLLONG:
    case L_CTYPE_ULLLONG:
    case L_CTYPE_POINTER:
      return Lop_MOV;
    case L_CTYPE_FLOAT:
      return Lop_MOV_F;
    case L_CTYPE_DOUBLE:
      return Lop_MOV_F2;
    default:
      L_punt ("L_move_from_ctype: illegal ctype");
      return (0);
    }
}

int
L_corresponding_load (L_Oper * oper)
{
  if (oper == NULL)
    L_punt ("L_corresponding_load: oper is NULL");

  switch (oper->opc)
    {
    case Lop_LD_UC:
    case Lop_LD_C:
    case Lop_LD_UC2:
    case Lop_LD_C2:
    case Lop_LD_UI:
    case Lop_LD_I:
    case Lop_LD_Q:
    case Lop_LD_F:
    case Lop_LD_F2:
      return oper->opc;
    case Lop_ST_C:
      return (Lop_LD_C);
    case Lop_ST_C2:
      return (Lop_LD_C2);
    case Lop_ST_I:
      return (Lop_LD_I);
    case Lop_ST_Q:
      return (Lop_LD_Q);
    case Lop_ST_F:
      return (Lop_LD_F);
    case Lop_ST_F2:
      return (Lop_LD_F2);
    default:
      L_punt ("L_corresponding_load: illegal opcode");
      return (0);
    }
}

int
L_corresponding_preincrement_load (L_Oper * oper)
{
  if (oper == NULL)
    L_punt ("L_corresponding_preincrement_load: oper is NULL");

  switch (oper->opc)
    {
    case Lop_LD_UC:
      return (Lop_LD_PRE_UC);
    case Lop_LD_C:
      return (Lop_LD_PRE_C);
    case Lop_LD_UC2:
      return (Lop_LD_PRE_UC2);
    case Lop_LD_C2:
      return (Lop_LD_PRE_C2);
    case Lop_LD_UI:
      return (Lop_LD_PRE_UI);
    case Lop_LD_I:
      return (Lop_LD_PRE_I);
    case Lop_LD_Q:
      return (Lop_LD_PRE_Q);
    case Lop_LD_F:
      return (Lop_LD_PRE_F);
    case Lop_LD_F2:
      return (Lop_LD_PRE_F2);
    case Lop_ST_C:
      return (Lop_LD_PRE_C);
    case Lop_ST_C2:
      return (Lop_LD_PRE_C2);
    case Lop_ST_I:
      return (Lop_LD_PRE_I);
    case Lop_ST_F:
      return (Lop_LD_PRE_F);
    case Lop_ST_F2:
      return (Lop_LD_PRE_F2);
    default:
      L_punt ("L_corresponding_preincrement_load: illegal opcode");
      return (0);
    }
}

int
L_corresponding_postincrement_load (L_Oper * oper)
{
  if (oper == NULL)
    L_punt ("L_corresponding_postincrement_load: oper is NULL");

  switch (oper->opc)
    {
    case Lop_LD_UC:
      return (Lop_LD_POST_UC);
    case Lop_LD_C:
      return (Lop_LD_POST_C);
    case Lop_LD_UC2:
      return (Lop_LD_POST_UC2);
    case Lop_LD_C2:
      return (Lop_LD_POST_C2);
    case Lop_LD_UI:
      return (Lop_LD_POST_UI);
    case Lop_LD_I:
      return (Lop_LD_POST_I);
    case Lop_LD_Q:
      return (Lop_LD_POST_Q);
    case Lop_LD_F:
      return (Lop_LD_POST_F);
    case Lop_LD_F2:
      return (Lop_LD_POST_F2);
    case Lop_ST_C:
      return (Lop_LD_POST_C);
    case Lop_ST_C2:
      return (Lop_LD_POST_C2);
    case Lop_ST_I:
      return (Lop_LD_POST_I);
    case Lop_ST_F:
      return (Lop_LD_POST_F);
    case Lop_ST_F2:
      return (Lop_LD_POST_F2);
    default:
      L_punt ("L_corresponding_postincrement_load: illegal opcode");
      return (0);
    }
}

int
L_corresponding_store (L_Oper * oper)
{
  if (oper == NULL)
    L_punt ("L_corresponding_store: oper is NULL");

  switch (oper->opc)
    {
    case Lop_LD_UC:
      return (Lop_ST_C);
    case Lop_LD_C:
      return (Lop_ST_C);
    case Lop_LD_UC2:
      return (Lop_ST_C2);
    case Lop_LD_C2:
      return (Lop_ST_C2);
    case Lop_LD_UI:
      return (Lop_ST_I);
    case Lop_LD_I:
      return (Lop_ST_I);
    case Lop_LD_Q:
      return (Lop_ST_Q);
    case Lop_LD_F:
      return (Lop_ST_F);
    case Lop_LD_F2:
      return (Lop_ST_F2);
    case Lop_ST_C:
      return (Lop_ST_C);
    case Lop_ST_C2:
      return (Lop_ST_C2);
    case Lop_ST_I:
      return (Lop_ST_I);
    case Lop_ST_Q:
      return (Lop_ST_Q);
    case Lop_ST_F:
      return (Lop_ST_F);
    case Lop_ST_F2:
      return (Lop_ST_F2);
    default:
      L_punt ("L_corresponding_store: illegal opcode");
      return (0);
    }
}

int
L_corresponding_preincrement_store (L_Oper * oper)
{
  if (oper == NULL)
    L_punt ("L_corresponding_preincrement_store: oper is NULL");

  switch (oper->opc)
    {
    case Lop_LD_UC:
      return (Lop_ST_PRE_C);
    case Lop_LD_C:
      return (Lop_ST_PRE_C);
    case Lop_LD_UC2:
      return (Lop_ST_PRE_C2);
    case Lop_LD_C2:
      return (Lop_ST_PRE_C2);
    case Lop_LD_UI:
      return (Lop_ST_PRE_I);
    case Lop_LD_I:
      return (Lop_ST_PRE_I);
    case Lop_LD_Q:
      return (Lop_ST_PRE_Q);
    case Lop_LD_F:
      return (Lop_ST_PRE_F);
    case Lop_LD_F2:
      return (Lop_ST_PRE_F2);
    case Lop_ST_C:
      return (Lop_ST_PRE_C);
    case Lop_ST_C2:
      return (Lop_ST_PRE_C2);
    case Lop_ST_I:
      return (Lop_ST_PRE_I);
    case Lop_ST_Q:
      return (Lop_ST_PRE_Q);
    case Lop_ST_F:
      return (Lop_ST_PRE_F);
    case Lop_ST_F2:
      return (Lop_ST_PRE_F2);
    default:
      L_punt ("L_corresponding_preincrement_store: illegal opcode");
      return (0);
    }
}

int
L_corresponding_postincrement_store (L_Oper * oper)
{
  if (oper == NULL)
    L_punt ("L_corresponding_postincrement_store: oper is NULL");

  switch (oper->opc)
    {
    case Lop_LD_UC:
      return (Lop_ST_POST_C);
    case Lop_LD_C:
      return (Lop_ST_POST_C);
    case Lop_LD_UC2:
      return (Lop_ST_POST_C2);
    case Lop_LD_C2:
      return (Lop_ST_POST_C2);
    case Lop_LD_UI:
      return (Lop_ST_POST_I);
    case Lop_LD_I:
      return (Lop_ST_POST_I);
    case Lop_LD_Q:
      return (Lop_ST_POST_Q);
    case Lop_LD_F:
      return (Lop_ST_POST_F);
    case Lop_LD_F2:
      return (Lop_ST_POST_F2);
    case Lop_ST_C:
      return (Lop_ST_POST_C);
    case Lop_ST_C2:
      return (Lop_ST_POST_C2);
    case Lop_ST_I:
      return (Lop_ST_POST_I);
    case Lop_ST_Q:
      return (Lop_ST_POST_Q);
    case Lop_ST_F:
      return (Lop_ST_POST_F);
    case Lop_ST_F2:
      return (Lop_ST_POST_F2);
    default:
      L_punt ("L_corresponding_postincrement_store: illegal opcode");
      return (0);
    }
}

int
L_corresponding_mov (L_Oper * oper)
{
  if (oper == NULL)
    L_punt ("L_corresponding_mov: oper is NULL");

  switch (oper->opc)
    {
    case Lop_LD_UC:
      return (Lop_MOV);
    case Lop_LD_C:
      return (Lop_MOV);
    case Lop_LD_UC2:
      return (Lop_MOV);
    case Lop_LD_C2:
      return (Lop_MOV);
    case Lop_LD_UI:
      return (Lop_MOV);
    case Lop_LD_I:
      return (Lop_MOV);
    case Lop_LD_Q:
      return (Lop_MOV);
    case Lop_LD_F:
      return (Lop_MOV_F);
    case Lop_LD_F2:
      return (Lop_MOV_F2);
    case Lop_ST_C:
      return (Lop_MOV);
    case Lop_ST_C2:
      return (Lop_MOV);
    case Lop_ST_I:
      return (Lop_MOV);
    case Lop_ST_Q:
      return (Lop_MOV);
    case Lop_ST_F:
      return (Lop_MOV_F);
    case Lop_ST_F2:
      return (Lop_MOV_F2);
    default:
      L_punt ("L_corresponding_store: illegal opcode");
      return (0);
    }
}

int
L_corresponding_add (L_Oper * oper)
{
  if (oper == NULL)
    L_punt ("L_corresponding_add: oper is NULL");

  switch (oper->opc)
    {
    case Lop_ADD:
    case Lop_ADD_U:
      return oper->opc;
    case Lop_SUB:
      return (Lop_ADD);
    case Lop_SUB_U:
      return (Lop_ADD_U);
    default:
      L_punt ("L_corresponding_add: illegal opcode");
      return (0);
    }
}

int
L_corresponding_mul_add (L_Oper * oper)
{
  if (oper == NULL)
    L_punt ("L_corresponding_mul_add: oper is NULL");

  switch (oper->opc)
    {
    case Lop_ADD:
      return (Lop_MUL_ADD);
    case Lop_ADD_U:
      return (Lop_MUL_ADD_U);
    case Lop_ADD_F:
      return (Lop_MUL_ADD_F);
    case Lop_ADD_F2:
      return (Lop_MUL_ADD_F2);
    case Lop_MUL:
      return (Lop_MUL_ADD);
    case Lop_MUL_U:
      return (Lop_MUL_ADD_U);
    case Lop_MUL_F:
      return (Lop_MUL_ADD_F);
    case Lop_MUL_F2:
      return (Lop_MUL_ADD_F2);
    default:
      L_punt ("L_corresponding_mul_add: illegal opcode");
      return (0);
    }
}

int
L_corresponding_mul_sub (L_Oper * oper)
{
  if (oper == NULL)
    L_punt ("L_corresponding_mul_sub: oper is NULL");

  switch (oper->opc)
    {
    case Lop_SUB:
      return (Lop_MUL_SUB);
    case Lop_SUB_U:
      return (Lop_MUL_SUB_U);
    case Lop_SUB_F:
      return (Lop_MUL_SUB_F);
    case Lop_SUB_F2:
      return (Lop_MUL_SUB_F2);
    case Lop_MUL:
      return (Lop_MUL_SUB);
    case Lop_MUL_U:
      return (Lop_MUL_SUB_U);
    case Lop_MUL_F:
      return (Lop_MUL_SUB_F);
    case Lop_MUL_F2:
      return (Lop_MUL_SUB_F2);
    default:
      L_punt ("L_corresponding_mul_sub: illegal opcode");
      return (0);
    }
}

int
L_corresponding_mul_sub_rev (L_Oper * oper)
{
  if (oper == NULL)
    L_punt ("L_corresponding_mul_sub_rev: oper is NULL");

  switch (oper->opc)
    {
    case Lop_SUB:
      return (Lop_MUL_SUB_REV);
    case Lop_SUB_U:
      return (Lop_MUL_SUB_REV_U);
    case Lop_SUB_F:
      return (Lop_MUL_SUB_REV_F);
    case Lop_SUB_F2:
      return (Lop_MUL_SUB_REV_F2);
    case Lop_MUL:
      return (Lop_MUL_SUB_REV);
    case Lop_MUL_U:
      return (Lop_MUL_SUB_REV_U);
    case Lop_MUL_F:
      return (Lop_MUL_SUB_REV_F);
    case Lop_MUL_F2:
      return (Lop_MUL_SUB_REV_F2);
    default:
      L_punt ("L_corresponding_mul_sub_rev: illegal opcode");
      return (0);
    }
}


int
L_inverse_arithmetic (int opc)
{
  switch (opc)
    {
    case Lop_ADD:
      return (Lop_SUB);
    case Lop_ADD_U:
      return (Lop_SUB_U);
    case Lop_SUB:
      return (Lop_ADD);
    case Lop_SUB_U:
      return (Lop_ADD_U);
    default:
      L_punt ("L_inverse_arithmetic: illegal opcode");
      return (0);

    }
}


/* SER: For use in constant combining and logic reduction. Returns 1
 * if a constant ends up in src[1] of oper.
 */
int
L_has_const_operand_and_realign_oper (L_Oper * oper)
{
  L_Operand *temp;

  /* convert subtraction of negative constants to positive addition */
  if (L_int_sub_opcode (oper) && L_is_int_constant (oper->src[1]) &&
      ((oper->src[1]->value.i) < 0))
    {
      temp = L_new_gen_int_operand (-(oper->src[1]->value.i));
      L_change_opcode (oper, L_inverse_arithmetic (oper->opc));
      L_delete_operand (oper->src[1]);
      oper->src[1] = temp;
      return 1;
    }

  if (!L_commutative_opcode (oper))
    {
      if (L_is_constant (oper->src[1]))
	return 1;
      else
	return 0;
    }

  if (L_is_constant (oper->src[1]))
    return 1;

  if (L_is_constant (oper->src[0]))
    {
      temp = oper->src[1];
      oper->src[1] = oper->src[0];
      oper->src[0] = temp;
      return 1;
    }

  /* reorder macros, return 0 */
  if (L_is_macro (oper->src[1]))
    {}
  else if (L_is_macro (oper->src[0]))
    {
      temp = oper->src[1];
      oper->src[1] = oper->src[0];
      oper->src[0] = temp;
    }
  return 0;

}

/*=====================================================================*/
/*
 *      Functions combining opers
 */
/*=====================================================================*/

void
L_undo_and_combine (L_Oper * opB, L_Oper * opC)
{
  ITintmax s1_B_const, s2_B_const, s1_C_const, s2_C_const;
  ITintmax base, offset;

  if ((opB == NULL) || (opC == NULL))
    {
      L_punt ("L_undo_and_combine: opB, opC cannot be NIL");
      return;			/* L_punt doesn't return */
    }
  s1_B_const = L_is_int_constant (opB->src[0]);
  s2_B_const = L_is_int_constant (opB->src[1]);
  s1_C_const = L_is_int_constant (opC->src[0]);
  s2_C_const = L_is_int_constant (opC->src[1]);
  if (s1_B_const)
    offset = opB->src[0]->value.i;
  else if (s2_B_const)
    offset = opB->src[1]->value.i;
  else
    {
      L_punt ("L_undo_and_combine: either s1 or s2 of opB must be num const");
      return;			/* L_punt doesn't return */
    }

  if (s1_C_const)
    base = opC->src[0]->value.i;
  else if (s2_C_const)
    base = opC->src[1]->value.i;
  else
    {
      L_punt ("L_undo_and_combine: s1 or s2 of opC must be a num const");
      return;			/* L_punt doesn't return */
    }
  if (L_int_add_opcode (opB))
    offset = -offset;
  L_delete_operand (opC->src[0]);
  L_delete_operand (opC->src[1]);
  if (L_load_opcode (opC) || L_store_opcode (opC) || L_int_add_opcode (opC))
    {
      if (s1_C_const)
	{
	  opC->src[0] = L_new_gen_int_operand (base + offset);
	  opC->src[1] = L_copy_operand (opB->dest[0]);
	}
      else
	{
	  opC->src[0] = L_copy_operand (opB->dest[0]);
	  opC->src[1] = L_new_gen_int_operand (base + offset);
	}
    }
  else
    {
      if (s1_C_const)
	{
	  opC->src[0] = L_new_gen_int_operand (base - offset);
	  opC->src[1] = L_copy_operand (opB->dest[0]);
	}
      else
	{
	  opC->src[0] = L_copy_operand (opB->dest[0]);
	  opC->src[1] = L_new_gen_int_operand (base - offset);
	}
    }
}

/*
 *  opA and opB both have 1 const operand, merge into 1 operation opB
 */
int
L_combine_operations (L_Oper * opA, L_Oper * opB)
{
  ITintmax s1_A_const, s2_A_const, s1_B_const, s2_B_const;
  ITintmax base, offset;
  int old_num_oper, new_num_oper;

  L_Operand *old_src1, *old_src2;
  if ((opA == NULL) || (opB == NULL))
    {
      L_punt ("L_combine_operations: opA and opB cannot be NIL");
      return (-1);		/* L_punt doesn't return */
    }
  old_num_oper = M_num_oper_required_for (opB, L_fn->name);
  old_src1 = opB->src[0];
  old_src2 = opB->src[1];
  s1_A_const = L_is_int_constant (opA->src[0]);
  s2_A_const = L_is_int_constant (opA->src[1]);
  s1_B_const = L_is_int_constant (opB->src[0]);
  s2_B_const = L_is_int_constant (opB->src[1]);
  if (s1_A_const)
    {
      offset = opA->src[0]->value.i;
    }
  else if (s2_A_const)
    {
      offset = opA->src[1]->value.i;
      if (L_int_sub_opcode (opA))
	offset = -offset;
    }
  else
    {
      L_punt ("L_combine_operations: either s1 or s2 of opA must be a const");
      return (-1);		/* L_punt doesn't return */
    }

  if (s1_B_const)
    base = opB->src[0]->value.i;
  else if (s2_B_const)
    base = opB->src[1]->value.i;
  else
    {
      L_punt ("L_combine_operations: either s1 or s2 of opB must be a const");
      return (-1);
    }
  if (L_load_opcode (opB) || L_store_opcode (opB) || L_int_add_opcode (opB))
    {
      if (s1_A_const)
	{
	  opB->src[0] = L_copy_operand (opA->src[1]);
	  opB->src[1] = L_new_gen_int_operand (base + offset);
	}
      else
	{
	  opB->src[0] = L_copy_operand (opA->src[0]);
	  opB->src[1] = L_new_gen_int_operand (base + offset);
	}
    }
  else
    {
      if (s1_A_const)
	{
	  if (s1_B_const)
	    {
	      opB->src[0] = L_new_gen_int_operand (base - offset);
	      opB->src[1] = L_copy_operand (opA->src[1]);
	    }
	  else
	    {
	      opB->src[0] = L_copy_operand (opA->src[1]);
	      opB->src[1] = L_new_gen_int_operand (base - offset);
	    }
	}
      else
	{
	  if (s1_B_const)
	    {
	      opB->src[0] = L_new_gen_int_operand (base - offset);
	      opB->src[1] = L_copy_operand (opA->src[0]);
	    }
	  else
	    {
	      opB->src[0] = L_copy_operand (opA->src[0]);
	      opB->src[1] = L_new_gen_int_operand (base - offset);
	    }
	}
    }
  new_num_oper = M_num_oper_required_for (opB, L_fn->name);
  if (new_num_oper > old_num_oper)
    {
      L_delete_operand (opB->src[0]);
      L_delete_operand (opB->src[1]);
      opB->src[0] = old_src1;
      opB->src[1] = old_src2;
      return 0;
    }
  else
    {
      L_delete_operand (old_src1);
      L_delete_operand (old_src2);
      return 1;
    }
}

/*
 *  opA = increment op, opB move above opA
 */
void
L_undo_increment (L_Oper * opA, L_Oper * opB)
{
  ITintmax base, offset;
  if ((opA == NULL) || (opB == NULL))
    {
      L_punt ("L_undo_increment: opA and opB cannot be NIL");
      return;			/* L_punt doesn't return */
    }
  if (L_is_int_constant (opA->src[0]))
    offset = opA->src[0]->value.i;
  else if (L_is_int_constant (opA->src[1]))
    offset = opA->src[1]->value.i;
  else
    {
      L_punt ("L_undo_increment: s1 or s2 of opA must be constant");
      return;			/* L_punt doesn't return */
    }

  if (L_is_int_constant (opB->src[0]))
    base = opB->src[0]->value.i;
  else if (L_is_int_constant (opB->src[1]))
    base = opB->src[1]->value.i;
  else
    {
      L_punt ("L_undo_increment: s1 or s2 of opB must be constant");
      return;			/* L_punt doesn't return */
    }
  if (L_int_sub_opcode (opA))
    offset = -offset;
  if (L_load_opcode (opB) || L_store_opcode (opB) || L_int_add_opcode (opB))
    {
      if (L_is_int_constant (opB->src[0]))
	{
	  L_delete_operand (opB->src[0]);
	  L_delete_operand (opB->src[1]);
	  opB->src[0] = L_new_gen_int_operand (base + offset);
	  opB->src[1] = L_copy_operand (opA->dest[0]);
	}
      else
	{
	  L_delete_operand (opB->src[0]);
	  L_delete_operand (opB->src[1]);
	  opB->src[0] = L_copy_operand (opA->dest[0]);
	  opB->src[1] = L_new_gen_int_operand (base + offset);
	}
    }
  else
    {
      if (L_is_int_constant (opB->src[0]))
	{
	  L_delete_operand (opB->src[0]);
	  L_delete_operand (opB->src[1]);
	  opB->src[0] = L_new_gen_int_operand (base - offset);
	  opB->src[1] = L_copy_operand (opA->dest[0]);
	}
      else
	{
	  L_delete_operand (opB->src[0]);
	  L_delete_operand (opB->src[1]);
	  opB->src[0] = L_copy_operand (opA->dest[0]);
	  opB->src[1] = L_new_gen_int_operand (base - offset);
	}
    }
}


/*
 *  Combine the effects of opA and opB into opA.
 */
void
L_combine_increment_operations (L_Oper * opA, L_Oper * opB)
{
  ITintmax inc1, inc2;
  int unsgn1, unsgn2;

  if (L_is_opcode (Lop_ADD_U, opA) || L_is_opcode (Lop_SUB_U, opA))
    unsgn1 = 1;
  else
    unsgn1 = 0;
  if (L_is_opcode (Lop_ADD_U, opB) || L_is_opcode (Lop_SUB_U, opB))
    unsgn2 = 1;
  else
    unsgn2 = 0;
#if 0
  /* Unsigned/signed arithmetic allowed to mix now */
  if (unsgn1 != unsgn2)
    {
      fprintf (stderr, " opA = %d opB = %d\n", opA->id, opB->id);
      L_punt ("L_combine_increment_operations: incompatible ops");
    }
#endif
  if (L_is_int_constant (opA->src[0]))
    inc1 = opA->src[0]->value.i;
  else
    inc1 = opA->src[1]->value.i;
  if (L_is_int_constant (opB->src[0]))
    inc2 = opB->src[0]->value.i;
  else
    inc2 = opB->src[1]->value.i;
  if (L_int_sub_opcode (opA))
    inc1 = -inc1;
  if (L_int_sub_opcode (opB))
    inc2 = -inc2;
  L_delete_operand (opA->src[0]);
  L_delete_operand (opA->src[1]);
  opA->src[0] = L_copy_operand (opA->dest[0]);

  /* ADA 6/20/96: Added condition to allow case of 'opA->opc==Lop_SUB'
     handled correctly */
  if (L_int_add_opcode (opA))
    opA->src[1] = L_new_gen_int_operand (inc1 + inc2);
  else
    opA->src[1] = L_new_gen_int_operand (-inc1 - inc2);
}


/*=====================================================================*/
/*
 *      Functions for reodering opers
 */
/*=====================================================================*/

void
L_move_oper_before (L_Cb * cb, L_Oper * move_op, L_Oper * before_op)
{
  L_Flow *move_flow = NULL;
  int move_op_is_control = 0;

  if ((move_op == NULL) || (before_op == NULL))
    L_punt ("L_move_oper_before: move_op and before_op cannot be NIL");

  /* Remove move_op's flow */

  move_op_is_control = L_is_control_oper (move_op);
  if (move_op_is_control)
    {
      move_flow = L_find_flow_for_branch (cb, move_op);
      cb->dest_flow = L_remove_flow (cb->dest_flow, move_flow);
    }

  /* Set <M> flag for move_op if neccesary */

  if (L_mask_potential_exceptions && !L_safe_for_speculation (move_op))
    {
      if ((before_op->prev_op &&
	   !L_no_br_between (before_op->prev_op, move_op)) ||
	  (!before_op->prev_op &&
	   (L_general_branch_opcode (before_op) ||
	    L_check_branch_opcode (before_op) ||
	    !L_no_br_between (before_op, move_op))))
	{
	  if (L_general_load_opcode (move_op) &&
	      (!(L_EXTRACT_BIT_VAL (move_op->flags, L_OPER_MASK_PE))))
	    L_insert_check_after (cb, move_op, move_op);

	  L_mark_oper_speculative (move_op);
	}
    }

  /* Move the instruction */

  if (cb->first_op == move_op)
    cb->first_op = move_op->next_op;
  if (cb->last_op == move_op)
    cb->last_op = move_op->prev_op;
  if (move_op->prev_op != NULL)
    move_op->prev_op->next_op = move_op->next_op;
  if (move_op->next_op != NULL)
    move_op->next_op->prev_op = move_op->prev_op;
  move_op->next_op = before_op;
  move_op->prev_op = before_op->prev_op;
  if (move_op->prev_op != NULL)
    move_op->prev_op->next_op = move_op;
  else
    cb->first_op = move_op;
  if (move_op->next_op != NULL)
    move_op->next_op->prev_op = move_op;
  else
    cb->last_op = move_op;

  /* Insert move_op's flow */

  if (move_op_is_control)
    {
      L_Flow *after_flow = NULL;
      L_Oper *temp_oper;

      /* Find the flow for the first control oper BEFORE move_op, 
         and insert move_flow AFTER its flow. */

      temp_oper = move_op->prev_op;
      while (temp_oper)
	{
	  if (L_is_control_oper (temp_oper))
	    break;
	  temp_oper = temp_oper->prev_op;
	}

      /* If no control oper was found, then insert
         move_flow as the first flow. */
      if (!temp_oper)
	{
	  cb->dest_flow = L_concat_flow (move_flow, cb->dest_flow);
	}
      else
	{
	  if (!(after_flow = L_find_flow_for_branch (cb, temp_oper)))
	    L_punt ("L_move_oper_after: "
		    "Unable to locate flow for op %d.", temp_oper->id);
	  cb->dest_flow =
	    L_insert_flow_after (cb->dest_flow, after_flow, move_flow);
	}
    }

  return;
}


void
L_move_oper_after (L_Cb * cb, L_Oper * move_op, L_Oper * after_op)
{
  L_Oper *ptr;
  L_Flow *move_flow = NULL;
  int move_op_is_control = 0;

  if ((move_op == NULL) || (after_op == NULL))
    L_punt ("L_move_oper_after: move_op and after_op cannot be NULL");

  /* Remove move_op's flow */

  move_op_is_control = L_is_control_oper (move_op);
  if (move_op_is_control)
    {
      move_flow = L_find_flow_for_branch (cb, move_op);
      cb->dest_flow = L_remove_flow (cb->dest_flow, move_flow);
    }

  /* SAM 3-96, set <M> flag for all ops between move_op and after op if necc */

  if (L_mask_potential_exceptions &&
      (L_general_branch_opcode (move_op) ||
       L_check_branch_opcode (move_op)))
    {
      for (ptr = move_op->next_op; ptr != NULL; ptr = ptr->next_op)
	{

	  if (L_mask_potential_exceptions && L_general_load_opcode (ptr) &&
	      (!L_is_trivially_safe (ptr)) &&
	      (!(L_EXTRACT_BIT_VAL (ptr->flags, L_OPER_MASK_PE))))
	    {
	      L_insert_check_after (cb, ptr, after_op);
	    }

	  L_mark_oper_speculative (ptr);

	  if (ptr == after_op)
	    break;
	}
    }

  /* Mark move_op as speculative if neccesary */

  if ((after_op->next_op && !L_no_br_between (after_op->next_op, move_op)))
    {
      if (L_mask_potential_exceptions && L_general_load_opcode (move_op) &&
	  (!L_is_trivially_safe (move_op)) &&
	  (!(L_EXTRACT_BIT_VAL (move_op->flags, L_OPER_MASK_PE))))
	L_insert_check_after (cb, move_op, move_op);

      L_mark_oper_speculative (move_op);
    }

  /* Move the instruction */

  if (cb->first_op == move_op)
    cb->first_op = move_op->next_op;
  if (cb->last_op == move_op)
    cb->last_op = move_op->prev_op;
  if (move_op->prev_op != NULL)
    move_op->prev_op->next_op = move_op->next_op;
  if (move_op->next_op != NULL)
    move_op->next_op->prev_op = move_op->prev_op;
  move_op->prev_op = after_op;
  move_op->next_op = after_op->next_op;
  if (move_op->prev_op != NULL)
    move_op->prev_op->next_op = move_op;
  else
    cb->first_op = move_op;
  if (move_op->next_op != NULL)
    move_op->next_op->prev_op = move_op;
  else
    cb->last_op = move_op;

  /* Insert move_op's flow */

  if (move_op_is_control)
    {
      L_Flow *after_flow = NULL;
      L_Oper *temp_oper;

      /* Find the flow for the first control oper BEFORE move_op, 
         and insert move_flow AFTER its flow. */

      temp_oper = move_op->prev_op;
      while (temp_oper)
	{
	  if (L_is_control_oper (temp_oper))
	    break;
	  temp_oper = temp_oper->prev_op;
	}

      /* If no control oper was found, then insert
         move_flow as the first flow. */
      if (!temp_oper)
	{
	  cb->dest_flow = L_concat_flow (move_flow, cb->dest_flow);
	}
      else
	{
	  if (!(after_flow = L_find_flow_for_branch (cb, temp_oper)))
	    L_punt ("L_move_oper_before: "
		    "Unable to locate flow for op %d.", temp_oper->id);
	  cb->dest_flow =
	    L_insert_flow_after (cb->dest_flow, after_flow, move_flow);
	}
    }

  return;
}


void
L_move_dest_flow_after_emn (L_Cb * from_cb, L_Flow * dst_flow,
			    L_Cb * to_cb, L_Flow * to_after_flow)
{
  L_Flow *src_flow = NULL;

  if (!dst_flow)
    L_punt ("L_move_dest_flow_emn: can not find dst_flow");

  src_flow = L_find_matching_flow (dst_flow->dst_cb->src_flow, dst_flow);
  if (!src_flow)
    L_punt ("L_move_dest_flow_emn: can not find src_flow");

  /* Move dest flow */
  from_cb->dest_flow = L_remove_flow (from_cb->dest_flow, dst_flow);
  dst_flow->src_cb = to_cb;
  to_cb->dest_flow = L_insert_flow_after (to_cb->dest_flow,
                                          to_after_flow, dst_flow);

  /* Change src of src flow */
  src_flow->src_cb = to_cb;
}


L_Flow *
L_move_op_after_emn (L_Cb * from_cb, L_Oper * op,
		     L_Cb * to_cb, L_Oper * to_after_op)
{
  L_Oper *cop = NULL;
  L_Flow *dst_flow = NULL;
  L_Flow *nxt_flow = NULL;
  L_Flow *to_after_flow = NULL;

  if (L_is_control_oper (op))
    {
      /* Get matching flows for op
       */
      dst_flow = L_find_flow_for_branch (from_cb, op);
      if (dst_flow)
        nxt_flow = dst_flow->next_flow;

      /* Find last control op before(and including) to_after_op
       */
      for (cop = to_after_op; (cop && !L_is_control_oper (cop));
           cop = cop->prev_op);

      to_after_flow = cop ? L_find_flow_for_branch (to_cb, cop) : NULL;

      L_move_dest_flow_after_emn (from_cb, dst_flow, to_cb, to_after_flow);
    }

  /* Move the oper itself
   */
  L_remove_oper (from_cb, op);
  L_insert_oper_after (to_cb, to_after_op, op);

  return nxt_flow;
}


L_Cb *
L_split_cb_after (L_Func * fn, L_Cb * cb, L_Oper * op)
{
  L_Cb *new_cb = NULL;
  L_Cb *orig_next_cb = NULL;
  L_Flow *flow = NULL;
  L_Flow *last_flow = NULL;
  L_Oper *move_op = op->next_op;
  L_Oper *next_op = NULL;
  L_Oper *iter_op;
  double weight;

  /* Find new cb weight */

  weight = cb->weight;
  iter_op = cb->first_op;
  while (iter_op)
    {
      if (L_is_control_oper (iter_op) && 
	  (flow = L_find_flow_for_branch (cb, iter_op)))
	weight -= flow->weight;

      if (iter_op != op)
	iter_op = iter_op->next_op;
      else
	break;
    }

  if (iter_op != op)
    L_punt ("L_split_cb_after: op not found");

  orig_next_cb = cb->next_cb;

  /* Create a new cb
   */
  new_cb = L_create_cb (weight);
  new_cb->flags = cb->flags;

  /* Conservation of L_CB_HYPERBLOCK_NO_FALLTHRU flags. */

  if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK_NO_FALLTHRU))
    {
      cb->flags = L_CLR_BIT_FLAG (cb->flags, L_CB_HYPERBLOCK_NO_FALLTHRU);
      new_cb->flags = L_SET_BIT_FLAG (cb->flags, L_CB_HYPERBLOCK_NO_FALLTHRU);
    }

  L_insert_cb_after (fn, cb, new_cb);

  if ((last_flow = L_find_last_flow(cb->dest_flow)) &&
      L_find_branch_for_flow (cb, last_flow))
    last_flow = NULL;

  /* Relocate the opers. */

  while (move_op)
    {
      next_op = move_op->next_op;
      
      L_move_op_after_emn (cb, move_op, new_cb, new_cb->last_op);
      
      move_op = next_op;
    }

  /* Move a fall-through flow from original cb into the new cb. */

  if (last_flow)
    {
      L_Flow *new_fallthrough, *matching_flow;
      
      matching_flow = L_find_matching_flow (orig_next_cb->src_flow, 
					    last_flow);
      
      new_fallthrough = L_copy_single_flow (last_flow);
      new_cb->dest_flow = L_concat_flow (new_cb->dest_flow, new_fallthrough);
      
      cb->dest_flow = L_delete_flow (cb->dest_flow, last_flow);
      L_change_src (new_fallthrough, cb, new_cb);
      L_change_src (matching_flow, cb, new_cb);
    }

  /* Add fallthru flow: to orig cb dest_flow */
  flow = L_new_flow (0, cb, new_cb, weight);
  cb->dest_flow = L_concat_flow (cb->dest_flow, flow);

  /* Add fallthru flow: to new_cb src flow */
  flow = L_new_flow (0, cb, new_cb, weight);
  new_cb->src_flow = L_concat_flow (new_cb->src_flow, flow);


  if (L_dest_flow_out_of_order (cb))
    L_punt ("L_split_cb_after: flows incorrect for orig cb \n");

  if (L_dest_flow_out_of_order (new_cb))
    L_punt ("L_split_cb_after: flows incorrect for new cb \n");

  return new_cb;
}


/*
 * L_split_arc
 * ----------------------------------------------------------------------
 * Split an arc, inserting a CB containing only a jump in the middle.
 * Insert this new CB at the end of the function, or after the source
 * cb if the arc is a fallthru, and either update the branch
 * corresponding to the specified arc or add a new jump to the new
 * block, as appropriate.
 */
L_Cb *
L_split_arc (L_Func *fn, L_Cb *src_cb, L_Flow *dst_fl)
{
  L_Cb *new_cb, *dst_cb;
  L_Flow *src_fl, *new_fl;
  L_Oper *br, *jmp;

  dst_cb = dst_fl->dst_cb;
  src_fl = L_find_matching_flow (dst_cb->src_flow, dst_fl);

  new_cb = L_create_cb (dst_fl->weight);

  if ((br = L_find_branch_for_flow (src_cb, dst_fl)))
    {
      /* splitting a taken arc */
      L_insert_cb_after (fn, fn->last_cb, new_cb);
      L_change_branch_dest (br, dst_cb, new_cb);

      jmp = L_create_new_op (Lop_JUMP);
      jmp->src[0] = L_new_cb_operand (dst_cb);
      L_insert_oper_after (new_cb, new_cb->last_op, jmp);
      src_fl->cc = 1;
    }
  else
    {
      /* splitting a fallthru arc */
      
      if (dst_cb != src_cb->next_cb)
	L_punt ("L_split_arc: fallthru assumption violated");

      L_insert_cb_after (fn, src_cb, new_cb);
    }

  dst_fl->dst_cb = new_cb;
  new_fl = L_copy_single_flow (dst_fl);
  new_cb->src_flow = L_concat_flow (new_cb->src_flow, new_fl);

  src_fl->src_cb = new_cb;
  new_fl = L_new_flow (src_fl->cc, new_cb, dst_cb, new_cb->weight);
  new_cb->dest_flow = L_concat_flow (new_cb->dest_flow, new_fl);

  return new_cb;
}


/*
 * L_expand_flow
 * ----------------------------------------------------------------------
 * Expand the cb at the target of flow oefl into src_cb, given that
 * the conditions specified by the flags are satisfied.  Return 1 on
 * success and 0 on failure.
 */

int
L_expand_flow (L_Func * fn, L_Cb * src_cb, L_Flow * oefl, int flags)
{
  L_Cb *dst_cb;
  L_Flow *ofl, *ifl, *iefl;
  L_Oper *op, *br_op, *tgt_op;
  L_Operand *pred = NULL;
  L_Attr *attr;
  int dst_contains_pred = 0, dst_contains_br_ind = 0,
    dst_contains_jsr = 0;
  double fwt, dwt, drat, frat;

  dst_cb = oefl->dst_cb;

  /* find ratio by which to scale newly incorporated arcs
   * and the dest cb after replication
   */

  fwt = oefl->weight;
  dwt = dst_cb->weight;

  if (fwt > dwt)
    dwt = fwt;

  drat = (dwt > 0.0) ? (dwt - fwt) / dwt : 0.0;

  if (drat < 0.0)
    drat = 0.0;
  else if (drat > 1.0)
    drat = 1.0;

  frat = 1.0 - drat;

  iefl = L_find_matching_flow (dst_cb->src_flow, oefl);

  /* Determine if dst_cb contains predication.  If it does,
   * and if the incorporation of its ops into src_cb requires
   * guarding under a predicate, the transformation requires
   * some additional work.  Defer this case for now.
   */

  for (op = dst_cb->first_op; op; op = op->next_op)
    {
      if (L_is_predicated (op) || L_general_predicate_define_opcode (op))
	dst_contains_pred = 1;

      if (op->opc == Lop_JUMP_RG || op->opc == Lop_JUMP_RG_FS)
	dst_contains_br_ind = 1;

      if (L_subroutine_call_opcode (op))
	dst_contains_jsr = 1;
    }

  /* Determine which case we are dealing with and if
   * in that case the transformation is permitted
   */

  if (!(br_op = L_find_branch_for_flow (src_cb, oefl)))
    {
      /* fallthrough */

      if (!(flags & LOPTI_EXPAND_ALLOW_FALLTHRU))
	return 0;

      /* Incorporating the tail will necessarily leave the
       * block without a fallthrough path.
       */

      if (L_EXTRACT_BIT_VAL (src_cb->flags, L_CB_HYPERBLOCK))
	src_cb->flags = L_SET_BIT_FLAG (src_cb->flags,
					L_CB_HYPERBLOCK_NO_FALLTHRU);

      tgt_op = NULL;
    }				/* if */
  else if (L_uncond_branch_opcode (br_op))
    {
      pred = br_op->pred[0];

      if (!(flags & LOPTI_EXPAND_ALLOW_UNCOND) || 
	  (pred && dst_contains_pred) ||
	  dst_contains_br_ind)
	return 0;

      /* delete the jump op */

      tgt_op = br_op->next_op;

      br_op->pred[0] = NULL;
      L_delete_oper (src_cb, br_op);
    }				/* else if */
  else if (L_cond_branch_opcode (br_op))
    {
      if (!(flags & LOPTI_EXPAND_ALLOW_COND) || 
	  dst_contains_pred || dst_contains_br_ind || dst_contains_jsr)
	return 0;

      /* Convert the branch to a compare generating
       * the predicate needed to guard the incorporated
       * operations.
       */

      L_change_to_cmp_op (br_op);
      L_delete_operand (br_op->src[2]);
      br_op->src[2] = NULL;
      br_op->dest[0] = L_new_register_operand (++L_fn->max_reg_id,
					       L_CTYPE_PREDICATE,
					       L_PTYPE_UNCOND_T);
      pred = L_new_register_operand (L_fn->max_reg_id,
				     L_CTYPE_PREDICATE, L_PTYPE_NULL);
      tgt_op = br_op->next_op;
    }				/* else if */
  else
    {
      return 0;
    }				/* else */

  src_cb->flags |= L_CLR_BIT_FLAG (dst_cb->flags,
				   L_CB_HYPERBLOCK_NO_FALLTHRU);

  /* expand dst_cb at target */

  for (op = dst_cb->first_op; op; op = op->next_op)
    {
      L_Oper *new_op;

      new_op = L_copy_operation (op);

      if (pred)
	new_op->pred[0] = L_copy_operand (pred);

      L_insert_oper_before (src_cb, tgt_op, new_op);
    }				/* for op */

  if ((attr = L_find_attr (dst_cb->attr, "prologue")))
    src_cb->attr = L_concat_attr (src_cb->attr, L_copy_attr (attr));

  for (ofl = dst_cb->dest_flow; ofl; ofl = ofl->next_flow)
    {
      L_Flow *nofl, *nifl;
      L_Cb *o_cb;

      o_cb = ofl->dst_cb;

      ifl = L_find_matching_flow (o_cb->src_flow, ofl);
      nofl = L_copy_single_flow (ofl);
      nifl = L_copy_single_flow (ifl);

      nofl->src_cb = src_cb;
      nifl->src_cb = src_cb;

      nofl->weight *= frat;
      nifl->weight *= frat;

      if (nofl->cc == 0)
	{
	  nofl->cc = 1;
	  nifl->cc = 1;
	}			/* if */

      o_cb->src_flow = L_concat_flow (o_cb->src_flow, nifl);
      src_cb->dest_flow = L_insert_flow_before (src_cb->dest_flow,
						oefl, nofl);
    }				/* for ofl */

  src_cb->dest_flow = L_delete_flow (src_cb->dest_flow, oefl);
  dst_cb->src_flow = L_delete_flow (dst_cb->src_flow, iefl);

  if (L_has_fallthru_to_next_cb (dst_cb))
    {
      L_Oper *new_op;

      ofl = L_find_last_flow (dst_cb->dest_flow);
      new_op = L_create_new_op (Lop_JUMP);
      if (pred)
	{
	  new_op->pred[0] = L_copy_operand (pred);
	  new_op->pred[0]->ptype = L_PTYPE_NULL;
	}			/* if */
      new_op->src[0] = L_new_cb_operand (ofl->dst_cb);
      L_insert_oper_before (src_cb, tgt_op, new_op);
    }				/* if */

  L_scale_cb_weight (dst_cb, drat);

  /* If dst_cb was single-predecessor, the original has been rendered
   * unreachable.
   */

  if (!dst_cb->src_flow)
    {
      for (ofl = dst_cb->dest_flow; ofl; ofl = ofl->next_flow)
	{
	  ifl = L_find_matching_flow (ofl->dst_cb->src_flow, ofl);
	  ofl->dst_cb->src_flow = L_delete_flow (ofl->dst_cb->src_flow, ifl);
	}			/* for ofl */
      L_delete_cb (fn, dst_cb);
    }				/* if */

  L_delete_operand (pred);

  return 1;
}				/* L_expand_flow */


/*
 *      if copy=1, make copy of oper to insert, if copy=0, just insert oper
 */
void
L_insert_op_at_dest_of_br (L_Cb * cb, L_Oper * br_op, L_Oper * op, int copy)
{
  int i;
  L_Oper *new_op;
  L_Cb *dst_cb;
  L_Flow *flow;

  if (!(L_cond_branch_opcode (br_op) || 
	L_uncond_branch_opcode (br_op) ||
	L_check_branch_opcode (br_op)))
    L_punt ("L_insert_op_at_dest_of_br: br_op not a branch");

  new_op = copy ? L_copy_operation (op) : op;

  /* see if can nullify predicate when copying out of cb */

  if (PG_superset_predicate_ops (op, br_op))
    {
      for (i = 0; i < L_max_pred_operand; i++)
	{
	  if (new_op->pred[i])
	    {
	      L_delete_operand (new_op->pred[i]);
	      new_op->pred[i] = NULL;
	    }
	}
    }

  flow = L_find_flow_for_branch (cb, br_op);
  dst_cb = flow->dst_cb;
  new_op->weight = dst_cb->weight;

  if (!L_single_predecessor_cb (dst_cb))
    dst_cb = L_split_arc (L_fn, cb, flow);

  L_insert_oper_before (dst_cb, dst_cb->first_op, new_op);

  if (new_op->sync_info != NULL)
    L_update_sync_arcs_for_new_cb (cb, dst_cb, new_op);

  /* set hyperblock flag if inserted a predicated instr */
  if (L_is_predicated (new_op))
    dst_cb->flags = L_SET_BIT_FLAG (dst_cb->flags, L_CB_HYPERBLOCK);

  return;
}


void
L_insert_op_at_fallthru_dest (L_Cb * cb, L_Oper * op, int copy)
{
  L_Oper *new_op;
  L_Cb *fallthru_cb;
  L_Flow *flow;

  if (L_uncond_branch(cb->last_op))
    {
      fallthru_cb = cb->last_op->src[0]->value.cb;
    }
  else if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK) &&
	   L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK_NO_FALLTHRU))
    {
      if (!L_uncond_branch_opcode (cb->last_op))
	L_punt ("L_insert_op_at_fallthru_dest: "
		"Hyperblock without fallthru has illegal last op");
      fallthru_cb = cb->last_op->src[0]->value.cb;
    }
  else
    {
      fallthru_cb = cb->next_cb;
    }

  new_op = copy ? L_copy_operation (op) : op;

  if (!L_single_predecessor_cb (fallthru_cb))
    {
      /* make new block to insert new_op */

      flow = L_find_last_flow (cb->dest_flow);
      fallthru_cb = L_split_arc (L_fn, cb, flow);

      /* set hyperblock flag if inserted a predicated instr */
      if (L_is_predicated (new_op))
	fallthru_cb->flags = L_SET_BIT_FLAG (fallthru_cb->flags,
					     L_CB_HYPERBLOCK_NO_FALLTHRU);
    }

  /* set hyperblock flag if inserted a predicated instr */

  if (L_is_predicated (new_op))
    fallthru_cb->flags = L_SET_BIT_FLAG (fallthru_cb->flags,
					 L_CB_HYPERBLOCK);
  L_insert_oper_before (fallthru_cb, fallthru_cb->first_op, new_op);

  if (new_op->sync_info != NULL)
    L_update_sync_arcs_for_new_cb (cb, fallthru_cb, new_op);

  return;
}


/*==========================================================================*/
/*
 * Loop optimization functions
 */
/*==========================================================================*/
L_Oper *
L_setup_conditional_op_with_pred (L_Cb * preheader)
{
  L_Oper *pred_clear = NULL;
  L_Oper *pred_set = NULL;

  pred_clear = L_create_new_op (Lop_CMP);
  pred_clear->dest[0] = L_new_register_operand (++L_fn->max_reg_id,
						L_CTYPE_PREDICATE,
						L_PTYPE_COND_F);
  pred_clear->com[0] = L_CTYPE_INT;
  pred_clear->com[1] = Lcmp_COM_EQ;
  pred_clear->src[0] = L_new_int_operand (0, L_CTYPE_INT);
  pred_clear->src[1] = L_new_int_operand (0, L_CTYPE_INT);
  L_insert_oper_before (preheader, preheader->first_op, pred_clear);
  preheader->flags = L_SET_BIT_FLAG (preheader->flags, L_CB_HYPERBLOCK);

  pred_set = L_create_new_op (Lop_CMP);
  pred_set->com[0] = L_CTYPE_INT;
  pred_set->com[1] = Lcmp_COM_EQ;
  pred_set->dest[0] = L_new_register_operand (pred_clear->dest[0]->value.r,
					      L_CTYPE_PREDICATE,
					      L_PTYPE_COND_T);
  pred_set->src[0] = L_new_int_operand (0, L_CTYPE_INT);
  pred_set->src[1] = L_new_int_operand (0, L_CTYPE_INT);

  return pred_set;
}

L_Oper *
L_setup_conditional_op_with_cbs (L_Cb * preheader)
{
  L_Oper *reg_clear = NULL;
  L_Oper *reg_set = NULL;

  reg_clear = L_create_new_op(Lop_MOV);
  reg_clear->src[0] = L_new_int_operand (0, L_CTYPE_INT);
  reg_clear->dest[0] = L_new_register_operand (++L_fn->max_reg_id,
					       L_native_machine_ctype,
					       L_PTYPE_NULL);
  L_insert_oper_before (preheader, preheader->first_op, reg_clear);

  reg_set = L_create_new_op (Lop_MOV);
  reg_set->src[0] = L_new_int_operand (1, L_CTYPE_INT);
  reg_set->dest[0] = L_new_register_operand (reg_clear->dest[0]->value.r,
					     L_native_machine_ctype,
					     L_PTYPE_NULL);

  return reg_set;
}

L_Cb *
L_create_conditional_op_with_pred_in_cb (L_Oper * select, L_Oper * cond_op,
					 L_Cb * at_cb, L_Oper * at_op)
{
  if (!select || !cond_op || !at_cb)
    L_punt ("L_create_conditional_op_with_pred_in_cb");
  if (!select->dest[0])
    L_punt ("L_create_conditional_op_with_pred_in_cb");
  if (cond_op->pred[0])
    L_punt ("L_create_conditional_op_with_pred_in_cb");

  cond_op->pred[0] = L_copy_operand (select->dest[0]);
  L_assign_ptype_null (cond_op->pred[0]);
  at_cb->flags = L_SET_BIT_FLAG (at_cb->flags, L_CB_HYPERBLOCK);

  L_insert_oper_before (at_cb, at_op, cond_op);

  return at_cb;
}


L_Cb *
L_create_conditional_op_with_cbs_at_cb (L_Func * fn,
					L_Oper * select,
					L_Oper * cond_op,
					L_Cb * at_cb, L_Oper * at_op)
{
  L_Oper *new_op = NULL;
  L_Oper *op = NULL;
  L_Oper *next_op = NULL;
  L_Flow *flow = NULL;
  L_Flow *next_flow = NULL;
  L_Flow *src_flow = NULL;
  L_Flow *at_flow = NULL;
  L_Cb *cond_cb = NULL;
  L_Cb *end_cb = NULL;

  if (!select || !cond_op || !at_cb)
    L_punt ("L_create_conditional_op_with_cbs_before_cb");
  if (select->opc != Lop_MOV)
    L_punt ("L_create_conditional_op_with_pred_in_cb");

  cond_cb = L_create_cb (at_cb->weight);
  L_insert_cb_after (fn, at_cb, cond_cb);
  end_cb = L_create_cb (at_cb->weight);
  L_insert_cb_after (fn, cond_cb, end_cb);
  end_cb->flags = at_cb->flags;
  at_cb->flags = 0;
  printf ("Creating new control flow: cb %d cb %d\n", cond_cb->id, end_cb->id);

  /* Put in conditional op */
  L_insert_oper_after (cond_cb, cond_cb->first_op, cond_op);

  /* Move ops to end */
  at_flow = NULL;
  for (op = at_op; op; op = next_op)
    {
      next_op = op->next_op;
      if (!at_flow && L_is_control_oper (op))
	at_flow = L_find_flow_for_branch (at_cb, op);
      L_remove_oper (at_cb, op);
      L_insert_oper_after (end_cb, end_cb->last_op, op);
    }
  if (!at_flow && at_cb->dest_flow)
    at_flow = L_find_last_flow (at_cb->dest_flow);

  /* Move all dest flows to end */
  for (flow = at_flow; flow; flow = next_flow)
    {
      next_flow = flow->next_flow;
      at_cb->dest_flow = L_remove_flow (at_cb->dest_flow, flow);
      end_cb->dest_flow = L_concat_flow (end_cb->dest_flow, flow);
      src_flow = L_find_matching_flow (flow->dst_cb->src_flow, flow);
      flow->src_cb = end_cb;
      src_flow->src_cb = end_cb;
    }

  /* Add branch oper */
  new_op = L_create_new_op (Lop_BR);
  new_op->com[0] = L_CTYPE_INT;
  new_op->com[1] = Lcmp_COM_EQ;
  new_op->src[0] = L_copy_operand (select->dest[0]);
  new_op->src[1] = L_new_int_operand (0, L_CTYPE_INT);
  new_op->src[2] = L_new_cb_operand (end_cb);
  L_insert_oper_after (at_cb, at_cb->last_op, new_op);
  /* Src/Dest flow for branch to end_cb */
  flow = L_new_flow (1, at_cb, end_cb, 0.0);
  at_cb->dest_flow = L_concat_flow (at_cb->dest_flow, flow);
  flow = L_new_flow (1, at_cb, end_cb, 0.0);
  end_cb->src_flow = L_concat_flow (end_cb->src_flow, flow);
  /* Src/Dest flow for fallthru to cond_cb */
  flow = L_new_flow (0, at_cb, cond_cb, at_cb->weight);
  at_cb->dest_flow = L_concat_flow (at_cb->dest_flow, flow);
  flow = L_new_flow (0, at_cb, cond_cb, at_cb->weight);
  cond_cb->src_flow = L_concat_flow (cond_cb->src_flow, flow);

  /* Src/Dest flow for fallthru from cond_cb to end_cb */
  flow = L_new_flow (0, cond_cb, end_cb, at_cb->weight);
  cond_cb->dest_flow = L_concat_flow (cond_cb->dest_flow, flow);
  flow = L_new_flow (0, cond_cb, end_cb, at_cb->weight);
  end_cb->src_flow = L_concat_flow (end_cb->src_flow, flow);

  L_check_cb (fn, at_cb);
  L_check_cb (fn, cond_cb);
  L_check_cb (fn, end_cb);

  return cond_cb;
}


L_Oper *
L_find_last_def_in_cb (L_Cb * cb, L_Operand * operand)
{
  int i;
  L_Oper *op;
  for (op = cb->last_op; op != NULL; op = op->prev_op)
    {
      for (i = 0; i < L_max_dest_operand; i++)
	{
	  if (L_same_operand (op->dest[i], operand))
	    return op;
	}
    }
  return NULL;
}

/*
 *      op = blt (ind_var) (const) (dest_cb) or (ble,bgt,bge)
 *
 *      Convert to less expensive branch: if original branch target to loop
 *      header, convert to bne, else to beq.
 *
 *      L_simplify_loop_branch1 handles case when all parameters are constant
 */
void
L_simplify_loop_branch1 (L_Loop * loop, L_Oper * op)
{
  L_Operand *indvar, *indvar_inc;
  ITintmax ind_init, ind_inc, limit, temp1, temp2, newlimit, target_is_header;
  L_Ind_Info *info;
  L_Cb *target;

  indvar = op->src[0];
  info = L_find_ind_info (loop->ind_info, indvar, 1);
  if (info == NULL)
    L_punt ("L_simplify_loop_branch1: corrupt ind info");
  ind_init = info->offset;
  indvar_inc = info->increment;
  if (!L_is_numeric_constant (indvar_inc))
    L_punt ("L_simplify_loop_branch: ind var must have num const inc");
  ind_inc = indvar_inc->value.i;
  if (!L_is_numeric_constant (op->src[1]))
    L_punt ("L_simplify_loop_branch: src2 of br must be const");
  limit = op->src[1]->value.i;

  /* find new branch limit */
  temp1 = (limit - ind_init) / ind_inc;
  temp2 = (temp1 * ind_inc) + ind_init;
  if (ind_inc > 0)
    {
      if ((temp2 != limit) ||
	  (L_int_bgt_branch_opcode (op)) || (L_int_ble_branch_opcode (op)))
	newlimit = temp2 + ind_inc;
      else
	newlimit = temp2;
    }
  else
    {
      if ((temp2 != limit) ||
	  (L_int_bge_branch_opcode (op)) || (L_int_blt_branch_opcode (op)))
	newlimit = temp2 + ind_inc;
      else
	newlimit = temp2;
    }

  /* If target of br is loop header, convert to BNE, else to BEQ */
  target = op->src[2]->value.cb;
  target_is_header = (target == loop->header);

  if (target_is_header)
    op->com[1] = Lcmp_COM_NE;
  else
    op->com[1] = Lcmp_COM_EQ;

  L_delete_operand (op->src[1]);
  op->src[1] = L_new_gen_int_operand (newlimit);
}

/*
 *      L_simplify_loop_branch2 handles the case where initial val and final
 *      val of ind var may not be known.
 */
void
L_simplify_loop_branch2 (L_Loop * loop, L_Oper * op)
{
  int target_is_header;
  L_Operand *ind_inc, *new_limit;
  L_Cb *preheader, *target;
  L_Oper *last, *new_op;

  new_limit = NULL;
  ind_inc = L_find_basic_induction_increment (op->src[0], loop->ind_info);
  if (ind_inc->value.i > 0)
    {
      if (L_int_bgt_branch_opcode (op) || L_int_ble_branch_opcode (op))
	{
	  preheader = loop->preheader;
	  last = preheader->last_op;
	  new_limit = L_new_register_operand (++L_fn->max_reg_id,
					      L_native_machine_ctype,
					      L_PTYPE_NULL);
	  new_op = L_create_new_op (Lop_ADD);
	  new_op->dest[0] = new_limit;
	  new_op->src[0] = L_copy_operand (op->src[1]);
	  new_op->src[1] = L_copy_operand (ind_inc);
	  L_insert_oper_after (preheader, preheader->last_op, new_op);
	}
    }
  else
    {
      if (L_int_bge_branch_opcode (op) || L_int_blt_branch_opcode (op))
	{
	  preheader = loop->preheader;
	  last = preheader->last_op;
	  new_limit = L_new_register_operand (++L_fn->max_reg_id,
					      L_native_machine_ctype,
					      L_PTYPE_NULL);
	  new_op = L_create_new_op (Lop_ADD);
	  new_op->dest[0] = new_limit;
	  new_op->src[0] = L_copy_operand (op->src[1]);
	  new_op->src[1] = L_copy_operand (ind_inc);
	  L_insert_oper_after (preheader, preheader->last_op, new_op);
	}
    }
  target = op->src[2]->value.cb;
  target_is_header = (target == loop->header);

  if (target_is_header)
    op->com[1] = Lcmp_COM_NE;
  else
    op->com[1] = Lcmp_COM_EQ;

  if (new_limit != NULL)
    {
      L_delete_operand (op->src[1]);
      op->src[1] = L_copy_operand (new_limit);
    }
}


/*
 *      Resultant opcode convert to after loop induction strength reduction.
 */
int
L_str_reduced_opcode (L_Oper * op)
{
  switch (op->opc)
    {
    case Lop_ADD:
      return Lop_ADD;
    case Lop_ADD_U:
      return Lop_ADD_U;
    case Lop_SUB:
      return Lop_ADD;
    case Lop_SUB_U:
      return Lop_ADD_U;
    case Lop_MUL:
      return Lop_ADD;
    case Lop_MUL_U:
      return Lop_ADD_U;
    case Lop_LSL:
      return Lop_ADD;
    default:
      L_punt ("L_str_reduced_opcode: illegal opcode");
      return (0);
    }
}


ITintmax
L_evaluate_str_reduced_opcode (int opc, ITintmax s1, ITintmax s2)
{
  ITintmax t1, t2;
  ITuintmax ut1, ut2;
  switch (opc)
    {
    case Lop_ADD:
      t1 = s1;
      t2 = s2;
      return (t1 + t2);
    case Lop_ADD_U:
      ut1 = s1;
      ut2 = s2;
      return (ut1 + ut2);
    case Lop_SUB:
      t1 = s1;
      t2 = s2;
      return (t1 - t2);
    case Lop_SUB_U:
      ut1 = s1;
      ut2 = s2;
      return (ut1 - ut2);
    case Lop_MUL:
      t1 = s1;
      t2 = s2;
      return (t1 * t2);
    case Lop_MUL_U:
      ut1 = s1;
      ut2 = s2;
      return (ut1 * ut2);
    case Lop_LSL:
      ut1 = s1;
      ut2 = s2;
      return (ut1 << ut2);
    case Lop_LSR:
      ut1 = s1;
      ut2 = s2;
      return (ut1 >> ut2);
    case Lop_ASR:
      t1 = s1;
      t2 = s2;
      return (t1 >> t2);
    case Lop_AND:
      t1 = s1;
      t2 = s2;
      return (t1 & t2);
    case Lop_OR:
      t1 = s1;
      t2 = s2;
      return (t1 | t2);
    case Lop_XOR:
      t1 = s1;
      t2 = s2;
      return (t1 ^ t2);
    case Lop_REM:
      t1 = s1;
      t2 = s2;
      return (t1 % t2);
    case Lop_REM_U:
      ut1 = s1;
      ut2 = s2;
      return (ut1 % ut2);
    case Lop_DIV:
      t1 = s1;
      t2 = s2;
      return (t1 / t2);
    case Lop_DIV_U:
      ut1 = s1;
      ut2 = s2;
      return (ut1 / ut2);
    default:
      L_punt ("L_evaluate_str_reduced_opcode: illegal opcode %d ", opc);
      return (0);
    }
}


/*
 *  op:  (opcode ((dest)) ((basic_ind_var) (const))
 *  create a new operation of the form,
 *      (add ((dest)) ((dest)(new_increment)))
 *      where increment is the amount basic_ind_var is incremented by
 *  A copy of the new operation is then inserted after each place
 *  basic_ind_var is incremented.
 */
void
L_insert_strength_reduced_op_into_loop (L_Loop * loop, int *loop_cb,
					int num_cb, L_Oper * op,
					L_Operand * new_reg)
{
  int i, new_opc;
  L_Operand *basic_ind_var, *orig_increment, *ind_increment, *new_increment;
  L_Cb *preheader, *cb;
  L_Oper *oper, *new_op;
  L_Ind_Info *info;

  new_op = NULL;
  new_opc = L_str_reduced_opcode (op);
  basic_ind_var = op->src[0];
  orig_increment = op->src[1];
  ind_increment =
    L_find_basic_induction_increment (basic_ind_var, loop->ind_info);
  if (L_int_add_opcode (op) || L_int_sub_opcode (op))
    {
      new_increment = L_copy_operand (ind_increment);
    }
  else
    {
      if (L_is_numeric_constant (ind_increment) &&
	  L_is_numeric_constant (orig_increment))
	{
	  ITuintmax imval;
	  imval =
	    L_evaluate_str_reduced_opcode (op->opc, ind_increment->value.i,
					   orig_increment->value.i);
	  new_increment = L_new_gen_int_operand (imval);
	}
      else
	{
	  preheader = loop->preheader;
	  new_increment = L_new_register_operand (++L_fn->max_reg_id,
						  L_opcode_ctype (op),
						  L_PTYPE_NULL);
	  new_op = L_create_new_op (op->opc);
	  new_op->dest[0] = L_copy_operand (new_increment);
	  new_op->src[0] = L_copy_operand (ind_increment);
	  new_op->src[1] = L_copy_operand (orig_increment);
	  L_insert_oper_after (preheader, preheader->last_op, new_op);
	}
    }
  for (i = 0; i < num_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
	{
	  if (!L_same_operand (oper->dest[0], basic_ind_var))
	    continue;
	  new_op = L_create_new_op (new_opc);
	  new_op->dest[0] = L_copy_operand (new_reg);
	  new_op->src[0] = L_copy_operand (new_reg);
	  new_op->src[1] = L_copy_operand (new_increment);
	  L_insert_oper_after (cb, oper, new_op);
	  loop->basic_ind_var_op =
	    Set_add (loop->basic_ind_var_op, new_op->id);
	}
    }
  if (new_op == NULL)
    L_punt ("L_insert_strength_reduced_op_into_loop: no incs of ind var");

  /* update other ind info */
  loop->basic_ind_var = Set_add (loop->basic_ind_var, new_reg->value.r);
  info = L_new_ind_info (new_reg, 1);
  loop->ind_info = L_concat_ind_info (loop->ind_info, info);
  info->increment = L_copy_operand (new_op->src[1]);

  /* delete extra allocated operand */
  L_delete_operand (new_increment);
}


/*
 *      Want to make last_use have the int 0 operand
 */
void
L_reinit_induction_var (L_Loop * loop, int *loop_cb, int num_cb,
			L_Operand * operand, L_Oper * last_use)
{
  int i, is_d, is_s;
  ITintmax val, old_offset_val;
  L_Cb *cb, *preheader;
  L_Oper *op, *new_op;
  L_Operand *old_offset, *new_base;
  L_Ind_Info *info_new;

  new_base = L_new_register_operand (++L_fn->max_reg_id,
				     L_native_machine_ctype, L_PTYPE_NULL);
  if (L_same_operand (operand, last_use->src[0]))
    old_offset = L_copy_operand (last_use->src[1]);
  else if (L_same_operand (operand, last_use->src[1]))
    old_offset = L_copy_operand (last_use->src[0]);
  else
    {
      L_punt ("L_reinit_induction_var: last_use is corrupt");
      return;
    }

  if (L_is_int_constant (old_offset))
    old_offset_val = old_offset->value.i;
  else
    old_offset_val = -99999;	/* random number for debugging */

  /*
   *  2 cases here, if we dealing with all int constants, allow offsets
   *  to be different since can modify them in the ld/st.  If not int
   *  constants all offsets with operand as base must be the same.
   */
  for (i = 0; i < num_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);
      for (op = cb->first_op; op != NULL; op = op->next_op)
	{
	  is_d = L_is_dest_operand (operand, op);
	  is_s = L_is_src_operand (operand, op);
	  if (is_d)
	    {
	      if (!Set_in (loop->basic_ind_var_op, op->id))
		L_punt ("L_reinit_induction_var: illegal def of operand");
	      L_delete_operand (op->dest[0]);
	      L_delete_operand (op->src[0]);
	      op->dest[0] = L_copy_operand (new_base);
	      op->src[0] = L_copy_operand (new_base);
	      continue;
	    }
	  if (!is_s)
	    continue;
	  if ((L_load_opcode (op) || L_store_opcode (op)) &&
	      !L_EXTRACT_BIT_VAL (op->flags, L_OPER_VOLATILE))
	    {
	      if (L_same_operand (op->src[0], operand))
		{
		  if (L_is_int_constant (op->src[1]))
		    {
		      val = op->src[1]->value.i;
		      L_delete_operand (op->src[0]);
		      L_delete_operand (op->src[1]);
		      op->src[0] = L_copy_operand (new_base);
		      op->src[1] =
			L_new_gen_int_operand (val - old_offset_val);
		    }
		  else if (L_same_operand (op->src[1], old_offset))
		    {
		      L_delete_operand (op->src[0]);
		      L_delete_operand (op->src[1]);
		      op->src[0] = L_copy_operand (new_base);
		      op->src[1] = L_new_gen_int_operand (0);
		    }
		  else
		    {
		      L_punt
			("L_reinit_induction_var: illegal use of operand");
		    }
		}
	      else
		{
		  if (L_is_int_constant (op->src[0]))
		    {
		      val = op->src[0]->value.i;
		      L_delete_operand (op->src[0]);
		      L_delete_operand (op->src[1]);
		      op->src[0] =
			L_new_gen_int_operand (val - old_offset_val);
		      op->src[1] = L_copy_operand (new_base);
		    }
		  else if (L_same_operand (op->src[0], old_offset))
		    {
		      L_delete_operand (op->src[0]);
		      L_delete_operand (op->src[1]);
		      op->src[0] = L_new_gen_int_operand (0);
		      op->src[1] = L_copy_operand (new_base);
		    }
		  else
		    {
		      L_punt
			("L_reinit_induction_var: illegal use of operand");
		    }
		}
	    }
	  else if (L_int_cond_branch_opcode (op))
	    {
	      if (L_same_operand (op->src[0], operand))
		{
		  if (!L_is_int_constant (op->src[1]))
		    L_punt ("L_reinit_induction_var: illegal use of operand");
		  val = op->src[1]->value.i;
		  L_delete_operand (op->src[0]);
		  L_delete_operand (op->src[1]);
		  op->src[0] = L_copy_operand (new_base);
		  op->src[1] = L_new_gen_int_operand (old_offset_val + val);
		}
	      else
		{
		  if (!L_is_int_constant (op->src[0]))
		    L_punt ("L_reinit_induction_var: illegal use of operand");
		  val = op->src[0]->value.i;
		  L_delete_operand (op->src[0]);
		  L_delete_operand (op->src[1]);
		  op->src[0] = L_new_gen_int_operand (old_offset_val + val);
		  op->src[1] = L_copy_operand (new_base);
		}
	    }
	  else
	    {
	      L_punt ("L_reinit_induction_var: illegal use of operand");
	    }
	}
    }

  {
    L_Operand *scott;
    scott = L_find_basic_induction_increment (operand, loop->ind_info);
  }
  /* insert new op into preheader to initialize ind var */
  preheader = loop->preheader;
  new_op = L_create_new_op (Lop_ADD);
  new_op->dest[0] = new_base;
  new_op->src[0] = L_copy_operand (operand);
  new_op->src[1] = old_offset;
  L_insert_oper_after (preheader, preheader->last_op, new_op);

  {
    L_Operand *scott;
    scott = L_find_basic_induction_increment (operand, loop->ind_info);
  }
  /* update induction var info */
  loop->basic_ind_var = Set_delete (loop->basic_ind_var, operand->value.r);
  loop->basic_ind_var = Set_add (loop->basic_ind_var, new_base->value.r);
  info_new = L_new_ind_info (new_base, 1);
  loop->ind_info = L_concat_ind_info (loop->ind_info, info_new);
  info_new->increment =
    L_copy_operand (L_find_basic_induction_increment
		    (operand, loop->ind_info));
  L_invalidate_ind_var (operand, loop->ind_info);
}


void
L_delete_all_basic_ind_var_op_from_loop (L_Loop * loop, int *loop_cb,
					 int num_cb, L_Operand * operand)
{
  int i;
  L_Cb *cb;
  L_Oper *op, *next;
  for (i = 0; i < num_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);
      for (op = cb->first_op; op != NULL; op = next)
	{
	  next = op->next_op;
	  if (!L_same_operand (op->dest[0], operand))
	    continue;
	  if (!Set_in (loop->basic_ind_var_op, op->id))
	    L_punt ("L_delete_all_basic_ind_var_op_from_loop: illegal op");
	  loop->basic_ind_var_op =
	    Set_delete (loop->basic_ind_var_op, op->id);
	  L_delete_oper (cb, op);
	}
    }
}


/*
 *      There should be atleast 1 use of operand between op1 and op2, try
 *      to move op1 below last use of operand or move op2 above the first use
 *      of operand.
 */
int
L_reorder_ops_so_no_use_betw (L_Loop * loop, L_Cb * cb, L_Operand * operand,
			      L_Oper * op1, L_Oper * op2)
{
  int i;
  L_Oper *op, *first_use, *last_use;
  if (!loop || !cb || !operand || !op1 || !op2)
    L_punt ("L_reorder_ops_so_no_use_betw: one of the args is NIL");

  first_use = NULL;
  last_use = NULL;
  for (op = op1; op; op = op->next_op)
    {
      if (op == op2)
	break;
      if (Set_in (loop->basic_ind_var_op, op->id))
	continue;
      for (i = 0; i < L_max_src_operand; i++)
	{
	  if (L_same_operand (op->src[i], operand))
	    {
	      first_use = op;
	      break;
	    }
	}
      if (first_use)
	break;
    }
  if (!first_use)
    L_punt ("L_reorder_ops_so_no_use_betw: first use not found");

  for (op = op2; op; op = op->prev_op)
    {
      if (op == op1)
	break;
      if (Set_in (loop->basic_ind_var_op, op->id))
	continue;
      for (i = 0; i < L_max_src_operand; i++)
	{
	  if (L_same_operand (op->src[i], operand))
	    {
	      last_use = op;
	      break;
	    }
	}
      if (last_use)
	break;
    }
  if (!last_use)
    L_punt ("L_reorder_ops_so_no_use_betw: last use not found");

  if (L_can_move_below (cb, last_use, op1))
    {
      L_move_oper_after (cb, op1, last_use);
      return 1;
    }
  else if (L_can_move_above (cb, first_use, op2))
    {
      L_move_oper_before (cb, op2, first_use);
      return 1;
    }
  return 0;
}


/*
 *      SAM 2-97, analgous changes to those made to
 *      L_ind_constant_offset_initial_val() in l_opti_predicates.c
 */
int
L_find_ind_initial_offset (L_Cb * preheader, L_Operand * operand1,
			   L_Operand * operand2, L_Oper * start_op1,
			   L_Oper * start_op2, L_Ind_Info * ind_info)
{
  L_Oper *op1, *op2;
  L_Ind_Info *info1, *info2;

  info1 = L_find_ind_info (ind_info, operand1, 1);
  info2 = L_find_ind_info (ind_info, operand2, 1);

  if (!info1 && !info2)
    {
      if (!start_op1)
	start_op1 = preheader->last_op;
      if (!start_op2)
	start_op2 = preheader->last_op;

      if (start_op1 && start_op2 &&
	  (op1 = L_prev_def (operand1, start_op1)) && L_int_add_opcode (op1) &&
	  (op2 = L_prev_def (operand2, start_op2)) && L_int_add_opcode (op2))
	{
	  if (L_same_operand (op1->src[0], op2->src[0]))
	    return (L_find_ind_initial_offset (preheader, op1->src[1],
					       op2->src[1], op1, op2,
					       ind_info));
	  else if (L_same_operand (op1->src[0], op2->src[1]))
	    return (L_find_ind_initial_offset (preheader, op1->src[1],
					       op2->src[0], op1, op2,
					       ind_info));
	  else if (L_same_operand (op1->src[1], op2->src[0]))
	    return (L_find_ind_initial_offset (preheader, op1->src[0],
					       op2->src[1], op1, op2,
					       ind_info));
	  else if (L_same_operand (op1->src[1], op2->src[1]))
	    return (L_find_ind_initial_offset (preheader, op1->src[0],
					       op2->src[0], op1, op2,
					       ind_info));
	  else
	    L_punt ("L_find_ind_initial_offset: op1 and op2 must share src");
	}
      else
	{
	  L_punt ("L_find_ind_initial_offset: illegal op1/op2");
	}
    }
  else if (!info1)
    {
      if (!L_same_operand (info2->base, operand1))
	L_punt ("L_find_ind_initial_offset: base2 must be same as operand1");
      if (info2->coeff != 1)
	L_punt ("L_find_ind_initial_offset: coeff2 must be 1");
      return (-info2->offset);
    }
  else if (!info2)
    {
      if (!L_same_operand (info1->base, operand2))
	L_punt ("L_find_ind_initial_offset: base1 must be same as operand2");
      if (info1->coeff != 1)
	L_punt ("L_find_ind_initial_offset: coeff1 must be 1");
      return (info1->offset);
    }
  else
    {
      if (!L_same_operand (info1->base, info2->base))
	L_punt ("L_find_ind_initial_offset: base1 must be same as base2");
      if (info1->coeff != info2->coeff)
	L_punt ("L_find_ind_initial_offset: coeff1 must be same as coeff2");
      return (info1->offset - info2->offset);
    }
  
  L_punt ("L_find_ind_initial_offset: shouldnt reach this point");
  return (0);
}


/*
 *      Ind_Info must be setup before calling this.
 */
void
L_simplify_combs_of_ind_vars (L_Loop * loop, int *loop_cb, int num_cb)
{
  int i, offset;
  L_Cb *cb;
  L_Oper *op;
  for (i = 0; i < num_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);
      for (op = cb->first_op; op != NULL; op = op->next_op)
	{
	  /*
	   *  Match pattern
	   */
	  if (!L_int_sub_opcode (op))
	    continue;
	  if (!L_basic_induction_var (loop, op->src[0]))
	    continue;
	  if (!L_basic_induction_var (loop, op->src[1]))
	    continue;
	  if (!L_same_ind_increment (op->src[0], op->src[1], loop->ind_info))
	    continue;
	  if (!L_ind_constant_offset_initial_val (loop->preheader, op->src[0],
						  op->src[1], NULL, NULL,
						  loop->ind_info))
	    continue;
	  /*
	   *  Replace pattern
	   */
	  offset = L_find_ind_initial_offset (loop->preheader, op->src[0],
					      op->src[1], NULL, NULL,
					      loop->ind_info);
	  L_convert_to_move (op, L_copy_operand (op->dest[0]),
			     L_new_gen_int_operand (offset));
	  STAT_COUNT ("L_simplify_combs_of_ind_vars", 1, NULL);
	}
    }
}


/*
 *      Eliminate operand1, replace with operand2. -- Delete all inductive
 *      defs of operand1, and change all uses of operand1 to operand2.
 */
void
L_induction_elim_1 (L_Loop * loop, int *loop_cb, int num_cb,
		    L_Operand * operand1, L_Operand * operand2)
{
  int i, j;
  L_Cb *cb;
  L_Oper *op, *next;
  for (i = 0; i < num_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);
      for (op = cb->first_op; op != NULL; op = next)
	{
	  next = op->next_op;
	  if (L_same_operand (op->dest[0], operand1))
	    {
	      if (!Set_in (loop->basic_ind_var_op, op->id))
		L_punt ("L_induction_elim_1: illegal def of operand1");
	      loop->basic_ind_var_op =
		Set_delete (loop->basic_ind_var_op, op->id);
	      L_delete_oper (cb, op);
	      continue;
	    }
	  for (j = 0; j < L_max_src_operand; j++)
	    {
	      if (L_same_operand (op->src[j], operand1))
		{
		  L_delete_operand (op->src[j]);
		  op->src[j] = L_copy_operand (operand2);
		}
	    }
	}
    }
}

void
L_change_offset_for_all_uses (L_Loop * loop, int *loop_cb, int num_cb,
			      L_Operand * operand, int offset)
{
  int i;
  ITintmax new;
  L_Cb *cb;
  L_Oper *op;

  for (i = 0; i < num_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);
      for (op = cb->first_op; op != NULL; op = op->next_op)
	{
	  if (!L_is_src_operand (operand, op))
	    continue;
	  if (L_load_opcode (op) || L_store_opcode (op))
	    {
	      if (L_same_operand (operand, op->src[2]))
		L_punt
		  ("L_change_offset_for_all_uses: illegal use of operand");
	      if (!
		  (L_is_int_constant (op->src[0])
		   || L_is_int_constant (op->src[1])))
		L_punt
		  ("L_change_offset_for_all_uses: s1 or s2 must be const");
	      if (L_same_operand (operand, op->src[0]))
		{
		  new = op->src[1]->value.i + (ITintmax) offset;
		  L_delete_operand (op->src[1]);
		  op->src[1] = L_new_gen_int_operand (new);
		}
	      else
		{
		  new = op->src[0]->value.i + (ITintmax) offset;
		  L_delete_operand (op->src[0]);
		  op->src[0] = L_new_gen_int_operand (new);
		}
	    }
	  else
	    {
	      L_punt ("L_change_offset_for_all_uses: illegal opcode");
	    }
	}
    }
}

/*
 *      Eliminate oper1 replace with oper2
 */
void
L_induction_elim_2 (L_Loop * loop, int *loop_cb, int num_cb,
		    L_Operand * operand1, L_Operand * operand2)
{
  int i;
  ITintmax offset, new_offset;
  L_Cb *cb, *preheader;
  L_Oper *op, *next, *new_op;
  L_Operand *new_reg;

  offset = L_find_ind_initial_offset (loop->preheader, operand1, operand2,
				      NULL, NULL, loop->ind_info);
#if 0
  if (offset < 0)
    L_punt ("L_induction_elim_2: offset should be positive");
#endif
  preheader = loop->preheader;
  for (i = 0; i < num_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);
      for (op = cb->first_op; op != NULL; op = next)
	{
	  next = op->next_op;
	  if (L_same_operand (op->dest[0], operand1))
	    {
	      if (!Set_in (loop->basic_ind_var_op, op->id))
		L_punt ("L_induction_elim_2: illegal def of oper1");
	      loop->basic_ind_var_op =
		Set_delete (loop->basic_ind_var_op, op->id);
	      L_delete_oper (cb, op);
	      continue;
	    }
	  else if (L_same_operand (op->src[0], operand1))
	    {
	      L_delete_operand (op->src[0]);
	      op->src[0] = L_copy_operand (operand2);
	      if (L_move_opcode (op))
		{
		  L_change_opcode (op, Lop_ADD);
		  op->src[1] = L_new_gen_int_operand (offset);
		}
	      else if (L_load_opcode (op) && 
		       !L_EXTRACT_BIT_VAL (op->flags, L_OPER_VOLATILE))
		{
		  if (!L_is_int_constant (op->src[1]))
		    L_punt ("L_induction_elim_2: src2 must be constant");
		  new_offset = op->src[1]->value.i + offset;
		  L_delete_operand (op->src[1]);
		  op->src[1] = L_new_gen_int_operand (new_offset);
		}
	      else if (L_store_opcode (op) &&
		       !L_EXTRACT_BIT_VAL (op->flags, L_OPER_VOLATILE))
		{
		  if (!L_is_int_constant (op->src[1]))
		    L_punt ("L_induction_elim_2: src2 must be constant");
		  new_offset = op->src[1]->value.i + offset;
		  L_delete_operand (op->src[1]);
		  op->src[1] = L_new_gen_int_operand (new_offset);
		}
	      else if (L_int_add_opcode (op))
		{
		  if (L_is_int_constant (op->src[1]))
		    {
		      new_offset = op->src[1]->value.i + offset;
		      L_delete_operand (op->src[1]);
		      op->src[1] = L_new_gen_int_operand (new_offset);
		    }
		  else
		    if (L_only_used_as_base_addr_with_const_offset_in_loop
			(loop, loop_cb, num_cb, op->dest[0], offset))
		    {
		      L_change_offset_for_all_uses (loop, loop_cb, num_cb,
						    op->dest[0], offset);
		    }
		  else
		    {
		      L_punt ("L_induction_elim_2: illegal case for int add");
		    }
		}
	      else if (L_int_sub_opcode (op))
		{
		  if (!L_is_int_constant (op->src[1]))
		    L_punt ("L_induction_elim_2: src2 must be constant");
		  new_offset = op->src[1]->value.i - offset;
		  L_delete_operand (op->src[1]);
		  op->src[1] = L_new_gen_int_operand (new_offset);
		}
	      else if (L_int_comparison_opcode (op))
		{
		  if (!L_is_int_constant (op->src[1]))
		    L_punt ("L_induction_elim_2: src2 must be constant");
		  new_offset = op->src[1]->value.i - offset;
		  L_delete_operand (op->src[1]);
		  op->src[1] = L_new_gen_int_operand (new_offset);
		}
	      else if (L_int_cond_branch_opcode (op))
		{
		  if (L_is_int_constant (op->src[1]))
		    {
		      new_offset = op->src[1]->value.i - offset;
		      L_delete_operand (op->src[1]);
		      op->src[1] = L_new_gen_int_operand (new_offset);
		    }
		  else
		    {
		      new_reg = L_new_register_operand (++L_fn->max_reg_id,
							L_native_machine_ctype,
							L_PTYPE_NULL);
		      new_op = L_create_new_op (Lop_SUB);
		      new_op->dest[0] = new_reg;
		      new_op->src[0] = L_copy_operand (op->src[1]);
		      new_op->src[1] = L_new_gen_int_operand (offset);
		      L_insert_oper_after (preheader, preheader->last_op,
					   new_op);
		      L_delete_operand (op->src[1]);
		      op->src[1] = L_copy_operand (new_reg);
		    }
		}
	      else
		{
		  L_punt ("L_induction_elim_2: illegal opcode for op");
		}
	    }
	  else if (L_same_operand (op->src[1], operand1))
	    {
	      L_delete_operand (op->src[1]);
	      op->src[1] = L_copy_operand (operand2);
	      if (L_move_opcode (op))
		{
		  L_punt ("L_induction_elim_2: move with 2 srcs????");
		}
	      else if (L_load_opcode (op))
		{
		  if (!L_is_int_constant (op->src[0]))
		    L_punt ("L_induction_elim_2: src1 must be constant");
		  new_offset = op->src[0]->value.i + offset;
		  L_delete_operand (op->src[0]);
		  op->src[0] = L_new_gen_int_operand (new_offset);
		}
	      else if (L_store_opcode (op))
		{
		  if (!L_is_int_constant (op->src[0]))
		    L_punt ("L_induction_elim_2: src1 must be constant");
		  new_offset = op->src[0]->value.i + offset;
		  L_delete_operand (op->src[0]);
		  op->src[0] = L_new_gen_int_operand (new_offset);
		}
	      else if (L_int_add_opcode (op))
		{
		  if (L_is_int_constant (op->src[0]))
		    {
		      new_offset = op->src[0]->value.i + offset;
		      L_delete_operand (op->src[0]);
		      op->src[0] = L_new_gen_int_operand (new_offset);
		    }
		  else
		    if (L_only_used_as_base_addr_with_const_offset_in_loop
			(loop, loop_cb, num_cb, op->dest[0], offset))
		    {
		      L_change_offset_for_all_uses (loop, loop_cb, num_cb,
						    op->dest[0], offset);
		    }
		  else
		    {
		      L_punt ("L_induction_elim_2: illegal case for int add");
		    }
		}
	      else if (L_int_sub_opcode (op))
		{
		  if (!L_is_int_constant (op->src[0]))
		    L_punt ("L_induction_elim_2: src1 must be constant");
		  new_offset = op->src[0]->value.i - offset;
		  L_delete_operand (op->src[0]);
		  op->src[0] = L_new_gen_int_operand (new_offset);
		}
	      else if (L_int_comparison_opcode (op))
		{
		  if (!L_is_int_constant (op->src[0]))
		    L_punt ("L_induction_elim_2: src1 must be constant");
		  new_offset = op->src[0]->value.i - offset;
		  L_delete_operand (op->src[0]);
		  op->src[0] = L_new_gen_int_operand (new_offset);
		}
	      else if (L_int_cond_branch_opcode (op))
		{
		  if (L_is_int_constant (op->src[0]))
		    {
		      new_offset = op->src[0]->value.i - offset;
		      L_delete_operand (op->src[0]);
		      op->src[0] = L_new_gen_int_operand (new_offset);
		    }
		  else
		    {
		      new_reg = L_new_register_operand (++L_fn->max_reg_id,
							L_native_machine_ctype,
							L_PTYPE_NULL);
		      new_op = L_create_new_op (Lop_SUB);
		      new_op->dest[0] = new_reg;
		      new_op->src[0] = L_copy_operand (op->src[0]);
		      new_op->src[1] = L_new_gen_int_operand (offset);
		      L_insert_oper_after (preheader, preheader->last_op,
					   new_op);
		      L_delete_operand (op->src[0]);
		      op->src[0] = L_copy_operand (new_reg);
		    }
		}
	      else
		{
		  L_punt ("L_induction_elim_2: illegal opcode for op");
		}
	    }
	  else if (L_is_src_operand (operand1, op))
	    {
	      L_punt ("L_induction_elim_2: illegal src of op");
	      return;
	    }

	}
    }
}

/*
 *      Eliminate operand1, replace with operand2
 */
void
L_induction_elim_3 (L_Loop * loop, int *loop_cb, int num_cb,
		    L_Operand * operand1, L_Operand * operand2)
{
  int i;
  L_Cb *preheader, *cb;
  L_Oper *op, *next, *new_op;
  L_Operand *other_operand, *new_reg1, *new_reg2 = NULL;

  for (i = 0; i < num_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);
      for (op = cb->first_op; op != NULL; op = next)
	{
	  next = op->next_op;
	  if (L_same_operand (op->dest[0], operand1))
	    {
	      if (!Set_in (loop->basic_ind_var_op, op->id))
		L_punt ("L_induction_elim_3: illegal def of operand1");
	      loop->basic_ind_var_op =
		Set_delete (loop->basic_ind_var_op, op->id);
	      L_delete_oper (cb, op);
	      continue;
	    }
	  if (!L_is_src_operand (operand1, op))
	    continue;
	  if (!(L_same_operand (operand1, op->src[0]) ||
		L_same_operand (operand1, op->src[1])))
	    L_punt ("L_induction_elim_3: illegal use of operand1");
	  if (L_same_operand (operand1, op->src[0]))
	    other_operand = op->src[1];
	  else
	    other_operand = op->src[0];

	  /* Insert preheader ops */
	  preheader = loop->preheader;
	  if (L_move_opcode (op))
	    {
	      new_reg1 = L_new_register_operand (++L_fn->max_reg_id,
						 L_native_machine_ctype,
						 L_PTYPE_NULL);
	      new_op = L_create_new_op (Lop_SUB);
	      L_insert_oper_after (preheader, preheader->last_op, new_op);
	      new_op->dest[0] = new_reg1;
	      new_op->src[0] = L_copy_operand (operand1);
	      new_op->src[1] = L_copy_operand (operand2);
	    }
	  else if (L_int_cond_branch_opcode (op) ||
		   L_int_comparison_opcode (op) || L_int_sub_opcode (op))
	    {
	      new_reg1 = L_new_register_operand (++L_fn->max_reg_id,
						 L_native_machine_ctype,
						 L_PTYPE_NULL);
	      new_reg2 = L_new_register_operand (++L_fn->max_reg_id,
						 L_native_machine_ctype,
						 L_PTYPE_NULL);
	      new_op = L_create_new_op (Lop_SUB);
	      L_insert_oper_after (preheader, preheader->last_op, new_op);
	      new_op->dest[0] = new_reg1;
	      new_op->src[0] = L_copy_operand (operand2);
	      new_op->src[1] = L_copy_operand (operand1);
	      new_op = L_create_new_op (Lop_ADD);
	      L_insert_oper_after (preheader, preheader->last_op, new_op);
	      new_op->dest[0] = new_reg2;
	      new_op->src[0] = L_copy_operand (new_reg1);
	      new_op->src[1] = L_copy_operand (other_operand);
	    }
	  else if ((L_load_opcode (op) ||
		    L_store_opcode (op) || L_int_add_opcode (op)) &&
		   !L_EXTRACT_BIT_VAL (op->flags, L_OPER_VOLATILE))
	    {
	      new_reg1 = L_new_register_operand (++L_fn->max_reg_id,
						 L_native_machine_ctype,
						 L_PTYPE_NULL);
	      new_reg2 = L_new_register_operand (++L_fn->max_reg_id,
						 L_native_machine_ctype,
						 L_PTYPE_NULL);
	      new_op = L_create_new_op (Lop_SUB);
	      L_insert_oper_after (preheader, preheader->last_op, new_op);
	      new_op->dest[0] = new_reg1;
	      new_op->src[0] = L_copy_operand (operand1);
	      new_op->src[1] = L_copy_operand (operand2);
	      new_op = L_create_new_op (Lop_ADD);
	      L_insert_oper_after (preheader, preheader->last_op, new_op);
	      new_op->dest[0] = new_reg2;
	      new_op->src[0] = L_copy_operand (new_reg1);
	      new_op->src[1] = L_copy_operand (other_operand);
	    }
	  else
	    {
	      L_punt ("L_induction_elim_3: illegal opcode for op");
	      return;
	    }

	  /* Modify op */
	  if (L_move_opcode (op))
	    {
	      L_change_opcode (op, Lop_ADD);
	      L_delete_operand (op->src[0]);
	      op->src[0] = L_copy_operand (operand2);
	      op->src[1] = L_copy_operand (new_reg1);
	    }
	  else if (L_same_operand (operand1, op->src[0]))
	    {
	      L_delete_operand (op->src[0]);
	      L_delete_operand (op->src[1]);
	      op->src[0] = L_copy_operand (operand2);
	      op->src[1] = L_copy_operand (new_reg2);
	    }
	  else
	    {
	      L_delete_operand (op->src[0]);
	      L_delete_operand (op->src[1]);
	      op->src[0] = L_copy_operand (new_reg2);
	      op->src[1] = L_copy_operand (operand2);
	    }
	}
    }

}

/*
 *      Elim operand1 replace with operand2
 */
void
L_induction_elim_4 (L_Loop * loop, int *loop_cb, int num_cb,
		    L_Operand * operand1, L_Operand * operand2)
{
  int i;
  ITintmax ratio;
  L_Operand *inc1, *inc2;
  L_Cb *cb, *preheader;
  L_Oper *op, *next, *new_op;
  L_Operand *other_operand, *new_reg1, *new_reg2, *new_reg3;

  inc1 = L_find_basic_induction_increment (operand1, loop->ind_info);
  inc2 = L_find_basic_induction_increment (operand2, loop->ind_info);
  if (!(L_is_numeric_constant (inc1) && L_is_numeric_constant (inc2)))
    L_punt ("L_induction_elim_4: inc1 and inc2 must be int constants");
  ratio = inc2->value.i / inc1->value.i;

  for (i = 0; i < num_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);
      for (op = cb->first_op; op != NULL; op = next)
	{
	  next = op->next_op;
	  if (L_same_operand (op->dest[0], operand1))
	    {
	      if (!Set_in (loop->basic_ind_var_op, op->id))
		L_punt ("L_induction_elim_4: illegal def of operand1");
	      loop->basic_ind_var_op =
		Set_delete (loop->basic_ind_var_op, op->id);
	      L_delete_oper (cb, op);
	      continue;
	    }
	  if (!L_is_src_operand (operand1, op))
	    continue;
	  if (!(L_same_operand (operand1, op->src[0]) ||
		L_same_operand (operand1, op->src[1])))
	    L_punt ("L_induction_elim_3: illegal use of operand1");
	  if (L_same_operand (operand1, op->src[0]))
	    other_operand = op->src[1];
	  else
	    other_operand = op->src[0];

	  /* Insert preheader ops */
	  preheader = loop->preheader;
	  new_reg1 = L_new_register_operand (++L_fn->max_reg_id,
					     L_native_machine_ctype,
					     L_PTYPE_NULL);
	  new_reg2 = L_new_register_operand (++L_fn->max_reg_id,
					     L_native_machine_ctype,
					     L_PTYPE_NULL);
	  new_reg3 = L_new_register_operand (++L_fn->max_reg_id,
					     L_native_machine_ctype,
					     L_PTYPE_NULL);
	  if (L_int_cond_branch_opcode (op) || L_int_comparison_opcode (op))
	    {
	      new_op = L_create_new_op (Lop_SUB);
	      L_insert_oper_after (preheader, preheader->last_op, new_op);
	      new_op->dest[0] = new_reg1;
	      new_op->src[0] = L_copy_operand (other_operand);
	      new_op->src[1] = L_copy_operand (operand1);
	      new_op = L_create_new_op (Lop_MUL);
	      L_insert_oper_after (preheader, preheader->last_op, new_op);
	      new_op->dest[0] = new_reg2;
	      new_op->src[0] = L_copy_operand (new_reg1);
	      new_op->src[1] = L_new_gen_int_operand (ratio);
	      new_op = L_create_new_op (Lop_ADD);
	      L_insert_oper_after (preheader, preheader->last_op, new_op);
	      new_op->dest[0] = new_reg3;
	      new_op->src[0] = L_copy_operand (new_reg2);
	      new_op->src[1] = L_copy_operand (operand2);
	    }
	  else
	    {
	      L_punt ("L_induction_elim_4: illegal opcode for op");
	    }

	  /* Modify op */
	  if (L_same_operand (operand1, op->src[0]))
	    {
	      L_delete_operand (op->src[0]);
	      L_delete_operand (op->src[1]);
	      op->src[0] = L_copy_operand (operand2);
	      op->src[1] = L_copy_operand (new_reg3);
	    }
	  else
	    {
	      L_delete_operand (op->src[0]);
	      L_delete_operand (op->src[1]);
	      op->src[0] = L_copy_operand (new_reg3);
	      op->src[1] = L_copy_operand (operand2);
	    }
	  if ((L_int_cond_branch_opcode (op)) && (ratio < 0))
	    L_swap_compare (op);
	}
    }
}

L_Oper *
L_find_last_use_of_ind_var (L_Loop * loop, int *loop_cb, int num_cb,
			    int *backedge_cb, int num_backedge_cb, L_Cb * cb,
			    L_Oper * op)
{
  int i;
  L_Cb *curr_cb, *marked_cb;
  L_Oper *ptr, *last_use, *marked_oper;
  L_Operand *ind_var;

  ind_var = op->dest[0];

  /* first just check cb for prev use */
  last_use = L_prev_use (ind_var, op);
  if (last_use)
    return (last_use);

  /*
   *  Handle a special case for separate basic blocks, hopefully it will
   *          capture the majority of cases..
   *  1. cb executed on every iteration
   *  2. all uses of ind_var in same cb
   *  3. last_use dominates op
   */

  /* 1 */
  if (!L_cb_dominates_all_loop_backedge_cb
      (loop, backedge_cb, num_backedge_cb, cb))
    return (NULL);
  marked_cb = NULL;
  marked_oper = NULL;
  for (i = 0; i < num_cb; i++)
    {
      curr_cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);
      if (curr_cb == cb)
	continue;
      for (ptr = curr_cb->last_op; ptr != NULL; ptr = ptr->prev_op)
	{
	  if (L_is_src_operand (ind_var, ptr))
	    {
	      /* 2 */
	      if (marked_cb == NULL)
		{
		  marked_cb = curr_cb;
		  marked_oper = ptr;
		  break;
		}
	      else
		{
		  return (NULL);
		}
	    }
	}
    }
  /* 3 */
  if ((!marked_cb) || (!L_in_cb_DOM_set (cb, marked_cb->id)))
    return (NULL);

  if (L_in_nested_loop (loop, cb) || L_in_nested_loop (loop, marked_cb))
    return (NULL);

  return (marked_oper);
}

int
L_num_ind_var_to_reassociate (L_Loop * loop, int *loop_cb, int num_cb,
			      L_Operand * ind_var)
{
  int i, j, count, const_flag;
  L_Cb *cb;
  L_Oper *oper;
  L_Operand *other;

  count = 0;
  const_flag = 0;
  for (i = 0; i < num_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
	{
	  if (Set_in (loop->basic_ind_var_op, oper->id))
	    continue;
	  if (!L_is_src_operand (ind_var, oper))
	    continue;
	  for (j = 2; j < L_max_src_operand; j++)
	    {
	      if (L_same_operand (ind_var, oper->src[j]))
		L_punt
		  ("L_num_ind_var_to_reassociate: illegal use of ind_var");
	    }
	  if (L_same_operand (ind_var, oper->src[0]))
	    other = oper->src[1];
	  else
	    other = oper->src[0];
	  if (L_is_int_constant (other))
	    {
	      if (!const_flag)
		{
		  count++;
		  const_flag = 1;
		}
	    }
	  else
	    {
	      count++;
	    }
	}
    }

  return (count);
}

/*
 *      Let reinit handle int constant offsets, so just deal other cases
 *      with reassociation.
 */
void
L_reassociate_ind_var (L_Loop * loop, int *loop_cb, int num_cb,
		       L_Operand * ind_var, L_Oper * last_use,
		       int num_new_ind_var)
{
  int i, j, new_ind_var_index;
  L_Cb *cb, *preheader;
  L_Oper *oper, *new_op;
  L_Operand *other, *increment, **new_ind_var;

  /* allocate new ind vars */
  new_ind_var = (L_Operand **) alloca (sizeof (L_Operand *) * num_new_ind_var);

  for (j = 0; j < num_new_ind_var; j++)
    {
      new_ind_var[j] = L_new_register_operand (++L_fn->max_reg_id,
					       L_native_machine_ctype,
					       L_PTYPE_NULL);
    }

  new_ind_var_index = 0;
  preheader = loop->preheader;
  increment = L_find_basic_induction_increment (ind_var, loop->ind_info);

  /* now go thru and reassoc ind vars */
  for (i = 0; i < num_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
	{
	  if (Set_in (loop->basic_ind_var_op, oper->id))
	    continue;

	  if (L_same_operand (ind_var, oper->src[0]))
	    other = oper->src[1];
	  else if (L_same_operand (ind_var, oper->src[1]))
	    other = oper->src[0];
	  else
	    continue;

	  if (!L_is_int_constant (other))
	    {
	      new_op = L_create_new_op (Lop_ADD);
	      new_op->dest[0] =
		L_copy_operand (new_ind_var[new_ind_var_index]);
	      new_op->src[0] = L_copy_operand (ind_var);
	      new_op->src[1] = L_copy_operand (other);
	      L_insert_oper_after (preheader, preheader->last_op, new_op);
	      L_delete_operand (oper->src[0]);
	      L_delete_operand (oper->src[1]);
	      oper->src[0] = L_copy_operand (new_ind_var[new_ind_var_index]);
	      oper->src[1] = L_new_gen_int_operand (0);
	      new_ind_var_index++;
	    }
	}
    }

  /* now insert increments of new ind vars */
  for (i = 0; i < num_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
	{
	  if (Set_in (loop->basic_ind_var_op, oper->id) &&
	      L_same_operand (oper->dest[0], ind_var))
	    {
	      for (j = 0; j < new_ind_var_index; j++)
		{
		  new_op = L_create_new_op (Lop_ADD);
		  new_op->dest[0] = L_copy_operand (new_ind_var[j]);
		  new_op->src[0] = L_copy_operand (new_ind_var[j]);
		  new_op->src[1] = L_copy_operand (increment);
		  L_insert_oper_after (cb, oper, new_op);
		}
	    }
	}
    }

  /* deallocate new ind vars */
  for (j = 0; j < num_new_ind_var; j++)
    L_delete_operand (new_ind_var[j]);

  return;
}

/*========================================================================*/
/*
 *      Pre/Post increment related functions
 */
/*========================================================================*/

void
L_mark_as_post_increment (L_Oper * mem_op, L_Oper * inc_op)
{
  L_Attr *attr;

  /* error checking */
  if ((mem_op == NULL) || (inc_op == NULL))
    L_punt ("L_mark_as_post_increment: mem_op or inc_op is NULL");
  if (L_find_attr (mem_op->attr, L_POST_INC_MARK))
    L_punt ("L_mark_as_post_increment: op %d already marked as post inc",
	    mem_op->id);
  if (L_find_attr (mem_op->attr, L_PRE_INC_MARK))
    L_punt ("L_mark_as_post_increment: op %d already marked as pre inc",
	    mem_op->id);
  if (L_find_attr (inc_op->attr, L_POST_INC_MARK))
    L_punt ("L_mark_as_post_increment: op %d already marked as post inc",
	    inc_op->id);
  if (L_find_attr (inc_op->attr, L_PRE_INC_MARK))
    L_punt ("L_mark_as_post_increment: op %d already marked as pre inc",
	    inc_op->id);

  attr = L_new_attr (L_POST_INC_MARK, 1);
  L_set_int_attr_field (attr, 0, inc_op->id);
  mem_op->attr = L_concat_attr (mem_op->attr, attr);

  attr = L_new_attr (L_POST_INC_MARK, 1);
  L_set_int_attr_field (attr, 0, mem_op->id);
  inc_op->attr = L_concat_attr (inc_op->attr, attr);
}

void
L_mark_as_pre_increment (L_Oper * mem_op, L_Oper * inc_op)
{
  L_Attr *attr;

  /* error checking */
  if ((mem_op == NULL) || (inc_op == NULL))
    L_punt ("L_mark_as_post_increment: mem_op or inc_op is NULL");
  if (L_find_attr (mem_op->attr, L_POST_INC_MARK))
    L_punt ("L_mark_as_pre_increment: op %d already marked as post inc",
	    mem_op->id);
  if (L_find_attr (mem_op->attr, L_PRE_INC_MARK))
    L_punt ("L_mark_as_pre_increment: op %d already marked as pre inc",
	    mem_op->id);
  if (L_find_attr (inc_op->attr, L_POST_INC_MARK))
    L_punt ("L_mark_as_pre_increment: op %d already marked as post inc",
	    inc_op->id);
  if (L_find_attr (inc_op->attr, L_PRE_INC_MARK))
    L_punt ("L_mark_as_pre_increment: op %d already marked as pre inc",
	    inc_op->id);

  attr = L_new_attr (L_PRE_INC_MARK, 1);
  L_set_int_attr_field (attr, 0, inc_op->id);
  mem_op->attr = L_concat_attr (mem_op->attr, attr);

  attr = L_new_attr (L_PRE_INC_MARK, 1);
  L_set_int_attr_field (attr, 0, mem_op->id);
  inc_op->attr = L_concat_attr (inc_op->attr, attr);
}

void
L_unmark_as_post_increment (L_Oper * mem_op, L_Oper * inc_op)
{
  L_Attr *attr;

  if (mem_op)
    {

      /* error checking */
      if (!L_find_attr (mem_op->attr, L_POST_INC_MARK))
	L_punt ("L_unmark_as_post_increment: op %d NOT marked as post inc",
		mem_op->id);
      if (L_find_attr (mem_op->attr, L_PRE_INC_MARK))
	L_punt ("L_unmark_as_post_increment: op %d already marked as pre inc",
		mem_op->id);

      /* do it */
      attr = L_find_attr (mem_op->attr, L_POST_INC_MARK);
      mem_op->attr = L_delete_attr (mem_op->attr, attr);
    }

  if (inc_op)
    {

      /* error checking */
      if (!L_find_attr (inc_op->attr, L_POST_INC_MARK))
	L_punt ("L_unmark_as_post_increment: op %d NOT marked as post inc",
		inc_op->id);
      if (L_find_attr (inc_op->attr, L_PRE_INC_MARK))
	L_punt ("L_mark_as_post_increment: op %d already marked as pre inc",
		inc_op->id);

      /* do it */
      attr = L_find_attr (inc_op->attr, L_POST_INC_MARK);
      inc_op->attr = L_delete_attr (inc_op->attr, attr);
    }
}

void
L_unmark_as_pre_increment (L_Oper * mem_op, L_Oper * inc_op)
{
  L_Attr *attr;

  if (mem_op)
    {

      /* error checking */
      if (!L_find_attr (mem_op->attr, L_PRE_INC_MARK))
	L_punt ("L_unmark_as_pre_increment: op %d NOT marked as pre inc",
		mem_op->id);
      if (L_find_attr (mem_op->attr, L_POST_INC_MARK))
	L_punt ("L_unmark_as_pre_increment: op %d already marked as post inc",
		mem_op->id);

      /* do it */
      attr = L_find_attr (mem_op->attr, L_PRE_INC_MARK);
      mem_op->attr = L_delete_attr (mem_op->attr, attr);
    }

  if (inc_op)
    {

      /* error checking */
      if (!L_find_attr (inc_op->attr, L_PRE_INC_MARK))
	L_punt ("L_unmark_as_pre_increment: op %d NOT marked as pre inc",
		inc_op->id);
      if (L_find_attr (inc_op->attr, L_POST_INC_MARK))
	L_punt ("L_mark_as_pre_increment: op %d already marked as post inc",
		inc_op->id);

      /* do it */
      attr = L_find_attr (inc_op->attr, L_PRE_INC_MARK);
      inc_op->attr = L_delete_attr (inc_op->attr, attr);
    }
}

void
L_unmark_as_pre_post_increment (L_Oper * oper)
{
  L_Attr *attr_pre, *attr_post;
  L_Oper *other;

  attr_pre = L_find_attr (oper->attr, L_PRE_INC_MARK);
  attr_post = L_find_attr (oper->attr, L_POST_INC_MARK);

  if (attr_pre)
    {
      other = L_oper_hash_tbl_find_oper (L_fn->oper_hash_tbl,
					 (int) attr_pre->field[0]->value.i);
      L_unmark_as_pre_increment (oper, other);
    }
  else if (attr_post)
    {
      other = L_oper_hash_tbl_find_oper (L_fn->oper_hash_tbl,
					 (int) attr_post->field[0]->value.i);
      L_unmark_as_post_increment (oper, other);
    }
}

/*
 *      Breakup post inc ops so its easier to optimize the program, later
 *      will put them back together.
 */
int
L_breakup_pre_post_inc_ops (L_Func * fn)
{
  int change;
  L_Cb *cb;
  L_Oper *oper, *next, *new_op;

  change = 0;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (oper = cb->first_op; oper != NULL; oper = next)
	{
	  next = oper->next_op;

	  /* 02/07/03 REK Adding a check to make sure we don't touch a volatile
	   *              oper. */
	  if (L_EXTRACT_BIT_VAL (oper->flags, L_OPER_VOLATILE))
	    continue;

	  /*
	   *  Format of Lcode post increment load
	   *   d0,d1 ld_post s0,s1,s2
	   *          d0 = MEM(s0+s1)
	   *          d1 = s0 + s2;
	   */
	  if (L_postincrement_load_opcode (oper))
	    {
	      new_op = L_create_new_op (Lop_ADD);
	      L_insert_oper_after (cb, oper, new_op);
	      new_op->dest[0] = oper->dest[1];
	      new_op->src[0] = L_copy_operand (oper->src[0]);
	      new_op->src[1] = oper->src[2];
	      new_op->pred[0] = L_copy_operand (oper->pred[0]);
	      new_op->attr = L_copy_attr (oper->attr);
	      /* convert post inc load to regular load */
	      L_change_opcode (oper, L_base_memory_opcode (oper));
	      oper->dest[1] = NULL;
	      oper->src[2] = NULL;
	      L_mark_as_post_increment (oper, new_op);

#ifdef DEBUG_BREAKUP_PRE_POST_INC
	      fprintf (ERR, "Postincrement load breakup %d  -> (%d %d)\n",
		       oper->id, oper->id, new_op->id);
#endif
	      change++;
	    }

	  /*
	   *  Format of Lcode pre increment load
	   *  d0,d1 ld_pre s0,s1,s2
	   *          d0 = MEM(s0+s1+s2)
	   *          d1 = s0 + s2
	   */
	  else if (L_preincrement_load_opcode (oper))
	    {
	      new_op = L_create_new_op (Lop_ADD);
	      L_insert_oper_before (cb, oper, new_op);
	      new_op->dest[0] = oper->dest[1];
	      new_op->src[0] = L_copy_operand (oper->src[0]);
	      new_op->src[1] = oper->src[2];
	      new_op->pred[0] = L_copy_operand (oper->pred[0]);
	      new_op->attr = L_copy_attr (oper->attr);
	      /* convert post inc load to regular load */
	      L_change_opcode (oper, L_base_memory_opcode (oper));
	      oper->dest[1] = NULL;
	      oper->src[2] = NULL;
	      L_mark_as_pre_increment (oper, new_op);

#ifdef DEBUG_BREAKUP_PRE_POST_INC
	      fprintf (ERR, "Preincrement load breakup %d  -> (%d %d)\n",
		       oper->id, new_op->id, oper->id);
#endif
	      change++;
	    }

	  /*
	   *  Format of Lcode post increment store
	   *   d0 st_post s0,s1,s2,s3
	   *          MEM(s0+s1) = s2
	   *          d0 = s0 + s3;
	   */
	  else if (L_postincrement_store_opcode (oper))
	    {
	      new_op = L_create_new_op (Lop_ADD);
	      L_insert_oper_after (cb, oper, new_op);
	      new_op->dest[0] = oper->dest[0];
	      new_op->src[0] = L_copy_operand (oper->src[0]);
	      new_op->src[1] = oper->src[3];
	      new_op->pred[0] = L_copy_operand (oper->pred[0]);
	      new_op->attr = L_copy_attr (oper->attr);
	      /* convert post inc store to regular store */
	      L_change_opcode (oper, L_base_memory_opcode (oper));
	      oper->dest[0] = NULL;
	      oper->src[3] = NULL;
	      L_mark_as_post_increment (oper, new_op);

#ifdef DEBUG_BREAKUP_PRE_POST_INC
	      fprintf (ERR, "Postincrement store breakup %d  -> (%d %d)\n",
		       oper->id, oper->id, new_op->id);
#endif
	      change++;
	    }

	  /*
	   *  Format of Lcode pre increment store
	   *   d0 st_pre s0,s1,s2,s3
	   *          MEM(s0+s1+s3) = s2
	   *          d0 = s0 + s3;
	   */
	  else if (L_preincrement_store_opcode (oper))
	    {
	      new_op = L_create_new_op (Lop_ADD);
	      L_insert_oper_before (cb, oper, new_op);
	      new_op->dest[0] = oper->dest[0];
	      new_op->src[0] = L_copy_operand (oper->src[0]);
	      new_op->src[1] = oper->src[3];
	      new_op->pred[0] = L_copy_operand (oper->pred[0]);
	      new_op->attr = L_copy_attr (oper->attr);
	      /* convert post inc store to regular store */
	      L_change_opcode (oper, L_base_memory_opcode (oper));
	      oper->dest[0] = NULL;
	      oper->src[3] = NULL;
	      L_mark_as_pre_increment (oper, new_op);

#ifdef DEBUG_BREAKUP_PRE_POST_INC
	      fprintf (ERR, "Preincrement store breakup %d  -> (%d %d)\n",
		       oper->id, new_op->id, oper->id);
#endif
	      change++;
	    }
	}
    }

  return (change);
}

/*
 *      Note this routine just does some rudimentary checks if the post inc is
 *      possible.  It assumes you have used more extensive analysis to mark
 *      increments and memory ops, so the purpose of this routine is just to 
 *      make sure other optimizations either havent eliminated one of the
 *      ops in the pre/post inc combo, or changed one of the ops in such
 *      a way to prevent the combo, like converting one to a move, or changing
 *      one of the operands.  Also, this routine puts the mem/inc into the 
 *      proper format (ie operand positions, opcodes) for combining into
 *      pre/post inc.
 */
int
L_remove_uncombinable_pre_post_inc_ops (L_Func * fn)
{
  int change;
  L_Cb *cb;
  L_Oper *oper, *inc, *mem;
  L_Attr *attr;
  L_Operand *temp;

  change = 0;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
	{

	  if (L_marked_as_post_increment (oper))
	    {
	      attr = L_find_attr (oper->attr, L_POST_INC_MARK);
	      if (L_load_opcode (oper) || L_store_opcode (oper))
		{
		  mem = oper;
		  inc = L_oper_hash_tbl_find_oper (fn->oper_hash_tbl,
						   (int) attr->field[0]->
						   value.i);
		}
	      else if (L_int_add_opcode (oper) || L_int_sub_opcode (oper))
		{
		  mem = L_oper_hash_tbl_find_oper (fn->oper_hash_tbl,
						   (int) attr->field[0]->
						   value.i);
		  inc = oper;
		}
	      else
		{
		  mem = NULL;
		  inc = NULL;
		}

	      /* change inc opcode to add if possible */
	      if (L_int_sub_opcode (inc) && L_is_int_constant (inc->src[1]))
		{
		  temp = inc->src[1];
		  inc->src[1] = L_new_gen_int_operand (-(temp->value.i));
		  L_delete_operand (temp);
		  L_change_opcode (inc, L_corresponding_add (inc));
		}

	      /* swap inc operands if necessary */
	      if (L_int_add_opcode (inc) &&
		  L_same_operand (inc->dest[0], inc->src[1]))
		{
		  temp = inc->src[0];
		  inc->src[0] = inc->src[1];
		  inc->src[1] = temp;
		}

	      /* swap mem address operands if necessary */
	      if ((L_load_opcode (mem) || L_store_opcode (mem)) && (inc) &&
		  (L_same_operand (inc->dest[0], mem->src[1])))
		{
		  temp = mem->src[0];
		  mem->src[0] = mem->src[1];
		  mem->src[1] = temp;
		}

	      /* check if combo is legal */
	      if (		/* check if mem is correct */
		   !mem ||
		   !(L_load_opcode (mem) || L_store_opcode (mem)) ||
		   !(L_marked_as_post_increment (mem)) ||
		   /* check if inc is correct */
		   !inc ||
		   !L_int_add_opcode (inc) ||
		   !(L_marked_as_post_increment (inc)) ||
		   /* check if operands are matched */
		   !L_same_operand (inc->dest[0], inc->src[0]) ||
		   !L_same_operand (inc->dest[0], mem->src[0]) ||
		   /* SAM 6-97: 
		      check if surrounding ops mess up recombination */
		   !L_can_recombine_mem_inc_ops (mem, inc) ||
		   /* check if combo supported in the arch efficiently */
		   !L_can_make_post_inc (mem, inc))
		{

#ifdef DEBUG_BREAKUP_PRE_POST_INC
		  {
		    int mem_id, inc_id;
		    mem_id = (mem != NULL) ? mem->id : -1;
		    inc_id = (inc != NULL) ? inc->id : -1;
		    fprintf (ERR,
			     "No post increment combo possible (%d %d)\n",
			     mem_id, inc_id);
		  }
#endif

		  L_unmark_as_post_increment (mem, inc);
		  change++;
		}
	    }

	  else if (L_marked_as_pre_increment (oper))
	    {
	      attr = L_find_attr (oper->attr, L_PRE_INC_MARK);
	      if (L_load_opcode (oper) || L_store_opcode (oper))
		{
		  mem = oper;
		  inc = L_oper_hash_tbl_find_oper (fn->oper_hash_tbl,
						   (int) attr->field[0]->
						   value.i);
		}
	      else if (L_int_add_opcode (oper) || L_int_sub_opcode (oper))
		{
		  mem = L_oper_hash_tbl_find_oper (fn->oper_hash_tbl,
						   (int) attr->field[0]->
						   value.i);
		  inc = oper;
		}
	      else
		{
		  mem = NULL;
		  inc = NULL;
		}

	      /* change inc opcode to add if possible */
	      if (L_int_sub_opcode (inc) && L_is_int_constant (inc->src[1]))
		{
		  temp = inc->src[1];
		  inc->src[1] = L_new_gen_int_operand (-(temp->value.i));
		  L_delete_operand (temp);
		  L_change_opcode (inc, L_corresponding_add (inc));
		}

	      /* swap inc operands if necessary */
	      if (L_int_add_opcode (inc) &&
		  L_same_operand (inc->dest[0], inc->src[1]))
		{
		  temp = inc->src[0];
		  inc->src[0] = inc->src[1];
		  inc->src[1] = temp;
		}

	      /* swap mem address operands if necessary */
	      if ((L_load_opcode (mem) || L_store_opcode (mem)) &&
		  (L_same_operand (inc->dest[0], mem->src[1])))
		{
		  temp = mem->src[0];
		  mem->src[0] = mem->src[1];
		  mem->src[1] = temp;
		}

	      /* check if combo is legal */
	      if (		/* check if mem is correct */
		   !mem ||
		   !(L_load_opcode (mem) || L_store_opcode (mem)) ||
		   !L_marked_as_pre_increment (mem) ||
		   /* check if inc is correct */
		   !inc ||
		   !L_int_add_opcode (inc) ||
		   !L_marked_as_pre_increment (inc) ||
		   /* check if operands are matched */
		   !L_same_operand (inc->dest[0], inc->src[0]) ||
		   !L_same_operand (inc->dest[0], mem->src[0]) ||
		   /* SAM 6-97: 
		      check if surrounding ops mess up recombination */
		   !L_can_recombine_inc_mem_ops (mem, inc) ||
		   /* check if combo supported in the arch efficiently */
		   !L_can_make_pre_inc (mem, inc))
		{

#ifdef DEBUG_BREAKUP_PRE_POST_INC
		  fprintf (ERR, "No pre increment combo possible (%d %d)\n",
			   inc->id, mem->id);
#endif

		  L_unmark_as_pre_increment (mem, inc);
		  change++;
		}
	    }
	}
    }

  return (change);

}

/*
 *      All post increment ops set up with attributes indicating the
 *      corresponding mem_op/increment.  They are not actually generated
 *      until this is called!!.  Note no checking is done in this routine
 *      to verify the combo is legal, so need to call
 *      L_remove_uncombinable_pre_post_inc_ops() before calling this!!
 */
int
L_generate_pre_post_inc_ops (L_Func * fn)
{
  int opc, change;
  L_Cb *cb, *inc_cb;
  L_Oper *oper, *inc, *mem, *next;
  L_Attr *attr;

  change = 0;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (oper = cb->first_op; oper != NULL; oper = next)
	{
	  next = oper->next_op;

	  /* 02/07/03 REK Adding a check to make sure we don't touch a volatile
	   *              oper. */
	  if (L_EXTRACT_BIT_VAL (oper->flags, L_OPER_VOLATILE))
	    continue;

	  /* handle post increments */
	  if (L_marked_as_post_increment (oper))
	    {
	      attr = L_find_attr (oper->attr, L_POST_INC_MARK);
	      if (L_load_opcode (oper) || L_store_opcode (oper))
		{
		  mem = oper;
		  inc = L_oper_hash_tbl_find_oper (fn->oper_hash_tbl,
						   (int) attr->field[0]->
						   value.i);
		  inc_cb =
		    L_oper_hash_tbl_find_cb (fn->oper_hash_tbl,
					     (int) attr->field[0]->value.i);
		  if (inc == NULL)
		    L_punt ("L_generate_pre_post_increment_ops: "
			    "corresp inc op not found for mem %d", mem->id);
		  if (!(L_int_add_opcode (inc) || L_int_sub_opcode (inc)))
		    L_punt
		      ("L_generate_pre_post_increment_ops: illegal inc %d",
		       inc->id);
		}
	      else if (L_int_add_opcode (oper) || L_int_sub_opcode (oper))
		{
		  mem = L_oper_hash_tbl_find_oper (fn->oper_hash_tbl,
						   (int) attr->field[0]->
						   value.i);
		  inc = oper;
		  inc_cb = cb;
		  if (mem == NULL)
		    L_punt ("L_generate_pre_post_increment_ops: "
			    "corresp mem op not found for inc %d", inc->id);
		  if (!(L_load_opcode (mem) || L_store_opcode (mem)))
		    L_punt
		      ("L_generate_pre_post_increment_ops: illegal mem %d",
		       mem->id);
		}
	      else
		{
		  L_punt ("L_generate_pre_post_increment_ops: "
			  "illegal op %d marked as post increment", oper->id);
		  return (-1);
		}

	      /* do the conversion */
	      L_unmark_as_post_increment (mem, inc);
	      if (L_load_opcode (mem))
		{
		  opc = L_corresponding_postincrement_load (mem);
		  L_change_opcode (mem, opc);
		  mem->dest[1] = L_copy_operand (inc->dest[0]);
		  mem->src[2] = L_copy_operand (inc->src[1]);
		}
	      else
		{
		  opc = L_corresponding_postincrement_store (mem);
		  L_change_opcode (mem, opc);
		  mem->dest[0] = L_copy_operand (inc->dest[0]);
		  mem->src[3] = L_copy_operand (inc->src[1]);
		}
#ifdef DEBUG_BREAKUP_PRE_POST_INC
	      fprintf (ERR, "Postincrement combination (%d %d)  -> %d\n",
		       mem->id, inc->id, mem->id);
#endif
	      if (inc == next)
		next = next->next_op;
	      L_delete_oper (inc_cb, inc);
	      change++;
	    }

	  /* handle pre increments */
	  else if (L_marked_as_pre_increment (oper))
	    {
	      attr = L_find_attr (oper->attr, L_PRE_INC_MARK);
	      if (L_load_opcode (oper) || L_store_opcode (oper))
		{
		  mem = oper;
		  inc = L_oper_hash_tbl_find_oper (fn->oper_hash_tbl,
						   (int) attr->field[0]->
						   value.i);
		  inc_cb =
		    L_oper_hash_tbl_find_cb (fn->oper_hash_tbl,
					     (int) attr->field[0]->value.i);
		  if (inc == NULL)
		    L_punt ("L_generate_pre_post_increment_ops: "
			    "corresp inc op not found for mem %d", mem->id);
		  if (!(L_int_add_opcode (inc) || L_int_sub_opcode (inc)))
		    L_punt ("L_generate_pre_post_increment_ops: "
			    "illegal inc %d", inc->id);
		}
	      else if (L_int_add_opcode (oper) || L_int_sub_opcode (oper))
		{
		  mem = L_oper_hash_tbl_find_oper (fn->oper_hash_tbl,
						   (int) attr->field[0]->
						   value.i);
		  inc = oper;
		  inc_cb = cb;
		  if (mem == NULL)
		    L_punt ("L_generate_pre_post_increment_ops: "
			    "corresp mem op not found for inc %d", inc->id);
		  if (!(L_load_opcode (mem) || L_store_opcode (mem)))
		    L_punt
		      ("L_generate_pre_post_increment_ops: illegal mem %d",
		       mem->id);
		}
	      else
		{
		  L_punt ("L_generate_pre_post_increment_ops: "
			  "illegal op %d marked as post increment", oper->id);
		  return (-1);
		}

	      /* do the conversion */
	      L_unmark_as_pre_increment (mem, inc);
	      if (L_load_opcode (mem))
		{
		  opc = L_corresponding_preincrement_load (mem);
		  L_change_opcode (mem, opc);
		  mem->dest[1] = L_copy_operand (inc->dest[0]);
		  mem->src[2] = L_copy_operand (inc->src[1]);
		}
	      else
		{
		  opc = L_corresponding_preincrement_store (mem);
		  L_change_opcode (mem, opc);
		  mem->dest[0] = L_copy_operand (inc->dest[0]);
		  mem->src[3] = L_copy_operand (inc->src[1]);
		}
#ifdef DEBUG_BREAKUP_PRE_POST_INC
	      fprintf (ERR, "Preincrement combination (%d %d)  -> %d\n",
		       inc->id, mem->id, mem->id);
#endif
	      if (inc == next)
		next = next->next_op;
	      L_delete_oper (inc_cb, inc);
	      change++;
	    }
	}
    }

  return (change);
}

void
L_unmark_all_pre_post_increments_for_ind (L_Loop * loop, int *loop_cb,
					  int num_cb, L_Operand * ind_var)
{
  int i;
  L_Cb *cb;
  L_Oper *oper, *mem;
  L_Attr *attr;

  for (i = 0; i < num_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
	{
	  if (!L_same_operand (oper->dest[0], ind_var))
	    continue;
	  if (L_marked_as_pre_increment (oper))
	    {
	      attr = L_find_attr (oper->attr, L_PRE_INC_MARK);
	      mem = L_oper_hash_tbl_find_oper (L_fn->oper_hash_tbl,
					       (int) attr->field[0]->value.i);
	      L_unmark_as_pre_increment (mem, oper);
	    }
	  else if (L_marked_as_post_increment (oper))
	    {
	      attr = L_find_attr (oper->attr, L_POST_INC_MARK);
	      mem = L_oper_hash_tbl_find_oper (L_fn->oper_hash_tbl,
					       (int) attr->field[0]->value.i);
	      L_unmark_as_post_increment (mem, oper);
	    }
	}
    }
}

void
L_unmark_all_pre_post_increments_for_operand (L_Func * fn,
					      L_Operand * operand)
{
  L_Cb *cb;
  L_Oper *oper, *mem;
  L_Attr *attr;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
	{
	  if (!L_same_operand (oper->dest[0], operand))
	    continue;
	  if (L_marked_as_pre_increment (oper))
	    {
	      attr = L_find_attr (oper->attr, L_PRE_INC_MARK);
	      mem = L_oper_hash_tbl_find_oper (L_fn->oper_hash_tbl,
					       (int) attr->field[0]->value.i);
	      L_unmark_as_pre_increment (mem, oper);
	    }
	  else if (L_marked_as_post_increment (oper))
	    {
	      attr = L_find_attr (oper->attr, L_POST_INC_MARK);
	      mem = L_oper_hash_tbl_find_oper (L_fn->oper_hash_tbl,
					       (int) attr->field[0]->value.i);
	      L_unmark_as_post_increment (mem, oper);
	    }
	}
    }
}

void
L_unmark_all_pre_post_increments (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *oper;
  L_Attr *attr;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
	{
	  attr = L_find_attr (oper->attr, L_PRE_INC_MARK);
	  if (attr)
	    oper->attr = L_delete_attr (oper->attr, attr);
	  attr = L_find_attr (oper->attr, L_POST_INC_MARK);
	  if (attr)
	    oper->attr = L_delete_attr (oper->attr, attr);
	}
    }
}

/*========================================================================*/
/*
 *      Register pressure estimates (seemed like a kool thing to do :P)
 */
/*========================================================================*/

L_Reg_Count L_max_reg_count = { 0, 0, 0, 0 };
L_Reg_Count L_cur_reg_count = { 0, 0, 0, 0 };

int
L_num_reg_used (int ctype)
{
  switch (ctype)
    {
    case L_CTYPE_CHAR:
    case L_CTYPE_UCHAR:
    case L_CTYPE_SHORT:
    case L_CTYPE_USHORT:
    case L_CTYPE_INT:
    case L_CTYPE_UINT:
    case L_CTYPE_LONG:
    case L_CTYPE_ULONG:
    case L_CTYPE_LLONG:
    case L_CTYPE_ULLONG:
    case L_CTYPE_LLLONG:
    case L_CTYPE_ULLLONG:
    case L_CTYPE_POINTER:
      return (L_max_reg_count.int_count);
    case L_CTYPE_FLOAT:
      return (L_max_reg_count.flt_count);
    case L_CTYPE_DOUBLE:
      return (L_max_reg_count.dbl_count);
    case L_CTYPE_PREDICATE:
      return (L_max_reg_count.prd_count);
    default:
      L_punt ("L_num_reg_used: count for ctype %d not computed", ctype);
      return (0);
    }
}

void
L_reset_reg_count (L_Reg_Count * reg_count)
{
  reg_count->int_count = 0;
  reg_count->flt_count = 0;
  reg_count->dbl_count = 0;
  reg_count->prd_count = 0;
}

void
L_increment_reg_count (int reg_id, L_Reg_Count * reg_count)
{
  int ctype;

  ctype = Lopti_ctype_array[reg_id];
  switch (ctype)
    {
    case L_CTYPE_INT:
    case L_CTYPE_LLONG:
      (reg_count->int_count)++;
      break;
    case L_CTYPE_FLOAT:
      (reg_count->flt_count)++;
      break;
    case L_CTYPE_DOUBLE:
      (reg_count->dbl_count)++;
      break;
    case L_CTYPE_PREDICATE:
      (reg_count->prd_count)++;
      break;
    default:
      L_punt ("L_increment_reg_count: illegal ctype %d", ctype);
    }
}

void
L_update_reg_count (L_Reg_Count * max, L_Reg_Count * cur)
{
  if (cur->int_count > max->int_count)
    max->int_count = cur->int_count;
  if (cur->flt_count > max->flt_count)
    max->flt_count = cur->flt_count;
  if (cur->dbl_count > max->dbl_count)
    max->dbl_count = cur->dbl_count;
  if (cur->prd_count > max->prd_count)
    max->prd_count = cur->prd_count;
}

void
L_reset_ctype_array ()
{
  Lopti_ctype_array_size = 0;
  Lopti_ctype_max_reg = 0;
  if (Lopti_ctype_array)
    {
      Lcode_free (Lopti_ctype_array);
      Lopti_ctype_array = NULL;
    }
}

/* 
 *      Fill in an array which maps register numbers to ctype
 */
void
L_setup_ctype_array (L_Func * fn)
{
  int i, id, size;
  L_Cb *cb;
  L_Oper *oper;
  L_Operand *operand;

  /* Assume if max_reg_id hasn't changed, don't need to redo this */
  if (Lopti_ctype_max_reg == fn->max_reg_id)
    return;

  /* check if need to realloc the array */
  if (fn->max_reg_id >= Lopti_ctype_array_size)
    {
      if (Lopti_ctype_array != NULL)
	Lcode_free (Lopti_ctype_array);
      size = C_log2 (fn->max_reg_id + 1);
      size = C_pow2 (size + 1);
      Lopti_ctype_array_size = size;
      Lopti_ctype_array = (int *) Lcode_malloc (sizeof (int) * size);
      Lopti_ctype_max_reg = fn->max_reg_id;
    }

  /* initialization */
  for (i = 0; i < Lopti_ctype_array_size; i++)
    {
      Lopti_ctype_array[i] = L_CTYPE_VOID;
    }

  for (cb = L_fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
	{

	  /* dest operands */
	  for (i = 0; i < L_max_dest_operand; i++)
	    {
	      operand = oper->dest[i];
	      if (L_is_register (operand))
		{
		  id = operand->value.r;
		  if (Lopti_ctype_array[id] == L_CTYPE_VOID)
		    Lopti_ctype_array[id] = L_return_old_ctype (operand);
		  else if (Lopti_ctype_array[id] !=
			   L_return_old_ctype (operand))
		    L_punt ("L_setup_ctype_array: mismatched ctype reg %d "
			    "appears as both ctype %d and %d.",
			    id, Lopti_ctype_array[id],
			    L_return_old_ctype (operand));
		}

	    }

	  /* src operands */
	  for (i = 0; i < L_max_src_operand; i++)
	    {
	      operand = oper->src[i];
	      if (L_is_register (operand))
		{
		  id = operand->value.r;
		  if (Lopti_ctype_array[id] == L_CTYPE_VOID)
		    Lopti_ctype_array[id] = L_return_old_ctype (operand);
		  else if (Lopti_ctype_array[id] !=
			   L_return_old_ctype (operand))
		    L_punt ("L_setup_ctype_array: mismatched ctype reg %d",
			    id);
		}
	    }

	  /* predicate operands */
	  for (i = 0; i < L_max_pred_operand; i++)
	    {
	      operand = oper->pred[i];
	      if (L_is_register (operand))
		{
		  id = operand->value.r;
		  if (Lopti_ctype_array[id] == L_CTYPE_VOID)
		    Lopti_ctype_array[id] = L_return_old_ctype (operand);
		  else if (Lopti_ctype_array[id] !=
			   L_return_old_ctype (operand))
		    L_punt ("L_setup_ctype_array: mismatched ctype reg %d",
			    id);
		}
	    }
	}
    }
}

void
L_estimate_num_live_regs_in_cb (L_Cb * cb)
{
  int j, num, *buf;
  L_Operand *dest;
  L_Oper *oper;
  Set IN;

  L_setup_ctype_array (L_fn);

  buf =
    (int *) Lcode_malloc (sizeof (int) * (L_fn->max_reg_id + L_MAC_LAST + 1));

  L_reset_reg_count (&L_max_reg_count);

  for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
    {
      L_reset_reg_count (&L_cur_reg_count);
      IN = L_get_oper_IN_set (oper);
      num = Set_2array (IN, buf);

      /* count up live in's */
      for (j = 0; j < num; j++)
	{
	  if (L_IS_MAPPED_REG (buf[j]))
	    {
	      L_increment_reg_count (L_UNMAP_REG (buf[j]), &L_cur_reg_count);
	    }
	}

      /* count up defs of oper */
      for (j = 0; j < L_max_dest_operand; j++)
	{
	  dest = oper->dest[j];
	  if (L_is_register (dest))
	    {
	      L_increment_reg_count (dest->value.r, &L_cur_reg_count);
	    }
	}

      L_update_reg_count (&L_max_reg_count, &L_cur_reg_count);
    }

  Lcode_free (buf);
}

void
L_estimate_num_live_regs_in_loop (L_Loop * loop, int *loop_cb, int num_cb)
{
  int i;
  L_Cb *cb;
  L_Reg_Count loop_max_reg_count;

  L_reset_reg_count (&loop_max_reg_count);

  for (i = 0; i < num_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);
      L_estimate_num_live_regs_in_cb (cb);
      L_update_reg_count (&loop_max_reg_count, &L_max_reg_count);
    }

  L_update_reg_count (&L_max_reg_count, &loop_max_reg_count);
}

void
L_estimate_num_live_regs_in_func (L_Func * fn)
{
  L_Cb *cb;
  L_Reg_Count fn_max_reg_count;

  L_reset_reg_count (&fn_max_reg_count);

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      L_estimate_num_live_regs_in_cb (cb);
      L_update_reg_count (&fn_max_reg_count, &L_max_reg_count);
    }

  L_update_reg_count (&L_max_reg_count, &fn_max_reg_count);
}


/*========================================================================*/
/*
 *      Setting function/cb/oper flags
 */
/*========================================================================*/

void
L_set_function_mask_flag (L_Func * fn)
{
  int flag;
  L_Cb *cb;
  L_Oper *oper;

  flag = 0;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
	{
	  if (L_EXTRACT_BIT_VAL (oper->flags, L_OPER_MASK_PE))
	    {
	      flag = 1;
	      break;
	    }
	}
    }

  if (flag)
    fn->flags = L_SET_BIT_FLAG (fn->flags, L_FUNC_MASK_PE);
  else
    fn->flags = L_CLR_BIT_FLAG (fn->flags, L_FUNC_MASK_PE);
}

void
L_mark_superblocks (L_Func * fn)
{
  int fn_flag, br_count;
  L_Cb *cb;
  L_Oper *oper, *last;

  fn_flag = 0;
  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_SUPERBLOCK))
	{
	  fn_flag = 1;
	  continue;
	}
      if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK))
	continue;

      last = cb->last_op;
      if (L_uncond_branch (last) && L_cond_branch_opcode (last->prev_op))
	last = last->prev_op;

      br_count = 0;
      for (oper = last; oper != NULL; oper = oper->prev_op)
	{
	  if (L_general_branch_opcode (oper) ||
	      L_check_branch_opcode (oper))
	    {
	      br_count++;
	    }
	}

      /* normal case, 2 or more branches not counting final jump */
      if (br_count > 1)
	{
	  fn_flag = 1;
	  cb->flags = L_SET_BIT_FLAG (cb->flags, L_CB_SUPERBLOCK);
	}
      /* weird case, branch in middle of cb, but fallthru at bottom */
      else if ((br_count == 1)
	       && (!L_general_branch_opcode (last))
	       && (!L_check_branch_opcode (last)))
	{
	  fn_flag = 1;
	  cb->flags = L_SET_BIT_FLAG (cb->flags, L_CB_SUPERBLOCK);
	}
    }

  if (fn_flag)
    fn->flags = L_SET_BIT_FLAG (fn->flags, L_FUNC_SUPERBLOCK);
  else
    fn->flags = L_CLR_BIT_FLAG (fn->flags, L_FUNC_SUPERBLOCK);
}


/*
 * L_rename_subsequent_uses
 * ----------------------------------------------------------------------
 * Replace any use of dest_reg on or following ren_oper with a use of
 * new_reg, provided that the use is dominated by def_oper, and until
 * a redefinition on an intersecting predicate is encountered.
 */

void
L_rename_subsequent_uses (L_Oper *ren_oper, L_Oper *def_oper, 
			  L_Operand *dest_reg, L_Operand *new_reg)
{  
  L_Oper *ptr;
  int i;

  /* rename all uses of opB->dest to end of cb */
  for (ptr = ren_oper; ptr; ptr = ptr->next_op)
    {
      if (!PG_intersecting_predicates_ops (def_oper, ptr))
	continue;
      
      if (PG_superset_predicate_ops (def_oper, ptr))
	{
	  for (i = 0; i < L_max_src_operand; i++)
	    {
	      if (L_same_operand (dest_reg, ptr->src[i]))
		{
		  L_delete_operand (ptr->src[i]);
		  ptr->src[i] = L_copy_operand (new_reg);
		}
	    }
	}
      
      if (L_is_dest_operand (dest_reg, ptr))
	break;
    }
  return;
}



