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
 *      File :          l_lcode.c
 *      Description :   General lcode functions 
 *      Creation Date : July, 1990
 *      Author :        Scott Mahlke, Pohua Chang, Wen-mei Hwu
 *
 *==========================================================================*/
/* 09/19/02 REK Updating the copy functions to copy the completers field. */
/* 01/13/03 REK Fixing L_opcode_ctype to return void for normal stored, and
 *              the default int register type for pre and post increment
 *              stores. */
/* 01/14/03 REK Adding L_store_ctype to return the ctype stored by a store
 *              opcode. */

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>

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
 *      L_Operand boolean predicates
 */
/*===========================================================================*/

int
L_type_is_obsolete (L_Operand * operand)
{
  if (!operand)
    return 0;
  return (((operand)->type) & 0X80);
}

int
L_ctype_is_obsolete (L_Operand * operand)
{
  if (!operand)
    return 0;
  return (((operand)->ctype) & 0X80);
}

int
L_operand_is_signed (L_Operand * operand)
{
  if (!operand)
    return 0;
  return (((operand)->ctype) & 0X40);
}

int
L_ctype_is_signed (ITuint8 ctype)
{
  return (ctype & 0X40);
}

ITuint8
L_ctype_unsigned_version (ITuint8 ctype)
{
  switch (ctype)
    {
    case L_CTYPE_CHAR:
    case L_CTYPE_UCHAR:
      return (L_CTYPE_UCHAR);
    case L_CTYPE_SHORT:
    case L_CTYPE_USHORT:
      return (L_CTYPE_USHORT);
    case L_CTYPE_INT:
    case L_CTYPE_UINT:
      return (L_CTYPE_UINT);
    case L_CTYPE_LONG:
    case L_CTYPE_ULONG:
      return (L_CTYPE_ULONG);
    case L_CTYPE_LLONG:
    case L_CTYPE_ULLONG:
      return (L_CTYPE_ULLONG);
    case L_CTYPE_LLLONG:
    case L_CTYPE_ULLLONG:
      return (L_CTYPE_ULLLONG);
    default:
      L_punt ("L_get_unsigned_ctype_version: Illegal ctype %d", ctype);
      return (0);
    }
}


ITuint8
L_ctype_signed_version (ITuint8 ctype)
{
  switch (ctype)
    {
    case L_CTYPE_CHAR:
    case L_CTYPE_UCHAR:
      return (L_CTYPE_CHAR);
    case L_CTYPE_SHORT:
    case L_CTYPE_USHORT:
      return (L_CTYPE_SHORT);
    case L_CTYPE_INT:
    case L_CTYPE_UINT:
      return (L_CTYPE_INT);
    case L_CTYPE_LONG:
    case L_CTYPE_ULONG:
      return (L_CTYPE_LONG);
    case L_CTYPE_LLONG:
    case L_CTYPE_ULLONG:
      return (L_CTYPE_LLONG);
    case L_CTYPE_LLLONG:
    case L_CTYPE_ULLLONG:
      return (L_CTYPE_LLLONG);
    default:
      L_punt ("L_get_signed_ctype_version: Illegal ctype %d", ctype);
      return (0);
    }
}

int
L_is_null (L_Operand * operand)
{
  return ((operand) == NULL);
}

int
L_operand_type_same (L_Operand * op1, L_Operand * op2)
{
  if (!op1 || !op2)
    return 0;
  return (op1->type == op2->type);
}

int
L_operand_ctype_same (L_Operand * op1, L_Operand * op2)
{
  if (!op1 || !op2)
    return 0;
  return (op1->ctype == op2->ctype);
}

int
L_is_ctype_void (L_Operand * operand)
{
  if (!operand)
    return 0;
  return ((operand)->ctype == L_CTYPE_VOID);
}

int
L_is_ctype_void_direct (ITuint8 ctype)
{
  return (ctype == L_CTYPE_VOID);
}

int
L_is_ctype_integer (L_Operand * operand)
{
  if (!operand)
    return 0;
  return ((operand->ctype & 0X30) == 0X00);
}

int
L_is_ctype_llong (L_Operand * operand)
{
  if(!operand)
    return 0;
  return ((operand->ctype == L_CTYPE_LLONG) || (operand->ctype == L_CTYPE_ULLONG));
}

int
L_is_ctype_ullong (L_Operand * operand)
{
  if(!operand)
    return 0;
  return (operand->ctype == L_CTYPE_ULLONG);
}

int
L_is_ctype_int_direct (ITuint8 ctype)
{
  return (!(ctype & 0X30));
}

/* Returns the ctype class (integer, float, double, etc) */
int
L_ctype_numerical_class (L_Operand * oper)
{
  switch (oper->ctype)
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
      return L_CTYPE_INT;

    case L_CTYPE_VOID:
    case L_CTYPE_FLOAT:
    case L_CTYPE_DOUBLE:
    case L_CTYPE_PREDICATE:
    case L_CTYPE_CONTROL:
    case L_CTYPE_BTR:
    case L_CTYPE_LOCAL_ABS:
    case L_CTYPE_LOCAL_GP:
    case L_CTYPE_GLOBAL_ABS:
    case L_CTYPE_GLOBAL_GP:
      return oper->ctype;

    default:
      L_punt ("L_ctype_numerical_class: Illegal type %d", oper->ctype);
      return 0;
    }
}


/* Returns the ctype class (integer, float, double, etc) */
int
L_ctype_numerical_class_direct (ITuint8 ctype)
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
      return L_CTYPE_INT;

    case L_CTYPE_VOID:
    case L_CTYPE_FLOAT:
    case L_CTYPE_DOUBLE:
    case L_CTYPE_PREDICATE:
    case L_CTYPE_CONTROL:
    case L_CTYPE_BTR:
    case L_CTYPE_LOCAL_ABS:
    case L_CTYPE_LOCAL_GP:
    case L_CTYPE_GLOBAL_ABS:
    case L_CTYPE_GLOBAL_GP:
      return ctype;

    default:
      L_punt ("L_ctype_numerical_class_direct: Illegal type %d", ctype);
      return 0;
    }
}

int
L_is_size_char (L_Operand * operand)
{
  if (!operand)
    return 0;
  return (((operand)->ctype & 0x3F) == 0x01);
}

int
L_is_size_char_direct (ITuint8 ctype)
{
  return ((ctype & 0x3F) == 0x01);
}

extern int
L_is_size_short (L_Operand * operand)
{
  if (!operand)
    return 0;
  return (((operand)->ctype & 0x3F) == 0x02);
}

extern int
L_is_size_short_direct (ITuint8 ctype)
{
  return ((ctype & 0x3F) == 0x02);
}

extern int
L_is_size_int (L_Operand * operand)
{
  if (!operand)
    return 0;
  return (((operand)->ctype & 0x3F) == 0x03);
}

extern int
L_is_size_int_direct (ITuint8 ctype)
{
  return ((ctype & 0x3F) == 0x03);
}

extern int
L_is_size_long (L_Operand * operand)
{
  if (!operand)
    return 0;
  return (((operand)->ctype & 0x3F) == 0x04);
}

extern int
L_is_size_long_direct (ITuint8 ctype)
{
  return ((ctype & 0x3F) == 0x04);
}

extern int
L_is_size_llong (L_Operand * operand)
{
  if (!operand)
    return 0;
  return (((operand)->ctype & 0x3F) == 0x05);
}

extern int
L_is_size_llong_direct (ITuint8 ctype)
{
  return ((ctype & 0x3F) == 0x05);
}

extern int
L_is_size_lllong (L_Operand * operand)
{
  if (!operand)
    return 0;
  return (((operand)->ctype & 0x3F) == 0x06);
}

extern int
L_is_size_lllong_direct (ITuint8 ctype)
{
  return ((ctype & 0x3F) == 0x06);
}

extern int
L_is_pointer (L_Operand * operand)
{
  if (!operand)
    return 0;
  return (((operand)->ctype & 0x3F) == 0x07);
}

extern int
L_is_pointer_direct (ITuint8 ctype)
{
  return ((ctype & 0x3F) == 0x07);
}


int
L_is_ctype_flt (L_Operand * operand)
{
  if (!operand)
    return 0;
  return ((operand)->ctype == L_CTYPE_FLOAT);
}

int
L_is_ctype_float (L_Operand * operand)
{
  int cond = (!operand ? 0 : (((operand)->ctype == L_CTYPE_FLOAT) ||
			      ((operand)->ctype == L_CTYPE_DOUBLE)));
  return (cond);

}

int
L_is_ctype_float_direct (ITuint8 ctype)
{
  return (ctype == L_CTYPE_FLOAT);
}

int
L_is_ctype_dbl (L_Operand * operand)
{
  if (!operand)
    return 0;
  return ((operand)->ctype == L_CTYPE_DOUBLE);
}

int
L_is_ctype_double_direct (ITuint8 ctype)
{
  return (ctype == L_CTYPE_DOUBLE);
}

int
L_is_ctype_control (L_Operand * operand)
{
  if (!operand)
    return 0;
  return ((operand)->ctype == L_CTYPE_CONTROL);
}

int
L_is_ctype_control_direct (ITuint8 ctype)
{
  return (ctype == L_CTYPE_CONTROL);
}

int
L_is_ctype_btr (L_Operand * operand)
{
  if (!operand)
    return 0;
  return ((operand)->ctype == L_CTYPE_BTR);
}

int
L_is_ctype_btr_direct (ITuint8 ctype)
{
  return (ctype == L_CTYPE_BTR);
}

int
L_is_ctype_predicate (L_Operand * operand)
{
  if (!operand)
    return 0;
  return ((operand)->ctype == L_CTYPE_PREDICATE);
}

int
L_is_ctype_predicate_direct (ITuint8 ctype)
{
  return (ctype == L_CTYPE_PREDICATE);
}

void
L_assign_type_cb (L_Operand * operand)
{
  if (!operand)
    L_punt ("L_return_old_type: operand is NULL.");
  (operand)->type = L_OPERAND_CB;
  /*temporary assign */
  (operand)->ctype = L_CTYPE_INT;
}

void
L_assign_type_string (L_Operand * operand)
{
  if (!operand)
    L_punt ("L_return_old_type: operand is NULL.");
  (operand)->type = L_OPERAND_STRING;
  /*temporary assign */
  (operand)->ctype = L_CTYPE_LOCAL_ABS;
}

void
L_assign_type_GLOBAL_GP_string (L_Operand * operand)
{
  (operand)->type = L_OPERAND_STRING;
  (operand)->ctype = L_CTYPE_GLOBAL_GP;
}

void
L_assign_type_label (L_Operand * operand)
{
  if (!operand)
    L_punt ("L_return_old_type: operand is NULL.");
  (operand)->type = L_OPERAND_LABEL;
  /*temporary assign */
  (operand)->ctype = L_CTYPE_GLOBAL_ABS;
}

void
L_assign_type_GLOBAL_GP_label (L_Operand * operand)
{
  (operand)->type = L_OPERAND_LABEL;
  (operand)->ctype = L_CTYPE_GLOBAL_GP;
}

void
L_assign_type_LOCAL_GP_label (L_Operand * operand)
{
  (operand)->type = L_OPERAND_LABEL;
  (operand)->ctype = L_CTYPE_LOCAL_GP;
}

int
L_is_reserved (L_Operand * operand)
{
  if (!operand)
    return 0;
  return ((operand)->type == L_OPERAND_RESERVED);
}

int
L_is_register (L_Operand * operand)
{
  int cond;

  if (!operand)
    return 0;
  cond = (((operand)->type == L_OPERAND_REGISTER) ||
          ((operand)->type == L_OPERAND_RREGISTER) ||
          ((operand)->type == L_OPERAND_EVR));
  return (cond);
}

int
L_is_reg (L_Operand * operand)
{
  if (!operand)
    return 0;
  return ((operand)->type == L_OPERAND_REGISTER);
}

int
L_is_reg_direct (ITuint8 type)
{
  return (type == L_OPERAND_REGISTER);
}

int
L_is_rregister (L_Operand * operand)
{
  if (!operand)
    return 0;
  return ((operand)->type == L_OPERAND_RREGISTER);
}

int
L_is_rregister_direct (ITuint8 type)
{
  return (type == L_OPERAND_RREGISTER);
}

int
L_is_evr (L_Operand * operand)
{
  if (!operand)
    return 0;
  return ((operand)->type == L_OPERAND_EVR);
}

int
L_is_evr_direct (ITuint8 type)
{
  return (type == L_OPERAND_EVR);
}

int
L_is_macro (L_Operand * operand)
{
  if (!operand)
    return 0;
  return ((operand)->type == L_OPERAND_MACRO);
}

int
L_is_macro_direct (ITuint8 type)
{
  return (type == L_OPERAND_MACRO);
}

int
L_is_int_constant (L_Operand * operand)
{
  int cond;

  if (!operand)
    return 0;
  cond = (((operand)->type == L_OPERAND_IMMED)
          && (L_is_ctype_integer (operand)));
  return (cond);
}

int
L_is_flt_constant (L_Operand * operand)
{
  int cond;

  if (!operand)
    return 0;
  cond = (((operand)->type == L_OPERAND_IMMED)
          && ((operand)->ctype == L_CTYPE_FLOAT));
  return (cond);
}

int
L_is_dbl_constant (L_Operand * operand)
{
  int cond;

  if (!operand)
    return 0;
  cond = (((operand)->type == L_OPERAND_IMMED)
          && ((operand)->ctype == L_CTYPE_DOUBLE));
  return (cond);
}

int
L_is_string (L_Operand * operand)
{
  if (!operand)
    return 0;
  return ((operand)->type == L_OPERAND_STRING);
}

int
L_is_string_direct (ITuint8 type)
{
  return (type == L_OPERAND_STRING);
}

int
L_is_cb (L_Operand * operand)
{
  if (!operand)
    return 0;
  return ((operand)->type == L_OPERAND_CB);
}

int
L_is_cb_direct (ITuint8 type)
{
  return (type == L_OPERAND_CB);
}

int
L_is_label (L_Operand * operand)
{
  if (!operand)
    return 0;
  return ((operand)->type == L_OPERAND_LABEL);
}

int
L_is_label_direct (ITuint8 type)
{
  return (type == L_OPERAND_LABEL);
}


int
L_is_GLOBAL_GP_string (L_Operand * operand)
{
  int cond =
    ((operand) ==
     NULL ? 0 : (((operand)->type == L_OPERAND_STRING)
                 && ((operand)->ctype == L_CTYPE_GLOBAL_GP)));
  return (cond);
}

int
L_is_GLOBAL_GP_label (L_Operand * operand)
{
  int cond = (!operand ? 0 : (((operand)->type == L_OPERAND_LABEL) &&
			      ((operand)->ctype ==
			       L_CTYPE_GLOBAL_GP)));
  return (cond);
}

int
L_is_LOCAL_GP_label (L_Operand * operand)
{
  int cond = (!operand ? 0 : (((operand)->type == L_OPERAND_LABEL) &&
			      ((operand)->ctype ==
			       L_CTYPE_LOCAL_GP)));
  return (cond);
}


int
L_is_integer_reg (L_Operand * operand)
{
  int cond = (!operand ? 0 : (((operand)->type == L_OPERAND_REGISTER)
			      && (L_is_ctype_integer ((operand)))));
  return (cond);
}


int
L_is_float_reg (L_Operand * operand)
{
  int cond =
    ((operand) ==
     NULL ? 0 : (((operand)->type == L_OPERAND_REGISTER)
                 && (L_is_ctype_float ((operand)))));
  return (cond);
}

int
L_is_branch_reg (L_Operand * operand)
{
  int cond =
    ((operand) ==
     NULL ? 0 : (((operand)->type == L_OPERAND_REGISTER)
                 && (L_is_ctype_btr ((operand)))));
  return (cond);
}

int
L_is_predicate_reg (L_Operand * operand)
{
  int cond =
    ((operand) ==
     NULL ? 0 : (((operand)->type == L_OPERAND_REGISTER)
                 && (L_is_ctype_predicate ((operand)))));
  return (cond);
}

int
L_is_ctype_LOCAL_ABS_direct (ITuint8 ctype)
{
  return (ctype == L_CTYPE_LOCAL_ABS);
}

int
L_is_ctype_LOCAL_GP_direct (ITuint8 ctype)
{
  return (ctype == L_CTYPE_LOCAL_GP);
}

int
L_is_ctype_GLOBAL_ABS_direct (ITuint8 ctype)
{
  return (ctype == L_CTYPE_GLOBAL_ABS);
}

int
L_is_ctype_GLOBAL_GP_direct (ITuint8 ctype)
{
  return (ctype == L_CTYPE_GLOBAL_GP);
}

int
L_is_constant (L_Operand * operand)
{
  int cond;

  if (!operand)
    return 0;
  cond = (((operand)->type == L_OPERAND_IMMED) ||
          ((operand)->type == L_OPERAND_STRING) ||
          ((operand)->type == L_OPERAND_CB) ||
          ((operand)->type == L_OPERAND_LABEL));
  return (cond);
}

int
L_is_numeric_constant (L_Operand * operand)
{
  int cond;

  if (!operand)
    return 0;
  cond = ((operand)->type == L_OPERAND_IMMED);
  return (cond);
}

int
L_is_variable (L_Operand * operand)
{
  int cond;

  if (!operand)
    return 0;
  cond = (((operand)->type == L_OPERAND_REGISTER) ||
          ((operand)->type == L_OPERAND_RREGISTER) ||
          ((operand)->type == L_OPERAND_EVR) ||
          ((operand)->type == L_OPERAND_MACRO));
  return (cond);
}

int
L_is_int_zero (L_Operand * operand)
{
  int cond;

  if (!operand)
    return 0;
  cond = ((operand->type == L_OPERAND_IMMED &&
           ((operand->ctype & 0x30) == 0x00)) && ((operand)->value.i == 0));
  return (cond);
}

int
L_is_int_one (L_Operand * operand)
{
  int cond;

  if (!operand)
    return 0;
  cond = ((operand->type == L_OPERAND_IMMED &&
           ((operand->ctype & 0x30) == 0x00)) && ((operand)->value.i == 1));
  return (cond);
}

int
L_is_int_neg_one (L_Operand * operand)
{
  int cond;

  if (!operand)
    return 0;
  cond = ((operand->type == L_OPERAND_IMMED &&
           ((operand->ctype & 0x30) == 0x00)) && ((operand)->value.i == -1));
  return (cond);
}

int
L_is_zero (L_Operand * operand)
{
  int cond;

  if (!operand)
    return 0;
  cond =
    (((operand->type == L_OPERAND_IMMED && ((operand->ctype & 0x30) == 0x00))
      && ((operand)->value.i == 0)) || ((operand->type == L_OPERAND_IMMED
                                         && operand->ctype == L_CTYPE_FLOAT)
                                        && ((operand)->value.f == 0.0))
     ||
     ((operand->type == L_OPERAND_IMMED && operand->ctype == L_CTYPE_DOUBLE)
      && ((operand)->value.f2 == 0.0)));
  return (cond);
}

int
L_is_one (L_Operand * operand)
{
  int cond;

  if (!operand)
    return 0;
  cond =
    (((operand->type == L_OPERAND_IMMED && ((operand->ctype & 0x30) == 0x00))
      && ((operand)->value.i == 1)) || ((operand->type == L_OPERAND_IMMED
                                         && operand->ctype == L_CTYPE_FLOAT)
                                        && ((operand)->value.f == 1.0))
     ||
     ((operand->type == L_OPERAND_IMMED && operand->ctype == L_CTYPE_DOUBLE)
      && ((operand)->value.f2 == 1.0)));
  return (cond);
}

int
L_is_neg_one (L_Operand * operand)
{
  int cond;

  if (!operand)
    return 0;
  cond =
    (((operand->type == L_OPERAND_IMMED && ((operand->ctype & 0x30) == 0x00))
      && ((operand)->value.i == -1)) || ((operand->type == L_OPERAND_IMMED
                                          && operand->ctype == L_CTYPE_FLOAT)
                                         && ((operand)->value.f == -1.0))
     ||
     ((operand->type == L_OPERAND_IMMED && operand->ctype == L_CTYPE_DOUBLE)
      && ((operand)->value.f2 == -1.0)));
  return (cond);
}

int
L_is_power_of_two (L_Operand * operand)
{
  int cond;

  if (!operand)
    return 0;
  cond =
    ((operand->type == L_OPERAND_IMMED && ((operand->ctype & 0x30) == 0x00))
     && C_is_log2 ((int) (operand)->value.i));
  return (cond);
}

int
L_abs_is_power_of_two (L_Operand * operand)
{
  int cond;

  if (!operand)
    return 0;
  cond =
    ((operand->type == L_OPERAND_IMMED && ((operand->ctype & 0x30) == 0x00))
     && C_is_log2 (abs ((int) (operand)->value.i)));
  return (cond);
}

/* do not use this function in code development */
int
L_return_old_type (L_Operand * oper)
{
  if (oper == NULL)
    L_punt ("L_return_old_type: operand is NULL.");
  switch (oper->type)
    {
    case L_OPERAND_IMMED:
      {
        if (L_is_ctype_integer (oper))
          return L_OPERAND_INT;
        if (L_is_ctype_flt (oper))
          return L_OPERAND_FLOAT;
        if (L_is_ctype_dbl (oper))
          return L_OPERAND_DOUBLE;
        L_warn ("L_return_old_type: Illegal type");
        return 0;
      }
    case L_OPERAND_CB:
    case L_OPERAND_STRING:
    case L_OPERAND_LABEL:
    case L_OPERAND_MACRO:
    case L_OPERAND_REGISTER:
    case L_OPERAND_RREGISTER:
    case L_OPERAND_EVR:
    case L_OPERAND_RESERVED:
    case L_OPERAND_VOID:
      return oper->type;
    default:
      L_punt ("L_return_old_type: Illegal type");
    }
  return L_OPERAND_VOID;
}

/* do not use this function in new code development */
int
L_return_old_ctype (L_Operand * oper)
{
  if (oper == NULL)
    L_punt ("L_return_old_ctype: operand is NULL.");
  switch (oper->ctype)
    {
    case L_CTYPE_POINTER:
      if (M_arch == M_STARCORE)
	return L_CTYPE_POINTER;
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
      if (M_native_int_register_ctype () == L_CTYPE_INT)
        return L_CTYPE_INT;
      else if (M_native_int_register_ctype () == L_CTYPE_LLONG)
        return L_CTYPE_LLONG;
      else
	L_punt ("L_return_old_ctype: Unsupported machine reg size\n");
    case L_CTYPE_VOID:
    case L_CTYPE_FLOAT:
    case L_CTYPE_DOUBLE:
    case L_CTYPE_PREDICATE:
    case L_CTYPE_CONTROL:
    case L_CTYPE_BTR:
      return oper->ctype;
    case L_CTYPE_LOCAL_ABS:
    case L_CTYPE_LOCAL_GP:
    case L_CTYPE_GLOBAL_ABS:
    case L_CTYPE_GLOBAL_GP:
      return L_CTYPE_INT;
    default:
      L_punt ("L_return_old_ctype: Illegal type");
    }
  return FALSE;
}

/* do not use this function in new code development */
int
L_operand_case_type (L_Operand * oper)
{
  if (!oper)
    L_punt ("L_operand_case_type: operand is NULL.");
  return (L_return_old_type (oper));
}

/* do not use this function in new code development */
int
L_operand_case_ctype (L_Operand * oper)
{
  if (!oper)
    L_punt ("L_operand_case_ctype: operand is NULL.");
  return (L_return_old_ctype (oper));
}

void
L_assign_same_type (L_Operand * dest, L_Operand * src)
{
  dest->type = src->type;
  dest->ctype = src->ctype;
}

void
L_assign_type (L_Operand * oper, int old_type)
{
  oper->type = old_type;
}

void
L_assign_same_ctype (L_Operand * dest, L_Operand * src)
{
  dest->ctype = src->ctype;
}

void
L_assign_ctype (L_Operand * oper, ITuint8 old_ctype)
{
  if (!oper)
    L_punt ("L_assign_ctype: operand is NULL.");
  oper->ctype = old_ctype;
}

void
L_assign_type_int (L_Operand * oper, ITuint8 ctype)
{
  if (!oper)
    L_punt ("L_assign_type_int: operand is NULL.");
  oper->type = L_OPERAND_IMMED;
  oper->ctype = ctype;
}

void
L_assign_type_float (L_Operand * oper)
{
  if (!oper)
    L_punt ("L_assign_type_float: operand is NULL.");
  oper->type = L_OPERAND_IMMED;
  oper->ctype = L_CTYPE_FLOAT;
}

void
L_assign_type_double (L_Operand * oper)
{
  if (!oper)
    L_punt ("L_assign_type_double: operand is NULL.");
  oper->type = L_OPERAND_IMMED;
  oper->ctype = L_CTYPE_DOUBLE;
}

void
L_assign_type_general_macro (L_Operand * oper)
{
  if (!oper)
    L_punt ("L_assign_type_macro: operand is NULL.");
  oper->type = L_OPERAND_MACRO;
}

void
L_assign_type_void_macro (L_Operand * oper)
{
  if (!oper)
    L_punt ("L_assign_type_void_macro: operand is NULL.");
  oper->type = L_OPERAND_MACRO;
  oper->ctype = L_CTYPE_VOID;
}

void
L_assign_type_int_macro (L_Operand * oper)
{
  if (!oper)
    L_punt ("L_assign_type_int_macro: operand is NULL.");
  oper->type = L_OPERAND_MACRO;
  oper->ctype = M_native_int_register_ctype ();
}

void
L_assign_type_float_macro (L_Operand * oper)
{
  if (!oper)
    L_punt ("L_assign_type_float_macro: operand is NULL.");
  oper->type = L_OPERAND_MACRO;
  oper->ctype = L_CTYPE_FLOAT;
}

void
L_assign_type_double_macro (L_Operand * oper)
{
  if (!oper)
    L_punt ("L_assign_type_double_macro: operand is NULL.");
  oper->type = L_OPERAND_MACRO;
  oper->ctype = L_CTYPE_DOUBLE;
}

void
L_assign_type_control_macro (L_Operand * oper)
{
  if (!oper)
    L_punt ("L_assign_type_control_macro: operand is NULL.");
  oper->type = L_OPERAND_MACRO;
  oper->ctype = L_CTYPE_CONTROL;
}

void
L_assign_type_btr_macro (L_Operand * oper)
{
  if (!oper)
    L_punt ("L_assign_type_btr_macro: operand is NULL.");
  oper->type = L_OPERAND_MACRO;
  oper->ctype = L_CTYPE_BTR;
}

void
L_assign_type_predicate_macro (L_Operand * oper)
{
  if (!oper)
    L_punt ("L_assign_type_predicate_macro: operand is NULL.");
  oper->type = L_OPERAND_MACRO;
  oper->ctype = L_CTYPE_PREDICATE;
}

void
L_assign_type_general_register (L_Operand * oper)
{
  if (!oper)
    L_punt ("L_assign_type_general_register: operand is NULL.");
  oper->type = L_OPERAND_REGISTER;
}

void
L_assign_type_void_register (L_Operand * oper)
{
  if (!oper)
    L_punt ("L_assign_type_void_register: operand is NULL.");
  oper->type = L_OPERAND_REGISTER;
  oper->ctype = L_CTYPE_VOID;
}

void
L_assign_type_int_register (L_Operand * oper)
{
  if (!oper)
    L_punt ("L_assign_type_int_register: operand is NULL.");
  oper->type = L_OPERAND_REGISTER;
  oper->ctype = M_native_int_register_ctype ();
}

void
L_assign_type_float_register (L_Operand * oper)
{
  if (!oper)
    L_punt ("L_assign_type_float_register: operand is NULL.");
  oper->type = L_OPERAND_REGISTER;
  oper->ctype = L_CTYPE_FLOAT;
}

void
L_assign_type_double_register (L_Operand * oper)
{
  if (!oper)
    L_punt ("L_assign_type_double_register: operand is NULL.");
  oper->type = L_OPERAND_REGISTER;
  oper->ctype = L_CTYPE_DOUBLE;
}

void
L_assign_type_btr_register (L_Operand * oper)
{
  if (!oper)
    L_punt ("L_assign_type_btr_register: operand is NULL.");
  oper->type = L_OPERAND_REGISTER;
  oper->ctype = L_CTYPE_BTR;
}

void
L_assign_type_predicate_register (L_Operand * oper)
{
  if (!oper)
    L_punt ("L_assign_type_predicate_register: operand is NULL.");
  oper->type = L_OPERAND_REGISTER;
  oper->ctype = L_CTYPE_PREDICATE;
}

void
L_assign_type_general_rregister (L_Operand * oper)
{
  if (!oper)
    L_punt ("L_assign_type_general_rregister: operand is NULL.");
  oper->type = L_OPERAND_RREGISTER;
}

void
L_assign_type_void_rregister (L_Operand * oper)
{
  if (!oper)
    L_punt ("L_assign_type_void_rregister: operand is NULL.");
  oper->type = L_OPERAND_RREGISTER;
  oper->ctype = L_CTYPE_VOID;
}

void
L_assign_type_int_rregister (L_Operand * oper)
{
  if (!oper)
    L_punt ("L_assign_type_int_rregister: operand is NULL.");
  oper->type = L_OPERAND_RREGISTER;
  oper->ctype = L_CTYPE_INT;
}

void
L_assign_type_float_rregister (L_Operand * oper)
{
  if (!oper)
    L_punt ("L_assign_type_float_rregister: operand is NULL.");
  oper->type = L_OPERAND_RREGISTER;
  oper->ctype = L_CTYPE_FLOAT;
}

void
L_assign_type_double_rregister (L_Operand * oper)
{
  if (!oper)
    L_punt ("L_assign_type_double_rregister: operand is NULL.");
  oper->type = L_OPERAND_RREGISTER;
  oper->ctype = L_CTYPE_DOUBLE;
}

void
L_assign_type_btr_rregister (L_Operand * oper)
{
  if (!oper)
    L_punt ("L_assign_type_btr_rregister: operand is NULL.");
  oper->type = L_OPERAND_RREGISTER;
  oper->ctype = L_CTYPE_BTR;
}

void
L_assign_type_predicate_rregister (L_Operand * oper)
{
  if (!oper)
    L_punt ("L_assign_type_predicate_rregister: operand is NULL.");
  oper->type = L_OPERAND_RREGISTER;
  oper->ctype = L_CTYPE_PREDICATE;
}

void
L_assign_type_general_evr (L_Operand * oper)
{
  if (!oper)
    L_punt ("L_assign_type_general_evr: operand is NULL.");
  oper->type = L_OPERAND_EVR;
}

void
L_assign_type_void_evr (L_Operand * oper)
{
  if (!oper)
    L_punt ("L_assign_type_void_evr: operand is NULL.");
  oper->type = L_OPERAND_EVR;
  oper->ctype = L_CTYPE_VOID;
}

void
L_assign_type_int_evr (L_Operand * oper)
{
  if (!oper)
    L_punt ("L_assign_type_int_evr: operand is NULL.");
  oper->type = L_OPERAND_EVR;
  oper->ctype = L_CTYPE_INT;
}

void
L_assign_type_float_evr (L_Operand * oper)
{
  if (!oper)
    L_punt ("L_assign_type_float_evr: operand is NULL.");
  oper->type = L_OPERAND_EVR;
  oper->ctype = L_CTYPE_FLOAT;
}

void
L_assign_type_double_evr (L_Operand * oper)
{
  if (!oper)
    L_punt ("L_assign_type_double_evr: operand is NULL.");
  oper->type = L_OPERAND_EVR;
  oper->ctype = L_CTYPE_DOUBLE;
}

void
L_assign_type_btr_evr (L_Operand * oper)
{
  if (!oper)
    L_punt ("L_assign_type_btr_evr: operand is NULL.");
  oper->type = L_OPERAND_EVR;
  oper->ctype = L_CTYPE_BTR;
}

void
L_assign_type_predicate_evr (L_Operand * oper)
{
  if (!oper)
    L_punt ("L_assign_type_predicate_evr: operand is NULL.");
  oper->type = L_OPERAND_EVR;
  oper->ctype = L_CTYPE_PREDICATE;
}

int
L_all_src_is_numeric_constant (L_Oper * oper)
{
  int i;

  if (!oper)
    return (0);

  for (i = 0; i < L_max_src_operand; i++)
    {
      if (oper->src[i] == NULL)
        continue;
      if (!L_is_numeric_constant (oper->src[i]))
        return (0);
    }

  return (1);
}

/*
 *      Fragile means its value is updated across subroutine calls
 */
int
L_is_fragile_macro (L_Operand * operand)
{
  if (!L_is_macro (operand))
    return 0;

  return (M_fragile_macro (operand->value.mac));
}

int
L_has_fragile_macro_dest_operand (L_Oper * oper)
{
  int i;

  if (!oper)
    return (0);

  for (i = 0; i < L_max_dest_operand; i++)
    {
      if (L_is_fragile_macro (oper->dest[i]))
        return (1);
    }

  return (0);
}

int
L_has_fragile_macro_src_operand (L_Oper * oper)
{
  int i;

  if (!oper)
    return (0);

  for (i = 0; i < L_max_src_operand; i++)
    {
      if (L_is_fragile_macro (oper->src[i]))
        return (1);
    }

  return (0);
}

int
L_has_fragile_macro_operand (L_Oper * oper)
{
  return (L_has_fragile_macro_dest_operand (oper) ||
          L_has_fragile_macro_src_operand (oper));
}

/*
 *      Unsafe means the optimizer shouldn't touch it, the code generator
 *      expects it in certain places.
 */
int
L_is_unsafe_macro (L_Operand * operand)
{
  if (!L_is_macro (operand))
    return (0);
  return (M_is_unsafe_macro (operand));
}

int
L_has_unsafe_macro_dest_operand (L_Oper * oper)
{
  int i;

  if (!oper)
    return (0);

  for (i = 0; i < L_max_dest_operand; i++)
    {
      if (L_is_unsafe_macro (oper->dest[i]))
        return (1);
    }

  return (0);
}

int
L_has_unsafe_macro_src_operand (L_Oper * oper)
{
  int i;

  if (!oper)
    return (0);

  for (i = 0; i < L_max_src_operand; i++)
    {
      if (L_is_unsafe_macro (oper->src[i]))
        return (1);
    }

  return (0);
}

int
L_has_unsafe_macro_operand (L_Oper * oper)
{
  return (L_has_unsafe_macro_dest_operand (oper) ||
          L_has_unsafe_macro_src_operand (oper));
}

int
L_number_of_dest_operands (L_Oper * oper)
{
  int i, count;

  if (!oper)
    L_punt ("L_number_of_dest_operands: oper is NULL");

  count = 0;
  for (i = 0; i < L_max_dest_operand; i++)
    {
      if (oper->dest[i] != NULL)
        count++;
    }
  return (count);
}

int
L_same_operand (L_Operand * operand1, L_Operand * operand2)
{
  if ((operand1 == NULL) && (operand2 == NULL))
    return 1;

  if ((operand1 == NULL) || (operand2 == NULL))
    return 0;

  if (!L_operand_type_same (operand1, operand2))
    return 0;

  if (!L_operand_ctype_same (operand1, operand2))
    return 0;

  switch (L_operand_case_type (operand1))
    {
    case L_OPERAND_STRING:
      return (!strcmp (operand1->value.s, operand2->value.s));
    case L_OPERAND_LABEL:
      return (!strcmp (operand1->value.l, operand2->value.l));
    case L_OPERAND_REGISTER:
    case L_OPERAND_MACRO:
      {
        /* SSA can be different, but still be considered the same register */
        if (L_is_ctype_predicate (operand1))
          {
            return (operand1->value.pred.reg == operand2->value.pred.reg);
          }
        else
          {
            return ((operand1->value.init.u == operand2->value.init.u) &&
                    (operand1->value.init.l == operand2->value.init.l));
          }
      }
    default:
      return ((operand1->value.init.u == operand2->value.init.u) &&
              (operand1->value.init.l == operand2->value.init.l));
    }
}

int
L_same_src_operands (L_Oper * oper1, L_Oper * oper2)
{
  int i;
  for (i = 0; i < L_max_src_operand; i++)
    {
      if (!L_same_operand (oper1->src[i], oper2->src[i]))
        return 0;
    }
  return 1;
}

int
L_same_dest_operands (L_Oper * oper1, L_Oper * oper2)
{
  int i;
  for (i = 0; i < L_max_dest_operand; i++)
    {
      if (!L_same_operand (oper1->dest[i], oper2->dest[i]))
        return 0;
    }
  return 1;
}

int
L_different_operand (L_Operand * operand1, L_Operand * operand2)
{
  return (!L_same_operand (operand1, operand2));
}

int
L_different_src_and_dest_operands (L_Oper * oper)
{
  int i, j;
  for (i = 0; i < L_max_dest_operand; i++)
    {
      if (oper->dest[i] == NULL)
        continue;
      for (j = 0; j < L_max_src_operand; j++)
        {
          if (L_same_operand (oper->dest[i], oper->src[j]))
            return 0;
        }
    }
  return 1;
}

int
L_is_dest_operand (L_Operand * operand, L_Oper * oper)
{
  int i;

  if (!operand)
    L_punt ("L_is_dest_operand: NULL operand");

  for (i = 0; i < L_max_dest_operand; i++)
    {
      if (oper->dest[i] && L_same_operand (operand, oper->dest[i]))
        return 1;
    }
  return 0;
}

int
L_is_src_operand (L_Operand * operand, L_Oper * oper)
{
  int i;

  if (!operand)
    L_punt ("L_is_src_operand: NULL operand");

  for (i = 0; i < L_max_src_operand; i++)
    {
      if (oper->src[i] && L_same_operand (operand, oper->src[i]))
        return 1;
    }
  return 0;
}

int
L_is_implied_ret_destination_operand (L_Oper * op, L_Operand * operand)
{
  L_Attr *attr;

  if ((attr = L_find_attr (op->attr, "ret")))
    {
      int indx;

      for (indx = 0; indx < attr->max_field; indx++)
	if (L_same_operand (operand, attr->field[indx]))
	  return (1);
    }

  return (0);
}

/*
 *     Not sure this completely right, but for opti seems to work right
 *     Actually, it didn't work, had to add case of "ret" for jsrs -DAC 5-26-00
 */
int
L_same_def_reachs (L_Operand * operand, L_Oper * opA, L_Oper * opB)
{
  int found;
  L_Oper *pA, *pB;

  if (!opA || !opB)
    L_punt ("L_same_def_reachs: opA and opB cannot be NULL");

  if (!operand)
    return 1;

  found = 0;
  for (pA = opA->prev_op; pA; pA = pA->prev_op)
    {
      if (!PG_intersecting_predicates_ops (pA, opA))
	continue;

      if (L_is_dest_operand (operand, pA) ||
	  L_is_implied_ret_destination_operand (pA, operand))
	break;
    }

  found = 0;
  for (pB = opB->prev_op; pB; pB = pB->prev_op)
    {
      if (!PG_intersecting_predicates_ops (pB, opB))
	continue;
      
      if (L_is_dest_operand (operand, pB) ||
	  L_is_implied_ret_destination_operand (pB, operand))
	break;
    }

  return (pA == pB);
}

/*
 *      return 1 if same def of all src operands of oper reach opA and opB
 */
int
L_all_src_operand_same_def_reachs (L_Oper * oper, L_Oper * opA, L_Oper * opB)
{
  int i;
  for (i = 0; i < L_max_src_operand; i++)
    {
      if (oper->src[i] && !L_same_def_reachs (oper->src[i], opA, opB))
        return 0;
    }
  return 1;
}

int
L_all_dest_operand_same_def_reachs (L_Oper * oper, L_Oper * opA, L_Oper * opB)
{
  int i;
  for (i = 0; i < L_max_dest_operand; i++)
    {
      if (oper->dest[i] && !L_same_def_reachs (oper->dest[i], opA, opB))
        return 0;
    }
  return 1;
}

int
L_no_defs_between (L_Operand * operand, L_Oper * opA, L_Oper * opB)
{
  int i;
  L_Oper *pA;

  if (!opA || !opB)
    L_punt ("L_no_defs_between: opA and opB cannot be NULL");
  if (!operand)
    return 1;

  if (L_is_predicated (opB))
    {
      for (pA = opA->next_op; pA != NULL; pA = pA->next_op)
        {
          if (pA == opB)
            return 1;
          if (!PG_intersecting_predicates_ops (pA, opB))
            continue;
          for (i = 0; i < L_max_dest_operand; i++)
            {
              if (pA->dest[i] && L_same_operand (pA->dest[i], operand))
                return 0;
            }
        }
    }
  else
    {
      for (pA = opA->next_op; pA != NULL; pA = pA->next_op)
        {
          if (pA == opB)
            return 1;
          for (i = 0; i < L_max_dest_operand; i++)
            {
              if (pA->dest[i] && L_same_operand (pA->dest[i], operand))
                return 0;
            }
        }
    }

  L_punt ("L_no_defs_between: opB not found");
  return (0);
}

/* This function is the same as L_no_defs_between except that it checks if the
   defining instruction is executed under a predicate that intersects with the
   predicate on opA instead of on opB.  That is, it searches for a definition
   that could clobber the operand of opA rather than a definition that is
   clobbered or used by op B. */
int
L_no_defs_between_wrt_opA (L_Operand * operand, L_Oper * opA, L_Oper * opB)
{
  int i;
  L_Oper *pA;

  if ((opA == NULL) || (opB == NULL))
    L_punt ("L_no_defs_between_wrt_opA: opA and opB cannot be NULL");
  if (!operand)
    return 1;

  if (L_is_predicated (opB))
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

  else
    {
      for (pA = opA->next_op; pA != NULL; pA = pA->next_op)
        {
          if (pA == opB)
            return 1;
          for (i = 0; i < L_max_dest_operand; i++)
            {
              if (L_same_operand (pA->dest[i], operand))
                return 0;
            }
        }
    }

  L_punt ("L_no_defs_between_wrt_opA: opB not found");
  return (0);
}

int
L_no_defs_in_range (L_Operand * operand, L_Oper * opA, L_Oper * opB)
{
  int i;
  L_Oper *pA;

  if ((opA == NULL) || (opB == NULL))
    L_punt ("L_no_defs_in_range: opA and opB cannot be NULL");
  if (!operand)
    return 1;

  if (L_is_predicated (opB))
    {
      for (pA = opA; pA != NULL; pA = pA->next_op)
        {
          if (PG_intersecting_predicates_ops (pA, opB))
            {
              for (i = 0; i < L_max_dest_operand; i++)
                {
                  if (L_same_operand (pA->dest[i], operand))
                    return 0;
                }
            }
          if (pA == opB)
            {
              return 1;
            }
        }
    }
  else
    {
      for (pA = opA; pA != NULL; pA = pA->next_op)
        {
          for (i = 0; i < L_max_dest_operand; i++)
            {
              if (L_same_operand (pA->dest[i], operand))
                return 0;
            }
          if (pA == opB)
            {
              return 1;
            }
        }
    }

  L_punt ("L_no_defs_in_range: opB not found");
  return (0);
}

int
L_all_src_operand_no_defs_between (L_Oper * oper, L_Oper * opA, L_Oper * opB)
{
  int i;
  for (i = 0; i < L_max_src_operand; i++)
    {
      if (!L_no_defs_between (oper->src[i], opA, opB))
        return 0;
    }
  return 1;
}

int
L_all_src_operand_no_defs_between_wrt_opA (L_Oper * oper, L_Oper * opA, 
					   L_Oper * opB)
{
  int i;
  for (i = 0; i < L_max_src_operand; i++)
    {
      if (!L_no_defs_between_wrt_opA (oper->src[i], opA, opB))
        return 0;
    }
  return 1;
}

int
L_all_dest_operand_no_defs_between (L_Oper * oper, L_Oper * opA, L_Oper * opB)
{
  int i;
  for (i = 0; i < L_max_dest_operand; i++)
    {
      if (!L_no_defs_between (oper->dest[i], opA, opB))
        return 0;
    }
  return 1;
}

/* Same as L_all_dest_operand_no_defs_between except that it calls
   L_no_defs_between_wrt_opA. See L_no_defs_between_wrt_opA  for explanation */
int
L_all_dest_operand_no_defs_between_wrt_opA (L_Oper * oper, L_Oper * opA,
                                            L_Oper * opB)
{
  int i;
  for (i = 0; i < L_max_dest_operand; i++)
    {
      if (!L_no_defs_between_wrt_opA (oper->dest[i], opA, opB))
        return 0;
    }
  return 1;
}

/*
 *      This excludes the endpoints
 */
int
L_no_uses_between (L_Operand * operand, L_Oper * opA, L_Oper * opB)
{
  int i;
  L_Oper *pA;

  if ((opA == NULL) || (opB == NULL))
    L_punt ("L_no_uses_between: opA and opB cannot be NULL");
  if (!operand)
    return 1;

  if (L_is_predicated (opA))
    {
      for (pA = opA->next_op; pA != NULL; pA = pA->next_op)
        {
          if (pA == opB)
            return 1;
          if (!PG_intersecting_predicates_ops (opA, pA))
            continue;
          for (i = 0; i < L_max_src_operand; i++)
            {
              if (L_same_operand (pA->src[i], operand))
                return 0;
            }
        }
    }
  else
    {
      for (pA = opA->next_op; pA != NULL; pA = pA->next_op)
        {
          if (pA == opB)
            return 1;
          for (i = 0; i < L_max_src_operand; i++)
            {
              if (L_same_operand (pA->src[i], operand))
                return 0;
            }
        }
    }

  L_punt ("L_no_uses_between: opB not found");
  return (0);
}


/*
 *      This excludes the endpoints
 */
int
L_no_pred_uses_between (L_Operand * operand, L_Oper * opA, L_Oper * opB)
{
  int i;
  L_Oper *pA;

  if ((opA == NULL) || (opB == NULL))
    L_punt ("L_no_uses_between: opA and opB cannot be NULL");
  if (!operand)
    return 1;

  /* Unlike L_no_uses_between, we must check all ops for pred usage. Also,
     the types of the preds will be different, so need to check for index. */
  for (pA = opA->next_op; pA != NULL; pA = pA->next_op)
    {
      if (pA == opB)
	return 1;
      for (i = 0; i < L_max_pred_operand; i++)
	{
	  if (L_same_operand (pA->pred[i], operand))
	    return 0;
	}
    }

  L_punt ("L_no_uses_between: opB not found");
  return (0);
}


/*
 *      This includes the endpoints
 */
int
L_no_uses_in_range (L_Operand * operand, L_Oper * opA, L_Oper * opB)
{
  int i;
  L_Oper *pA;

  if ((opA == NULL) || (opB == NULL))
    L_punt ("L_no_in_range: opA and opB cannot be NULL");
  if (!operand)
    return 1;

  if (L_is_predicated (opA))
    {
      for (pA = opA; pA != NULL; pA = pA->next_op)
        {
          if (PG_intersecting_predicates_ops (opA, pA))
            {
              for (i = 0; i < L_max_src_operand; i++)
                {
                  if (L_same_operand (pA->src[i], operand))
                    return 0;
                }
            }
          if (pA == opB)
            {
              return 1;
            }
        }
    }
  else
    {
      for (pA = opA; pA != NULL; pA = pA->next_op)
        {
          for (i = 0; i < L_max_src_operand; i++)
            {
              if (L_same_operand (pA->src[i], operand))
                return 0;
            }
          if (pA == opB)
            {
              return 1;
            }
        }
    }

  L_punt ("L_no_uses_in_range: opB not found");
  return (0);
}

int
L_all_src_operand_no_uses_between (L_Oper * oper, L_Oper * opA, L_Oper * opB)
{
  int i;
  for (i = 0; i < L_max_src_operand; i++)
    {
      if (!L_no_uses_between (oper->src[i], opA, opB))
        return 0;
    }
  return 1;
}

int
L_all_dest_operand_no_uses_between (L_Oper * oper, L_Oper * opA, L_Oper * opB)
{
  int i;
  for (i = 0; i < L_max_dest_operand; i++)
    {
      if (!L_no_uses_between (oper->dest[i], opA, opB))
        return 0;
    }
  return 1;
}

/****************************************************************************
  routine: L_no_other_def_use_in_cb()
  purpose: Return 1 if the given operand is not used or defined
           anywhere in the given cb, with some exceptions.  The operand may be 
           defined and/or used by any of up to 3 opers passed. 
  
  input: cb - block to process.
         operand - operand being checked
         except1,2,3 - these opers are allowed to define and/or use the
                       operand.
  output:
  returns: 1 if no other defs or uses.  0 otherwise.
  modified: 9/6/97 - Dan Lavery - created
  note: 
 ***************************************************************************/

int
L_no_other_def_use_in_cb (L_Cb * cb, L_Operand * operand, L_Oper * except1,
                          L_Oper * except2, L_Oper * except3)
{
  L_Oper *oper;
  int i;

  for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
    {
      if (oper != except1 && oper != except2 && oper != except3)
        {
          for (i = 0; i < L_max_src_operand; i++)
            {
              if (L_same_operand (oper->src[i], operand))
                return 0;
            }
          for (i = 0; i < L_max_dest_operand; i++)
            {
              if (L_same_operand (oper->dest[i], operand))
                return 0;
            }
        }
    }

  return 1;
}

/****************************************************************************
  routine: L_no_other_def_in_cb()
  purpose: Return 1 if the given operand is not defined
           anywhere in the given cb, with some exceptions.  The operand may be 
           defined any of up to 3 opers passed. 
  
  input: cb - block to process.
         operand - operand being checked
         except1,2,3 - these opers are allowed to define the operand.
  output:
  returns: 1 if no other defs.  0 otherwise.
  modified: 9/6/97 - Dan Lavery - created
  note: 
 ***************************************************************************/

int
L_no_other_def_in_cb (L_Cb * cb, L_Operand * operand, L_Oper * except1,
                      L_Oper * except2, L_Oper * except3)
{
  L_Oper *oper;
  int i;

  for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
    {
      if (oper != except1 && oper != except2 && oper != except3)
        {
          for (i = 0; i < L_max_dest_operand; i++)
            {
              if (L_same_operand (oper->dest[i], operand))
                return 0;
            }
        }
    }

  return 1;
}

/****************************************************************************
  routine: L_no_other_use_in_cb_after()
  purpose: Return 1 if the given operand is not used
           anywhere in the given cb after after_oper except in except_oper.  
  
  input: cb - block to process.
         operand - operand being checked
         after_oper - search starts after this oper
         except_oper - the oper allowed to use the operand.
  output:
  returns: 1 if no other uses.  0 otherwise.
  modified: 11/21/97 - Jim Pierce - created
  note: 
 ***************************************************************************/

int
L_no_other_use_in_cb_after (L_Cb * cb, L_Operand * operand,
                            L_Oper * after_oper, L_Oper * except_oper)
{
  L_Oper *oper;
  int i;

  if (!after_oper)
    return 1;

  for (oper = after_oper->next_op; oper != NULL; oper = oper->next_op)
    if (oper != except_oper)
      for (i = 0; i < L_max_src_operand; i++)
	if (L_same_operand (oper->src[i], operand))
	  return 0;

  return 1;
}


/*===========================================================================*/
/*
 *      L_Oper boolean predicates
 */
/*===========================================================================*/

int
L_sync_opcode (L_Oper * oper)
{
  int opc;
  if (!oper)
    return 0;
  /* SAM 9-95, sync is either specified by opcode or by this new flag!! */
  if (L_EXTRACT_BIT_VAL (oper->flags, L_OPER_SYNC))
    return 1;

  opc = oper->opc;
  switch (opc)
    {
    case Lop_PROLOGUE:
    case Lop_EPILOGUE:
    case Lop_FETCH_AND_ADD:
    case Lop_FETCH_AND_OR:
    case Lop_FETCH_AND_AND:
    case Lop_FETCH_AND_ST:
    case Lop_FETCH_AND_COND_ST:
    case Lop_ADVANCE:
    case Lop_AWAIT:
    case Lop_MUTEX_B:
    case Lop_MUTEX_E:
    case Lop_CO_PROC:
      return 1;
    default:
      return 0;
    }
}

static int
L_abs_opc (int opc)
{
  switch (opc)
    {
    case Lop_JSR:
    case Lop_JSR_FS:
      return Lop_JSR;
    case Lop_RTS:
    case Lop_RTS_FS:
      return Lop_RTS;
    case Lop_JUMP:
    case Lop_JUMP_FS:
      return Lop_JUMP;
    case Lop_JUMP_RG:
    case Lop_JUMP_RG_FS:
      return Lop_JUMP_RG;
    default:
      return opc;
    }
}

int
L_equivalent_opcode (L_Oper * opA, L_Oper * opB)
{
  if (!opA && !opB)
    return 1;
  if (!opA || !opB)
    return 0;
  return (L_abs_opc (opA->opc) == L_abs_opc (opB->opc));
}

int
L_same_opcode (L_Oper * opA, L_Oper * opB)
{
  if (!opA && !opB)
    return 1;
  if (!opA || !opB)
    return 0;
  return (opA->opc == opB->opc);
}

/*
 *      same if same opcode, same dest operands, same src operands
 */
int
L_same_operation (L_Oper * oper1, L_Oper * oper2, int ignore_fs)
{
  int i;

  if (!oper1 && !oper2)
    return 1;
  if (!oper1 || !oper2)
    return 0;

  if (ignore_fs)
    {
      if (!L_equivalent_opcode (oper1, oper2))
        return 0;
    }
  else
    {
      if (!L_same_opcode (oper1, oper2))
        return 0;
    }

  for (i = 0; i < L_max_dest_operand; i++)
    {
      if (!L_same_operand (oper1->dest[i], oper2->dest[i]))
        return 0;
    }

  for (i = 0; i < L_MAX_CMPLTR; i++)
    {
      if (oper1->com[i] != oper2->com[i])
	return 0;
    }

  for (i = 0; i < L_max_src_operand; i++)
    {
      if (!L_same_operand (oper1->src[i], oper2->src[i]))
        return 0;
    }

  for (i = 0; i < L_max_pred_operand; i++)
    {
      if (!L_same_operand (oper1->pred[i], oper2->pred[i]))
        return 0;
    }

  return 1;
}

/*
 *      same as L_same_operation, but remove dest operand requirement
 */
int
L_same_computation (L_Oper * oper1, L_Oper * oper2)
{
  int i;

  if ((oper1 == NULL) && (oper2 == NULL))
    return 1;
  if ((oper1 == NULL) || (oper2 == NULL))
    return 0;
  if (!L_same_opcode (oper1, oper2))
    return 0;

  for (i = 0; i < L_MAX_CMPLTR; i++)
    {
      if (oper1->com[i] != oper2->com[i])
	return 0;
    }

  for (i = 0; i < L_max_src_operand; i++)
    {
      if (!L_same_operand (oper1->src[i], oper2->src[i]))
        return 0;
    }

  for (i = 0; i < L_max_pred_operand; i++)
    {
      if (!L_same_operand (oper1->pred[i], oper2->pred[i]))
        return 0;
    }

  return 1;
}

int
L_safe_to_delete_opcode (L_Oper * oper)
{
  int i, safe, has_dest;

  /* Added L_intrinsic_opcode. -ITI/JWJ 8.2.1999 */
  safe = (L_general_arithmetic_opcode (oper) ||
          L_general_move_opcode (oper) ||
          L_general_load_opcode (oper) ||
          L_logic_opcode (oper) ||
          L_shift_opcode (oper) ||
          L_intrinsic_opcode (oper) ||
          L_sign_or_zero_extend_opcode (oper) ||
          (oper->opc == Lop_BIT_EXTRACT) || (oper->opc == Lop_BIT_DEPOSIT) ||
	  (oper->opc == Lop_ALLOC) ||
          L_pbr_opcode (oper) ||
          L_general_comparison_opcode (oper) || L_pred_define_opcode (oper));

  /* if the oper has no destinations, not safe to delete */
  if (safe)
    {
      has_dest = 0;
      for (i = 0; i < L_max_dest_operand; i++)
        {
          if (oper->dest[i] != NULL)
            {
              has_dest = 1;
              break;
            }
        }
      if (!has_dest)
        safe = 0;
    }

  return (safe);
}

int
L_can_move_opcode (L_Oper * oper)
{
  /* Added L_intrinsic_opcode. -ITI/JWJ 8.2.1999 */
  return (L_general_arithmetic_opcode (oper) ||
          L_general_move_opcode (oper) ||
          L_general_load_opcode (oper) ||
          L_general_store_opcode (oper) ||
          L_sign_or_zero_extend_opcode (oper) || 
	  L_intrinsic_opcode (oper));
}

int
L_unsigned_int_opcode (L_Oper * oper)
{
  return (L_is_opcode (Lop_ADD_U, oper) ||
          L_is_opcode (Lop_SUB_U, oper) ||
          L_is_opcode (Lop_MUL_U, oper) ||
          L_is_opcode (Lop_DIV_U, oper) ||
          L_is_opcode (Lop_REM_U, oper) ||
	  L_is_opcode (Lop_EXTRACT_U, oper) ||
          L_is_opcode (Lop_MUL_ADD_U, oper) ||
          L_is_opcode (Lop_MUL_SUB_U, oper) ||
          L_is_opcode (Lop_MUL_SUB_REV_U, oper) ||
          L_unsigned_int_comparison_opcode (oper) ||
          L_unsigned_int_pred_comparison_opcode (oper) ||
          L_unsigned_int_cond_branch_opcode (oper) ||
          L_is_opcode (Lop_ADD_CARRY_U, oper) ||
          L_is_opcode (Lop_SUB_CARRY_U, oper) ||
          L_is_opcode (Lop_MUL_WIDE_U, oper));
}

int
L_unsigned_int_comparative_opcode (L_Oper * oper)
{
  return (L_unsigned_int_comparison_opcode (oper) ||
          L_unsigned_int_pred_comparison_opcode (oper) ||
          L_unsigned_int_cond_branch_opcode (oper));
}

int
L_int_opcode (L_Oper * oper)
{
  return (L_int_move_opcode (oper) ||
          L_int_arithmetic_opcode (oper) ||
          L_int_cond_branch_opcode (oper) ||
          L_uncond_branch_opcode (oper) ||
          L_register_branch_opcode (oper) ||
          L_subroutine_call_opcode (oper) ||
          L_subroutine_return_opcode (oper) ||
          L_int_load_opcode (oper) ||
          L_int_preincrement_load_opcode (oper) ||
          L_int_postincrement_load_opcode (oper) ||
          L_int_store_opcode (oper) ||
          L_int_preincrement_store_opcode (oper) ||
          L_int_postincrement_store_opcode (oper));
}

int
L_flt_opcode (L_Oper * oper)
{
  return (L_flt_move_opcode (oper) ||
          L_flt_arithmetic_opcode (oper) ||
          L_flt_cond_branch_opcode (oper) ||
          L_flt_load_opcode (oper) ||
          L_flt_preincrement_load_opcode (oper) ||
          L_flt_postincrement_load_opcode (oper) ||
          L_flt_store_opcode (oper) ||
          L_flt_preincrement_store_opcode (oper) ||
          L_flt_postincrement_store_opcode (oper));
}

int
L_dbl_opcode (L_Oper * oper)
{
  return (L_dbl_move_opcode (oper) ||
          L_dbl_arithmetic_opcode (oper) ||
          L_dbl_cond_branch_opcode (oper) ||
          L_dbl_load_opcode (oper) ||
          L_dbl_preincrement_load_opcode (oper) ||
          L_dbl_postincrement_load_opcode (oper) ||
          L_dbl_store_opcode (oper) ||
          L_dbl_preincrement_store_opcode (oper) ||
          L_dbl_postincrement_store_opcode (oper));
}

int
L_commutative_opcode (L_Oper * oper)
{
  if (L_load_opcode (oper))
    return 1;
  else if (L_store_opcode (oper))
    return 1;
  else
    switch (oper->opc)
      {
        /* some opers are missing here */
        case Lop_ADD:
        case Lop_ADD_U:
        case Lop_MUL:
        case Lop_MUL_U:
        case Lop_OR:
        case Lop_AND:
        case Lop_XOR:
        case Lop_NOR:
        case Lop_NAND:
        case Lop_NXOR:
        case Lop_ADD_F2:
        case Lop_MUL_F2:
        case Lop_ADD_F:
        case Lop_MUL_F:
          return 1;
        case Lop_BR:
          if (L_gen_beq_branch_opcode (oper) || L_gen_bne_branch_opcode (oper))
            return 1;
        default:
          return 0;
      }
}

int
L_pointer_store (L_Oper * oper)
{
  if (!L_general_store_opcode (oper))
    return 0;

  if (L_EXTRACT_BIT_VAL (oper->flags, L_OPER_LABEL_REFERENCE))
    return (0);
  if (L_EXTRACT_BIT_VAL (oper->flags, L_OPER_STACK_REFERENCE))
    return (0);
  if (M_is_stack_operand (oper->src[0]) || M_is_stack_operand (oper->src[1]))
    return 0;
  /* recheck label here incase got some due to constant propagation */
  if (L_is_label (oper->src[0]) || L_is_label (oper->src[1]))
    return 0;

  return 1;
}

/*
 *      conservative for now, don't check predicates
 */
int
L_no_danger (int macro_flag, int load_flag, int store_flag, L_Oper * opA,
             L_Oper * opB)
{
  int memory_flag;
  L_Oper *op;

  memory_flag = load_flag | store_flag;

  for (op = opA; op != NULL; op = op->next_op)
    {
      if (op == opB)
        break;
      if (L_sync_opcode (op))
        return 0;
      if (memory_flag && L_subroutine_call_opcode (op) &&
	  !L_independent_memory_and_jsr (NULL, opA, op))
	return 0;
      if (macro_flag && L_general_subroutine_call_opcode (op))
        return 0;
    }

  return 1;
}

/*
 *      For this predicate memory flags are set to 0 always
 */
int
L_no_danger_in_cb (L_Cb * cb)
{
  int macro_flag;
  L_Oper *oper;
  macro_flag = 0;
  for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
    {
      if (L_has_fragile_macro_operand (oper))
        {
          macro_flag = 1;
          break;
        }
    }
  return (L_no_danger (macro_flag, 0, 0, cb->first_op, cb->last_op));
}

/*
 *      conservative for now, don't check predicates
 */
int
L_no_br_between (L_Oper * opA, L_Oper * opB)
{
  L_Oper *oper;
  /*
   *  No branch op in (opA, opB)
   */
  for (oper = opA->next_op; oper != NULL; oper = oper->next_op)
    {
      if (oper == opB)
        break;
      if (L_is_control_oper (oper))
        return 0;
    }
  return 1;
}

/*
 *      conservative for now, don't check predicates
 */
int
L_no_jsr_between (L_Oper * opA, L_Oper * opB)
{
  L_Oper *oper;
  /*
   *  No branch op in (opA, opB)
   */
  for (oper = opA->next_op; oper != NULL; oper = oper->next_op)
    {
      if (oper == opB)
        break;
      if (L_subroutine_call_opcode (oper))
        return 0;
    }
  return 1;
}

int
L_no_control_oper_between (L_Oper * opA, L_Oper * opB)
{
  L_Oper * oper;
  for (oper = opA->next_op; oper != NULL; oper = oper->next_op)
    {
      if (oper == opB)
	break;
      if (L_is_control_oper (oper))
	return 0;
    }
  return 1;
}

/*
 *      return true if can change the value of operand in oper.
 *      This predicate used to prevent changing operand of instructions
 *      which require a specific destination.  The only example of this
 *      now is pre/post inc memory instructions where addr dest and
 *      base addr src must be same.
 */
int
L_can_change_dest_operand (L_Oper * oper, L_Operand * operand)
{
  if (!operand)
    return 0;

  if (M_arch == M_TAHOE)
    {
      if (oper->opc == Lop_DEFINE)
        return (0);

      /* Bob McGowan - 6/97 - branch registers can only go on moves */
      /* MCM - can't change a destination register to a br reg without
	 changing is MOV proc_opc.  So disallow that for now, too. 9/01 */

      if (L_is_ctype_btr (operand))
	return (0);
    }

  /* 2nd dest of pre/post inc load is modified address */
  if (L_preincrement_load_opcode (oper) || L_postincrement_load_opcode (oper))
    {
      return !L_same_operand (operand, oper->dest[0]);
    }
  /* 1st dest of pre/post inc store is modified address */
  else if (L_preincrement_store_opcode (oper) ||
           L_postincrement_store_opcode (oper))
    {
      return !L_same_operand (operand, oper->dest[0]);
    }
  /* Don't modify dest of CMOV - JEM 1/7/95 */
  else if (L_select_opcode (oper) &&
           L_EXTRACT_BIT_VAL (oper->flags, L_OPER_IS_CMOV))
    {
      return 0;
    }
  else if (L_bit_deposit_opcode (oper))
    {
      return 0;
    }

  return 1;
}

int
L_can_change_src_operand (L_Oper * oper, L_Operand * operand)
{
  if (!operand)
    return 0;

  if (M_arch == M_TAHOE)
    {
      if (oper->opc == Lop_DEFINE)
        return (0);

      /* Bob McGowan - 6/97 - branch registers can only go on moves */
      if (L_is_ctype_btr (operand) && (oper->opc != Lop_MOV))
	return (0);
    }

  /* 1st and 2nd srcs of pre/post inc load/store is address */

  if (L_preincrement_load_opcode (oper) || 
      L_postincrement_load_opcode (oper) ||
      L_preincrement_store_opcode (oper) ||
      L_postincrement_store_opcode (oper))
    {
      return !(L_same_operand (operand, oper->src[0]) ||
	       L_same_operand (operand, oper->src[1]));
    }
  /* Don't modify srcs of CMOV - JEM 1/7/95 */
  else if (L_select_opcode (oper) &&
           L_EXTRACT_BIT_VAL (oper->flags, L_OPER_IS_CMOV))
    {
      return !L_same_operand (oper->dest[0], operand);
    }
  else if (L_bit_deposit_opcode (oper))
    {
      return !L_same_operand (oper->dest[0], operand);
    }
  return 1;
}

/*===========================================================================*/
/*
 *      L_Cb boolean predicates
 */
/*===========================================================================*/

int
L_single_predecessor_cb (L_Cb * cb)
{
  if (!cb)
    L_punt ("L_single_predecessor_cb: cb is NIL");
  if (!cb->src_flow)
    return 0;
  return (!cb->src_flow->next_flow);
}

/* 
 * Predicate analysis must be available
 */

int
L_has_fallthru_to_next_cb (L_Cb * cb)
{
  L_Oper *oper;
  int result;

  if (!cb)
    L_punt ("L_has_fallthru_to_next_cb: cb is NULL");

  if (!cb->dest_flow)
    return 0;

  /* if not a hyperblock, just look at last instr */
  if (!L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK))
    {
      oper = cb->last_op;
      result = !(L_uncond_branch_opcode (oper) ||
		 L_register_branch_opcode (oper) ||
		 L_subroutine_return_opcode (oper));
    }
  /* For hyperblock, have to look at successors of all instrs in cb */
  else if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK_NO_FALLTHRU))
    {
      /* If-conversion has recorded that this CB has no fallthrough */
      result = 0;
    }
  else
    {
      List jump_list = NULL;

      for (oper = cb->last_op; oper; oper = oper->prev_op)
        {
          if (L_uncond_branch_opcode (oper))
            {
              if (!oper->pred[0])
		break;
              jump_list = List_insert_last (jump_list, oper);
            }
        }

      result = oper ? 0 : 
	!PG_collectively_exhaustive_predicates_ops (jump_list);

      jump_list = List_reset (jump_list);
    }
  return result;
}


/*===========================================================================*/
/*
 *      L_Data boolean predicate
 */
/*===========================================================================*/

int
L_data_token_type (int token)
{
  switch (token)
    {
    case L_INPUT_MS:
    case L_INPUT_VOID:
    case L_INPUT_BYTE:
    case L_INPUT_WORD:
    case L_INPUT_LONG:
    case L_INPUT_LONGLONG:
    case L_INPUT_FLOAT:
    case L_INPUT_DOUBLE:
    case L_INPUT_ALIGN:
    case L_INPUT_ASCII:
    case L_INPUT_ASCIZ:
    case L_INPUT_RESERVE:
    case L_INPUT_SKIP:
    case L_INPUT_GLOBAL:
    case L_INPUT_WB:
    case L_INPUT_WW:
    case L_INPUT_WI:
    case L_INPUT_WQ:
    case L_INPUT_WF:
    case L_INPUT_WF2:
    case L_INPUT_WS:
    case L_INPUT_ELEMENT_SIZE:
    case L_INPUT_DEF_STRUCT:    /* Folded in SAM fix. -JCG 5/99 */
    case L_INPUT_DEF_UNION:
    case L_INPUT_DEF_ENUM:
    case L_INPUT_FIELD:
    case L_INPUT_ENUMERATOR:
      return (1);
    default:
      return (0);
    }
}

/*===========================================================================*/
/*
 *      L_Attr functions
 */
/*===========================================================================*/

/* copy individual attr, dont follow next ptr */
L_Attr *
L_copy_attr_element (L_Attr * attr)
{
  int i;
  L_Attr *new_attr;

  new_attr = L_new_attr (attr->name, attr->max_field);

  for (i = 0; i < attr->max_field; i++)
    new_attr->field[i] = L_copy_operand (attr->field[i]);

  return (new_attr);
}

/* this copies the entire list!! */
L_Attr *
L_copy_attr (L_Attr * list)
{
  L_Attr *ptr, *new_attr;

  if (!list)
    return NULL;

  new_attr = NULL;
  for (ptr = list; ptr != NULL; ptr = ptr->next_attr)
    new_attr = L_concat_attr (new_attr, L_copy_attr_element (ptr));

  return (new_attr);
}

/* merge list1 and list2 into list2                  */
/* Note: this doesn't maintain order in field arrays */
L_Attr *
L_merge_attr_lists (L_Attr * list1, L_Attr * list2)
{
  int i, j, flag;
  L_Attr *ptr1, *ptr2, *new_attr;
  L_Operand *field1, *field2;

  for (ptr2 = list2; ptr2 != NULL; ptr2 = ptr2->next_attr)
    {
      /* it doesn't exist in list1, so just make new one from scratch */
      if (!(ptr1 = L_find_attr (list1, ptr2->name)))
        {
          new_attr = L_copy_attr_element (ptr2);
          list1 = L_concat_attr (list1, new_attr);
        }
      /* it does exist in list1, so just merge fields the best we can */
      else
        {
          for (i = 0; i < ptr2->max_field; i++)
            {
              if (!(field2 = ptr2->field[i]))
                continue;
              flag = 0;
              for (j = 0; j < ptr1->max_field; j++)
                {
                  field1 = ptr1->field[j];
                  if (L_same_operand (field1, field2))
                    {
                      flag = 1; /* exact match, no need to copy */
                      break;
                    }
                }
              if (!flag)
                {
                  if (ptr1->field[i] == NULL)
                    L_set_attr_field (ptr1, i, L_copy_operand (field2));
                  else
                    L_set_attr_field (ptr1, ptr1->max_field,
                                      L_copy_operand (field2));
                }
            }
        }
    }

  return (list1);
}

L_Operand *
L_find_attr_field (L_Attr * attr, int type)
{
  int i;

  if (attr == NULL)
    return (NULL);

  for (i = 0; i < attr->max_field; i++)
    {
      if (!attr->field[i])
        continue;
      if (L_return_old_type (attr->field[i]) == type)
        return (attr->field[i]);
    }

  return (NULL);
}

L_Operand *
L_find_cb_attr_field (L_Attr * attr, L_Cb * cb)
{
  int i;

  if (!attr)
    return (NULL);

  for (i = 0; i < attr->max_field; i++)
    {
      if (!L_is_cb (attr->field[i]))
        continue;
      if (attr->field[i]->value.cb == cb)
        return (attr->field[i]);
    }

  return (NULL);
}

L_Operand *
L_find_int_attr_field (L_Attr * attr, int val)
{
  int i;

  if (!attr)
    return (NULL);

  for (i = 0; i < attr->max_field; i++)
    {
      if (!L_is_int_constant (attr->field[i]))
        continue;
      if ((int) attr->field[i]->value.i == val)
        return (attr->field[i]);
    }

  return (NULL);
}

L_Operand *
L_find_float_attr_field (L_Attr * attr, float val)
{
  int i;

  if (!attr)
    return (NULL);

  for (i = 0; i < attr->max_field; i++)
    {
      if (!L_is_flt_constant (attr->field[i]))
        continue;
      if (attr->field[i]->value.f == val)
        return (attr->field[i]);
    }

  return (NULL);
}

L_Operand *
L_find_double_attr_field (L_Attr * attr, double val)
{
  int i;

  if (!attr)
    return (NULL);

  for (i = 0; i < attr->max_field; i++)
    {
      if (!L_is_dbl_constant (attr->field[i]))
        continue;
      if (attr->field[i]->value.f2 == val)
        return (attr->field[i]);
    }

  return (NULL);
}

L_Operand *
L_find_string_attr_field (L_Attr * attr, char *str)
{
  int i;

  if (!attr)
    return (NULL);

  for (i = 0; i < attr->max_field; i++)
    {
      if (!L_is_string (attr->field[i]))
        continue;
      if (!strcmp (attr->field[i]->value.l, str))
        return (attr->field[i]);
    }

  return (NULL);
}

L_Operand *
L_find_macro_attr_field (L_Attr * attr, int mac, int ctype, int ptype)
{
  int i;

  if (!attr)
    return (NULL);

  for (i = 0; i < attr->max_field; i++)
    {
      if (!L_is_macro (attr->field[i]))
        continue;
      if ((attr->field[i]->value.mac == mac) &&
          (L_return_old_ctype (attr->field[i]) == ctype) &&
          (attr->field[i]->ptype == ptype))
        return (attr->field[i]);
    }

  return (NULL);
}

L_Operand *
L_find_register_attr_field (L_Attr * attr, int index, int ctype, int ptype)
{
  int i;

  if (!attr)
    return (NULL);

  for (i = 0; i < attr->max_field; i++)
    {
      if (!L_is_register (attr->field[i]))
        continue;
      if ((attr->field[i]->value.r == index) &&
          (attr->field[i]->ctype == ctype) &&
          (attr->field[i]->ptype == ptype))
        return (attr->field[i]);
    }

  return (NULL);
}

L_Operand *
L_find_label_attr_field (L_Attr * attr, char *label)
{
  int i;

  if (!attr)
    return (NULL);

  for (i = 0; i < attr->max_field; i++)
    {
      if (!L_is_label (attr->field[i]))
        continue;
      if (!strcmp (attr->field[i]->value.l, label))
        return (attr->field[i]);
    }

  return (NULL);
}

L_Operand *
L_find_rregister_attr_field (L_Attr * attr, int index, int ctype, int ptype)
{
  int i;

  if (!attr)
    return (NULL);

  for (i = 0; i < attr->max_field; i++)
    {
      if (!L_is_register (attr->field[i]))
        continue;
      if ((attr->field[i]->value.rr == index) &&
          (L_return_old_ctype (attr->field[i]) == ctype) &&
          (attr->field[i]->ptype == ptype))
        return (attr->field[i]);
    }

  return (NULL);
}

L_Operand *
L_find_evr_attr_field (L_Attr * attr, int index, int omega,
                       int ctype, int ptype)
{
  int i;

  if (!attr)
    return (NULL);

  for (i = 0; i < attr->max_field; i++)
    {
      if ((!L_is_register (attr->field[i])))
        continue;
      if ((attr->field[i]->value.evr.num == index) &&
          (attr->field[i]->value.evr.omega == omega) &&
          (L_return_old_ctype (attr->field[i]) == ctype) &&
          (attr->field[i]->ptype == ptype))
        return (attr->field[i]);
    }

  return (NULL);
}

/*===========================================================================*/
/*
 *      L_Sync functions
 */
/*===========================================================================*/

L_Sync *
L_create_new_sync (int num, char prob, char freq, int dist,
                   int flags, int prof)
{
  L_Sync *new_sync;
  L_Oper *dep_oper;
  short info;

  dep_oper = L_oper_hash_tbl_find_and_alloc_oper (L_fn->oper_hash_tbl, num);

  new_sync = L_new_sync (dep_oper);
  new_sync->dist = dist;
  new_sync->prof_info = prof;

  info = (short) flags;
  info &= 0x1fff;

  if ((prob == 'D') || (prob == 'd'))
    info = SET_DEFINITE_SYNC (info);

  /* BCC - use mnemonic names for frequency  - 2/2/99 */
  if ((prob == 'P') || (prob == 'p'))
    info = SET_PROFILE_SYNC (info);

  if (IS_PROFILE_SYNC (flags))
    {
      if (prof == 0)
        info |= 0x6000;
      else if (prof < 30)
        info |= 0x4000;
      else if (prof < 80)
        info |= 0x2000;
    }
  else
    {
      switch (freq)
        {
        case 'A':
        case 'a':
          break;
        case 'F':
        case 'f':
          info |= 0x2000;
          break;
        case 'S':
        case 's':
          info |= 0x4000;
          break;
        case 'I':
        case 'i':
          info |= 0x6000;
          break;
        default:
          L_punt ("L_create_new_sync: unexpected freq type");
        }
    }
  new_sync->info = info;

  return (new_sync);
}

void
L_insert_all_syncs_in_dep_opers (L_Oper * oper)
{
  int i;
  L_Sync_Info *sync_info;

  L_Sync *tail_sync, *head_sync;

  sync_info = oper->sync_info;

  if (!sync_info)
    return;

  for (i = 0; i < sync_info->num_sync_out; i++)
    {
      tail_sync = L_copy_sync (sync_info->sync_out[i]);
      tail_sync->dep_oper = oper;
      L_insert_tail_sync_in_oper (sync_info->sync_out[i]->dep_oper,
                                  tail_sync);
    }

  for (i = 0; i < sync_info->num_sync_in; i++)
    {
      head_sync = L_copy_sync (sync_info->sync_in[i]);
      head_sync->dep_oper = oper;
      L_insert_head_sync_in_oper (sync_info->sync_in[i]->dep_oper, head_sync);
    }

  for (i = 0; i < sync_info->num_sync_out; i++)
    L_add_sync_between_opers (sync_info->sync_out[i]->dep_oper, oper);

  for (i = 0; i < sync_info->num_sync_in; i++)
    L_add_sync_between_opers (oper, sync_info->sync_in[i]->dep_oper);

  return;
}

/*===========================================================================*/
/*
 *      L_AccSpec functions
 */
/*===========================================================================*/

L_AccSpec *
L_new_mem_acc_spec (int is_def, int id, int version, int offset, int size)
{
  L_AccSpec *mas;

  mas = (L_AccSpec *) L_alloc (L_alloc_acc_spec);

  mas->is_def = is_def;
  mas->id = id;
  mas->version = version;
  mas->offset = offset;
  mas->size = size;
  mas->next = NULL;

  return mas;
}


L_AccSpec *
L_copy_mem_acc_spec (L_AccSpec *mas)
{
  return L_new_mem_acc_spec (mas->is_def, mas->id, mas->version, 
			     mas->offset, mas->size);
}


L_AccSpec *
L_delete_mem_acc_spec (L_AccSpec *mas)
{
  L_free (L_alloc_acc_spec, (void *)mas);
  return NULL;
}


L_AccSpec *
L_delete_mem_acc_spec_list (L_AccSpec *mas)
{
  L_AccSpec *nxt;

  while (mas)
    {
      nxt = mas->next;
      L_delete_mem_acc_spec (mas);
      mas = nxt;
    }

  return NULL;
}

/*===========================================================================*/
/*
 *      L_Flow functions
 */
/*===========================================================================*/

L_Flow *
L_find_flow (L_Flow * list, int cc, L_Cb * src_cb, L_Cb * dst_cb)
{
  L_Flow *f;
  for (f = list; f != NULL; f = f->next_flow)
    {
      if (f->cc != cc)
        continue;
      if (f->src_cb != src_cb)
        continue;
      if (f->dst_cb != dst_cb)
        continue;
      return f;
    }
  L_punt ("L_find_flow: cannot find flow arc");
  return (NULL);
}

L_Flow *
L_find_flow_with_dst_cb (L_Flow * list, L_Cb * dst_cb)
{
  L_Flow *f;

  for (f = list; f != NULL; f = f->next_flow)
    {
      if (f->dst_cb == dst_cb)
        return (f);
    }

  L_punt ("L_find_flow_with_dst_cb: cannot find flow arc");
  return (NULL);
}

L_Flow *
L_find_flow_with_src_cb (L_Flow * list, L_Cb * src_cb)
{
  L_Flow *f;

  for (f = list; f != NULL; f = f->next_flow)
    {
      if (f->src_cb == src_cb)
        return (f);
    }

  L_punt ("L_find_flow_with_src_cb: cannot find flow arc");
  return (NULL);
}

L_Flow *
L_find_matching_flow (L_Flow * list, L_Flow * flow)
{
  L_Flow *f;
  for (f = list; f != NULL; f = f->next_flow)
    {
      if (f->cc != flow->cc)
        continue;
      if (f->src_cb != flow->src_cb)
        continue;
      if (f->dst_cb != flow->dst_cb)
        continue;
      if ((f->weight < (flow->weight - ZERO_EQUIVALENT)) ||
          (f->weight > (flow->weight + ZERO_EQUIVALENT)))
        continue;
      return f;
    }
  for (f = list; f != NULL; f = f->next_flow)
    {
      fprintf (stderr, "\t%d: cb%d->cb%d (%f)\n",
               f->cc, f->src_cb->id, f->dst_cb->id, f->weight);
    }
  fprintf (stderr, "Flow %d: cb%d->cb%d (%f) is not in list\n",
           flow->cc, flow->src_cb->id, flow->dst_cb->id, flow->weight);
  L_punt ("L_find_matching_flow: cannot find flow arc");
  return (NULL);
}

L_Flow *
L_find_last_flow (L_Flow * list)
{
  L_Flow *f;
  if (list == NULL)
    L_punt ("L_find_last_flow: list cannot be NIL");
  f = list;
  while (f->next_flow != NULL)
    {
      f = f->next_flow;
    }
  return f;
}

L_Flow *
L_find_second_to_last_flow (L_Flow * list)
{
  L_Flow *f;
  if (list == NULL)
    L_punt ("L_find_second_to_last_flow: list cannot be NIL");
  if (list->next_flow == NULL)
    L_punt ("L_find_second_to_last_flow: list only has 1 element");
  f = list;
  while (f->next_flow->next_flow != NULL)
    {
      f = f->next_flow;
    }
  return f;
}

L_Flow *
L_find_max_weight_flow (L_Flow * list)
{
  double max_weight;
  L_Flow *f, *max;
  if (list == NULL)
    L_punt ("L_find_max_weight_flow: list is NULL");
  max = list;
  max_weight = list->weight;
  for (f = list->next_flow; f != NULL; f = f->next_flow)
    {
      if (f->weight > max_weight)
        {
          max = f;
          max_weight = f->weight;
        }
    }
  return max;
}

L_Flow *
L_find_flow_for_branch (L_Cb * cb, L_Oper * oper)
{
  L_Flow *flow;
  L_Oper *ptr;
  flow = cb->dest_flow;
  for (ptr = cb->first_op; ptr != NULL; ptr = ptr->next_op)
    {
      if (ptr == oper)
        return flow;
      if (L_is_control_oper (ptr))
        flow = flow->next_flow;
    }
  L_punt ("L_find_flow_for_branch: oper not found");
  return (NULL);
}

L_Oper *
L_find_branch_for_flow (L_Cb * cb, L_Flow * flow)
{
  L_Oper *oper;
  L_Flow *ptr, *f;
  ptr = cb->dest_flow;
  for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
    {
      if (!L_is_control_oper (oper))
        continue;
      if (ptr == flow)
        return oper;
      ptr = ptr->next_flow;
    }
  /* for jump_rg, multiple flows associated with arc */
  if (L_register_branch_opcode (cb->last_op))
    {
      for (f = ptr; f != NULL; f = f->next_flow)
        {
          if (f == flow)
            return (cb->last_op);
        }
    }
  return NULL;
}

L_Flow *
L_copy_flow (L_Flow * list)
{
  L_Flow *new_flow, *ptr, *head;

  if (list == NULL)
    return NULL;

  head = NULL;
  for (ptr = list; ptr != NULL; ptr = ptr->next_flow)
    {
      new_flow = L_new_flow (ptr->cc, ptr->src_cb, ptr->dst_cb, ptr->weight);
      /* REH 8/20/95 - copy the flags here, rather than change */
      /*   the interface to L_new_flow.                        */
      new_flow->flags = ptr->flags;
      head = L_concat_flow (head, new_flow);
    }

  return head;
}

/*
 *      Same as above but don't flow next/prev ptrs
 */
L_Flow *
L_copy_single_flow (L_Flow * flow)
{
  L_Flow *new_flow;

  if (flow == NULL)
    return (NULL);

  new_flow = L_new_flow (flow->cc, flow->src_cb, flow->dst_cb, flow->weight);
  /* REH 8/20/95 - copy the flags here, rather than change */
  /*   the interface to L_new_flow.                    */
  new_flow->flags = flow->flags;

  return (new_flow);
}

void
L_change_flow (L_Flow * list, int cc, L_Cb * src_cb, L_Cb * dst_cb,
               double weight)
{
  list->cc = cc;
  list->src_cb = src_cb;
  list->dst_cb = dst_cb;
  list->weight = weight;
}

void
L_change_dest (L_Flow * list, L_Cb * o_dest, L_Cb * n_dest)
{
  L_Flow *ptr;
  if (list == NULL)
    L_punt ("L_change_dest: list cannot be NIL");
  if (o_dest == n_dest)
    L_punt ("L_change_dest: o_dest and n_dest must be different");
  for (ptr = list; ptr != NULL; ptr = ptr->next_flow)
    {
      if (ptr->dst_cb == o_dest)
        ptr->dst_cb = n_dest;
    }
}

void
L_change_src (L_Flow * list, L_Cb * o_src, L_Cb * n_src)
{
  L_Flow *ptr;
  if (list == NULL)
    L_punt ("L_change_src_cb: list cannot be NIL");
  if (o_src == n_src)
    L_punt ("L_change_src_cb: o_src and n_src must be different");
  for (ptr = list; ptr != NULL; ptr = ptr->next_flow)
    {
      if (ptr->src_cb == o_src)
        ptr->src_cb = n_src;
    }
}

void
L_change_dest_cb (L_Cb * src, L_Cb * odest, L_Cb * ndest)
{
  L_Flow *ptr, *new_flow, *next;
  if (odest == ndest)
    L_punt ("L_change_dest_cb: o_dest and n_dest must be different");
  /* Fix all src arcs */
  for (ptr = odest->src_flow; ptr != NULL; ptr = next)
    {
      next = ptr->next_flow;
      if (ptr->src_cb != src)
        continue;
      new_flow = L_new_flow (ptr->cc, src, ndest, ptr->weight);
      odest->src_flow = L_delete_flow (odest->src_flow, ptr);
      ndest->src_flow = L_concat_flow (ndest->src_flow, new_flow);
    }
  /* Fix all dest arcs */
  for (ptr = src->dest_flow; ptr != NULL; ptr = ptr->next_flow)
    {
      if (ptr->dst_cb != odest)
        continue;
      ptr->dst_cb = ndest;
    }
}

void
L_change_src_cb (L_Cb * dest, L_Cb * osrc, L_Cb * nsrc)
{
  L_Flow *ptr, *new_flow, *next;
  if (osrc == nsrc)
    L_punt ("L_change_src_cb: o_src and n_src must be different");
  /* Fix all dest arcs */
  for (ptr = osrc->dest_flow; ptr != NULL; ptr = next)
    {
      next = ptr->next_flow;
      if (ptr->dst_cb != dest)
        continue;
      new_flow = L_new_flow (ptr->cc, nsrc, dest, ptr->weight);
      osrc->dest_flow = L_delete_flow (osrc->dest_flow, ptr);
      nsrc->dest_flow = L_concat_flow (nsrc->dest_flow, new_flow);
    }
  /* Fix all src arcs */
  for (ptr = dest->src_flow; ptr != NULL; ptr = ptr->next_flow)
    {
      if (ptr->src_cb != osrc)
        continue;
      ptr->src_cb = nsrc;
    }
}

void
L_scale_flow_weights (L_Flow * list, double scale)
{
  L_Flow *flow;

  for (flow = list; flow != NULL; flow = flow->next_flow)
    flow->weight *= scale;

  return;
}

/*
 *      Return 1 if dest flows are not in correct order of the branches.  This
 *      will occur if branches are reordered in a SB or HB.
 */
int
L_dest_flow_out_of_order (L_Cb * cb)
{
  L_Cb *target_cb;
  L_Oper *oper, *last;
  L_Flow *flow;

  if (!cb->first_op)
    return (0);

  last = cb->last_op;
  flow = cb->dest_flow;
  for (oper = cb->first_op; oper != last; oper = oper->next_op)
    {
      if (!L_is_control_oper (oper))
        continue;
      target_cb = L_find_branch_dest (oper);
      if (flow->dst_cb != target_cb)
        return (1);
      flow = flow->next_flow;
    }

  /* if all inner flows are correct, last one must be also */
  return (0);
}

/*
 *    Reorder dest flows with error, 2 or more flows to the same target are not
 *    guaranteed to be matched exactly, it will just pick one of the 
 *    available ones to go with each branch, so you will get correct flows
 *    but not necessarily the correct taken frequencies.  Someone can fix
 *    this in the future, I am just not patient enough to figure it out
 *    right now... SAM 11-14-94
 */
void
L_reorder_dest_flows (L_Cb * cb)
{
  L_Cb *target_cb;
  L_Oper *oper;
  L_Flow *list, *flow;

  list = cb->dest_flow;
  cb->dest_flow = NULL;

  for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
    {
      if (!L_is_control_oper (oper))
        continue;
      if (L_cond_branch_opcode (oper) || L_uncond_branch_opcode (oper)
          || L_check_opcode (oper))
        {
          target_cb = L_find_branch_dest (oper);
          flow = L_find_flow_with_dst_cb (list, target_cb);
          list = L_remove_flow (list, flow);
          cb->dest_flow = L_concat_flow (cb->dest_flow, flow);
        }
      else if (L_register_branch_opcode (oper))
        {
          cb->dest_flow = L_concat_flow (cb->dest_flow, list);
          list = NULL;
        }
      else
        {
          L_punt ("L_reorder_dest_flows: illegal branch %d", oper->id);
        }
    }

  /* concatenate last flow if it exists onto end of list */
  if (list != NULL)
    {
      cb->dest_flow = L_concat_flow (cb->dest_flow, list);
      list = NULL;
    }
}

/*===========================================================================*/
/*
 *      L_Operand functions
 */
/*===========================================================================*/

/* DIA - Rehacked L_copy_operand */
L_Operand *
L_copy_operand (L_Operand * operand)
{
  L_Operand *new_operand;

  if (!operand)
    return NULL;
  switch (L_operand_case_type (operand))
    {
    case L_OPERAND_CB:
      return (L_new_cb_operand (operand->value.cb));
    case L_OPERAND_INT:
      return (L_new_int_operand (operand->value.i, operand->ctype));
    case L_OPERAND_FLOAT:
      return (L_new_float_operand (operand->value.f));
    case L_OPERAND_DOUBLE:
      return (L_new_double_operand (operand->value.f2));
    case L_OPERAND_STRING:
      return (L_new_string_operand (operand->value.s, operand->ctype));
    case L_OPERAND_MACRO:
      new_operand = L_new_macro_operand (operand->value.mac, operand->ctype,
					 operand->ptype);
      if (L_is_ctype_predicate (operand))
        new_operand->value.pred.ssa = operand->value.pred.ssa;
      return new_operand;
    case L_OPERAND_REGISTER:
      new_operand = L_new_register_operand (operand->value.r, operand->ctype,
                                            operand->ptype);
      if (L_is_ctype_predicate (operand))
        new_operand->value.pred.ssa = operand->value.pred.ssa;
      return new_operand;
    case L_OPERAND_RREGISTER:
      return (L_new_rregister_operand (operand->value.rr, operand->ctype,
                                       operand->ptype));
    case L_OPERAND_EVR:
      return (L_new_evr_operand (operand->value.evr.num,
                                 operand->value.evr.omega, operand->ctype,
                                 operand->ptype));
    case L_OPERAND_LABEL:
      return (L_new_label_operand (operand->value.l, operand->ctype));
    default:
      {
        new_operand = (L_Operand *) L_alloc (L_alloc_operand);
        new_operand->type = operand->type;
        new_operand->ctype = operand->ctype;
        new_operand->ptype = operand->ptype;
        new_operand->value.init.u = operand->value.init.u;
        new_operand->value.init.l = operand->value.init.l;
        return (new_operand);
      }
    }
}

/*===========================================================================*/
/*
 *      L_Oper functions
 */
/*===========================================================================*/

L_Oper *
L_create_new_op (int opc)
{
  L_Oper *op;
  L_fn->max_oper_id++;
  op = L_new_oper (L_fn->max_oper_id);
  op->opc = opc;
  op->proc_opc = opc;
  op->opcode = L_opcode_name (opc);
  return op;
}

L_Oper *
L_create_new_op_with_id (int opc, int id)
{
  L_Oper *op;
  op = L_new_oper (id);
  op->opc = opc;
  op->proc_opc = opc;
  op->opcode = L_opcode_name (opc);
  return op;
}

/* 
 * This routine creates a new L_Oper but carries over characteristics
 * from the defining L_Oper.  Specifically, it includes the predicate,
 * and flags.  If the flag of the "using" oper is a "parent", it will
 * be marked as the parent opcode in the parent pointer and the parent_op 
 * flag will be cleared.  If the flag of the "using" oper is not a 
 * "parent" op, the parent pointer will set to the parent pointer of 
 * the "using" oper.
 */
/* 09/19/02 REK Updating to copy the completers. */
L_Oper *
L_create_new_op_using (int opc, L_Oper * using_op)
{
  L_Oper *op;
  L_Attr *attr;
  int i;

  /* if using is NULL, resort to default of just creating a op 
     with given opc */
  if (using_op == NULL)
    {
      return (L_create_new_op (opc));
    }

  /* Create a new L_oper and initialize the opcode. */
  L_fn->max_oper_id++;
  op = L_new_oper (L_fn->max_oper_id);
  op->opc = opc;
  op->proc_opc = opc;
  op->completers = using_op->completers;
  op->opcode = L_opcode_name (opc);

  /* Copy the predicates */
  for (i = 0; i < L_max_pred_operand; i++)
    op->pred[i] = L_copy_operand (using_op->pred[i]);

  /* Copy the flags and defining parent oper pointer */
  if ((L_EXTRACT_BIT_VAL (using_op->flags, L_OPER_PARENT)) ||
      (using_op->parent_op == NULL))
    {
      op->parent_op = using_op;
      op->flags = L_CLR_BIT_FLAG (using_op->flags, L_OPER_PARENT);
    }
  else
    {
      op->parent_op = using_op->parent_op;
      op->flags = using_op->flags;
    }

  /* Copy attribute field */
  attr = L_copy_attr (using_op->attr);
  op->attr = L_concat_attr (op->attr, attr);

  return (op);
}

/*
 *      Same as L_create_new_op_using() minus all the parent op stuff
 */
L_Oper *
L_create_new_similar_op (int opc, L_Oper * using_op)
{
  int i;
  L_Oper *new_op;
  L_Attr *attr;

  new_op = L_create_new_op (opc);
  if (using_op == NULL)
    return (new_op);

  /* Copy the flags */
  new_op->flags = using_op->flags;

  /* Copy the predicates */
  for (i = 0; i < L_max_pred_operand; i++)
    new_op->pred[i] = L_copy_operand (using_op->pred[i]);

  /* Copy the attributes */
  attr = L_copy_attr (using_op->attr);
  new_op->attr = L_concat_attr (new_op->attr, attr);

  return (new_op);
}

/*
 *      copy all fields excluding ext and ext1
 */
/* 09/19/02 REK Updating this function to copy the completers field. */
L_Oper *
L_copy_operation (L_Oper * op)
{
  L_Oper *new_op;
  L_Attr *attr;
  L_Sync_Info *sync_info;
  int i;

  new_op = L_new_oper (++L_fn->max_oper_id);
  new_op->opc = op->opc;
  new_op->proc_opc = op->proc_opc;
  /* 09/19/02 REK Copy the completers field as well. */
  new_op->completers = op->completers;
  new_op->opcode = op->opcode;
  new_op->flags = op->flags;
  new_op->parent_op = op->parent_op;
  new_op->weight = op->weight;

  for (i = 0; i < L_MAX_CMPLTR; i++)
    new_op->com[i] = op->com[i];

  /* Copy the operands */
  for (i = 0; i < L_max_dest_operand; i++)
    if (op->dest[i] != NULL)
      new_op->dest[i] = L_copy_operand (op->dest[i]);

  for (i = 0; i < L_max_src_operand; i++)
    if (op->src[i] != NULL)
      new_op->src[i] = L_copy_operand (op->src[i]);

  for (i = 0; i < L_max_pred_operand; i++)
    if (op->pred[i] != NULL)
      new_op->pred[i] = L_copy_operand (op->pred[i]);

  /* Copy sync arcs */

  if (L_func_contains_dep_pragmas || L_func_acc_omega)
    {
      if (L_general_load_opcode (op) || L_general_store_opcode (op) ||
          L_subroutine_call_opcode (op))
        {
          if (op->sync_info)
            {
              sync_info = op->sync_info;
              new_op->sync_info = L_copy_sync_info (sync_info);

              /* for all other opers which are dependent upon op, 
                 add sync arcs to op */
              /* Brought insertion of sync arcs within confines of 
                 sync_info check -DAC */
              L_insert_all_syncs_in_dep_opers (new_op);
            }

	  /* If appropriate, add sync arcs between the op and its copy */

	  if ((L_func_contains_dep_pragmas || 
	       (L_func_acc_omega && op->sync_info)) && 
	      (L_general_store_opcode (op) || L_subroutine_call_opcode (op)))
	    {
	      L_add_sync_between_opers (op, new_op);
	      L_add_sync_between_opers (new_op, op);
	      L_add_sync_between_opers (new_op, new_op);
	    }
	}
    }

  if (L_func_acc_specs && op->acc_info)
    new_op->acc_info = L_copy_mem_acc_spec_list (op->acc_info);

  /* Copy attribute field */
  attr = L_copy_attr (op->attr);
  new_op->attr = L_concat_attr (new_op->attr, attr);

  return (new_op);
}

void
L_move_op_to_end_of_block (L_Cb * from_cb, L_Cb * to_cb, L_Oper * op)
{
  if (!from_cb || !to_cb || !op)
    L_punt ("L_move_op_to_end_of_block: from_cb, to_cb, op cannot be NULL");

  L_remove_oper (from_cb, op);
  L_insert_oper_after (to_cb, to_cb->last_op, op);
}

void
L_move_op_to_start_of_block (L_Cb * from_cb, L_Cb * to_cb, L_Oper * op)
{
  if (!from_cb || !to_cb || !op)
    L_punt ("L_move_op_to_start_of_block: from_cb, to_cb, op cannot be NULL");

  L_remove_oper (from_cb, op);
  L_insert_oper_before (to_cb, to_cb->first_op, op);
}

/*
 *      Change br_op to new opcode and destination
 *              opcode should be a legal branch opcode
 */
void
L_change_cond_br (L_Oper * br_op, ITuint8 newcom, L_Cb * dest)
{
  if (!L_cond_branch_opcode (br_op))
    L_punt ("L_change_cond_br: illegal opcode");
  L_set_compare_type (br_op, newcom);
  br_op->src[2]->value.cb = dest;
}

L_Cb *
L_find_branch_dest (L_Oper * op)
{
  if (!op)
    L_punt ("L_find_branch_dest: op is NULL");

  if (L_uncond_branch_opcode (op))
    return (op->src[0]->value.cb);
  else if (L_cond_branch_opcode (op))
    return (op->src[2]->value.cb);
  else if (L_check_opcode (op))
    return (op->src[1]->value.cb);
  else
    {
      L_punt ("L_find_branch_dest: illegal opcode");
      return (NULL);
    }
}

int
L_change_branch_dest (L_Oper * op, L_Cb * old_cb, L_Cb * new_cb)
{
  int retval = 0;

  if (L_uncond_branch_opcode (op))
    {
      if (op->src[0]->value.cb == old_cb)
        {
          op->src[0]->value.cb = new_cb;
          retval = 1;
        }
    }
  else if (L_cond_branch_opcode (op))
    {
      if (op->src[2]->value.cb == old_cb)
        {
          /* Properly handle the conditional branches created by
           * the multiway jump optimizations that create explicit
           * tests of a label to a cb id and then jumps to that same
           * cb.  -JCG 10/99
           */
          if (L_same_operand (op->src[1], op->src[2]))
            op->src[1]->value.cb = new_cb;
          op->src[2]->value.cb = new_cb;
          retval = 1;
        }
    }
  else if (L_register_branch_opcode (op))
    {
      L_Cb *cb_iter;
      L_Oper *oper_iter;
      int src_indx;

      retval = L_change_cb_id (old_cb->id, new_cb->id);

      /* JWS 20000722 - Suboptimally prevent cb operand obsolescence.
       * This is required due to multiway branch expansion, which uses cb
       * operands in comparisons on conditional branches.
       */

      for (cb_iter = L_fn->first_cb; cb_iter != NULL;
           cb_iter = cb_iter->next_cb)
        {
          for (oper_iter = cb_iter->first_op; oper_iter != NULL;
               oper_iter = oper_iter->next_op)
            {
              if (!L_cond_branch_opcode (oper_iter))
                continue;
              for (src_indx = 0; src_indx <= 1; src_indx++)
                {
                  if (!L_is_cb (oper_iter->src[src_indx]))
                    continue;
                  if (oper_iter->src[src_indx]->value.cb == old_cb)
                    oper_iter->src[src_indx]->value.cb = new_cb;
                }
            }
        }
    }
  else if (L_check_opcode (op))
    {
      if (op->src[1]->value.cb == old_cb)
        {
          op->src[1]->value.cb = new_cb;
          retval = 1;
        }
    }
  else
    {
      L_punt ("L_change_branch_dest: illegal op");
    }
  return retval;
}

void
L_change_all_branch_dest (L_Cb * cb, L_Cb * old, L_Cb * new_cb)
{
  L_Oper *op;

  for (op = cb->first_op; op != NULL; op = op->next_op)
    {
      if (!L_is_control_oper (op))
        continue;
      L_change_branch_dest (op, old, new_cb);
    }
}

L_Oper *
L_next_def (L_Operand * operand, L_Oper * op)
{
  int i;
  L_Oper *ptr;

  if ((op == NULL) || (!operand))
    L_punt ("L_next_def: op and operand cannot be NIL");

  if (L_is_predicated (op))
    {
      for (ptr = op->next_op; ptr != NULL; ptr = ptr->next_op)
        {
          if (!PG_intersecting_predicates_ops (op, ptr))
            continue;
          for (i = 0; i < L_max_dest_operand; i++)
            {
              if (L_same_operand (ptr->dest[i], operand))
                return ptr;
            }
        }
    }
  else
    {
      for (ptr = op->next_op; ptr != NULL; ptr = ptr->next_op)
        {
          for (i = 0; i < L_max_dest_operand; i++)
            {
              if (L_same_operand (ptr->dest[i], operand))
                return ptr;
            }
        }
    }
  return NULL;
}

L_Oper *
L_prev_def (L_Operand * operand, L_Oper * op)
{
  int i;
  L_Oper *ptr;

  if ((!operand) || (op == NULL))
    L_punt ("L_prev_def: operand and op cannot be NIL");

  for (ptr = op->prev_op; ptr != NULL; ptr = ptr->prev_op)
    {
      if (!PG_intersecting_predicates_ops (ptr, op))
	continue;
      for (i = 0; i < L_max_dest_operand; i++)
	if (L_same_operand (ptr->dest[i], operand))
	  return ptr;
    }

  return NULL;
}

L_Oper *
L_next_use (L_Operand * operand, L_Oper * op)
{
  int i;
  L_Oper *ptr;

  if ((!operand) || (op == NULL))
    L_punt ("L_next_use: operand and op cannot be NIL");

  for (ptr = op->next_op; ptr != NULL; ptr = ptr->next_op)
    {
      if (!PG_intersecting_predicates_ops (op, ptr))
	continue;
      for (i = 0; i < L_max_src_operand; i++)
	if (L_same_operand (ptr->src[i], operand))
	  return ptr;
    }

  return NULL;
}

L_Oper *
L_prev_use (L_Operand * operand, L_Oper * op)
{
  int i;
  L_Oper *ptr;

  if ((!operand) || (op == NULL))
    L_punt ("L_prev_use: operand and op cannot be NIL");

  for (ptr = op->prev_op; ptr != NULL; ptr = ptr->prev_op)
    {
      if (!PG_intersecting_predicates_ops (ptr, op))
	continue;
      for (i = 0; i < L_max_src_operand; i++)
	if (L_same_operand (ptr->src[i], operand))
	  return ptr;
    }

  return NULL;
}

L_Oper *
L_find_def (L_Cb * cb, L_Operand * operand)
{
  int i;
  L_Oper *ptr;

  if ((cb == NULL) || (!operand))
    L_punt ("L_find_def: cb and operand cannot be NIL");

  for (ptr = cb->first_op; ptr != NULL; ptr = ptr->next_op)
    {
      for (i = 0; i < L_max_dest_operand; i++)
        {
          if (L_same_operand (ptr->dest[i], operand))
            return ptr;
        }
    }

  return NULL;
}

L_Oper *
L_find_use (L_Cb * cb, L_Operand * operand, L_Oper * omit)
{
  int i;
  L_Oper *ptr;

  if ((cb == NULL) || (!operand))
    L_punt ("L_find_use: cb and operand cannot be NIL");

  for (ptr = cb->first_op; ptr != NULL; ptr = ptr->next_op)
    {
      if (ptr == omit)
        continue;
      for (i = 0; i < L_max_src_operand; i++)
        {
          if (L_same_operand (ptr->src[i], operand))
            return ptr;
        }
    }
  return NULL;
}

L_Oper *
L_prev_tm_memory_def (L_Oper * op, int tm)
{
  L_Oper *ptr;
  L_Attr *attr;

  if (L_is_predicated (op))
    {
      for (ptr = op->prev_op; ptr != NULL; ptr = ptr->prev_op)
        {
          if (!PG_intersecting_predicates_ops (ptr, op))
            continue;

          if (!L_general_store_opcode (ptr))
            continue;

          if ((attr = L_find_attr (ptr->attr, "tm")))
            {

              if ((int) attr->field[0]->value.i == tm)
                return (ptr);
            }
        }
    }
  else
    {
      for (ptr = op->prev_op; ptr != NULL; ptr = ptr->prev_op)
        {
          if (!L_general_store_opcode (ptr))
            continue;

          if ((attr = L_find_attr (ptr->attr, "tm")))
            {
              if ((int) attr->field[0]->value.i == tm)
                return (ptr);
            }
        }
    }
  return NULL;
}

void
L_nullify_operation (L_Oper * oper)
{
  int i;

  L_change_opcode (oper, Lop_NO_OP);
  oper->flags = 0;

  for (i = 0; i < L_max_dest_operand; i++)
    if (oper->dest[i])
      {
	L_delete_operand (oper->dest[i]);
	oper->dest[i] = NULL;
      }

  for (i = 0; i < L_max_src_operand; i++)
    if (oper->src[i])
      {
	L_delete_operand (oper->src[i]);
	oper->src[i] = NULL;
      }

  for (i = 0; i < L_max_pred_operand; i++)
    if (oper->pred[i])
      {
	L_delete_operand (oper->pred[i]);
	oper->pred[i] = NULL;
      }

  /* 1 indicates to delete corresponding syncs from all dep opers also */
  L_delete_all_sync (oper, 1);

  if (oper->acc_info)
    oper->acc_info = L_delete_mem_acc_spec_list (oper->acc_info);

  L_delete_all_attr (oper->attr);
  oper->attr = NULL;
}

void
L_change_opcode (L_Oper * oper, int opc)
{
  oper->opc = opc;
  oper->proc_opc = opc;
  oper->opcode = L_opcode_name (opc);
}

int
L_base_memory_opcode (L_Oper * oper)
{
  if (oper == NULL)
    L_punt ("L_base_memory_opcode: oper is NULL");

  switch (oper->opc)
    {
    case Lop_LD_UC:
      return (Lop_LD_UC);
    case Lop_LD_C:
      return (Lop_LD_C);
    case Lop_LD_UC2:
      return (Lop_LD_UC2);
    case Lop_LD_C2:
      return (Lop_LD_C2);
    case Lop_LD_I:
      return (Lop_LD_I);
    case Lop_LD_UI:
      return (Lop_LD_UI);
    case Lop_LD_Q:
      return (Lop_LD_Q);
    case Lop_LD_F:
      return (Lop_LD_F);
    case Lop_LD_F2:
      return (Lop_LD_F2);

    case Lop_LD_PRE_UC:
      return (Lop_LD_UC);
    case Lop_LD_PRE_C:
      return (Lop_LD_C);
    case Lop_LD_PRE_UC2:
      return (Lop_LD_UC2);
    case Lop_LD_PRE_C2:
      return (Lop_LD_C2);
    case Lop_LD_PRE_I:
      return (Lop_LD_I);
    case Lop_LD_PRE_UI:
      return (Lop_LD_UI);
    case Lop_LD_PRE_Q:
      return (Lop_LD_Q);
    case Lop_LD_PRE_F:
      return (Lop_LD_F);
    case Lop_LD_PRE_F2:
      return (Lop_LD_F2);

    case Lop_LD_POST_UC:
      return (Lop_LD_UC);
    case Lop_LD_POST_C:
      return (Lop_LD_C);
    case Lop_LD_POST_UC2:
      return (Lop_LD_UC2);
    case Lop_LD_POST_C2:
      return (Lop_LD_C2);
    case Lop_LD_POST_I:
      return (Lop_LD_I);
    case Lop_LD_POST_UI:
      return (Lop_LD_UI);
    case Lop_LD_POST_Q:
      return (Lop_LD_Q);
    case Lop_LD_POST_F:
      return (Lop_LD_F);
    case Lop_LD_POST_F2:
      return (Lop_LD_F2);

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

    case Lop_ST_PRE_C:
      return (Lop_ST_C);
    case Lop_ST_PRE_C2:
      return (Lop_ST_C2);
    case Lop_ST_PRE_I:
      return (Lop_ST_I);
    case Lop_ST_PRE_Q:
      return (Lop_ST_Q);
    case Lop_ST_PRE_F:
      return (Lop_ST_F);
    case Lop_ST_PRE_F2:
      return (Lop_ST_F2);

    case Lop_ST_POST_C:
      return (Lop_ST_C);
    case Lop_ST_POST_C2:
      return (Lop_ST_C2);
    case Lop_ST_POST_I:
      return (Lop_ST_I);
    case Lop_ST_POST_Q:
      return (Lop_ST_Q);
    case Lop_ST_POST_F:
      return (Lop_ST_F);
    case Lop_ST_POST_F2:
      return (Lop_ST_F2);

    default:
      L_punt ("L_base_memory_opcode: illegal opcode");
      return (0);
    }
}

/*
 *      if u_static is >= 1, must first call L_compute_static_cb_weight()
 *      from l_loop.c.
 *
 *      Note if you have hyperblocks, this doesnt take predicate into account
 *      for computing weight.  Thus weight here represents the number of times
 *      the instructions is fetched, and it is executed some fraction of that.
 *
 *      This routine can now be used to approximate the branch direction
 *      using a common static weight for all fall-thru paths in a superblock.
 *      Thus, if u_static is > 40, then it is assumed to be the fall thru
 *      weight for the branch.
 *
 *      NOTE: the static fallthru weight is ignored if u_dynamic is
 *      specified since this would not make much sense!
 */
void
L_compute_oper_weight (L_Func * fn, int u_static, int u_dynamic)
{
  L_Cb *cb;
  L_Oper *oper;
  L_Flow *flow;
  double dweight, sweight;
  double fallthru_weight = 1.0;

  if ((!u_static) && (!u_dynamic))
    L_punt ("L_compute_oper_weight: must have one of input flags set");

  if ((!u_dynamic) && (u_static > 1))
    fallthru_weight = u_static / 100.0;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      dweight = 0.0;
      sweight = 0.0;

      if (u_dynamic)
        dweight = cb->weight;   /* profile weight */

      if (u_static)
        sweight = cb->weight2;  /* static weight */

      flow = cb->dest_flow;
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
        {
          oper->weight = dweight + sweight;

          if (oper->weight < 0.0)
            oper->weight = 0.0;

          if (L_is_control_oper (oper))
            {
              if (u_dynamic)
                {
                  dweight -= flow->weight;
                  flow = flow->next_flow;
                }
              else if (u_static > 1)
                {
                  sweight = sweight * fallthru_weight;
                }
            }
        }
    }
  return;
}

/*
 *    if u_static is 1, must first call L_compute_static_cb_weight() (l_loop.c)
 *
 *    Note this is the same as the above function, but the predicate is
 *    accounted for, so this function gives you the number of times the
 *    instruction is truly executed.
 */
void
L_compute_oper_exec_weight (L_Func * fn, int u_static, int u_dynamic)
{
  L_Cb *cb;
  L_Oper *oper;
  L_Flow *flow;
  L_Attr *attr;
  double dweight, sweight, ratio;

  if ((!u_static) && (!u_dynamic))
    L_punt ("L_compute_oper_weight: must have one of input flags set");

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      dweight = 0.0;
      sweight = 0.0;

      if (u_dynamic)
        dweight = cb->weight;   /* profile weight */

      if (u_static)
        sweight = cb->weight2;  /* static weight */

      flow = cb->dest_flow;

      if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK))
        {                       /* hyperblock */
          for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
            {
              if ((attr = L_find_attr (oper->attr, "wgt")))
                ratio = ((double) attr->field[0]->value.i) /
                  ((double) L_WEIGHT_VALUE);
              else
                ratio = 1.0;
              oper->weight = ratio * dweight + sweight;
              if ((u_dynamic) && (L_is_control_oper (oper)))
                {
                  dweight -= flow->weight;
                  flow = flow->next_flow;
                }
            }
        }
      else
        {                       /* non-hyperblock */
          for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
            {
              oper->weight = dweight + sweight;
              if ((u_dynamic) && (L_is_control_oper (oper)))
                {
                  dweight -= flow->weight;
                  flow = flow->next_flow;
                }
            }
        }
    }
  return;
}

double
L_find_taken_prob (L_Cb *cb, L_Oper *br)
{
  L_Flow *fl;
  L_Oper *op;

  double path_weight, prob;

  path_weight = cb->weight;

  if (!cb->weight)
    return 0.0;

  for (op = cb->first_op, fl = cb->dest_flow; op && (op != br);
       op = op->next_op)
    if (L_is_control_oper (op))
      {
	path_weight -= fl->weight;
	fl = fl->next_flow;
      }

  if (!op)
    L_punt ("L_find_taken_prob: branch not found");

  if (!fl)
    L_punt ("L_find_taken_prob: flow not found for branch");

  if (path_weight < 0.01)
    return 0.0;

  prob = fl->weight / path_weight;

  if (prob > 1.0)
    prob = 1.0;

  if (prob < 0.0)
    prob = 0.0;

  return prob;
}


#if 0
/* JWS 20000719 This is no longer supported. */
void
L_branch_prediction (L_Func * fn)
{
  double taken_weight, fallthru_weight;
  L_Cb *cb;
  L_Oper *oper;
  L_Flow *flow;

  L_compute_oper_weight (fn, 0, 1);

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
        {
          if (L_cond_branch_opcode (oper) ||
              (L_uncond_branch_opcode (oper) && L_is_predicated (oper)))
            {
              flow = L_find_flow_for_branch (cb, oper);
              taken_weight = flow->weight;
              fallthru_weight = oper->weight - taken_weight;
              if ((taken_weight >= L_min_fs_weight) &&
                  (taken_weight > fallthru_weight))
                L_change_opcode (oper, L_fs_branch_opcode (oper));
              else
                L_change_opcode (oper, L_nonfs_branch_opcode (oper));
            }
          else if (L_subroutine_call_opcode (oper) ||
                   L_subroutine_return_opcode (oper) ||
                   L_uncond_branch_opcode (oper) ||
                   L_register_branch_opcode (oper))
            {
              if (oper->weight >= L_min_fs_weight)
                L_change_opcode (oper, L_fs_branch_opcode (oper));
              else
                L_change_opcode (oper, L_nonfs_branch_opcode (oper));
            }
        }
    }
}
#endif

int
L_find_control_op_index (L_Oper * oper)
{
  int i;
  if (!oper)
    L_punt ("L_find_control_op_index: op cannot be NIL");
  if (!L_cnt_oper)
    L_punt ("L_find_control_op_index: coloring not done");
  for (i = 0; i < L_n_cnt_oper; i++)
    {
      if (oper == L_cnt_oper[i])
        return i;
    }
  L_punt ("L_find_control_op_index: oper NOT found");
  return (0);
}

/*
 *      return ctype of dest operand, note should only be called for opcodes
 *      which have a unique result ctype, therefore should not be called
 *      predicate setting instructions.
 */
/* 01/13/03 REK This should return void for a normal store, or an int
 *              register type for a pre or post increment store.  See
 *              L_store_ctype and l_memory.c:L_opcode_ctype2 for stores. */
int
L_opcode_ctype (L_Oper * oper)
{
  if (!oper)
    L_punt ("L_opcode_ctype: oper is NULL");
  switch (oper->opc)
    {
    case Lop_NO_OP:
      return L_CTYPE_VOID;
    case Lop_JSR:
      return L_CTYPE_VOID;
    case Lop_JSR_FS:
      return L_CTYPE_VOID;
    case Lop_RTS:
      return L_CTYPE_VOID;
    case Lop_RTS_FS:
      return L_CTYPE_VOID;
    case Lop_PROLOGUE:
      return L_CTYPE_VOID;
    case Lop_EPILOGUE:
      return L_CTYPE_VOID;
    case Lop_DEFINE:
      return M_native_int_register_ctype ();
    case Lop_ALLOC:
      return M_native_int_register_ctype ();
    case Lop_JUMP:
      return L_CTYPE_VOID;
    case Lop_JUMP_FS:
      return L_CTYPE_VOID;
    case Lop_JUMP_RG:
      return L_CTYPE_VOID;
    case Lop_JUMP_RG_FS:
      return L_CTYPE_VOID;
    case Lop_BR:
      return L_CTYPE_VOID;
    case Lop_MOV:
      return M_native_int_register_ctype ();
    case Lop_MOV_F:
      return L_CTYPE_FLOAT;
    case Lop_MOV_F2:
      return L_CTYPE_DOUBLE;
    case Lop_L_MAC:
      return M_native_int_register_ctype ();
    case Lop_L_MSU:
      return M_native_int_register_ctype ();
    case Lop_ADD_SAT:
      return M_native_int_register_ctype ();
    case Lop_ADD_SAT_U:
      return M_native_int_register_ctype ();
    case Lop_SUB_SAT:
      return M_native_int_register_ctype ();
    case Lop_SUB_SAT_U:
      return M_native_int_register_ctype ();
    case Lop_MUL_SAT:
      return M_native_int_register_ctype ();
    case Lop_MUL_SAT_U:
      return M_native_int_register_ctype ();
    case Lop_SAT:
      return M_native_int_register_ctype ();
    case Lop_SAT_U:
      return M_native_int_register_ctype ();
    case Lop_ADD:
      return M_native_int_register_ctype ();
    case Lop_ADD_U:
      return M_native_int_register_ctype ();
    case Lop_SUB:
      return M_native_int_register_ctype ();
    case Lop_SUB_U:
      return M_native_int_register_ctype ();
    case Lop_MUL:
      return M_native_int_register_ctype ();
    case Lop_MUL_U:
      return M_native_int_register_ctype ();
    case Lop_DIV:
      return M_native_int_register_ctype ();
    case Lop_DIV_U:
      return M_native_int_register_ctype ();
    case Lop_REM:
      return M_native_int_register_ctype ();
    case Lop_REM_U:
      return M_native_int_register_ctype ();
    case Lop_ABS:
      return M_native_int_register_ctype ();
    case Lop_MUL_ADD:
      return M_native_int_register_ctype ();
    case Lop_MUL_ADD_U:
      return M_native_int_register_ctype ();
    case Lop_MUL_SUB:
      return M_native_int_register_ctype ();
    case Lop_MUL_SUB_U:
      return M_native_int_register_ctype ();
    case Lop_MUL_SUB_REV_U:
      return M_native_int_register_ctype ();
    case Lop_MAX:
      return M_native_int_register_ctype ();
    case Lop_MIN:
      return M_native_int_register_ctype ();
    case Lop_OR:
      return M_native_int_register_ctype ();
    case Lop_AND:
      return M_native_int_register_ctype ();
    case Lop_XOR:
      return M_native_int_register_ctype ();
    case Lop_NOR:
      return M_native_int_register_ctype ();
    case Lop_NAND:
      return M_native_int_register_ctype ();
    case Lop_NXOR:
      return M_native_int_register_ctype ();
    case Lop_OR_NOT:
      return M_native_int_register_ctype ();
    case Lop_AND_NOT:
      return M_native_int_register_ctype ();
    case Lop_OR_COMPL:
      return M_native_int_register_ctype ();
    case Lop_AND_COMPL:
      return M_native_int_register_ctype ();
    case Lop_RCMP:
      return M_native_int_register_ctype ();
    case Lop_RCMP_F:
      return M_native_int_register_ctype ();
    case Lop_LSL:
      return M_native_int_register_ctype ();
    case Lop_LSR:
      return M_native_int_register_ctype ();
    case Lop_ASR:
      return M_native_int_register_ctype ();
    case Lop_REV:
      return M_native_int_register_ctype ();
    case Lop_BIT_POS:
      return M_native_int_register_ctype ();
    case Lop_ADD_F2:
      return L_CTYPE_DOUBLE;
    case Lop_SUB_F2:
      return L_CTYPE_DOUBLE;
    case Lop_MUL_F2:
      return L_CTYPE_DOUBLE;
    case Lop_DIV_F2:
      return L_CTYPE_DOUBLE;
    case Lop_RCP_F2:
      return L_CTYPE_DOUBLE;
    case Lop_ABS_F2:
      return L_CTYPE_DOUBLE;
    case Lop_MUL_ADD_F2:
      return L_CTYPE_DOUBLE;
    case Lop_MUL_SUB_F2:
      return L_CTYPE_DOUBLE;
    case Lop_MUL_SUB_REV_F2:
      return L_CTYPE_DOUBLE;
    case Lop_SQRT_F2:
      return L_CTYPE_DOUBLE;
    case Lop_MAX_F2:
      return L_CTYPE_DOUBLE;
    case Lop_MIN_F2:
      return L_CTYPE_DOUBLE;
    case Lop_ADD_F:
      return L_CTYPE_FLOAT;
    case Lop_SUB_F:
      return L_CTYPE_FLOAT;
    case Lop_MUL_F:
      return L_CTYPE_FLOAT;
    case Lop_DIV_F:
      return L_CTYPE_FLOAT;
    case Lop_RCP_F:
      return L_CTYPE_FLOAT;
    case Lop_ABS_F:
      return L_CTYPE_FLOAT;
    case Lop_MUL_ADD_F:
      return L_CTYPE_FLOAT;
    case Lop_MUL_SUB_F:
      return L_CTYPE_FLOAT;
    case Lop_MUL_SUB_REV_F:
      return L_CTYPE_FLOAT;
    case Lop_SQRT_F:
      return L_CTYPE_FLOAT;
    case Lop_MAX_F:
      return L_CTYPE_FLOAT;
    case Lop_MIN_F:
      return L_CTYPE_FLOAT;
    case Lop_F2_I:
      return M_native_int_register_ctype ();
    case Lop_I_F2:
      return L_CTYPE_DOUBLE;
    case Lop_F_I:
      return M_native_int_register_ctype ();
    case Lop_I_F:
      return L_CTYPE_FLOAT;
    case Lop_F2_F:
      return L_CTYPE_FLOAT;
    case Lop_F_F2:
      return L_CTYPE_DOUBLE;
      /* 01/13/03 REK Changing this so that ST results in void, and pre
       *              and post result in int reg type. */
    case Lop_ST_C:
      return L_CTYPE_VOID;
    case Lop_ST_PRE_C:
      return M_native_int_register_ctype ();
    case Lop_ST_POST_C:
      return M_native_int_register_ctype ();
    case Lop_LD_C:
      return L_CTYPE_CHAR;
    case Lop_LD_PRE_C:
      return L_CTYPE_CHAR;
    case Lop_LD_POST_C:
      return L_CTYPE_UCHAR;
    case Lop_LD_UC:
      return L_CTYPE_UCHAR;
    case Lop_LD_PRE_UC:
      return L_CTYPE_UCHAR;
    case Lop_LD_POST_UC:
      return L_CTYPE_UCHAR;
      /* 01/13/03 REK Changing this so that ST results in void, and pre
       *              and post result in int reg type. */
    case Lop_ST_C2:
      return L_CTYPE_VOID;
    case Lop_ST_PRE_C2:
      return M_native_int_register_ctype ();
    case Lop_ST_POST_C2:
      return M_native_int_register_ctype ();
    case Lop_LD_C2:
      return L_CTYPE_SHORT;
    case Lop_LD_PRE_C2:
      return L_CTYPE_SHORT;
    case Lop_LD_POST_C2:
      return L_CTYPE_SHORT;
    case Lop_LD_UC2:
      return L_CTYPE_USHORT;
    case Lop_LD_PRE_UC2:
      return L_CTYPE_USHORT;
    case Lop_LD_POST_UC2:
      return L_CTYPE_USHORT;
      /* 01/13/03 REK Changing this so that ST results in void, and pre
       *              and post result in int reg type. */
    case Lop_ST_I:
      return L_CTYPE_VOID;
    case Lop_ST_PRE_I:
      return M_native_int_register_ctype ();
    case Lop_ST_POST_I:
      return M_native_int_register_ctype ();
    case Lop_LD_UI:
      return L_CTYPE_UINT;
    case Lop_LD_PRE_UI:
      return L_CTYPE_UINT;
    case Lop_LD_POST_UI:
      return L_CTYPE_UINT;
    case Lop_LD_I:
      return L_CTYPE_INT;
    case Lop_LD_PRE_I:
      return L_CTYPE_INT;
    case Lop_LD_POST_I:
      return L_CTYPE_INT;
      /* 01/13/03 REK Changing this so that ST results in void, and pre
       *              and post result in int reg type. */
    case Lop_ST_F:
      return L_CTYPE_VOID;
    case Lop_ST_PRE_F:
      return M_native_int_register_ctype ();
    case Lop_ST_POST_F:
      return M_native_int_register_ctype ();
    case Lop_LD_Q:
      return L_CTYPE_LLONG;
    case Lop_LD_F:
      return L_CTYPE_FLOAT;
    case Lop_LD_PRE_F:
      return L_CTYPE_FLOAT;
    case Lop_LD_POST_F:
      return L_CTYPE_FLOAT;
      /* 01/13/03 REK Changing this so that ST results in void, and pre
       *              and post result in int reg type. */
    case Lop_ST_F2:
      return L_CTYPE_VOID;
    case Lop_ST_PRE_F2:
      return M_native_int_register_ctype ();
    case Lop_ST_POST_F2:
      return M_native_int_register_ctype ();
    case Lop_LD_F2:
      return L_CTYPE_DOUBLE;
    case Lop_LD_PRE_F2:
      return L_CTYPE_DOUBLE;
    case Lop_LD_POST_F2:
      return L_CTYPE_DOUBLE;
    case Lop_CMP:
    case Lop_PRED_ST:
    case Lop_PRED_ST_BLK:
    case Lop_PRED_LD:
    case Lop_PRED_LD_BLK:
      return L_CTYPE_PREDICATE;
      /* 01/13/03 REK Changing this so that ST results in void, and pre
       *              and post result in int reg type. */
    case Lop_ST_Q:
      return L_CTYPE_VOID;
    case Lop_ST_PRE_Q:
      return M_native_int_register_ctype ();
    case Lop_ST_POST_Q:
      return M_native_int_register_ctype ();
    case Lop_EXTRACT_C:
      return M_native_int_register_ctype ();
    case Lop_EXTRACT_C2:
      return M_native_int_register_ctype ();
    case Lop_EXTRACT:
      return M_native_int_register_ctype ();
    case Lop_EXTRACT_U:
      return M_native_int_register_ctype ();
    case Lop_DEPOSIT:
      return M_native_int_register_ctype ();

    case Lop_FETCH_AND_ADD:
      return M_native_int_register_ctype ();
    case Lop_FETCH_AND_OR:
      return M_native_int_register_ctype ();
    case Lop_FETCH_AND_AND:
      return M_native_int_register_ctype ();
    case Lop_FETCH_AND_ST:
      return M_native_int_register_ctype ();
    case Lop_FETCH_AND_COND_ST:
      return M_native_int_register_ctype ();
    case Lop_ADVANCE:
      return L_CTYPE_VOID;
    case Lop_AWAIT:
      return L_CTYPE_VOID;
    case Lop_MUTEX_B:
      return L_CTYPE_VOID;
    case Lop_MUTEX_E:
      return L_CTYPE_VOID;
    case Lop_CO_PROC:
      return M_native_int_register_ctype ();
    case Lop_CHECK:
    case Lop_CHECK_ALAT:     
      return L_CTYPE_VOID;
    case Lop_ADD_CARRY:
    case Lop_ADD_CARRY_U:
    case Lop_SUB_CARRY:
    case Lop_SUB_CARRY_U:
    case Lop_MUL_WIDE:
    case Lop_MUL_WIDE_U:
      return M_native_int_register_ctype ();

    case Lop_CONFIRM:
      return L_CTYPE_VOID;
    case Lop_CMOV:
      return M_native_int_register_ctype ();
    case Lop_CMOV_COM:
      return M_native_int_register_ctype ();
    case Lop_CMOV_F:
      return L_CTYPE_FLOAT;
    case Lop_CMOV_COM_F:
      return L_CTYPE_FLOAT;
    case Lop_CMOV_F2:
      return L_CTYPE_DOUBLE;
    case Lop_CMOV_COM_F2:
      return L_CTYPE_DOUBLE;
    case Lop_SELECT:
      return M_native_int_register_ctype ();
    case Lop_SELECT_F:
      return L_CTYPE_FLOAT;
    case Lop_SELECT_F2:
      return L_CTYPE_DOUBLE;
    case Lop_PREF_LD:
      return M_native_int_register_ctype ();
    case Lop_JSR_ND:
      return L_CTYPE_VOID;
    case Lop_EXPAND:
      return M_native_int_register_ctype ();
    case Lop_MEM_COPY:
      return M_native_int_register_ctype ();
    case Lop_MEM_COPY_BACK:
      return M_native_int_register_ctype ();
    case Lop_MEM_COPY_CHECK:
      return M_native_int_register_ctype ();
    case Lop_MEM_COPY_RESET:
      return M_native_int_register_ctype ();
    case Lop_MEM_COPY_SETUP:
      return M_native_int_register_ctype ();
    case Lop_MEM_COPY_TAG:
      return M_native_int_register_ctype ();
    case Lop_SIM_DIR:
      return L_CTYPE_VOID;
    case Lop_BOUNDARY:
      return L_CTYPE_VOID;
    case Lop_BIT_EXTRACT:
      return M_native_int_register_ctype ();
    case Lop_BIT_DEPOSIT:
      return M_native_int_register_ctype ();
    case Lop_SXT_C:
      return L_CTYPE_CHAR;
    case Lop_SXT_C2:
      return L_CTYPE_SHORT;
    case Lop_SXT_I:
      return L_CTYPE_INT;
    case Lop_ZXT_C:
      return L_CTYPE_UCHAR;
    case Lop_ZXT_C2:
      return L_CTYPE_USHORT;
    case Lop_ZXT_I:
      return L_CTYPE_UINT;
    default:
      L_punt ("L_opcode_ctype: illegal opcode");
      return (0);
    }
}

/* 01/14/03 REK L_store_ctype returns the ctype stored by a store opcode. */
int
L_store_ctype (L_Oper * oper)
{
  if (!oper)
    L_punt ("L_store_ctype: oper is NULL");
  switch (oper->opc)
    {
    case Lop_ST_C:
      return L_CTYPE_CHAR;
    case Lop_ST_PRE_C:
      return L_CTYPE_CHAR;
    case Lop_ST_POST_C:
      return L_CTYPE_CHAR;
    case Lop_ST_C2:
      return L_CTYPE_SHORT;
    case Lop_ST_PRE_C2:
      return L_CTYPE_SHORT;
    case Lop_ST_POST_C2:
      return L_CTYPE_SHORT;
    case Lop_ST_I:
      return L_CTYPE_INT;
    case Lop_ST_PRE_I:
      return L_CTYPE_INT;
    case Lop_ST_POST_I:
      return L_CTYPE_INT;
    case Lop_ST_F:
      return L_CTYPE_FLOAT;
    case Lop_ST_PRE_F:
      return L_CTYPE_FLOAT;
    case Lop_ST_POST_F:
      return L_CTYPE_FLOAT;
    case Lop_ST_F2:
      return L_CTYPE_DOUBLE;
    case Lop_ST_PRE_F2:
      return L_CTYPE_DOUBLE;
    case Lop_ST_POST_F2:
      return L_CTYPE_DOUBLE;
    case Lop_ST_Q:
      return L_CTYPE_LONG;
    case Lop_ST_PRE_Q:
      return L_CTYPE_LONG;
    case Lop_ST_POST_Q:
      return L_CTYPE_LONG;
    default:
      L_punt ("L_store_ctype: illegal opcode");
      return (0);
    }
}

int
L_num_src_operand (L_Oper * oper)
{
  int count, i;
  count = 0;

  if (!oper)
    return 0;

  for (i = 0; i < L_max_src_operand; i++)
    if (oper->src[i])
      count++;

  return count;
}

int
L_num_dest_operand (L_Oper * oper)
{
  int count, i;
  count = 0;

  if (!oper)
    return 0;

  for (i = 0; i < L_max_dest_operand; i++)
    if (oper->dest[i])
      count++;

  return count;
}

void
L_clear_all_reserved_oper_flags (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *oper;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
      oper->flags = L_CLR_BIT_FLAG (oper->flags, (L_OPER_RESERVED_TEMP1 |
						  L_OPER_RESERVED_TEMP2 |
						  L_OPER_RESERVED_TEMP3 |
						  L_OPER_RESERVED_TEMP4 |
						  L_OPER_RESERVED_TEMP5 |
						  L_OPER_RESERVED_TEMP6 |
						  L_OPER_RESERVED_TEMP7 |
						  L_OPER_RESERVED_TEMP8));
  return;
}

int
L_is_unsigned_ld (L_Oper * opA)
{
  switch (opA->opc)
    {
    case Lop_LD_UC:
    case Lop_LD_UC2:
    case Lop_LD_UI:
      return (1);

    case Lop_LD_C:
    case Lop_LD_C2:
    case Lop_LD_I:
      return (0);

    default:
      L_punt ("L_is_unsigned_ld: op %d is not a known ld type", opA->id);
      return 0;
    }

}

/*===========================================================================*/
/*
 *      L_Cb functions
 */
/*===========================================================================*/

/*
 *      If cb->last not a branch or is cond br, return next block in layout,
 *      else return -1.
 */
L_Cb *
L_fall_thru_path (L_Cb * cb)
{
  if (L_uncond_branch (cb->last_op) ||
      L_register_branch_opcode (cb->last_op))
    return NULL;
  return (cb->next_cb);
}

L_Cb *
L_create_cb (double weight)
{
  L_Cb *cb;

  if (L_cb_hash_tbl_find (L_fn->cb_hash_tbl, ++L_fn->max_cb_id))
    L_punt ("L_create_cb: max_cb_id count in error");
  cb = L_new_cb (L_fn->max_cb_id);
  cb->weight = weight;
  return cb;
}


/* DML - Create new cb and insert it after cb.  If use_existing_cb = 1,
   and fall thru cb has single predecessor, just return pointer to
   fall thru cb.  If use_existing_cb = 0, always create new cb. */
L_Cb *
L_create_cb_at_fall_thru_path (L_Cb * cb, int use_existing_cb)
{
  int need_jump;                /* flag to indicate if cb ends with a jump */
  L_Oper *jump_op;
  L_Cb *fallthru_cb, *new_cb;
  L_Flow *flow, *new_flow, *src_flow;

  need_jump = 0;

  /* find fall thru cb */
  if (L_uncond_branch (cb->last_op))
    {
      fallthru_cb = cb->last_op->src[0]->value.cb;
      if (fallthru_cb == cb)
        L_punt ("L_create_cb_at_fall_thru_path: CB has no proper fallthru");
      need_jump = 1;
    }
  else
    {
      fallthru_cb = cb->next_cb;
    }

  if (!use_existing_cb || !L_single_predecessor_cb (fallthru_cb))
    {
      /* Can't use existing fallthru cb; must make a new one */
      flow = L_find_last_flow (cb->dest_flow);
      new_cb = L_create_cb (flow->weight);
      L_insert_cb_after (L_fn, cb, new_cb);
      
      cb->flags =
	L_CLR_BIT_FLAG (cb->flags, L_CB_HYPERBLOCK_NO_FALLTHRU);
      
      /* Insert jump in new block to original fallthru_cb */
      if (need_jump)
        {
          if (new_cb->weight > L_min_fs_weight)
            jump_op = L_create_new_op (Lop_JUMP_FS);
          else
            jump_op = L_create_new_op (Lop_JUMP);
          jump_op->src[0] = L_new_cb_operand (fallthru_cb);
          L_insert_oper_after (new_cb, new_cb->last_op, jump_op);
        }

      /* create new dest and src flows for new_cb */
      new_flow = L_new_flow (flow->cc, new_cb, fallthru_cb, new_cb->weight);
      new_cb->dest_flow = L_concat_flow (new_cb->dest_flow, new_flow);
      new_flow = L_new_flow (0, cb, new_cb, new_cb->weight);
      new_cb->src_flow = L_concat_flow (new_cb->src_flow, new_flow);
      
      /* update transfer to fall thru cb in cb */
      if (need_jump)
	L_delete_oper (cb, cb->last_op);
      
      /* update src flow of fallthru_cb */
      src_flow = L_find_matching_flow (fallthru_cb->src_flow, flow);
      src_flow->src_cb = new_cb;

      /* update dst flow of cb */
      flow->dst_cb = new_cb;
      flow->cc = 0;

      fallthru_cb = new_cb;
    }
  return (fallthru_cb);
}


/* 5/12/98 - Chad Lester
 *
 * L_find_complement_flow(), first counts how many prev_flow's
 * match the flow itself.  It then uses that count to find 
 * the n'th matching flow in the list. 
 */

L_Flow *
L_find_complement_flow (L_Flow * list, L_Flow * flow)
{
  L_Flow *f;
  int count = 0;
  for (f = flow; f != NULL; f = f->prev_flow)
    {
      if (f->cc != flow->cc)
        continue;
      if (f->src_cb != flow->src_cb)
        continue;
      if (f->dst_cb != flow->dst_cb)
        continue;
      if ((f->weight < (flow->weight - ZERO_EQUIVALENT)) ||
          (f->weight > (flow->weight + ZERO_EQUIVALENT)))
        continue;
      count++;
    }
  for (f = list; f != NULL; f = f->next_flow)
    {
      if (f->cc != flow->cc)
        continue;
      if (f->src_cb != flow->src_cb)
        continue;
      if (f->dst_cb != flow->dst_cb)
        continue;
      if ((f->weight < (flow->weight - ZERO_EQUIVALENT)) ||
          (f->weight > (flow->weight + ZERO_EQUIVALENT)))
        continue;
      if (!--count)
        return f;
    }
  for (f = list; f != NULL; f = f->next_flow)
    {
      fprintf (stderr, "\t%d: cb%d->cb%d (%f)\n",
               f->cc, f->src_cb->id, f->dst_cb->id, f->weight);
    }
  fprintf (stderr, "Flow %d: cb%d->cb%d (%f) is not in list\n",
           flow->cc, flow->src_cb->id, flow->dst_cb->id, flow->weight);
  L_punt ("L_find_compliment_flow: cannot find flow arc");
  return (NULL);
}


/*
 *      Copy all operations in cb old_id into cb new_id
 */
void
L_copy_block_contents (L_Cb * old, L_Cb * new_cb)
{
  L_Oper *oper, *new_oper;
  for (oper = old->first_op; oper != NULL; oper = oper->next_op)
    {
      new_oper = L_copy_operation (oper);
      L_insert_oper_after (new_cb, new_cb->last_op, new_oper);
    }
}


/*
 *      Copy all operations in cb old to cb new_cb starting from first_op
 */
void
L_copy_block_contents_from_oper (L_Cb * old, L_Cb * new_cb, L_Oper * first_op)
{
  L_Oper *oper, *new_oper;
  for (oper = first_op; oper != NULL; oper = oper->next_op)
    {
      new_oper = L_copy_operation (oper);
      L_insert_oper_after (new_cb, new_cb->last_op, new_oper);
    }
}


int
L_cb_size (L_Cb * cb)
{
  int count = 0;
  L_Oper *oper;
  for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
    count++;
  return count;
}

int
L_num_dst_cb (L_Cb * cb)
{
  int num;
  L_Flow *flow;

  num = 0;
  for (flow = cb->dest_flow; flow; flow = flow->next_flow)
    num++;

  return num;
}

int
L_num_src_cb (L_Cb * cb)
{
  int num;
  L_Flow *flow;

  num = 0;
  for (flow = cb->src_flow; flow; flow = flow->next_flow)
    num++;

  return num;
}

L_Oper *
L_find_last_branch (L_Cb * cb)
{
  L_Oper *oper;

  for (oper = cb->last_op; oper; oper = oper->prev_op)
    if (L_is_control_oper (oper))
      return oper;

  return NULL;
}

void
L_clear_all_reserved_cb_flags (L_Func * fn)
{
  L_Cb *cb;

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    cb->flags = L_CLR_BIT_FLAG (cb->flags, (L_CB_RESERVED_TEMP1 |
					    L_CB_RESERVED_TEMP2 |
					    L_CB_RESERVED_TEMP3 |
					    L_CB_RESERVED_TEMP4 |
					    L_CB_RESERVED_TEMP5 |
					    L_CB_RESERVED_TEMP6 |
					    L_CB_RESERVED_TEMP7 |
					    L_CB_RESERVED_TEMP8));
}

/*===========================================================================*/
/*
 *      L_Loop functions
 */
/*===========================================================================*/

L_Loop *
L_find_loop (L_Func * fn, int id)
{
  L_Loop *loop;

  for (loop = fn->first_loop; loop != NULL; loop = loop->next_loop)
    if (loop->id == id)
      return (loop);

  return (NULL);
}

/*===========================================================================*/
/*
 *      L_Inner_Loop functions
 */
/*===========================================================================*/

L_Inner_Loop *
L_find_inner_loop (L_Func * fn, int id)
{
  L_Inner_Loop *loop;

  for (loop = fn->first_inner_loop; loop != NULL;
       loop = loop->next_inner_loop)
    if (loop->id == id)
      return (loop);

  return (NULL);
}

/*===========================================================================*/
/*
 *      L_Datalist functions
 */
/*===========================================================================*/

/* Merge l2 into the end of l1 */
void
L_merge_datalists (L_Datalist * l1, L_Datalist * l2)
{
  /* Assume the lists structs themselves have been created when 
     this is called */
  if (!l1 || !l2)
    L_punt ("L_merge_datalists: l1 and l2 must be real datalists!!");

  /* Handle 4 cases */
  if (l1->first_element == NULL)
    {
      /* both lists have 0 elements: do nothing */
      if (!l2->first_element)
        {
          return;
        }
      /* l1 is empty, l2 is not: move l2 contents into l1 */
      else
        {
          l1->first_element = l2->first_element;
          l1->last_element = l2->last_element;
          l2->first_element = NULL;
          l2->last_element = NULL;
        }
    }
  else
    {
      /* l1 is not, l2 is empty: do nothing! */
      if (!l2->first_element)
        {
          return;
        }
      /* both are not empty: merge l2 into l1 */
      else
        {
          l1->last_element->next_element = l2->first_element;
          l1->last_element = l2->last_element;
          l2->first_element = NULL;
          l2->last_element = NULL;
        }
    }
}

/*======================================================================*/
/*
 *      L_Func functions
 */
/*======================================================================*/

/*
 * L_scale_fn_weight
 * ----------------------------------------------------------------------
 * Scale weights of all cbs and flows in a function by a constant factor.
 */

void
L_scale_fn_weight (L_Func * fn, double scale_factor)
{
  L_Cb *cb;

  fn->weight *= scale_factor;
  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      L_scale_flow_weights (cb->dest_flow, scale_factor);
      L_scale_flow_weights (cb->src_flow, scale_factor);
      cb->weight *= scale_factor;
    }
}


/*
 * L_scale_cb_weight
 * ----------------------------------------------------------------------
 * Scale the weight of a cb and all outgoing flows by a constant factor.
 */

void
L_scale_cb_weight (L_Cb *cb, double scale_factor)
{
  L_Flow *flo, *fli;
  L_Cb *dcb;

  cb->weight = cb->weight * scale_factor;

  for (flo = cb->dest_flow; flo; flo = flo->next_flow)
    {
      dcb = flo->dst_cb;

      fli = L_find_matching_flow (dcb->src_flow, flo);

      flo->weight = flo->weight * scale_factor;
      fli->weight = fli->weight * scale_factor;
    }

  return;
}

/*===========================================================================*/
/*
 *      L_Expression functions
 */
/*===========================================================================*/


static inline int
L_PCE_get_operand_token (L_Operand * operand)
{
  int token;

  if (operand)
    {
      token = (int) operand->type;
      token = token << 3;

      if (L_is_reg_direct(operand->type))
	token += ((int) operand->value.r);
      else if (L_is_macro_direct(operand->type))
	token += ((int) operand->value.mac);
      else if (L_is_int_constant(operand))
	token += ((int) operand->value.i);
      else if (L_is_string_direct(operand->type))
	token += (((int) *(operand->value.s) << 2) +
		  (int) *((operand->value.s)+1));
      else if (L_is_label_direct(operand->type))
	token += (((int) *(operand->value.l) << 2) +
		  (int) *((operand->value.s)+1));
    }
  else
    token = 0;

  return token;
}

int
L_generate_expression_token_from_oper (L_Oper *oper)
{
  int token;
  L_Operand *operand;

  token = oper->opc;
  token = token << 3;

  operand = oper->src[0];
  token += L_PCE_get_operand_token (operand);
  token = token << 6;

  operand = oper->src[1];
  token += L_PCE_get_operand_token (operand);

  return token;
}


int
L_generate_assignment_token_from_oper (L_Oper * oper, int short_flag)
{
  int token;
  L_Operand * operand;

  token = oper->opc;
  token = token << 3;

  operand = oper->src[0];
  token += L_PCE_get_operand_token (operand);
  token = token << 6;

  operand = oper->src[1];
  token += L_PCE_get_operand_token (operand);
  token = token << 6;

  if (!short_flag && L_store_opcode (oper))
    {
      operand = oper->src[2];
      token += L_PCE_get_operand_token (operand);
    }
  else
    {
      operand = oper->dest[0];
      token += L_PCE_get_operand_token (operand);
    }

  return token;
}


/* SER 07/16/03: This function generates a token for an oper but with a
   different opcode.
 */
int
L_generate_expression_token_different_opcode (L_Oper *oper, int opc)
{
  int token;
  L_Operand *operand;

  token = opc;
  token = token << 3;

  operand = oper->src[0];
  token += L_PCE_get_operand_token (operand);
  token = token << 6;

  operand = oper->src[1];
  token += L_PCE_get_operand_token (operand);

  return token;
}



L_Oper *
L_generate_oper_from_expression_index (int expression_index)
{
  L_Expression_Hash_Entry *entry;
  L_Expression *expression;
  L_Oper *new_op;
  int i;

  entry = L_expression_hash_tbl_find_entry (L_fn->expression_index_hash_tbl, 
					    expression_index);
  expression = entry->expression;

  new_op = L_create_new_op(expression->opc);

  for (i = 0; i < L_max_src_operand; i++)
    if (expression->src[i] != NULL)
      new_op->src[i] = L_copy_operand (expression->src[i]);
    else
      break;

  for (i = 0; i < L_MAX_CMPLTR; i++)
    new_op->com[i] = expression->com[i];

  if (!L_general_store_opcode (expression))
    new_op->dest[0] =
      L_create_operand_for_expression_index (expression_index); 

  return new_op;
}


L_Oper *
L_generate_oper_from_assignment_index (int assignment_index, int pred_guard)
{
  L_Expression_Hash_Entry *entry;
  L_Expression * assignment;
  L_Oper * new_op;
  int i;

  entry = L_expression_hash_tbl_find_entry (L_fn->expression_index_hash_tbl,
					    assignment_index);
  assignment = entry->expression;

  new_op = L_create_new_op (assignment->opc);

  for (i = 0; i < L_max_src_operand; i++)
    if (assignment->src[i] != NULL)
      new_op->src[i] = L_copy_operand (assignment->src[i]);

  for (i = 0; i < L_max_dest_operand; i++)
    if (assignment->dest[i] != NULL)
      new_op->dest[i] = L_copy_operand (assignment->dest[i]);

  if (L_store_opcode (assignment) && !(assignment->src[2]))
    new_op->src[2] = L_create_operand_for_expression_index (assignment_index);

  for (i = 0; i < L_MAX_CMPLTR; i++)
    new_op->com[i] = assignment->com[i];

  if (pred_guard)
    new_op->pred[0] = L_create_pred_for_assignment_index (assignment_index);

  return new_op;
}


L_Oper *
L_generate_complement_load_oper_from_assignment_index (int expression_index)
{
  L_Expression_Hash_Entry *entry;
  L_Expression *expression;
  L_Oper *new_op;
  int i, opc;

  entry = L_expression_hash_tbl_find_entry (L_fn->expression_index_hash_tbl,
					    expression_index);
  expression = entry->expression;

  switch (expression->opc)
    {
    case Lop_ST_C:
      opc = Lop_LD_C;
      break;
    case Lop_ST_C2:
      opc = Lop_LD_C2;
      break;
    case Lop_ST_I:
      opc = Lop_LD_I;
      break;
    case Lop_ST_Q:
      opc = Lop_LD_Q;
      break;
    case Lop_ST_F:
      opc = Lop_LD_F;
      break;
    case Lop_ST_F2:
      opc = Lop_LD_F2;
      break;
    default:
      L_punt ("Invalid opcode in L_generate_complement_load_oper_from_"
	      "assignment_index.");
      return NULL;
    }

  new_op = L_create_new_op (opc);

  for (i = 0; i < L_max_src_operand; i++)
    if (expression->src[i] != NULL)
      new_op->src[i] = L_copy_operand (expression->src[i]);
    else
      break;

  new_op->dest[0] = L_create_operand_for_expression_index (expression_index);

  return new_op;
}


L_Operand *
L_create_operand_for_expression_index (int expression_index)
{
  L_Expression_Hash_Entry * entry;
  L_Expression * expression;
  L_Operand * new_operand;

  entry = L_expression_hash_tbl_find_entry (L_fn->expression_index_hash_tbl,
                                           expression_index);
  expression = entry->expression;

  if (expression->reg_id == 0)
    expression->reg_id = ++L_fn->max_reg_id;

  new_operand = L_new_register_operand (expression->reg_id, 
		                        (int) expression->dest_ctype, 
                                        L_PTYPE_NULL);
  return new_operand;
}


L_Operand *
L_create_pred_for_assignment_index (int assignment_index)
{
  L_Expression_Hash_Entry * entry;
  L_Expression * assignment;
  L_Operand * new_operand;

  entry = L_expression_hash_tbl_find_entry (L_fn->expression_index_hash_tbl,
                                           assignment_index);
  assignment = entry->expression;

  if (assignment->pred_id == 0)
    assignment->pred_id = ++L_fn->max_reg_id;

  new_operand = L_new_register_operand (assignment->pred_id, 
		                        L_CTYPE_PREDICATE, L_PTYPE_NULL);
  return new_operand;
}


void
L_clear_expression_reg_ids (L_Func * fn)
{
  L_Expression_Hash_Entry *entry;
  int i;

  for (i = 1; i <= fn->n_expression; i++)
    {
      entry = L_expression_hash_tbl_find_entry (fn->expression_index_hash_tbl,
                                                i);
      entry->expression->reg_id = 0;
      entry->expression->pred_id = 0;
      entry->expression->weight = 0.0;
    }
}


/* SER: once we know which expressions we want to avoid, we can construct a set 
 * of flags and use this to construct an ignore set in Lopti.
 */
Set
L_get_expression_subset (int flags)
{
  L_Expression_Hash_Entry *entry;
  L_Expression *expression;
  Set return_set;
  int i;

  /* sort flags here */

  return_set = NULL;

  for (i = 1; i <= L_fn->n_expression; i++)
    {
      entry = 
	L_expression_hash_tbl_find_entry (L_fn->expression_index_hash_tbl, i);
      expression = entry->expression;

      if (flags & L_EXPRESSION_SINGLE_SOURCE)
	{
	  if (expression->src[1] == NULL)
	    return_set = Set_add (return_set, i);
	}
      else if (flags & L_EXPRESSION_MOVE)
	{
	  if (L_move_opcode (expression))
	    return_set = Set_add (return_set, i);
	}
      else if (flags & L_EXPRESSION_MOVE_NUM_CONST)
	{
	  if (L_move_opcode (expression))
	    if ((expression->src[0])->type == L_OPERAND_IMMED)
	      return_set = Set_add (return_set, i);
	}

      if (flags & L_EXPRESSION_MEMORY)
	if (L_general_load_opcode (expression) || 
	    L_general_store_opcode (expression))
          {
            /* fprintf (stderr, "Adding expression %d to ignore set.\n", i); */
	    return_set = Set_add (return_set, i);
          }
    }
  return (return_set);
}


/* 6/4/03 SER: Function to initialize function live-ins, in case a store
 * operation trying to store the value. */
void
L_initialize_function_live_ins (L_Func * fn)
{
  L_Cb * first_cb, * search_cb;
  L_Oper * new_op, * use_op;
  L_Operand * found_operand, * operand;
  int * registers;
  Set live_ins;
  int i, j, cnt_livein, reg_index;


  first_cb = fn->first_cb;
  live_ins = L_get_cb_IN_set (first_cb);
  cnt_livein = Set_size (live_ins);
  registers = (int *) malloc (sizeof (int) * cnt_livein);
  Set_2array (live_ins, registers);

  for (i = 0; i < cnt_livein; i++)
    {
      if ((registers[i])%2)
	continue;
      reg_index = registers[i] >> 1;
      fprintf (stderr, "Inserting initialization for live-in register %d, "
	       "function %s.\n", reg_index, fn->name);

      found_operand = NULL;
      for (search_cb = first_cb; search_cb; search_cb = search_cb->next_cb)
	{
	  for (use_op = search_cb->first_op; use_op; use_op = use_op->next_op)
	    {
	      for (j = 0; j < L_max_dest_operand; j++)
		if (L_is_register (operand = use_op->dest[j]))
		  if (operand->value.r == reg_index)
		    {
		      found_operand = operand;
		      break;
		    }
	      for (j = 0; j < L_max_src_operand; j++)
		if (L_is_register (operand = use_op->src[j]))
		  if (operand->value.r == reg_index)
		    {
		      found_operand = operand;
		      break;
		    }
	      if (found_operand)
		break;
	    }
	  if (found_operand)
	    break;
	}
      if (!found_operand)
	L_punt ("L_initialize_function_live_ins: no operand found for live-in "
		"register index.");

      switch (found_operand->ctype)
	{
	  case L_CTYPE_INT:
	  case L_CTYPE_UINT:
	  case L_CTYPE_LLONG:
	  case L_CTYPE_ULLONG:
	    new_op = L_create_new_op (Lop_MOV);
	    new_op->src[0] = L_new_gen_int_operand (0);
	    break;

	  case L_CTYPE_FLOAT:
	    new_op = L_create_new_op (Lop_MOV_F);
	    new_op->src[0] = L_new_float_operand (0.0);
	    break;

	  case L_CTYPE_DOUBLE:
	    new_op = L_create_new_op (Lop_MOV_F2);
	    new_op->src[0] = L_new_double_operand (0.0);
	    break;

	  default:
	    L_punt ("L_initialize_function_live_ins: illegal opcode type.");
	    return;
	}
      new_op->dest[0] = L_copy_operand (found_operand);

      for (use_op = first_cb->first_op; use_op; use_op = use_op->next_op)
	{
 	  if (L_is_src_operand (found_operand, use_op))
	    break;
	  if (L_is_control_oper (use_op))
	    break;
	}
      if (use_op)
	L_insert_oper_before (first_cb, use_op, new_op);
      else
	L_insert_oper_after (first_cb, first_cb->last_op, new_op);
    }

  free (registers);
}


