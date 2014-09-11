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
 *      File: l_markpipe.h
 *      Description: external declarations of loop marking functions
 *      Creation Date: April, 1997 - extracted from l_pipe_util.h
 *      Author: Daniel Lavery
 *
 *      Revision Date: Summer 1999
 *      Authors: IMPACT Technologies: Matthew Merten and John Gyllenhaal
\*****************************************************************************/

#ifndef L_MARKPIPE_H
#define L_MARKPIPE_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <stdio.h>
#include <Lcode/l_main.h>
#ifdef HAMM
#include <Lcode/smh.h>
#include <machine/tmdes_hamm.h>
#include <machine/hamm_internal.h>
#include <machine/hamm_debug.h>
#include <machine/m_tahoe.h>
#else
#include <Lcode/sm.h>
#include <machine/m_impact.h>
#endif
#include <Lcode/l_mii.h>

/*************************************************************************
                Macro definitions
*************************************************************************/
#define min(a,b) ((a) <= (b) ? (a) : (b))
/* 10/22/04 REK max is also defined in Lsuperscalar/l_strength.c. */
#ifndef max
#define max(a,b) ((a) >= (b) ? (a) : (b))
#endif

/*************************************************************************
                Lmarkpipe definitions
*************************************************************************/

#define L_OPER_START_NODE (L_OPER_RESERVED_TEMP7)
#define L_OPER_STOP_NODE  (L_OPER_RESERVED_TEMP8)
#define L_START_NODE(oper) (L_EXTRACT_BIT_VAL(oper->flags, L_OPER_START_NODE))
#define L_STOP_NODE(oper) (L_EXTRACT_BIT_VAL(oper->flags, L_OPER_STOP_NODE))
#define L_START_STOP_NODE(oper) (L_START_NODE(oper) || L_STOP_NODE(oper))
#define SM_START_STOP_DEP SM_USER_DEP_MARK1

/*************************************************************************
                Lmarkpipe global parameter declarations
*************************************************************************/

/* Externs need for Lsuperscalar */
extern int Lpipe_mark_potential_softpipe_loops;
extern int Lpipe_print_marking_statistics;
extern int Lpipe_min_ii;	/* If nonzero, limit ii
				   to the specified value.
				   (debug flag) */
extern int Lpipe_max_ii;        /* Prevent pipelining when II is too large. */
extern int Lpipe_max_stages;    /* Increase II until loop is under this
				   many stages. */

/*************************************************************************
                Lsoftpipe function declarations
*************************************************************************/

extern void L_read_parm_lmarkpipe (Parm_Parse_Info *);
extern int Lpipe_is_OK_softpipe_loop (L_Inner_Loop *, FILE *, int *, int);
extern int Lpipe_is_OK_ii(L_Inner_Loop *, FILE *, L_Attr *);

#endif
