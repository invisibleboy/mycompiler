/* Routine to interpret the entries in imult_table
 *
 * Success or failure is returned as the result of the function:
 *  0-success, 1-failure; can't handle that value, 2-string too short
 * If success, it modifies a user-supplied string to encode the instructions
 *   Sn a b.   shift-and-add ra,n,rb
 *   <n a.     shift-left ra,n
 *   + a b.    add ra,rb
 *   - a b.    sub ra,rb
 * Each result is implicitly numbered from 2; result 1 refers to the input.
 * The result of the last operation will contain the result
 */

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


/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <stdlib.h>

typedef struct Imul_entry
{
  char depth;
  char i_count;
  char opcode;
  char shift_amt;
  short int shiftee;
  short int addend;
}
Imul_entry_t;

#include "phase1_imult_tab.h"

/* globals shared by the visitations (to avoid recomputing partial results ) */

static int temp_number[100];
static int num_temps;
static int cursor;
static int max_cursor;		/* global copy of parameter */

/* return codes */
#define OUT_OF_RANGE 1
#define STRING_TOO_SHORT 2

int visit_entries (int N, char *result);
char lookup (int N);
void add_to_temps (int N);

#define emit( c ) if(cursor<max_cursor) result[cursor++]=c ; else  return STRING_TOO_SHORT;

/* imul_sequence: external interface to this package
 *
 * imul_sequence initializes the result string and resets partial_top before
 * letting visit_entries do the work
 */

int
imul_sequence (int N, char *result, int string_len, int *depth, int *i_count)
{
  int ret_value;

  int abs_n = abs (N);
  int count;

  if (abs_n < 2 || abs_n > IMUL_MAX)
    return OUT_OF_RANGE;

  count = Imul_table[abs_n].i_count;

  cursor = 0;
  max_cursor = string_len;
  temp_number[0] = 1;
  num_temps = 1;
  /* recursively generate instructions (post-order visit) */

  ret_value = visit_entries (abs_n, result);
  if (N < 0)
    {
      emit ('N');
      emit (' ');
      emit ('1' + count);
      emit (' ');
      *depth = Imul_table[abs_n].depth + 1;
      *i_count = count + 1;
    }
  else
    {
      *depth = Imul_table[abs_n].depth;
      *i_count = count;
    }
  emit ((char) 0);
  return ret_value;

}

/* interpret one entry in Imul_table
 *   update result and cursor as we write the result
 *   update the lookup table with this entry
 * return 1 if successful,
 *  0 otherwise (out of string space)
 */
int
visit_entries (int N, char *result)
{
  int shiftee = Imul_table[N].shiftee;
  int addend = Imul_table[N].addend;
  /* visit any partial results not yet visited */

  /*  printf( "visit_entries: entered %d, (cursor %d)\n", N, cursor ); */
  if ((shiftee > 1) && !lookup (shiftee))
    {
      visit_entries (shiftee, result);
    }

  if ((addend > 1) && !lookup (addend))
    {
      visit_entries (addend, result);
    }

  emit (Imul_table[N].opcode);
  switch (Imul_table[N].opcode)
    {
    case 'S':
      emit (Imul_table[N].shift_amt + '0');
      emit (' ');
      emit (lookup (shiftee));
      emit (' ');
      emit (lookup (addend));
      break;
    case '<':
      emit (Imul_table[N].shift_amt + '0');
      emit (' ');
      emit (lookup (shiftee));
      break;
    case '+':
    case '-':
      emit (' ');
      emit (lookup (shiftee));
      emit (' ');
      emit (lookup (addend));
      break;
    }
  emit (' ');

  add_to_temps (N);
  /*  printf( "visit_entries: exited %d, (cursor %d)\n", N, cursor ); */
  return 0;
}

/* maintain a simple mapping from register number in Imult_table to the
 * result number in this output string
 * return the position as a character (1-based).
 *  Positions > 9 just keep using the ASCII ordering.  Trouble around 80
 */

char
lookup (int N)
{
  int i;

  for (i = 0; i < num_temps; i++)
    if (temp_number[i] == N)
      {
	return i + '1';
      }
  return (char) 0;
}

void
add_to_temps (int N)
{
  temp_number[num_temps++] = N;
}
