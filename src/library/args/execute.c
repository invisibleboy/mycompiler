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
 *      File:   execute.c
 *      Author: Pohua Chang
 *
 *      Revision: March 25, Ada  Hsieh for PC/UNIVEL
 *
 *      Copyright (c) 1991 Pohua Chang, Wen-Mei Hwu. All rights reserved.  
 *      All rights granted to the University of Illinois.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>

#include <stdio.h>
#ifndef __WIN32__
#include <unistd.h>
#include <sys/wait.h>
#else
#include <process.h>
#endif
#include <library/execute.h>
#include <library/c_basic.h>

int error_code = 0;
static int dryrun = 0;
static int verbose = 0;

/*
 *      Initialize a command buffer.
 */
int
InitArgv (Argv argv)
{
  argv->argc = 0;
  argv->argv[0] = 0;
  return 0;
}
/*
 *      Add an argument to a command.
 *      Return -1 if the command line is too long.
 *      Otherwise, returns the current argument count.
 */
int
AddArgument (Argv argv, char *str)
{
  char line[512];
  int i;
  for (i = 0; i < 512; i++)
    {
      if ((str[i] == ' ') || (str[i] == '\0'))
        break;
      line[i] = str[i];
    }
  line[i] = '\0';
  if (argv->argc >= (MAX_ARGC - 1))
    return -1;
  argv->argv[argv->argc] = C_findstr (line);
  argv->argc += 1;
  argv->argv[argv->argc] = 0;
  return argv->argc;
}
/*
 *      Change the Nth argument. (0<N)
 */
int
ChangeArgument (Argv argv, int N, char *str)
{
  char line[512];
  int i;
  for (i = 0; i < 512; i++)
    {
      if ((str[i] == ' ') || (str[i] == '\0'))
        break;
      line[i] = str[i];
    }
  line[i] = '\0';
  if ((N < 1) || (N >= argv->argc))
    return -1;
  argv->argv[N - 1] = C_findstr (line);
  return N;
}
/*
 *      Enable/disable the execution system.
 */
int
DebugExecute (int flag)
{
  if (flag < 0)
    {
      dryrun = 1;
      verbose = 0;
    }
  else if (flag == 0)
    {
      dryrun = 0;
      verbose = 0;
    }
  else
    {
      dryrun = 0;
      verbose = 1;
    }
  return 0;
}
/*
 *      Execute a command and returns the status.
 */
int
Execute (Argv argv)
{
#ifndef __WIN32__

#if defined(i386) || defined(_SOLARIS_SOURCE)
  int status;
#else
  union wait status;
#endif
#endif
  if (dryrun)
    {
      int i;
      printf ("execute :");
      for (i = 0; i < argv->argc; i++)
        {
          printf (" %s", argv->argv[i]);
        }
      printf ("\n");
      return 0;
    }
  if (verbose)
    {
      int i;
      printf ("execute :");
      for (i = 0; i < argv->argc; i++)
        {
          printf (" %s", argv->argv[i]);
        }
      printf ("\n");
    }

#ifndef __WIN32__
  if (vfork () == 0)
    execvp (argv->argv[0], argv->argv);
  wait ((int *) &status);

#if defined(i386) || defined(_SOLARIS_SOURCE)
  if (WCOREDUMP (status))
    {                           /* core dump */
      error_code = 0;
      return CORE_DUMP;
    }
  else if (WTERMSIG (status))
    {                           /* program stops due to signal */
      error_code = WTERMSIG (status);
      return TERM_SIG;
    }
  else if (WEXITSTATUS (status) & 0x80)
    {                           /* program returns negative */
      error_code = 0;
      return EXIT_NEGATIVE;
    }
  else
    {                           /* normal return */
      error_code = 0;
      return WEXITSTATUS (status);
    }
#else
  if (status.w_coredump)
    {                           /* core dump */
      error_code = 0;
      return CORE_DUMP;
    }
  else if (status.w_termsig != 0)
    {                           /* program stops due to signal */
      error_code = status.w_termsig;
      return TERM_SIG;
    }
  else if (status.w_retcode & 0x80)
    {                           /* program returns negative */
      error_code = 0;
      return EXIT_NEGATIVE;
    }
  else
    {                           /* normal return */
      error_code = 0;
      return status.w_retcode;
    }
#endif

#else /* __WIN32__ */
  /* ADA 5/29/96: Win95/NT does have equvivalent forms for fork()/wait(), but
     the infra-structure is so different that we rather use the returned
     value just for now */

  return execvp (argv->argv[0], argv->argv);

#endif

}
