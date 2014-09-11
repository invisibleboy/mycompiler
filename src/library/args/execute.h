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
 *      File:   execute.h
 *      Author: Pohua Paul Chang
 *      Copyright (c) 1991 Pohua Paul Chang, Wen-Mei Hwu . All rights reserved.  
 *      All rights granted to the University of Illinois.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/
#ifndef IMPACT_EXECUTE_H
#define IMPACT_EXECUTE_H

/* 10/29/02 REK Adding config.h */
#include <config.h>

#define MAX_ARGC        100

typedef struct _Argv
{
  int argc;
  char *argv[MAX_ARGC];
}
_Argv, *Argv;

#ifdef __cplusplus
extern "C"
{
#endif

  extern int InitArgv (Argv argv);
  extern int AddArgument (Argv argv, char *str);
  extern int ChangeArgument (Argv argv, int N, char *str);
  extern int DebugExecute (int flag);
  extern int Execute (Argv argv);

#ifdef __cplusplus
}
#endif

/*
 *      Normal execution should return a value
 *      between 0 and 127 inclusively.
 *      If a value larger than 127 is returned,
 *      the result is unpredictable.
 *      exit(-1) can be used to designate error
 *      condition, in which case, Execute() returns
 *      EXIT_NEGATIVE.
 */
#define EXIT_NEGATIVE   -1
#define CORE_DUMP       -2
#define TERM_SIG        -3

#ifdef __cplusplus
extern "C"
{
#endif

  extern int error_code;

#ifdef __cplusplus
}
#endif

#endif
