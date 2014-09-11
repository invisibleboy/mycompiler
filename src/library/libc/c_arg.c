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
 *      File:   c_arg.c
 *      Author: Po-hua Chang
 *      Creation Date:  June 1990
 *      Modified By: XXX, date, time, why
 *      Copyright (c) 1991 Po-hua Chang and Wen-mei Hwu. All rights reserved.  
 *      All rights granted to the University of Illinois.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

/*===========================================================================
 *      Description :   Process C argc. argv
 *==========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <library/c_basic.h>
#include <library/c_arg.h>

int
C_get_arg (int argc, char **argv, C_Arg arg[], int n)
{
  int i, num;
  char *word;
  /*
   *  Reset the argument structure.
   */
  for (i = 0; i < n; i++)
    arg[i].option = 0;
  /*
   *  Find the starting location of option lists.
   */
  num = 0;
  for (i = 0; i < argc; i++)
    {
      word = argv[i];
      if (word[0] == '-')
        {                       /* create a new option descriptor */
          num++;
          if (num >= n)
            C_assert (0, "C_get_arg: too many options");
          arg[num - 1].option = C_savestr (word);
          arg[num - 1].spec = argv + (i + 1);
          arg[num - 1].count = 0;
        }
      else
        {
          arg[num - 1].count += 1;
        }
    }
  return num;
}
