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
 *      File :          l_memory.c
 *      Description :   memory disambiguation
 *      Author :        Pohua Chang, Scott Mahlke, Wen-mei Hwu
 *      Date :          July, 1990
 *
 *==========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>
#include "l_accspec.h"

#undef DEBUG_LEVEL
#undef DEBUG_PARAM

#define L_MAX_PARAM                     20

int
L_is_nonconflicting_stack_operand (L_Operand * operand)
{
  if ((operand->value.mac == L_MAC_IP) || (operand->value.mac == L_MAC_OP))
    return (1);

  return (0);
}

/*
 *      Return 1 if successful, 0 if not.  first_op and second_op
 *      are return ops.  Rewritten to not take a cb ptr (SAM 5-98).
 */
int
L_find_first_seq_op (L_Oper * op1, L_Oper * op2,
                     L_Oper ** first_op, L_Oper ** second_op)
{
  L_Oper *oper;
  int op1_is_first, op2_is_first;

  op1_is_first = op2_is_first = 0;
  for (oper = op1->next_op; oper != NULL; oper = oper->next_op)
    {
      if (oper == op2)
        {
          op1_is_first = 1;
          break;
        }
    }

  if (!op1_is_first)
    {
      for (oper = op1->prev_op; oper != NULL; oper = oper->prev_op)
        {
          if (oper == op2)
            {
              op2_is_first = 1;
              break;
            }
        }
    }

  if (op1_is_first)
    {
      *first_op = op1;
      *second_op = op2;
      return (1);
    }
  else if (op2_is_first)
    {
      *first_op = op2;
      *second_op = op1;
      return (1);
    }
  else
    {
      *first_op = NULL;
      *second_op = NULL;
      return (0);
    }
}

int
L_constant_load (L_Oper * op)
{
  return (L_find_attr (op->attr, "Cload")) ? 1 : 0;
}

int
L_has_label_in_attr (L_Oper * op)
{
  return (L_EXTRACT_BIT_VAL (op->flags, L_OPER_LABEL_REFERENCE));
}

int
L_stack_reference (L_Oper * op)
{
  return (L_EXTRACT_BIT_VAL (op->flags, L_OPER_STACK_REFERENCE));
}

int
L_spill_code (L_Oper * op)
{
  return (L_EXTRACT_BIT_VAL (op->flags, L_OPER_SPILL_CODE));
}

int
L_spill_offset (L_Oper * op, int *offset)
{
  L_Attr *attr;

  if (op == NULL)
    L_punt ("L_spill_offset: op is NULL");

  attr = L_find_attr (op->attr, "offset");
  if (attr != NULL)
    {
      *offset = (int) attr->field[0]->value.i;
      return 1;
    }
  else
    {
      *offset = 0;
      return 0;
    }
}

int
L_memory_access_size (L_Oper * op)
{
  if (op == NULL)
    L_punt ("L_memory_access_size: op is NULL");

  switch (op->opc)
    {
    case Lop_LD_UC:
    case Lop_LD_UC_CHK:
    case Lop_LD_C:
    case Lop_LD_C_CHK:
    case Lop_LD_PRE_UC:
    case Lop_LD_PRE_C:
    case Lop_LD_POST_UC:
    case Lop_LD_POST_C:
      return 1;
    case Lop_LD_UC2:
    case Lop_LD_UC2_CHK:
    case Lop_LD_C2:
    case Lop_LD_C2_CHK:
    case Lop_LD_PRE_UC2:
    case Lop_LD_PRE_C2:
    case Lop_LD_POST_UC2:
    case Lop_LD_POST_C2:
      return (M_type_size (M_TYPE_SHORT) / M_SIZE_CHAR);
    case Lop_LD_I:
    case Lop_LD_I_CHK:
    case Lop_LD_UI:
    case Lop_LD_UI_CHK:
    case Lop_LD_PRE_I:
    case Lop_LD_POST_I:
      return (M_type_size (M_TYPE_INT) / M_SIZE_CHAR);
    case Lop_LD_Q:
    case Lop_LD_Q_CHK:
    case Lop_LD_PRE_Q:
    case Lop_LD_POST_Q:
      return (M_type_size (M_TYPE_LLONG) / M_SIZE_CHAR);
    case Lop_LD_F:
    case Lop_LD_F_CHK:
    case Lop_LD_PRE_F:
    case Lop_LD_POST_F:
      return (M_type_size (M_TYPE_FLOAT) / M_SIZE_CHAR);
    case Lop_LD_F2:
    case Lop_LD_F2_CHK:
    case Lop_LD_PRE_F2:
    case Lop_LD_POST_F2:
      return (M_type_size (M_TYPE_DOUBLE) / M_SIZE_CHAR);

    case Lop_ST_C:
      return 1;
    case Lop_ST_C2:
      return (M_type_size (M_TYPE_SHORT) / M_SIZE_CHAR);
    case Lop_ST_I:
      return (M_type_size (M_TYPE_INT) / M_SIZE_CHAR);
    case Lop_ST_Q:
      return (M_type_size (M_TYPE_LLONG) / M_SIZE_CHAR);
    case Lop_ST_F:
      return (M_type_size (M_TYPE_FLOAT) / M_SIZE_CHAR);
    case Lop_ST_F2:
      return (M_type_size (M_TYPE_DOUBLE) / M_SIZE_CHAR);
    case Lop_ST_PRE_C:
      return 1;
    case Lop_ST_PRE_C2:
      return (M_type_size (M_TYPE_SHORT) / M_SIZE_CHAR);
    case Lop_ST_PRE_I:
      return (M_type_size (M_TYPE_INT) / M_SIZE_CHAR);
    case Lop_ST_PRE_Q:
      return (M_type_size (M_TYPE_LLONG) / M_SIZE_CHAR);
    case Lop_ST_PRE_F:
      return (M_type_size (M_TYPE_FLOAT) / M_SIZE_CHAR);
    case Lop_ST_PRE_F2:
      return (M_type_size (M_TYPE_DOUBLE) / M_SIZE_CHAR);
    case Lop_ST_POST_C:
      return 1;
    case Lop_ST_POST_C2:
      return (M_type_size (M_TYPE_SHORT) / M_SIZE_CHAR);
    case Lop_ST_POST_I:
      return (M_type_size (M_TYPE_INT) / M_SIZE_CHAR);
    case Lop_ST_POST_Q:
      return (M_type_size (M_TYPE_LLONG) / M_SIZE_CHAR);
    case Lop_ST_POST_F:
      return (M_type_size (M_TYPE_FLOAT) / M_SIZE_CHAR);
    case Lop_ST_POST_F2:
      return (M_type_size (M_TYPE_DOUBLE) / M_SIZE_CHAR);

    case Lop_PRED_LD:
      return 1;
      /* this dependent on the number of predicate regs, assume 64 now */
    case Lop_PRED_LD_BLK:
      return 8;
    case Lop_PRED_ST:
      return 1;
      /* this dependent on the number of predicate regs, assume 64 now */
    case Lop_PRED_ST_BLK:
      return 8;

      /* SLARSEN: This function is used by L_independent_memory_ops for
	 rudimentary dependence checking.  The cases where the access size
	 is used should never be reached for vector memops.  Return anything. */ 
    case Lop_VLD_UC:
    case Lop_VLD_C:
    case Lop_VLD_UC2:
    case Lop_VLD_C2:
    case Lop_VLD_I:
    case Lop_VLD_F:
    case Lop_VLD_F2:
    case Lop_VST_C:
    case Lop_VST_C2:
    case Lop_VST_I:
    case Lop_VST_F:
    case Lop_VST_F2:		return (M_type_size(M_TYPE_DOUBLE) / M_SIZE_CHAR);

    default:
      return (M_memory_access_size (op));
    }
}

int
L_no_overlap (int offset1, int size1, int offset2, int size2)
{
  if (offset1 == offset2)
    return 0;
  else if (offset1 < offset2)
    return (offset2 >= (offset1 + size1));
  else
    return (offset1 >= (offset2 + size2));
}

int
L_classify_overlap (int offset1, int size1, int offset2, int size2,
		    int *p_indep, int *p_dep, int *p_pdep)
{
  int indep;

  if (offset1 == offset2)
    indep = 0;
  else if (offset1 < offset2)
    indep = (offset2 >= (offset1 + size1));
  else
    indep = (offset1 >= (offset2 + size2));

  if (indep)
    {
      if (p_indep)
	*p_indep = 1;
    }
  else
    {
      if (p_pdep)
	*p_pdep = 1;

      if (p_dep && (offset2 >= offset1) &&
	  ((offset2 + size2) <= (offset1 + size1)))
	*p_dep = 1;
    }

  return indep;
}

int
L_in_same_block (L_Oper * op1, L_Oper * op2)
{
  L_Oper *ptr;
  if ((op1 == NULL) || (op2 == NULL))
    L_punt ("L_in_same_block: op1 and op2 cannot be NIL");
  for (ptr = op1->prev_op; ptr != NULL; ptr = ptr->prev_op)
    {
      if (ptr == op2)
        return 1;
    }
  for (ptr = op1->next_op; ptr != NULL; ptr = ptr->next_op)
    {
      if (ptr == op2)
        return 1;
    }
  return 0;
}

int
L_get_data_type (L_Oper * oper)
{
  int opc;
  int datatype;

  opc = oper->opc;

  switch (opc)
    {
    case Lop_LD_UC:
    case Lop_LD_PRE_UC:
    case Lop_LD_POST_UC:
    case Lop_LD_C:
    case Lop_LD_PRE_C:
    case Lop_LD_POST_C:
    case Lop_ST_C:
    case Lop_ST_PRE_C:
    case Lop_ST_POST_C:
      datatype = L_MEMORY_ACCESS_CHAR;
      break;

    case Lop_LD_UC2:
    case Lop_LD_PRE_UC2:
    case Lop_LD_POST_UC2:
    case Lop_LD_C2:
    case Lop_LD_PRE_C2:
    case Lop_LD_POST_C2:
    case Lop_ST_C2:
    case Lop_ST_PRE_C2:
    case Lop_ST_POST_C2:
      datatype = L_MEMORY_ACCESS_CHAR2;
      break;

    case Lop_LD_UI:
    case Lop_LD_PRE_UI:
    case Lop_LD_POST_UI:
    case Lop_LD_I:
    case Lop_LD_PRE_I:
    case Lop_LD_POST_I:
    case Lop_ST_I:
    case Lop_ST_PRE_I:
    case Lop_ST_POST_I:
      datatype = L_MEMORY_ACCESS_INT;
      break;

    case Lop_LD_Q:
    case Lop_LD_PRE_Q:
    case Lop_LD_POST_Q:
    case Lop_ST_Q:
    case Lop_ST_PRE_Q:
    case Lop_ST_POST_Q:
      datatype = L_MEMORY_ACCESS_LLONG;
      break;

    case Lop_LD_F:
    case Lop_LD_PRE_F:
    case Lop_LD_POST_F:
    case Lop_ST_F:
    case Lop_ST_PRE_F:
    case Lop_ST_POST_F:
      datatype = L_MEMORY_ACCESS_FLOAT;
      break;

    case Lop_LD_F2:
    case Lop_LD_PRE_F2:
    case Lop_LD_POST_F2:
    case Lop_ST_F2:
    case Lop_ST_PRE_F2:
    case Lop_ST_POST_F2:
      datatype = L_MEMORY_ACCESS_DOUBLE;
      break;

    case Lop_PRED_LD:
    case Lop_PRED_ST:
    case Lop_PRED_LD_BLK:
    case Lop_PRED_ST_BLK:
      datatype = L_MEMORY_ACCESS_PREDICATE;
      break;

    default:
      datatype = M_get_data_type (oper);
    }

  return (datatype);
}

int
L_same_data_types (L_Oper * op1, L_Oper * op2)
{
  int datatype1, datatype2;

  if ((op1 == NULL) || (op2 == NULL))
    L_punt ("L_same_data_types: op1 and op2 cannot be NULL");

  datatype1 = L_get_data_type (op1);
  datatype2 = L_get_data_type (op2);

  if (datatype1 == datatype2)
    return (1);

  return (0);
}

/* L_check_attr_labels
 * ----------------------------------------------------------------------
 * Check for independence using label and stack attributes
 */

int
L_check_attr_labels (L_Oper * op1, L_Oper * op2, int stack1, int stack2)
{
  L_Operand *field;
  L_Attr *attr1 = NULL, *attr2 = NULL;
  int label1 = 0, label2 = 0;

  if (!op1 || !op2)
    L_punt ("L_check_attr_labels: op1 and op2 cannot be NULL");

  if (L_EXTRACT_BIT_VAL (op1->flags, L_OPER_LABEL_REFERENCE))
    {
      label1 = 1;
      if (!(attr1 = L_find_attr (op1->attr, "label")))
        L_punt ("L_check_attr_labels: label attr not found");
    }

  if (L_EXTRACT_BIT_VAL (op2->flags, L_OPER_LABEL_REFERENCE))
    {
      label2 = 1;
      if (!(attr2 = L_find_attr (op2->attr, "label")))
        L_punt ("L_check_attr_labels: label attr not found");
    }

  if ((label1 && stack2) || (stack1 && label2))
    {
      /* Case 3: Stack and Label accesses are independent */
      return 1;
    }
  else if (label1 && label2)
    {
      /* Case 4: Two label accesses */
      char *lname1, *lname2;

      if (!(field = L_find_attr_field (attr1, L_OPERAND_LABEL)))
        L_punt ("L_check_attr_labels: label field not found");
      lname1 = field->value.l;

      if (!(field = L_find_attr_field (attr2, L_OPERAND_LABEL)))
        L_punt ("L_check_attr_labels: label field not found");
      lname2 = field->value.l;

      if (!strcmp (lname1, lname2))
	{
	  int loffset1, loffset2;
	  int size1, size2;

	  size1 = L_memory_access_size (op1);
	  size2 = L_memory_access_size (op2);

	  if ((field = L_find_attr_field (attr1, L_OPERAND_INT)))
	    {
	      loffset1 = (int) field->value.i;

	      if ((field = L_find_attr_field (attr2, L_OPERAND_INT)))
		{
		  loffset2 = (int) field->value.i;

		  return (L_no_overlap (loffset1, size1, loffset2, size2));
		}
	    }
	}
      else
        {
          return 1;
        }
    }
  else if (L_label_and_reg_access_indep)
    {
      /* Case 5: Register and label access independent -- UNSAFE */
      L_Operand *src1, *src2, *base1, *base2;

      src1 = op1->src[0];
      src2 = op1->src[1];
      if ((L_is_macro (src2) &&
	   ((src2->value.mac == L_MAC_SP) || (src2->value.mac == L_MAC_FP))) ||
	  L_is_label (src2) || L_is_int_constant (src1))
	base1 = src2;
      else
	base1 = src1;

      src1 = op2->src[0];
      src2 = op2->src[1];
      if ((L_is_macro (src2) &&
	   ((src2->value.mac == L_MAC_SP) || (src2->value.mac == L_MAC_FP))) ||
	  L_is_label (src2) || L_is_int_constant (src1))
	base2 = src2;
      else
	base2 = src1;

      if ((label1 && L_is_reg (base2)) ||
	  (L_is_reg (base1) && label2))
	return 1;
    }

  return 0;
}

/* L_check_different_parameters
 * ----------------------------------------------------------------------
 * Accesses are to different function parameters (FORTRAN assumptions)
 */

int
L_check_different_parameters (L_Oper * op1, L_Oper * op2)
{
  int i, different;
  L_Attr *attr1, *attr2;
  Set param1, param2;

  if ((op1 == NULL) || (op2 == NULL))
    L_punt ("L_check_different_parameters: op1 and op2 cannot be NULL");

  attr1 = L_find_attr (op1->attr, "param");
  attr2 = L_find_attr (op2->attr, "param");

  if ((attr1 == NULL) || (attr2 == NULL))
    return (0);

  param1 = param2 = NULL;
  for (i = 0; i < attr1->max_field; i++)
    if (L_is_int_constant (attr1->field[i]))
      param1 = Set_add (param1, (int) attr1->field[i]->value.i);

  for (i = 0; i < attr2->max_field; i++)
    if (L_is_int_constant (attr2->field[i]))
      param2 = Set_add (param2, (int) attr2->field[i]->value.i);

  different = Set_intersect_empty (param1, param2);
  Set_dispose (param1);
  Set_dispose (param2);

  return (different);
}


int
L_check_different_iteration (L_Cb * cb, L_Oper * op1, L_Oper * op2)
{
  L_Attr *attr;
  L_Operand *field;
  int iter1, iter2;

  if ((cb == NULL) || (op1 == NULL) || (op2 == NULL))
    L_punt
      ("L_check_different_iteration: cb, op1, op2 not allowed to be NULL");

  iter1 = iter2 = 0;

  attr = L_find_attr (cb->attr, "DOALL");
  if (attr == NULL)
    return (0);

  attr = L_find_attr (op1->attr, "iter");
  if (attr != NULL)
    {
      field = L_find_attr_field (attr, L_OPERAND_INT);
      if (field != NULL)
        iter1 = (int) field->value.i;
      else
        L_punt ("L_check_different_iteration: iter attr has no int field");
    }

  attr = L_find_attr (op2->attr, "iter");
  if (attr != NULL)
    {
      field = L_find_attr_field (attr, L_OPERAND_INT);
      if (field != NULL)
        iter2 = (int) field->value.i;
      else
        L_punt ("L_check_different_iteration: iter attr has no int field");
    }

  if ((iter1 == 0) || (iter2 == 0))
    return (0);

  return (iter1 != iter2);
}


int
L_opcode_ctype2 (L_Oper * op)
{
  if (op == NULL)
    L_punt ("L_opcode_ctype2: op is NULL");
  switch (op->opc)
    {
    case Lop_ST_C:
    case Lop_ST_PRE_C:
    case Lop_ST_POST_C:
    case Lop_LD_C:
    case Lop_LD_PRE_C:
    case Lop_LD_POST_C:
    case Lop_LD_UC:
    case Lop_LD_PRE_UC:
    case Lop_LD_POST_UC:
    case Lop_ST_C2:
    case Lop_ST_PRE_C2:
    case Lop_ST_POST_C2:
    case Lop_LD_C2:
    case Lop_LD_PRE_C2:
    case Lop_LD_POST_C2:
    case Lop_LD_UC2:
    case Lop_LD_PRE_UC2:
    case Lop_LD_POST_UC2:
    case Lop_ST_I:
    case Lop_ST_PRE_I:
    case Lop_ST_POST_I:
    case Lop_LD_I:
    case Lop_LD_PRE_I:
    case Lop_LD_POST_I:
    case Lop_LD_UI:
    case Lop_LD_PRE_UI:
    case Lop_LD_POST_UI:
    case Lop_ST_Q:
    case Lop_ST_PRE_Q:
    case Lop_ST_POST_Q:
    case Lop_LD_Q:
    case Lop_LD_PRE_Q:
    case Lop_LD_POST_Q:
      return (M_native_int_register_ctype ());
    case Lop_ST_F:
    case Lop_ST_PRE_F:
    case Lop_ST_POST_F:
    case Lop_LD_F:
    case Lop_LD_PRE_F:
    case Lop_LD_POST_F:
      return (L_CTYPE_FLOAT);
    case Lop_ST_F2:
    case Lop_ST_PRE_F2:
    case Lop_ST_POST_F2:
    case Lop_LD_F2:
    case Lop_LD_PRE_F2:
    case Lop_LD_POST_F2:
      return (L_CTYPE_DOUBLE);
    case Lop_PRED_ST:
    case Lop_PRED_ST_BLK:
    case Lop_PRED_LD:
    case Lop_PRED_LD_BLK:
      return (L_CTYPE_PREDICATE);
    default:
      fprintf (stderr, "op %d opcode %s opc %d\n", op->id, op->opcode,
               op->opc);
      L_punt ("L_opcode_ctype2: Unsupported opcode!");
      return (0);
    }
}


void
L_test_memory_disamb (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *oper1, *oper2;
  int indep;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (oper1 = cb->first_op; oper1 != NULL; oper1 = oper1->next_op)
        {
          if (!(L_general_load_opcode (oper1) ||
                L_general_store_opcode (oper1)))
            continue;
          for (oper2 = oper1->next_op; oper2 != NULL; oper2 = oper2->next_op)
            {
              if (!(L_general_load_opcode (oper2) ||
                    L_general_store_opcode (oper2)))
                continue;
              indep = L_independent_memory_ops (cb, oper1, oper2, 0);
              if (indep)
                fprintf (stderr, "op %d and op %d are INDEPENDENT!\n",
                         oper1->id, oper2->id);
              else
                fprintf (stderr, "op %d and op %d are DEPENDENT!\n",
                         oper1->id, oper2->id);
            }
        }
    }
}


/*===========================================================================*/
/*
 *      Associate labels with memory operations to aid with mem disamb.
 *      Labels are stored in attribute fields of the operations.  The memory
 *      disambiguator looks at both the src operands of the memory op and any
 *      label attributes to make its decision.
 *      NOTE: this is not a very smart algorithm at the present time but works
 *      sufficiently immeditialy after Lcode generation.
 */
/*===========================================================================*/

static int L_ind_flow_dep;

/*
 *      Return 1 if op2 is flow dependent on op1, be conservative and only
 *      check dest1 vs src1 and src2.  This is because want to check flow
 *      dependence for addr calc, so want to exclude src3 of store from the
 *      flow dep check
 */
static int
L_flow_dependent (L_Oper * op1, L_Oper * op2)
{
  int i, j;
  /* upper bound should be L_max_dest_operand for general case */
  for (i = 0; i < 1; i++)
    {
      if (op1->dest[i] == NULL)
        continue;
      /* upper bound should be L_max_src_operand for general case */
      for (j = 0; j < 2; j++)
        {
          if (op2->src[i] == NULL)
            continue;
          if (!L_same_operand (op1->dest[i], op2->src[j]))
            continue;
          if (L_no_defs_between (op1->dest[i], op1, op2))
            return 1;
        }
    }
  return 0;
}


static void
L_indirectly_flow_dependent (L_Oper * op1, L_Oper * op2)
{
  L_Oper *ptr;
  for (ptr = op1->next_op; ptr != NULL; ptr = ptr->next_op)
    {
      if (L_ind_flow_dep)
        return;
      if (!L_flow_dependent (op1, ptr))
        continue;
      if (ptr == op2)
        {
          L_ind_flow_dep = 1;
          return;
        }
      /* cannot continue thru load or store */
      if (L_general_load_opcode (ptr) || L_general_store_opcode (ptr))
        continue;
      L_indirectly_flow_dependent (ptr, op2);
    }
}


/*
 *      SAM 6-94: Change so generate lab and labO attributes to save
 *      array/struct names.  Lab = name alone, labO = name + offset.
 */
void
L_find_memory_labels (L_Func * fn)
{
  int offset_flag;
  L_Cb *cb;
  L_Oper *op, *ptr, *label_op, *prev_op;
  L_Attr *attr;
  L_Operand *label_src, *offset_src, *label_field, *offset_field, *reg_src;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (op = cb->first_op; op != NULL; op = op->next_op)
        {

          if (!(L_general_load_opcode (op) || L_general_store_opcode (op)))
            continue;

          reg_src = label_src = offset_src = NULL;
          if (L_is_label (op->src[0]))
            {
              label_src = op->src[0];
              offset_src = op->src[1];
            }
          else if (L_is_label (op->src[1]))
            {
              offset_src = op->src[0];
              label_src = op->src[1];
            }

          /* Hack :P, check prev op for add of label + immediate */
          prev_op = op->prev_op;
          if ((label_src == NULL) && (L_int_add_opcode (prev_op)))
            {
              if (L_is_register (op->src[0]) && L_is_int_zero (op->src[1]))
                {
                  reg_src = op->src[0];
                }
              else if (L_is_register (op->src[1])
                       && L_is_int_zero (op->src[0]))
                {
                  reg_src = op->src[1];
                }
              if (L_same_operand (reg_src, prev_op->dest[0]))
                {
                  if (L_is_label (prev_op->src[0]))
                    {
                      label_src = prev_op->src[0];
                      offset_src = prev_op->src[1];
                    }
                  else if (L_is_label (prev_op->src[1]))
                    {
                      offset_src = prev_op->src[0];
                      label_src = prev_op->src[1];
                    }
                }
            }

          offset_flag = 0;
          label_field = NULL;
          offset_field = NULL;

          if (label_src)
            {
              if (L_is_int_constant (offset_src))
                offset_flag = 1;

              attr = L_find_attr (op->attr, "label");

              /* do some error checking here */
              if (attr != NULL)
                {
                  label_field = L_find_attr_field (attr, L_OPERAND_LABEL);
                  /* existing one must have label field */
                  if (label_field == NULL)
                    L_punt
                      ("L_find_memory_labels: attr does not contain label");
                  /* existing and new labels should match */
                  if (strcmp (label_field->value.l, label_src->value.l))
                    L_punt ("L_find_memory_labels: "
                            "new and existing label dont match");
                  /* existing and new offsets should match */
                  offset_field = L_find_attr_field (attr, L_OPERAND_INT);
                  if ((offset_flag) && (offset_field) &&
                      ((int) offset_field->value.i !=
                       (int) offset_src->value.i))
                    L_punt ("L_find_memory_labels: "
                            "new and existing offset dont match");
                }

              if (attr == NULL)
                {
                  attr = L_new_attr ("label", 2);
                  op->attr = L_concat_attr (op->attr, attr);
                }

              if (label_field == NULL)
                L_set_label_attr_field (attr, 0, label_src->value.l);

              if ((offset_flag) && (offset_field == NULL))
                L_set_int_attr_field (attr, 1, (int) offset_src->value.i);

              /* SAM 8-94: flag label offset lds/sts as safe, this will later
                 become unnecessary when Rogers stuff is fully integrated */
              if ((offset_flag) || (offset_field != NULL))
                op->flags = L_SET_BIT_FLAG (op->flags, L_OPER_SAFE_PEI);

              op->flags = L_SET_BIT_FLAG (op->flags, L_OPER_LABEL_REFERENCE);

              continue;
            }

          /*
           *  Search upward for an arithmetic instruction which uses a label,
           *  then see if there is indirect flow dependence connecting label
           *  to memory operation.
           */
          label_op = NULL;
          for (ptr = op->prev_op; ptr != NULL; ptr = ptr->prev_op)
            {
              if (!(L_general_arithmetic_opcode (ptr) ||
                    L_general_move_opcode (ptr)))
                continue;
              if (L_is_label (ptr->src[0]))
                {
                  label_src = ptr->src[0];
                  label_op = ptr;
                  break;
                }
              if (L_is_label (ptr->src[1]))
                {
                  label_src = ptr->src[1];
                  label_op = ptr;
                  break;
                }
            }
          if (label_op == NULL)
            continue;
          L_ind_flow_dep = 0;
          L_indirectly_flow_dependent (label_op, op);
          if (!L_ind_flow_dep)
            continue;

          attr = L_find_attr (op->attr, "label");

          /* do some error checking here */
          if (attr != NULL)
            {
              label_field = L_find_attr_field (attr, L_OPERAND_LABEL);
              /* existing one must have label field */
              if (label_field == NULL)
                L_punt ("L_find_memory_labels: attr does not contain label");
              /* existing and new labels should match */
              if (strcmp (label_field->value.l, label_src->value.l))
                L_punt
                  ("L_find_memory_labels: new and existing label dont match");
            }

          if (attr == NULL)
            {
              attr = L_new_attr ("label", 1);
              op->attr = L_concat_attr (op->attr, attr);
            }

          if (label_field == NULL)
            L_set_label_attr_field (attr, 0, label_src->value.l);

          op->flags = L_SET_BIT_FLAG (op->flags, L_OPER_LABEL_REFERENCE);
        }
    }


  /* make sure oper flags are set correctly, in case people inserted labels
     into the Lcode, but didnt set the flag :)  */
  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (op = cb->first_op; op != NULL; op = op->next_op)
        {
          if (!(L_general_load_opcode (op) || L_general_store_opcode (op)))
            continue;
          attr = L_find_attr (op->attr, "label");
          if (attr != NULL)
            op->flags = L_SET_BIT_FLAG (op->flags, L_OPER_LABEL_REFERENCE);
        }
    }
}

/*===========================================================================*/
/*
 *      Indentify memory accesses which use as an address an incoming
 *      parameter to the function.  This allows the memory disambiguator
 *      in some cases to indentify 2 mem accesses as independent if they
 *      are known to be from different parameters.
 */
/*===========================================================================*/

static int *level;
static int *cb_place;

/*
 *      This function requires dominator info!!!!!!!!!!
 */
static void
L_levelize (L_Func * fn)
{
  int *input, *stack, top_stack, bottom_stack, i, tmp, max, place;
  L_Cb *cb, *src_cb, *dest_cb;
  L_Flow *flow, *flow2;

  max = 0;
  level = (int *) malloc (sizeof (int) * (fn->max_cb_id + 1));
  input = (int *) malloc (sizeof (int) * (fn->max_cb_id + 1));
  cb_place = (int *) malloc (sizeof (int) * (fn->max_cb_id + 1));
  stack = (int *) malloc (sizeof (int) * (fn->max_cb_id + 1));

  for (i = 0; i <= fn->max_cb_id; i++)
    {
      level[i] = 0;
      cb_place[i] = 0;
      input[i] = -1;
    }

  /* record the number of input arcs to each cb, exclude loop backward arcs */
  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      tmp = 0;
      for (flow = cb->src_flow; flow != NULL; flow = flow->next_flow)
        {
          src_cb = flow->src_cb;
          if (!L_in_cb_DOM_set (src_cb, cb->id))
            tmp++;
        }
      input[cb->id] = tmp;
#ifdef DEBUG_LEVEL
      fprintf (stderr, "CB = %d, n_input = %d\n", cb->id, input[cb->id]);
#endif
    }


  /* Insert all elements into stack with no incoming arcs */
  top_stack = 0;
  bottom_stack = 0;
  for (i = 0; i <= fn->max_cb_id; i++)
    {
      if (input[i] == 0)
        {
          stack[bottom_stack++] = i;
          level[i] = 1;
#ifdef DEBUG_LEVEL
          fprintf (stderr, "bb = %d, level = 1\n", i);
#endif
        }
    }

  /*
   * topological sort
   * start from the entrance cb, delete 1 from input of all dest cb
   * put cb in queue only if input number is 0
   * assign level number equal to max level number of all src cb's plus 1
   * stop when queue is empty
   */
  while (top_stack != bottom_stack)
    {
      cb = L_cb_hash_tbl_find (fn->cb_hash_tbl, stack[top_stack++]);
      for (flow = cb->dest_flow; flow != NULL; flow = flow->next_flow)
        {
          /* if dest cb input becomes 0, add to stack */
          dest_cb = flow->dst_cb;
          if (--input[dest_cb->id] == 0)
            {
              stack[bottom_stack++] = dest_cb->id;
              max = 0;
              /* check backward flow arc and find all src cb level number */
              for (flow2 = dest_cb->src_flow; flow2 != NULL;
                   flow2 = flow2->next_flow)
                {
                  src_cb = flow2->src_cb;
                  if (!L_in_cb_DOM_set (src_cb, dest_cb->id))
                    max = IMPACT_MAX (max, level[src_cb->id]);
                }
              level[dest_cb->id] = max + 1;
#ifdef DEBUG_LEVEL
              fprintf (stderr, "cb = %d level = %d\n", dest_cb->id, max + 1);
#endif
            }
        }
    }

  /* Place cb's according to level */
  place = 0;
  max++;
#ifdef DEBUG_LEVEL
  fprintf (stderr, "max level = %d\n", max);
#endif
  for (tmp = 1; tmp <= max; tmp++)
    {
      for (i = 0; i <= fn->max_cb_id; i++)
        {
          if (level[i] == tmp)
            cb_place[place++] = i;
        }
    }
#ifdef DEBUG_LEVEL
  for (i = 0; i < place; i++)
    {
      fprintf (stderr, "cb_place[%d] = %d level = %d\n", i, cb_place[i],
               level[cb_place[i]]);
    }
#endif
  free (input);
  free (stack);
}


/*
 *      This should probably only be called immediately after Lcode is formed.
 *      Otherwise, it may not be safe.  Note we are only concerned with integer
 *      parameters, since we are only looking at addresses for ld/st's.
 */
void
L_find_incoming_parameters (L_Func * fn)
{
  int i, j, k, l, index, check_src1, check_src2;
  L_Cb *cb;
  L_Oper *op;
  Set param[L_MAX_PARAM];
  L_Operand *dest, *src, *src1, *src2, *field;
  L_Attr *attr;

  for (i = 0; i < L_MAX_PARAM; i++)
    param[i] = NULL;

  L_levelize (fn);

  cb = fn->first_cb;
  if (cb_place[0] != cb->id)
    L_punt ("L_find_incoming_parameters: placement is corrupted");
  index = 0;
  for (op = cb->first_op; op != NULL; op = op->next_op)
    {
      /* only interested in integer parameters!!! */
      if ((L_is_opcode (Lop_MOV, op)) && (L_is_register (op->dest[0])))
        {
          param[index] = Set_add (param[index], op->dest[0]->value.r);
          /* only handle first k parameters */
          if (++index >= L_MAX_PARAM)
            break;
        }
      else if ((L_is_opcode (Lop_LD_I, op)) && (L_is_register (op->dest[0])))
        {
          param[index] = Set_add (param[index], op->dest[0]->value.r);
          /* only handle first k parameters */
          if (++index >= L_MAX_PARAM)
            break;
        }
    }

  for (i = 1; i < fn->max_cb_id; i++)
    {
      cb = L_cb_hash_tbl_find (fn->cb_hash_tbl, cb_place[i]);
      if (cb == NULL)
        continue;
      for (op = cb->first_op; op != NULL; op = op->next_op)
        {
          /*
           *  if any src operand is in param[], then add all dest int reg
           *    operands to param[] set.
           */
          for (j = 0; j < L_max_src_operand; j++)
            {
              src = op->src[j];
              if ((src == NULL) ||
                  (!L_is_register (src)) || (!L_is_ctype_integer (src)))
                continue;
              for (k = 0; k < index; k++)
                {
                  if (!Set_in (param[k], src->value.r))
                    continue;
                  for (l = 0; l < L_max_dest_operand; l++)
                    {
                      dest = op->dest[l];
                      if ((dest == NULL) ||
                          (!L_is_register (dest)) ||
                          (L_is_ctype_integer (dest)))
                        continue;
                      param[k] = Set_add (param[k], dest->value.r);
                    }
                }
            }
        }
    }
#ifdef DEBUG_PARAM
  for (i = 0; i < index; i++)
    Set_print (stderr, "param", param[i]);
#endif

  /*
   * Install param attributes for loads which use (direct/indirect) a paramater
   * as part of its address.
   */
  for (i = 0; i < fn->max_cb_id; i++)
    {
      cb = L_cb_hash_tbl_find (fn->cb_hash_tbl, cb_place[i]);
      if (cb == NULL)
        continue;
      for (op = cb->first_op; op != NULL; op = op->next_op)
        {
          if (!(L_general_load_opcode (op) || L_general_store_opcode (op)))
            continue;
          check_src1 = 0;
          check_src2 = 0;
          src1 = op->src[0];
          src2 = op->src[1];
          if ((src1) && (L_is_register (src1)) && (L_is_ctype_integer (src1)))
            check_src1 = 1;
          if ((src2) && (L_is_register (src2)) && (L_is_ctype_integer (src2)))
            check_src2 = 1;
          for (j = 0; j < index; j++)
            {
              if ((check_src1) && (Set_in (param[j], src1->value.r)))
                {
                  attr = L_find_attr (op->attr, "param");
                  if (attr == NULL)
                    {
                      attr = L_new_attr ("param", 1);
                      op->attr = L_concat_attr (op->attr, attr);
                      L_set_int_attr_field (attr, 0, j);
                    }
                  else
                    {
                      field = L_find_int_attr_field (attr, j);
                      if (field)
                        continue;
                      L_set_int_attr_field (attr, attr->max_field, j);
                    }
                }
              else if ((check_src2) && (Set_in (param[j], src2->value.r)))
                {
                  attr = L_find_attr (op->attr, "param");
                  if (attr == NULL)
                    {
                      attr = L_new_attr ("param", 1);
                      op->attr = L_concat_attr (op->attr, attr);
                      L_set_int_attr_field (attr, 0, j);
                    }
                  else
                    {
                      field = L_find_int_attr_field (attr, j);
                      if (field)
                        continue;
                      L_set_int_attr_field (attr, attr->max_field, j);
                    }
                }
            }
        }
    }

  free (level);
  free (cb_place);
  for (i = 0; i < index; i++)
    Set_dispose (param[i]);
}




/************************************************************************/
/*
 * Given a mem op, return a pointer to the base and offset
 * or return 0 if the memory does not have two operands
 */
static int
L_memory_get_base_offset (L_Oper * op, L_Operand ** p_base, 
			  L_Operand ** p_offset, int *p_used_attrs,
			  int attr_allowed)
{
  L_Operand *src1, *src2;
  L_Attr *attr;
  int first_operand, num_operand;

  src1 = src2 = NULL;
  *p_base = NULL;
  *p_offset = NULL;

  /* Use stack / label information if available */

  if (attr_allowed)
    {
      if (L_EXTRACT_BIT_VAL (op->flags, L_OPER_STACK_REFERENCE) &&
	  (attr = L_find_attr (op->attr, "stack")))
	{
	  *p_base = attr->field[0];
	  *p_offset = attr->field[1];
	  *p_used_attrs = 1;
	  return 1;
	}
      else if (L_EXTRACT_BIT_VAL (op->flags, L_OPER_LABEL_REFERENCE) &&
	       (attr = L_find_attr (op->attr, "label")))
	{
	  *p_base = attr->field[0];

	  if ((attr->max_field > 1) && (attr->field[1] != NULL))
	    *p_offset = attr->field[1];
	  else
	    *p_offset = NULL;
	  *p_used_attrs = 1;
	  return 1;
	}
    }

  /*  
   * SAM 4-94, call mspec to determine which are the address operands,
   * for now we only know how to handle <= 2 address operands, so if either
   * is >2 return NOT independent.  Note Lcode requires at least 2 address
   * operands, so any less is an error 
   */

  M_get_memory_operands (&first_operand, &num_operand, op->proc_opc);

  /* Get src1 and src2 */

  if (num_operand == 1)
    {
      src1 = op->src[first_operand];
      src2 = NULL;
    }
  else if (num_operand == 2)
    {
      src1 = op->src[first_operand];
      src2 = op->src[first_operand + 1];
    }
  else
    {
      /* More than two operands  */
      return 0;
    }

  /* Pick the base and offset.  Note there are some ambiguous cases
   * in the else... reg+reg loads, for example.
   */

  if (L_is_int_constant (src1) || (M_is_stack_operand (src2) ||
				   L_is_label (src2)))
    {
      *p_base = src2;
      *p_offset = src1;
    }
  else
    {
      *p_base = src1;
      *p_offset = src2;
    }

  *p_used_attrs = 0;

  return 1;
}


/************************************************************************/
/*
 *      Assumpts for this funct:
 *              1. cbA dominates cbB
 *              2. operand is source operand of opA
 *      ret 1 if opA is only expression of operand that reachs opB
 */
static int
L_mem_same_def_reaches (L_Operand * operand, L_Cb * cbA, L_Oper * opA,
                          L_Cb * cbB, L_Oper * opB)
{
  int i;
  L_Oper *ptr, *find;

  if (!operand)
    return 1;

  if (cbA == cbB)
    {
      /* In the same cb */
      find = NULL;
      for (ptr = cbA->first_op; ptr != NULL; ptr = ptr->next_op)
        {
          if (find)
            {
              /* Reached other op without a redef */
              if (ptr == find)
                return 1;

              /* Does this op redef operand */
              for (i = 0; i < L_max_dest_operand; i++)
		if (L_same_operand (ptr->dest[i], operand))
		  return 0;
            }
          else
            {
              /* Have we found either opA or opB */
              if (ptr == opA)
                find = opB;
              else if (ptr == opB)
                find = opA;
            }
        }

      L_punt ("L_mem_same_def_reaches: opA or opB not found");
    }

  for (ptr = cbB->first_op; ptr; ptr = ptr->next_op)
    {
      if (ptr == opB)
        break;
      for (i = 0; i < L_max_dest_operand; i++)
	if (L_same_operand (ptr->dest[i], operand))
	  return 0;
    }
  if (ptr != opB)
    L_punt ("L_mem_same_def_reaches: opB not found");
  return (L_in_cb_EIN_set (cbB, opA));
}


/*
 * L_check_offset
 * ----------------------------------------------------------------------
 * Assumes a 2-operand memory format and that the two operations
 * have already been proven to have a common base.  The function will
 * set *p_indep, *p_dep, and/or *p_partial_dep, or none of the above,
 * as appropriate.
 */
static void
L_check_offset (L_Cb *cb1, L_Oper *op1, L_Operand *ofst1,
		L_Cb *cb2, L_Oper *op2, L_Operand *ofst2,
		int *p_indep, int *p_dep, int *p_partial_dep)
{
  int size1, size2;

  size1 = L_memory_access_size (op1);
  size2 = L_memory_access_size (op2);

  if (L_is_int_constant (ofst1) && L_is_int_constant (ofst2))
    {
      int v1, v2;
      v1 = (int) ofst1->value.i;
      v2 = (int) ofst2->value.i;

      L_classify_overlap (v1, size1, v2, size2,
			  p_indep, p_dep, p_partial_dep);
    }
  else if (cb1 && cb2 && L_same_operand (ofst1, ofst2))
    {
      if ((L_in_cb_DOM_set (cb2, cb1->id) &&
	   L_mem_same_def_reaches (ofst1, cb1, op1, cb2, op2)) ||
	  (L_in_cb_DOM_set (cb1, cb2->id) &&
	   L_mem_same_def_reaches (ofst2, cb2, op2, cb1, op1)))
	{
	  *p_dep = (size1 >= size2);
	  *p_partial_dep = 1;	  
	}
    }
  return;
}


/************************************************************************/

/* Moved here from l_indep_mem.c so that codegenerators can have their
 * own version of  L_independent_memory_ops. -JCG 10/99
 */

/************************************************************************/
/* Assumes other calls to predicate graph for checking intersecting ops */
int
L_memory_dependence_relation (L_Oper * mem_op, L_Oper * jsr_op)
{
  L_Attr *mem_attr, *jsr_attr;
  int mem_index, jsr_index;
  int mem_id, jsr_id;

  if ((mem_attr = L_find_attr (mem_op->attr, "tm")) != NULL)
    {
      if ((jsr_attr = L_find_attr (jsr_op->attr, "tm")) != NULL)
        {
          for (mem_index = 0; mem_index < mem_attr->max_field; mem_index++)
            {
              mem_id = (int) mem_attr->field[mem_index]->value.i;
              for (jsr_index = 0; jsr_index < jsr_attr->max_field;
                   jsr_index++)
                {
                  jsr_id = (int) jsr_attr->field[jsr_index]->value.i;
                  if (mem_id == jsr_id)
                    return (1);
                }
            }
        }
    }

  return (0);
}


static int
L_mem_op_uses_stack (L_Oper * oper)
{
  int i;

  for (i = 0; i < L_max_src_operand; i++)
    {
      if (M_is_stack_operand (oper->src[i]))
        return (1);
    }

  return (0);
}


int
L_sync_no_jsr_dependence (L_Oper * jsr, L_Oper * oper)
{
  int indep;
  L_Attr *attr1, *attr2;
  int max_field1, max_field2;
  int dep_flags = SET_NONLOOP_CARRIED (0);

  indep = L_depinfo_indep_mem_ops (jsr, oper, dep_flags);

  /* we don't have good sync arcs for jsrs which return structs, so
     if the jsr returns a struct, and the mem op involves the
     SP, then we must assume a dependence */

  if ((L_find_attr (jsr->attr, "ret_st")) && (L_mem_op_uses_stack (oper)))
    indep = 0;

  /* BCC we don't handle parameters passed through memory well either 8/99 */
  if ((attr1 = L_find_attr (jsr->attr, "tm")) &&
      (attr2 = L_find_attr (oper->attr, "tm")))
    {
      for (max_field1 = 0;
           indep != 0 && max_field1 < attr1->max_field; max_field1++)
        for (max_field2 = 0;
             indep != 0 && max_field2 < attr2->max_field; max_field2++)
          if ((int) attr1->field[max_field1]->value.i ==
              (int) attr2->field[max_field2]->value.i)
            indep = 0;
    }

  /* BCC/ITI - struct */
  if ((attr1 = L_find_attr (jsr->attr, "tmso")) &&
      (attr2 = L_find_attr (oper->attr, "tms")))
    indep = 0;

  /* BCC/ITI -  spill code */
  if (L_spill_code (oper))
    indep = 0;

  if ((attr1 = L_find_attr(oper->attr,"stack")))
    {
      if (!L_is_macro(attr1->field[0]))
	return 0;
      if (attr1->field[0]->value.mac == L_MAC_OP ||
	  attr1->field[0]->value.mac == L_MAC_IP)
	return 0;
    }

  /* BCC/ITI -  avoid bypassing setjmp (sigsetjmp on linux) */
  if (L_load_opcode (oper) &&
      (jsr->src[0]->type == L_OPERAND_LABEL) &&
      !(strcmp (jsr->src[0]->value.s, "_$fn___sigsetjmp") &&
        strcmp (jsr->src[0]->value.s, "_$fn___setjmp")))
    indep = 0;

  return (indep);
}


/************************************************************************/
/* Assumes other calls to predicate graph for checking intersecting ops */
int
L_independent_memory_and_jsr (L_Cb * cb, L_Oper * mem_op, L_Oper * jsr_op)
{
  /* Dependence flags cannot be profile based */
  int dep_flags = SET_NONLOOP_CARRIED (0);

  if (!L_stack_reference (mem_op))
    {
      /* Check for using acc specs or sync arcs */

      if (L_func_acc_specs)
	return L_mem_indep_acc_specs (mem_op, jsr_op, dep_flags);
      else if (!L_ignore_sync_arcs_for_opti &&
	  L_use_sync_arcs && L_func_contains_dep_pragmas)
	return L_analyze_syncs_for_independence (mem_op, jsr_op, dep_flags);
    }

  if (L_general_load_opcode (mem_op) && L_side_effect_free_sub_call (jsr_op))
    return 1;

  return 0;
}


int
L_is_ida_memory_ops (L_Cb * cb1, L_Oper * op1, L_Cb * cb2, L_Oper * op2,
                     int dep_flags)
{
  int r_indep, r_dep, res;

  /* Two subroutines are always ambiguous */
  if (L_subroutine_call_opcode (op1) && L_subroutine_call_opcode (op2))
    return MEM_AMB;

  /* Use side effect free subroutines and sync arcs for jsr vs. mem */
  if (L_subroutine_call_opcode (op1))
    {
      if (L_has_fragile_macro_operand (op2))
        return MEM_AMB;
      if (L_side_effect_free_sub_call (op1) ||
          L_independent_memory_and_jsr (NULL, op2, op1))
        return MEM_IND;
      else
        return MEM_AMB;
    }
  else if (L_subroutine_call_opcode (op2))
    {
      if (L_has_fragile_macro_operand (op1))
        return MEM_AMB;
      if (L_side_effect_free_sub_call (op2) ||
          L_independent_memory_and_jsr (NULL, op1, op2))
        return MEM_IND;
      else
        return MEM_AMB;
    }

  /* Load and store dependence processing */
  L_check_memory_op_dependence (cb1, op1, cb2, op2, dep_flags, &r_indep,
                                &r_dep);

  /* Use results to set appropriate flag */
  if (r_dep)
    {
      if (!r_indep)
        {
	  res = MEM_DEP;
        }
      else
        {
          fprintf (stderr, "WARNING: "
                   "L_is_ida_memory_ops: Ops %d and %d both indep and dep\n",
                   op1->id, op2->id);
          L_print_oper (stderr, op1);
          L_print_oper (stderr, op2);

          if (!L_ignore_sync_arcs_for_opti &&
              L_use_sync_arcs && L_func_contains_dep_pragmas)
	    {
	      res = MEM_IND;
	    }
          else
            {
              L_punt ("This should not happen without sync arcs\n");
	      res = 0;
            }
        }
    }
  else
    {
      res = r_indep ? MEM_IND : MEM_AMB;
    }

  return res;
}


/************************************************************************/
/* 
   NOTE: cb1 and cb2 can be NULL. No dataflow analysis results
   will be used if this is the case. Loop iteration analysis
   will only be run if cb1 and cb2 are the same cb.
*/
void
L_check_memory_op_dependence (L_Cb * cb1, L_Oper * op1,
                              L_Cb * cb2, L_Oper * op2,
                              int dep_flags, int *r_indep, int *r_dep)
{
  L_Operand *base1, *base2, *offset1, *offset2;

  int label1 = 0, label2 = 0;
  int stack1 = 0, stack2 = 0;

  int used_attrs = 0;

  int indep = 0, dep = 0, partial_dep = 0, unsafe = 0;

  if (!op1 || !op2)
    L_punt ("L_check_memory_op_dependence: op is NULL");

  if (L_debug_memory_disamb)
    {
      fprintf (stderr, "CHECK: op %d cb %d and op %d cb %d: \n",
               op1->id, cb1 ? cb1->id : -1, op2->id, cb2 ? cb2->id : -1);
      L_print_oper (stderr, op1);
      L_print_oper (stderr, op2);
    }
  
  /* Override analysis
   * ----------------------------------------------------------------------
   */

  if (L_load_store_always_indep)
    {
      *r_indep = 1;
      *r_dep = 0;
      return;
    }
  else if (L_mem_never_indep)
    {
      *r_indep = 0;
      *r_dep = 0;
      return;
    }
  else if (op1 == op2)
    {
      *r_indep = 0;
      *r_dep = 1;
      return;
    }

  /* (stk ptr / const label) base + (int const) offset operations
   * ----------------------------------------------------------------------
   * Incorporate info from "stack" and "label" attrs, if present.
   */

  if (!L_memory_get_base_offset (op1, &base1, &offset1, &used_attrs, 1) ||
      !L_memory_get_base_offset (op2, &base2, &offset2, &used_attrs, 1))
    {
      /* More than 2 src operands, Ambiguous */
      *r_indep = 0;
      *r_dep = 0;
      return;
    }

  if (M_is_stack_operand (base1))
    stack1 = 1;
  else if (L_is_label (base1))
    label1 = 1;
  
  if (M_is_stack_operand (base2))
    stack2 = 1;
  else if (L_is_label (base2))
    label2 = 1;

  /* Traditional dependence checking routines -- definite (safe) cases
   * ----------------------------------------------------------------------
   */

  if (L_constant_load (op1) || L_constant_load (op2))
    {
      /* CASE 1: A const load depends on nothing */
	
      indep = 1;

      if (L_debug_memory_disamb)
	fprintf (stderr, "CASE 1 i%dd%dp%d\n", indep, dep, partial_dep);
    }
  else if (L_spill_code (op1) || L_spill_code (op2))
    {
      if (L_spill_code (op1) ^ L_spill_code (op2))
	{
	  /* CASE 2: Spill space is unavailable to other accesses */
	
	  indep = 1;

	  if (L_debug_memory_disamb)
	    fprintf (stderr, "CASE 2 i%dd%dp%d\n", indep, dep, partial_dep);
	}
      else
	{
	  /* CASE 3: All spill/fill ops are annotated with "offset"
	   * attrs which identify their point of access
	   */

	  int spill_os1, spill_os2;
	  int size1, size2;

	  size1 = L_memory_access_size (op1);
	  size2 = L_memory_access_size (op2);

	  if (!L_spill_offset (op1, &spill_os1) ||
	      !L_spill_offset (op2, &spill_os2))
	    L_punt ("L_check_memory_op_dependence: no spill offset found\n");
	
	  L_classify_overlap (spill_os1, size1, spill_os2, size2,
			      &indep, &dep, &partial_dep);

	  if (L_debug_memory_disamb)
	    fprintf (stderr, "CASE 3 i%dd%dp%d\n", indep, dep, partial_dep);
	}
    }
  else if (stack1 && stack2)
    {
      /* CASE 4: Two non-spill stack accesses */
	
      if (!L_same_operand (base1, base2))
	{
	  /* SUBCASE 4A: Accesses to different stack subframes are
	   * independent */
	    
	  indep = 1;

	  if (L_debug_memory_disamb)
	    fprintf (stderr, "CASE 4A i%dd%dp%d\n", indep, dep, partial_dep);
	}
      else if (offset1 && offset2)
	{
	  /* SUBCASE 4B: Accesses to same stack subframe are independent
	   * if they can be proven not to overlap, and dependent if
	   * they can be proven to overlap. */

	  L_check_offset (cb1, op1, offset1, cb2, op2, offset2,
			  &indep, &dep, &partial_dep);

	  if (L_debug_memory_disamb)
	    fprintf (stderr, "CASE 4B i%dd%dp%d\n", indep, dep, partial_dep);
	}
    }
  else if ((label1 && stack2) || (label2 && stack1))
    {
      /* CASE 5: Stack and global accesses are independent */
	
      indep = 1;

      if (L_debug_memory_disamb)
	fprintf (stderr, "CASE 5 i%dd%dp%d\n", indep, dep, partial_dep);
    }
  else if (label1 && label2)
    {
      /* CASE 6: Two global label accesses with label operands */

      if (strcmp (base1->value.l, base2->value.l))
	{
	  /* SUBCASE 6A: Independent if based on different labels */
	    
	  indep = 1;

	  if (L_debug_memory_disamb)
	    fprintf (stderr, "CASE 6A i%dd%dp%d\n", indep, dep, partial_dep);
	}
      else if (offset1 && offset2)
	{
	  /* SUBCASE 6B: Accesses to same label are independent
	   * if they can be proven not to overlap, and dependent if
	   * they can be proven to overlap. */

	  L_check_offset (cb1, op1, offset1, cb2, op2, offset2,
			  &indep, &dep, &partial_dep);
	    
	  if (L_debug_memory_disamb)
	    fprintf (stderr, "CASE 6B i%dd%dp%d\n", indep, dep, partial_dep);
	}
    }
  else if (cb1 && cb2 && offset1 && offset2 && L_same_operand (base1, base2))
    {
      /* CASE 7: Accesses relative to the same register can
       * be disambiguated on the basis of the offset */

      int same_base_reg = 0;

      if (L_in_cb_DOM_set (cb2, cb1->id))
	{
	  /* cb1 dominates cb2 */
	  if (L_debug_memory_disamb)
	    fprintf (stderr, "op %d dominates.\n", op1->id);
	  same_base_reg =
	    L_mem_same_def_reaches (base1, cb1, op1, cb2, op2);
	}
      else if (L_in_cb_DOM_set (cb1, cb2->id))
	{
	  /* cb2 dominates cb1 */
	  if (L_debug_memory_disamb)
	    fprintf (stderr, "op %d dominates.\n", op2->id);
	  same_base_reg =
	    L_mem_same_def_reaches (base2, cb2, op2, cb1, op1);
	}
      else
	{
	  if (L_debug_memory_disamb)
	    fprintf (stderr, "Neither dominates.\n");
	}

      if (same_base_reg)
	{
	  L_check_offset (cb1, op1, offset1, cb2, op2, offset2,
			  &indep, &dep, &partial_dep);

	  if (L_debug_memory_disamb)
	    fprintf (stderr, "CASE 7 i%dd%dp%d\n", 
		     indep, dep, partial_dep);
	}
    }

  if (!partial_dep && !indep && cb1 && cb2 && used_attrs)
    {
      /* CASE 8: Accesses relative to the same register can
       * be disambiguated on the basis of the offset.
       * Try it again, this time without the attrs */

      L_memory_get_base_offset (op1, &base1, &offset1, &used_attrs, 0);
      L_memory_get_base_offset (op2, &base2, &offset2, &used_attrs, 0);
  
      if (L_same_operand (base1, base2) &&
	  ((L_in_cb_DOM_set (cb2, cb1->id) &&
	    L_mem_same_def_reaches (base1, cb1, op1, cb2, op2)) ||
	   (L_in_cb_DOM_set (cb1, cb2->id) &&
	    L_mem_same_def_reaches (base2, cb2, op2, cb1, op1))))
	{
	  L_check_offset (cb1, op1, offset1, cb2, op2, offset2,
			  &indep, &dep, &partial_dep);

	  if (L_debug_memory_disamb)
	    fprintf (stderr, "CASE 8 i%dd%dp%d\n", 
		     indep, dep, partial_dep);
	}
    }

  /* Apply unsafe analyses
   * ----------------------------------------------------------------------
   */

  if (!partial_dep && !indep)
    {
      if (L_incoming_param_indep)
	indep = indep || L_check_different_parameters (op1, op2);
	
      if (L_use_loop_iter && cb1 && cb2 && (cb1->id == cb2->id))
	indep = indep || L_check_different_iteration (cb1, op1, op2);

      if (L_label_and_reg_access_indep && !indep && !partial_dep && 
	  ((label1 && !label2) || (label2 && !label1)))
	{
	  /* CASE 9: Register and label access indep (UNSAFE) */
	  
	  indep = 1;

	  if (L_debug_memory_disamb)
	    fprintf (stderr, "CASE 9 i%dd%dp%d\n", 
		     indep, dep, partial_dep);
	}
      else if (L_sp_and_reg_access_indep &&
	       ((stack1 && !stack2) || (stack2 && !stack1)))
	{
	  /* CASE 10: Register and stack access indep (UNSAFE) */
	  
	  indep = 1;

	  if (L_debug_memory_disamb)
	    fprintf (stderr, "CASE 10 i%dd%dp%d\n", indep, dep, partial_dep);
	}

      /* Unsafe option */
      if (L_diff_data_types_indep &&
	  !L_find_attr (op1->attr, "block_move") &&
	  !L_find_attr (op2->attr, "block_move"))
	indep = indep || (!L_same_data_types (op1, op2));

      if (indep)
	unsafe = 1;
    }

  /* Acc Spec analysis
   * ----------------------------------------------------------------------
   * Examine acc specs when other analyses are inconclusive.
   */

  {
    int asrv;
    
    if (!partial_dep && !indep && L_func_acc_specs &&
	((op1->acc_info && op2->acc_info) || (!stack1 || !stack2)) &&
	(asrv = L_mem_indep_acc_specs (op1, op2, dep_flags)))
      {
	indep = asrv; /* 1 = indep by acc spec; 2 = indep by sync arc */
	dep = 0;
      }
  }

  /* Sync arc analysis
   * ----------------------------------------------------------------------
   * Examine sync arcs only if other analyses have been inconclusive,
   * sync arcs are available and not disabled, and the query is not
   * between two loads (load-load sync arcs are not generated)
   */

  if (!partial_dep && !indep &&
      L_use_sync_arcs && !L_ignore_sync_arcs_for_opti && 
      L_func_contains_dep_pragmas &&
      !(L_general_load_opcode (op1) && L_general_load_opcode (op2)))
    {
      int sync_indep = 0;

      sync_indep = L_analyze_syncs_for_independence (op1, op2, dep_flags);
      if (sync_indep && L_debug_memory_disamb)
	fprintf (stderr, "CASE SYNC\n");

      /* If indep=1 then sync_indep should =1 */

      if (!sync_indep && indep)
        {
	  if (L_punt_on_sync_arcs_failure)
	    L_punt ("L_independent_memory_ops: sync arcs failed");
	  else if (L_debug_sync_arcs)
	    L_warn ("L_independent_memory_ops: "
		    "sync arcs disagree with traditional,"
		    "(%d->%d)", op1->id, op2->id);
        }
      else if (sync_indep && partial_dep)
	{
	  if (L_punt_on_sync_arcs_failure)
	    L_punt ("L_independent_memory_ops: sync arcs do not reflect"
		    "the dependence (op %d -> op %d)", op1->id, op2->id);
	  else if (L_debug_sync_arcs)
	    L_warn ("L_independent_memory_ops: "
		    "sync arcs disrespect dep,"
		    "(%d--->%d)", op1->id, op2->id);
	}
      /* Did syncarcs find indep not found in the cases above */
      else if (sync_indep && !indep)
        {
          if ((!stack1 || op1->sync_info) &&
              (!stack2 || op2->sync_info))
            if (L_debug_sync_arcs)
	      L_warn ("L_independent_memory_ops: "
		      "sync arcs found indep, "
		      "(%d-X->%d)", op1->id, op2->id);
        }

      /* for stack operations, there may not be sync arcs
         in the code (e.g. spills).  Thus, we don't use sync
         info in this case. */

      /* actually, sync arcs should be safe for stack operations
         involving local variables.  For spills, and incoming
         and outgoing parameters, the old code will provide 
         accurate results.  So, we will use sync arcs for stack unless
         old code proves definitely dependent */

      if (((op1->sync_info && op2->sync_info) ||
	   (!stack1 || !stack2)) && sync_indep)
	{
	  indep = 1;
	  dep = 0;
	  if (L_debug_memory_disamb)
	    fprintf (stderr, "CASE SYNC-SET\n");
	}
    }

  /* Print summary info for debug
   * ----------------------------------------------------------------------
   */

  if (L_debug_memory_disamb)
    {
      fprintf (stderr, "op %d and op %d are (", op1->id, op2->id);

      if (indep)
        fprintf (stderr, " independent");
      if (dep)
        fprintf (stderr, " dependent");
      if (!dep && partial_dep)
        fprintf (stderr, " partially-dependent");
      if (!dep && !indep)
        fprintf (stderr, " ambiguous");
      if (unsafe)
        fprintf (stderr, " UNSAFE");

      fprintf (stderr, " )\n");
    }

  *r_indep = indep;
  *r_dep = dep;
  return;
}


int
L_depinfo_indep_mem_ops (L_Oper *op1, L_Oper *op2, int dep_flags)
{
  if (L_func_acc_specs)
    return L_mem_indep_acc_specs (op1, op2, dep_flags);
  else if (!L_ignore_sync_arcs_for_opti &&
	   L_use_sync_arcs && L_func_contains_dep_pragmas)
    return L_analyze_syncs_for_independence (op1, op2, dep_flags);
  else
    return 0;
}


/* SER 20041214
 * \brief Returns a set of conflicting memory opers for an L_Oper.  For
 *  stores, does not conflict with stores of the same expression.
 *
 * \param oper
 *  operation to be checked for alias
 *
 * \note This function creates a Set that must be destroyed after use.
 */
Set
L_mem_find_all_conflicting_expression_opers (L_Oper * oper)
{
  int i, store_flag, def_flag;
  Set conflicts = NULL;

  if (L_load_opcode (oper))
    {
      store_flag = 0;
      def_flag = 0;
    }
  else if (L_store_opcode (oper))
    {
      store_flag = 1;
      def_flag = 1;
    }
  else if (L_subroutine_call_opcode (oper))
    {
      store_flag = 0;
      def_flag = 1;
    }
  else
    return NULL;

  for (i = 1; i <= L_fn->n_oper; i++)
    {
      L_Oper * alias_op;

     if (i == oper->id)
	continue; 
     alias_op = L_oper_hash_tbl_find_oper (L_fn->oper_hash_tbl, i);
      if (alias_op == NULL)
	continue;
      if (def_flag)
	{
	  if (store_flag)
	    {
	      if (L_opers_same_expression (oper, alias_op))
		continue;
	    }
	  if (!(L_load_opcode (alias_op) || L_store_opcode (alias_op) ||
		L_subroutine_call_opcode (alias_op)))
	    continue;
	}
      else
	{
	  if (!(L_store_opcode (alias_op) ||
		L_subroutine_call_opcode (alias_op)))
	    continue;
	}
      if (!L_depinfo_indep_mem_ops (oper, alias_op, SET_NONLOOP_CARRIED (0)))
	conflicts = Set_add (conflicts, i);
    }
  return conflicts;
}
