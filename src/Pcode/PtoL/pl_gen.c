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
 *	File:	pl_gen.c
 *	Author:	Po-hua Chang, Wen-mei Hwu
 *	Creation Date:	June 1990
 *      Modified by:  Nancy Warter
 *	Modified by:  Roger A. Bringmannn 2/8/93
 *	    Modified to support new Lcode parenthesization format
 *      Revised: Dave Gallagher, Scott Mahlke - June 1994
 *              Build Lcode structure rather than just printing out text file
 *      Revised by: Ben-Chung Cheng - June 1995
 *              Change M_SIZE_INT, M_SIZE_CHAR to H_INT_SIZE, P_CHAR_SIZE
 *              Those should be determined in runtime, not compile time
 *        Chien-Wei Li - 12/2001
 *          change H_CHAR_SIZE to P_CHAR_SIZE.         
 *                 H_punt to P_punt.
\*****************************************************************************/
/* 01/02/03 REK Changing PLI_gen_call to cast integer return values to a long,
 *              thereby sign extending them to 64 bits. */
#include <config.h>
#include <alloca.h>
#include <string.h>
#include "pl_main.h"

/* PLI_gen_addr* functions compute the address of the given expression
 * or determine the register into which it has been allocated.  The return
 * value indicates which of these is the case.  These may return a
 * complex result (i.e. base + offset address)
 * ----------------------------------------------------------------------
 */
static int PLI_gen_addr_var (L_Cb * cb, Expr expr, PL_Ret ret);
static int PLI_gen_addr_indr (L_Cb * cb, Expr expr, PL_Ret ret);
static int PLI_gen_addr_index (L_Cb * cb, Expr expr, PL_Ret ret);
static int PLI_gen_addr_dot (L_Cb * cb, Expr expr, PL_Ret ret);
static int PLI_gen_addr_arrow (L_Cb * cb, Expr expr, PL_Ret ret);

#define PLI_VAR_IN_REGISTER	  0
#define PLI_VAR_IN_MEMORY	  1

/* PLI_gen* functions generate various expression types.  Not all result
 * in a simple PL_Ret (the generation to Lcode may be unfinished).  This
 * is to allow for the use of, for example, load operations which can
 * compute a base+offset address.  Use PLI_simplify on a PL_Ret to
 * complete the generation to Lcode.
 * ----------------------------------------------------------------------
 */

/* THE FOLLOWING GENERATE NO RESULTS */

static void PLI_gen_store (L_Cb * cb, Expr expr, PL_Ret addr,
			   PL_Ret data, Key type);

/* THE FOLLOWING ALWAYS GENERATE SIMPLE RESULTS */

static void PLI_gen_assign (L_Cb * cb, Expr expr, PL_Ret ret);

static L_Oper *PLI_gen_load (L_Cb * cb, Expr expr, PL_Ret addr,
			     PL_Ret ret, Key type);
static void PLI_gen_extract_bit_field (L_Cb * cb, PL_Ret ret, int bit_shift,
				       ITuintmax bit_mask, int unsign);
static void PLI_gen_deposit_bit_field (L_Cb * cb, PL_Ret ret, PL_Ret field,
				       int bit_shift, ITuintmax bit_mask);
static void PLI_gen_cast (L_Cb * cb, Expr expr, PL_Ret ret);

static void PLI_gen_unary (L_Cb * cb, Expr expr, PL_Ret ret);
static void PLI_gen_incr (L_Cb * cb, Expr expr, PL_Ret ret);
static void PLI_gen_arith (L_Cb * cb, Expr expr, PL_Ret ret);
static void PLI_gen_shift (L_Cb * cb, Expr expr, PL_Ret ret);
static void PLI_gen_Aarith (L_Cb * cb, Expr expr, PL_Ret ret);

/* THE FOLLOWING MAY GENERATE COMPLEX RESULTS */

static void PLI_gen_var (L_Cb * cb, Expr expr, PL_Ret ret);
static void PLI_gen_indr (L_Cb * cb, Expr expr, PL_Ret ret);
static void PLI_gen_index (L_Cb * cb, Expr expr, PL_Ret ret);
static void PLI_gen_field (L_Cb * cb, Expr expr, PL_Ret ret);

static void PLI_gen_add (L_Cb * cb, Expr expr, PL_Ret ret);
static void PLI_gen_sub (L_Cb * cb, Expr expr, PL_Ret ret);
static void PLI_gen_logic (L_Cb * cb, Expr expr, PL_Ret ret);
static void PLI_gen_compare (L_Cb * cb, Expr expr, PL_Ret ret);

static void PLI_gen_call (L_Cb * cb, Expr expr, PL_Ret result);


/* PLI_ret* functions operate directly on other PL_Ret's to directly
 * generate a result (they take no Pcode Expr).  These are generally
 * used to construct complex operations.  */

static void PLI_ret_mulC (L_Cb * cb, PL_Ret ret, int n);
static void PLI_ret_add (L_Cb * cb, PL_Ret sum, PL_Ret x, PL_Ret y);
static void PLI_ret_reset (PL_Ret ret);

static int PL_lcode_compare_completer (int opc);
static void PL_assign_to (L_Cb * cb, Expr lhs_expr, PL_Ret rhs, PL_Ret ret);

/* ====================================================================== */

/* REH 09/05/94 - global to aid alloc removal */
static PL_Operand PLI_func_return_struct = NULL;

static void
PL_assert_not_AF(Expr expr)
{
  Key type = PST_ExprType(PL_symtab, expr);
  if (!PST_IsBaseType(PL_symtab, type))
    {
      if (PST_IsArrayType(PL_symtab, type))
	{
	  /*
	   *  It is not wise, but some people actually
	   *  define some parameter to be char XXX[];
	   *  and later say  XXX++
	   */
	  if (PST_GetTypeArraySize(PL_symtab, type))
	    P_punt ("x=y: x cannot be an array");
	}
      else if (PST_IsFunctionType(PL_symtab, type))
	{
	  P_punt ("x=y: x cannot be a function");
	}
    }
}


/* WARNING: callers rely on return being 1 only when the access
 * is improved to be a native width!
 */
static int
PLI_improve_bitfield_access (L_Cb *cb, PL_Ret addr, Type *ptype,
			     int *pbit_shift, int *pbit_length,
			     ITuintmax *pbit_mask)
{
  Type type = *ptype;
  enum _BasicType btu = PST_IsUnsignedType (PL_symtab, type) ? BT_UNSIGNED : 0;
  int bit_shift = *pbit_shift, bit_length = *pbit_length,
    addr_adj;
  ITuintmax bit_mask = *pbit_mask;

  if (!PL_gen_improved_bitfields)
    return 0;

  if (bit_length == P_CHAR_SIZE)
    type = PST_FindBasicType (PL_symtab, btu | BT_CHAR);
  else if (bit_length == P_SHORT_SIZE)
    type = PST_FindBasicType (PL_symtab, btu | BT_SHORT);
  else if (bit_length == P_INT_SIZE)
    type = PST_FindBasicType (PL_symtab, btu | BT_INT);
  else
    return 0;

  if (bit_shift % bit_length)
    return 0;

  /* Improve the access */

  addr_adj = bit_shift / P_CHAR_SIZE;
  bit_shift -= addr_adj * P_CHAR_SIZE;
  bit_mask >>= addr_adj * P_CHAR_SIZE;
  
  /* Adjust address */

  if (addr_adj)
    {
      if (addr->type == PL_RET_ADD && (addr->op2.type == PL_I))
	{
	  addr->op2.value.i += addr_adj;
	}
      else if (addr->type == PL_RET_SIMPLE)
	{
	  addr->type = PL_RET_ADD;
	  PL_new_int_const (&(addr->op2), addr_adj, M_TYPE_INT, 0);
	}
      else 
	{
	  PLI_simplify (cb, addr);
	  addr->type = PL_RET_ADD;
	  PL_new_int_const (&(addr->op2), addr_adj, M_TYPE_INT, 0);
	}
    }

  *ptype = type;
  *pbit_length = bit_length;
  *pbit_shift = bit_shift;

  /* return 1 if the bit_length is the size of the access */

  return 1;
}


static L_Oper *
PLI_gen_load_bitfield (L_Cb * cb, Expr expr,
				 PL_Ret addr, PL_Ret data, Key st_type)
{
  Field field;
  Key type, st_base_type;
  int bit_field_length, bit_field_shift, improved;
  ITuintmax bit_field_mask;
  L_Oper *ld;

  /* st_type may be a struct, pointer to a struct, etc.
     The address and data should have been generated 
     appropriately, so get the struct type for field
     info */
  st_base_type = PST_GetBaseType(PL_symtab, st_type);

  /* Determine bit field type from the expression type */
  type = PST_GetBaseType(PL_symtab, 
			 PST_ExprType(PL_symtab, expr));
  
  field = PL_key_get_field(st_base_type, expr->value.string);
  assert(field->is_bit_field);
  bit_field_length = PL_bit_field_info(field, &bit_field_mask, 
				       &bit_field_shift);

  improved = PLI_improve_bitfield_access (cb, addr, &type, 
					  &bit_field_shift,
					  &bit_field_length, &bit_field_mask);

  ld = PLI_gen_load (cb, expr, addr, data, type);

  if (!improved)
    PLI_gen_extract_bit_field (cb, data,
			       bit_field_shift, bit_field_mask,
			       PST_IsUnsignedType(PL_symtab, type));

  return ld;
}


static void
PLI_gen_store_bitfield (L_Cb * cb, Expr expr,
			PL_Ret addr, PL_Ret data, Key st_type)
{
  Field field;
  _PL_Ret tmpdata;
  Key type;
  Key st_base_type;
  int bit_field_length, bit_field_shift, improved;
  ITuintmax bit_field_mask;
  
  PLI_ret_reset (&tmpdata);
  
  /* st_type may be a struct, pointer to a struct, etc.
     The address and data should have been generated 
     appropriately, so get the struct type for field
     info */
  st_base_type = PST_GetBaseType(PL_symtab, st_type);

  /* Determine bit field type from the expression type */
  type = PST_GetBaseType(PL_symtab, 
			 PST_ExprType(PL_symtab, expr));

  field = PL_key_get_field(st_base_type, expr->value.string);
  assert(field->is_bit_field);
  bit_field_length = PL_bit_field_info(field, &bit_field_mask, 
				       &bit_field_shift);

  improved = PLI_improve_bitfield_access (cb, addr, &type, 
					  &bit_field_shift,
					  &bit_field_length, &bit_field_mask);

  if (improved)
    {
      /* If improved to a normal-width access, no need to load/dep */
      PLI_gen_store (cb, expr, addr, data, type);
    }
  else
    {
      PLI_gen_load (cb, expr, addr, &tmpdata, type);
      PLI_gen_deposit_bit_field (cb, &tmpdata, data,
				 bit_field_shift, bit_field_mask);
      PLI_gen_store (cb, expr, addr, &tmpdata, type);
    }

  return;
}


/*
 * PLI_ret_mulC
 * ----------------------------------------------------------------------
 * Multiply a PL_Ret by a constant
 */
static void
PLI_ret_mulC (L_Cb * cb, PL_Ret ret, int n)
{
  PL_Operand op;
  int rtype;

  PLI_simplify (cb, ret);

  op = &(ret->op1);

  rtype = default_unary_promotion (op->data_type);

  if (op->type == PL_I)
    {
      op->value.i *= n;
    }
  else if (op->type == PL_F)
    {
      op->value.f *= n;
    }
  else if (op->type == PL_F2)
    {
      op->value.f2 *= n;
    }
  else
    {
      _PL_Operand dest, scale;

      PL_new_register (&dest, PL_next_reg_id (), rtype, op->unsign);
      PL_new_int (&scale, n, 0);
      PL_gen_mul (cb, &dest, op, &scale, NULL, 0, 0);
      ret->op1 = dest;
      ret->type = PL_RET_SIMPLE;
    }
}


/* assume unsigned add/sub */
static void
PLI_ret_add (L_Cb * cb, PL_Ret sum, PL_Ret x, PL_Ret y)
{
  int unsign;

  unsign = x->op1.unsign || y->op1.unsign;
  PLI_ret_reset (sum);

  if ((x->type == PL_RET_SIMPLE) && (x->op1.type == PL_I))
    {
      if ((y->type == PL_RET_SIMPLE) && (y->op1.type == PL_I))
	{
	  PLI_ret_reset (sum);
	  sum->type = PL_RET_SIMPLE;

	  PL_new_int (&(sum->op1), x->op1.value.i + y->op1.value.i, unsign);
	}
      else if ((y->type == PL_RET_ADD) && (y->op2.type == PL_I))
	{
	  sum->type = PL_RET_ADD;
	  sum->op1 = y->op1;

	  PL_new_int (&(sum->op2), x->op1.value.i + y->op2.value.i, unsign);
	}
      else if ((y->type == PL_RET_ADD) && (y->op1.type == PL_I))
	{
	  sum->type = PL_RET_ADD;
	  sum->op1 = y->op2;

	  PL_new_int (&(sum->op2), x->op1.value.i + y->op1.value.i, unsign);
	}
      else
	{
	  PLI_simplify (cb, x);
	  PLI_simplify (cb, y);
	  sum->type = PL_RET_ADD;
	  sum->op1 = x->op1;
	  sum->op2 = y->op1;
	}
    }
  else if ((y->type == PL_RET_SIMPLE) && (y->op1.type == PL_I))
    {
      if ((x->type == PL_RET_ADD) && (x->op2.type == PL_I))
	{
	  sum->type = PL_RET_ADD;
	  sum->op1 = x->op1;

	  PL_new_int (&(sum->op2), y->op1.value.i + x->op2.value.i, unsign);
	}
      else if ((x->type == PL_RET_ADD) && (x->op1.type == PL_I))
	{
	  sum->type = PL_RET_ADD;
	  sum->op1 = x->op2;

	  PL_new_int (&(sum->op2), y->op1.value.i + x->op1.value.i, unsign);
	}
      else
	{
	  PLI_simplify (cb, x);
	  PLI_simplify (cb, y);
	  sum->type = PL_RET_ADD;
	  sum->op1 = x->op1;
	  sum->op2 = y->op1;
	}
    }
  else
    {
      PLI_simplify (cb, x);
      PLI_simplify (cb, y);
      sum->type = PL_RET_ADD;
      sum->op1 = x->op1;
      sum->op2 = y->op1;
    }
  return;
}


static void
PLI_ret_reset (PL_Ret ret)
{
  if (!ret)
    P_punt ("PLI_ret_reset: nil argument");

  ret->type = PL_RET_SIMPLE;
  ret->op1.type = PL_I;
  ret->op1.data_type = PL_native_int_reg_mtype;
  ret->op1.value.i = 0;
  ret->op2.type = PL_I;
  ret->op2.data_type = PL_native_int_reg_mtype;
  ret->op2.value.i = 0;
  ret->sreg = 0;
  ret->ereg = 0;
}


static int
PL_lcode_compare_completer (int opc)
{
  int com;
  switch (opc)
    {
    case PL_RET_EQ:
      com = Lcmp_COM_EQ;
      break;
    case PL_RET_NE:
      com = Lcmp_COM_NE;
      break;
    case PL_RET_GT:
    case PL_RET_GT_U:
      com = Lcmp_COM_GT;
      break;
    case PL_RET_GE:
    case PL_RET_GE_U:
      com = Lcmp_COM_GE;
      break;
    case PL_RET_LT:
    case PL_RET_LT_U:
      com = Lcmp_COM_LT;
      break;
    case PL_RET_LE:
    case PL_RET_LE_U:
      com = Lcmp_COM_LE;
      break;
    default:
      com = 0;
    }

  return com;
}


/*
 * PLI_simplify (L_cb *cb, PL_Ret ret)
 * ----------------------------------------------------------------------
 * Simplify a flat expression into a single operand, contined in a
 * SIMPLE PL_Ret.
 */
void
PLI_simplify (L_Cb * cb, PL_Ret ret)
{
  _PL_Operand dest;
  PL_Operand src1, src2;
  _M_Type mtype;
  int com;

  if (!ret)
    P_punt ("PLI_simplify: ret is NULL");

  src1 = &(ret->op1);
  src2 = &(ret->op2);

  iso_default_promotion (&mtype,
			 src1->data_type, src1->unsign,
			 src2->data_type, src2->unsign);

  switch (ret->type)
    {
    case PL_RET_NONE:
      PLI_ret_reset (ret);	/* initialize to 0 */
      break;
    case PL_RET_SIMPLE:	/* already simple */
      break;

      /* binary arithmetic operators
       * ------------------------------------------------------------
       * default promotion
       */

    case PL_RET_ADD:
      if ((src2->type == PL_I) && (src2->value.i == 0))
	{
	  ;			/* Result already simple */
	}
      else if ((src1->type == PL_I) && (src1->value.i == 0))
	{
	  *src1 = *src2;
	}
      else if ((src1->type == PL_I) && (src2->type == PL_I))
	{
	  PL_new_int_const (src1, src1->value.i + src2->value.i,
			    mtype.type, mtype.unsign);
	}
      else
	{
	  PL_new_register (&dest, PL_next_reg_id (), mtype.type,
			   mtype.unsign);
	  PL_gen_add (cb, &dest, src1, src2, 1, 0);
	  *src1 = dest;
	}
      break;

    case PL_RET_SUB:
      if ((src2->type == PL_I) && (src2->value.i == 0))
	{
	  ;			/* Result already simple */
	}
      else if ((src1->type == PL_I) && (src2->type == PL_I))
	{
	  PL_new_int_const (src1, src1->value.i - src2->value.i,
			    mtype.type, mtype.unsign);
	}
      else
	{
	  PL_new_register (&dest, PL_next_reg_id (), mtype.type,
			   mtype.unsign);
	  PL_gen_sub (cb, &dest, src1, src2, 1, 0);
	  *src1 = dest;
	}
      break;

      /* binary logical operators
       * ------------------------------------------------------------
       * default promotion
       */

    case PL_RET_AND:
    case PL_RET_OR:
    case PL_RET_XOR:
      PL_new_register (&dest, PL_next_reg_id (), mtype.type, mtype.unsign);
      PL_gen_logic (cb, ret->type, &dest, src1, src2, NULL);
      *src1 = dest;
      break;

      /* relational operators
       * ------------------------------------------------------------
       * ALWAYS have a signed int result 
       */

    case PL_RET_EQ:
    case PL_RET_NE:
      PL_new_register (&dest, PL_next_reg_id (), M_TYPE_INT, 0);
      com = PL_lcode_compare_completer (ret->type);
      PL_gen_rcmp (cb, &dest, src1, src2, com, 0, NULL);
      *src1 = dest;
      break;

    case PL_RET_GT:
    case PL_RET_GE:
    case PL_RET_LT:
    case PL_RET_LE:
      PL_new_register (&dest, PL_next_reg_id (), M_TYPE_INT, 0);
      com = PL_lcode_compare_completer (ret->type);
      PL_gen_rcmp (cb, &dest, src1, src2, com, 0, NULL);
      if (mtype.unsign)
	P_warn ("PLI_simplify: questionable ISO unsign conformance (op %d)",
		cb->last_op->id);
      *src1 = dest;
      break;

    case PL_RET_GT_U:
    case PL_RET_GE_U:
    case PL_RET_LT_U:
    case PL_RET_LE_U:
      PL_new_register (&dest, PL_next_reg_id (), M_TYPE_INT, 0);
      com = PL_lcode_compare_completer (ret->type);
      PL_gen_rcmp (cb, &dest, src1, src2, com, 1, NULL);
      if (!mtype.unsign)
	P_warn ("PLI_simplify: questionable ISO unsign conformance (op %d)",
		cb->last_op->id);
      *src1 = dest;
      break;

    case PL_RET_LIST:
      *src1 = ret->oplist[0];
      break;
    default:
      P_punt ("PLI_simplify: illegal type");
    }

  if(ret->type != PL_RET_LIST)
    ret->type = PL_RET_SIMPLE;

  PL_new_int (src2, 0, 0);
  return;
}


/*
 *	if the returned type is a struct/union/array/function,
 *	then we consider it to be an address. PLI_gen_data()
 *	will not try to load the value, but returns its
 *	starting address.
 */

static int
PLI_is_addr_expr (Expr expr)
{
  Key type = PST_ExprType(PL_symtab, expr);

  if (!PST_IsBaseType(PL_symtab, type))
    return (PST_IsArrayType(PL_symtab, type) || 
      PST_IsFunctionType(PL_symtab, type));
  else
    return PL_is_aggr_type (type, NULL);
}


static int
PLI_gen_vararg_addr (L_Cb * cb, Expr expr, PL_Ret ret)
{
  int in_reg, reg_id, mem_offset;
  char *mem_base_macro;
  int sreg, ereg;
  _M_Type type;

  if (PL_find_param_var (expr->value.var.key,
			 &type, &in_reg, &reg_id, &mem_base_macro,
			 &mem_offset, &sreg, &ereg))
    {
      ret->type = PL_RET_ADD;
      PL_new_macro (&(ret->op1), mem_base_macro, M_TYPE_POINTER, 0);
      PL_new_int (&(ret->op2), mem_offset, 0);
    }
  else
    {
      P_punt ("PLI_gen_vararg_addr: target not found in params");
    }
  return PLI_VAR_IN_MEMORY;
}


/* PLI_gen_addr
 * ----------------------------------------------------------------------
 * Generate the address of an expression.  Returns PLI_VAR_IN_MEMORY if
 * returing the address or PLI_VAR_IN_REGISTER if the expression is
 * available in a register.  "ret" is updated accordingly, and contains
 * either the address of the expression or the register containing
 * the expression.
 */
int
PLI_gen_addr (L_Cb * cb, Expr expr, PL_Ret ret)
{
  int opcode = expr->opcode;

  PLI_ret_reset (ret);

  switch (opcode)
    {
    case OP_var:
      return PLI_gen_addr_var (cb, expr, ret);
    case OP_indr:
      return PLI_gen_addr_indr (cb, expr, ret);
    case OP_index:
      return PLI_gen_addr_index (cb, expr, ret);
    case OP_dot:
      return PLI_gen_addr_dot (cb, expr, ret);
    case OP_arrow:
      return PLI_gen_addr_arrow (cb, expr, ret);
    case OP_cast:
      /* 
       * BCC - 2/26/96 Consider ((int *)p)++; where p is a char
       * pointer. All we have to do here is generate the address the
       * address of p. As for the size of increment (1 or 4), that's
       * PLI_gen_incr's business.  */
      return (PLI_gen_addr (cb, expr->operands, ret));
    case OP_addr:
      P_punt ("PLI_gen_addr: It is illegal to take address of &");
      break;
    default:
      if (PLI_is_addr_expr (expr))
	{
	  PLI_gen_data (cb, expr, ret);
	  return PLI_VAR_IN_MEMORY;
	}
    }

  P_punt ("PLI_gen_addr: incomplete coverage");
  return PLI_VAR_IN_REGISTER;
}


static int
PLI_gen_addr_var (L_Cb * cb, Expr expr, PL_Ret ret)
{
  _M_Type type, mtype;
  int in_reg, reg_id, mem_offset;
  char *mem_base_macro;
  int sreg, ereg;
  char *vname = expr->value.var.name;
  Key vkey = expr->value.var.key;

  if (PST_IsFunctionTypeExpr(PL_symtab, expr))
    {
      ret->type = PL_RET_SIMPLE;
      PL_new_label (&(ret->op1), PL_fmt_var_name(vname, vkey), 1);
      PL_new_int (&(ret->op2), 0, 0);
      return PLI_VAR_IN_MEMORY;
    }
  else if (PST_GetFuncDclEntry(PL_symtab, vkey))
    {
      P_warn("\nBIG BIG ERROR: ALL FUNCTIONS SHOULD BE OF FUNCTION TYPE\n");
      ret->type = PL_RET_SIMPLE;
      PL_new_label (&(ret->op1), PL_fmt_var_name(vname, vkey), 1);
      PL_new_int (&(ret->op2), 0, 0);
      return PLI_VAR_IN_MEMORY;      
    }
  else if (PL_find_local_var (vkey, &type, &in_reg, &reg_id, &mem_base_macro,
			      &mem_offset))
    {
      mtype = type;

      if (in_reg)
	{
	  ret->type = PL_RET_SIMPLE;
	  PL_new_register (&(ret->op1), reg_id, mtype.type, mtype.unsign);
	  PL_new_int (&(ret->op2), 0, 0);
	  return PLI_VAR_IN_REGISTER;
	}
      else
	{
	  ret->type = PL_RET_ADD;
	  PL_new_macro (&(ret->op1), mem_base_macro, M_TYPE_POINTER, 0);
	  PL_new_int (&(ret->op2), mem_offset, 0);
	  return PLI_VAR_IN_MEMORY;
	}
    }
  else if (PL_find_param_var (vkey, &type, &in_reg, &reg_id, &mem_base_macro,
			      &mem_offset, &sreg, &ereg))
    {
#if 0
      fprintf(stderr,"PARAM: %s\n",vname);
#endif
      mtype = type;

      if (in_reg)
	{
	  ret->type = PL_RET_SIMPLE;
	  PL_new_register (&(ret->op1), reg_id, mtype.type, mtype.unsign);

	  PL_new_int (&(ret->op2), 0, 0);
	  ret->sreg = sreg;
	  ret->ereg = ereg;
	  return PLI_VAR_IN_REGISTER;
	}
      else
	{
	  ret->type = PL_RET_ADD;
	  PL_new_macro (&(ret->op1), mem_base_macro, M_TYPE_POINTER, 0);
	  PL_new_int (&(ret->op2), mem_offset, 0);
	  return PLI_VAR_IN_MEMORY;
	}
    }
  else if (!strncmp (vname, "__impact_builtin", 16))
    {
      L_warn ("Generating an __impact_builtin");

      if (!strcmp (vname, "__impact_builtin_mac_ip"))
	{
	  ret->type = PL_RET_SIMPLE;
	  PL_new_macro (&(ret->op1), strdup ("$IP"), M_TYPE_POINTER, 0);
	  PL_new_int (&(ret->op2), 0, 0);
	}

      P_punt ("PLI_gen_addr_var: unidentified __impact_builtin");
      return PLI_VAR_IN_REGISTER;
    }
  else
    {
      int is_func = PL_is_func_var(vkey);
      assert(is_func == 0);

      ret->type = PL_RET_SIMPLE;
      PL_new_label (&(ret->op1), PL_fmt_var_name(vname, vkey), 0);
      PL_new_int (&(ret->op2), 0, 0);
      return PLI_VAR_IN_MEMORY;
    }
}


static int
PLI_gen_addr_indr (L_Cb * cb, Expr expr, PL_Ret ret)
{
  Expr op1;

  if (!(op1 = expr->operands))
    P_punt ("PLI_gen_addr_indr: missing operand");

  PLI_gen_data (cb, op1, ret);

  return PLI_VAR_IN_MEMORY;
}


static int
PLI_gen_addr_index (L_Cb * cb, Expr expr, PL_Ret ret)
{
  Expr op1, op2;
  int typesize;
  _PL_Ret x, y;

  op1 = expr->operands;
  op2 = expr->operands->sibling;
  typesize = PL_key_get_size (PST_ExprType(PL_symtab, expr));

  PLI_gen_data (cb, op1, &x);
  PLI_gen_data (cb, op2, &y);

  if (typesize != 1)
    PLI_ret_mulC (cb, &y, typesize);	/* y * typesize */

  PLI_ret_add (cb, ret, &x, &y);	/* x + (y * typesize) */

  return PLI_VAR_IN_MEMORY;
}


static int
PLI_gen_addr_dot (L_Cb * cb, Expr expr, PL_Ret ret)
{
  Expr op1;
  long offset;
  Field field;
  _PL_Ret x, y;
  Key type;

  op1 = expr->operands;
  type = PST_ExprType(PL_symtab, op1);

  if (PL_is_link_multi (type, NULL))
    return PLI_gen_addr (cb, op1, ret);
   
  if (!PST_IsStructureType(PL_symtab, type))
    P_punt ("PLI_gen_addr_dot: LHS is not a structure");

  PLI_gen_addr (cb, op1, &x);

  field = PL_key_get_field(type, expr->value.string);
  offset = PST_GetFieldContainerOffset (PL_symtab, field);

  PLI_ret_reset (&y);
  y.type = PL_RET_SIMPLE;
  PL_new_int (&(y.op1), offset, 0);
  PLI_ret_add (cb, ret, &x, &y);

  return PLI_VAR_IN_MEMORY;
}


static int
PLI_gen_addr_arrow (L_Cb * cb, Expr expr, PL_Ret ret)
{
  Expr op1;
  Key type;
  long offset;
  Field field;
  _PL_Ret x, y;

  op1 = expr->operands;
  type = PST_ExprType(PL_symtab, op1);

  if (!PST_IsPointerType(PL_symtab, type))
    P_punt ("PLI_gen_addr_arrow: LHS is not a pointer");

  /* Check if this is a pointer to a structure */
  type = PST_GetTypeType(PL_symtab, type);
  assert(PST_IsStructureType(PL_symtab, type));

  PLI_gen_data (cb, op1, &x);

  field = PL_key_get_field(type, expr->value.string);
  offset = PST_GetFieldContainerOffset (PL_symtab, field);

  PLI_ret_reset (&y);
  y.type = PL_RET_SIMPLE;
  PL_new_int (&(y.op1), offset, 0);
  PLI_ret_add (cb, ret, &x, &y);

  return PLI_VAR_IN_MEMORY;
}


/*
 * PLI_gen_data
 * ----------------------------------------------------------------------
 * Generate a Pcode Expr, filling in a PL_Ret structure.  Any necessary
 * Lcode will be generated; however the PL_Ret need not be simple at
 * the conclusion.  Final simplification, if necessary, may be performed
 * by the consumer.  Expression must have been ReduceExpr'ed.
 */
void
PLI_gen_data (L_Cb * cb, Expr expr, PL_Ret ret)
{
  int opcode;
  Expr op1;
  Key type;

  PLI_ret_reset (ret);
  opcode = expr->opcode;
  switch (opcode)
    {
    case OP_var:
      PLI_gen_var (cb, expr, ret);
      break;

    case OP_enum:		/* 2-6-1991 */
      /* CWL - 11/19/00
       * pcode should have "inlined" enum's.
       */

      P_punt ("hl_gen.c: PLI_gen_data: OP_enum");
      break;

    case OP_char:
      PL_new_char (&(ret->op1), P_IntegralExprValue (expr),
		   PST_IsUnsignedTypeExpr(PL_symtab, expr));
      ret->type = PL_RET_SIMPLE;
      break;

    case OP_int:		/* CWL - 11/27/00 */
      {
	int type = PST_GetTypeBasicType(PL_symtab, 
					PST_ExprType(PL_symtab, expr));
	int unsign = PST_IsUnsignedTypeExpr(PL_symtab, expr);
	ITintmax val = P_IntegralExprValue (expr);

	if (type & BT_LONGLONG)
	  PL_new_llong (&(ret->op1), val, unsign);
	else if (type & BT_LONG)
	  PL_new_long (&(ret->op1), val, unsign);
	else if (type & BT_INT)
	  PL_new_int (&(ret->op1), val, unsign);
	else if (type & BT_SHORT)
	  PL_new_short (&(ret->op1), val, unsign);
	else if (type & BT_CHAR)
	  PL_new_char (&(ret->op1), val, unsign);
	else if (type & BT_POINTER)
	  PL_new_pointer(&(ret->op1), val);
	else
	  assert(0);

	ret->type = PL_RET_SIMPLE;
      }
      break;

    case OP_float:
      if (!PST_IsFloatTypeExpr (PL_symtab, expr))
	P_punt ("OP_float type mismatch");
      PL_new_float (&(ret->op1), expr->value.real);
      ret->type = PL_RET_SIMPLE;
      break;

    case OP_double:
      if (!PST_IsDoubleTypeExpr (PL_symtab, expr)) 
	P_punt ("OP_double type mismatch");
      PL_new_double (&(ret->op1), expr->value.real);
      ret->type = PL_RET_SIMPLE;
      break;

    case OP_expr_size:
      op1 = expr->operands;
      type = op1->type;

      PL_new_int (&(ret->op1), PL_key_get_size (type), 0);
      ret->type = PL_RET_SIMPLE;
      break;

    case OP_type_size:
      PL_new_int (&(ret->op1), PL_key_get_size (expr->value.type), 0);
      ret->type = PL_RET_SIMPLE;
      break;

    case OP_string:
      PL_new_string (&(ret->op1), expr->value.string);
      ret->type = PL_RET_SIMPLE;
      break;

    case OP_dot:
    case OP_arrow:
      PLI_gen_field (cb, expr, ret);
      break;

    case OP_cast:
      PLI_gen_cast (cb, expr, ret);
      break;

    case OP_quest:
    case OP_disj:
    case OP_conj:
      P_punt ("Pcode must have been flattened before generating Lcode");
      break;

    case OP_compexpr:
      {
	Expr e = expr->operands;

	do
	  PLI_gen_data (cb, e, ret);
	while ((e = e->next));
      }
      break;

    case OP_assign:
      PLI_gen_assign (cb, expr, ret);
      break;

    case OP_or:
    case OP_xor:
    case OP_and:
      PLI_gen_logic (cb, expr, ret);
      break;

    case OP_eq:
    case OP_ne:
    case OP_lt:
    case OP_le:
    case OP_ge:
    case OP_gt:
      PLI_gen_compare (cb, expr, ret);
      break;

    case OP_rshft:
    case OP_lshft:
      PLI_gen_shift (cb, expr, ret);
      break;

    case OP_add:
      PLI_gen_add (cb, expr, ret);
      break;

    case OP_sub:
      PLI_gen_sub (cb, expr, ret);
      break;

    case OP_mul:
    case OP_div:
    case OP_mod:
      PLI_gen_arith (cb, expr, ret);
      break;

    case OP_neg:
    case OP_not:
    case OP_inv:
      PLI_gen_unary (cb, expr, ret);
      break;

    case OP_preinc:
    case OP_predec:
    case OP_postinc:
    case OP_postdec:
      PLI_gen_incr (cb, expr, ret);
      break;

    case OP_Aadd:
    case OP_Asub:
    case OP_Amul:
    case OP_Adiv:
    case OP_Amod:
    case OP_Arshft:
    case OP_Alshft:
    case OP_Aand:
    case OP_Aor:
    case OP_Axor:
      PLI_gen_Aarith (cb, expr, ret);
      break;

    case OP_indr:
      PLI_gen_indr (cb, expr, ret);
      break;

    case OP_addr:
      op1 = expr->operands;
      if (PLI_gen_addr (cb, op1, ret) == PLI_VAR_IN_REGISTER)
	P_punt ("PLI_gen_data: cannot compute memory address of a register");
      break;

    case OP_index:
      PLI_gen_index (cb, expr, ret);
      break;

    case OP_call:
      /* CWL - 12/21/00
       * Add call_info pragma.
       */

      {
	Dyn_str_t *pragmastr = NULL;
	Dyn_str_t *temp_str = NULL;
	char *new_str;	

	pragmastr = PL_dstr_new(256);
	temp_str = PL_dstr_new(256);	

	/* BCC 5/16/98 */
	PL_dstr_sprintf (pragmastr, "call_info\\$");
	PL_encode_type_name (PST_ExprType(PL_symtab, expr), temp_str);
	PL_dstr_strcat (pragmastr, temp_str->str);
	for (op1 = expr->operands->sibling; op1; op1 = op1->next)
	  {
	    PL_dstr_strcat (pragmastr, "%");
	    PL_encode_type_name (PST_ExprType(PL_symtab, op1), temp_str);
	    PL_dstr_strcat (pragmastr, temp_str->str);
	  }
	new_str = strdup (pragmastr->str);
	expr->pragma = P_AppendPragmaNext (expr->pragma, 
					   P_NewPragmaWithSpecExpr(new_str, 
								   NULL));

	PL_dstr_free(pragmastr);
	PL_dstr_free(temp_str);
      }

      PLI_gen_call (cb, expr, ret);
      break;

    default:
      P_punt ("PLI_gen_data: illegal opcode");
    }

  return;
}


static void
PLI_gen_var (L_Cb * cb, Expr expr, PL_Ret ret)
{
  _PL_Ret addr;

  PLI_ret_reset (&addr);

  if (PLI_gen_addr_var (cb, expr, &addr) == PLI_VAR_IN_REGISTER)
    {
      Key type;
      int mtype_type, is_unsigned;

      type = PST_ExprType(PL_symtab, expr);
      mtype_type = PL_key_get_mtype (type);      
      is_unsigned = PST_IsUnsignedType(PL_symtab, type);

      if (PST_IsIntegralTypeExpr (PL_symtab, expr))
	{
	  _PL_Operand dest;
	  enum _BasicType bt = PST_GetTypeBasicType(PL_symtab, type);

	  if (bt & BT_CHAR)
	    {
	      PL_new_register (&dest, PL_next_reg_id (), mtype_type,
			       is_unsigned);
	      PL_cast (cb, &dest, &addr.op1, M_TYPE_CHAR, is_unsigned);
	      addr.op1 = dest;
	    }
	  else if (bt & BT_SHORT)
	    {
	      PL_new_register (&dest, PL_next_reg_id (), mtype_type,
			       is_unsigned);
	      PL_cast (cb, &dest, &addr.op1, M_TYPE_SHORT, is_unsigned);
	      addr.op1 = dest;
	    }
#if 1
	  else if ((PL_native_int_size > P_INT_SIZE) && (bt & BT_INT))
	    {
	      PL_new_register (&dest, PL_next_reg_id (), mtype_type,
			       is_unsigned);
	      PL_cast (cb, &dest, &addr.op1, M_TYPE_INT, is_unsigned);
	      addr.op1 = dest;
	    }
#endif
	}
      *ret = addr;
    }
  else if (!PLI_is_addr_expr (expr))
    {
      PLI_gen_load (cb, expr, &addr, ret, PST_ExprType(PL_symtab, expr));
    }
  else
    {
      *ret = addr;
    }

  return;
}

static void
PLI_gen_longlong_load(L_Cb * cb, Expr expr, PL_Ret addr, PL_Ret ret, Key type)
{
  L_Attr *attr = NULL;
  L_Oper *op;

  /* KVM : A "long long" will always be loaded into "int" registers.
   * The number of registers required is equal to M_SIZE_LLONG/M_SIZE_INT.
   * The ratio is assumed to be integral.
   */
  int num_loads = M_SIZE_LLONG/M_SIZE_INT;
  int i;
  PL_Operand base, ofst;

  _PL_Operand zero;
  PL_new_int (&zero, 0, 0);

  if (addr->type == PL_RET_SIMPLE)
    {
      ofst = &zero;
    }
  else if (addr->type == PL_RET_ADD)
    {
      ofst = &(addr->op2);
    }
  else if ((addr->type == PL_RET_SUB) & (addr->op2.type == PL_I))
    {
      addr->op2.value.i = -addr->op2.value.i;
      addr->type = PL_RET_ADD;
      ofst = &(addr->op2);
    }
  else
    {
      PLI_simplify (cb, addr);
      ofst = &zero;
    }

  base = &(addr->op1);

  attr = expr->pragma ? PL_gen_attr_from_pragma (expr->pragma) : NULL;

  ret->type = PL_RET_LIST;
  ret->oplist = (_PL_Operand *)malloc(num_loads * sizeof(_PL_Operand));

  _PL_Ret temp;

  for(i=0; i<num_loads; i++) {
    _PL_Operand dest;
    _PL_Operand inc;
    PL_new_register (&dest, PL_next_reg_id (), M_TYPE_INT, 1);

    op = PL_gen_load (cb, expr, &dest, base, ofst, 1, attr);

    if (PST_GetTypeQualifier(PL_symtab, type) & TY_VOLATILE)
      op->flags = L_SET_BIT_FLAG (op->flags, L_OPER_VOLATILE);

    ret->oplist[i] = dest;

    PL_new_int(&inc, M_SIZE_INT/M_SIZE_CHAR, 1);


    PLI_ret_reset(&temp);

    temp.op1 = *ofst;
    temp.op2 = inc;
    temp.type = PL_RET_ADD;

    PLI_simplify(cb, &temp);
    ofst = &(temp.op1);
  }
}


static L_Oper *
PLI_gen_load (L_Cb * cb, Expr expr, PL_Ret addr, PL_Ret ret, Key type)
{
  int mtype_type, is_unsigned;
  _PL_Operand dest, zero;
  PL_Operand base, ofst;
  L_Attr *attr = NULL;
  L_Oper *op;

  mtype_type = PL_key_get_mtype (type);
  
  /*
  if(mtype_type == M_TYPE_LLONG) {
    P_warn("PLI_gen_load : experimental support for \"long long\"\n");
    PLI_gen_longlong_load(cb, expr, addr, ret, type);
    return;
  }
  */

  is_unsigned = PST_IsUnsignedType(PL_symtab, type);

  PL_new_register (&dest, PL_next_reg_id (), mtype_type, is_unsigned);

  PL_new_int (&zero, 0, 0);

  attr = expr->pragma ? PL_gen_attr_from_pragma (expr->pragma) : NULL;

  if (addr->type == PL_RET_SIMPLE)
    {
      ofst = &zero;
    }
  else if (addr->type == PL_RET_ADD)
    {
      ofst = &(addr->op2);
    }
  else if ((addr->type == PL_RET_SUB) & (addr->op2.type == PL_I))
    {
      addr->op2.value.i = -addr->op2.value.i;
      addr->type = PL_RET_ADD;
      ofst = &(addr->op2);
    }
  else
    {
      PLI_simplify (cb, addr);
      ofst = &zero;
    }

  base = &(addr->op1);

  op = PL_gen_load (cb, expr, &dest, base, ofst, is_unsigned, attr);

  /* JWS 20010116 - Mark accesses to volatile variables
   * to curtail optimization in the back end 
   */

  if (PST_GetTypeQualifier(PL_symtab, type) & TY_VOLATILE)
    op->flags = L_SET_BIT_FLAG (op->flags, L_OPER_VOLATILE);

  ret->type = PL_RET_SIMPLE;
  ret->op1 = dest;
  ret->op2 = zero;

  return op;
}


static void
PLI_gen_store (L_Cb * cb, Expr expr, PL_Ret addr, PL_Ret data, Key type)
{
  _PL_Operand zero;
  PL_Operand base, ofst;
  L_Attr *attr = NULL;
  L_Oper *op;
  int mtype_type;

  mtype_type = PL_key_get_mtype (type);
  PLI_simplify (cb, data);
  PL_new_int (&zero, 0, 0);

  attr = expr->pragma ? PL_gen_attr_from_pragma (expr->pragma) : NULL;

  if (addr->type == PL_RET_SIMPLE)
    {
      ofst = &zero;
    }
  else if (addr->type == PL_RET_ADD)
    {
      ofst = &(addr->op2);
    }
  else if ((addr->type == PL_RET_SUB) & (addr->op2.type == PL_I))
    {
      addr->op2.value.i = -addr->op2.value.i;
      addr->type = PL_RET_ADD;
      ofst = &(addr->op2);
    }
  else
    {
      PLI_simplify (cb, addr);
      ofst = &zero;
    }

  base = &(addr->op1);

  op = PL_gen_store (cb, expr, base, ofst, &(data->op1), mtype_type, attr);

  /* JWS 20010116 - Mark accesses to volatile variables
   * to curtail optimization in the back end 
   */

  if (PST_GetTypeQualifier(PL_symtab, type) & TY_VOLATILE)
    op->flags = L_SET_BIT_FLAG (op->flags, L_OPER_VOLATILE);

  return;
}


static int
PL_is_bit_mask_block (ITuintmax mask, int *mask_length)
{
  int cnt = 0;

  *mask_length = 0;

  while (!(mask & 0x01))
    {
      mask >>= 1;
      cnt++;
    }

  if (cnt == PL_MType_Size(PL_native_int_reg_mtype))
    return 0;

  while ((mask & 0x01))
    {
      mask >>= 1;
      *mask_length = *mask_length + 1;
      cnt += 1;
    }

  return (mask == 0);
}


/*
 * PLI_gen_extract_bit_field
 * ----------------------------------------------------------------------
 * ret is bidirectional!
 */
static void
PLI_gen_extract_bit_field (L_Cb * cb,
			   PL_Ret ret, int bit_shift,
			   ITuintmax bit_mask, int unsign)
{
  _PL_Operand dest, src1, src2, src3;
  int length;

  PLI_simplify (cb, ret);
  src1 = ret->op1;
  PL_new_register (&dest, PL_next_reg_id (), PL_native_int_reg_mtype, 0);

  if (PL_gen_bit_field_operations && PL_is_bit_mask_block (bit_mask, &length))
    {
      /* If Lop_EXTRACT(_U) is available and mask is contiguous,
       * generate this single-operation solution.
       */

      PL_new_int (&src2, bit_shift, 0);
      PL_new_int (&src3, length, 0);
      PL_gen_extract_bits (cb, &dest, &src1, &src2, &src3, unsign);
    }
  else
    {
      if (unsign)
	{
	  PL_new_int (&src2, bit_mask, 1);
	  PL_gen_logic (cb, PL_RET_AND, &dest, &src1, &src2, NULL);

	  /* shift */
	  if (bit_shift != 0)
	    {
	      src1 = dest;
	      PL_new_int (&src2, bit_shift, 1);
	      PL_gen_lsr (cb, &dest, &src1, &src2, NULL);
	    }
	}
      else
	{
	  int sign_shift;

	  if (!PL_is_bit_mask_block (bit_mask, &length))
	    P_punt ("PLI_gen_extract_bit_field error");

	  sign_shift = PL_MType_Size(PL_native_int_reg_mtype)
	    - length - bit_shift;

	  PL_new_int (&src2, sign_shift, 0);
	  PL_gen_lsl (cb, &dest, &src1, &src2, NULL);
	  src1 = dest;

	  PL_new_int (&src2, sign_shift + bit_shift, 0);
	  PL_gen_asr (cb, &dest, &src1, &src2, NULL);
	}
    }

  ret->type = PL_RET_SIMPLE;
  ret->op1 = dest;
  PL_new_int (&(ret->op2), 0, 0);

  return;
}


/*
 * PLI_gen_deposit_bit_field
 * ----------------------------------------------------------------------
 * ret is bidirectional!
 */
static void
PLI_gen_deposit_bit_field (L_Cb * cb,
			   PL_Ret ret, PL_Ret field,
			   int bit_shift, ITuintmax bit_mask)
{
  _PL_Operand dest1, dest2, src1, src2, src3, src4;
  int length;

  PLI_simplify (cb, ret);
  PLI_simplify (cb, field);

  if (PL_gen_bit_field_operations && PL_is_bit_mask_block (bit_mask, &length))
    {
      src1 = ret->op1;
      src2 = field->op1;
      PL_new_register (&dest1, PL_next_reg_id (), PL_native_int_reg_mtype, 0);	/* JWS TYPE */
      PL_gen_mov (cb, &dest1, &src1, 0);
      PL_new_int (&src3, bit_shift, 0);
      PL_new_int (&src4, length, 0);
      PL_gen_deposit_bits (cb, &dest1, &src2, &src3, &src4, &dest1);
      ret->op1 = dest1;
    }
  else
    {
      /*
       *  dest1 = ret & ~mask
       */
      src1 = ret->op1;
      PL_new_int (&src2, ~bit_mask, 0);
      PL_new_register (&dest1, PL_next_reg_id (), PL_native_int_reg_mtype, 0);
      PL_gen_logic (cb, PL_RET_AND, &dest1, &src1, &src2, 0);
      /*
       *  field = field << shift
       */
      src1 = field->op1;
      PL_new_int (&src2, bit_shift, 0);
      PL_new_register (&dest2, PL_next_reg_id (), PL_native_int_reg_mtype, 0);
      PL_gen_lsl (cb, &dest2, &src1, &src2, 0);
      /*
       *  dest2 = field & mask
       */
      src1 = dest2;
      PL_new_int (&src2, bit_mask, 0);
      PL_gen_logic (cb, PL_RET_AND, &dest2, &src1, &src2, 0);
      /*
       *  ret = dest1 | dest2
       */
      src1 = dest1;
      src2 = dest2;
      PL_gen_logic (cb, PL_RET_OR, &dest2, &src1, &src2, 0);
      ret->op1 = dest2;
    }

  ret->type = PL_RET_SIMPLE;
  PL_new_int (&(ret->op2), 0, 0);

  return;
}


/*
 * PLI_gen_field
 * ----------------------------------------------------------------------
 * Handle OP_dot and OP_arrow for struct and union aggregates.
 */
static void
PLI_gen_field (L_Cb * cb, Expr expr, PL_Ret ret)
{
  _PL_Ret addr;
  
  if (expr->opcode == OP_dot)
    PLI_gen_addr_dot (cb, expr, &addr);
  else if (expr->opcode == OP_arrow)
    PLI_gen_addr_arrow (cb, expr, &addr);
  else
    P_punt ("PLI_gen_field: handles only OP_dot and OP_arrow");

  if (!PLI_is_addr_expr (expr))
    {
      _PL_Ret data;

      PLI_ret_reset (&data);

      if (PST_IsBitFieldExpr (PL_symtab, expr))
	{
	  PLI_gen_load_bitfield (cb, expr, &addr, &data, 
				 PST_ExprType (PL_symtab, expr->operands));
	}
      else
	{
	  L_Oper *ld = PLI_gen_load (cb, expr, &addr, &data, 
				     PST_ExprType(PL_symtab, expr));

	  /* JWS 20041109: mark union loads for wild load cspec avoidance */
	  {
	    Type t;
	    _BasicType bt;
	    t = PST_ExprType (PL_symtab, expr->operands);
	    if (expr->opcode == OP_arrow)
	      t = PST_DereferenceType (PL_symtab, t);
	    bt = PST_GetTypeBasicType (PL_symtab, t);
	    if (bt & BT_UNION)
	      ld->attr = L_concat_attr (ld->attr,
					L_new_attr ("UnLD", 0));
	  }
	}
      *ret = data;
    }
  else
    {
      *ret = addr;
    }

  return;
}


/*
 * PLI_gen_cast
 * ----------------------------------------------------------------------
 * Generate explicit casts.
 */
static void
PLI_gen_cast (L_Cb * cb, Expr expr, PL_Ret ret)
{
  Expr src_op;
  Key dest_type;
  int mtype_type, is_unsigned;

  dest_type = PST_ExprType(PL_symtab, expr);
  mtype_type = PL_key_get_mtype (dest_type);
  is_unsigned = PST_IsUnsignedType(PL_symtab, dest_type);

  src_op = expr->operands;

  PLI_gen_data (cb, src_op, ret);
  PLI_simplify (cb, ret);

  if (PST_IsRealType (PL_symtab, dest_type))
    {
      _PL_Operand src = ret->op1;

      PL_new_register (&(ret->op1), PL_next_reg_id (),
		       mtype_type, is_unsigned);

      PL_cast (cb, &(ret->op1), &src, mtype_type, is_unsigned);
    }
  else if (PST_IsIntegralType (PL_symtab, dest_type))
    {
      _PL_Operand src = ret->op1;
      int smtype_type, s_is_unsigned;
      Key stype;

      stype = PST_ExprType(PL_symtab, src_op);
      smtype_type = PL_key_get_mtype (stype);
      s_is_unsigned = PST_IsUnsignedType(PL_symtab, stype);

      PL_new_register (&(ret->op1), PL_next_reg_id (),
		       mtype_type, is_unsigned);

      /* If the dest is "larger" than the source, we need to
       * ensure that the source's extension bits are "pure"
       */

      if (PST_IsIntegralTypeExpr (PL_symtab, src_op) &&
	  (PL_key_get_size(dest_type) > PL_key_get_size(stype)))
	{
	  PL_cast (cb, &(ret->op1), &src, smtype_type, s_is_unsigned);
	  src = ret->op1;
	}

      PL_cast (cb, &(ret->op1), &src, mtype_type, is_unsigned);
    }

  /* JWS -- this should be unnecessary */

  ret->op1.data_type = mtype_type;
  ret->op1.unsign = is_unsigned;

  return;
}


static void
PLI_gen_indr (L_Cb * cb, Expr expr, PL_Ret ret)
{
  _PL_Ret addr;

  PLI_ret_reset (&addr);
  PLI_gen_addr_indr (cb, expr, &addr);

  if (!PLI_is_addr_expr (expr))
    PLI_gen_load (cb, expr, &addr, ret, PST_ExprType(PL_symtab, expr));
  else
    *ret = addr;
  return;
}


static void
PLI_gen_index (L_Cb * cb, Expr expr, PL_Ret ret)
{
  _PL_Ret addr;

  PLI_ret_reset (&addr);
  PLI_gen_addr_index (cb, expr, &addr);
  if (!PLI_is_addr_expr (expr))
    PLI_gen_load (cb, expr, &addr, ret, PST_ExprType(PL_symtab, expr));
  else
    *ret = addr;
  return;
}


static void
PL_gen_parameters (L_Cb *cb, Expr *pin, PL_Operand pout, int num)
{
  _PL_Ret ret;
  int i;

  for (i = 0; i < num; i++)
    {
      PLI_gen_data (cb, pin[i], &ret);
      PLI_simplify (cb, &ret);
      pout[i] = ret.op1;
    }
  return;
}


static void
PL_gen_destination (PL_Operand dest, PL_Ret result, int type, int unsign)
{
  PL_new_register (dest, PL_next_reg_id (), type, unsign);
  result->type = PL_RET_SIMPLE;
  result->op1 = *dest;
  PL_new_int (&(result->op2), 0, 0);
  return;
}


static void
PL_gen_va_start (char *name, L_Cb *cb, PL_Ret result, Expr *opds, int num)
{
  L_Attr *va_attr;
  ITintmax first_va, va_ofst;
  _PL_Ret rhs;
  _PL_Operand src0, src1;

  if (num < 1)
    P_punt ("%s() should have at least one argument", name);

  if (!(va_attr = L_find_attr (L_fn->attr, "VARARG")))
    P_punt ("no VARARG attr found in a function containing %s()", name);

  /*
   * Compute $IP-relative offset of first vararg
   * opds[0] is the va_list variable
   */

  first_va = va_attr->field[0]->value.i;
	  
  if (M_arch == M_TAHOE)
    va_ofst = 8 * first_va - 48;
  else if (M_arch == M_ARM)
    va_ofst = 4 * first_va - 16;
  else
    va_ofst = first_va;

  PL_new_macro (&src0, strdup ("$IP"), M_TYPE_POINTER, 0);
  PL_new_int (&src1, va_ofst, 0);
  rhs.op1 = src0;
  rhs.op2 = src1;
  rhs.type = PL_RET_ADD;

  PLI_simplify (cb, &rhs);
  PL_assign_to (cb, opds[0], &rhs, result);

  return;
}


static void
PL_gen_va_arg (char *name, L_Cb *cb, PL_Ret result, Expr *opds, int num)
{
  L_Attr *va_attr;
  int va_step;
  _PL_Ret prev_addr;
  _PL_Operand hc_step, dest;
  L_Attr *mark;
  int loc;
  Expr va_param;

  if (num < 1)
    P_punt ("%s() should have at least one argument", name);

  if (!(va_attr = L_find_attr (L_fn->attr, "VARARG")))
    P_punt ("no VARARG attr found in a function containing %s()", name);

  va_param = opds[0];

  if (va_param->opcode == OP_addr)
    va_param = va_param->operands;

  PLI_ret_reset (&prev_addr);

  loc = PLI_gen_vararg_addr (cb, va_param, &prev_addr);
  PLI_simplify (cb, &prev_addr);

  va_step = PL_MType_Size(PL_native_int_reg_mtype) / 8;
  mark = L_new_attr ("BUILTIN", 0);
  PL_new_int (&hc_step, va_step, 0);
  PL_gen_destination (&dest, result, M_TYPE_POINTER, 0);
  PL_gen_add (cb, &dest, &hc_step, &prev_addr.op1, 1, mark);

  return;
}


static void
PLI_gen_call (L_Cb * cb, Expr expr, PL_Ret result)
{
  Expr *temp_expr;
  _M_Type *temp_mtype;
  long *temp_offset;
  int *temp_mode;
  int *temp_reg;
  int *temp_paddr;
  int *temp_su_sreg;
  int *temp_su_ereg;
  int *temp_real_sreg;
  int *temp_real_ereg;
  int temp_count;

  char *temp_base_macro;
  int n_opds, tsize, offset, i, r, r2, size, tmcount;
  L_Attr *new_attr, *var_attr = NULL;
  Expr op, called_func;
  _PL_Ret ret, fn_addr, unsimp_ret;
  _PL_Operand dest, src1, src2, src3;
  _M_Type mtype;
  _M_Param rv_param;
  _PL_Operand *param;
  L_Attr *attr = NULL;
  L_Attr *tr_attr = NULL;
  L_Attr *func_attr = NULL;
  Key return_type;
  char line[128];

  int rv_is_aggr, rv_is_indir_aggr = 0;
  int rv_is_longlong = 0;

  L_Oper *jsr_op;

  PLI_ret_reset (&ret);
  PLI_ret_reset (&fn_addr);
  PLI_ret_reset (&unsimp_ret);

  /*
   *  Handle IMPACT builtins
   */

  return_type = PST_ExprType (PL_symtab, expr);

  rv_is_aggr = PL_is_aggr_type (return_type, &return_type);

  rv_is_longlong = PST_IsLongLongType(PL_symtab, return_type);

  PL_pcode2lcode_type (return_type, &(rv_param.mtype), 0);

  if (rv_is_aggr)
    {
      M_layout_retvar (&rv_param, M_GET_FNVAR);
      rv_is_indir_aggr = !PL_gen_compliant_struct_return ||
	(rv_param.mode != M_THRU_REGISTER);
    }

  called_func = expr->operands;


  /* Count operands and set up temporary structures */

  for (op = called_func->sibling, n_opds = 0; op; op = op->next, n_opds++);

  temp_expr = alloca (n_opds * sizeof (Expr));
  temp_mtype = alloca (n_opds * sizeof (_M_Type));
  temp_offset = alloca (n_opds * sizeof (long));
  temp_mode = alloca (n_opds * sizeof (int));
  temp_reg = alloca (n_opds * sizeof (int));
  temp_paddr = alloca (n_opds * sizeof (int));
  temp_su_sreg = alloca (n_opds * sizeof (int));
  temp_su_ereg = alloca (n_opds * sizeof (int));
  temp_real_sreg = alloca (n_opds * sizeof (int));
  temp_real_ereg = alloca (n_opds * sizeof (int));
  param = alloca (n_opds * sizeof (_PL_Operand));

  if ((called_func->opcode == OP_var) && 
      !(PST_IsEnumTypeExpr(PL_symtab, called_func)))
    {
      char *callee_name = called_func->value.var.name;

      for (op = expr->operands->sibling, i = 0; op; op = op->next, i++)
	temp_expr[i] = op;
      
      /*
       * Handle special "builtin" subroutine calls
       * ---------------------------------------------------------------------
       */

      /* Search for intrinsic operations if they are enabled.
       * -ITI/JWJ 6.23.1999
       */
      if ((L_intrinsic_support_enabled == 1) &&
	  (PL_insert_intrinsics == 1) &&
	  (PL_intrinsic_intrinsify (callee_name, result,
				    temp_expr, ret, cb, &dest, param, n_opds)))
	{
	  return;
	}
      else if (!strcmp (callee_name, "__builtin_stdarg_start") ||
	       !strcmp (callee_name, "__builtin_varargs_start") ||
	       !strcmp (callee_name, "__builtin_va_start") ||
	       !strcmp (callee_name, "va_start"))
	{
	  PL_gen_va_start (callee_name, cb, result, temp_expr, n_opds);
	  return;
	}
      else if (!strcmp (callee_name, "__builtin_va_arg") ||
	       !strcmp (callee_name, "__builtin_next_arg"))
	{
	  PL_gen_va_arg (callee_name, cb, result, temp_expr, n_opds);
	  return;
	}
      else if (!strcmp (callee_name, "sqrt") && M_oper_supported_in_arch(Lop_SQRT_F2))
	{
	  PL_gen_parameters (cb, temp_expr, param, n_opds);
	  PL_gen_destination (&dest, result, M_TYPE_DOUBLE, 0);
	  PL_gen_lcode_sqrt_f2 (cb, &dest, param, n_opds);
	  return;
	}
      else if (!strcmp (callee_name, "fabs") && M_oper_supported_in_arch(Lop_ABS_F2))
	{
	  PL_gen_parameters (cb, temp_expr, param, n_opds);
	  PL_gen_destination (&dest, result, M_TYPE_DOUBLE, 0);
	  PL_gen_lcode_abs_f2 (cb, &dest, param, n_opds);
	  return;
	}
      else if (!strcmp (callee_name, "fmin") && M_oper_supported_in_arch(Lop_MIN_F2))
	{
	  PL_gen_parameters (cb, temp_expr, param, n_opds);
	  PL_gen_destination (&dest, result, M_TYPE_DOUBLE, 0);
	  PL_gen_lcode_min_f2 (cb, &dest, param, n_opds);
	  return;
	}
      else if (!strcmp (callee_name, "fmax") && M_oper_supported_in_arch(Lop_MAX_F2))
	{
	  PL_gen_parameters (cb, temp_expr, param, n_opds);
	  PL_gen_destination (&dest, result, M_TYPE_DOUBLE, 0);
	  PL_gen_lcode_max_f2 (cb, &dest, param, n_opds);
	  return;
	}
      else if (!strncmp (callee_name, HCfn_PREFIX, HCfn_PREFIX_LENGTH))
	{
	  if (!strcmp (callee_name, HCfn_SELECT_I))
	    {
	      PL_gen_parameters (cb, temp_expr, param, n_opds);
	      PL_gen_destination (&dest, result, PL_native_int_reg_mtype, 0);
	      PL_gen_lcode_select_i (cb, &dest, param, n_opds);
	    }
	  else if (!strcmp (callee_name, HCfn_SELECT_F))
	    {
	      PL_gen_parameters (cb, temp_expr, param, n_opds);
	      PL_gen_destination (&dest, result, M_TYPE_FLOAT, 0);
	      PL_gen_lcode_select_f (cb, &dest, param, n_opds);
	    }
	  else if (!strcmp (callee_name, HCfn_SELECT_F2))
	    {
	      PL_gen_parameters (cb, temp_expr, param, n_opds);
	      PL_gen_destination (&dest, result, M_TYPE_DOUBLE, 0);
	      PL_gen_lcode_select_f2 (cb, &dest, param, n_opds);
	    }
	  else if (!strcmp (callee_name, HCfn_REV))
	    {
	      PL_gen_parameters (cb, temp_expr, param, n_opds);
	      PL_gen_destination (&dest, result, PL_native_int_reg_mtype, 0);
	      PL_gen_lcode_rev (cb, &dest, param, n_opds);
	    }
	  else if (!strcmp (callee_name, HCfn_BIT_POS))
	    {
	      PL_gen_parameters (cb, temp_expr, param, n_opds);
	      PL_gen_destination (&dest, result, PL_native_int_reg_mtype, 0);
	      PL_gen_lcode_bit_pos (cb, &dest, param, n_opds);
	    }
	  else if (!strcmp (callee_name, HCfn_ABS_I))
	    {
	      PL_gen_parameters (cb, temp_expr, param, n_opds);
	      PL_gen_destination (&dest, result, PL_native_int_reg_mtype, 0);
	      PL_gen_lcode_abs_i (cb, &dest, param, n_opds);
	    }
	  else if (!strcmp (callee_name, HCfn_ABS_F))
	    {
	      PL_gen_parameters (cb, temp_expr, param, n_opds);
	      PL_gen_destination (&dest, result, M_TYPE_FLOAT, 0);
	      PL_gen_lcode_abs_f (cb, &dest, param, n_opds);
	    }
	  else if (!strcmp (callee_name, HCfn_CO_PROC))
	    {
	      PL_gen_parameters (cb, temp_expr, param, n_opds);
	      PL_gen_destination (&dest, result, PL_native_int_reg_mtype, 0);
	      PL_gen_lcode_co_proc (cb, &dest, param, n_opds);
	    }
	  else if (!strcmp (callee_name, HCfn_FETCH_AND_ADD))
	    {
	      PL_gen_parameters (cb, temp_expr, param, n_opds);
	      PL_gen_destination (&dest, result, PL_native_int_reg_mtype, 0);
	      PL_gen_lcode_fetch_add (cb, &dest, param, n_opds);
	    }
	  else if (!strcmp (callee_name, HCfn_FETCH_AND_OR))
	    {
	      PL_gen_parameters (cb, temp_expr, param, n_opds);
	      PL_gen_destination (&dest, result, PL_native_int_reg_mtype, 0);
	      PL_gen_lcode_fetch_or (cb, &dest, param, n_opds);
	    }
	  else if (!strcmp (callee_name, HCfn_FETCH_AND_AND))
	    {
	      PL_gen_parameters (cb, temp_expr, param, n_opds);
	      PL_gen_destination (&dest, result, PL_native_int_reg_mtype, 0);
	      PL_gen_lcode_fetch_and (cb, &dest, param, n_opds);
	    }
	  else if (!strcmp (callee_name, HCfn_FETCH_AND_ST))
	    {
	      PL_gen_parameters (cb, temp_expr, param, n_opds);
	      PL_gen_destination (&dest, result, PL_native_int_reg_mtype, 0);
	      PL_gen_lcode_fetch_st (cb, &dest, param, n_opds);
	    }
	  else if (!strcmp (callee_name, HCfn_FETCH_AND_COND_ST))
	    {
	      PL_gen_parameters (cb, temp_expr, param, n_opds);
	      PL_gen_destination (&dest, result, PL_native_int_reg_mtype, 0);
	      PL_gen_lcode_fetch_cond_st (cb, &dest, param, n_opds);
	    }
	  else
	    {
	      P_punt ("Unhandled %s* builtin function %s()", HCfn_PREFIX,
		      callee_name);
	    }
	  return;
	}
      else if (PL_generate_abs_instructions && 
	       !strcmp (callee_name, "abs"))
	{
	  PL_gen_parameters (cb, temp_expr, param, n_opds);
	  PL_gen_destination (&dest, result, PL_native_int_reg_mtype, 0);
	  PL_gen_lcode_abs_i (cb, &dest, param, n_opds);
	  return;
	}
      else if (PL_generate_abs_instructions &&
	       !strcmp (callee_name, "fabs"))
	{
	  PL_gen_parameters (cb, temp_expr, param, n_opds);
	  PL_gen_destination (&dest, result, M_TYPE_DOUBLE, 0);
	  PL_gen_lcode_abs_f2 (cb, &dest, param, n_opds);
	}

      /*
       * Handle "normal" direct subroutine calls
       * ---------------------------------------------------------------------
       */

      PLI_ret_reset (&fn_addr);

      /* Make sure this is a function */
      assert(PST_GetFuncDclEntry(PL_symtab, called_func->value.var.key));
      PLI_gen_addr (cb, called_func, &fn_addr);

      {
	/* JWS 20040708: Acquire attrs from the callee FuncDcl */

	Key callee_key = called_func->value.var.key;
	FuncDcl callee_fdcl = PSI_GetFuncDclEntry (callee_key);
	Pragma p;

	for (p = callee_fdcl->pragma; p; p = p->next)
	  if (!strcmp (p->specifier, "Cattr"))
	    {
	      L_Attr *a = PL_gen_single_attr_from_pragma (NULL, p);
	      func_attr = L_concat_attr (func_attr, a);
	    }
      }
    }
  else
    {
      /*
       * Handle indirect subroutine calls
       * ---------------------------------------------------------------------
       */

      PLI_gen_data (cb, called_func, &fn_addr);
    }

  PLI_simplify (cb, &fn_addr);

  /*
   *  figure out how to pass parameters.
   */

  for (op = expr->operands->sibling, i = 0; op; op = op->next, i++)
    {
      temp_expr[i] = op;
      PL_pcode2lcode_type (PST_ExprType(PL_symtab, op), temp_mtype + i, 1);
    }

  /*
   *  Here, we are preparing arguments to be sent out to a callee
   *  function. We need to worry about passing structure.
   */

  size = M_fnvar_layout (n_opds, temp_mtype,
			 temp_offset, temp_mode,
			 temp_reg, temp_paddr,
			 &temp_base_macro,
			 temp_su_sreg, temp_su_ereg,
			 &temp_count, rv_is_aggr, M_PUT_FNVAR);
  if (M_arch != M_TAHOE)
    {
      // zzz
/*       printf("\n\nFUNC: %s\n\n", called_func->value.var.name); */
      size /= P_CHAR_SIZE;
      for (i = 0; i < n_opds; i++)
	{
	  temp_offset[i] /= P_CHAR_SIZE;
	  temp_paddr[i] /= P_CHAR_SIZE;
/* 	  printf("offset: %i\naddr:   %i\n\n", temp_offset[i], temp_paddr[i]); */
	}
    }

  /*
   *  generate arguments.
   */
  tmcount = L_TM_START_VALUE;
  for (i = 0; i < n_opds; i++)
    {
      L_Attr *new_attr;
      Type ty;

      int base;
      PLI_gen_data (cb, temp_expr[i], &ret);
      unsimp_ret = ret;
      PLI_simplify (cb, &ret);
      switch (temp_mode[i])
	{
	case M_THRU_REGISTER:
	  if (((M_arch == M_TAHOE) || (M_arch == M_ARM)) &&
	      (temp_mtype[i].type == M_TYPE_UNION ||
	       temp_mtype[i].type == M_TYPE_STRUCT))
	    {
	      int struct_size = temp_mtype[i].size / 8, 
		struct_align = temp_mtype[i].align / 8, last;

	      /* Passing structure through reg in IA64 */
	      /* Get real regs as temps for param regs */
	      last = temp_real_sreg[i] = PL_next_reg_id ();
	      for (r = temp_su_sreg[i] + 1; r <= temp_su_ereg[i]; r++)
		last = PL_next_reg_id ();
	      temp_real_ereg[i] = last;

	      new_attr = L_new_attr ("struct", 0);

	      src1 = ret.op1;
	      base = 0;

	      offset = 0;
	      tsize = PL_MType_Size(PL_native_int_reg_mtype) / 8;

#if 0
	      printf
		("Param(%d %d), REAL(%d %d), Offset(%d), Paddr(%d)\n",
		 temp_su_sreg[i], temp_su_ereg[i],
		 temp_real_sreg[i], temp_real_ereg[i],
		 temp_offset[i], (temp_paddr[i] - 6) * tsize);
	      printf ("base %d offset %d\n", base, offset);
#endif
	      for (r = temp_real_sreg[i]; r <= temp_real_ereg[i]; r++)
		{
		  PL_new_register (&dest, r, PL_native_int_reg_mtype,
				   mtype.unsign);
		  PL_new_int (&src2, base + offset, 0);
		  PL_gen_load (cb, temp_expr[i],
			       &dest, &src1, &src2, dest.data_type, NULL);
		  offset += tsize;
		}

	      if (struct_size > offset)
		{
		  int in_offset, out_offset, bytes_left;
		  int copy_type = 0, copy_size = 0;
		  bytes_left = struct_size - offset;
		  if(M_arch == M_ARM)
		    out_offset = 0;
		  else
		    out_offset = 16;
		  in_offset = offset + base;
		  while (bytes_left > 0)
		    {
		      if ((bytes_left >= 8) && (struct_align >= 8))
			{
			  copy_size = 8;
			  copy_type = M_TYPE_LLONG;
			}
		      else if ((bytes_left >= 4) && (struct_align >= 4))
			{
			  copy_size = 4;
			  copy_type = M_TYPE_INT;
			}
		      else if ((bytes_left >= 2) && (struct_align >= 2))
			{
			  copy_size = 2;
			  copy_type = M_TYPE_SHORT;
			}
		      else if ((bytes_left >= 1))
			{
			  copy_size = 1;
			  copy_type = M_TYPE_CHAR;
			}
		      PL_new_register (&dest, PL_next_reg_id (),
				       copy_type, mtype.unsign);

                      /* KVM : Above line used to be : PL_native_int_reg_mtype, mtype.unsign);
                       * PL_native_int_reg_mtype was used to store a llong. Changed it to LLONG_TYPE, so
                       * that later llong_translation can fix it
                       */

		      PL_new_int (&src2, in_offset, 0);
		      PL_gen_load (cb, temp_expr[i], &dest, &src1, &src2,
				   copy_type, NULL);
		      PL_new_macro (&src3, temp_base_macro,
				    M_TYPE_POINTER, 0);
		      PL_new_int (&src2, out_offset, 0);
		      PL_gen_store (cb, ((void *) (-1)), &src3, &src2, &dest,
				    copy_type, NULL);
		      in_offset += copy_size;
		      out_offset += copy_size;
		      bytes_left -= copy_size;
		    }
		}
	    }
	  param[i] = ret.op1;
	  break;
	case M_THRU_MEMORY:
	  new_attr = L_new_attr ("tm", 1);
	  L_set_int_attr_field (new_attr, 0, tmcount++);
	  /* mem[base_macro+offset] = ret */
	  ty = PST_ExprType (PL_symtab, temp_expr[i]);
	  if (PST_IsStructureType (PL_symtab, ty) &&
	      !PL_is_link_multi (ty, NULL))
	    {
	      PL_new_macro (&dest, temp_base_macro, M_TYPE_POINTER, 0);
	      src1 = unsimp_ret.op1;
	      PL_gen_block_mov (cb, temp_expr[i], &dest,
				temp_offset[i], &src1,
				unsimp_ret.op2.value.i,
				temp_mtype[i].size / P_CHAR_SIZE,
				temp_mtype[i].align / P_CHAR_SIZE,
				new_attr, 0, 0, 1);
	    }
	  else
	    {
	      int mtype_type;
	      /* need to promote type */
	      mtype_type = PL_key_get_mtype (PST_ExprType(PL_symtab, 
							  temp_expr[i]));
	      if (mtype_type != temp_mtype[i].type)
		{
		  PL_new_register (&src3, PL_next_reg_id (),
				   temp_mtype[i].type, temp_mtype[i].unsign);

		  PL_gen_mov (cb, &src3, &(ret.op1), 0);
		}
	      else
		{
		  src3 = ret.op1;
		}
	      PL_new_macro (&src1, temp_base_macro, M_TYPE_POINTER, 0);
	      PL_new_int (&src2, temp_offset[i], 0);
	      PL_gen_store (cb, temp_expr[i], &src1, &src2, &src3,
			    temp_mtype[i].type, new_attr);
	    }
	  break;
	case M_INDIRECT_THRU_REGISTER:
	  /*
	   *  move_block ret -> temp_paddr
	   *  temp_paddr + temp_base_macro -> param[i]
	   */
	  if (PST_IsStructureTypeExpr (PL_symtab,temp_expr[i]))
	    {
	      /* REH 5/20/95  - Add the "tms" attribute to all operations
	         involved in the block copy of a struture passed as a param. */
	      new_attr = L_new_attr ("tms", 1);
	      L_set_int_attr_field (new_attr, 0, temp_paddr[i]);
	      /* HER */
	      PL_new_macro (&dest, temp_base_macro, M_TYPE_POINTER, 0);
	      src1 = ret.op1;
	      PL_gen_block_mov (cb, temp_expr[i], &dest,
				temp_paddr[i], &src1, 0,
				temp_mtype[i].size / P_CHAR_SIZE, 
				temp_mtype[i].align / P_CHAR_SIZE,
				L_copy_attr (new_attr), 0, 0, 1);
	      PL_new_register (&dest, PL_next_reg_id (), M_TYPE_POINTER, 0);
	      PL_new_macro (&src1, temp_base_macro, M_TYPE_POINTER, 0);
	      PL_new_int (&src2, temp_paddr[i], 0);
	      PL_gen_add (cb, &dest, &src1, &src2, 1, new_attr);
	      param[i] = dest;
	    }
	  else
	    {
	      P_punt
		("M_INDIRECT_THRU_REGISTER mode can be used to pass structure only");
	    }
	  break;
	case M_INDIRECT_THRU_MEMORY:
	  /*
	   *  move_block ret -> temp_paddr
	   *  temp_paddr + temp_base_macro -> memory[temp_base_macro+offset]
	   */
	  if (PST_IsStructureTypeExpr (PL_symtab,temp_expr[i]))
	    {
	      new_attr = L_new_attr ("tms", 1);
	      L_set_int_attr_field (new_attr, 0, temp_paddr[i]);

	      PL_new_macro (&dest, temp_base_macro, M_TYPE_POINTER, 0);
	      src1 = ret.op1;
	      PL_gen_block_mov (cb, temp_expr[i], &dest,
				temp_paddr[i], &src1, 0,
				temp_mtype[i].size / P_CHAR_SIZE, 
				temp_mtype[i].align / P_CHAR_SIZE,
				L_copy_attr (new_attr), 0, 0, 1);
	      PL_new_register (&dest, PL_next_reg_id (), M_TYPE_POINTER, 0);
	      PL_new_macro (&src1, temp_base_macro, M_TYPE_POINTER, 0);
	      PL_new_int (&src2, temp_paddr[i], 0);
	      PL_gen_add (cb, &dest, &src1, &src2, 1, L_copy_attr (new_attr));
	      src3 = dest;
	      PL_new_macro (&src1, temp_base_macro, M_TYPE_POINTER, 0);
	      PL_new_int (&src2, temp_offset[i], 0);
	      /* Store address to the stack */
	      PL_gen_store (cb, ((void *) (-1)), &src1, &src2, &src3,
			    M_TYPE_POINTER, new_attr);
	    }
	  else
	    {
	      P_punt
		("M_INDIRECT_THRU_MEMORY mode can be used to pass structure only");
	    }
	  break;
	default:
	  P_punt ("illegal argument mode returned by M_fnvar_layout");
	}
    }
  /*
   *  now, move into $P registers.
   */
  for (i = 0; i < n_opds; i++)
    {
      switch (temp_mode[i])
	{
	case M_THRU_REGISTER:
	  /* $Pi = ret.op1 */

	  if (temp_mtype[i].type == M_TYPE_UNION ||
	      temp_mtype[i].type == M_TYPE_STRUCT)
	    {
	      if (M_arch != M_TAHOE && M_arch != M_ARM)
		P_punt ("Struct/Union thru reg only in IA64/ARM\n");

	      /* Passing structure through reg in IA64 */
	      for (r = temp_su_sreg[i], r2 = temp_real_sreg[i];
		   r <= temp_su_ereg[i]; r++, r2++)
		{
		  sprintf (line, "$P%d", r);
		  PL_new_register (&src1, r2, PL_native_int_reg_mtype, 0);
		  PL_new_macro (&dest, line, PL_native_int_reg_mtype,
				temp_mtype[i].unsign);
		  PL_gen_mov (cb, &dest, &src1, 0);
		}
	    }
	  else
	    {
	      sprintf (line, "$P%d", temp_reg[i]);

	      PL_new_macro (&dest, line,
			    temp_mtype[i].type, temp_mtype[i].unsign);

	      if (M_arch == M_TAHOE)
		PL_gen_mov (cb, &dest, param + i, 0);
	      else
		PL_gen_mov (cb, &dest, param + i, attr);
	    }
	  break;
	case M_INDIRECT_THRU_REGISTER:
	  /* $Pi = ret.op1 */
	  sprintf (line, "$P%d", temp_reg[i]);
	  PL_new_macro (&dest, line, M_TYPE_POINTER, 0);
	  PL_gen_mov (cb, &dest, param + i, attr);
	  break;
	default:
	  break;
	}
    }
  /*
   *  if the function returns a structure, need to allocate
   *  space for it.
   */
  if (rv_is_indir_aggr)
    {
      /* If the function returns an aggregate through the "indirect by
       * register" method, set up the structure pointer with the
       * address of the return buffer.
       */
      char name[128];
      _PL_Operand offset;
      if (!M_return_value_thru_stack())	
        {

	  sprintf (name, "$P%d", M_structure_pointer (M_PUT_FNVAR));
	  PL_new_macro (&dest, name, M_TYPE_POINTER, 0);
	  PL_new_int (&offset, 0, 0);
	  if (!PLI_func_return_struct)
	    P_punt ("PLI_gen_call: structure return address is NULL!");
	  PL_gen_add (cb, &dest, PLI_func_return_struct, &offset, 1, NULL);
	  PLI_func_return_struct = NULL;
	}
      else
	{
	  PL_new_int(&offset,0, 0);
	  PL_new_register(&dest, PL_next_reg_id(), M_TYPE_POINTER, 0);
	  PL_gen_add(cb, &dest, PLI_func_return_struct, &offset, 1, NULL);
	  PL_new_macro(&src1, temp_base_macro, M_TYPE_POINTER, 0);
	  PL_new_int(&src2, M_return_value_offset(), 0);
	  PL_gen_store(cb, expr, &src1, &src2, &(dest), M_TYPE_POINTER, 0);
	}
    }

/* REH 3/7/93 add attirbutes to jsr to identify parameters used */
  {
    int trcount = 0;
    int tmcount = 0;
    int tsize;
    L_Attr *tm_attr = NULL;
    L_Attr *tmo_attr = NULL;
    L_Attr *tmso_attr = NULL;
    L_Attr *trsof_attr = NULL;
    L_Attr *trse_attr = NULL;
    L_Attr *count_attr = NULL;
    L_Attr *parm_attr = NULL;
    int nth_param = 0;
    int nth_output_reg = 0;
    int tr_pos = 0;
    Key tmp_type;
    int mtype_type, is_unsigned;
    
    if (var_attr)
      func_attr = L_concat_attr (func_attr, var_attr);

    parm_attr = L_new_attr ("op_info", 0);
    for (i = 0; i < n_opds; i++)
      {
	if ((temp_mode[i] == M_THRU_REGISTER) ||
	    (temp_mode[i] == M_INDIRECT_THRU_REGISTER))
	  {
	    if (trcount == 0)
	      tr_attr = L_new_attr ("tr", 0);

	    if (((M_arch == M_TAHOE) || (M_arch == M_ARM)) &&
		(temp_mtype[i].type == M_TYPE_UNION ||
		 temp_mtype[i].type == M_TYPE_STRUCT))
	      {
		/* Passing structure through reg in IA64 */
		for (r = temp_su_sreg[i]; r <= temp_su_ereg[i]; r++)
		  {
		    /* Set parameter reg info */
		    L_set_int_attr_field (parm_attr, 2 * tr_pos, nth_param);
		    nth_output_reg = r - 8;
		    L_set_int_attr_field (parm_attr, 2 * tr_pos + 1,
					  nth_output_reg);
		    nth_output_reg += 1;
		    tr_pos++;

		    L_set_macro_attr_field (tr_attr, trcount++,
					    L_MAC_P0 + r,
					    PL_native_int_reg_ctype,
					    L_PTYPE_NULL);
		  }
		nth_param++;
		/* This is the offset from which the copied struct
		   will start (relative to IP) */
		tsize = PL_MType_Size(PL_native_int_reg_mtype) / 8;
		if (!tmso_attr)
		  tmso_attr = L_new_attr ("tmso", 0);

                // Amir: This set the tmso_attr for the JSR instruction         
                if (M_arch == M_ARM)
                {
                  L_set_int_attr_field (tmso_attr, i, temp_su_sreg[i] * tsize - 16);
                }
                else
                {
                  L_set_int_attr_field (tmso_attr, i,
				      (temp_paddr[i] - 6) * tsize);
                }
		/* Set struct/union register range (trse) 
		 * thru register start end */
		if (!trse_attr)
		  trse_attr = L_new_attr ("trse", 0);

		L_set_int_attr_field (trse_attr, 2 * i, temp_su_sreg[i]);
		L_set_int_attr_field (trse_attr, 2 * i + 1, temp_su_ereg[i]);

		/* This is the start and end offset that the struct
		   will occupy once copied into the stack */
		if (!trsof_attr)
		  trsof_attr = L_new_attr ("trsof", 0);

		L_set_int_attr_field (trsof_attr, 2 * i,
				      (temp_paddr[i] - 6) * tsize);
		L_set_int_attr_field (trsof_attr,
				      2 * i + 1,
				      ((temp_paddr[i] - 6)
				       * tsize + temp_mtype[i].size / 8));
	      }
	    else
	      {
		/* Set parameter reg info */
		L_set_int_attr_field (parm_attr, 2 * tr_pos, nth_param);
		L_set_int_attr_field (parm_attr, 2 * tr_pos + 1,
				      nth_output_reg);

		if (temp_mtype[i].size > PL_native_int_size)
		  {
		    /* The param is larger than one int reg */
		    int incr;
		    incr = (temp_mtype[i].size / PL_native_int_size);
		    if (temp_mtype[i].size % PL_native_int_size)
		      incr++;
		    nth_output_reg += incr;
		  }
		else
		  {
		    nth_output_reg++;
		  }

		nth_param++;
		tr_pos++;

		L_set_macro_attr_field (tr_attr, trcount++,
					L_MAC_P0 + temp_reg[i],
					PL_reg_ctype (temp_mtype[i].type,
						      temp_mtype[i].unsign),
					L_PTYPE_NULL);

		/* Also, add structure offsets for indirect thru 
		 * register -ITI/JCG 3/99
		 */
		if (temp_mode[i] == M_INDIRECT_THRU_REGISTER)
		  {
		    if (!tmso_attr)
		      tmso_attr = L_new_attr ("tmso", 0);

		    L_set_int_attr_field (tmso_attr, i, temp_paddr[i]);
		  }
	      }
	  }
	else
	  {
	    if (tmcount == 0)
	      {
		tm_attr = L_new_attr ("tm", 0);
		tmo_attr = L_new_attr ("tmo", 0);
	      }
	    L_set_int_attr_field (tm_attr, tmcount,
				  L_TM_START_VALUE + tmcount);
	    /* To make Lemulate robust, explicitly specify memory offsets
	     * for thru-memory parameters on JSRs.  -ITI/JCG 3/99 
	     */
	    L_set_int_attr_field (tmo_attr, tmcount, temp_offset[i]);
	    /* Also, add structure offsets for indirect thru 
	     * memory -ITI/JCG 3/99
	     */
	    if (temp_mode[i] == M_INDIRECT_THRU_MEMORY)
	      {
		if (!tmso_attr)
		  tmso_attr = L_new_attr ("tmso", 0);

		L_set_int_attr_field (tmso_attr, i, temp_paddr[i]);
	      }
	    tmcount++;
	  }
      }
    if (tr_attr)
      func_attr = L_concat_attr (func_attr, tr_attr);
    if (tm_attr)
      func_attr = L_concat_attr (func_attr, tm_attr);
    if (tmo_attr)
      func_attr = L_concat_attr (func_attr, tmo_attr);
    if (tmso_attr)
      func_attr = L_concat_attr (func_attr, tmso_attr);
    if (trse_attr)
      func_attr = L_concat_attr (func_attr, trse_attr);
    if (trsof_attr)
      func_attr = L_concat_attr (func_attr, trsof_attr);
    count_attr = L_new_attr ("op", 0);
    L_set_int_attr_field (count_attr, 0, temp_count);
    func_attr = L_concat_attr (func_attr, parm_attr);
    func_attr = L_concat_attr (func_attr, count_attr);

    tmp_type = PST_ExprType(PL_symtab, expr);
    mtype_type = PL_key_get_mtype (tmp_type);
    is_unsigned = PST_IsUnsignedType(PL_symtab, tmp_type);

    /* If the function returns a structure, denote that the 
       return value register is actually used by the subroutine call */
    if (!M_return_value_thru_stack() && rv_is_indir_aggr && (M_arch != M_SPARC))
      {
	/* REH - 3/3/95  Sparc doesn't pass struct return address thru reg */

	/* If the jsr returns a structure add a use of the return macro */
	if (!tr_attr)
	  {
	    tr_attr = L_new_attr ("tr", 0);
	    func_attr = L_concat_attr (func_attr, tr_attr);
	  }

	new_attr = L_new_attr ("ret_st", 0);
	func_attr = L_concat_attr (func_attr, new_attr);

	L_set_macro_attr_field (new_attr, 0,
				L_MAC_P0 + M_return_register (mtype_type,
							      M_GET_FNVAR),
				PL_reg_ctype (mtype_type, is_unsigned),
				L_PTYPE_NULL);

	L_set_macro_attr_field (tr_attr, trcount,
				L_MAC_P0 + M_return_register (mtype_type,
							      M_GET_FNVAR),
				PL_parm_ctype (mtype_type, is_unsigned),
				L_PTYPE_NULL);
      }

    /* Provide the return register macro name */

    if (!M_return_value_thru_stack()) 
      {

	new_attr = L_new_attr ("ret", 1);

	L_set_macro_attr_field (new_attr, 0,
				L_MAC_P0 + M_return_register (mtype_type,
							      M_GET_FNVAR),
				PL_reg_ctype (mtype_type, is_unsigned),
				L_PTYPE_NULL);

	func_attr = L_concat_attr (func_attr, new_attr);

      }

    new_attr = L_new_attr ("param_size", 1);
    L_set_int_attr_field (new_attr, 0, size);
    func_attr = L_concat_attr (func_attr, new_attr);
    jsr_op = PL_gen_jsr (cb, expr, &(fn_addr.op1), n_opds, func_attr);

    if (rv_is_aggr && M_arch == M_TAHOE)
      {
	if (PL_is_HFA_type (tmp_type))
	  P_warn ("Non-SCRAG-compliant HFA struct return in %s(), jsr op %d",
		  L_fn->name, jsr_op->id);
	else if (PL_key_get_size (tmp_type) <= 32)
	  P_warn ("Non-SCRAG-compliant small struct return in %s(), jsr op %d",
		  L_fn->name, jsr_op->id);
      }
  }
  /*
   *  move the return value to appropriate location.
   */
  if (PST_IsVoidTypeExpr(PL_symtab, expr))
    {
      PLI_ret_reset (result);
    }
  else if (!rv_is_aggr)
    {
      /* r <- $Pn, n = M_return_register(mtype.type) */
      int mtype_type, is_unsigned;

      mtype_type = PL_key_get_mtype (return_type);
      is_unsigned = PST_IsUnsignedType(PL_symtab, return_type);

      PL_new_register (&dest, PL_next_reg_id (), mtype_type, is_unsigned);

      sprintf (line, "$P%d", M_return_register (mtype_type, M_GET_FNVAR));

      PL_new_macro (&src1, line, mtype_type, is_unsigned);

      /* 01/02/03 REK If the return value is an integer, cast it to a long. 
       *              This makes sure that it is properly sign extended, as
       *              the Software Conventions and Runtime Guide only
       *              guarantees the return value is sign extended to 32 bits.
       */
      if (!M_return_value_thru_stack() && !rv_is_longlong) 
        {
	  if (is_integer (mtype_type))
	    {
	      PL_cast (cb, &dest, &src1, M_TYPE_LONG, is_unsigned);
	    }
	  else
	    {
	      PL_gen_mov (cb, &dest, &src1, attr);
	    }
	}
      else if(rv_is_longlong) {
	  PL_new_macro(&src1, temp_base_macro, M_TYPE_LLONG, 0);
	  PL_new_int(&src2, M_return_value_offset(), 0);
	  PL_gen_load(cb, expr, &dest, &src1, &src2, mtype.unsign, 0);
      }
      else
	{
	  PL_new_macro(&src1, temp_base_macro, M_TYPE_POINTER, 0);
	  PL_new_int(&src2, M_return_value_offset(), 0);
	  PL_gen_load(cb, expr, &dest, &src1, &src2, mtype.unsign, 0);
	}

      result->type = PL_RET_SIMPLE;
      result->op1 = dest;
      PL_new_int (&(result->op2), 0, 0);
    }
  else if (!rv_is_indir_aggr)
    {
      /* An aggregate not passed by indirection!  We need to perform the
       * assignment from return registers into the LHS buffer.  This
       * sort of defeats the purpose of passing these through registers,
       * but we'll have to live with it for now!
       */
      char name[32];
      _PL_Operand opd_ofst, opd_val;
      int tsize = PL_MType_Size(PL_native_int_reg_mtype) / 8,
	offset = 0, r;

      P_warn ("Returning an aggr through register(s) at %s() jsr op %d",
	      L_fn->name, jsr_op->id);

      if (!PLI_func_return_struct)
	P_punt ("PLI_gen_call: structure return address is NULL!");

      for (r = rv_param.su_sreg; r <= rv_param.su_ereg; r++)
	{
	  sprintf (name, "$P%d", M_return_register (M_TYPE_INT, M_PUT_FNVAR));
	  PL_new_macro (&opd_val, name, PL_native_int_reg_mtype, 0);
	  PL_new_int (&opd_ofst, offset, 0);
	  /* JWS - at a loss for a shadow expr here */
	  PL_gen_store (cb, expr,
			PLI_func_return_struct, &opd_ofst, &opd_val,
			opd_val.data_type,
			L_new_attr ("RVSPILL", 0));
	  offset += tsize;
	}

      {
	int i, nreg = rv_param.su_ereg - rv_param.su_sreg + 1;
      
	if (!tr_attr)
	  {
	    tr_attr = L_new_attr ("tr", 0);
	    jsr_op->attr = L_concat_attr (jsr_op->attr, tr_attr);
	  }

	for (i = 0; i < nreg; i++)
	  L_set_macro_attr_field (tr_attr, tr_attr->max_field, 
				  L_MAC_P0 + rv_param.su_sreg + i,
				  PL_native_int_reg_ctype,
				  L_PTYPE_NULL);
      }

      PLI_func_return_struct = NULL;
    }

  return;
}


static void
PL_assign_to (L_Cb * cb, Expr lhs_expr, PL_Ret rhs, PL_Ret ret)
{
  /* Note: can't handle rhs being a call, assumes rhs has been simplified */

  int in_reg, is_bit_field;
  _PL_Ret x;
  L_Attr *attr = NULL;

  PLI_ret_reset (&x);

  PL_assert_not_AF(lhs_expr);

  /*
   *  generate the lhs_expr.
   */

  in_reg = (PLI_gen_addr (cb, lhs_expr, &x) == PLI_VAR_IN_REGISTER);
  is_bit_field = PST_IsBitFieldExpr(PL_symtab, lhs_expr);

  /*
   *  generate the assignment.
   */
  if (PL_is_aggr_type (PST_ExprType (PL_symtab, lhs_expr), NULL))
    {
      /* aggregate assignment -- generate a block move */
      int type_size, type_align;
      Key tmp_type;
      
      tmp_type = PST_ExprType(PL_symtab, lhs_expr);
      type_size = PL_key_get_size (tmp_type);
      type_align = PL_key_get_align (tmp_type);

      PLI_simplify (cb, &x);
      if (lhs_expr->pragma)
	attr = PL_gen_attr_from_pragma (lhs_expr->pragma);
      PL_gen_block_mov (cb, lhs_expr, &(x.op1), 0, &(rhs->op1), 0, 
			type_size, type_align,
			attr, 0, 0, 1);
    }
  else if (in_reg)
    {
      PLI_simplify (cb, &x);
      if (lhs_expr->pragma)
	attr = PL_gen_attr_from_pragma (lhs_expr->pragma);
      PL_gen_mov (cb, &(x.op1), &(rhs->op1), attr);
    }
  else if (is_bit_field)
    {
      PLI_gen_store_bitfield (cb, lhs_expr, &x, rhs, 
			      PST_ExprType (PL_symtab, lhs_expr->operands));
    }
  else
    {
      PLI_gen_store (cb, lhs_expr, &x, rhs, PST_ExprType(PL_symtab, lhs_expr));
    }

  *ret = *rhs;
  return;
}


static void
PLI_gen_assign (L_Cb * cb, Expr expr, PL_Ret ret)
{
  int is_structure, in_reg = 0, is_bit_field;
  Expr LHS, RHS;
  _PL_Ret x, y;
  int LHS_addr = 0;
  Type t;

  LHS = expr->operands;
  RHS = expr->operands->sibling;

  PLI_ret_reset (&x);
  PLI_ret_reset (&y);

  t = PST_ExprType (PL_symtab, expr);

  is_structure = PL_is_aggr_type (t, &t);

  PL_assert_not_AF(LHS);

  /*
   * generate the RHS.
   * ----------------------------------------------------------------------
   * There is some ambiguity here: *x = (x=&y) depends on whether RHS
   * or LHS is evaluated first.  I choose RHS first (so consistent
   * when x in register), unless the RHS is a call returning a
   * structure, in which case the LHS must be evaluated first.
   *
   * It is unclear given current flattening that this is required to
   * be the case. (JWS)
   */

  if (is_structure && (RHS->opcode == OP_call))
    {
      LHS_addr = 1;
      in_reg = (PLI_gen_addr (cb, LHS, &x) == PLI_VAR_IN_REGISTER);
      PLI_simplify (cb, &x);
      PLI_func_return_struct = &(x.op1);
    }

  PLI_gen_data (cb, RHS, &y);
  PLI_simplify (cb, &y);

  /*
   *  generate the LHS.
   */

  if (!LHS_addr)
    in_reg = (PLI_gen_addr (cb, LHS, &x) == PLI_VAR_IN_REGISTER);

  is_bit_field = PST_IsBitFieldExpr(PL_symtab, LHS);

  /*
   *  generate the assignment.
   */

  if (is_structure)
    {
      /* REH 09/05/94 - modified to eliminate need for alloc */
      if (RHS->opcode != OP_call)
	{
	  int type_size, type_align;

	  type_size = PL_key_get_size (t);
	  type_align = PL_key_get_align (t);

	  PLI_simplify (cb, &x);
	  PL_gen_block_mov (cb, LHS, &(x.op1), 0, &(y.op1), 0, 
			    type_size, type_align,
			    expr->pragma ?
			    PL_gen_attr_from_pragma (expr->pragma) : NULL,
			    0, 0, 1);
	}
    }
  else if (in_reg)
    {
      PLI_simplify (cb, &x);
      PL_gen_mov (cb, &(x.op1), &(y.op1),
		  expr->pragma ? PL_gen_attr_from_pragma (expr->pragma) :
		  NULL);
    }
  else if (is_bit_field)
    {
      PLI_gen_store_bitfield(cb, LHS, &x, &y,
			     PST_ExprType(PL_symtab, LHS->operands));
    }
  else
    {
      PLI_gen_store (cb, LHS, &x, &y, PST_ExprType(PL_symtab, expr));
    }

  *ret = y;

  return;
}


static void
PLI_gen_incr (L_Cb * cb, Expr expr, PL_Ret ret)
{
  Expr op1;
  int increment, in_reg, opcode;
  _PL_Ret x, y;
  _PL_Operand dest, src1, src2, result;
  int is_bit_field;
  L_Attr *attr = NULL;
  int mtype_type, is_unsigned;
  Key tmp_type;

  PLI_ret_reset (&x);
  PLI_ret_reset (&y);
  PLI_ret_reset (ret);
  op1 = expr->operands;
  if (PST_IsStructureTypeExpr (PL_symtab, op1))
    P_punt ("PLI_gen_incr: cannot pre/post increment a structure");
  /*
   *  determine the increment amount.
   */

  if (PST_IsPointerTypeExpr (PL_symtab, op1))
    {
      Key type = PST_ExprType(PL_symtab, op1);
      increment = PL_key_get_size (PST_GetTypeType(PL_symtab,
						   type));
    }
  else
    {
      increment = 1;
    }

  /*
   *  compute the address of x.
   */
  in_reg = (PLI_gen_addr (cb, op1, &x) == PLI_VAR_IN_REGISTER);
  is_bit_field = PST_IsBitFieldExpr(PL_symtab, op1);

  /*
   *  get the original value of x.
   */
  tmp_type = PST_ExprType(PL_symtab, expr);
  mtype_type = PL_key_get_mtype (tmp_type);
  is_unsigned = PST_IsUnsignedType(PL_symtab, tmp_type);
  

/* BCC - mtype sould not be changed by M_eval_type in PLI_gen_addr_var()  
   sometimes char/short types are changed to int in PLI_gen_addr_var() 7/10/95 */

  if (x.type == PL_RET_SIMPLE)
    {
      /* BCC - bug fix - 4/14/96
       * e.g. (*db)++
       * The final type for the above expression is a double but we
       * shouldn't change the type for db to be double. The type of db
       * should stay to be INT or whatever it was. We only do the
       * conversion when the mtype.type is integer type.
       */
      if (is_integer (mtype_type) && (x.op1.data_type != M_TYPE_POINTER))
	x.op1.data_type = mtype_type;
    }
  if (in_reg)
    {
      if (is_bit_field)
	P_punt ("bit field cannot be in register");
      y = x;
    }
  else if (is_bit_field)
    {
      PLI_gen_load_bitfield (cb, op1, &x, &y,
			     PST_ExprType (PL_symtab, op1->operands));
    }
  else
    {
      PLI_gen_load (cb, op1, &x, &y, PST_ExprType(PL_symtab, op1));
    }

  PLI_simplify (cb, &y);
  if (y.op1.type == PL_R)
    result = y.op1;
  else
    PL_new_register (&result, PL_next_reg_id (), mtype_type, is_unsigned);

  /*
   *  generate the operation.
   */
  src1 = y.op1;
  PL_new_int (&src2, increment, 0);
  opcode = expr->opcode;

  attr = expr->pragma ? PL_gen_attr_from_pragma (expr->pragma) : NULL;

  if (opcode == OP_preinc)
    {
      /* result = reg = reg + increment */
      PL_gen_add (cb, &result, &src1, &src2, is_unsigned, attr);
      dest = result;
    }
  else if (opcode == OP_predec)
    {
      PL_gen_sub (cb, &result, &src1, &src2, is_unsigned, attr);
      dest = result;
    }
  else if (opcode == OP_postinc)
    {
      /* result = reg; reg = reg + increment */

      PL_new_register (&dest, PL_next_reg_id (), mtype_type, is_unsigned);

      PL_gen_mov (cb, &dest, &src1, attr);
      PL_gen_add (cb, &result, &src1, &src2,
		  is_unsigned, L_copy_attr (attr));
    }
  else if (opcode == OP_postdec)
    {
      PL_new_register (&dest, PL_next_reg_id (), mtype_type, is_unsigned);

      PL_gen_mov (cb, &dest, &src1, attr);
      PL_gen_sub (cb, &result, &src1, &src2,
		  is_unsigned, L_copy_attr (attr));
    }
  else
    {
      P_punt ("PLI_gen_incr: illegal opcode");
    }
  /*
   *  update x.
   */
  if (!in_reg)
    {
      if (is_bit_field)
	{
	  y.op1 = result;
	  y.type = PL_RET_SIMPLE;
	  PLI_gen_store_bitfield(cb, op1, &x, &y,
				 PST_ExprType(PL_symtab, op1->operands));
	}
      else
	{
	  y.op1 = result;
	  y.type = PL_RET_SIMPLE;
	  PLI_gen_store (cb, op1, &x, &y, PST_ExprType(PL_symtab, op1));
	}
    }
  ret->type = PL_RET_SIMPLE;
  ret->op1 = dest;
  return;
}


static void
PLI_gen_Aarith (L_Cb * cb, Expr expr, PL_Ret ret)
{
  int increment, opcode, in_reg;
  Expr LHS, RHS;
  _PL_Ret x, y, addr;
  _PL_Operand dest, src1, src2, tm, inc;
  int lhs_is_ptr, rhs_is_ptr, is_bit_field, cast_result = 0;
  L_Attr *attr = NULL;
  Key tmp_type;
  int mtype_type, is_unsigned;

  PLI_ret_reset (&x);
  PLI_ret_reset (&y);
  PLI_ret_reset (&addr);
  opcode = expr->opcode;
  LHS = expr->operands;
  RHS = expr->operands->sibling;

  PL_assert_not_AF(LHS);

  lhs_is_ptr = (PST_IsPointerTypeExpr (PL_symtab, LHS) || 
		PST_IsArrayTypeExpr (PL_symtab, LHS));
  rhs_is_ptr = (PST_IsPointerTypeExpr (PL_symtab, RHS) || 
		PST_IsArrayTypeExpr (PL_symtab, RHS));

  if (PST_IsStructureTypeExpr (PL_symtab, expr))
    P_punt ("sorry, we do not support op= on structures");

  /*
   *  generate the RHS.
   */
  PLI_gen_data (cb, RHS, &y);
  PLI_simplify (cb, &y);
  /*
   *  generate the LHS.
   */
  tmp_type = PST_ExprType(PL_symtab, expr);
  mtype_type = PL_key_get_mtype (tmp_type);
  is_unsigned = PST_IsUnsignedType(PL_symtab, tmp_type);  
  in_reg = (PLI_gen_addr (cb, LHS, &x) == PLI_VAR_IN_REGISTER);
  is_bit_field = PST_IsBitFieldExpr(PL_symtab, LHS);

  PLI_simplify (cb, &x);
  addr = x;
  if (in_reg)
    {
      if (is_bit_field)
	P_punt ("bit field cannot be in register");
    }
  else if (is_bit_field)
    {
      PLI_gen_load_bitfield (cb, LHS, &addr, &x,
			     PST_ExprType (PL_symtab, LHS->operands));     
    }
  else
    {
      PLI_gen_load (cb, expr->operands, &addr, &x, 
		    PST_ExprType(PL_symtab, LHS));
    }

  /*
   *  find out what the increment is.
   */

  if (((opcode == OP_Aadd) || (opcode == OP_Asub)) &&
      (PST_IsPointerTypeExpr (PL_symtab, LHS) || 
       PST_IsArrayTypeExpr (PL_symtab, LHS)))
    {
      Key type = PST_ExprType(PL_symtab, LHS);
      increment = PL_key_get_size (PST_GetTypeType(PL_symtab,
						   type));
    }
  else
    {
      increment = 1;
    }

  /*
   *  evaluate.
   */

  if (in_reg)
    dest = addr.op1;
  else
    PL_new_register (&dest, PL_next_reg_id (), mtype_type, is_unsigned);

  src1 = x.op1;
  src2 = y.op1;

  switch (opcode)
    {
    case OP_Aadd:
      /*
       *  dest = src1 + (src2 * increment).
       */
      if (increment != 1)
	{
	  if (src2.type == PL_I)
	    {
	      PL_new_int (&tm, src2.value.i * increment, 0);
	    }
	  else
	    {
	      PL_new_register (&tm, PL_next_reg_id (),
			       PL_native_int_reg_mtype, 0);
	      PL_new_int (&inc, increment, 0);
	      attr = expr->pragma ? PL_gen_attr_from_pragma (expr->pragma) :
		NULL;
	      PL_gen_mul (cb, &tm, &src2, &inc, NULL, 0, attr);
	    }
	}
      else
	{
	  tm = src2;
	}
      attr = expr->pragma ? PL_gen_attr_from_pragma (expr->pragma) : NULL;
      PL_gen_add (cb, &dest, &src1, &tm, is_unsigned, attr);
      /* BCC - in a += 5; if a is not a pointer, we have to mask the result */
      if (!lhs_is_ptr)
	cast_result = 1;
      break;
    case OP_Asub:
      if (lhs_is_ptr & rhs_is_ptr)
	{
	  P_punt ("sorry, (pointer -= pointer) is not allowed");
	  /*
	   *  dest = (src1 - src2) / increment.
	   */
	  if (increment == 0)
	    P_punt ("cannot apply -= on (void*)");
	  attr = expr->pragma ? PL_gen_attr_from_pragma (expr->pragma) : NULL;
	  PL_gen_sub (cb, &dest, &src1, &src2, is_unsigned, attr);
	  if (increment != 1)
	    {
	      tm = dest;
	      PL_new_int (&inc, increment, 0);
	      if (expr->pragma != 0)
		attr = PL_gen_attr_from_pragma (expr->pragma);
	      PL_gen_div (cb, &dest, &tm, 
			  &inc, -1, 0, attr);
	    }
	}
      else
	{
	  /*
	   *  dest = src1 - (src2 * increment).
	   */
	  if (increment != 1)
	    {
	      if (src2.type == PL_I)
		{
		  PL_new_int (&tm, src2.value.i * increment, 0);
		}
	      else
		{
		  PL_new_register (&tm, PL_next_reg_id (),
				   PL_native_int_reg_mtype, 0);
		  PL_new_int (&inc, increment, 0);
		  attr = expr->pragma ?
		    PL_gen_attr_from_pragma (expr->pragma) : NULL;
		  PL_gen_mul (cb, &tm, &src2, &inc, NULL, 1, attr);
		}
	    }
	  else
	    {
	      tm = src2;
	    }
	  attr = expr->pragma ? PL_gen_attr_from_pragma (expr->pragma) : NULL;
	  PL_gen_sub (cb, &dest, &src1, &tm, is_unsigned, attr);
	}
      /* BCC - in a -= 5; if a is not a pointer, we have to mask the result */
      if (!lhs_is_ptr)
	cast_result = 1;
      break;
    case OP_Amul:
      attr = expr->pragma ? PL_gen_attr_from_pragma (expr->pragma) : NULL;
      PL_gen_mul (cb, &dest, &src1, &src2, NULL, is_unsigned, attr);
      /* BCC - need a second pass to mask the result */
      cast_result = 1;
      break;
    case OP_Adiv:
      attr = expr->pragma ? PL_gen_attr_from_pragma (expr->pragma) : NULL;
      PL_gen_div (cb, &dest, &src1, &src2, mtype_type, is_unsigned, attr);
      /* BCC - need a second pass to mask the result */
      cast_result = 1;
      break;
    case OP_Amod:
      attr = expr->pragma ? PL_gen_attr_from_pragma (expr->pragma) : NULL;
      PL_gen_mod (cb, &dest, &src1, &src2, mtype_type, is_unsigned, attr);
      break;
    case OP_Arshft:
      attr = expr->pragma ? PL_gen_attr_from_pragma (expr->pragma) : NULL;
      if (is_unsigned)
	PL_gen_lsr (cb, &dest, &src1, &src2, attr);
      else
	PL_gen_asr (cb, &dest, &src1, &src2, attr);
      cast_result = 1;
      break;
    case OP_Alshft:
      attr = expr->pragma ? PL_gen_attr_from_pragma (expr->pragma) : NULL;
      PL_gen_lsl (cb, &dest, &src1, &src2, attr);
      cast_result = 1;
      break;
    case OP_Aand:
      attr = expr->pragma ? PL_gen_attr_from_pragma (expr->pragma) : NULL;
      PL_gen_logic (cb, PL_RET_AND, &dest, &src1, &src2, attr);
      cast_result = 1;
      break;
    case OP_Aor:
      attr = expr->pragma ? PL_gen_attr_from_pragma (expr->pragma) : NULL;
      PL_gen_logic (cb, PL_RET_OR, &dest, &src1, &src2, attr);
      cast_result = 1;
      break;
    case OP_Axor:
      attr = expr->pragma ? PL_gen_attr_from_pragma (expr->pragma) : NULL;
      PL_gen_logic (cb, PL_RET_XOR, &dest, &src1, &src2, attr);
      cast_result = 1;
      break;
    default:
      P_punt ("gen_Aop: illegal opcode");
    }

  if (cast_result)
    {
      if (mtype_type == M_TYPE_CHAR)
	{
	  dest.data_type = M_TYPE_CHAR;
	  PL_cast (cb, &dest, &dest, M_TYPE_CHAR, is_unsigned);
	}
      else if (mtype_type == M_TYPE_SHORT)
	{
	  dest.data_type = M_TYPE_SHORT;
	  PL_cast (cb, &dest, &dest, M_TYPE_SHORT, is_unsigned);
	}
      else if (mtype_type == M_TYPE_INT &&
	       PL_native_int_reg_mtype != M_TYPE_INT)
	{
	  dest.data_type = M_TYPE_INT;
	  PL_cast (cb, &dest, &dest, M_TYPE_INT, is_unsigned);
	}
    }

  /*
   *  update result.
   */

  ret->type = PL_RET_SIMPLE;
  ret->op1 = dest;

  if (!in_reg)
    {
      if (is_bit_field)
	PLI_gen_store_bitfield (cb, LHS, &addr, ret,
				PST_ExprType(PL_symtab, LHS->operands));
      else
	PLI_gen_store (cb, expr->operands, &addr, ret, 
		       PST_ExprType(PL_symtab, expr));
    }
  return;
}


static void
PLI_gen_logic (L_Cb * cb, Expr expr, PL_Ret ret)
{
  Expr op1, op2;
  _PL_Ret x, y;

  op1 = expr->operands;
  PLI_gen_data (cb, op1, &x);
  PLI_simplify (cb, &x);

  op2 = expr->operands->sibling;
  PLI_gen_data (cb, op2, &y);
  PLI_simplify (cb, &y);

  switch (expr->opcode)
    {
    case OP_or:
      ret->type = PL_RET_OR;
      break;
    case OP_xor:
      ret->type = PL_RET_XOR;
      break;
    case OP_and:
      ret->type = PL_RET_AND;
      break;
    default:
      P_punt ("PLI_gen_logic: illegal opcode");
    }
  ret->op1 = x.op1;
  ret->op2 = y.op1;
  return;
}


static void
PLI_gen_compare (L_Cb * cb, Expr expr, PL_Ret ret)
{
  Expr op1, op2;
  _PL_Ret x, y;
  int is_unsign, is_float;
  L_Attr *attr = NULL;

  op1 = expr->operands;
  PLI_gen_data (cb, op1, &x);
  PLI_simplify (cb, &x);

  op2 = expr->operands->sibling;
  PLI_gen_data (cb, op2, &y);
  PLI_simplify (cb, &y);

  /* JWS -- does this follow ISO default promotion? */

  is_unsign = (PST_IsUnsignedTypeExpr(PL_symtab, op1) ||
	       PST_IsUnsignedTypeExpr(PL_symtab, op2) ||
	       PST_IsPointerTypeExpr(PL_symtab, op1) ||
	       PST_IsPointerTypeExpr(PL_symtab, op2));

  is_float = (PST_IsRealTypeExpr (PL_symtab, op1) || 
	      PST_IsRealTypeExpr (PL_symtab, op2));

  if (is_float)
    {
      _PL_Operand dest, src1, src2;
      src1 = x.op1;
      src2 = y.op1;

      /* BCC - use an integer register to hold the result of 
       * comparison - 6/19/95 */

      PL_new_register (&dest, PL_next_reg_id (), M_TYPE_INT, 0);

      if (expr->pragma != 0)
	attr = PL_gen_attr_from_pragma (expr->pragma);

      switch (expr->opcode)
	{
	case OP_eq:
	  PL_gen_rcmp (cb, &dest, &src1, &src2, Lcmp_COM_EQ, 0, attr);
	  break;
	case OP_ne:
	  PL_gen_rcmp (cb, &dest, &src1, &src2, Lcmp_COM_NE, 0, attr);
	  break;
	case OP_gt:
	  PL_gen_rcmp (cb, &dest, &src1, &src2, Lcmp_COM_GT, is_unsign, attr);
	  break;
	case OP_ge:
	  PL_gen_rcmp (cb, &dest, &src1, &src2, Lcmp_COM_GE, is_unsign, attr);
	  break;
	case OP_lt:
	  PL_gen_rcmp (cb, &dest, &src1, &src2, Lcmp_COM_LT, is_unsign, attr);
	  break;
	case OP_le:
	  PL_gen_rcmp (cb, &dest, &src1, &src2, Lcmp_COM_LE, is_unsign, attr);
	  break;
	default:
	  P_punt ("PLI_gen_compare: illegal opcode");
	}
      ret->type = PL_RET_SIMPLE;
      ret->op1 = dest;
    }
  else
    {
      switch (expr->opcode)
	{
	case OP_eq:
	  ret->type = PL_RET_EQ;
	  break;
	case OP_ne:
	  ret->type = PL_RET_NE;
	  break;
	case OP_gt:
	  ret->type = is_unsign ? PL_RET_GT_U : PL_RET_GT;
	  break;
	case OP_ge:
	  ret->type = is_unsign ? PL_RET_GE_U : PL_RET_GE;
	  break;
	case OP_lt:
	  ret->type = is_unsign ? PL_RET_LT_U : PL_RET_LT;
	  break;
	case OP_le:
	  ret->type = is_unsign ? PL_RET_LE_U : PL_RET_LE;
	  break;
	default:
	  P_punt ("PLI_gen_compare: illegal opcode");
	}
      ret->op1 = x.op1;
      ret->op2 = y.op1;
    }
  return;
}


static void
PLI_gen_shift (L_Cb * cb, Expr expr, PL_Ret ret)
{
  Expr op1, op2;
  _PL_Ret x, y;
  _PL_Operand dest, src1, src2;
  L_Attr *attr = NULL;
  Key tmp_type;
  int mtype_type, is_unsigned;
  int is_unsigned2;

  op1 = expr->operands;
  PLI_gen_data (cb, op1, &x);
  PLI_simplify (cb, &x);

  op2 = expr->operands->sibling;
  PLI_gen_data (cb, op2, &y);
  PLI_simplify (cb, &y);

  tmp_type = PST_ExprType(PL_symtab, expr);
  mtype_type = PL_key_get_mtype (tmp_type);
  is_unsigned = PST_IsUnsignedType(PL_symtab,tmp_type);

  tmp_type = PST_ExprType(PL_symtab, expr->operands);
  is_unsigned2 = PST_IsUnsignedType(PL_symtab,tmp_type);

  src1 = x.op1;
  src2 = y.op1;

  PL_new_register (&dest, PL_next_reg_id (), mtype_type, is_unsigned);

  if (expr->pragma != 0)
    attr = PL_gen_attr_from_pragma (expr->pragma);
  switch (expr->opcode)
    {
    case OP_rshft:
      if (is_unsigned2)
	PL_gen_lsr (cb, &dest, &src1, &src2, attr);
      else
	PL_gen_asr (cb, &dest, &src1, &src2, attr);
      break;
    case OP_lshft:
      PL_gen_lsl (cb, &dest, &src1, &src2, attr);
      break;
    default:
      P_punt ("gen_shift: illegal opcode");
    }

  ret->type = PL_RET_SIMPLE;
  ret->op1 = dest;
  return;
}


/*
 * PLI_gen_add
 * ----------------------------------------------------------------------
 * Simplify operands.  If result type is not a pointer, return simple
 * result; else return PL_ADD
 */
static void
PLI_gen_add (L_Cb * cb, Expr expr, PL_Ret ret)
{
  Expr op1, op2;
  _PL_Ret x, y;
  _PL_Operand dest;
  int is_ptr, increment;
  Key tmp_type;
  int mtype_type, is_unsigned;

  op1 = expr->operands;
  op2 = expr->operands->sibling;

  tmp_type = PST_ExprType(PL_symtab, expr);
  mtype_type = PL_key_get_mtype (tmp_type);
  is_unsigned = PST_IsUnsignedType(PL_symtab, tmp_type);

  /* If one operand is a pointer, ensure it is the first */

  is_ptr = 0;
  if (PST_IsPointerTypeExpr (PL_symtab, op1) || 
      PST_IsArrayTypeExpr (PL_symtab, op1))
    {
      is_ptr = 1;
      if (PST_IsPointerTypeExpr (PL_symtab, op2) || 
	  PST_IsArrayTypeExpr (PL_symtab, op2))
	P_punt ("PLI_gen_add: cannot add two pointers");
    }
  else if (PST_IsPointerTypeExpr (PL_symtab, op2) || 
	   PST_IsArrayTypeExpr (PL_symtab, op2))
    {
      Expr temp = op1;
      op1 = op2;
      op2 = temp;
      is_ptr = 1;
    }

  PLI_gen_data (cb, op1, &x);
  PLI_simplify (cb, &x);

  PLI_gen_data (cb, op2, &y);
  PLI_simplify (cb, &y);

  if (is_ptr)
    {
      Key type = PST_ExprType(PL_symtab, op1);
      increment = PL_key_get_size (PST_GetTypeType(PL_symtab,
						   type));
      if (increment != 1)
	PLI_ret_mulC (cb, &y, increment);

      PLI_ret_add (cb, ret, &x, &y);

      /* result may not be simple! */
    }
  else
    {
      PL_new_register (&dest, PL_next_reg_id (), mtype_type, is_unsigned);

      PL_gen_add (cb, &dest, &x.op1, &y.op1, is_unsigned, expr->pragma ?
		  PL_gen_attr_from_pragma (expr->pragma) : NULL);

      ret->type = PL_RET_SIMPLE;
      ret->op1 = dest;
    }

  return;
}


static void
PLI_gen_sub (L_Cb * cb, Expr expr, PL_Ret ret)
{
  Expr op1, op2;
  _PL_Ret x, y;
  int op1_is_ptr, op2_is_ptr;
  _PL_Operand dest, src1, src2, temp, inc;
  int increment;
  Key tmp_type;
  int mtype_type, is_unsigned;

  op1 = expr->operands;
  op2 = expr->operands->sibling;

  op1_is_ptr = (PST_IsPointerTypeExpr (PL_symtab, op1) || 
		PST_IsArrayTypeExpr (PL_symtab, op1));
  op2_is_ptr = (PST_IsPointerTypeExpr (PL_symtab, op2) || 
		PST_IsArrayTypeExpr (PL_symtab, op2));

  if (op2_is_ptr && !op1_is_ptr)
    P_punt ("x-y: y cannot be a pointer, if x is not");

  PLI_gen_data (cb, op1, &x);
  PLI_simplify (cb, &x);

  PLI_gen_data (cb, op2, &y);
  PLI_simplify (cb, &y);

  increment = 1;

  if (op1_is_ptr)
    {
      Key type = PST_ExprType(PL_symtab, op1);
      increment = PL_key_get_size (PST_GetTypeType(PL_symtab,
						   type));
      if (!op2_is_ptr && (increment != 1))
	PLI_ret_mulC (cb, &y, increment);
    }

  src1 = x.op1;
  src2 = y.op1;

  tmp_type = PST_ExprType(PL_symtab, expr);
  mtype_type = PL_key_get_mtype (tmp_type);
  is_unsigned = PST_IsUnsignedType(PL_symtab, tmp_type);

  PL_new_register (&dest, PL_next_reg_id (), mtype_type, is_unsigned);

  PL_gen_sub (cb, &dest, &src1, &src2, is_unsigned, expr->pragma ?
	      PL_gen_attr_from_pragma (expr->pragma) : NULL);

  if (op1_is_ptr && op2_is_ptr && (increment != 1))
    {
      temp = dest;
      PL_new_int (&inc, increment, 0);
      PL_gen_div (cb, &dest, &temp, &inc, -1, 0, NULL);
    }

  ret->type = PL_RET_SIMPLE;
  ret->op1 = dest;
  return;
}


static void
PLI_gen_arith (L_Cb * cb, Expr expr, PL_Ret ret)
{
  Expr op1, op2;
  _PL_Ret x, y;
  _PL_Operand dest, src1, src2;
  L_Attr *attr = NULL;
  Key tmp_type;
  int mtype_type, is_unsigned;

  op1 = expr->operands;

  PLI_gen_data (cb, op1, &x);
  PLI_simplify (cb, &x);
  src1 = x.op1;

  op2 = expr->operands->sibling;
  PLI_gen_data (cb, op2, &y);
  PLI_simplify (cb, &y);
  src2 = y.op1;

  tmp_type = PST_ExprType(PL_symtab, expr);
  mtype_type = PL_key_get_mtype (tmp_type);
  is_unsigned = PST_IsUnsignedType(PL_symtab, tmp_type);

  PL_new_register (&dest, PL_next_reg_id (), mtype_type, is_unsigned);

  attr = expr->pragma ? PL_gen_attr_from_pragma (expr->pragma) : NULL;

  switch (expr->opcode)
    {
    case OP_mul:
      PL_gen_mul (cb, &dest, &src1, &src2, NULL, is_unsigned, attr);
      break;
    case OP_div:
      PL_gen_div (cb, &dest, &src1, &src2, mtype_type, is_unsigned, attr);
      break;
    case OP_mod:
      PL_gen_mod (cb, &dest, &src1, &src2, mtype_type, is_unsigned, attr);
      break;
    default:
      P_punt ("PLI_gen_arith: unhandled opcode %d", expr->opcode);
    }

  ret->type = PL_RET_SIMPLE;
  ret->op1 = dest;
  return;
}


static void
PLI_gen_unary (L_Cb * cb, Expr expr, PL_Ret ret)
{
  int type;
  Expr op1;
  _PL_Ret x;
  _PL_Operand dest, src1, zero, all;
  L_Attr *attr = NULL;
  Key tmp_type;
  int mtype_type, is_unsigned;

  op1 = expr->operands;
  PLI_gen_data (cb, op1, &x);
  PLI_simplify (cb, &x);
  src1 = x.op1;

  tmp_type = PST_ExprType(PL_symtab, expr);
  mtype_type = PL_key_get_mtype (tmp_type);
  is_unsigned = PST_IsUnsignedType(PL_symtab, tmp_type);

  type = eval_mtype (mtype_type);

  if (src1.data_type == M_TYPE_FLOAT)
    PL_new_float (&zero, (double) 0.0);
  else if (src1.data_type == M_TYPE_DOUBLE)
    PL_new_double (&zero, (double) 0.0);
  else if (src1.data_type == M_TYPE_LONG)
    PL_new_long (&zero, 0, 0);
  else
    PL_new_int_const (&zero, 0, M_TYPE_INT, 0);

  PL_new_int_const (&all, ~0, M_TYPE_INT, 0);

  attr = expr->pragma ? PL_gen_attr_from_pragma (expr->pragma) : NULL;

  switch (expr->opcode)
    {
    case OP_not:		/* logical */
      PL_new_register (&dest, PL_next_reg_id (), M_TYPE_INT, 0);
      PL_gen_rcmp (cb, &dest, &zero, &src1, Lcmp_COM_EQ, is_unsigned, attr);
      break;
    case OP_neg:		/* arithmetic */
      PL_new_register (&dest, PL_next_reg_id (), type, is_unsigned);
      PL_gen_sub (cb, &dest, &zero, &src1, is_unsigned, attr);
      break;
    case OP_inv:		/* boolean */
      PL_new_register (&dest, PL_next_reg_id (), type, is_unsigned);
      PL_gen_logic (cb, PL_RET_XOR, &dest, &all, &src1, attr);
      break;
    default:
      P_punt ("PLI_gen_unary: unhandled opcode %d", expr->opcode);
    }
  ret->type = PL_RET_SIMPLE;
  ret->op1 = dest;
  return;
}
