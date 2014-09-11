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
 *      File:   old_sm.h (an early and very preliminary version of the
 *              new SM framework provided to allow current scheduler to 
 *              use .lmdes2).  Used for my MICRO29 paper.
 *      Author: John C. Gyllenhaal
 *      Creation Date:  March 1996
 *      Copyright (c) 1996 John C. Gyllenhaal, Wen-mei Hwu and
 *                         The Board of Trustees of the University of Illinois.
 *                         All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#ifndef OLD_SM_H
#define OLD_SM_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>
#include <library/md.h>
#include <machine/mdes2.h>
#include <machine/sm_mdes.h>

typedef struct OLD_SM_Dep
{
    unsigned char	type;		/* type of dependence arc */
    unsigned char	from_index;	/* index of src operand of dep arc */
    unsigned char	to_index;	/* index of dest operand of dep arc */
    short		omega;		/* dependence distance in iterations */
    short		distance;	/* dependence latency */
    struct OLD_SM_Oper	*from_op;	/* src operation of dep arc */
    struct OLD_SM_Oper	*to_op;		/* dest operation of dep arc */ 
    struct OLD_SM_Dep	*from_next_dep; /* next output dep in src oper */
    struct OLD_SM_Dep	*to_next_dep;   /* next input dep in dest oper */
} OLD_SM_Dep;

typedef struct OLD_SM_Oper
{
    L_Oper		*op;		
    struct OLD_SM_Cb 	*sm_cb;
    unsigned int	flags;
    unsigned int	sched_cycle;
    unsigned int	min_cycle; /* Could store instead before_op/after_op */
    unsigned int	max_cycle; 
    unsigned short	sched_slot;
    unsigned short	min_slot;
    unsigned short	max_slot;
    unsigned short	num_unsched_in;
    unsigned short     	num_unsched_out;
    OLD_SM_Dep		*input_dep;
    OLD_SM_Dep		*output_dep;
    struct OLD_SM_Oper	*next_smop;
    struct OLD_SM_Oper	*prev_smop;
} OLD_SM_Oper;

typedef struct OLD_SM_Cb
{
    struct SM_Mdes	*sm_mdes;
    L_Cb		*cb;
    struct OLD_SM_Func	*sm_func;
    OLD_SM_Oper		*first_sm_op;
    OLD_SM_Oper		*last_sm_op;
    struct OLD_SM_Cb	*next_sm_cb;
    struct OLD_SM_Cb	*prev_sm_cb;

    unsigned int	*map_array;	  /* Resource map array */
    int			map_array_size;	  /* Size of map array (power of 2) */
    int			map_start_offset; /* Offset of map_array[0] */
    int			map_end_offset;   /* Offset of the end of map_array */
    int			min_init_offset;  /* Min usage offset intialized */
    int			max_init_offset;  /* Max usage offset intialized */
} OLD_SM_Cb;

typedef struct OLD_SM_Func
{
    OLD_SM_Cb		*first_sm_cb;
    OLD_SM_Cb		*last_sm_cb;

} OLD_SM_Func;

typedef struct OLD_SM_Stats
{
    int			num_oper_checks;
    int			num_oper_checks_failed;
    int			num_table_checks;
    int			num_table_checks_failed;
    int			num_usage_checks;
    int			num_usage_checks_failed;
    int			num_slot_checks;
    int			num_slot_checks_failed;
} OLD_SM_Stats;

extern OLD_SM_Cb *OLD_SM_new_cb (struct Mdes2 *mdes2, OLD_SM_Func *sm_func, 
				 L_Cb *cb);
extern void OLD_SM_delete_cb (OLD_SM_Cb *sm_cb);
extern int OLD_SM_sched_table (OLD_SM_Cb *sm_cb, SM_Table *table, 
			       int time, unsigned short min_slot, 
			       unsigned short max_slot,
			       int commit, Mdes_Stats *stats);
extern void OLD_SM_print_option (FILE *out, SM_Mdes *sm_mdes, 
				 SM_Option *option);
extern void OLD_SM_print_map (FILE *out, SM_Mdes *sm_mdes, 
			      OLD_SM_Cb *sm_cb);



extern int OLD_SM_choose_first_avail_options (unsigned int *map, SM_Table *table,
                                   unsigned short *choices_made,
                                   unsigned int min_slot,
                                   unsigned int max_slot,
                                   Mdes_Stats *stats);


#endif
