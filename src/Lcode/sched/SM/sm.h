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
 *      File:   sm.h
 *      Author: John C. Gyllenhaal, Wen-mei Hwu
 *      Additions, Modifications: David August
 *      Modulo Scheduling support: IMPACT Technologies (John Gyllenhaal)
 *      Creation Date:  March 1996
\*****************************************************************************/

#ifndef SM_H
#define SM_H

#include <config.h>
#include <Lcode/l_main.h>
#include <library/md.h>
#include <machine/mdes2.h>
#include <machine/sm_mdes.h>
/* 20020823 SZU */
#include <library/set.h>
/* 20030602 SZU */
#include <Lcode/l_code.h>
#include <machine/m_tahoe.h>

/* 20021210 SZU 
 * Used to debug producer/consumer pair latencies
 */
#define DEBUG_PCLAT 0

/********** Flags for SM_Reg_Actions (bit field flags) ***********/

/* Indicates whether action is for a register definition or usages.
 *
 * Every action must have either SM_DEF_ACTION or SM_USE_ACTION set!
 */
#define SM_DEF_ACTION           0x00000001
#define SM_USE_ACTION           0x00000002

/* Indicates whether this action is actually for this register, or if
 * this action is for another register that conflicts (overlaps) with this
 * action.
 *
 * Every action must have either SM_ACTUAL_ACTION or SM_CONFLICTING_ACTION set!
 */
#define SM_ACTUAL_ACTION        0x00000004
#define SM_CONFLICTING_ACTION   0x00000008

/* Indicates type of predicate definition (if a predicate register) */

/* JWS 19991229 Replace SM_PRED_*_DEF with two flags:
 * SM_PRED_UNCOND_DEF -> The predicate value may be modified regardless
 *                       of the guard (incoming) predicate value.
 * SM_PRED_TRANS_DEF  -> The previous value of the destination is significant
 *                       even when the guard (incoming) predicate is true.
 */

#define SM_PRED_UNCOND_DEF      0x00000010
#define SM_PRED_TRANS_DEF       0x00000020

/* Indicates type of predicate use (if a predicate register) */
#define SM_PRED_UNCOND_USE      0x00000100      /* Currently pred[0] */
#define SM_PRED_COND_USE        0x00000200      /* Currently all other uses */


/* Indicates that an action is explicit (normal) or implicit. 
 * 
 * Implicit actions are currently used only for modeling fragile macro
 * usage for jsrs, and live out registers for branches
 */
#define SM_EXPLICIT_ACTION      0x00000400
#define SM_IMPLICIT_ACTION      0x00000800

/* Indicates that an action can occur at different times, depending
 * on which scheduling alternative is checked.  These actions will
 * lead to dependences with variable delays.
 */
#define SM_VARIABLE_ACTION      0x00001000

/* Indicates that an def action should be treated as "possible" but
 * not guaranteed to actually occur.  Dataflow calls these definitions
 * "transparent", so I will too. :)  Basically, these definitions will
 * not kill another definition.
 */
#define SM_TRANSPARENT_ACTION   0x00002000

/* A use action is exposed out the top of the block */
#define SM_LIVEIN_ACTION        0x00004000

/********* End flags for SM_Reg_Actions (bit field flags) **********/

/********* Start flags for SM_Reg_Info (bit field flags) *********/
#define SM_FRAGILE_MACRO        0x00000001
#define SM_REG_LIVE_IN          0x00000002
/********* End flags for SM_Reg_Info (bit field flags) **********/

/********* Start flags for SM_Dep (bit field flags) *********/
#define SM_FIXED_DELAY                  0x00000001
#define SM_MDES_BASED_DELAY             0x00000002
#define SM_VARIABLE_DELAY               0x00000004

/* A dependence can be only be one of hard or soft. */
#define SM_HARD_DEP                     0x00000008
#define SM_SOFT_DEP                     0x00000010

#define SM_REG_DEP                      0x00000040
#define SM_MEM_DEP                      0x00000080
#define SM_CTRL_DEP                     0x00000100
#define SM_SYNC_DEP                     0x00000200
#define SM_VLIW_DEP                     0x00000400

#define SM_FLOW_DEP                     0x00000800
#define SM_ANTI_DEP                     0x00001000
#define SM_OUTPUT_DEP                   0x00002000

#define SM_BUILD_DEP_IN                 0x00004000
#define SM_BUILD_DEP_OUT                0x00008000
#define SM_PREVENT_REDUNDANT_DEPS       0x00010000

#define SM_USER_DEP_MARK1               0x00020000
#define SM_USER_DEP_MARK2               0x00040000
#define SM_USER_DEP_MARK3               0x00080000
#define SM_USER_DEP_MARK4               0x00100000

#define SM_SOFTFIX_PROMOTION            0x01000000
#define SM_FORCE_DEP                    0x02000000

/* For breakable soft deps */
#define SOFTFIX_POSSIBLE       0
#define SOFTFIX_COMMIT         1
#define SOFTFIX_UNDO           2

/********* End flags for SM_Dep (bit field flags) *********/

/********* Start flags for SM_Oper (bit field flags) *********/
#define SM_HAS_SILENT_VERSION   0x00000001

#define SM_OP_SCHEDULED         0x00000002
#define SM_OP_SPECULATIVE       0x00000004
#define SM_OP_SILENT            0x00000008

#define SM_OP_TESTED            0x00000010
#define SM_RWC_UNBENEFICIAL     0x00000020
#define SM_EWC_UNBENEFICIAL     0x00000040

/* Flags that operation has actions with variable use times, thus
 * dependences need special checking during scheduling.
 */
#define SM_OP_VARIABLE_ACTIONS  0x00000080

#define SM_USER_OP_MARK1        0x00000200
#define SM_USER_OP_MARK2        0x00000400
#define SM_USER_OP_MARK3        0x00000800
#define SM_USER_OP_MARK4        0x00001000

/* 20021219 SZU
 * Indicate SM_Oper is the loop_back branch of modulo scheduled loop
 */
#define SM_LOOP_BACK_BR		0x00002000
/********* End flags for SM_Oper (bit field flags) *********/

/********* Start flags for ignore field (bit field flags) *********/

#define SM_CAN_SPEC_IGNORE           0x00000001

#define SM_USER_IGNORE_MARK1    0x00000100
#define SM_USER_IGNORE_MARK2    0x00000200
#define SM_USER_IGNORE_MARK3    0x00000400
#define SM_USER_IGNORE_MARK4    0x00000800

#define SM_PRIC_PRIO_IGNORE     0x00001000
#define SM_MVE_IGNORE           0x00002000      /* ITI/JCG 8/99 */

/********* End flags for ignore field (bit field flags) *********/

/********* Start flags for SM_Cb (bit field flags) *********/

/* 
 * External SM_Cb flags.  
 * User may specify in SM_new_cb/cb_flags. -ITI/JCG 8/99 
 */

/* Flag that register allocation has not been done */
#define SM_PREPASS              0x00000002

/* Flag that register allocation has been done (no virtual registers) */
#define SM_POSTPASS             0x00000004

/* Flag that DHASY acyclic scheduling will be done on the cb */
#define SM_DHASY                0x00000008

/* Flag that modulo cyclic scheduling will be done on the cb */
#define SM_MODULO               0x00000010

/* Flag that the issue times should be normalized to 0 (or near to 0)
 * when the isl attributes are written out by SM_commit_cb.  Does
 * not change any internal data structures (i.e. something scheduled
 * in cycle -7 remains in cycle -7 as far as SM is concerned), only
 * changes the isl attribute. -ITI/JCG 9/99
 *
 * This allows scheduling algorithms to use arbitrary starting points
 * and/or negative issue times.  
 *
 * For acyclic scheduling (e.g., SM_DHASY):
 *  This flags causes issue times to be adjusted so that the first 
 *  issue time is 0 (which IMPACT tools expect).
 *
 * For cyclic scheduling (e.g., SM_MODULO):
 *  This means adjusting the issue times so that the first stage is 0 
 *  and the backedge branch is placed in the last cycle in the kernel (II-1).
 */
#define SM_NORMALIZE_ISSUE      0x00000020

/* 
 * Internal SM_Cb flags.  
 * Must not be set by user in SM_new_cb/cb_flags.
 */

/* Flags that cross-iteration dependences should be drawn for the cb.
 * Currently set if SM_MODULO is specified by user. -ITI/JCG 8/99.
 */
#define SM_CROSS_ITERATION      0x00000100

/* Flags that Modulo variable expansion will be performed after scheduling.
 * This causes many anti and output dependences to be ignored.
 * Currently set if SM_MODULO is specified by user. -ITI/JCG 8/99 
 */
#define SM_ASSUME_MVE           0x00000200

/* Flag that a modulo resource map should be used for the cb.
 * Currently set if SM_MODULO is specified by user. -ITI/JCG 8/99.
 */
#define SM_MODULO_RESOURCES     0x00000400

/* Used to draw dependences properly (using dataflow info) */
#define SM_CB_HAS_FALL_THRU     0x00001000
#define SM_CB_NO_FALL_THRU      0x00002000

/* Flags that resources need to be checked for only the first alt
 * that meets the format and dependence requirements for scheduling.
 * (Otherwise, all the alternatives that meet the first two requirements
 *  will be tested.)
 * This allows more efficient scheduling of SuperSPARC's cascadable operations.
 * 
 * Set by the MDES2 parameter "check_resources_for_only_one_alt".
 */
#define SM_CB_TEST_ONLY_ONE_ALT 0x00004000

/* Internal flags that should not be explicitly specified by user.
 * SM_new_cb punts if any of these are specified by user in cb_flags.
 */
#define SM_CB_RESERVED_FLAGS \
          (SM_CROSS_ITERATION | SM_ASSUME_MVE | \
           SM_MODULO_RESOURCES | SM_CB_HAS_FALL_THRU | \
           SM_CB_NO_FALL_THRU | SM_CB_TEST_ONLY_ONE_ALT)

/* 20040712SZU */
/* Flag that sequential order will be retained in the cb */
#define SM_SEQUENTIAL           0x00010000

/********* End flags for SM_Cb (bit field flags) *********/

/********* Start flags for SM_schedule_op (bit field flags) *********/

/* Test to see if the operation can be scheduled, but don't schedule it */
#define SM_TEST_ONLY            0x00000001

/********* End flags for SM_schedule_op (bit field flags) *********/



/********* Start defines for sm_op->ext_dest and sm_op->ext_src *********/
#define SM_MEM_ACTION_INDEX     0
#define SM_CTRL_ACTION_INDEX    1
#define SM_SYNC_ACTION_INDEX    2
#define SM_VLIW_ACTION_INDEX    3

#define SM_MEM_ACTION_OPERAND   (&_sm_mem_action_operand)
#define SM_CTRL_ACTION_OPERAND  (&_sm_ctrl_action_operand)
#define SM_SYNC_ACTION_OPERAND  (&_sm_sync_action_operand)
#define SM_VLIW_ACTION_OPERAND  (&_sm_vliw_action_operand)
/********* End defines for sm_op->ext_dest and sm_op->ext_src *********/

/********* Start defines for sm_reg_info->type  *********/
#define SM_REGISTER_TYPE        0x01
#define SM_MACRO_TYPE           0x02
#define SM_EXT_ACTION_TYPE      0x04
/* Because float and double share reg numbers */
#define SM_CTYPE_DOUBLE         0x08

#define MAX_SM_TYPE_POWER       4       /* Number of bits used by above */
/********* End defines for sm_reg_info->type  *********/

/********* Start defines for sm_trans->type ********/
/* Make usable in a bit field to allow selective transformation enabling */
#define RENAMING_WITH_COPY      0x00000001
#define EXPR_WITH_COPY          0x00000002
/********* End defines for sm_trans->type ********/

/********* Start defines for sm_trans->flags ********/
#define SM_CAN_TRANS_SRC0       0x00000001
#define SM_CAN_TRANS_SRC1       0x00000002

#define SM_CANNOT_DO_TRANS      0x00000004
#define SM_IGNORE_TRANS         0x00000008

#define SM_MUST_REORDER_OPS     0x00000010
#define SM_NEEDS_RENAMING       0x00000020
#define SM_DUPLICATED_DEF       0x00000040
#define SM_DELETED_DEF          0x00000080
/********* End defines for sm_trans->flags ********/

/********* Start constant defines ********/
#define SM_MAX_SLOT             65535
#define SM_MAX_CYCLE            2000000000
#define SM_MIN_CYCLE            -SM_MAX_CYCLE
/* 20030603 SZU
 * Itanium scheduling constants
 */
#define SM_NO_TEMPLATE		-1
/* 20030609 SZU
 * Constants for creating Itanium stop bits.
 * Need to re-examine interface to template op creation and generalize.
 */
#define S_AFTER_3RD (0x1)	/* stop bit after instr 3 in bundle */
#define S_AFTER_2ND (0x2)	/* stop bit after instr 2 in bundle */
#define S_AFTER_1ST (0x4)	/* stop bit after instr 1 in bundle */
#define NO_S_BIT    (0)		/* no stop bit in bundle */

/* 20030609 SZU
 * Bundle template constants used in Ltahoe
 */
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

/********* End constant defines ********/

/* Structures for SM_Oper queue */
typedef struct SM_Oper_Queue
{
  int num_qentries;             /* Number of sm_ops in queue */
  struct SM_Oper_Qentry *first_qentry;  /* First sm_op in queue */
  struct SM_Oper_Qentry *last_qentry;   /* Last sm_op in queue */
}
SM_Oper_Queue;

typedef struct SM_Oper_Qentry
{
  SM_Oper_Queue *queue;         /* Queue this qentry is in */
  struct SM_Oper *sm_op;        /* The sm_op this qentry holds */
  struct SM_Oper_Qentry *next_qentry;   /* Next sm_op in queue */
  struct SM_Oper_Qentry *prev_qentry;   /* Prev sm_op in queue */
  struct SM_Oper_Qentry *next_queue;    /* Next queue this sm_op is in */
  struct SM_Oper_Qentry *prev_queue;    /* Prev queue this sm_op is in */
}
SM_Oper_Qentry;


/* Structures for SM_Reg_Action queue */
typedef struct SM_Action_Queue
{
  int num_qentries;             /* Number of actions in queue */
  struct SM_Action_Qentry *first_qentry;        /* First action in queue */
  struct SM_Action_Qentry *last_qentry; /* Last action in queue */
}
SM_Action_Queue;

typedef struct SM_Action_Qentry
{
  SM_Action_Queue *queue;       /* Queue this qentry is in */
  struct SM_Reg_Action *action; /* The action this qentry holds */
  struct SM_Action_Qentry *next_qentry; /* Next action in queue */
  struct SM_Action_Qentry *prev_qentry; /* Prev action in queue */
  struct SM_Action_Qentry *next_queue;  /* Next queue this action is in */
  struct SM_Action_Qentry *prev_queue;  /* Prev queue this action is in */
}
SM_Action_Qentry;


/* Structures for SM_Trans queue */
typedef struct SM_Trans_Queue
{
  char *name;
  int num_qentries;             /* Number of trans in queue */
  struct SM_Trans_Qentry *first_qentry; /* First trans in queue */
  struct SM_Trans_Qentry *last_qentry;  /* Last trans in queue */
}
SM_Trans_Queue;

typedef struct SM_Trans_Qentry
{
  SM_Trans_Queue *queue;        /* Queue this qentry is in */
  struct SM_Trans *trans;       /* The trans this qentry holds */
  struct SM_Trans_Qentry *next_qentry;  /* Next trans in queue */
  struct SM_Trans_Qentry *prev_qentry;  /* Prev trans in queue */
  struct SM_Trans_Qentry *next_queue;   /* Next queue this trans is in */
  struct SM_Trans_Qentry *prev_queue;   /* Prev queue this trans is in */
}
SM_Trans_Qentry;

/* 20021210 SZU
 * Added to form priority queues while shuffling in issue group
 */
typedef struct SM_Priority_Queue
{
  int num_qentries;		/* Number of sm_ops in queue */
  int num_not_sched;	/* Number of sm_ops in queue not scheduled */
  struct SM_Priority_Qentry *first_qentry;  /* First sm_op in queue */
  struct SM_Priority_Qentry *last_qentry;   /* Last sm_op in queue */
}
SM_Priority_Queue;

typedef struct SM_Priority_Qentry
{
  SM_Priority_Queue *queue;	/* Queue this qentry is in */
  struct SM_Oper *oper;		/* The sm_op this qentry holds */
  int priority;			/* Resource priority (PORT) this sm_op has */
  int scheduled;		/* 1 if oper is scheduled */
  struct SM_Priority_Qentry *next_qentry;
  struct SM_Priority_Qentry *prev_qentry;
}
SM_Priority_Qentry;

typedef struct SM_Reg_Action
{
  struct SM_Oper *sm_op;        /* Op performing this action */
  unsigned char operand_type;   /* MDES operand type */
  unsigned char operand_number; /* operand number */
  unsigned short index;         /* MDES operand index */
  unsigned int flags;           /* See #defs above */
  int add_lat;

  /* Fields for dependence management */
  struct SM_Dep *first_dep_in;  /* List of deps in to action */
  struct SM_Dep *first_dep_out; /* List of deps out of action */

  /* Range of early register use times possible and the actual early use 
   * time when operation is actually scheduled.
   *
   * Used for use-time-based dependences into register action.
   */
  short min_early_use_time;
  short max_early_use_time;
  short actual_early_use_time;

  /* Range of late register use times possible and the actual late use 
   * time when operation is actually scheduled.
   *
   * Used for use-time-based dependences out of register action.
   */
  short min_late_use_time;
  short max_late_use_time;
  short actual_late_use_time;

  /* Fields for register action conflicts.  One entry for the actual and
   * each of the conflicting action generated for this action.
   */
  struct SM_Reg_Action **conflict;

  /* Fields for register action list management */
  struct SM_Reg_Info *rinfo;    /* Action's register info */
  struct SM_Reg_Action *next_complete;  /* Actual/conflicting def/use */
  struct SM_Reg_Action *prev_complete;  /* actions */
  struct SM_Reg_Action *next_actual;    /* Actual def/use actions */
  struct SM_Reg_Action *prev_actual;
  struct SM_Reg_Action *next_def;       /* Actual def actions */
  struct SM_Reg_Action *prev_def;

  struct SM_Reg_Action *next_op_action; /* For operand list in sm_op */
  struct SM_Reg_Action *prev_op_action;

  SM_Action_Qentry *first_queue;        /* Queues this action is in */
}
SM_Reg_Action;

typedef struct SM_Reg_Info
{
  int id;                       /* Register #, Macro id */
  int type;                     /* Operand type */
  L_Operand *operand;           /* Operand for lcode queries */
  unsigned int flags;           /* Rinfo flags */

  /* Fields for register conflict management.  This reg_info will not
   * not be in the array (unlike Mspec call).  The array pointer will
   * be NULL if there are no register conflicts (typical when dealing
   * with virtual register).
   */
  struct SM_Reg_Info **reg_conflict;    /* Null if no reg conflicts */
  int num_conflicts;            /* Size of reg_conflict */

  /* Fields for register action list management */
  SM_Reg_Action *first_complete;        /* Complete list of actual */
  SM_Reg_Action *last_complete; /* and conflicting actions */
  SM_Reg_Action *first_actual;  /* List of all actual actions */
  SM_Reg_Action *last_actual;
  SM_Reg_Action *first_def;     /* List of actual def actions */
  SM_Reg_Action *last_def;

  /* Fields for sm_cb's rinfo hash and list management */
  struct SM_Cb *sm_cb;
  struct SM_Reg_Info *next_hash;        /* For rinfo hash table */
  struct SM_Reg_Info *prev_hash;
  struct SM_Reg_Info *next_rinfo;       /* For cb's rinfo list */
  struct SM_Reg_Info *prev_rinfo;
  void *ext;                            /* Used for lr_info in mod. sched. */
}
SM_Reg_Info;

/* 20020824 SZU
 * List of PCLat for SM_Dep
 */
typedef struct SM_Dep_PCLat
{
  SM_PCLat *pclat;
  int from_penalty, to_penalty;

  struct SM_Dep_PCLat *next_dep_pclat;
  struct SM_Dep_PCLat *prev_dep_pclat;
}
SM_Dep_PCLat;

typedef struct SM_Dep
{
  SM_Reg_Action *from_action;   /* src of dep arc */
  SM_Reg_Action *to_action;     /* dest of dep arc */

  unsigned int flags;           /* Dep flags (type, delay type, etc) */
  unsigned int ignore;          /* Dep ignore flags */

  short delay_offset;           /* Offset of use-time-based delays */
  short min_delay;              /* minimum dependence latency */
  short max_delay;              /* maximum dependence latency */
  short omega;                  /* dependence distance in iterations */

  struct SM_Dep *next_dep_out;  /* next dep_out in from_action */
  struct SM_Dep *prev_dep_out;  /* prev dep_out in from_action */
  struct SM_Dep *next_dep_in;   /* next dep_in in to_action */
  struct SM_Dep *prev_dep_in;   /* prev dep_in in to_action */

  /* 20020824 SZU */
  struct SM_Dep_PCLat *pclat_list;
}
SM_Dep;

typedef struct SM_Compatible_Alt
{
  Mdes_Alt *normal_version;
  Mdes_Alt *silent_version;
  struct SM_Compatible_Alt *next_compatible_alt;
}
SM_Compatible_Alt;

typedef struct SM_Oper
{
  L_Oper *lcode_op;
  struct SM_Cb *sm_cb;
  unsigned int flags;           /* SM flags, see top of header file */
  unsigned int ignore;          /* Ignore flags */
  unsigned int mdes_flags;      /* From lmdes2, use OP_FLAG_XXX */

  Mdes_Operation *mdes_op;

  /* 20021210 SZU
   * Added to keep track of issue group and bundles in Itanium
   */
  struct SM_Issue_Group *issue_group;	/* Issue group */
  unsigned int syll_type;		/* Syllable type scheduled with */
  int old_issue_time;		/* Store previous scheduled issue time */
  				/* Important for modulo scheduling */
  SM_Priority_Qentry *qentry;

  /* Linked list of scheduling alternatives that have an operand format
   * compatible with the operation.
   */
  SM_Compatible_Alt *first_compatible_alt;

  /* Array of reg action pointers for all operands */
  SM_Reg_Action **operand;      /* Indexed by mdes operand index */

  /* The pointers below point to the appropriate place in the above array */
  SM_Reg_Action **dest;         /* Array of ptrs to dest reg actions */
  SM_Reg_Action **src;          /* Array of ptrs to src reg actions */
  SM_Reg_Action **pred;         /* Array of ptrs to pred reg actions */
  SM_Reg_Action **ext_dest;     /* Array of ptrs to ext dest reg acts */
  SM_Reg_Action **ext_src;      /* Array of ptrs to ext src reg acts */

  /* Implicit dest queue defined only for jsrs, NULL otherwise 
   * Implicit src queue defined only for cbrs and jsrs, NULL otherwise 
   */
  SM_Action_Queue *implicit_dests;      /* Queue of implicit dest actions */
  SM_Action_Queue *implicit_srcs;       /* Queue of implicit src actions */

  /* List of the actual actions for this op (only non-NULL actions) */
  SM_Reg_Action *first_op_action;
  SM_Reg_Action *last_op_action;

  /* Priority of this operation (may be static or dynamic) */
  float priority;

  /* Early and late times for this operation.  
   * One late time per exit and are relative to that exit.
   * A late time of -2000000000 indicates that this operation does
   * not need to be executed for that exit.
   */
  int early_time;
  int *late_time;

  /* Where this operation is scheduled */
  int sched_cycle;
  unsigned short sched_slot;

  /* The alt and an array of table options chosen when scheduled */
  SM_Compatible_Alt *alt_chosen;
  unsigned short *options_chosen;

  /* The true bounds on where this operation may be scheduled */
  int cycle_lower_bound;
  int cycle_upper_bound;
  int nosoft_cycle_lower_bound;
  int nosoft_cycle_upper_bound;
  unsigned short slot_lower_bound;
  unsigned short slot_upper_bound;
  unsigned short nosoft_slot_lower_bound;
  unsigned short nosoft_slot_upper_bound;

  /* The dependences casuing the upper and lower bounds (NULL if none) */
  SM_Dep *dep_lower_bound;
  SM_Dep *dep_upper_bound;
  SM_Dep *nosoft_dep_lower_bound;
  SM_Dep *nosoft_dep_upper_bound;

  unsigned short num_hard_dep_in;
  unsigned short num_unresolved_hard_dep_in;
  unsigned short num_soft_dep_in;
  unsigned short num_unresolved_soft_dep_in;
  unsigned short num_ignore_dep_in;
  unsigned short num_unresolved_ignore_dep_in;

  unsigned short num_hard_dep_out;
  unsigned short num_unresolved_hard_dep_out;
  unsigned short num_soft_dep_out;
  unsigned short num_unresolved_soft_dep_out;
  unsigned short num_ignore_dep_out;
  unsigned short num_unresolved_ignore_dep_out;

  /* Used to place operations in scheduled order in sm_cb */
  struct SM_Oper *next_sched_op;
  struct SM_Oper *prev_sched_op;

  SM_Oper_Qentry *first_queue;  /* Queues this sm_op is in */

  /* Pointers to qentries that the sm manages */
  SM_Oper_Qentry *dep_in_resolved_qentry;

  /* Maintains serial order of operations (for generating dep graph,
   * inserting new operations into cb, etc.)  May change from original
   * order but must always stay a valid order for superscalar operation.
   */
  unsigned int serial_number;   /* Compare to determine serial order */
  struct SM_Oper *next_serial_op;
  struct SM_Oper *prev_serial_op;

  /* Fields used by error detection routines */
  int temp_height;              /* Used to find bad deps */

  /* 20030909 SZU
   * Used to indicate oper has been moved down as much as possible to reduce
   * live range and therefore register pressure.
   * Introduced to avoid flip-flop case.
   */
  int liverange_reduced;
}
SM_Oper;

/* 20030603 SZU
 * Added so variable # of templates possible in issue group.
 * Arose from versatile definition format in mdes.
 */
typedef struct SM_Bundle
{
  unsigned int template_mask;	/* Mask of the current template */
  int template_index;		/* Index of the template in mdes */
  int stop;			/* Only one stop bit per issue, after slot # */
  int empty;			/* Indicates nothing in this bundle */

  /* 20030708 SZU
   * Added field to lock templates for compaction w/ internal stop bits.
   * Also added field to indicate internal stop bit being used.
   */
  int template_lock;
  int internal_stop_bit;
}
SM_Bundle;

/* 20021210 SZU
 * Keep track of Itanium issue groups
 * 20030602 SZU
 * Altered to correspond with format from mdes.
 */
typedef struct SM_Issue_Group
{
  struct SM_Cb *sm_cb;
  SM_Oper **slots;

  int issue_time;		/* Cycle time for issue group */
  int full;			/* 1 if all slots taken */
  /* 20030715 SZU
   * Indicate number of available slots left; empty + nop.
   * More precise than full, faster fail, especially for compaction.
   */
  int num_slots_left;

  SM_Bundle **bundles;		/* Array w/ template info per bundle */

  struct SM_Issue_Group *next_issue_group;
  struct SM_Issue_Group *prev_issue_group;
}
SM_Issue_Group;

typedef struct SM_Cb
{
  L_Cb *lcode_cb;
  L_Func *lcode_fn;             /* Until sm_func is used -JCG */
  struct SM_Func *sm_func;
  struct SM_Mdes *sm_mdes;
  Mdes *version1_mdes;          /* Still using version1 structures */
  unsigned int flags;

  /* Used for calling mspec to find out which operands conflict */
  int (*conflicting_operands) ();
  int prepass_sched;            /* Set to 1 if using virtual regs */

  /* Operation List Fields (both serial and scheduled order lists) */
  unsigned int op_count;
  unsigned int num_unsched;     /* Num ops unscheduled */
  unsigned int num_ignored;     /* Num ops ignored */
  SM_Oper *first_sched_op;
  SM_Oper *last_sched_op;
  SM_Oper *first_serial_op;
  SM_Oper *last_serial_op;

  /* 20020904 SZU
   * Added for Itanium bundles.
   * Keep track of the issue groups in the cb
   */
  SM_Issue_Group *first_issue_group;
  SM_Issue_Group *last_issue_group;

  /* Queue of scheduled kernel operations, in the committed Lcode order 
   * (i.e., the isl attribute order).  The first_sched_op, etc. above holds
   * the order in the "unrolled" version of the code. 
   * Will be NULL until a modulo-scheduling kernel is committed. 
   * -ITI/JCG 9/99
   */
  SM_Oper_Queue *kernel_queue;

  /* Queue of unscheduled operations with all the dep_in resolved.
   * These are the operations a list-scheduler should focus on.
   */
  SM_Oper_Queue *dep_in_resolved;

  /* Dependences with these special ignore flags are considered actual
   * dependences by the early and late time calculations but are truly
   * ignored everywhere else (like normal).
   */
  unsigned int special_dep_ignore_flags;

  /* SM_Cb List Fields */
  struct SM_Cb *next_sm_cb;
  struct SM_Cb *prev_sm_cb;

  /* CB exit info fields */
  int num_exits;
  SM_Oper **exit_op;            /* Array of exit ops, */
  /* Fall thru op will be NULL */
  double *exit_weight;          /* Array of exit weights */
  double *exit_percentage;      /* Array of exit percentages */
  double cb_weight;             /* Total weight of cb */

  /* Register Info Fields */
  SM_Reg_Info **rinfo_hash;     /* Array of size hash_size */
  unsigned int rinfo_hash_size; /* Must be power of 2 */
  unsigned int rinfo_hash_mask; /* AND mask, (hash_size - 1) */
  unsigned int rinfo_resize_size;       /* When reached, resize hash table */
  SM_Reg_Info *first_rinfo;     /* List of Reg Info's for cb */
  SM_Reg_Info *last_rinfo;
  unsigned int rinfo_count;     /* Number of Reg Infos */

  /* For quick access to the appropriate lists */
  SM_Reg_Info *mem_rinfo;       /* Rinfo for mem actions */
  SM_Reg_Info *ctrl_rinfo;      /* Rinfo for ctrl actions */
  SM_Reg_Info *sync_rinfo;      /* Rinfo for sync actions */
  SM_Reg_Info *vliw_rinfo;      /* Rinfo for vliw actions */

  /* Resource Map Fields */
  unsigned int *map_array;      /* Resource map array */
  int map_array_size;           /* Size of map array (power of 2) */
  int map_start_offset;         /* Offset of map_array[0] */
  int map_end_offset;           /* Offset of the end of map_array */
  int min_init_offset;          /* Min usage offset intialized */
  int max_init_offset;          /* Max usage offset intialized */

  /* Modulo scheduling fields */
  int II;                       /* Iteration interval */
  int stages;                   /* Number of modulo stages */
  int sched_cycle_offset;       /* Cycle normalization constant */

  /* Checks added for potential recovery code */
  List chk_list;
} SM_Cb;

typedef struct SM_Func
{
  SM_Cb *first_sm_cb;
  SM_Cb *last_sm_cb;
} SM_Func;

typedef struct SM_Trans
{
  int cb_id;                    /* For debuging, usually an lcode_id */
  int op_id;                    /* For debuging, usually an lcode_id */
  int type;                     /* Type of transformation */
  unsigned int flags;           /* Flags for transformation */
  SM_Oper *target_sm_op;        /* Operation transformation benefits */
  SM_Oper *def_sm_op;           /* Define that we are targeting */
  SM_Oper *def2_sm_op;          /* Second define that we are targeting */
  short target_index;           /* dest/src operand affected */
  short other_index;            /* other dest/src operand affected */
  int target_sched_cycle;       /* Cycle target sched in */

  /* Information for undoing transformations */
  SM_Oper *orig_prev_serial_op; /* So can undo reordering */
  SM_Oper *new_sm_op;           /* Op created for trans, (for undo) */
  SM_Oper *renaming_sm_op;      /* Op used for renaming, (for undo) */
  L_Operand *orig_src[2];       /* Original target source operands */
  int orig_opc;                 /* Orig target_sm_op opc */
  int orig_proc_opc;            /* Orig target_sm_op proc_opc */
  ITintmax orig_ext;            /* Orig target_sm_op ext */
  L_Operand *orig_def_src[2];   /* Original def source operands */
  int orig_def_opc;             /* Orig def_sm_op opc */
  int orig_def_proc_opc;        /* Orig def_sm_op proc_opc */
  ITintmax orig_def_ext;        /* Orig def_sm_op ext */
  SM_Oper *orig_def_prev_serial_op;     /* So can undo reordering */
  L_Oper *deleted_lcode_op;     /* So can undo */
  L_Oper *deleted_def_lcode_op; /* So can undo */

  SM_Trans_Qentry *first_queue; /* Queues this trans is in */
}
SM_Trans;

typedef struct SM_Stats
{
  int num_oper_checks;
  int num_oper_checks_failed;
  int num_table_checks;
  int num_table_checks_failed;
  int num_usage_checks;
  int num_usage_checks_failed;
  int num_slot_checks;
  int num_slot_checks_failed;
}
SM_Stats;

extern void SM_init (Parm_Macro_List * command_line_macro_list);
/* 20030108 SZU
 * Add SM_finalize to wrap things up at end of Ltahoe.
 * Clean up memory usage.
 * 20031118 SZU
 * Doesn't seem to be defined; get rid of
extern void SM_finalize (void);
 */

extern void SM_schedule_fn (L_Func * fn, Mdes * version1_mdes,
                            int prepass_sched);
extern void SM_bundle_fn (L_Func * fn, Mdes * version1_mdes);
extern SM_Oper *SM_find_sm_op (SM_Cb * sm_cb, L_Oper * l_oper);
extern SM_Cb *SM_new_cb (Mdes * version1_mdes, L_Cb * lcode_cb, int cb_flags);
extern void SM_set_cb_II (SM_Cb * sm_cb, int II);
extern void SM_schedule_cb (SM_Cb * sm_cb);
extern void SM_unschedule_cb (SM_Cb * sm_cb);
extern double SM_calc_best_case_cycles (SM_Cb * sm_cb);
extern L_Attr *SM_attach_isl (SM_Oper * sm_op, int sched_cycle_offset);
extern int SM_set_sched_cycle_offset (SM_Cb *sm_cb);
extern void SM_commit_cb (SM_Cb * sm_cb);
extern void SM_print_cb_schedule (SM_Cb * sm_cb);
extern void SM_delete_cb (SM_Cb * sm_cb);
extern void SM_construct_temp_kernel_queue (SM_Cb * sm_cb);
extern SM_Oper *SM_insert_oper_after (SM_Cb * sm_cb, L_Oper * lcode_op,
                                      SM_Oper * after_sm_op);
extern void SM_move_oper_after (SM_Oper * move_sm_op, SM_Oper * after_sm_op);
extern void SM_delete_oper (SM_Oper * sm_op);
extern void SM_ignore_op (SM_Oper *, int);
extern void SM_enable_ignored_op (SM_Oper *, int);

/* 20021213 SZU
 * Modify to accomodate Itanium
 */
extern int SM_sched_table (SM_Cb * sm_cb, SM_Oper * sm_op,
			   unsigned int template_index, SM_Table * table,
                           unsigned short *choice_array, int time,
                           unsigned short min_slot, unsigned short max_slot,
                           int commit, Mdes_Stats * stats);
#if 0
extern int SM_sched_table (SM_Cb * sm_cb, SM_Table * table,
                           unsigned short *choice_array, int time,
                           unsigned short min_slot, unsigned short max_slot,
                           int commit, Mdes_Stats * stats);
#endif
extern void SM_unsched_table (SM_Cb * sm_cb, SM_Table * table,
                              unsigned short *choice_array, int time);
extern void SM_print_option (FILE * out, SM_Mdes * sm_mdes,
                             SM_Option * option);
extern void SM_print_map (FILE * out, SM_Mdes * sm_mdes, SM_Cb * sm_cb);

extern void SM_free_alloc_pools ();

extern void SM_build_reg_info_table (SM_Cb * sm_cb);
extern void SM_free_reg_info_table (SM_Cb * sm_cb);
extern SM_Reg_Info *SM_add_reg_info (SM_Cb * sm_cb, L_Operand * reg_operand);
extern SM_Reg_Info *SM_find_reg_info (SM_Cb * sm_cb, L_Operand * reg_operand);

extern SM_Reg_Action *SM_add_reg_action (SM_Oper * sm_op, int operand_type,
                                         int operand_number,
                                         L_Operand * reg_operand);
extern void SM_add_reg_actions_for_op (SM_Oper * sm_op);
extern void SM_delete_reg_action (SM_Reg_Action * action);
extern void SM_reposition_reg_action (SM_Reg_Action * action);

extern void SM_build_oper_dependences (SM_Cb * sm_cb, SM_Oper * sm_op,
                                       unsigned int flags);
extern void SM_build_cb_dependences (SM_Cb * sm_cb);
extern void SM_print_oper_dependences (FILE * out, SM_Oper * sm_op);
extern void SM_print_cb_dependences (FILE * out, SM_Cb * sm_cb);
extern void SM_print_oper (FILE * out, SM_Oper * sm_op);
extern void SM_print_dep (FILE * out, SM_Dep * dep);
extern void SM_print_action_id (FILE * out, SM_Reg_Action * action);
extern void SM_add_dep (SM_Reg_Action * from_action,
                        SM_Reg_Action * to_action, unsigned int flags,
                        unsigned int ignore, short delay_offset, 
                        short omega, unsigned int mode);

/* 10/22/04 REK Inserting prototype from sm_dep.c. */
extern void SM_insert_sync_dep_betw_ops (SM_Oper *from_op, SM_Oper *to_op);

/* Use the following dep routines only when doing transformations */
extern void SM_build_src_reg_deps (SM_Reg_Action * src_action,
                                   unsigned int flags);
extern void SM_build_dest_reg_deps (SM_Reg_Action * dest_action,
                                    unsigned int flags);
extern void SM_rebuild_dest_reg_deps (SM_Reg_Action * dest_action,
                                      unsigned int flags);
extern void SM_build_src_mem_deps (SM_Reg_Action * src_action,
                                   unsigned int flags);
extern void SM_build_dest_mem_deps (SM_Reg_Action * dest_action,
                                    unsigned int flags);
extern void SM_build_src_ctrl_deps (SM_Cb * sm_cb, SM_Reg_Action * src_action,
                                    unsigned int flags);
extern void SM_build_dest_ctrl_deps (SM_Cb * sm_cb,
                                     SM_Reg_Action * dest_action,
                                     unsigned int flags);
extern void SM_build_src_sync_deps (SM_Reg_Action * src_action,
                                    unsigned int flags);
extern void SM_build_dest_sync_deps (SM_Reg_Action * dest_action,
                                     unsigned int flags);
extern void SM_delete_dep (SM_Dep * dep);
extern void SM_ignore_dep (SM_Dep * dep, int flag);
extern void SM_enable_ignored_dep (SM_Dep * dep, int flag);

extern void SM_ignore_dep_out (SM_Oper * sm_op, int flag);
extern void SM_enable_ignored_dep_out (SM_Oper * sm_op, int flag);
extern void SM_ignore_dep_in (SM_Oper * sm_op, int flag);
extern void SM_enable_ignored_dep_in (SM_Oper * sm_op, int flag);

extern int SM_def_post_dominates_action (SM_Reg_Action * def_after,
                                         SM_Reg_Action * action);
extern void SM_print_action_operand (FILE * out, SM_Reg_Action * action);

extern void SM_print_reg_action (FILE * out, SM_Reg_Action * action);
extern void SM_print_reg_info (FILE * out, SM_Reg_Info * rinfo);
extern void SM_print_reg_info_operand (FILE * out, SM_Reg_Info * rinfo);
extern void SM_print_reg_info_table (FILE * out, SM_Cb * sm_cb);
extern void SM_print_sorted_reg_info_table (FILE * out, SM_Cb * sm_cb);




extern void SM_calculate_priorities (SM_Cb * sm_cb, int min_early_time);
extern int SM_calculate_early_times (SM_Cb * sm_cb, int min_early_time);
extern void SM_calculate_late_times (SM_Cb * sm_cb, int max_late_time);
extern void SM_print_early_and_late_times (FILE * out, SM_Cb * sm_cb);
extern int SM_recalculate_early_time (SM_Oper * sm_op, int min_early_time);

extern void SM_recalculate_lower_bound (SM_Oper * sm_op);
extern void SM_recalculate_upper_bound (SM_Oper * sm_op);
extern int SM_schedule_oper (SM_Oper * sm_op, int issue_time,
                             int earliest_slot, int latest_slot,
                             unsigned int sched_flags);

/* 20030829 SZU
 * New version w/ new parameter.
 * New parameter is flag/data, to be used to avoid useless dep checks
 * when shuffling for Itanium.
 */
extern void SM_unschedule_oper (SM_Oper * sm_op,
				SM_Priority_Queue *priority_queue);
#if 0
extern void SM_unschedule_oper (SM_Oper * sm_op);
#endif

extern SM_Oper_Queue *SM_new_oper_queue ();
extern void SM_delete_oper_queue (SM_Oper_Queue * queue);
extern SM_Oper_Qentry *SM_enqueue_oper_before (SM_Oper_Queue * queue,
                                               SM_Oper * sm_op,
                                               SM_Oper_Qentry *
                                               before_qentry);
extern SM_Oper_Qentry *SM_enqueue_oper_after (SM_Oper_Queue * queue,
                                              SM_Oper * sm_op,
                                              SM_Oper_Qentry * after_qentry);
extern void SM_dequeue_oper (SM_Oper_Qentry * qentry);
extern void SM_dequeue_oper_from_all (SM_Oper * sm_op);
extern void SM_print_oper_queue (FILE * out, SM_Oper_Queue * queue);


extern SM_Action_Queue *SM_new_action_queue ();
extern void SM_delete_action_queue (SM_Action_Queue * queue);
extern SM_Action_Qentry *SM_enqueue_action_before (SM_Action_Queue * queue,
                                                   SM_Reg_Action * action,
                                                   SM_Action_Qentry *
                                                   before_qentry);
extern SM_Action_Qentry *SM_enqueue_action_after (SM_Action_Queue * queue,
                                                  SM_Reg_Action * action,
                                                  SM_Action_Qentry *
                                                  after_qentry);
extern void SM_dequeue_action (SM_Action_Qentry * qentry);
extern void SM_dequeue_action_from_all (SM_Reg_Action * action);
extern void SM_print_action_queue (FILE * out, SM_Action_Queue * queue);


extern SM_Trans_Queue *SM_new_trans_queue (char *name);
extern void SM_delete_trans_queue (SM_Trans_Queue * queue);
extern SM_Trans_Qentry *SM_enqueue_trans_before (SM_Trans_Queue * queue,
                                                 SM_Trans * trans,
                                                 SM_Trans_Qentry *
                                                 before_qentry);
extern SM_Trans_Qentry *SM_enqueue_trans_after (SM_Trans_Queue * queue,
                                                SM_Trans * trans,
                                                SM_Trans_Qentry *
                                                after_qentry);
extern void SM_dequeue_trans (SM_Trans_Qentry * qentry);
extern void SM_dequeue_trans_from_all (SM_Trans * trans);
extern void SM_print_trans_queue (FILE * out, SM_Trans_Queue * queue);

extern void SM_change_operand (SM_Oper * sm_op, int operand_type,
                               int operand_number, L_Operand * new_operand);

extern void SM_set_ext (SM_Oper * sm_op, ITintmax ext);
extern ITintmax SM_get_ext (SM_Oper * sm_op);

/* SMH reconciliation */
extern void SM_do_relocate_cond_opti(SM_Cb * sm_cb);

extern SM_Trans *SM_new_trans (int type, SM_Oper * target_sm_op, int flags);
extern void SM_delete_trans (SM_Trans * sm_trans);
extern void SM_print_trans (FILE * out, SM_Trans * sm_trans);
extern SM_Trans *SM_can_rename_with_copy (SM_Oper * sm_op);
extern void SM_do_renaming_with_copy (SM_Trans * sm_trans);
extern void SM_undo_renaming_with_copy (SM_Trans * sm_trans);
extern void SM_update_sched_based_trans_info (SM_Trans_Queue * queue);
extern int SM_calc_action_avail_time (SM_Reg_Action * action);
extern void SM_do_trivial_renaming (SM_Cb * sm_cb);

extern void SM_do_trans (SM_Trans * trans);
extern int SM_can_undo_trans (SM_Trans * trans);
extern void SM_undo_trans (SM_Trans * trans);

extern SM_Trans_Queue *SM_find_potential_trans (SM_Cb * sm_cb,
                                                unsigned int allowed_trans);
extern void SM_delete_trans_and_queue (SM_Trans_Queue * queue);

extern double SM_calc_do_trans_priority (SM_Trans * sm_trans,
                                         int max_late_time);
extern SM_Trans *SM_dequeue_best_do_trans (SM_Trans_Queue * queue);

extern void SM_gen_code (Parm_Macro_List * external_macro_list);
extern int SM_def_dominates_action (SM_Reg_Action * def_before,
                                    SM_Reg_Action * action);
int SM_def_has_exactly_one_use (SM_Reg_Action * def_action);

void SM_create_map (SM_Cb * sm_cb, int min_usage_offset,
                    int max_usage_offset);
void SM_init_for_min_usage (SM_Cb * sm_cb, int min_usage_offset);
void SM_init_for_max_usage (SM_Cb * sm_cb, int max_usage_offset);

void SM_update_upper_bounds (SM_Dep * dep, int flags);
void SM_update_lower_bounds (SM_Dep * dep, int flags);
void SM_check_deps (SM_Cb * sm_cb);


/* 20030218 SZU
 * SMH reconciliation
 */
void SM_add_recovery_code (L_Func * fn);
int SM_softfix_promotion (SM_Dep * dep_in, int issue_time, int mode);
void SM_undo_fix_soft_dep (SM_Oper * sm_op);
extern double SM_do_classic_renaming_with_copy (SM_Cb * sm_cb);

/* 20021213 SZU
 * Modify to accomodate Itanium
 */
int SM_modulo_sched_table (SM_Cb * sm_cb, SM_Oper * sm_op, 
			   unsigned int template_index, SM_Table * table,
                           unsigned short *choice_array, int time,
                           unsigned short min_slot, unsigned short max_slot,
                           int commit, Mdes_Stats * stats);
#if 0
int SM_modulo_sched_table (SM_Cb * sm_cb, SM_Table * table,
                           unsigned short *choice_array, int time,
                           unsigned short min_slot, unsigned short max_slot,
                           int commit, Mdes_Stats * stats);
#endif

void SM_modulo_unsched_table (SM_Cb * sm_cb, SM_Table * table,
                              unsigned short *choice_array, int time);

/* 20030218 SZU
 * SMH reconciliation
 */
int SM_fix_soft_dep (SM_Oper * sm_op, int issue_time, int mode);

/* These need to be externally defined by 
   the code generator */
extern int S_machine_check (L_Oper * oper);
extern void S_machine_rts (L_Oper * oper);
extern void S_machine_jump (L_Oper * oper);

/* Use to determine if an operand is a register */
#define SM_is_reg(operand) (((operand) != NULL) && \
                            (((operand)->type == L_OPERAND_REGISTER) || \
                             ((operand)->type == L_OPERAND_MACRO)))

/* Special operands used for mem, ctrl, sync, and vliw action operands */
extern L_Operand _sm_mem_action_operand;
extern L_Operand _sm_ctrl_action_operand;
extern L_Operand _sm_sync_action_operand;
extern L_Operand _sm_vliw_action_operand;

/* Special parameters to changed only by Lbx86 -JCG 3/6/98 
 * See SM_dep.c for details on what these parameters do
 */
extern int SM_make_Lbx86_assumptions;
extern int SM_prevent_dead_Lbx86_defs_from_reordering;
extern int SM_use_fake_Lbx86_flow_analysis;

/* SM Parameters */
extern int SM_check_dependence_symmetry;
extern int SM_debug_use_sched_cb_bounds;
extern int SM_debug_lower_sched_cb_bound;
extern int SM_debug_upper_sched_cb_bound;

/* For Lhyper port to SM -JCG 6/99 */
extern int SM_use_fake_dataflow_info;

extern int SM_verify_reg_conflicts;

/* Dependence distance on output deps -- JWS 11/9/2000 */
extern int SM_output_dep_distance;

extern int SM_ignore_pred_analysis;
/* 20030218 SZU
 * SMH reconciliation
 */
extern int SM_perform_rename_with_copy;
extern int SM_perform_relocate_cond_opti;
/* JWS 20040105 */
extern int SM_sched_slack_loads_for_miss;

/* 20031024 SZU
 * New parms to indicate template bundling and compaction.
 */
extern int SM_do_template_bundling;
extern int SM_do_bundle_compaction;

/* Check list of special producer->consumer latencies -- SZU 20020804 */
extern void SM_check_prod_cons_lat_match (SM_Dep * dep,
					  SM_Reg_Action * prod_action,
					  SM_Reg_Action * cons_action);

/* Priority Queue functions -- SZU 20021210
 * Derived from Lsoftpipe Queue
 */
extern SM_Priority_Queue *SM_new_priority_queue ();
extern void SM_reinit_priority_queue (SM_Priority_Queue *);
extern void SM_delete_priority_queue (SM_Priority_Queue *);
extern void SM_enqueue_increasing_priority (SM_Priority_Queue *, SM_Oper *,
					    int);
extern SM_Oper *SM_dequeue_priority (SM_Priority_Queue *);
extern SM_Oper *SM_peek_end (SM_Priority_Queue *);
extern SM_Oper *SM_peek_head (SM_Priority_Queue *);

/* 20030709 SZU
 * Itanium scheduling functions
 */
/* nop functions */
extern SM_Oper *SM_create_nop (SM_Cb * sm_cb, int proc_opc);
extern int SM_insert_nop (SM_Issue_Group *issue_group_ptr, int slot,
			  unsigned int sched_flags);
extern void SM_delete_nop (SM_Oper *sm_op);
extern void SM_unschedule_nop (SM_Oper * sm_op);

/* Template bundle functions */
extern void SM_calculate_template_vector (SM_Issue_Group * issue_group);
extern void SM_assign_stop_bit (SM_Issue_Group * issue_group_ptr);

/* Issue group functions */
extern SM_Issue_Group *SM_create_issue_group (SM_Cb * sm_cb, int issue_time);
extern void SM_copy_issue (SM_Issue_Group *dest_issue_group_ptr,
			   SM_Issue_Group *src_issue_group_ptr);
extern void SM_restore_issue_group (SM_Issue_Group *issue_group_ptr,
				    SM_Issue_Group *save_issue_group_ptr);

/* Scheduling functions */
extern int SM_schedule_oper_priority (SM_Oper *sm_op, int issue_time,
				      int earliest_slot, int latest_slot,
				      unsigned int sched_flags, int compact);

/* Misc */
extern void SM_compact_w_internal_sbits (SM_Cb *sm_cb);
extern int SM_find_drule (SM_Oper * sm_op, unsigned int abs_slot,
			  unsigned int template_index, int prev_index);
extern int *SM_init_port_array (SM_Cb * sm_cb);
extern SM_Issue_Group *SM_check_for_issue_group (SM_Cb *sm_cb, int issue_time);

/* 20030218 SZU
 * SMH reconciliation
 */
#if defined(RC_CODE)
#include <Lcode/sm_recovery.h>
#endif

/* 20030603 SZU
 * Macros added for certain accesses.
 * Added after moving hard coded machine description to mdes.
 */
#define SM_get_num_slots(sm_cb) (sm_cb->version1_mdes->num_slots)
#define SM_get_slots_per_template(sm_cb) (sm_cb->sm_mdes->slots_per_template)
#define SM_is_nop(sm_op) (sm_op->mdes_op->nop)

#define SM_get_num_ports(sm_cb) (sm_cb->sm_mdes->num_ports)

#define SM_get_template_shift_var(sm_cb) \
  (sm_cb->sm_mdes->template_shift_var)
#define SM_get_template(sm_cb, template_index) \
  (sm_cb->sm_mdes->template_array[template_index].mask)
#define SM_get_template_per_issue(sm_cb) \
  (sm_cb->sm_mdes->max_template_per_issue)


#define SM_get_num_restricts(sm_cb) (sm_cb->sm_mdes->num_restricts)
#define SM_get_restrict_num(sm_cb, index) \
  (sm_cb->sm_mdes->restrict_array[index].num)
#define SM_get_restrict_mask(sm_cb, index) \
  (sm_cb->sm_mdes->restrict_array[index].mask)

#define SM_get_drule(sm_cb, index) (&sm_cb->sm_mdes->drule_array[index])
#define SM_get_drsrc(drule, drsrc_index) \
  (drule->rsrc[drsrc_index])

/* 20030609 SZU
 * Copied from mckinley_mdes.h, tmdes_instr.h.
 * Used in SM_commit_cb by SM_create_template_op.
 */
#define M_new_template(tmpl_op, tmpl_type) \
      ((tmpl_op)->src[0] = L_new_gen_int_operand(tmpl_type) )
#define M_new_stop_bit_mask(tmpl_op, stop_mask ) \
      ((tmpl_op)->src[1] = L_new_gen_int_operand(stop_mask))

/* 20030609 SZU
 * Copied from mckinley_mdes.h, tmdes_instr.h
 * Used in l_pipe_util
 */
#define M_is_template_op(op)       ((op)->proc_opc == TAHOEop_NON_INSTR && \
                                    (op)->dest[0] && \
                                    (op)->dest[0]->type == L_OPERAND_MACRO && \
                                    (op)->dest[0]->value.mac == \
                                    TAHOE_MAC_TEMPLATE)
#endif
