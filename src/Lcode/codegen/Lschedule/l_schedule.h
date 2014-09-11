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
 *
 *  File:  l_schedule.h
 *
 *  Description: definition file for scheduler.
 *
 *  Creation Date :  May, 1993
 *
 *  Author:  Roger A. Bringmann
 *
 *  Revision 1.2  1995/10/07 22:20:17  mahlke
 *  Added some function prototypes.
 *
 *  Revision 1.1.1.1  1995/08/30 16:49:04  david
 *  Import of IMPACT source
 *
 * Revision 1.4  1994/02/05  22:25:59  roger
 * Final version of speculative yield heuristic
 *
 * Revision 1.3  1994/01/28  04:08:48  roger
 * no entry
 *
 * Revision 1.2  1994/01/26  03:36:39  roger
 * Speculative yield heuristic version 1
 *
 * Revision 1.1  1994/01/19  18:49:31  roger
 * Initial revision
 *
 *
 * 	All rights granted to University of Illinois Board of Regents.
 *
\*****************************************************************************/
#ifndef L_SCHED_H
#define L_SCHED_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>
/* #include <Lcode/r_regproto.h> */
#include "l_dependence.h"
#include "RU_manager.h"
#include "l_ru_interface.h"
#include <machine/lmdes.h>
#include <machine/lmdes_interface.h>

/******************************************************************************\
 *
 *
 *
\******************************************************************************/

#define TRUE  1
#define FALSE 0

/******************************************************************************\
 *
 * Data structures to support scheduling queues.
 *
\******************************************************************************/

typedef struct Squeue
{
    char                name[40];       /* Name of queue */
    int                 size;           /* Number of sinfo in queue */
    struct Sq_entry     *head;          /* First entry in queue */
    struct Sq_entry     *tail;          /* Last entry in queue */
    struct Sq_entry	*current;	/* current entry in queue */
} Squeue;

typedef struct Sq_entry
{
    Squeue              *queue;         /* The queue this entry is in */
    struct Sched_Info	*sinfo;         /* The sinfo this entry holds */
    struct Sq_entry     *next_entry;    /* next sinfo in queue */
    struct Sq_entry     *prev_entry;    /* prev sinfo in queue */
    struct Sq_entry     *next_queue;    /* next queue this sinfo is in */
    struct Sq_entry     *prev_queue;    /* prev queue this sinfo is in */
} Sq_entry;

extern	L_Alloc_Pool	*Squeue_pool;
extern	L_Alloc_Pool	*Sq_entry_pool;

extern Squeue		*scheduled_queue, 
			*priority_ready_queue, 
			*regpres_ready_queue,
			*pending_ready_queue, 
			*not_ready_queue; 

/******************************************************************************\
 *
 * Data structures to support register pressure.
 *
\******************************************************************************/

typedef struct
{
    float	p;
    float	i;
    float	f;
    float	d;
} Regtypes;

typedef struct
{
    float       current_reg;    	/* current total of this type */
    float       threshhold;     	/* Threshhold for heuristic */

    float       caller_reg;     	/* Maximum number of registers of this type */
    float       caller_thresh;     	/* Threshhold for heuristic */
    float       callee_reg;		/* Maximum number of registers of this type */
    float       callee_thresh;   	/* Threshhold for heuristic */
    float       size;           	/* size in bits of register */
    Regtypes	change;
} CB_REG_INFO;

extern CB_REG_INFO     preg_info;
extern CB_REG_INFO     ireg_info;
extern CB_REG_INFO     freg_info;
extern CB_REG_INFO     dreg_info;

extern int              Lsched_use_register_pressure_heuristic;
extern int              Lsched_register_pressure_threshhold;

typedef struct
{
    int		live_in_fall_thru_path;
    int		ctype;
    Squeue	*use_queue;
} Vreg;

extern	Vreg	*virt_reg;

/* Flags used for filling branch delay slots */
#define FILLER		L_OPER_RESERVED_TEMP1
#define FILLED_ABOVE	L_OPER_RESERVED_TEMP2
#define FILLED_BELOW	L_OPER_RESERVED_TEMP3

/******************************************************************************\
 *
 * Schedular data structure
 *
\******************************************************************************/

#define	CAN_SCHEDULE		1
#define	CANT_SCHEDULE		2

/* 
 * This structure contains information used for the scheduler.  It will
 * be placed in the extension field of each oper
 */
typedef struct Sched_Info 
{
    float		sort_key1;		/* queue sort key 1*/
    float		sort_key2;		/* queue sort key 2*/
    float		rsort_key1;		/* regpres sort key 1*/
    float		rsort_key2;		/* regpres sort key 1*/

    L_Oper		*oper;			/* oper being scheduled */
    int			id;			/* oper id */
    int			proc_opc;		/* processor specific opc for oper */
    RU_Info		*ru_info;		/* Resource manager info for oper */
    Dep_Info		*dep_info;		/* dependence info for oper */
    Mdes_Info		*mdes_info;		/* mdes info for oper */
    Sq_entry		*entry_list;		/* linked list of Sq_entries */

    /* Support for safe"r" restricted percolation and wbs */
    int			spec_cond;		/* Condition under which instruction can
						   be speculated */
    int			ctl_dep_level;		/* control dependence level */

    /* 
     * Information used to determine when an oper can be taken off 
     * of the not_ready queue.
     */
    int			num_depend;		/* original num input deps */
    int			*operand_ready_times;	

    /* Information used for dynamic register pressure heuristic */
    float		benefit;
    Regtypes		kill_set;
    Regtypes		def_set;
    int			branch_kill_set_size;
    int			*branch_kill_set;

    /* Information used for static scheduling heuristic */
    float		priority;
    double		weight;			/* oper weight */
    float		instr_prob;		/* Probability that the instr.
						   will be executed. */
    float		taken_instr_prob;	/* Probability that the branch will
						   be taken */ 
    float		not_taken_instr_prob;	/* Probability that the branch will
						   not be taken */ 
    int			index;			/* Index used when computing 
						   static priorities */
    int			on_stack;		/* flag to simplify static priority */

    /* Additional information used for static scheduling heuristic */
    int			etime;			/* earliest time an operation
						   can be scheduled given
						   dependencies */
    int			*ltimes;		/* latest times an operation
						   can be scheduled given
						   dependencies.  this is an
						   array with one entry per
						   exit */

    /* Information used to exactly specify an instr schedule */
    int			scheduled;		/* whether oper is scheduled */
    int         	issue_time;		/* Cycle instr is issued*/
    int			issue_slot;		/* Actual slot scheduled for */
#if 0
no longer needed now that relative latencies are used. BLD 6/95
    int         	completion_time;	/* Cycle instr is complete */
#endif
    int			*relative_latency;	/* BLD 6/95 */
						/* latencies associated with
						   each dest operand.  -1 means
						   there is no corresponding
						   dest operand. */
    int			ready_time;		/* Cycle instr is ready */
    int			earliest_slot;		/* earliest slot if scheduled in
						   the same cycle as an input
						   dependent instruction. */

    /* Candiates for delay slot */
    struct Sched_Info	*delay_sinfo;

    /* Information to support compiler controlled speculative execution */
    int			home_block;
    int			current_block;
    int			orig_block;		/* used to ensure that flow
						 * reordering is done
						 * correctly. */
    L_Oper 		*prev_br;
    L_Oper		*post_br;

    /* Information used to support write-back suppression */
    L_Oper		*extend_lr_down;	/* instr to extend all src operand
    					   	   live ranges down to */
    L_Oper		*check_op;		/* best case check op */


#if TEST_SENTINEL
    /* Information to support sentinel scheduling */
    struct L_Oper	*extend_lr_up;	/* instr to extend specified 
					   source operands live ranges up to. */
    int			extend_src_up;	/* bit array indicating which source 
					   operands are to be extended upwards*/
    int			extend_src_down;/* bit array indicating which source 
					   operands are to be extended 
					   downwards */
    struct L_Oper	*demote_pt;	/* instr must move immediatedly
					   below to make non-speculative */
    int			protected;
    struct L_Oper	*latest_check_op;	/* worst case check op */
    Set			src_regs;		/* src registers used in 
						   PEI-sentinel chain */
#endif 
    L_Oper_List  *store_list;    	/* list of stores potentially bypassed*/

} Sched_Info;

/******************************************************************************\
 *
 * Function declarations
 *
\******************************************************************************/

#define SCHED_INFO(oper) ((Sched_Info *)((oper)->ext))

extern int Lsched_is_branch(int);

#define IS_UCOND_BRANCH(opc)	op_flag_set(opc, OP_FLAG_JMP)
#define IS_COND_BRANCH(opc) 	op_flag_set(opc, OP_FLAG_CBR)
#define IS_BRANCH(opc)		op_flag_set(opc, OP_FLAG_CBR|OP_FLAG_JMP)
#define IS_CTL(opc) 		op_flag_set(opc, OP_FLAG_CBR|OP_FLAG_JMP|OP_FLAG_RTS|OP_FLAG_JSR)
#define IS_JSR(opc)		op_flag_set(opc, OP_FLAG_JSR)


#define IS_MEMORY_LOAD(opc) 	op_flag_set(opc, OP_FLAG_LOAD)
#define IS_MEMORY_STORE(opc) 	op_flag_set(opc, OP_FLAG_STORE)
#define IS_MEMORY_OP(opc)	op_flag_set(opc, OP_FLAG_STORE|OP_FLAG_LOAD)


/* Right now we assume there is only one delay slot! */
#define HAS_DELAY_SLOT(opc)	op_flag_set(opc, OP_FLAG_NI)
#define IS_IGNORE(opc)		op_flag_set(opc, OP_FLAG_IGNORE)
#define IS_EXCEPTING(opc)	op_flag_set(opc, OP_FLAG_EXCEPT)

extern char *Lsched_model_name;

#define BASIC_BLOCK		0
#define RESTRICTED		1
#define GENERAL		 	2
#define WBS			3
#define WRITEBACK_SUPPRESSION	3
#define SENTINEL		4
#define	BOOSTING		5
#define MCB		 	6

extern int Lsched_model;

extern int Lsched_processor_model;	/* scalar, superscalar, vliw */
extern int Lsched_total_issue_slots;

extern int Lsched_debug_prepass_scheduling;
extern int Lsched_debug_postpass_scheduling;
extern int Lsched_debug_messages;
extern int Lsched_do_postpass_scheduling;

extern char *Lsched_profile_info_to_use;

extern int Lsched_use_fan_out;
extern int Lsched_infinite_issue;
extern int Lsched_debug_squashing_branches;
extern int Lsched_debug_nonsquashing_branches;
extern int Lsched_debug_unfilled_branches;
extern int Lsched_do_fill_squashing_branches;
extern int Lsched_do_fill_nonsquashing_branches;
extern int Lsched_do_fill_unfilled_branches;
extern int Lsched_fill_delay_slots;
extern int Lsched_include_only_oper_lt_hb;
extern int Lsched_print_statistics;
extern int Lsched_print_prepass_statistics;
extern int Lsched_demote_all_the_way;
extern int Lsched_do_sentinel_recovery;
extern int Lsched_prepass;
extern struct L_Oper *Lsched_latest_br;
extern int Lsched_use_register_pressure_heuristic;
extern int Lsched_register_pressure_threshhold;
extern int Lsched_regpres_heuristic;
extern int Lsched_num_branches_per_cycle;

extern L_Alloc_Pool *Operand_ready_pool;
extern L_Alloc_Pool *Sched_Info_pool;

/* used to print out statistics while scheduling */
#define EXTERN_LOG_FILE           "IMPACT_001"
#define EXTERN_LOG_FILE_PREPASS   "IMPACT_001_PREPASS"

extern int	Lsched_loads_per_cycle(void);
extern int	Lsched_stores_per_cycle(void);
extern int	Lsched_branches_per_cycle(void);
extern int	Lsched_issue_time(L_Oper *);
extern int	Lsched_completion_time(L_Oper *);
extern int	Lsched_latency(L_Oper *);

extern void 	Lsched_prepass_code_scheduling(L_Func *);
extern void 	Lsched_postpass_code_scheduling(L_Func *);
extern int  	Lsched_update_ctl_delay_sinfo(Squeue*, Sched_Info*);
extern void	Lsched_check_for_fill_candidates(Squeue*);
extern void	Lsched_schedule_with_delay_op(L_Cb*, Sched_Info*, int, int, int, int);
extern void 	Lsched_fill_nonsquashing_branches(L_Func *);
extern void 	Lsched_fill_squashing_branches(L_Func *);
extern void 	Lsched_fill_unfilled_branches(L_Func *);
extern void 	Lsched_init();
extern int 	Lsched_can_schedule_op(L_Cb *, Sched_Info*, int, int, int*, int*, int*);
extern void 	Lsched_schedule_op(L_Cb *, Sched_Info*, int, int);
extern void 	Lsched_make_ready(L_Cb*, int, Sched_Info*, Sched_Info*, int, 
int, int);
extern void	L_delete_sched_info (Sched_Info*);

extern void     Lsched_add_isl_attr (L_Func *fn);
extern void     Lsched_delete_isl_attr (L_Func *fn);

extern void 	wbs_remove_check(); /* (cb, dep_info, current_time) */
extern void 	wbs_insert_check();	/* (cb, build_iolist_func) */
extern void 	wbs_extend_live_range(); /* (cb) */


extern void Lsched_schedule_block(L_Cb *cb);




extern Sched_Info* L_create_sched_info (L_Oper *oper, int home_block, L_Oper *prev_br,
                                 L_Oper *post_br);

extern void L_mcb_remove_check (L_Cb *cb, L_Oper *oper, int ready_time);


extern Squeue*		L_create_queue (char *, int);
extern void 		L_delete_queue(Squeue *queue);
extern void		L_reset_queue_current (Squeue *);
extern Sched_Info*	L_get_queue_next_entry (Squeue *);
extern Sched_Info*	L_get_queue_head (Squeue *);
extern int		L_get_queue_size (Squeue *);
extern void		L_enqueue (Squeue *, Sched_Info *);
extern void		L_dequeue (Squeue *, Sched_Info *);
extern void		L_dequeue_from_all (Sched_Info *);
extern void		L_enqueue_min_to_max_1 (Squeue*, Sched_Info*, float);
extern void		L_enqueue_max_to_min_1 (Squeue*, Sched_Info*, float);
extern void		L_enqueue_min_to_max_2 (Squeue*, Sched_Info*, float, float);
extern void		L_enqueue_max_to_min_2 (Squeue*, Sched_Info*, float, float);
extern void L_enqueue_regpres (Squeue *queue, Sched_Info *sinfo, float key1, float key2);
extern int		L_in_queue (Squeue*, Sched_Info*);

extern void		Lregpres_init_func (L_Func *);
extern void		Lregpres_deinit_func (L_Func *);
extern void		Lregpres_init_cb (L_Cb *);

extern void		L_init_ready_queue();
extern Sched_Info*	L_get_next_entry();
extern void		L_insert_entry(Sched_Info*);
extern void		L_update_virt_reg(Sched_Info*);

extern double		L_approx_cb_time(L_Cb *cb);

extern L_Cb* L_copy_all_oper_in_cb (L_Func *fn, L_Cb *cb);
extern void Lsched_mcb_prepass_schedule_block(L_Func *fn, L_Cb *cb, 
					L_Cb *nomcb_cb);

extern void Lsched_mcb_schedule_block(L_Func *fn, L_Cb *cb, L_Cb *nomcb_cb);

extern int Lsched_handle_spilled_branch(L_Cb *cb, Sched_Info *sinfo);

extern void L_print_schedule(char *F, L_Func *fn);


#endif
