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
 *	File:	parms.c
 *	Author:	David August, Grant Haab, Nancy Warter and Wen-mei Hwu
 * 	Copyright (c) 1991 Grant Haab, Wen-mei Hm
 *		and The Board of Trustees of the University of Illinois.
 *		All rights reserved.
 *	The University of Illinois software License Agreement
 * 	specifies the terms and conditions for redistribution.
\*****************************************************************************/

#include <config.h>
#include <library/l_parms.h>
#include "impact_global.h"
#include "parms.h"

/*
 * 	This file is used to process the Pcode execution parameters.
 */
char *F_input = "stdin";	/* program input */
FILE *Fin = NULL;
char *F_output = "stdout";	/* program output */
FILE *Fout = NULL;
char *F_output2 = "stdout";	/* program output */
FILE *Fout2 = NULL;
char *F_out_ip = "impact.pc.ip";
FILE *Fout_ip;
char *F_input_ip = "impact.pc.ip1";
FILE *Finput_ip;
char *F_error = "stderr";	/* error file */
FILE *Ferr = NULL;
char *F_log = "stderr";		/* log file */
FILE *Flog = NULL;
char *F_out_gvar = "impact.pc.gvar";	/* BCC - 2/25/97 */
FILE *Fout_gvar;

int num_struct = 0;
int num_union = 0;
int num_enum = 0;
int num_gvar = 0;
int num_func = 0;
int num_expandable_func = 0;
int num_total_func_call = 0;

char *F_stat_pcode = "stderr";	/* file to record statistics for Pcode */
FILE *Fstpcode = NULL;

char *F_annot = "stdin";	/* file with annotation info */
FILE *Fannot = NULL;
char *F_annot_index = "stdin";	/* file with annotation info */
FILE *Fannot_index = NULL;
char *F_pcode_position = "stdout";	/* file for pcode positions/profile info */
FILE *Fpcode_position = NULL;

/* LCW - file handler for probing information and profiling files  - 10/12/95 */
FILE *Fallprobe;
FILE *Fdump_probe_code;
FILE *Fprofile;
FILE *Fnull;			/* LCW - 2/19/96 */
FILE *Falias_vars;

int debug_yes = FALSE;		/* do all checks */
int verbose_yes = FALSE;	/* be verbose */
int line_yes = FALSE;		/* include line #'s in output */
int check_fields_on_free = TRUE; /* warn when freeing a struct with non-null
				  * pointer field. */

int reuse_ip_table = FALSE;     /* Write a new interprocedural symbol table
				 * after each module. */

int allow_input_update_in_place = TRUE;
                                /* Determines if a module will allow input
				 * files to be modified in place (output file
				 * has same name as input). */
int warn_on_update_in_place = TRUE;
                                /* Prints a warning if a file is to be updated
				 * in place. */

int pcode_nice_value = 10;	/* automatic renice value for Pcode executable */

int P_allow_all_matching_suffixes = FALSE;

bool OPEN_STAT_PCODE = FALSE;

/* BCC - interface for Mspec - 6/12/95 */
char *P_arch = "impact";
char *P_model = "v1.0";
char *P_swarch = "default";
char *P_lmdes_file_name = 0;

/* BCC - no CFG, no analysis, no cast, no reduce, just plain translation - 11/6/96 */
int P_fast_mode = FALSE;


void
P_read_parm_Pcode (Parm_Parse_Info * ppi)
{
  /* Parameter file configuration - these parms must be read first */
  L_read_parm_b (ppi, "warn_parm_not_defined", &L_warn_parm_not_defined);
  L_read_parm_b (ppi, "warn_parm_defined_twice", &L_warn_parm_defined_twice);
  L_read_parm_b (ppi, "warn_parm_not_used", &L_warn_parm_not_used);
  L_read_parm_b (ppi, "dump_parms", &L_dump_parms);
  L_read_parm_s (ppi, "parm_warn_file_name", &L_parm_warn_file_name);
  L_read_parm_s (ppi, "parm_dump_file_name", &L_parm_dump_file_name);

  L_read_parm_s (ppi, "input_file", &F_input);
  L_read_parm_s (ppi, "log_file", &F_log);
  L_read_parm_s (ppi, "error_file", &F_error);
  L_read_parm_s (ppi, "pcode_statistics_file", &F_stat_pcode);
  L_read_parm_s (ppi, "annot_file", &F_annot);
  L_read_parm_s (ppi, "annot_index_file", &F_annot_index);
  L_read_parm_s (ppi, "pcode_position_file", &F_pcode_position);
  L_read_parm_b (ppi, "debug_log", &debug_yes);
  L_read_parm_b (ppi, "verbose_log", &verbose_yes);
  L_read_parm_b (ppi, "reuse_ip_table", &reuse_ip_table);

  /* BCC - fast mode - 11/6/96 */
  L_read_parm_b (ppi, "fast_mode", &P_fast_mode);
  L_read_parm_b (ppi, "generate_source_pos_in_output", &line_yes);

  L_read_parm_i (ppi, "nice_value", &pcode_nice_value);

  return;
}

void
P_read_parm_arch (Parm_Parse_Info * ppi)
{
  /* BCC - machine arch and model - 6/12/95 */
  L_read_parm_s (ppi, "arch", &P_arch);
  L_read_parm_s (ppi, "model", &P_model);
  L_read_parm_s (ppi, "swarch", &P_swarch);
  L_read_parm_s (ppi, "lmdes", &P_lmdes_file_name);
  return;
}


