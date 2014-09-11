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
 *  	  File :	ml_tahoe.c 
 * Description : 	Machine dependent specification.  
 *     Authors :        Dan Connors and Jim Pierce
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
/* 09/18/02 REK Updating to use the new opcode map and completers scheme. */

#include  <config.h>
#include  <Lcode/l_main.h>
#include  <Lcode/ltahoe_completers.h>
#include  "m_spec.h"
#include  "m_tahoe.h"

Set Set_tahoe_fragile_macro = NULL;

/*---------------------------------------------------------------------------*/

/* This function specifies the macro registers that are assumed to be killed */
/* by jsr's, so returning a 1 means killed. */

/* output and return params are fragile (can't be moved over function call)  */

int
M_tahoe_fragile_macro (int macro_value)
{
  if (M_IPF_MODEL_OK (M_model))
    {
      switch (macro_value)
	{
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

	case TAHOE_MAC_AP:

	case TAHOE_MAC_TMPREG1:
	case TAHOE_MAC_TMPREG2:

	case TAHOE_MAC_LC:
	case TAHOE_MAC_EC:

	  /* JWS -- this is a temp fix for alloca */
	case L_MAC_OP:
	  return (1);

	default:
	  return (0);
	}
    }
  else
    {
      M_assert (0, "M_tahoe_fragile_macro: illegal machine model");
      return (0);
    }
}


Set
M_tahoe_fragile_macro_set (void)
{
  if (!Set_tahoe_fragile_macro)
    {
      Set_tahoe_fragile_macro = Set_add (Set_tahoe_fragile_macro, L_MAC_P8);
      Set_tahoe_fragile_macro = Set_add (Set_tahoe_fragile_macro, L_MAC_P9);
      Set_tahoe_fragile_macro = Set_add (Set_tahoe_fragile_macro, L_MAC_P10);
      Set_tahoe_fragile_macro = Set_add (Set_tahoe_fragile_macro, L_MAC_P11);
      Set_tahoe_fragile_macro = Set_add (Set_tahoe_fragile_macro, L_MAC_P12);
      Set_tahoe_fragile_macro = Set_add (Set_tahoe_fragile_macro, L_MAC_P13);
      Set_tahoe_fragile_macro = Set_add (Set_tahoe_fragile_macro, L_MAC_P14);
      Set_tahoe_fragile_macro = Set_add (Set_tahoe_fragile_macro, L_MAC_P15);
      Set_tahoe_fragile_macro = Set_add (Set_tahoe_fragile_macro, L_MAC_P16);
      Set_tahoe_fragile_macro = Set_add (Set_tahoe_fragile_macro, L_MAC_P17);
      Set_tahoe_fragile_macro = Set_add (Set_tahoe_fragile_macro, L_MAC_P18);
      Set_tahoe_fragile_macro = Set_add (Set_tahoe_fragile_macro, L_MAC_P19);
      Set_tahoe_fragile_macro = Set_add (Set_tahoe_fragile_macro, L_MAC_P20);
      Set_tahoe_fragile_macro = Set_add (Set_tahoe_fragile_macro, L_MAC_P21);
      Set_tahoe_fragile_macro = Set_add (Set_tahoe_fragile_macro, L_MAC_P22);
      Set_tahoe_fragile_macro = Set_add (Set_tahoe_fragile_macro, L_MAC_P23);
      Set_tahoe_fragile_macro = Set_add (Set_tahoe_fragile_macro, L_MAC_P24);
      Set_tahoe_fragile_macro = Set_add (Set_tahoe_fragile_macro, L_MAC_P25);
      Set_tahoe_fragile_macro = Set_add (Set_tahoe_fragile_macro, L_MAC_P26);
      Set_tahoe_fragile_macro = Set_add (Set_tahoe_fragile_macro, L_MAC_P27);
      Set_tahoe_fragile_macro = Set_add (Set_tahoe_fragile_macro, L_MAC_P28);
      Set_tahoe_fragile_macro = Set_add (Set_tahoe_fragile_macro, L_MAC_P29);
      Set_tahoe_fragile_macro = Set_add (Set_tahoe_fragile_macro, L_MAC_P30);
      Set_tahoe_fragile_macro = Set_add (Set_tahoe_fragile_macro, L_MAC_P31);
      Set_tahoe_fragile_macro = Set_add (Set_tahoe_fragile_macro, 
					 TAHOE_MAC_AP);
      Set_tahoe_fragile_macro = Set_add (Set_tahoe_fragile_macro,
					 TAHOE_MAC_TMPREG1);
      Set_tahoe_fragile_macro = Set_add (Set_tahoe_fragile_macro,
					 TAHOE_MAC_TMPREG2);
      Set_tahoe_fragile_macro = Set_add (Set_tahoe_fragile_macro, 
					 TAHOE_MAC_LC);
      Set_tahoe_fragile_macro = Set_add (Set_tahoe_fragile_macro, 
					 TAHOE_MAC_EC);
      Set_tahoe_fragile_macro = Set_add (Set_tahoe_fragile_macro, 
					 L_MAC_OP);
    }

  return Set_tahoe_fragile_macro;
}

/****************************************************************************
 *
 * routine: M_tahoe_extra_pred_define_opcode()
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
 *           M_tahoe_extra_pred_define_typeX functions, too (mbt)
 *-------------------------------------------------------------------------*/
/* 09/18/02 REK Updating to use the new TAHOEops.  The CMP4 thing is
 *              a little more difficult, as we will need the completer
 *              to figure that out.  I can't find anything that actually
 *              uses this function at the moment, so I'll leave that for
 *              later.
 */
int
M_tahoe_extra_pred_define_opcode (int proc_opc)
{
    switch (proc_opc)
    {
    case TAHOEop_FRCPA:
    case TAHOEop_FRSQRTA:
    case TAHOEop_TBIT:
#if 0
	/* 09/18/02 REK This needs to be fixed at some point. */
    case TAHOEop_CMP4_NE_AND:	/* Lop_PRED_CLEAR */
    case TAHOEop_CMP4_EQ_OR:	/* Lop_PRED_SET */
#endif
	return (1);
    default:
	return (0);
    } /* switch */
} /* M_tahoe_extra_pred_define_opcode */

/************************************************************************/
/* predicate-defining opcodes where both dests are pred regs (i.e. tbit)*/
/************************************************************************/
/* 09/18/02 REK Updating to use the new TAHOEops. */
int
M_tahoe_extra_pred_define_type1 (L_Oper * oper)
{
  switch (oper->proc_opc)
    {
    case TAHOEop_TBIT:
	return (1);

    case TAHOEop_CMP:
	if ((oper->completers & TC_CMP_4) && \
	    ((TC_GET_CMP_OP (oper->completers) == TC_CMP_OP_NE && \
	      TC_GET_CMP_TYPE (oper->completers) == TC_CMP_TYPE_AND) || \
	     (TC_GET_CMP_OP (oper->completers) == TC_CMP_OP_EQ && \
	      TC_GET_CMP_TYPE (oper->completers) == TC_CMP_TYPE_UNC)))
	    return (1);
	else
	    return (0);

    default:
	return (0);
    } /* switch */
} /* M_tahoe_extra_pred_define_type1 */



/********************************************************************/
/* predicate-defining opcodes where dest[1] is a pred. (i.e. frcpa) */
/********************************************************************/

int
M_tahoe_extra_pred_define_type2 (L_Oper * oper)
{

  switch (oper->proc_opc)
    {
    case TAHOEop_FRCPA:
    case TAHOEop_FRSQRTA:
      return (1);

    default:
      return (0);
    }
}



/*--------------------------------------------------------------------------*/

int
M_tahoe_subroutine_call (int opc)
{

  if (M_IPF_MODEL_OK (M_model))
    {
      return ((opc == Lop_JSR) || (opc == Lop_JSR_FS));
    }
  else
    {
      M_assert (0, "M_tahoe_subroutine_call:  Illegal model specified!");
      return (0);
    }
}

/*--------------------------------------------------------------------------*/


/* return 1 for macros that should be included in dataflow analysis */

int
M_tahoe_dataflow_macro (int id)
{

  if (M_IPF_MODEL_OK (M_model))
    {
      switch (id)
	{
	case TAHOE_MAC_ZERO:
	case TAHOE_MAC_FZERO:
	case TAHOE_MAC_FONE:
	case TAHOE_MAC_PRED_TRUE:
	case TAHOE_MAC_TEMPLATE:
	case TAHOE_MAC_LABEL:
	  return FALSE;
	default:
	  return TRUE;
	}
    }
  else
    {
      M_assert (0, "M_tahoe_dataflow_macro: illegal machine model");
      return (0);
    }
}

/*
 * Declare code generator specific macro registers to the front end parser.
 */

void
M_define_macros_tahoe (STRING_Symbol_Table * sym_tbl)
{

  if (M_IPF_MODEL_OK (M_model))
    {
      M_add_symbol (sym_tbl, "r0", TAHOE_MAC_ZERO);
      M_add_symbol (sym_tbl, "gp", TAHOE_MAC_GP);
      M_add_symbol (sym_tbl, "ap", TAHOE_MAC_AP);
      M_add_symbol (sym_tbl, "b0", TAHOE_MAC_RETADDR);
      M_add_symbol (sym_tbl, "p0", TAHOE_MAC_PRED_TRUE);
      M_add_symbol (sym_tbl, "f0", TAHOE_MAC_FZERO);
      M_add_symbol (sym_tbl, "f1", TAHOE_MAC_FONE);

      M_add_symbol (sym_tbl, "tmpreg1", TAHOE_MAC_TMPREG1);
      M_add_symbol (sym_tbl, "tmpreg2", TAHOE_MAC_TMPREG2);

      M_add_symbol (sym_tbl, "pred_save_reg", TAHOE_PRED_SAVE_REG);
      M_add_symbol (sym_tbl, "pred_blk_reg", TAHOE_PRED_BLK_REG);

      M_add_symbol (sym_tbl, "gp_save_reg", TAHOE_GP_SAVE_REG);

      M_add_symbol (sym_tbl, "Template", TAHOE_MAC_TEMPLATE);
      M_add_symbol (sym_tbl, "Label", TAHOE_MAC_LABEL);

      M_add_symbol (sym_tbl, "ar.fpsr", TAHOE_MAC_FPSR);
      M_add_symbol (sym_tbl, "ar.itc", TAHOE_MAC_ITC);
      M_add_symbol (sym_tbl, "ar.rsc", TAHOE_MAC_RSC);
      M_add_symbol (sym_tbl, "ar.bsp", TAHOE_MAC_BSP);
      M_add_symbol (sym_tbl, "ar.rnat", TAHOE_MAC_RNAT);
      M_add_symbol (sym_tbl, "ar.unat", TAHOE_MAC_UNAT);
      M_add_symbol (sym_tbl, "ar.ccv", TAHOE_MAC_CCV);
      M_add_symbol (sym_tbl, "ar.k0", TAHOE_MAC_KR0);
      M_add_symbol (sym_tbl, "ar.k1", TAHOE_MAC_KR1);
      M_add_symbol (sym_tbl, "ar.k2", TAHOE_MAC_KR2);
      M_add_symbol (sym_tbl, "ar.k3", TAHOE_MAC_KR3);
      M_add_symbol (sym_tbl, "ar.lc", TAHOE_MAC_LC);
      M_add_symbol (sym_tbl, "ar.ec", TAHOE_MAC_EC);
      M_add_symbol (sym_tbl, "ar.pfs", TAHOE_MAC_AR_PFS);
      M_add_symbol (sym_tbl, "psp", TAHOE_MAC_PSP);
      M_add_symbol (sym_tbl, "pspill", TAHOE_MAC_PSPILL);
      /* 1 if leaf function, 0 if non-leaf */
      M_add_symbol (sym_tbl, "$leaf", TAHOE_MAC_LEAF);
      /* LV space for int spill around sync */
      M_add_symbol (sym_tbl, "$sync_size", TAHOE_MAC_SYNC_SIZE);
      /* total alloc requirements */
      M_add_symbol (sym_tbl, "$mem_alloc_size", TAHOE_MAC_MEM_ALLOC);
      /* number of integer and float callee saved registers used */
    }
  else
    {
      M_assert (0, "M_define_macros_tahoe: illegal machine model");
    }
}

char *
M_get_macro_name_tahoe (int id)
{

  if (M_IPF_MODEL_OK (M_model))
    {
      switch (id)
	{
	case TAHOE_MAC_ZERO:
	  return "r0";
	case TAHOE_MAC_GP:
	  return "gp";
	case TAHOE_MAC_AP:
	  return "ap";
	case TAHOE_MAC_RETADDR:
	  return "b0";
	case TAHOE_MAC_FZERO:
	  return "f0";
	case TAHOE_MAC_FONE:
	  return "f1";
	case TAHOE_MAC_PRED_TRUE:
	  return "p0";
	case TAHOE_MAC_TMPREG1:
	  return "tmpreg1";
	case TAHOE_MAC_TMPREG2:
	  return "tmpreg2";
	case TAHOE_MAC_LEAF:
	  return "$leaf";
	case TAHOE_MAC_SYNC_SIZE:
	  return ("$sync_size");
	case TAHOE_MAC_MEM_ALLOC:
	  return "$mem_alloc_size";
	case TAHOE_PRED_SAVE_REG:
	  return "pred_save_reg";
	case TAHOE_PRED_BLK_REG:
	  return "pred_blk_reg";
	case TAHOE_GP_SAVE_REG:
	  return "gp_save_reg";
	case TAHOE_MAC_TEMPLATE:
	  return "Template";
	case TAHOE_MAC_LABEL:
	  return "Label";
	case TAHOE_MAC_FPSR:
	  return "ar.fpsr";
	case TAHOE_MAC_ITC:
	  return "ar.itc";
	case TAHOE_MAC_RSC:
	  return "ar.rsc";
	case TAHOE_MAC_BSP:
	  return "ar.bsp";
	case TAHOE_MAC_RNAT:
	  return "ar.rnat";
	case TAHOE_MAC_UNAT:
	  return "ar.unat";
	case TAHOE_MAC_CCV:
	  return "ar.ccv";
	case TAHOE_MAC_KR0:
	  return "ar.k0";
	case TAHOE_MAC_KR1:
	  return "ar.k1";
	case TAHOE_MAC_KR2:
	  return "ar.k2";
	case TAHOE_MAC_KR3:
	  return "ar.k3";
	case TAHOE_MAC_LC:
	  return "ar.lc";
	case TAHOE_MAC_EC:
	  return "ar.ec";
	case TAHOE_MAC_AR_PFS:
	  return "ar.pfs";
	case TAHOE_MAC_PSP:
	  return "psp";
	case TAHOE_MAC_PSPILL:
	  return "pspill";
	default:
	  return "???";
	}
    }
  else
    {
      M_assert (0, "M_get_macro_name_tahoe: illegal machine model");
      return (0);
    }
}


/*--------------------------------------------------------------------------*/
/*
 * Return true (1) if the instruction is supported in the hardware of the
 * processor.  Return false (0) otherwise.
 */
int
M_oper_supported_in_arch_tahoe (int opc)
{

  if (M_IPF_MODEL_OK (M_model))
    {
      switch (opc)
	{
	case Lop_MUL_ADD:
	case Lop_MUL_ADD_U:
	case Lop_MUL_SUB:
	case Lop_MUL_SUB_U:
	case Lop_MUL_SUB_REV:
	case Lop_MUL_SUB_REV_U:
	case Lop_OR_NOT:
	case Lop_AND_NOT:
	case Lop_MIN:
	case Lop_MAX:
	case Lop_ABS:
	case Lop_NOR:
	case Lop_NAND:
	case Lop_NXOR:
	case Lop_DIV:
	case Lop_ABS_F:
	case Lop_MIN_F:
	case Lop_MAX_F:
	case Lop_RCP_F:
	  return (0);

	default:
	  return (1);
	}
    }
  else
    {
      M_assert (0, "M_oper_supported_in_arch_tahoe: illegal machine model");
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
M_num_oper_required_for_tahoe (L_Oper * oper, char *name)
{

  if (M_IPF_MODEL_OK (M_model))
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
	case Lop_LD_Q:

	case Lop_ST_C:
	case Lop_ST_C2:
	case Lop_ST_I:
	case Lop_ST_F:
	case Lop_ST_F2:
	case Lop_ST_Q:

	case Lop_LD_POST_UC:
	case Lop_LD_POST_UC2:
	case Lop_LD_POST_C:
	case Lop_LD_POST_C2:
	case Lop_LD_POST_I:
	case Lop_LD_POST_UI:
	case Lop_LD_POST_Q:
	case Lop_LD_POST_F:
	case Lop_LD_POST_F2:

	case Lop_ST_POST_C:
	case Lop_ST_POST_C2:
	case Lop_ST_POST_I:
	case Lop_ST_POST_Q:
	case Lop_ST_POST_F:
	case Lop_ST_POST_F2:

	case Lop_LD_C_CHK:
	case Lop_LD_UC_CHK:
	case Lop_LD_C2_CHK:
	case Lop_LD_UC2_CHK:
	case Lop_LD_I_CHK:
	case Lop_LD_UI_CHK:
	case Lop_LD_Q_CHK:
	case Lop_LD_F_CHK:
	case Lop_LD_F2_CHK:

	  if (label_base (oper))
	    {
	      if (!zero_offset (oper))
		return (3);
	    }
	  else
	    {
	      /* Must be register base */
	      if (zero_offset (oper))
		return (1);
	    }
	  return (2);

	case Lop_LD_PRE_UC:
	case Lop_LD_PRE_C:
	case Lop_LD_PRE_UC2:
	case Lop_LD_PRE_C2:
	case Lop_LD_PRE_I:
	case Lop_LD_PRE_UI:
	case Lop_LD_PRE_Q:

	case Lop_LD_PRE_F:
	case Lop_LD_PRE_F2:

	case Lop_ST_PRE_C:
	case Lop_ST_PRE_C2:
	case Lop_ST_PRE_I:
	case Lop_ST_PRE_Q:
	case Lop_ST_PRE_F:
	case Lop_ST_PRE_F2:

	  if (label_base (oper))
	    {
	      if (!L_is_int_constant (oper->src[1]))
		return (4);
	    }
	  else
	    {
	      /* Must be register base */
	      if (L_is_int_constant (oper->src[1]))
		return (2);
	    }
	  return (3);

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
	  return (3);

	case Lop_MUL_ADD:
	case Lop_MUL_ADD_U:
	case Lop_MUL_SUB:
	case Lop_MUL_SUB_U:
	  return (4);

	case Lop_MUL_SUB_REV:
	case Lop_MUL_SUB_REV_U:
	  return (5);

	case Lop_DIV:
	case Lop_DIV_U:
	  return (8);

	default:
	  return (1);
	}
    }
  else
    {
      M_assert (0, "M_num_oper_required_for_tahoe: illegal machine model");
      return (0);
    }
}

int
M_num_registers_tahoe (int ctype)
{
  if (L_is_ctype_integer_direct (ctype))
    {
      return (128);
    }
  else
    {
      switch (ctype)
	{
	case L_CTYPE_FLOAT:
	  return (128);
	case L_CTYPE_DOUBLE:
	  return (128);
	case L_CTYPE_PREDICATE:
	  return (64);
	case L_CTYPE_BTR:
	  return (8);
	default:
	  return (0);
	}
    }
}


int
M_is_stack_operand_tahoe (L_Operand * operand)
{

  if (M_IPF_MODEL_OK (M_model))
    {
      if (L_is_macro (operand) &&
	  (operand->value.mac == L_MAC_SP ||
	   operand->value.mac == L_MAC_FP ||
	   operand->value.mac == L_MAC_LV ||
	   operand->value.mac == L_MAC_IP ||
	   operand->value.mac == L_MAC_OP ||
	   operand->value.mac == TAHOE_MAC_PSP ||
	   operand->value.mac == TAHOE_MAC_PSPILL))
	{
	  return (1);
	}
      return (0);
    }
  else
    {
      M_assert (0, "M_is_stack_operand_tahoe: illegal machine model");
      return (0);
    }
}

int
M_is_unsafe_macro_tahoe (L_Operand * operand)
{

  if (M_IPF_MODEL_OK (M_model))
    {
      if (!L_is_macro (operand))
	return (0);

      switch (operand->value.mac)
	{
	case TAHOE_PRED_BLK_REG: /* Register containing all the predicates   */
	case TAHOE_MAC_AP:
	case TAHOE_MAC_FPSR: /* Floating Point Status Register               */
	case TAHOE_MAC_ITC:  /* Interval Time Counter                        */
	case TAHOE_MAC_RSC:  /* Register Stack Configuration Register        */
	case TAHOE_MAC_BSP:  /* Backing Store Pointer For the register stack */
	case TAHOE_MAC_RNAT: /* RSE NAT Collection register                  */
	case TAHOE_MAC_UNAT: /* User NAT Collection register                 */
	case TAHOE_MAC_CCV:  /* Compare and Exchange Compare Value Register  */
	case TAHOE_MAC_KR0:  /* Kernel Register 0 */
	case TAHOE_MAC_KR1:  /* Kernel Register 1 */
	case TAHOE_MAC_KR2:  /* Kernel Register 2 */
	case TAHOE_MAC_KR3:  /* Kernel Register 3 */

	case TAHOE_MAC_LC:	/* Loop count             */
	case TAHOE_MAC_EC:	/* Epilogue stage counter */
	case TAHOE_MAC_AR_PFS:	/* Previous Frame State   */

	case L_MAC_SP:
	case L_MAC_OP:

	  return (1);
	default:
	  return (0);
	}
    }
  else
    {
      M_assert (0, "M_is_unsafe_macro_tahoe: illegal machine model");
      return (0);
    }
}

int
M_operand_type_tahoe (L_Operand * operand)
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

	  case L_CTYPE_FLOAT:
	  case L_CTYPE_DOUBLE:
	    return (MDES_OPERAND_f);
	  case L_CTYPE_PREDICATE:
	    return (MDES_OPERAND_p);
	  case L_CTYPE_BTR:
	    return (MDES_OPERAND_btr);
	  default:
	    M_assert (0, "M_operand_type_tahoe: Unknown ctype");
	    return (MDES_OPERAND_NULL);
	  }

    default:
      M_assert (0, "M_operand_type_tahoe: Unknown type");
      return (MDES_OPERAND_NULL);
    }
}


int
M_conflicting_operands_tahoe (L_Operand * operand,
			      L_Operand * conflict_array[],
			      int len, int prepass)
{
  int i = 0, j;

  if (L_is_reg (operand))
    {
      conflict_array[i++] = L_copy_operand (operand);

      if (L_is_ctype_predicate (operand) && !prepass)
	conflict_array[i++] =
	  L_new_macro_operand (TAHOE_PRED_BLK_REG, L_CTYPE_LLONG,
			       L_PTYPE_NULL);
      else if (L_is_ctype_flt (operand))
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
	  conflict_array[i++] =
	    L_new_macro_operand (L_MAC_SP, L_CTYPE_LLONG, L_PTYPE_NULL);
	  conflict_array[i++] =
	    L_new_macro_operand (L_MAC_FP, L_CTYPE_LLONG, L_PTYPE_NULL);
	  conflict_array[i++] =
	    L_new_macro_operand (L_MAC_SAFE_MEM, L_CTYPE_LLONG, L_PTYPE_NULL);
	  conflict_array[i++] =
	    L_new_macro_operand (L_MAC_LV, L_CTYPE_LLONG, L_PTYPE_NULL);
	  conflict_array[i++] =
	    L_new_macro_operand (L_MAC_OP, L_CTYPE_LLONG, L_PTYPE_NULL);
	  conflict_array[i++] =
	    L_new_macro_operand (L_MAC_IP, L_CTYPE_LLONG, L_PTYPE_NULL);
	  break;

	case TAHOE_PRED_BLK_REG:
	  conflict_array[i++] =
	    L_new_macro_operand (TAHOE_PRED_BLK_REG, L_CTYPE_LLONG,
				 L_PTYPE_NULL);
	  for (j = 1; j <= 63; j++)
	    {
	      conflict_array[i++] =
		L_new_register_operand (TAHOE_PRED_REG_BASE + j,
					L_CTYPE_PREDICATE, L_PTYPE_NULL);
	    }
	  break;

	  /* Floating-point parameter regs doubled-up on tahoe */
	case L_MAC_P20:
	case L_MAC_P21:
	case L_MAC_P22:
	case L_MAC_P23:
	case L_MAC_P24:
	case L_MAC_P25:
	case L_MAC_P26:
	case L_MAC_P27:
	  conflict_array[i++] =
	    L_new_macro_operand (operand->value.mac,
				 L_CTYPE_FLOAT, L_PTYPE_NULL);
	  conflict_array[i++] =
	    L_new_macro_operand (operand->value.mac,
				 L_CTYPE_DOUBLE, L_PTYPE_NULL);
	  conflict_array[i++] =
	    L_new_macro_operand (operand->value.mac + 8,
				 L_CTYPE_FLOAT, L_PTYPE_NULL);
	  conflict_array[i++] =
	    L_new_macro_operand (operand->value.mac + 8,
				 L_CTYPE_DOUBLE, L_PTYPE_NULL);
	  break;
	case L_MAC_P28:
	case L_MAC_P29:
	case L_MAC_P30:
	case L_MAC_P31:
	case L_MAC_P32:
	case L_MAC_P33:
	case L_MAC_P34:
	case L_MAC_P35:
	  conflict_array[i++] =
	    L_new_macro_operand (operand->value.mac,
				 L_CTYPE_FLOAT, L_PTYPE_NULL);
	  conflict_array[i++] =
	    L_new_macro_operand (operand->value.mac,
				 L_CTYPE_DOUBLE, L_PTYPE_NULL);
	  conflict_array[i++] =
	    L_new_macro_operand (operand->value.mac - 8,
				 L_CTYPE_FLOAT, L_PTYPE_NULL);
	  conflict_array[i++] =
	    L_new_macro_operand (operand->value.mac - 8,
				 L_CTYPE_DOUBLE, L_PTYPE_NULL);
	  break;

	case TAHOE_MAC_ZERO:
	case TAHOE_MAC_FZERO:
	case TAHOE_MAC_FONE:
	case TAHOE_MAC_PRED_TRUE:
	  break;

	default:
	  conflict_array[i++] = L_copy_operand (operand);
	}
    }
  M_assert (i <= len, "Ltahoe_conflicting_operands: too many conflicts!");
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
M_opc_from_proc_opc_tahoe (int proc_opc)
{
    switch (proc_opc)
    {
    case TAHOEop_BR_CALL:
	return (Lop_JSR);
    case TAHOEop_BR_RET:
	return (Lop_RTS);
    case TAHOEop_BR_COND:
	return (Lop_JUMP);
    case TAHOEop_BRP:
	return (Lop_PBR);
	
    case TAHOEop_BR_CLOOP:
	return (Lop_BR);
	
    case TAHOEop_BR_CTOP:
    case TAHOEop_BR_CEXIT:
    case TAHOEop_BR_WTOP:
    case TAHOEop_BR_WEXIT:
      return (Lop_JUMP);

    case TAHOEop_SXT1:
	return (Lop_SXT_C);
    case TAHOEop_SXT2:
      return (Lop_SXT_C2);
    case TAHOEop_SXT4:
	return (Lop_SXT_I);
    case TAHOEop_ZXT1:
	return (Lop_ZXT_C);
    case TAHOEop_ZXT2:
	return (Lop_ZXT_C2);
    case TAHOEop_ZXT4:
	return (Lop_ZXT_I);

    case TAHOEop_MOVI:
    case TAHOEop_MOVL:
    case TAHOEop_MOV_GR:
    case TAHOEop_MOV_TOBR:
    case TAHOEop_MOV_FRBR:
    case TAHOEop_MOV_TOPR:
    case TAHOEop_MOV_FRPR:
    case TAHOEop_MOV_TOAR_I:
    case TAHOEop_MOV_FRAR_I:
    case TAHOEop_MOV_TOAR_M:
    case TAHOEop_MOV_FRAR_M:
    case TAHOEop_MOV_FR:
	return (Lop_MOV);

    case TAHOEop_GETF_S:
    case TAHOEop_GETF_D:
    case TAHOEop_GETF_EXP:
    case TAHOEop_GETF_SIG:
	return (Lop_F2_I);
	
    case TAHOEop_SETF_S:
    case TAHOEop_SETF_D:
    case TAHOEop_SETF_EXP:
    case TAHOEop_SETF_SIG:
	return (Lop_I_F2);
	
    case TAHOEop_ADD:
    case TAHOEop_ADDS:
    case TAHOEop_ADDL:
    case TAHOEop_ADDP4:
	return (Lop_ADD);
    case TAHOEop_SUB:
	return (Lop_SUB);
    case TAHOEop_AND:
	return (Lop_AND);
    case TAHOEop_ANDCM:
	return (Lop_AND_COMPL);
    case TAHOEop_OR:
	return (Lop_OR);
    case TAHOEop_XOR:
	return (Lop_XOR);

    case TAHOEop_SHL:
	return (Lop_LSL);
    case TAHOEop_SHLADD:
	return (Lop_LSLADD);
    case TAHOEop_SHLADDP4:
	return (Lop_LSLADD);

    case TAHOEop_DEP:
    case TAHOEop_DEP_Z:
	return (Lop_DEPOSIT);
    case TAHOEop_SHR:
	return (Lop_ASR);
    case TAHOEop_SHR_U:
	return (Lop_LSR);
    case TAHOEop_EXTR:
	return (Lop_EXTRACT);
    case TAHOEop_EXTR_U:
	return (Lop_EXTRACT_U);
	
    case TAHOEop_CMP:
    case TAHOEop_TBIT:
      return (Lop_CMP);

    case TAHOEop_FCMP:
      return (Lop_CMP_F);

    case TAHOEop_FADD_D:
    case TAHOEop_FADD_S:
    case TAHOEop_FADD:
	return (Lop_ADD_F2);
    case TAHOEop_FSUB_D:
    case TAHOEop_FSUB_S:
    case TAHOEop_FSUB:
	return (Lop_SUB_F2);
    case TAHOEop_FMAX:
	return (Lop_MAX_F2);
    case TAHOEop_FMIN:
	return (Lop_MIN_F2);
    case TAHOEop_FAMAX:
	return (Lop_MAX_F2);
    case TAHOEop_FAMIN:
	return (Lop_MIN_F2);
    case TAHOEop_FABS:
	return (Lop_ABS_F2);
    case TAHOEop_FMA:
    case TAHOEop_FMA_S:
    case TAHOEop_FMA_D:
	return (Lop_MUL_ADD_F2);
    case TAHOEop_FNMA:
    case TAHOEop_FNMA_S:
    case TAHOEop_FNMA_D:
	return (Lop_MUL_SUB_REV_F2);
    case TAHOEop_FMS:
    case TAHOEop_FMS_S:
    case TAHOEop_FMS_D:
	return (Lop_MUL_SUB_F2);

    case TAHOEop_FRCPA:
	return (Lop_DIV_F2);
    case TAHOEop_FRSQRTA:
	return (Lop_SQRT_F2);

/*    case TAHOEop_XMA_L:
	return (Lop_MUL);
    case TAHOEop_XMA_LU:
	return (Lop_MUL);
    case TAHOEop_XMA_H:
	return (Lop_MUL);
    case TAHOEop_XMA_HU:
      return (Lop_MUL);
*/

    case TAHOEop_XMA_L:
	return (Lop_MUL_ADD);
    case TAHOEop_XMA_H:
	return (Lop_MUL_ADD);
    case TAHOEop_XMA_HU:
	return (Lop_MUL_ADD);

    case TAHOEop_CHK_S:
    case TAHOEop_CHK_S_I:
    case TAHOEop_CHK_S_F:
    case TAHOEop_CHK_S_M:
	return (Lop_CHECK);

    case TAHOEop_CHK_A:
	return (Lop_CHECK_ALAT);

    case TAHOEop_NOP_I:
    case TAHOEop_NOP_B:
    case TAHOEop_NOP_M:
    case TAHOEop_NOP_F:
    case TAHOEop_NOP_X:
    case TAHOEop_INVALID:
	return (Lop_NO_OP);

    case TAHOEop_ALLOC:
	return (Lop_ALLOC);

    case TAHOEop_LD8_FILL:
    case TAHOEop_LD8:
    case TAHOEop_LD8_C:
	return (Lop_LD_Q);
/*    case TAHOEop_LD8_POST:
    case TAHOEop_LD8_POST_A:
    case TAHOEop_LD8_POST_S:
    case TAHOEop_LD8_POST_SA:    
    return (Lop_LD_POST_Q);
*/
    case TAHOEop_ST8_SPILL:
    case TAHOEop_ST8:
	return (Lop_ST_Q);
/*    case TAHOEop_ST8_SPILL_POST:
    case TAHOEop_ST8_POST:
    case TAHOEop_ST8_POST_O:
    return (Lop_ST_POST_Q);
*/
    case TAHOEop_LD4:
    case TAHOEop_LD4_C:
	return (Lop_LD_UI);
/*    case TAHOEop_LD4_POST:
    case TAHOEop_LD4_POST_A:
    case TAHOEop_LD4_POST_S:
    case TAHOEop_LD4_POST_SA:
    return (Lop_LD_POST_UI);
*/
    case TAHOEop_ST4:
	return (Lop_ST_I);
/*    case TAHOEop_ST4_POST:
    case TAHOEop_ST4_POST_O:
    return (Lop_ST_POST_I);
*/
    case TAHOEop_LD2:
    case TAHOEop_LD2_C:
	return (Lop_LD_UC2);
/*    case TAHOEop_LD2_POST:
    case TAHOEop_LD2_POST_A:
    case TAHOEop_LD2_POST_S:
    case TAHOEop_LD2_POST_SA:
    return (Lop_LD_POST_UC2);
*/
    case TAHOEop_ST2:
	return (Lop_ST_C2);
/*    case TAHOEop_ST2_POST:
    case TAHOEop_ST2_POST_O:
    return (Lop_ST_POST_C2);
*/
    case TAHOEop_LD1:
    case TAHOEop_LD1_C:
	return (Lop_LD_UC);
/*    case TAHOEop_LD1_POST:
    case TAHOEop_LD1_POST_A:
    case TAHOEop_LD1_POST_S:
    case TAHOEop_LD1_POST_SA:
    return (Lop_LD_POST_UC);
*/
    case TAHOEop_ST1:
	return (Lop_ST_C);
/*    case TAHOEop_ST1_POST:
    case TAHOEop_ST1_POST_O:
    return (Lop_ST_POST_C);
*/
 
    case TAHOEop_LDFS:
    case TAHOEop_LDFS_C:
	return (Lop_LD_F);
    case TAHOEop_STFS:
	return (Lop_ST_F);
/*    case TAHOEop_LDF_S_POST:
    case TAHOEop_LDF_S_POST_S:
    case TAHOEop_LDF_S_POST_A:
    case TAHOEop_LDF_S_POST_SA:
    return (Lop_LD_POST_F);
    case TAHOEop_STF_S_POST:
    return (Lop_ST_POST_F);
*/
    case TAHOEop_LDF_FILL:
    case TAHOEop_LDFD:
    case TAHOEop_LDFD_C:
	return (Lop_LD_F2);
    case TAHOEop_STF_SPILL:
    case TAHOEop_STFD:
	return (Lop_ST_F2);
/*    case TAHOEop_LDF_FILL_POST:
    case TAHOEop_LDF_D_POST:
    case TAHOEop_LDF_D_POST_S:
    case TAHOEop_LDF_D_POST_A:
    case TAHOEop_LDF_D_POST_SA:
    return (Lop_LD_POST_F2);
    case TAHOEop_STF_SPILL_POST:
    case TAHOEop_STF_D_POST:
    return (Lop_ST_POST_F2);
*/

    case TAHOEop_LFETCH:
	return (Lop_PREF_LD);
      
    case TAHOEop_FMERGE_NS:
    case TAHOEop_FMERGE_S:
    case TAHOEop_FMERGE_SE:
	I_warn("M_opc_from_proc_opc_tahoe: "
	       "No Lcode equivalent for TAHOEop_FMERGE.  Guessing Lop_ABS_F2");
	return (Lop_ABS_F2);
      
    case TAHOEop_FCVT_XF:
	return (Lop_I_F2);
    case TAHOEop_FCVT_FX:
    case TAHOEop_FCVT_FXU:
	return (Lop_I_F2);

    case TAHOEop_FPACK:
    case TAHOEop_FPAMAX:
    case TAHOEop_FPAMIN:
    case TAHOEop_FPMAX:
    case TAHOEop_FPMIN:
    case TAHOEop_FPCMP:
    case TAHOEop_FPCVT_FX:
    case TAHOEop_FPCVT_FX_TRUNC:
    case TAHOEop_FPCVT_FXU:
    case TAHOEop_FPCVT_FXU_TRUNC:
    case TAHOEop_FPMA:
    case TAHOEop_FPNMA:
    case TAHOEop_FPMS:
/*    case TAHOEop_FPMPY:
      case TAHOEop_FPNMPY:
      case TAHOEop_FPNEG:
      case TAHOEop_FPNEGABS:*/
    case TAHOEop_FPRCPA:
    case TAHOEop_FPRSQRTA:
    case TAHOEop_FPMERGE_NS:
    case TAHOEop_FPMERGE_S:
    case TAHOEop_FPMERGE:
    case TAHOEop_PACK2_SSS:
    case TAHOEop_PACK2_USS:
    case TAHOEop_PACK4_SSS:
    case TAHOEop_PSAD1:
    case TAHOEop_PAVG1:
    case TAHOEop_PAVG1_RAZ:
    case TAHOEop_PAVG2:
    case TAHOEop_PAVG2_RAZ:
    case TAHOEop_PAVGSUB1:
    case TAHOEop_PAVGSUB2:
    case TAHOEop_PSHL2:
    case TAHOEop_PSHL4:
    case TAHOEop_PSHLADD2:
    case TAHOEop_PSHR2:
    case TAHOEop_PSHR2_U:
    case TAHOEop_PSHR4:
    case TAHOEop_PSHR4_U:
    case TAHOEop_PSHRADD2:
    case TAHOEop_PADD1:
    case TAHOEop_PADD1_SSS:
    case TAHOEop_PADD1_UUS:
    case TAHOEop_PADD1_UUU:
    case TAHOEop_PADD2:
    case TAHOEop_PADD2_SSS:
    case TAHOEop_PADD2_UUS:
    case TAHOEop_PADD2_UUU:
    case TAHOEop_PADD4:
    case TAHOEop_PSUB1:
    case TAHOEop_PSUB1_SSS:
    case TAHOEop_PSUB1_UUS:
    case TAHOEop_PSUB1_UUU:
    case TAHOEop_PSUB2:
    case TAHOEop_PSUB2_SSS:
    case TAHOEop_PSUB2_UUS:
    case TAHOEop_PSUB2_UUU:
    case TAHOEop_PSUB4:
    case TAHOEop_PMPY2_R:
    case TAHOEop_PMPY2_L:
    case TAHOEop_PMPYSHR2:
    case TAHOEop_PMPYSHR2_U:
    case TAHOEop_PCMP1_EQ:
    case TAHOEop_PCMP1_GT:
    case TAHOEop_PCMP2_EQ:
    case TAHOEop_PCMP2_GT:
    case TAHOEop_PCMP_EQ:
    case TAHOEop_PCMP_GT:
    case TAHOEop_PMAX1_U:
    case TAHOEop_PMAX2:
    case TAHOEop_PMIN1_U:
    case TAHOEop_PMIN2:
    case TAHOEop_MUX1:
    case TAHOEop_MUX2:
	L_warn("Using Parallel Proc Op -> Intrinsic\n");
	return (Lop_INTRINSIC);
	break;

    default:
	fprintf (stderr, "Illegal proc_opc %d\n", proc_opc);
	M_assert (0, "M_opc_from_proc_opc_tahoe: illegal proc_opc");
	return (-1);
    } /* switch */
} /* M_opc_from_proc_opc_tahoe */


/*--------------------------------------------------------------------------*/

L_Operand *
M_tahoe_epilogue_cntr_register ()
{
  return L_new_macro_operand (TAHOE_MAC_EC, L_CTYPE_LLONG, L_PTYPE_NULL);
}

/*--------------------------------------------------------------------------*/

L_Operand *
M_tahoe_loop_cntr_register ()
{
  I_punt ("M_tahoe_loop_cntr_register");
  return NULL;
}

/*--------------------------------------------------------------------------*/


/****************************************************************************
 *
 * routine: M_cannot_predicate_tahoe
 * purpose: returns true if the oper cannot be predicated
 * input:
 * output:
 * returns:
 * modified: Created JEP 1/29/98
 * note:
    Pred clear instructions are assumed to have no qualifying 
    predicate by predicate flow analysis.  
 *-------------------------------------------------------------------------*/

int
M_cannot_predicate_tahoe (L_Oper * oper)
{

  switch (oper->opc)
    {

    case Lop_PRED_CLEAR:
    case Lop_PRED_SET:
      return (1);

    default:
      return (0);
    }
}

void
M_get_memory_operands_tahoe (int *first, int *number, int proc_opc)
{
  /*
   * After annotation, tahoe memory ops have no offset.
   *
   * This is grotesque, but it allows the TAHOE model to be used
   * before and after annotation.
   */

  if (proc_opc <= Lop_LAST_OP)
    {
      *first = 0;
      *number = 2;
    }
  else
    {
      *first = 0;
      *number = 1;
    }
}






