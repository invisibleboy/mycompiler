/*****************************************************************************\
 *
 *                    Illinois Open Source License
 *                     University of Illinois/NCSA
 *                         Open Source License
 *
 * Copyright (c) 2004, The University of Illinois at Urbana-Champaign.
 * All rights reserved.
 *
 * Developed by:
 *
 *              IMPACT Research Group
 *
 *              University of Illinois at Urbana-Champaign
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
 *      File:    pipa_consg.c
 *      Author:  James Player
 *      Copyright (c) 2003  James Player, Erik Nystrom, Wen-mei Hwu
 *              and The Board of Trustees of the University of Illinois.
 *              All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#include "pipa_common.h"
#include "pipa_print_graph.h"

#ifndef NULL
#define NULL ((void*) 0)
#endif

#define NUM_FUNCS 400
#define BUFF_SIZE 4096

char *str_array[NUM_FUNCS];

/*
 * Read in a list of function names to print out.
 */
void
IPA_init_func_names ()
{
  FILE *funcs_file;
  char buff[BUFF_SIZE];
  char *tmp;
  int bytes_read, i;

  memset (buff, 0, BUFF_SIZE);
  memset (str_array, 0, sizeof(char*) * NUM_FUNCS);

  if (!(funcs_file = fopen ("print_functions", "r"))) 
    {
      return;
    }
  bytes_read = fread (buff, 1, BUFF_SIZE, funcs_file);

  if (!feof(funcs_file))
    {
      fprintf (stderr, "IPA_init_func_names: Didn't read entire file. "
	       "(need bigger buffer)\n");
      exit (1);
    }

  tmp = strtok (buff, "\n");

  for (i=0; i<NUM_FUNCS && tmp != NULL; i++)
    {
      str_array[i] = strdup(tmp);      
      printf("WILL PRINT [%s]\n",str_array[i]);
      tmp = strtok (NULL, "\n");
    }

  str_array[i] = NULL;
}

/*
 * Compare the substring of "name" before the first '.' to all the function
 *  names read during the initialization.  If the word "all" appears in the
 *  list of function names, always return true.
 */
int
IPA_compare_func_name (char *name)
{
  int i = 0;
  char *fn_name, temp_name[256];
  
  if (str_array[0] == NULL)
    return 1;
  assert(strlen(name) < 256);

  strcpy (temp_name, name);
  fn_name = strtok (temp_name, ".");

  while (str_array[i] != NULL && i < NUM_FUNCS)
    {
      /*printf("CMP [%s] [%s]\n",fn_name, str_array[i]);*/
      if (!strcmp(fn_name, str_array[i]) ||
	  !strcmp("all", str_array[i])) return 1;
      i++;
    }

  return 0;
}
