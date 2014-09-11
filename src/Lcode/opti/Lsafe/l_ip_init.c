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
 *  File:  l_ip_main.c
 *
 *  Description:  
 *	Main entry point for safe speculation.
 *
 *  Creation Date :  May 1994
 *
 *  Author:  Roger A. Bringmann and Wen-mei Hwu
 *
 *
 *  Copyright (c) 1994 Roger A. Bringmann , Wen-mei Hwu and The Board of
 *		  Trustees of the University of Illinois. 
 *		  All rights reserved.
 *
 *  The University of Illinois software License Agreement
 *  specifies the terms and conditions for redistribution.
 *
\*****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "l_interproc.h"

void L_read_parm_safe (Parm_Parse_Info *ppi);


/******************************************************************************\
 *
 *
 *
\******************************************************************************/

L_Alloc_Pool    *L_alloc_value = NULL;

L_Alloc_Pool    *L_alloc_cg_arc = NULL;
L_Alloc_Pool    *L_alloc_cg_node = NULL;

L_Alloc_Pool    *L_alloc_db_entry = NULL;
L_Alloc_Pool    *L_alloc_db_callsite = NULL;

L_Alloc_Pool	*L_alloc_resolved = NULL;

L_Alloc_Pool	*L_alloc_reg = NULL;
L_Alloc_Pool	*L_alloc_reg_bank = NULL;
L_Alloc_Pool	*L_alloc_reg_file = NULL;

L_Alloc_Pool	*L_alloc_memory = NULL;
L_Alloc_Pool	*L_alloc_memory_cell = NULL;

L_Alloc_Pool	*L_alloc_ud_graph = NULL;
L_Alloc_Pool	*L_alloc_arc = NULL;
L_Alloc_Pool	*L_alloc_node = NULL;
L_Alloc_Pool	*L_alloc_retrace = NULL;
L_Alloc_Pool	*L_alloc_def_oper = NULL;
L_Alloc_Pool	*L_alloc_def_list = NULL;

L_Operand	*zero;

int		building_callgraph=0;

int		L_debug_callgraph=0;
int		L_mark_sef_jsr=1;
char*		L_sef_file_ext=NULL;
int		L_mark_safe_pei=0;
int		L_analysis_level=INTER_PROCEDURAL_ANALYSIS;
int		L_print_callgraph=0;
int		L_add_unknown_arcs=1;

void L_interproc_init(Parm_Macro_List *command_line_macro_list)
{
    L_load_parameters (L_parm_file, command_line_macro_list,
		       "(Lsafe", L_read_parm_safe);

    if (!((L_analysis_level==TRIVIAL_ANALYSIS) ||
          (L_analysis_level==INTRA_PROCEDURAL_ANALYSIS) ||
          (L_analysis_level==INTER_PROCEDURAL_ANALYSIS))) 
	L_punt("L_interproc_init: invalid analysis level of %d\n",
	    L_analysis_level);

    /* Memory pools for callgraph */
    L_alloc_cg_arc = L_create_alloc_pool("CG_Arc", sizeof(CG_Arc), 64);
    L_alloc_cg_node = L_create_alloc_pool("CG_Node", sizeof(CG_Node), 64);    

    /* Memory pools for interprocedural database */
    L_alloc_db_entry = L_create_alloc_pool("Database_Entry",
        sizeof(Database_Entry), 100);
    L_alloc_db_callsite = L_create_alloc_pool("Database_Callsite",
        sizeof(Database_Callsite), 100);

    /* Memory pools for resolved registers and memory */
    L_alloc_resolved = L_create_alloc_pool("Resolved", sizeof(Resolved), 1);

    /* Memory pools for values in registers and memory */
    L_alloc_value = L_create_alloc_pool("Value", sizeof(Value), 128);

    /* Memory pools for pseudo-register file */
    L_alloc_reg = L_create_alloc_pool("Reg", sizeof(Reg), 64);
    L_alloc_reg_file = L_create_alloc_pool("RegFile", sizeof(RegFile), 8);
    L_alloc_reg_bank = L_create_alloc_pool("RegBank", sizeof(RegBank), 16);

    /* Memory pools for pseudo-memory space */
    L_alloc_memory = L_create_alloc_pool("Memory", sizeof(Memory), 4);
    L_alloc_memory_cell = L_create_alloc_pool("Memory_Cell", 
	sizeof(Memory_Cell), 64);

    /* Memory pools for use/def graph */
    L_alloc_ud_graph = L_create_alloc_pool("UD_Graph", sizeof(UD_Graph), 16);
    L_alloc_arc = L_create_alloc_pool("UD_Arc", sizeof(UD_Arc), 128);
    L_alloc_def_oper = L_create_alloc_pool("Def_Oper", sizeof(Def_Oper), 128);
    L_alloc_def_list = L_create_alloc_pool("Def_List", sizeof(Def_List), 64);
    L_alloc_node = L_create_alloc_pool("UD_Node", sizeof(UD_Node), 128);
    L_alloc_retrace = L_create_alloc_pool("Retrace", sizeof(Retrace), 128);

    L_database_init();

    zero = L_new_gen_int_operand(0);
}

/******************************************************************************\
 *
 *
 *
\******************************************************************************/

void L_read_parm_safe (Parm_Parse_Info *ppi)
{
    L_read_parm_b(ppi, "debug_callgraph", &L_debug_callgraph);
    L_read_parm_b(ppi, "mark_sef_jsr", &L_mark_sef_jsr);
    L_read_parm_s(ppi, "sef_file_extension", &L_sef_file_ext);
    L_read_parm_b(ppi, "mark_safe_pei", &L_mark_safe_pei);
    L_read_parm_i(ppi, "analysis_level", &L_analysis_level);
    L_read_parm_b(ppi, "add_unknown_arcs", &L_add_unknown_arcs);
    L_read_parm_b(ppi, "print_callgraph", &L_print_callgraph); 
}
