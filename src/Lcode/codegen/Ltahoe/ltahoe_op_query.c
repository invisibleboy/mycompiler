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
/* 9/16/02 Robert Kidd
 * This file defines the Tmdes oper queries that Ltahoe uses.  They have
 * been moved from Tmdes to Ltahoe to eliminate Tmdes and make Ltahoe easier
 * to maintain.
 */
/* 09/25/02 REK Updating to only use unsigned compare operations for 
 *              non-parallel compare types (UNC and NONE).
 */

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "ltahoe_op_query.h"
#include "ltahoe_completers.h"

int LT_SYLLABLE_TYPE_TABLE[NUMBER_OF_REAL_TEMPLATES][3] = {
  /* MII     */ {M_SYLL, I_SYLL, I_SYLL},
  /* MISI    */ {M_SYLL, I_SYLL, I_SYLL},
  /* MLI     */ {M_SYLL, L_SYLL, I_SYLL},
  /* RSVD_T1 */ {INVALID_SYLL, INVALID_SYLL, INVALID_SYLL},
  /* MMI     */ {M_SYLL, M_SYLL, I_SYLL},
  /* MSMI    */ {M_SYLL, M_SYLL, I_SYLL},
  /* MFI     */ {M_SYLL, F_SYLL, I_SYLL},
  /* MMF     */ {M_SYLL, M_SYLL, F_SYLL},
  /* MIB     */ {M_SYLL, I_SYLL, B_SYLL},
  /* MBB     */ {M_SYLL, B_SYLL, B_SYLL},
  /* RSVD_T3 */ {INVALID_SYLL, INVALID_SYLL, INVALID_SYLL},
  /* BBB     */ {B_SYLL, B_SYLL, B_SYLL},
  /* MMB     */ {M_SYLL, M_SYLL, B_SYLL},
  /* RSVD_T4 */ {INVALID_SYLL, INVALID_SYLL, INVALID_SYLL},
  /* MFB     */ {M_SYLL, F_SYLL, B_SYLL},
  /* RSVD_T5 */ {INVALID_SYLL, INVALID_SYLL, INVALID_SYLL},
};

char *LT_TEMPLATE_NAME[NUMBER_OF_REAL_TEMPLATES] = {
  /* MII     */ "mii",
  /* MISI    */ "mii",
  /* MLI     */ "mlx",
  /* RSVD_T1 */ "???",
  /* MMI     */ "mmi",
  /* MSMI    */ "mmi",
  /* MFI     */ "mfi",
  /* MMF     */ "mmf",
  /* MIB     */ "mib",
  /* MBB     */ "mbb",
  /* RSVD_T3 */ "???",
  /* BBB     */ "bbb",
  /* MMB     */ "mmb",
  /* RSVD_T4 */ "???",
  /* MFB     */ "mfb",
  /* RSVD_T5 */ "???"
};

/* 09/16/02 REK Defining some functions that are coming in from tmdes. */
int
LT_is_input_param_operand (L_Operand * operand)
{
  if (operand == NULL)
    L_punt ("LT_is_input_param_operand: NULL operand");

  if (!L_is_macro (operand))
    return 0;

  if ((operand->value.mac >= L_MAC_P0) && (operand->value.mac <= L_MAC_P7))
    return 1;
  else
    return 0;
}				/* LT_is_input_param_operand */

int
LT_is_int_output_param_operand (L_Operand * operand)
{
  if (operand == NULL)
    L_punt ("LT_is_input_param_operand: NULL operand");

  if (!L_is_macro (operand))
    return 0;

  if ((operand->value.mac >= L_MAC_P8) && (operand->value.mac <= L_MAC_P15))
    return 1;
  else
    return 0;
}				/* LT_is_int_output_param_operand */

/* 09/16/02 REK This function is part of a pair that replace
 *              M_tahoe_cmp_proc_opc from Tmdes.  This selects the new
 *              proc_opc.  See also LT_tahoe_cmp_completer.
 */
int
LT_tahoe_cmp_proc_opc (ITuint8 ctype, ITuint8 com)
{
  if (L_is_ctype_int_direct (ctype))
    {
      if (com == Lcmp_COM_TZ || com == Lcmp_COM_TN)
	return TAHOEop_TBIT;
      else
	return TAHOEop_CMP;
    }				/* if */
  else
    {
      return TAHOEop_FCMP;
    }				/* else */
}				/* LT_tahoe_cmp_proc_opc */


/* 09/16/02 REK This function is part of a pair that replace
 *              M_tahoe_cmp_proc_opc from Tmdes.  This selects the new
 *              completer.  See also LT_tahoe_cmp_proc_opc.
 */
int
LT_tahoe_cmp_completer (ITuint8 ctype, ITuint8 com, int cmp_type)
{
  int completer = 0;

  int isInt = L_is_ctype_int_direct (ctype);
  int isSigned = L_is_ctype_signed_direct (ctype);

  if (L_is_size_int_direct (ctype))
    completer |= TC_CMP_4;
  /* 8 byte compare is the default. */
  /* else if (L_is_size_llong_direct (ctype)) */

  /* Set the compare op */
  switch (com)
    {
    case Lcmp_COM_EQ:
      TC_SET_CMP_OP (completer, TC_CMP_OP_EQ);
      break;

    case Lcmp_COM_NE:
      /* CMP uses NE, but FCMP uses NEQ */
      if (isInt)
	TC_SET_CMP_OP (completer, TC_CMP_OP_NE);
      else
	TC_SET_CMP_OP (completer, TC_CMP_OP_NEQ);
      break;

    case Lcmp_COM_LT:
      if (isInt && !isSigned && (cmp_type == TC_CMP_TYPE_UNC ||
				 cmp_type == TC_CMP_TYPE_NONE))
	TC_SET_CMP_OP (completer, TC_CMP_OP_LTU);
      else
	TC_SET_CMP_OP (completer, TC_CMP_OP_LT);
      break;

    case Lcmp_COM_GT:
      if (isInt && !isSigned && (cmp_type == TC_CMP_TYPE_UNC ||
				 cmp_type == TC_CMP_TYPE_NONE))
	TC_SET_CMP_OP (completer, TC_CMP_OP_GTU);
      else
	TC_SET_CMP_OP (completer, TC_CMP_OP_GT);
      break;

    case Lcmp_COM_LE:
      if (isInt && !isSigned && (cmp_type == TC_CMP_TYPE_UNC ||
				 cmp_type == TC_CMP_TYPE_NONE))
	TC_SET_CMP_OP (completer, TC_CMP_OP_LEU);
      else
	TC_SET_CMP_OP (completer, TC_CMP_OP_LE);
      break;

    case Lcmp_COM_GE:
      if (isInt && !isSigned && (cmp_type == TC_CMP_TYPE_UNC ||
				 cmp_type == TC_CMP_TYPE_NONE))
	TC_SET_CMP_OP (completer, TC_CMP_OP_GEU);
      else
	TC_SET_CMP_OP (completer, TC_CMP_OP_GE);
      break;

    case Lcmp_COM_TZ:
      TC_SET_CMP_OP (completer, TC_CMP_OP_Z);
      break;

    case Lcmp_COM_TN:
      TC_SET_CMP_OP (completer, TC_CMP_OP_NZ);
      break;
    }				/* switch */

  /* Set the compare type */
  TC_SET_CMP_TYPE (completer, cmp_type);

  return completer;
}				/* LT_tahoe_cmp_completer */


L_Oper *
LT_create_nop (int tahoeop, int value)
{
  L_Oper *new_oper;

  new_oper = L_create_new_op (Lop_NO_OP);
  new_oper->proc_opc = tahoeop;
  new_oper->src[0] = L_new_gen_int_operand (value);

  return (new_oper);
}				/* LT_create_nop */


L_Oper *
LT_create_template_op (int template_type, int stop_bit_mask)
{
  L_Oper *new_oper;

  new_oper = L_create_new_op (Lop_DEFINE);
  new_oper->proc_opc = TAHOEop_NON_INSTR;
  new_oper->dest[0] = L_new_macro_operand (TAHOE_MAC_TEMPLATE,
					   L_CTYPE_INT, 0);
  LT_new_template (new_oper, template_type);
  LT_new_stop_bit_mask (new_oper, stop_bit_mask);
  return (new_oper);
}				/* LT_create_template_op */

int
LT_is_float_output_param_operand (L_Operand * operand)
{
  if (operand == NULL)
    L_punt ("LT_is_input_param_operand: NULL operand");

  if (!L_is_macro (operand))
    return 0;

  if ((operand->value.mac >= L_MAC_P20) && (operand->value.mac <= L_MAC_P27))
    return 1;
  else
    return 0;
}				/* LT_is_float_output_param_operand */
