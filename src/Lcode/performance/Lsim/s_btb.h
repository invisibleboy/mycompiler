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
 *      File:   s_btb.h
 *      Author: Daniel A. Connors
 *      Creation Date:  1995
 *      Copyright (c) 1995 Daniel A. Connors, Wen-mei Hwu and
 *                         The Board of Trustees of the University of Illinois.
 *                         All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#ifndef _LSIM_S_BTB_H_
#define _LSIM_S_BTB_H_

#include <config.h>

/* Return stack models */
enum {
  BTB_RETURN_STACK_NONE =0,
  BTB_RETURN_STACK_REAL,
  BTB_RETURN_STACK_PERFECT
};

/* Counter models */
enum {
  BTB_COUNTER_MODEL_A2=0,
  BTB_COUNTER_MODEL_A3,
  BTB_COUNTER_MODEL_LT
};

/* Predicate branch prediction models */
enum {
   BTB_PREDICATE_MODEL_POP=0,
   BTB_PREDICATE_MODEL_PEP,
   BTB_PREDICATE_MODEL_PEP_HISTORY
};

typedef struct BTB_addr_node 
{
  int addr;
  struct BTB_addr_node *next,*prev;
} BTB_addr_node;


typedef struct BTB
{
  Scache *cache;
  char **history;                     /* Simulating multiple history tables */
  struct Pnode *pnode;
  int *BHT;			/* contains history patterns on a per-set basis */
  int number_BHT_sets;
  int BHT_index_set_at_bit;
  int address_bits_used;	/* number of bits used from address in gshare, gselect */
  int CBR_only;
/* SCM 7-21-00 */
/* made global vars from s_main.c and s_btb.c elements of BTB for modularity */
  int model;
  char *model_name;
  int size;
  int block_size;
  int assoc;
  int history_size;
  char *counter_type;
  int counter_model;
  int number_history_sets;
  int index_set_at_bit;
  char *return_stack_type;
  int return_stack_size;
  int track_addresses;
  char *predicate_prediction_type;
  int predicate_prediction_model;
  int max_history_pattern;
  int default_taken_history_pattern;
  int default_not_taken_history_pattern;
  int pure_taken_history_pattern;
  int pure_not_taken_history_pattern;
  int *return_stack;
  int return_stack_pointer;
  int stack_type;
  int predicate_predictor_pred1;
} BTB;


typedef struct BTB_data
{
  int entry_type;                     /* Type of branch */
  int counter;                        /* Used as counter or history pattern */
  int counter2;                       /* Second counter/history for entry   */
  int target;
  int branch_addr;
  int pred_predictor;                 /* Predicate prediction register */
} BTB_data;


/*
 * BTB stats
 */
typedef struct BTB_Stats
{
    unsigned            hits;
    unsigned            miss_pred;
    unsigned            miss_addr;
    unsigned            entries_kicked_out;
    unsigned		dynamic_cond;
    unsigned		dynamic_uncond_pred;
    unsigned		dynamic_call;
    unsigned		dynamic_ret;
    unsigned            miss_pred_cond;
    unsigned            miss_pred_uncond_pred;
    unsigned            miss_pred_call;
    unsigned            miss_pred_ret;
    BTB_addr_node	*cond_address_node;
    BTB_addr_node	*uncond_address_node;
    BTB_addr_node	*call_address_node;
    BTB_addr_node	*ret_address_node;
    BTB_addr_node	*uncond_pred_address_node;
} BTB_Stats;

extern void S_add_stats_btb (BTB_Stats * dest, BTB_Stats * src1,
			     BTB_Stats * src2);
extern void S_print_stats_region_btb (FILE * out, Stats * stats,
				      char *rname, Stats * total_stats);

#endif
