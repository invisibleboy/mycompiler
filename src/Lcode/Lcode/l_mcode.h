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
 *  File:  l_mcode.h
 *
 *  Description: Mcode specific interface routines to support easier 
 *     creation/modification of Lcode/Mcode operations.
 *
 *  Creation Date :  February, 1993
 *
 *  Author:  Roger A. Bringmann, Scott Mahlke, Wen-mei Hwu
 *
 *  Revisions:
 *
 *
\*****************************************************************************/
#ifndef L_MCODE_H
#define L_MCODE_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <library/l_parms.h>

#ifdef __cplusplus
extern "C"
{
#endif

  extern void L_mcode_init_function (void);
  extern L_Oper *L_copy_parent_oper (L_Oper *);
  extern L_Oper *L_convert_to_parent (L_Cb *, L_Oper *);
  extern L_Attr *L_attr_defined (L_Oper *, char *);
  extern void L_get_attribute (L_Oper *, int *);
  extern void L_set_attribute (L_Oper *, int);
  extern void L_get_preload (L_Oper *, int *);
  extern void L_set_preload (L_Oper *, int);

  extern void L_read_parm_mcode (Parm_Parse_Info *);

  extern int L_debug_messages;
  extern int L_debug_memory_usage;
  extern int L_codegen_phase;
  extern int L_do_machine_opt;
  extern int L_do_software_pipelining;
  extern int L_do_prepass_sched;
  extern int L_do_register_allocation;
  extern int L_do_postpass_code_annotation;
  extern int L_do_peephole_opt;
  extern int L_do_postpass_sched;
  extern int L_print_lcode_phase_3;
  extern int L_print_mcode_phase_3;
  extern int L_loop_unrolled;
  extern int L_do_recovery_code;
  extern int L_do_super_speculation;

#ifdef __cplusplus
}
#endif

#endif
