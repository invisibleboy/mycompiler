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
/*===========================================================================*\
 *
 *  File:  l_time.h
 *
 *  Description:
 *    
 *
 *  Creation Date :  October 1995.
 *
 *  Author:  Richard E. Hank , Wen-mei Hwu
 *
 * 
 *===========================================================================*/
#ifndef REGION_TIME_H
#define REGION_TIME_H

/* 10/29/02 REK Adding config.h */
#include <config.h>

typedef struct L_Time
{
  double base_time;
  double current_time;
  double total_time;
#if 0
  long int base_ticks;
  long int current_ticks;
  long int total_ticks;
#endif
  double clk_tck;
}
L_Time;



#ifdef __cplusplus
extern "C"
{
#endif

  extern L_Time L_module_execution_time;
  extern L_Time L_module_global_dataflow_time;

  extern void L_annotate_function_with_cpu_time (L_Func * fn);
  extern double L_final_time (L_Time * time);
  extern void L_init_time (L_Time * time);
  extern void L_start_time (L_Time * time);
  extern void L_stop_time (L_Time * time);

#ifdef __cplusplus
}
#endif

#endif
