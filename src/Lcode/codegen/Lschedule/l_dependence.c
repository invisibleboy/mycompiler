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
 *	File :		l_dependence.c
 *	Description :	Dependence graph operations.
 *	Author : 	Scott Mahlke/William Chen/Pohua Chang.
 *
 *	Modifications:
 *	Roger A. Bringmann, February 1993 
 *	-Modified to support new Lcode format.  Reduces memory requirements
 *	 for a code generator.  Also, adds a more friendly interface for
 *	 code generation.
 *	Richard E. Hank, May 1993
 *	-Modified for MDES interface, and attempt to make dependence graph
 *	 contstruction faster and more intelligent
 *	Roger A. Bringmann, March 1994 
 *	-Modified to prevent addition of control dependence arcs for
 *	 instructions that are safe to be speculated.
 *      Daniel M. Lavery, March 1995
 *      -Modified to add cross-iteration register and memory dependences
 *      Daniel M. Lavery, May 1996
 *      -Modified to add control dependences for cyclic scheduling
 *
 *	(C) Copyright 1990, Pohua Chang.
 *	All rights granted to University of Illinois Board of Regents.
 *==========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>
#include "l_dependence.h"
#include "l_schedule.h"
/*
 * Dependence analysis parameters
 */
int Ldep_option = 0;
int Ldep_resolve_dependences = 0;
int Ldep_resolve_all_memory_dep = 0;
int Ldep_resolve_all_control_dep = 0;
int Ldep_resolve_all_anti_output_dep = 0;
int Ldep_add_copy_check_dependences = 0;
int Ldep_add_copy_back_dependences = 0;

int Ldep_allow_concurrent_issue = 0;

int Ldep_remove_jsr_dependences = 0;
int Ldep_add_pred_hardware_arcs = 1;

int Ldep_allow_upward_code_perc = 0;
int Ldep_branch_perc_limit = -1;
int Ldep_except_branch_perc_limit = -1;
int Ldep_ignore_live_out = 0;
int Ldep_allow_speculative_stores = 0;

int Ldep_allow_downward_code_perc = 0;

int Ldep_print_dependence_graph = 0;
int Ldep_remove_always_safe = 0;
int Ldep_remove_ctl_dep_safe = 0;
int Ldep_remove_complex_safe = 0;
int Ldep_use_iter = 1;

int Ldep_hb_keep_branch_order = 0;
int Ldep_program_order = 0;

int Ldep_check_profiled_memory_dependences = 1;

int Ldep_allow_lat_dangles_into_jsrs = 1;

int Ldep_debug_memory_disambiguation = 0;
int Ldep_debug_concurrent_issue = 0;
int Ldep_debug_upward_code_perc = 0;
int Ldep_debug_downward_code_perc = 0;
int (*L_conflicting_operands)();

#define DEP_MEM_OPERAND		0
#define	DEP_CNT_OPERAND		1
#define	DEP_SYNC_OPERAND	2
#define DEP_VLIW_OPERAND	3

L_Alloc_Pool *L_alloc_dep = NULL;
L_Alloc_Pool *L_alloc_dep_info = NULL;
L_Alloc_Pool *L_alloc_dep_operand = NULL;

static Set loop_set;

static int Ldep_mode = LDEP_MODE_ACYCLIC;

/*----------------------------------------------------------------------
 *
 * Initialization Routines
 *
 *----------------------------------------------------------------------*/
static void L_read_parm_ldep(Parm_Parse_Info *ppi)
{
    L_read_parm_b(ppi,"resolve_dependences",
		  &Ldep_resolve_dependences);
    L_read_parm_b(ppi,"resolve_all_memory_dep",
		  &Ldep_resolve_all_memory_dep);
    L_read_parm_b(ppi,"resolve_all_control_dep",
		  &Ldep_resolve_all_control_dep);
    L_read_parm_b(ppi,"resolve_all_anti_output_dep",
		  &Ldep_resolve_all_anti_output_dep);

    L_read_parm_b(ppi,"check_profiled_memory_dependences",
		  &Ldep_check_profiled_memory_dependences);
    
    L_read_parm_b(ppi,"add_copy_check_dependences",
		  &Ldep_add_copy_check_dependences);
    L_read_parm_b(ppi,"add_copy_back_dependences",
		  &Ldep_add_copy_back_dependences);
    
    L_read_parm_b(ppi,"allow_concurrent_issue",
		  &Ldep_allow_concurrent_issue);
    L_read_parm_b(ppi,"remove_jsr_dependences",
		  &Ldep_remove_jsr_dependences);
    
    L_read_parm_b(ppi,"add_pred_hardware_arcs",
		  &Ldep_add_pred_hardware_arcs);
    
    L_read_parm_b(ppi,"allow_upward_code_perc",
		  &Ldep_allow_upward_code_perc);
    L_read_parm_i(ppi,"branch_perc_limit",
		  &Ldep_branch_perc_limit);
    L_read_parm_i(ppi,"except_branch_perc_limit",
		  &Ldep_except_branch_perc_limit);
    L_read_parm_b(ppi,"ignore_live_out",
		  &Ldep_ignore_live_out);
    L_read_parm_b(ppi,"allow_speculative_stores",
		  &Ldep_allow_speculative_stores);
    
    L_read_parm_b(ppi,"allow_downward_code_perc",
		  &Ldep_allow_downward_code_perc);
    
    L_read_parm_b(ppi,"print_dependence_graph",
		  &Ldep_print_dependence_graph);
    
    L_read_parm_b(ppi,"remove_always_safe",
		  &Ldep_remove_always_safe);
    L_read_parm_b(ppi,"remove_ctl_dep_safe",
		  &Ldep_remove_ctl_dep_safe);
    L_read_parm_b(ppi,"remove_complex_safe",
		  &Ldep_remove_complex_safe);
    L_read_parm_b(ppi,"use_iter",
		  &Ldep_use_iter);
    
    L_read_parm_b(ppi,"debug_memory_disambiguation",
		  &Ldep_debug_memory_disambiguation);
    L_read_parm_b(ppi,"debug_concurrent_issue",
		  &Ldep_debug_concurrent_issue);
    L_read_parm_b(ppi,"debug_upward_code_perc",
		  &Ldep_debug_upward_code_perc);
    L_read_parm_b(ppi,"debug_downward_code_perc",
		  &Ldep_debug_downward_code_perc); 

    L_read_parm_b(ppi, "hb_keep_branch_order",
                  &Ldep_hb_keep_branch_order);
    L_read_parm_b(ppi, "program_order",
                  &Ldep_program_order);

    L_read_parm_b(ppi, "allow_lat_dangles_into_jsrs",
		  &Ldep_allow_lat_dangles_into_jsrs);
}

void Ldep_init(Parm_Macro_List *command_line_macro_list)
{

    /* Renamed 'Dependence' to 'Ldependence' -JCG 5/26/98 */
    L_load_parameters_aliased(L_parm_file,command_line_macro_list,
		      "(Ldependence", "(Dependence",L_read_parm_ldep);
    
    L_alloc_dep = L_create_alloc_pool("L_Dep",sizeof(struct L_Dep),1024);
    L_alloc_dep_info = L_create_alloc_pool("Dep_Info",sizeof(struct Dep_Info),128);
    L_alloc_dep_operand = L_create_alloc_pool("Dep_Operand",sizeof(struct Dep_Operand),256);

    L_conflicting_operands = M_dep_conflicting_operands();

}


/*----------------------------------------------------------------------
 *
 * Special safe analysis routines
 *
 * Designed to improve scheduling freedom for restricted percolation.
 * However, it also covers general percolation!
 *
 *----------------------------------------------------------------------*/
void Ldep_change_ctl_dep_level_same_iter(L_Oper *load_oper)
{
    int		br_iter, load_iter, br_level, load_level;
    L_Oper 	*oper;
    L_Attr	*attr, *load_attr;


    /*
     * Since this routine is only called for unrolled superblocks,
     * there must be oper levels and iterations as well as
     * branch levels and iterations for this routine to do
     * anything useful.  If we determine that the information
     * is missing, then we will exit the routine without any
     * changes.
     */

    if ((attr = L_find_attr(load_oper->attr, "iter"))!=0)
        load_iter = attr->field[0]->value.i;
    else
	return;

    if ((load_attr = L_find_attr(load_oper->attr, "cd_lev"))!=0)
        load_level = load_attr->field[0]->value.i; /* We need to remember this attr ptr */
    else
        return;
    
    for (oper = load_oper->prev_op; oper != NULL; oper = oper->prev_op)
    {
	if (!L_general_branch_opcode(oper)) continue;

        if ((attr = L_find_attr(oper->attr, "cd_lev"))!=0)
            br_level = attr->field[0]->value.i;
        else
            return;

        if ((attr = L_find_attr(oper->attr, "iter"))!=0)
            br_iter = attr->field[0]->value.i;
        else
	    return;

	if ((br_iter == load_iter) && (load_level < br_level))
	{
	    load_attr->field[0]->value.i = br_level;
	}
	else if (br_iter < load_iter) 
	    return;
    }
}

int Ldep_invariant_oper(L_Cb *cb, L_Oper *load_oper)
{
    int 	i, j;
    L_Oper 	*oper;
    L_Operand	*dest;

    for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
    {
	for (i = 0; i < L_max_dest_operand; i++)
	{
	    dest = oper->dest[i];

	    if (dest==NULL) continue;

	    for (j = 0; j < L_max_src_operand; j++)
	    {
		if (L_same_operand(dest, load_oper->src[j]))
		    return 0;
	    }
	}
    }
    return 1;
}
/* Returns 1 if def_operand will be defined even if the operation is predicate
 * squashed.  Currently, only unconditionally defined predicate operands should
 * return 1.  Otherwise, 0 is returned.  
 *
 * Assume passed only registers (or macros).
 */
static int Ldep_uncond_def (L_Operand *def_operand)
{
    /* Don't expect to be NULL, so punt if is */
    if (def_operand == NULL)
	L_punt ("L_uncond_pred: NULL operand not expected!");

    /* Currently, only unconditionally defined predicates should return 1 */
    if (L_is_ctype_predicate (def_operand) &&
	(L_uncond_ptype (def_operand->ptype)))
	return (1);

    /* If got here, not a uncond def */
    return (0);
}

/* Returns 1 if dep_dest1 and dest2 may be legally reordered.  Currently,
 * only two OR type predicate defs or two AND type predicate defs can be
 * legally reordered.  Otherwise, 0 is returned.
 */
static int Ldep_defs_may_reorder (Dep_Operand *dep_dest1, L_Operand *dest2)
{
    /* If not both predicate definitions, cannot reorder */
    if ((dep_dest1->ctype != L_CTYPE_PREDICATE) || 
	(dest2->ctype != L_CTYPE_PREDICATE))
	return (0);

    /* If both are OR type predicates, may reorder */
    if (L_or_ptype(dep_dest1->ptype) &&
	L_or_ptype(dest2->ptype))
	return (1);

    /* If both are AND type predicates, may reorder */
    if (L_and_ptype(dep_dest1->ptype) &&
	L_and_ptype(dest2->ptype))
	return (1);

    /* If both are SAND type predicates, may reorder */
    if (L_sand_ptype(dep_dest1->ptype) &&
	L_sand_ptype(dest2->ptype))
	return (1);

    /* Otherwise, may not reorder */
    return (0);
}

void Ldep_mark_safe_invariant(L_Func *fn)
{
    int 		load_level, header_level;
    L_Oper		*oper;
    L_Cb		*cb;
    L_Attr		*attr;
    L_Loop		*loop;

    if (!Ldep_remove_ctl_dep_safe) return;

    /* 
     * If we are going to do control dependence safe speculation, we
     * need knowledge of what cb's are in what loops.
     */
    L_loop_detection(fn, 0);

    loop_set = NULL;
    for (loop = fn->first_loop; loop!=NULL; loop=loop->next_loop)
        loop_set = Set_union_acc(loop_set, loop->loop_cb);

    if (!Ldep_remove_always_safe ) return;

#if 0
    if (!Ldep_remove_complex_safe ) return;
#endif

    /* THIS STUFF IS USELESS WITHOUT BOTH CTL_DEP_SAFE AND ALWAYS SAFE */

    /* 
     * We are currently only looking at unrolled loops!
     *
     * Mark invarient loads that have the same control dependence level as the
     * superblock loop header as safe.  This is legal since we never speculate
     * outside the superblock.  
     *
     * BEWARE RICK!!!  NOT GOOD FOR REGION STUFF!
     */

    for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
	if (!L_EXTRACT_BIT_VAL(cb->flags, L_CB_UNROLLED))
	    continue;

	header_level = -1;
	for (oper = cb->first_op; oper!= NULL; oper = oper->next_op)
	{
	    if (L_general_branch_opcode(oper))
	    {
        	if ((attr = L_find_attr(oper->attr, "cd_lev"))!=0)
            	    header_level = attr->field[0]->value.i;
		break;
	    }
	}

	if (header_level == -1) continue;

	for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
	{
	    if (L_EXTRACT_BIT_VAL(oper->flags, L_OPER_SAFE_PEI))
		continue;

	    if (L_load_opcode(oper))
	    {
        	if ((attr = L_find_attr(oper->attr, "cd_lev"))!=0)
            	    load_level = attr->field[0]->value.i;
        	else
		    continue;
		
		if (load_level <= header_level) 
		{
		    if (Ldep_invariant_oper(cb, oper))
		    {
		        oper->flags = L_SET_BIT_FLAG(oper->flags, 
			    L_OPER_SAFE_PEI);
		    }
		}
	    }
	}
    }
}

int L_safe_operands(L_Operand *reg_src, L_Operand *reg_dest,
    L_Operand *immed_src, L_Operand *immed_dest)
{
    if (!L_is_register(reg_src) && !L_is_macro(reg_src))
	return 0;

    if (!L_is_register(reg_dest) && !L_is_macro(reg_dest))
	return 0;

    if (!L_is_constant(immed_src) && !L_is_constant(immed_dest))
	return 0;

    if (!L_same_operand(reg_src, reg_dest))
	return 0;

    return 1;
}

int L_safe_load(L_Oper *load_oper, L_Oper *br_oper, L_Cb *cb)
{
    int		same_addr;
    L_Oper 	*oper;

    same_addr = 0;

    /* Find a memory reference to the same address before the branch */
    for (oper = cb->first_op; oper != br_oper ; oper = oper->next_op)
    {
	if (L_general_load_opcode(oper) ||
	    L_general_store_opcode(oper))
	{
#if 0
	    if (!L_EXTRACT_BIT_VAL(oper->flags, L_OPER_SAFE_PEI))
		continue;
#endif

	    if (L_safe_operands(oper->src[1], load_oper->src[1],
		oper->src[0], load_oper->src[0]) &&
		L_no_defs_between(oper->src[1], oper, load_oper) &&
		!L_same_operand(oper->dest[0], load_oper->src[1]))
	    {
		same_addr = 1;
		break;
	    }

	    if (L_safe_operands(oper->src[0], load_oper->src[0],
		oper->src[1], load_oper->src[1]) &&
		L_no_defs_between(oper->src[0], oper, load_oper) &&
		!L_same_operand(oper->dest[0], load_oper->src[0]))
	    {
		same_addr = 1;
		break;
	    }

	    if (L_safe_operands(oper->src[0], load_oper->src[1],
		oper->src[1], load_oper->src[0]) &&
		L_no_defs_between(oper->src[0], oper, load_oper) &&
		!L_same_operand(oper->dest[0], load_oper->src[1]))
	    {
		same_addr = 1;
		break;
	    }

	    if (L_safe_operands(oper->src[1], load_oper->src[0],
		oper->src[0], load_oper->src[1]) &&
		L_no_defs_between(oper->src[1], oper, load_oper) &&
		!L_same_operand(oper->dest[0], load_oper->src[0]))
	    {
		same_addr = 1;
		break;
	    }
	}
    }

    return same_addr;
}

void Ldep_mark_safe(L_Func *fn)
{
    Ldep_mark_safe_invariant(fn);
}

/*
 * An instruction is safe to speculate above any branch if the instruction
 * is always safe.
 */
int L_is_always_safe(L_Oper *oper)
{
    if (!Ldep_remove_always_safe) return 0;

    if (!oper) return 0;

    if (L_EXTRACT_BIT_VAL(oper->flags, L_OPER_SAFE_PEI))
        return 1;
    else
        return 0;
}

/*
 * An instruction is safe to speculate above the target branch if
 * it's control dependence level <= the control dependence level of
 * the branch.  (The condition is assuming that only branches that
 * are on the control path to the oper are being used.)
 */
int L_is_ctl_dep_safe (L_Oper *oper, L_Oper *br, L_Cb *cb)
{
    L_Attr	*attr;
    int         oper_level, br_level, oper_iter, br_iter;


    return 0;

    if (!Ldep_remove_ctl_dep_safe) return 0;

    /*
     * We can only handle loops right now if they are unrolled
     * superblock loops.  Therefore, we will skip all other
     * cb's that are not of this class.
    if (Set_in(loop_set, cb->id)) return 0;
     */

    if (L_general_branch_opcode(oper)) return 0;

    if (!oper || !br) return 0;

    if (L_general_branch_opcode(br))
    {
        if ((attr = L_find_attr(oper->attr, "cd_lev"))!=0)
            oper_level = attr->field[0]->value.i;
        else
            return 0;

        if ((attr = L_find_attr(br->attr, "cd_lev"))!=0)
            br_level = attr->field[0]->value.i;
        else
            return 0;

        if ((attr = L_find_attr(oper->attr, "iter"))!=0)
            oper_iter = attr->field[0]->value.i;
        else
            oper_iter = 0;
	
        if ((attr = L_find_attr(br->attr, "iter"))!=0)
            br_iter = attr->field[0]->value.i;
        else
            br_iter = 0;
	
        if ((oper_level <= br_level) && 
	    ((Ldep_use_iter && (oper_iter == br_iter)) ||
	     (!Ldep_use_iter)))
            return 1;
        else
            return 0;
    }
    else
        L_punt("L_is_safe_to_speculate_above: branch must be second operand");
}

/*
 * A safe divide is one whose divisor can be determined to be a non-zero constant
 * along all paths that reach the divide.
 */
int L_safe_divide(L_Oper *oper, L_Oper *br, L_Cb *cb)
{
    return 0;
}

/*
 * A safe remainder is one whose divisor can be determined to be a non-zero constant
 * along all paths that reach the remainder.
 */
int L_safe_rem(L_Oper *oper, L_Oper *br, L_Cb *cb)
{
    return 0;
}

/*
 * An instruction is safe to speculate above any branch if the instruction
 * is always safe.  It is assumed that the less restrictive safety tests have
 * already been performed!
 */
int L_is_complex_safe(L_Oper *oper, L_Oper *br, L_Cb *cb)
{
    if (!Ldep_remove_complex_safe) return 0;

    if (!oper) return 0;

    if (L_load_opcode(oper))
	return L_safe_load(oper, br, cb);

    if (L_int_div_opcode(oper))
	return L_safe_divide(oper, br, cb);

    if (L_int_rem_opcode(oper))
	return L_safe_rem(oper, br, cb);


    return 0;
}

/*----------------------------------------------------------------------
 *
 * Dependence Analysis Hash Table Routines
 *
 *----------------------------------------------------------------------*/

/*==========================================
    Dependence Analysis Hash Table Routines
 *==========================================*/

static Dep_Operand *Dep_Hash_Table[DEP_HASH_TABLE_SIZE];

static void L_init_dep_hash_table()
{
    int i;
    for ( i = 0; i < DEP_HASH_TABLE_SIZE; i++ )
	Dep_Hash_Table[i] = NULL;
}

static void L_reset_dep_hash_table()
{
    int i;
    
    for ( i = 0; i < DEP_HASH_TABLE_SIZE; i++ )  {
	Dep_Operand *dep_operand = Dep_Hash_Table[i];
	while ( dep_operand )  {
	    Dep_Operand *tmp = dep_operand->next;
	    L_free(L_alloc_dep_operand,dep_operand);
	    dep_operand = tmp;
	}
	Dep_Hash_Table[i] = NULL;
    }
}

static Dep_Operand *L_find_dep_operand(L_Operand *operand)
{
    unsigned int hash_val;
    Dep_Operand *dep_operand;

    hash_val = operand->value.r & DEP_HASH_MASK;
    dep_operand = Dep_Hash_Table[hash_val];
    
    while ( dep_operand != NULL )  {
	if ( operand->value.r == dep_operand->value &&
	     L_return_old_ctype(operand) == dep_operand->ctype &&
	     L_return_old_type(operand) == dep_operand->type )
 	         return(dep_operand);
	dep_operand = dep_operand->next;
    }
    
    return(NULL);
}

/*
 *  When building the dependence graph for a hyperblock, a virtual/macro
 *  register may have more than one entry in the hash table 
 */
static Dep_Operand *L_find_another_dep_operand(Dep_Operand *dep)
{
    Dep_Operand *dep_operand = dep->next;
    
    while ( dep_operand != NULL )  {
	if ( dep_operand->value == dep->value &&
	     dep_operand->ctype == dep->ctype &&
	     dep_operand->type == dep->type )
	         return(dep_operand);
	dep_operand = dep_operand->next;
    }
    return(NULL);
}

static void L_insert_dep_operand(int index, L_Operand *operand, L_Oper *oper)
{
    unsigned int hash_val;
    Dep_Operand *dep_operand, *table_list;
    
    hash_val = operand->value.r & DEP_HASH_MASK;

    dep_operand = (Dep_Operand *)L_alloc(L_alloc_dep_operand);
    dep_operand->type = L_return_old_type(operand);
    dep_operand->ctype = L_return_old_ctype(operand);
    dep_operand->ptype = operand->ptype;
    dep_operand->value = operand->value.r;
    dep_operand->index = index;
    dep_operand->oper = oper;

    /* Flag if an unconditional defintion of this dest register (indepenent
     * of oper's predicate. -JCG 1/27/98
     */
    dep_operand->uncond_def = Ldep_uncond_def (operand);
    
    table_list = Dep_Hash_Table[hash_val];
    if ( table_list == NULL )  {
	dep_operand->next = NULL;
    }
    else  {
	table_list->prev = dep_operand;
    	dep_operand->next = table_list;
    }
    dep_operand->prev = NULL;
    Dep_Hash_Table[hash_val] = dep_operand;
}

static void L_remove_dep_operand(Dep_Operand *dep_operand)
{
    Dep_Operand *next = dep_operand->next;
    Dep_Operand *prev = dep_operand->prev;
    
    if ( next )
    	if ( prev )  {
 	    next->prev = prev;
	    prev->next = next; 
	}
        else  {
	    Dep_Hash_Table[dep_operand->value & DEP_HASH_MASK] = next;
	    next->prev = NULL;
	}
    else {
	if ( prev )
	    prev->next = NULL;
	else
	    Dep_Hash_Table[dep_operand->value & DEP_HASH_MASK] = NULL;
    }
    L_free(L_alloc_dep_operand,dep_operand);
}

/*==========================================
    Dependence Arc Management Routines
 *==========================================*/

static void 
L_add_input_dep(int type, int dist, int from_index, int to_index, 
		L_Oper *from_oper, L_Oper *to_oper, int omega)
{
    /* initalize new dependence arc */
    L_Dep *dep = (L_Dep *)L_alloc(L_alloc_dep);
    dep->type = type;
    dep->distance = dist;
    dep->omega = omega;
    dep->from_index = from_index;
    dep->to_index = to_index;
    dep->from_oper = from_oper;
    dep->to_oper = to_oper;
    
    /* Add dependence to input dependence list of <to_oper> */
    dep->next_dep = DEP_INFO(to_oper)->input_dep;
    DEP_INFO(to_oper)->input_dep = dep;
    DEP_INFO(to_oper)->n_input_dep += 1;
}

static void 
L_add_output_dep(int type, int dist, int from_index, int to_index, 
		L_Oper *from_oper, L_Oper *to_oper, int omega)
{
    /* initalize new dependence arc */
    L_Dep *dep = (L_Dep *)L_alloc(L_alloc_dep);
    dep->type = type;
    dep->distance = dist;
    dep->omega = omega;
    dep->from_index = from_index;
    dep->to_index = to_index;
    dep->from_oper = from_oper;
    dep->to_oper = to_oper;
    
    /* Add dependence to output dependence list of <from_oper> */
    dep->next_dep = DEP_INFO(from_oper)->output_dep;
    DEP_INFO(from_oper)->output_dep = dep;
    DEP_INFO(from_oper)->n_output_dep += 1;
}

void L_add_dep(int type, int dist, int from_index, int to_index,
		L_Oper *from_oper, L_Oper *to_oper, int omega) 
{
    L_Dep *dep;

    /* Determine if the dependence already exists */
    for (dep = DEP_INFO(from_oper)->output_dep; dep != NULL; dep = dep->next_dep)
    {
	if ( (dep->type == type) && 
	     (dep->distance == dist) &&
	     (dep->omega == omega) &&
	     (dep->from_index == from_index) &&
	     (dep->to_index == to_index) &&
	     (dep->from_oper == from_oper) &&
	     (dep->to_oper == to_oper) )
	    return;
    }

    /* If we have reached this point, there is no existing dependence */
    L_add_output_dep(type, dist, from_index, to_index, from_oper, to_oper, omega);
    L_add_input_dep(type, dist, from_index, to_index, from_oper, to_oper, omega);
}

L_Dep *L_find_dep(L_Dep *dep, L_Oper *from_oper, L_Oper *to_oper, int type)
{
    L_Dep *ptr;
    
    for ( ptr = dep; ptr != NULL ; ptr = ptr->next_dep ) {
	if ( ptr->type == type &&
	     ptr->from_oper == from_oper &&
	     ptr->to_oper == to_oper )
	    return(ptr);
    }
    return(NULL);
}

L_Dep *L_remove_dep(L_Dep *dep, int *count, int type, L_Oper *from_oper, 
		    L_Oper *to_oper)
{
    
    /* NEED to alter the count */
    
    L_Dep *head,*prev,*cur,*next;

    prev = NULL;
    head = cur = dep;
    
    while ( cur )  {
	/* if dependence is not of correct type, keep looking */
	if ( cur->type != type )  {
	    if ( head != cur )
	        prev = cur;
	    else
	        prev = head;
	    cur = cur->next_dep;
	    continue;
	}
	/* if from(to) opers of dependence arc match, delete it */
	if ( (!from_oper || (from_oper == cur->from_oper))  &&
	     (!to_oper	 || (to_oper == cur->to_oper)) )  {
	    next = cur->next_dep;
	    if ( cur != head )  {
		prev->next_dep = cur->next_dep;
	    	L_free(L_alloc_dep,cur);
		(*count) -= 1;
		cur = next;
	    }
	    else  {
		L_free(L_alloc_dep,cur);
		(*count) -= 1;
		head = cur = next;
		prev = NULL;
	    }
	    continue;
	}
	if ( head != cur )
	    prev = cur;
	else
	    prev = head;
	cur = cur->next_dep;
    }
    return(head);
}

/* remove dependence arc from list at both from_oper and to_oper */
void L_remove_dep_pair(int type, L_Oper *from_oper, L_Oper *to_oper)
{
  Dep_Info *dep_info;
  L_Dep *dep;
  int *count;

  /* remove dependence from from_oper's list */
  dep_info = DEP_INFO(from_oper);
  dep = dep_info->output_dep;
  count = &(dep_info->n_output_dep);
  dep_info->output_dep = L_remove_dep(dep, count, type, from_oper, to_oper);

  /* remove dependence from to_oper's list */
  dep_info = DEP_INFO(to_oper);
  dep = dep_info->input_dep;
  count = &(dep_info->n_input_dep);
  dep_info->input_dep = L_remove_dep(dep, count, type, from_oper, to_oper);
}

/*------------------------------------------------------------------------------*/


/*==========================================
    Dependence Predicate Functions 
 *==========================================*/

static int L_same_register_operand(L_Operand *op1, L_Operand *op2)
{
    if (!L_operand_type_same(op1, op2))
        return 0;
   
    if (!L_operand_ctype_same(op1,op2))
        return 0;


    if (L_is_reg(op1))
	return (op1->value.r==op2->value.r);
    else
	return (op1->value.mac==op2->value.mac);
}

/*
 * Returns 1 if the oper can safely be moved above/below a jsr.
 * This is valid only during prepass.
 * 
 * The requirements for safe moving above/below a jsr are:
 * 1) Does not use/alter fragile macros.
 * 2) May be a memory oper if the jsr is "safe"
 *
 */
static int L_jsr_independent_oper (L_Oper *jsr, L_Oper *oper)
{
    int i;
    int safe;

    if (L_EXTRACT_BIT_VAL(jsr->flags, L_OPER_SYNC) ||
	L_EXTRACT_BIT_VAL(oper->flags, L_OPER_SYNC))
	return (0);

	/* if the function contains valid sync arcs
		for jsrs, then use this info instead
		of the l_safe.c functions */
    if (L_func_contains_jsr_dep_pragmas &&
	L_use_sync_arcs && 
	(L_general_load_opcode (oper) ||
	 L_general_store_opcode (oper) ) ) {

	safe = L_sync_no_jsr_dependence(jsr,oper);
    }
    else {
	safe = L_side_effect_free_sub_call(jsr);
    }



    /* Must be a arithmetic instruction only (Don't use &) */
    if ( !op_flag_set(oper->proc_opc, OP_FLAG_CBR | OP_FLAG_JMP |
	    			      OP_FLAG_RTS | OP_FLAG_JSR |
				      OP_FLAG_SYNC | OP_FLAG_CHK |
		      		      OP_FLAG_LOAD | OP_FLAG_STORE) ||
	 ( safe && op_flag_set(oper->proc_opc, OP_FLAG_LOAD) ) )  {
	/*
    if ( L_general_arithmetic_opcode(oper) ||
	 L_move_opcode(oper) ||
	 (safe && (L_general_load_opcode(oper) ||
		   L_general_store_opcode(oper))) )  {
		   */
	for ( i = 0; i < L_max_dest_operand; i++ )  {
	    L_Operand *dest = oper->dest[i];
	    if ( dest != NULL && L_is_fragile_macro(dest) )
		return(0);
	}
	for ( i = 0; i < L_max_src_operand; i++ )  {
	    L_Operand *src = oper->src[i];
	    if ( src != NULL && L_is_fragile_macro(src) )
		return(0);
	}
	/* Only one predicated supported for now - RAB */
	for ( i = 0; i < 1 /* L_max_pred_operand */; i++ )  {
	    L_Operand *pred = oper->pred[i];
	    if ( pred != NULL && L_is_fragile_macro(pred) )
		return(0);
	}
	return(1);
    }
    return(0);
}

static int L_safe_to_move_below_branch(L_Cb *cb, L_Oper *oper, L_Oper *branch)
{
    int i,not_safe = 0;
    
     /* check if safe to percolate downwards  */	
    if ( op_flag_set(oper->proc_opc, OP_FLAG_CBR | OP_FLAG_JMP | 
			             OP_FLAG_RTS | OP_FLAG_JSR |
		      		     OP_FLAG_CHK | OP_FLAG_SYNC) )
	return(0);

    if ( op_flag_set(oper->proc_opc, OP_FLAG_STORE) && 
         (Ldep_mode == LDEP_MODE_ACYCLIC) ) 
        return(0);

    if (L_EXTRACT_BIT_VAL(oper->flags, L_OPER_SYNC))
	return (0);

    if ( Ldep_ignore_live_out )
        return(1);
    
    for (i=0; i< L_max_dest_operand; i++)
        if ( L_in_oper_OUT_set(cb, branch, oper->dest[i], TAKEN_PATH) &&
             (Ldep_mode == LDEP_MODE_ACYCLIC) )  {
	    not_safe = 1;
	    break;
	}
    return(!not_safe);
}

/* Added hyperblock version to support unconditional predicate defs -JCG 1/27/98 */
static int L_safe_to_move_below_branch_hb(L_Cb *cb, L_Oper *oper, L_Oper *branch)
{
    int i,not_safe = 0;
    int intersecting_predicates;


    /* Only need to test most constraints if branch's and oper's predicates can be 1
     * at the same time. -JCG 1/27/98
     */
    intersecting_predicates = PG_intersecting_predicates_ops(branch, oper);
    if (intersecting_predicates)
    {
	/* check if safe to percolate downwards  */	
	if ( op_flag_set(oper->proc_opc, OP_FLAG_CBR | OP_FLAG_JMP | 
			 OP_FLAG_RTS | OP_FLAG_JSR |
			 OP_FLAG_CHK | OP_FLAG_SYNC) )
	    return(0);
	
	if ( op_flag_set(oper->proc_opc, OP_FLAG_STORE) && 
	     (Ldep_mode == LDEP_MODE_ACYCLIC) ) 
	    return(0);
	
	if (L_EXTRACT_BIT_VAL(oper->flags, L_OPER_SYNC))
	    return (0);
    }

    if ( Ldep_ignore_live_out )
        return(1);
    
    for (i=0; i< L_max_dest_operand; i++)
    {
        if ( L_in_oper_OUT_set(cb, branch, oper->dest[i], TAKEN_PATH) &&
             (Ldep_mode == LDEP_MODE_ACYCLIC) )  
	{
	    /* Unsafe if operation's predicates can both be 1
	     * at the same time or if register's def is unconditional
	     * (independent of the predicate's value). -JCG 1/27/98
	     */
	    if (intersecting_predicates || Ldep_uncond_def (oper->dest[i]))
	    {
		not_safe = 1;
		break;
	    }
	}
    }
    return(!not_safe);
}

static int L_safe_to_move_above_branch(L_Cb *cb, L_Oper *oper, L_Oper *branch)
{
    int i;

    /* check if safe to percolate upwards  */	
    if ( op_flag_set(oper->proc_opc, OP_FLAG_CBR | OP_FLAG_JMP | 
	 OP_FLAG_RTS | OP_FLAG_JSR | OP_FLAG_CHK | OP_FLAG_SYNC | 
	 OP_FLAG_NOSPEC ) )
	return DEP_CANT_SPECULATE;

    if (L_EXTRACT_BIT_VAL(oper->flags, L_OPER_SYNC))
	return DEP_CANT_SPECULATE;

    /* An instruction can not be speculated if it is in the live out of the 
       branch except in CYCLIC mode. */
    if ( !Ldep_ignore_live_out && (Ldep_mode == LDEP_MODE_ACYCLIC)) 
        for (i=0; i< L_max_dest_operand; i++)
            if ( L_in_oper_OUT_set(cb, branch, oper->dest[i], TAKEN_PATH) )  
	        return DEP_CANT_SPECULATE;
    
    /* Determine possible reasons why an instruction could be speculated */
    if (!op_flag_set(oper->proc_opc,OP_FLAG_EXCEPT))
	return DEP_NON_EXCEPTING;

    if (op_flag_set(oper->proc_opc,OP_FLAG_STORE))
      {
	if (Ldep_allow_speculative_stores)
	    return DEP_SAFE_STORE;
	else
	    return DEP_CANT_SPECULATE;
      }

    if (Ldep_mode == LDEP_MODE_ACYCLIC) {
        if (L_is_always_safe(oper)) 
	    return DEP_ALWAYS_SAFE;
    }

    if (Ldep_mode == LDEP_MODE_ACYCLIC) {
        if (L_is_complex_safe(oper, branch, cb))
	    return DEP_COMPLEX_SAFE;
    }

    if (Ldep_mode == LDEP_MODE_ACYCLIC) {
        if (L_is_ctl_dep_safe(oper, branch, cb)) 
	    return DEP_CTL_DEP_SAFE;
    }

    if (any_alt_flag_set(MDES_INFO(oper), ALT_FLAG_SILENT))
    {
	if ( (L_spec_model == GENERAL) || (L_spec_model == MCB) )
	    return DEP_SILENT;

	if ((L_spec_model == WBS) || (L_spec_model == SENTINEL) ||
	    (L_spec_model == SRB))
	    return DEP_DELAYS_EXCEPTION;
    }

    return DEP_CANT_SPECULATE;

}

static int L_safe_to_move_above_branch_hb(L_Cb *cb, L_Oper *oper, L_Oper *branch)
{
    int i,not_safe = 0;
    int intersecting_predicates;


    /* Only need to test most constraints if branch's and oper's predicates can be 1
     * at the same time. -JCG 1/27/98
     */
    intersecting_predicates = PG_intersecting_predicates_ops(branch, oper);
    if (intersecting_predicates)
    {
	/* check if safe to percolate upwards  */
	if ( op_flag_set(oper->proc_opc, OP_FLAG_CBR | OP_FLAG_JMP |
			 OP_FLAG_RTS | OP_FLAG_JSR |
			 OP_FLAG_CHK | OP_FLAG_SYNC) )
	    return(0);
	
	if (L_EXTRACT_BIT_VAL(oper->flags, L_OPER_SYNC))
	    return (0);
	
	/* If an instruction causes an exception, and does not have a speculative
	   version, the instruction may not be speculated                           */
	if (!L_is_always_safe(oper) &&
	    op_flag_set(oper->proc_opc,OP_FLAG_EXCEPT) &&
	    !any_alt_flag_set(MDES_INFO(oper),ALT_FLAG_SILENT))
	    return(0);
	
	/* We know the store instruction has a speculative version, and it may be
	   speculated give the appropriate hardware support */
	if ( op_flag_set(oper->proc_opc,OP_FLAG_STORE) )
	  {
	    if ( Ldep_allow_speculative_stores )
		return(1);
	    else
		return(0);
	  }
    }

    /* The instruction may or may not be excepting, but if it is excepting, we
       now know it has a speculative version.  Thus if we are ignoring live out
       the instruction may be speculated */
    if ( Ldep_ignore_live_out )
        return(1);

    for (i=0; i< L_max_dest_operand; i++) {
	if (oper->dest[i]==NULL)
	    continue;
        if ( L_in_oper_OUT_set(cb, branch, oper->dest[i], TAKEN_PATH) )  
	{
	    /* Unsafe if operation's predicates can both be 1
	     * at the same time or if register's def is unconditional
	     * (independent of the predicate's value). -JCG 1/27/98
	     */
	    if (intersecting_predicates || Ldep_uncond_def (oper->dest[i]))
	    {
		not_safe = 1;
		break;
	    }
        }
    }
    return(!not_safe);

}

/*------------------------------------------------------------------------------*/

/*===============================================================================
 *
 *  Basic Block and Superblock dependence generation functions
 *
 *===============================================================================*/
static void L_compute_register_dependence(L_Cb *cb, int prepass)
{
    int i,j,to_index,from_index,distance;
    int n_conflict;
    L_Operand *conflict[64];
    L_Oper *oper;
    Dep_Operand *dep;
    Mdes_Info *mdes_info = NULL;
    L_Attr *attr;
    
    L_init_dep_hash_table();
    
    for ( oper = cb->first_op; oper != NULL; oper = oper->next_op )  {
        if ( !(DEP_INFO(oper)) ) continue;
	
	mdes_info = MDES_INFO(oper);
	
        /*
     	 *  	1. Compute register flow dependences.
     	 */  

	/* flow deps for source operands */
	for ( i = 0; i < L_max_src_operand; i++ )  {
	    L_Operand *src;
	    if ( (src = oper->src[i]) == NULL ) continue;
	    
	    if (!(L_is_reg(src) ||
		  L_is_macro(src)))
		continue;
	    
	    /* determine conflicting operands */
	    n_conflict = L_conflicting_operands(src,conflict,64,prepass);
	    for ( j = 0 ; j < n_conflict; j++ )  {
	        /* does the operand exist in hash table ?? */
		dep = L_find_dep_operand(conflict[j]);
		if ( dep == NULL ) continue;
		
		/* calculate flow dependence distance, minimum distance = 0 */
		to_index = operand_index(MDES_SRC,i);
		distance = min_operand_time(MDES_INFO(dep->oper),dep->index) -
		    	   max_operand_time(mdes_info,to_index);
		if ( distance < 0 ) distance = 0;
		
	        /* yes, add flow dependence from dep->oper to oper */
	        L_add_output_dep(L_DEP_REG_FLOW,distance,dep->index,to_index,
					dep->oper,oper,0);
	        L_add_input_dep(L_DEP_REG_FLOW,distance,dep->index,to_index,
				       dep->oper,oper,0);
	    }
	    for (j = 0; j < n_conflict; j++) {
		L_delete_operand(conflict[j]);
	    }
	}

	/* flow deps for pred operands. Only one predicated supported for now - RAB */
	for ( i = 0; i < 1 /* L_max_pred_operand */; i++ )  {
	    L_Operand *pred;
	    if ( (pred = oper->pred[i]) == NULL ) continue;
	    
	    if (!(L_is_reg(pred) || L_is_macro(pred)))
		continue;
	    
	    /* determine conflicting operands */
	    n_conflict = L_conflicting_operands(pred,conflict,64,prepass);
	    for ( j = 0 ; j < n_conflict; j++ )  {
	        /* does the operand exist in hash table ?? */
		dep = L_find_dep_operand(conflict[j]);
		if ( dep == NULL ) continue;
	    
		/* calculate flow dependence distance, minimum distance = 0 */
		to_index = operand_index(MDES_PRED,i);
		distance = min_operand_time(MDES_INFO(dep->oper),dep->index) -
		    	   max_operand_time(mdes_info,to_index);
		if ( distance < 0 ) distance = 0;
		 
	        /* yes, add output depedence from dep->oper to oper */
	        L_add_output_dep(L_DEP_REG_FLOW,distance,dep->index,to_index,
					dep->oper,oper,0);
	        L_add_input_dep(L_DEP_REG_FLOW,distance,dep->index,to_index,
				       dep->oper,oper,0);
	    }
	    for (j = 0; j < n_conflict; j++) {
		L_delete_operand(conflict[j]);
	    }
	}

        /* SAM 8-95, flow deps for implicit JSR/RTS src operands */
        if ((Ldep_allow_lat_dangles_into_jsrs == 0) &&
	        (op_flag_set(oper->proc_opc, OP_FLAG_JSR) ||
	         op_flag_set(oper->proc_opc, OP_FLAG_RTS))) {
	    attr = L_find_attr(oper->attr, "tr");
	    if (attr!=NULL) {
		for (i=0; i<attr->max_field; i++) {
		    L_Operand *macro;
		    macro = attr->field[i];
		    if (macro==NULL)
			continue;
		    if (!L_is_macro(macro))
			continue;
		    /* determine conflicting operands */
		    n_conflict = L_conflicting_operands(macro,conflict,64,prepass);
		    for ( j = 0 ; j < n_conflict; j++ )  {
                	/* does the operand exist in hash table ?? */
                	dep = L_find_dep_operand(conflict[j]);
                	if ( dep == NULL ) continue;

			/* calculate flow dependence distance, minimum dist= 0,
			   use DEP_SYNC_OPERAND as to index as a shortcut, so I
			   don't need to declare a new sync operand */
                	to_index = operand_index(MDES_SYNC_IN,DEP_SYNC_OPERAND);
			distance = min_operand_time(MDES_INFO(dep->oper),dep->index) -
                                   max_operand_time(mdes_info,to_index);
			if ( distance < 0 ) distance = 0;

	        	/* yes, add output depedence from dep->oper to oper */
	        	L_add_output_dep(L_DEP_REG_FLOW,distance,dep->index,to_index,
						dep->oper,oper,0);
	        	L_add_input_dep(L_DEP_REG_FLOW,distance,dep->index,to_index,
				       	dep->oper,oper,0);
		    }
		    for (j = 0; j < n_conflict; j++) {
			L_delete_operand(conflict[j]);
		    }
		}
	    }
	}

	/*
     	 *	2. Compute register output dependences.
     	 */

	/* output deps for dest operands */
	for ( i = 0; i < L_max_dest_operand; i++ )  {
	    L_Operand *dest;
	    if ( (dest = oper->dest[i]) == NULL ) continue;
	    
	    if (!(L_is_reg(dest) || L_is_macro(dest)))
		continue;
	    
	    n_conflict = L_conflicting_operands(dest,conflict,64,prepass);
	    for ( j = 0 ; j < n_conflict; j++ )  {
	        /* does the operand exist in hash table ?? */
		dep = L_find_dep_operand(conflict[j]);
		if ( dep == NULL ) continue;
	    
		/* calculate output dependence distance, minimum distance = 0 */
		to_index = operand_index(MDES_DEST,i);
		distance = min_operand_time(MDES_INFO(dep->oper),dep->index) -
		    	   max_operand_time(mdes_info,to_index);
		if ( distance < 0 ) distance = 0;
		
	        /* yes, add output depedence from dep->oper to oper */
	        L_add_output_dep(L_DEP_REG_OUTPUT,distance,dep->index,to_index,
					dep->oper,oper,0);
	        L_add_input_dep(L_DEP_REG_OUTPUT,distance,dep->index,to_index,
				       dep->oper,oper,0);
	    }	
	    for (j = 0; j < n_conflict; j++) {
		L_delete_operand(conflict[j]);
	    }
	}

        /* SAM 8-95, output deps for implicit JSR dest operands */
        if ((Ldep_allow_lat_dangles_into_jsrs == 0) &&
	        (op_flag_set(oper->proc_opc, OP_FLAG_JSR))) {
	    attr = L_find_attr(oper->attr, "ret");
	    if (attr!=NULL) {
		for (i=0; i<attr->max_field; i++) {
		    L_Operand *macro;
		    macro = attr->field[i];
		    if (macro==NULL)
			continue;
		    if (!L_is_macro(macro))
			continue;
		    /* determine conflicting operands */
		    n_conflict = L_conflicting_operands(macro,conflict,64,prepass);
		    for ( j = 0 ; j < n_conflict; j++ )  {
                	/* does the operand exist in hash table ?? */
                	dep = L_find_dep_operand(conflict[j]);
                	if ( dep == NULL ) continue;

			/* calculate output dependence distance, minimum dist= 0,
			   use DEP_SYNC_OPERAND as to index as a shortcut, so I
			   don't need to declare a new sync operand */
                	to_index = operand_index(MDES_SYNC_OUT,DEP_SYNC_OPERAND);
			distance = min_operand_time(MDES_INFO(dep->oper),dep->index) -
                                   max_operand_time(mdes_info,to_index);
			if ( distance < 0 ) distance = 0;

	        	/* yes, add output depedence from dep->oper to oper */
	        	L_add_output_dep(L_DEP_REG_OUTPUT,distance,dep->index,to_index,
						dep->oper,oper,0);
	        	L_add_input_dep(L_DEP_REG_OUTPUT,distance,dep->index,to_index,
				       	dep->oper,oper,0);
		    }
		    for (j = 0; j < n_conflict; j++) {
			L_delete_operand(conflict[j]);
		    }
		}
	    }
	}

	/* During postpass scheduling, no percolation is allowed */
	/* across subroutine calls, thus no register depedences  */
	/* are required across jsr's.  This also makes it easier */
	/* for the scheduler to fill subroutine call delay slots */
	/* KMC - 9/22/98 - Eliminating because it is probably unsafe, 
         * The code should be re-written to deal with filling delay slots
         */
	/*	if (((L_spec_model != SENTINEL) || (L_spec_model != SRB)) && !prepass 
	 *          && Lsched_fill_delay_slots && 
         *           op_flag_set(oper->proc_opc,OP_FLAG_JSR))
         *             L_reset_dep_hash_table();
	 */
 
	/*
	 * Update hash tables
	 */
	for ( i = 0; i < L_max_dest_operand; i++ )  {
	    L_Operand *dest;
	    if ( (dest = oper->dest[i]) == NULL ) continue;
	    
	    if (!(L_is_reg(dest) || L_is_macro(dest)))
		continue;
	    
	    /* detemine conflicting operands */
	    n_conflict = L_conflicting_operands(dest,conflict,64, prepass);
	    for ( j = 0 ; j < n_conflict; j++ )  {
	        /* does the operand exist in hash table ?? */
		dep = L_find_dep_operand(conflict[j]);
		if ( dep == NULL )  {
		    /* insert the operand into the hash table if it */
		    /* is not already present			    */
		    if ( L_same_register_operand(dest,conflict[j]) )
		        L_insert_dep_operand(operand_index(MDES_DEST,i),
					     conflict[j],oper);
		}
		else {
		    /* if the operand is present, simply change the */
		    /* instruction pointer			    */
		    if ( L_same_register_operand(dest,conflict[j]) )
		        dep->oper = oper;
#if 0   /* REH 2/20/94 - cannot delete if isn't identical to current destination */
		    else
			/* if the operand is present, but is not identical to     */
			/* the current destination, remove it from the hash table */
			L_remove_dep_operand(dep);
#endif
		}
	    }
	    for (j = 0; j < n_conflict; j++) {
		L_delete_operand(conflict[j]);
	    }
	}

	/* SAM 8-95, record JSR defining its return macros */
        if ((Ldep_allow_lat_dangles_into_jsrs == 0) &&
	    (op_flag_set(oper->proc_opc, OP_FLAG_JSR))) {
	    attr = L_find_attr(oper->attr, "ret");
	    if (attr!=NULL) {
		for (i=0; i<attr->max_field; i++) {
                    L_Operand *macro;
                    macro = attr->field[i];
                    if (macro==NULL)
                        continue;
		    if (!L_is_macro(macro))
                        continue;
		    /* determine conflicting operands */
		    n_conflict = L_conflicting_operands(macro,conflict,64,prepass);
		    for ( j = 0 ; j < n_conflict; j++ )  {
                	/* does the operand exist in hash table ?? */
                	dep = L_find_dep_operand(conflict[j]);
                	if ( dep == NULL )  {
                    	    /* insert the operand into the hash table if it */
                            /* is not already present                       */
                            if ( L_same_register_operand(macro,conflict[j]) )
                                 L_insert_dep_operand(operand_index(MDES_SYNC_OUT,
					DEP_SYNC_OPERAND),conflict[j],oper);
			}
			else {
                            /* if the operand is present, simply change the */
                            /* instruction pointer                          */
                            if ( L_same_register_operand(macro,conflict[j]) )
                                dep->oper = oper;
			}
		    }
		    for (j = 0; j < n_conflict; j++) {
			L_delete_operand(conflict[j]);
		    }
		}
	    }
	}
    }

    L_reset_dep_hash_table();
    
    for ( oper = cb->last_op; oper != NULL; oper = oper->prev_op )  {
        if ( !(DEP_INFO(oper)) ) continue;
	
    	/*
      	 *	3. Compute register anti dependences.
     	 */

	/* anti deps for src operands */
	for ( i = 0; i < L_max_src_operand; i++ )  {
	    L_Operand *src;
	    if ( (src = oper->src[i]) == NULL ) continue;
	    
	    if (!(L_is_reg(src) || L_is_macro(src)))
		continue;
	    
	    n_conflict = L_conflicting_operands(src,conflict,64,prepass);
	    for ( j = 0 ; j < n_conflict; j++ )  {
	        /* does the operand exist in hash table ?? */
		dep = L_find_dep_operand(conflict[j]);
		if ( dep == NULL ) continue;
	    
		/* calculate anti-dependence distance, minimum = 0 */
		from_index = operand_index(MDES_SRC,i);
		distance = max_operand_time(mdes_info,from_index) - 
		    	   min_operand_time(MDES_INFO(dep->oper),dep->index);
		if ( distance < 0 ) distance = 0;
		
	        /* yes, add anti depedence from dep->oper to oper */
	        L_add_output_dep(L_DEP_REG_ANTI,distance,from_index,dep->index,
					oper,dep->oper,0);
	        L_add_input_dep(L_DEP_REG_ANTI,distance,from_index,dep->index,
				       oper,dep->oper,0);
	    }
	    for (j = 0; j < n_conflict; j++) {
		L_delete_operand(conflict[j]);
	    }
	}

        /* SAM 8-95, anti deps for implicit JSR/RTS src operands */
        if ((Ldep_allow_lat_dangles_into_jsrs == 0) &&
	        (op_flag_set(oper->proc_opc, OP_FLAG_JSR) ||
	         op_flag_set(oper->proc_opc, OP_FLAG_RTS))) {
	    attr = L_find_attr(oper->attr, "tr");
	    if (attr!=NULL) {
		for (i=0; i<attr->max_field; i++) {
		    L_Operand *macro;
		    macro = attr->field[i];
		    if (macro==NULL)
			continue;
		    if (!L_is_macro(macro))
			continue;
		    /* determine conflicting operands */
		    n_conflict = L_conflicting_operands(macro,conflict,64,prepass);
		    for ( j = 0 ; j < n_conflict; j++ )  {
                	/* does the operand exist in hash table ?? */
                	dep = L_find_dep_operand(conflict[j]);
                	if ( dep == NULL ) continue;

			/* calculate flow dependence distance, minimum dist= 0,
			   use DEP_SYNC_OPERAND as to index as a shortcut, so I
			   don't need to declare a new sync operand */
                	from_index = operand_index(MDES_SYNC_IN,DEP_SYNC_OPERAND);
			distance = max_operand_time(mdes_info,from_index) -
                                   min_operand_time(MDES_INFO(dep->oper),dep->index);
			if ( distance < 0 ) distance = 0;

	        	/* yes, add anti depedence from dep->oper to oper */
	        	L_add_output_dep(L_DEP_REG_ANTI,distance,from_index,dep->index,
                                        	oper,dep->oper,0);
                        L_add_input_dep(L_DEP_REG_ANTI,distance,from_index,dep->index,
                                       		oper,dep->oper,0);
		    }
		    for (j = 0; j < n_conflict; j++) {
			L_delete_operand(conflict[j]);
		    }
		}
	    }
	}
	
	/* During postpass scheduling, no percolation is allowed */
	/* across subroutine calls, thus no register depedences  */
	/* are required across jsr's.  This also makes it easier */
	/* for the scheduler to fill subroutine call delay slots */
	/* KMC - 9/22/98 - Eliminating because it is probably unsafe, 
         * The code should be re-written to deal with filling delay slots
         */
	/*	if (((L_spec_model != SENTINEL) || (L_spec_model != SRB)) && !prepass 
	 *          && Lsched_fill_delay_slots && 
         *           op_flag_set(oper->proc_opc,OP_FLAG_JSR))
         *             L_reset_dep_hash_table();
	 */

	/*
	 * Update hash tables
	 */
	for ( i = 0; i < L_max_dest_operand; i++ )  { 
	    L_Operand *dest;
	    if ( (dest = oper->dest[i]) == NULL ) continue;
	    
	    if (!(L_is_reg(dest) || L_is_macro(dest)))
		continue;
	    
	    n_conflict = L_conflicting_operands(dest,conflict,64,prepass);
	    for ( j= 0 ; j < n_conflict; j++ )  {
	        /* does the operand exist in hash table ?? */
		dep = L_find_dep_operand(conflict[j]);
		if ( dep == NULL )  {
		    /* insert the operand into the hash table if it */
		    /* is not already present			    */
		    if ( L_same_register_operand(dest,conflict[j]) )
		        L_insert_dep_operand(operand_index(MDES_DEST,i),
					     conflict[j],oper);
		}
		else {
		    /* if the operand is present, simply change the */
		    /* instruction pointer			    */
		    if ( L_same_register_operand(dest,conflict[j]) )
		        dep->oper = oper;
#if 0   /* REH 2/20/94 - cannot delete if isn't identical to current destination */
		    else
			/* if the operand is present, but is not identical to     */
			/* the current destination, remove it from the hash table */
			L_remove_dep_operand(dep);
#endif
		}
	    }
	    for (j = 0; j < n_conflict; j++) {
		L_delete_operand(conflict[j]);
	    }
	}

	/* SAM 8-95, record JSR defining its return macros */
        if ((Ldep_allow_lat_dangles_into_jsrs == 0) &&
	    (op_flag_set(oper->proc_opc, OP_FLAG_JSR))) {
	    attr = L_find_attr(oper->attr, "ret");
	    if (attr!=NULL) {
		for (i=0; i<attr->max_field; i++) {
                    L_Operand *macro;
                    macro = attr->field[i];
                    if (macro==NULL)
                        continue;
		    if (!L_is_macro(macro))
                        continue;
		    /* determine conflicting operands */
		    n_conflict = L_conflicting_operands(macro,conflict,64,prepass);
		    for ( j = 0 ; j < n_conflict; j++ )  {
                	/* does the operand exist in hash table ?? */
                	dep = L_find_dep_operand(conflict[j]);
                	if ( dep == NULL )  {
                    	    /* insert the operand into the hash table if it */
                            /* is not already present                       */
                            if ( L_same_register_operand(macro,conflict[j]) )
                                 L_insert_dep_operand(operand_index(MDES_SYNC_OUT,
					DEP_SYNC_OPERAND),conflict[j],oper);
			}
			else {
                            /* if the operand is present, simply change the */
                            /* instruction pointer                          */
                            if ( L_same_register_operand(macro,conflict[j]) )
                                dep->oper = oper;
			}
		    }
		    for (j = 0; j < n_conflict; j++) {
			L_delete_operand(conflict[j]);
		    }
		}
	    }
	}
    }

    L_reset_dep_hash_table();
}

/* In cyclic mode, compute cross-iteration flow dependences for 
   virtual registers.   For macro registers, compute cross-iteration flow,
   output, and anti dependences because macros cannot be renamed. */
static void L_compute_cross_iter_register_dependence(L_Cb *cb, int prepass)
{
    int i,j,to_index,from_index,distance;
    int n_conflict;
    L_Operand *conflict[64];
    L_Oper *oper;
    Dep_Operand *dep;
    Mdes_Info *mdes_info;

    L_init_dep_hash_table();

    /* In first pass, add all definitions to hash table that could
       possibly be used by a subsequent iteration. */

    for ( oper = cb->first_op; oper != NULL; oper = oper->next_op )  {
        if ( !(DEP_INFO(oper)) ) continue;
	
	mdes_info = MDES_INFO(oper);
	
	for ( i = 0; i < L_max_dest_operand; i++ )  {
	    L_Operand *dest;
	    if ( (dest = oper->dest[i]) == NULL ) continue;
	    
	    if (!(L_is_reg(dest) || L_is_macro(dest)))
		continue;
	    
	    /* detemine conflicting operands */
	    n_conflict = L_conflicting_operands(dest,conflict,64, prepass);
	    for ( j = 0 ; j < n_conflict; j++ )  {
	        /* does the operand exist in hash table ?? */
		dep = L_find_dep_operand(conflict[j]);
		if ( dep == NULL )  {
		    /* insert the operand into the hash table if it */
		    /* is not already present			    */
                    /* Only add the name used by the current oper.  No
                       need to add all conflicting operands because all
                       conflicting operands will be checked at the
                       sink of the dependence. */
		    if ( L_same_register_operand(dest,conflict[j]) )
		        L_insert_dep_operand(operand_index(MDES_DEST,i),
					     conflict[j],oper);
		}
		else {
		    /* if the operand is present, simply change the */
		    /* instruction pointer.     */
		    if ( L_same_register_operand(dest,conflict[j]) )
		        dep->oper = oper;
		    else
			/* if a conflicting operand is present, assume this
                           definition kills the definition of the conflicting
                           operand.  This assumption is valid for HP and 
                           Sparc for prepass software pipelining when 
                           loops with function calls are not allowed.    */
			L_remove_dep_operand(dep);
		}
	    }
	    for (j = 0; j < n_conflict; j++) {
		L_delete_operand(conflict[j]);
	    }
	}
    }

    /* In second pass, find the flow and output dependences */

    for ( oper = cb->first_op; oper != NULL; oper = oper->next_op )  {
        if ( !(DEP_INFO(oper)) ) continue;
	
	mdes_info = MDES_INFO(oper);
	
        /*
     	 *  	1. Compute register flow dependences.
     	 */  

	for ( i = 0; i < L_max_src_operand; i++ )  {
	    L_Operand *src;
	    if ( (src = oper->src[i]) == NULL ) continue;
	    
	    if (!(L_is_reg(src) ||L_is_macro(src)))
		continue;
	    
	    /* determine conflicting operands */
	    n_conflict = L_conflicting_operands(src,conflict,64, prepass);
	    for ( j = 0 ; j < n_conflict; j++ )  {
	        /* does the operand exist in hash table ?? */
		dep = L_find_dep_operand(conflict[j]);
		if ( dep == NULL ) continue;
		
		/* calculate flow dependence distance, minimum distance = 0 */
		to_index = operand_index(MDES_SRC,i);
		distance = min_operand_time(MDES_INFO(dep->oper),dep->index) -
		    	   max_operand_time(mdes_info,to_index);
		if ( distance < 0 ) distance = 0;
		
	        /* add flow dependence from dep->oper to oper */
	        L_add_output_dep(L_DEP_REG_FLOW,distance,dep->index,to_index,
					dep->oper,oper,1);
	        L_add_input_dep(L_DEP_REG_FLOW,distance,dep->index,to_index,
				       dep->oper,oper,1);
	    }
	    for (j = 0; j < n_conflict; j++) {
		L_delete_operand(conflict[j]);
	    }
	}

	/*
     	 *	2. Compute register output dependences.
     	 */

	for ( i = 0; i < L_max_dest_operand; i++ )  {
	    L_Operand *dest;
	    if ( (dest = oper->dest[i]) == NULL ) continue;
	    
	    if (!L_is_macro(dest))
		continue;
	    
	    n_conflict = L_conflicting_operands(dest,conflict,64, prepass);
	    for ( j = 0 ; j < n_conflict; j++ )  {
	        /* does the operand exist in hash table ?? */
		dep = L_find_dep_operand(conflict[j]);
		if ( dep == NULL ) continue;
	    
		/* calculate output dependence distance, minimum distance = 0 */
		to_index = operand_index(MDES_DEST,i);
		distance = min_operand_time(MDES_INFO(dep->oper),dep->index) -
		    	   max_operand_time(mdes_info,to_index);
		if ( distance < 0 ) distance = 0;
		
	        /* add output depedence from dep->oper to oper */
	        L_add_output_dep(L_DEP_REG_OUTPUT,distance,dep->index,to_index,
					dep->oper,oper,1);
	        L_add_input_dep(L_DEP_REG_OUTPUT,distance,dep->index,to_index,
				       dep->oper,oper,1);
	    }
	    for (j = 0; j < n_conflict; j++) {
		L_delete_operand(conflict[j]);
	    }
	}

	/*
	 * Update hash tables
	 */
        /* Any definition here kills the definition from the previous
           iteration.  Don't want to add the current definition, because
           it will create intra-iteration deps.  We only want to see the
           inter-iteration deps. */

	for ( i = 0; i < L_max_dest_operand; i++ )  {
	    L_Operand *dest;
	    if ( (dest = oper->dest[i]) == NULL ) continue;

	    if (!(L_is_reg(dest) || L_is_macro(dest)))
		continue;
	    
	    /* detemine conflicting operands */
	    n_conflict = L_conflicting_operands(dest,conflict,64, prepass);
	    for ( j = 0 ; j < n_conflict; j++ )  {
	        /* does the operand exist in hash table ?? */
		dep = L_find_dep_operand(conflict[j]);
		/* if the operand is present, simply remove it */
		if ( dep != NULL )  {
		    L_remove_dep_operand(dep);
		}
	    }
	    for (j = 0; j < n_conflict; j++) {
		L_delete_operand(conflict[j]);
	    }
	}
    }

    L_reset_dep_hash_table();

    /* In third pass, add to the hash table all possible redefinitions 
       of variants used by a previous iteration. */

    for ( oper = cb->last_op; oper != NULL; oper = oper->prev_op )  {
        if ( !(DEP_INFO(oper)) ) continue;
	
	mdes_info = MDES_INFO(oper);
	
	for ( i = 0; i < L_max_dest_operand; i++ )  {
	    L_Operand *dest;
	    if ( (dest = oper->dest[i]) == NULL ) continue;
	    
	    if (!L_is_macro(dest))
		continue;
	    
	    /* detemine conflicting operands */
	    n_conflict = L_conflicting_operands(dest,conflict,64, prepass);
	    for ( j = 0 ; j < n_conflict; j++ )  {
	        /* does the operand exist in hash table ?? */
		dep = L_find_dep_operand(conflict[j]);
		if ( dep == NULL )  {
		    /* insert the operand into the hash table if it */
		    /* is not already present			    */
                    /* Only add the name used by the current oper.  No
                       need to add all conflicting operands because all
                       conflicting operands will be checked and the
                       sink of the dependence. */
		    if ( L_same_register_operand(dest,conflict[j]) )
		        L_insert_dep_operand(operand_index(MDES_DEST,i),
					     conflict[j],oper);
		}
		else {
		    /* if the operand is present, simply change the */
		    /* instruction pointer.     */
		    if ( L_same_register_operand(dest,conflict[j]) )
		        dep->oper = oper;
		    else
			/* if a conflicting operand is present, assume this
                           definition kills the definition of the conflicting
                           operand.  This assumption is valid for HP and 
                           Sparc for prepass software pipelining when 
                           loops with function calls are not allowed.    */
			L_remove_dep_operand(dep);
		}
	    }
	    for (j = 0; j < n_conflict; j++) {
		L_delete_operand(conflict[j]);
	    }
	}
    }

    /* In fourth pass, find the anti-dependences */

    for ( oper = cb->last_op; oper != NULL; oper = oper->prev_op )  {
        if ( !(DEP_INFO(oper)) ) continue;
	
	mdes_info = MDES_INFO(oper);
	
	/*
	 * Update hash tables
	 */
        /* Any definition here kills the definition from the later
           iteration.  Don't want to add the current definition, because
           it will create intra-iteration deps.  We only want to see the
           inter-iteration deps. */

	for ( i = 0; i < L_max_dest_operand; i++ )  {
	    L_Operand *dest;
	    if ( (dest = oper->dest[i]) == NULL ) continue;
	    
	    if (!L_is_macro(dest))
		continue;
	    
	    /* detemine conflicting operands */
	    n_conflict = L_conflicting_operands(dest,conflict,64, prepass);
	    for ( j = 0 ; j < n_conflict; j++ )  {
	        /* does the operand exist in hash table ?? */
		dep = L_find_dep_operand(conflict[j]);
		/* if the operand is present, simply remove it */
		if ( dep != NULL )  {
		    L_remove_dep_operand(dep);
		}
	    }
	    for (j = 0; j < n_conflict; j++) {
		L_delete_operand(conflict[j]);
	    }
	}

        /*
     	 *  	Compute register anti-dependences.
     	 */  

	for ( i = 0; i < L_max_src_operand; i++ )  {
	    L_Operand *src;
	    if ( (src = oper->src[i]) == NULL ) continue;
	    
	    if (!L_is_macro(src))
		continue;
	    
	    /* determine conflicting operands */
	    n_conflict = L_conflicting_operands(src,conflict,64, prepass);
	    for ( j = 0 ; j < n_conflict; j++ )  {
	        /* does the operand exist in hash table ?? */
		dep = L_find_dep_operand(conflict[j]);
		if ( dep == NULL ) continue;
	    
		/* calculate anti-dependence distance, minimum = 0 */
		from_index = operand_index(MDES_SRC,i);
		distance = min_operand_time(mdes_info,from_index) - 
		    	   max_operand_time(MDES_INFO(dep->oper),dep->index);
		if ( distance < 0 ) distance = 0;
		
	        /* yes, add output depedence from dep->oper to oper */
	        L_add_output_dep(L_DEP_REG_ANTI,distance,from_index,dep->index,
					oper,dep->oper,1);
	        L_add_input_dep(L_DEP_REG_ANTI,distance,from_index,dep->index,
				       oper,dep->oper,1);
	    }
	    for (j = 0; j < n_conflict; j++) {
		L_delete_operand(conflict[j]);
	    }
	}
    }
    L_reset_dep_hash_table();
}

static void L_compute_memory_dependence(L_Cb *cb)
{
    int out_index, in_index, distance, id = 0;
    L_Oper *oper;
    Dep_Info *load_head,*store_head;
    Dep_Info *st_dep1,*st_dep2, *ld_dep;
    int dep_flags;

    dep_flags = SET_NONLOOP_CARRIED(0);

    /* use profile generated memory dependences */
    if (Ldep_check_profiled_memory_dependences) 
	dep_flags |= SET_PROFILE_CONFLICT(0);

    /* construct linked lists of loads and store instructions */
    load_head = store_head = NULL;
    for ( oper = cb->first_op; oper != NULL; oper = oper->next_op )  {
	if ( op_flag_set(oper->proc_opc, OP_FLAG_STORE) )  {
	    DEP_INFO(oper)->level = id++;
	    DEP_INFO(oper)->oper = oper;
	    DEP_INFO(oper)->next = store_head;
	    store_head = DEP_INFO(oper);
	}
	else if ( op_flag_set(oper->proc_opc, OP_FLAG_LOAD) )  {
	    DEP_INFO(oper)->level = id++;
	    DEP_INFO(oper)->oper = oper;
	    DEP_INFO(oper)->next = load_head;
	    load_head = DEP_INFO(oper);
	}
    }
    
    out_index = operand_index(MDES_SYNC_OUT,DEP_MEM_OPERAND);
    in_index = operand_index(MDES_SYNC_IN,DEP_MEM_OPERAND);
    
    for ( st_dep1 = store_head; st_dep1 != NULL ; st_dep1 = st_dep1->next )  {
	/*
         *	1. Compute memory output dependences
         */
	for ( st_dep2 = st_dep1->next ; st_dep2 != NULL; st_dep2 = st_dep2->next )  {
	    if ( !L_independent_memory_ops(cb, st_dep1->oper, st_dep2->oper, dep_flags) )  {
		/* calculate memory output dependence distance, min = 0 */
		distance = min_operand_time(MDES_INFO(st_dep2->oper),out_index) -
		    	   max_operand_time(MDES_INFO(st_dep1->oper),out_index);
		if ( distance < 0 ) distance = 0;
		
		/* The linked list has the store instructions in reverse order, 
		    thus the output dependence goes from st_dep2->oper to
		    st_dep1->oper */
		L_add_output_dep(L_DEP_MEM_OUTPUT,distance,out_index,out_index,
				 st_dep2->oper,st_dep1->oper,0);
	        L_add_input_dep(L_DEP_MEM_OUTPUT,distance,out_index,out_index,
				st_dep2->oper,st_dep1->oper,0);
	    }
	    else  {
		if ( Ldep_debug_memory_disambiguation ) 
		   fprintf(stderr,"memory disambiguation: op %d ^ op %d\n",
			   st_dep2->oper->id, st_dep1->oper->id);
	    }
	}
	for ( ld_dep = load_head; ld_dep != NULL; ld_dep = ld_dep->next )  {
	    if ( !L_independent_memory_ops(cb, st_dep1->oper, ld_dep->oper, dep_flags) )  {
		/*
         	 *    2. Compute memory flow dependences
         	 */
		if ( st_dep1->level < ld_dep->level )  {
		    /* calculate memory flow dependence distance, min = 0 */
	    	    distance = 
			min_operand_time(MDES_INFO(st_dep1->oper),out_index) -
		    	max_operand_time(MDES_INFO(ld_dep->oper),in_index);
	     	    if ( distance < 0 ) distance = 0;
		
	      	    L_add_output_dep(L_DEP_MEM_FLOW,distance,out_index,in_index,
				     st_dep1->oper,ld_dep->oper,0);
	    	    L_add_input_dep(L_DEP_MEM_FLOW,distance,out_index,in_index,
				    st_dep1->oper,ld_dep->oper,0);
		}
		/*
     		 *    3. Compute memory anti dependences
     	 	 */
		else {
		    /* calculate memory anti-dependence distance, min = 0 */
	    	    distance = 
			max_operand_time(MDES_INFO(ld_dep->oper),in_index) -
		        min_operand_time(MDES_INFO(st_dep1->oper),out_index);
	      	    if ( distance < 0 ) distance = 0;
	    
	    	    L_add_output_dep(L_DEP_MEM_ANTI,distance,in_index,out_index,
				     ld_dep->oper,st_dep1->oper,0);
	    	    L_add_input_dep(L_DEP_MEM_ANTI,distance,in_index,out_index,
				    ld_dep->oper,st_dep1->oper,0);
		}
	    }
	    else  {
		if ( Ldep_debug_memory_disambiguation ) 
		   fprintf(stderr,"memory disambiguation: op %d ^ op %d\n",
			   st_dep1->oper->id, ld_dep->oper->id);
	    }
	}
    }
}


static void L_compute_cross_iter_memory_dependence(L_Cb *cb)
{
    L_Oper *oper;
    Dep_Info *load_head,*store_head;
    Dep_Info *st_dep1,*st_dep2, *ld_dep;
    int out_index, in_index; 
    int id = 0;     /* unique id for each load and store */
    int dep_flags;  /* bit mask to indicate which types of cross-iteration
                       memory dependences to consider */
    int distance;   /* delay in cycles associated with the dependence */
    int forward;    /* set if dependence being checked is from an oper to
                       a lexically later oper */
    int omega;      /* dependence distance in iterations */

    if (!L_func_contains_dep_pragmas) {
        L_punt ("L_compute_cross_iter_memory_dependence: cannot add cross "
                "iteration memory dependences without sync arcs\n", L_fn->name);
    }

    /* only want to add cross-iteration dependences that are carried by
       the inner loop */
    dep_flags = SET_INNER_CARRIED(0);

    /* use profile generated memory dependences */
    if (Ldep_check_profiled_memory_dependences) 
	dep_flags |= SET_PROFILE_CONFLICT(0);

    /* Construct linked lists of load and store instructions.  Note that
       in the final list, the opers will appear in reverse program order. */
    load_head = store_head = NULL;
    for ( oper = cb->first_op; oper != NULL; oper = oper->next_op )  {
        if ( op_flag_set(oper->proc_opc, OP_FLAG_STORE) )  {
            DEP_INFO(oper)->level = id++;
            DEP_INFO(oper)->oper = oper;
            DEP_INFO(oper)->next = store_head;
            store_head = DEP_INFO(oper);
        }
        else if ( op_flag_set(oper->proc_opc, OP_FLAG_LOAD) )  {
            DEP_INFO(oper)->level = id++;
            DEP_INFO(oper)->oper = oper;
            DEP_INFO(oper)->next = load_head;
            load_head = DEP_INFO(oper);
        }
    }

    out_index = operand_index(MDES_SYNC_OUT,DEP_MEM_OPERAND);
    in_index = operand_index(MDES_SYNC_IN,DEP_MEM_OPERAND);

    for ( st_dep1 = store_head; st_dep1 != NULL ; st_dep1 = st_dep1->next ) {
        /*
         *      1. Compute memory output dependences
         */
        for ( st_dep2 = store_head; st_dep2 != NULL; st_dep2 = st_dep2->next ) {
            /* Linked list has the store instructions in reverse order. Thus,
               the output dependence goes from st_dep2->oper to st_dep1->oper.
               If st_dep2->oper comes before st_dep1->oper, dependence is
               lexicographically forward. */
            if (st_dep2->level < st_dep1->level) {
                forward = 1;
            }
            else {
                forward = 0;
            }
            /* only check sync arcs because L_independent_memory_ops does not
               understand cross-iteration dependences */
            if (!L_analyze_syncs_for_cross_iter_independence(st_dep2->oper,
                                  st_dep1->oper, dep_flags, forward, &omega)) {

               /* calculate memory output dependence delay, min = 0 */
               distance = min_operand_time(MDES_INFO(st_dep2->oper),out_index) -
                          max_operand_time(MDES_INFO(st_dep1->oper),out_index);
               if ( distance < 0 ) distance = 0;

               /* The linked list has the store instructions in reverse order.
                  Thus, the output dependence goes from st_dep2->oper to
                  st_dep1->oper */
               L_add_output_dep(L_DEP_MEM_OUTPUT,distance,out_index,out_index,
                                 st_dep2->oper,st_dep1->oper,omega);
               L_add_input_dep(L_DEP_MEM_OUTPUT,distance,out_index,out_index,
                                st_dep2->oper,st_dep1->oper,omega);
            }
            else  {
                if ( Ldep_debug_memory_disambiguation )
                   fprintf(stderr,"cross-iter memory disambiguation: op %d to op %d\n",
                               st_dep2->oper->id, st_dep1->oper->id);
            }
        }
        for ( ld_dep = load_head; ld_dep != NULL; ld_dep = ld_dep->next )  {
            /*
             *    2. Compute memory flow dependences
             */
            /* If st_dep2->oper comes before ld_dep->oper, dependence is
               lexicographically forward. */
            if (st_dep1->level < ld_dep->level) {
                forward = 1;
            }
            else {
                forward = 0;
            }
            /* only check sync arcs because L_independent_memory_ops does not
               understand cross-iteration dependences */
            if (!L_analyze_syncs_for_cross_iter_independence(st_dep1->oper,
                                  ld_dep->oper, dep_flags, forward, &omega)) {

               /* calculate memory flow dependence distance, min = 0 */
               distance = min_operand_time(MDES_INFO(st_dep1->oper),out_index) -
                      max_operand_time(MDES_INFO(ld_dep->oper),in_index);
               if ( distance < 0 ) distance = 0;
              
               L_add_output_dep(L_DEP_MEM_FLOW,distance,out_index,in_index,
                                     st_dep1->oper,ld_dep->oper,omega);
               L_add_input_dep(L_DEP_MEM_FLOW,distance,out_index,in_index,
                                    st_dep1->oper,ld_dep->oper,omega);
            }
            else  {
                if ( Ldep_debug_memory_disambiguation )
                   fprintf(stderr,"cross iter memory disambiguation: op %d to op %d\n",
                           st_dep1->oper->id, ld_dep->oper->id);
            }
            /*
             *    3. Compute memory anti dependences
             */
            /* If st_dep2->oper comes after ld_dep->oper, dependence is
               lexicographically forward. */
            if (st_dep1->level > ld_dep->level) {
                forward = 1;
            }
            else {
                forward = 0;
            }
            /* only check sync arcs because L_independent_memory_ops does not
               understand cross-iteration dependences */
            if (!L_analyze_syncs_for_cross_iter_independence(ld_dep->oper,
                                  st_dep1->oper, dep_flags, forward, &omega)) {

                /* calculate memory anti-dependence distance, min = 0 */
                distance = min_operand_time(MDES_INFO(ld_dep->oper),in_index) -
                        max_operand_time(MDES_INFO(st_dep1->oper),out_index);
                if ( distance < 0 ) distance = 0;

                L_add_output_dep(L_DEP_MEM_ANTI,distance,in_index,out_index,
                                     ld_dep->oper,st_dep1->oper,omega);
                L_add_input_dep(L_DEP_MEM_ANTI,distance,in_index,out_index,
                                    ld_dep->oper,st_dep1->oper,omega);
            }
            else  {
                if ( Ldep_debug_memory_disambiguation )
                   fprintf(stderr,"cross iter memory disambiguation: op %d to op %d\n",
                           ld_dep->oper->id, st_dep1->oper->id);
            }
        }
    }
}


static void L_compute_control_dependence(L_Cb *cb, int prepass)
{
    int from_index, to_index, distance, spec_cond;
    L_Oper *oper; 
    Dep_Info *head;
    int delay;
    L_Oper *loop_back_br;
    L_Oper *next_oper;
    
    from_index = operand_index(MDES_SYNC_OUT,DEP_CNT_OPERAND);
    to_index = operand_index(MDES_SYNC_IN,DEP_CNT_OPERAND);  
    
    /*
     *   Compute control "flow" dependence, from previous instruction to branch.
     */
    head = NULL;
    for ( oper = cb->first_op; oper != NULL; oper = oper->next_op )  {
        if ( !(DEP_INFO(oper)) ) continue;
	    
	/* Examine the list to see if any of the opers require 
	   a control dependence on the current control instruction.  Once
	   an oper gets a control dependence, remove it from the list. */
	if ( op_flag_set(oper->proc_opc, OP_FLAG_CBR | OP_FLAG_JMP | 
			   OP_FLAG_RTS | OP_FLAG_JSR) )  {
	    Dep_Info *dep, *next_dep, *prev = NULL;
	    if ( op_flag_set(oper->proc_opc, OP_FLAG_CBR) )  {
		for ( dep = head; dep != NULL; dep = next_dep )  {
		    next_dep = dep->next;
		    if ( !Ldep_allow_downward_code_perc ||
			 !L_safe_to_move_below_branch(cb,dep->oper,oper) )  {
			/* calculate control "flow" dependence distance, min = 0 */
	    	    	distance = max_operand_time(MDES_INFO(dep->oper),from_index) -
		               min_operand_time(MDES_INFO(oper),to_index);
	    	    	if ( distance < 0 ) distance = 0;
		
	      	    	L_add_output_dep(L_DEP_CNT,distance,from_index,to_index,
					dep->oper,oper,0);
	      	    	L_add_input_dep(L_DEP_CNT,distance,from_index,to_index,
				        dep->oper,oper,0);
			
			/* remove instruction from the list */
			if ( prev )
			    prev->next = dep->next;
			else
			    head = dep->next;
			dep->next = NULL;
			
			continue;   /* we don't need to change the prev pointer */
		    }
		    if ( prev )
			prev = prev->next;
		    else
			prev = head;
		}
	    }
	    else if ( op_flag_set(oper->proc_opc, OP_FLAG_JSR) )  {
		for ( dep = head; dep != NULL; dep = next_dep )  {
		    next_dep = dep->next;
		    
		    if ( !prepass ||
			 !Ldep_remove_jsr_dependences ||
			 !L_jsr_independent_oper(oper,dep->oper) ) {
#if 0
			if (Lsched_processor_model==MDES_VLIW)
			{
			    /* 
			     * RAB - support for VLIW  8/4/94
			     *
			     * control flow distance will be the instruction 
			     * latency - the latency of the jsr.
			     */
			    distance = 
				mdes_max_completion_time(dep->oper->proc_opc,
				   mdes_heuristic_alt_id(dep->oper->proc_opc)) -
				mdes_max_completion_time(oper->proc_opc,
				   mdes_heuristic_alt_id(oper->proc_opc));
			}
			else
#endif
			{
			    /* 
			     * calculate control "flow" dependence distance, 
			     * min = 0 
			     */
	    	    	    distance = max_operand_time(MDES_INFO(dep->oper),
			        from_index) - min_operand_time(MDES_INFO(oper),
			        to_index);
			}

	    	    	if ( distance < 0 ) distance = 0;
		
	      	    	L_add_output_dep(L_DEP_CNT,distance,from_index,to_index,
					dep->oper,oper,0);
	      	    	L_add_input_dep(L_DEP_CNT,distance,from_index,to_index,
				        dep->oper,oper,0);
			
			/* remove instruction from the list */
			if ( prev )
			    prev->next = dep->next;
			else
			    head = dep->next;
			dep->next = NULL;
			
			continue;   /* we don't need to change the prev pointer */
		    }
		    if ( prev )
			prev = prev->next;
		    else
			prev = head;
		}
	    }
	    else if (L_register_branch_opcode(oper))
	    {
		/* Hashing jumps require control dependences */
	    	for ( dep = head; dep != NULL; dep = next_dep )  {
		    next_dep = dep->next;
		    
		    dep->next = NULL;  /* empty the list as we go */
		    
#if 0
		    if (Lsched_processor_model==MDES_VLIW)
		    {
			/* 
			 * RAB - support for VLIW  8/4/94
			 *
			 * control flow distance will be the instruction 
			 * latency - the latency of the jsr.
			 */
			distance = 
			    mdes_max_completion_time(dep->oper->proc_opc,
				mdes_heuristic_alt_id(dep->oper->proc_opc)) -
			    mdes_max_completion_time(oper->proc_opc,
				mdes_heuristic_alt_id(oper->proc_opc));
		    }
		    else
#endif
		    {
		        /* 
			 * calculate control "flow" dependence distance, 
			 * min = 0 
			 */
	    	        distance = max_operand_time(MDES_INFO(dep->oper),
			    from_index) - min_operand_time(MDES_INFO(oper),
			    to_index);
		    }

	    	    if ( distance < 0 ) distance = 0;
		
	      	    L_add_output_dep(L_DEP_CNT,distance,from_index,to_index,
					dep->oper,oper,0);
	      	    L_add_input_dep(L_DEP_CNT,distance,from_index,to_index,
				        dep->oper,oper,0);
		}
		head = NULL;
	    }
	    else
	    {
		/* The current control oper is OP_FLAG_JMP or OP_FLAG_RTS */
		/* so a control depedence is required		      */
	    	for ( dep = head; dep != NULL; dep = next_dep )  {
		    next_dep = dep->next;
		    
		    dep->next = NULL;  /* empty the list as we go */
		    
		    /* calculate control "flow" dependence distance, min = 0 */
	    	    distance = max_operand_time(MDES_INFO(dep->oper),from_index) -
		               min_operand_time(MDES_INFO(oper),to_index);
	    	    if ( distance < 0 ) distance = 0;
		
	      	    L_add_output_dep(L_DEP_CNT,distance,from_index,to_index,
					dep->oper,oper,0);
	      	    L_add_input_dep(L_DEP_CNT,distance,from_index,to_index,
				        dep->oper,oper,0);
		}
		head = NULL;
	    }
	}
	
	/* add the current instruction to the list of opers w/o control dependence */
        DEP_INFO(oper)->oper = oper;
	DEP_INFO(oper)->next = head;
	head = DEP_INFO(oper);
    }	

    /*
     *   Compute control "flow" dependence, from branch to subsequent instr.
     */
    head = NULL;
    for ( oper = cb->last_op; oper != NULL; oper = oper->prev_op )  {
        if ( !(DEP_INFO(oper)) ) continue;
	    
	/* Examine the list to see if any of the opers require 
	   a control dependence on the current control instruction.  Once
	   an oper gets a control dependence, remove it from the list. */
	if ( op_flag_set(oper->proc_opc, OP_FLAG_CBR | OP_FLAG_JMP | 
			   OP_FLAG_RTS | OP_FLAG_JSR) )  {
	    Dep_Info *dep, *next_dep, *prev = NULL;
	    if ( op_flag_set(oper->proc_opc, OP_FLAG_CBR) )  
	    {
		for ( dep = head; dep != NULL; dep = next_dep )  
		{
		    next_dep = dep->next;
		    
		    spec_cond = L_safe_to_move_above_branch (cb, dep->oper, oper);

		    if ( !Ldep_allow_upward_code_perc || 
  			 (spec_cond == DEP_CANT_SPECULATE) ||
			 ( ((spec_cond == DEP_SILENT) || 
			    (spec_cond == DEP_DELAYS_EXCEPTION)) &&
			   (Ldep_except_branch_perc_limit != -1) && 
			   (dep->level >= Ldep_except_branch_perc_limit) ) ||
			 ( (Ldep_branch_perc_limit != -1) && 
			   (dep->level >= Ldep_branch_perc_limit) ) )
		    {
			/* calculate control "anti-" dependence distance, min = 0 */
	    	        distance = max_operand_time(MDES_INFO(oper),from_index) -
		    	           min_operand_time(MDES_INFO(dep->oper),to_index);
	    	        if ( distance < 0 ) distance = 0;
		
	    	        L_add_output_dep(L_DEP_CNT,distance,from_index,to_index,
					oper,dep->oper,0);
	    	        L_add_input_dep(L_DEP_CNT,distance,from_index,to_index,
				        oper,dep->oper,0);
			
			/* remove instruction from the list */
			if ( prev )
			    prev->next = dep->next;
			else
			    head = dep->next;

			dep->next = NULL;
			
			continue;   /* we don't need to change the prev pointer */
		    }
		    else 
			dep->spec_cond = spec_cond;

		    if ( prev )
			prev = prev->next;
		    else
			prev = head;
		    
		    /* increment to percolation level of the oper */
		    dep->level += 1;
		}
	    }
	    else if ( op_flag_set(oper->proc_opc, OP_FLAG_JSR) )  {
		for ( dep = head; dep != NULL; dep = next_dep )  {
		    next_dep = dep->next;
		    
		    if ( !prepass ||
			 !L_jsr_independent_oper(oper, dep->oper) ||
			 !Ldep_remove_jsr_dependences )  {
			/* calculate control "anti-" dependence distance, min = 0 */
	    	        distance = max_operand_time(MDES_INFO(oper),from_index) -
		    	           min_operand_time(MDES_INFO(dep->oper),to_index);
	    	        if ( distance < 0 ) distance = 0;
		
	    	        L_add_output_dep(L_DEP_CNT,distance,from_index,to_index,
					oper,dep->oper,0);
	    	        L_add_input_dep(L_DEP_CNT,distance,from_index,to_index,
				        oper,dep->oper,0);
			
			/* remove instruction from the list */
			if ( prev )
			    prev->next = dep->next;
			else
			    head = dep->next;
			dep->next = NULL;
			
			continue;   /* we don't need to change the prev pointer */
		    }
		    if ( prev )
			prev = prev->next;
		    else
			prev = head;
		}
	    }
	    else  {  
		/* The current control oper is OP_FLAG_JMP or OP_FLAG_RTS */
		/* so a control depedence is required		      */
	    	for ( dep = head; dep != NULL; dep = next_dep )  {
		    next_dep = dep->next;
		    
		    dep->next = NULL;  /* empty the list as we go */
		    
		    /* calculate control "anti-" dependence distance, min = 0 */
	    	    distance = max_operand_time(MDES_INFO(oper),from_index) -
		    	   min_operand_time(MDES_INFO(dep->oper),to_index);
	    	    if ( distance < 0 ) distance = 0;
		
	    	    L_add_output_dep(L_DEP_CNT,distance,from_index,to_index,
					oper,dep->oper,0);
	    	    L_add_input_dep(L_DEP_CNT,distance,from_index,to_index,
				        oper,dep->oper,0);
		}
		head = NULL;
	    }
	}
	else {
	    /* add the current instruction to the list of opers w/o control dependence */
	    /* NOTE: in this pass, control instructions are not added to the list */
	    
	    /* If SCHED_INFO does not exist, set the level to 0 */
	    if ( SCHED_INFO(oper) ) {
	        /* temporarily used to count percolation level */
	        DEP_INFO(oper)->level = SCHED_INFO(oper)->home_block -
				        SCHED_INFO(oper)->current_block;  
	    }
	    else
		DEP_INFO(oper)->level = 0;
	    
            DEP_INFO(oper)->oper = oper;
 	    DEP_INFO(oper)->next = head;
	    head = DEP_INFO(oper);
	}
    }
 
    /* compute cross-iteration control dependences */
    if (Ldep_mode == LDEP_MODE_CYCLIC) {

        /* Find loop back branch.  Expects cb to end with a conditional
           branch back to same cb.  This is intended to be used for
           software pipelining and the pipeliner makes sure the loop is
           in this form. */
        loop_back_br = cb->last_op;
        if (!op_flag_set(loop_back_br->proc_opc, OP_FLAG_CBR) ||
            (loop_back_br->src[2]->value.cb != cb)) {
            L_punt("L_compute_control_dependence: cb not in form expected "
                   "for cyclic mode - cb: %d\n", cb->id);
        }

        /* compute cross-iteration dependences between the loop back branch
           and the opers in the first basic block of the cb */
        next_oper = cb->first_op;
        do {  
            oper = next_oper;
            next_oper = oper->next_op;

            spec_cond = L_safe_to_move_above_branch (cb, oper, loop_back_br);

            if ( !Ldep_allow_upward_code_perc ||
                 (spec_cond == DEP_CANT_SPECULATE) ) {

                /* calculate control dependence delay, min = 0 */
                delay = min_operand_time(MDES_INFO(loop_back_br), from_index) -
                             max_operand_time(MDES_INFO(oper), to_index);
                if ( delay < 0 ) delay = 0;

                /* make delay 1 because loop back branch will be scheduled
                   in the last slot and nothing can be scheduled after it */
                delay = 1;
                L_add_output_dep(L_DEP_CNT, delay, from_index, to_index,
                                        loop_back_br, oper, 1);
                L_add_input_dep(L_DEP_CNT, delay, from_index, to_index,
                                        loop_back_br, oper, 1);
            }
        } while (!op_flag_set(oper->proc_opc, OP_FLAG_CBR));

    }
}

static void L_compute_synchronization_dependence(L_Cb *cb)
{ 
    int from_index, to_index, distance;
    L_Oper *oper, *last_sync;  
    
    from_index = operand_index(MDES_SYNC_OUT,DEP_SYNC_OPERAND);
    to_index = operand_index(MDES_SYNC_IN,DEP_SYNC_OPERAND);
    
    last_sync = NULL;
    /*
     *  Compute synchronization dependence.
     */
    for ( oper = cb->first_op; oper != NULL; oper = oper->next_op )  
    {
        if ( !(DEP_INFO(oper)) ) continue;
	
	if ( last_sync )  {
		
   	    /* calculate sync instruction "flow" dependence, min = 0 */
	    /* from previous instructions to sync operation          */
	    distance = max_operand_time(MDES_INFO(last_sync),from_index) -
		    	   min_operand_time(MDES_INFO(oper),to_index);
	    if ( distance < 0 ) distance = 0;
	    L_add_output_dep(L_DEP_SYNC,distance,from_index,to_index,
					last_sync,oper,0);
	    L_add_input_dep(L_DEP_SYNC,distance,from_index,to_index,
				       last_sync,oper,0);
	}
	if ( (op_flag_set (oper->proc_opc, OP_FLAG_SYNC)) ||
	      (L_EXTRACT_BIT_VAL(oper->flags, L_OPER_SYNC))) 
	    last_sync = oper;
    }
    last_sync = NULL;
    for ( oper = cb->last_op; oper != NULL; oper = oper->prev_op )  {
        if ( !(DEP_INFO(oper)) ) continue;
	
	if ( last_sync && !((op_flag_set(oper->proc_opc, OP_FLAG_SYNC)) ||
			    (L_EXTRACT_BIT_VAL(oper->flags, L_OPER_SYNC)))) {
		
	    /* calculate sync instruction "flow" dependence, min = 0 */
	    /* from sync operations to subsequent instructions       */
	    distance = max_operand_time(MDES_INFO(oper),from_index) -
		    	   min_operand_time(MDES_INFO(last_sync),to_index);
	    if ( distance < 0 ) distance = 0;
		
	    L_add_output_dep(L_DEP_SYNC,distance,from_index,to_index,
					oper,last_sync,0);
	    L_add_input_dep(L_DEP_SYNC,distance,from_index,to_index,
				        oper,last_sync,0);
	}
	if ( (op_flag_set (oper->proc_opc, OP_FLAG_SYNC)) ||
	      (L_EXTRACT_BIT_VAL(oper->flags, L_OPER_SYNC))) 
	    last_sync = oper;
    }
}    

/*===============================================================================
 *
 *  END of Basic Block and Superblock dependence generation functions
 *
 *===============================================================================*/




/*===============================================================================
 *
 *  Hyperblock dependence generation functions
 *
 *===============================================================================*/

static void L_compute_hb_register_dependence(L_Cb *cb, int prepass)
{
    int i,j,to_index,from_index,distance;
    int n_conflict;
    L_Operand *conflict[64];
    L_Oper *oper;
    Dep_Operand *dep;
    Mdes_Info *mdes_info = NULL;
    L_Attr *attr;
    
    L_init_dep_hash_table();
    
    for ( oper = cb->first_op; oper != NULL; oper = oper->next_op )  {
        if ( !(DEP_INFO(oper)) ) continue;
	
	mdes_info = MDES_INFO(oper);
	
        /*
     	 *  	1. Compute register flow dependences.
     	 */  
	for ( i = 0; i < L_max_src_operand; i++ )  {
	    L_Operand *src;
	    if ( (src = oper->src[i]) == NULL ) continue;
	    
	    if (!(L_is_reg(src) || L_is_macro(src)))
		continue;
	    
	    /* determine conflicting operands */
	    n_conflict = L_conflicting_operands(src,conflict,64, prepass);
	    for ( j = 0 ; j < n_conflict; j++ )  {
	        /* does the operand exist in hash table ?? */
		dep = L_find_dep_operand(conflict[j]);
		
		while ( dep )  {
		    /* Draw dependence if operation's predicates can both be 1
		     * at the same time or if register's def is unconditional
		     * (independent of the predicate's value). -JCG 1/27/98
		     */
		    if (PG_intersecting_predicates_ops(dep->oper, oper) ||
			dep->uncond_def) 
		    {
			/* calculate flow dependence distance, minimum distance = 0 */
			to_index = operand_index(MDES_SRC,i);
			distance = min_operand_time(MDES_INFO(dep->oper),dep->index) -
		    	           max_operand_time(mdes_info,to_index);
			if ( distance < 0 ) distance = 0;
		
	        	/* yes, add flow dependence from dep->oper to oper */
	        	L_add_output_dep(L_DEP_REG_FLOW,distance,dep->index,to_index,
					dep->oper,oper,0);
	       	 	L_add_input_dep(L_DEP_REG_FLOW,distance,dep->index,to_index,
				       dep->oper,oper,0);
		    }
		    dep = L_find_another_dep_operand(dep);
		}
	    }
	    for (j = 0; j < n_conflict; j++) {
		L_delete_operand(conflict[j]);
	    }
	}
	/* Only one predicated supported for now - RAB */
	for ( i = 0; i < 1 /* L_max_pred_operand */; i++ )  {
	    L_Operand *pred;
	    if ( (pred = oper->pred[i]) == NULL ) continue;
	    
	    if (!(L_is_reg(pred) || L_is_macro(pred)))
		continue;
	    
	    /* determine conflicting operands */
	    n_conflict = L_conflicting_operands(pred,conflict,64, prepass);
	    for ( j = 0 ; j < n_conflict; j++ )  {
	        /* does the operand exist in hash table ?? */
		dep = L_find_dep_operand(conflict[j]);
		
		while ( dep )  {
		    /* Draw dependence if operation's predicates can both be 1
		     * at the same time or if register's def is unconditional
		     * (independent of the predicate's value). -JCG 1/27/98
		     */
		    if (PG_intersecting_predicates_ops(dep->oper, oper) ||
			dep->uncond_def) 
		    {
	    
			/* calculate flow dependence distance, minimum distance = 0 */
			to_index = operand_index(MDES_PRED,i);
			distance = min_operand_time(MDES_INFO(dep->oper),dep->index) -
		    	           max_operand_time(mdes_info,to_index);
			if ( distance < 0 ) distance = 0;
		 
	        	/* yes, add output depedence from dep->oper to oper */
	        	L_add_output_dep(L_DEP_REG_FLOW,distance,dep->index,to_index,
					dep->oper,oper,0);
	        	L_add_input_dep(L_DEP_REG_FLOW,distance,dep->index,to_index,
				       dep->oper,oper,0);
		    }
		    dep = L_find_another_dep_operand(dep);
		}
	    }
	    for (j = 0; j < n_conflict; j++) {
		L_delete_operand(conflict[j]);
	    }
	}

	/* SAM 8-95, flow deps for implicit JSR/RTS src operands */
        if ((Ldep_allow_lat_dangles_into_jsrs == 0) &&
                (op_flag_set(oper->proc_opc, OP_FLAG_JSR) ||
                 op_flag_set(oper->proc_opc, OP_FLAG_RTS))) {
            attr = L_find_attr(oper->attr, "tr");
            if (attr!=NULL) {
                for (i=0; i<attr->max_field; i++) {
                    L_Operand *macro;
                    macro = attr->field[i];
                    if (macro==NULL)
                        continue;
		    if (!L_is_macro(macro))
                        continue;
                    /* determine conflicting operands */
                    n_conflict = L_conflicting_operands(macro,conflict,64,prepass);
                    for ( j = 0 ; j < n_conflict; j++ )  {
                        /* does the operand exist in hash table ?? */
                        dep = L_find_dep_operand(conflict[j]);

			while (dep) {
			    /* Draw dependence if operation's predicates can both be 1
			     * at the same time or if register's def is unconditional
			     * (independent of the predicate's value). -JCG 1/27/98
			     */
			    if (PG_intersecting_predicates_ops(dep->oper, oper) ||
				dep->uncond_def) 
			    {
                                /* calculate flow dependence distance, minimum dist= 0,
                                   use DEP_SYNC_OPERAND as to index as a shortcut, so I
                                   don't need to declare a new sync operand */
                                to_index = operand_index(MDES_SYNC_IN,DEP_SYNC_OPERAND);
                                distance = min_operand_time(MDES_INFO(dep->oper),dep->index) -
                                           max_operand_time(mdes_info,to_index);
                                if ( distance < 0 ) distance = 0;

                                /* yes, add output depedence from dep->oper to oper */
                                L_add_output_dep(L_DEP_REG_FLOW,distance,dep->index,
							to_index, dep->oper,oper,0);
                                L_add_input_dep(L_DEP_REG_FLOW,distance,dep->index,
							to_index, dep->oper,oper,0);
			    }
			    dep = L_find_another_dep_operand(dep);
			}
                    }
		    for (j = 0; j < n_conflict; j++) {
			L_delete_operand(conflict[j]);
		    }
                }
            }
        }

	/*
     	 *	2. Compute register output dependences.
     	 */
	for ( i = 0; i < L_max_dest_operand; i++ )  {
	    L_Operand *dest;
	    if ( (dest = oper->dest[i]) == NULL ) continue;
	    
	    if (!(L_is_reg(dest) || L_is_macro(dest)))
		continue;
	    
	    n_conflict = L_conflicting_operands(dest,conflict,64, prepass);
	    for ( j = 0 ; j < n_conflict; j++ )  {
	        /* does the operand exist in hash table ?? */
		dep = L_find_dep_operand(conflict[j]);
		
		while ( dep )  { 
		    int pred2 = PREDICATE(dep->oper);
		    /* Draw dependence if operation's predicates can both be 1
		     * at the same time or if either registers' def is unconditional
		     * (independent of the predicate's value). -JCG 1/27/98
		     */
		    /* Assume special hardware support to allow AND/OR type
		     * predicates to be reordered.  Do not draw output dependence
		     * if both are AND or OR type predicates (mix and match
		     * is not allowed). -JCG 1/28/98
		     */
		    if ((PG_intersecting_predicates_ops(dep->oper, oper) ||
			 dep->uncond_def || Ldep_uncond_def (dest)) &&
			!Ldep_defs_may_reorder (dep, dest)) 
		    {
			/* calculate output dependence distance, minimum distance = 0 */
			to_index = operand_index(MDES_DEST,i);
			distance = min_operand_time(MDES_INFO(dep->oper),dep->index) -
		    	  	    max_operand_time(mdes_info,to_index);
			if ( distance < 0 ) distance = 0;

	        	/* yes, add output depedence from dep->oper to oper */
	        	L_add_output_dep(L_DEP_REG_OUTPUT,distance,dep->index,to_index,
					dep->oper,oper,0);
	        	L_add_input_dep(L_DEP_REG_OUTPUT,distance,dep->index,to_index,
				       dep->oper,oper,0);
		    }
		    /* Some predicated architectures may not have the bypassing support
		     * to resolve two writes to the same register in the same
		     * cycle, where the second write is predicated (so the first
		     * write's value may still be live).  If this is the case, 
		     * set Ldep_add_pred_hardware_arcs to 1 and the postpass scheduler
		     * will prevent two such writes from occuring in the same cycle
		     * (set the simulator parameter the same way!).  The simulator
		     * model treats the destination as another source, so a flow
		     * arch is drawn (and as a hack, it is drawn to the predicate
		     * to get the mdes latencies correct).  
		     *
		     * We do however always assume hardware support for multiple
		     * predicate writes in the same cycle (usualy OR type). 
		     * However, unconditional ones should also work since they
		     * ALWAYS write the predicates value and the old value will
		     * never be used. (Conditional predicates still need the
		     * extra dependence -JCG 1/28/98.)
		     */
		    if ( (pred2 != 0) && !prepass && Ldep_add_pred_hardware_arcs &&
			 ((!(L_is_ctype_predicate(dest))) || (dest->ptype == L_PTYPE_COND_F) ||
			  (dest->ptype == L_PTYPE_COND_F)))
		    {
			/* HACK, draw dep to predicate instead of dest to get proper latency */
			to_index = operand_index(MDES_PRED,0);
			distance = min_operand_time(MDES_INFO(dep->oper),dep->index) -
			    	    max_operand_time(mdes_info,to_index);
			
			if ( distance < 0 ) distance = 0;
			
			/* add register dependence */
	        	L_add_output_dep(L_DEP_REG_FLOW,distance,dep->index,to_index,
					dep->oper,oper,0);
	        	L_add_input_dep(L_DEP_REG_FLOW,distance,dep->index,to_index,
				       dep->oper,oper,0);
		    }
		    dep = L_find_another_dep_operand(dep);
		}
	    }
	    for (j = 0; j < n_conflict; j++) {
		L_delete_operand(conflict[j]);
	    }
	}

        /* SAM 8-95, output deps for implicit JSR dest operands */
        if ((Ldep_allow_lat_dangles_into_jsrs == 0) &&
                (op_flag_set(oper->proc_opc, OP_FLAG_JSR))) {
            attr = L_find_attr(oper->attr, "ret");
            if (attr!=NULL) {
                for (i=0; i<attr->max_field; i++) {
                    L_Operand *macro;
                    macro = attr->field[i];
                    if (macro==NULL)
                        continue;
		    if (!L_is_macro(macro))
                        continue;
                    /* determine conflicting operands */
                    n_conflict = L_conflicting_operands(macro,conflict,64,prepass);
                    for ( j = 0 ; j < n_conflict; j++ )  {
                        /* does the operand exist in hash table ?? */
                        dep = L_find_dep_operand(conflict[j]);

			while ( dep )  {
			    /* Draw dependence if operation's predicates can both be 1
			     * at the same time or if either registers' def is 
			     * unconditional (independent of the predicate's value). 
			     * -JCG 1/27/98
			     */
			    if (PG_intersecting_predicates_ops(dep->oper, oper) ||
				dep->uncond_def || Ldep_uncond_def (macro)) {

                                /* calculate output dependence distance, minimum dist= 0,
                                   use DEP_SYNC_OPERAND as to index as a shortcut, so I
                                   don't need to declare a new sync operand */
                                to_index = operand_index(MDES_SYNC_OUT,DEP_SYNC_OPERAND);
                                distance = min_operand_time(MDES_INFO(dep->oper),dep->index) -
                                           max_operand_time(mdes_info,to_index);
                                if ( distance < 0 ) distance = 0;

                                /* yes, add output depedence from dep->oper to oper */
                                L_add_output_dep(L_DEP_REG_OUTPUT,distance,dep->index,
							to_index, dep->oper,oper,0);
                                L_add_input_dep(L_DEP_REG_OUTPUT,distance,dep->index,
							to_index, dep->oper,oper,0);
			    }
			    dep = L_find_another_dep_operand(dep);
			}
                    }
		    for (j = 0; j < n_conflict; j++) {
			L_delete_operand(conflict[j]);
		    }
                }
            }
        }

	/*
	 * Update hash tables
	 */
	for ( i = 0; i < L_max_dest_operand; i++ )  {
	    L_Operand *dest;
	    if ( (dest = oper->dest[i]) == NULL ) continue;
	    
	    if (!(L_is_reg(dest) || L_is_macro(dest)))
		continue;
	    
	    /* detemine conflicting operands */
	    n_conflict = L_conflicting_operands(dest,conflict,64, prepass);
	    for ( j = 0 ; j < n_conflict; j++ )  {
	        /* does the operand exist in hash table ?? */
		dep = L_find_dep_operand(conflict[j]);
		if ( dep == NULL )  {
		    /* insert the operand into the hash table if it */
		    /* is not already present			    */
		    if ( L_same_register_operand(dest,conflict[j]) )
		        L_insert_dep_operand(operand_index(MDES_DEST,i),
					     conflict[j],oper);
		}
		else {
		    if ( L_same_register_operand(dest,conflict[j]) )  {
			/* if the operand is present and the predicate of dep->oper */
			/* implies the predicate of oper, switch the oper *         */
			int pred1 = PREDICATE(dep->oper);
			if (PG_subset_predicate_ops(dep->oper, oper) &&
			    !(L_is_ctype_predicate(dest)&&
			      !Ldep_uncond_def(dest)))
			  {
			    dep->ptype = dest->ptype;
		            dep->oper = oper;
			    dep->uncond_def = Ldep_uncond_def (dest);
			  }
			/* else, we need to add this definition to the hash table */
			else
			    L_insert_dep_operand(operand_index(MDES_DEST,i),
					         conflict[j],oper);
			dep = L_find_another_dep_operand(dep);
			while (dep)  {
			    Dep_Operand *tmp = L_find_another_dep_operand(dep);
			    pred1 = PREDICATE(dep->oper);
			    if (PG_subset_predicate_ops(dep->oper, oper) &&
				!(L_is_ctype_predicate(dest)&&
				  !Ldep_uncond_def(dest)))
			      L_remove_dep_operand(dep);
			    dep = tmp;
			}
		    }
#if 0   /* REH 2/20/94 - cannot delete if isn't identical to current destination */
		    else  {
			    /* if the operand is present, but is not identical to the */
			    /* the current destination and the predicate of dep->oper */
			    /* implies the predicate of oper, remove the operand      */
			    int pred1 = PREDICATE(dep->oper);
		    	    int pred2 = PREDICATE(oper);
			    if (PG_subset_predicate_ops(dep->oper, oper))
			   	L_remove_dep_operand(dep);
		    }
#endif
		}
	    }
	    for (j = 0; j < n_conflict; j++) {
		L_delete_operand(conflict[j]);
	    }
	}

	/* SAM 8-95, record JSR defining its return macros */
        if ((Ldep_allow_lat_dangles_into_jsrs == 0) &&
            (op_flag_set(oper->proc_opc, OP_FLAG_JSR))) {
            attr = L_find_attr(oper->attr, "ret");
            if (attr!=NULL) {
                for (i=0; i<attr->max_field; i++) {
                    L_Operand *macro;
                    macro = attr->field[i];
                    if (macro==NULL)
                        continue;
		    if (!L_is_macro(macro))
                        continue;
                    /* determine conflicting operands */
                    n_conflict = L_conflicting_operands(macro,conflict,64,prepass);
                    for ( j = 0 ; j < n_conflict; j++ )  {
                        /* does the operand exist in hash table ?? */
                        dep = L_find_dep_operand(conflict[j]);
                        if ( dep == NULL )  {
                            /* insert the operand into the hash table if it */
                            /* is not already present                       */
                            if ( L_same_register_operand(macro,conflict[j]) )
                                 L_insert_dep_operand(operand_index(MDES_SYNC_OUT,
                                        DEP_SYNC_OPERAND),conflict[j],oper);
                        }
                        else {
                            if ( L_same_register_operand(macro,conflict[j]) ) {
				/* if the operand is present and the predicate of
				   dep->oper implies the predicate of oper, switch
				   the oper */
				int pred1 = PREDICATE(dep->oper);
				if (PG_subset_predicate_ops(dep->oper, oper))
                                    dep->oper = oper;
				/* else, we need to add this definition to the hash tbl */
				else
				    L_insert_dep_operand(operand_index(MDES_DEST,i),
                                                         conflict[j],oper);
				dep = L_find_another_dep_operand(dep);
                        	while (dep)  {
                            	    Dep_Operand *tmp = L_find_another_dep_operand(dep);
                                    pred1 = PREDICATE(dep->oper);
                                    if (PG_subset_predicate_ops(dep->oper, oper))
                                        L_remove_dep_operand(dep);
                                    dep = tmp;
                                }
			    }
                        }
                    }
		    for (j = 0; j < n_conflict; j++) {
			L_delete_operand(conflict[j]);
		    }
                }
            }
        }
    }

    L_reset_dep_hash_table();
    
    for ( oper = cb->last_op; oper != NULL; oper = oper->prev_op )  {
        if ( !(DEP_INFO(oper)) ) continue;
	
    	/*
      	 *	3. Compute register anti dependences.
     	 */
	for ( i = 0; i < L_max_src_operand; i++ )  {
	    L_Operand *src;
	    if ( (src = oper->src[i]) == NULL ) continue;
	    
	    if (!(L_is_reg(src) || L_is_macro(src)))
		continue;
	    
	    n_conflict = L_conflicting_operands(src,conflict,64, prepass);
	    for ( j = 0 ; j < n_conflict; j++ )  {
	        /* does the operand exist in hash table ?? */
		dep = L_find_dep_operand(conflict[j]);
		while ( dep )  {
		    /* Draw dependence if operation's predicates can both be 1
		     * at the same time or if register's def is unconditional 
		     * (independent of the predicate's value). -JCG 1/27/98
		     */
		    if (PG_intersecting_predicates_ops(oper, dep->oper) ||
			dep->uncond_def) 
		    {
			/* calculate anti-dependence distance, minimum = 0 */
			from_index = operand_index(MDES_SRC,i);
			distance = max_operand_time(mdes_info,from_index) - 
		    	   	   min_operand_time(MDES_INFO(dep->oper),dep->index);
			if ( distance < 0 ) distance = 0;
		
	        	/* yes, add output depedence from dep->oper to oper */
	        	L_add_output_dep(L_DEP_REG_ANTI,distance,from_index,dep->index,
					oper,dep->oper,0);
	        	L_add_input_dep(L_DEP_REG_ANTI,distance,from_index,dep->index,
				       oper,dep->oper,0);
		    }
		    dep = L_find_another_dep_operand(dep);
		}
	    }
	    for (j = 0; j < n_conflict; j++) {
		L_delete_operand(conflict[j]);
	    }
	}

	/* SAM added 9-1-94 */
        for ( i = 0; i < 1 /* L_max_pred_operand */; i++ )  {
            L_Operand *pred;
            if ( (pred = oper->pred[i]) == NULL ) continue;

	    if (!(L_is_reg(pred) || L_is_macro(pred)))
                continue;

            n_conflict = L_conflicting_operands(pred,conflict,64, prepass);
            for ( j = 0 ; j < n_conflict; j++ )  {
                /* does the operand exist in hash table ?? */
                dep = L_find_dep_operand(conflict[j]);
                while ( dep )  {
		    /* Draw dependence if operation's predicates can both be 1
		     * at the same time or if register's def is unconditional 
		     * (independent of the predicate's value). -JCG 1/27/98
		     */
                    if (PG_intersecting_predicates_ops(oper, dep->oper) |
			dep->uncond_def) 
		    {
                        /* calculate anti-dependence distance, minimum = 0 */
                        from_index = operand_index(MDES_PRED,i);
                        distance = max_operand_time(mdes_info,from_index) -
                                   min_operand_time(MDES_INFO(dep->oper),dep->index);
                        if ( distance < 0 ) distance = 0;

                        /* yes, add output depedence from dep->oper to oper */
                        L_add_output_dep(L_DEP_REG_ANTI,distance,from_index,dep->index,
                                        oper,dep->oper,0);
                        L_add_input_dep(L_DEP_REG_ANTI,distance,from_index,dep->index,
                                       oper,dep->oper,0);
                    }
                    dep = L_find_another_dep_operand(dep);
                }
            }
	    for (j = 0; j < n_conflict; j++) {
		L_delete_operand(conflict[j]);
	    }
        }
	/* End SAM added 9-1-94 */

	/* SAM 8-95, anti deps for implicit JSR/RTS src operands */
        if ((Ldep_allow_lat_dangles_into_jsrs == 0) &&
                (op_flag_set(oper->proc_opc, OP_FLAG_JSR) ||
                 op_flag_set(oper->proc_opc, OP_FLAG_RTS))) {
            attr = L_find_attr(oper->attr, "tr");
            if (attr!=NULL) {
                for (i=0; i<attr->max_field; i++) {
                    L_Operand *macro;
                    macro = attr->field[i];
                    if (macro==NULL)
                        continue;
		    if (!L_is_macro(macro))
                        continue;
                    /* determine conflicting operands */
                    n_conflict = L_conflicting_operands(macro,conflict,64,prepass);
                    for ( j = 0 ; j < n_conflict; j++ )  {
                        /* does the operand exist in hash table ?? */
                        dep = L_find_dep_operand(conflict[j]);

			while ( dep )  {
			    /* Draw dependence if operation's predicates can both be 1
			     * at the same time or if register's def is unconditional 
			     * (independent of the predicate's value). -JCG 1/27/98
			     */
                            if (PG_intersecting_predicates_ops(oper, dep->oper) ||
				dep->uncond_def) 
			    {

                                /* calculate flow dependence distance, minimum dist= 0,
                                   use DEP_SYNC_OPERAND as to index as a shortcut, so I
                                   don't need to declare a new sync operand */
                                from_index = operand_index(MDES_SYNC_IN,DEP_SYNC_OPERAND);
                                distance = max_operand_time(mdes_info,from_index) -
                                           min_operand_time(MDES_INFO(dep->oper),dep->index);
                                if ( distance < 0 ) distance = 0;

                                /* yes, add anti depedence from dep->oper to oper */
                                L_add_output_dep(L_DEP_REG_ANTI,distance,from_index,
							dep->index, oper,dep->oper,0);
                                L_add_input_dep(L_DEP_REG_ANTI,distance,from_index,
							dep->index, oper,dep->oper,0);
			    }
			    dep = L_find_another_dep_operand(dep);
			}
                    }
		    for (j = 0; j < n_conflict; j++) {
			L_delete_operand(conflict[j]);
		    }
                }
            }
        }

	/*
	 * Update hash tables
	 */
	for ( i = 0; i < L_max_dest_operand; i++ )  { 
	    L_Operand *dest;
	    if ( (dest = oper->dest[i]) == NULL ) continue;
	    
	    if (!(L_is_reg(dest) || L_is_macro(dest)))
		continue;
	    
	    n_conflict = L_conflicting_operands(dest,conflict,64, prepass);
	    for ( j= 0 ; j < n_conflict; j++ )  {
	        /* does the operand exist in hash table ?? */
		dep = L_find_dep_operand(conflict[j]);
		if ( dep == NULL )  {
		    /* insert the operand into the hash table if it */
		    /* is not already present			    */
		    if ( L_same_register_operand(dest,conflict[j]) )
		        L_insert_dep_operand(operand_index(MDES_DEST,i),
					     conflict[j],oper);
		}
		else {
		    if ( L_same_register_operand(dest,conflict[j]) )  {
			/* if the operand is present and the predicate of dep->oper */
			/* implies the predicate of oper, switch the oper *         */
			int pred2 = PREDICATE(dep->oper);
			if (PG_subset_predicate_ops(dep->oper, oper)&&
			    !(L_is_ctype_predicate(dest)&&
			      ((dest->ptype==L_PTYPE_AND_T) ||
			      (dest->ptype==L_PTYPE_OR_T) ||
			      (dest->ptype==L_PTYPE_OR_F) ||
			      (dest->ptype==L_PTYPE_AND_F)))) /* JWS 3 Feb 98 */
			  {
			    dep->ptype = dest->ptype;
		            dep->oper = oper;
			    dep->uncond_def = Ldep_uncond_def (dest);
			  }
			/* else, we need to add this definition to the hash table */
			else
			    L_insert_dep_operand(operand_index(MDES_DEST,i),
					         conflict[j],oper);
			dep = L_find_another_dep_operand(dep);
			while (dep)  {
			    Dep_Operand *tmp = L_find_another_dep_operand(dep);
			    pred2 = PREDICATE(dep->oper);
			    if (PG_subset_predicate_ops(dep->oper, oper) &&
				!(L_is_ctype_predicate(dest)&&
				  ((dest->ptype==L_PTYPE_AND_T) ||
				   (dest->ptype==L_PTYPE_OR_T) ||
				   (dest->ptype==L_PTYPE_OR_F) ||
				   (dest->ptype==L_PTYPE_AND_F)))) /* JWS 3 Feb 98 */
				L_remove_dep_operand(dep);
			    dep = tmp;
			}
		    }
#if 0   /* REH 2/20/94 - cannot delete if isn't identical to current destination */
		    else  {
			    /* if the operand is present, but is not identical to the */
			    /* the current destination and the predicate of dep->oper */
			    /* implies the predicate of oper, remove the operand      */
			    int pred2 = PREDICATE(dep->oper);
		    	    int pred1 = PREDICATE(oper);
			    if (PG_subset_predicate_ops(dep->oper, oper))
			   	L_remove_dep_operand(dep);
		    }
#endif
		}
	    }
	    for (j = 0; j < n_conflict; j++) {
		L_delete_operand(conflict[j]);
	    }
	}

	/* SAM 8-95, record JSR defining its return macros */
        if ((Ldep_allow_lat_dangles_into_jsrs == 0) &&
            (op_flag_set(oper->proc_opc, OP_FLAG_JSR))) {
            attr = L_find_attr(oper->attr, "ret");
            if (attr!=NULL) {
                for (i=0; i<attr->max_field; i++) {
                    L_Operand *macro;
                    macro = attr->field[i];
                    if (macro==NULL)
                        continue;
		    if (!L_is_macro(macro))
                        continue;
                    /* determine conflicting operands */
                    n_conflict = L_conflicting_operands(macro,conflict,64,prepass);
                    for ( j = 0 ; j < n_conflict; j++ )  {
                        /* does the operand exist in hash table ?? */
                        dep = L_find_dep_operand(conflict[j]);
                        if ( dep == NULL )  {
                            /* insert the operand into the hash table if it */
                            /* is not already present                       */
                            if ( L_same_register_operand(macro,conflict[j]) )
                                 L_insert_dep_operand(operand_index(MDES_SYNC_OUT,
                                        DEP_SYNC_OPERAND),conflict[j],oper);
                        }
                        else {
                            if ( L_same_register_operand(macro,conflict[j]) ) {
				/* if the operand is present and the predicate of
				   dep->oper implies the predicate of oper, switch
				   the oper */
                                int pred2 = PREDICATE(dep->oper);
                                if (PG_subset_predicate_ops(oper, dep->oper))
                                    dep->oper = oper;
				/* else, we need to add this definition to the hash tbl */
                                else
                                    L_insert_dep_operand(operand_index(MDES_DEST,i),
                                                 	 conflict[j],oper);
                                dep = L_find_another_dep_operand(dep);
                                while (dep)  {
                                    Dep_Operand *tmp = L_find_another_dep_operand(dep);
                                    pred2 = PREDICATE(dep->oper);
                                    if (PG_subset_predicate_ops(dep->oper, oper))
                                        L_remove_dep_operand(dep);
                                    dep = tmp;
                                }
                            }
                        }
                    }
		    for (j = 0; j < n_conflict; j++) {
			L_delete_operand(conflict[j]);
		    }
                }
            }
        }
    }

    L_reset_dep_hash_table();
}

static void L_compute_hb_memory_dependence(L_Cb *cb)
{
    int out_index, in_index, distance, id = 0;
    L_Oper *oper;
    Dep_Info *load_head,*store_head;
    Dep_Info *st_dep1,*st_dep2, *ld_dep;
    int pred1,pred2;
    int dep_flags;

    dep_flags = SET_NONLOOP_CARRIED(0);

    if (Ldep_check_profiled_memory_dependences) 
	dep_flags |= SET_PROFILE_CONFLICT(0);

    /* construct linked lists of loads and store instructions */
    load_head = store_head = NULL;
    for ( oper = cb->first_op; oper != NULL; oper = oper->next_op )  {
	if ( op_flag_set(oper->proc_opc, OP_FLAG_STORE) )  {
	    DEP_INFO(oper)->level = id++;
	    DEP_INFO(oper)->oper = oper;
	    DEP_INFO(oper)->next = store_head;
	    store_head = DEP_INFO(oper);
	}
	else if ( op_flag_set(oper->proc_opc, OP_FLAG_LOAD) )  {
	    DEP_INFO(oper)->level = id++;
	    DEP_INFO(oper)->oper = oper;
	    DEP_INFO(oper)->next = load_head;
	    load_head = DEP_INFO(oper);
	}
    }
    
    out_index = operand_index(MDES_SYNC_OUT,DEP_MEM_OPERAND);
    in_index = operand_index(MDES_SYNC_IN,DEP_MEM_OPERAND);
    
    for ( st_dep1 = store_head; st_dep1 != NULL ; st_dep1 = st_dep1->next )  {
	/*
         *	1. Compute memory output dependences
         */
	pred1 = PREDICATE(st_dep1->oper);
	
	for ( st_dep2 = st_dep1->next ; st_dep2 != NULL; st_dep2 = st_dep2->next )  {
	    
	    pred2 = PREDICATE(st_dep2->oper);
	    if ( !PG_intersecting_predicates_ops(st_dep1->oper, st_dep2->oper))
		continue;
	    
	    if ( !L_independent_memory_ops(cb, st_dep1->oper, st_dep2->oper, dep_flags) )  {
		/* calculate memory output dependence distance, min = 0 */
		distance = min_operand_time(MDES_INFO(st_dep2->oper),out_index) -
		    	   max_operand_time(MDES_INFO(st_dep1->oper),out_index);
		if ( distance < 0 ) distance = 0;
		
		/* The linked list has the store instructions in reverse order, 
		    thus the output dependence goes from st_dep2->oper to
		    st_dep1->oper */
		L_add_output_dep(L_DEP_MEM_OUTPUT,distance,out_index,out_index,
				 st_dep2->oper,st_dep1->oper,0);
	        L_add_input_dep(L_DEP_MEM_OUTPUT,distance,out_index,out_index,
				st_dep2->oper,st_dep1->oper,0);
	    }
	    else  {
		if ( Ldep_debug_memory_disambiguation ) 
		   fprintf(stderr,"memory disambiguation: op %d ^ op %d\n",
			   st_dep2->oper->id, st_dep1->oper->id);
	    }
	}
	for ( ld_dep = load_head; ld_dep != NULL; ld_dep = ld_dep->next )  {
	    
	    pred2 = PREDICATE(ld_dep->oper);
	    if ( !PG_intersecting_predicates_ops(st_dep1->oper, ld_dep->oper))
		continue;
	    
	    if ( !L_independent_memory_ops(cb, st_dep1->oper, ld_dep->oper, dep_flags) )  {
		/*
         	 *    2. Compute memory flow dependences
         	 */
		if ( st_dep1->level < ld_dep->level )  {
		    /* calculate memory flow dependence distance, min = 0 */
	    	    distance = 
			min_operand_time(MDES_INFO(st_dep1->oper),out_index) -
		    	max_operand_time(MDES_INFO(ld_dep->oper),in_index);
	     	    if ( distance < 0 ) distance = 0;
		
	      	    L_add_output_dep(L_DEP_MEM_FLOW,distance,out_index,in_index,
				     st_dep1->oper,ld_dep->oper,0);
	    	    L_add_input_dep(L_DEP_MEM_FLOW,distance,out_index,in_index,
				    st_dep1->oper,ld_dep->oper,0);
		}
		/*
     		 *    3. Compute memory anti dependences
     	 	 */
		else {
		    /* calculate memory anti-dependence distance, min = 0 */
	    	    distance = 
			max_operand_time(MDES_INFO(ld_dep->oper),in_index) -
		        min_operand_time(MDES_INFO(st_dep1->oper),out_index);
	      	    if ( distance < 0 ) distance = 0;
	    
	    	    L_add_output_dep(L_DEP_MEM_ANTI,distance,in_index,out_index,
				     ld_dep->oper,st_dep1->oper,0);
	    	    L_add_input_dep(L_DEP_MEM_ANTI,distance,in_index,out_index,
				    ld_dep->oper,st_dep1->oper,0);
		}
	    }
	    else  {
		if ( Ldep_debug_memory_disambiguation ) 
		   fprintf(stderr,"memory disambiguation: op %d ^ op %d\n",
			   st_dep1->oper->id, ld_dep->oper->id);
	    }
	}
    }
}

static void L_compute_hb_control_dependence(L_Cb *cb, int prepass)
{
    int from_index, to_index, distance;
    L_Oper *oper; 
    Dep_Info *head;
    int pred1,pred2;
    
    from_index = operand_index(MDES_SYNC_OUT,DEP_CNT_OPERAND);
    to_index = operand_index(MDES_SYNC_IN,DEP_CNT_OPERAND);  
    
    /*
     *   Compute control "flow" dependence, from previous instruction to branch.
     */
    head = NULL;
    for ( oper = cb->first_op; oper != NULL; oper = oper->next_op )  {
        if ( !(DEP_INFO(oper)) ) continue;
	    
	pred2 = PREDICATE(oper);
	
	/* Examine the list to see if any of the opers require 
	   a control dependence on the current control instruction.  Once
	   an oper gets a control dependence, remove it from the list. */
	if ( op_flag_set(oper->proc_opc, OP_FLAG_CBR | OP_FLAG_JMP | 
			   OP_FLAG_RTS | OP_FLAG_JSR) )  {
	    Dep_Info *dep, *next_dep, *prev = NULL;
	    if ( op_flag_set(oper->proc_opc, OP_FLAG_CBR) || 
		 op_flag_set(oper->proc_opc, OP_FLAG_JMP) )  {
		for ( dep = head; dep != NULL; dep = next_dep )  {
		    next_dep = dep->next;
		    
		    pred1 = PREDICATE(dep->oper);
		    
		    /* Restructured to make this if correct when dealing with 
		     * unconditional predicate definitions. -JCG 1/27/98
		     */
		    if ( (PG_intersecting_predicates_ops(dep->oper, oper) &&
			  (!Ldep_allow_downward_code_perc)) ||
			 !L_safe_to_move_below_branch_hb(cb,dep->oper,oper) )  
		    {
			/* calculate control "flow" dependence distance, min = 0 */
	    	    	distance = max_operand_time(MDES_INFO(dep->oper),from_index) -
		               min_operand_time(MDES_INFO(oper),to_index);
	    	    	if ( distance < 0 ) distance = 0;
		
	      	    	L_add_output_dep(L_DEP_CNT,distance,from_index,to_index,
					dep->oper,oper,0);
	      	    	L_add_input_dep(L_DEP_CNT,distance,from_index,to_index,
				        dep->oper,oper,0);
			
			/* remove instruction from the list */
			if ((pred2==0) || (pred2==pred1)) {
			    if ( prev )
			        prev->next = dep->next;
			    else
			        head = dep->next;
			    dep->next = NULL;
		    	
			    continue;   /* we don't need to change the prev ptr */
			}
		    }
		    if ( prev )
			prev = prev->next;
		    else
			prev = head;
		}
	    }
	    else if ( op_flag_set(oper->proc_opc, OP_FLAG_JSR) )  {
		for ( dep = head; dep != NULL; dep = next_dep )  {
		    next_dep = dep->next;
		    
		    pred1 = PREDICATE(dep->oper);
		    
		    /* Changed to allow operations to move past jsr's only
		     * during prepass (even if predicated).  For now,
		     * fragile predicate macros may not work properly
		     * (too messy a fix for now). -JCG 1/27/98
		     */
		     if ( (!prepass ||
			   (PG_intersecting_predicates_ops(dep->oper, oper) &&
			    (!L_jsr_independent_oper(oper,dep->oper) ||
			     !Ldep_remove_jsr_dependences))))
		     {
			/* calculate control "flow" dependence distance, min = 0 */
	    	    	distance = max_operand_time(MDES_INFO(dep->oper),from_index) -
		               min_operand_time(MDES_INFO(oper),to_index);
	    	    	if ( distance < 0 ) distance = 0;
		
	      	    	L_add_output_dep(L_DEP_CNT,distance,from_index,to_index,
					dep->oper,oper,0);
	      	    	L_add_input_dep(L_DEP_CNT,distance,from_index,to_index,
				        dep->oper,oper,0);
			
			/* remove instruction from the list */
			if ((pred2==0) || (pred2==pred1)) {
			    if ( prev )
			        prev->next = dep->next;
			    else
			        head = dep->next;
			    dep->next = NULL;
		    	
			    continue;   /* we don't need to change the prev ptr */
			}
		    }
		    if ( prev )
			prev = prev->next;
		    else
			prev = head;
		}
	    }
	    else  {  
		/* The current control oper is OP_FLAG_JMP or OP_FLAG_RTS */
		/* so a control dependence is required		      */
	    	for ( dep = prev = head; dep != NULL; dep = next_dep )  {
		    next_dep = dep->next;
		    
		    pred1 = PREDICATE(dep->oper);
		    
		    if ( PG_intersecting_predicates_ops(dep->oper, oper)) {
		    
		    	/* calculate control "flow" dependence distance, min = 0 */
	    	    	distance = max_operand_time(MDES_INFO(dep->oper),from_index) -
		                   min_operand_time(MDES_INFO(oper),to_index);
	    	   	if ( distance < 0 ) distance = 0;
		
	      	   	L_add_output_dep(L_DEP_CNT,distance,from_index,to_index,
					dep->oper,oper,0);
	      	    	L_add_input_dep(L_DEP_CNT,distance,from_index,to_index,
				        dep->oper,oper,0);
			
			/* delete the oper from the list */
			if ((pred2==0) || (pred2==pred1)) {
			    if ( dep == head ) 
			        head = next_dep;
			    else
			        prev->next = next_dep;
			    dep->next = NULL;
			}
		    }
		    else
			prev = dep;
		}
	    }
	}
	
	/* add the current instruction to the list of opers w/o control dependence */
        DEP_INFO(oper)->oper = oper;
	DEP_INFO(oper)->next = head;
	head = DEP_INFO(oper);
    }	
    
    /*
     *   Compute control "flow" dependence, from branch to subsequent instr.
     */
    head = NULL;
    for ( oper = cb->last_op; oper != NULL; oper = oper->prev_op )  {
        if ( !(DEP_INFO(oper)) ) continue;
	    
	pred1 = PREDICATE(oper);
	
	/* Examine the list to see if any of the opers require 
	   a control dependence on the current control instruction.  Once
	   an oper gets a control dependence, remove it from the list. */
	if ( op_flag_set(oper->proc_opc, OP_FLAG_CBR | OP_FLAG_JMP | 
			   OP_FLAG_RTS | OP_FLAG_JSR) )  {
	    Dep_Info *dep, *next_dep, *prev = NULL;
	    if ( op_flag_set(oper->proc_opc, OP_FLAG_CBR) ||
 		 op_flag_set(oper->proc_opc, OP_FLAG_JMP) )  { 
		for ( dep = head; dep != NULL; dep = next_dep )  {
		    int perc_limit;
		    
		    next_dep = dep->next;
		    
		    /* the percolation limit of an instruction is dependent
		       upon whether the instruction is excepting or not */
		    if (!op_flag_set(dep->oper->proc_opc, OP_FLAG_EXCEPT) ||
			L_EXTRACT_BIT_VAL(dep->oper->flags, L_OPER_SAFE_PEI))
			perc_limit = Ldep_branch_perc_limit;
		    else
			perc_limit = Ldep_except_branch_perc_limit;

		    pred2 = PREDICATE(dep->oper);
		    
		    /* Restructured to make L_safe_to_move_above_branch_hb
		     * (and this if) correct when dealing with unconditional
		     * predicate definitions. -JCG 1/27/98
		     */
		    if ( (PG_intersecting_predicates_ops(oper, dep->oper) &&
			  (!Ldep_allow_upward_code_perc ||
			   ((perc_limit != -1) && (dep->level >= perc_limit)))) ||
			 !L_safe_to_move_above_branch_hb(cb,dep->oper,oper) )  {
			
			/* calculate control "anti-" dependence distance, min = 0 */
	    	        distance = max_operand_time(MDES_INFO(oper),from_index) -
		    	           min_operand_time(MDES_INFO(dep->oper),to_index);
	    	        if ( distance < 0 ) distance = 0;
		
	    	        L_add_output_dep(L_DEP_CNT,distance,from_index,to_index,
					oper,dep->oper,0);
	    	        L_add_input_dep(L_DEP_CNT,distance,from_index,to_index,
				        oper,dep->oper,0);
			
			/* remove instruction from the list */
			if ((pred1==0) || (pred2==pred1)) {
			    if ( prev )
			        prev->next = dep->next;
			    else
			        head = dep->next;
			    dep->next = NULL;
		    	
			    continue;   /* we don't need to change the prev ptr */
			}
		    }
		    if ( prev )
			prev = prev->next;
		    else
			prev = head;
		    
		    /* increment to percolation level of the oper */
		    dep->level += 1;
		}
	    }
	    else if ( op_flag_set(oper->proc_opc, OP_FLAG_JSR) )  {
		for ( dep = head; dep != NULL; dep = next_dep )  {
		    next_dep = dep->next;
		    
		    pred2 = PREDICATE(dep->oper);
		    
		    /* Changed to allow operations to move past jsr's only
		     * during prepass (even if predicated).  For now,
		     * fragile predicate macros may not work properly
		     * (too messy a fix for now). -JCG 1/27/98
		     */
		    if ( !prepass ||
			 (PG_intersecting_predicates_ops(oper, dep->oper) &&
			  (!L_jsr_independent_oper(oper, dep->oper) ||
			   !Ldep_remove_jsr_dependences)))
		    {
			/* calculate control "anti-" dependence distance, min = 0 */
	    	        distance = max_operand_time(MDES_INFO(oper),from_index) -
		    	           min_operand_time(MDES_INFO(dep->oper),to_index);
	    	        if ( distance < 0 ) distance = 0;
		
	    	        L_add_output_dep(L_DEP_CNT,distance,from_index,to_index,
					oper,dep->oper,0);
	    	        L_add_input_dep(L_DEP_CNT,distance,from_index,to_index,
				        oper,dep->oper,0);
			
			/* remove instruction from the list */
			if ((pred1==0) || (pred2==pred1)) {
			    if ( prev )
			        prev->next = dep->next;
			    else
			        head = dep->next;
			    dep->next = NULL;
			
		   	    continue;   /* we don't need to change the prev ptr */
			}
		    }
		    if ( prev )
			prev = prev->next;
		    else
			prev = head;
		}
	    }
	    else  {  
		/* The current control oper is OP_FLAG_JMP or OP_FLAG_RTS */
		/* so a control depedence is required		      */
	    	for ( dep = prev = head; dep != NULL; dep = next_dep )  {
		    next_dep = dep->next;
		    
		    pred2 = PREDICATE(dep->oper);
		    
		    if ( PG_intersecting_predicates_ops(oper, dep->oper)) {
		    	dep->next = NULL;  /* empty the list as we go */
		    
		    	/* calculate control "anti-" dependence distance, min = 0 */
	    	    	distance = max_operand_time(MDES_INFO(oper),from_index) -
		    	   	min_operand_time(MDES_INFO(dep->oper),to_index);
	    	    	if ( distance < 0 ) distance = 0;
		
	    	   	 L_add_output_dep(L_DEP_CNT,distance,from_index,to_index,
					oper,dep->oper,0);
	    	    	L_add_input_dep(L_DEP_CNT,distance,from_index,to_index,
				        oper,dep->oper,0);
			
		    	/* delete the oper from the list */
			if ((pred1==0) || (pred2==pred1)) {
			    if ( dep == head ) 
			        head = next_dep;
			    else
			        prev->next = next_dep;
			    dep->next = NULL;
			}
		    }
		    else
			prev = dep;
		}
	    }
	}
	else {
	    /* add the current instruction to the list of opers w/o control dependence */
	    /* NOTE: in this pass, control instructions are not added to the list */
	    
	    /* If SCHED_INFO does not exist, set the level to 0 */
	    if ( SCHED_INFO(oper) ) {
	        /* temporarily used to count percolation level */
	        DEP_INFO(oper)->level = SCHED_INFO(oper)->home_block -
				        SCHED_INFO(oper)->current_block;  
	    }
	    else
		DEP_INFO(oper)->level = 0;
	    
            DEP_INFO(oper)->oper = oper;
 	    DEP_INFO(oper)->next = head;
	    head = DEP_INFO(oper);
	}
    }

    if (Ldep_hb_keep_branch_order) {
        L_Oper *prev_br;
        L_Dep *dep;
        int control_dep_found;

        prev_br = NULL;
        for (oper = cb->first_op; oper; oper = oper->next_op) {
            if (L_general_branch_opcode(oper) || 
                L_general_subroutine_call_opcode(oper)) {
                if (prev_br) {
                    control_dep_found = FALSE;
                    for (dep = DEP_INFO(prev_br)->output_dep;
                         dep && !control_dep_found; dep = dep->next_dep) {
                        if (dep->to_oper == oper)
                            control_dep_found = TRUE;
                    }
                    if (!control_dep_found) {
                        L_add_output_dep (L_DEP_CNT, 0, from_index, to_index,
                                          prev_br, oper, 0);
                        L_add_input_dep (L_DEP_CNT, 0, from_index, to_index,
                                         prev_br, oper, 0);
                    }
                }
                prev_br = oper;
            }
        }
    }
}

static void L_compute_hb_synchronization_dependence(L_Cb *cb)
{ 
    int from_index, to_index, distance;
    L_Oper *oper, *last_sync;  
    int pred1,pred2;
    
    from_index = operand_index(MDES_SYNC_OUT,DEP_SYNC_OPERAND);
    to_index = operand_index(MDES_SYNC_IN,DEP_SYNC_OPERAND);
    
    last_sync = NULL;
    /*
     *  Compute synchronization dependence.
     */
    for ( oper = cb->first_op; oper != NULL; oper = oper->next_op )  
    {
        if ( !(DEP_INFO(oper)) ) continue;
	
	if ( last_sync )  {
	    pred2 = PREDICATE(oper);
	    if ( PG_intersecting_predicates_ops(last_sync, oper)) {
   	    	/* calculate sync instruction "flow" dependence, min = 0 */
	    	/* from previous instructions to sync operation          */
	    	distance = max_operand_time(MDES_INFO(last_sync),from_index) -
		    	   min_operand_time(MDES_INFO(oper),to_index);
	    	if ( distance < 0 ) distance = 0;
	    	L_add_output_dep(L_DEP_SYNC,distance,from_index,to_index,
					last_sync,oper,0);
	   	 L_add_input_dep(L_DEP_SYNC,distance,from_index,to_index,
				       last_sync,oper,0);
	    }
	}
	if ( (op_flag_set (oper->proc_opc, OP_FLAG_SYNC) ) ||
	     (L_EXTRACT_BIT_VAL(oper->flags, L_OPER_SYNC))) {
	    last_sync = oper;
	    pred1 = PREDICATE(oper);
	}
    }
    last_sync = NULL;
    for ( oper = cb->last_op; oper != NULL; oper = oper->prev_op )  {
        if ( !(DEP_INFO(oper)) ) continue;
	
	if ( last_sync && !((op_flag_set(oper->proc_opc, OP_FLAG_SYNC) ) ||
			    (L_EXTRACT_BIT_VAL(oper->flags, L_OPER_SYNC)))) {
	    pred2 = PREDICATE(oper);
	    if ( PG_intersecting_predicates_ops(last_sync, oper)) {
	    	/* calculate sync instruction "flow" dependence, min = 0 */
	    	/* from sync operations to subsequent instructions       */
	    	distance = max_operand_time(MDES_INFO(oper),from_index) -
		    	   min_operand_time(MDES_INFO(last_sync),to_index);
	    	if ( distance < 0 ) distance = 0;
		
	    	L_add_output_dep(L_DEP_SYNC,distance,from_index,to_index,
					oper,last_sync,0);
	    	L_add_input_dep(L_DEP_SYNC,distance,from_index,to_index,
				        oper,last_sync,0);
	    }
	}
	if ( (op_flag_set (oper->proc_opc, OP_FLAG_SYNC) ) ||
	     (L_EXTRACT_BIT_VAL(oper->flags, L_OPER_SYNC))) {
	    last_sync = oper;
	    pred1 = PREDICATE(oper);
	}
    }
}

static void L_compute_hb_blk_dependence(L_Cb *cb)
{
    int i,from_index, to_index, distance;
    int pred1, pred2;
    L_Oper  *oper, *last_blk_ld, *last_blk_st;
    
    last_blk_ld = NULL;
    from_index = operand_index(MDES_DEST,0);
    
    for ( oper = cb->first_op; oper != NULL; oper = oper->next_op )  {
	if ( !(DEP_INFO(oper)) ) continue;

	if ( last_blk_ld )  {
	    pred2 = PREDICATE(oper);
	    if ( PG_intersecting_predicates_ops(last_blk_ld, oper)) {
		
		/* flow dep from destination of pred_blk_ld to predicate of oper */
		to_index = operand_index(MDES_PRED,0);
		distance = min_operand_time(MDES_INFO(last_blk_ld),from_index) -
		    	   max_operand_time(MDES_INFO(oper),to_index);
		if ( distance < 0 ) distance = 0;
		
		L_add_output_dep(L_DEP_REG_FLOW,distance,from_index,to_index,
					last_blk_ld,oper,0);
	    	L_add_input_dep(L_DEP_REG_FLOW,distance,from_index,to_index,
				        last_blk_ld,oper,0);
	    }
		
	    /* output dep from dest of pred_blk_ld to predicate dest of oper */
	    for ( i = 0; i < L_max_dest_operand ; i++ )  {
		L_Operand *pred;
		if ( (pred = oper->dest[i]) == NULL ) continue;
		if (!L_is_ctype_predicate(pred)) continue;
		    
		/* Draw dependence if operation's predicates can both be 1
		 * at the same time or if register's def is unconditional 
		 * (independent of the predicate's value). -JCG 1/27/98
		 */
		if ( PG_intersecting_predicates_ops(last_blk_ld, oper) ||
		    Ldep_uncond_def (pred)) 
		{
		    to_index = operand_index(MDES_DEST,i);
		    distance = min_operand_time(MDES_INFO(last_blk_ld),from_index) -
		    	       max_operand_time(MDES_INFO(oper),to_index);
		    if ( distance < 0 ) distance = 0;
		   
		    L_add_output_dep(L_DEP_REG_OUTPUT,distance,from_index,to_index,
					last_blk_ld,oper,0);
	    	    L_add_input_dep(L_DEP_REG_OUTPUT,distance,from_index,to_index,
				        last_blk_ld,oper,0);
		}
	    }
	    /* WHY ARE NOT REG ANTI DEPENDENCES DRAWN??? -JCG 1/27/98 */
	}
	if ( oper->opc == Lop_PRED_LD_BLK )  {
	    last_blk_ld = oper;
	    pred1 = PREDICATE(oper);
	}
    }
    
    last_blk_st = NULL;
    to_index = operand_index(MDES_SRC,2);
    
    for ( oper = cb->last_op; oper != NULL; oper = oper->prev_op )  {
	if ( !(DEP_INFO(oper)) ) continue;

	if ( last_blk_st )  {
	    pred1 = PREDICATE(oper);
		
	    /* flow dep from predicate destination to pred_blk_st */
	    for ( i = 0; i < L_max_dest_operand; i++ )  {
		L_Operand *pred;
		if ( (pred = oper->dest[i]) == NULL ) continue;
		if (!L_is_ctype_predicate(pred)) continue;

		/* Draw dependence if operation's predicates can both be 1
		 * at the same time or if register's def is unconditional 
		 * (independent of the predicate's value). -JCG 1/27/98
		 */
		if ( PG_intersecting_predicates_ops(oper, last_blk_st) ||
		    Ldep_uncond_def (pred)) 
		{
		    from_index = operand_index(MDES_DEST,i);
		    distance = min_operand_time(MDES_INFO(oper),from_index) -
		    	       max_operand_time(MDES_INFO(oper),to_index);
		    if ( distance < 0 ) distance = 0;
		
		    L_add_output_dep(L_DEP_REG_FLOW,distance,from_index,to_index,
					oper,last_blk_st,0);
	    	    L_add_input_dep(L_DEP_REG_FLOW,distance,from_index,to_index,
				        oper,last_blk_st,0);
		}
	    }
	    /* WHY ARE NOT OTHER REG DEPENDENCES DRAWN??? -JCG 1/27/98 */
	}
	if ( oper->opc == Lop_PRED_ST_BLK )  {
	    last_blk_st = oper;
	    pred2 = PREDICATE(oper);
	}
    }
}

/*===============================================================================
 *
 *  END of Hyperblock dependence generation functions
 *
 *===============================================================================*/


/*--------------------------------------------------------------------------*/

static void L_resolve_dependence(L_Cb *cb)
{
    L_Oper *oper;
    Dep_Info *dinfo;
    
    if ( Ldep_resolve_all_memory_dep )
        Ldep_option |= L_DEP_IGNORE_ALL_MEMORY_DEP;
    if ( Ldep_resolve_all_control_dep )
        Ldep_option |= L_DEP_IGNORE_CNT_DEP;
    if ( Ldep_resolve_all_anti_output_dep ) {
        Ldep_option |= L_DEP_IGNORE_REG_ANTI_DEP;
        Ldep_option |= L_DEP_IGNORE_REG_OUTPUT_DEP;
    }
    /*
     *  Remove register dependences
     */
    if ( Ldep_option & L_DEP_IGNORE_REG_OUTPUT_DEP ) {
        for ( oper = cb->first_op; oper != NULL; oper = oper->next_op ) {
	    if ( (dinfo = DEP_INFO(oper)) == NULL ) continue;
            dinfo->input_dep = L_remove_dep(dinfo->input_dep,&dinfo->n_input_dep,
						     L_DEP_REG_OUTPUT,NULL,NULL);
            dinfo->output_dep = L_remove_dep(dinfo->output_dep,&dinfo->n_output_dep,
						     L_DEP_REG_OUTPUT,NULL,NULL);
        }
    }
    if ( Ldep_option & L_DEP_IGNORE_REG_ANTI_DEP ) {
        for ( oper = cb->first_op; oper != NULL; oper = oper->next_op ) {
	    if ( (dinfo = DEP_INFO(oper)) == NULL ) continue;
            dinfo->input_dep = L_remove_dep(dinfo->input_dep,&dinfo->n_input_dep,
						     L_DEP_REG_ANTI,NULL,NULL);
            dinfo->output_dep = L_remove_dep(dinfo->output_dep,&dinfo->n_output_dep,
						     L_DEP_REG_ANTI,NULL,NULL);
        }
    }
    if ( Ldep_option & L_DEP_IGNORE_REG_FLOW_DEP ) {
        for ( oper = cb->first_op; oper != NULL; oper = oper->next_op ) {
	    if ( (dinfo = DEP_INFO(oper)) == NULL ) continue;
            dinfo->input_dep = L_remove_dep(dinfo->input_dep,&dinfo->n_input_dep, 
						     L_DEP_REG_FLOW,NULL,NULL);
            dinfo->output_dep = L_remove_dep(dinfo->output_dep,&dinfo->n_output_dep,
						     L_DEP_REG_FLOW,NULL,NULL);
        }
    }
    /*
     *  Remove memory dependences
     */
    if ( Ldep_option & L_DEP_IGNORE_MEM_OUTPUT_DEP ) {
        for ( oper = cb->first_op; oper != NULL; oper = oper->next_op ) {
	    if ( (dinfo = DEP_INFO(oper)) == NULL ) continue;
            dinfo->input_dep = L_remove_dep(dinfo->input_dep,&dinfo->n_input_dep,
						     L_DEP_MEM_OUTPUT,NULL,NULL);
            dinfo->output_dep = L_remove_dep(dinfo->output_dep,&dinfo->n_output_dep,
						     L_DEP_MEM_OUTPUT,NULL,NULL);
        }
    }
    if ( Ldep_option & L_DEP_IGNORE_MEM_ANTI_DEP ) {
        for ( oper = cb->first_op; oper != NULL; oper = oper->next_op ) {
	    if ( (dinfo = DEP_INFO(oper)) == NULL ) continue;
            dinfo->input_dep = L_remove_dep(dinfo->input_dep,&dinfo->n_input_dep,
						     L_DEP_MEM_ANTI,NULL,NULL);
            dinfo->output_dep = L_remove_dep(dinfo->output_dep,&dinfo->n_output_dep,
						     L_DEP_MEM_ANTI,NULL,NULL);
        }
    }
    if ( Ldep_option & L_DEP_IGNORE_MEM_FLOW_DEP )  {
        for ( oper = cb->first_op; oper != NULL; oper = oper->next_op ) {
	    if ( (dinfo = DEP_INFO(oper)) == NULL ) continue;
            dinfo->input_dep = L_remove_dep(dinfo->input_dep,&dinfo->n_input_dep,
						     L_DEP_MEM_FLOW,NULL,NULL);
            dinfo->output_dep = L_remove_dep(dinfo->output_dep,&dinfo->n_output_dep,
						     L_DEP_MEM_FLOW,NULL,NULL);
        }
    }        
    /*
     *  Remove control dependences
     */
    if ( Ldep_option & L_DEP_IGNORE_CNT_DEP ) {
        for ( oper = cb->first_op; oper != NULL; oper = oper->next_op ) {
	    if ( (dinfo = DEP_INFO(oper)) == NULL ) continue;
            dinfo->input_dep = L_remove_dep(dinfo->input_dep,&dinfo->n_input_dep,
						     L_DEP_CNT,NULL,NULL);
            dinfo->output_dep = L_remove_dep(dinfo->output_dep,&dinfo->n_output_dep,
						     L_DEP_CNT,NULL,NULL);
        }
    }
}

/*--------------------------------------------------------------------------*/
/*
 *      Reduce flow dependence distance to 0.
 *      e.g.
 *              x = y cmp K1
 *              bge x K2
 */
static void L_allow_concurrent_issue(L_Cb *cb)
{
    L_Oper *opA, *opB;
    Dep_Info *dinfoA, *dinfoB;
    L_Dep *dep;
    
    /*   
     *	pattern 1:
     *  cmp/add/sub dest, src1, K1
     *  cond_br src1, K2
     */  
    for ( opA = cb->first_op; opA != NULL; opA = opA->next_op ) {
	if ( !L_cond_branch_opcode(opA) )
            continue;  

	if ((!L_is_int_constant(opA->src[0])) &&
	    (!L_is_int_constant(opA->src[1])))
	    continue;
	
        for ( opB = opA->prev_op; opB != NULL; opB = opB->prev_op) {
            /*
             *  match patterns.
             */  
            if ( !L_general_comparison_opcode(opB) &&
                 !L_int_add_opcode(opB) &&
                 !L_int_sub_opcode(opB) )
                continue;
	    
	if ((!L_is_int_constant(opB->src[0])) &&
	    (!L_is_int_constant(opB->src[1])))
                continue;
	    
            if ( !L_same_operand(opB->dest[0], opA->src[0]))
                continue;
            if ( !L_no_defs_between(opB->src[1], opB, opA) )
                continue;
            if ( !L_no_defs_between(opB->src[0], opB, opA) )
                continue;
            if ( !L_no_defs_between(opB->dest[0], opB, opA) )
                continue;
            if ( !L_no_danger(0, 0, 0, opB, opA) )
                break;
            /*
             *  replace patterns.
             */
            dinfoA = DEP_INFO(opA);
            dinfoB = DEP_INFO(opB);
            if ( (dinfoA == 0) || (dinfoB == 0) )
                L_punt("allow_concurrent_issue: bad operation extension",
                        L_ERR_INTERNAL);
	    
            dep = L_find_dep(dinfoB->output_dep, opB, opA, L_DEP_REG_FLOW);
            if ( dep == NULL )
                L_punt("allow_concurrent_issue: bad dependence graph",                        
		       L_ERR_INTERNAL);
	    dep->type = L_DEP_VLIW;
	    dep->from_index = operand_index(MDES_SYNC_OUT,DEP_VLIW_OPERAND);
    	    dep->to_index = operand_index(MDES_SYNC_IN,DEP_VLIW_OPERAND);
            dep->distance = 0;
	    
            dep = L_find_dep(dinfoA->input_dep, opB, opA, L_DEP_REG_FLOW);
            if ( dep == NULL )
                L_punt("allow_concurrent_issue: bad dependence graph",
                        L_ERR_INTERNAL);
            dep->type = L_DEP_VLIW;
	    dep->from_index = operand_index(MDES_SYNC_OUT,DEP_VLIW_OPERAND);
    	    dep->to_index = operand_index(MDES_SYNC_IN,DEP_VLIW_OPERAND);
            dep->distance = 0;
	    
	    if ( Ldep_debug_concurrent_issue )
                fprintf(stderr, "allow concurrent issue: op %d, op %d\n",
                    opB->id, opA->id);
        }
    }
    /*   
     *	pattern 2:
     *  add/sub dest, src1, K1
     *  ld/st/add/sub dest, K2
     */  
    for ( opA = cb->first_op; opA != NULL; opA = opA->next_op )  {
	if ( !L_int_add_opcode(opA) &&
	     !L_int_sub_opcode(opA) &&
	     !L_load_opcode(opA) &&
	     !L_store_opcode(opA) )
                continue;  
	    if ((!L_is_int_constant(opA->src[0])) &&
		(!L_is_int_constant(opA->src[1])))
                continue;
	    
        for ( opB = opA->prev_op; opB != NULL; opB = opB->prev_op)  {
            /*
             *  match patterns.
             */  
            if ( !L_int_add_opcode(opB) && !L_int_sub_opcode(opB) )
                continue;
	    if ((!L_is_int_constant(opB->src[0])) &&
		(!L_is_int_constant(opB->src[1])))
                continue;
            if ( !L_same_operand(opB->dest[0], opA->src[0]) )
                continue;
            if ( !L_no_defs_between(opB->src[1], opB, opA) )
                continue;
            if ( !L_no_defs_between(opB->src[0], opB, opA) )
                continue;
            if ( !L_no_defs_between(opB->dest[0], opB, opA) )
                continue;
            if ( !L_no_danger(0, 0, 0, opB, opA) )
                break;
            /*
             *  replace patterns.
             */
            dinfoA = DEP_INFO(opA);
            dinfoB = DEP_INFO(opB);
            if ( (dinfoA == 0) || (dinfoB == 0) )
                L_punt("allow_concurrent_issue: bad operation extension",
                        L_ERR_INTERNAL);
	    
	    dep = L_find_dep(dinfoB->output_dep, opB, opA, L_DEP_REG_FLOW);
            if ( dep == NULL )
                L_punt("allow_concurrent_issue: bad dependence graph",                        L_ERR_INTERNAL);
            dep->type = L_DEP_VLIW;
	    dep->from_index = operand_index(MDES_SYNC_OUT,DEP_VLIW_OPERAND);
    	    dep->to_index = operand_index(MDES_SYNC_IN,DEP_VLIW_OPERAND);
            dep->distance = 0;
	    
            dep = L_find_dep(dinfoA->input_dep, opB, opA, L_DEP_REG_FLOW);
            if ( dep == NULL )
                L_punt("allow_concurrent_issue: bad dependence graph",
                        L_ERR_INTERNAL);
            dep->type = L_DEP_VLIW;
	    dep->from_index = operand_index(MDES_SYNC_OUT,DEP_VLIW_OPERAND);
    	    dep->to_index = operand_index(MDES_SYNC_IN,DEP_VLIW_OPERAND);
            dep->distance = 0;
	    
	    if ( Ldep_debug_concurrent_issue )
                fprintf(stderr, "allow concurrent issue: op %d, op %d\n",
			opB->id, opA->id);
        }
    }
}

/*--------------------------------------------------------------------------*/

void L_compute_mem_copy_dependences(L_Cb *cb)
{
    L_Oper      *oper, *last_check=NULL, *last_copy_back=NULL;
    int         from_index, to_index, distance=0;

    from_index = operand_index(MDES_SYNC_OUT, DEP_SYNC_OPERAND);
    to_index = operand_index(MDES_SYNC_IN, DEP_SYNC_OPERAND);

#if 0
    /*
     * Add zero-cycle sync arcs between Lop_MEM_COPY and Lop_MEM_COPY_CHECK 
     * instructions to prevent them from re-ordering.
     */
    for (oper=cb->first_op; oper != NULL; oper = oper->next_op)
    {
        if (oper->opc == Lop_MEM_COPY)
	{
	    for (check = oper->next_op; check != NULL; check = check->next_op)
	    {
        	if (check->opc == Lop_MEM_COPY_CHECK)
		{
                    /* 
		     * Add the dependence arc from the mem_copy to 
		     * the current check 
		     */
                    L_add_output_dep(L_DEP_SYNC, distance, from_index, 
			to_index, oper, check,0);

                    L_add_input_dep(L_DEP_SYNC, distance, from_index, 
			to_index, oper, check,0);
		}
	    }
	}
    }
#endif

    /*
     * Add zero-cycle sync arcs between nearby Lop_MEM_COPY_CHECK and
     * Lop_MEM_COPY_BACK instructions to prevent reordering.
     */
    last_check = NULL;
    last_copy_back = NULL;
    for (oper=cb->first_op; oper != NULL; oper = oper->next_op)
    {
        if ((oper->opc == Lop_MEM_COPY_CHECK) && last_check &&
            Ldep_add_copy_check_dependences)
        {
            /* Add the dependence arc from last_check to the current check */
            L_add_output_dep(L_DEP_SYNC, distance, from_index, to_index,
                last_check, oper,0);

            L_add_input_dep(L_DEP_SYNC, distance, from_index, to_index,
                last_check, oper,0);

            last_check = oper;
        }
        else if ((oper->opc == Lop_MEM_COPY_BACK) && last_copy_back &&
            Ldep_add_copy_back_dependences)
        {
            /* Add the dependence arc from last_check to the current check */
            L_add_output_dep(L_DEP_SYNC, distance, from_index, to_index,
                last_copy_back, oper,0);

            L_add_input_dep(L_DEP_SYNC, distance, from_index, to_index,
                last_copy_back, oper,0);

            last_copy_back = oper;
        }
    }

    /*
     * Add dependence arcs from Lop_MEM_COPY_CHECK command down to
     * subsequent load and store instructions until a Lop_MEM_COPY_BACK
     * or a Lop_MEM_COPY_CHECK are encountered.
     */
    if (Ldep_add_copy_check_dependences)
    {
        for (oper=cb->first_op; oper != NULL; oper = oper->next_op)
        {
            if (oper->opc == Lop_MEM_COPY_CHECK)
            {
                last_check = oper;
            }
                else if (oper->opc == Lop_MEM_COPY_BACK)
                {
                last_check = NULL;
            }
            else if ((L_load_opcode(oper) || L_store_opcode(oper)) && last_check)
            {
                /* Add the dependence arc from last_check to the load or store */
                L_add_output_dep(L_DEP_SYNC, distance, from_index, to_index,
                    last_check, oper,0);

                L_add_input_dep(L_DEP_SYNC, distance, from_index, to_index,
                    last_check, oper,0);
            }
        }
    }


    /*
     * Add dependence arcs from Lop_MEM_COPY_BACK command up to previous
     * store instructions until a Lop_MEM_COPY_BACK or a Lop_MEM_COPY_CHECK
     * are encountered.
     */
    if (Ldep_add_copy_back_dependences)
    {
        for (oper=cb->last_op; oper != NULL; oper = oper->prev_op)
        {
            if (oper->opc == Lop_MEM_COPY_BACK)
            {
                last_copy_back = oper;
            }
            else if (oper->opc == Lop_MEM_COPY_CHECK)
            {
                last_copy_back = NULL;
            }
            else if (L_store_opcode(oper) && last_copy_back)
            {
                /* Add the dependence arc from last_copy_back to the load or store */
                L_add_output_dep(L_DEP_SYNC, distance, from_index, to_index,
                    oper, last_copy_back,0);

                L_add_input_dep(L_DEP_SYNC, distance, from_index, to_index,
                    oper, last_copy_back,0);
            }
        }
    }
}

/*--------------------------------------------------------------------------*/

void L_compute_dependence_level(L_Cb *cb)
{
    L_Oper *oper;
    
    for ( oper = cb->first_op; oper != NULL; oper = oper->next_op )  {
        int max;
	L_Dep *dep;
	Dep_Info *dep_info = DEP_INFO(oper);

	if ( dep_info == NULL ) continue;
	max = 0;	/* top level */
	for ( dep = dep_info->input_dep ; dep != NULL ; dep = dep->next_dep )  {
            /* don't look at cross-iteration deps when computing level */
            if (dep->omega == 0) {
                Dep_Info *dep_info2 = DEP_INFO(dep->from_oper);
                int n = dep_info2->level + dep->distance;
                if (n > max)
                    max = n;
            }
	}
        dep_info->level = max;
    }
}

/*--------------------------------------------------------------------------*/

/*==========================================
    Dependence Graph Cleanup Routines
 *==========================================*/

void L_delete_dependence_info(Dep_Info *dep_info)
{
    L_Dep *tmp,*next;
    
    if ( dep_info == NULL ) return;
    
    for ( tmp = dep_info->input_dep; tmp != NULL; tmp = next )  {
	next = tmp->next_dep;
	L_free(L_alloc_dep,tmp);
    }
    for ( tmp = dep_info->output_dep; tmp != NULL; tmp = next )  {
	next = tmp->next_dep;
	L_free(L_alloc_dep,tmp);
    }
    L_free(L_alloc_dep_info,dep_info);
}
 
void L_delete_dependence_graph(L_Cb *cb)
{
    L_Oper *oper;
    
    for ( oper = cb->first_op; oper != NULL; oper = oper->next_op ) 
    {
    	L_delete_dependence_info(DEP_INFO(oper));
	oper->dep_info = NULL;
    }
}

/*--------------------------------------------------------------------------*/

/*==========================================
    Dependence Graph Print Routines
 *==========================================*/

void L_print_dependences(FILE *F, L_Dep *dep)
{
    int cnt;
    L_Dep *ptr;
    
    cnt = 0;
    for ( ptr = dep; ptr != NULL ; ptr = ptr->next_dep ) {
	if ( cnt == 0 ) fprintf(F, "\t    ");
        fprintf(F, "(%d:%d->%d:%d::", ptr->from_oper->id, ptr->from_index,
				      ptr->to_oper->id, ptr->to_index);
        switch (ptr->type) {
        case L_DEP_REG_FLOW:    fprintf(F, "rf");       break;
        case L_DEP_REG_ANTI:    fprintf(F, "ra");       break;
        case L_DEP_REG_OUTPUT:  fprintf(F, "ro");       break;
        case L_DEP_MEM_FLOW:    fprintf(F, "mf");       break;
        case L_DEP_MEM_ANTI:    fprintf(F, "ma");       break;
        case L_DEP_MEM_OUTPUT:  fprintf(F, "mo");       break;
        case L_DEP_CNT:         fprintf(F, "cnt");      break;
        case L_DEP_SYNC:        fprintf(F, "sync");     break;
        default:                fprintf(F, "???");      break;
        }
        fprintf(F, ":%d", ptr->distance);
        fprintf(F, "[%d])", ptr->omega);
	
	if ( cnt == 2 )  {
	    cnt = 0;
	    fprintf(F, "\n");
	}
	else
	    cnt += 1;
    }
    fprintf(F, "\n");
}

void L_print_dependence_graph(FILE *F, L_Cb *cb)
{
    L_Oper *oper;
   
    fprintf(F,"*** CB %d ***\n",cb->id);
    for ( oper = cb->first_op ; oper != NULL ; oper = oper->next_op )  {
	L_print_oper(F,oper);
	if ( DEP_INFO(oper) )  {
	    fprintf(F, "\tLevel %d, Input dep %d, Output dep %d\n",
		DEP_INFO(oper)->level,DEP_INFO(oper)->n_input_dep,
		DEP_INFO(oper)->n_output_dep);
	    fprintf(F,"\tInput Dependences:\n");
    	    L_print_dependences(F, DEP_INFO(oper)->input_dep);
	    fprintf(F,"\tOutput Dependences:\n");
	    L_print_dependences(F, DEP_INFO(oper)->output_dep);
	}
	fprintf(F,"\n");
    }
    
}

/*--------------------------------------------------------------------------*/


Dep_Info *L_new_dep_info()
{
    Dep_Info *dinfo;
    
    dinfo = (Dep_Info *)L_alloc(L_alloc_dep_info);
    dinfo->level = 0;
    dinfo->n_input_dep = 0;
    dinfo->input_dep = NULL;
    dinfo->n_output_dep = 0;
    dinfo->output_dep = NULL;
    dinfo->spec_cond = DEP_NON_EXCEPTING;
	
    return(dinfo);
}

void L_insert_program_order_dependence(L_Cb *cb)
{   
  L_Oper *oper;
  int out_indx;
  int distance;
 
  for ( oper = cb->first_op; oper != NULL; oper = oper->next_op )  
    {
      if(!oper->next_op)
	continue;

      if ( !(DEP_INFO(oper)) ||
	   !(DEP_INFO(oper->next_op))) 
	continue;
      
      out_indx = operand_index(MDES_SYNC_OUT,DEP_MEM_OPERAND);

      distance = min_operand_time(MDES_INFO(oper->next_op), out_indx) -
        max_operand_time(MDES_INFO(oper), out_indx);

      if ( distance < 0 ) 
	distance = 0;

      L_add_output_dep(L_DEP_REG_FLOW, distance, out_indx, 
		       out_indx, oper->next_op, oper, 0);
      L_add_input_dep(L_DEP_REG_FLOW, distance, out_indx,
		      out_indx, oper->next_op, oper, 0);
    }
}

/*==========================================
    Dependence Construction Main Routine
 *==========================================*/

void L_build_dependence_graph(L_Cb *cb, int prepass, int mode)
{
    L_Oper *oper;

    /* Set Ldep_mode to determine whether or not cross-iteration dependence
       arcs are added to the graph. */
    Ldep_mode = mode;
    
    /*
     * 1.  (Re)Initialize dependece information
     */
    for ( oper = cb->first_op; oper != NULL; oper = oper->next_op ) { 
    	/*
     	 * 1.  (Re)Initialize dependece information
      	 */

        if ( op_flag_set(oper->proc_opc, OP_FLAG_IGNORE) ) continue;
	
	if ( DEP_INFO(oper) != NULL )
	    L_delete_dependence_info(DEP_INFO(oper));
	  
	oper->dep_info = (void *)L_new_dep_info();
    }

    /*
     *  2. Build dependence graph.
     */
    if ( L_EXTRACT_BIT_VAL(cb->flags,L_CB_HYPERBLOCK) )  {
	PG_setup_pred_graph(L_fn);
	if (Ldep_program_order && !prepass)
	  L_insert_program_order_dependence(cb);
	L_compute_hb_register_dependence(cb,prepass);
	L_compute_hb_blk_dependence(cb);
	L_compute_hb_memory_dependence(cb);
	L_compute_hb_control_dependence(cb,prepass);
	L_compute_hb_synchronization_dependence(cb);
	L_compute_mem_copy_dependences(cb);
    }
    else  {
      if (Ldep_program_order && !prepass)
	L_insert_program_order_dependence(cb);
      L_compute_register_dependence(cb, prepass);
      if (Ldep_mode == LDEP_MODE_CYCLIC) {
	L_compute_cross_iter_register_dependence(cb, prepass);
      }
      L_compute_memory_dependence(cb);
      if ((Ldep_mode == LDEP_MODE_CYCLIC) && L_use_sync_arcs) {
	L_compute_cross_iter_memory_dependence(cb);
      }
      L_compute_control_dependence(cb,prepass);
      L_compute_synchronization_dependence(cb);
      L_compute_mem_copy_dependences(cb);
    }
    
    /*
     *	3. Reduce dependences
     */
    if ( Ldep_resolve_dependences )
      {
	if ( L_EXTRACT_BIT_VAL(cb->flags,L_CB_HYPERBLOCK) )
	    L_warn("L_build_dependence_graph: can't resolve hyperblock dependences\n");
	else
	    L_resolve_dependence(cb);
      }

    if ( Ldep_allow_concurrent_issue )
      {
	if ( L_EXTRACT_BIT_VAL(cb->flags,L_CB_HYPERBLOCK) )
	    L_warn("L_build_dependence_graph: can't allow concurrent issue in hyperblock\n");
    	else
	  L_allow_concurrent_issue(cb);
      }

    /*
     *  4. Compute dependence level
     */
    L_compute_dependence_level(cb);
    
    if ( Ldep_print_dependence_graph )
        L_print_dependence_graph(stdout,cb);
}


/*--------------------------------------------------------------------------*/


