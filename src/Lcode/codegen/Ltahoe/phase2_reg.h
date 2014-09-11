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
 * File:  phase2_reg.h 
 * Purpose: Register file information
 * Modified: 2/16/96 - Bob McGowan
 *                     seperated from ltahoe_phase3_func.c
 *           5/19/96 - Bob McGowan
 *                     changed stop bit printing to allow for stop bits in the
 *                     middle of a bundle.
 *        
 *===========================================================================*/

#ifndef LTAHOE_PHASE2_REG_H_
#define LTAHOE_PHASE2_REG_H_

/* 10/29/02 REK Adding config.h */
#include <config.h>

/* These defines are now located in Mspec/m_tahoe.h */

/* size of the register file */
#define NUM_INT_REG          TAHOE_NUM_INT_REG
#define NUM_INT_STATIC_REG   TAHOE_NUM_INT_STATIC_REG
#define NUM_INT_STACKED_REG  TAHOE_NUM_INT_STACKED_REG
#define NUM_PRED_REG         TAHOE_NUM_PRED_REG
#define NUM_FLOAT_REG        TAHOE_NUM_FLOAT_REG
#define NUM_BRANCH_REG       TAHOE_NUM_BRANCH_REG

#define MIN_INT_REGISTER_ID          TAHOE_MIN_INT_REGISTER_ID
#define MAX_INT_REGISTER_ID          TAHOE_MAX_INT_REGISTER_ID
#define MIN_INT_STATIC_REGISTER_ID   TAHOE_MIN_INT_STATIC_REGISTER_ID
#define MAX_INT_STATIC_REGISTER_ID   TAHOE_MAX_INT_STATIC_REGISTER_ID
#define MIN_INT_STACKED_REGISTER_ID  TAHOE_MIN_INT_STACKED_REGISTER_ID
#define MAX_INT_STACKED_REGISTER_ID  TAHOE_MAX_INT_STACKED_REGISTER_ID
#define MAX_PRED_REGISTER_NUMBER     TAHOE_MAX_PRED_REGISTER_NUMBER
#define MAX_FLOAT_REGISTER_NUMBER    TAHOE_MAX_FLOAT_REGISTER_NUMBER
#define MAX_BRANCH_REGISTER_NUMBER   TAHOE_MAX_BRANCH_REGISTER_NUMBER

/* not sure about these */
#define NUM_SPECIAL_INT_REG    TAHOE_NUM_SPECIAL_INT_REG
#define NUM_SPECIAL_PRED_REG   TAHOE_NUM_SPECIAL_PRED_REG
#define NUM_SPECIAL_FLOAT_REG  TAHOE_NUM_SPECIAL_FLOAT_REG
#define NUM_SPECIAL_BRANCH_REG TAHOE_NUM_SPECIAL_BRANCH_REG


/* Base register numbers for mapping between internal representation and
   the number define by the architecture.
   This mapping is done so that all registers have a unique id, despite the
   type of the register */
#define INT_REG_BASE       TAHOE_INT_REG_BASE
#define INT_STACK_REG_BASE TAHOE_INT_STACK_REG_BASE
#define INT_SPILL_REG_BASE TAHOE_INT_SPILL_REG_BASE
#define FLOAT_REG_BASE     TAHOE_FLOAT_REG_BASE
#define PRED_REG_BASE      TAHOE_PRED_REG_BASE
#define BRANCH_REG_BASE    TAHOE_BRANCH_REG_BASE

/* after register allocation. */
#define IS_INT_REGISTER(id) (((id) >= INT_REG_BASE) && \
                             ((id) < (INT_REG_BASE + NUM_INT_REG)))
#define IS_FP_REGISTER(id)  (((id) >= FLOAT_REG_BASE) && \
                             ((id) < (FLOAT_REG_BASE + NUM_FLOAT_REG)))
#define IS_BRANCH_REGISTER(id) (((id) >= BRANCH_REG_BASE) && \
                                ((id) < (BRANCH_REG_BASE + NUM_BRANCH_REG)))
#define IS_PREDICATE_REGISTER(id) (((id) >= PRED_REG_BASE) && \
                                   ((id) < (PRED_REG_BASE + NUM_PRED_REG)))

#define MAX_INT_INPUT_REGS    (8)
#define MAX_INT_OUTPUT_REGS   (8)
#define MAX_INT_RETURN_REGS   (4)
#define INT_RETURN_VALUE_REG  (8)

#define FLT_RETURN_VALUE_REG  (8)
#define FLT_INPUT_PARMS_START (8)


/* size of a spill/fill in bytes */
#define INT_CALLEE_SAVE_SIZE      (8)
#define DOUBLE_CALLEE_SAVE_SIZE   (16)
#define PRED_BLK_CALLEE_SAVE_SIZE (8)
#define BTR_CALLEE_SAVE_SIZE      (8)


/* Globals declared in phase2_reg - look there for description */

extern int *callee_pred_array;
extern int *callee_int_array;
extern int *callee_flt_array;
extern int *callee_btr_array;

extern int callee_pred_num;
extern int callee_int_num;
extern int callee_flt_num;
extern int callee_btr_num;

/* External Function Declarations */

extern int O_is_caller_save_predicate (L_Operand * pred);
extern L_Oper *O_spill_reg (int reg, int type, L_Operand * operand,
			    int spill_offset, L_Operand ** pred,
			    int type_flag);
extern L_Oper *O_fill_reg (int reg, int type, L_Operand * operand,
			   int fill_offset, L_Operand ** pred, int type_flag);
extern L_Oper *O_jump_oper (int opc, L_Cb * dest_cb);

extern double R_caller_cost (int lcode_ctype, int leaf);
extern double R_callee_cost (int lcode_ctype, int leaf, int callee_allocated);
extern double R_spill_store_cost (int lcode_ctype);
extern double R_spill_load_cost (int lcode_ctype);
extern void O_register_init (void);
extern void O_register_cleanup ();
extern void O_register_allocation (L_Func * fn,
				   Parm_Macro_List * command_line_macro_list,
				   int *int_swap_space_size,
				   int *fp_swap_space_size,
				   int *pred_swap_space_size);
extern void O_update_alloc_operands (L_Oper * oper, int num_special);
extern void O_remove_spill_code (L_Cb * cb, L_Oper * oper);


#endif
