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
/*! \file
 * \brief Parameter definitions for the Pcode library.
 *
 * \author David August, Grant Haab, Nancy Warter, Wen-mei Hwu
 *
 * Copyright (c) 1991 Grant Haab, Wen-mei Hwu, and the Board of Trustees
 * of the University of Illinois.
 * All rights reserved.
 *
 * Parameter definitions for the Pcode library.
 */
/*****************************************************************************/

#ifndef _PCODE_PARMS_H_
#define _PCODE_PARMS_H_

#include <config.h>
#include <library/l_parms.h>
#include "impact_global.h"


/*
 * The overall control is specified by the following options.
 */

/*----------------------------------------------------------------------*/
/* INPUT / OUTPUT */

extern char *F_input;
extern FILE *Fin;
extern char *F_input_ip;
extern FILE *Finput_ip;
extern char *F_output;
extern FILE *Fout;
extern char *F_output2;
extern FILE *Fout2;
extern char *F_out_ip;
extern FILE *Fout_ip;
extern char *F_error;
extern FILE *Ferr;
extern char *F_log;
extern FILE *Flog;
extern char *F_stat_pcode;
extern FILE *Fstpcode;
extern char *F_annot;
extern FILE *Fannot;
extern char *F_annot_index;
extern FILE *Fannot_index;
extern char *F_pcode_position;
extern FILE *Fpcode_position;
extern char *F_out_gvar;	/* BCC - 2/25/97 */
extern FILE *Fout_gvar;

/* LCW - file handlers for probe status file, dump prode code file and 
 * profiling data file - 10/12/95 
 */
extern FILE *Fallprobe;
extern FILE *Fdump_probe_code;
extern FILE *Fprofile;
extern FILE *Fnull;		/* LCW - 2/19/96 */
extern FILE *Falias_vars;


/*----------------------------------------------------------------------*/
/* VERBOSE */

/*! If set, constantly dump out internal compiler state. */
extern int debug_yes;		/* debug */

/*! If set, tell the user what's being done or will be done. */
extern int verbose_yes;		/* be verbose */

/*! If set, include line numbers in output. */
extern int line_yes;		/* include line numbers */

/*! If set, the P_Free* functions will print warnings for any non-null
 * pointer fields. */
extern int check_fields_on_free;

/*! If TRUE, the interprocedural symbol table (a.out.stl) is reused through
 * compilation.  If FALSE, a new interprocedural symbol table is written
 * with the specified file extension.  Defaults to FALSE. */
extern int reuse_ip_table;

/*! Determines if a module will allow input files to be modified in place 
 * (output file has same name as input file).  Defaults to TRUE. */
extern int allow_input_update_in_place;

/*! Prints a warning if input files are being modified in place.
 * Defaults to TRUE. */
extern int warn_on_update_in_place;

/*----------------------------------------------------------------------*/

/* automatic renice value for Pcode executable */
extern int pcode_nice_value;

/*----------------------------------------------------------------------*/
/* output in one of the specified form. */
/* Parallel C code is an intermediate form that uses calls to represent the
 * parallel data construct.  It can be compiled by a sequential C compiler
 * but will not produce the correct results since loop stmts will be
 * missing.  The reason for not replacing the parallel loop constructs with
 * sequential loop constructs is to allow for more aggressive sequential
 * optimizations.  We assume that the sequential optimizer will not move
 * code beyond the parallel stmts. 
 */
#define OUTPUT_NONE	0	/* no output */
#define OUTPUT_HCODE	1	/* convert to HCODE */
#define OUTPUT_PCODE	2	/* print out PCODE */
#define OUTPUT_NM	3	/* print out "nm -g" information */
#define OUTPUT_CCODE	4	/* CWL - convert Pcode to Ccode */
#define OUTPUT_LCODE	5	/* CWL - convert Pcode to Hcode */
extern int output_form;


/*----------------------------------------------------------------------*/
/* amount of work done */
extern int num_struct;
extern int num_union;
extern int num_enum;
extern int num_gvar;
extern int num_func;
extern int num_expandable_func;	/* BCC - 8/30/96 */
extern int num_total_func_call;	/* BCC - 8/30/96 */


/* flags for whether or not to open statistics files */
extern bool OPEN_STAT_PCODE;

/* ITI(JCG) - set in Psplit -2/99 */
extern int sp_create_layout_info_generator;

/* external functions */

extern void P_read_parm_Pcode (Parm_Parse_Info * ppi);

/* BCC - arch and model for casting - 6/12/95 */
extern void P_read_parm_arch (Parm_Parse_Info * ppi);
extern char *P_arch;
extern char *P_model;
extern char *P_swarch;
extern char *P_lmdes_file_name;

/* BCC - just translate - 11/6/96 */
extern int P_fast_mode;

#endif

















