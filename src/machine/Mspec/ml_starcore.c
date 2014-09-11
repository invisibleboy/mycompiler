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
 *  	  File :	ml_starcore.c 
 * Description : 	Machine dependent specification.  
 *     Authors :        Christopher Shannon
 *
 *==========================================================================*/

/*****************************************************************************\
 * NOTICE OF CONVENTION                                                      *
 * ------------------------------------------------------------------------- *
 * Mspec links to Pcode, Hcode, and Lcode modules.  In order to allow this   *
 * to take place without requiring front-end modules to link to liblcode.a,  *
 * Mspec code is divided into two classes as follows:                        *
 *  - mi_*.c must not depend on linkage to liblcode.a                        *
 *  - ml_*.c may depend on linkage to liblcode.a                             *
\*****************************************************************************/

#include  <config.h>
#include  <Lcode/l_main.h>
#include  "m_spec.h"
#include  "m_starcore.h"

Set Set_starcore_fragile_macro = NULL;

/*---------------------------------------------------------------------------*/

/* This function specifies the macro registers that are assumed to be killed */
/* by jsr's, so returning a 1 means killed. */

/* output and return params are fragile (can't be moved over function call)  */

int
M_starcore_fragile_macro (int macro_value)
{

  if (EM_SC_MODEL_OK (M_EM_SC_model))
    {
      switch (macro_value)
	{
	case L_MAC_P0:
	case L_MAC_P1:
	case L_MAC_P2:
	case L_MAC_P3:
	case L_MAC_P4:
	case L_MAC_P5:
	case L_MAC_P6:
	case L_MAC_P7:

	  /* Integer output parameters */
	case L_MAC_P8:
	case L_MAC_P9:
	case L_MAC_P10:
	case L_MAC_P11:
	case L_MAC_P12:
	case L_MAC_P13:
	case L_MAC_P14:
	case L_MAC_P15:

	  /* Integer return parameters */
	case L_MAC_P16:
	case L_MAC_P17:
	case L_MAC_P18:
	case L_MAC_P19:

	  /* FP input/output parameters */
	case L_MAC_P20:
	case L_MAC_P21:
	case L_MAC_P22:
	case L_MAC_P23:
	case L_MAC_P24:
	case L_MAC_P25:
	case L_MAC_P26:
	case L_MAC_P27:

	  /* FP return parameters */
	case L_MAC_P28:
	case L_MAC_P29:
	case L_MAC_P30:
	case L_MAC_P31:

	case STARCORE_MAC_T_BIT:
	case STARCORE_MAC_T_BIT_INV:
	case STARCORE_MAC_SR:

	case L_MAC_OP:
	  return (1);

	default:
	  return (0);
	}
    }
  else
    {
      M_assert (0, "M_starcore_fragile_macro: illegal machine model");
      return (0);
    }
}


Set
M_starcore_fragile_macro_set (void)
{
  if (Set_starcore_fragile_macro)
    {
      return Set_starcore_fragile_macro;
    }
  Set_starcore_fragile_macro = Set_add (Set_starcore_fragile_macro, L_MAC_P0);
  Set_starcore_fragile_macro = Set_add (Set_starcore_fragile_macro, L_MAC_P1);
  Set_starcore_fragile_macro = Set_add (Set_starcore_fragile_macro, L_MAC_P2);
  Set_starcore_fragile_macro = Set_add (Set_starcore_fragile_macro, L_MAC_P3);
  Set_starcore_fragile_macro = Set_add (Set_starcore_fragile_macro, L_MAC_P4);
  Set_starcore_fragile_macro = Set_add (Set_starcore_fragile_macro, L_MAC_P5);
  Set_starcore_fragile_macro = Set_add (Set_starcore_fragile_macro, L_MAC_P6);
  Set_starcore_fragile_macro = Set_add (Set_starcore_fragile_macro, L_MAC_P7);
  Set_starcore_fragile_macro = Set_add (Set_starcore_fragile_macro, L_MAC_P8);
  Set_starcore_fragile_macro = Set_add (Set_starcore_fragile_macro, L_MAC_P9);
  Set_starcore_fragile_macro = Set_add (Set_starcore_fragile_macro, L_MAC_P10);
  Set_starcore_fragile_macro = Set_add (Set_starcore_fragile_macro, L_MAC_P11);
  Set_starcore_fragile_macro = Set_add (Set_starcore_fragile_macro, L_MAC_P12);
  Set_starcore_fragile_macro = Set_add (Set_starcore_fragile_macro, L_MAC_P13);
  Set_starcore_fragile_macro = Set_add (Set_starcore_fragile_macro, L_MAC_P14);
  Set_starcore_fragile_macro = Set_add (Set_starcore_fragile_macro, L_MAC_P15);
  Set_starcore_fragile_macro = Set_add (Set_starcore_fragile_macro, L_MAC_P16);
  Set_starcore_fragile_macro = Set_add (Set_starcore_fragile_macro, L_MAC_P17);
  Set_starcore_fragile_macro = Set_add (Set_starcore_fragile_macro, L_MAC_P18);
  Set_starcore_fragile_macro = Set_add (Set_starcore_fragile_macro, L_MAC_P19);
  Set_starcore_fragile_macro = Set_add (Set_starcore_fragile_macro, L_MAC_P20);
  Set_starcore_fragile_macro = Set_add (Set_starcore_fragile_macro, L_MAC_P21);
  Set_starcore_fragile_macro = Set_add (Set_starcore_fragile_macro, L_MAC_P22);
  Set_starcore_fragile_macro = Set_add (Set_starcore_fragile_macro, L_MAC_P23);
  Set_starcore_fragile_macro = Set_add (Set_starcore_fragile_macro, L_MAC_P24);
  Set_starcore_fragile_macro = Set_add (Set_starcore_fragile_macro, L_MAC_P25);
  Set_starcore_fragile_macro = Set_add (Set_starcore_fragile_macro, L_MAC_P26);
  Set_starcore_fragile_macro = Set_add (Set_starcore_fragile_macro, L_MAC_P27);
  Set_starcore_fragile_macro = Set_add (Set_starcore_fragile_macro, L_MAC_P28);
  Set_starcore_fragile_macro = Set_add (Set_starcore_fragile_macro, L_MAC_P29);
  Set_starcore_fragile_macro = Set_add (Set_starcore_fragile_macro, L_MAC_P30);
  Set_starcore_fragile_macro = Set_add (Set_starcore_fragile_macro, L_MAC_P31);
  Set_starcore_fragile_macro = Set_add (Set_starcore_fragile_macro, L_MAC_OP);
  Set_starcore_fragile_macro = Set_add (Set_starcore_fragile_macro, 
					STARCORE_MAC_T_BIT);
  Set_starcore_fragile_macro = Set_add (Set_starcore_fragile_macro, 
					STARCORE_MAC_T_BIT_INV);
  Set_starcore_fragile_macro = Set_add (Set_starcore_fragile_macro, 
					STARCORE_MAC_SR);

  return Set_starcore_fragile_macro;
}


/****************************************************************************
 *
 * routine: M_starcore_extra_pred_define_opcode()
 * purpose: Determine if the proc_opc opcode generates a predicate.
 *          Only machine specific predicate generating opcodes above and
 *          beyond the normal pred defines, clears, and stores need to be
 *          listed.
 * input:
 * output:
 * returns: 1 if the opcode generates a predicate.  0 otherwise
 * modified: Bob McGowan, 9/11/96 created
 *           Mark Tozer   5/30/97 added cmp4_ne_and for pred_clear
 * note:
 *       Add new opcodes to one of the 
 *           M_starcore_extra_pred_define_typeX functions, too (mbt)
 *-------------------------------------------------------------------------*/

int
M_starcore_extra_pred_define_opcode (int proc_opc)
{

  switch (proc_opc)
    {
    case STARCOREop_CMPEQ:
    case STARCOREop_CMPGT:
    case STARCOREop_CMPHI:
    case STARCOREop_DECEQ:
    case STARCOREop_DECGE:
    case STARCOREop_TSTEQ:
    case STARCOREop_TSTGE:
    case STARCOREop_TSTGT:
    case STARCOREop_CMPEQA:
    case STARCOREop_CMPGTA:
    case STARCOREop_CMPHIA:
    case STARCOREop_DECEQA:
    case STARCOREop_DECGEA:
    case STARCOREop_TSTEQA_L:
    case STARCOREop_TSTEQA_W:
    case STARCOREop_TSTGEA:
    case STARCOREop_TSTGTA:
    case STARCOREop_BMTSTC:
    case STARCOREop_BMTSTC_W_2:
    case STARCOREop_BMTSTC_W_3:
    case STARCOREop_BMTSTS:
    case STARCOREop_BMTSTS_W_2:
    case STARCOREop_BMTSTS_W_3:
      return (1);
    default:
      return (0);
    }
}

/************************************************************************/
/* predicate-defining opcodes where both dests are pred regs (i.e. cmpeq)*/
/************************************************************************/

int
M_starcore_extra_pred_define_type1 (L_Oper * oper)
{
  switch (oper->proc_opc)
    {
    case STARCOREop_CMPEQ:
    case STARCOREop_CMPGT:
    case STARCOREop_CMPHI:
    case STARCOREop_TSTEQ:
    case STARCOREop_TSTGE:
    case STARCOREop_TSTGT:
    case STARCOREop_CMPEQA:
    case STARCOREop_CMPGTA:
    case STARCOREop_CMPHIA:
    case STARCOREop_TSTEQA_L:
    case STARCOREop_TSTEQA_W:
    case STARCOREop_TSTGEA:
    case STARCOREop_TSTGTA:
    case STARCOREop_BMTSTC:
    case STARCOREop_BMTSTC_W_2:
    case STARCOREop_BMTSTC_W_3:
    case STARCOREop_BMTSTS:
    case STARCOREop_BMTSTS_W_2:
    case STARCOREop_BMTSTS_W_3:
      return (1);

    default:
      return (0);
    }
}



/********************************************************************/
/* predicate-defining opcodes where dest[1] is a pred. (i.e. frcpa) */
/********************************************************************/

int
M_starcore_extra_pred_define_type2 (L_Oper * oper)
{

  switch (oper->proc_opc)
    {
    case STARCOREop_DECEQ:
    case STARCOREop_DECGE:
    case STARCOREop_DECEQA:
    case STARCOREop_DECGEA:
      return (1);

    default:
      return (0);
    }
}



/*--------------------------------------------------------------------------*/

int
M_starcore_subroutine_call (int opc)
{

  if (EM_SC_MODEL_OK (M_EM_SC_model))
    {
      switch (opc)
	{
	case Lop_JSR:
	case Lop_JSR_FS:
	  /* Library calls */
	case Lop_DIV:
	case Lop_DIV_U:
	case Lop_REM:
	case Lop_REM_U:
	case Lop_ABS_F:
	case Lop_ABS_F2:
	case Lop_ADD_F:
	case Lop_ADD_F2:
	case Lop_DIV_F:
	case Lop_DIV_F2:
	case Lop_MUL_F:
	case Lop_MUL_F2:
	case Lop_RCMP_F:
	case Lop_SUB_F:
	case Lop_SUB_F2:
	case Lop_F_I:
	case Lop_F2_I:
	case Lop_I_F:
	case Lop_I_F2:
	case Lop_BR_F:
	  return 1;
	default:
	  return 0;
	}
    }
  else
    {
      M_assert (0, "M_starcore_subroutine_call:  Illegal model specified!");
      return (0);
    }
}

/*--------------------------------------------------------------------------*/


/* return 1 for macros that should be included in dataflow analysis */

int
M_starcore_dataflow_macro (int id)
{

  if (EM_SC_MODEL_OK (M_EM_SC_model))
    {
      switch (id)
	{
	case STARCORE_MAC_TEMPLATE:
	case STARCORE_MAC_LABEL:
	  return FALSE;
	default:
	  return TRUE;
	}
    }
  else
    {
      M_assert (0, "M_starcore_dataflow_macro: illegal machine model");
      return (0);
    }
}

/*
 * Declare code generator specific macro registers to the front end parser.
 */

void
M_define_macros_starcore (STRING_Symbol_Table * sym_tbl)
{

  if (EM_SC_MODEL_OK (M_EM_SC_model))
    {
      M_add_symbol (sym_tbl, "sp", STARCORE_MAC_SP);
      M_add_symbol (sym_tbl, "sr", STARCORE_MAC_SR);
      M_add_symbol (sym_tbl, "tmp", STARCORE_MAC_TEMP);
      M_add_symbol (sym_tbl, "Template", STARCORE_MAC_TEMPLATE);
      M_add_symbol (sym_tbl, "Label", STARCORE_MAC_LABEL);

      /* 1 if leaf function, 0 if non-leaf */
      M_add_symbol (sym_tbl, "$leaf", STARCORE_MAC_LEAF);
      /* LV space for int spill around sync */
      M_add_symbol (sym_tbl, "$sync_size", STARCORE_MAC_SYNC_SIZE);
      /* total alloc requirements */
      M_add_symbol (sym_tbl, "$mem_alloc_size", STARCORE_MAC_MEM_ALLOC);
      M_add_symbol (sym_tbl, "t", STARCORE_MAC_T_BIT);
      M_add_symbol (sym_tbl, "t'", STARCORE_MAC_T_BIT_INV);
    }
  else
    {
      M_assert (0, "M_define_macros_starcore: illegal machine model");
    }
}

char *
M_get_macro_name_starcore (int id)
{

  if (EM_SC_MODEL_OK (M_EM_SC_model))
    {
      switch (id)
	{
	case STARCORE_MAC_SP:
	  return "sp";
	case STARCORE_MAC_SR:
	  return "sr";
	case STARCORE_MAC_TEMP:
	  return "tmp";
	case STARCORE_MAC_LEAF:
	  return "$leaf";
	case STARCORE_MAC_SYNC_SIZE:
	  return ("$sync_size");
	case STARCORE_MAC_MEM_ALLOC:
	  return "$mem_alloc_size";
	case STARCORE_MAC_TEMPLATE:
	  return "Template";
	case STARCORE_MAC_LABEL:
	  return "Label";
	case STARCORE_MAC_T_BIT:
	  return "t";
	case STARCORE_MAC_T_BIT_INV:
	  return "t'";
	default:
	  return "???";
	}
    }
  else
    {
      M_assert (0, "M_get_macro_name_starcore: illegal machine model");
      return (0);
    }
}


/*--------------------------------------------------------------------------*/
/*
 * Return true (1) if the instruction is supported in the hardware of the
 * processor.  Return false (0) otherwise.
 */
int
M_oper_supported_in_arch_starcore (int opc)
{

  if (EM_SC_MODEL_OK (M_EM_SC_model))
    {
      switch (opc)
	{
	  /* Divides implemented in software */
	case Lop_DIV:
	case Lop_DIV_U:
	case Lop_REM:
	case Lop_REM_U:

	case Lop_MUL_SUB:
	case Lop_MUL_SUB_U:
	case Lop_MUL_SUB_REV:
	case Lop_MUL_SUB_REV_U:
	case Lop_NOR:
	case Lop_NAND:
	case Lop_NXOR:
	case Lop_OR_NOT:
	case Lop_AND_NOT:
	case Lop_OR_COMPL:
	case Lop_AND_COMPL:
	case Lop_REV:
	case Lop_BIT_POS:

	  /* Floating point implemented in software */
	case Lop_F2_I:
	case Lop_I_F2:
	case Lop_F_I:
	case Lop_I_F:
	case Lop_F2_F:
	case Lop_F_F2:

	  /* No Pre- or Post-increment loads or stores */
	case Lop_LD_PRE_UC:
	case Lop_LD_PRE_C:
	case Lop_LD_PRE_UC2:
	case Lop_LD_PRE_C2:
	case Lop_LD_PRE_UI:
	case Lop_LD_PRE_I:
	case Lop_LD_PRE_Q:
	case Lop_LD_PRE_F:
	case Lop_LD_PRE_F2:
	case Lop_LD_POST_UC:
	case Lop_LD_POST_C:
	case Lop_LD_POST_UC2:
	case Lop_LD_POST_C2:
	case Lop_LD_POST_UI:
	case Lop_LD_POST_I:
	case Lop_LD_POST_Q:
	case Lop_LD_POST_F:
	case Lop_LD_POST_F2:
	case Lop_ST_PRE_C:
	case Lop_ST_PRE_C2:
	case Lop_ST_PRE_I:
	case Lop_ST_PRE_Q:
	case Lop_ST_PRE_F:
	case Lop_ST_PRE_F2:
	case Lop_ST_POST_C:
	case Lop_ST_POST_C2:
	case Lop_ST_POST_I:
	case Lop_ST_POST_Q:
	case Lop_ST_POST_F:
	case Lop_ST_POST_F2:

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
	case Lop_CHECK:
	case Lop_CONFIRM:

	  /* No explicit predicate manipulation operations */
	case Lop_PRED_CLEAR:
	case Lop_PRED_SET:
	case Lop_PRED_LD:
	case Lop_PRED_ST:
	case Lop_PRED_LD_BLK:
	case Lop_PRED_ST_BLK:
	case Lop_PRED_MERGE:
	case Lop_PRED_AND:
	case Lop_PRED_COMPL:
	case Lop_PRED_COPY:
	case Lop_PRED_MASK_AND:
	case Lop_PRED_MASK_OR:

	case Lop_SELECT:
	case Lop_PREF_LD:
	case Lop_EXPAND:

	  /* No floating point support - implemented in software */
	case Lop_BR_F:
	case Lop_MOV_F:
	case Lop_MOV_F2:
	case Lop_ADD_F2:
	case Lop_SUB_F2:
	case Lop_MUL_F2:
	case Lop_DIV_F2:
	case Lop_RCP_F2:
	case Lop_ABS_F2:
	case Lop_MUL_ADD_F2:
	case Lop_MUL_SUB_F2:
	case Lop_MUL_SUB_REV_F2:
	case Lop_SQRT_F2:
	case Lop_MAX_F2:
	case Lop_MIN_F2:
	case Lop_ADD_F:
	case Lop_SUB_F:
	case Lop_MUL_F:
	case Lop_DIV_F:
	case Lop_RCP_F:
	case Lop_ABS_F:
	case Lop_MUL_ADD_F:
	case Lop_MUL_SUB_F:
	case Lop_MUL_SUB_REV_F:
	case Lop_SQRT_F:
	case Lop_MAX_F:
	case Lop_MIN_F:
	case Lop_LD_F:
	case Lop_LD_F2:
	case Lop_ST_F:
	case Lop_ST_F2:
	case Lop_CMP_F:
	case Lop_RCMP_F:
	case Lop_CMOV_F:
	case Lop_CMOV_COM_F:
	case Lop_CMOV_F2:
	case Lop_CMOV_COM_F2:
	case Lop_SELECT_F:
	case Lop_SELECT_F2:
	  return (0);

	default:
	  return (1);
	}
    }
  else
    {
      M_assert (0, "M_oper_supported_in_arch_starcore: illegal machine model");
      return (0);
    }
}


/*
 * Returns the number of machine instructions required to implement the
 * specified oper in the best case.  It is assumed that this is for
 * supported instructions.  A call to M_oper_supported_in_arch should be
 * made for abnormal instructions.
 */
#define zero_offset(a)   ((L_is_int_constant((a)->src[1])) && \
                          ((a)->src[1]->value.i == 0))
#define label_base(a)    (L_is_label((a)->src[0]))

int
M_num_oper_required_for_starcore (L_Oper * oper, char *name)
{

  if (EM_SC_MODEL_OK (M_EM_SC_model))
    {
      switch (oper->opc)
	{
	case Lop_LD_UC:
	case Lop_LD_C:
	case Lop_LD_UC2:
	case Lop_LD_C2:
	case Lop_LD_I:
	case Lop_LD_UI:
	case Lop_LD_F:
	case Lop_LD_F2:

	case Lop_ST_C:
	case Lop_ST_C2:
	case Lop_ST_I:
	case Lop_ST_F:
	case Lop_ST_F2:

	  if (label_base (oper))
	    {
	      if (!zero_offset (oper))
		return (2);
	    }
	  else
	    {
	      /* Must be register base */
	      if (!zero_offset (oper))
		return (2);
	    }
	  return (1);

	case Lop_BEQ_FS:
	case Lop_BEQ:
	case Lop_BNE_FS:
	case Lop_BNE:
	case Lop_BGT_FS:
	case Lop_BGT:
	case Lop_BGE_FS:
	case Lop_BGE:
	case Lop_BLT_FS:
	case Lop_BLT:
	case Lop_BLE_FS:
	case Lop_BLE:
	case Lop_BGT_U_FS:
	case Lop_BGT_U:
	case Lop_BGE_U_FS:
	case Lop_BGE_U:
	case Lop_BLT_U_FS:
	case Lop_BLT_U:
	case Lop_BLE_U_FS:
	case Lop_BLE_U:
	case Lop_BEQ_F_FS:
	case Lop_BEQ_F:
	case Lop_BNE_F_FS:
	case Lop_BNE_F:
	case Lop_BGT_F_FS:
	case Lop_BGT_F:
	case Lop_BGE_F_FS:
	case Lop_BGE_F:
	case Lop_BLT_F_FS:
	case Lop_BLT_F:
	case Lop_BLE_F_FS:
	case Lop_BLE_F:
	case Lop_BEQ_F2_FS:
	case Lop_BEQ_F2:
	case Lop_BNE_F2_FS:
	case Lop_BNE_F2:
	case Lop_BGT_F2_FS:
	case Lop_BGT_F2:
	case Lop_BGE_F2_FS:
	case Lop_BGE_F2:
	case Lop_BLT_F2_FS:
	case Lop_BLT_F2:
	  return (2);

	case Lop_RCMP:
	case Lop_RCMP_F:
	case Lop_EQ:
	case Lop_NE:
	case Lop_GT:
	case Lop_GE:
	case Lop_LT:
	case Lop_LE:
	case Lop_GT_U:
	case Lop_GE_U:
	case Lop_LT_U:
	case Lop_LE_U:
	case Lop_EQ_F:
	case Lop_GT_F:
	case Lop_GE_F:
	case Lop_NE_F:
	case Lop_LT_F:
	case Lop_LE_F:
	case Lop_EQ_F2:
	case Lop_GT_F2:
	case Lop_GE_F2:
	case Lop_NE_F2:
	case Lop_LT_F2:
	case Lop_LE_F2:
	  return (3);

	case Lop_MUL:
	case Lop_MUL_U:
	  return (6);

	case Lop_DIV:
	case Lop_DIV_U:
	  return (8);

	default:
	  return (1);
	}
    }
  else
    {
      M_assert (0, "M_num_oper_required_for_starcore: illegal machine model");
      return (0);
    }
}

int
M_num_registers_starcore (int ctype)
{
  switch (ctype)
    {
    case L_CTYPE_INT:
    case L_CTYPE_POINTER:
      return (16);
    default:
      return (0);
    }
}


int
M_is_stack_operand_starcore (L_Operand * operand)
{

  if (EM_SC_MODEL_OK (M_EM_SC_model))
    {
      if (L_is_macro (operand) &&
	  (operand->value.mac == L_MAC_SP ||
	   operand->value.mac == L_MAC_FP ||
	   operand->value.mac == L_MAC_LV ||
	   operand->value.mac == L_MAC_IP ||
	   operand->value.mac == L_MAC_OP ||
	   operand->value.mac == STARCORE_MAC_SP))
	{
	  return (1);
	}
      return (0);
    }
  else
    {
      M_assert (0, "M_is_stack_operand_starcore: illegal machine model");
      return (0);
    }
}

int
M_is_unsafe_macro_starcore (L_Operand *operand)
{

  if (EM_SC_MODEL_OK (M_EM_SC_model))
    {
      if (!L_is_macro (operand))
	return (0);

      switch (operand->value.mac)
	{
	case L_MAC_P0:
	case L_MAC_P16:
	  return (1);
	default:
	  return (0);
	}
    }
  else
    {
      M_assert (0, "M_is_unsafe_macro_starcore: illegal machine model");
      return (0);
    }
}

int
M_operand_type_starcore (L_Operand * operand)
{

  /* If NULL operand pointer, then return MDES_OPERAND_NULL */
  if (operand == NULL)
    return (MDES_OPERAND_NULL);

  switch (operand->type)
    {

    case L_OPERAND_IMMED:
    case L_OPERAND_CB:
    case L_OPERAND_LABEL:
    case L_OPERAND_STRING:
      return (MDES_OPERAND_Lit);

    case L_OPERAND_MACRO:
    case L_OPERAND_REGISTER:

      if (L_is_ctype_integer (operand))
	{
	  return (MDES_OPERAND_i);
	}

      else
	switch (operand->ctype)
	  {
	  case L_CTYPE_POINTER:
	    return (MDES_OPERAND_addr);
	  case L_CTYPE_PREDICATE:
	    return (MDES_OPERAND_p);
	  case L_CTYPE_CONTROL:
	    return (MDES_OPERAND_cntl);
	  case L_CTYPE_FLOAT:
	  case L_CTYPE_DOUBLE:
	    return (MDES_OPERAND_f);
	  default:
	    M_assert (0, "M_operand_type_starcore: Unknown ctype");
	    return (MDES_OPERAND_NULL);
	  }

    default:
      M_assert (0, "M_operand_type_starcore: Unknown type");
      return (MDES_OPERAND_NULL);
    }
}


int
M_conflicting_operands_starcore (L_Operand * operand,
			      L_Operand * conflict_array[],
			      int len, int prepass)
{
  int i = 0;

  if (L_is_reg (operand))
    {
      conflict_array[i++] = L_copy_operand (operand);

      if (L_is_ctype_flt (operand))
	conflict_array[i++] =
	  L_new_register_operand (operand->value.r,
				  L_CTYPE_DOUBLE, L_PTYPE_NULL);
      else if (L_is_ctype_dbl (operand))
	conflict_array[i++] =
	  L_new_register_operand (operand->value.r,
				  L_CTYPE_FLOAT, L_PTYPE_NULL);
    }
  else if (L_is_macro (operand))
    {
      switch (operand->value.mac)
	{
	case L_MAC_SP:
	case L_MAC_FP:
	case L_MAC_SAFE_MEM:
	case L_MAC_LV:
	case L_MAC_OP:
	case L_MAC_IP:
	case STARCORE_MAC_SP:
	  conflict_array[i++] =
	    L_new_macro_operand (L_MAC_SP, L_CTYPE_POINTER, L_PTYPE_NULL);
	  conflict_array[i++] =
	    L_new_macro_operand (L_MAC_FP, L_CTYPE_POINTER, L_PTYPE_NULL);
	  conflict_array[i++] =
	    L_new_macro_operand (L_MAC_SAFE_MEM, L_CTYPE_POINTER, 
				 L_PTYPE_NULL);
	  conflict_array[i++] =
	    L_new_macro_operand (L_MAC_LV, L_CTYPE_POINTER, L_PTYPE_NULL);
	  conflict_array[i++] =
	    L_new_macro_operand (L_MAC_OP, L_CTYPE_POINTER, L_PTYPE_NULL);
	  conflict_array[i++] =
	    L_new_macro_operand (L_MAC_IP, L_CTYPE_POINTER, L_PTYPE_NULL);
	  conflict_array[i++] =
	    L_new_macro_operand (STARCORE_MAC_SP, L_CTYPE_POINTER, 
				 L_PTYPE_NULL);
	  break;

	default:
	  conflict_array[i++] = L_copy_operand (operand);
	}
    }
  M_assert (i <= len, "Lstarcore_conflicting_operands: too many conflicts!");
  return (i);
}


/*
 *    Return the opc used to represent the proc_opc in Lcode.  This
 *      function is called if the user knows how to create proc_opcs,
 *      but does not know how that opcode will eventually be stored
 *      in Lcode using the opc/proc_opc combo.  This is relied upon
 *      by the binary optimizer.
 */

int
M_opc_from_proc_opc_starcore (int proc_opc)
{

  switch (proc_opc)
    {
    case STARCOREop_JSR_1:
    case STARCOREop_JSR_3:
    case STARCOREop_JSRD_1:
    case STARCOREop_JSRD_3:
    case STARCOREop_BSR_1:
    case STARCOREop_BSR_2:
    case STARCOREop_BSRD_1:
    case STARCOREop_BSRD_2:
      return (Lop_JSR);
    case STARCOREop_RTS:
    case STARCOREop_RTSD:
    case STARCOREop_RTSTK:
    case STARCOREop_RTSTKD:
      return (Lop_RTS);
    case STARCOREop_JF_1:
    case STARCOREop_JF_3:
    case STARCOREop_JFD_1:
    case STARCOREop_JFD_3:
    case STARCOREop_JT_1:
    case STARCOREop_JT_3:
    case STARCOREop_JTD_1:
    case STARCOREop_JTD_3:
    case STARCOREop_BF_1:
    case STARCOREop_BF_2:
    case STARCOREop_BFD_1:
    case STARCOREop_BFD_2:
    case STARCOREop_BT_1:
    case STARCOREop_BT_2:
    case STARCOREop_BTD_1:
    case STARCOREop_BTD_2:
      return (Lop_BR);
    case STARCOREop_JMP_1:
    case STARCOREop_JMP_3:
    case STARCOREop_JMPD_1:
    case STARCOREop_JMPD_3:
    case STARCOREop_BRA_1:
    case STARCOREop_BRA_2:
    case STARCOREop_BRAD_1:
    case STARCOREop_BRAD_2:
      return (Lop_JUMP);

    case STARCOREop_ABS:
      return (Lop_ABS);
    case STARCOREop_MAX:
    case STARCOREop_MAX2:
    case STARCOREop_MAX2VIT:
    case STARCOREop_MAXM:
      return (Lop_MAX);
    case STARCOREop_MIN:
      return (Lop_MIN);
    case STARCOREop_SAT_F:
    case STARCOREop_SAT_L:
      return (Lop_SAT);

    case STARCOREop_EXTRACT:
      return (Lop_EXTRACT);
    case STARCOREop_EXTRACTU:
      return (Lop_EXTRACT_U);
    case STARCOREop_INSERT:
      return (Lop_DEPOSIT);

    case STARCOREop_SXT_B:
    case STARCOREop_SXTA_B:
      return (Lop_SXT_C);
    case STARCOREop_SXT_W:
    case STARCOREop_SXTA_W:
      return (Lop_SXT_C2);
    case STARCOREop_SXT_L:
      return (Lop_SXT_I);
    case STARCOREop_ZXT_B:
    case STARCOREop_ZXTA_B:
      return (Lop_ZXT_C);
    case STARCOREop_ZXT_W:
    case STARCOREop_ZXTA_W:
      return (Lop_ZXT_C2);
    case STARCOREop_ZXT_L:
      return (Lop_ZXT_I);

    case STARCOREop_TFR:
    case STARCOREop_TFRT:
    case STARCOREop_TFRF:
    case STARCOREop_TFRA:
    case STARCOREop_MOVE_W_MV_1:
    case STARCOREop_MOVE_W_MV_2:
    case STARCOREop_MOVEU_W_MV:
    case STARCOREop_MOVE_L_MV_1:
    case STARCOREop_MOVE_L_MV_3:
    case STARCOREop_MOVEU_L_MV:
    case STARCOREop_MOVE_F_MV_2:
    case STARCOREop_MOVET:
    case STARCOREop_MOVEF:
      return (Lop_MOV);

    case STARCOREop_ADD:
    case STARCOREop_ADD2:
    case STARCOREop_ADDNC_W:
    case STARCOREop_ADR:
    case STARCOREop_ADC:
    case STARCOREop_IADD:
    case STARCOREop_ADDA_1:
    case STARCOREop_ADDA_2:
      return (Lop_ADD);
    case STARCOREop_SUB:
    case STARCOREop_SUB2:
    case STARCOREop_SUBNC_W:
    case STARCOREop_SBR:
    case STARCOREop_SBC:
    case STARCOREop_SUBA:
      return (Lop_SUB);
    case STARCOREop_AND_1:
    case STARCOREop_AND_2:
      return (Lop_AND);
    case STARCOREop_OR_1:
    case STARCOREop_OR_2:
      return (Lop_OR);
    case STARCOREop_EOR_1:
    case STARCOREop_EOR_2:
      return (Lop_XOR);

    case STARCOREop_LSLL:
      return (Lop_LSL);
    case STARCOREop_ADDL1A:
    case STARCOREop_ADDL2A:
      return (Lop_LSLADD);
    case STARCOREop_LSR:
    case STARCOREop_LSRA:
    case STARCOREop_LSRR:
    case STARCOREop_LSRW:
      return (Lop_LSR);

    case STARCOREop_ASL:
    case STARCOREop_ASL2A:
    case STARCOREop_ASLA:
    case STARCOREop_ASLL:
    case STARCOREop_ASLW:
      return (Lop_LSL);
    case STARCOREop_ASR:
    case STARCOREop_ASRA:
    case STARCOREop_ASRR:
    case STARCOREop_ASRW:
      return (Lop_ASR);

    case STARCOREop_CMPEQ:
    case STARCOREop_CMPGT:
    case STARCOREop_CMPHI:
    case STARCOREop_TSTEQ:
    case STARCOREop_TSTGE:
    case STARCOREop_TSTGT:
    case STARCOREop_CMPEQA:
    case STARCOREop_CMPGTA:
    case STARCOREop_CMPHIA:
    case STARCOREop_TSTEQA_L:
    case STARCOREop_TSTEQA_W:
    case STARCOREop_TSTGEA:
    case STARCOREop_TSTGTA:
      return (Lop_CMP);

    case STARCOREop_IMPY:
    case STARCOREop_IMPY_W:
    case STARCOREop_IMPYHLUU:
    case STARCOREop_IMPYSU:
    case STARCOREop_IMPYUU:
    case STARCOREop_MPY:
    case STARCOREop_MPYR:
    case STARCOREop_MPYSU:
    case STARCOREop_MPYUS:
    case STARCOREop_MPYUU:
      return (Lop_MUL);

    case STARCOREop_IMAC:
    case STARCOREop_IMACLHUU:
    case STARCOREop_IMACUS:
    case STARCOREop_MAC_1:
    case STARCOREop_MAC_2:
    case STARCOREop_MACR:
    case STARCOREop_MACSU:
    case STARCOREop_MACUS:
    case STARCOREop_MACUU:
      return (Lop_MUL_ADD);

    case STARCOREop_MOVE_B_ST_1:
    case STARCOREop_MOVE_B_ST_2:
    case STARCOREop_MOVE_B_ST_3:
      return (Lop_ST_C);
    case STARCOREop_MOVE_W_ST_1:
    case STARCOREop_MOVE_W_ST_2:
    case STARCOREop_MOVE_W_ST_3:
    case STARCOREop_MOVE_F_ST_1:
    case STARCOREop_MOVES_F_ST_1:
    case STARCOREop_MOVES_F_ST_2:
    case STARCOREop_MOVES_F_ST_3:
      return (Lop_ST_C2);
    case STARCOREop_MOVE_2W_ST:
    case STARCOREop_MOVE_L_ST_1:
    case STARCOREop_MOVE_L_ST_2:
    case STARCOREop_MOVE_L_ST_3:
    case STARCOREop_MOVES_L_ST:
    case STARCOREop_MOVES_2F_ST:
      return (Lop_ST_I);
    case STARCOREop_MOVE_4W_ST:
    case STARCOREop_MOVE_2L_ST:
    case STARCOREop_MOVES_4F_ST:
      return (Lop_ST_Q);

    case STARCOREop_MOVE_B_LD_1:
    case STARCOREop_MOVE_B_LD_2:
    case STARCOREop_MOVEU_B_LD_1:
    case STARCOREop_MOVEU_B_LD_2:
    case STARCOREop_MOVEU_B_LD_3:
      return (Lop_LD_C);
    case STARCOREop_MOVE_W_LD_1:
    case STARCOREop_MOVE_W_LD_2:
    case STARCOREop_MOVE_W_LD_3:
    case STARCOREop_MOVEU_W_LD_1:
    case STARCOREop_MOVEU_W_LD_2:
    case STARCOREop_MOVEU_W_LD_3:
    case STARCOREop_MOVE_F_LD_1:
    case STARCOREop_MOVE_F_LD_2:
    case STARCOREop_MOVE_F_LD_3:
      return (Lop_LD_C2);
    case STARCOREop_MOVE_2W_LD:
    case STARCOREop_MOVE_L_LD_1:
    case STARCOREop_MOVE_L_LD_2:
    case STARCOREop_MOVE_L_LD_3:
    case STARCOREop_MOVE_2F_LD:
      return (Lop_LD_I);
    case STARCOREop_MOVE_4W_LD:
    case STARCOREop_MOVE_2L_LD:
    case STARCOREop_MOVE_4F_LD:
      return (Lop_LD_Q);

    case STARCOREop_NOP:
      return (Lop_NO_OP);

    default:
      fprintf (stderr, "Illegal proc_opc %d\n", proc_opc);
      M_assert (0, "M_opc_from_proc_opc_starcore: illegal proc_opc");
      return (-1);
    }

  return 0;
}


/*--------------------------------------------------------------------------*/

L_Operand *
M_starcore_epilogue_cntr_register ()
{
  return NULL;
}

/*--------------------------------------------------------------------------*/

L_Operand *
M_starcore_loop_cntr_register ()
{
  I_punt ("M_starcore_loop_cntr_register");
  return NULL;
}

/*--------------------------------------------------------------------------*/


void
M_get_memory_operands_starcore (int *first, int *number, int proc_opc)
{
  *first = 0;
  *number = 2;
}

void
M_define_opcode_name_starcore (STRING_Symbol_Table *sym_tbl)
{
}

char *
M_get_opcode_name_starcore (int id)
{
  return NULL;
}

int
M_memory_access_size_starcore (L_Oper *op)
{
  return 0;
}

int
M_get_data_type_starcore (L_Oper *op)
{
  return 0;
}

int
M_is_implicit_memory_op_starcore (L_Oper *oper)
{
  return 0;
}

/* Return a bit vector that represents the src operands to try and coalesce.
 * 01 - src[0], 10 - src[1], 11 - src[0] & src[1]
 */
int
M_starcore_coalescing_oper (L_Oper *oper)
{
  switch (oper->opc)
    {
    case Lop_ABS:
    case Lop_LSR:
    case Lop_ASR:
    case Lop_LSL:
      if (L_same_operand (oper->src[0], oper->dest[0]))
	return 0;
      else
	return 1;

    case Lop_ADD:
      if (L_is_integer_reg (oper->dest[0]) ||
	  L_same_operand (oper->src[0], oper->dest[0]) ||
	  L_same_operand (oper->src[1], oper->dest[0]))
	return 0;
      else
	return 3;
    case Lop_SUB:
      if (!L_is_int_constant (oper->src[1]) ||
	  L_same_operand (oper->src[0], oper->dest[0]))
	return 0;
      else
	return 1;

    case Lop_AND:
    case Lop_NAND:
    case Lop_OR:
    case Lop_NOR:
    case Lop_XOR:
    case Lop_NXOR:
      if (L_same_operand (oper->src[0], oper->dest[0]) ||
	  L_same_operand (oper->src[1], oper->dest[0]))
	return 0;
      else
	return 3;

    default:
      return 0;
    }
}

void
M_starcore_negate_compare (L_Oper *oper)
{
  if (oper->pred[0] && 
      (oper->pred[0]->value.mac == STARCORE_MAC_T_BIT))
    oper->pred[0]->value.mac = STARCORE_MAC_T_BIT_INV;
  else if (oper->pred[0] &&
	   (oper->pred[0]->value.mac == STARCORE_MAC_T_BIT_INV))
    oper->pred[0]->value.mac = STARCORE_MAC_T_BIT;
  else
    L_punt ("M_starcore_negate_compare: STARCORE assumption");
  
  if (oper->pred[1])
    L_delete_operand (oper->pred[1]);
  oper->pred[1] = L_copy_operand (oper->pred[0]);

  if (oper->proc_opc == STARCOREop_JT_3)
    oper->proc_opc = STARCOREop_JF_3;
  else if (oper->proc_opc == STARCOREop_JF_3)
    oper->proc_opc = STARCOREop_JT_3;
  else
    L_punt ("M_starcore_negate_compare: STARCORE assumption 2");

  return;
}
