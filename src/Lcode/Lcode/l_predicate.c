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
 *      File :          l_predicate.c
 *      Description :   Generic Lcode functions handling predicates
 *      Creation Date : November 1993
 *      Author :        Scott Mahlke, David August, John Sias, Wen-mei Hwu
 *
 *==========================================================================*/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>

#define ERR stderr
#define DEBUG_DEMOTION
#undef DEBUG_UNCOND
#undef DEBUG_COMBINE
#undef DEBUG_SPLIT

/*
 * Predicate define types (reproduced from l_code.h)
 * ---------------------------------------------------------------------------
 * Values written ('-' denotes preservation of the previous value)
 *
 *  (Pg,C)                    (0,0)   (0,1)   (1,0)   (1,1)
 * +-------------------+----+-------+-------+-------+-------+
 * | L_PTYPE_NULL      |    |       |       |       |       |
 * | L_PTYPE_UNCOND_T  | ut |   0   |   0   |   0   |   1   |
 * | L_PTYPE_UNCOND_F  | uf |   0   |   0   |   1   |   0   |
 * | L_PTYPE_COND_T    | ct |   -   |   -   |   0   |   1   |
 * | L_PTYPE_COND_F    | cf |   -   |   -   |   1   |   0   |
 * | L_PTYPE_OR_T      | ot |   -   |   -   |   -   |   1   |
 * | L_PTYPE_OR_F      | of |   -   |   -   |   1   |   -   |
 * | L_PTYPE_AND_T     | at |   -   |   -   |   0   |   -   |
 * | L_PTYPE_AND_F     | af |   -   |   -   |   -   |   0   |
 * | L_PTYPE_SAND_T    | st |   0   |   0   |   0   |   -   |
 * | L_PTYPE_SAND_F    | sf |   0   |   0   |   -   |   0   |
 * +-------------------+----+-------+-------+-------+-------+
 *
 */

/*===========================================================================*/
/*
 *      L_Attr boolean predicates
 */
/*===========================================================================*/

/*===========================================================================*/
/*
 *      L_Flow boolean predicates
 */
/*===========================================================================*/

/*===========================================================================*/
/*
 *      L_PTYPE boolean predicates
 */
/*===========================================================================*/

/*
 * L_is_update_predicate_ptype
 * ---------------------------------------------------------------------------
 * Return 1 if predicate ptype conditionally writes a single value,
 * rendering it commutative with other applications of the same ptype.
 */

int
L_is_update_predicate_ptype (int ptype)
{
  switch (ptype)
    {
    case L_PTYPE_NULL:
    case L_PTYPE_UNCOND_T:
    case L_PTYPE_UNCOND_F:
    case L_PTYPE_COND_T:
    case L_PTYPE_COND_F:
      return (0);
    case L_PTYPE_OR_T:
    case L_PTYPE_OR_F:
    case L_PTYPE_AND_T:
    case L_PTYPE_AND_F:
    case L_PTYPE_SAND_T:
    case L_PTYPE_SAND_F:
      return (1);
    default:
      L_punt ("L_is_update_predicate_type: illegal predicate type");
    }
  return (0);
}

/*
 * L_is_transparent_predicate_ptype
 * ---------------------------------------------------------------------------
 * Return 1 if predicate ptype writes the destination less than totally.
 */

int
L_is_transparent_predicate_ptype (int ptype)
{
  switch (ptype)
    {
    case L_PTYPE_NULL:
    case L_PTYPE_UNCOND_T:
    case L_PTYPE_UNCOND_F:
      return (0);
    case L_PTYPE_COND_T:
    case L_PTYPE_COND_F:
    case L_PTYPE_OR_T:
    case L_PTYPE_OR_F:
    case L_PTYPE_AND_T:
    case L_PTYPE_AND_F:
    case L_PTYPE_SAND_T:
    case L_PTYPE_SAND_F:
      return (1);
    default:
      L_punt ("L_is_transparent_predicate_type: illegal predicate type");
    }
  return (0);
}

/*
 * L_is_uncond_predicate_ptype
 * ---------------------------------------------------------------------------
 * Return 1 if predicate ptype unconditionally writes a value.
 */

int
L_is_uncond_predicate_ptype (int ptype)
{
  switch (ptype)
    {
    case L_PTYPE_NULL:
    case L_PTYPE_UNCOND_T:
    case L_PTYPE_UNCOND_F:
    case L_PTYPE_SAND_T:
    case L_PTYPE_SAND_F:
      return (1);
    case L_PTYPE_COND_T:
    case L_PTYPE_COND_F:
    case L_PTYPE_OR_T:
    case L_PTYPE_OR_F:
    case L_PTYPE_AND_T:
    case L_PTYPE_AND_F:
      return (0);
    default:
      L_punt ("L_is_uncond_predicate_type: illegal predicate type");
    }
  return (0);
}

int
L_uncond_ptype (int ptype)
{
  switch (ptype)
    {
    case L_PTYPE_UNCOND_T:
    case L_PTYPE_UNCOND_F:
      return 1;
    default:
      return 0;
    }
}

int
L_uncond_true_ptype (int ptype)
{
  switch (ptype)
    {
    case L_PTYPE_UNCOND_T:
      return 1;
    default:
      return 0;
    }
}

int
L_uncond_false_ptype (int ptype)
{
  switch (ptype)
    {
    case L_PTYPE_UNCOND_F:
      return 1;
    default:
      return 0;
    }
}

int
L_cond_ptype (int ptype)
{
  switch (ptype)
    {
    case L_PTYPE_COND_T:
    case L_PTYPE_COND_F:
      return 1;
    default:
      return 0;
    }
}

int
L_cond_true_ptype (int ptype)
{
  switch (ptype)
    {
    case L_PTYPE_COND_T:
      return 1;
    default:
      return 0;
    }
}

int
L_cond_false_ptype (int ptype)
{
  switch (ptype)
    {
    case L_PTYPE_COND_F:
      return 1;
    default:
      return 0;
    }
}

int
L_or_ptype (int ptype)
{
  switch (ptype)
    {
    case L_PTYPE_OR_T:
    case L_PTYPE_OR_F:
      return 1;
    default:
      return 0;
    }
}

int
L_or_true_ptype (int ptype)
{
  switch (ptype)
    {
    case L_PTYPE_OR_T:
      return 1;
    default:
      return 0;
    }
}

int
L_or_false_ptype (int ptype)
{
  switch (ptype)
    {
    case L_PTYPE_OR_F:
      return 1;
    default:
      return 0;
    }
}

int
L_and_ptype (int ptype)
{
  switch (ptype)
    {
    case L_PTYPE_AND_T:
    case L_PTYPE_AND_F:
      return 1;
    default:
      return 0;
    }
}

int
L_and_true_ptype (int ptype)
{
  switch (ptype)
    {
    case L_PTYPE_AND_T:
      return 1;
    default:
      return 0;
    }
}

int
L_and_false_ptype (int ptype)
{
  switch (ptype)
    {
    case L_PTYPE_AND_F:
      return 1;
    default:
      return 0;
    }
}

int
L_sand_ptype (int ptype)
{
  switch (ptype)
    {
    case L_PTYPE_SAND_T:
    case L_PTYPE_SAND_F:
      return 1;
    default:
      return 0;
    }
}

int
L_sand_true_ptype (int ptype)
{
  switch (ptype)
    {
    case L_PTYPE_SAND_T:
      return 1;
    default:
      return 0;
    }
}

int
L_sand_false_ptype (int ptype)
{
  switch (ptype)
    {
    case L_PTYPE_SAND_F:
      return 1;
    default:
      return 0;
    }
}

int
L_false_ptype (int ptype)
{
  return (L_uncond_false_ptype (ptype) || L_cond_false_ptype (ptype) ||
          L_and_false_ptype (ptype) || L_or_false_ptype (ptype) ||
          L_sand_false_ptype (ptype));
}

int
L_opposite_ptype (int ptype)
{
  switch (ptype)
    {
    case L_PTYPE_NULL:
      return (L_PTYPE_NULL);
    case L_PTYPE_UNCOND_T:
      return (L_PTYPE_UNCOND_F);
    case L_PTYPE_UNCOND_F:
      return (L_PTYPE_UNCOND_T);
    case L_PTYPE_COND_T:
      return (L_PTYPE_COND_F);
    case L_PTYPE_COND_F:
      return (L_PTYPE_COND_T);
    case L_PTYPE_OR_T:
      return (L_PTYPE_OR_F);
    case L_PTYPE_OR_F:
      return (L_PTYPE_OR_T);
    case L_PTYPE_AND_T:
      return (L_PTYPE_AND_F);
    case L_PTYPE_AND_F:
      return (L_PTYPE_AND_T);
    case L_PTYPE_SAND_T:
      return (L_PTYPE_SAND_F);
    case L_PTYPE_SAND_F:
      return (L_PTYPE_SAND_T);
    default:
      L_punt ("L_opposite_ptype: illegal ptype");
    }

  return (0);
}

/*===========================================================================*/
/*
 *      L_Operand boolean predicates
 */
/*===========================================================================*/

int
L_is_pred_register (L_Operand * operand)
{
  if (!L_is_register (operand))
    return (0);

  return (L_is_ctype_predicate (operand));
}

int
L_is_pred_macro (L_Operand * operand)
{
  if (!L_is_macro (operand))
    return (0);

  return (L_is_ctype_predicate (operand));
}

int
L_is_pred_variable (L_Operand * operand)
{
  if (!L_is_register (operand) && !L_is_macro (operand))
    return (0);

  return (L_is_ctype_predicate (operand));
}

int
L_is_update_predicate_ptype_operand (L_Operand * operand)
{
  int ptype;

  if (!(L_is_ctype_predicate (operand)))
    return (0);

  ptype = operand->ptype;

  return (L_is_update_predicate_ptype (ptype));
}

int
L_equivalent_ptype (L_Operand * dest1, L_Operand * dest2)
{
  if ((dest1 == NULL) || (dest2 == NULL))
    L_punt ("L_equivalent_ptype: dests cannot be NULL");

  switch (dest1->ptype)
    {
    case L_PTYPE_UNCOND_T:
      return ((dest2->ptype == L_PTYPE_UNCOND_T) ||
              (dest2->ptype == L_PTYPE_OR_T));
    case L_PTYPE_UNCOND_F:
      return (dest2->ptype == L_PTYPE_UNCOND_F);
    case L_PTYPE_COND_T:
      return (dest2->ptype == L_PTYPE_COND_T);
    case L_PTYPE_COND_F:
      return (dest2->ptype == L_PTYPE_COND_F);
    case L_PTYPE_OR_T:
      return ((dest2->ptype == L_PTYPE_OR_T) ||
              (dest2->ptype == L_PTYPE_UNCOND_T));
    case L_PTYPE_OR_F:
      return (dest2->ptype == L_PTYPE_OR_F);
    case L_PTYPE_AND_T:
      return (dest2->ptype == L_PTYPE_AND_T);
    case L_PTYPE_AND_F:
      return (dest2->ptype == L_PTYPE_AND_F);
    case L_PTYPE_SAND_T:
      return (dest2->ptype == L_PTYPE_SAND_T);
    case L_PTYPE_SAND_F:
      return (dest2->ptype == L_PTYPE_SAND_F);

    default:
      L_punt ("L_equivalent_ptype: unexpected ptype\n");
      return (0);
    }
}

int
L_complement_ptype (L_Operand * dest1, L_Operand * dest2)
{
  if ((dest1 == NULL) || (dest2 == NULL))
    L_punt ("L_complement_ptype: dests cannot be NULL");

  switch (dest1->ptype)
    {
    case L_PTYPE_UNCOND_T:
      return (dest2->ptype == L_PTYPE_UNCOND_F);
    case L_PTYPE_UNCOND_F:
      return ((dest2->ptype == L_PTYPE_UNCOND_T) ||
              (dest2->ptype == L_PTYPE_OR_T));
    case L_PTYPE_COND_T:
      return (dest2->ptype == L_PTYPE_COND_F);
    case L_PTYPE_COND_F:
      return (dest2->ptype == L_PTYPE_COND_T);
    case L_PTYPE_OR_T:
      return ((dest2->ptype == L_PTYPE_OR_F) ||
              (dest2->ptype == L_PTYPE_UNCOND_F));
    case L_PTYPE_OR_F:
      return (dest2->ptype == L_PTYPE_OR_T);
    case L_PTYPE_AND_T:
      return (dest2->ptype == L_PTYPE_AND_F);
    case L_PTYPE_AND_F:
      return (dest2->ptype == L_PTYPE_AND_T);
    case L_PTYPE_SAND_T:
      return (dest2->ptype == L_PTYPE_SAND_F);
    case L_PTYPE_SAND_F:
      return (dest2->ptype == L_PTYPE_SAND_T);

    default:
      L_punt ("L_complement_ptype: unexpected ptype\n");
      return (0);
    }
}

int
L_pred_defined_multiple_times (L_Cb * cb, L_Operand * pred)
{
  int count;
  L_Oper *ptr;

  if (pred == NULL)
    L_punt ("L_pred_defined_multiple_times: pred is NULL");

  count = 0;
  for (ptr = cb->first_op; ptr != NULL; ptr = ptr->next_op)
    {
      if (!L_general_pred_comparison_opcode (ptr))
        continue;
      if (L_is_dest_operand (pred, ptr))
        count++;
    }

  return (count > 1);
}

int
L_no_redefs_between (L_Operand * operand, L_Oper * opA, L_Oper * opB)
{
  int i;
  L_Oper *pA;

  if ((opA == NULL) || (opB == NULL))
    L_punt ("L_no_redefs_between: opA and opB cannot be NULL");
  if (operand == NULL)
    return 1;

  if (!L_is_predicated (opA))
    {
      L_punt ("L_no_redefs_between: opA is not predicated");
    }
  else
    {
      for (pA = opA->next_op; pA != NULL; pA = pA->next_op)
        {
          if (pA == opB)
            return 1;
          if (!PG_intersecting_predicates_ops (pA, opA))
            continue;
          for (i = 0; i < L_max_dest_operand; i++)
            {
              if (L_same_operand (pA->dest[i], operand))
                return 0;
            }
        }
    }

  L_punt ("L_no_redefs_between: opB not found");
  return (0);
}

int
L_all_src_operand_no_redefs_between (L_Oper * oper, L_Oper * opA,
                                     L_Oper * opB)
{
  int i;
  for (i = 0; i < L_max_src_operand; i++)
    {
      if (!L_no_redefs_between (oper->src[i], opA, opB))
        return 0;
    }
  return 1;
}

int
L_all_dest_operand_no_redefs_between (L_Oper * oper, L_Oper * opA,
                                      L_Oper * opB)
{
  int i;
  for (i = 0; i < L_max_dest_operand; i++)
    {
      if (!L_no_redefs_between (oper->dest[i], opA, opB))
        return 0;
    }
  return 1;
}

/*===========================================================================*/
/*
 *      L_Oper boolean predicates
 */
/*===========================================================================*/

int
L_special_pred_define_opcode (L_Oper * oper)
{
  if (oper == NULL)
    return (0);

  switch (oper->opc)
    {
    case Lop_DIV_F:
    case Lop_DIV_F2:
      if (oper->dest[1] && L_is_ctype_predicate (oper->dest[1]))
        return (1);
    case Lop_SQRT_F:
    case Lop_SQRT_F2:
      if (oper->dest[1] && L_is_ctype_predicate (oper->dest[1]))
        return (1);
    case Lop_DEFINE:
      if (oper->dest[0] && L_is_ctype_predicate (oper->dest[0]))
        return (1);
    default:
      /* Insert cases for divides, etc. with predicate dests here */
      return (0);
    }
}

int
L_initializing_pred_define_opcode (L_Oper * oper)
{
  if (oper == NULL)
    return (0);

  switch (oper->opc)
    {
    case Lop_PRED_CLEAR:
    case Lop_PRED_SET:
    case Lop_PRED_COPY:
    case Lop_PRED_COMPL:

    case Lop_PRED_MASK_AND:
    case Lop_PRED_MASK_OR:
      return (1);
    }
  return (0);
}

int
L_pred_combine_opcode (L_Oper * oper)
{
  if (oper == NULL)
    return (0);

  switch (oper->opc)
    {
    case Lop_PRED_MASK_AND:
    case Lop_PRED_MASK_OR:
      return (1);
    }
  return (0);
}

int
L_pred_compl_opcode (L_Oper * oper)
{
  if (oper == NULL)
    return (0);

  switch (oper->opc)
    {
    case Lop_PRED_COMPL:
      return (1);
    }
  return (0);
}

int
L_pred_define_opcode (L_Oper * oper)
{
  return (L_general_pred_comparison_opcode (oper) ||
          L_initializing_pred_define_opcode (oper) ||
          L_special_pred_define_opcode (oper) ||
          L_pred_combine_opcode (oper) || L_pred_compl_opcode (oper) ||
	  L_pred_load_opcode (oper));
}

int
L_pred_load_opcode (L_Oper * oper)
{
  int opc;
  if (oper == NULL)
    return 0;
  opc = oper->opc;
  return ((opc == Lop_PRED_LD) || (opc == Lop_PRED_LD_BLK));
}

int
L_pred_store_opcode (L_Oper * oper)
{
  int opc;
  if (oper == NULL)
    return 0;
  opc = oper->opc;
  return ((opc == Lop_PRED_ST) || (opc == Lop_PRED_ST_BLK));
}

int
L_is_predicated (L_Oper * oper)
{
  if (oper == NULL)
    return 0;

  if (L_max_pred_operand <= 0)
    return 0;

  return (oper->pred[0] != NULL);
}

/* WARNING: This checks only for name identity, not value identity.
 * This is NO SUBSTITUTE for PG_equivalent_predicates_ops
 */
int
L_same_predicate (L_Oper * oper1, L_Oper * oper2)
{
  L_Operand *p1, *p2;

  p1 = oper1->pred[0];
  p2 = oper2->pred[0];
  return (L_same_operand (p1, p2));
}

int
L_is_reg_const_pred_compare (L_Oper * oper)
{
  if (!oper || !L_general_pred_comparison_opcode (oper))
    return FALSE;

  if (L_is_variable (oper->src[0]))
    {
      if (L_is_int_constant (oper->src[1]))
        return TRUE;
    }
  else if (L_is_variable (oper->src[1]))
    {
      if (L_is_int_constant (oper->src[0]))
        return TRUE;
    }
  return FALSE;
}

int
L_is_reg_const_cond_br (L_Oper * oper)
{
  if (!oper || !L_cond_branch_opcode (oper))
    return FALSE;

  if (L_is_variable (oper->src[0]))
    {
      if (L_is_int_constant (oper->src[1]))
        return TRUE;
    }
  else if (L_is_variable (oper->src[1]))
    {
      if (L_is_int_constant (oper->src[0]))
        return TRUE;
    }
  return FALSE;
}

int
L_pred_compare_const_first (L_Oper * oper)
{
  if (!oper)
    L_punt ("L_pred_compare_const_first called on NULL oper");
  if (L_is_variable (oper->src[0]))
    {
      if (L_is_int_constant (oper->src[1]))
        return FALSE;
    }
  else if (L_is_variable (oper->src[1]))
    {
      if (L_is_int_constant (oper->src[0]))
        return TRUE;
    }
  L_punt ("L_pred_compare_const_first called on non reg-const compare");
  return FALSE;
}

L_Operand *
L_pred_compare_get_reg_operand (L_Oper * oper)
{
  if (!oper)
    L_punt ("L_pred_compare_get_reg_operand called on NULL oper");
  if (L_is_variable (oper->src[0]))
    {
      if (L_is_int_constant (oper->src[1]))
        return oper->src[0];
    }
  else if (L_is_variable (oper->src[1]))
    {
      if (L_is_int_constant (oper->src[0]))
        return oper->src[1];
    }
  L_punt ("L_pred_compare_get_register_operand"
          " called on non reg-const compare");
  return NULL;
}

ITuintmax
L_pred_compare_get_int_const (L_Oper * oper)
{
  if (!oper)
    L_punt ("L_pred_compare_get_int_const called on NULL oper");
  if (L_is_variable (oper->src[0]))
    {
      if (L_is_int_constant (oper->src[1]))
        return (oper->src[1]->value.i);
    }
  else if (L_is_variable (oper->src[1]))
    {
      if (L_is_int_constant (oper->src[0]))
        return (oper->src[0]->value.i);
    }
  L_punt ("L_pred_compare_get_int_const called on non reg-const compare");
  return 0;
}

/*===========================================================================*/
/*
 *      L_Cb boolean predicates
 */
/*===========================================================================*/

/*===========================================================================*/
/*
 *      L_Attr functions
 */
/*===========================================================================*/

/*===========================================================================*/
/*
 *      L_Flow functions
 */
/*===========================================================================*/

/*===========================================================================*/
/*
 *      L_Operand functions
 */
/*===========================================================================*/

int
L_find_pred_clear (L_Cb * cb, L_Operand * operand)
{
  int count;
  L_Oper *ptr, *next;

  count = 0;
  for (ptr = cb->first_op; ptr != NULL; ptr = next)
    {
      next = ptr->next_op;
      if (!L_is_opcode (Lop_PRED_CLEAR, ptr))
        continue;
      if (!L_same_operand (operand, ptr->dest[0]))
        continue;
      if (count == 0)
        {
          count++;
        }
      else
        {
          L_punt ("L_find_pred_clear: multiple clears for same reg found");
        }
    }

  if (count == 0)
    return 0;
  
  return 1;
}


void
L_delete_pred_clear (L_Cb * cb, L_Operand * operand)
{
  int count;
  L_Oper *ptr, *next;

  count = 0;
  for (ptr = cb->first_op; ptr != NULL; ptr = next)
    {
      next = ptr->next_op;
      if (!L_is_opcode (Lop_PRED_CLEAR, ptr))
        continue;
      if (!L_same_operand (operand, ptr->dest[0]))
        continue;
      if (count == 0)
        {
          L_delete_oper (cb, ptr);
          count++;
        }
      else
        {
          L_punt ("L_delete_pred_clear: multiple clears for same reg found");
        }
    }

  if (count == 0)
    L_punt ("L_delete_pred_clear: pred clear not found");
}


static void
L_delete_pred_init (L_Cb * cb, L_Operand * operand)
{
  int count;
  L_Oper *ptr, *next;

  count = 0;
  for (ptr = cb->first_op; ptr != NULL; ptr = next)
    {
      next = ptr->next_op;
      if (!L_is_opcode (Lop_PRED_CLEAR, ptr) &&
          !L_is_opcode (Lop_PRED_SET, ptr))
        continue;
      if (!L_same_operand (operand, ptr->dest[0]))
        continue;
      if (count == 0)
        {
          L_delete_oper (cb, ptr);
          count++;
        }
      else
        {
          L_punt
            ("L_delete_pred_init: multiple pred inits for same reg found");
        }
    }

  if (count == 0)
    L_punt ("L_delete_pred_init: pred init not found");
}


/*===========================================================================*/
/*
 *      L_Oper functions
 */
/*===========================================================================*/


/*
 *      Duality map is the following, to allow more effective optimization
 *      Ops currently handled:
 *              r1 = r2 + 1 (p)   =>   r1 = r2 + p (1)
 *              r1 = r2 - 1 (p)   =>   r1 = r2 - p (1)
 */
void
L_convert_pred_ops_to_dual_format (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *oper;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
        {
          if (!L_is_predicated (oper))
            continue;
          if (L_int_add_opcode (oper))
            {
              if (L_is_int_one (oper->src[0]))
                {
                  L_delete_operand (oper->src[0]);
                  oper->src[0] = oper->pred[0];
                  oper->pred[0] = NULL;
                }
              else if (L_is_int_one (oper->src[1]))
                {
                  L_delete_operand (oper->src[1]);
                  oper->src[1] = oper->pred[0];
                  oper->pred[0] = NULL;
                }
              else if (L_is_int_neg_one (oper->src[1]))
                {
                  L_delete_operand (oper->src[1]);
                  oper->src[1] = oper->pred[0];
                  oper->pred[0] = NULL;
                  if (L_is_opcode (Lop_ADD, oper))
                    L_change_opcode (oper, Lop_SUB);
                  else
                    L_change_opcode (oper, Lop_SUB_U);
                }
            }
          else if (L_int_sub_opcode (oper))
            {
              if (L_is_int_one (oper->src[1]))
                {
                  L_delete_operand (oper->src[1]);
                  oper->src[1] = oper->pred[0];
                  oper->pred[0] = NULL;
                }
              else if (L_is_int_neg_one (oper->src[1]))
                {
                  L_delete_operand (oper->src[1]);
                  oper->src[1] = oper->pred[0];
                  oper->pred[0] = NULL;
                  if (L_is_opcode (Lop_SUB, oper))
                    L_change_opcode (oper, Lop_ADD);
                  else
                    L_change_opcode (oper, Lop_ADD_U);
                }
            }
        }
    }
}

void
L_convert_dual_format_to_pred_ops (L_Func * fn)
{
  int i, flag;
  L_Cb *cb;
  L_Oper *oper;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
        {
          flag = 0;
          for (i = 0; i < L_max_src_operand; i++)
            {
              if (L_is_pred_register (oper->src[i]))
                {
                  flag = 1;
                  break;
                }
            }

          if (!flag)
            continue;
          if (oper->pred[0] != NULL)
            L_punt ("L_convert_dual_format_to_pred_ops: corrupt pred field");

          if (L_int_add_opcode (oper))
            {
              if (L_is_pred_register (oper->src[0]))
                {
                  oper->pred[0] = oper->src[0];
                  oper->src[0] = L_new_gen_int_operand (1);
                }
              else if (L_is_pred_register (oper->src[1]))
                {
                  oper->pred[0] = oper->src[1];
                  oper->src[1] = L_new_gen_int_operand (1);
                }
              else
                {
                  L_punt ("L_convert_dual_format_to_pred_ops: "
                          "illegal pred src op");
                }
            }
          else if (L_int_sub_opcode (oper))
            {
              if (L_is_pred_register (oper->src[1]))
                {
                  oper->pred[0] = oper->src[1];
                  oper->src[1] = L_new_gen_int_operand (1);
                }
              else
                {
                  L_punt ("L_convert_dual_format_to_pred_ops: "
                          "illegal pred src op");
                }
            }
          else
            {
              L_punt
                ("L_convert_dual_format_to_pred_ops: illegal use of pred");
            }
        }
    }
}

void
L_save_virtual_pred_reg_numbers_in_attr (L_Func * fn)
{
  int i;
  L_Cb *cb;
  L_Oper *oper;
  L_Attr *attr;

  /* Make sure do this only 1x */
  if (L_EXTRACT_BIT_VAL (fn->flags, L_FUNC_PRED_REGS_IN_ATTR))
    return;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
        {
          for (i = 0; i < L_max_dest_operand; i++)
            {
              if (!L_is_ctype_predicate (oper->dest[i]))
                continue;
              attr = L_find_attr (oper->attr, L_VPRED_DST_ATTR_NAME);
              if (attr == NULL)
                {
                  attr = L_new_attr (L_VPRED_DST_ATTR_NAME, 1);
                  oper->attr = L_concat_attr (oper->attr, attr);
                }
              L_set_attr_field (attr, i, L_copy_operand (oper->dest[i]));
            }
          for (i = 0; i < L_max_src_operand; i++)
            {
              if (!L_is_ctype_predicate (oper->src[i]))
                continue;
              attr = L_find_attr (oper->attr, L_VPRED_SRC_ATTR_NAME);
              if (attr == NULL)
                {
                  attr = L_new_attr (L_VPRED_SRC_ATTR_NAME, 1);
                  oper->attr = L_concat_attr (oper->attr, attr);
                }
              L_set_attr_field (attr, i, L_copy_operand (oper->src[i]));
            }
          for (i = 0; i < L_max_pred_operand; i++)
            {
              if (!L_is_ctype_predicate (oper->pred[i]))
                continue;
              attr = L_find_attr (oper->attr, L_VPRED_PRD_ATTR_NAME);
              if (attr == NULL)
                {
                  attr = L_new_attr (L_VPRED_PRD_ATTR_NAME, 1);
                  oper->attr = L_concat_attr (oper->attr, attr);
                }
              L_set_attr_field (attr, i, L_copy_operand (oper->pred[i]));
            }
        }
    }

  fn->flags = L_SET_BIT_FLAG (fn->flags, L_FUNC_PRED_REGS_IN_ATTR);
}

L_Operand *
L_get_vpred_dest_from_attr (L_Oper * oper, int dest_id)
{
  L_Attr *attr;

  if (oper == NULL)
    L_punt ("L_get_vpred_dest_from_attr: oper is NULL");

  attr = L_find_attr (oper->attr, L_VPRED_DST_ATTR_NAME);
  if (attr == NULL)
    return (0);

  if (dest_id >= attr->max_field)
    return (0);

  return (attr->field[dest_id]);
}

L_Operand *
L_get_vpred_src_from_attr (L_Oper * oper, int src_id)
{
  L_Attr *attr;

  if (oper == NULL)
    L_punt ("L_get_vpred_dest_from_attr: oper is NULL");

  attr = L_find_attr (oper->attr, L_VPRED_SRC_ATTR_NAME);
  if (attr == NULL)
    return (0);

  if (src_id >= attr->max_field)
    return (0);

  return (attr->field[src_id]);
}

L_Operand *
L_get_vpred_pred_from_attr (L_Oper * oper, int pred_id)
{
  L_Attr *attr;

  if (oper == NULL)
    L_punt ("L_get_vpred_pred_from_attr: oper is NULL");

  attr = L_find_attr (oper->attr, L_VPRED_PRD_ATTR_NAME);
  if (attr == NULL)
    return (0);

  if (pred_id >= attr->max_field)
    return (0);

  return (attr->field[pred_id]);
}


/*===========================================================================*/
/*
 *      L_Cb functions
 */
/*===========================================================================*/


int
L_count_cb_predicates (L_Cb * cb)
{
  L_Oper *oper;
  L_Operand *pred;
  L_Operand *dest;
  Set predicates = NULL;
  int indx;
  int size;

  for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
    {
      if ((pred = oper->pred[0]))
	predicates = Set_add (predicates, pred->value.r);

      for (indx = 0; indx < L_max_dest_operand; indx++)
        {
          if (!(dest = oper->dest[indx]) ||
	      !L_is_pred_register (dest))
            continue;
          predicates = Set_add (predicates, dest->value.r);
        }
    }

  size = Set_size (predicates);
  Set_dispose (predicates);
  return size;
}

/*===========================================================================*/
/*
 *      L_Func functions
 */
/*===========================================================================*/

void
L_set_hb_no_fallthru_flag (L_Func * fn)
{
  L_Cb *cb, *dst_cb;
  L_Flow *last_flow, *match_flow;
  L_Oper *last_branch;

  PG_setup_pred_graph (fn);
  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      if (!L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK))
        continue;
      cb->flags = L_CLR_BIT_FLAG (cb->flags, L_CB_HYPERBLOCK_NO_FALLTHRU);

      if (!cb->dest_flow)
	continue;

      last_flow = L_find_last_flow (cb->dest_flow);

      if (!(L_find_branch_for_flow (cb, last_flow)))
	{
	  if (!L_has_fallthru_to_next_cb (cb))
	    {
	      /* make sure this is no fallthru flow */

              dst_cb = last_flow->dst_cb;
              match_flow = L_find_matching_flow (dst_cb->src_flow, last_flow);
              dst_cb->src_flow = L_delete_flow (dst_cb->src_flow, match_flow);
              cb->dest_flow = L_delete_flow (cb->dest_flow, last_flow);

              /* Delete any trailing ops on FT path in this cb, 
                 since they're dead code */
              last_flow = L_find_last_flow (cb->dest_flow);
              if ((last_branch = L_find_branch_for_flow (cb, last_flow)))
                while (cb->last_op != last_branch)
                  L_delete_oper (cb, cb->last_op);
	    }
	  else if (last_flow->dst_cb == cb->next_cb)
	    {
	      /* make sure there is a fallthru flow arc, if yes, fine, if
	       * no, we will assume the block really doesnt have a
	       * fallthru path.  False fallthru paths may arise after
	       * predicate promotion, so this extra check is made.. */

	      continue;
	    }
	}

      cb->flags = L_SET_BIT_FLAG (cb->flags, L_CB_HYPERBLOCK_NO_FALLTHRU);
    }
}

void
L_insert_dummy_op_into_hyperblocks (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *dummy_op;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      if (!L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK))
        continue;
      if (!L_is_predicated (cb->first_op))
        continue;
      dummy_op = L_create_new_op (Lop_PRED_CLEAR);
      dummy_op->dest[0] = L_new_register_operand (++L_fn->max_reg_id,
                                                  L_CTYPE_PREDICATE,
                                                  L_PTYPE_NULL);
      L_insert_oper_before (cb, cb->first_op, dummy_op);
    }
}

/************************************************************************
 *                                                                      *
 *  PREDICATE PROMOTION                                                 *
 *                                                                      *
 ************************************************************************/

void
L_mark_oper_promoted (L_Oper * oper)
{
  oper->flags = L_SET_BIT_FLAG (oper->flags, L_OPER_PROMOTED);

  if (L_mask_potential_exceptions &&
      L_is_pei (oper) && !L_is_trivially_safe (oper))
    {
      oper->flags = L_SET_BIT_FLAG (oper->flags, L_OPER_MASK_PE);
    }
  return;
}

/* Moves pred[0] to pred[1],
   sets promoted flag (P),
   moves input parm pred into pred[0] (can be NULL),

   pred[1] is lost */

void
L_promote_predicate (L_Oper * oper, L_Operand * pred)
{
  L_Operand *old_pred;

  if (!oper)
    L_punt ("L_promote_predicate: oper is NULL");

  if (!L_is_predicated (oper))
    {
      if (!pred)
        return;
      L_punt ("L_promote_predicate: oper not predicated, pred not NULL");
    }

  if (L_max_pred_operand < 2)
    L_punt ("L_promote_predicate: L_max_pred_operand is less than two");

  old_pred = oper->pred[0];

  oper->pred[0] = pred ? L_copy_operand (pred) : NULL;

  if (oper->pred[1])
    L_delete_operand (old_pred);
  else
    oper->pred[1] = old_pred;

  L_mark_oper_promoted (oper);
}

int
L_is_promoted (L_Oper * oper)
{
  if (L_EXTRACT_BIT_VAL (oper->flags, L_OPER_PROMOTED))
    return TRUE;
  return FALSE;
}

/************************************************************************
 *                                                                      *
 *  PREDICATE DEMOTION                                                  *
 *                                                                      *
 ************************************************************************/

void
L_demote_predicate (L_Oper * oper, L_Operand * pred)
{
  if (oper == NULL)
    L_punt ("L_demote_predicate: oper is NULL");

  if (pred == NULL)
    L_punt ("L_demote_predicate: trying to demote to NULL predicate");

  if (L_max_pred_operand < 2)
    L_punt ("L_demote_predicate: L_max_pred_operand is less than two");

  /* no demotion necessary */
  if (L_same_operand (oper->pred[0], pred))
    return;

  /* demoted predicate = original predicate, remove original predicate */
  if (L_same_operand (oper->pred[1], pred))
    {
      L_delete_operand (oper->pred[0]);
      oper->pred[0] = L_copy_operand (pred);
      oper->pred[0]->ptype = L_PTYPE_NULL;
      L_delete_operand (oper->pred[1]);
      oper->pred[1] = NULL;

      /* demotion may be done after scheduling, so dont reset exception
       * mask if oper is speculative 
       */
#if 0
      /* Since demotion may not demote back to the original predicate,
	 this operation may still be speculative. MCM 8/01 */
      oper->flags = L_CLR_BIT_FLAG (oper->flags, L_OPER_PROMOTED);

      if (!L_EXTRACT_BIT_VAL (oper->flags, L_OPER_SPECULATIVE))
        oper->flags = L_CLR_BIT_FLAG (oper->flags, L_OPER_MASK_PE);
#endif
    }
  else
    {
      L_delete_operand (oper->pred[0]);
      oper->pred[0] = L_copy_operand (pred);
      oper->pred[0]->ptype = L_PTYPE_NULL;
    }
}

static void record_def (HashTable hash, L_Operand *pred);
static void record_use (HashTable hash, L_Operand *pred);

/* L_predicate_demotion (JWS -- replaces L_do_predicate_demotion)
 * ----------------------------------------------------------------------
 * Demote each oper's predicate to the strongest predicate that is
 * both available and satisfies all the oper's dependences.
 * N.B. Requires up-to-date run of L_partial_dead_code() -- needs both
 * pred[1] and pred[1]->value.pred.ssa to be accurate.  
 */

int
L_predicate_demotion (L_Func *fn)
{
  L_Cb *cb;
  L_Oper *dem_op;
  L_Attr *attr;
  HashTable avail_preds;
  PG_Pred_Graph *pg;
  int count = 0, op_cy;

  if (!(pg = PG_pred_graph))
    return 0;

  avail_preds = HashTable_create(512);

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      if(L_EXTRACT_BIT_VAL(cb->flags, L_CB_SOFTPIPE))
	continue;

      for (dem_op = cb->first_op; dem_op != NULL; dem_op = dem_op->next_op)
	{
	  L_Operand *dem_pred, *orig_pred, *dest_pred, *strongest_pred;
	  int i;
	  DdNode *orig_node, *strongest_node, *dest_node, *dem_node;

	  orig_pred = dem_op->pred[0];
	  orig_node = orig_pred ?
	    orig_pred->value.pred.ssa->node : pg->one;

	  if ((attr = L_find_attr(dem_op->attr, "isl")))
	    op_cy = attr->field[0]->value.i;
	  else
	    op_cy = -1;

	  /* See if a stronger predicate is available */

	  strongest_pred = orig_pred;
	  strongest_node = strongest_pred ? 
	    strongest_pred->value.pred.ssa->node : pg->one;

	  dem_pred = dem_op->pred[1];
	  dem_node = dem_pred ?
	    dem_pred->value.pred.ssa->node : pg->one;

	  if (!Cudd_bddLeq(pg->dd, dem_node, orig_node))
	    {
	      DB_print_oper(dem_op);
	      L_punt("L_predicate_demotion: pred[1] > pred[0]");
	    }

	  if (dem_pred && (dem_node != strongest_node) &&
	      !L_pred_define_opcode(dem_op))
	    {
	      /* dem_pred exists and differs from orig_pred */

	      HashTable_start(avail_preds);

	      while ((dest_pred = 
		      (L_Operand *)HashTable_next (avail_preds)))
		{
		  dest_node = dest_pred->value.pred.ssa->node;

		  if (dest_pred->value.pred.ssa->oper &&
		      (attr = L_find_attr(dest_pred->value.pred.ssa->oper->attr, "isl")) &&
		      (attr->field[0]->value.i == op_cy))
		    continue;

		  if((dest_node != strongest_node) &&
		     Cudd_bddLeq (pg->dd, dest_node, strongest_node) &&
		     Cudd_bddLeq (pg->dd, dem_node, dest_node))
		    {
		      strongest_pred = dest_pred;
		      strongest_node = strongest_pred->value.pred.ssa->node;
		    }
		}

	      if (strongest_pred != orig_pred)
		{
		  L_demote_predicate(dem_op, strongest_pred);
#if 0
		  fprintf(stderr,"Demoted op %d\n", dem_op->id);
		  DB_print_oper(dem_op);
#endif
		  count++;
		}
	    }

	  /* Add predicates referenced */

	  if (dem_op->pred[0])
	    record_use(avail_preds, dem_op->pred[0]);

	  /* Add predicates defined */

	  if (L_pred_define_opcode(dem_op))
	    {
	      for (i = 0; i < L_max_dest_operand; i++)
		{
		  if (!(dest_pred = dem_op->dest[i]))
		    continue;

		  if (!L_is_ctype_predicate (dest_pred))
		    continue;

		  if (!dest_pred->value.pred.ssa)
		    continue;

		  record_def(avail_preds, dest_pred);
		}
	    }
	}
      HashTable_reset(avail_preds);
    }
  HashTable_free(avail_preds);

#if 0
  fprintf(stderr,"Demoted %d opers\n", count);
#endif

  return count;
}

static void
record_def (HashTable hash, L_Operand *pred)
{
  if (HashTable_find_or_null(hash, pred->value.pred.reg))
    HashTable_remove(hash, pred->value.pred.reg);
  if (pred->value.pred.ssa)
    HashTable_insert(hash, pred->value.pred.reg, (void *)pred);
  return;
}

static void
record_use (HashTable hash, L_Operand *pred)
{
  if (!HashTable_find_or_null(hash, pred->value.pred.reg) &&
      pred->value.pred.ssa)
    HashTable_insert(hash, pred->value.pred.reg, (void *)pred);
  return;
}

/*===========================================================================*/
/*
 *      L_Fn functions
 */
/*===========================================================================*/

void
L_set_hyperblock_flags (L_Func * fn)
{
  int hyper;
  L_Cb *cb;
  L_Oper *oper;

  hyper = FALSE;
  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK))
        {
          hyper = TRUE;
          continue;
        }

      for (oper = cb->first_op; oper; oper = oper->next_op)
        {
          if (L_pred_define_opcode (oper) || oper->pred[0] || oper->pred[1])
            {
              hyper = TRUE;
              cb->flags = L_SET_BIT_FLAG (cb->flags, L_CB_HYPERBLOCK);
            }
        }
    }

  if (hyper)
    fn->flags = L_SET_BIT_FLAG (fn->flags, L_FUNC_HYPERBLOCK);
  else
    fn->flags = L_CLR_BIT_FLAG (fn->flags, L_FUNC_HYPERBLOCK);
}



void
L_set_hyperblock_func_flag (L_Func * fn)
{
  int hb_flag = 0;
  L_Cb *cb;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK))
        {
          hb_flag = 1;
          break;
        }
    }

  if (hb_flag)
    fn->flags = L_SET_BIT_FLAG (fn->flags, L_FUNC_HYPERBLOCK);
  else
    fn->flags = L_CLR_BIT_FLAG (fn->flags, L_FUNC_HYPERBLOCK);
}


int
L_preds_same_class (L_Operand * pred1, L_Operand * pred2)
{
  int ptype1, ptype2;

  if (!L_is_pred_register (pred1))
    L_punt ("L_preds_same_class: pred1 is not a predicate register");
  if (!L_is_pred_register (pred2))
    L_punt ("L_preds_same_class: pred2 is not a predicate register");

  ptype1 = pred1->ptype;
  ptype2 = pred2->ptype;

  switch (ptype1)
    {
    case L_PTYPE_UNCOND_T:
    case L_PTYPE_UNCOND_F:
      return ((ptype2 == L_PTYPE_UNCOND_T) || (ptype2 == L_PTYPE_UNCOND_F));
    case L_PTYPE_COND_T:
    case L_PTYPE_COND_F:
      return ((ptype2 == L_PTYPE_COND_T) || (ptype2 == L_PTYPE_COND_F));
    case L_PTYPE_OR_T:
    case L_PTYPE_OR_F:
      return ((ptype2 == L_PTYPE_OR_T) || (ptype2 == L_PTYPE_OR_F));
    case L_PTYPE_AND_T:
    case L_PTYPE_AND_F:
      return ((ptype2 == L_PTYPE_AND_T) || (ptype2 == L_PTYPE_AND_F));
    default:
      return (0);
    }
}


void
L_remove_PD_attr (L_Oper * oper, int indx)
{
  L_Attr *attr;
  char PDname[6];
  sprintf (PDname, "PD%d", indx);
  attr = L_find_attr (oper->attr, PDname);
  if (!attr)
    return;

  oper->attr = L_delete_attr (oper->attr, attr);

  return;
}


void
L_delete_PD_attrs (L_Oper * oper)
{
  L_Attr *attr;
  int indx;
  char PDname[6];

  for (indx = 0; indx < L_max_dest_operand; indx++)
    {
      sprintf (PDname, "PD%d", indx);
      attr = L_find_attr (oper->attr, PDname);
      if (attr)
        oper->attr = L_delete_attr (oper->attr, attr);
    }
  return;
}


void
L_copy_PD_attr (L_Oper * opA, int destA_indx, L_Oper * opB, int destB_indx)
{
  L_Attr *old_attr, *new_attr;
  L_Operand *cb_wt, *flow_wt;
  char PDname[6];
  sprintf (PDname, "PD%d", destB_indx);
  old_attr = L_find_attr (opB->attr, PDname);
  if (!old_attr)
    return;

  cb_wt = L_copy_operand (old_attr->field[0]);
  flow_wt = L_copy_operand (old_attr->field[1]);

  sprintf (PDname, "PD%d", destA_indx);

  new_attr = L_new_attr (PDname, 2);

  L_set_attr_field (new_attr, 0, cb_wt);
  L_set_attr_field (new_attr, 1, flow_wt);

  opA->attr = L_concat_attr (opA->attr, new_attr);

  return;
}


void
L_compress_pred_dests (L_Oper * oper)
{
  int dest_indx, next_to_fill;

  next_to_fill = 0;

  for (dest_indx = 0; dest_indx < L_max_dest_operand; dest_indx++)
    {
      if (!oper->dest[dest_indx])
        continue;
      if (next_to_fill < dest_indx)
        {
          oper->dest[next_to_fill] = oper->dest[dest_indx];
          oper->dest[dest_indx] = NULL;
          L_copy_PD_attr (oper, next_to_fill, oper, dest_indx);
          L_remove_PD_attr (oper, dest_indx);
        }
      next_to_fill++;
    }
}


/*
 * copies PD0 of opB into PD1 of opA
 */
void
L_merge_PD_attrs (L_Oper * opA, L_Oper * opB)
{
  L_Attr *attrB0, *attrA1;
  L_Operand *cb_wt, *flow_wt;
  if (!opA || !opB)
    L_punt ("L_merge_PD_attrs: Attempt to merge on null opers.");
  attrB0 = L_find_attr (opB->attr, "PD0");
  if (!attrB0)
    return;

  attrA1 = L_find_attr (opA->attr, "PD1");

  if (attrA1)
    L_punt ("L_merge_PD_attrs: opA already has PD1 attr.");

  cb_wt = L_copy_operand (attrB0->field[0]);
  flow_wt = L_copy_operand (attrB0->field[1]);

  attrA1 = L_new_attr ("PD1", 2);

  L_set_attr_field (attrA1, 0, cb_wt);
  L_set_attr_field (attrA1, 1, flow_wt);

  opA->attr = L_concat_attr (opA->attr, attrA1);

  return;
}


int
L_split_pred_defines (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *op, *next_op, *new_op;
  int count, old_dest_indx, new_dest_indx;

  count = 0;

  if (L_max_dest_operand < 2)
    return 0;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      if (!L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK))
        continue;
      for (op = cb->first_op; op != NULL; op = next_op)
        {
          next_op = op->next_op;
          if (!L_general_pred_comparison_opcode (op))
            continue;
          if (L_num_dest_operand (op) == 1)
            continue;

          /* Split op */

          for (old_dest_indx = 1; old_dest_indx < L_max_dest_operand;
               old_dest_indx++)
            {
              if (op->dest[old_dest_indx] == NULL)
                continue;

              new_op = L_copy_operation (op);
              L_insert_oper_after (cb, op, new_op);

              L_delete_operand (op->dest[old_dest_indx]);
              op->dest[old_dest_indx] = NULL;

              L_delete_operand (new_op->dest[0]);
              new_op->dest[0] = new_op->dest[old_dest_indx];
              for (new_dest_indx = 1; new_dest_indx < L_max_dest_operand;
                   new_dest_indx++)
                new_op->dest[new_dest_indx] = NULL;

              L_delete_PD_attrs (new_op);

              L_copy_PD_attr (new_op, 0, op, old_dest_indx);

              L_remove_PD_attr (op, old_dest_indx);
            }

#ifdef DEBUG_SPLIT
          fprintf (ERR, "Split pred def %d into %d and %d\n",
                   op->id, op->id, new_op->id);
#endif
          count++;
        }
    }
  return count;
}

int *L_pred_mask_table = NULL;

int
L_pred_def_can_accomodate_dest (L_Oper * oper, int new_dest_ptype)
{
  int indx, dest_indx, comp_indx, matched, next_mask_avail;
  L_Operand *dest_pred;

  next_mask_avail = 0;

  if (L_num_dest_operand (oper) == L_max_dest_operand)
    return 0;

  for (indx = 0; indx < 2 * L_max_pred_masks; indx++)
    L_pred_mask_table[indx] = 0;

  for (dest_indx = 0; dest_indx < L_max_dest_operand; dest_indx++)
    {
      if (!(dest_pred = oper->dest[dest_indx]))
        continue;
      matched = 0;
      for (comp_indx = 0; comp_indx < next_mask_avail; comp_indx++)
        if ((dest_pred->ptype == L_pred_mask_table[2 * comp_indx])
            && (L_pred_mask_table[2 * comp_indx + 1] < L_max_preds_per_mask))
          {
            L_pred_mask_table[2 * comp_indx + 1]++;
            matched = 1;
            break;
          }
      if (!matched)
        {
          if (next_mask_avail < L_max_pred_masks)
            {
              L_pred_mask_table[2 * next_mask_avail] = dest_pred->ptype;
              L_pred_mask_table[2 * next_mask_avail + 1] = 1;
              next_mask_avail++;
            }
          else
            L_punt ("Max pred masks exceeded!");
        }
    }

  /* see if new dest can be accomodated */

  if (next_mask_avail < L_max_pred_masks)
    return 1;
  else
    {
      for (indx = 0; indx < next_mask_avail; indx++)
        {
          if ((L_pred_mask_table[2 * indx] == new_dest_ptype) &&
              (L_pred_mask_table[2 * indx + 1] < L_max_preds_per_mask))
            return 1;
        }
      return 0;
    }
}


int
L_insert_pred_dest (L_Oper * oper, L_Operand * new_dest)
{
  int dest_indx;

  for (dest_indx = 0; dest_indx < L_max_dest_operand; dest_indx++)
    {
      if (oper->dest[dest_indx] == 0)
        {
          oper->dest[dest_indx] = new_dest;
          return dest_indx;
        }

    }
  L_punt ("L_insert_pred_dest: No room to insert new pred dest.");

  return -1;
}


int
L_combine_pred_defines (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *opA, *opB, *nextA, *nextB;
  L_Operand *destB, *new_dest;
  int count, destA_indx, destB_indx;
  int new_ptype;

  if (L_max_dest_operand < 2)
    return 0;

  L_pred_mask_table = calloc (2 * L_max_pred_masks, sizeof (int));

  count = 0;

  PG_setup_pred_graph (fn);

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      if (!L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK))
        continue;
      for (opA = cb->first_op; opA != NULL; opA = nextA)
        {
          nextA = opA->next_op;
          if (!L_general_pred_comparison_opcode (opA))
            continue;
          if (L_num_dest_operand (opA) == L_max_dest_operand)
            continue;
          for (opB = nextA; opB != NULL; opB = nextB)
            {
              nextB = opB->next_op;
              if (!L_general_pred_comparison_opcode (opB))
                continue;
              for (destB_indx = 0;
                   destB_indx < L_max_dest_operand; destB_indx++)
                {
                  if (!(destB = opB->dest[destB_indx]))
                    continue;
                  if ((L_combine_only_same_class_preds) &&
                      (!L_preds_same_class (opA->dest[0], destB)))
                    continue;
                  if (!(L_equivalent_comparison_opcode (opA, opB) ||
                        L_complement_comparison_opcode (opA, opB)))
                    continue;
		  if (!L_no_defs_between (destB, opA, opB))
		    continue;
		  if (!L_no_pred_uses_between (destB, opA, opB))
		    continue;
                  if (!L_same_src_operands (opA, opB))
                    continue;
                  if (!L_same_operand (opA->pred[0], opB->pred[0]))
                    continue;
                  if (!L_all_src_operand_same_def_reachs (opA, opA, opB))
                    continue;
                  if (!L_no_br_between (opA, opB))
                    continue;
                  if (!L_no_jsr_between (opA, opB))
                    continue;
                  if (!L_no_danger (0, 0, 0, opA, opB))
                    continue;

                  if (L_equivalent_comparison_opcode (opA, opB))
                    new_ptype = destB->ptype;
                  else
                    new_ptype = L_opposite_ptype (destB->ptype);

                  if (!L_pred_def_can_accomodate_dest (opA, new_ptype))
                    continue;

#ifdef DEBUG_COMBINE
                  fprintf (ERR,
                           "Comb pred defs %d -> %d into %d (same opc)\n",
                           opA->id, opB->id, opA->id);
#endif

                  new_dest = L_copy_operand (destB);
                  new_dest->ptype = new_ptype;
                  L_delete_operand (destB);
                  opB->dest[destB_indx] = NULL;
                  destA_indx = L_insert_pred_dest (opA, new_dest);
                  L_copy_PD_attr (opA, destA_indx, opB, destB_indx);
                  L_remove_PD_attr (opB, destB_indx);
                  count++;
                }

              if (L_num_dest_operand (opB) == 0)
                {
                  if (nextA == opB)
                    nextA = nextA->next_op;
                  L_delete_oper (cb, opB);
                }
            }
        }
    }

  free (L_pred_mask_table);

  return count;
}


void
L_create_uncond_pred_defines (L_Func * fn)
{
  /*
   * Transform pred_clear + pred_xx o{t|f} pairs into 
   * unconditional predciate defines where possible.
   */
  int i;
  L_Cb *cb;
  L_Oper *oper;
  L_Operand *dest;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      if (!L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK))
        continue;
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
        {
          if (!L_general_pred_comparison_opcode (oper))
            continue;
          for (i = 0; i < L_max_dest_operand; i++)
            {
              if (!(dest = oper->dest[i]))
                continue;
              if (!L_or_ptype (dest->ptype) &&
                  !L_and_ptype (dest->ptype) && !L_sand_ptype (dest->ptype))
                continue;
              if (L_pred_defined_multiple_times (cb, dest))
                continue;
#ifdef DEBUG_UNCOND
              fprintf (ERR, "Change r %d (op %d) to uncond predicate\n",
                       dest->value.r, oper->id);
#endif
              if (L_false_ptype (dest->ptype))
                dest->ptype = L_PTYPE_UNCOND_F;
              else
                dest->ptype = L_PTYPE_UNCOND_T;
              L_delete_pred_init (cb, dest);
            }
        }
    }
}


int
L_is_frp (L_Cb * cb)
{
  int is_frp;
  L_Oper *oper;
  PG_Pred_Graph *pg;
  DdManager *dd;
  DdNode *last_fallthru_node, *oper_node;

  pg = PG_pred_graph;

  if (!L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK))
    return 0;

  if (!pg)
    L_punt ("L_is_frp: build pred graph!");

  dd = pg->dd;

  last_fallthru_node = pg->one;

  is_frp = 1;

  for (oper = cb->first_op; oper; oper = oper->next_op)
    {
      if (!oper->pred[0])
        {
          if (last_fallthru_node != pg->one)
            return 0;
          else
            continue;
        }
      oper_node = oper->pred[0]->value.pred.ssa->node;

      if (!Cudd_bddLeq (dd, oper_node, last_fallthru_node))
        return 0;

      if (L_cond_branch_opcode (oper))
        L_punt ("L_is_frp: Encountered conditional branch at oper %d.",
                oper->id);

      if (L_uncond_branch_opcode (oper))
	last_fallthru_node = Cudd_Not (oper->pred[0]->value.pred.ssa->node);
    }
  return 1;
}


#if 0
static void
L_declare_available_pred (List * available_preds, int reg, DdNode * node)
{
  L_Available_Pred *new_avail;

  new_avail = L_alloc (L_avail_alloc_pool);

  new_avail->reg = reg;
  new_avail->node = node;

  (*available_preds) = List_insert_last (*available_preds, new_avail);
  return;
}


static int
L_find_available_pred (DdNode * match_node, List available_preds)
{
  PG_Pred_Graph *pg;
  DdManager *dd;
  L_Available_Pred *avail_indx;

  pg = PG_pred_graph;

  dd = pg->dd;

  List_start (available_preds);

  while ((avail_indx = List_next (available_preds)))
    {
      if (Cudd_bddLeq (dd, avail_indx->node, match_node) &&
          Cudd_bddLeq (dd, match_node, avail_indx->node))
        return avail_indx->reg;
    }
  return 0;
}


static L_Oper *
L_new_trivial_st_def (int guard_reg, int dest_reg)
{
  L_Oper *oper;

  oper = L_create_new_op (Lop_CMP);
  oper->com[0] = L_CTYPE_INT;
  oper->com[1] = Lcmp_COM_EQ;
  oper->pred[0] = L_new_register_operand (guard_reg,
                                          L_CTYPE_PREDICATE, L_PTYPE_NULL);
  oper->dest[0] = L_new_register_operand (dest_reg,
                                          L_CTYPE_PREDICATE, L_PTYPE_SAND_T);

  return oper;
}

static void
L_insert_pred_set (L_Cb * cb, L_Oper * def, int dest_reg)
{
  L_Oper *oper_indx, *last_indx, *set_oper;
  L_Operand *set_operand;

  last_indx = def;
  for (oper_indx = def->prev_op; oper_indx; oper_indx = oper_indx->prev_op)
    {
      if (L_is_opcode (Lop_PROLOGUE, oper_indx))
        break;
      last_indx = oper_indx;
    }

  set_operand = L_new_register_operand (dest_reg,
                                        L_CTYPE_PREDICATE, L_PTYPE_NULL);
  set_oper = L_create_new_op (Lop_PRED_SET);
  set_oper->dest[0] = set_operand;
  L_insert_oper_before (cb, last_indx, set_oper);
  return;
}


void
L_ensure_frp (L_Func * fn, L_Cb * cb)
{
  L_Oper *oper, *invert_op, *new_oper, *last_jump_op;
  PG_Pred_Graph *pg;
  DdManager *dd;
  DdNode *fallthru_node, *oper_node, *new_oper_node;
  List available_preds;
  int fallthru_reg, repl_pred;
  L_Available_Pred *avail_indx;

  invert_op = NULL;

  if (L_is_frp (cb))
    return;

  pg = PG_pred_graph;

  if (!L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK))
    return;

  if (!pg)
    L_punt ("L_ensure_frp: build pred graph!");

  dd = pg->dd;

  L_avail_alloc_pool = L_create_alloc_pool ("L_Available_Pred",
                                            sizeof (L_Available_Pred), 64);

  available_preds = NULL;

  last_jump_op = NULL;

  fallthru_node = pg->one;

  /* need to build table of available predicates */

  for (oper = cb->first_op; oper; oper = oper->next_op)
    {
      if (!oper->pred[0] && fallthru_node != pg->one)
        {
          oper->pred[0] = L_new_register_operand (fallthru_reg,
                                                  L_CTYPE_PREDICATE,
                                                  L_PTYPE_NULL);
          continue;
        }

      oper_node = oper->pred[0]->value.pred.ssa->node;

      if (Cudd_bddLeq (dd, oper_node, fallthru_node))
        {
          /* add oper_node to available? */
          continue;
        }


      /* fix frp */

      if (!invert_op)
        {
          /* generate fallthru predicate if not available */
          invert_op = L_create_new_op (Lop_PRED_COMPL);
          invert_op->pred[0] = L_copy_operand (last_jump_op->pred[0]);
          invert_op->dest[0] = L_copy_operand (last_jump_op->pred[0]);
          L_assign_ptype_uncond_true (invert_op->dest[0]);
          fallthru_reg = ++fn->max_reg_id;
          invert_op->dest[0]->value.pred.reg = fallthru_reg;
          L_insert_oper_before (cb, last_jump_op, invert_op);

          /* really want to insert immediately after def of exit pred */
        }

      new_oper_node = Cudd_bddIte (dd, oper_node, fallthru_node, 0);

      repl_pred = L_find_available_pred (new_oper_node, available_preds);

      if (!repl_pred)
        {
          /* create new predicate */

          repl_pred = ++fn->max_reg_id;

          L_insert_pred_set (cb, oper, repl_pred);

          new_oper = L_new_trivial_st_def (fallthru_reg, repl_pred);
          L_insert_oper_before (cb, oper, new_oper);

          new_oper = L_new_trivial_st_def (oper->pred[0]->value.pred.reg,
                                           repl_pred);
          L_insert_oper_before (cb, oper, new_oper);

          L_declare_available_pred (&available_preds, repl_pred,
                                    new_oper_node);
        }

      oper->pred[0]->value.pred.reg = repl_pred;

      if (L_cond_branch_opcode (oper))
        L_punt ("L_is_frp: Encountered conditional branch at oper %d.",
                oper->id);

      if (L_uncond_branch_opcode (oper) && oper->pred[0])
        {
          /* Generate new fallthru node */
          fallthru_node = Cudd_Not (oper->pred[0]->value.pred.ssa->node);
          last_jump_op = oper;
          invert_op = NULL;
        }
    }

  List_start (available_preds);

  while (avail_indx = List_next (available_preds))
    L_free (L_avail_alloc_pool, avail_indx);

  available_preds = List_reset (available_preds);

  L_free_alloc_pool (L_avail_alloc_pool);

  L_avail_alloc_pool = NULL;

  return;
}
#endif

void
L_branch_split_cb (L_Func * fn, L_Cb * cb)
{
  L_Oper *oper;
  L_Oper *new_jump;
  L_Oper *next_oper;
  int change;
  int has_fallthrough;

  change = 0;

  has_fallthrough = L_has_fallthru_to_next_cb (cb);

  for (oper = cb->first_op; oper; oper = next_oper)
    {
      next_oper = oper->next_op;

      if (!L_cond_branch_opcode (oper))
        continue;

      L_change_to_cmp_op (oper);

      oper->dest[0] = L_new_register_operand (++fn->max_reg_id,
                                              L_CTYPE_PREDICATE,
                                              L_PTYPE_UNCOND_T);

      new_jump = L_create_new_op (Lop_JUMP);

      new_jump->src[0] = oper->src[2];
      oper->src[2] = NULL;

      new_jump->pred[0] = L_new_register_operand (fn->max_reg_id,
                                                  L_CTYPE_PREDICATE,
                                                  L_PTYPE_NULL);

      new_jump->attr = L_copy_attr (oper->attr);
      new_jump->weight = oper->weight;

      L_insert_oper_after (cb, oper, new_jump);
      change++;
    }
  if (change)
    {
      cb->flags = L_SET_BIT_FLAG (cb->flags, L_CB_HYPERBLOCK);
      fn->flags = L_SET_BIT_FLAG (fn->flags, L_FUNC_HYPERBLOCK);
      if (!has_fallthrough)
        cb->flags = L_SET_BIT_FLAG (cb->flags, L_CB_HYPERBLOCK_NO_FALLTHRU);
    }
}

void
L_branch_split_func (L_Func * fn)
{
  L_Cb *cb;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    L_branch_split_cb (fn, cb);
}

