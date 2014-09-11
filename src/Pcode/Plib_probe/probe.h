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
 *      File:   probe.h
 *      Author: Le-Chun Wu and Wen-mei Hwu
\*****************************************************************************/

#ifndef _PROBE_H_
#define _PROBE_H_
#define NEW_PROBING 1

#include <config.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <Pcode/impact_global.h>
#include <Pcode/pcode.h>
#include <Pcode/parms.h>
#include <Pcode/struct.h>
#include <Pcode/query.h>
#include <Pcode/cast.h>
#include <library/i_error.h>

extern FILE *Fprvprobe;
extern FILE *Fprvloopid;
extern char *probe_array_name;
extern int next_probe;
extern int next_probe_BB;
extern int next_loop_id;	/* LCW - 3/24/99 */
extern int last_lp_id;		/* Tahir - 04/02/04 */
extern int next_ipc_id;		/* LCW - 3/24/99 */
extern int lp_id_counter_diff;

extern void PP_C_insert_middle_probe ();
extern void PP_C_insert_probe ();
/* LCW - functions for loop iter count profiling - 3/24/99 */
extern void PP_C_gen_loop_iter_counter_initial (FILE *, int, int);
extern void PP_C_insert_loop_iter_counter (FILE *, int);
extern void PP_C_gen_update_lp_iter_func (FILE *, int);
extern void PP_C_gen_update_lp_iter_func_in_middle (FILE *, int);
extern void PP_annotate_loop_iter_count (Stmt);
extern void PP_gen_init_c_code (int array_size, int loop_no, int ipc_no);
/* WCL */

extern void PP_annotate_ipc (Expr, int);

extern void PP_read_parm_Pprobe (Parm_Parse_Info * ppi);

extern int PP_probe;
extern int PP_probe_ip;
extern int PP_probe_lp;
extern int PP_annotate;
extern int PP_annotate_ip;
extern int PP_annotate_lp;

#endif /* _PROBE_H_ */
