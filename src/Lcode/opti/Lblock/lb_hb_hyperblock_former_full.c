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
 *      File :          l_hyperblock_former.c
 *      Description :   Driver for hyperblock formation
 *      Creation Date : February 1998
 *      Authors :       Kevin Crozier
 *        modified from Scott Mahlke's l_hyperblock.c
 *
 *==========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "lb_hb_hyperblock.h"
#include "lb_hb_peel.h"
#include "../../tools/wcet_analysis/wcet_analysis.h"
LB_HB_Stat LB_hb_stat;

#undef DEBUG_HB_FORMER

// Added by {Morteza}:

#include <stdio.h>
#include <string.h>
//#include <./lb_hb_block_enum.c>









static int LB_mark_ncycle_regions(LB_TraceRegion_Header *header) {
    LB_TraceRegion *tr;
    int found = 0;

    List_start(header->traceregions);
    while ((tr = List_next(header->traceregions))) {
        Set tr_set = LB_return_cbs_region_as_set(tr);
        if (LB_hb_region_contains_cycle(tr_set, tr->header)) {
            tr->flags |= L_TRACEREGION_FLAG_NESTED_CYCLE;
            found++;
        }
        Set_dispose(tr_set);
    }

    return found;
}


//full if conversion

void LB_hyperblock_formation_full(L_Func * fn) {

    //fprintf(stderr,"QQQQQQQQQQQQQQQQQQQQQQQ\nwwwwwwwwwwwwwwwwwwwwwwwww\nDDDDDDDDDDDDDDDDDDDDDDDDD\n");

    if (strcmp(fn->name, "_main") == 0) {
        LB_TraceRegion_Header *header;
        LB_TraceRegion *tr;
        L_Cb *cb;
        L_Oper *op;
        int do_peel, final_ops;
        if (fn->n_cb == 0)
            return;
        int i;

        memset(&LB_hb_stat, 0, sizeof (LB_HB_Stat));

        LB_split_exit_block(fn);

        header = LB_function_init(fn);

        L_breakup_pre_post_inc_ops(fn);

        LB_clr_hyperblock_flag(fn);

        L_compute_oper_weight(fn, 0, 1);
        /* make sure all cbs contain at most 2 targets */

        LB_convert_to_strict_basic_block_code(fn, L_CB_SUPERBLOCK |
                L_CB_HYPERBLOCK |
                L_CB_ENTRANCE_BOUNDARY |
                L_CB_EXIT_BOUNDARY);

        for (cb = fn->first_cb; cb; cb = cb->next_cb) {
            L_Oper *next_op;
            for (op = cb->first_op; op; op = next_op) {
                next_op = op->next_op;

                if (op->opc == Lop_NO_OP)
                    L_delete_oper(cb, op);
                else
                    LB_hb_stat.orig_ops++;
            }
        }

        L_do_flow_analysis(fn, DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE);

        L_loop_detection(fn, 0);

        if (LB_hb_verbose_level >= 7) {
            fprintf(stderr, "Initial Loop Detection Phase for (fn %s):\n", fn->name);
            L_print_loop_data(fn);
        }

        LB_elim_all_loop_backedges(fn);

        L_delete_unreachable_blocks(fn);

        L_do_flow_analysis(fn, DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE);
        L_reset_loop_headers(fn);
        L_loop_detection(fn, 0);

        if (LB_hb_verbose_level >= 7) {
            fprintf(stderr, "Final Loop Detection Phase for (fn %s):\n", fn->name);
            L_print_loop_data(fn);
        }

        L_compute_oper_weight(fn, 0, 1);
        LB_mark_jrg_flag(fn);

        //**********************************************************************

        L_partial_dead_code_removal(fn);
        L_do_flow_analysis(fn,
                DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE | SUPPRESS_PG);

        //fprintf(stderr,"Resid 0\n");
        {
            ILP_cfg_wcet_analyize(fn, "(0).txt");
            Ldot_display_cfg(fn, "t0.dot", 0);
            FILE * file = fopen("rrr.txt", "w");
            L_print_func(file, fn);
            fclose(file);
        }

        for (i = 1; i < 100; i++) {

            //	L_do_flow_analysis(fn,
            //				DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE | SUPPRESS_PG);
            //L_partial_dead_code_removal(fn);
            L_do_flow_analysis(fn,
                    DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE | SUPPRESS_PG);
            L_reset_loop_headers(fn);
            L_loop_detection(fn, 0);

            {
                char tp[100];
                sprintf(tp, "t%d.dot", i);
                ILP_cfg_wcet_analyize(fn, "(2).txt");
                Ldot_display_cfg(fn, tp, 0);
                LB_summarize_traceregions(stderr, header);
            }
            //fprintf(stderr,"Resid1\n");
            my_trace_formation3(fn, header);
            //			L_Cb * start= L_cb_hash_tbl_find(fn->cb_hash_tbl,227);
            //			L_Cb * end= L_cb_hash_tbl_find(fn->cb_hash_tbl,240);
            //			Set temp=Set_new();
            //			Set_add(temp,227);
            //			Set_add(temp,229);
            //			Set_add(temp,228);
            ////			Set_add(temp,233);
            ////			Set_add(temp,236);
            ////			Set_add(temp,237);
            ////			Set_add(temp,238);
            ////			Set_add(temp,234);
            ////			Set_add(temp,231);
            ////			Set_add(temp,240);
            //
            //
            //			tr=My_simple_trace_formation(fn,header,L_TRACEREGION_HAMMOCK,start,end,temp,header->next_id++);
            //			header->traceregions=List_insert_last(header->traceregions,tr);

            if (List_size(header->traceregions) == 0)
                break;

            LB_hb_reset_max_oper_id(fn);

            LB_remove_partially_subsumed_traceregions(header);

            LB_summarize_traceregions(stderr, header);

            LB_remove_conflicting_traceregions(header);

            LB_set_hyperblock_flag(fn);

            LB_set_hyperblock_func_flag(fn);

            LB_tail_duplicate(fn, header, LB_DUP_OUTSIDE_REGION);

            {
                L_check_func(fn);
            }

            //Ldot_display_cfg(fn,"GGGGG.dot",0);
            //fprintf(stderr,"Resid1\n");
            LB_predicate_traceregions(fn, header);
            //fprintf(stderr,"Resid2\n");
            /*  Mark all cbs with any type of hyperblock flag with L_CB_HYPERBLOCK */

            //Ldot_display_cfg(fn,"MMMMM.dot",0);

            LB_set_hyperblock_flag(fn);
            LB_remove_unnec_hyperblock_flags(fn);
            LB_set_hyperblock_func_flag(fn);

            LB_free_all_traceregions(header);

            LB_remove_empty_cbs(fn);

            L_delete_unreachable_blocks(fn);

            LB_convert_to_strict_basic_block_code(fn, L_CB_HYPERBLOCK | L_CB_SUPERBLOCK);

            {
                DB_spit_func(fn, "PH1");
                L_check_func(fn);
            }

        }

        //**********************************************************************
        fprintf(stderr, "***************\n**************\n***************\n***************\n");

        {
            //L_partial_dead_code_removal(fn);
            L_do_flow_analysis(fn,
                    DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE | SUPPRESS_PG);
            ILP_cfg_wcet_analyize(fn, "(99).txt");
            Ldot_display_cfg(fn, "t99.dot", 0);

            // LB_summarize_traceregions (stderr, header);
        }
        L_do_flow_analysis(fn,
                DOMINATOR | POST_DOMINATOR | LIVE_VARIABLE | SUPPRESS_PG);

        //*******************************************************************


        //******************************************************************


        /*
         *  Split branches into pred defines and predicated jumps
         */
        if (LB_hb_branch_split)
            LB_branch_split_func(fn);

        /*
         *        Generate multiple defn pred defines, generate Uncond pred defines
         *  (Note initially only OR-type pred defines are created!
         */

        L_create_uncond_pred_defines(fn);

        PG_setup_pred_graph(fn);
        if (LB_do_lightweight_pred_opti)
            L_lightweight_pred_opti(fn);
        L_combine_pred_defines(fn);

        /*
         *        remove unnecessary uncond jumps
         */

        LB_uncond_branch_elim(fn);

        /*
         *        Merge ops on opposite predicates (partial redundancy elim)
         */

        L_unmark_all_pre_post_increments(fn);

        if (LB_hb_do_pred_merge) {
            PG_setup_pred_graph(fn);
            L_do_flow_analysis(fn, LIVE_VARIABLE | REACHING_DEFINITION);
            LB_hb_pred_merging(fn);
        }

#ifdef DEBUG_HB_FORMER
        L_check_func(fn);
#endif

        PG_destroy_pred_graph();
        D_delete_dataflow(fn);

        /* For each source code function we process deinit */
        LB_function_deinit(header);

        {
            double rat = 1.0;
            L_Attr *eattr;

            final_ops = 0;

            for (cb = fn->first_cb; cb; cb = cb->next_cb) {
                L_Oper *next_op;
                for (op = cb->first_op; op; op = next_op) {
                    next_op = op->next_op;

                    if (op->opc != Lop_NO_OP)
                        final_ops++;
                }
            }

            if (LB_hb_stat.orig_ops)
                rat = (double) final_ops / LB_hb_stat.orig_ops;

            eattr = L_new_attr("hbe", 2);
            L_set_int_attr_field(eattr, 0, LB_hb_stat.orig_ops);
            L_set_double_attr_field(eattr, 1, rat);
            fn->attr = L_concat_attr(fn->attr, eattr);
        }

    }

    return;
}

