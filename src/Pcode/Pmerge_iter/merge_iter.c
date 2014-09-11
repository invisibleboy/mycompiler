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
 *      File:   merge_iter.c (merge Pcode loop iteratation profiles)
 *      Author: John C. Gyllenhaal, Wen-mei Hwu
 *      Creation Date:  April. 1999
\*****************************************************************************/

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <library/i_error.h>
#include <library/dynamic_symbol.h>
#include <library/heap.h>

int expected_loop_count = 0;
INT_Symbol_Table **loop = NULL;

void
print_usage ()
{
  fprintf (stderr, "Usage: Pmerge_iter profile_iter.*\n");
  fprintf (stderr, "       Generates profile.iter\n");
}


/* Read in entry loop iteration profile file -JCG 4/99 */
void
read_loop_iter_profile (char *input_name, int input_id, int num_inputs)
{
  FILE *in;
  INT_Symbol *iter_symbol;
  int loop_count, input_count;
  int loop_id, entry_id, file_loop_id, entry_count, iter, ch, i;
  double *weight_array;

  if (!(in = fopen (input_name, "r")))
    I_punt ("read_loop_iter_profile: could not open '%s' for reading!",
	    input_name);

  /* Get loop and input count */
  if (fscanf (in, "%d %d\n\n", &loop_count, &input_count) != 2)
    I_punt ("read_loop_iter_profile: error reading loop and input count "
	    "in '%s'!", input_name);

  /* Initialize things if processing first input */
  if (!loop)
    {
      /* Malloc the array to hold all the loop iteration info */
      if (!(loop = (INT_Symbol_Table **) malloc (loop_count *
						 sizeof (INT_Symbol_Table *))))
	I_punt ("read_loop_iter_profile: Out of memory");

      /* Initialize all loop pointers to NULL */
      for (loop_id = 0; loop_id < loop_count; loop_id++)
	loop[loop_id] = NULL;

      /* Record expected loop count */
      expected_loop_count = loop_count;
    }

  /* Sanity check, make sure have expected loop count */
  if (expected_loop_count != loop_count)
    I_punt ("Error: '%s' has %i loops, expect %i!",
	    input_name, loop_count, expected_loop_count);

  /* Sanity check, input count better be 1 */
  if (input_count != 1)
    I_punt ("read_loop_iter_profile: invalid input count (%i)!",
	    input_count);

  /* Read in each loop's info */
  for (loop_id = 0; loop_id < loop_count; loop_id++)
    {
      if (fscanf (in, "%d %d\n", &file_loop_id, &entry_count) != 2)
	I_punt ("read_loop_iter_profile: error reading loop id and "
		"entry count in '%s'!", input_name);

      /* Sanity check, loop id's better match! */
      if (loop_id != file_loop_id)
	I_punt ("read_loop_iter_profile: loop id mismatch in '%s'",
		input_name);

      /* Sanity check, better be non-negative number */
      if (entry_count < 0)
	I_punt ("read_loop_iter_profile: invalid negative entry count");

      /* Create symbol table for loop (if necessary), unless no entries */
      if (!loop[loop_id] && (entry_count != 0))
	loop[loop_id] = INT_new_symbol_table ("loop", 0);

      /* Read in each loop entry */
      for (entry_id = 0; entry_id < entry_count; entry_id++)
	{
	  /* Read in the iter count */
	  if (fscanf (in, "%d", &iter) != 1)
	    I_punt ("read_loop_iter_profile: Error reading iter count!");

	  /* Get iter entry, add if necessary */
	  if (!(iter_symbol = INT_find_symbol (loop[loop_id], iter)))
	    {
	      /* Malloc weight array */
	      if (!(weight_array =
		    (double *) malloc (num_inputs * sizeof (double))))
		I_punt ("read_loop_iter_profile: Out of memory");

	      /* Initialize weight array */
	      for (i = 0; i < num_inputs; i++)
		weight_array[i] = 0.0;

	      INT_add_symbol (loop[loop_id], iter, (void *) weight_array);
	    }
	  /* Otherwise, get weight array from symbol */
	  else
	    {
	      weight_array = (double *) iter_symbol->data;
	    }

	  /* Read in input weight */
	  if (fscanf (in, "%lf", &weight_array[input_id]) != 1)
	    I_punt ("read_loop_iter_profile: Error reading weight!");
	}
    }

  /* Sanity check, better only have whitespace left in file */
  while ((ch = getc (in)) != EOF)
    if (!isspace (ch))
      I_punt ("read_loop_iter_profile: EOF expected!");

  fclose (in);
}

/* Write out combined loop iteration profile to output_name */
void
write_loop_iter_profile (char *output_name, int num_inputs)
{
  FILE *out;
  INT_Symbol *iter_symbol;
  int loop_id, input_id, iter;
  double *weight_array;
  Heap *iter_heap;

  if (!(out = fopen (output_name, "w")))
    I_punt ("Error, cannot open profile output file: %s\n", output_name);

  /* Print out the number of loops and number of inputs profiled 
   */
  fprintf (out, "%i %i\n", expected_loop_count, num_inputs);

  /* Go through each loop and print out iterations for each */
  for (loop_id = 0; loop_id < expected_loop_count; loop_id++)
    {
      if (loop[loop_id])
	{
	  /* Print out if exists */
	  /* Print out loop id and number of entries for this loop */
	  fprintf (out, "\n%i %i\n", loop_id, loop[loop_id]->symbol_count);

	  /* Create heap in order to sort iterations before printing them
	   * out.
	   */
	  iter_heap = Heap_Create (HEAP_MIN);

	  /* Put all the iteration data in a heap so we can sort the
	   * iterations before we print them out
	   */
	  /* Insert weight array (data) and sort by iterations (value) */
	  for (iter_symbol = loop[loop_id]->head_symbol; iter_symbol != NULL;
	       iter_symbol = iter_symbol->next_symbol)
	    Heap_Insert (iter_heap, iter_symbol->data, 
			 (double) iter_symbol->value);

	  /* Print out in iteration order */
	  while ((weight_array = Heap_Top (iter_heap)))
	    {
	      /* Get iteration count */
	      iter = (int) *(Heap_TopWeight (iter_heap));

	      /* Print out iteration count */
	      fprintf (out, "  %i", iter);

	      /* Print out each iteration's data */
	      for (input_id = 0; input_id < num_inputs; input_id++)
		fprintf (out, " %.0f", weight_array[input_id]);

	      /* Finish this line */
	      fprintf (out, "\n");

	      /* Pop data off the heap */
	      Heap_ExtractTop (iter_heap);
	    }

	  /* Free heap */
	  iter_heap = Heap_Dispose (iter_heap, NULL);
	}
      else
	{
	  /* Otherwise, print out loop id and no entries for this loop */
	  fprintf (out, "\n%i %i\n", loop_id, 0);
	}
    }

  /* Close the iter profile file */
  fclose (out);

}

/* Get and process the command-line arguments */
int
main (int argc, char **argv)
{
  char *output_file_name;
  char **input_name;
  int num_inputs, input_id;

  /* Require at least one input file */
  if (argc < 2)
    {
      print_usage ();
      exit (1);
    }

  /* Use fixed output name for now */
  output_file_name = "profile.iter";

  /* Calculate number of inputs */
  num_inputs = argc - 1;

  /* Point to start of input names */
  input_name = &argv[1];

  /* Read in all input files */
  for (input_id = 0; input_id < num_inputs; input_id++)
    read_loop_iter_profile (input_name[input_id], input_id, num_inputs);

  /* Write out the combined input data */
  write_loop_iter_profile (output_file_name, num_inputs);

  return (0);
}
