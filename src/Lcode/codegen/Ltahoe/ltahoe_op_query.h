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
 * This file declares the Tmdes oper queries that Ltahoe uses.  They have
 * been moved from Tmdes to Ltahoe to eliminate Tmdes and make Ltahoe easier
 * to maintain.
 */

#ifndef _LTAHOE_OP_QUERY_H
#define _LTAHOE_OP_QUERY_H

#ifndef _TMDES_INSTR_H_

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>
#include <Lcode/l_code.h>
#include <machine/m_tahoe.h>

/* Maximum values for immediate operands */

#define INT_2EXP(i)             (LLCONST(0x01) << (i))

#define UIMM_4(a)      ((((a) >= 0) && ((a) <= INT_2EXP(4)))?1:0)

/* 6 bit unsigned immediate field */
#define UIMM_6(a)      ((((a) >= 0) && ((a) <= INT_2EXP(6)))?1:0)

/* 8 bit signed immediate field: 1bit sign, 7 bit number */
#define SIMM_8(a)     ((((a) > -INT_2EXP(7))&&((a) < INT_2EXP(7) ))?1:0)

#define UIMM_8(a)      ((((a) >= 0) && ((a) <= INT_2EXP(8)))?1:0)

/* 9 bit signed immediate field: 1bit sign, 7 bit number */
#define SIMM_9(a)     ((((a) > -INT_2EXP(8))&&((a) < INT_2EXP(8) ))?1:0)

/* 14 bit signed immediate field: */
#define SIMM_14(a)     ((((a) > -INT_2EXP(13))&&((a) < INT_2EXP(13)))?1:0)

/* 22 bit signed immediate field: */
#define SIMM_22(a)     ((((a) > -INT_2EXP(21))&&((a) < INT_2EXP(21)))?1:0)

#define LT_is_R0_operand(op)        ((op) && \
                                     (op)->type == L_OPERAND_MACRO && \
                                     (op)->value.mac == TAHOE_MAC_ZERO)

#define LT_is_zero_value(op)        (LT_is_R0_operand(op) || L_is_zero(op))

#define LT_is_P0_operand(op)        ((op) && \
                                     (op)->type == L_OPERAND_MACRO && \
                                     (op)->value.mac == TAHOE_MAC_PRED_TRUE)

#define LT_is_non_instr(op)         ((op)->proc_opc == TAHOEop_NON_INSTR)

#define LT_is_cond_br(op)           (((op)->proc_opc == TAHOEop_BR_COND) || \
                                     ((op)->proc_opc == TAHOEop_BR_CLOOP) || \
                                     ((op)->proc_opc == TAHOEop_BR_CTOP) || \
                                     ((op)->proc_opc == TAHOEop_BR_CEXIT) || \
                                     ((op)->proc_opc == TAHOEop_BR_WTOP) || \
                                     ((op)->proc_opc == TAHOEop_BR_WEXIT))

#define LT_is_indir_br(op)          (((op)->proc_opc == TAHOEop_BR_COND) && \
				     (L_is_variable ((op)->src[0])))

#define LT_is_call_br(op)           ((op)->proc_opc == TAHOEop_BR_CALL)

#define LT_is_ret_br(op)            ((op)->proc_opc == TAHOEop_BR_RET)

#define LT_is_setf(op)              (((op)->proc_opc == TAHOEop_SETF_D) || \
                                     ((op)->proc_opc == TAHOEop_SETF_EXP) || \
                                     ((op)->proc_opc == TAHOEop_SETF_S) || \
                                     ((op)->proc_opc == TAHOEop_SETF_SIG))

#define LT_is_brp(op)               (((op)->proc_opc == TAHOEop_BRP) || \
                                     ((op)->proc_opc == TAHOEop_BRP_RET))

#define LT_is_mov_to_br(op)         ((op)->proc_opc == TAHOEop_MOV_TOBR)

#define LT_is_label_op(op)          ((op)->proc_opc == TAHOEop_NON_INSTR && \
                                     (op)->dest[0] && \
                                     (op)->dest[0]->type == \
                                     L_OPERAND_MACRO && \
                                     (op)->dest[0]->value.mac == \
                                     TAHOE_MAC_LABEL)

#define LT_is_template_op(op)       ((op)->proc_opc == TAHOEop_NON_INSTR && \
                                     (op)->dest[0] && \
                                     (op)->dest[0]->type == \
                                     L_OPERAND_MACRO && \
                                     (op)->dest[0]->value.mac == \
                                     TAHOE_MAC_TEMPLATE)

#define LT_is_cmp_op(op)            (((op)->proc_opc == TAHOEop_CMP) || \
                                     ((op)->proc_opc == TAHOEop_FCMP))

#define LT_is_fill_op(op)           (((op)->proc_opc == TAHOEop_LD8_FILL) || \
                                     ((op)->proc_opc == TAHOEop_LDF_FILL))

#define LT_get_template(tmpl_op)    ((tmpl_op)->src[0]->value.i)

#define LT_new_template(tmpl_op, tmpl_type) \
                                    ((tmpl_op)->src[0] = \
                                           L_new_gen_int_operand(tmpl_type) )

#define LT_set_template(tmpl_op, tmpl_type) \
                                    ((tmpl_op)->src[0]->value.i = tmpl_type)

#define LT_get_stop_bit_mask(tmpl_op) \
                                    ((tmpl_op)->src[1]->value.i)

#define LT_new_stop_bit_mask(tmpl_op, stop_mask ) \
                                    ((tmpl_op)->src[1] = \
                                           L_new_gen_int_operand(stop_mask))

#define LT_set_stop_bit_mask(tmpl_op, stop_mask ) \
                                    ((tmpl_op)->src[1]->value.i = stop_mask)

#define LT_get_density(tmpl_op)     ((tmpl_op)->src[2]->value.i)

#define LT_new_density(tmpl_op,density) \
                                    ((tmpl_op)->src[2] = \
                                           L_new_gen_int_operand (density))

#define LT_set_density(tmpl_op, density) \
                                    ((tmpl_op)->src[2]->value.i = density)

#define S_AFTER_3RD (0x1)	/* stop bit after 3 instr in bundle */
#define S_AFTER_2ND (0x2)	/* stop bit after 2 instr in bundle */
#define S_AFTER_1ST (0x4)	/* stop bit after 1 instr in bundle */
#define NO_S_BIT    (0)		/* no stop bit on this bundle */

#define MII     (0x0)
#define MISI    (0x1)
#define MLI     (0x2)
#define RSVD_T1 (0x3)
#define MMI     (0x4)
#define MSMI    (0x5)
#define MFI     (0x6)
#define MMF     (0x7)
#define MIB     (0x8)
#define MBB     (0x9)
#define RSVD_T3 (0xA)
#define BBB     (0xB)
#define MMB     (0xC)
#define RSVD_T4 (0xD)
#define MFB     (0xE)
#define RSVD_T5 (0xF)

#define M_SYLL          ( 0)
#define I_SYLL          ( 1)
#define F_SYLL          ( 2)
#define B_SYLL          ( 3)
#define L_SYLL          ( 4)
#define INVALID_SYLL    (0xFF)

/* return the type of the syllable for a given instruction.
   returns a value of M_SYLL, I_SYLL, L_SYLL, or F_SYLL.
   Takes a real template (from Templ_First or Templ_Second) and an offset
   into the bundle (starting at 0) as input.
   For example Syllable_Type(MIB, 1) => I_SYLL */

#define NUMBER_OF_REAL_TEMPLATES 16

#define LT_syllable_type(tmp, syl)  LT_SYLLABLE_TYPE_TABLE[(tmp)][(syl)]

extern int LT_SYLLABLE_TYPE_TABLE[NUMBER_OF_REAL_TEMPLATES][3];
extern char *LT_TEMPLATE_NAME[NUMBER_OF_REAL_TEMPLATES];

#define LT_template_name(i)   ((((i)>=0)&&((i)<NUMBER_OF_REAL_TEMPLATES)) \
                                   ? LT_TEMPLATE_NAME[i] \
                                   : "???")

extern int LT_is_input_param_operand (L_Operand *);
extern int LT_is_int_output_param_operand (L_Operand *);
extern int LT_is_float_output_param_operand (L_Operand *);

/* 09/16/02 REK Two functions to replace M_tahoe_cmp_proc_opc.  These
 *              take advantage of the new opcode map and completers
 *              scheme.
 */
extern int LT_tahoe_cmp_proc_opc (ITuint8, ITuint8);
extern int LT_tahoe_cmp_completer (ITuint8, ITuint8, int);

extern L_Oper *LT_create_nop (int, int);
extern L_Oper *LT_create_template_op (int, int);

#endif
#endif
