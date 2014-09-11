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
 *
 *      File :          l_time.c
 *      Description :   Compilation time routines....
 *      Creation Date : October 1995
 *      Author :        Richard Hank, Wen-mei Hwu
 *                 - Currently supports only HP-UX 9.x and SunOS
 *
 *
 *===========================================================================*/
/*===========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <stdio.h>
#include <Lcode/l_main.h>
#include "l_time.h"

L_Time L_module_execution_time;
L_Time L_module_global_dataflow_time;

/* 
 * Necessary stuff to time the execution of an Lcode module.
 * For the moment, this only works for HP-UX.
 */
#if defined(__hpux)
#include <sys/times.h>
#include <unistd.h>
#endif
#if defined(sun) || defined(X86LIN_SOURCE) || defined(IA64LIN_SOURCE)
#include <sys/time.h>
#include <sys/resource.h>
#endif

void
L_init_time (L_Time * time)
{
#if defined (__hpux)
  struct tms buffer;
  double clk_tck;

  clk_tck = (double) sysconf (_SC_CLK_TCK);

  times (&buffer);
  time->base_time = (buffer.tms_utime) / clk_tck;

#elif defined(sun) || defined(X86LIN_SOURCE) || defined(IA64LIN_SOURCE)

  struct rusage rusage;

  getrusage (RUSAGE_SELF, &rusage);
  time->base_time = (double) rusage.ru_utime.tv_sec +
    (double) rusage.ru_utime.tv_usec / 1000000.0;

#else
  time->base_time = 0.0;
#endif
  time->total_time = 0.0;
}

void
L_stop_time (L_Time * time)
{
  double current_time;

#if defined (__hpux)
  struct tms buffer;
  double clk_tck;

  clk_tck = (double) sysconf (_SC_CLK_TCK);
  times (&buffer);

  current_time = buffer.tms_utime / clk_tck;

#elif defined(sun) || defined(X86LIN_SOURCE) || defined(IA64LIN_SOURCE)

  struct rusage rusage;

  getrusage (RUSAGE_SELF, &rusage);
  current_time = (double) rusage.ru_utime.tv_sec +
    (double) rusage.ru_utime.tv_usec / 1000000.0;
#else
  current_time = 0.0;
#endif
  time->total_time += current_time - time->base_time;
}

void
L_start_time (L_Time * time)
{
#if defined (__hpux)
  struct tms buffer;
  double clk_tck;

  clk_tck = (double) sysconf (_SC_CLK_TCK);
  times (&buffer);

  time->base_time = buffer.tms_utime / clk_tck;

#elif defined(sun) || defined(X86LIN_SOURCE) || defined(IA64LIN_SOURCE)

  struct rusage rusage;

  getrusage (RUSAGE_SELF, &rusage);
  time->base_time = (double) rusage.ru_utime.tv_sec +
    (double) rusage.ru_utime.tv_usec / 1000000.0;
#else
  time->base_time = 0.0;
#endif
}

double
L_final_time (L_Time * time)
{
  return (time->total_time);
}



void
L_annotate_function_with_cpu_time (L_Func * fn)
{
  L_Attr *attr;
  double cpu_time;
  double dataflow_time;
  double bdd_time;
  double ssa_time;
  void *data_end;

  /* Moved to L_print_func() -JCG 6/99
   * L_stop_time(&L_module_execution_time);
   */
  dataflow_time = L_final_time (&L_module_global_dataflow_time);
  cpu_time = L_final_time (&L_module_execution_time);
  bdd_time = L_final_time (&PG_bdd_build_time);
  ssa_time = L_final_time (&PG_ssa_build_time);

  if ((attr = L_find_attr (fn->attr, "cpu_time")) != NULL)
    {
      L_set_double_attr_field (attr, 0, cpu_time);
    }
  else
    {
      attr = L_new_attr ("cpu_time", 1);
      L_set_double_attr_field (attr, 0, cpu_time);
      fn->attr = L_concat_attr (fn->attr, attr);
    }
  if ((attr = L_find_attr (fn->attr, "df_time")) != NULL)
    {
      L_set_double_attr_field (attr, 0, dataflow_time);
    }
  else
    {
      attr = L_new_attr ("df_time", 1);
      L_set_double_attr_field (attr, 0, dataflow_time);
      fn->attr = L_concat_attr (fn->attr, attr);
    }
  if ((attr = L_find_attr (fn->attr, "pg_ssa_time")) != NULL)
    {
      L_set_double_attr_field (attr, 0, ssa_time);
    }
  else
    {
      attr = L_new_attr ("pg_ssa_time", 1);
      L_set_double_attr_field (attr, 0, ssa_time);
      fn->attr = L_concat_attr (fn->attr, attr);
    }
  if ((attr = L_find_attr (fn->attr, "pg_bdd_time")) != NULL)
    {
      L_set_double_attr_field (attr, 0, bdd_time);
    }
  else
    {
      attr = L_new_attr ("pg_bdd_time", 1);
      L_set_double_attr_field (attr, 0, bdd_time);
      fn->attr = L_concat_attr (fn->attr, attr);
    }
#ifndef __WIN32__
  data_end = sbrk (0);
#else
/* ADA 5/29/96: There is no easy way to emulate sbrk() on Win95/NT */
  data_end = L_data_segment_start;
#endif
  if ((attr = L_find_attr (fn->attr, "mem_usage")) != NULL)
    {
      L_set_int_attr_field (attr, 0, (ITuintmax)(long)data_end - 
			    (ITuintmax)(long)L_data_segment_start);
    }
  else
    {
      attr = L_new_attr ("mem_usage", 1);
      L_set_int_attr_field (attr, 0, (ITuintmax)(long)data_end - 
			    (ITuintmax)(long)L_data_segment_start);
      fn->attr = L_concat_attr (fn->attr, attr);
    }

}
