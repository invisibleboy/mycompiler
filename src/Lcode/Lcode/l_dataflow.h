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
 *      File :          l_dataflow.h
 *      Description :   data flow interface functions
 *      Creation Date : February 1993
 *      Author :        Scott Mahlke, Wen-mei Hwu
 *
 *==========================================================================*/
#ifndef L_DATAFLOW_H
#define L_DATAFLOW_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "l_pred_flow.h"

#define DOMINATOR                 (0x00001)
#define POST_DOMINATOR            (0x00002)
#define LIVE_VARIABLE             (0x00004)
#define REACHING_DEFINITION       (0x00008)
#define AVAILABLE_DEFINITION      (0x00010)
#define AVAILABLE_EXPRESSION      (0x00020)
#define MEM_REACHING_DEFINITION   (0x00040)
#define MEM_AVAILABLE_DEFINITION  (0x00080)
#define CRITICAL_VARIABLE         (0x10000)
#define SUPPRESS_PG               (0x20000)
#define INTERFERENCE              (0x40000)
#define PCE			  (0x100000)

/* WARNING: must avoid DOM flag overlap, dom analysis outside PCE checks. */
#define PRE                       (0x00004)
#define PCE_MEM                   (0x00008)
#define PCE_MEM_CONSERVATIVE      (0x00010)
#define PCE_MEM_COPY_PROP	  (0x00020)
#define PRE_LOAD_DIFF_TYPES       (0x00040)
#define PCE_CUTSET_METRIC	  (0x00080)
#define PRE_SPEC                  (0x00400)
#define PRE_ONLY_EXCEPTING        (0x04000)
#define PCE_REACHING_DEFINITION   (0x08000)
#define MEM_REACHING_LOCATIONS    (0x10000)
#define MEM_ANT_EXPRESSIONS       (0x40000)

#define PDE			  (0x80000)
#define PDE_MIN_CUT               (0x200000)
#define PDE_PREDICATED            (0x400000)
#define DEAD_LOCAL_MEM_VAR        (0x800000)
#define PDE_STORE                 (0x1000000)
#define PDE_STORE_ONLY            (0x2000000)

#define DOMINATOR_CB            (0x00100)
#define POST_DOMINATOR_CB       (0x00200)
#define LIVE_VARIABLE_CB        (0x00400)
#define POINTER_VALUE_CB        (0x00800)

#define DOMINATOR_INT           (0x01000)
#define POST_DOMINATOR_INT      (0x02000)
#define LIVE_VARIABLE_INT       (0x04000)

#define TAKEN_PATH      1
#define FALL_THRU_PATH  2
#define BOTH_PATHS      3

/* 
 *  If L_is_reg or L_is_macro ever become macros, this would
 *  need to change 
 */
#define L_REG_MAC_INDEX(i)   (L_is_reg(i) ?                               \
                              ((i)->value.r << 1) :                       \
                              (L_is_macro(i) ?                            \
                               (((i)->value.mac << 1) + 1) :              \
                               (L_is_rregister(i) ?                       \
                                ((i)->value.rr << 1) :                    \
                                (L_punt ("L_REG_MAC_INDEX: "              \
                                         "Not Macro or Register"), -1))))

/*
 *      maps reg numbers to numbers with 0 in LSB
 *      1 -> 2, 2 -> 4, 3 -> 6, ...
 */
#define L_REG_INDEX(i)     ((i) << 1)
#define L_IS_MAPPED_REG(i) (!((i)&1))
#define L_UNMAP_REG(i)     ((i) >> 1)

/*
 *      maps macro numbers to number with 1 in LSB
 *      1 -> 3, 2 -> 5, 3 -> 7, ...
 */
#define L_MAC_INDEX(i)     (((i) << 1) + 1)
#define L_IS_MAPPED_MAC(i) ((i)&1)
#define L_UNMAP_MAC(i)     ((i) >> 1)

/*=======================================================================*/
/*
 *      External functions
 */
/*=======================================================================*/

#ifdef __cplusplus
extern "C"
{
#endif

  extern int L_partial_dead_code_removal (L_Func * fn);
  extern void L_clear_partial_dead_code_markings (L_Func * fn);
  extern void L_demote_branches (L_Func * fn);

  extern void L_do_flow_analysis (L_Func *, int);
  extern void L_do_pred_flow_analysis (L_Func *, int);
  extern void L_update_pred_flow_analysis (L_Func *, int);
  extern void L_update_flow_analysis (L_Func *, int);
  extern void L_invalidate_dataflow (void);
  extern void L_dataflow_analysis (int);
  extern void L_setup_dataflow (L_Func * fn);
  extern void L_setup_dataflow_no_operands (L_Func * fn);
  extern void L_delete_dataflow (L_Func *);
  extern void L_add_src_operand_reg (L_Oper *, int, int, int);
  extern void L_add_dest_operand_reg (L_Oper *, int, int, int);

  extern int L_in_cb_IN_set (L_Cb *, L_Operand *);
  extern int L_in_cb_OUT_set (L_Cb *, L_Operand *);
  extern int L_in_cb_IN_set_reg (L_Cb *, int);
  extern int L_in_cb_OUT_set_reg (L_Cb *, int);
  extern void L_add_to_cb_IN_set (L_Cb *cb, int reg);
  extern Set L_get_cb_IN_set (L_Cb *);
  extern Set L_get_cb_OUT_set (L_Cb *);

  extern int L_in_oper_IN_set (L_Oper *, L_Operand *);
  extern int L_in_oper_OUT_set (L_Cb *, L_Oper *, L_Operand *, int);
  extern int L_in_oper_IN_set_reg (L_Oper *, int);
  extern int L_in_oper_OUT_set_reg (L_Cb *, L_Oper *, int, int);
  extern Set L_get_oper_IN_set (L_Oper *);
  extern Set L_get_oper_OUT_set (L_Cb *, L_Oper *, int);

  extern Set L_get_oper_RIN_set (L_Oper * oper);
  extern Set L_get_oper_ROUT_set (L_Oper * oper);
  extern Set L_get_mem_oper_RIN_set (L_Oper * oper);
  extern Set L_get_mem_oper_ROUT_set (L_Oper * oper);
  extern Set L_get_mem_oper_RIN_set_rid (L_Oper * oper);
  extern Set L_get_mem_oper_ROUT_set_rid (L_Oper * oper);
  extern int L_in_oper_RIN_set (L_Oper *, L_Oper *, L_Operand *);
  extern int L_in_oper_RIN_set_reg (L_Oper *, L_Oper *, int);
  extern int L_in_oper_ROUT_set (L_Oper *, L_Oper *, L_Operand *, int);
  extern int L_in_oper_ROUT_set_reg (L_Oper *, L_Oper *, int, int);
  extern Set L_get_oper_RIN_defining_opers (L_Oper *, L_Operand *);
  extern Set L_get_cb_RIN_set (L_Cb *);
  extern int L_in_cb_RIN_set (L_Cb *, L_Oper *, L_Operand *);
  extern int L_in_cb_RIN_set_reg (L_Cb *, L_Oper *, int);
  extern Set L_get_cb_RIN_defining_opers (L_Cb *, L_Operand *);

  extern Set L_get_oper_ROUT_using_opers (L_Oper *, L_Operand *);
  extern Set L_get_cb_ROUT_using_opers (L_Cb *, L_Operand *);

  extern Set L_get_mem_oper_RIN_defining_opers (L_Oper *, int flags);
  extern Set L_get_mem_cb_RIN_defining_opers (L_Cb *, int flags);
  extern Set L_get_mem_oper_ROUT_using_opers (L_Oper *, int flags);
  extern Set L_get_mem_cb_ROUT_using_opers (L_Cb *, int flags);

  extern Set L_get_oper_AIN_set (L_Oper *);
  extern Set L_get_oper_AOUT_set (L_Oper *);
  extern Set L_get_mem_oper_AIN_set (L_Oper *);
  extern Set L_get_mem_oper_AOUT_set (L_Oper *);
  extern Set L_get_mem_oper_AIN_set_rid (L_Oper *);
  extern Set L_get_mem_oper_AOUT_set_rid (L_Oper *);
  extern Set L_get_oper_AIN_set (L_Oper *);
  extern Set L_get_oper_AOUT_set (L_Oper *);
  extern int L_in_oper_AIN_set (L_Oper *, L_Oper *, L_Operand *);
  extern int L_in_oper_AOUT_set (L_Oper *, L_Oper *, L_Operand *, int);
  extern int L_in_cb_AIN_set (L_Cb *, L_Oper *, L_Operand *);
  extern Set L_get_cb_AIN_set (L_Cb *);
  extern Set L_get_oper_AIN_defining_opers (L_Oper * oper,
                                            L_Operand * operand);
  extern Set L_get_cb_AIN_defining_opers (L_Cb * cb, L_Operand * operand);
  extern Set L_get_mem_oper_AIN_defining_opers (L_Oper * oper, int flags);
  extern Set L_get_mem_cb_AIN_defining_opers (L_Cb * cb, int flags);


  extern Set L_get_oper_EIN_set (L_Oper *);
  extern Set L_get_oper_EOUT_set (L_Oper *);
  extern int L_in_oper_EIN_set (L_Oper *, L_Oper *);
  extern int L_in_oper_EOUT_set (L_Oper *, L_Oper *);
  extern int L_in_cb_EIN_set (L_Cb *, L_Oper *);
  extern Set L_get_cb_EIN_set (L_Cb *);
  extern void L_remove_from_oper_EIN_set (L_Oper *, L_Oper *);
  extern void L_remove_from_all_EIN_set (L_Oper *);

  extern int L_PRE_cb_no_changes (L_Cb * cb, Set ignore_set);
  extern int L_PDE_cb_no_changes (L_Cb * cb);
  extern DF_PCE_INFO* L_get_PCE_bb_info (L_Cb * cb, L_Oper * first_op);
  extern Set L_get_PCE_bb_complement_set (L_Cb * cb, L_Oper * first_op);
  extern Set L_get_PCE_bb_nd_safe_set (L_Cb * cb, L_Oper * first_op);
  extern Set L_get_PCE_bb_xd_safe_set (L_Cb * cb, L_Oper * last_op);
  extern Set L_get_PCE_bb_n_insert_set (L_Cb * cb, L_Oper * first_op);
  extern Set L_get_PCE_bb_x_insert_set (L_Cb * cb, L_Oper * last_op);
  extern Set L_get_PCE_bb_n_replace_set (L_Cb * cb, L_Oper * first_op);
  extern Set L_get_PCE_bb_x_replace_set (L_Cb * cb, L_Oper * last_op);
  extern Set L_get_PCE_cb_mem_reaching_definitions (L_Cb * cb);

  extern Set L_get_mem_oper_overwrite_or_dead_set (L_Oper *);

  extern Set L_map_reg_set (Set);
  extern Set L_map_macro_set (Set);
  extern Set L_unmap_reg_set (Set);
  extern Set L_unmap_macro_set (Set);
  extern Set L_unmap_fragile_macro_set (Set);
  extern void L_unmap_rdid (int rdid, int *oper_id, int *operand_id);

  extern void L_print_dataflow (L_Func * fn);

/* Dan Lavery's enhancement.  ITI/MCM 8/17/99 */
  extern void L_remove_from_oper_RIN_set (L_Oper * oper,
                                          L_Oper * reaching_oper,
                                          L_Operand * operand);
  extern void L_remove_from_oper_RIN_set_reg (L_Oper * oper,
                                              L_Oper * reaching_oper,
                                              int reg);

#ifdef __cplusplus
}
#endif

#endif
