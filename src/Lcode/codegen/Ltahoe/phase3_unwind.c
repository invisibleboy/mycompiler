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
 * phase3_unwind.h                                                           *
 * ------------------------------------------------------------------------- *
 * Generation of unwind directives                                           *
 *                                                                           *
 * AUTHORS: R.D. Barnes                                                      *
 *****************************************************************************/
/* 09/16/02 REK Updating to use the new opcode map and completers scheme. */

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "ltahoe_main.h"
#include "phase2_reg.h"
#include "phase3.h"
#include "phase3_unwind.h"

unwind_info unwind;

void
L_get_unwind_info (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *oper;
  int tahoeop;
  UnwindReg *uwr;

  memset (&unwind, 0, sizeof (unwind));

#define UPDATEFIRSTLAST(un, op) \
	 { if (!(un).first_prologue_inst) \
            (un).first_prologue_inst = oper; \
	   (un).last_prologue_inst = oper; }

#define ISTMPREG1(opd) \
         (L_is_macro ((opd)) && (opd)->value.mac == TAHOE_MAC_TMPREG1)

  if (Ltahoe_generate_unwind_directives)
    {
      for (cb = fn->first_cb; cb; cb = cb->next_cb)
	{
	  for (oper = cb->first_op; oper; oper = oper->next_op)
	    {
	      tahoeop = oper->proc_opc;
	      switch (tahoeop)
		{
		  /* spill-to-reg cases */

		case TAHOEop_ALLOC:
		  unwind.pfs.save_op = oper;
		  unwind.pfs.in_reg = 1;
		  unwind.pfs.loc.reg = oper->dest[0];
		  UPDATEFIRSTLAST (unwind, oper);
		  break;

		case TAHOEop_MOV_FRBR:
		  if ((L_is_macro (oper->src[0]))
		      && (oper->src[0]->value.mac == TAHOE_MAC_RETADDR))
		    {
		      unwind.rp.save_op = oper;
		      unwind.rp.in_reg = 1;
		      unwind.rp.loc.reg = oper->dest[0];
		      UPDATEFIRSTLAST (unwind, oper);
		    }		/* if */
		  break;

		case TAHOEop_MOV_FRAR_I:
		case TAHOEop_MOV_FRAR_M:
		  if (!L_is_macro (oper->src[0]))
		    L_punt ("L_get_unwind_info: MOV_FROM_AR w/o mac src");

		  switch (oper->src[0]->value.mac)
		    {
		    case TAHOE_MAC_UNAT:
		      uwr = (&unwind.unat);
		      break;
		    case TAHOE_MAC_RNAT:
		      uwr = (&unwind.rnat);
		      break;
		    case TAHOE_MAC_LC:
		      uwr = (&unwind.lc);
		      break;
		    case TAHOE_MAC_FPSR:
		      uwr = (&unwind.fpsr);
		      break;
		    case TAHOE_MAC_BSP:
		      L_punt ("L_get_unwind_info error: "
			      "Saving bsp!  (See RDB)");
		    default:
		      uwr = NULL;
		    }		/* switch */

		  if (uwr)
		    {
		      uwr->save_op = oper;
		      uwr->in_reg = 1;
		      uwr->loc.reg = oper->dest[0];
		      UPDATEFIRSTLAST (unwind, oper);
		    }		/* if */
		  break;

		case TAHOEop_MOV_FRPR:
		  if (!unwind.pr.save_op)
		    {
		      unwind.pr.save_op = oper;
		      unwind.pr.in_reg = 1;
		      unwind.pr.loc.reg = oper->dest[0];
		      UPDATEFIRSTLAST (unwind, oper);
		    }		/* if */
		  break;

		  /* spill-to-mem cases */

		case TAHOEop_ST8:
		case TAHOEop_ST8_SPILL:
		  if (unwind.pfs.in_reg &&
		      L_same_operand (oper->src[1], unwind.pfs.loc.reg))
		    uwr = &(unwind.pfs);
		  else if (unwind.rp.in_reg &&
			   L_same_operand (oper->src[1], unwind.rp.loc.reg))
		    uwr = &(unwind.rp);
		  else if (unwind.unat.in_reg &&
			   L_same_operand (oper->src[1], unwind.unat.loc.reg))
		    uwr = &(unwind.unat);
		  else if (unwind.rnat.in_reg &&
			   L_same_operand (oper->src[1], unwind.rnat.loc.reg))
		    uwr = &(unwind.rnat);
		  else if (unwind.lc.in_reg &&
			   L_same_operand (oper->src[1], unwind.lc.loc.reg))
		    uwr = &(unwind.lc);
		  else if (unwind.fpsr.in_reg &&
			   L_same_operand (oper->src[1], unwind.fpsr.loc.reg))
		    uwr = &(unwind.fpsr);
		  else if (unwind.pr.in_reg &&
			   L_same_operand (oper->src[1], unwind.pr.loc.reg))
		    uwr = &(unwind.pr);
		  else
		    uwr = NULL;

		  if (uwr)
		    {
		      if (!ISTMPREG1 (oper->src[0]))
			L_punt ("L_get_unwind_info error: "
				"Spill not using TMPREG1!");

		      uwr->save_op = oper;
		      uwr->in_reg = 0;
		      uwr->loc.ofst = unwind.temp_reg_offset;
		      UPDATEFIRSTLAST (unwind, oper);
		    }		/* if */

		  break;

		  /* SP-adjustment and address calculation */

		case TAHOEop_ADD:
		case TAHOEop_ADDS:
		  if (L_is_macro (oper->src[1]) &&
		      (oper->src[1]->value.mac == L_MAC_SP) &&
		      L_is_macro (oper->dest[0]))
		    {
		      switch (oper->dest[0]->value.mac)
			{
			case L_MAC_SP:
			  if (!unwind.mem_stack)
			    {
			      unwind.mem_stack = oper;
			      if (L_is_constant (oper->src[0]))
				{
				  unwind.mem_stack_size =
				    oper->src[0]->value.i;
				}	/* if */
			      else if ((L_is_macro (oper->src[0]))
				       && (oper->src[0]->value.mac ==
					   TAHOE_MAC_TMPREG1))
				{
				  if (!unwind.temp_reg_sp_rel)
				    unwind.mem_stack_size =
				      unwind.temp_reg_absolute;
				  else
				    L_punt
				      ("L_get_unwind_info error:  "
				       "Non-constant stack-frame adjustment!");
				}	/* else if */
			      else
				{
				  L_punt
				    ("L_get_unwind_info error:  "
				     "Non-constant stack-frame adjustment!");
				}	/* else */
			      UPDATEFIRSTLAST (unwind, oper);
			    }	/* if */
			  else
			    {
			      if (L_is_constant (oper->src[0]))
				{
				  if ((-(oper->src[0]->value.i)) ==
				      unwind.mem_stack_size)
				    unwind.mem_stack_dealloc = oper;
				  else
				    L_punt ("L_get_unwind_info error:  "
					    "Non-matching stack frame adjustments!");
				}	/* if */
			      else if ((L_is_macro (oper->src[0]))
				       && (oper->src[0]->value.mac ==
					   TAHOE_MAC_TMPREG1))
				{
				  if (unwind.temp_reg_sp_rel == 0)
				    {
				      if ((-(unwind.temp_reg_absolute)) ==
					  unwind.mem_stack_size)
					unwind.mem_stack_dealloc = oper;
				      else
					L_punt ("L_get_unwind_info error:  "
						"Non-matching stack frame adjustments!");
				    }	/* if */
				  else
				    {
				      L_punt ("L_get_unwind_info error:  "
					      "Non-constant stack-frame adjustment!");
				    }	/* else */
				}	/* else if */
			      else
				{
				  L_punt ("L_get_unwind_info error:  "
					  "Non-constant stack-frame adjustment!");
				}	/* else */
			    }	/* else */

			  break;
			case TAHOE_MAC_TMPREG1:
			  if (L_is_constant (oper->src[0]))
			    {
			      unwind.temp_reg_sp_rel = 1;
			      unwind.temp_reg_offset = oper->src[0]->value.i;
			    }	/* if */
			  else
			    {
			      L_punt
				("L_get_unwind_info error:  Non-constant stack offset!");
			    }	/* else */
			  break;
			default:
			  break;
			}	/* switch */
		    }		/* if */
		  break;

		case TAHOEop_MOVI:
		case TAHOEop_MOVL:
		  if ((L_is_macro (oper->dest[0]))
		      && (oper->dest[0]->value.mac == TAHOE_MAC_TMPREG1))
		    {
		      if (L_is_constant (oper->src[0]))
			{
			  unwind.temp_reg_sp_rel = 0;
			  unwind.temp_reg_absolute = oper->src[0]->value.i;
			}	/* if */
		      else
			{
			  L_warn ("L_get_unwind_info warning: "
				  "Moving an unknown register into r2.");
			}	/* else */
		    }		/* if */

		default:
		  break;
		}		/* switch */
	    }			/* for oper */
	}			/* for cb */
    }				/* if */
}				/* L_get_unwind_info */
