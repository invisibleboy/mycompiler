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
 *  File:  lmdes_build.c
 *
 *  Description:
 *    Reads hmdes file and builds internal data structures,
 *    and writes lmdes file.
 *
 *  Creation Date :  April, 1993 
 *
 *  Authors:  John C. Gyllenhaal, Wen-mei Hwu
 *
 *
\*****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <ctype.h>
#include <machine/hmdes.h>

char *program_name;

void
print_usage ()
{
  fprintf (stderr,
	   "usage: %s hmdes_file_name lmdes_file_name "
	   "[-Dname=value] [-verbose]\n", program_name);
  exit (1);
}


int
main (int argc, char **argv, char **envp)
{
  char *hmdes_file_name;
  char *lmdes_file_name;
  FILE *out;
  Hmdes *hmdes;
  int num_args, i;

  program_name = argv[0];

  num_args = 0;
  for (i = 1; argv[i] != 0; i++)
    {
      if ((argv[i][0] != '-') ||
	  ((argv[i][1] != 'D') && (strcmp (argv[i], "-verbose") != 0)))
	num_args++;
    }

  if (num_args != 2)
    print_usage ();

  hmdes_file_name = argv[1];
  lmdes_file_name = argv[2];

  /* Make sure they are not the same */
  if (strcmp (hmdes_file_name, lmdes_file_name) == 0)
    H_punt ("May not read and write the same file '%s'\n", hmdes_file_name);

  hmdes = create_hmdes (hmdes_file_name, argv, envp);

  /* Pull register file identifiers from header file just read */
  /* 
     associate_reg_files (hmdes);
   */
  /* Pull opcodes from header file just read */
  /*p 
     associate_opcodes (hmdes);
   */
  /* Open output file */
  if ((out = fopen (lmdes_file_name, "w")) == NULL)
    H_punt ("Error opening '%s' for writing", lmdes_file_name);

  printf ("  %s -> %s\n", hmdes_file_name, lmdes_file_name);

  /* Write lmdes out */
  write_lmdes (hmdes, out);

  return (0);
}
