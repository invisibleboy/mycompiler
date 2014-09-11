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
 *	File :		l_dependence.h
 *	Description :	Dependence graph.
 *	Creation Date :	May, 1993
 *	Author : 	Richard E. Hank
 *
 *  Revision 1.1.1.1  1995/08/30 16:49:02  david
 *  Import of IMPACT source
 *
 * Revision 1.1  1994/01/19  18:49:20  roger
 * Initial revision
 *
 *
 *	All rights granted to University of Illinois Board of Regents.
 *==========================================================================*/
#ifndef L_DEPENDENCE_H
#define L_DEPENDENCE_H

/* 10/29/02 REK Adding config.h */
#include <config.h>

/*--------------------------------------------------------------------------*/
/*
 *      DEPENDENCE.
 */
#define LDEP_MODE_ACYCLIC       0       /* include only intra-iteration arcs */
#define LDEP_MODE_CYCLIC        1       /* also include inter-iteration arcs */

#define LDEP_PREPASS		1
#define LDEP_POSTPASS		0

#define L_DEP_ANY               -1      /* match any dependence */
#define L_DEP_NONE              0       /* undefined dependence (for recycle) */
#define L_DEP_REG_FLOW          1       /* register flow dependence */
#define L_DEP_REG_ANTI          2       /* register anti dependence */
#define L_DEP_REG_OUTPUT        3       /* register output dependence */
#define L_DEP_MEM_FLOW          4       /* memory flow dependence */
#define L_DEP_MEM_ANTI          5       /* memory anti dependence */
#define L_DEP_MEM_OUTPUT        6       /* memory output dependence */
#define L_DEP_CNT               7       /* control dependence */
#define L_DEP_SYNC              8       /* synchronization dependence */
#define L_DEP_VLIW		9	/* vliw 0 cycle flow dependence */
 
#define L_DEP_IGNORE_MEM_FLOW_DEP	0x1
#define L_DEP_IGNORE_MEM_OUTPUT_DEP	0x2
#define L_DEP_IGNORE_MEM_ANTI_DEP	0x4
#define L_DEP_IGNORE_ALL_MEMORY_DEP	0x7

#define L_DEP_IGNORE_REG_FLOW_DEP	0x10
#define L_DEP_IGNORE_REG_OUTPUT_DEP	0x20
#define L_DEP_IGNORE_REG_ANTI_DEP	0x40
#define L_DEP_IGNORE_ALL_REG_DEP	0x70

#define L_DEP_IGNORE_CNT_DEP		0x100


#define DEP_HASH_TABLE_SIZE	1024
#define DEP_HASH_MASK		(DEP_HASH_TABLE_SIZE - 1)

#define DEP_MEM_OPERAND		0
#define DEP_CNT_OPERAND		1
#define DEP_SYNC_OPERAND	2
#define DEP_VLIW_OPERAND	3

typedef struct Dep_Operand  {
    unsigned char	ctype;
    unsigned char	type;
    unsigned char	ptype;
    unsigned char	uncond_def;
    int		value;
    int		index;
    struct L_Oper	*oper;
    struct Dep_Operand *next;
    struct Dep_Operand *prev;
} Dep_Operand;

#define DEP_CANT_SPECULATE          -1
#define DEP_NON_EXCEPTING           0
#define DEP_ALWAYS_SAFE             1
#define DEP_CTL_DEP_SAFE            2
#define DEP_COMPLEX_SAFE            3
#define DEP_SILENT                  4
#define DEP_DELAYS_EXCEPTION        5
#define DEP_SAFE_STORE              6

typedef struct L_Dep {
        int      	type;           /* type of dependence arc */
        int             omega;          /* dependence distance in iterations */
        int             distance;       /* dependence latency */
	int 		from_index;	/* index of source operand of dep arc */
	int		to_index;	/* index of dest operand of dep arc */
	struct L_Oper	*from_oper;	/* source instruction of dep arc */
	struct L_Oper	*to_oper;	/* dest instruction of dep arc */
        struct L_Dep    *next_dep;
} L_Dep;
 
typedef struct Dep_Info {
   	int	level;
	int	spec_cond;
	struct L_Oper 	*prev_branch;
	struct L_Oper  *post_branch;
   	int	n_input_dep;
	int     n_output_dep;
   	L_Dep	*input_dep;
	L_Dep 	*output_dep;
	/* dependence internal use only */
	struct L_Oper *oper;
	struct Dep_Info *next;
}  Dep_Info;

#define DEP_INFO(oper)  ((Dep_Info *)(oper->dep_info))

extern int Ldep_branch_perc_limit;
extern int Ldep_alloc_target_size_dep;
extern int Ldep_alloc_target_size_dep_info;
extern int Ldep_alloc_target_size_dep_operand;
extern int Ldep_branch_perc_limit;
extern int Ldep_except_branch_perc_limit;
extern int Ldep_allow_speculative_stores;
extern int Ldep_allow_upward_code_perc;
extern int Ldep_allow_downward_code_perc;
extern int Ldep_remove_always_safe;
extern int Ldep_remove_ctl_dep_safe;
extern int Ldep_remove_complex_safe;
extern int Ldep_hb_keep_branch_order;
extern int Ldep_check_profiled_memory_dependences;
extern int Ldep_allow_lat_dangles_into_jsrs;

extern void     Ldep_mark_safe_invariant(L_Func *fn);
extern void     Ldep_mark_safe(L_Func *fn);
extern void     Ldep_init(Parm_Macro_List *);
extern void     L_build_dependence_graph(L_Cb *, int, int);
extern void	L_delete_dependence_graph(L_Cb *);
extern void 	L_compute_dependence_level(L_Cb *);
extern void	L_delete_dependence_info(Dep_Info *);
extern void	L_add_dep(int, int, int, int, L_Oper *, L_Oper *, int);
extern void     L_remove_dep_pair(int, L_Oper *, L_Oper *);
extern L_Dep 	*L_remove_dep(L_Dep *, int *, int, L_Oper *, L_Oper *);
extern L_Dep 	*L_find_dep(L_Dep *, L_Oper *, L_Oper *, int);
extern Dep_Info *L_new_dep_info();

#endif
