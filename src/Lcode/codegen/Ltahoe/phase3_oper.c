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
/*===========================================================================*\
 *
 * File: phase3_oper.c 
 * Purpose: print out instructions
 *        
 *===========================================================================*/
/* 09/12/02 REK Updating file to use the new opcode map and completers
 *              scheme. */
/* 01/13/03 REK Changing references to psp to pspr.  GNU as considers psp a
 *              reserved name and does not like using it as an operand. */

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "ltahoe_main.h"
#include "ltahoe_table.h"
#include "ltahoe_completers.h"
#include "ltahoe_op_query.h"
#include "phase3.h"
#include "phase1_func.h"
#include "phase2_func.h"
#include "phase2_reg.h"
#include "phase3_unwind.h"

/************************************************************************
 ** LINE FORMATTING                                                    **
 ************************************************************************/

#define P_COMMENT_COL 52
#define P_TAB_SIZE     8

static int P_line_length = 0;

/*
 * void P_line_print()
 * ----------------------------------------------------------------------
 * Print out a line to the assembler, counting characters for alignment.
 */
/* 12/15/04 REK Trying to avoid calling malloc in the common case.
 *              We use a 200 byte buffer on the stack.  If our string
 *              fits in that buffer, great.  If we need more space, we
 *              call malloc to set up big_str and use that. */

void
P_line_print (char *format, ...)
{
  #define DEF_STR_LEN 200

  va_list ptr;
  char *str = NULL, def_str[DEF_STR_LEN], *big_str = NULL;
  int str_len;
  int diff;
  char *ch;

  va_start (ptr, format);
  str_len = vsnprintf (def_str, DEF_STR_LEN, format, ptr);

  /* If we need more space, allocate big_str and call vsnprintf() again. */
  if (str_len >= DEF_STR_LEN)
    {
      /* Add a byte for the trailing null. */
      str_len += 1;
      big_str = malloc (sizeof (char) * str_len);
      str_len = vsnprintf (big_str, str_len, format, ptr);
      str = big_str;
    }
  else
    {
      str = def_str;
    }

  for (ch = str; *ch; ch++)
    {
      if (*ch == '\t')
	{
	  P_line_length += P_TAB_SIZE;
	  diff = P_line_length % P_TAB_SIZE;
	  P_line_length -= diff;
	}			/* if */
      else if (*ch == '\n')
	{
	  P_line_length = 0;
	}			/* else if */
      else if (!iscntrl (*ch))
	{
	  P_line_length++;
	}			/* else if */
    }				/* for ch */

  fprintf (L_OUT, "%s", str);
  va_end (ptr);

  if (big_str)
    free (big_str);

  return;
}				/* P_line_print */


/****************************************************************************
 *
 * routine: P_line_pad
 * purpose: Pad the ouput until the specified column
 * input:
 * output:
 * returns:
 * modified:
 * note:
 *-------------------------------------------------------------------------*/

void
P_line_pad (int column)
{
  char str[100];
  int diff;

  diff = column - P_line_length;
  P_line_length = 0;

  if (diff > 0)
    {
      memset (str, ' ', diff);
      str[diff] = '\0';
      fprintf (L_OUT, "%s", str);
    }				/* if */
}				/* P_line_pad */

/************************************************************************
 ** BUNDLING                                                           **
 ************************************************************************/

static struct
{
  int explicit_mode;
  int bundle_active;
  int syllable_mask;
  int stop_bit_mask;
  int new_group;
  int instrs_remaining;
  int bundle_indx;
}
P_bundle_status =
{
1, 0, 0, 0, 0};

/*
 * void P_set_explicit_bundling(void)
 * ----------------------------------------------------------------------
 * Turn on explicit bundling mode.
 */

void
P_set_explicit_bundling (void)
{
  if (P_bundle_status.bundle_active)
    L_punt ("P_set_explicit_bundling: a bundle is open");
  P_bundle_status.explicit_mode = 1;
  P_bundle_status.new_group = 1;
  return;
}				/* P_set_explicit_bundling */

/*
 * void P_set_explicit_bundling(void)
 * ----------------------------------------------------------------------
 * Turn off explicit bundling mode.  If a bundle is open, punt!
 */

void
P_set_implicit_bundling (void)
{
  if (P_bundle_status.bundle_active)
    L_punt ("P_set_implicit_bundling: a bundle is open");
  P_bundle_status.explicit_mode = 0;
  return;
}				/* P_set_implicit_bundling */

void
P_reset_bundle_indx (void)
{
  P_bundle_status.bundle_indx = 0;
  return;
}				/* P_reset_bundle_indx */

/*
 * void P_open_bundle(L_Oper *oper)
 * ----------------------------------------------------------------------
 * Called on a bundle-defining define operation to start a new bundle.
 */

void
P_open_bundle (L_Oper * oper)
{
  int template_type;

  /*  src[0] is template type */
  /*  src[1] is mask of stop bits (0 means no stopbit) */
  /*  src[2] is the non-nop op density */

  if (P_bundle_status.bundle_active)
    L_punt ("P_open_bundle: Failed to open inside existing bundle (op %d)",
	    oper->id);

  if (!P_bundle_status.explicit_mode)
    return;

  template_type = LT_get_template (oper);

  P_line_print (" { .%s // %d\n",
		LT_template_name (template_type),
		P_bundle_status.bundle_indx++);

  P_bundle_status.instrs_remaining = (template_type != MLI) ? 3 : 2;
  P_bundle_status.bundle_active = 1;
  P_bundle_status.syllable_mask = S_AFTER_1ST;
  P_bundle_status.stop_bit_mask = LT_get_stop_bit_mask (oper);
  return;
}				/* P_open_bundle */

/*
 * void P_bundle_instr(void)
 * ----------------------------------------------------------------------
 * Called after printing an op to print a stop bit or bundle closure
 * as required.
 */

void
P_bundle_instr (void)
{
  if (!P_bundle_status.explicit_mode)
    return;

  if (!P_bundle_status.bundle_active)
    return;

  if (P_bundle_status.syllable_mask & P_bundle_status.stop_bit_mask)
    {
      P_line_print (" ;;");
      P_bundle_status.new_group = 1;
    }				/* if */
  else
    {
      P_bundle_status.new_group = 0;
    }				/* else */

  if (--P_bundle_status.instrs_remaining <= 0)
    {
      P_bundle_status.bundle_active = 0;
      P_line_print (" }");
    }				/* if */

  P_bundle_status.syllable_mask >>= 1;

  return;
}				/* P_bundle_instr */

/************************************************************************
 ** COMMENTS                                                           **
 ************************************************************************/

/*
 * void P_print_comment(L_Oper * oper,
 *		        unsigned int instr_offset, unsigned int issue_cycle)
 * ----------------------------------------------------------------------
 * Prints the comment following an op.
 */

void
P_print_comment (L_Oper * oper, unsigned int instr_offset,
		 unsigned int issue_cycle)
{
  L_Attr *attr;
  int promoted;
  int homeblock;
  char flagstr[64];

  if (Ltahoe_print_issue_time ||
      Ltahoe_print_offset || Ltahoe_print_latency || Ltahoe_print_op_id)
    {
      P_line_pad (P_COMMENT_COL);
      fprintf (L_OUT, "//");
      if (Ltahoe_print_op_id)
	{
	  fprintf (L_OUT, " op %d", oper->id);
	}			/* if */
      if (Ltahoe_print_issue_time)
	{
	  fprintf (L_OUT, " cy %d", issue_cycle);
	}			/* if */

      if ((attr = L_find_attr (oper->attr, "slack")))
	fprintf (L_OUT, " slk %d", L_get_int_attr_field (attr, 0));

      if ((attr = L_find_attr (oper->attr, "ADDLAT")))
	{
	  int i;
	  fprintf (L_OUT, " +LAT[");
	  for (i = 0; i <= attr->max_field; i++)
	    {
	      if (attr->field[i])
		{
#if LP64_ARCHITECTURE
		  fprintf (L_OUT, "%ld", (long)attr->field[i]->value.i);
#else
		  fprintf (L_OUT, "%d", attr->field[i]->value.i);
#endif
		}
	      if (i < attr->max_field)
		fprintf (L_OUT, ",");
	    }
	  fprintf (L_OUT, "]");
	}
      if (Ltahoe_print_latency)
	{
	  if ((attr = L_find_attr (oper->attr, "isl")))
	    fprintf (L_OUT, ":%d ", L_get_int_attr_field (attr, 2));
	  else
	    fprintf (L_OUT, ":  ");

	  if ((attr = L_find_attr (oper->attr, "elp")))
	    {
	      fprintf (L_OUT, " elp " ITintmaxformat ":"
		       ITintmaxformat "-"
		       ITintmaxformat ":%0.3f",
		       attr->field[0]->value.i,
		       attr->field[1]->value.i,
		       attr->field[2]->value.i, attr->field[3]->value.f2);
	    }			/* if */
	}			/* if */
      else
	{
	  if (Ltahoe_print_issue_time)
	    putc (' ', L_OUT);
	}			/* else */

      if (Ltahoe_print_offset)
	{
	  fprintf (L_OUT, "[%2d]", instr_offset);
	}			/* if */
      if (Ltahoe_print_iteration)
	{
	  if ((attr = L_find_attr (oper->attr, "iter")) != NULL)
	    fprintf (L_OUT, "i%d", L_get_int_attr_field (attr, 0));
	}			/* if */

      if ((Ltahoe_tag_loads)&&(L_load_opcode(oper))&&
	  (Ltahoe_print_cache_stats))
	{
	  L_Attr *load_attr;
	  int l1misses, l2misses, l3misses;
	 
	  if ((load_attr = L_find_attr (oper->attr, "cmiss")))
	    {
	      l1misses = load_attr->field[0]->value.i;
	      l2misses = load_attr->field[1]->value.i;
	      l3misses = load_attr->field[2]->value.i;
	      fprintf (L_OUT, " cache misses(%d)(%d)(%d)", 
		       l1misses, l2misses, l3misses);
	    }
	}
      
      if (Ltahoe_print_issue_time)
	{
	  L_Attr *stk_attr;
	  int ofst;

	  L_oper_flags_to_string (flagstr, oper->flags);

	  if (flagstr[0] != '\0')
	    fprintf (L_OUT, " <%s>", flagstr);

	  if ((stk_attr = L_find_attr (oper->attr, "stack")))
	    {
	      ofst = L_get_int_attr_field (stk_attr, 1);
	      fprintf (L_OUT, "K(%d)", ofst);
	    }			/* if */

	  promoted = L_EXTRACT_BIT_VAL (oper->flags, L_OPER_PROMOTED) != 0;

	  if (L_EXTRACT_BIT_VAL (oper->flags, L_OPER_SPECULATIVE))
	    {
	      if ((attr = L_find_attr (oper->attr, "hb")))
		{
		  homeblock = L_get_int_attr_field (attr, 0);
		  if (!promoted || homeblock)
		    fprintf (L_OUT, "S%d ", homeblock);
		}		/* if */
	    }			/* if */
	  if ((attr = L_find_attr (oper->attr, "pibs")))
	    {
	      homeblock = L_get_int_attr_field (attr, 0);
	      fprintf (L_OUT, "I%d", homeblock);
	    }			/* if */
	  putc (' ', L_OUT);
	}			/* if */

      if ((L_load_opcode (oper) || L_store_opcode (oper)) &&
	  (attr = L_find_attr (oper->attr, "label")))
	{
	  fprintf (L_OUT, "addr [%s#", attr->field[0]->value.s);

	  if (attr->max_field > 1)
	    {
	      if (L_is_int_constant (attr->field[1]))
		fprintf (L_OUT, "+" ITintmaxformat, attr->field[1]->value.i);
	      else
		fprintf (L_OUT, "+?");
	    }			/* if */

	  fprintf (L_OUT, "]");
	}			/* if */

      if (LT_is_cond_br (oper)
	  && (attr = L_find_attr (oper->attr, "br_info")))
	fprintf (L_OUT, "<%d%% tk %d wt> ", L_get_int_attr_field (attr, 0),
		 L_get_int_attr_field (attr, 1));
    }				/* if */
}				/* P_print_comment */

/************************************************************************
 ** OPERANDS                                                           **
 ************************************************************************/

void
P_print_int_reg_name (int real_reg_id)
{
  if (real_reg_id < INT_STACK_REG_BASE)
    P_line_print ("r%d", real_reg_id);
  else if (real_reg_id < INT_STACK_REG_BASE + num_reg_stack_inputs)
    P_line_print ("in%d", real_reg_id - INT_STACK_REG_BASE);
  else if (real_reg_id < INT_STACK_REG_BASE + num_reg_stack_inputs +
	   num_reg_stack_locals)
    P_line_print ("loc%d", real_reg_id - INT_STACK_REG_BASE -
		  num_reg_stack_inputs);
  else if (real_reg_id < INT_STACK_REG_BASE + num_reg_stack_inputs +
	   num_reg_stack_locals + num_reg_stack_outputs)
    P_line_print ("out%d", real_reg_id - INT_STACK_REG_BASE -
		  num_reg_stack_inputs - num_reg_stack_locals);
  else
    L_punt ("P_print_int_reg_name: Failed to map register %d", real_reg_id);
  return;
}				/* P_print_int_reg_name */

/****************************************************************************
 *
 * routine: P_print_predicate()
 * purpose: Print out (pX) before an instruction
 * input: oper - current operation with a predicate
 * output:
 * returns:
 * modified:
 * note:
 *-------------------------------------------------------------------------*/

void
P_print_predicate (L_Oper * oper)
{
  if (oper->pred[0] == NULL)
    return;

  if (!L_is_ctype_predicate (oper->pred[0]))
    L_punt ("P_print_predicate: not a predicate ");

  P_line_print ("  (p");
  P_line_print ("%d", oper->pred[0]->value.r - PRED_REG_BASE);
  P_line_print (")");
}				/* P_print_predicate */


/****************************************************************************
 *
 * routine: P_print_register_operand_asm()
 * purpose: Print out a register to the .s file
 * input:
 * output:
 * returns:
 * modified:
 * note:
 *-------------------------------------------------------------------------*/

void
P_print_register_operand_asm (L_Operand * operand, int real)
{
  int real_reg_id;

  if (L_is_ctype_integer (operand))
    {
      real_reg_id = operand->value.r;
      if (real_reg_id > MAX_INT_REGISTER_ID)
	L_punt ("ERROR: Integer register file overflow\n");

      if (real)
	P_line_print ("r%d", real_reg_id);
      else
	P_print_int_reg_name (real_reg_id);
      return;
    }				/* if */

  switch (operand->ctype)
    {
    case L_CTYPE_FLOAT:
    case L_CTYPE_DOUBLE:
      P_line_print ("f%d", operand->value.r - FLOAT_REG_BASE);
      return;

    case L_CTYPE_PREDICATE:
      P_line_print ("p%d", operand->value.r - PRED_REG_BASE);
      return;

    case L_CTYPE_BTR:
      P_line_print ("b%d", operand->value.r - BRANCH_REG_BASE);
      return;

    default:
      L_punt ("P_print_register: Unknown register type\n");
    }				/* switch */
}				/* P_print_register_operand_asm */



/****************************************************************************
 *
 * routine: P_print_macro_operand_asm()
 * purpose: Print out a register to the .s file
 * input:
 * output:
 * returns:
 * modified:
 * note:
 *-------------------------------------------------------------------------*/

void
P_print_macro_operand_asm (L_Oper * oper, L_Operand * operand, int real)
{
  int mac, real_reg_id;

  switch (operand->value.mac)
    {
    case L_MAC_SP:
    case L_MAC_SAFE_MEM:
    case L_MAC_LV:
    case L_MAC_OP:
    case L_MAC_IP:
      if (real)
	P_line_print ("r12");
      else
	P_line_print ("sp");	/* memory stack pointer */
      break;
    case TAHOE_MAC_GP:
      P_line_print ("gp");
      break;
    case TAHOE_MAC_AP:
      P_line_print ("r9");
      break;
    case TAHOE_MAC_ZERO:
      P_line_print ("r0");
      break;
    case TAHOE_MAC_TMPREG1:
      P_line_print ("r2");
      break;
    case TAHOE_MAC_TMPREG2:
      P_line_print ("r3");
      break;
    case TAHOE_MAC_FZERO:
      P_line_print ("f0");
      break;
    case TAHOE_MAC_FONE:
      P_line_print ("f1");
      break;
    case TAHOE_MAC_RETADDR:
      P_line_print ("b0");
      break;
    case TAHOE_MAC_PRED_TRUE:
      P_line_print ("p0");
      break;
    case TAHOE_MAC_AR_PFS:
      P_line_print ("ar.pfs");
      break;
    case TAHOE_MAC_UNAT:
      P_line_print ("ar.unat");
      break;
    case TAHOE_MAC_LC:
      P_line_print ("ar.lc");
      break;
    case TAHOE_MAC_EC:
      P_line_print ("ar.ec");
      break;
    case TAHOE_PRED_SAVE_REG:
      real_reg_id =
	INT_STACK_REG_BASE + num_reg_stack_inputs + num_reg_stack_locals - 2;
      if (real)
	P_line_print ("r%d", real_reg_id);
      else
	P_line_print ("prsav");
      break;
    case TAHOE_PRED_BLK_REG:
      P_line_print ("pr");
      break;
    case TAHOE_GP_SAVE_REG:
      real_reg_id =
	INT_STACK_REG_BASE + num_reg_stack_inputs + num_reg_stack_locals - 1;
      if (real)
	P_line_print ("r%d", real_reg_id);
      else
	P_line_print ("gpsav");
      break;
    case L_MAC_P0:
    case L_MAC_P1:
    case L_MAC_P2:
    case L_MAC_P3:
    case L_MAC_P4:
    case L_MAC_P5:
    case L_MAC_P6:
    case L_MAC_P7:
      /* These are input parameters */
      mac = operand->value.mac - L_MAC_P0;
      if ((mac + 1) > num_reg_stack_inputs)
	{
	  fprintf (stderr, "WARNING: Input register index past number "
		   "of reg_stack inputs oper: %d   %d %d\n",
		   oper->id, mac, num_reg_stack_inputs);
	  fprintf (stderr, "\tOK if varargs function\n");
	}			/* if */
      real_reg_id = INT_STACK_REG_BASE + mac;
      if (real)
	P_line_print ("r%d", real_reg_id);
      else
	P_line_print ("in%d", mac);
      break;

    case L_MAC_P8:
    case L_MAC_P9:
    case L_MAC_P10:
    case L_MAC_P11:
    case L_MAC_P12:
    case L_MAC_P13:
    case L_MAC_P14:
    case L_MAC_P15:
      /* These are integer output parameters */
      mac = operand->value.mac - L_MAC_P8;
      if ((mac + 1) > num_reg_stack_outputs)
	fprintf (stderr, "WARNING: Output register index past number "
		 "of reg_stack outputs oper: %d  %d %d\n",
		 oper->id, mac, num_reg_stack_outputs);
      real_reg_id = INT_STACK_REG_BASE + num_reg_stack_inputs
	+ num_reg_stack_locals + mac;
      if (real)
	P_line_print ("r%d", real_reg_id);
      else
	P_line_print ("out%d", mac);
      break;

    case L_MAC_P16:
    case L_MAC_P17:
    case L_MAC_P18:
    case L_MAC_P19:
      /* This is a return value, and doesn't go through register stack */
      mac = operand->value.mac - L_MAC_P16;
      real_reg_id = INT_RETURN_VALUE_REG + mac;
      P_line_print ("r%d", real_reg_id);
      break;

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

      mac = operand->value.mac - L_MAC_P20;
      real_reg_id = FLT_INPUT_PARMS_START + mac;

      P_line_print ("f%d", real_reg_id);
      break;

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
      mac = operand->value.mac - L_MAC_P28;
      real_reg_id = FLT_RETURN_VALUE_REG + mac;
      P_line_print ("f%d", real_reg_id);
      break;

    case TAHOE_MAC_PSP:
      real_reg_id = INT_STACK_REG_BASE + num_reg_stack_inputs +
	num_reg_stack_locals - 3;
      if (real)
	P_line_print ("r%d", real_reg_id);
      else
	P_line_print ("pspr", real_reg_id);
      break;

    default:
      L_print_operand (stderr, operand, 1);
      L_punt ("P_print_operand_asm: bogus macro??  oper: %d", oper->id);
    }				/* switch */
}				/* P_print_macro_operand_asm */


/****************************************************************************
 *
 * routine: P_print_immed_operand_asm
 * purpose: print immediate operand
 * input:
 * output:
 * returns:
 * modified:
 * note: the no-sign-extend attribute is for constants generated by
         the constant divide routine.  The 0x0 prevents the assembler from
         sign extending to 64 bits.
 *-------------------------------------------------------------------------*/

void
P_print_immed_operand_asm (L_Oper * oper, L_Operand * operand)
{
  union CONVERT convert;

  if (L_is_ctype_integer (operand))
    {
      if ((oper->proc_opc == TAHOEop_MOVL) &&
	  (L_find_attr (oper->attr, "no-sign-extend")))
	P_line_print ("0x0%llx", operand->value.i);
      else
	{
	  if ((oper->opc == Lop_CMP) &&
	      (oper->com[0] == L_CTYPE_UINT) && (operand->value.i & 0x80))
	    {
	      /* For the unsigned cmp/cmp4 ias will not accept a
	         negative number. Must bethe _32_bit hex
	         representation of sign extended 8bit immediate */
	      P_line_print ("0x%x", (int) operand->value.i);
	    }			/* if */
	  else
	    P_line_print ("%lld", operand->value.i);
	}			/* else */
    }				/* if */
  else if (L_is_ctype_flt (operand))
    {
      convert.sgl = operand->value.f;
#if defined(X86LIN_SOURCE) || defined(WIN32) || defined(IA64LIN_SOURCE)
      P_line_print ("0x%08x", convert.integer.lo);
#elif defined(_SOLARIS_SOURCE) || defined(_HPUX_SOURCE)
      P_line_print ("0x%08x", convert.integer.hi);
#else
#error Unsupported host platform
#endif
    }				/* else if */
  else if (L_is_ctype_dbl (operand))
    {
      convert.dbl = operand->value.f2;
      P_line_print ("0x0%llx", convert.q);
    }				/* else if */
}				/* P_print_immed_operand_asm */

/****************************************************************************
 *
 * routine: P_print_label_operand_asm
 * purpose: 
 * input:
 * output:
 * returns:
 * modified:
 * note:
 *-------------------------------------------------------------------------*/

void
P_print_label_operand_asm (L_Oper * oper, L_Operand * operand)
{
  char *label_str;
  char fun_label;

  if (operand->value.l == 0)
    L_punt ("P_print_operand_asm: bogus label  oper: %d", oper->id);

  if (!strncmp (operand->value.l, "_$fn", 4))
    {
      fun_label = TRUE;
      label_str = operand->value.l + 5;	/* remove "_$fn_" */
    }				/* if */
  else
    {
      fun_label = FALSE;
      label_str = operand->value.l + 1;	/* remove "_" */
    }				/* else */

  switch (oper->proc_opc)
    {
    case TAHOEop_BR_CALL:
      P_symtab_add_label (label_str, 1);
      break;
    case TAHOEop_MOVL:
    case TAHOEop_ADDL:
    case TAHOEop_NON_INSTR:
      P_symtab_add_label (label_str, 0);
      break;
    case TAHOEop_BRP:
      break;
    default:
      fprintf (stderr, "Warning - label: %s not tracked in oper %d\n",
	       label_str, oper->id);
    }				/* switch */

  if (oper->proc_opc == TAHOEop_NON_INSTR)
    P_line_print ("\n%s:\n", label_str);	/* label for hint */
  else if (Ltahoe_use_gp_rel_addressing)
    {
      if (L_is_LOCAL_GP_label (operand))
	P_line_print ("@gprel(%s#)", label_str);
      else if (L_is_GLOBAL_GP_label (operand))
	if (fun_label)
	  {
	    P_line_print ("@ltoff(@fptr(%s#))", label_str);
	  }			/* if */
	else
	  {
	    P_line_print ("@ltoff(%s#)", label_str);
	  }			/* else */
      else
	P_line_print ("%s#", label_str);
    }				/* else if */
  else
    {
      P_line_print ("%s#", label_str);
    }				/* else */
}				/* P_print_label_operand_asm */


/****************************************************************************
 *
 * routine: P_print_operand_asm()
 * purpose: 
 * input:
 * output:
 * returns:
 * modified:
 * note:
 *-------------------------------------------------------------------------*/

void
P_print_operand_asm (L_Oper * oper, L_Operand * operand)
{
  if (operand == NULL)
    {
      L_print_oper (stderr, oper);
      L_punt
	("P_print_operand_asm: can't print a null operand from op id %d",
	 oper->id, L_ERR_INTERNAL);
    }				/* if */

  /*   fprintf(stderr, "Operand for oper:%d, type:%s value:%d\n", */
  /*      oper->id, D_operand_type_str(operand), operand->value.i); */

  switch (operand->type)
    {
    case L_OPERAND_IMMED:
      P_print_immed_operand_asm (oper, operand);
      break;
    case L_OPERAND_CB:
      /* Note, the +1 is to not print out a leading underscore */
      /* DAC, added the "#" for code blocks */
#ifdef LOCAL_LABELS
      P_line_print (".%s_%d#", L_fn->name + 1, (operand->value.cb)->id);
#else
      P_line_print ("%s?%d#", L_fn->name + 1, (operand->value.cb)->id);
#endif
      break;
    case L_OPERAND_MACRO:
      P_print_macro_operand_asm (oper, operand, Ltahoe_print_real_regs);
      break;
    case L_OPERAND_REGISTER:
      P_print_register_operand_asm (operand, Ltahoe_print_real_regs);
      break;
    case L_OPERAND_LABEL:
      P_print_label_operand_asm (oper, operand);
      break;
    case L_OPERAND_STRING:
      if (Ltahoe_use_gp_rel_addressing && (L_is_GLOBAL_GP_string (operand)))
	P_line_print ("@gprel($$$%s_%lld#)", L_fn->name, operand->value.i);
      else
	P_line_print ("$$$%s_%lld#", L_fn->name, operand->value.i);
      break;

    default:
      L_print_oper (stderr, oper);
      L_punt ("P_print_operand_asm: unknown operand type", L_ERR_INTERNAL);
    }				/* switch */
}				/* P_print_operand_asm */


void
P_print_var_operand_asm (L_Oper * oper, L_Operand * operand, int real)
{
  switch (operand->type)
    {
    case L_OPERAND_MACRO:
      P_print_macro_operand_asm (oper, operand, real);
      break;
    case L_OPERAND_REGISTER:
      P_print_register_operand_asm (operand, real);
      break;
    default:
      L_print_oper (stderr, oper);
      L_punt ("P_print_var_operand_asm: unknown operand type",
	      L_ERR_INTERNAL);
    }				/* switch */
  return;
}				/* P_print_var_operand_asm */

/****************************************************************************
 *
 * routine: P_convert_reg_nums_operand()
 * purpose: 
 * input:
 * output:
 * returns:
 * modified:
 * note:
 *-------------------------------------------------------------------------*/

void
P_convert_reg_nums_operand (L_Oper * oper, L_Operand * operand)
{
  int real_reg_id;
  int mac;

  if (operand == NULL)
    return;

  /*   fprintf(stderr, "Operand for oper:%d, type:%s value:%d\n", */
  /*      oper->id, D_operand_type_str(operand), operand->value.i); */

  switch (operand->type)
    {
    case L_OPERAND_IMMED:
    case L_OPERAND_CB:
    case L_OPERAND_LABEL:
    case L_OPERAND_STRING:
      /* No conversion required */
      break;
    case L_OPERAND_MACRO:
      switch (operand->value.mac)
	{
	  /* Convert the Px macros into reg numbers.
	     The special macros will be converted in the
	     IMPACT-Vulcan bridge. MCM 8/2000 */
#if 0
	case L_MAC_SP:
	case L_MAC_SAFE_MEM:
	case L_MAC_LV:
	case L_MAC_OP:
	case L_MAC_IP:
	  break;
	case TAHOE_MAC_GP:
	  break;
	case TAHOE_MAC_AP:
	  break;
	case TAHOE_MAC_ZERO:
	  break;
	case TAHOE_MAC_TMPREG1:
	  break;
	case TAHOE_MAC_FZERO:
	  break;
	case TAHOE_MAC_FONE:
	  break;
	case TAHOE_MAC_RETADDR:
	  break;
	case TAHOE_MAC_PRED_TRUE:
	  break;
	case TAHOE_MAC_AR_PFS:
	  break;
	case TAHOE_MAC_UNAT:
	  break;
	case TAHOE_PRED_BLK_REG:
	  break;
#endif
	case TAHOE_PRED_SAVE_REG:
	  real_reg_id =
	    INT_STACK_REG_BASE + num_reg_stack_inputs +
	    num_reg_stack_locals - 2;
	  operand->type = L_OPERAND_REGISTER;
	  operand->value.r = real_reg_id;
	  break;

	case TAHOE_GP_SAVE_REG:
	  real_reg_id =
	    INT_STACK_REG_BASE + num_reg_stack_inputs +
	    num_reg_stack_locals - 1;
	  operand->type = L_OPERAND_REGISTER;
	  operand->value.r = real_reg_id;
	  break;

	case L_MAC_P0:
	case L_MAC_P1:
	case L_MAC_P2:
	case L_MAC_P3:
	case L_MAC_P4:
	case L_MAC_P5:
	case L_MAC_P6:
	case L_MAC_P7:
	  /* These are input parameters */
	  mac = operand->value.mac - L_MAC_P0;
	  if (mac > num_reg_stack_inputs)
	    L_punt ("P_convert_reg_nums_operand: "
		    "nput register index past number "
		    "of reg_stack inputs oper: %d   %d %d\n",
		    oper->id, mac, num_reg_stack_inputs);
	  real_reg_id = INT_STACK_REG_BASE + mac;
	  operand->type = L_OPERAND_REGISTER;
	  operand->value.r = real_reg_id;
	  break;

	case L_MAC_P8:
	case L_MAC_P9:
	case L_MAC_P10:
	case L_MAC_P11:
	case L_MAC_P12:
	case L_MAC_P13:
	case L_MAC_P14:
	case L_MAC_P15:
	  /* These are integer output parameters */
	  mac = operand->value.mac - L_MAC_P8;
	  if (mac > num_reg_stack_outputs)
	    L_punt ("P_convert_reg_nums_operand: "
		    "nput register index past number "
		    "of reg_stack outputs oper: %d   %d %d\n",
		    oper->id, mac, num_reg_stack_outputs);
	  real_reg_id = INT_STACK_REG_BASE + num_reg_stack_inputs
	    + num_reg_stack_locals + mac;
	  operand->type = L_OPERAND_REGISTER;
	  operand->value.r = real_reg_id;
	  break;

	case L_MAC_P16:
	case L_MAC_P17:
	case L_MAC_P18:
	case L_MAC_P19:
	  /* This is a return value, and doesn't go through register stack */
	  mac = operand->value.mac - L_MAC_P16;
	  real_reg_id = INT_RETURN_VALUE_REG + mac;
	  operand->type = L_OPERAND_REGISTER;
	  operand->value.r = real_reg_id;
	  break;

	case L_MAC_P20:
	case L_MAC_P21:
	case L_MAC_P22:
	case L_MAC_P23:
	case L_MAC_P24:
	case L_MAC_P25:
	case L_MAC_P26:
	case L_MAC_P27:
	  /* These are all float input and return parameters */
	  /* Map the parameter position to the floating point registers */
	  mac = operand->value.mac - L_MAC_P20;
	  real_reg_id = FLT_INPUT_PARMS_START + mac;
	  operand->type = L_OPERAND_REGISTER;
	  operand->value.r = real_reg_id;
	  break;

	case L_MAC_P28:
	case L_MAC_P29:
	case L_MAC_P30:
	case L_MAC_P31:
	case L_MAC_P32:
	case L_MAC_P33:
	case L_MAC_P34:
	case L_MAC_P35:
	  L_punt
	    ("P_convert_reg_nums_operand: P28-P35 deprecated old fp ret macros detected.");
	  break;

	default:
	  /* Leave the rest of the macros alone. */
	  break;
	}			/* switch */
      break;

    case L_OPERAND_REGISTER:
      switch (operand->ctype)
	{
	case L_CTYPE_INT:
	case L_CTYPE_LLONG:
	  operand->value.r -= INT_REG_BASE;
	  real_reg_id = operand->value.r;
	  if (real_reg_id > MAX_INT_REGISTER_ID)
	    {
	      L_punt ("P_convert_reg_nums_operand: "
		      "Integer register file overflow: "
		      "using reg %d\n", real_reg_id);
	    }			/* if */
	  break;
	case L_CTYPE_FLOAT:
	case L_CTYPE_DOUBLE:
	  operand->value.r -= FLOAT_REG_BASE;
	  break;

	case L_CTYPE_PREDICATE:
	  operand->value.r -= PRED_REG_BASE;
	  break;

	case L_CTYPE_BTR:
	  operand->value.r -= BRANCH_REG_BASE;
	  break;

	default:
	  L_punt ("P_convert_reg_nums_operand: Unknown register ctype %d\n",
		  operand->ctype);
	}			/* switch */
      break;
    default:
      L_print_oper (stderr, oper);
      L_punt ("P_convert_reg_nums_operand: "
	      "unknown operand type %d", operand->type);
    }				/* switch */
}				/* P_convert_reg_nums_operand */

/****************************************************************************
 *
 * routine: P_convert_reg_nums()
 * purpose:
 * input:
 * output:
 * returns:
 * modified:
 * note:
 *-------------------------------------------------------------------------*/

void
P_convert_reg_nums (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *oper;
  int i;

  if (fn == NULL)
    L_punt ("P_convert_reg_nums: NULL function pointer.\n");

  L_get_reg_stack_info (fn, &num_reg_stack_inputs, &num_reg_stack_locals,
			&num_reg_stack_outputs, &num_reg_stack_rots);

  if (num_reg_stack_inputs == -1 ||
      num_reg_stack_locals == -1 ||
      num_reg_stack_outputs == -1 || num_reg_stack_rots == -1)
    L_punt ("P_convert_reg_nums: num_reg uninitialized.\n");

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
	{
	  if (oper->opc != Lop_DEFINE)
	    {
	      for (i = 0; i < L_max_src_operand; i++)
		P_convert_reg_nums_operand (oper, oper->src[i]);
	      for (i = 0; i < L_max_dest_operand; i++)
		P_convert_reg_nums_operand (oper, oper->dest[i]);
	      for (i = 0; i < L_max_pred_operand; i++)
		P_convert_reg_nums_operand (oper, oper->pred[i]);
	    }			/* if */
	}			/* for oper */
    }				/* for cb */
}				/* P_convert_reg_nums */

/************************************************************************
 ** OPERS                                                              **
 ************************************************************************/

/****************************************************************************
 *
 * routine: P_print_define_oper()
 * purpose: 
 * input:
 * output: 
 * returns:
 * modified:
 * note:
 *-------------------------------------------------------------------------*/

void
P_print_define_oper (L_Oper * oper)
{
  if (L_is_macro (oper->dest[0]))
    {
      switch (oper->dest[0]->value.mac)
	{
	case TAHOE_MAC_LABEL:
	  P_print_operand_asm (oper, oper->src[0]);
	  break;
	case TAHOE_MAC_TEMPLATE:
	  P_open_bundle (oper);
	  break;
	}			/* switch */
    }				/* if */
}				/* P_print_define_oper */

/* 09/13/02 REK Print routines start here */




/****************************************************************************
 *
 * routine: P_print_nop
 * purpose: Prints the assembly code for a nop.
 * input: A pointer to the L_Oper structure for this instruction.
 * output:
 * returns:
 * modified: 9/13/02 REK Updating to use the new opcode map/completers
 *                       scheme.
 * note:
 *-------------------------------------------------------------------------*/

void
P_print_nop (L_Oper * oper)
{
  P_line_print ("\t%s\t",
		LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).opc_str);

  if (oper->src[0] != NULL)
    P_print_operand_asm (oper, oper->src[0]);
}				/* P_print_nop */


/****************************************************************************
 *
 * routine: P_print_break
 * purpose: Prints the assembly code for a break instruction.
 * input: A pointer to the L_Oper structure for this instruction.
 * output:
 * returns:
 * modified: 9/13/02 REK Updating to use the new opcode map/completers
 *                       scheme.
 * note:
 *-------------------------------------------------------------------------*/

void
P_print_break (L_Oper * oper)
{
  P_line_print ("\t%s\t",
		LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).opc_str);

  if (oper->src[0] != NULL)
    P_print_operand_asm (oper, oper->src[0]);
}				/* P_print_break */


/****************************************************************************
 *
 * routine: P_print_add
 * purpose: Prints the assembly code for an add, adds, or addl.
 * input: A pointer to the L_Oper structure for the add* opcode.
 * output:
 * returns:
 * modified: 9/13/02 REK Updating function to use the new opcode map and
 *                       completers scheme.
 * note:
 *-------------------------------------------------------------------------*/

void
P_print_add (L_Oper * oper)
{
  int operand;

  if (Ltahoe_generate_unwind_directives)
    {
      if (unwind.mem_stack == oper)
	P_line_print ("\t.fframe\t%d\n", (-unwind.mem_stack_size));
      else if (unwind.mem_stack_dealloc == oper)
	P_line_print ("\t.restore\n");
    }				/* if */

  P_print_predicate (oper);
  P_line_print ("\t%s\t",
		LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).opc_str);
  P_print_operand_asm (oper, oper->dest[0]);
  if (oper->dest[1] != NULL)
    {
      P_line_print (",");
      P_print_operand_asm (oper, oper->dest[1]);
    }				/* if */
  P_line_print (" = ");
  P_print_operand_asm (oper, oper->src[0]);
  for (operand = 1; oper->src[operand] != NULL; operand++)
    {
      P_line_print (",");
      P_print_operand_asm (oper, oper->src[operand]);
    }				/* for operand */
}				/* P_print_add */


/****************************************************************************
 *
 * routine: P_print_load()
 * purpose: Prints the assembly code for a load instruction.
 * input: A pointer to the L_Oper structure for the load opcode.
 * output:
 * returns:
 * modified: 9/16/02 REK Updating to work with the new opcode map and 
 *                       completers scheme.
 * note:
 *-------------------------------------------------------------------------*/

void
P_print_load (L_Oper * oper)
{
  P_print_predicate (oper);
  P_line_print ("\t%s",
		LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).opc_str);

  /* Print the completers for this opcode */
  if (LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).print_completers)
    LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).print_completers (oper);

  P_line_print ("\t");

  P_print_operand_asm (oper, oper->dest[0]);
  P_line_print (" = [");
  P_print_operand_asm (oper, oper->src[0]);
  P_line_print ("]");

  if (L_postincrement_load_opcode (oper))
    {				/* optional post-incr in src1 */
      P_check_post_src_equal_dest (oper, oper->src[0], oper->dest[1]);
      P_line_print (",");
      P_print_operand_asm (oper, oper->src[1]);
    }				/* if */
}				/* P_print_load */


/****************************************************************************
 *
 * routine: P_print_fill_int_load()
 * purpose: Print integer load fills.
 * input: A pointer to the L_Oper structure for a load fill opcode.
 * output:
 * returns:
 * modified: 9/16/02 REK Updating to use the new opcode map and completers
 *                       scheme.
 * note:
 *-------------------------------------------------------------------------*/

void
P_print_fill_int_load (L_Oper * oper)
{
  P_print_predicate (oper);
  P_line_print ("\t%s",
		LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).opc_str);

  /* Print the completers for this opcode */
  if (LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).print_completers)
    LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).print_completers (oper);

  P_line_print ("\t");
  P_print_operand_asm (oper, oper->dest[0]);
  P_line_print (" = [");
  P_print_operand_asm (oper, oper->src[0]);
  P_line_print ("]");
  /* unat in src1 */
  if (oper->src[2] != NULL)
    {				/* optional post-incr in src2 */
      P_check_post_src_equal_dest (oper, oper->src[0], oper->dest[1]);
      P_line_print (",");
      P_print_operand_asm (oper, oper->src[2]);
    }				/* if */
}				/* P_print_fill_int_load */


/****************************************************************************
 *
 * routine: P_print_lfetch
 * purpose: Prints the assembly code for an lfetch instruction.
 * input: A pointer to the L_Oper structure for an lfetch opcode.
 * output:
 * returns:
 * modified:
 * note: Created 9/16/02 REK
 *-------------------------------------------------------------------------*/

void
P_print_lfetch (L_Oper * oper)
{
  P_print_predicate (oper);
  P_line_print ("\t%s",
		LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).opc_str);

  if (LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).print_completers)
    LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).print_completers (oper);

  P_line_print ("\t[");
  P_print_operand_asm (oper, oper->src[0]);
  P_line_print ("]");

  if (oper->src[1] != NULL)
    {
      P_line_print (",");
      P_print_operand_asm (oper, oper->src[1]);
    }				/* if */
}				/* P_print_lfetch */


/****************************************************************************
 *
 * routine: P_print_check
 * purpose: Prints the assembly code for a chk instruction.
 * input: A pointer to the L_Oper structure for a chk opcode.
 * output:
 * returns:
 * modified: 9/16/02 REK Updating to use the new opcode map and completers
 *                       scheme.
 * note:
 *-------------------------------------------------------------------------*/

void
P_print_check (L_Oper * oper)
{
  if (!L_do_recovery_code)
    return;

  P_print_predicate (oper);
  P_line_print ("\t%s",
		LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).opc_str);

  /* Print the completers for this opcode */
  if (LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).print_completers)
    LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).print_completers (oper);

  P_line_print ("\t");
  P_print_operand_asm (oper, oper->src[0]);
  P_line_print (", ");
  if (oper->src[1])
    {
      P_print_operand_asm (oper, oper->src[1]);
    }				/* if */
  else
    {
      L_print_oper (stderr, oper);
      L_punt ("P_print_check: check is missing branch source, asm invalid\n");
    }				/* else */
}				/* P_print_check */


/****************************************************************************
 *
 * routine: P_print_fchkf
 * purpose: Prints the assembly code for an fchkf instruction.
 * input: A pointer to an L_Oper structure for an fchkf opcode.
 * output:
 * returns:
 * modified:
 * note: Created 9/16/02 REK
 *       This function assumes that the target will be in src[0]
 *-------------------------------------------------------------------------*/

void
P_print_fchkf (L_Oper * oper)
{
  P_print_predicate (oper);

  P_line_print ("\t%s",
		LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).opc_str);

  /* Print the completers for this opcode */
  if (LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).print_completers)
    LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).print_completers (oper);

  P_line_print ("\t");

  /* Assumes that target25 is in src[0] */
  P_print_operand_asm (oper, oper->src[0]);
}				/* P_print_fchkf */


/****************************************************************************
 *
 * routine: P_print_fclrf
 * purpose: Prints the assembly code for an fclrf instruction.
 * input: A pointer to an L_Oper structure for an fclrf opcode.
 * output:
 * returns:
 * modified:
 * note: Created 9/16/02 REK
 *-------------------------------------------------------------------------*/

void
P_print_fclrf (L_Oper * oper)
{
  P_print_predicate (oper);

  P_line_print ("\t%s",
		LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).opc_str);

  /* Print the completers for this opcode */
  if (LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).print_completers)
    LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).print_completers (oper);

  P_line_print ("\t");
}				/* P_print_fclrf */


/****************************************************************************
 *
 * routine: P_print_fsetc
 * purpose: Prints the assembly code for an fsetc instruction.
 * input: A pointer to an L_Oper structure for an fsetc opcode.
 * output:
 * returns:
 * modified:
 * note: Created 9/16/02 REK
 *       Assumes that amask7 is in src[0] and omask7 is in src[1]
 *-------------------------------------------------------------------------*/
void
P_print_fsetc (L_Oper * oper)
{
  P_print_predicate (oper);

  P_line_print ("\t%s",
		LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).opc_str);

  /* Print the completers for this opcode */
  if (LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).print_completers)
    LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).print_completers (oper);

  P_line_print ("\t");
  P_print_operand_asm (oper, oper->src[0]);
  P_line_print (",");
  P_print_operand_asm (oper, oper->src[1]);
}				/* P_print_fsetc */


/****************************************************************************
 *
 * routine: P_print_branch()
 * purpose: Prints the assembly code for a branch instruction.
 * input: A pointer to an L_Oper structure for a branch opcode.
 * output:
 * returns:
 * modified: 9/16/02 REK Updating to use the new opcode map and completers
 *                       scheme.
 * note:
 *-------------------------------------------------------------------------*/

void
P_print_branch (L_Oper * oper)
{
  P_print_predicate (oper);
  P_line_print ("\t%s",
		LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).opc_str);

  /* Print the completers if this instruction has them */
  if (LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).print_completers)
    LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).print_completers (oper);

  P_line_print ("\t");
  if (oper->dest[0] != NULL)
    {
      P_print_operand_asm (oper, oper->dest[0]);
      P_line_print ("=");
    }				/* if */
  P_print_operand_asm (oper, oper->src[0]);
}				/* P_print_branch */


/****************************************************************************
 *
 * routine: P_print_branch_hint
 * purpose: Prints the assembly code for a branch hint instruction.
 * input: A pointer to an L_Oper structure for the BRP opcode.
 * output:
 * returns:
 * modified: 9/16/02 REK Updating to use the new opcode map and completers
 *                       scheme.
 * note:
 *-------------------------------------------------------------------------*/

void
P_print_branch_hint (L_Oper * oper)
{
  P_print_predicate (oper);
  P_line_print ("\t%s",
		LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).opc_str);

  /* Print the completers if this instruction has them */
  if (LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).print_completers)
    LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).print_completers (oper);

  P_line_print ("\t");
  P_print_operand_asm (oper, oper->src[0]);
  P_line_print (",");
  P_print_operand_asm (oper, oper->src[1]);
}				/* P_print_branch_hint */


/****************************************************************************
 *
 * routine: P_print_mnemonic_only
 * purpose: Prints the assembly code for an instruction that has no dests,
 *          sources, or completers.
 * input: A pointer to an L_Oper structure for the opcode.
 * output:
 * returns:
 * modified:
 * note: Created 9/16/02 REK
 *-------------------------------------------------------------------------*/

void
P_print_mnemonic_only (L_Oper * oper)
{
  P_print_predicate (oper);

  P_line_print ("\t%s\t",
		LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).opc_str);
}				/* P_print_mnemonic_only */


/****************************************************************************
 *
 * routine: P_print_cmpxchg
 * purpose: Prints the assembly code for a cmpxchg instruction.
 * input: A pointer to an L_Oper structure for the cmpxchg opcode.
 * output:
 * returns:
 * modified:
 * note: Created 9/16/02 REK
 *-------------------------------------------------------------------------*/

void
P_print_cmpxchg (L_Oper * oper)
{
  P_print_predicate (oper);

  P_line_print ("\t%s",
		LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).opc_str);

  if (LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).print_completers)
    LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).print_completers (oper);

  P_line_print ("\t");
  P_print_operand_asm (oper, oper->dest[0]);
  P_line_print (" = [");
  P_print_operand_asm (oper, oper->src[0]);
  P_line_print ("],");
  P_print_operand_asm (oper, oper->src[1]);
  P_line_print (", ar.ccv");
}				/* P_print_cmpxchg */


/****************************************************************************
 *
 * routine: P_print_fetchadd_xchg
 * purpose: Prints the assembly code for a fetchadd or xchg instruction.
 * input: A pointer to an L_Oper structure for the fetchadd or xchg opcode.
 * output:
 * returns:
 * modified:
 * note: Created 9/16/02 REK
 *       Assumes inc3 is stored in src[1]
 *-------------------------------------------------------------------------*/

void
P_print_fetchadd_xchg (L_Oper * oper)
{
  P_print_predicate (oper);

  P_line_print ("\t%s",
		LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).opc_str);

  if (LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).print_completers)
    LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).print_completers (oper);

  P_line_print ("\t");
  P_print_operand_asm (oper, oper->dest[0]);
  P_line_print (" = [");
  P_print_operand_asm (oper, oper->src[0]);
  P_line_print (",");
  P_print_operand_asm (oper, oper->src[1]);
}				/* P_print_fetchadd_xchg */


/****************************************************************************
 *
 * routine: P_print_store()
 * purpose: Print store opers.
 * input: A pointer to an L_Oper structure for the store instruction.
 * output:
 * returns:
 * modified: 9/16/02 REK Updating to use the new opcode map and completers
 *                       scheme.
 * note: Target address is stored in src0, not dest0
 *-------------------------------------------------------------------------*/

void
P_print_store (L_Oper * oper)
{
  if (Ltahoe_generate_unwind_directives &&
      ((oper->proc_opc == TAHOEop_ST8) ||
       (oper->proc_opc == TAHOEop_ST8_SPILL)))
    {
      if (oper == unwind.pfs.save_op)
	P_line_print ("\t.savesp\tar.pfs, %d\n", unwind.pfs.loc.ofst);
      else if (oper == unwind.rp.save_op)
	P_line_print ("\t.savesp\trp, %d\n", unwind.rp.loc.ofst);
      else if (oper == unwind.unat.save_op)
	P_line_print ("\t.savesp\tar.unat, %d\n", unwind.unat.loc.ofst);
      else if (oper == unwind.rnat.save_op)
	P_line_print ("\t.savesp\tar.rnat, %d\n", unwind.rnat.loc.ofst);
      else if (oper == unwind.lc.save_op)
	P_line_print ("\t.savesp\tar.lc, %d\n", unwind.lc.loc.ofst);
      else if (oper == unwind.fpsr.save_op)
	P_line_print ("\t.savesp\tar.fpsr, %d\n", unwind.fpsr.loc.ofst);
      else if (oper == unwind.pr.save_op)
	P_line_print ("\t.savesp\tpr, %d\n", unwind.pr.loc.ofst);
    }				/* if */

  P_print_predicate (oper);

  P_line_print ("\t%s",
		LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).opc_str);

  /* Print the completers for this instruction. */
  if (LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).print_completers)
    LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).print_completers (oper);

  P_line_print ("\t[");
  P_print_operand_asm (oper, oper->src[0]);
  /* store target address in src0 */
  P_line_print ("] = ");
  /* store source in src1 */
  P_print_operand_asm (oper, oper->src[1]);

  /* Handle the post form of st8.spill specially */
  if (oper->proc_opc == TAHOEop_ST8_SPILL &&
      L_postincrement_store_opcode (oper))
    {
      /* post-incr in src2 and dest2 */
      P_check_post_src_equal_dest (oper, oper->src[0], oper->dest[1]);
      P_line_print (",");
      P_print_operand_asm (oper, oper->src[2]);
    }				/* if */
  else if (oper->src[2] != NULL)
    {				/* optional post-incr in src2 */
      P_check_post_src_equal_dest (oper, oper->src[0], oper->dest[0]);
      P_line_print (",");
      P_print_operand_asm (oper, oper->src[2]);
    }				/* else if */
}				/* P_print_store */


/****************************************************************************
 *
 * routine: P_print_invala
 * purpose: Prints the assembly code for an invala instruction.
 * input: A pointer to an L_Oper structure for the invala opcode.
 * output:
 * returns:
 * modified:
 * note: Created 9/16/02 REK
 *-------------------------------------------------------------------------*/

void
P_print_invala (L_Oper * oper)
{
  P_print_predicate (oper);

  P_line_print ("\t%s\t",
		LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).opc_str);

  if (oper->src[0] != NULL)
    P_print_operand_asm (oper, oper->src[0]);
}				/* P_print_invala */


/****************************************************************************
 *
 * routine: P_print_cc
 * purpose: Prints the assembly code for a cc instruction.
 * input: A pointer to an L_Oper structure for the cc opcode.
 * output:
 * returns:
 * modified:
 * note: Created 9/16/02 REK
 *-------------------------------------------------------------------------*/

void
P_print_cc (L_Oper * oper)
{
  P_print_predicate (oper);

  P_line_print ("\t%s\t",
		LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).opc_str);

  P_print_operand_asm (oper, oper->src[0]);
}				/* P_print_cc */


/****************************************************************************
 *
 * routine: P_print_halt
 * purpose: Prints the assembly code for a halt instruction.
 * input: A pointer to an L_Oper structure for the halt opcode.
 * output:
 * returns:
 * modified:
 * note: Created 9/16/02 REK
 *-------------------------------------------------------------------------*/

void
P_print_halt (L_Oper * oper)
{
  P_print_predicate (oper);

  P_line_print ("\t%s\t",
		LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).opc_str);

  if (oper->src[0] != NULL)
    P_print_operand_asm (oper, oper->src[0]);
}				/* P_print_halt */


/****************************************************************************
 *
 * routine: P_print_alloc
 * purpose: Prints the assembly code for an alloc instruction.
 * input: A pointer to an L_Oper structure for the alloc opcode
 * output:
 * returns:
 * modified: 9/16/02 REK Updating to use the new opcode map and completers
 *                       scheme.
 * note:
 *-------------------------------------------------------------------------*/

void
P_print_alloc (L_Oper * oper)
{
  if (Ltahoe_generate_unwind_directives && (unwind.pfs.save_op == oper))
    {
      P_line_print ("\t.save\tar.pfs, ");
      P_print_var_operand_asm (oper, oper->dest[0], 1);
      P_line_print ("\n");
    }				/* if */

  P_line_print ("\t%s\t",
		LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).opc_str);
  P_print_operand_asm (oper, oper->dest[0]);
  P_line_print (" = ar.pfs, %lld,%lld,%lld,%lld",
		oper->src[0]->value.i, oper->src[1]->value.i,
		oper->src[2]->value.i, oper->src[3]->value.i);
}				/* P_print_alloc */


/****************************************************************************
 *
 * routine: P_print_fc
 * purpose: Prints the assembly code for an fc instruction.
 * input: A pointer to an L_Oper structure for the fc opcode.
 * output:
 * returns:
 * modified:
 * note: Created 9/16/02 REK
 *-------------------------------------------------------------------------*/

void
P_print_fc (L_Oper * oper)
{
  P_print_predicate (oper);

  P_line_print ("\t%s\t",
		LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).opc_str);

  P_print_operand_asm (oper, oper->src[0]);
}				/* P_print_fc */


/****************************************************************************
 *
 * routine: P_print_itc
 * purpose: Prints the assembly code for an itc instruction.
 * input: A pointer to an L_Oper structure for the itc opcode.
 * output:
 * returns:
 * modified:
 * note: Created 9/16/02 REK
 *-------------------------------------------------------------------------*/

void
P_print_itc (L_Oper * oper)
{
  P_print_predicate (oper);

  P_line_print ("\t%s\t",
		LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).opc_str);

  P_print_operand_asm (oper, oper->src[0]);
}				/* P_print_itc */


/****************************************************************************
 *
 * routine: P_print_itr
 * purpose: Prints the assembly code for an itr instruction.
 * input: A pointer to an L_Oper structure for the itr opcode.
 * output:
 * returns:
 * modified:
 * note: Created 9/16/02 REK
 *       This function assumes that r3 is in dest[0] and r2 is in src[0]
 *-------------------------------------------------------------------------*/

void
P_print_itr (L_Oper * oper)
{
  P_print_predicate (oper);

  P_line_print ("\t%s\titr[",
		LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).opc_str);

  P_print_operand_asm (oper, oper->dest[0]);
  P_line_print ("] = ");
  P_print_operand_asm (oper, oper->src[0]);
}				/* P_print_itr */


/****************************************************************************
 *
 * routine: P_print_probe_fault
 * purpose: Prints assembly code for the probe*fault instructions.
 * input: A pointer to an L_Oper structure for the probe*fault opcode.
 * output:
 * returns:
 * modified:
 * note: Created 9/16/02 REK
 *-------------------------------------------------------------------------*/

void
P_print_probe_fault (L_Oper * oper)
{
  P_print_predicate (oper);

  P_line_print ("\t%s\t",
		LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).opc_str);

  P_print_operand_asm (oper, oper->src[0]);
  P_line_print (",");
  P_print_operand_asm (oper, oper->src[1]);
}				/* P_print_probe_fault */


/****************************************************************************
 *
 * routine: P_print_purge
 * purpose: Prints the assembly code for the ptc and ptr instructions.
 * input: A pointer to an L_Oper structure for the pct or pct opcode.
 * output:
 * returns:
 * modified:
 * note: Created 9/16/02 REK
 *-------------------------------------------------------------------------*/

void
P_print_purge (L_Oper * oper)
{
  P_print_predicate (oper);

  P_line_print ("\t%s\t",
		LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).opc_str);

  P_print_operand_asm (oper, oper->src[0]);

  /* Print the second operand if the instruction has it. */
  if (oper->src[1] != NULL)
    {
      P_line_print (",");
      P_print_operand_asm (oper, oper->src[1]);
    }				/* if */
}				/* P_print_purge */


/****************************************************************************
 *
 * routine: P_print_mask_op
 * purpose: Prints the assembly code for the rsm, rum, ssm, sum instructions.
 * input: A pointer to an L_Oper structure for the opcode.
 * output:
 * returns:
 * modified:
 * note: Created 9/16/02 REK
 *       This function assumes that the immediate is stored in src[0]
 *-------------------------------------------------------------------------*/

void
P_print_mask_op (L_Oper * oper)
{
  P_print_predicate (oper);

  P_line_print ("\t%s\t",
		LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).opc_str);

  P_print_operand_asm (oper, oper->src[0]);
}				/* P_print_mask_op */


/****************************************************************************
 *
 * routine: P_print_mov_from_br()
 * purpose: Print move from branch register opers.
 * input: A pointer to an L_Oper structure for the opcode.
 * output:
 * returns:
 * modified: 9/16/02 REK Updating to use new opcode map and completers
 *                       scheme.
 * note:
 *-------------------------------------------------------------------------*/

void
P_print_mov_from_br (L_Oper * oper)
{
  int operand;

  if (Ltahoe_generate_unwind_directives && (unwind.rp.save_op == oper))
    {
      P_line_print ("\t.save\trp, ");
      P_print_var_operand_asm (oper, oper->dest[0], 1);
      P_line_print ("\n");
    }

  P_print_predicate (oper);
  P_line_print ("\t%s",
		LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).opc_str);

  if (LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).print_completers)
    LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).print_completers (oper);

  P_line_print ("\t");

  P_print_operand_asm (oper, oper->dest[0]);
  if (oper->dest[1] != NULL)
    {
      P_line_print (",");
      P_print_operand_asm (oper, oper->dest[1]);
    }				/* if */
  P_line_print (" = ");
  P_print_operand_asm (oper, oper->src[0]);
  for (operand = 1; oper->src[operand] != NULL; operand++)
    {
      P_line_print (",");
      P_print_operand_asm (oper, oper->src[operand]);
    }				/* for operand */
}				/* P_print_mov_from_br */


/****************************************************************************
 *
 * routine: P_print_mov_from_ar()
 * purpose: Print move from application register opers.
 * input: A pointer to an L_Oper structure for the opcode.
 * output:
 * returns:
 * modified: 9/16/02 REK Updating to use the new opcode map and completers
 *                       scheme.
 * note:
 *-------------------------------------------------------------------------*/

void
P_print_mov_from_ar (L_Oper * oper)
{
  int operand;

  if (Ltahoe_generate_unwind_directives)
    {
      if (unwind.unat.save_op == oper)
	{
	  P_line_print ("\t.save\tar.unat, ");
	  P_print_var_operand_asm (oper, oper->dest[0], 1);
	  P_line_print ("\n");
	}			/* if */
      else if (unwind.rnat.save_op == oper)
	{
	  P_line_print ("\t.save\tar.rnat, ");
	  P_print_var_operand_asm (oper, oper->dest[0], 1);
	  P_line_print ("\n");
	}			/* else if */
      else if (unwind.lc.save_op == oper)
	{
	  P_line_print ("\t.save\tar.lc, ");
	  P_print_var_operand_asm (oper, oper->dest[0], 1);
	  P_line_print ("\n");
	}			/* else if */
      else if (unwind.fpsr.save_op == oper)
	{
	  P_line_print ("\t.save\tar.fpsr, ");
	  P_print_var_operand_asm (oper, oper->dest[0], 1);
	  P_line_print ("\n");
	}			/* else if */
    }				/* if */

  P_print_predicate (oper);
  P_line_print ("\t%s",
		LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).opc_str);

  if (LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).print_completers)
    LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).print_completers (oper);

  P_line_print ("\t");

  P_print_operand_asm (oper, oper->dest[0]);
  if (oper->dest[1] != NULL)
    {
      P_line_print (",");
      P_print_operand_asm (oper, oper->dest[1]);
    }				/* if */
  P_line_print (" = ");
  P_print_operand_asm (oper, oper->src[0]);
  for (operand = 1; oper->src[operand] != NULL; operand++)
    {
      P_line_print (",");
      P_print_operand_asm (oper, oper->src[operand]);
    }				/* for operand */
}				/* P_print_mov_from_ar */


/****************************************************************************
 *
 * routine: P_print_mov_from_pr()
 * purpose: Print move from predicates opers.
 * input: A pointer to an L_Oper structure for the opcode.
 * output:
 * returns:
 * modified: 9/16/02 REK Updating to use the new opcode map and completers
 *                       scheme.
 * note:
 *-------------------------------------------------------------------------*/

void
P_print_mov_from_pr (L_Oper * oper)
{
  int operand;

  if (Ltahoe_generate_unwind_directives && (unwind.pr.save_op == oper))
    {
      P_line_print ("\t.save\tpr, ");
      P_print_var_operand_asm (oper, oper->dest[0], 1);
      P_line_print ("\n");
    }				/* if */

  P_print_predicate (oper);
  P_line_print ("\t%s",
		LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).opc_str);

  if (LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).print_completers)
    LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).print_completers (oper);

  P_line_print ("\t");

  P_print_operand_asm (oper, oper->dest[0]);
  if (oper->dest[1] != NULL)
    {
      P_line_print (",");
      P_print_operand_asm (oper, oper->dest[1]);
    }				/* if */
  P_line_print (" = ");
  P_print_operand_asm (oper, oper->src[0]);
  for (operand = 1; oper->src[operand] != NULL; operand++)
    {
      P_line_print (",");
      P_print_operand_asm (oper, oper->src[operand]);
    }				/* for operand */
}				/* P_print_mov_from_pr */


/****************************************************************************
 *
 * routine: P_check_post_src_equal_dest
 * purpose: makes sure src and dest copy of effective addr. register are
 *          the same
 * input:
 * output:
 * returns:
 * modified:
 * note:
 *-------------------------------------------------------------------------*/

int
P_check_post_src_equal_dest (L_Oper * oper, L_Operand * src, L_Operand * dest)
{
  if (L_is_register (src) && L_is_register (dest) &&
      (src->value.r == dest->value.r))
    return (1);

  if (L_is_macro (src) && L_is_macro (dest) &&
      (src->value.mac == dest->value.mac))
    return (1);

  L_print_oper (stderr, oper);
  L_punt ("P_check_post_src_equal_dest: src and dest of post_inc not the "
	  "same in oper %d\n", oper->id);
  return (0);
}				/* P_check_post_src_equal_dest */


/****************************************************************************
 *
 * routine: P_print_non_instr()
 * purpose: Print out info related to non-instructions
 * input: oper - pointer to non-instrction op
 * output: syllable_mask - initialized only if this non-instr is a template
 *         stop_bit_mask - same
 * returns:
 * modified:
 * note:
 *-------------------------------------------------------------------------*/

void
P_print_non_instr (L_Oper * oper)
{
  switch (oper->opc)
    {
    case Lop_DEFINE:
      P_print_define_oper (oper);	/* Can start a bundle */
      break;

    case Lop_PROLOGUE:
    case Lop_EPILOGUE:
      break;

    default:
      L_punt ("Unknown TAHOEop_NON_INSTR  oper:%d\n", oper->id);
    }				/* switch */
}				/* P_print_non_instr */


/*
 * Restore MAC_PRED_TRUE operands where necessary
 * and move dests to appropriate slots, so as not to break
 * ASM generation -- JWS 20001201
 */
/* 09/16/02 REK Updating to use the new opcode map and completers scheme. */
void
P_fix_pred_compare_dests (L_Oper * oper)
{
  int compare_type;
  int ptype[2];
  L_Operand *dest[2];

  dest[0] = oper->dest[0];
  dest[1] = oper->dest[1];

  if (!dest[0])
    {
      dest[0] = dest[1];
      dest[1] = NULL;
    }				/* if */

  /* 09/16/02 REK Using the new completers scheme to get the compare type. */
  /* compare_type = M_tahoe_compare_type (oper->proc_opc); */
  compare_type = TC_GET_CMP_TYPE (oper->completers);

  switch (compare_type)
    {
    case TC_CMP_TYPE_UNC:
      ptype[0] = L_PTYPE_UNCOND_T;
      ptype[1] = L_PTYPE_UNCOND_F;
      break;
    case TC_CMP_TYPE_NONE:
      ptype[0] = L_PTYPE_COND_T;
      ptype[1] = L_PTYPE_COND_F;
      break;
    case TC_CMP_TYPE_AND:
      ptype[0] = L_PTYPE_AND_T;
      ptype[1] = L_PTYPE_AND_T;
      break;
    case TC_CMP_TYPE_OR:
      ptype[0] = L_PTYPE_OR_T;
      ptype[1] = L_PTYPE_OR_T;
      break;
    case TC_CMP_TYPE_OR_ANDCM:
      ptype[0] = L_PTYPE_OR_T;
      ptype[1] = L_PTYPE_AND_F;
      break;
    case TC_CMP_TYPE_AND_ORCM:
      ptype[0] = L_PTYPE_AND_T;
      ptype[1] = L_PTYPE_OR_F;
      break;
    default:
      L_punt ("P_fix_pred_compare_dests: Bad compare type");
    }				/* switch */

  if (dest[0] && dest[1])
    {
      if (dest[0]->ptype != ptype[0] || dest[1]->ptype != ptype[1])
	L_punt ("P_fix_pred_compare_dests: Two-dest pred compare messed up");
    }				/* if */
  else if (dest[0])
    {
      if (dest[0]->ptype == ptype[0])
	{
	  oper->dest[0] = dest[0];
	  oper->dest[1] = L_new_macro_operand (TAHOE_MAC_PRED_TRUE,
					       L_CTYPE_PREDICATE, ptype[1]);
	}			/* if */
      else if (dest[0]->ptype == ptype[1])
	{
	  oper->dest[1] = dest[0];
	  oper->dest[0] = L_new_macro_operand (TAHOE_MAC_PRED_TRUE,
					       L_CTYPE_PREDICATE, ptype[0]);
	}			/* else if */
      else
	{
	  L_punt ("P_fix_pred_compare_dests: Single-dest pred cmp messed up");
	}			/* else */
    }				/* else if */
  else
    {
      L_punt ("P_fix_pred_compare_dests: Empty pred def");
    }				/* else */
}				/* P_fix_pred_compare_dests */


/* Fix predicate destinations on a function basis.  Currently used
   for Vulcan generated code. */
void
P_fix_pred_compare_dests_func (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *oper;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
	{
	  if (L_general_pred_comparison_opcode (oper))
	    P_fix_pred_compare_dests (oper);
	}			/* for oper */
    }				/* for cb */
}				/* P_fix_pred_compare_dests_func */


  /* Print predicate relation hints for predicate uses in the
   * current bundle to avoid pissing off the assembler
   */
void
P_print_pred_deps (L_Oper * oper, int softpipe)
{
  L_Oper *op_iter;
  L_Oper *last_op = NULL;
  Set pdests = NULL;
  Set copds = NULL;

  int syllable_mask = P_bundle_status.syllable_mask;
  int stop_bit_mask = P_bundle_status.stop_bit_mask;

  int i;
  /* See what predicates are used */

  op_iter = oper;
  while (op_iter)
    {
      last_op = op_iter->next_op;
      if (op_iter->opc == Lop_DEFINE)
	{
	  if (L_is_macro (op_iter->dest[0]))
	    {
	      switch (op_iter->dest[0]->value.mac)
		{
		case TAHOE_MAC_LABEL:
		  syllable_mask = stop_bit_mask = 1;
		  break;
		case TAHOE_MAC_TEMPLATE:
		  syllable_mask = S_AFTER_1ST;
		  stop_bit_mask = LT_get_stop_bit_mask (op_iter);
		  break;
		}		/* switch */
	    }			/* if */
	}			/* if */
      else
	{
	  if (op_iter->pred[0])
	    {
	      for (i = 0; i < L_max_src_operand; i++)
		{
		  if (!L_is_variable (op_iter->src[i]))
		    continue;
		  if (Set_in (pdests, L_REG_MAC_INDEX (op_iter->src[i])))
		    copds = Set_add (copds,
				     L_REG_MAC_INDEX (op_iter->src[i]));
		}		/* for i */
	      for (i = 0; i < L_max_dest_operand; i++)
		{
		  if (!L_is_variable (op_iter->dest[i]))
		    continue;
		  if (Set_in (pdests, L_REG_MAC_INDEX (op_iter->dest[i])))
		    copds = Set_add (copds,
				     L_REG_MAC_INDEX (op_iter->dest[i]));
		  pdests = Set_add (pdests,
				    L_REG_MAC_INDEX (op_iter->dest[i]));
		}		/* for i */
	    }			/* if */
	  if (syllable_mask & stop_bit_mask)
	    break;
	  syllable_mask >>= 1;
	}			/* else */
      op_iter = op_iter->next_op;
    }				/* while */

  /* copds contains all potentially conflicting operand indices */

  if (Set_size (copds))
    {
      List preds = NULL;
      Set cpreds = NULL;
      L_Oper *fopd, *lopd;

      op_iter = oper;
      while (op_iter != last_op)
	{
	  if (op_iter->opc != Lop_DEFINE)
	    {
	      if (op_iter->pred[0] && L_is_reg (op_iter->pred[0]))
		{
		  for (i = 0; i < L_max_src_operand; i++)
		    {
		      if (!L_is_variable (op_iter->src[i]))
			continue;
		      if (Set_in (copds, L_REG_MAC_INDEX (op_iter->src[i]))
			  && !Set_in (cpreds,
				      L_REG_MAC_INDEX (op_iter->pred[0])))
			{
			  preds = List_insert_last (preds, op_iter);
			  cpreds =
			    Set_add (cpreds,
				     L_REG_MAC_INDEX (op_iter->pred[0]));
			}	/* if */
		    }		/* for i */
		  for (i = 0; i < L_max_dest_operand; i++)
		    {
		      if (!L_is_variable (op_iter->dest[i]))
			continue;
		      if (Set_in (copds, L_REG_MAC_INDEX (op_iter->dest[i]))
			  && !Set_in (cpreds,
				      L_REG_MAC_INDEX (op_iter->pred[0])))
			{
			  preds = List_insert_last (preds, op_iter);
			  cpreds =
			    Set_add (cpreds,
				     L_REG_MAC_INDEX (op_iter->pred[0]));
			}	/* if */
		    }		/* for i */
		}		/* if */
	    }			/* if */
	  op_iter = op_iter->next_op;
	}			/* while */

      /* Print relations pairwise in preds */

      if (List_size (preds) > 1)
	fprintf (L_OUT, ".pred.rel \"clear\"\n");

      List_start (preds);
      while ((fopd = List_first (preds)))
	{
	  preds = List_delete_current (preds);
	  List_start (preds);
	  while ((lopd = List_next (preds)))
	    {
	      if (softpipe)
		{
		  /* In software pipelined code with rotating
		     registers, predicate relationships can no longer
		     be analyzed.  Rely on the scheduler to have had
		     made correct decisions about which instructions
		     to schedule in parallel. MCM 5/28/01 */
		  fprintf (L_OUT, ".pred.rel \"mutex\",p%d,p%d\n",
			   fopd->pred[0]->value.r - PRED_REG_BASE,
			   lopd->pred[0]->value.r - PRED_REG_BASE);
		}		/* if */
	      else if (!PG_intersecting_predicates_ops (fopd, lopd))
		{
		  fprintf (L_OUT, ".pred.rel \"mutex\",p%d,p%d\n",
			   fopd->pred[0]->value.r - PRED_REG_BASE,
			   lopd->pred[0]->value.r - PRED_REG_BASE);
		}		/* else if */
	      else if (PG_superset_predicate_ops (fopd, lopd))
		{
		  fprintf (L_OUT, ".pred.rel \"imply\",p%d,p%d\n",
			   lopd->pred[0]->value.r - PRED_REG_BASE,
			   fopd->pred[0]->value.r - PRED_REG_BASE);
		}		/* else if */
	      else if (PG_superset_predicate_ops (lopd, fopd))
		{
		  fprintf (L_OUT, ".pred.rel \"imply\",p%d,p%d\n",
			   fopd->pred[0]->value.r - PRED_REG_BASE,
			   lopd->pred[0]->value.r - PRED_REG_BASE);
		}		/* else if */
	    }			/* while */
	}			/* while */
      List_reset (preds);
      Set_dispose (cpreds);
    }				/* if */

  Set_dispose (copds);
  Set_dispose (pdests);

  return;
}				/* P_print_pred_deps */


/****************************************************************************
 *
 * routine: P_print_oper()
 * purpose: Prints the non-data Mcode operations.
 * input:
 * output:
 * returns:
 * modified: 9/13/02 REK Updating to use the new opcode map and
 *                       completers scheme.
 * note:
 *-------------------------------------------------------------------------*/

void
P_print_oper (L_Cb * cb, L_Oper * oper,
	      unsigned int *instr_offset, unsigned int *issue_cycle)
{
  /* the above two variables are set when we get a template */
  int tahoeop;
  int operand;
  L_Attr *attr;
  int softpipe = 0;

  tahoeop = oper->proc_opc;
  /* asm_str = TMDES_tahoeop_asm_str (tahoeop); */

  if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_SOFTPIPE) &&
      L_find_attr (cb->attr, "kernel"))
    softpipe = 1;

  if (L_pred_compare_opcode (oper))
    P_fix_pred_compare_dests (oper);

  if (tahoeop == TAHOEop_NON_INSTR)
    {
      P_print_non_instr (oper);
      return;
    }				/* if */
  else
    {
      if (Ltahoe_output_for_ias &&
	  P_bundle_status.explicit_mode && 
	  P_bundle_status.new_group)
	P_print_pred_deps (oper, softpipe);

      if (LTAHOE_TABLE_ENTRY (Ltahoe_table,
			      tahoeop).completer_sanity_check &&
	  LTAHOE_TABLE_ENTRY (Ltahoe_table,
			      tahoeop).completer_sanity_check (oper) == 1)
	{
	  printf ("Error: oper %d has invalid completers (0x%x)\n", oper->id,
		  oper->completers);
	}			/* if */
      else
	{
	  if (LTAHOE_TABLE_ENTRY (Ltahoe_table, tahoeop).print_opcode)
	    {
	      LTAHOE_TABLE_ENTRY (Ltahoe_table, tahoeop).print_opcode (oper);
	    }			/* if */
	  else
	    {
	      P_print_predicate (oper);
	      P_line_print ("\t%s",
			    LTAHOE_TABLE_ENTRY (Ltahoe_table,
						tahoeop).opc_str);

	      if (LTAHOE_TABLE_ENTRY (Ltahoe_table, tahoeop).print_completers)
		LTAHOE_TABLE_ENTRY (Ltahoe_table,
				    tahoeop).print_completers (oper);

	      P_line_print ("\t");

	      P_print_operand_asm (oper, oper->dest[0]);
	      if (oper->dest[1] != NULL)
		{
		  P_line_print (",");
		  P_print_operand_asm (oper, oper->dest[1]);
		}		/* if */
	      P_line_print (" = ");
	      P_print_operand_asm (oper, oper->src[0]);
	      for (operand = 1; oper->src[operand] != NULL; operand++)
		{
		  P_line_print (",");
		  P_print_operand_asm (oper, oper->src[operand]);
		}		/* for operand */
	    }			/* else */
	}			/* else */
    }				/* else */

  if (tahoeop != TAHOEop_NON_INSTR)
    P_bundle_instr ();

  (*instr_offset)++;

  if (oper->proc_opc == TAHOEop_MOVL)
    (*instr_offset)++;

  if ((attr = L_find_attr (oper->attr, "isl")) ||
      (attr = L_find_attr (oper->attr, "cycle")))
    *issue_cycle = L_get_int_attr_field (attr, 0);

  /* 20021112 SZU
   * Try using issue_time instead for modulo scheduled loops
   */
  if ((attr = L_find_attr (oper->attr, "issue_time")))
    *issue_cycle = L_get_int_attr_field (attr, 0);

  P_print_comment (oper, *instr_offset, *issue_cycle);
  fprintf (L_OUT, "\n");

  if ((Ltahoe_tag_loads)&&(L_load_opcode(oper)))
    {
      P_print_load_table (oper, cb, *instr_offset, 
			  P_bundle_status.bundle_indx - 1);
    }

  return;
}				/* P_print_oper */
