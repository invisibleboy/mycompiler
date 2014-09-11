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
/* 09/19/02 REK Modifying print_MOV_TOBR_completers.  The .ret completer
 *              is not completely supported yet, so it is not printed.
 */

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "ltahoe_main.h"
#include "ltahoe_completers.h"
#include "phase3.h"
#include "phase1_func.h"
#include "phase2_func.h"
#include "phase2_reg.h"
#include "phase3_unwind.h"
#include "phase3_oper.h"
#include <Lcode/l_code.h>
#include <stdio.h>

/* print_standard_completers prints the completers from all standard fields.
 * No opcode actually uses all the possible standard completers, so it is
 * necessary to provide a sanity checking function to make sure that only
 * the fields the opcode uses are defined in its completers field.
 */
void
print_standard_completers (L_Oper * op)
{
  print_branch_whether_completer (op);
  print_load_type_completer (op);
  print_comparison_op_completer (op);
  print_comparison_type_completer (op);
  print_branch_hint_completer (op, 1);
  print_branch_hint_completer (op, 2);
  print_temporal_locality_completer (op);
}				/* void print_standard_completers */


void
print_branch_whether_completer (L_Oper * op)
{
  switch (TC_GET_BR_WTHR (op->completers))
    {
    case TC_BR_WTHR_DPTK:
      P_line_print (".dptk");
      break;
    case TC_BR_WTHR_DPNT:
      P_line_print (".dpnt");
      break;
    case TC_BR_WTHR_SPTK:
      P_line_print (".sptk");
      break;
    case TC_BR_WTHR_SPNT:
      P_line_print (".spnt");
      break;
    case TC_BR_WTHR_NONE:
    default:
      break;
    }				/* switch */
}				/* print_branch_whether_completer */


void
print_load_type_completer (L_Oper * op)
{
  switch (TC_GET_LD_TYPE (op->completers))
    {
    case TC_LD_TYPE_A:
      P_line_print (".a");
      break;
    case TC_LD_TYPE_S:
      P_line_print (".s");
      break;
    case TC_LD_TYPE_SA:
      P_line_print (".sa");
      break;
    case TC_LD_TYPE_NONE:
    default:
      break;
    }				/* switch */
}				/* print_load_type_completer */


void
print_comparison_op_completer (L_Oper * op)
{
  switch (TC_GET_CMP_OP (op->completers))
    {
    case TC_CMP_OP_EQ:
      P_line_print (".eq");
      break;
    case TC_CMP_OP_NE:
      P_line_print (".ne");
      break;
    case TC_CMP_OP_LT:
      P_line_print (".lt");
      break;
    case TC_CMP_OP_LE:
      P_line_print (".le");
      break;
    case TC_CMP_OP_GT:
      P_line_print (".gt");
      break;
    case TC_CMP_OP_GE:
      P_line_print (".ge");
      break;
    case TC_CMP_OP_LTU:
      P_line_print (".ltu");
      break;
    case TC_CMP_OP_LEU:
      P_line_print (".leu");
      break;
    case TC_CMP_OP_GTU:
      P_line_print (".gtu");
      break;
    case TC_CMP_OP_GEU:
      P_line_print (".geu");
      break;
    case TC_CMP_OP_NEQ:
      P_line_print (".neq");
      break;
    case TC_CMP_OP_NLT:
      P_line_print (".nlt");
      break;
    case TC_CMP_OP_NLE:
      P_line_print (".nle");
      break;
    case TC_CMP_OP_NGT:
      P_line_print (".ngt");
      break;
    case TC_CMP_OP_NGE:
      P_line_print (".nge");
      break;
    case TC_CMP_OP_ORD:
      P_line_print (".ord");
      break;
    case TC_CMP_OP_UNORD:
      P_line_print (".unord");
      break;
    case TC_CMP_OP_Z:
      P_line_print (".z");
      break;
    case TC_CMP_OP_NZ:
      P_line_print (".nz");
      break;
    case TC_CMP_OP_M:
      P_line_print (".m");
      break;
    case TC_CMP_OP_NM:
      P_line_print (".nm");
      break;
    case TC_CMP_OP_NONE:
    default:
      break;
    }				/* switch */
}				/* print_comparison_op_completer */


void
print_comparison_type_completer (L_Oper * op)
{
  switch (TC_GET_CMP_TYPE (op->completers))
    {
    case TC_CMP_TYPE_UNC:
      P_line_print (".unc");
      break;
    case TC_CMP_TYPE_OR:
      P_line_print (".or");
      break;
    case TC_CMP_TYPE_AND:
      P_line_print (".and");
      break;
    case TC_CMP_TYPE_OR_ANDCM:
      P_line_print (".or.andcm");
      break;
    case TC_CMP_TYPE_ORCM:
      P_line_print (".orcm");
      break;
    case TC_CMP_TYPE_ANDCM:
      P_line_print (".andcm");
      break;
    case TC_CMP_TYPE_AND_ORCM:
      P_line_print (".and.orcm");
      break;
    case TC_CMP_TYPE_NONE:
    default:
      break;
    }				/* switch */
}				/* print_comparison_type_completer */


void
print_branch_hint_completer (L_Oper * op, int whichHint)
{
  int hint = TC_BR_HNT_NONE;

  switch (whichHint)
    {
    case 1:
      hint = TC_GET_BR_HNT1 (op->completers);
      break;
    case 2:
      hint = TC_GET_BR_HNT2 (op->completers);
      break;
    default:
      printf ("Unknown hint\n");
      break;
    }				/* switch */

  switch (hint)
    {
    case TC_BR_HNT_TK:
      P_line_print (".tk");
      break;
    case TC_BR_HNT_NT:
      P_line_print (".nt");
      break;
    case TC_BR_HNT_DC:
      P_line_print (".dc");
      break;
    case TC_BR_HNT_NONE:
    default:
      break;
    }				/* switch */
}				/* print_branch_hint_completer */


void
print_temporal_locality_completer (L_Oper * op)
{
  switch (TC_GET_TEMPORAL_LOCALITY (op->completers))
    {
    case TC_TEMPORAL_LOCALITY_NT1:
      P_line_print (".nt1");
      break;
    case TC_TEMPORAL_LOCALITY_NT2:
      P_line_print (".nt2");
      break;
    case TC_TEMPORAL_LOCALITY_NTA:
      P_line_print (".nta");
      break;
    case TC_TEMPORAL_LOCALITY_NONE:
    default:
      break;
    }				/* switch */
}				/* print_temporal_locality_completer */


void
print_LFETCH_completers (L_Oper * op)
{
  if (op->completers & TC_LFETCH_EXCL)
    P_line_print (".excl");

  print_temporal_locality_completer (op);
}				/* print_LFETCH_completers */


void
print_LD_C_completers (L_Oper * op)
{
  if (op->completers & TC_LD_C_NC)
    P_line_print (".nc");
  else if (op->completers & TC_LD_C_CLR)
    {
      P_line_print (".clr");

      if (op->completers & TC_LD_C_CLR_ACQ)
	P_line_print (".acq");
    }				/* else if */

  print_temporal_locality_completer (op);
}				/* print_LD_C_completers */


void
print_LDF_C_completers (L_Oper * op)
{
  if (op->completers & TC_LD_C_NC)
    P_line_print (".nc");
  else if (op->completers & TC_LD_C_CLR)
    P_line_print (".clr");

  print_temporal_locality_completer (op);
}				/* print_LD_C_completers */


void
print_CHK_A_completers (L_Oper * op)
{
  if (op->completers & TC_LD_C_NC)
    P_line_print (".nc");
  else if (op->completers & TC_LD_C_CLR)
    P_line_print (".clr");
}				/* print_CHK_A_completers */


void
print_FP_S_REG_completers (L_Oper * op)
{
  P_line_print (".s%d", TC_GET_FP_STATUS_REG (op->completers));
}				/* print_FP_S_REG_completers */


/* 09/19/02 REK Leaving off the .ret completer as it is not completely
 *              supported yet.
 */
void
print_MOV_TOBR_completers (L_Oper * op)
{
/*      if (op->completers & TC_MOV_TOBR_RET) */
/*  	P_line_print (".ret"); */

  print_branch_whether_completer (op);

  if (op->completers & TC_MOV_TOBR_MANY)
    P_line_print (".many");

  print_branch_hint_completer (op, 1);
  print_branch_hint_completer (op, 2);

  if (op->completers & TC_MOV_TOBR_IMP)
    P_line_print (".imp");
}				/* print_MOV_TOBR_completers */


void
print_CMP_completers (L_Oper * op)
{
  if (op->completers & TC_CMP_4)
    P_line_print ("4");

  print_comparison_op_completer (op);
  print_comparison_type_completer (op);
}				/* print_CMP_completers */


void
print_BR_completers (L_Oper * op)
{
  print_branch_whether_completer (op);

  switch (op->completers & TC_BR_FEW_MANY_MASK)
    {
    case TC_BR_FEW:
      P_line_print (".few");
      break;
    case TC_BR_MANY:
      P_line_print (".many");
      break;
    case TC_BR_NONE:
    default:
      break;
    }				/* switch */

  if (op->completers & TC_BR_CLR)
    P_line_print (".clr");
}				/* print_BR_completers */


void
print_BRP_completers (L_Oper * op)
{
  print_branch_whether_completer (op);

  if (op->completers & TC_BRP_LOOP)
    P_line_print (".loop");
  else if (op->completers & TC_BRP_EXIT)
    P_line_print (".exit");

  if (op->completers & TC_BRP_MANY)
    P_line_print (".many");

  print_branch_hint_completer (op, 1);
  print_branch_hint_completer (op, 2);

  if (op->completers & TC_BRP_IMP)
    P_line_print (".imp");
}				/* print_BRP_completers */


void
print_FCMP_completers (L_Oper * op)
{
  print_comparison_op_completer (op);
  print_comparison_type_completer (op);
  print_FP_S_REG_completers (op);
}				/* print_FCMP_completers */


void
print_FCVT_completers (L_Oper * op)
{
  if (op->completers & TC_FCVT_TRUNC)
    P_line_print (".trunc");

  print_FP_S_REG_completers (op);
}				/* print_FCVT_completers */


void
print_FPCMP_completers (L_Oper * op)
{
  print_comparison_op_completer (op);
  print_FP_S_REG_completers (op);
}				/* print_FPCMP_completers */


/* A function to perform sanity checking on the standard branch hint completer
 * fields.  
 * The invalid combinations of the hint completer fields are as follows:
 * Hint 1          Hint 2
 * NONE            TK, NT, or DC
 * DC              TK or DC
 * TK, NT, or DC   NONE
 * This function returns 0 if the branch hints are valid or 1 otherwise.
 */
int
check_branch_hints (L_Oper * op)
{
  int brHint1 = TC_GET_BR_HNT1 (op->completers);
  int brHint2 = TC_GET_BR_HNT2 (op->completers);

  if (brHint1 == TC_BR_HNT_NONE)
    {
      if (brHint2 != TC_BR_HNT_NONE)
	{
	  return (1);
	}			/* if */
    }				/* if */
  else if (brHint2 == TC_BR_HNT_NONE)
    {
      return (1);
    }				/* else if */
  else if (brHint1 == TC_BR_HNT_DC)
    {
      if (brHint2 != TC_BR_HNT_NT)
	{
	  return (1);
	}			/* if */
    }				/* else if */

  return (0);
}				/* check_branch_hints */


/* A function to perform sanity checking on the completers field of an LD
 * opcode.
 * An LD opcode may only use the TC_LD_TYPE and TC_TEMPORAL_LOCALITY standard
 * fields.  TC_TEMPORAL_LOCALITY_NT2 is invalid.
 * This function returns 0 if the completers field is valid or 1 otherwise.
 */
int
check_LD (L_Oper * op)
{
  if (TC_GET_TEMPORAL_LOCALITY (op->completers) == TC_TEMPORAL_LOCALITY_NT2)
    return (1);

  if (TC_GET_BR_HNT1 (op->completers) != TC_BR_HNT_NONE)
    return (1);
  if (TC_GET_BR_HNT2 (op->completers) != TC_BR_HNT_NONE)
    return (1);
  if (TC_GET_BR_WTHR (op->completers) != TC_BR_WTHR_NONE)
    return (1);
  if (TC_GET_CMP_OP (op->completers) != TC_CMP_OP_NONE)
    return (1);
  if (TC_GET_CMP_TYPE (op->completers) != TC_CMP_TYPE_NONE)
    return (1);

  /* This oper checks out. */
  return (0);
}				/* check_LD */


/* A function to perform sanity checking on the completers field of an 
 * *_ACQ, *_REL, *_BIAS, or *_FILL opcode.
 * These opcodes may only use the TC_TEMPORAL_LOCALITY standard
 * field.  TC_TEMPORAL_LOCALITY_NT2 is invalid.
 * This function returns 0 if the completers field is valid or 1 otherwise.
 */
int
check_ACQ_REL_BIAS_FILL (L_Oper * op)
{
  if (TC_GET_TEMPORAL_LOCALITY (op->completers) == TC_TEMPORAL_LOCALITY_NT2)
    return (1);

  if (TC_GET_BR_HNT1 (op->completers) != TC_BR_HNT_NONE)
    return (1);
  if (TC_GET_BR_HNT2 (op->completers) != TC_BR_HNT_NONE)
    return (1);
  if (TC_GET_BR_WTHR (op->completers) != TC_BR_WTHR_NONE)
    return (1);
  if (TC_GET_CMP_OP (op->completers) != TC_CMP_OP_NONE)
    return (1);
  if (TC_GET_CMP_TYPE (op->completers) != TC_CMP_TYPE_NONE)
    return (1);
  if (TC_GET_LD_TYPE (op->completers) != TC_LD_TYPE_NONE)
    return (1);

  /* This oper checks out. */
  return (0);
}				/* check_ACQ_REL_BIAS_FILL */


/* A function to perform sanity checking on the completers field of an
 * LDF* (but not LDF*_A) opcode.
 * An LDF* opcode may only use the TC_TEMPORAL_LOCALITY and TC_LD_TYPE
 * standard fields.  TC_TEMPORAL_LOCALITY_NT2, TC_LD_TYPE_SA, and TC_LD_TYPE_A
 * are invalid.
 * This function returns 0 if the completers field is valid or 1 otherwise.
 */
int
check_LDF (L_Oper * op)
{
  int ldType = TC_GET_LD_TYPE (op->completers);

  if (TC_GET_TEMPORAL_LOCALITY (op->completers) == TC_TEMPORAL_LOCALITY_NT2)
    return (1);

  if (ldType == TC_LD_TYPE_A || ldType == TC_LD_TYPE_SA)
    return (1);
  if (TC_GET_BR_HNT1 (op->completers) != TC_BR_HNT_NONE)
    return (1);
  if (TC_GET_BR_HNT2 (op->completers) != TC_BR_HNT_NONE)
    return (1);
  if (TC_GET_BR_WTHR (op->completers) != TC_BR_WTHR_NONE)
    return (1);
  if (TC_GET_CMP_OP (op->completers) != TC_CMP_OP_NONE)
    return (1);
  if (TC_GET_CMP_TYPE (op->completers) != TC_CMP_TYPE_NONE)
    return (1);

  /* This oper checks out. */
  return (0);
}				/* check_LDF */


/* A function to perform sanity checking on the completers field of an
 * LDF*_A opcode.
 * An LDF*_A opcode may only use the TC_TEMPORAL_LOCALITY and TC_LD_TYPE
 * standard fields.  TC_TEMPORAL_LOCALITY_NT2, TC_LD_TYPE_NONE, and
 * TC_LD_TYPE_S are invalid.
 * This function returns 0 if the completers field is valid or 1 otherwise.
 */
int
check_LDF_A (L_Oper * op)
{
  int ldType = TC_GET_LD_TYPE (op->completers);

  if (TC_GET_TEMPORAL_LOCALITY (op->completers) == TC_TEMPORAL_LOCALITY_NT2)
    return (1);

  if (ldType == TC_LD_TYPE_NONE || ldType == TC_LD_TYPE_S)
    return (1);
  if (TC_GET_BR_HNT1 (op->completers) != TC_BR_HNT_NONE)
    return (1);
  if (TC_GET_BR_HNT2 (op->completers) != TC_BR_HNT_NONE)
    return (1);
  if (TC_GET_BR_WTHR (op->completers) != TC_BR_WTHR_NONE)
    return (1);
  if (TC_GET_CMP_OP (op->completers) != TC_CMP_OP_NONE)
    return (1);
  if (TC_GET_CMP_TYPE (op->completers) != TC_CMP_TYPE_NONE)
    return (1);

  /* This oper checks out. */
  return (0);
}				/* check_LDF_A */


/* A function to perform sanity checking on the completers field of a MOV_TOBR
 * opcode.
 * A MOV_TOBR opcode may only use the TC_BR_WTHR and TC_BR_HINT standard 
 * fields.  TC_BR_WTHR_DPNT and SPNT are not valid in the BR_WTHR field.
 * This function returns 0 if the completers field is valid or 1 otherwise.
 */
int
check_MOV_TOBR (L_Oper * op)
{
  int brWthr = TC_GET_BR_WTHR (op->completers);

  if (brWthr == TC_BR_WTHR_DPNT || brWthr == TC_BR_WTHR_SPNT)
    return (1);

  return (check_branch_hints (op));
}				/* check_MOV_TOBR */


/* A function to perform sanity checking on the completers field of a CMP
 * opcode.
 * CMP has its own print_completers function, so we only need to make sure
 * that CMP_OP is not NONE, NEQ, NLT, NLE, NGT, NGE, ORD, UNORD, Z, NZ, M, or
 * NM.
 * This function returns 0 if the completers field is valid or 1 otherwise.
 */
/* 09/25/02 REK Updating to only accept unsigned compare operations for 
 *              non-parallel compare types (UNC and NONE). */
int
check_CMP (L_Oper * op)
{
  int cmpOp = TC_GET_CMP_OP (op->completers);
  int cmpType = TC_GET_CMP_TYPE (op->completers);

  switch (cmpOp)
    {
    case TC_CMP_OP_NONE:
    case TC_CMP_OP_NEQ:
    case TC_CMP_OP_NLT:
    case TC_CMP_OP_NLE:
    case TC_CMP_OP_NGT:
    case TC_CMP_OP_NGE:
    case TC_CMP_OP_ORD:
    case TC_CMP_OP_UNORD:
    case TC_CMP_OP_Z:
    case TC_CMP_OP_NZ:
    case TC_CMP_OP_M:
    case TC_CMP_OP_NM:
      return (1);
    default:
      break;
    }				/* switch */

  /* LTU, LEU, GTU, and GEU are valid only for a compare types UNC and
   * NONE. */
  if ((cmpType != TC_CMP_TYPE_UNC && cmpType != TC_CMP_TYPE_NONE) &&
      (cmpOp == TC_CMP_OP_LTU || cmpOp == TC_CMP_OP_LEU ||
       cmpOp == TC_CMP_OP_GTU || cmpOp == TC_CMP_OP_GEU))
    return (1);

  return (0);
}				/* check_CMP */


/* A function to perform sanity checking on the completers field of a TBIT or
 * TNAT opcode.
 * TBIT and TNAT use only the TC_CMP_OP (Z or NZ only) and TC_CMP_TYPE
 * standard fields.
 * This function returns 0 if the completers field is valid or 1 otherwise.
 */
int
check_TBIT_TNAT (L_Oper * op)
{
  int cmpOp = TC_GET_CMP_OP (op->completers);

  if (TC_GET_TEMPORAL_LOCALITY (op->completers) != TC_TEMPORAL_LOCALITY_NONE)
    return (1);

  if (TC_GET_LD_TYPE (op->completers) != TC_LD_TYPE_NONE)
    return (1);
  if (TC_GET_BR_HNT1 (op->completers) != TC_BR_HNT_NONE)
    return (1);
  if (TC_GET_BR_HNT2 (op->completers) != TC_BR_HNT_NONE)
    return (1);
  if (TC_GET_BR_WTHR (op->completers) != TC_BR_WTHR_NONE)
    return (1);
  if (cmpOp != TC_CMP_OP_Z && cmpOp != TC_CMP_OP_NZ)
    return (1);

  return (0);
}				/* check_TBIT_TNAT */


/* A function to perform sanity checking on the completers field of a BRP
 * opcode.
 * BRP uses only TC_BR_WTHR_SPTK, DPTK, and the branch hints fields.
 * One of TC_BR_WTHR_SPTK, TC_BR_WTHR_DPTK, TC_BRP_LOOP, or TC_BRP_EXIT must
 * be set.
 * This function returns 0 if the completers field is valid or 1 otherwise.
 */
int
check_BRP (L_Oper * op)
{
  int brWthr = TC_GET_BR_WTHR (op->completers);

  if (brWthr == TC_BR_WTHR_NONE && !(op->completers & TC_BRP_LOOP ||
				     op->completers & TC_BRP_EXIT))
    return (1);

  if (brWthr != TC_BR_WTHR_SPTK &&
      brWthr != TC_BR_WTHR_DPTK && brWthr != TC_BR_WTHR_NONE)
    return (1);

  return (check_branch_hints (op));
}				/* check_BRP */


/* A function to perform sanity checking on the completers field of a BRP_RET
 * opcode.
 * BRP_RET uses only TC_BR_WTHR_SPTK, DPTK, and the branch hints fields.
 * Additionally, TC_BRP_LOOP and TC_BRP_EXIT are invalid for this opcode.
 * This function returns 0 if the completers field is valid or 1 otherwise.
 */
int
check_BRP_RET (L_Oper * op)
{
  int brWthr = TC_GET_BR_WTHR (op->completers);

  if (brWthr != TC_BR_WTHR_SPTK && brWthr != TC_BR_WTHR_DPTK)
    return (1);

  if (op->completers & TC_BRP_LOOP || op->completers & TC_BRP_EXIT)
    return (1);

  return (check_branch_hints (op));
}				/* check_BRP_RET */


/* A function to perform sanity checking on the completers field of an FCLASS
 * opcode.
 * FCLASS uses only TC_CMP_OP_M, TC_CMP_OP_NM, TC_CMP_TYPE_NONE, and 
 * TC_CMP_TYPE_UNC.
 * This function returns 0 if the completers field is valid or 1 otherwise.
 */
int
check_FCLASS (L_Oper * op)
{
  int cmpOp = TC_GET_CMP_OP (op->completers);
  int cmpType = TC_GET_CMP_TYPE (op->completers);

  if ((cmpOp != TC_CMP_OP_M && cmpOp != TC_CMP_OP_NM) ||
      (cmpType != TC_CMP_TYPE_NONE && cmpType != TC_CMP_TYPE_UNC))
    return (1);

  return (0);
}				/* check_FCLASS */


/* A function to perform sanity checking on the completers field of an FCMP
 * opcode.
 * FCMP uses only TC_CMP_OP and TC_CMP_TYPE.  For the CMP_TYPE field, only
 * NONE and UNC are allowed.  For the CMP_OP field, the following values
 * are invalid: NONE, NE, LTU, LEU, GTU, GEU, Z, NZ, M, and NM.
 * This function returns 0 if the completers field is valid or 1 otherwise.
 */
int
check_FCMP (L_Oper * op)
{
  int cmpType = TC_GET_CMP_TYPE (op->completers);

  if (cmpType != TC_CMP_TYPE_NONE && cmpType != TC_CMP_TYPE_UNC)
    return (1);

  switch (TC_GET_CMP_OP (op->completers))
    {
    case TC_CMP_OP_NONE:
    case TC_CMP_OP_NE:
    case TC_CMP_OP_LTU:
    case TC_CMP_OP_LEU:
    case TC_CMP_OP_GTU:
    case TC_CMP_OP_GEU:
    case TC_CMP_OP_Z:
    case TC_CMP_OP_NZ:
    case TC_CMP_OP_M:
    case TC_CMP_OP_NM:
      return (1);
    default:
      break;
    }				/* switch */

  return (0);
}				/* check_FCMP */


/* A function to perform sanity checking on the completers field of an FPCMP
 * opcode.
 * FPCMP uses only TC_CMP_OP.  The following values are invalid: NONE, NE, LTU
 * LEU, GTU, GEU, Z, NZ, M, and NM.
 * This function returns 0 if the completers field is valid or 1 otherwise.
 */
int
check_FPCMP (L_Oper * op)
{
  switch (TC_GET_CMP_OP (op->completers))
    {
    case TC_CMP_OP_NONE:
    case TC_CMP_OP_NE:
    case TC_CMP_OP_LTU:
    case TC_CMP_OP_LEU:
    case TC_CMP_OP_GTU:
    case TC_CMP_OP_GEU:
    case TC_CMP_OP_Z:
    case TC_CMP_OP_NZ:
    case TC_CMP_OP_M:
    case TC_CMP_OP_NM:
      return (1);
    default:
      break;
    }				/* switch */

  return (0);
}				/* check_FPCMP */


/* A function to perform sanity checking on the completers field of an ST
 * opcode.
 * ST uses only TC_TEMPORAL_LOCALITY.  Only TC_TEMPORAL_LOCALITY_NONE and NTA
 * are allowed.
 * This function returns 0 if the completers field is valid or 1 otherwise.
 */
int
check_ST (L_Oper * op)
{
  int temporalLocality = TC_GET_TEMPORAL_LOCALITY (op->completers);

  if (temporalLocality == TC_TEMPORAL_LOCALITY_NT1 ||
      temporalLocality == TC_TEMPORAL_LOCALITY_NT2)
    return (1);

  if (TC_GET_BR_HNT1 (op->completers) != TC_BR_HNT_NONE)
    return (1);
  if (TC_GET_BR_HNT2 (op->completers) != TC_BR_HNT_NONE)
    return (1);
  if (TC_GET_BR_WTHR (op->completers) != TC_BR_WTHR_NONE)
    return (1);
  if (TC_GET_CMP_OP (op->completers) != TC_CMP_OP_NONE)
    return (1);
  if (TC_GET_CMP_TYPE (op->completers) != TC_CMP_TYPE_NONE)
    return (1);

  /* This oper checks out. */
  return (0);
}				/* check_ST */
