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
 *      File :          l_loop.h
 *      Description :   Lcode loop data structures
 *      Creation Date : July, 1990
 *      Author :        Scott Mahlke, Pohua Chang, Wen-mei Hwu
 *
 *==========================================================================*/

#ifndef L_LOOP_H
#define L_LOOP_H

/* 10/29/02 REK Adding config.h */
#include <config.h>

struct L_Func;
struct L_Cb;
struct L_Operand;
struct L_Oper;

/*==========================================================================*/
/*
 *      General loop data structure
 */
/*==========================================================================*/

/*
 * The L_Loop structure represents only proper (single entry) loops
 */

typedef struct L_Loop
{
  int id;
  unsigned int flags;
  struct L_Cb *preheader;
  struct L_Cb *header;
  double num_invocation;
  int nesting_level;

  Set loop_cb;                    /* CBs within the loop                    */
  Set back_edge_cb;               /* source CBs of loop backedge flows      */
  Set exit_cb;                    /* source CBs of loop exit flows          */
  Set out_cb;                     /* destination CBs of loop exit flows     */

  Set nested_loops;

  Set basic_ind_var;
  Set basic_ind_var_op;
  struct L_Ind_Info *ind_info;

  struct L_Loop *prev_loop;
  struct L_Loop *next_loop;
  struct L_Loop *parent_loop;
  struct L_Loop *sibling_loop;
  struct L_Loop *child_loop;
} L_Loop;

/*==========================================================================*/
/*
 *      General loop external functions
 */
/*==========================================================================*/

#ifdef __cplusplus
extern "C"
{
#endif

  extern L_Loop *L_new_loop (struct L_Cb *);
  extern void L_delete_loop (struct L_Func *, L_Loop *);
  extern void L_delete_all_loop (L_Loop *);
  extern L_Loop *L_concat_loop (L_Loop *, L_Loop *);
  extern void L_reset_loop_headers (struct L_Func *);
  extern void L_print_loop_data (struct L_Func *);
  extern void L_print_specific_loop_data (struct L_Loop *);
  extern void L_print_loop_data_only_innermost (struct L_Func *);
  extern void L_single_par_nesting (struct L_Func *);
  extern void L_loop_detection (struct L_Func *, int);
  extern void L_loop_detection_nomerge (struct L_Func *, int);
  extern void L_find_basic_ind_var (struct L_Func *);
  extern int L_is_loop_inv_operand (struct L_Loop *, int *, int,
                                    struct L_Operand *);
  extern int L_unique_def_in_loop (struct L_Loop *, int *, int,
                                   struct L_Oper *);
  extern void L_find_all_ind_var_op (struct L_Loop *, int *, int);
  extern void L_move_basic_ind_var_to_src1 (struct L_Loop *, int *, int);
  extern void L_merge_two_loops (struct L_Func *, struct L_Loop *,
                                 struct L_Loop *);

#ifdef __cplusplus
}
#endif

/*==========================================================================*/
/*
 *      Inner loop (Superblock loops) data struct
 */
/*==========================================================================*/

typedef struct L_Inner_Loop
{
  int id;
  unsigned int flags;
  struct L_Cb *preheader;
  struct L_Cb *cb;
  struct L_Cb *fall_thru;
  struct L_Oper *feedback_op;
  double num_invocation;
  double weight;
  double ave_iteration;
  Set out_cb;
  Set basic_ind_var;
  Set basic_ind_var_op;
  struct L_Ind_Info *ind_info;
  struct L_Inner_Loop *prev_inner_loop;
  struct L_Inner_Loop *next_inner_loop;
}
L_Inner_Loop;

/*==========================================================================*/
/*
 *      Inner loop (Superblock loops) external functions
 */
/*==========================================================================*/

#ifdef __cplusplus
extern "C"
{
#endif

  extern L_Inner_Loop *L_new_inner_loop (struct L_Cb *);
  extern void L_delete_inner_loop (struct L_Func *, L_Inner_Loop *);
  extern void L_delete_all_inner_loop (L_Inner_Loop *);
  extern L_Inner_Loop *L_concat_inner_loop (L_Inner_Loop *, L_Inner_Loop *);
  extern void L_print_inner_loop_data (struct L_Func *);
  extern void L_inner_loop_detection (struct L_Func *, int);
  extern void L_add_loop_structure_for_new_cb (struct L_Cb *new_cb,
                                               struct L_Cb *orig_cb);
  extern int L_is_inner_loop_ind_op (struct L_Cb *, struct L_Oper *);
  extern struct L_Oper *L_find_inner_loop_counter (struct L_Inner_Loop *);
  extern int L_is_counted_inner_loop (struct L_Inner_Loop *);

#ifdef __cplusplus
}
#endif

/*==========================================================================*/
/*
 *      Loop induction variable information data struct
 */
/*==========================================================================*/

typedef struct L_Ind_Info
{
  int valid_ind_var;
  struct L_Operand *var;
  struct L_Operand *increment;
  int valid_init_val;
  int coeff;
  struct L_Operand *base;
  struct L_Oper *initop; /* Initializing operation */
  int offset;
  struct L_Ind_Info *prev_info;
  struct L_Ind_Info *next_info;
}
L_Ind_Info;

/*==========================================================================*/
/*
 *      Loop induction variable information external functions
 */
/*==========================================================================*/

#ifdef __cplusplus
extern "C"
{
#endif

  extern L_Ind_Info *L_new_ind_info (struct L_Operand *, int);
  extern void L_delete_ind_info (L_Ind_Info *, L_Ind_Info **);
  extern void L_delete_all_invalid_ind_var (L_Ind_Info *);
  extern void L_delete_all_ind_info (L_Ind_Info **);
  extern L_Ind_Info *L_concat_ind_info (L_Ind_Info *, L_Ind_Info *);
  extern L_Ind_Info *L_find_ind_info (L_Ind_Info *, struct L_Operand *, int);
  extern void L_invalidate_ind_var (struct L_Operand *, L_Ind_Info *);
  extern void L_invalidate_initial_val (struct L_Operand *, L_Ind_Info *);
  extern void L_print_all_ind_info (struct L_Loop *, L_Ind_Info *);
  extern void L_find_initial_val_for_cb (struct L_Cb *, L_Ind_Info **);
  extern void L_find_all_ind_info (L_Loop *, int *, int);
  extern struct L_Operand *L_find_basic_induction_increment (struct L_Operand
                                                             *, L_Ind_Info *);
/* BCC - 2/8/99 */
  extern int L_is_oper_in_loop (L_Loop *, int *, int, struct L_Oper *);
  extern int L_is_mem_op_address_invariant_in_loop (L_Loop *, int *, int,
                                                    struct L_Oper *, char *);

#ifdef __cplusplus
}
#endif

/*==========================================================================*/
/*
 *      Static weight external functions
 */
/*==========================================================================*/

#define L_STATIC_LOOP_ITER      10.0

#ifdef __cplusplus
extern "C"
{
#endif

  extern void L_compute_static_cb_weight (struct L_Func *);

#ifdef __cplusplus
}
#endif

/* Iteration info attributes for loop profiling */

#define L_ITER_INFO_HEADER              "iteration_header"
#define L_ITER_PREFIX                   "iter_"
#define L_ITER_PREFIX_LENGTH            5

#endif



