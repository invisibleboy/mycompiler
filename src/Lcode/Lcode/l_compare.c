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
 *      File :          l_compare.c
 *      Description :   New comparison format for branches, compares, and
 *                      register compares
 *      Creation Date : August 2000
 *      Author :        John Sias, Wen-mei Hwu
 *
 *==========================================================================*/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>

void
L_swap_compare (L_Oper * oper)
{
  if (!oper)
    L_punt ("L_swap_compare: oper is NULL");

  if (!oper->com[1])
    L_punt ("L_swap_compare: no compare found on oper %d", oper->id);

  oper->com[1] = L_inverse_pred_completer (oper->com[1]);
  return;
}

void
L_negate_compare (L_Oper * oper)
{
  if (!oper)
    L_punt ("L_negate_compare: oper is NULL");

  if (!oper->com[1])
    L_punt ("L_negate_compare: no compare found on oper %d", oper->id);

  oper->com[1] = L_opposite_pred_completer (oper->com[1]);
  return;
}

void
L_set_compare_type (L_Oper * oper, ITuint8 com)
{
  if (!oper)
    L_punt ("L_set_compare_type: oper is NULL");

  oper->com[1] = com;

  return;
}

ITuint8 L_get_compare_type (L_Oper * oper)
{
  if (!oper)
    L_punt ("L_get_compare_type: oper is NULL");

  return oper->com[1];
}

ITuint8 L_get_compare_ctype (L_Oper * oper)
{
  if (!oper)
    L_punt ("L_get_compare_ctype: oper is NULL");

  return oper->com[0];
}

void
L_set_compare (L_Oper * oper, ITuint8 ctype, ITuint8 com)
{
  if (!oper)
    L_punt ("L_set_compare: oper is NULL");

  oper->com[0] = ctype;
  oper->com[1] = com;

  return;
}

int
L_same_compare (L_Oper * opA, L_Oper * opB)
{
  if (!opA || !opB)
    L_punt ("L_same_compare: op is NULL");

  if (opA->com[0] != opB->com[0])
    return 0;
  else if (opA->com[1] != opB->com[1])
    return 0;
  else
    return 1;
}

int
L_opposite_compare (L_Oper * opA, L_Oper * opB)
{
  if (!opA || !opB)
    L_punt ("L_opposite_compare: op is NULL");

  if (opA->com[0] != opB->com[0])
    return 0;
  else if (opA->com[1] != L_opposite_pred_completer (opB->com[1]))
    return 0;
  else
    return 1;
}

int
L_inverse_compare (L_Oper * opA, L_Oper * opB)
{
  if (!opA || !opB)
    L_punt ("L_inverse_compare: op is NULL");

  if (opA->com[0] != opB->com[0])
    return 0;
  else if (opA->com[1] != L_inverse_pred_completer (opB->com[1]))
    return 0;
  else
    return 1;
}

int
L_inv_opp_compare (L_Oper * opA, L_Oper * opB)
{
  if (!opA || !opB)
    L_punt ("L_inv_opp_compare: op is NULL");

  if (opA->com[0] != opB->com[0])
    return 0;
  else if (opA->com[1] !=
           L_inverse_pred_completer (L_opposite_pred_completer (opB->com[1])))
    return 0;
  else
    return 1;
}

void
L_copy_compare (L_Oper * oper_to, L_Oper * oper_from)
{
  if (!oper_to || !oper_from)
    L_punt ("L_negate_compare: oper is NULL");

  oper_to->com[0] = oper_from->com[0];
  oper_to->com[1] = oper_from->com[1];

  return;
}


ITuint8 L_opposite_pred_completer (ITuint8 com)
{
  return (com ^ 0x01u);
}

ITuint8 L_inverse_pred_completer (ITuint8 com)
{
  if ((com >= Lcmp_COM_GT) && (com <= Lcmp_COM_LT))
    return (com ^ 0x03u);
  else
    return (com);
}

int
L_equivalent_comparison_opcode (L_Oper * op1, L_Oper * op2)
{
  if ((op1 == NULL) || (op2 == NULL))
    L_punt ("L_equivalent_comparison_opcode: ops cannot be NULL");

  if (op1->opc != op2->opc)
    return 0;
  if (!L_general_pred_comparison_opcode (op1) &&
      !L_general_comparison_opcode (op1) && !L_cond_branch_opcode (op1))
    return 1;

  /* Check completers */

  if ((op1->com[0] == op2->com[0]) && (op1->com[1] == op2->com[1]))
    return 1;
  else
    return 0;
}

int
L_complement_comparison_opcode (L_Oper * op1, L_Oper * op2)
{
  if ((op1 == NULL) || (op2 == NULL))
    L_punt ("L_complement_pred_opcode: ops cannot be NULL");

  if (op1->opc != op2->opc)
    return 0;
  if (!L_general_pred_comparison_opcode (op1) &&
      !L_general_comparison_opcode (op1) && !L_cond_branch_opcode (op1))
    L_punt ("L_complement_pred_opcode: called on non-compare");

  /* Check completers */

  if ((op1->com[1] ^ op2->com[1]) == 0x01)
    return 1;
  else
    return 0;
}

int
L_reversed_comparison_opcode (L_Oper * op1, L_Oper * op2)
{
  if ((op1 == NULL) || (op2 == NULL))
    L_punt ("L_reversed_pred_opcode: ops cannot be NULL");

  if (op1->opc != op2->opc)
    return 0;

  if (!L_general_pred_comparison_opcode (op1) &&
      !L_general_comparison_opcode (op1) && !L_cond_branch_opcode (op1))
    L_punt ("L_reversed_pred_opcode: called on non-compare");

  /* Check completers */

  if (op1->com[1] == Lcmp_COM_TZ || op1->com[1] == Lcmp_COM_TN)
    return 0;

  if (op1->com[1] == L_inverse_pred_completer (op2->com[1]))
    return 1;
  else
    return 0;
}

int
L_rev_complement_comparison_opcode (L_Oper * op1, L_Oper * op2)
{
  if ((op1 == NULL) || (op2 == NULL))
    L_punt ("L_rev_complement_pred_opcode: ops cannot be NULL");

  if (op1->opc != op2->opc)
    return 0;

  if (!L_general_pred_comparison_opcode (op1) &&
      !L_general_comparison_opcode (op1) && !L_cond_branch_opcode (op1))
    L_punt ("L_rev_complement_pred_opcode: called on non-compare");

  if (op1->com[1] == Lcmp_COM_TZ || op1->com[1] == Lcmp_COM_TN)
    return 0;

  /* Check completers */

  if ((op1->com[1] ^ L_inverse_pred_completer (op2->com[1])) == 0x01)
    return 1;
  else
    return 0;
}

void
L_change_to_cmp_op (L_Oper * op)
{
  int new_opc;

  if (!op)
    L_punt ("L_change_to_cmp_op: op is NULL");

  switch (op->opc)
    {
    case Lop_RCMP:
    case Lop_BR:
      new_opc = Lop_CMP;
      break;
    case Lop_RCMP_F:
    case Lop_BR_F:
      new_opc = Lop_CMP_F;
      break;
    default:
      L_punt ("L_change_to_cmp_op: called on non-compare");
      return;
    }
  L_change_opcode (op, new_opc);
  return;
}

void
L_change_to_rcmp_op (L_Oper * op)
{
  int new_opc;

  if (!op)
    L_punt ("L_change_to_rcmp_op: op is NULL");

  switch (op->opc)
    {
    case Lop_CMP:
    case Lop_BR:
      new_opc = Lop_RCMP;
      break;
    case Lop_CMP_F:
    case Lop_BR_F:
      new_opc = Lop_RCMP_F;
      break;
    default:
      L_punt ("L_change_to_rcmp_op: called on non-compare");
      return;
    }
  L_change_opcode (op, new_opc);
  return;
}

void
L_change_to_br_op (L_Oper * op)
{
  int new_opc;

  if (!op)
    L_punt ("L_change_to_br_op: op is NULL");

  switch (op->opc)
    {
    case Lop_RCMP:
    case Lop_CMP:
      new_opc = Lop_BR;
      break;
    case Lop_RCMP_F:
    case Lop_CMP_F:
      new_opc = Lop_BR_F;
      break;
    default:
      L_punt ("L_change_to_br_op: called on non-compare");
      return;
    }
  L_change_opcode (op, new_opc);
  return;
}

L_Oper *
L_new_pred_copy (L_Operand * dest, L_Operand * src)
{
  L_Oper *new_op;

  new_op = L_create_new_op (Lop_CMP);
  L_set_compare (new_op, L_CTYPE_INT, Lcmp_COM_EQ);
  new_op->src[0] = L_new_gen_int_operand (0);
  new_op->src[1] = L_new_gen_int_operand (0);
  new_op->pred[0] = L_copy_operand (src);
  new_op->dest[0] = L_copy_operand (dest);
  dest->ptype = L_PTYPE_NULL;
  return new_op;
}
