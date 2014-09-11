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
 *      File :          l_sync.h
 *      Description :   Lcode sync info
 *      Creation Date : July, 1994
 *      Author :        Dave Gallagher, Dan Connors, Wen-mei Hwu
 *
 *==========================================================================*/
#ifndef L_SYNC_H
#define L_SYNC_H

/* 10/29/02 REK Adding config.h */
#include <config.h>

/*==========================================================================*/
/*
 *      General 
 */
/*==========================================================================*/

#define nonloop_carried         (int) 0x00000001
#define inner_carried           (int) 0x00000002
#define outer_carried           (int) 0x00000004
#define inner_serloop           (int) 0x00000008
#define distance_unknown        (int) 0x00000010
#define profile_conflict        (int) 0x00000020

/* BCC - added always, frequent, sometimes, 
   infrequent to be complete - 2/2/99 */
#define always_sync             (int) 0x00000000
#define frequent_sync           (int) 0x00002000
#define sometimes_sync          (int) 0x00004000
#define infrequent_sync         (int) 0x00006000
#define definite_sync           (int) 0x00008000
#define profile_sync            (int) 0x00000800

#define SET_OUTER_CARRIED(flg)      ( (flg) | outer_carried)
#define SET_INNER_CARRIED(flg)      ( (flg) | inner_carried)
#define SET_NONLOOP_CARRIED(flg)    ( (flg) | nonloop_carried)
#define SET_INNER_SERLOOP(flg)      ( (flg) | inner_serloop)
#define SET_DISTANCE_UNKNOWN(flg)   ( (flg) | distance_unknown)
#define SET_PROFILE_CONFLICT(flg)   ( (flg) | profile_conflict)

#define SET_DEFINITE_SYNC(flg)      ( (flg) | definite_sync)
#define SET_PROFILE_SYNC(flg)       ( (flg) | profile_sync)

#define IS_OUTER_CARRIED(flg)      ( (flg) & outer_carried)
#define IS_INNER_CARRIED(flg)      ( (flg) & inner_carried)
#define IS_NONLOOP_CARRIED(flg)    ( (flg) & nonloop_carried)
#define IS_INNER_SERLOOP(flg)      ( (flg) & inner_serloop)
#define IS_DISTANCE_UNKNOWN(flg)   ( (flg) & distance_unknown)
#define IS_PROFILE_CONFLICT(flg)   ( (flg) & profile_conflict)

#define IS_DEFINITE_SYNC(flg)      ( (flg) & definite_sync)
#define IS_PROFILE_SYNC(flg)       ( (flg) & profile_sync)

#define SYNC_INFO(oper) ((L_Sync_Info *)(oper->sync_info))

/*
 *      Function Prototypes 
 */
/*==========================================================================*/

#ifdef __cplusplus
extern "C"
{
#endif

  extern void L_create_child_list (L_Func * fn);
  extern void L_delete_child_list (void);
  extern void L_add_to_child_list (L_Oper * parent_oper, L_Oper * child_oper);

  extern void L_relink_child_sync_arcs (L_Func * fn);
  extern int L_difference_in_nesting_level (L_Oper * oper,
                                            int loop_nesting_level);
  extern int L_analyze_syncs (L_Oper * op1, L_Oper * op2, int dep_flags);
  extern int L_analyze_syncs_for_independence (L_Oper * op1, L_Oper * op2,
                                               int dep_flags);
  extern int L_analyze_syncs_cross (L_Oper * from_oper, L_Oper * to_oper, 
				    int dep_flags, int forward, int *distance);
  extern int L_analyze_syncs_for_cross_iter_independence (L_Oper * from_oper,
                                                          L_Oper * to_oper,
                                                          int dep_flags,
                                                          int forward,
                                                          int *distance);
  extern void L_adjust_syncs_for_target_expansion (L_Oper * first_oper,
                                                   L_Cb * orig_cb,
                                                   L_Cb * new_cb);
  extern void L_adjust_syncs_for_movement_out_of_loop (L_Oper * oper,
                                                       L_Cb * new_cb);
  extern void L_make_sync_arcs_conservative (L_Oper * oper);
  extern void L_update_sync_arcs_for_new_cb (L_Cb * old_cb, L_Cb * new_cb,
                                             L_Oper * oper);
  extern void L_add_specific_sync_between_opers (L_Oper * oper1, 
						 L_Oper * oper2,
						 short info, char dist, 
						 char prof_info);
  extern void L_add_sync_between_opers (L_Oper * oper1, L_Oper * oper2);
  extern void L_add_maybe_sync_between_opers (L_Oper * oper1, L_Oper * oper2);
  extern void L_add_maybe_backwards_sync_between_opers (L_Oper * oper1,
                                                        L_Oper * oper2);
  extern int L_address_varies_between_iterations (L_Oper * oper);
  extern void L_add_sync_between_unrolled_opers (L_Oper * oper1,
                                                 L_Oper * oper2);
  extern int L_find_unroll_iter (L_Oper * oper);
  extern int L_oper_is_in_cb (L_Cb * cb, L_Oper * oper);
  extern void L_adjust_syncs_after_unrolling (L_Cb * cb, int num_unroll);

  extern int L_sync_no_jsr_dependence (L_Oper * jsr, L_Oper * oper);
  extern void L_adjust_syncs_for_remainder (L_Cb * header_cb,
                                            L_Cb * remainder_cb);
  extern void L_adjust_invalid_sync_arcs_in_cb (L_Cb * cb);
  extern void L_adjust_invalid_sync_arcs_in_func (L_Func * fn);

  extern void L_union_sync_in_arc_info (L_Oper * opA, L_Oper * opB);
  extern void L_union_sync_out_arc_info (L_Oper * opA, L_Oper * opB);
  extern void L_union_sync_arc_info (L_Oper * opA, L_Oper * opB);

  extern void L_init_child_list (L_Func * fn);
  extern void L_deinit_child_list (L_Func * fn);



/*
 *      SAM, 4-98.  See l_sync.c for explanation of how this works
 */
#define L_SYNC_TYPE_NONE                0x0
#define L_SYNC_TYPE_LS_LOCAL            0x1
#define L_SYNC_TYPE_LS_GLOBAL           0x2
#define L_SYNC_TYPE_JSR_LOCAL           0x10
#define L_SYNC_TYPE_JSR_GLOBAL          0x20
  extern 
  void L_build_sync_arcs_from_lcode_disamb (L_Func * fn, int sync_type,
                                            int insert_consv_cross_iter_arcs);
  void L_adjust_incoming_sync_arc_for_backedge_movement (L_Oper * op,
                                                         L_Sync * sync);
  void L_adjust_outgoing_sync_arc_for_backedge_movement (L_Oper * op,
                                                         L_Sync * sync);

#ifdef __cplusplus
}
#endif

#endif
