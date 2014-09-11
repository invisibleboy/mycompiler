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
/*****************************************************************************
 * phase1_opgen.c                                                            *
 * ------------------------------------------------------------------------- *
 * Routines to generate integer division and remainder algorithms            *
 *                                                                           *
 * AUTHORS: J.W. Sias, C.J. Shannon                                          *
 *****************************************************************************/
/* 09/12/02 REK Updating file to support the new opcode map. */

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "ltahoe_main.h"
#include "ltahoe_completers.h"
#include "phase1_opgen.h"

/* 09/12/02 REK Updating function to use the completers bitfield. */
int
Ltahoe_get_fsf (L_Oper * oper)
{
  return (TC_GET_FP_STATUS_REG (oper->completers));
}				/* Ltahoe_get_fsf */

/* 09/12/02 REK Updating function to use the completers bitfield. */
void
Ltahoe_set_fsf (L_Oper * oper, int fsf)
{
  TC_SET_FP_STATUS_REG (oper->completers, fsf);
  return;
}				/* Ltahoe_set_fsf */

/* 09/12/02 REK Modifying this function to work with the new opcode scheme.
 *              Each floating point opcode that takes a pc completer now
 *              has separate forms for each completer.
 */
int
Ltahoe_get_fpc (L_Oper * oper)
{
  switch (oper->proc_opc)
    {
    case TAHOEop_FMA_S:
    case TAHOEop_FMS_S:
    case TAHOEop_FNMA_S:
    case TAHOEop_SETF_S:
    case TAHOEop_GETF_S:
      return FPC_S;
      break;

    case TAHOEop_FMA_D:
    case TAHOEop_FMS_D:
    case TAHOEop_FNMA_D:
    case TAHOEop_SETF_D:
    case TAHOEop_GETF_D:
      return FPC_D;
      break;

    case TAHOEop_FMA:
    case TAHOEop_FMS:
    case TAHOEop_FNMA:
      return FPC_NONE;

    default:
      L_punt ("Ltahoe_get_fpc: invalid proc_opc for op %d", oper->id);
      break;
    }				/* switch */

  return 0; /* Should never reach this point. */
}				/* Ltahoe_get_fpc */

/* 09/12/02 REK Modifying this function to work with the new opcode scheme.
 *              Each floating point opcode that takes a pc completer now
 *              has separate forms for each completer.
 */
void
Ltahoe_set_fpc (L_Oper * oper, int fpc)
{
  switch (oper->proc_opc)
    {
    case TAHOEop_FMA:
    case TAHOEop_FMA_S:
    case TAHOEop_FMA_D:
      switch (fpc)
	{
	case FPC_NONE:
	  oper->proc_opc = TAHOEop_FMA;
	  break;

	case FPC_S:
	  oper->proc_opc = TAHOEop_FMA_S;
	  break;

	case FPC_D:
	  oper->proc_opc = TAHOEop_FMA_D;
	  break;

	default:
	  L_punt ("Ltahoe_set_fpc: invalid fpc %d", fpc);
	}			/* switch */
      break;

    case TAHOEop_FMS:
    case TAHOEop_FMS_S:
    case TAHOEop_FMS_D:
      switch (fpc)
	{
	case FPC_NONE:
	  oper->proc_opc = TAHOEop_FMS;
	  break;

	case FPC_S:
	  oper->proc_opc = TAHOEop_FMS_S;
	  break;

	case FPC_D:
	  oper->proc_opc = TAHOEop_FMS_D;
	  break;

	default:
	  L_punt ("Ltahoe_set_fpc: invalid fpc %d", fpc);
	}			/* switch */
      break;

    case TAHOEop_FNMA:
    case TAHOEop_FNMA_S:
    case TAHOEop_FNMA_D:
      switch (fpc)
	{
	case FPC_NONE:
	  oper->proc_opc = TAHOEop_FNMA;
	  break;

	case FPC_S:
	  oper->proc_opc = TAHOEop_FNMA_S;
	  break;

	case FPC_D:
	  oper->proc_opc = TAHOEop_FNMA_D;
	  break;

	default:
	  L_punt ("Ltahoe_set_fpc: invalid fpc %d", fpc);
	}			/* switch */
      break;

    default:
      L_punt ("Ltahoe_set_fpc: invalid proc_opc for op %d", oper->id);
    }				/* switch */

  return;
}				/* Ltahoe_set_fpc */

/* Ltahoe_new_fma
 * ----------------------------------------------------------------------
 * Generate a floating point multiply-accumulate instruction.
 * Copy the specified operands.  Use a template operation if specified.
 */
/* 09/12/02 REK Updating function to use new TAHOEops. */
L_Oper *
Ltahoe_new_fma (L_Operand * pred,
		L_Operand * dest,
		L_Operand * src0, L_Operand * src1, L_Operand * src2,
		int fsf, int fpc, L_Oper * using)
{
  L_Oper *new_oper;

  new_oper = L_create_new_op_using (Lop_MUL_ADD_F, using);
  new_oper->proc_opc = TAHOEop_FMA;
  Ltahoe_set_fpc (new_oper, fpc);
  Ltahoe_set_fsf (new_oper, fsf);

  if (new_oper->pred[0])
    L_delete_operand (new_oper->pred[0]);
  new_oper->pred[0] = L_copy_operand (pred);
  if (new_oper->pred[0])
    new_oper->pred[0]->ptype = L_PTYPE_NULL;
  new_oper->dest[0] = dest ? L_copy_operand (dest) :
    L_new_register_operand (++(L_fn->max_reg_id),
			    L_CTYPE_DOUBLE, L_PTYPE_NULL);
  new_oper->src[0] = L_copy_operand (src0);
  new_oper->src[1] = L_copy_operand (src1);
  new_oper->src[2] = L_copy_operand (src2);

  return new_oper;
}				/* Ltahoe_new_fma */

/* 09/12/02 REK Updating function to use new TAHOEops. */
L_Oper *
Ltahoe_new_fnma (L_Operand * pred,
		 L_Operand * dest,
		 L_Operand * src0, L_Operand * src1, L_Operand * src2,
		 int fsf, int fpc, L_Oper * using)
{
  L_Oper *new_oper;

  new_oper = L_create_new_op_using (Lop_MUL_ADD_F, using);
  new_oper->proc_opc = TAHOEop_FNMA;
  Ltahoe_set_fpc (new_oper, fpc);
  Ltahoe_set_fsf (new_oper, fsf);

  if (new_oper->pred[0])
    L_delete_operand (new_oper->pred[0]);
  new_oper->pred[0] = L_copy_operand (pred);
  if (new_oper->pred[0])
    new_oper->pred[0]->ptype = L_PTYPE_NULL;
  new_oper->dest[0] = dest ? L_copy_operand (dest) :
    L_new_register_operand (++(L_fn->max_reg_id),
			    L_CTYPE_DOUBLE, L_PTYPE_NULL);
  new_oper->src[0] = L_copy_operand (src0);
  new_oper->src[1] = L_copy_operand (src1);
  new_oper->src[2] = L_copy_operand (src2);

  return new_oper;
}				/* Ltahoe_new_fnma */

/* 09/12/02 REK Updating function to use new TAHOEops. */
L_Oper *
Ltahoe_new_fadd (L_Operand * pred,
		 L_Operand * dest,
		 L_Operand * src0, L_Operand * src1, int fsf, int fpc,
		 L_Oper * using)
{
  L_Oper *new_oper;

  /* Used to generate TAHOEop_FADD */

  new_oper = L_create_new_op_using (Lop_MUL_ADD_F, using);
  new_oper->proc_opc = TAHOEop_FMA;
  Ltahoe_set_fpc (new_oper, fpc);
  Ltahoe_set_fsf (new_oper, fsf);

  if (new_oper->pred[0])
    L_delete_operand (new_oper->pred[0]);
  new_oper->pred[0] = L_copy_operand (pred);
  if (new_oper->pred[0])
    new_oper->pred[0]->ptype = L_PTYPE_NULL;
  new_oper->dest[0] = dest ? L_copy_operand (dest) :
    L_new_register_operand (++(L_fn->max_reg_id),
			    L_CTYPE_DOUBLE, L_PTYPE_NULL);
  new_oper->src[0] = L_copy_operand (src0);
  new_oper->src[1] = Ltahoe_FMAC (FONE);
  new_oper->src[2] = L_copy_operand (src1);

  return new_oper;
}				/* Ltahoe_new_fadd */

/* 09/12/02 REK Updating function to use new TAHOEops. */
L_Oper *
Ltahoe_new_frcpa (L_Operand * pred,
		  L_Operand * dest0, L_Operand * dest1,
		  L_Operand * src0, L_Operand * src1, int fsf, L_Oper * using)
{
  L_Oper *new_oper;

  new_oper = L_create_new_op_using (Lop_DIV_F2, using);
  new_oper->proc_opc = TAHOEop_FRCPA;

  Ltahoe_set_fsf (new_oper, fsf);

  if (new_oper->pred[0])
    L_delete_operand (new_oper->pred[0]);
  new_oper->pred[0] = L_copy_operand (pred);
  if (new_oper->pred[0])
    new_oper->pred[0]->ptype = L_PTYPE_NULL;
  new_oper->dest[0] = dest0 ? L_copy_operand (dest0) :
    L_new_register_operand (++(L_fn->max_reg_id),
			    L_CTYPE_DOUBLE, L_PTYPE_NULL);
  new_oper->dest[1] = dest1 ? L_copy_operand (dest1) :
    L_new_register_operand (++(L_fn->max_reg_id),
			    L_CTYPE_PREDICATE, L_PTYPE_UNCOND_T);
  new_oper->dest[1]->ptype = L_PTYPE_UNCOND_T;
  new_oper->src[0] = L_copy_operand (src0);
  new_oper->src[1] = L_copy_operand (src1);

  return new_oper;
}				/* Ltahoe_new_frcpa */

/* 09/12/02 REK Updating function to use new TAHOEops. */
L_Oper *
Ltahoe_new_fcvt_fx_trunc (L_Operand * pred,
			  L_Operand * dest0, L_Operand * src0, int fsf,
			  L_Oper * using)
{
  L_Oper *new_oper;

  new_oper = L_create_new_op_using (Lop_F2_I, using);

  if (using->opc == Lop_REM_U || using->opc == Lop_DIV_U)
    {
      new_oper->proc_opc = TAHOEop_FCVT_FXU;
      new_oper->completers |= TC_FCVT_TRUNC;
    }				/* if */
  else
    {
      new_oper->proc_opc = TAHOEop_FCVT_FX;
      new_oper->completers |= TC_FCVT_TRUNC;
    }				/* else */

  Ltahoe_set_fsf (new_oper, fsf);

  if (new_oper->pred[0])
    L_delete_operand (new_oper->pred[0]);
  new_oper->pred[0] = L_copy_operand (pred);
  if (new_oper->pred[0])
    new_oper->pred[0]->ptype = L_PTYPE_NULL;
  new_oper->dest[0] = L_copy_operand (dest0);
  new_oper->src[0] = L_copy_operand (src0);

  return new_oper;
}				/* Ltahoe_new_fcvt_fx_trunc */

/* 09/12/02 REK Updating function to use new TAHOEops. */
L_Oper *
Ltahoe_new_getf_sig (L_Operand * pred,
		     L_Operand * dest0, L_Operand * src0, L_Oper * using)
{
  L_Oper *new_oper;

  new_oper = L_create_new_op_using (Lop_F_I, using);
  new_oper->proc_opc = TAHOEop_GETF_SIG;

  if (new_oper->pred[0])
    L_delete_operand (new_oper->pred[0]);
  new_oper->pred[0] = L_copy_operand (pred);
  if (new_oper->pred[0])
    new_oper->pred[0]->ptype = L_PTYPE_NULL;
  new_oper->dest[0] = L_copy_operand (dest0);
  new_oper->src[0] = L_copy_operand (src0);

  return new_oper;
}				/* Ltahoe_new_getf_sig */

/* 09/12/02 REK Updating function to use new TAHOEops. */
L_Oper *
Ltahoe_new_movi (L_Operand * pred,
		 L_Operand * dest0, ITint64 value, L_Oper * using)
{
  L_Oper *new_oper;

  new_oper = L_create_new_op_using (Lop_MOV, using);
  new_oper->proc_opc = TAHOEop_MOVI;

  if (new_oper->pred[0])
    L_delete_operand (new_oper->pred[0]);
  new_oper->pred[0] = L_copy_operand (pred);
  if (new_oper->pred[0])
    new_oper->pred[0]->ptype = 0;
  new_oper->src[0] = L_new_llong_operand (value, L_CTYPE_LLONG);
  new_oper->dest[0] = L_copy_operand (dest0);

  return new_oper;
}				/* Ltahoe_new_movi */

/* 09/12/02 REK Updating function to use new TAHOEops. */
L_Oper *
Ltahoe_new_movl (L_Operand * pred,
		 L_Operand * dest0, ITint64 value, L_Oper * using)
{
  L_Oper *new_oper;

  new_oper = L_create_new_op_using (Lop_MOV, using);
  new_oper->proc_opc = TAHOEop_MOVL;

  if (new_oper->pred[0])
    L_delete_operand (new_oper->pred[0]);
  new_oper->pred[0] = L_copy_operand (pred);
  if (new_oper->pred[0])
    new_oper->pred[0]->ptype = L_PTYPE_NULL;
  new_oper->src[0] = L_new_llong_operand (value, L_CTYPE_LLONG);
  new_oper->dest[0] = L_copy_operand (dest0);

  return new_oper;
}				/* Ltahoe_new_movl */

/* 09/12/02 REK Updating function to use new TAHOEops. */
L_Oper *
Ltahoe_new_setf_exp (L_Operand * pred,
		     L_Operand * dest0, L_Operand * src0, L_Oper * using)
{
  L_Oper *new_oper;

  new_oper = L_create_new_op_using (Lop_I_F, using);
  new_oper->proc_opc = TAHOEop_SETF_EXP;
  if (new_oper->pred[0])
    L_delete_operand (new_oper->pred[0]);
  new_oper->pred[0] = L_copy_operand (pred);
  if (new_oper->pred[0])
    new_oper->pred[0]->ptype = L_PTYPE_NULL;
  new_oper->src[0] = L_copy_operand (src0);
  new_oper->dest[0] = L_copy_operand (dest0);

  return new_oper;
}				/* Ltahoe_new_setf_exp */

/* 09/12/02 REK Updating function to use new TAHOEops */
L_Oper *
Ltahoe_new_setf_s (L_Operand * pred,
		   L_Operand * dest0, L_Operand * src0, L_Oper * using)
{
  L_Oper *new_oper;
  /* L_Attr *attr; */

  new_oper = L_create_new_op_using (Lop_I_F, using);
  new_oper->proc_opc = TAHOEop_SETF_S;

  /* 09/12/02 REK This is redundant to specify for SETF_S */
  /* attr = L_new_attr ("fpc", 1); */
  /* L_set_int_attr_field (attr, 0, FPC_S); */
  /* new_oper->attr = L_concat_attr (new_oper->attr, attr); */

  if (new_oper->pred[0])
    L_delete_operand (new_oper->pred[0]);
  new_oper->pred[0] = L_copy_operand (pred);
  if (new_oper->pred[0])
    new_oper->pred[0]->ptype = L_PTYPE_NULL;
  new_oper->src[0] = L_copy_operand (src0);
  new_oper->dest[0] = L_copy_operand (dest0);

  return new_oper;
}				/* Ltahoe_new_setf_s */

/* 09/12/02 REK Updating function to use new TAHOEops. */
L_Oper *
Ltahoe_new_setf_sig (L_Operand * pred,
		     L_Operand * dest0, L_Operand * src0, L_Oper * using)
{
  L_Oper *new_oper;
  /* L_Attr *attr; */

  new_oper = L_create_new_op_using (Lop_I_F, using);
  new_oper->proc_opc = TAHOEop_SETF_SIG;

  /* 09/12/02 REK Setting FPC_S makes no sense for SETF_SIG. */
  /* attr = L_new_attr ("fpc", 1); */
  /* L_set_int_attr_field (attr, 0, FPC_S); */
  /* new_oper->attr = L_concat_attr (new_oper->attr, attr); */

  if (new_oper->pred[0])
    L_delete_operand (new_oper->pred[0]);
  new_oper->pred[0] = L_copy_operand (pred);
  if (new_oper->pred[0])
    new_oper->pred[0]->ptype = L_PTYPE_NULL;
  new_oper->src[0] = L_copy_operand (src0);
  new_oper->dest[0] = L_copy_operand (dest0);

  return new_oper;
}				/* Ltahoe_new_setf_sig */

/* 09/12/02 REK Updating function to use new TAHOEops. */
L_Oper *
Ltahoe_new_xma_l (L_Operand * pred,
		  L_Operand * dest,
		  L_Operand * src0, L_Operand * src1, L_Operand * src2,
		  L_Oper * using)
{
  L_Oper *new_oper;

  new_oper = L_create_new_op_using (Lop_MUL_ADD_F, using);
  new_oper->proc_opc = TAHOEop_XMA_L;

  if (new_oper->pred[0])
    L_delete_operand (new_oper->pred[0]);
  new_oper->pred[0] = L_copy_operand (pred);
  if (new_oper->pred[0])
    new_oper->pred[0]->ptype = L_PTYPE_NULL;
  new_oper->dest[0] = L_copy_operand (dest);
  new_oper->src[0] = L_copy_operand (src0);
  new_oper->src[1] = L_copy_operand (src1);
  new_oper->src[2] = L_copy_operand (src2);

  return new_oper;
}				/* Ltahoe_new_xma_l */

static void
Ltahoe_generate_temp_regs (L_Operand ** opdarry, int count, int ctype,
			   int ptype)
{
  int i;

  for (i = 0; i < count; i++)
    opdarry[i] = L_new_register_operand (++(L_fn->max_reg_id), ctype, ptype);
}

static void
Ltahoe_free_temp_regs (L_Operand ** opdarry, int count)
{
  int i;

  for (i = 0; i < count; i++)
    L_delete_operand (opdarry[i]);
}

void
Ltahoe_annotate_EM_int_divide64_lat (L_Cb * cb, L_Oper * oper)
{
  L_Operand *ftreg[3];
  L_Operand *ptreg[1];
  L_Operand *fmac1, *fmac0;

  L_Operand *divisor, *dividend, *quotient;

  L_Oper *new_oper;

  Ltahoe_generate_temp_regs (ftreg, 3, L_CTYPE_DOUBLE, L_PTYPE_NULL);
  Ltahoe_generate_temp_regs (ptreg, 1, L_CTYPE_PREDICATE, L_PTYPE_NULL);
  fmac1 = Ltahoe_FMAC (FONE);
  fmac0 = Ltahoe_FMAC (FZERO);

  quotient = oper->dest[0];
  dividend = oper->src[0];
  divisor = oper->src[1];

  /* Algorithm from Intel Programmer's Guide to Assembler for IA-64
   *  q = floor(a/b)
   *
   * (1)  y0 = 1/b * (1 + e)  e < 2^(-8.886)
   * (2)  e0 = (1 - b*y0)
   * (3)  q0 = (a*y0)
   * (4)  e1 = (e0*e0)
   * (5)  q1 = (q0 + e0*q0)
   * (6)  y1 = (y0 + e0*y1)
   * (7)  q2 = (q1 + e1*q1)
   * (8)  y2 = (y1 + e1*y1)
   * (9)  r2 = (a - b*q2)
   * (10) q3 = (q2 + r2*y2)
   * (11) q = trunc(q3)
   */

  /* Four FP temp regs
   *  dest[0] = y0 (1-6) y1 (6-8) y2 (8-10) q3 (10-11)
   * ftreg[0] = e0 (2-5) r2 (9-10)
   * ftreg[1] = q0 (3-5) q1 (5-7) q2 (7-10)
   * ftreg[2] = e1 (4-8)
   */


  /* (1)  y0 = 1/b * (1 + e)  e < 2^(-8.886) */
  new_oper = Ltahoe_new_frcpa (oper->pred[0],
			       quotient, ptreg[0],
			       dividend, divisor, FSF_S1, oper);
  L_insert_oper_before (cb, oper, new_oper);

  /* (2)  e0 = (1 - b*y0) */
  new_oper = Ltahoe_new_fnma (ptreg[0],
			      ftreg[0], divisor, quotient, fmac1,
			      FSF_S1, FPC_NONE, NULL);
  L_insert_oper_before (cb, oper, new_oper);

  /* (3)  q0 = (a*y0) */
  new_oper = Ltahoe_new_fma (ptreg[0],
			     ftreg[1], quotient, dividend, fmac0,
			     FSF_S1, FPC_NONE, NULL);
  L_insert_oper_before (cb, oper, new_oper);

  /* (4)  e1 = (e0*e0) */
  new_oper = Ltahoe_new_fma (ptreg[0],
			     ftreg[2], ftreg[0], ftreg[0], fmac0,
			     FSF_S1, FPC_NONE, NULL);
  L_insert_oper_before (cb, oper, new_oper);

  /* (5)  q1 = (q0 + e0*q0) */
  new_oper = Ltahoe_new_fma (ptreg[0],
			     ftreg[1], ftreg[1], ftreg[0], ftreg[1],
			     FSF_S1, FPC_NONE, NULL);
  L_insert_oper_before (cb, oper, new_oper);

  /* (6)  y1 = (y0 + e0*y0) */
  new_oper = Ltahoe_new_fma (ptreg[0],
			     quotient, quotient, ftreg[0], quotient,
			     FSF_S1, FPC_NONE, NULL);
  L_insert_oper_before (cb, oper, new_oper);

  /* (7) q2 = (q1 + e1*q1) */
  new_oper = Ltahoe_new_fma (ptreg[0],
			     ftreg[1], ftreg[1], ftreg[2], ftreg[1],
			     FSF_S1, FPC_NONE, NULL);
  L_insert_oper_before (cb, oper, new_oper);

  /* (8)  y2 = (y1 + e1*y1) */
  new_oper = Ltahoe_new_fma (ptreg[0],
			     quotient, quotient, ftreg[2], quotient,
			     FSF_S1, FPC_NONE, NULL);
  L_insert_oper_before (cb, oper, new_oper);

  /* (9)  r2 = (a - b*q2) */
  new_oper = Ltahoe_new_fnma (ptreg[0],
			      ftreg[0], ftreg[1], divisor, dividend,
			      FSF_S1, FPC_NONE, NULL);
  L_insert_oper_before (cb, oper, new_oper);

  /* (10) q3 = (q2 + r2*y2) */
  new_oper = Ltahoe_new_fma (ptreg[0],
			     quotient, quotient, ftreg[0], ftreg[1],
			     FSF_S1, FPC_NONE, NULL);
  L_insert_oper_before (cb, oper, new_oper);

  /* Step 11 done in L_annotate_int_complex_division in phase1_imath.c */

  Ltahoe_free_temp_regs (ftreg, 3);
  Ltahoe_free_temp_regs (ptreg, 1);
  L_delete_operand (fmac1);
  L_delete_operand (fmac0);
}

void
Ltahoe_annotate_EM_int_divide64_thr (L_Cb * cb, L_Oper * oper)
{
  L_Operand *ftreg[2];
  L_Operand *ptreg[1];
  L_Operand *fmac1, *fmac0;

  L_Operand *divisor, *dividend, *quotient;

  L_Oper *new_oper;

  Ltahoe_generate_temp_regs (ftreg, 2, L_CTYPE_DOUBLE, L_PTYPE_NULL);
  Ltahoe_generate_temp_regs (ptreg, 1, L_CTYPE_PREDICATE, L_PTYPE_NULL);
  fmac1 = Ltahoe_FMAC (FONE);
  fmac0 = Ltahoe_FMAC (FZERO);

  quotient = oper->dest[0];
  dividend = oper->src[0];
  divisor = oper->src[1];

  /* Algorithm from Intel Programmer's Guide to Assembler for IA-64
   *  q = floor(a/b)
   *
   * (1)  y0 = 1/b * (1 + e)  e < 2^(-8.886)
   * (2)  e0 = (1 - b*y0)
   * (3)  y1 = (y0 + e0*y0)
   * (4)  e1 = (e0*e0)
   * (5)  y2 = (y1 + e1*y1)
   * (6)  q2 = (a*y2)
   * (7)  r2 = (a - b*q2)
   * (8)  q3 = (q2 + r2*y2)
   * (9)  q = trunc(q3)
   */

  /* Three FP temp regs
   *  dest[0] = y0 (1-3) y1 (3-5) y2 (5-8) q3 (8-9)
   * ftreg[0] = e0 (2-4) e1 (4-5) r2 (7-8)
   * ftreg[1] = q2 (6-8)
   */



  /* (1)  y0 = 1/b * (1 + e)  e < 2^(-8.886) */
  new_oper = Ltahoe_new_frcpa (oper->pred[0],
			       quotient, ptreg[0],
			       dividend, divisor, FSF_S1, oper);
  L_insert_oper_before (cb, oper, new_oper);

  /* (2)  e0 = (1 - b*y0) */
  new_oper = Ltahoe_new_fnma (ptreg[0],
			      ftreg[0], divisor, quotient, fmac1,
			      FSF_S1, FPC_NONE, NULL);
  L_insert_oper_before (cb, oper, new_oper);

  /* (3)  y1 = (y0 + e0*y0) */
  new_oper = Ltahoe_new_fma (ptreg[0],
			     quotient, quotient, ftreg[0], quotient,
			     FSF_S1, FPC_NONE, NULL);
  L_insert_oper_before (cb, oper, new_oper);

  /* (4)  e1 = (e0*e0) */
  new_oper = Ltahoe_new_fma (ptreg[0],
			     ftreg[0], ftreg[0], ftreg[0], fmac0,
			     FSF_S1, FPC_NONE, NULL);
  L_insert_oper_before (cb, oper, new_oper);

  /* (5)  y2 = (y1 + e1*y1) */
  new_oper = Ltahoe_new_fma (ptreg[0],
			     quotient, quotient, ftreg[0], quotient,
			     FSF_S1, FPC_NONE, NULL);
  L_insert_oper_before (cb, oper, new_oper);

  /* (6)  q2 = (a*y2) */
  new_oper = Ltahoe_new_fma (ptreg[0],
			     ftreg[1], quotient, dividend, fmac0,
			     FSF_S1, FPC_NONE, NULL);
  L_insert_oper_before (cb, oper, new_oper);

  /* (7)  r2 = (a - b*q2) */
  new_oper = Ltahoe_new_fnma (ptreg[0],
			      ftreg[0], ftreg[1], divisor, dividend,
			      FSF_S1, FPC_NONE, NULL);
  L_insert_oper_before (cb, oper, new_oper);

  /* (8) q3 = (q2 + r2*y2) */
  new_oper = Ltahoe_new_fma (ptreg[0],
			     quotient, quotient, ftreg[0], ftreg[1],
			     FSF_S1, FPC_NONE, NULL);
  L_insert_oper_before (cb, oper, new_oper);

  /* Step 9 done in L_annotate_int_complex_division in phase1_imath.c */

  Ltahoe_free_temp_regs (ftreg, 2);
  Ltahoe_free_temp_regs (ptreg, 1);
  L_delete_operand (fmac1);
  L_delete_operand (fmac0);
}

void
Ltahoe_annotate_EM_int_divide32 (L_Cb * cb, L_Oper * oper)
{
  L_Operand *ftreg[2];
  L_Operand *ptreg[1];
  L_Operand *ltreg[1];
  L_Operand *fmac1, *fmac0;

  L_Operand *divisor, *dividend, *quotient;

  L_Oper *new_oper;

  Ltahoe_generate_temp_regs (ftreg, 2, L_CTYPE_DOUBLE, L_PTYPE_NULL);
  Ltahoe_generate_temp_regs (ptreg, 1, L_CTYPE_PREDICATE, L_PTYPE_NULL);
  Ltahoe_generate_temp_regs (ltreg, 1, L_CTYPE_LLONG, L_PTYPE_NULL);
  fmac1 = Ltahoe_FMAC (FONE);
  fmac0 = Ltahoe_FMAC (FZERO);

  quotient = oper->dest[0];
  dividend = oper->src[0];
  divisor = oper->src[1];

  /* Algorithm from Intel Programmer's Guide to Assembler for IA-64
   *  q = floor(a/b)
   *
   * (1)  y0 = 1/b * (1 + e)  e < 2^(-8.886)
   * (2)  q0 = (a*y0)
   * (3)  e0 = (1 - b*y0)
   * (4)  q1 = (q0 + e0*q0)
   * (5)  e1 = (e0*e0 + 2^(-34))
   * (6)  q2 = (q1 + e1*q1)
   * (7)   q = trunc(q2)
   */

  /* Three FP temp regs
   *  dest[0] = y0 (1-3) e0 (3-5) e1 (5-6) q2 (6-7)
   * ftreg[0] = q0 (1-4) q1 (4-6)
   * ftreg[1] = 2^(-34)
   */


  /* mov r2 = 0x0ffdd */
  new_oper = Ltahoe_new_movi (oper->pred[0], ltreg[0], 0x0ffdd, oper);
  L_insert_oper_before (cb, oper, new_oper);

  /* setf.exp f9 = r2 */
  new_oper = Ltahoe_new_setf_exp (oper->pred[0], ftreg[1], ltreg[0], NULL);
  L_insert_oper_before (cb, oper, new_oper);

  /* (1)  y0 = 1/b * (1 + e)  e < 2^(-8.886) */
  new_oper = Ltahoe_new_frcpa (oper->pred[0],
			       quotient, ptreg[0],
			       dividend, divisor, FSF_S1, NULL);
  L_insert_oper_before (cb, oper, new_oper);

  /* (2)  q0 = (a*y0) */
  new_oper = Ltahoe_new_fma (ptreg[0],
			     ftreg[0], dividend, quotient, fmac0,
			     FSF_S1, FPC_NONE, NULL);
  L_insert_oper_before (cb, oper, new_oper);

  /* (3)  e0 = (1 - b*y0) */
  new_oper = Ltahoe_new_fnma (ptreg[0],
			      quotient, divisor, quotient, fmac1,
			      FSF_S1, FPC_NONE, NULL);
  L_insert_oper_before (cb, oper, new_oper);

  /* (4)  q1 = (q0 + e0*q0) */
  new_oper = Ltahoe_new_fma (ptreg[0],
			     ftreg[0], quotient, ftreg[0], ftreg[0],
			     FSF_S1, FPC_NONE, NULL);
  L_insert_oper_before (cb, oper, new_oper);

  /* (5)  e1 = (e0*e0 + 2^(-34)) */
  new_oper = Ltahoe_new_fma (ptreg[0],
			     quotient, quotient, quotient, ftreg[1],
			     FSF_S1, FPC_NONE, NULL);
  L_insert_oper_before (cb, oper, new_oper);

  /* (6)  q2 = (q1 + e1*q1) */
  new_oper = Ltahoe_new_fma (ptreg[0],
			     quotient, quotient, ftreg[0], ftreg[0],
			     FSF_S1, FPC_NONE, NULL);
  L_insert_oper_before (cb, oper, new_oper);

  /* Step 7 done in L_annotate_int_complex_division in phase1_imath.c */

  Ltahoe_free_temp_regs (ftreg, 2);
  Ltahoe_free_temp_regs (ptreg, 1);
  Ltahoe_free_temp_regs (ltreg, 1);
  L_delete_operand (fmac1);
  L_delete_operand (fmac0);
}

void
Ltahoe_annotate_EM_int_divide16_thr (L_Cb * cb, L_Oper * oper)
{
  L_Operand *ftreg[2];
  L_Operand *ptreg[1];
  L_Operand *ltreg[1];
  L_Operand *fmac0;

  L_Operand *divisor, *dividend, *quotient;

  L_Oper *new_oper;

  Ltahoe_generate_temp_regs (ftreg, 2, L_CTYPE_DOUBLE, L_PTYPE_NULL);
  Ltahoe_generate_temp_regs (ptreg, 1, L_CTYPE_PREDICATE, L_PTYPE_NULL);
  Ltahoe_generate_temp_regs (ltreg, 1, L_CTYPE_LLONG, L_PTYPE_NULL);
  fmac0 = Ltahoe_FMAC (FZERO);

  quotient = oper->dest[0];
  dividend = oper->src[0];
  divisor = oper->src[1];

  /* Algorithm from Intel Programmer's Guide to Assembler for IA-64
   *  q = floor(a/b)
   *
   * (1)  y0 = 1/b * (1 + e)  e < 2^(-8.886)
   * (2)  q0 = (a*y0)
   * (3)  e0 = (1+2^(-17) - b*y0)
   * (4)  q1 = (q0 + e0*q0)
   * (5)   q = trunc(q1)
   */

  /* Three FP temp regs
   *  dest[0] = y0 (1-3) e0 (3-4)
   * ftreg[0] = q0 (2-4) q1 (4-5)
   * ftreg[1] = 1+2^(-17)
   */


  /* mov r2 = 0x3f800040 */
  new_oper = Ltahoe_new_movl (oper->pred[0], ltreg[0], 0x3f800040, oper);
  L_insert_oper_before (cb, oper, new_oper);

  /* setf.s f8 = r2 */
  new_oper = Ltahoe_new_setf_s (oper->pred[0], ftreg[1], ltreg[0], NULL);
  L_insert_oper_before (cb, oper, new_oper);

  /* (1)  y0 = 1/b * (1 + e)  e < 2^(-8.886) */
  new_oper = Ltahoe_new_frcpa (oper->pred[0],
			       quotient, ptreg[0],
			       dividend, divisor, FSF_S1, NULL);
  L_insert_oper_before (cb, oper, new_oper);

  /* (2)  q0 = (a*y0) */
  new_oper = Ltahoe_new_fma (ptreg[0],
			     ftreg[0], dividend, quotient, fmac0,
			     FSF_S1, FPC_NONE, NULL);
  L_insert_oper_before (cb, oper, new_oper);

  /* (3)  e0 = (1+2^(-17) - b*y0) */
  new_oper = Ltahoe_new_fnma (ptreg[0],
			      quotient, divisor, quotient, ftreg[1],
			      FSF_S1, FPC_NONE, NULL);
  L_insert_oper_before (cb, oper, new_oper);

  /* (4)  q1 = (q0 + e0*q0) */
  new_oper = Ltahoe_new_fma (ptreg[0],
			     quotient, quotient, ftreg[0], ftreg[0],
			     FSF_S1, FPC_NONE, NULL);
  L_insert_oper_before (cb, oper, new_oper);

  /* Step 5 done in L_annotate_int_complex_division in phase1_imath.c */

  Ltahoe_free_temp_regs (ftreg, 2);
  Ltahoe_free_temp_regs (ptreg, 1);
  Ltahoe_free_temp_regs (ltreg, 1);
  L_delete_operand (fmac0);
}

void
Ltahoe_annotate_EM_int_divide8_thr (L_Cb * cb, L_Oper * oper)
{
  L_Operand *ftreg[1];
  L_Operand *ptreg[1];
  L_Operand *ltreg[1];
  L_Operand *fmac0;

  L_Operand *divisor, *dividend, *quotient;

  L_Oper *new_oper;

  Ltahoe_generate_temp_regs (ftreg, 1, L_CTYPE_DOUBLE, L_PTYPE_NULL);
  Ltahoe_generate_temp_regs (ptreg, 1, L_CTYPE_PREDICATE, L_PTYPE_NULL);
  Ltahoe_generate_temp_regs (ltreg, 1, L_CTYPE_LLONG, L_PTYPE_NULL);
  fmac0 = Ltahoe_FMAC (FZERO);

  quotient = oper->dest[0];
  dividend = oper->src[0];
  divisor = oper->src[1];

  /* Algorithm from Intel Programmer's Guide to Assembler for IA-64
   *  q = floor(a/b)
   *
   * (0)  a' = a*(1+2^(-8))
   * (1)  y0 = 1/b * (1 + e)  e < 2^(-8.886)
   * (2)  q0 = (a'*y0)
   * (3)   q = trunc(q1)
   */

  /* Two FP temp regs
   *  dest[0] = y0 (1-2) q0 (2-3)
   * ftreg[0] = 1+2^(-8) a' (0-2)
   */


  /* mov r2 = 0x3f808000 */
  new_oper = Ltahoe_new_movl (oper->pred[0], ltreg[0], 0x3f808000, oper);
  L_insert_oper_before (cb, oper, new_oper);

  /* setf.s f8 = r2 */
  new_oper = Ltahoe_new_setf_s (oper->pred[0], ftreg[0], ltreg[0], NULL);
  L_insert_oper_before (cb, oper, new_oper);

  /* (0)  a' = a*(1+2^(-8)) */
  new_oper = Ltahoe_new_fma (oper->pred[0],
			     ftreg[0], dividend, ftreg[0], fmac0,
			     FSF_S1, FPC_NONE, NULL);
  L_insert_oper_before (cb, oper, new_oper);

  /* (1)  y0 = 1/b * (1 + e)  e < 2^(-8.886) */
  new_oper = Ltahoe_new_frcpa (oper->pred[0],
			       quotient, ptreg[0],
			       dividend, divisor, FSF_S1, NULL);
  L_insert_oper_before (cb, oper, new_oper);

  /* (2)  q0 = (a'*y0) */
  new_oper = Ltahoe_new_fma (ptreg[0],
			     quotient, ftreg[0], quotient, fmac0,
			     FSF_S1, FPC_NONE, NULL);
  L_insert_oper_before (cb, oper, new_oper);

  /* Step 3 done in L_annotate_int_complex_division in phase1_imath.c */

  Ltahoe_free_temp_regs (ftreg, 1);
  Ltahoe_free_temp_regs (ptreg, 1);
  Ltahoe_free_temp_regs (ltreg, 1);
  L_delete_operand (fmac0);
}
