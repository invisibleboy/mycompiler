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
 * phase3_func.c                                                             *
 * ------------------------------------------------------------------------- *
 * Assembly printing                                                         *
 *                                                                           *
 * AUTHORS: B. McGowan, D.A. Connors, J.W. Sias                              *
 *****************************************************************************/
/* 09/12/02 REK Updating file to use the new opcode map and completers
 *              scheme. */
/* 01/13/03 REK Changing references to psp to pspr.  GNU as considers psp a
 *              reserved name and does not like using it as an operand. */

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "ltahoe_main.h"
#include "ltahoe_op_query.h"
#include <Lcode/r_regalloc.h>
#include "phase3.h"
#include "phase3_unwind.h"
#include "phase2_reg.h"
#include "phase1_func.h"
#include "phase1_varargs.h"
#include "phase2_br_hint.h"

#include <assert.h>

#undef DEBUG

int num_reg_stack_inputs = -1;
int num_reg_stack_locals = -1;
int num_reg_stack_outputs = -1;
int num_reg_stack_rots = -1;


/****************************************************************************
 *
 * routine: P_init()
 * purpose: Phase 3 intialization
 * input:
 * output:
 * returns:
 * modified:
 * note:
 *-------------------------------------------------------------------------*/

void
P_init (Parm_Macro_List * command_line_macro_list)
{
  ;
}				/* P_init */

extern void P_clear_current_ms (void);

void
P_file_init (void)
{
  fprintf (L_OUT, "// IMPACT/iA64 Generated Assembly Code\n");
  P_symtab_init ();
  P_clear_current_ms ();
}				/* P_file_init */


/****************************************************************************
 *
 * routine:
 * purpose: 
 * input:
 * output:
 * returns:
 * modified:
 * note:
 *-------------------------------------------------------------------------*/

void
P_end (void)
{
  ;
}				/* P_end */

void
P_file_end (void)
{
  P_symtab_print_extern ();
  P_symtab_deinit ();
}				/* P_file_end */

static char P_macro_buf[16];

static char *
P_int_reg_name (int reg_id)
{
  if (Ltahoe_print_real_regs || (reg_id < INT_STACK_REG_BASE))
    sprintf (P_macro_buf, "r%d", reg_id);
  else if (reg_id < INT_STACK_REG_BASE + num_reg_stack_inputs)
    sprintf (P_macro_buf, "in%d", reg_id - INT_STACK_REG_BASE);
  else if (reg_id < INT_STACK_REG_BASE + num_reg_stack_inputs +
	   num_reg_stack_locals)
    sprintf (P_macro_buf, "loc%d", reg_id - INT_STACK_REG_BASE -
	     num_reg_stack_inputs);
  else if (reg_id < INT_STACK_REG_BASE + num_reg_stack_inputs +
	   num_reg_stack_locals + num_reg_stack_outputs)
    sprintf (P_macro_buf, "out%d", reg_id - INT_STACK_REG_BASE -
	     num_reg_stack_inputs - num_reg_stack_locals);
  else
    L_punt ("P_print_int_reg_name: Failed to map register %d", reg_id);
  return P_macro_buf;
}				/* P_int_reg_name */

static char *
P_macro_name (int reg_id)
{
  int mac, real_reg_id;

  switch (reg_id)
    {
    case L_MAC_SP:
    case L_MAC_SAFE_MEM:
    case L_MAC_LV:
    case L_MAC_OP:
    case L_MAC_IP:
      if (Ltahoe_print_real_regs)
	return "r12";
      else
	return "sp";

    case TAHOE_MAC_GP:
      return "gp";

    case TAHOE_MAC_AP:
      return "r9";

    case TAHOE_MAC_ZERO:
      return "r0";

    case TAHOE_MAC_TMPREG1:
      return "r2";

    case TAHOE_MAC_TMPREG2:
      return "r3";

    case TAHOE_MAC_FZERO:
      return "f0";

    case TAHOE_MAC_FONE:
      return "f1";

    case TAHOE_MAC_RETADDR:
      return "b0";

    case TAHOE_MAC_PRED_TRUE:
      return "p0";

    case TAHOE_MAC_AR_PFS:
      return "ar.pfs";

    case TAHOE_MAC_UNAT:
      return "ar.unat";

    case TAHOE_MAC_LC:
      return "ar.lc";

    case TAHOE_MAC_EC:
      return "ar.ec";

    case TAHOE_PRED_SAVE_REG:
      if (Ltahoe_print_real_regs)
	{
	  real_reg_id = INT_STACK_REG_BASE +
	    num_reg_stack_inputs + num_reg_stack_locals - 2;
	  sprintf (P_macro_buf, "r%d", real_reg_id);
	  return P_macro_buf;
	}			/* if */
      else
	{
	  return "prsav";
	}			/* else */

    case TAHOE_PRED_BLK_REG:
      return "pr";

    case TAHOE_GP_SAVE_REG:
      if (Ltahoe_print_real_regs)
	{
	  real_reg_id = INT_STACK_REG_BASE +
	    num_reg_stack_inputs + num_reg_stack_locals - 1;
	  sprintf (P_macro_buf, "r%d", real_reg_id);
	  return P_macro_buf;
	}			/* if */
      else
	{
	  return "gpsav";
	}			/* else */

    case L_MAC_P0:
    case L_MAC_P1:
    case L_MAC_P2:
    case L_MAC_P3:
    case L_MAC_P4:
    case L_MAC_P5:
    case L_MAC_P6:
    case L_MAC_P7:
      /* These are input parameters */
      mac = reg_id - L_MAC_P0;
      real_reg_id = INT_STACK_REG_BASE + mac;
      if (Ltahoe_print_real_regs)
	sprintf (P_macro_buf, "r%d", real_reg_id);
      else
	sprintf (P_macro_buf, "in%d", mac);
      return P_macro_buf;

    case L_MAC_P8:
    case L_MAC_P9:
    case L_MAC_P10:
    case L_MAC_P11:
    case L_MAC_P12:
    case L_MAC_P13:
    case L_MAC_P14:
    case L_MAC_P15:
      /* These are integer output parameters */
      mac = reg_id - L_MAC_P8;
      real_reg_id = INT_STACK_REG_BASE + num_reg_stack_inputs
	+ num_reg_stack_locals + mac;
      if (Ltahoe_print_real_regs)
	sprintf (P_macro_buf, "r%d", real_reg_id);
      else
	sprintf (P_macro_buf, "out%d", mac);
      return P_macro_buf;

    case L_MAC_P16:
    case L_MAC_P17:
    case L_MAC_P18:
    case L_MAC_P19:
      /* This is a return value, and doesn't go through register stack */
      mac = reg_id - L_MAC_P16;
      real_reg_id = INT_RETURN_VALUE_REG + mac;
      sprintf (P_macro_buf, "r%d", real_reg_id);
      return P_macro_buf;

    case L_MAC_P20:
    case L_MAC_P21:
    case L_MAC_P22:
    case L_MAC_P23:
    case L_MAC_P24:
    case L_MAC_P25:
    case L_MAC_P26:
    case L_MAC_P27:
      /* These are all float output parameters */
      /* Map the parameter position to the floating point registers */
      mac = reg_id - L_MAC_P20;
      real_reg_id = FLT_INPUT_PARMS_START + mac;
      sprintf (P_macro_buf, "f%d", real_reg_id);
      return P_macro_buf;

    case L_MAC_P28:
    case L_MAC_P29:
    case L_MAC_P30:
    case L_MAC_P31:
    case L_MAC_P32:
    case L_MAC_P33:
    case L_MAC_P34:
    case L_MAC_P35:
      /* This is a return value, and doesn't go through register
         stack */
      mac = reg_id - L_MAC_P28;
      real_reg_id = FLT_RETURN_VALUE_REG + mac;
      sprintf (P_macro_buf, "f%d", real_reg_id);
      return P_macro_buf;

    case TAHOE_MAC_PSP:
      real_reg_id = INT_STACK_REG_BASE + num_reg_stack_inputs +
	num_reg_stack_locals - 3;
      if (Ltahoe_print_real_regs)
	sprintf (P_macro_buf, "r%d", real_reg_id);
      else
	sprintf (P_macro_buf, "pspr");
      return P_macro_buf;

    default:
      L_punt ("P_macro_name: bogus macro %d", reg_id);
    }				/* switch */
  return NULL;
}				/* P_macro_name */

/****************************************************************************
 *
 * routine: P_print_register_set()
 * purpose: Print out the registers in the given set.  The set has been
 *          taken from live variable analysis.  The register numbers for
 *          integer, fp, etc do not overlap.  Each group has a unique range.
 *          The numbers should be converted to the more conventional human
 *          readable form.  Also recall that registers use only the even
 *          numbers since the odd numbers are reserved for the macros.
 *          ex: 40 -> r20
 *              41 -> macro 20
 * input: reg_set - set of registers to print
 * output:
 * returns:
 * modified: 2/7/97 - Bob McGowan - created
 * note:
 *-------------------------------------------------------------------------*/

#define BEST_LIVE_REGS_PER_LINE 10	/* number of live registers to print per
					 * line in the cb header in assembly 
					 */
#define MAX_LIVE_REGS_PER_LINE 12

#define IS_ODD( a )  (((a) & 0x1) ? 1 : 0)
#define IS_EVEN( a ) (((a) & 0x1) ? 0 : 1)

void
P_print_register_set (Set reg_set)
{
  int spot, even, reg_id, set_length, regs_printed = 0;
  int reg_array[NUM_INT_REG + NUM_PRED_REG + NUM_FLOAT_REG + NUM_BRANCH_REG];

  set_length = Set_size (reg_set);
  Set_2array (reg_set, reg_array);

  fprintf (L_OUT, "// Live-in: ");
#ifdef DEBUG
  Set_print (L_OUT, "Live-in: ", reg_set);
#endif

  for (even = 0; even <= 1; even++)
    {
      for (spot = 0; spot < set_length; spot++)
	{
	  if ((regs_printed >= BEST_LIVE_REGS_PER_LINE) &&
	      ((set_length - spot) > (MAX_LIVE_REGS_PER_LINE -
				      BEST_LIVE_REGS_PER_LINE)))
	    {
	      fprintf (L_OUT, "\n//          ");
	      regs_printed = 0;
	    }			/* if */
	  reg_id = reg_array[spot];
	  if (even && IS_EVEN (reg_id))
	    {
	      reg_id >>= 1;	/* divide by 2 */
	      if (IS_INT_REGISTER (reg_id))
		{
		  /* integer register */
		  fprintf (L_OUT, "%s ", P_int_reg_name (reg_id));
		  regs_printed++;
		  continue;
		}		/* if */
	      if (IS_FP_REGISTER (reg_id))
		{
		  /* floating point register */
		  fprintf (L_OUT, "f%d ", reg_id - FLOAT_REG_BASE);
		  regs_printed++;
		  continue;
		}		/* if */
	      if (IS_BRANCH_REGISTER (reg_id))
		{
		  /* branch register */
		  fprintf (L_OUT, "b%d ", reg_id - BRANCH_REG_BASE);
		  regs_printed++;
		  continue;
		}		/* if */
	      if (IS_PREDICATE_REGISTER (reg_id))
		{
		  /* predicate register */
		  fprintf (L_OUT, "p%d ", reg_id - PRED_REG_BASE);
		  regs_printed++;
		  continue;
		}		/* if */
	    }			/* if */
	  else if (!even && IS_ODD (reg_id))
	    {
	      /* odd number means that it is a macro */
	      reg_id >>= 1;	/* divide by 2 */

	      fprintf (L_OUT, "%s ", P_macro_name (reg_id));
	      regs_printed++;
	    }			/* else if */
	}			/* for spot */
    }				/* for even */
  fprintf (L_OUT, "\n");
  return;
}				/* P_print_register_set */


/****************************************************************************
 *
 * routine: P_print_cb_info()
 * purpose: Print information about the cb to the .s file
 *          This prints the block number, successors, predecessors, and
 *          cb weight.
 * input:
 * output:
 * returns:
 * modified:
 * note:
 *-------------------------------------------------------------------------*/

void
P_print_cb_info (L_Func * fn, L_Cb * cb)
{
  L_Flow *flow_ptr;
  L_Attr *attr;
  char flagstr[64];

  if (cb != fn->first_cb)
    fprintf (L_OUT, "\n");

  L_cb_flags_to_string (flagstr, cb->flags);

  /* Print out all of the CB successors and info */
  fprintf (L_OUT, "// CB %d <%s> ", cb->id, flagstr);

  /* region */

  if ((attr = L_find_attr (cb->attr, "region")))
    {
      int region_id;

      if ((region_id = L_get_int_attr_field (attr, 0)))
	fprintf (L_OUT, "Region %d  ", region_id);
    }				/* if */

  if ((attr = L_find_attr (cb->attr, "live1")))
    {
      int l1, l2;
      double exprat;
      l1 = (int) L_get_int_attr_field (attr, 0);
      attr = L_find_attr (cb->attr, "live2");
      l2 = (int) L_get_int_attr_field (attr, 0);
      exprat = (double) l2 / l1;

      fprintf (L_OUT, "regpress %d %d %0.2f ", l1, l2, exprat);
    }

  /* Predecessors */
  fprintf (L_OUT, "Pred: ");
  for (flow_ptr = cb->src_flow; flow_ptr; flow_ptr = flow_ptr->next_flow)
    fprintf (L_OUT, "%d ", flow_ptr->src_cb->id);
  fprintf (L_OUT, "  ");

  /* Successors */
  fprintf (L_OUT, "Succ: ");
  for (flow_ptr = cb->dest_flow; flow_ptr; flow_ptr = flow_ptr->next_flow)
    fprintf (L_OUT, "%d ", flow_ptr->dst_cb->id);

  fprintf (L_OUT, "\n// ");

  /* frequency i.e. cb weight */
  fprintf (L_OUT, "Freq %12.3f ", cb->weight);

  /* unrolled loop */
  if ((attr = L_find_attr (cb->attr, "unroll_AMP")))
    fprintf (L_OUT, "\tUnrolled: %d", L_get_int_attr_field (attr, 0));
  fprintf (L_OUT, "\n");

  if ((attr = L_find_attr (cb->attr, "LOOP")))
    {
      char *type, *file = "(unknown file)";
      int line;

      type = attr->field[0]->value.s;
      line = (int) attr->field[1]->value.i;

      if ((attr = L_find_attr (cb->attr, "FILE")))
	file = attr->field[0]->value.s;

      fprintf (L_OUT, "// Loop %s @ %s:%d\n", type, file, line);
    }				/* if */

  /* live-in registers */
  if (Ltahoe_print_live_registers)
    P_print_register_set (L_get_cb_IN_set (cb));

  /* Hyperblock Information */
  if ((attr = L_find_attr (cb->attr, "hyper-list")) != NULL)
    {
      int n, i;

      n = attr->max_field;
      fprintf (L_OUT, "\n// Hyperblock: %s ( ", attr->field[0]->value.l);
      for (i = 1; i < n; i++)
	fprintf (L_OUT, "%d ", L_get_int_attr_field (attr, i));
      fprintf (L_OUT, ")\n");
    }				/* if */
  /* print modulo scheduling information */
  if (L_EXTRACT_BIT_VAL(cb->flags, L_CB_SOFTPIPE))
    {
      int i, stages=-1, ii=-1, rr=-1;

      /* read attr info */
      if ((attr = L_find_attr (cb->attr, "II")))
	ii = L_get_int_attr_field(attr, 0);
      if ((attr = L_find_attr (cb->attr, "stage_count")))
	stages = L_get_int_attr_field(attr, 0);
      if ((attr = L_find_attr (cb->attr, "rr")))
	rr = L_get_int_attr_field(attr, 0);
      
      fprintf(L_OUT, "// II: %d - STAGES: %d - RR: ", ii, stages);
      /* print rotating register mappings */
      
      for (i = 0; i < rr; i++)
	{
	  if (i < num_reg_stack_inputs)
	    fprintf(L_OUT, "in%d => ", i);
	  else if (i < num_reg_stack_inputs + num_reg_stack_locals)
	    fprintf(L_OUT, "loc%d => ", i - num_reg_stack_inputs);
	  else if (i < num_reg_stack_inputs + num_reg_stack_locals + num_reg_stack_outputs)
	    fprintf(L_OUT, "loc%d => ", i - num_reg_stack_inputs - num_reg_stack_outputs);
	  else
	    L_punt("Number of Rotating Regs is > (locals + inputs + outputs)\n");
	}
      fprintf(L_OUT, "x\n");
    }
}				/* P_print_cb_info */

/****************************************************************************
 *
 * routine: P_force_recovery_execution()
 * purpose: Changes source of speculative load instructions to NULL to force
            the execution of recovery blocks.
 * input:   
 * output:
 * returns:
 * modified:
 * note:
 *-------------------------------------------------------------------------*/
void
P_force_recovery_execution (L_Func * fn)
{
  L_Oper *oper, *add_op, *define_op, *new_define_op, *prev_define_op, *no_op;
  L_Oper *following_op;
  L_Cb *cb;
  int proc_opc, num_following = 0;
  int lds_count = -1;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
	{
	  if (L_general_load_opcode (oper) &&
	      L_EXTRACT_BIT_VAL (oper->flags, L_OPER_MASK_PE))
	    {
#if 0
	      if (!L_find_attr (oper->attr, "SPECID"))
		{
		  /* SPECIAL CASE 
		   *  Op does is speculative only because it is dependent
		   *  on another speculative load. There is no recovery
		   *  code so forcing can break program.
		   */
		  printf ("Not forcing op%d\n", oper->id);
		  continue;
		}		/* if */
#endif
	      lds_count++;
	      if (lds_count > Ltahoe_force_recovery_upper ||
		  lds_count < Ltahoe_force_recovery_lower)
		continue;

	      if (L_postincrement_load_opcode (oper))
		{
		  proc_opc = oper->proc_opc;
		  L_change_opcode (oper, L_base_memory_opcode (oper));

/* 09/12/02 REK proc_opc will already be in a normal LD form */
/*  		    switch (proc_opc) */
/*  		    { */
/*  		    case TAHOEop_LD1_POST_A: */
/*  		      oper->proc_opc = TAHOEop_LD1_A; */
/*  		      break; */
/*  		    case TAHOEop_LD1_POST_S: */
/*  		      oper->proc_opc = TAHOEop_LD1_S; */
/*  		      break; */
/*  		    case TAHOEop_LD1_POST_SA: */
/*  		      oper->proc_opc = TAHOEop_LD1_SA; */
/*  		      break; */
/*  		    case TAHOEop_LD2_POST_A: */
/*  		      oper->proc_opc = TAHOEop_LD2_A; */
/*  		      break; */
/*  		    case TAHOEop_LD2_POST_S: */
/*  		      oper->proc_opc = TAHOEop_LD2_S; */
/*  		      break; */
/*  		    case TAHOEop_LD2_POST_SA: */
/*  		      oper->proc_opc = TAHOEop_LD2_SA; */
/*  		      break; */
/*  		    case TAHOEop_LD4_POST_A: */
/*  		      oper->proc_opc = TAHOEop_LD4_A; */
/*  		      break; */
/*  		    case TAHOEop_LD4_POST_S: */
/*  		      oper->proc_opc = TAHOEop_LD4_S; */
/*  		      break; */
/*  		    case TAHOEop_LD4_POST_SA: */
/*  		      oper->proc_opc = TAHOEop_LD4_SA; */
/*  		      break; */
/*  		    case TAHOEop_LD8_POST_A: */
/*  		      oper->proc_opc = TAHOEop_LD8_A; */
/*  		      break; */
/*  		    case TAHOEop_LD8_POST_S: */
/*  		      oper->proc_opc = TAHOEop_LD8_S; */
/*  		      break; */
/*  		    case TAHOEop_LD8_POST_SA: */
/*  		      oper->proc_opc = TAHOEop_LD8_SA; */
/*  		      break; */
/*                      case TAHOEop_LDF_S_POST_S: */
/*                        oper->proc_opc = TAHOEop_LDF_S_S; */
/*                        break; */
/*                      case TAHOEop_LDF_S_POST_A: */
/*                        oper->proc_opc = TAHOEop_LDF_S_A; */
/*                        break; */
/*                      case TAHOEop_LDF_S_POST_SA: */
/*                        oper->proc_opc = TAHOEop_LDF_S_SA; */
/*                        break; */
/*                      case TAHOEop_LDF_D_POST_S: */
/*                        oper->proc_opc = TAHOEop_LDF_D_S; */
/*                        break; */
/*                      case TAHOEop_LDF_D_POST_A: */
/*                        oper->proc_opc = TAHOEop_LDF_D_A; */
/*                        break; */
/*                      case TAHOEop_LDF_D_POST_SA: */
/*                        oper->proc_opc = TAHOEop_LDF_D_SA; */
/*                        break; */

/*  		    default: */
/*  		      L_punt ("Op %d a masked post-inc load that " */
/*  			      "does not have a speculative proc_opc\n", */
/*  			      oper->id); */
/*  		    } */

		  /* Need to split ld_post and ld and inc */
		  add_op = L_create_new_op (Lop_ADD);
		  add_op->proc_opc = TAHOEop_ADDS;
		  add_op->dest[0] = L_copy_operand (oper->dest[1]);
		  add_op->src[1] = L_copy_operand (oper->src[0]);
		  add_op->src[0] = L_copy_operand (oper->src[1]);

		  L_delete_operand (oper->dest[1]);
		  L_delete_operand (oper->src[1]);
		  oper->dest[1] = NULL;
		  oper->src[1] = NULL;

		  /* Find the next bundle */
		  num_following = 0;
		  for (define_op = oper->next_op;
		       define_op != NULL; define_op = define_op->next_op)
		    {
		      if (define_op->proc_opc != TAHOEop_NON_INSTR)
			num_following++;
		      if ((define_op->opc == Lop_DEFINE) &&
			  (L_is_macro (define_op->dest[0])) &&
			  (define_op->dest[0]->value.mac ==
			   TAHOE_MAC_TEMPLATE))
			break;
		    }		/* for define_op */

		  if ((num_following > 2) || (num_following < 1))
		    L_punt ("P_force_recovery_execution: %d instructions"
			    " following memory op in same bundle.",
			    num_following);

		  if (num_following == 1)
		    {
		      new_define_op = L_create_new_op (Lop_DEFINE);
		      new_define_op->proc_opc = TAHOEop_NON_INSTR;
		      new_define_op->dest[0] =
			L_new_macro_operand (TAHOE_MAC_TEMPLATE,
					     L_CTYPE_LLONG, L_PTYPE_NULL);
		      new_define_op->src[0] =
			L_new_int_operand (0, L_CTYPE_LLONG);
		      /* Put in a stop bit to be safe */
		      new_define_op->src[1] =
			L_new_int_operand (1, L_CTYPE_LLONG);

		      no_op = L_create_new_op (Lop_NO_OP);
		      no_op->proc_opc = TAHOEop_NOP_I;
		      no_op->src[0] = L_new_int_operand (0, L_CTYPE_LLONG);

		      L_delete_operand (oper->src[0]);
		      oper->src[0] =
			L_new_macro_operand (TAHOE_MAC_ZERO,
					     L_CTYPE_LLONG, L_PTYPE_NULL);

		      if (define_op == NULL)
			{
			  L_insert_oper_after (cb, cb->last_op,
					       new_define_op);
			  L_insert_oper_after (cb, cb->last_op,
					       L_copy_operation (oper));
			  L_insert_oper_after (cb, cb->last_op, add_op);
			  L_insert_oper_after (cb, cb->last_op, no_op);
			}	/* if */
		      else
			{
			  L_insert_oper_before (cb, define_op, new_define_op);
			  L_insert_oper_before (cb, define_op,
						L_copy_operation (oper));
			  L_insert_oper_before (cb, define_op, add_op);
			  L_insert_oper_before (cb, define_op, no_op);
			}	/* else */

		      L_change_opcode (oper, Lop_NO_OP);
		      oper->proc_opc = TAHOEop_NOP_M;
		      L_delete_operand (oper->src[0]);
		      oper->src[0] = L_new_int_operand (0, L_CTYPE_LLONG);
		      L_delete_operand (oper->dest[0]);
		      oper->dest[0] = NULL;

		      /* Find the prev bundle */
		      for (prev_define_op = oper;
			   prev_define_op != NULL;
			   prev_define_op = prev_define_op->prev_op)
			{
			  if ((prev_define_op->opc == Lop_DEFINE) &&
			      (L_is_macro (prev_define_op->dest[0])) &&
			      (prev_define_op->dest[0]->value.mac ==
			       TAHOE_MAC_TEMPLATE))
			    break;
			}	/* for prev_define_op */

		      if (prev_define_op == NULL)
			L_punt ("P_force_recovery_execution:  "
				"No template found.");

		      new_define_op = L_create_new_op (Lop_DEFINE);
		      new_define_op->proc_opc = TAHOEop_NON_INSTR;
		      new_define_op->dest[0] =
			L_new_macro_operand (TAHOE_MAC_TEMPLATE,
					     L_CTYPE_LLONG, L_PTYPE_NULL);

		      no_op = L_create_new_op (Lop_NO_OP);
		      no_op->src[0] = L_new_int_operand (0, L_CTYPE_LLONG);

		      switch ((ITint8) (prev_define_op->src[0]->value.i))
			{
			case MLI:
			  no_op->proc_opc = TAHOEop_NOP_I;
			  new_define_op->src[0] =
			    L_new_int_operand (MLI, L_CTYPE_LLONG);
			  break;

			case MII:
			case MISI:
			case MMI:
			case MSMI:
			case MFI:
			  no_op->proc_opc = TAHOEop_NOP_I;
			  new_define_op->src[0] =
			    L_new_int_operand (MMI, L_CTYPE_LLONG);
			  break;

			case MMF:
			  no_op->proc_opc = TAHOEop_NOP_F;
			  new_define_op->src[0] =
			    L_new_int_operand (MMF, L_CTYPE_LLONG);
			  break;

			case MIB:
			case MBB:
			case MMB:
			case MFB:
			  no_op->proc_opc = TAHOEop_NOP_B;
			  new_define_op->src[0] =
			    L_new_int_operand (MMB, L_CTYPE_LLONG);
			  break;

			default:
			  L_punt ("P_force_recovery_execution: "
				  "Unknown bundle type %d with a "
				  "memory operation.",
				  prev_define_op->src[0]);
			}	/* switch */

		      /* Put in a stop bit to be safe */
		      if (new_define_op->src[0]->value.i != MLI)
			new_define_op->src[1] =
			  L_new_int_operand (1, L_CTYPE_LLONG);
		      else
			new_define_op->src[1] =
			  L_new_int_operand (2, L_CTYPE_LLONG);

		      if (define_op == NULL)
			L_insert_oper_after (cb, cb->last_op, new_define_op);
		      else
			L_insert_oper_before (cb, define_op, new_define_op);

		      following_op = oper->next_op;
		      if (following_op == NULL)
			L_punt ("P_force_recovery_execution: "
				"Can't find following op.");

		      L_insert_oper_after (cb, new_define_op,
					   L_copy_operation (following_op));
		      L_insert_oper_after (cb, following_op, no_op);
		      L_delete_oper (cb, following_op);

		      no_op = L_create_new_op (Lop_NO_OP);
		      no_op->proc_opc = TAHOEop_NOP_M;
		      no_op->src[0] = L_new_int_operand (0, L_CTYPE_LLONG);

		      L_insert_oper_after (cb, new_define_op, no_op);
		      if (new_define_op->src[0]->value.i != MLI)
			L_insert_oper_after (cb, new_define_op,
					     L_copy_operation (no_op));
		    }		/* if */
		  else
		    {
		      /* Find the prev bundle */
		      for (prev_define_op = oper; prev_define_op != NULL;
			   prev_define_op = prev_define_op->prev_op)
			{
			  if ((prev_define_op->opc == Lop_DEFINE) &&
			      (L_is_macro (prev_define_op->dest[0])) &&
			      (prev_define_op->dest[0]->value.mac ==
			       TAHOE_MAC_TEMPLATE))
			    break;
			}	/* for prev_define_op */
		      if (prev_define_op == NULL)
			L_punt ("P_force_recovery_execution:  "
				"No template found.");

		      new_define_op = L_create_new_op (Lop_DEFINE);
		      new_define_op->proc_opc = TAHOEop_NON_INSTR;
		      new_define_op->dest[0] = Ltahoe_IMAC (TEMPLATE);
		      new_define_op->src[0] =
			L_new_int_operand (0, L_CTYPE_LLONG);
		      /* Put in a stop bit to be safe */
		      new_define_op->src[1] =
			L_new_int_operand (1, L_CTYPE_LLONG);

		      no_op = L_create_new_op (Lop_NO_OP);
		      no_op->proc_opc = TAHOEop_NOP_I;
		      no_op->src[0] = L_new_int_operand (0, L_CTYPE_LLONG);

#if 1
		      L_delete_operand (oper->src[0]);
		      oper->src[0] = Ltahoe_IMAC (ZERO);
#endif
		      L_insert_oper_before (cb, prev_define_op,
					    new_define_op);
		      L_insert_oper_before (cb, prev_define_op,
					    L_copy_operation (oper));
		      L_insert_oper_before (cb, prev_define_op, add_op);
		      L_insert_oper_before (cb, prev_define_op, no_op);

		      L_change_opcode (oper, Lop_NO_OP);
		      oper->proc_opc = TAHOEop_NOP_M;
		      L_delete_operand (oper->src[0]);
		      oper->src[0] = L_new_int_operand (0, L_CTYPE_LLONG);
		      L_delete_operand (oper->dest[0]);
		      oper->dest[0] = NULL;
		    }		/* else */
		}		/* if */
	      else
		{
#if 1
		  L_delete_operand (oper->src[0]);
		  oper->src[0] = Ltahoe_IMAC (ZERO);
#endif
		}		/* else */
	    }			/* if */
	}			/* for oper */
    }				/* for cb */
  fprintf (stderr, "Forced lds %d - %d of %d\n",
	   Ltahoe_force_recovery_lower,
	   Ltahoe_force_recovery_upper, lds_count);
}				/* P_force_recovery_execution */


/****************************************************************************
 *
 * routine: P_process_func()
 * purpose: Uses TAHOE specific macro information inserted during phase I
 *          and phase II annotation to produce function header.
 *          Will produce the assembly code for a function.
 * input:
 * output:
 * returns:
 * modified: 9/27/02 REK Updating to use new opcode map and completer scheme.
 * note:
 *-------------------------------------------------------------------------*/

void
P_process_func (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *oper;
  char *function_name;
  unsigned int instr_offset = 0;	/* The number of instr from the start of
					 * the function to the current op */
  unsigned int instr_issue = 0;	/* number of cycles in this BB */

  int implicit_fn = 0;

  LTD ("FUNCTION %s START PHASE 3", fn->name);

  function_name = fn->name + 1;	/* Suppress leading underscore */

  if (Ltahoe_tag_loads)
    {
      fprintf (LD_TABLE_OUT, "%s\n", function_name); 
    }

  P_symtab_add_def (function_name, 1);

  if (Ltahoe_insert_branch_hints)
    {
      LTD ("Inserting branch hints (BRP)");
      O_insert_branch_hints (fn);
    }				/* if */
  else
    {
      LTD ("Inserting branch modifiers (BRP)");
      O_insert_br_instr_hints_only (fn);
    }				/* else */

  if (Ltahoe_force_recovery_execution)
    {
      LTD ("Forcing recovery code execution (RC)");
      P_force_recovery_execution (fn);
    }				/* if */

  if (L_jump_tables_have_changes (fn))
    L_regenerate_all_jump_tables (fn);

  LTD ("Generating assembly output");

  L_get_reg_stack_info (fn, &num_reg_stack_inputs, &num_reg_stack_locals,
			&num_reg_stack_outputs, &num_reg_stack_rots);

  /* 12/02/02 REK Write the global line in a form for ias or as. */
  if (Ltahoe_output_for_ias)
    {
      fprintf (L_OUT, "\t.section .text\n\t.proc  %s#\n\t.align 32\n%s::\n",
	       function_name, function_name);
    }
  else
    {
      fprintf (L_OUT, "\t.section .text\n\t.proc  %s#\n", function_name);
      fprintf (L_OUT, "\t.align 32\n\t.global %s#\n%s:\n", function_name, 
	       function_name);
    }

  /*
   * Print map of "nice" register names
   * ----------------------------------------------------------------------
   */
  if (L_find_attr (fn->attr, "ALLOCA"))
    {
      /* define register psp */
      fprintf (L_OUT, "\tpspr = r%d\t\t// alloca function -- define pspr\n",
	       INT_STACK_REG_BASE + num_reg_stack_inputs +
	       num_reg_stack_locals - 3);
    }				/* if */

  fprintf (L_OUT, "\tprsav = r%d\t\t// pred save reg\n",
	   INT_STACK_REG_BASE + num_reg_stack_inputs +
	   num_reg_stack_locals - 2);

  fprintf (L_OUT, "\tgpsav = r%d\t\t// gp save reg\n",
	   INT_STACK_REG_BASE + num_reg_stack_inputs +
	   num_reg_stack_locals - 1);

  PG_setup_pred_graph (fn);

  if (Ltahoe_print_live_registers)
    L_do_flow_analysis (fn, LIVE_VARIABLE);

  if (Ltahoe_generate_unwind_directives)
    L_get_unwind_info (fn);

  if (L_find_attr (fn->attr, "TAHOE_IMPLICIT_OUTPUT"))
    {
      implicit_fn = 1;
      P_set_implicit_bundling ();
    }				/* if */
  else
    {
      implicit_fn = 0;
      P_set_explicit_bundling ();
    }				/* else */

  /* Print instructions */

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      if (!implicit_fn)
	{
	  if (L_find_attr (cb->attr, "TAHOE_IMPLICIT_OUTPUT"))
	    P_set_implicit_bundling ();
	  else
	    P_set_explicit_bundling ();
	}			/* if */

      if (Ltahoe_print_characteristics)
	P_print_cb_info (fn, cb);

      /* Print the basic block label made from the func name and cb number */
#ifdef LOCAL_LABELS
      fprintf (L_OUT, ".%s_%d:\n", function_name, cb->id);
#else
      fprintf (L_OUT, "%s?%d:\n", function_name, cb->id);
#endif

      P_reset_bundle_indx ();

      instr_issue = 0;
      for (oper = cb->first_op; oper; oper = oper->next_op)
	{
	  if (oper == unwind.first_prologue_inst)
	    fprintf (L_OUT, "\t.prologue\n");

	  P_print_oper (cb, oper, &instr_offset, &instr_issue);

	  if (oper == unwind.last_prologue_inst)
	    fprintf (L_OUT, "\t.body\n");
	}			/* for oper */
    }				/* for cb */

  fprintf (L_OUT, "\t.endp  %s\n", function_name);

  {
    L_Attr *attr;
    if ((attr = L_find_attr (fn->attr, "Cattr")))
      {
	int i;
	L_Operand *opd;
	for (i = 0; i < attr->max_field; i++)
	  {
	    if (!(opd = attr->field[i]) || !L_is_string (opd))
	      continue;

	    if (!strcmp (opd->value.s, "\"__constructor__\""))
	      {
		/* Constructors need entry in ctors */
		fprintf (L_OUT, "\t.section\t.ctors,\"aw\",@progbits\n"
			 "\t.align\t8\n"
			 "\tdata8\t @fptr(%s#)\n", function_name);
	      }
	  }
      }
  }

  if (fn->jump_tbls)
    {
      L_Datalist_Element *htble;
      char *name = NULL;

      fprintf (L_OUT, "\t// Jump tables for %s\n", function_name);
      P_print_section_title (L_MS_RODATA);
      for (htble = fn->jump_tbls->first_element; htble; 
	   htble = htble->next_element)
	{
	  L_Data *htbl = htble->data;
	  L_Expr *label;
	  char *lstr, cbstr[16];
	  int i, j;

	  switch (htbl->type)
	    {
	    case L_INPUT_ALIGN:
	      name = htbl->address->value.l;
	      fprintf (L_OUT, "\t.align 8\n%s:\n", name + 1);
	      break;
	    case L_INPUT_RESERVE:
	    case L_INPUT_MS:
	      break;
	    case L_INPUT_WQ:
	      if (!name)
		L_punt ("P_process_func: jump tbl failure");

	      label = htbl->value;

	      if (label->type != L_EXPR_LABEL)
		L_punt ("P_process_func: jump tbl failure");

	      lstr = label->value.l;

	      for (i = 0, j = 0; lstr[i] != '_' && lstr[i] != '\0'; i++)
		{
		  if (!isdigit(lstr[i]))
		    continue;

		  cbstr[j++] = lstr[i];
		}
		
	      cbstr[j] = '\0';

	      fprintf (L_OUT, "\tdata8.ua\t.%s_%s#\n",
		       fn->name + 1, cbstr);
	      break;
	    default:
	      L_punt ("P_process_func: unexpected token %d in jump tbl",
		      htbl->type);
	      break;
	    }
	}

      fprintf (L_OUT, "\t.skip\t\t1\n");
    }

#ifdef DEBUG
  fprintf (stderr, "Exiting P_process_func\n");
#endif

  LTD ("FUNCTION %s END PHASE 3", fn->name);
  return;
}				/* P_process_func */
