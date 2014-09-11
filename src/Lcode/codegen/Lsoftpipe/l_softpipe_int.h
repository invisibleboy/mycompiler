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
 *      File: l_softpipe.h
 *      Description: Structures and external function declarations for
 *                   modulo scheduler
 *      Creation Date: January, 1994
 *      Author: Daniel Lavery
 *
 *      Revision Date: Summer 1999
 *      Authors: IMPACT Technologies: Matthew Merten and John Gyllenhaal
\*****************************************************************************/

#ifndef L_SOFTPIPE_INT_H
#define L_SOFTPIPE_INT_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <Lcode/l_main.h>
#include <Lcode/r_regalloc.h>
#include <machine/lmdes.h>
#include <Lcode/sm.h>
#include <machine/m_impact.h>

#include <Lcode/l_softpipe.h>
#include <Lcode/l_markpipe.h>
#include <Lcode/l_mii.h>

#include "l_loop_prep.h"
#include "l_pipe_util.h"
#include "l_pipe_rename.h"
#include "l_softpipe_info.h"
#include "l_gen_pipe.h"

#define SOFTPIPE_OP_INFO(oper) ((Softpipe_Op_Info *)(oper->ext))
#define IMPOSSIBLE_PRIORITY -100	/* No oper should ever have
					   this priority */

#define Lpipe_ignore_kernel_inst(oper) ((oper->opc == Lop_DEFINE)|| \
                                        (oper->opc == Lop_NO_OP))

/* global vars - arrays of pointers to the first and last oper of
     each copy of the kernel code in the unrolled loop.  Copies are
     numbered 0 to unroll-1 */
extern L_Oper **kernel_copy_last_op;
extern L_Oper **kernel_copy_first_op;

/* other global variables */
extern L_Cb *header_cb;		/* this cb becomes the kernel */
extern L_Cb *preheader_cb;	/* original loop preheader */
extern L_Cb *prologue_cb;	/* prologue - there is no global
				   variable for the epilogue because
				   there can be more than one epilogue */
extern L_Cb *remainder_cb;	/* loop for executing remainder
				   iterations for remainder loop code
				   schema */
extern int Lpipe_stage_count;
extern int Lpipe_counted_loop;	/* flag to indicate that current loop is a
				   counted loop without early exits */
extern FILE *sched_file;	/* file to which to print the single
				   iteration schedule for each loop
				   scheduled */
extern int loop_dep_height;
extern int Lpipe_total_issue_slots;
#endif
