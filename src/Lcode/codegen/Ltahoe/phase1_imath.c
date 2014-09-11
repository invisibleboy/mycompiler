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
 * phase1_imath.c                                                            *
 * ------------------------------------------------------------------------- *
 * Integer multiply, iterative division and remainder implementations.       *
 * Includes table division code from Intel                                   *
 *                                                                           *
 * AUTHORS: J.W. Sias, C.J. Shannon, Intel                                   *
 *****************************************************************************/
/* 09/12/02 REK Updating file to use the new opcode map.
 *              Functions modified: L_table_imul, L_table_idiv.
 */

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "ltahoe_main.h"
#include "ltahoe_op_query.h"
#include "phase1_func.h"
#include "phase1_imult.h"
#include "phase1_idiv.h"
#include "phase1_opgen.h"

#define OUTPUT_LEN 128

/* 09/12/02 REK Modifying function to use new TAHOEops. */
static int
L_table_imul (L_Cb * cb, L_Oper * oper,
	      L_Operand * src_reg, L_Operand * src_int)
{
  L_Oper *new_oper = NULL;
  char output[OUTPUT_LEN];
  int op0, op1, op2, n;
  int cursor, status;
  int result_num, result_reg_id[100];
  int new_reg_id, depth, icount;


  /*   fprintf(stderr, "Using tahoe_imul for oper %d, const %d\n", */
  /*       oper->id, src_int->value.i); */

  /* handle boundary conditions first */

  /* Case of "register * 0" */
  if (src_int->value.i == 0)
    {
      new_oper = L_create_new_op_using (Lop_MOV, oper);
      new_oper->proc_opc = TAHOEop_MOV_GR;
      new_oper->src[0] =
	L_new_macro_operand (TAHOE_MAC_ZERO, L_CTYPE_LLONG, 0);
      new_oper->dest[0] = L_copy_operand (oper->dest[0]);
      L_insert_oper_before (cb, oper, new_oper);
      return (new_oper->dest[0]->value.r);
    }

  /* Case of "register * 1" */
  if (src_int->value.i == 1)
    {
      new_oper = L_create_new_op_using (Lop_MOV, oper);
      new_oper->proc_opc = TAHOEop_MOV_GR;
      new_oper->src[0] = L_copy_operand (src_reg);
      new_oper->dest[0] = L_copy_operand (oper->dest[0]);
      L_insert_oper_before (cb, oper, new_oper);
      return (new_oper->dest[0]->value.r);
    }

  /* Case of "register * -1" */
  if (src_int->value.i == -1)
    {
      new_oper = L_create_new_op_using (Lop_SUB, oper);
      new_oper->proc_opc = TAHOEop_SUB;
      new_oper->src[0] =
	L_new_macro_operand (TAHOE_MAC_ZERO, L_CTYPE_LLONG, 0);
      new_oper->src[1] = L_copy_operand (src_reg);
      new_oper->dest[0] = L_copy_operand (oper->dest[0]);
      L_insert_oper_before (cb, oper, new_oper);
      return (new_oper->dest[0]->value.r);
    }

  /* Now call Steve's table code */

  status = imul_sequence (src_int->value.i, output, OUTPUT_LEN,
			  &depth, &icount);

  switch (status)
    {
    case 0:
      /* Remove F (non-excepting) flag from oper */
      oper->flags = L_CLR_BIT_FLAG (oper->flags, L_OPER_SAFE_PEI);

      cursor = 0;
      result_num = 1;
      result_reg_id[1] = src_reg->value.r;

      while (output[cursor])
	{
	  switch (output[cursor++])
	    {
	    case '+':
	      cursor++;		/* skip the blank */
	      op0 = output[cursor++] - '0';
	      cursor++;
	      op1 = output[cursor++] - '0';
	      cursor++;

	      /*         fprintf(stderr, "\t + %d %d\n", op0, op1); */

	      new_oper = L_create_new_op_using (Lop_ADD, oper);
	      new_oper->proc_opc = TAHOEop_ADD;
	      new_oper->src[0] = L_new_register_operand (result_reg_id[op0],
							 L_CTYPE_LLONG, 0);
	      new_oper->src[1] = L_new_register_operand (result_reg_id[op1],
							 L_CTYPE_LLONG, 0);
	      new_reg_id = ++(L_fn->max_reg_id);
	      new_oper->dest[0] = L_new_register_operand (new_reg_id,
							  L_CTYPE_LLONG, 0);
	      result_reg_id[++result_num] = new_reg_id;
	      L_insert_oper_before (cb, oper, new_oper);
	      break;

	    case '-':
	      cursor++;		/* skip the blank */
	      op0 = output[cursor++] - '0';
	      cursor++;
	      op1 = output[cursor++] - '0';
	      cursor++;

	      /*         fprintf(stderr, "\t - %d %d\n", op0, op1); */

	      new_oper = L_create_new_op_using (Lop_SUB, oper);
	      new_oper->proc_opc = TAHOEop_SUB;
	      new_oper->src[0] = L_new_register_operand (result_reg_id[op0],
							 L_CTYPE_LLONG, 0);
	      new_oper->src[1] = L_new_register_operand (result_reg_id[op1],
							 L_CTYPE_LLONG, 0);
	      new_reg_id = ++(L_fn->max_reg_id);
	      new_oper->dest[0] = L_new_register_operand (new_reg_id,
							  L_CTYPE_LLONG, 0);
	      result_reg_id[++result_num] = new_reg_id;
	      L_insert_oper_before (cb, oper, new_oper);
	      break;

	    case '<':
	      n = output[cursor++] - '0';
	      cursor++;
	      op0 = output[cursor++] - '0';
	      cursor++;

	      /*         fprintf(stderr, "\t < %d %d\n", op0,n); */

	      new_oper = L_create_new_op_using (Lop_LSL, oper);
	      new_oper->src[0] = L_new_register_operand (result_reg_id[op0],
							 L_CTYPE_LLONG, 0);
	      if (n <= 4)
		{
		  new_oper->proc_opc = TAHOEop_SHLADD;
		  new_oper->src[1] = L_new_gen_int_operand (n);
		  new_oper->src[2] = L_new_macro_operand (TAHOE_MAC_ZERO,
							  L_CTYPE_LLONG, 0);
		}
	      else if (UIMM_6 (n))
		{
		  new_oper->proc_opc = TAHOEop_DEP_Z;
		  new_oper->src[1] = L_new_gen_int_operand (n);
		  new_oper->src[2] = L_new_gen_int_operand (64 - n);
		}
	      else
		{
		  new_oper->proc_opc = TAHOEop_SHL;
		  new_oper->src[1] = L_new_gen_int_operand (n);
		}

	      new_reg_id = ++(L_fn->max_reg_id);
	      new_oper->dest[0] = L_new_register_operand (new_reg_id,
							  L_CTYPE_LLONG, 0);
	      result_reg_id[++result_num] = new_reg_id;
	      L_insert_oper_before (cb, oper, new_oper);
	      break;

	    case 'N':
	      cursor++;		/* skip the blank */
	      op1 = output[cursor++] - '0';
	      cursor++;

	      /*         fprintf(stderr, "\t N %d\n", op1); */

	      new_oper = L_create_new_op_using (Lop_SUB, oper);
	      new_oper->proc_opc = TAHOEop_SUB;
	      new_oper->src[0] = L_new_macro_operand (TAHOE_MAC_ZERO,
						      L_CTYPE_LLONG, 0);
	      new_oper->src[1] = L_new_register_operand (result_reg_id[op1],
							 L_CTYPE_LLONG, 0);
	      new_reg_id = ++(L_fn->max_reg_id);
	      new_oper->dest[0] = L_new_register_operand (new_reg_id,
							  L_CTYPE_LLONG, 0);
	      result_reg_id[++result_num] = new_reg_id;
	      L_insert_oper_before (cb, oper, new_oper);
	      break;

	    case 'S':
	      n = output[cursor++] - '0';
	      cursor++;
	      op0 = output[cursor++] - '0';
	      cursor++;
	      op2 = output[cursor++] - '0';
	      cursor++;

	      new_oper = L_create_new_op_using (Lop_LSL, oper);

	      /*         fprintf(stderr, "\t S %d %d %d  (%d)\n", */
	      /*                 n, op0, op2, new_oper->id); */

	      new_oper->proc_opc = TAHOEop_SHLADD;
	      new_oper->src[0] = L_new_register_operand (result_reg_id[op0],
							 L_CTYPE_LLONG, 0);
	      new_oper->src[1] = L_new_gen_int_operand (n);
	      new_oper->src[2] = L_new_register_operand (result_reg_id[op2],
							 L_CTYPE_LLONG, 0);
	      new_reg_id = ++(L_fn->max_reg_id);
	      new_oper->dest[0] = L_new_register_operand (new_reg_id,
							  L_CTYPE_LLONG, 0);
	      result_reg_id[++result_num] = new_reg_id;
	      L_insert_oper_before (cb, oper, new_oper);
	      break;
	    }
	}
      L_delete_operand (new_oper->dest[0]);
      L_fn->max_reg_id--;
      new_oper->dest[0] = L_copy_operand (oper->dest[0]);
      return (new_oper->dest[0]->value.r);

    case 1:
      /*      fprintf(stderr, "Constant, %d, not it table - oper %d\n", */
      /*             src_int->value.i, oper->id); */
      return (0);

    case 2:
      L_punt ("L_table_imul: String size too small for oper %d", oper->id);
      return (0);
    }
  L_punt ("Control flows off the end of L_table_imul");
  return -1;
}

#define DO_PMPY 1

static void
L_tahoe_int_register_multiply (L_Cb * cb, L_Oper * oper,
			       L_Operand * src_reg0, L_Operand * src_reg1)
{
#if DO_PMPY
  if (!oper->com[0])
    {
#endif
      L_Oper *new_oper1, *new_oper2, *new_oper3, *new_oper4;

      /* Generate move int source 1 into float source 1 */
      new_oper1 = L_create_new_op_using (Lop_I_F, oper);
      new_oper1->src[0] = L_copy_operand (src_reg0);
      new_oper1->dest[0] = L_new_register_operand (++(L_fn->max_reg_id),
						   L_CTYPE_DOUBLE, 0);
      new_oper1->proc_opc = TAHOEop_SETF_SIG;
      L_insert_oper_before (cb, oper, new_oper1);
      
      /* Generate move int source 2 into float source 2 */
      new_oper2 = L_create_new_op_using (Lop_I_F, oper);
      new_oper2->src[0] = L_copy_operand (src_reg1);
      new_oper2->dest[0] = L_new_register_operand (++(L_fn->max_reg_id),
						   L_CTYPE_DOUBLE, 0);
      new_oper2->proc_opc = TAHOEop_SETF_SIG;
      L_insert_oper_before (cb, oper, new_oper2);

      /* Generate multiply */

      new_oper3 = L_create_new_op_using (Lop_MUL_F, oper);
      new_oper3->proc_opc = TAHOEop_XMA_L;
      new_oper3->src[0] = L_copy_operand (new_oper1->dest[0]);
      new_oper3->src[1] = L_copy_operand (new_oper2->dest[0]);
      new_oper3->src[2] =
	L_new_macro_operand (TAHOE_MAC_FZERO, L_CTYPE_DOUBLE, 0);
      new_oper3->dest[0] = L_copy_operand (new_oper1->dest[0]);

      L_insert_oper_before (cb, oper, new_oper3);

      /* Generate move float destination into int destination */
      new_oper4 = L_create_new_op_using (Lop_F_I, oper);
      new_oper4->src[0] = L_copy_operand (new_oper3->dest[0]);
      new_oper4->dest[0] = L_copy_operand (oper->dest[0]);
      new_oper4->proc_opc = TAHOEop_GETF_SIG;
      L_insert_oper_before (cb, oper, new_oper4);
#if DO_PMPY
    }
  else
    {
      L_Oper *new_oper1, *new_oper2, *new_oper3;
      L_Operand *src0, *src1;

      if (L_is_variable (oper->src[0]))
	{
	  src0 = L_copy_operand (oper->src[0]);
	}
      else
	{
	  src0 = L_new_register_operand (++(L_fn->max_reg_id), L_CTYPE_LLONG,
					0);
	  Ltahoe_int_constant_load (cb, oper, oper->src[0], src0);
	}
      if (L_is_variable (oper->src[1]))
	{
	  src1 = L_copy_operand (oper->src[1]);
	}
      else
	{
	  src1 = L_new_register_operand (++(L_fn->max_reg_id), L_CTYPE_LLONG,
					0);
	  Ltahoe_int_constant_load (cb, oper, oper->src[1], src1);
	}

      switch (oper->com[0])
	{
	case L_CTYPE_SHORT:
	  new_oper1 = L_create_new_op_using (Lop_MUL, oper);
	  new_oper1->proc_opc = TAHOEop_PMPY2_R;
	  L_insert_oper_before (cb, oper, new_oper1);
	  new_oper1->src[0] = src0;
	  new_oper1->src[1] = src1;
	  new_oper1->dest[0] = L_copy_operand (oper->dest[0]);
	  break;

	case L_CTYPE_USHORT:
	  new_oper1 = L_create_new_op_using (Lop_MUL, oper);
	  new_oper1->proc_opc= TAHOEop_PMPYSHR2_U;
	  L_insert_oper_before (cb, oper, new_oper1);
	  new_oper1->src[0] = L_copy_operand (src0);
	  new_oper1->src[1] = L_copy_operand (src1);
	  new_oper1->src[2] = L_new_gen_int_operand (0);
	  new_oper1->dest[0] = L_new_register_operand (++(L_fn->max_reg_id),
						       L_CTYPE_LLONG, 0);
	  new_oper2 = L_create_new_op_using (Lop_MUL, oper);
	  new_oper2->proc_opc= TAHOEop_PMPYSHR2_U;
	  L_insert_oper_before (cb, oper, new_oper2);
	  new_oper2->src[0] = src0;
	  new_oper2->src[1] = src1;
	  new_oper2->src[2] = L_new_gen_int_operand (16);
	  new_oper2->dest[0] = L_new_register_operand (++(L_fn->max_reg_id),
						       L_CTYPE_LLONG, 0);
	  new_oper3 = L_create_new_op_using (Lop_DEPOSIT, oper);
	  new_oper3->proc_opc = TAHOEop_DEP;
	  L_insert_oper_before (cb, oper, new_oper3);
	  new_oper3->src[0] = L_copy_operand (new_oper2->dest[0]);
	  new_oper3->src[1] = L_copy_operand (new_oper1->dest[0]);
	  new_oper3->src[2] = L_new_gen_int_operand (16);
	  new_oper3->src[3] = L_new_gen_int_operand (16);
	  new_oper3->dest[0] = L_copy_operand (oper->dest[0]);
	  break;
	default:
	  L_punt ("L_tahoe_int_register_multiply: bad completer");
	}
    }
#endif
}

void
L_annotate_int_multiply (L_Cb * cb, L_Oper * oper)
{
  L_Oper *new_oper;

  L_Operand *const_operand;
  L_Operand *src0 = oper->src[0];
  L_Operand *src1 = oper->src[1];

  int dest_reg_id;

  /* If one operand is int, make sure it's in src[0] */
  if (L_is_int_constant (src1))
    {
      src0 = oper->src[1];
      src1 = oper->src[0];
    }

  if (L_is_int_constant (src0))
    {
      if (L_is_int_constant (src1))
	{
	  const_operand =
	    L_new_gen_int_operand (src0->value.i * src1->value.i);
	  Ltahoe_int_constant_load (cb, oper, const_operand, oper->dest[0]);
	  L_delete_operand (const_operand);
	  fprintf (stderr,
		   "****** Poor Code Warning - L_annotate_int_multiply: two constant operands in %d\n",
		   oper->id);
	  return;
	}
      else
	{
	  switch (src1->type)
	    {
	    case L_OPERAND_MACRO:
	      new_oper = L_create_new_op_using (Lop_MOV, oper);
	      new_oper->proc_opc = TAHOEop_MOV_GR;
	      new_oper->src[0] = L_copy_operand (src1);
	      new_oper->dest[0] =
		L_new_register_operand (++(L_fn->max_reg_id), L_CTYPE_LLONG,
					0);
	      L_insert_oper_before (cb, oper, new_oper);
	      src1 = new_oper->dest[0];
	      /* No break here - must do both things */
	    case L_OPERAND_REGISTER:
	      dest_reg_id = L_table_imul (cb, oper, src1, src0);
	      if (!dest_reg_id)
		{
		  new_oper = L_create_new_op_using (Lop_MOV, oper);
		  new_oper->proc_opc = TAHOEop_MOVL;
		  new_oper->src[0] = L_copy_operand (src0);
		  new_oper->dest[0] =
		    L_new_register_operand (++(L_fn->max_reg_id),
					    L_CTYPE_LLONG, 0);
		  L_insert_oper_before (cb, oper, new_oper);
		  L_tahoe_int_register_multiply (cb, oper, src1,
						 new_oper->dest[0]);
		}
	      return;

	    default:
	      L_punt
		("L_annotate_int_multiply: unknown operand type 0x%x in %d",
		 src1->type, oper->id);
	    }
	}
    }
  else
    {				/* src0 not a contant integer */
      switch (src0->type)
	{
	case L_OPERAND_REGISTER:
	case L_OPERAND_MACRO:
	  if (L_is_variable (src1))
	    {
	      L_tahoe_int_register_multiply (cb, oper, src0, src1);
	      return;
	    }
	  else
	    L_punt
	      ("L_annotate_int_multiply: unknown operand type 0x%x in %d",
	       src1->type, oper->id);
	  break;

	case L_OPERAND_LABEL:
	  /* Load label into an integer register, 
	     then move to fp register */
	  new_oper = L_create_new_op_using (Lop_MOV, oper);
	  new_oper->proc_opc = TAHOEop_MOVI;
	  new_oper->src[0] = L_copy_operand (src0);
	  new_oper->dest[0] = L_new_register_operand (++(L_fn->max_reg_id),
						      L_CTYPE_LLONG, 0);
	  L_insert_oper_before (cb, oper, new_oper);
	  if (L_is_variable (src1))
	    {
	      L_tahoe_int_register_multiply (cb, new_oper, src0,
					     new_oper->dest[0]);
	    }
	  else
	    L_punt
	      ("L_annotate_int_multiply: unknown operand type 0x%x in %d",
	       src1->type, oper->id);
	  break;

	default:
	  L_punt ("L_annotate_int_multiply: unknown operand type 0x%x in %d",
		  src1->type, oper->id);
	}
    }
}



/*******************************************************************/
/*******************************************************************/


/* 09/12/02 REK Modifying function to use new TAHOEops. */
int
L_table_idiv (L_Cb * cb, L_Oper * oper,
	      L_Operand * src_reg, L_Operand * src_int)
{
  L_Oper *new_oper = NULL, *mul_oper, *shr_oper;
  char output[OUTPUT_LEN];
  int op0, op1, n;
  int cursor, status = 0;
  int result_num, result_reg_id[100];
  int new_reg_id, mul_dreg_id;
  ITuint32 umult = 0;
  ITint32 mult = 0;
  L_Attr *attr;

  /* Now call Steve's table code */

  if (oper->opc == Lop_DIV || oper->opc == Lop_REM)
    status = Recip_signed_idiv (output, &mult, src_int->value.i,
				OUTPUT_LEN, 0);
  else if (oper->opc == Lop_DIV_U || oper->opc == Lop_REM_U)
    status = Recip_unsigned_idiv (output, &umult, src_int->value.i,
				  OUTPUT_LEN, 0);
  else
    L_punt ("L_table_idiv: oper not div or div_u - oper %d", oper->id);

  /*   fprintf(stderr, "string=%s\n", output); */
  /*   fprintf(stderr, "umult=%x  mult=%x\n", umult, mult); */

  switch (status)
    {

    case 0:
      /* Remove F (non-excepting) flag from oper */
      oper->flags = L_CLR_BIT_FLAG (oper->flags, L_OPER_SAFE_PEI);

      cursor = 0;
      result_num = 1;
      result_reg_id[1] = src_reg->value.r;

      while (output[cursor])
	{
	  switch (output[cursor++])
	    {

	    case '+':
	      cursor++;		/* skip the blank */
	      op0 = output[cursor++] - '0';
	      cursor++;
	      op1 = output[cursor++] - '0';
	      cursor++;

	      /*      fprintf(stderr, "\t + %d %d\n", op0, op1); */

	      new_oper = L_create_new_op_using (Lop_ADD, oper);
	      new_oper->proc_opc = TAHOEop_ADD;
	      new_oper->src[0] = L_new_register_operand (result_reg_id[op0],
							 L_CTYPE_LLONG, 0);
	      new_oper->src[1] = L_new_register_operand (result_reg_id[op1],
							 L_CTYPE_LLONG, 0);
	      new_reg_id = ++(L_fn->max_reg_id);
	      new_oper->dest[0] = L_new_register_operand (new_reg_id,
							  L_CTYPE_LLONG, 0);
	      result_reg_id[++result_num] = new_reg_id;
	      L_insert_oper_before (cb, oper, new_oper);
	      break;

	    case '-':
	      cursor++;		/* skip the blank */
	      op0 = output[cursor++] - '0';
	      cursor++;
	      op1 = output[cursor++] - '0';
	      cursor++;

	      /*      fprintf(stderr, "\t - %d %d\n", op0, op1); */

	      new_oper = L_create_new_op_using (Lop_SUB, oper);
	      new_oper->proc_opc = TAHOEop_SUB;
	      new_oper->src[0] = L_new_register_operand (result_reg_id[op0],
							 L_CTYPE_LLONG, 0);
	      new_oper->src[1] = L_new_register_operand (result_reg_id[op1],
							 L_CTYPE_LLONG, 0);
	      new_reg_id = ++(L_fn->max_reg_id);
	      new_oper->dest[0] = L_new_register_operand (new_reg_id,
							  L_CTYPE_LLONG, 0);
	      result_reg_id[++result_num] = new_reg_id;
	      L_insert_oper_before (cb, oper, new_oper);
	      break;

	    case '>':
	      n = output[cursor++] - '0';
	      cursor++;
	      op0 = output[cursor++] - '0';
	      cursor++;

	      /*      fprintf(stderr, "\t > %d %d\n", op0,n); */

	      new_oper = L_create_new_op_using (Lop_LSR, oper);
	      new_oper->src[0] = L_new_register_operand (result_reg_id[op0],
							 L_CTYPE_LLONG, 0);
	      new_oper->src[1] = L_new_gen_int_operand (n);

	      if (UIMM_6 (n))
		{
		  new_oper->proc_opc = TAHOEop_EXTR_U;
		  new_oper->src[2] = L_new_gen_int_operand (64 - n);
		}
	      else
		{
		  new_oper->proc_opc = TAHOEop_SHR_U;
		}

	      new_reg_id = ++(L_fn->max_reg_id);
	      new_oper->dest[0] = L_new_register_operand (new_reg_id,
							  L_CTYPE_LLONG, 0);
	      result_reg_id[++result_num] = new_reg_id;
	      L_insert_oper_before (cb, oper, new_oper);
	      break;

	    case 'A':
	      n = output[cursor++] - '0';
	      cursor++;
	      op0 = output[cursor++] - '0';
	      cursor++;

	      /*      fprintf(stderr, "\t A %d %d\n", op0,n); */

	      new_oper = L_create_new_op_using (Lop_ASR, oper);
	      new_oper->src[0] = L_new_register_operand (result_reg_id[op0],
							 L_CTYPE_LLONG, 0);
	      new_oper->src[1] = L_new_gen_int_operand (n);

	      if (UIMM_6 (n))
		{
		  new_oper->proc_opc = TAHOEop_EXTR;
		  new_oper->src[2] = L_new_gen_int_operand (64 - n);
		}
	      else
		{
		  new_oper->proc_opc = TAHOEop_SHR;
		}

	      new_reg_id = ++(L_fn->max_reg_id);
	      new_oper->dest[0] = L_new_register_operand (new_reg_id,
							  L_CTYPE_LLONG, 0);
	      result_reg_id[++result_num] = new_reg_id;
	      L_insert_oper_before (cb, oper, new_oper);
	      break;

	    case 'N':
	      cursor++;		/* skip the blank */
	      op1 = output[cursor++] - '0';
	      cursor++;

	      /*      fprintf(stderr, "\t N %d\n", op1); */

	      new_oper = L_create_new_op_using (Lop_SUB, oper);
	      new_oper->proc_opc = TAHOEop_SUB;
	      new_oper->src[0] = L_new_macro_operand (TAHOE_MAC_ZERO,
						      L_CTYPE_LLONG, 0);
	      new_oper->src[1] = L_new_register_operand (result_reg_id[op1],
							 L_CTYPE_LLONG, 0);
	      new_reg_id = ++(L_fn->max_reg_id);
	      new_oper->dest[0] = L_new_register_operand (new_reg_id,
							  L_CTYPE_LLONG, 0);
	      result_reg_id[++result_num] = new_reg_id;
	      L_insert_oper_before (cb, oper, new_oper);
	      break;

	    case 'H':
	      /* NOTE: Used with Recip_signed_idiv !! (mult) */
	      cursor++;		/* skip the blank */
	      op0 = output[cursor++] - '0';
	      cursor++;


	      /*      fprintf(stderr, "\t * %d %d\n", op0, mult); */

	      mul_oper = L_create_new_op_using (Lop_MUL, oper);
	      mul_oper->src[0] = L_new_register_operand (result_reg_id[op0],
							 L_CTYPE_LLONG, 0);
	      mul_oper->src[1] = L_new_gen_int_operand (mult);
	      mul_dreg_id = ++(L_fn->max_reg_id);
	      mul_oper->dest[0] = L_new_register_operand (mul_dreg_id,
							  L_CTYPE_LLONG, 0);

	      L_insert_oper_before (cb, oper, mul_oper);
	      L_annotate_int_multiply (cb, mul_oper);

	      shr_oper = L_create_new_op_using (Lop_ASR, oper);
	      shr_oper->proc_opc = TAHOEop_EXTR;
	      shr_oper->src[0] = L_copy_operand (mul_oper->dest[0]);
	      shr_oper->src[1] = L_new_gen_int_operand (32);
	      shr_oper->src[2] = L_new_gen_int_operand (32);

	      new_reg_id = ++(L_fn->max_reg_id);
	      shr_oper->dest[0] = L_new_register_operand (new_reg_id,
							  L_CTYPE_LLONG, 0);
	      result_reg_id[++result_num] = new_reg_id;
	      L_insert_oper_before (cb, oper, shr_oper);

	      L_delete_oper (cb, mul_oper);

	      break;

	    case 'S':
	      cursor++;		/* skip the blank */
	      op0 = output[cursor++] - '0';
	      cursor++;

	      /*      fprintf(stderr, "\t * %d %d\n", op0, mult); */

	      new_oper = L_create_new_op_using (Lop_MUL, oper);
	      new_oper->src[0] = L_new_register_operand (result_reg_id[op0],
							 L_CTYPE_LLONG, 0);
	      new_oper->src[1] = L_new_gen_int_operand (mult);
	      new_reg_id = ++(L_fn->max_reg_id);
	      new_oper->dest[0] = L_new_register_operand (new_reg_id,
							  L_CTYPE_LLONG, 0);
	      result_reg_id[++result_num] = new_reg_id;
	      L_insert_oper_before (cb, oper, new_oper);
	      L_annotate_int_multiply (cb, new_oper);
	      L_delete_oper (cb, new_oper);
	      break;

	    case '%':
	      /* 02/22/01 JDM Does the same as 'H', but for unsigned (umult).
	         No idea what significance chars in this switch() have. ie.
	         Does '*' mean something?
	       */
	      cursor++;		/* skip the blank */
	      op0 = output[cursor++] - '0';
	      cursor++;

	      /*      fprintf(stderr, "\t * %d %d\n", op0, umult); */

	      mul_oper = L_create_new_op_using (Lop_MUL_U, oper);
	      mul_oper->src[0] = L_new_register_operand (result_reg_id[op0],
							 L_CTYPE_LLONG, 0);
	      mul_oper->src[1] = L_new_gen_int_operand (umult);
	      mul_dreg_id = ++(L_fn->max_reg_id);
	      mul_oper->dest[0] = L_new_register_operand (mul_dreg_id,
							  L_CTYPE_LLONG, 0);

	      L_insert_oper_before (cb, oper, mul_oper);
	      L_annotate_int_multiply (cb, mul_oper);

	      /* Get upper 32 bits */
	      shr_oper = L_create_new_op_using (Lop_ASR, oper);
	      shr_oper->proc_opc = TAHOEop_EXTR_U;
	      shr_oper->src[0] = L_copy_operand (mul_oper->dest[0]);
	      shr_oper->src[1] = L_new_gen_int_operand (32);
	      shr_oper->src[2] = L_new_gen_int_operand (32);
	      new_reg_id = ++(L_fn->max_reg_id);
	      shr_oper->dest[0] = L_new_register_operand (new_reg_id,
							  L_CTYPE_LLONG, 0);
	      result_reg_id[++result_num] = new_reg_id;
	      L_insert_oper_before (cb, oper, shr_oper);

	      L_delete_oper (cb, mul_oper);

	      break;


	    case '*':
	      cursor++;		/* skip the blank */
	      op0 = output[cursor++] - '0';
	      cursor++;

	      /*      fprintf(stderr, "\t * %d 0x%x\n", op0, umult); */

	      new_oper = L_create_new_op_using (Lop_MUL_U, oper);
	      new_oper->src[0] = L_new_register_operand (result_reg_id[op0],
							 L_CTYPE_LLONG, 0);
	      new_oper->src[1] = L_new_gen_int_operand (umult);
	      new_reg_id = ++(L_fn->max_reg_id);
	      new_oper->dest[0] = L_new_register_operand (new_reg_id,
							  L_CTYPE_LLONG, 0);
	      result_reg_id[++result_num] = new_reg_id;

	      attr = L_new_attr ("no-sign-extend", 0);
	      new_oper->attr = L_concat_attr (new_oper->attr, attr);

	      L_insert_oper_before (cb, oper, new_oper);
	      L_annotate_int_multiply (cb, new_oper);
	      L_delete_oper (cb, new_oper);
	      break;

	    default:
	      L_punt ("Unknown idiv case in oper %d\n", oper->id);
	    }
	}
      L_delete_operand (new_oper->dest[0]);
      L_fn->max_reg_id--;
      new_oper->dest[0] = L_copy_operand (oper->dest[0]);
      return (new_oper->dest[0]->value.r);

    case 2:
      L_punt ("L_table_idiv: String size too small for oper %d", oper->id);
      return (0);

    default:
      fprintf (stderr, "Table lookup didn't work for " ITintmaxformat
	       " int oper %d", src_int->value.i, oper->id);
      return (0);
    }
}



void
L_annotate_int_complex_division (L_Cb * cb, L_Oper * oper)
{
  L_Oper *conv_op0;
  L_Oper *conv_op1;
  L_Oper *conv_op2;
  L_Oper *ext_op0;
  L_Oper *ext_op1;
  L_Oper *conv_back;
  L_Oper *sub_op = NULL;
  L_Oper *add_op;
  L_Oper *division_op;
  L_Oper *rem_op;
  L_Oper *mov_op;
  L_Operand *quotient = oper->dest[0];
  L_Operand *dividend = oper->src[0];
  L_Operand *divisor = oper->src[1];
  L_Operand *new_dividend = NULL;
  int unsigned_conv = (oper->opc == Lop_REM_U || oper->opc == Lop_DIV_U);

  if (!(L_is_variable (dividend)))
    {
      L_print_oper (stderr, oper);
      L_punt ("L_annotate_int_complex_division: illegal source0 in oper: %d",
	      oper->id);
    }

  if (!(L_is_variable (divisor)))
    {
      L_print_oper (stderr, oper);
      L_punt ("L_annotate_int_complex_division: illegal source1 in oper: %d",
	      oper->id);
    }

  switch (oper->com[0])
    {
    case L_CTYPE_CHAR:
    case L_CTYPE_UCHAR:
      ext_op0 = Ltahoe_extend (!unsigned_conv, oper, oper->src[0], NULL, 1);
      ext_op1 = Ltahoe_extend (!unsigned_conv, oper, oper->src[1], NULL, 1);
      break;
    case L_CTYPE_SHORT:
    case L_CTYPE_USHORT:
      ext_op0 = Ltahoe_extend (!unsigned_conv, oper, oper->src[0], NULL, 2);
      ext_op1 = Ltahoe_extend (!unsigned_conv, oper, oper->src[1], NULL, 2);
      break;
    case L_CTYPE_INT:
    case L_CTYPE_UINT:
      ext_op0 = Ltahoe_extend (!unsigned_conv, oper, oper->src[0], NULL, 4);
      ext_op1 = Ltahoe_extend (!unsigned_conv, oper, oper->src[1], NULL, 4);
      break;
    case 0:
    case L_CTYPE_LONG:
    case L_CTYPE_ULONG:
    case L_CTYPE_LLONG:
    case L_CTYPE_ULLONG:
    default:
      ext_op0 = NULL;
      ext_op1 = NULL;
      break;
    }

  if (ext_op0)
    {
      L_insert_oper_before (cb, oper, ext_op0);
      dividend = ext_op0->dest[0];
    }

  if (ext_op1)
    {
      L_insert_oper_before (cb, oper, ext_op1);
      divisor = ext_op1->dest[0];
    }

  /* The operands are now guaranteed to be registers or macros  */
  /* Integer division can be annotated by calls to:              */
  /* L_annotate_int_to_float, L_annotate_float_divison, L_annotate_float_to_int  */

  conv_op0 = L_create_new_op_using (Lop_I_F, oper);
  conv_op0->src[0] = L_copy_operand (dividend);
  conv_op0->dest[0] =
    L_new_register_operand (++L_fn->max_reg_id, L_CTYPE_DOUBLE, 0);
  L_insert_oper_before (cb, oper, conv_op0);
  L_annotate_int_to_float (cb, conv_op0, unsigned_conv);

  conv_op1 = L_create_new_op_using (Lop_I_F, oper);
  conv_op1->src[0] = L_copy_operand (divisor);
  conv_op1->dest[0] =
    L_new_register_operand (++L_fn->max_reg_id, L_CTYPE_DOUBLE, 0);
  L_insert_oper_before (cb, oper, conv_op1);
  L_annotate_int_to_float (cb, conv_op1, unsigned_conv);

  if (L_int_rem_opcode (oper))
    {
      sub_op = L_create_new_op_using (Lop_SUB, oper);
      sub_op->src[0] = Ltahoe_IMAC (ZERO);
      sub_op->src[1] = L_copy_operand (divisor);
      sub_op->dest[0] =
	L_new_register_operand (++L_fn->max_reg_id, L_CTYPE_LLONG, 0);
      L_insert_oper_before (cb, oper, sub_op);
      L_annotate_int_sub (cb, sub_op);
    }

  division_op = L_create_new_op_using (Lop_DIV_F2, oper);
  division_op->src[0] = L_copy_operand (conv_op0->dest[0]);
  division_op->src[1] = L_copy_operand (conv_op1->dest[0]);
  division_op->dest[0] =
    L_new_register_operand (++L_fn->max_reg_id, L_CTYPE_DOUBLE, 0);
  L_insert_oper_before (cb, oper, division_op);

  switch (oper->com[0])
    {
    case L_CTYPE_CHAR:
    case L_CTYPE_UCHAR:
      Ltahoe_annotate_EM_int_divide8_thr (cb, division_op);
      break;
    case L_CTYPE_SHORT:
    case L_CTYPE_USHORT:
      Ltahoe_annotate_EM_int_divide16_thr (cb, division_op);
      break;
    case L_CTYPE_INT:
    case L_CTYPE_UINT:
      Ltahoe_annotate_EM_int_divide32 (cb, division_op);
      break;
    case 0:
    case L_CTYPE_LONG:
    case L_CTYPE_ULONG:
    case L_CTYPE_LLONG:
    case L_CTYPE_ULLONG:
    default:
      if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_SOFTPIPE))
	Ltahoe_annotate_EM_int_divide64_thr (cb, division_op);
      else
	Ltahoe_annotate_EM_int_divide64_lat (cb, division_op);
    }

  conv_back = Ltahoe_new_fcvt_fx_trunc (oper->pred[0], division_op->dest[0],
					division_op->dest[0], FSF_S0, oper);
  L_insert_oper_before (cb, oper, conv_back);

  if (L_int_rem_opcode (oper))
    {
      conv_op2 = Ltahoe_new_setf_sig (oper->pred[0], division_op->src[1],
				      sub_op->dest[0], oper);
      L_insert_oper_before (cb, oper, conv_op2);

      rem_op = Ltahoe_new_xma_l (oper->pred[0], division_op->dest[0],
				 division_op->dest[0], division_op->src[1],
				 Ltahoe_FMAC (FZERO), oper);
      L_insert_oper_before (cb, oper, rem_op);

      L_delete_oper (cb, sub_op);

      /* About to overwrite the quotient register.  This register
         MAY be the same as the dividend register, so move 
         the divident to a temporary register. MCM 7/2001 */
      if (L_same_operand (quotient, dividend))
	{
	  new_dividend = L_new_register_operand (++L_fn->max_reg_id,
						 L_CTYPE_LLONG, L_PTYPE_NULL);

	  mov_op = L_create_new_op_using (Lop_MOV, oper);
	  mov_op->proc_opc = TAHOEop_MOV_GR;
	  mov_op->dest[0] = L_copy_operand (new_dividend);
	  mov_op->src[0] = L_copy_operand (dividend);
	  L_insert_oper_before (cb, oper, mov_op);
	}
      else
	{
	  new_dividend = L_copy_operand (dividend);
	}
    }

  conv_back = Ltahoe_new_getf_sig (oper->pred[0], quotient,
				   division_op->dest[0], oper);
  L_insert_oper_before (cb, oper, conv_back);

  if (L_int_rem_opcode (oper))
    {
      add_op = L_create_new_op_using (Lop_ADD, oper);
      add_op->proc_opc = TAHOEop_ADD;
      add_op->src[0] = L_copy_operand (conv_back->dest[0]);
      add_op->src[1] = L_copy_operand (new_dividend);
      add_op->dest[0] = L_copy_operand (quotient);
      L_insert_oper_before (cb, oper, add_op);
      L_delete_operand (new_dividend);
    }

  L_delete_oper (cb, conv_op0);
  L_delete_oper (cb, conv_op1);
  L_delete_oper (cb, division_op);
}

void
L_annotate_int_divide (L_Cb * cb, L_Oper * oper)
{
  /* Table division can't handle division by zero */

  if (L_is_int_constant (oper->src[1]) && (oper->src[1]->value.i == 0))
    {
      L_delete_operand (oper->src[1]);
      oper->src[1] = L_new_macro_operand (TAHOE_MAC_ZERO, L_CTYPE_LLONG,
					  L_PTYPE_NULL);
    }

  /* constant / constant
   * ----------------------------------------------------------------------
   * generate constant move
   */

  if (L_is_int_constant (oper->src[0]) && L_is_int_constant (oper->src[1]))
    {
      ITint64 constant;
      L_Operand *const_operand;

      if (L_int_rem_opcode (oper))
	constant = oper->src[0]->value.i % oper->src[1]->value.i;
      else
	constant = oper->src[0]->value.i / oper->src[1]->value.i;

      const_operand = L_new_gen_int_operand (constant);
      Ltahoe_int_constant_load (cb, oper, const_operand, oper->dest[0]);
      L_delete_operand (const_operand);
    }

  /* variable / constant
   * ----------------------------------------------------------------------
   * generate table division
   */

  else if (L_is_int_constant (oper->src[1]))
    {
      int delayed_insert = 0, dest_reg_id;
      L_Oper *new_oper;
      L_Oper *mov_oper = NULL;

      /* table idiv can't handle same oper with same src as dest */
      if (L_is_macro (oper->dest[0]) ||
	   L_same_operand (oper->dest[0], oper->src[0]))
	{
	  /* Have to insert in order, so make the change to oper and */
	  /* mark as needed to insert mov oper before it after call  */
	  /* to l_table_idiv -- 10/30/97 MBT                         */
	  mov_oper = L_create_new_op_using (Lop_MOV, oper);
	  mov_oper->proc_opc = TAHOEop_MOV_GR;
	  mov_oper->src[0] = L_new_register_operand (++(L_fn->max_reg_id),
						     L_CTYPE_LLONG, 0);
	  mov_oper->dest[0] = L_copy_operand (oper->dest[0]);
	  L_delete_operand (oper->dest[0]);
	  oper->dest[0] = L_copy_operand (mov_oper->src[0]);
	  delayed_insert = 1;
	}

      if (L_is_variable (oper->src[0]))
	{
	  L_Operand *dividend = oper->src[0];

	  if (L_is_macro (oper->src[0]))
	    {
	      new_oper = L_create_new_op_using (Lop_MOV, oper);
	      new_oper->proc_opc = TAHOEop_MOV_GR;
	      new_oper->src[0] = L_copy_operand (oper->src[0]);
	      new_oper->dest[0] =
		L_new_register_operand (++(L_fn->max_reg_id), L_CTYPE_LLONG,
					0);
	      L_insert_oper_before (cb, oper, new_oper);
	      dividend = new_oper->dest[0];
	    }

	  /* Do an optimization for divide by -1 */

	  if (oper->src[1]->value.i == -1)
	    {
	      new_oper = L_create_new_op_using (Lop_SUB, oper);
	      new_oper->src[0] =
		L_new_macro_operand (TAHOE_MAC_ZERO, L_CTYPE_LLONG,
				     L_PTYPE_NULL);
	      new_oper->src[1] = L_copy_operand (dividend);
	      new_oper->dest[0] = L_copy_operand (oper->dest[0]);
	      new_oper->proc_opc = TAHOEop_SUB;
	      L_insert_oper_before (cb, oper, new_oper);
	    }
	  else
	    {
	      if (!(dest_reg_id =
		    L_table_idiv (cb, oper, dividend, oper->src[1])))
		L_punt ("Bad idiv: %d\n", oper->id);

	    }

	  if (L_int_rem_opcode (oper))
	    {
	      new_oper = L_create_new_op_using (Lop_MUL, oper);
	      new_oper->src[0] = L_copy_operand (oper->dest[0]);
	      new_oper->src[1] = L_copy_operand (oper->src[1]);
	      new_oper->dest[0] = L_copy_operand (oper->dest[0]);
	      L_insert_oper_before (cb, oper, new_oper);
	      L_annotate_int_multiply (cb, new_oper);
	      L_delete_oper (cb, new_oper);

	      new_oper = L_create_new_op_using (Lop_SUB, oper);
	      new_oper->proc_opc = TAHOEop_SUB;
	      new_oper->src[0] = L_copy_operand (dividend);
	      new_oper->src[1] = L_copy_operand (oper->dest[0]);
	      new_oper->dest[0] = L_copy_operand (oper->dest[0]);
	      L_insert_oper_before (cb, oper, new_oper);
	      L_annotate_int_sub (cb, new_oper);
	      L_delete_oper (cb, new_oper);
	    }
	}
      else
	{
	  L_punt ("L_annotate_int_divide - unimplemented src0 type oper %d\n",
		  oper->id);
	}

      /* can only insert before oper, so have to delay this - MBT 10/30/97 */

      if (delayed_insert)
	{
	  L_insert_oper_before (cb, oper, mov_oper);
	}

    }

  /* constant / variable
   * ----------------------------------------------------------------------
   * perform constant load; generate complex division
   */

  else if (L_is_int_constant (oper->src[0]))
    {
      L_Operand *new_src0;

      new_src0 =
	L_new_register_operand (++(L_fn->max_reg_id), L_CTYPE_LLONG, 0);
      Ltahoe_int_constant_load (cb, oper, oper->src[0], new_src0);
      L_delete_operand (oper->src[0]);
      oper->src[0] = new_src0;
      L_annotate_int_complex_division (cb, oper);
    }

  /* variable / variable
   * ----------------------------------------------------------------------
   * generate complex division
   */

  else
    {
      L_annotate_int_complex_division (cb, oper);
    }
  return;
}

void
L_annotate_int_remainder (L_Cb * cb, L_Oper * oper)
{
  L_Operand *const_operand;
  int constant;

  if (L_is_int_constant (oper->src[0]) && L_is_int_constant (oper->src[1]))
    {
      constant = oper->src[0]->value.i % oper->src[1]->value.i;
      const_operand = L_new_gen_int_operand (constant);
      Ltahoe_int_constant_load (cb, oper, const_operand, oper->dest[0]);
      L_delete_operand (const_operand);
      return;
    }

  if (L_is_int_one (oper->src[1]))
    {
      const_operand = L_new_gen_int_operand (0);
      Ltahoe_int_constant_load (cb, oper, const_operand, oper->dest[0]);
      L_delete_operand (const_operand);
      return;
    }

  L_annotate_int_divide (cb, oper);
}
