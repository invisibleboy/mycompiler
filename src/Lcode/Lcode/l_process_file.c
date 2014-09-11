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
 *      File :          l_process_file.c
 *      Description :   Perform reads and writes of entire files at one time.
 *                      Allows the same file name to be used for both reads and
 *                      writes.
 *      Original : Brian Deitrich, Wen-mei Hwu 1997
\*****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>

extern int L_binary_magic_number_emitted;

static L_File_Data *
L_process_input (L_File_Data * file_data, int read_data)
/* returns pointer to L_File_Data structure that is created in this routine. */
{
  switch (L_token_type)
    {
    case L_INPUT_EOF:
      break;
    case L_INPUT_MS:
    case L_INPUT_VOID:
    case L_INPUT_BYTE:
    case L_INPUT_WORD:
    case L_INPUT_LONG:
    case L_INPUT_LONGLONG:
    case L_INPUT_FLOAT:
    case L_INPUT_DOUBLE:
    case L_INPUT_ALIGN:
    case L_INPUT_ASCII:
    case L_INPUT_ASCIZ:
    case L_INPUT_ELEMENT_SIZE:
    case L_INPUT_RESERVE:
    case L_INPUT_SKIP:
    case L_INPUT_GLOBAL:
    case L_INPUT_WB:
    case L_INPUT_WW:
    case L_INPUT_WI:
    case L_INPUT_WQ:
    case L_INPUT_WF:
    case L_INPUT_WF2:
    case L_INPUT_WS:
      if (read_data)
        {
          file_data->next =
            (L_File_Data *) Lcode_malloc (sizeof (L_File_Data));

          file_data = file_data->next;
          file_data->type = L_token_type;
          file_data->ptr = L_data;
          file_data->next = NULL;
        }
      else
        L_delete_data (L_data);
      L_data = NULL;
      break;
    case L_INPUT_FUNCTION:
      file_data->next = (L_File_Data *) Lcode_malloc (sizeof (L_File_Data));

      file_data = file_data->next;
      file_data->type = L_token_type;
      file_data->ptr = L_fn;
      file_data->next = NULL;
      break;
    default:
      L_punt ("ERROR: L_process_input: illegal token.\n");
    }

  return (file_data);
}

static L_File_Data *
L_process_output (FILE * file_out, L_File_Data * file_data)
/* returns pointer to L_File_Data structure that is present after the data
 * structure being dealt with by this routine.  That data structure is free'd
 * in this routine. */
{
  L_File_Data *next_file_data;

  switch (file_data->type)
    {
    case L_INPUT_EOF:
      break;
    case L_INPUT_MS:
    case L_INPUT_VOID:
    case L_INPUT_BYTE:
    case L_INPUT_WORD:
    case L_INPUT_LONG:
    case L_INPUT_LONGLONG:
    case L_INPUT_FLOAT:
    case L_INPUT_DOUBLE:
    case L_INPUT_ALIGN:
    case L_INPUT_ASCII:
    case L_INPUT_ASCIZ:
    case L_INPUT_ELEMENT_SIZE:
    case L_INPUT_RESERVE:
    case L_INPUT_SKIP:
    case L_INPUT_GLOBAL:
    case L_INPUT_WB:
    case L_INPUT_WW:
    case L_INPUT_WI:
    case L_INPUT_WQ:
    case L_INPUT_WF:
    case L_INPUT_WF2:
    case L_INPUT_WS:
      L_data = (L_Data *) file_data->ptr;
      L_print_data (file_out, (L_Data *) file_data->ptr);
      L_delete_data ((L_Data *) file_data->ptr);
      break;
    case L_INPUT_FUNCTION:
      L_fn = (L_Func *) file_data->ptr;
      L_print_func (file_out, (L_Func *) file_data->ptr);
      L_delete_func ((L_Func *) file_data->ptr);
      break;
    default:
      L_punt ("ERROR: L_process_output: illegal token.\n");
    }

  next_file_data = file_data->next;
  Lcode_free (file_data);
  return (next_file_data);
}

L_File_Data *
L_read_entire_file (char *filename)
{
  L_File_Data start, *file_data;

  L_open_input_file (filename);
  file_data = &start;
  start.next = NULL;

  while (L_get_input () != L_INPUT_EOF)
    file_data = L_process_input (file_data, 1);
  L_close_input_file (filename);

  return (start.next);
}

L_File_Data *
L_read_entire_file_for_func (char *filename)
/* Similar to L_read_entire_file, except only functions are saved in the
 * L_File_Data linked list. */
{
  L_File_Data start, *file_data;

  L_open_input_file (filename);
  file_data = &start;
  start.next = NULL;

  while (L_get_input () != L_INPUT_EOF)
    file_data = L_process_input (file_data, 0);
  L_close_input_file (filename);

  return (start.next);
}

void
L_write_entire_file (char *filename, L_File_Data * file_data)
{
  FILE *out_file;

  out_file = L_open_output_file (filename);
  L_binary_magic_number_emitted = 0;

  while (file_data)
    file_data = L_process_output (out_file, file_data);
  L_close_output_file (out_file);
}

void
L_delete_file_data (L_File_Data * file_data)
{
  L_File_Data *data, *next_data;

  for (data = file_data; data; data = next_data)
    {
      next_data = data->next;
      switch (data->type)
        {
        case L_INPUT_EOF:
          break;
        case L_INPUT_MS:
        case L_INPUT_VOID:
        case L_INPUT_BYTE:
        case L_INPUT_WORD:
        case L_INPUT_LONG:
        case L_INPUT_LONGLONG:
        case L_INPUT_FLOAT:
        case L_INPUT_DOUBLE:
        case L_INPUT_ALIGN:
        case L_INPUT_ASCII:
        case L_INPUT_ASCIZ:
        case L_INPUT_ELEMENT_SIZE:
        case L_INPUT_RESERVE:
        case L_INPUT_SKIP:
        case L_INPUT_GLOBAL:
        case L_INPUT_WB:
        case L_INPUT_WW:
        case L_INPUT_WI:
        case L_INPUT_WQ:
        case L_INPUT_WF:
        case L_INPUT_WF2:
        case L_INPUT_WS:
          L_data = (L_Data *) data->ptr;
          L_delete_data ((L_Data *) data->ptr);
          break;
        case L_INPUT_FUNCTION:
          L_fn = (L_Func *) data->ptr;
          L_delete_func ((L_Func *) data->ptr);
          break;
        default:
          L_punt ("ERROR: L_delete_file_data: illegal token.\n");
        }
      Lcode_free (data);
    }
}
