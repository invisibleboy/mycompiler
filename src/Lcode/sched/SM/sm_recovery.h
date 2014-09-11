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
/*****************************************************************************\
 *      File:   sm_recovery.h
 *      Author: Erik Nystrom
 *      Creation Date:  March 2001
\*****************************************************************************/

#ifndef SM_RECOVERY_H
#define SM_RECOVERY_H

#include <config.h>

#define RC_MAX_CHK 8

typedef struct RC_dep_info
{
  L_Oper *from_op;
  L_Oper *to_op;
  L_Operand *oprd;
}
RC_dep_info;

typedef struct RC_cb_info
{
  Set  valid_cbs;
  List chk_history;
  int  val_history;
}
RC_cb_info;

#define RC_CB_INFO(s)   ((RC_cb_info*)((s)->ext))

typedef struct RC_flow_info
{
  /* Home cb */
  L_Cb *cb;

  /* Ops on which this op is flow dependent */
  List def_op_list;
  /* Ops flow dependent on this op */
  List use_op_list;

  /* Op to substitute for oper in RC */
  int skip_op;
  List add_before;
  List add_after;
  
  
#if 1
  /* Ops on which this oprd is anti-dep   */
  List anti_def_op_list;
  /* Ops anit-dep on this oprd */
  List anti_use_op_list;
#endif

  /* 
   * This info is on Speculative Loads Only 
   *****************************************/
  /* Orig checks that match spec load */
  L_Oper **chk_ops;

  List chk_list;

  /* If a load became speculative purely 
     because it moved across a check, then
     this points to the op the check must 
     follow */
  L_Oper *chk_mark;


  short chk_num;
}
RC_flow_info;


#define RC_FLOW_INFO(s)   ((RC_flow_info*)((s)->ext))

typedef enum
{ rc_notrc, rc_isrc, rc_total, rc_last }
rc_kind;


/*
 * Misc functions
 */
List RC_oprd_remove_from_list (List oprd_list, L_Operand * oprd);
int RC_oprd_in_list (List oprd_list, L_Operand * oprd);
int RC_check_preserves_oprd (L_Oper * chk_op, L_Operand * oprd);
void RC_add_flow_for_op (L_Cb * cb, L_Oper * op);
void RC_move_dest_flow_after (L_Cb * from_cb, L_Flow * dst_flow,
                              L_Cb * to_cb, L_Flow * to_after_flow);
L_Flow *RC_move_op_after (L_Cb * from_cb, L_Oper * op,
                          L_Cb * to_cb, L_Oper * to_after_op);
L_Cb *RC_split_cb_after (L_Func * fn, L_Cb * cb, L_Oper * op);
L_Oper *RC_copy_op (L_Oper * op, List * oprd_list);
void RC_dump_lcode (L_Func * fn, char *ext);
void RC_dump_lcode_cb (L_Func * fn, L_Cb * cb, char *ext);
void RC_delete_all_checks (L_Func * fn);
void RC_insert_antidep_defines (L_Func * fn);
void RC_delete_antidep_defines (L_Func * fn);


/*
 * Recovery code info management
 */
RC_flow_info *RC_new_flow_info ();
void RC_delete_flow_info (L_Oper * op);
void RC_init (L_Func * fn);
void RC_cleanup (L_Func * fn);
void RC_gather_stats (L_Func * fn, List lds_list);


/*
 * Dataflow interface
 */
void RC_free_dep (void *info);
RC_dep_info *RC_new_dep (L_Oper * from_op, L_Oper * to_op, L_Operand * oprd);
void RC_add_flow_dep (L_Oper * def_op, L_Oper * use_op, L_Operand * oprd);
void RC_add_anti_dep (L_Oper * def_op, L_Oper * use_op, L_Operand * oprd);
void RC_reset_dataflow (L_Func * fn);
int RC_anti_dep_skip (L_Oper * op);
void RC_generate_anti_deps_for_ops (L_Oper * use_op, L_Oper * def_op);
void RC_generate_global_flow_deps (L_Func * fn);


/*
 * Functions for handling anti-dep issues
 */
void RC_generate_anti_deps_for_ops (L_Oper * use_op, L_Oper * def_op);
void RC_generate_anti_deps (L_Func * fn, List lds_list);
void RC_fix_antideps (L_Func * fn, List lds_list, int check_only);


/*
 * Recovery code builders
 */
void RC_jump_opti (L_Func *fn, List *cb_list, L_Cb *end_cb);

void
RC_cbs_reachable_from (L_Func * fn, 
		       L_Cb * start_cb, L_Oper * start_op,
		       List chk_op_list, List exclude_chk_op_list, 
		       List *unrch_check_cb_list, 
		       Set *vop_set, List *ctrl_list, List *cb_list);

void RC_split_around_checks (L_Func * fn, int sched);
void RC_add_rc_return_jumps (L_Func * fn, int sched);
int RC_valid_dependence (L_Cb * cur_cb, L_Oper * cur_op,
                         L_Cb * dep_cb, L_Oper * dep_op,
                         L_Cb * lds_cb, L_Oper * lds_op,
                         L_Cb * chk_cb, L_Oper * chk_op);
List RC_find_spec_load_and_checks (L_Func * fn);
List RC_is_load_spec_above_check (L_Func *fn, List lds_list, L_Oper * cur_op,
                                  L_Oper * chk_op);
void RC_make_load_speculative (L_Func * fn, L_Oper * lds_op);
void RC_get_ctrl_for_rc (List * op_list, List cb_list,
                         L_Cb * lds_cb, L_Oper * lds_op);
void RC_get_ops_for_rc (L_Func *fn, List * op_list, List * dep_ctrl_list,
			List * rc_list, List * lds_list,
                        List * last_op_list, Set vop_set, List cb_list,
                        L_Cb * lds_cb, L_Oper * lds_op,
                        L_Cb * chk_cb, L_Oper * chk_op);
void RC_get_prsv_for_rc (List rc_list, List * psrv_list);
int RC_get_anti_ops_for_rc (List cb_list, Set vop_set,
                            List * rc_list, List psrv_list,
                            List * conf_list, List * svrt_list,
                            L_Cb * lds_cb, L_Oper * lds_op,
                            L_Cb * chk_cb, L_Oper * chk_op);
void RC_get_dests_for_rc (List rc_list, List * dest_list);
int RC_get_def_ops_for_rc (List cb_list, List dest_list,
			   List * rc_list,
			   L_Cb * lds_cb, L_Oper * lds_op,
			   L_Cb * chk_cb, L_Oper * chk_op);
void RC_build_rc_cbs (L_Func * fn, List cb_list, List rc_list,
                      List * new_cb_list, List * new_op_list,
                      L_Cb ** start_cb, L_Cb ** end_cb,
                      /*List * conf_list, List * svrt_list,*/
                      L_Cb * lds_cb, L_Oper * lds_op,
                      L_Cb * chk_cb, L_Oper * chk_op);
void RC_fix_anti_deps (List conf_list, List svrt_list, List * psrv_list,
                       L_Cb * start_cb, L_Cb * end_cb,
                       L_Cb * lds_cb, L_Oper * lds_op,
                       L_Cb * chk_cb, L_Oper * chk_op);
void RC_add_check_targets (L_Cb * start_cb, L_Cb * end_cb,
                           List last_op_list, L_Cb * chk_cb, L_Oper * chk_op);
void RC_create_recovery_code (L_Func * fn, List * lds_list);



/*
 * Master functions
 */
void L_read_parm_RC (Parm_Parse_Info * ppi);
void RC_generate_recovery_code (L_Func * fn);
void RC_recombine_cbs (L_Func * fn);
void RC_fix_recovery_code_bundles (L_Func * fn);



#if 0

/*
 * Recovery code Scheduling
 */
void RC_schedule_recovery_code (L_Func * fn, List lds_list);

/*
 * Recovery code register preservation
 */
void RC_setup_preservation_analysis (L_Func * fn, List lds_list);
void RC_find_preserved_regs (L_Func * fn, List lds_list);
void RC_add_preserved_regs_to_check (L_Func * fn, List lds_list);
void RC_verify_preserved_regs_on_check (L_Func * fn, List lds_list);
void RC_cleanup_preservation (L_Func * fn, List lds_list, int check_only);
#endif

#endif
