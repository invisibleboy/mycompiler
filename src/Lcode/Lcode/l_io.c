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
 *      File :          l_io.c
 *      Description :   LCODE input/output routines 
 *      Original: Po-hua Chang, Wen-mei Hwu, June 1990
 *      Modified : Righard E. Hank, February 1995 
 *              Redesigned Lcode parser to eliminate the need to
 *              use lex.  Lcode input/output routines extracted
 *              from l_code.c 
 *
 *      NOTE: EACH FUNCTION IN THIS FILE HAS A BINARY ANALOG IN THE FILE
 *              l_binaryio.c BOTH FUNCTIONS MUST BE KEPT IN SYNC.
 *
 *            CARE MUST BE TAKEN WHEN ALTERING THE FORMAT OF THE BINARY
 *            FILE SO THAT EXISTING BINARY LCODE DOES NOT BECOME INVALID.
 *              (IF THIS IS UNAVOIDABLE, A TRANSLATOR MUST BE PROVIDED!)
 *
\*****************************************************************************/
/* 09/23/02 REK Updating this function to write the completers field from
 *              the L_Oper to the popc attribute field along with the
 *              proc_opc.
 */

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>
#include <Lcode/l_event.h>
#include <Lcode/l_region.h>
#include <Lcode/l_io.h>
#include <Lcode/l_appendix.h>
#include <Lcode/l_debug.h>
#include <Lcode/l_time.h>

#include <glob.h>

#include "l_binaryio.h"

L_Input_Buf L_input_buf;

/* SAM 6-96: global array to hold value of a peeked token, use global to avoid
   having to malloc and free lots of temp strings */
char L_peek_token[2048];

int L_func_contains_dep_pragmas;
int L_func_contains_jsr_dep_pragmas;

/* JWS 20040820 - Access specifiers for Pipa pointer analysis.
 * ----------------------------------------------------------------------
 * L_func_acc_specs indicates function uses acc specs (ACC_SPECS pragma).
 * L_func_acc_omega indicates function has, in addition, "accessory "sync
 * arcs, derived from Omega test, to represent properties of specific
 * dependences.
 */

int L_func_acc_specs = 0;
int L_func_acc_omega = 0;

/* LCW - variable to indicate if a field is a bit-field - 4/20/96 */
static int is_bit_field;

int L_binary_magic_number_emitted = 0;

static L_Cb *L_read_cb (L_Func * fn, L_Region * region,
                        L_Input_Buf * input_buf);

static void L_print_acc_info_list (FILE *F, L_AccSpec *mas_list);

/* Print out buffer and put an arrow to the character specified */
void
L_print_buf_with_arrow (FILE * out, L_Input_Buf * input_buf)
{
  int i, ch;
  char *buf = input_buf->line_buf;
  int pos = input_buf->token_start - input_buf->line_buf;
  int col, arrow_col;

  /* Print the line number */
  fprintf (out, "Line #%d\n", input_buf->line_count);

  /* Start in column 0 */
  col = 0;
  arrow_col = 0;

  /* Print out text, upto newline after pos.
   * (Since expanded text can have many newlines in it)
   */
  for (i = 0; buf[i] != 0; i++)
    {
      ch = buf[i];

      /* Get column arrow should go at */
      if (i == pos)
        arrow_col = col;

      col++;

      /* Expand tabs */
      if (ch == '\t')
        {
          /* Put out first character */
          putc (' ', out);

          /* Add spaces until at multiple of 8 */
          while ((col % 8) != 0)
            {
              col++;
              putc (' ', out);
            }

        }
      else
        putc (ch, out);

      /* Handle newlines */
      if (ch == '\n')
        {
          /* Return to column 0 at newline */
          col = 0;

          /* Stop at newline after pos */
          if (i >= pos)
            break;
        }
    }

  /* Get column arrow should go to if at end of buffer */
  if (i < pos)
    arrow_col = col;

  /* Add trailing newline if none in buffer */
  if (buf[i] != '\n')
    putc ('\n', out);

  /* Put arrow at appropriate column detected in above loop */
  for (i = 0; i < arrow_col; i++)
    putc (' ', out);
  fprintf (out, "^\n");
}

int
L_is_token_char (int ch)
{
  switch (ch)
    {
    case '(':
    case ')':
    case '[':
    case ']':
    case '<':
    case '>':
    case '{':
    case '}':
    case '#':
    case ',':
      return (1);
    default:
      return (0);
    }
}

void
_L_addc_to_peek_buffer (L_Input_Buf * input_buf, char ch)
{
  char *old_line_buf;
  char *old_peek_buf;

  /* If at end of current buffer, malloc new one and copy over
   * contents.
   */
  /* Need to make sure line and peek buffers are the same size, */
  /* so the peek buffer is also increased with the line buffer  */

  if ((input_buf->peek_len + 1) >= input_buf->max_line_len)
    {
      old_line_buf = input_buf->line_buf;
      old_peek_buf = input_buf->peek_buf;

      /* Double size of buffer */
      input_buf->max_line_len = input_buf->max_line_len * 2;

      /* Malloc new line buffer */
      if ((input_buf->line_buf = (char *) malloc (input_buf->max_line_len)) ==
          NULL)
        L_punt ("Out of memory in addc_to_peek_buffer");


      /* Malloc new peek buffer as well */
      if ((input_buf->peek_buf = (char *) malloc (input_buf->max_line_len)) ==
          NULL)
        L_punt ("Out of memory in addc_to_peek_buffer");

      /* Copy over old buffers */
      strcpy (input_buf->line_buf, old_line_buf);
      strcpy (input_buf->peek_buf, old_peek_buf);

      /* Free old buffer */
      free (old_line_buf);
      free (old_peek_buf);
    }
  /* Add character to buf (and terminate) */
  input_buf->peek_buf[input_buf->peek_len] = ch;
  input_buf->peek_buf[input_buf->peek_len + 1] = 0;
  input_buf->peek_len++;
}

void
L_addc_to_line_buffer (L_Input_Buf * input_buf, char ch)
{
  char *old_line_buf;
  char *old_peek_buf;

  /* If at end of current buffer, malloc new one and copy over
   * contents.
   */
  /* Need to make sure line and peek buffers are the same size, */
  /* so the peek buffer is also increased with the line buffer  */

  if ((input_buf->line_len + 1) >= input_buf->max_line_len)
    {
      old_line_buf = input_buf->line_buf;
      old_peek_buf = input_buf->peek_buf;

      /* Double size of buffer */
      input_buf->max_line_len = input_buf->max_line_len * 2;

      /* Malloc new line buffer */
      if ((input_buf->line_buf = (char *) malloc (input_buf->max_line_len)) ==
          NULL)
        L_punt ("Out of memory in addc_to_line_buffer");


      /* Malloc new peek buffer as well */
      if ((input_buf->peek_buf = (char *) malloc (input_buf->max_line_len)) ==
          NULL)
        L_punt ("Out of memory in addc_to_line_buffer");

      /* Copy over old buffers */
      strcpy (input_buf->line_buf, old_line_buf);
      strcpy (input_buf->peek_buf, old_peek_buf);

      /* Free old buffer */
      free (old_line_buf);
      free (old_peek_buf);
    }
  /* Add character to buf (and terminate) */
  input_buf->line_buf[input_buf->line_len] = ch;
  input_buf->line_buf[input_buf->line_len + 1] = 0;
  input_buf->line_len++;
}

void
_L_addc_to_token_buffer (L_Input_Buf * input_buf, char ch)
{
  char *old_token_buf;

  /* If at end of current buffer, malloc new one and copy over
   * contents.
   */

  if ((input_buf->token_len + 1) >= input_buf->max_token_len)
    {
      old_token_buf = input_buf->token_buf;

      /* Double size of buffer */
      input_buf->max_token_len = input_buf->max_token_len * 2;

      /* Malloc new line buffer */
      if ((input_buf->token_buf = (char *) malloc (input_buf->max_token_len))
          == NULL)
        L_punt ("Out of memory in addc_to_token_buffer");

      /* Copy over old buffer */
      strcpy (input_buf->token_buf, old_token_buf);

      /* Free old buffer */
      free (old_token_buf);
    }
  /* Add character to buf (and terminate) */
  input_buf->token_buf[input_buf->token_len] = ch;
  input_buf->token_buf[input_buf->token_len + 1] = 0;
  input_buf->token_len++;
}

/* REH 4/14/95 */
/* "Partial inlining" of the L_addc_to_token_buffer() function. */
/* The original function has been renamed by adding a preceding */
/* _.  Now, the function is called only if the buffer is to be  */
/* resized.                                                     */
#define L_addc_to_token_buffer(a,b)                                           \
                         {  if ( (a)->token_len + 1 < (a)->max_token_len )  { \
                              (a)->token_buf[(a)->token_len] = (b);           \
                              (a)->token_buf[(a)->token_len + 1] = 0;         \
                              (a)->token_len++;                               \
                            }                                                 \
                            else                                              \
                              _L_addc_to_token_buffer((a),(b));               \
                         }

#define L_addc_to_peek_buffer(a,b)                                            \
                         {  if ( (a)->peek_len + 1 < (a)->max_line_len )  {   \
                              (a)->peek_buf[(a)->peek_len] = (b);             \
                              (a)->peek_buf[(a)->peek_len + 1] = 0;           \
                              (a)->peek_len++;                                \
                            }                                                 \
                            else                                              \
                              _L_addc_to_peek_buffer((a),(b));                \
                         }

void
L_init_input_buf (L_Input_Buf * input_buf)
{
  char ch;

  /* Initialize Lcode line/peek buffers */
  input_buf->max_line_len = 1024;
  if ((input_buf->line_buf = (char *) malloc (input_buf->max_line_len)) ==
      NULL)
    L_punt ("Out of memory in L_init_input_buf");

  if ((input_buf->peek_buf = (char *) malloc (input_buf->max_line_len)) ==
      NULL)
    L_punt ("Out of memory in L_init_input_buf");

  /* Initialize token buffer */
  input_buf->max_token_len = 1024;
  if ((input_buf->token_buf = (char *) malloc (input_buf->max_token_len)) ==
      NULL)
    L_punt ("Out of memory in L_init_input_buf");

  input_buf->line_buf[0] = 0;
  input_buf->peek_buf[0] = 0;
  input_buf->token_buf[0] = 0;
  input_buf->line_len = 0;
  input_buf->peek_len = 0;
  input_buf->token_len = 0;

  input_buf->token_start = NULL;
  input_buf->token_end = NULL;
  input_buf->eof = 0;
  input_buf->line_count += 1;

  if (!L_input_binary_format)
    {
      while ((ch = getc (L_IN)) != EOF)
        {
          L_addc_to_line_buffer (input_buf, ch);

          /* If hit newline, end line */
          if (ch == '\n')
            break;
        }
      while ((ch = getc (L_IN)) != EOF)
        {
          L_addc_to_peek_buffer (input_buf, ch);
          /* If hit newline, end line */
          if (ch == '\n')
            break;
        }
      if (ch == EOF)
        input_buf->eof = 1;
    }

  return;
}

void
L_free_input_buf (L_Input_Buf * input_buf)
{
  if (input_buf->line_buf != NULL)
    {
      free (input_buf->line_buf);
      input_buf->line_buf = NULL;
    }
  if (input_buf->peek_buf != NULL)
    {
      free (input_buf->peek_buf);
      input_buf->peek_buf = NULL;
    }
  if (input_buf->token_buf != NULL)
    {
      free (input_buf->token_buf);
      input_buf->token_buf = NULL;
    }
}

int
L_refill_input_buf (L_Input_Buf * input_buf)
{
  char ch;

  /* Copy the peek buffer to the line buffer, this is legal */
  /* since we know the buffers are the same size            */
  input_buf->line_len = input_buf->peek_len;
  strcpy (input_buf->line_buf, input_buf->peek_buf);

  input_buf->peek_len = 0;
  input_buf->peek_buf[0] = '\0';
  /* Place the next line in the peek buffer */
  while ((ch = getc (L_IN)) != EOF)
    {
      L_addc_to_peek_buffer (input_buf, ch);

      /* If hit newline, end line */
      if (ch == '\n')
        break;
    }

  if (ch == EOF)
    {
      if (input_buf->eof == 1)
        {
          return (0);
        }
      else
        {
          input_buf->eof = 1;
        }
    }
  /* Reset the token pointers */
  input_buf->token_start = NULL;
  input_buf->token_end = NULL;

  /* Increment the line counter */
  input_buf->line_count += 1;

  return (1);
}

char
L_peek_next_char (L_Input_Buf * input_buf)
{
  char *ptr = input_buf->token_end + 1;

  while (isspace (*ptr))
    {
      if (*ptr == '\n')
        {
          if (ptr == input_buf->peek_buf)
            {
              L_refill_input_buf (input_buf);
              ptr = input_buf->line_buf;
            }
          else
            ptr = input_buf->peek_buf;
        }
      else
        ptr++;
    }
  return (*ptr);
}

char *
L_get_next_lcode_token (L_Input_Buf * input_buf)
{
  char *ch_ptr = input_buf->token_end;

  int quoted_string, quote_ch = 0;

  if (ch_ptr == NULL)
    ch_ptr = input_buf->line_buf;
  else
    {
      ch_ptr++;
    }

  /* Skip any whitespace following the current token */
  while (isspace (*ch_ptr))
    {
      if (*ch_ptr == '\n')
        {
          if (L_refill_input_buf (input_buf) == 0)
            {
              return (NULL);
            }
          ch_ptr = input_buf->line_buf;
        }
      else
        {
          ch_ptr++;
        }
    }

  /* Next token starts here */
  input_buf->token_len = 0;
  input_buf->token_start = ch_ptr;

  if (L_is_token_char (*ch_ptr))
    {
      /* copy token character into token buffer */
      L_addc_to_token_buffer (input_buf, *ch_ptr);

      input_buf->token_end = ch_ptr;
      return (input_buf->token_buf);
    }

  /* Assume we don't have a quoted string */
  quoted_string = 0;

  /* Handle start of quoted string */
  if ((*ch_ptr == '\"') || (*ch_ptr == '\''))
    {
      /* Get the end quote character we are looking for */
      quote_ch = *ch_ptr;

      /* copy quote into token buffer */
      L_addc_to_token_buffer (input_buf, *ch_ptr);

      ch_ptr++;

      /* Mark as quoted string */
      quoted_string = 1;
    }

  /* Otherwise, get token until hit white space, end quote or token char */
  while (*ch_ptr != '\0')
    {

      /* For quoted strings, don't end token until end quote */
      if (quoted_string)
        {
          /* If hit end quote, finish string */
          if (*ch_ptr == quote_ch)
            {
              /* copy token character into token buffer */
              L_addc_to_token_buffer (input_buf, *ch_ptr);
              break;
            }
          /* May encounter an end of line before the end */
          /* of the quote so we must reload the buffer   */
          /* and continue */
          if (*ch_ptr == '\n')
            {
              if (L_refill_input_buf (input_buf) == 0)
                return (NULL);

              /* copy token character into token buffer */
              L_addc_to_token_buffer (input_buf, *ch_ptr);

              /* After the buffer has been reloaded, we need to move */
              /* the token start pointer to the beginning of the new */
              /* line and reset the character pointer <ch> to the    */
              /* beginning of the line as well.                      */
              input_buf->token_start = input_buf->line_buf;
              ch_ptr = input_buf->line_buf;

              /* Go back to the top of the loop, don't need to inc ch */
              continue;
            }
        }
      /* For unquoted strings, stop at white space or token char */
      else
        {
          if (isspace (*ch_ptr) || L_is_token_char (*ch_ptr))
            {
              ch_ptr--;         /* ch now points to last character in token */
              break;
            }
        }

      /* copy token character into token buffer */
      L_addc_to_token_buffer (input_buf, *ch_ptr);

      /* Handle backslash character */
      if (*ch_ptr == '\\')
        {
          ch_ptr++;

          L_addc_to_token_buffer (input_buf, *ch_ptr);

          /* May encounter an end of line before the end */
          /* of the quote so we must reload the buffer   */
          /* and continue */
          if (*ch_ptr == '\n')
            {
              if (L_refill_input_buf (input_buf) == 0)
                return (NULL);

              /* After the buffer has been reloaded, we need to move */
              /* the token start pointer to the beginning of the new */
              /* line and reset the character pointer <ch> to the    */
              /* beginning of the line as well.                      */
              input_buf->token_start = input_buf->line_buf;
              ch_ptr = input_buf->line_buf;

              /* Go back to the top of the loop, don't need to inc ch */
              continue;
            }
        }
      ch_ptr++;
    }

  /* token ends at current character */
  input_buf->token_end = ch_ptr;

  return (input_buf->token_buf);
}

void
L_get_next_lcode_token_expecting (L_Input_Buf * input_buf, char *expect,
                                  char *msg)
{
  char *token;

  token = L_get_next_lcode_token (input_buf);

  if (token == NULL)
    {
      L_punt ("%s : Premature EOF has occurred on line #%d\n",
              msg, input_buf->line_count);
    }

  if (strcmp (token, expect))
    {
      L_print_buf_with_arrow (stderr, input_buf);
      L_punt ("%s : Expecting token \'%s\' but found \'%s\'.",
              msg, expect, token);
    }
  return;
}

/* SAM 6-96: Peek at the next token in the input stream.  Note that the
   punctuation tokens are not considered 'real' by this function.  This
   function may not be as general as it could be, but it should suffice
   for its intended use. */
char *
L_peek_next_real_lcode_token (L_Input_Buf * input_buf)
{
  int len;
  char *ptr, *token_ptr;

  ptr = input_buf->token_end + 1;

  while (isspace (*ptr) || L_is_token_char (*ptr))
    {
      //if (*ptr == '\n') 
      if (*ptr == '\n' || *ptr == '#') //i think this is checking to see if it's OK
      {                              //to go to the next line
        
          if (ptr == input_buf->peek_buf)
            {
              if (L_refill_input_buf (input_buf) == 0)
                return (NULL);
              ptr = input_buf->line_buf;
            }
          else if (input_buf->peek_buf[0] != '\0')
            {
              ptr = input_buf->peek_buf;
            }
          else
            {
              return (NULL);
            }
        }
      else
        {
          ptr++;
        }
    }

  /* Next "real" token starts here (ptr) */

  /* Don't handle string tokens for now */
  if ((*ptr == '\"') || (*ptr == '\''))
    return (NULL);

  /* Get token until hit white space or token char */
  len = 0;
  token_ptr = &L_peek_token[0];
  while (*ptr != '\0')
    {
      if (isspace (*ptr) || L_is_token_char (*ptr))
        break;
      token_ptr[len] = *ptr;
      len++;
      ptr++;
    }
  token_ptr[len] = '\0';

  return (token_ptr);
}

int
L_strtol (char *start, char **end, int *num)
{
  char *end_ptr;

  end_ptr = start;

  *num = (int) strtol (start, &end_ptr, 10);

  if (end != NULL)
    *end = end_ptr;

  if (*end_ptr != 0)
    /* The token does not contain a valid integer */
    return (0);
  else
    return (1);
}

int
L_strtoll (char *start, ITintmax * num)
{
  int read;

  read = sscanf (start, ITintmaxformat, num);

  return (read);
}

int
L_strtod (char *start, char **end, double *num)
{
  char *end_ptr;

  end_ptr = start;

  *num = strtod (start, &end_ptr);

  if (end != NULL)
    *end = end_ptr;

  if (*end_ptr != 0)
    /* The token does not contain a valid floating point value */
    return (0);
  else
    return (1);
}

int
L_tell (void)
{
  int true_offset = (int) ftell (L_IN);
  int token_offset;
  int lcode_offset;

  if (L_input_buf.token_start != NULL)
    token_offset = L_input_buf.token_start - L_input_buf.line_buf;
  else
    token_offset = 0;

  lcode_offset = true_offset - L_input_buf.peek_len -
    (L_input_buf.line_len - token_offset) + 1;

  return (lcode_offset);
}

int
L_seek (int offset)
{

  /* Move the file pointer to the provided offset */
  /* from the beginning of the file               */
  if (fseek (L_IN, offset, 0))
    L_punt ("Unable to seek to offset %d in current input file\n", offset);

  /* Now, the Lcode input buffer contains two lines */
  /* of the file, they must be flushed and refilled */
  /* with the two lines starting from offset.       */
  /* Two calls to refill the buffer should do it.   */

  L_refill_input_buf (&L_input_buf);
  L_refill_input_buf (&L_input_buf);

  return (0);
}


/*============================================================
 *
 *  Input/Output file routines 
 *
 *============================================================*/
void
L_open_input_file (char *file_name)
{
  int ch;
  FILE *in;

  if (strcmp (file_name, "stdin"))
    {
      if ((in = fopen (file_name, "r")) == NULL)
        {
          fprintf (stderr, ": open %s\n", file_name);
          L_punt ("L_open_input_file: cannot open input file");
        }
    }
  else
    in = stdin;

  /*
   * Change the default buffer size to 64K, to increase
   * i/o performance.
   */
  setvbuf (in, NULL, _IOFBF, 64 * 1024);

  L_IN = in;

  /*
   * REH 4/11/96 - Determine if Lcode file is ascii or binary
   */
  ch = getc (L_IN);
  ungetc (ch, L_IN);
  if (!L_file_is_binary (ch))
    {
      /*
         * Intialize input file buffer
       */
      L_input_binary_format = 0;
    }
  else
    {
      L_check_binary_file_version (ch);
      L_input_binary_format = 1;
    }
  L_init_input_buf (&L_input_buf);

  return;
}

void
L_close_input_file (char *file_name)
{
  if (strcmp (file_name, "stdin"))
    fclose (L_IN);
  L_free_input_buf (&L_input_buf);
}

FILE *
L_open_output_file (char *name)
{
  FILE *F = 0;

  if (!strcmp (name, "stdout"))
    {
      return stdout;
    }
  if (!strcmp (name, "stderr"))
    {
      return stderr;
    }

  F = fopen (name, "w");
  if (F == 0)
    L_punt ("cannot open output file");

  /*
   * Change the default buffer size to 64K, to increase
   * i/o performance.
   */
  setvbuf (F, NULL, _IOFBF, 64 * 1024);

  return (F);
}

void
L_close_output_file (FILE * F)
{
  if (F == NULL)
    L_punt ("L_close_output_file: trying to close NULL file");

  if ((F == stdin) || (F == stdout) || (F == stderr))
    return;
  fclose (F);
  F = NULL;
}



void
L_open_filelist_file (char *file_name)
{
  FILE *in;
  int input_ext_len;
  char buf[256];
  L_File *new_file;

  if (strcmp (file_name, "stdin"))
    {
      if ((in = fopen (file_name, "r")) == NULL)
        {
          fprintf (stderr, ": open %s\n", file_name);
          L_punt ("L_open_filelist_file: cannot open filelist file");
        }
    }
  else
    in = stdin;

  L_FILELIST = in;
  L_filelist = NULL;

  input_ext_len = strlen (L_input_file_extension);

  while (fgets (buf, sizeof (buf), L_FILELIST) != NULL)
    {

      buf[strlen (buf) - 1] = '\0';
      new_file = (L_File *) L_alloc (L_alloc_file);
      new_file->input_file = (char *) Lcode_malloc (strlen (buf) + 1);
      strcpy (new_file->input_file, buf);

      buf[strlen (buf) - input_ext_len] = '\0';
      strcat (buf, L_output_file_extension);
      new_file->output_file = (char *) Lcode_malloc (strlen (buf) + 1);
      strcpy (new_file->output_file, buf);
      L_filelist = List_insert_last (L_filelist, new_file);
    }

  return;
}


void
L_create_glob_filelist ()
{
  int input_ext_len;
  char buf[256];
  L_File *new_file;
  char pattern[256];
  glob_t glob_results;
  int glob_flags = 0;
  char **match_ptr;
  size_t num_matches;

  /*  compute pattern  */
  strcpy (pattern, L_file_directory);
  strcat (pattern, "*.");
  strcat (pattern, L_input_file_extension);

  /*  call the globber  */
  glob (pattern, glob_flags, 0, &glob_results);

  num_matches = glob_results.gl_pathc;

  if (num_matches == 0)
    L_punt ("L_create_filelist : no %s files in directory %s",
            L_input_file_extension, L_file_directory);

  L_filelist = NULL;

  input_ext_len = strlen (L_input_file_extension);

  for (match_ptr = glob_results.gl_pathv; num_matches;
       match_ptr++, num_matches--)
    {
      new_file = (L_File *) L_alloc (L_alloc_file);
      new_file->input_file = (char *) Lcode_malloc (strlen (*match_ptr) + 1);
      strcpy (new_file->input_file, *match_ptr);

      strcpy (buf, *match_ptr);
      buf[strlen (buf) - input_ext_len] = '\0';
      strcat (buf, L_output_file_extension);
      buf[strlen (buf) + 1] = '\0';
      new_file->output_file = (char *) Lcode_malloc (strlen (buf) + 1);
      strcpy (new_file->output_file, buf);
      L_filelist = List_insert_last (L_filelist, new_file);
    }

  /*  free memory  */
  globfree (&glob_results);
}


void
L_create_filelist ()
{
  if (L_file_processing_model == L_FILE_MODEL_EXTENSION)
    {
      L_create_glob_filelist ();
    }
  else if (L_file_processing_model == L_FILE_MODEL_LIST)
    {
      L_open_filelist_file (L_filelist_file);
    }
  else
    {
      L_punt ("L_create_file_list : invalid file processing");
    }
}


void
L_delete_filelist ()
{
  L_File *file_list;

  List_start (L_filelist);
  while ((file_list = List_next (L_filelist)))
    {
      free (file_list->input_file);
      free (file_list->output_file);
      L_free (L_alloc_file, file_list);
    }
  List_reset (L_filelist);

  if (L_file_processing_model == L_FILE_MODEL_LIST)
    {
      fclose (L_FILELIST);
    }
}


static L_AccSpec * L_read_mem_acc_spec (L_Input_Buf * input_buf);

static void
L_read_sync_arcs (L_Input_Buf *input_buf, L_Oper *oper)
{
  char ch;
  L_Sync *tail_sync, *head_sync;
  L_AccSpec *mas, *last = NULL;


  L_get_next_lcode_token_expecting (input_buf, "{", "L_read_sync_arcs");
  ch = L_peek_next_char (input_buf);

  while (ch != '}')
    {
      L_get_next_lcode_token_expecting (input_buf, "(", "L_read_sync_arcs");
      ch = L_peek_next_char (input_buf);

      if (ch == 'U' || ch == 'D')
	{
	  /* Memory access specifier */
	  if ((mas = L_read_mem_acc_spec (input_buf)))
	    {
	      if (last)
		last->next = mas;
	      else
		oper->acc_info = mas;

	      last = mas;
	    }
	}
      else
	{
	  /* Sync arc */
	  if ((tail_sync = L_read_sync (input_buf)))
	    {
	      L_insert_tail_sync_in_oper (oper, tail_sync);
	  
	      head_sync = L_copy_sync (tail_sync);
	      head_sync->dep_oper = oper;
	      L_insert_head_sync_in_oper (tail_sync->dep_oper, head_sync);
	    }
	}

      ch = L_peek_next_char (input_buf);
    }

  L_get_next_lcode_token (input_buf);       /* Throw away '}' */

  return;
}

/*============================================================
 *
 *  Appendix input/output routines
 *
 *============================================================*/
void
L_read_oper_appendix (L_Func * fn, L_Input_Buf * input_buf)
{
  int op_id;
  char ch, *token;
  L_Attr *attr, *oper_attr;
  L_Oper *oper;

  token = L_get_next_lcode_token (input_buf);

  if (L_strtol (token, NULL, &op_id) == 0)
    {
      L_print_buf_with_arrow (stderr, input_buf);
      L_punt ("L_read_oper_appendix: This is not a valid integer value.");
    }

  oper = L_oper_hash_tbl_find_oper (fn->oper_hash_tbl, op_id);

  if (oper == NULL)
    L_punt ("L_read_oper_appendix: Oper %d does not exist in function %s\n",
            op_id, fn->name);

  ch = L_peek_next_char (input_buf);
  /*  
   * Read oper attributes if there are any.
   */
  if (ch == '<')
    {
      L_get_next_lcode_token (input_buf);       /* Throw away '<' */
      ch = L_peek_next_char (input_buf);

      /* 
       * Read region attributes if there are any <()()()>
       */
      if (ch == '(')
        {
          oper_attr = NULL;
          while (ch != '>')
            {
              attr = L_read_attr (NULL, input_buf);
              oper_attr = L_concat_attr (oper_attr, attr);

              if ((strcmp (attr->name, "SPECID") == 0) &&
                  fn->max_spec_id <= attr->field[0]->value.i)
                fn->max_spec_id = (attr->field[0]->value.i + 1);

              ch = L_peek_next_char (input_buf);
            }
          L_get_next_lcode_token (input_buf);   /* Throw away '>' */

          /*
           *  If the popc attribute was place in the appendix
           *  we must be sure to set oper->proc_opc when the
           *  appendix is input
           */
	  /* 09/23/02 REK Updating to read the completers field along
	   *              with the proc_opc field. */
          attr = L_find_attr (oper_attr, "popc");
          if (attr != NULL)
            {
              oper->proc_opc = (int) attr->field[0]->value.i;
	      oper->completers = (int) attr->field[1]->value.i;
            }

          oper->attr = L_concat_attr (oper->attr, oper_attr);

        }
      else
        {
          L_print_buf_with_arrow (stderr, input_buf);
          L_punt ("L_read_oper_appendix: Badly formatted attributes");
        }
      ch = L_peek_next_char (input_buf);
    }
  /*
   *  read optional sync arcs.
   */
  if (ch == '{')
    {
      L_read_sync_arcs (input_buf, oper);
      ch = L_peek_next_char (input_buf);
    }

  /* Read paren to terminate oper appendix item */
  L_get_next_lcode_token_expecting (input_buf, ")", "L_read_oper_appendix");
}

void
L_read_cb_appendix (L_Func * fn, L_Input_Buf * input_buf)
{
  char ch, *token;
  int cb_id;
  L_Attr *attr, *cb_attr;
  L_Cb *cb;

  token = L_get_next_lcode_token (input_buf);

  if (L_strtol (token, NULL, &cb_id) == 0)
    {
      L_print_buf_with_arrow (stderr, input_buf);
      L_punt ("L_read_cbr_appendix: This is not a valid integer value.");
    }

  cb = L_cb_hash_tbl_find (fn->cb_hash_tbl, cb_id);
  if (cb == NULL)
    L_punt ("L_read_cb_appendix: Cb %d does not exist in function %s\n",
            cb_id, fn->name);

  /*  
   * Read oper attributes if there are any.
   */
  ch = L_peek_next_char (input_buf);
  if (ch == '<')
    {
      L_get_next_lcode_token (input_buf);       /* Throw away '<' */
      ch = L_peek_next_char (input_buf);

      /* 
       * Read region attributes if there are any <()()()>
       */
      if (ch == '(')
        {
          cb_attr = NULL;
          while (ch != '>')
            {
              attr = L_read_attr (NULL, input_buf);
              cb_attr = L_concat_attr (cb_attr, attr);

              ch = L_peek_next_char (input_buf);
            }
          L_get_next_lcode_token (input_buf);   /* Throw away '>' */

          cb->attr = L_concat_attr (cb->attr, cb_attr);
        }
      else
        {
          L_print_buf_with_arrow (stderr, input_buf);
          L_punt ("L_read_cb_appendix: Badly formatted attributes");
        }
    }
  /* Read paren to terminate cb appendix item */
  L_get_next_lcode_token_expecting (input_buf, ")", "L_read_cb_appendix");
}

void
L_read_appendix (L_Func * fn, L_Input_Buf * input_buf)
{
  int type, end_appendix;
  char ch, *token, *name;

  token = L_get_next_lcode_token (input_buf);

  /* The appendix may be empty, so just return */
  if (*token == ')')
    return;

  if (*token != '(')
    {
      L_print_buf_with_arrow (stderr, input_buf);
      L_punt ("L_read_appendix : Expecting token '(' but found '%s'.\n",
              token);
    }

  /* Get the item name */
  name = L_get_next_lcode_token (input_buf);

  end_appendix = 0;
  while (!end_appendix)
    {
      type = L_lcode_id (name);
      switch (type)
        {
        case L_INPUT_CB:
        case L_INPUT_ACB:
          L_read_cb_appendix (fn, input_buf);
          break;
        case L_INPUT_OP:
        case L_INPUT_AOP:
          L_read_oper_appendix (fn, input_buf);
          break;
        default:
          L_punt ("L_read_appendix: Unknown item in appendix -> %s\n", name);
        }

      ch = L_peek_next_char (input_buf);
      if (ch != ')')
        {
          /* Get the open paren that starts the next appendix item */
          L_get_next_lcode_token_expecting (input_buf, "(",
                                            "L_read_appendix");

          name = L_get_next_lcode_token (input_buf);
        }
      else
        end_appendix = 1;
    }

  /* Get the close paren that terminates the appendix */
  L_get_next_lcode_token_expecting (input_buf, ")", "L_read_appendix");
}


void
L_print_appendix (FILE * F, L_Func * fn)
{
  int i, print_attr, print_sync, cnt;
  L_Attr *attr;
  L_Cb *cb;
  L_Oper *oper;

  fprintf (F, " (appendix\n");
  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      print_attr = 0;
      if (L_use_appendix_for_attr == 1)
        {
          for (attr = cb->attr; attr != NULL; attr = attr->next_attr)
            {
#ifdef LP64_ARCHITECTURE
              if (((int)((long)STRING_find_symbol_data (L_cb_attribute_table,
							attr->name))) == 2)
#else
              if (((int) STRING_find_symbol_data (L_cb_attribute_table,
                                                  attr->name)) == 2)
#endif
                {
                  if (print_attr == 0)
                    {
                      fprintf (F, "  (Acb %d <", cb->id);
                      print_attr = 1;
                    }
                  L_print_attr (F, attr);
                }
            }
          if (print_attr)
            fprintf (F, ">)\n");
        }

      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
        {
          print_attr = 0;

          if (L_use_appendix_for_attr == 1)
            {
              for (attr = oper->attr; attr != NULL; attr = attr->next_attr)
                {
#ifdef LP64_ARCHITECTURE
                  if (((int)((long)STRING_find_symbol_data \
			             (L_oper_attribute_table,
				      attr->name))) == 2)
#else
                  if (((int) STRING_find_symbol_data (L_oper_attribute_table,
                                                      attr->name)) == 2)
#endif
                    {
                      if (print_attr == 0)
                        {
                          fprintf (F, "   (Aop %d <", oper->id);
                          print_attr = 1;
                        }
                      L_print_attr (F, attr);
                    }
                }
              if (print_attr)
                fprintf (F, ">");
            }

          print_sync = 0;
          if (L_use_appendix_for_sync_arcs &&
	      (oper->acc_info ||
	       (oper->sync_info && oper->sync_info->num_sync_in > 0)))
            {
	      if (!print_attr)
		fprintf (F, "   (Aop %d {", oper->id);
	      else
		fprintf (F, "\n         {");

	      if (oper->acc_info)
		{
		  L_print_acc_info_list (F, oper->acc_info);
		}
  
	      if (oper->sync_info && (oper->sync_info->num_sync_in > 0))
		{
		  cnt = 0;
		  for (i = 0; i < oper->sync_info->num_sync_in; i++)
		    {
		      if (cnt == 5)
			{
			  fprintf (F, "\n          ");
			  cnt = 0;
			}
		      L_print_sync (F, oper->sync_info->sync_in[i]);
		      cnt++;
		    }
		}

	      fprintf (F, "}");
	      print_sync = 1;
	    }

          if (print_attr || print_sync)
            fprintf (F, ")\n");
        }
    }
  fprintf (F, "  )\n");
}


/*============================================================
 *
 *  Eventlist input/output routines
 *
 *============================================================*/

/*
 * NOTE: THESE FILES HAVE BEEN TEMPORARILY PLACED IN l_event.c
 *       WHILST I AM DOING CONSTANT DEVELOPMENT ON THEM AND
 *       THIS FILE IS HUGE!
 */

/*============================================================
 *
 *  Region input/output routines
 *
 *============================================================*/

/*
 * NOTE: THESE FILES HAVE BEEN TEMPORARILY PLACED IN l_region.c
 *       WHILST I AM DOING CONSTANT DEVELOPMENT ON THEM AND
 *       THIS FILE IS HUGE!
 */

/*============================================================
 *
 *  Data structure input/output routines
 *
 *============================================================*/

/*
 *      L_Expr
 */
L_Expr *
L_read_expr (L_Input_Buf * input_buf)
{
  L_Expr *new_expr;
  char *spec;
  ITintmax ival;
  double fval;
  int type;
  char *token;

  /* 
   * Termination condition for recursion
   */
  if (L_peek_next_char (input_buf) == ')')
    {
      return (NULL);
    }

  /*
   * Get the '(' that begins the expression
   */
  L_get_next_lcode_token_expecting (input_buf, "(", "L_read_expr");

  spec = L_get_next_lcode_token (input_buf);
  type = L_expr_id (spec);

  if (type == -1)
    {
      L_print_buf_with_arrow (stderr, input_buf);
      L_punt ("L_read_expr: illegal expression type.");
    }

  if (L_peek_next_char (input_buf) == ')')
    {
      L_print_buf_with_arrow (stderr, input_buf);
      L_punt ("L_read_expr: Expression data is missing");
    }

  new_expr = L_new_expr (type);
  switch (type)
    {
    case L_EXPR_INT:
      token = L_get_next_lcode_token (input_buf);
      if (L_strtoll (token, &ival) == 0)
        {
          L_print_buf_with_arrow (stderr, input_buf);
          L_punt ("L_read_expr: integer expr contains invalid integer.");
        }
      new_expr->value.i = ival;
      break;
    case L_EXPR_FLOAT:
      token = L_get_next_lcode_token (input_buf);
      if (L_strtod (token, NULL, &fval) == 0)
        {
          L_print_buf_with_arrow (stderr, input_buf);
          L_punt ("L_read_expr: float expr contains invalid fp number.");
        }
      new_expr->value.f = fval;
      break;
    case L_EXPR_DOUBLE:
      token = L_get_next_lcode_token (input_buf);
      if (L_strtod (token, NULL, &fval) == 0)
        {
          L_print_buf_with_arrow (stderr, input_buf);
          L_punt ("L_read_expr: double expr contains invalid fp number.");
        }
      new_expr->value.f2 = fval;
      break;
    case L_EXPR_LABEL:
      spec = L_get_next_lcode_token (input_buf);
      new_expr->value.l = L_add_string (L_string_table, spec);
      break;
    case L_EXPR_STRING:
      spec = L_get_next_lcode_token (input_buf);
      new_expr->value.s = L_add_string (L_string_table, spec);
      break;
    case L_EXPR_ADD:
    case L_EXPR_SUB:
    case L_EXPR_MUL:
    case L_EXPR_DIV:
      new_expr->A = L_read_expr (input_buf);
      new_expr->B = L_read_expr (input_buf);
      if (new_expr->A == NULL || new_expr->B == NULL)
        {
          L_print_buf_with_arrow (stderr, input_buf);
          L_punt ("L_read_expr: Malformed expression");
        }
      break;
    case L_EXPR_NEG:
    case L_EXPR_COM:
      new_expr->A = L_read_expr (input_buf);
      if (new_expr->A == NULL)
        {
          L_print_buf_with_arrow (stderr, input_buf);
          L_punt ("L_read_expr: Malformed expression");
        }
      break;
    default:
      L_print_buf_with_arrow (stderr, input_buf);
      L_punt ("L_read_expr: illegal Expr");
      break;
    }
  new_expr->next_expr = L_read_expr (input_buf);
  L_get_next_lcode_token_expecting (input_buf, ")", "L_read_expr");
  return new_expr;
}

void
L_print_expr (FILE * F, L_Expr * expr)
{
  if (expr == 0)
    return;
  switch (expr->type)
    {
    case L_EXPR_INT:
      fprintf (F, "(i " ITintmaxformat ")", expr->value.i);
      break;
    case L_EXPR_FLOAT:
      fprintf (F, "(f %.8e)", expr->value.f);
      break;
    case L_EXPR_DOUBLE:
      fprintf (F, "(f2 %.16e)", expr->value.f2);
      break;
    case L_EXPR_LABEL:
      fprintf (F, "(l %s)", expr->value.l);
      break;
    case L_EXPR_STRING:
      fprintf (F, "(s %s)", expr->value.s);
      break;
    case L_EXPR_ADD:
      fprintf (F, "(add ");
      L_print_expr (F, expr->A);
      L_print_expr (F, expr->B);
      fprintf (F, ")");
      break;
    case L_EXPR_SUB:
      fprintf (F, "(sub ");
      L_print_expr (F, expr->A);
      L_print_expr (F, expr->B);
      fprintf (F, ")");
      break;
    case L_EXPR_MUL:
      fprintf (F, "(mul ");
      L_print_expr (F, expr->A);
      L_print_expr (F, expr->B);
      fprintf (F, ")");
      break;
    case L_EXPR_DIV:
      fprintf (F, "(div ");
      L_print_expr (F, expr->A);
      L_print_expr (F, expr->B);
      fprintf (F, ")");
      break;
    case L_EXPR_NEG:
      fprintf (F, "(neg ");
      L_print_expr (F, expr->A);
      fprintf (F, ")");
      break;
    case L_EXPR_COM:
      fprintf (F, "(com ");
      L_print_expr (F, expr->A);
      fprintf (F, ")");
      break;
    default:
      L_punt ("L_print_expr: illegal argument type");
    }
}

/*
 *      L_Datalist_Element
 */


void
L_print_datalist_element (FILE * F, L_Datalist_Element * element)
{
  L_print_data (F, element->data);
}


void
L_print_datalist (FILE * F, L_Datalist * list)
{
  L_Datalist_Element *curr_element;

  if (list == NULL)
    L_punt ("L_print_datalist: NULL list");

  curr_element = list->first_element;

  while (curr_element != NULL)
    {

      L_print_datalist_element (F, curr_element);

      curr_element = curr_element->next_element;
    }
}



/*
 *      L_Data
 */

void
L_read_data (int type, L_Input_Buf * input_buf)
{
  int n;
  char *label, *str;
  char *token;
  L_Expr *bit_field;            /* LCW - bit field of a structure - 4/20/96 */

  /* 
   * Create a new data element 
   */
  L_data = L_new_data (type);

  switch (type)
    {
    case L_INPUT_MS:
      label = L_get_next_lcode_token (input_buf);
      L_data->N = L_ms_id (label);
      break;
    case L_INPUT_VOID:
    case L_INPUT_BYTE:
    case L_INPUT_WORD:
    case L_INPUT_LONG:
    case L_INPUT_LONGLONG:
    case L_INPUT_FLOAT:
    case L_INPUT_DOUBLE:
    case L_INPUT_ALIGN:
    case L_INPUT_ELEMENT_SIZE:
      token = L_get_next_lcode_token (input_buf);
      if (L_strtol (token, NULL, &n) == 0)
        {
          L_print_buf_with_arrow (stderr, input_buf);
          L_punt ("L_read_data: not a valid integer.");
        }
      L_data->N = n;
      L_data->address = L_new_expr (L_EXPR_LABEL);

      label = L_get_next_lcode_token (input_buf);
      L_data->address->value.l = L_add_string (L_string_table, label);

      /* If the next character is not a ')' then    */
      /* the data item has an initialized value and */
      /* we must parse that expression              */
      if (L_peek_next_char (input_buf) != ')')
        L_data->value = L_read_expr (input_buf);
      break;
    case L_INPUT_ASCII:
    case L_INPUT_ASCIZ:
      str = L_get_next_lcode_token (input_buf);
      L_data->value = L_new_expr (L_EXPR_STRING);
      L_data->value->value.s = L_add_string (L_string_table, str);
      L_data->address = L_new_expr (L_EXPR_LABEL);
      label = L_get_next_lcode_token (input_buf);
      L_data->address->value.l = L_add_string (L_string_table, label);
      break;
    case L_INPUT_RESERVE:
      token = L_get_next_lcode_token (input_buf);
      if (L_strtol (token, NULL, &n) == 0)
        {
          L_print_buf_with_arrow (stderr, input_buf);
          L_punt ("L_read_data: reserve size is not a valid integer.");
        }
      L_data->N = n;
      break;
    case L_INPUT_GLOBAL:
      token = L_get_next_lcode_token (input_buf);
      L_data->address = L_new_expr (L_EXPR_LABEL);
      L_data->address->value.l = L_add_string (L_string_table, token);
      /* LCW - if the next character is not a ')', the data item has */
      /* type information. - 4/18/96 */
      if (L_peek_next_char (input_buf) != ')')
	{
	  if (!strcmp(L_peek_next_real_lcode_token(input_buf),"objid"))
	      L_data->id = L_read_id (input_buf, NULL);
	  L_data->h_type = L_read_type (input_buf, NULL);
	}
      break;
    case L_INPUT_WB:
    case L_INPUT_WW:
    case L_INPUT_WI:
    case L_INPUT_WQ:
    case L_INPUT_WF:
    case L_INPUT_WF2:
    case L_INPUT_WS:
      L_data->address = L_read_expr (input_buf);
      L_data->value = L_read_expr (input_buf);
      break;
      /* LCW - new tokens for preserving debugging info - 4/17/96 */
    case L_INPUT_DEF_STRUCT:
    case L_INPUT_DEF_UNION:
    case L_INPUT_DEF_ENUM:
      token = L_get_next_lcode_token (input_buf);
      L_data->address = L_new_expr (L_EXPR_LABEL);
      L_data->address->value.l = L_add_string (L_string_table, token);
      break;
    case L_INPUT_FIELD:
      token = L_get_next_lcode_token (input_buf);
      L_data->address = L_new_expr (L_EXPR_LABEL);
      L_data->address->value.l = L_add_string (L_string_table, token);
      /* LCW - if the next character is not a ')', the field has */
      /* type information or/and is a bit field. - 4/18/96 */
      if (L_peek_next_char (input_buf) != ')')
        {
          is_bit_field = 0;
          bit_field = L_new_expr (L_EXPR_INT);
          L_data->h_type = L_read_type (input_buf, bit_field);
          if (is_bit_field)
            L_data->value = bit_field;
          else
            L_delete_expr_element (bit_field);
        }
      break;
    case L_INPUT_ENUMERATOR:
      token = L_get_next_lcode_token (input_buf);
      L_data->address = L_new_expr (L_EXPR_LABEL);
      L_data->address->value.l = L_add_string (L_string_table, token);
      if (L_peek_next_char (input_buf) != ')')
        L_data->value = L_read_expr (input_buf);
      break;
    case L_INPUT_SKIP:
      token = L_get_next_lcode_token (input_buf);
      if (L_strtol (token, NULL, &n) == 0)
        {
          L_print_buf_with_arrow (stderr, input_buf);
          L_punt ("L_read_data: skip size is not a valid integer.");
        }
      L_data->N = n;
      break;
    }
  /*
   * Better get a ')' to terminate the data item
   */
  L_get_next_lcode_token_expecting (input_buf, ")", "L_read_data");
}

void
L_print_data (FILE * F, L_Data * data)
{
  L_Expr *addr, *val;
  int t;      /* LCW - type bit_vector field of L_Type structure - 4/16/96 */
  L_Dcltr *dclptr;   /* LCW */
  if (data == NULL)
    {
      L_punt ("L_print_data: no argument");
    }

  /*
   *  Insert comments into output file
   */
  if ((!L_generation_info_printed) && (L_output_generation_info) &&
      (!L_output_binary_format))
    {
      L_insert_generic_info_to_output_file (F);
      L_generation_info_printed = 1;
    }

  if (L_output_binary_format == 1)
    {
      if (L_binary_magic_number_emitted == 0)
        {
          putc (CURRENT_BINARY_VERSION, F);
          fprintf (F, "# Binary Lcode!\n%c", DELIMIT);
          L_binary_magic_number_emitted = 1;
        }
      L_print_data_binary (F, data);
      return;
    }

  switch (data->type)
    {
    case L_INPUT_MS:
      fprintf (F, "(ms %s)\n", L_ms_name (data->N));
      break;
    case L_INPUT_GLOBAL:
    case L_INPUT_VOID:
      addr = data->address;
      if (addr == NULL)
        {
          L_punt ("L_print_data: bad address");
        }
      fprintf (F, "(%s %s", L_lcode_name (data->type), addr->value.l);

      /* HCH 5/10/04: for getting object id's from Pcode */      
      if (data->id)
	fprintf (F, " (objid %i)", data->id);
#if 0
      /* JWS - prevent generation of non-objids.  This seems pointless. */
      else 
	fprintf (F, " (objid -1)");
#endif
      /* LCW - print the type information of a global variable - 4/16/96 */
      if (data->h_type != NULL)
        {
          t = data->h_type->type;
          if (t & L_DATA_CONST)
            fprintf (F, " (const)");
          if (t & L_DATA_VOLATILE)
            fprintf (F, " (volatile)");
          if (t & L_DATA_NOALIAS)
            fprintf (F, " (noalias)");
          if (t & L_DATA_REGISTER)
            fprintf (F, " (register)");
          if (t & L_DATA_AUTO)
            fprintf (F, " (auto)");
          if (t & L_DATA_STATIC)
            fprintf (F, " (static)");
          if (t & L_DATA_EXTERN)
            fprintf (F, " (extern)");
          if (t & L_DATA_GLOBAL)
            fprintf (F, " (global)");
          if (t & L_DATA_PARAMETER)
            fprintf (F, " (parameter)");
          if (t & L_DATA_VOID)
            fprintf (F, " (void)");
          if (t & L_DATA_CHAR)
            fprintf (F, " (char)");
          if (t & L_DATA_SHORT)
            fprintf (F, " (short)");
          if (t & L_DATA_INT)
            fprintf (F, " (int)");
          if (t & L_DATA_LONG)
            fprintf (F, " (long)");
          if (t & L_DATA_LONGLONG)
            fprintf (F, " (longlong)");
          if (t & L_DATA_FLOAT)
            fprintf (F, " (float)");
          if (t & L_DATA_DOUBLE)
            fprintf (F, " (double)");
          if (t & L_DATA_LONGDOUBLE)
            fprintf (F, " (longdouble)");
          if (t & L_DATA_SIGNED)
            fprintf (F, " (signed)");
          if (t & L_DATA_UNSIGNED)
            fprintf (F, " (unsigned)");
          if (t & L_DATA_STRUCT)
            fprintf (F, " (struct %s)", data->h_type->struct_name);
          if (t & L_DATA_UNION)
            fprintf (F, " (union %s)", data->h_type->struct_name);
          if (t & L_DATA_ENUM)
            fprintf (F, " (enum %s)", data->h_type->struct_name);
          dclptr = data->h_type->dcltr;
          while (dclptr)
            {
              switch (dclptr->method)
                {
                case L_D_ARRY:
                  fprintf (F, " (a");
                  if (dclptr->index != NULL)
                    {
                      fprintf (F, " ");
                      L_print_expr (F, dclptr->index);
                    }
                  fprintf (F, ")");
                  break;
                case L_D_PTR:
                  fprintf (F, " (p)");
                  break;
                case L_D_FUNC:
                  fprintf (F, " (f)");
                  break;
                default:
                  L_punt
                    ("L_print_data: illegal declarator (access pattern)");
                }
              dclptr = dclptr->next;
            }
        }
      fprintf (F, ")\n");
      break;
    case L_INPUT_BYTE:
    case L_INPUT_WORD:
    case L_INPUT_LONG:
    case L_INPUT_LONGLONG:
    case L_INPUT_FLOAT:
    case L_INPUT_DOUBLE:
    case L_INPUT_ALIGN:
    case L_INPUT_ELEMENT_SIZE:
      addr = data->address;
      if (addr == NULL)
        {
          L_punt ("L_print_data: bad address");
        }
      fprintf (F, "(%s %d %s",
               L_lcode_name (data->type), data->N, addr->value.l);
      for (val = data->value; val != 0; val = val->next_expr)
        {
          fprintf (F, " ");
          L_print_expr (F, val);
        }
      fprintf (F, ")\n");
      break;
    case L_INPUT_ASCII:
    case L_INPUT_ASCIZ:
      addr = data->address;
      val = data->value;
      if ((addr == NULL) || (val == NULL))
        {
          L_punt ("L_print_data: bad pointer");
        }
      fprintf (F, "(%s %s %s)\n",
               L_lcode_name (data->type), val->value.s, addr->value.l);
      break;
    case L_INPUT_RESERVE:
      fprintf (F, "(reserve %d)\n", data->N);
      break;
    case L_INPUT_WB:
    case L_INPUT_WW:
    case L_INPUT_WI:
    case L_INPUT_WQ:
    case L_INPUT_WF:
    case L_INPUT_WF2:
    case L_INPUT_WS:
      addr = data->address;
      val = data->value;
      if ((addr == NULL) || (val == NULL))
        {
          L_punt ("L_print_data: bad pointer");
        }
      fprintf (F, "(%s ", L_lcode_name (data->type));
      L_print_expr (F, addr);
      fprintf (F, " ");
      L_print_expr (F, val);
      fprintf (F, ")\n");
      break;
      /* LCW - print struct, union, enum data - 4/16/96 */
    case L_INPUT_DEF_STRUCT:
    case L_INPUT_DEF_UNION:
    case L_INPUT_DEF_ENUM:
      addr = data->address;
      if (addr == NULL)
        {
          L_punt ("L_print_data: bad address");
        }
      fprintf (F, "(%s %s)\n", L_lcode_name (data->type), addr->value.l);
      break;
    case L_INPUT_FIELD:
    case L_INPUT_ENUMERATOR:
      addr = data->address;
      val = data->value;
      if (addr == NULL)
        {
          L_punt ("L_print_data: bad address");
        }
      fprintf (F, "(%s %s", L_lcode_name (data->type), addr->value.l);
      if (data->h_type != NULL)
        {
          t = data->h_type->type;
          if (t & L_DATA_CONST)
            fprintf (F, " (const)");
          if (t & L_DATA_VOLATILE)
            fprintf (F, " (volatile)");
          if (t & L_DATA_NOALIAS)
            fprintf (F, " (noalias)");
          if (t & L_DATA_REGISTER)
            fprintf (F, " (register)");
          if (t & L_DATA_AUTO)
            fprintf (F, " (auto)");
          if (t & L_DATA_STATIC)
            fprintf (F, " (static)");
          if (t & L_DATA_EXTERN)
            fprintf (F, " (extern)");
          if (t & L_DATA_GLOBAL)
            fprintf (F, " (global)");
          if (t & L_DATA_PARAMETER)
            fprintf (F, " (parameter)");
          if (t & L_DATA_VOID)
            fprintf (F, " (void)");
          if (t & L_DATA_CHAR)
            fprintf (F, " (char)");
          if (t & L_DATA_SHORT)
            fprintf (F, " (short)");
          if (t & L_DATA_INT)
            fprintf (F, " (int)");
          if (t & L_DATA_LONG)
            fprintf (F, " (long)");
          if (t & L_DATA_LONGLONG)
            fprintf (F, " (longlong)");
          if (t & L_DATA_FLOAT)
            fprintf (F, " (float)");
          if (t & L_DATA_DOUBLE)
            fprintf (F, " (double)");
          if (t & L_DATA_SIGNED)
            fprintf (F, " (signed)");
          if (t & L_DATA_UNSIGNED)
            fprintf (F, " (unsigned)");
          if (t & L_DATA_STRUCT)
            fprintf (F, " (struct %s)", data->h_type->struct_name);
          if (t & L_DATA_UNION)
            fprintf (F, " (union %s)", data->h_type->struct_name);
          if (t & L_DATA_ENUM)
            fprintf (F, " (enum %s)", data->h_type->struct_name);
          dclptr = data->h_type->dcltr;
          while (dclptr)
            {
              switch (dclptr->method)
                {
                case L_D_ARRY:
                  fprintf (F, " (a");
                  if (dclptr->index != NULL)
                    {
                      fprintf (F, " ");
                      L_print_expr (F, dclptr->index);
                    }
                  fprintf (F, ")");
                  break;
                case L_D_PTR:
                  fprintf (F, " (p)");
                  break;
                case L_D_FUNC:
                  fprintf (F, " (f)");
                  break;
                default:
                  L_punt
                    ("L_print_data: illegal declarator (access pattern)");
                }
              dclptr = dclptr->next;
            }
        }
      if (val != NULL)
        {
          fprintf (F, " ");
          L_print_expr (F, val);
        }
      fprintf (F, ")\n");
      break;
    case L_INPUT_SKIP:
      fprintf (F, "(skip %d)\n", data->N);
      break;
    default:
      L_punt ("L_print_data: illegal data type");
    }
}


/*
 *      SAM 7-96, read in an entire hash table into a L_Datalist
 *      Currently this is not used in Lcode for anything (for future use)
 *      Note this function calls L_read_data(), so the global var L_data
 *      is destroyed by this function.
 */
void
L_read_hash_tbl (L_Input_Buf * input_buf, L_Datalist * tbl)
{
  char *keyword;
  int i, type, align_value, reserve_value, tbl_size;
  L_Datalist_Element *element;

  /*
   *  1. The first entry of the table should be a data of the form:
   *     (align tbl_entry_size tbl_name)
   */
  L_get_next_lcode_token_expecting (input_buf, "(", "L_read_hash_tbl");
  keyword = L_get_next_lcode_token (input_buf);
  type = L_lcode_id (keyword);

  if(type == L_INPUT_MS) {
    //ignore extra (ms data)...if present -shoe
    L_get_next_lcode_token_expecting (input_buf, 
				      "data", "L_read_hash_tbl");
    L_get_next_lcode_token_expecting (input_buf, 
				      ")", "L_read_hash_tbl");
    L_get_next_lcode_token_expecting (input_buf,
				      "(", "L_read_hash_tbl");
    keyword = L_get_next_lcode_token (input_buf);
    type = L_lcode_id (keyword);
  }
  
  if (type != L_INPUT_ALIGN)
    {
      L_print_buf_with_arrow (stderr, input_buf);
      L_punt ("L_read_hash_tbl: missing align stmt on line %d",
              input_buf->line_count);
    }

  L_read_data (type, input_buf);
  align_value = L_data->N;

  /* Create datalist_element and append to tbl */
  element = L_new_datalist_element (L_data);
  L_concat_datalist_element (tbl, element);

  /*
   *  2. The next token should be a data of the form:
   *     (reserve tbl_size)
   */
  L_get_next_lcode_token_expecting (input_buf, "(", "L_read_hash_tbl");
  keyword = L_get_next_lcode_token (input_buf);
  type = L_lcode_id (keyword);

  if (type != L_INPUT_RESERVE)
    {
      L_print_buf_with_arrow (stderr, input_buf);
      L_punt ("L_read_hash_tbl: missing reserve stmt on line %d",
              input_buf->line_count);
    }

  L_read_data (type, input_buf);
  reserve_value = L_data->N;

  /* Create datalist_element and append to tbl */
  element = L_new_datalist_element (L_data);
  L_concat_datalist_element (tbl, element);

  /*
   *  3. Read in the actual table entries
   *     Each entry is a "wi" data item containing the address
   *     and the target cb
   */
  tbl_size = reserve_value / align_value;

  for (i = 0; i < tbl_size; i++)
    {
      L_get_next_lcode_token_expecting (input_buf, "(", "L_read_hash_tbl");
      keyword = L_get_next_lcode_token (input_buf);
      type = L_lcode_id (keyword);

      if (type != L_INPUT_WI && type != L_INPUT_WQ)
        {
          L_print_buf_with_arrow (stderr, input_buf);
          L_punt ("L_read_hash_tbl: missing wi stmt on line %d",
                  input_buf->line_count);
        }

      L_read_data (type, input_buf);
      /* Create datalist_element and append to tbl */
      element = L_new_datalist_element (L_data);
      L_concat_datalist_element (tbl, element);
    }
}


/*
 *      SAM 7-96, read all the hash tables into a L_Datalist
 *      Currently this is not used in Lcode for anything (for future use)
 *      Note this function calls L_read_data(), so the global var L_data
 *      is destroyed by this function.
 */
L_Datalist *
L_read_all_hashtbls (L_Input_Buf * input_buf, int num_tbls)
{
  L_Datalist *tbl;
  char *keyword;
  int i, type;
  L_Datalist_Element *element;

  tbl = L_new_datalist ();

  /* Before the tables is an initial (ms data), so grab that first */
  L_get_next_lcode_token_expecting (input_buf, "(", "L_read_all_hashtbls");
  keyword = L_get_next_lcode_token (input_buf);
  type = L_lcode_id (keyword);

  if (type != L_INPUT_MS)
    {
      L_print_buf_with_arrow (stderr, input_buf);
      L_punt ("L_read_all_hashtbls: missing ms stmt on line %d",
              input_buf->line_count);
    }

  L_read_data (type, input_buf);
  /* Create datalist_element and append to tbl */
  element = L_new_datalist_element (L_data);
  L_concat_datalist_element (tbl, element);

  /* Now read all the tables in, append the data items to end of tbl */
  for (i = 0; i < num_tbls; i++)
  {
    L_read_hash_tbl (input_buf, tbl);
  }

  return (tbl);
}


/*============================================================
 *
 *  Flow arc input routines  ->  (flow CC.d BB_ID.d WEIGHT.f)
 *
 *============================================================*/
L_Flow *
L_read_flow (L_Func * fn, L_Cb * src_cb, L_Input_Buf * input_buf)
{
  char *token;
  int cc, dest;
  double weight;
  L_Cb *dest_cb;

  L_get_next_lcode_token_expecting (input_buf, "(", "L_read_flow");
  L_get_next_lcode_token_expecting (input_buf, "flow", "L_read_flow");

  token = L_get_next_lcode_token (input_buf);
  if (L_strtol (token, NULL, &cc) == 0)
    {
      L_print_buf_with_arrow (stderr, input_buf);
      L_punt ("L_read_flow: The flow cc is not a valid integer value.");
    }
  token = L_get_next_lcode_token (input_buf);
  if (L_strtol (token, NULL, &dest) == 0)
    {
      L_print_buf_with_arrow (stderr, input_buf);
      L_punt
        ("L_read_flow: The flow dest cb id is not a valid integer value.");
    }
  token = L_get_next_lcode_token (input_buf);
  if (L_strtod (token, NULL, &weight) == 0)
    {
      L_print_buf_with_arrow (stderr, input_buf);
      L_punt
        ("L_read_flow: The flow weight is not a valid floating-point value.");
    }

  dest_cb = L_cb_hash_tbl_find_and_alloc (fn->cb_hash_tbl, dest);

  L_get_next_lcode_token_expecting (input_buf, ")", "L_read_flow");

  return (L_new_flow (cc, src_cb, dest_cb, weight));
}


/*============================================================
 *
 *  Attribute input routines
 *
 *============================================================*/
L_Attr *
L_read_attr (L_Func * fn, L_Input_Buf * input_buf)
{
  char *name;
  int i;
  L_Attr *new_attr;
  L_Operand *operand;

  L_get_next_lcode_token_expecting (input_buf, "(", "L_read_attr");

  /* Get attribute name */
  name = L_get_next_lcode_token (input_buf);
  new_attr = L_new_attr (name, 0);

  /*
   * If next character is ')' then attribute is of the form (name)
   */
  if (L_peek_next_char (input_buf) != ')')
    {
      /* 
       * The attribute may now be in two forms:
       *   (name int) or (name (operand))
       */
      if (L_peek_next_char (input_buf) != '(')
        {
          char *integer = L_get_next_lcode_token (input_buf);
          L_set_int_attr_field (new_attr, 0, atoi (integer));
        }
      else
        {
          i = 0;
          do
            {
              operand = L_read_operand (fn, input_buf);
              L_set_attr_field (new_attr, i, operand);
              i++;
            }
          while (L_peek_next_char (input_buf) != ')');
        }
    }

  L_get_next_lcode_token_expecting (input_buf, ")", "L_read_attr");

  return (new_attr);
}

void
L_print_attr (FILE * F, L_Attr * attr)
{
  int i;

  if (attr->max_field == 0)
    fprintf (F, "(%s)", attr->name);
  else
    {
      fprintf (F, "(%s ", attr->name);

      for (i = 0; i < attr->max_field; i++)
        L_print_operand (F, attr->field[i], 0);

      fprintf (F, ")");
    }

#if 0
  fprintf (F, "(%s %ld)", attr->name, attr->value);
#endif
}

/*============================================================
 *
 *  Sync arc input routines
 *
 *============================================================*/
L_Sync *
L_read_sync (L_Input_Buf * input_buf)
{
  char *token;
  char name[20];
  int dep_oper_id, dist, flags, prof;

  /* "(" has already been read, inside L_read_sync_arcs */

  token = L_get_next_lcode_token (input_buf);
  if (*token == ')')            /* empty sync arc */
    return (NULL);
  if (L_strtol (token, NULL, &dep_oper_id) == 0)
    {
      L_print_buf_with_arrow (stderr, input_buf);
      L_punt ("L_read_flow: "
              "The sync dependent oper id is not a valid integer value.");
    }

  token = L_get_next_lcode_token (input_buf);
  strcpy (name, token);

  token = L_get_next_lcode_token (input_buf);
  if (L_strtol (token, NULL, &dist) == 0)
    {
      L_print_buf_with_arrow (stderr, input_buf);
      L_punt ("L_read_flow: The sync distance is not a valid integer value.");
    }

  token = L_get_next_lcode_token (input_buf);
  if (L_strtol (token, NULL, &flags) == 0)
    {
      L_print_buf_with_arrow (stderr, input_buf);
      L_punt
        ("L_read_flow: The sync flags field is not a valid integer value.");
    }

  token = L_get_next_lcode_token (input_buf);
  if (L_strtol (token, NULL, &prof) == 0)
    {
      L_print_buf_with_arrow (stderr, input_buf);
      L_punt
        ("L_read_flow: The sync prof field is not a valid integer value.");
    }

  L_get_next_lcode_token_expecting (input_buf, ")", "L_read_sync");

  if (L_eliminate_sync_arcs)
    return (NULL);
  else
    return (L_create_new_sync
            (dep_oper_id, name[0], name[1], dist, flags, prof));

}
void
L_print_sync (FILE * F, L_Sync * sync)
{
  int flags, id, prof;
  char prob, freq = '\0';

  flags = (int) sync->info;
  prof = (int) sync->prof_info;

  /* Get the type of sync arc */
  if (IS_DEFINITE_SYNC (flags))
    prob = 'D';
  else if (IS_PROFILE_SYNC (flags))
    prob = 'P';
  else
    prob = 'M';

  /* Get the characteristics of the sync arcs */
  switch (flags & 0x6000)
    {
    case 0x0000:
      freq = 'A';
      break;
    case 0x2000:
      freq = 'F';
      break;
    case 0x4000:
      freq = 'S';
      break;
    case 0x6000:
      freq = 'I';
      break;
    }

  flags &= 0x1fff;

  /* This routine can get called at any time using DB_print_*, so try
     to avoid seg faulting when dep_oper is just a (small) number, ie.
     not a ptr to the full dep_oper struct. */
  if (((unsigned int) (long) sync->dep_oper) > 2048)
    {
      if (sync->dep_oper)
        id = sync->dep_oper->id;
      else
        id = -1;
    }
  else
    {
      id = (int)(long) sync->dep_oper;
    }

  fprintf (F, "(%d %c%c %d %d %d)", id, prob, freq, (int) sync->dist, flags,
           prof);
}


/*============================================================
 *
 *  Memory access specifier input routines
 *
 *============================================================*/
L_AccSpec *
L_read_mem_acc_spec (L_Input_Buf * input_buf)
{
  char *token;
  int is_def = 0, id, version, offset, size;

  /* "(" has already been read, inside L_read_sync_arcs */

  token = L_get_next_lcode_token (input_buf);
  if (*token == ')')            /* empty sync arc */
    return (NULL);

  if (*token == 'D')
    {
      is_def = 1;
    }
  else if (*token == 'U')
    {
      is_def = 0;
    }
  else
    {
      L_print_buf_with_arrow (stderr, input_buf);
      L_punt ("L_read_mem_acc_spec: parse error");
    }

  token = L_get_next_lcode_token (input_buf);

  if (L_strtol (token, NULL, &id) == 0)
    {
      L_print_buf_with_arrow (stderr, input_buf);
      L_punt ("L_read_mem_acc_spec: parse error");
    }

  if (L_versioned_acc_specs)
    {
      token = L_get_next_lcode_token (input_buf);

      if (L_strtol (token, NULL, &version) == 0)
	{
	  L_print_buf_with_arrow (stderr, input_buf);
	  L_punt ("L_read_mem_acc_spec: parse error");
	}
    }
  else
    {
      version = -1;
    }

  token = L_get_next_lcode_token (input_buf);

  if (L_strtol (token, NULL, &offset) == 0)
    {
      L_print_buf_with_arrow (stderr, input_buf);
      L_punt ("L_read_mem_acc_spec: parse error");
    }

  token = L_get_next_lcode_token (input_buf);

  if (L_strtol (token, NULL, &size) == 0)
    {
      L_print_buf_with_arrow (stderr, input_buf);
      L_punt ("L_read_mem_acc_spec: parse error");
    }

  L_get_next_lcode_token_expecting (input_buf, ")", "L_read_sync");

  if (L_eliminate_sync_arcs)
    return (NULL);
  else
    return (L_new_mem_acc_spec (is_def, id, version, offset, size));
}


void
L_print_mem_acc_spec (FILE * F, L_AccSpec *mas)
{
  if (L_versioned_acc_specs)
    fprintf (F, "(%c %d %d %d %d)", mas->is_def ? 'D' : 'U',
	     mas->id, mas->version, mas->offset, mas->size);
  else
    fprintf (F, "(%c %d %d %d)", mas->is_def ? 'D' : 'U',
	     mas->id, mas->offset, mas->size);
  return;
}


static void
L_print_acc_info_list (FILE *F, L_AccSpec *mas_list)
{
  L_AccSpec *mas;
  int cnt = 0;

  for (mas = mas_list; mas; mas = mas->next)
    {
      if (++cnt > 5)
	{
	  cnt = 0;
	  fprintf (F, "\n\t");
	}
      L_print_mem_acc_spec (F, mas);
    }

  return;
}

/*============================================================
 *
 *  Operand input/output routines
 *
 *============================================================*/
L_Operand *
L_read_operand (L_Func * fn, L_Input_Buf * input_buf)
{
  L_Operand *operand = NULL;
  char *ty, *token, *str, *colon;
  int ival, type, omega;
  ITintmax lival;
  int reg_id, ptype, ctype;
  int macro_id;
  double fval;
  L_Cb *cb;

  L_get_next_lcode_token_expecting (input_buf, "(", "L_read_operand");

  ty = L_get_next_lcode_token (input_buf);

  /* Check for empty (NULL) operand */
  if (*ty == ')')
    return (NULL);

  type = L_operand_id (ty);
  ctype = L_ctype_id (ty);

  token = L_get_next_lcode_token (input_buf);
  switch (type)
    {
    case L_OPERAND_CB:
      if (L_strtol (token, NULL, &ival) == 0)
        {
          L_print_buf_with_arrow (stderr, input_buf);
          L_punt ("L_read_operand: The cb id is not a valid integer value.");
        }
      cb = L_cb_hash_tbl_find_and_alloc (fn->cb_hash_tbl, ival);
      operand = L_new_cb_operand (cb);
      break;
    case L_OPERAND_IMMED:
      switch (ctype)
        {
        case L_CTYPE_CHAR:
        case L_CTYPE_UCHAR:
        case L_CTYPE_SHORT:
        case L_CTYPE_USHORT:
        case L_CTYPE_INT:
        case L_CTYPE_UINT:
        case L_CTYPE_LONG:
        case L_CTYPE_ULONG:
        case L_CTYPE_LLONG:
        case L_CTYPE_ULLONG:
        case L_CTYPE_LLLONG:
        case L_CTYPE_ULLLONG:
          if (L_strtoll (token, &lival) == 0)
            {
              L_print_buf_with_arrow (stderr, input_buf);
              L_punt ("L_read_operand: This is not a valid integer value.");
            }
          operand = L_new_int_operand (lival, ctype);
          break;
        case L_CTYPE_FLOAT:
          if (L_strtod (token, NULL, &fval) == 0)
            {
              L_print_buf_with_arrow (stderr, input_buf);
              L_punt
                ("L_read_operand: This is not a valid floating-point value.");
            }
          operand = L_new_float_operand ((float) fval);
          break;
        case L_CTYPE_DOUBLE:
          if (L_strtod (token, NULL, &fval) == 0)
            {
              L_print_buf_with_arrow (stderr, input_buf);
              L_punt
                ("L_read_operand: This is not a valid floating-point value.");
            }
          operand = L_new_double_operand (fval);
          break;
	case L_CTYPE_POINTER:
	  if (L_strtoll (token, &lival) == 0)
            {
              L_print_buf_with_arrow (stderr, input_buf);
              L_punt ("L_read_operand: This is not a valid integer value.");
            }
          operand = L_new_int_operand (lival, ctype);
          break;
        default:
          L_punt ("L_read_operand: This is not a valid immediate ctype: "
		  "ty='%s' type=%d ctype=%d token='%s'\n",
		  ty, type, ctype, token);
          break;
        }
      break;
    case L_OPERAND_STRING:
      /* Token should be a quoted string */
      if (token[0] != '"')
        {
          L_print_buf_with_arrow (stderr, input_buf);
          L_punt ("L_read_operand: This is not a valid string.");
        }
      operand = L_new_string_operand (token, ctype);
      break;
    case L_OPERAND_MACRO:
      /* Token should be a macro name */
      macro_id = L_macro_id (token);
      if (macro_id == -1)
        {
          L_print_buf_with_arrow (stderr, input_buf);
          L_punt ("L_read_operand: unknown macro <%s>\n \
*** Make sure this is %s/%s lcode! ***", token, L_arch, L_model);
        }
      token = L_get_next_lcode_token (input_buf);
      ctype = L_ctype_id (token);
      if (ctype == -1)
        {
          L_print_buf_with_arrow (stderr, input_buf);
          L_punt ("L_read_operand: illegal format (macro ctype)");
        }
      if (L_is_ctype_predicate_direct (ctype))
        {
          ptype = L_ptype_id (token);
          if (ptype == -1)
            {
              L_print_buf_with_arrow (stderr, input_buf);
              L_punt ("L_read_operand: illegal format (macro ptype)");
            }
        }
      else
        {
          ptype = L_PTYPE_NULL;
        }
      operand = L_new_macro_operand (macro_id, ctype, ptype);
      break;
    case L_OPERAND_REGISTER:
      if (L_strtol (token, NULL, &reg_id) == 0)
        {
          L_print_buf_with_arrow (stderr, input_buf);
          L_punt
            ("L_read_operand: The register id is not a valid integer value");
        }
      token = L_get_next_lcode_token (input_buf);
      ctype = L_ctype_id (token);

      if (ctype == -1)
        {
          L_print_buf_with_arrow (stderr, input_buf);
          L_punt ("L_read_operand: illegal format (r ctype)");
        }
      if (L_is_ctype_predicate_direct (ctype))
        {
          ptype = L_ptype_id (token);
          if (ptype == -1)
            {
              L_print_buf_with_arrow (stderr, input_buf);
              L_punt ("L_read_operand: illegal format (r ptype)");
            }
        }
      else
        {
          ptype = L_PTYPE_NULL;
        }
      operand = L_new_register_operand (reg_id, ctype, ptype);
      break;
    case L_OPERAND_LABEL:
      /* Token should be a label name */
      if (token[0] == '"' || L_peek_next_char (input_buf) != ')')
        {
          L_print_buf_with_arrow (stderr, input_buf);
          L_punt ("L_read_operand: This is not a valid label!\n");
        }
      operand = L_new_gen_label_operand (token);
      operand->ctype = ctype;
      break;
    case L_OPERAND_RREGISTER:
      if (L_strtol (token, NULL, &reg_id) == 0)
        {
          L_print_buf_with_arrow (stderr, input_buf);
          L_punt ("L_read_operand: "
                  "The rot register id is not a valid integer value");
        }
      token = L_get_next_lcode_token (input_buf);
      ctype = L_ctype_id (token);
      if (ctype == -1)
        {
          L_print_buf_with_arrow (stderr, input_buf);
          L_punt ("L_read_operand: illegal format (rr ctype)");
        }
      if (L_is_ctype_predicate_direct (ctype))
        {
          ptype = L_ptype_id (token);
          if (ptype == -1)
            {
              L_print_buf_with_arrow (stderr, input_buf);
              L_punt ("L_read_operand: illegal format (rr ptype)");
            }
        }
      else
        {
          ptype = L_PTYPE_NULL;
        }
      operand = L_new_rregister_operand (reg_id, ctype, ptype);
      break;
    case L_OPERAND_EVR:
      /* This token is of the form int:int , so we must */
      /* decompose into register id and omega value     */
      colon = strchr (token, ':');
      if (colon == NULL)
        {
          L_print_buf_with_arrow (stderr, input_buf);
          L_punt ("L_read_operand: Malformed evr specifier!\n");
        }
      colon--;

      (void) L_strtol (token, &colon, &reg_id);
      if (*colon != ':')
        {
          L_print_buf_with_arrow (stderr, input_buf);
          L_punt ("L_read_operand: The evr id is not a valid integer value");
        }
      if (L_strtol (colon + 1, NULL, &omega) == 0)
        {
          L_print_buf_with_arrow (stderr, input_buf);
          L_punt
            ("L_read_operand: The evr omega is not a valid integer value");
        }
      str = L_get_next_lcode_token (input_buf);
      ctype = L_ctype_id (str);

      if (ctype == -1)
        {
          L_print_buf_with_arrow (stderr, input_buf);
          L_punt ("L_read_operand: illegal format (evr ctype)");
        }
      if (L_is_ctype_predicate_direct (ctype))
        {
          ptype = L_ptype_id (str);
          if (ptype == -1)
            {
              L_punt ("L_read_operand: illegal format (evr ptype)");
            }
        }
      else
        {
          ptype = L_PTYPE_NULL;
        }
      operand = L_new_evr_operand (reg_id, omega, ctype, ptype);
      break;
    default:
      L_print_buf_with_arrow (stderr, input_buf);
      L_punt ("L_read_operand: illegal operand format");
    }

  L_get_next_lcode_token_expecting (input_buf, ")", "L_read_operand");

  return operand;
}

/*
 *      if flag = 1, print out floats with lots of accuracy, otherwise
 *      just print out 3 digits for things like attributes which dont
 *      require so much accuracy.
 */
void
L_print_operand (FILE * F, L_Operand * operand, int flag)
{
  if (operand == NULL)
    {
      fprintf (F, "()");
      return;
    }
  switch (operand->type)
    {
    case L_OPERAND_CB:
      fprintf (F, "(cb %d)", operand->value.cb->id);
      break;
    case L_OPERAND_IMMED:
      if (L_is_ctype_integer (operand))
        {
          if (L_output_obsolete_ctype_format)
            {
              fprintf (F, "(i " ITintmaxformat ")", operand->value.i);
            }
          else
            {
              switch (operand->ctype)
                {
                case L_CTYPE_CHAR:
                  fprintf (F, "(c %d)", (int) operand->value.i);
                  break;
                case L_CTYPE_UCHAR:
                  fprintf (F, "(uc %d)", (int) operand->value.i);
                  break;
                case L_CTYPE_SHORT:
                  fprintf (F, "(sh %d)", (int) operand->value.i);
                  break;
                case L_CTYPE_USHORT:
                  fprintf (F, "(ush %d)", (int) operand->value.i);
                  break;
                case L_CTYPE_INT:
                  fprintf (F, "(i %d)", (int) operand->value.i);
                  break;
                case L_CTYPE_UINT:
                  fprintf (F, "(ui %d)", (int) operand->value.i);
                  break;
                case L_CTYPE_LONG:
                  fprintf (F, "(lng %d)", (int) operand->value.i);
                  break;
                case L_CTYPE_ULONG:
                  fprintf (F, "(ulng %d)", (int) operand->value.i);
                  break;
#ifdef IT64BIT
                case L_CTYPE_LLONG:
                  fprintf (F, "(ll %lld)", operand->value.i);
                  break;
                case L_CTYPE_ULLONG:
                  fprintf (F, "(ull %lld)", operand->value.i);
                  break;
                case L_CTYPE_LLLONG:
                  fprintf (F, "(lll %lld)", operand->value.i);
                  break;
                case L_CTYPE_ULLLONG:
                  fprintf (F, "(ulll %lld)", operand->value.i);
                  break;
#else
                case L_CTYPE_LLONG:
                  fprintf (F, "(ll %d)", operand->value.i);
                  break;
                case L_CTYPE_ULLONG:
                  fprintf (F, "(ull %d)", operand->value.i);
                  break;
                case L_CTYPE_LLLONG:
                  fprintf (F, "(lll %d)", operand->value.i);
                  break;
                case L_CTYPE_ULLLONG:
                  fprintf (F, "(ulll %d)", operand->value.i);
                  break;
#endif
                case L_CTYPE_POINTER:
                  fprintf (F, "(pnt " ITintmaxformat ")", operand->value.i);
                  break;
                default:
                  L_warn ("L_print_operand : Illegal type %d",
                          operand->ctype);
                  break;
                }
            }
        }
      if (L_is_ctype_flt (operand))
        {
          if (flag)
            fprintf (F, "(f %.8e)", operand->value.f);
          else
            fprintf (F, "(f %.3g)", operand->value.f);
        }
      if (L_is_ctype_dbl (operand))
        {
          if (flag)
            fprintf (F, "(f2 %.16e)", operand->value.f2);
          else
            fprintf (F, "(f2 %.3g)", operand->value.f2);
        }
      break;
    case L_OPERAND_STRING:
      if (L_output_obsolete_ctype_format)
        {
          fprintf (F, "(s %s)", operand->value.s);
        }
      else
        {
          switch (operand->ctype)
            {
            case L_CTYPE_LOCAL_ABS:
              fprintf (F, "(s_l_abs %s)", operand->value.s);
              break;
            case L_CTYPE_LOCAL_GP:
              fprintf (F, "(s_l_gp %s)", operand->value.s);
              break;
            case L_CTYPE_GLOBAL_ABS:
              fprintf (F, "(s_g_abs %s)", operand->value.s);
              break;
            case L_CTYPE_GLOBAL_GP:
              fprintf (F, "(s_g_gp %s)", operand->value.s);
              break;
            }
        }
      break;
    case L_OPERAND_MACRO:
      if (!L_is_ctype_predicate (operand))
        fprintf (F, "(mac %s %s)", L_macro_name (operand->value.mac),
                 L_ctype_name (operand->ctype));
      else
        fprintf (F, "(mac %s %s)", L_macro_name (operand->value.mac),
                 L_ptype_name (operand->ptype));
      break;
    case L_OPERAND_REGISTER:
      if (!L_is_ctype_predicate (operand))
        {
          if (L_output_obsolete_ctype_format)
            fprintf (F, "(r %d %s)", operand->value.r,
                     L_ctype_name (L_return_old_ctype (operand)));
          else
            fprintf (F, "(r %d %s)", operand->value.r,
                     L_ctype_name (operand->ctype));
        }
      else
        fprintf (F, "(r %d %s)", operand->value.r,
                 L_ptype_name (operand->ptype));
      break;
    case L_OPERAND_LABEL:
      if (L_output_obsolete_ctype_format)
        fprintf (F, "(l %s)", operand->value.s);
      else
        {
          switch (operand->ctype)
            {
            case L_CTYPE_LOCAL_ABS:
              fprintf (F, "(l_l_abs %s)", operand->value.s);
              break;
            case L_CTYPE_LOCAL_GP:
              fprintf (F, "(l_l_gp %s)", operand->value.s);
              break;
            case L_CTYPE_GLOBAL_ABS:
              fprintf (F, "(l_g_abs %s)", operand->value.s);
              break;
            case L_CTYPE_GLOBAL_GP:
              fprintf (F, "(l_g_gp %s)", operand->value.s);
              break;
            }
        }
      break;
    case L_OPERAND_RREGISTER:
      if (!L_is_ctype_predicate (operand))
        {
          if (L_output_obsolete_ctype_format)
            fprintf (F, "(rr %d %s)", operand->value.r,
                     L_ctype_name (L_return_old_ctype (operand)));
          else
            fprintf (F, "(rr %d %s)", operand->value.r,
                     L_ctype_name (operand->ctype));
        }
      else
        fprintf (F, "(rr %d %s)", operand->value.r,
                 L_ptype_name (operand->ptype));
      break;
    case L_OPERAND_EVR:
      if (!L_is_ctype_predicate (operand))
        {
          if (L_output_obsolete_ctype_format)
            fprintf (F, "(evr %d:%d %s)", operand->value.evr.num,
                     operand->value.evr.omega,
                     L_ctype_name (L_return_old_ctype (operand)));
          else
            fprintf (F, "(evr %d:%d %s)", operand->value.evr.num,
                     operand->value.evr.omega, L_ctype_name (operand->ctype));
        }
      else
        fprintf (F, "(evr %d:%d %s)", operand->value.evr.num,
                 operand->value.evr.omega, L_ptype_name (operand->ptype));
      break;
    default:
      fprintf (F, "(?)");
    }
}


/*
 *      if flag = 1, print out floats with lots of accuracy, otherwise
 *      just print out 3 digits for things like attributes which dont
 *      require so much accuracy.
 *      Inserted by MCM on behalf of P. Eaton for Lvisualize.
 */
void
L_print_operand_buffer (char *buf, L_Operand * operand, int flag)
{
  char tmp_buf[128];
  int i = 0;
  int j = 0;

  if (operand == NULL)
    {
      sprintf (buf, "()");
      return;
    }
  switch (operand->type)
    {
    case L_OPERAND_CB:
      sprintf (tmp_buf, "(cb %d)", operand->value.cb->id);
      break;
    case L_OPERAND_IMMED:
      if (L_is_ctype_integer (operand))
        {
          if (L_output_obsolete_ctype_format)
            sprintf (tmp_buf, "(i %d)", (int) operand->value.i);
          else
            {
              switch (operand->ctype)
                {
                case L_CTYPE_CHAR:
                  sprintf (tmp_buf, "(c %d)", (int) operand->value.i);
                  break;
                case L_CTYPE_UCHAR:
                  sprintf (tmp_buf, "(uc %d)", (int) operand->value.i);
                  break;
                case L_CTYPE_SHORT:
                  sprintf (tmp_buf, "(sh %d)", (int) operand->value.i);
                  break;
                case L_CTYPE_USHORT:
                  sprintf (tmp_buf, "(ush %d)", (int) operand->value.i);
                  break;
                case L_CTYPE_INT:
                  sprintf (tmp_buf, "(i %d)", (int) operand->value.i);
                  break;
                case L_CTYPE_UINT:
                  sprintf (tmp_buf, "(ui %d)", (int) operand->value.i);
                  break;
                case L_CTYPE_LONG:
                  sprintf (tmp_buf, "(lng %d)", (int) operand->value.i);
                  break;
                case L_CTYPE_ULONG:
                  sprintf (tmp_buf, "(ulng %d)", (int) operand->value.i);
                  break;
                case L_CTYPE_LLONG:
                  sprintf (tmp_buf, "(ll %d)", (int) operand->value.i);
                  break;
                case L_CTYPE_ULLONG:
                  sprintf (tmp_buf, "(ull %d)", (int) operand->value.i);
                  break;
                case L_CTYPE_LLLONG:
                  sprintf (tmp_buf, "(lll %d)", (int) operand->value.i);
                  break;
                case L_CTYPE_ULLLONG:
                  sprintf (tmp_buf, "(ulll %d)", (int) operand->value.i);
                  break;
                case L_CTYPE_POINTER:
                  sprintf (tmp_buf, "(pnt %d)", (int) operand->value.i);
                  break;
                default:
                  L_warn ("L_print_operand_buffer : Illegal type %d",
                          operand->ctype);
                  break;
                }
            }
        }
      if (L_is_ctype_flt (operand))
        {
          if (flag)
            sprintf (tmp_buf, "(f %.8e)", operand->value.f);
          else
            sprintf (tmp_buf, "(f %.3g)", operand->value.f);
        }
      if (L_is_ctype_dbl (operand))
        {
          if (flag)
            sprintf (tmp_buf, "(f2 %.16e)", operand->value.f2);
          else
            sprintf (tmp_buf, "(f2 %.3g)", operand->value.f2);
        }
      break;
    case L_OPERAND_STRING:
      if (L_output_obsolete_ctype_format)
        sprintf (tmp_buf, "(s %s)", operand->value.s);
      else
        {
          switch (operand->ctype)
            {
            case L_CTYPE_LOCAL_ABS:
              sprintf (tmp_buf, "(s_l_abs %s)", operand->value.s);
              break;
            case L_CTYPE_LOCAL_GP:
              sprintf (tmp_buf, "(s_l_gp %s)", operand->value.s);
              break;
            case L_CTYPE_GLOBAL_ABS:
              sprintf (tmp_buf, "(s_g_abs %s)", operand->value.s);
              break;
            case L_CTYPE_GLOBAL_GP:
              sprintf (tmp_buf, "(s_g_gp %s)", operand->value.s);
              break;
            }
        }
      break;
    case L_OPERAND_MACRO:
      if (!L_is_ctype_predicate (operand))
        sprintf (tmp_buf, "(mac %s %s)", L_macro_name (operand->value.mac),
                 L_ctype_name (operand->ctype));
      else
        sprintf (tmp_buf, "(mac %s %s)", L_macro_name (operand->value.mac),
                 L_ptype_name (operand->ptype));
      break;
    case L_OPERAND_REGISTER:
      if (!L_is_ctype_predicate (operand))
        {
          if (L_output_obsolete_ctype_format)
            sprintf (tmp_buf, "(r %d %s)", operand->value.r,
                     L_ctype_name (L_return_old_ctype (operand)));
          else
            sprintf (tmp_buf, "(r %d %s)", operand->value.r,
                     L_ctype_name (operand->ctype));
        }
      else
        sprintf (tmp_buf, "(r %d %s)", operand->value.r,
                 L_ptype_name (operand->ptype));
      break;
    case L_OPERAND_LABEL:
      if (L_output_obsolete_ctype_format)
        sprintf (tmp_buf, "(l %s)", operand->value.s);
      else
        {
          switch (operand->ctype)
            {
            case L_CTYPE_LOCAL_ABS:
              sprintf (tmp_buf, "(l_l_abs %s)", operand->value.s);
              break;
            case L_CTYPE_LOCAL_GP:
              sprintf (tmp_buf, "(l_l_gp %s)", operand->value.s);
              break;
            case L_CTYPE_GLOBAL_ABS:
              sprintf (tmp_buf, "(l_g_abs %s)", operand->value.s);
              break;
            case L_CTYPE_GLOBAL_GP:
              sprintf (tmp_buf, "(l_g_gp %s)", operand->value.s);
              break;
            }
        }
      break;
    case L_OPERAND_RREGISTER:
      if (!L_is_ctype_predicate (operand))
        {
          if (L_output_obsolete_ctype_format)
            sprintf (tmp_buf, "(rr %d %s)", operand->value.r,
                     L_ctype_name (L_return_old_ctype (operand)));
          else
            sprintf (tmp_buf, "(rr %d %s)", operand->value.r,
                     L_ctype_name (operand->ctype));
        }
      else
        sprintf (tmp_buf, "(rr %d %s)", operand->value.r,
                 L_ptype_name (operand->ptype));
      break;
    case L_OPERAND_EVR:
      if (!L_is_ctype_predicate (operand))
        {
          if (L_output_obsolete_ctype_format)
            sprintf (tmp_buf, "(evr %d:%d %s)", operand->value.evr.num,
                     operand->value.evr.omega,
                     L_ctype_name (L_return_old_ctype (operand)));
          else
            sprintf (tmp_buf, "(evr %d:%d %s)", operand->value.evr.num,
                     operand->value.evr.omega, L_ctype_name (operand->ctype));
        }
      else
        sprintf (tmp_buf, "(evr %d:%d %s)", operand->value.evr.num,
                 operand->value.evr.omega, L_ptype_name (operand->ptype));
      break;
    default:
      sprintf (tmp_buf, "(?)");
    }

  /*  escape any quotes  */
  while (tmp_buf[i] != '\0')
    {
      if (tmp_buf[i] == '\n')
        {
          i++;
          continue;
        }
      if (tmp_buf[i] == '"')
        buf[j++] = '\\';
      buf[j++] = tmp_buf[i++];
    }
  buf[j] = '\0';
}


/*============================================================
 *
 *  Oper input/output routines
 *
 *============================================================*/
void
L_read_oper_flags (L_Oper * oper, L_Input_Buf * input_buf)
{
  int i;
  char *flags;
  int num_flags;

  /* Assumes the token next token will be the flag string */
  flags = L_get_next_lcode_token (input_buf);
  num_flags = strlen (flags);

  for (i = 0; i < num_flags; i++)
    {
      switch (flags[i])
        {
        case 'C':
        case 'c':
          oper->flags = L_SET_BIT_FLAG (oper->flags, L_OPER_CHECK);
          break;
        case 'F':
        case 'f':
          oper->flags = L_SET_BIT_FLAG (oper->flags, L_OPER_SAFE_PEI);
          break;
        case 'L':
        case 'l':
          oper->flags = L_SET_BIT_FLAG (oper->flags, L_OPER_LABEL_REFERENCE);
          break;
        case 'K':
        case 'k':
          oper->flags = L_SET_BIT_FLAG (oper->flags, L_OPER_STACK_REFERENCE);
          break;
        case 'T':
        case 't':
          oper->flags = L_SET_BIT_FLAG (oper->flags, L_OPER_ROTATE_REGISTERS);
          break;
        case 'P':
        case 'p':
          oper->flags = L_SET_BIT_FLAG (oper->flags, L_OPER_PROMOTED);
          break;
        case 'Q':
        case 'q':
          oper->flags = L_SET_BIT_FLAG (oper->flags, L_OPER_SQUASHING);
          break;
        case 'D':
        case 'd':
          oper->flags = L_SET_BIT_FLAG (oper->flags, L_OPER_DATA_SPECULATIVE);
          break;
        case 'R':
        case 'r':
          oper->flags = L_SET_BIT_FLAG (oper->flags, L_OPER_SPILL_CODE);
          break;
        case 'E':
        case 'e':
          oper->flags = L_SET_BIT_FLAG (oper->flags, L_OPER_SIDE_EFFECT_FREE);
          break;
        case 'S':
        case 's':
          oper->flags = L_SET_BIT_FLAG (oper->flags, L_OPER_SPECULATIVE);
          break;
        case 'M':
        case 'm':
          oper->flags = L_SET_BIT_FLAG (oper->flags, L_OPER_MASK_PE);
          break;
        case 'N':
        case 'n':
          oper->flags = L_SET_BIT_FLAG (oper->flags, L_OPER_NO_SPECULATION);
          break;
        case 'A':
        case 'a':
          oper->flags = L_SET_BIT_FLAG (oper->flags,
                                        L_OPER_SUPER_SPECULATION);
          break;
        case 'X':
        case 'x':
          oper->flags = L_SET_BIT_FLAG (oper->flags, L_OPER_PROBE_MARK);
          break;
        case 'Y':
        case 'y':
          oper->flags = L_SET_BIT_FLAG (oper->flags, L_OPER_SYNC);
          break;
        case '?':
          oper->flags =
            L_SET_BIT_FLAG (oper->flags, L_OPER_PROCESSOR_SPECIFIC);
          break;
        case 'V':
        case 'v':
          oper->flags = L_SET_BIT_FLAG (oper->flags, L_OPER_VOLATILE);
          break;
        default:
          L_print_buf_with_arrow (stderr, input_buf);
          L_punt ("L_read_oper_flags: invalid flag <%c> in current oper.",
                  flags[i]);
        }
    }
  L_get_next_lcode_token_expecting (input_buf, ">", "L_read_oper_flags");
}

/*
 * Read new Lcode format: (example)
 *
 *     id  op flags    pred      dest        sources         attributes
 * (op 12 add <AFS> <(r 1 p)> [(r 2 i)] [(r 3 i)(r 4 i)] 
 *   {(int charchar int int)...} <(issue 4)(i_t 3)>)
 *
 * <> represents optional fields
 * [] represents standard fields
 *
 * Destination, source, predicate and attribute fields only have parenthesis
 * for operands which are present.
 *
 */
static void
L_read_rest_oper (L_Func * fn, L_Input_Buf * input_buf, L_Oper * oper)
{
  int i;
  char *opcode;
  char ch, *token;
  L_Attr *attr;

  /*
   *  read Opcode.
   */
  opcode = L_get_next_lcode_token (input_buf);

  oper->opc = L_opcode_id (opcode);
  if (oper->opc == -1)
    {
      L_print_buf_with_arrow (stderr, input_buf);
      L_punt ("L_read_rest_oper: Current opcode is undefined");
    }

  /* Opcode completers */

  if (L_opc_vestigial (oper->opc))
    {
      L_convert_to_com (oper);
    }
  else
    {
      ch = L_peek_next_char(input_buf);

      if ((ch != '<') && (ch != '['))
	{
	  opcode = L_get_next_lcode_token (input_buf);
	  oper->com[0] = L_ctype_id (opcode);

	  ch = L_peek_next_char(input_buf);

	  if ((ch != '<') && (ch != '['))
	    {
	      opcode = L_get_next_lcode_token (input_buf);
	      oper->com[1] = L_cmp_compl_id (opcode);
	    }
	}

      if ((L_general_pred_comparison_opcode (oper) ||
	   L_cond_branch_opcode (oper) || L_general_comparison_opcode (oper))
	  && (!oper->com[0] || !oper->com[1]))
	L_punt("L_read_rest_oper: completer expected");
    }

  /* We need a permanent pointer to the opcode string. */
  /* This used to be stored in a name table created by */
  /* the lexer, however that is no longer available.   */
  /* So we just happened to have this function instead */
  oper->opcode = L_opcode_name (oper->opc);

  /*
   * Read flags and/or predicates - optional
   */
  token = L_get_next_lcode_token (input_buf);
  if (token && *token == '<')
    {
      ch = L_peek_next_char (input_buf);

      /*
       * Read oper flags if there are any <xxxx>
       */
      if (ch != '(')
        {
          /* Read in oper flags */
          L_read_oper_flags (oper, input_buf);

          if ((ch = L_peek_next_char (input_buf)) == '<')
            {
              token = L_get_next_lcode_token (input_buf);
              ch = L_peek_next_char (input_buf);
            }
        }
      /*
       * Read oper predicates if there are any <()()()>
       */
      if (ch == '(')
        {
          for (i = 0; (i < L_max_pred_operand) && (ch != '>'); i++)
            {
              oper->pred[i] = L_read_operand (fn, input_buf);
              ch = L_peek_next_char (input_buf);
            }
          L_get_next_lcode_token_expecting (input_buf, ">", "L_read_oper");
        }
      /* 
       * After reading any flags and/or predicates we
       * better get a '[' to start reading destination operands
       */
      L_get_next_lcode_token_expecting (input_buf, "[", "L_read_oper");
    }
  else
    {
      if (*token != '[')
        {
          L_print_buf_with_arrow (stderr, input_buf);
          L_punt ("L_read_oper: Expecting token '[' but found '%c'.", *token);
        }
    }


  /*
   * read destination operands
   */
  ch = L_peek_next_char (input_buf);
  for (i = 0; (i < L_max_dest_operand) && (ch != ']'); i++)
    {
      oper->dest[i] = L_read_operand (fn, input_buf);
      ch = L_peek_next_char (input_buf);
    }

  L_get_next_lcode_token_expecting (input_buf, "]", "L_read_oper");
  /*
   * After reading destinations we better get
   * a '[' to start reading source operands
   */
  L_get_next_lcode_token_expecting (input_buf, "[", "L_read_oper");

  /*
   *  read source operands.
   */
  ch = L_peek_next_char (input_buf);
  for (i = 0; (i <= L_max_src_operand) && (ch != ']'); i++)
    {
      oper->src[i] = L_read_operand (fn, input_buf);
      ch = L_peek_next_char (input_buf);
    }

  L_get_next_lcode_token_expecting (input_buf, "]", "L_read_oper");

  ch = L_peek_next_char (input_buf);
  /*
   *  read optional sync arcs.
   */
  if (ch == '{')
    {
      L_read_sync_arcs (input_buf, oper);
      ch = L_peek_next_char (input_buf);
    }

  /*
   *  read attribute fields.
   */
  if (ch == '<')
    {
      L_get_next_lcode_token (input_buf);       /* Throw away '>' */
      while (ch != '>')
        {
          attr = L_read_attr (fn, input_buf);
          oper->attr = L_concat_attr (oper->attr, attr);

          if ((strcmp (attr->name, "SPECID") == 0) &&
              fn->max_spec_id <= attr->field[0]->value.i)
            fn->max_spec_id = (attr->field[0]->value.i + 1);

          ch = L_peek_next_char (input_buf);
        }
      L_get_next_lcode_token (input_buf);       /* Throw away '>' */
    }

  /* 
   * Better get a ')' to terminate the oper 
   */
  L_get_next_lcode_token_expecting (input_buf, ")", "L_read_oper");

  /*
   * Search for any processor specific opcode.  If one exists, it
   * will be set as the proc_opc.  Otherwise the current opc will
   * be set as the proc_opc
   */
  /* 09/23/02 REK Updating to read the completers field along with the
   *              proc_opc one. */
  if ((attr = L_find_attr (oper->attr, "popc")))
    {
      oper->proc_opc = (int) attr->field[0]->value.i;
      oper->completers = (int) attr->field[1]->value.i;
    }
  else
    {
      oper->proc_opc = oper->opc;
    }
}

L_Oper *
L_read_oper (L_Func * fn, L_Input_Buf * input_buf)
{
  char *token;
  int id;
  L_Oper *oper;


  /*
   * Read oper id 
   */
  token = L_get_next_lcode_token (input_buf);
  if (L_strtol (token, NULL, &id) == 0)
    {
      L_print_buf_with_arrow (stderr, input_buf);
      L_punt ("L_read_cb:  The oper id is not an integer value.");
    }

  /* DMG - 7/94 - add tbl lookup while reading */
  oper = L_oper_hash_tbl_find_and_alloc_oper (fn->oper_hash_tbl, id);

  /* if opc is set, then a full read of this oper */
  /* id has been done before */
  if (oper->opc != 0)
    {
      L_print_buf_with_arrow (stderr, input_buf);
      L_punt ("L_read_oper: duplicate oper id encountered");
    }

  L_read_rest_oper (fn, input_buf, oper);
  return (oper);
}


L_Oper *
L_read_parent_oper (L_Func * fn, L_Input_Buf * input_buf)
{
  char *token;
  int id;
  L_Oper *oper;

  /*
   * Read oper id 
   */
  token = L_get_next_lcode_token (input_buf);
  if (L_strtol (token, NULL, &id) == 0)
    {
      L_print_buf_with_arrow (stderr, input_buf);
      L_punt ("L_read_cb:  The parent oper id is not an integer value.");
    }
  oper = L_new_parent_oper (id);

  L_read_rest_oper (fn, input_buf, oper);
  return (oper);
}

static void
L_print_rest_oper (FILE * F, L_Oper * oper)
{
  L_Attr *attr;
  int i, num, flags, flag_count;
  static char flag_string[33];

  /* Print completers */

  if (oper->com[0])
    fprintf(F, "%s ", L_ctype_name (oper->com[0]));
  if (oper->com[1])
    fprintf(F, "%s ", L_cmp_compl_name (oper->com[1]));

  /*
   *  Print flags - flags are optional
   *
   *  Remember to clear out any flags that should not print!
   */
  flags = oper->flags;
  flags = L_CLR_BIT_FLAG (flags, L_OPER_HIDDEN_FLAGS);

  flag_count = L_oper_flags_to_string(flag_string, flags);

  if (flag_count)
    fprintf(F,"<%s>",flag_string);

  /*
   *  Print predicate operands - predicates are optional
   */
  num = L_max_pred_operand;
  for (i = L_max_pred_operand - 1; i >= 0; i--)
    {
      if (oper->pred[i] != NULL)
        break;
      num--;
    }
  if (num != 0)
    {
      fprintf (F, "<");
      for (i = 0; i < num; i++)
        L_print_operand (F, oper->pred[i], 1);
      fprintf (F, "> [");
    }
  else
    fprintf (F, "[");

  /*
   *  Print dest operands
   */
  num = L_max_dest_operand;
  for (i = L_max_dest_operand - 1; i >= 0; i--)
    {
      if (oper->dest[i] != NULL)
        break;
      num--;
    }
  for (i = 0; i < num; i++)
    L_print_operand (F, oper->dest[i], 1);
  fprintf (F, "] [");

  /*
   *  Print src operands
   */
  num = L_max_src_operand;
  for (i = L_max_src_operand - 1; i >= 0; i--)
    {
      if (oper->src[i] != NULL)
        break;
      num--;
    }
  for (i = 0; i < num; i++)
    L_print_operand (F, oper->src[i], 1);
  fprintf (F, "]");

  /*
   * Print the sync arcs
   */

  if (!L_use_appendix_for_sync_arcs &&
      (oper->acc_info ||
       (oper->sync_info && oper->sync_info->num_sync_in > 0)))
    {
      fprintf (F, " {");

      if (oper->acc_info)
	{
	  L_print_acc_info_list (F, oper->acc_info);
	}

      if ((oper->sync_info != NULL) && (oper->sync_info->num_sync_in > 0))
        {
          /* if there are a lot of sync arcs, print them 4 per line */
          if (oper->sync_info->num_sync_in > 2)
            {
              for (i = 0; i < oper->sync_info->num_sync_in; i++)
                {
                  if ((i % 5) == 0)
                    fprintf (F, "\n\t");
                  L_print_sync (F, oper->sync_info->sync_in[i]);
                }
            }
          else
            {
              for (i = 0; i < oper->sync_info->num_sync_in; i++)
                L_print_sync (F, oper->sync_info->sync_in[i]);
            }
        }
      fprintf (F, "}");
    }

  /*
   * Search for any processor specific opcode.  If one exists, it
   * will be set as the proc_opc.  Otherwise the current opc will
   * be set as the proc_opc
   */
  /* 09/23/02 REK Updating to write the value of the completers field along
   *              with the value of the proc_opc field. */
  attr = L_find_attr (oper->attr, "popc");
  if (attr != NULL)
    {
      if (oper->opc != oper->proc_opc)
      {
        L_set_int_attr_field (attr, 0, oper->proc_opc);
	L_set_int_attr_field (attr, 1, oper->completers);
      }
      else
      {
        oper->attr = L_delete_attr (oper->attr, attr);
      }
    }
  else
    {
      if (oper->opc != oper->proc_opc)
        {
          attr = L_new_attr ("popc", 2);
          L_set_int_attr_field (attr, 0, oper->proc_opc);
	  L_set_int_attr_field (attr, 1, oper->completers);
          oper->attr = L_concat_attr (oper->attr, attr);
        }
    }

  /*
   *  Print the attributes
   */
  if (oper->attr != NULL)
    {
      int attr_printed = 0;
      int use_appendix = L_use_appendix_for_attr;

      for (attr = oper->attr; attr != NULL; attr = attr->next_attr)
        {
          if (use_appendix == 0 || L_should_go_in_oper_appendix (attr) == 0)
            {
              if (attr_printed == 0)
                {
                  fprintf (F, " <");
                  attr_printed = 1;
                }
              L_print_attr (F, attr);
            }
        }
      if (attr_printed == 1)
        fprintf (F, ">");
    }

  fprintf (F, ")\n");
}

void
L_print_oper (FILE * F, L_Oper * oper)
{
  if (oper == NULL)
    return;

  /* New predicate define structure with completers */

  if (L_EXTRACT_BIT_VAL (oper->flags, L_OPER_PARENT))
    fprintf (F, "(OP %d %s ", oper->id, oper->opcode);
  else
    fprintf (F, "(op %d %s ", oper->id, oper->opcode);

  L_print_rest_oper (F, oper);
}


/*============================================================
 *
 *  Expression input routines
 *
 *============================================================*/

void
L_print_expression (FILE * F, L_Expression * expression)
{
  int i;

  if (expression == NULL)
    return;

  fprintf (F, "(EXPRESSION %d, token %d, reg %d, %s ", expression->index,
           expression->token, expression->reg_id, L_opcode_name(expression->opc));

  /* Print dest operands */
  if (expression->dest)
    {
      fprintf (F, "[");
      for (i = 0; i < L_max_dest_operand; i++)
        {
          if (expression->dest[i] == NULL)
	    break;
          L_print_operand (F, expression->dest[i], 1);
        }
      fprintf (F, "] ");
    }

  /* Print completers */

  if (expression->com[0])
    fprintf(F, "%s ", L_ctype_name (expression->com[0]));
  if (expression->com[1])
    fprintf(F, "%s ", L_cmp_compl_name (expression->com[1]));

  /*
   *  Print src operands
   */
  fprintf (F, " [");
  for (i = 0; i < L_max_src_operand; i++)
    {
      if (expression->src[i] == NULL)
        break;
      L_print_operand (F, expression->src[i], 1);
    }
  fprintf (F, "]");
  fprintf (F, ")\n");

  return;
}

/*============================================================
 *
 *  CB input routines
 *
 *============================================================*/

static L_Cb *
L_read_cb (L_Func * fn, L_Region * region, L_Input_Buf * input_buf)
{
  char *flags, ch;
  char *token;
  int i, id, num_flags;
  double weight;
  L_Cb *cb;

  /*
   * Read cb id 
   */
  token = L_get_next_lcode_token (input_buf);
  if (L_strtol (token, NULL, &id) == 0)
    {
      L_print_buf_with_arrow (stderr, input_buf);
      L_punt ("L_read_cb:  The cb id is not an integer value.");
    }

  if (id < 0)
    {
      L_print_buf_with_arrow (stderr, input_buf);
      L_punt ("L_read_cb: The id of the current cb is negative, "
              "this is illegal.");
    }

  /* find if already allocated cb, else allocate new one */
  cb = L_cb_hash_tbl_find_and_alloc (fn->cb_hash_tbl, id);


  /* check if cb already in Lcode with this id */
  if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_PRESENCE) == 1)
    {
      L_print_buf_with_arrow (stderr, input_buf);
      L_punt ("L_read_cb: A cb with the current id already exists "
              "within the function.");
    }
  cb->flags = L_SET_BIT_FLAG (cb->flags, L_CB_PRESENCE);

  /*
   * Read cb weight
   */
  token = L_get_next_lcode_token (input_buf);
  if (L_strtod (token, NULL, &weight) == 0)
    {
      L_print_buf_with_arrow (stderr, input_buf);
      L_punt ("L_read_cb:  The cb weight is not a floating-point value.");
    }
  cb->weight = weight;

  ch = L_peek_next_char (input_buf);

  if (ch == '<')
    {
      /* 
       *  Read flags - optional
       */
      L_get_next_lcode_token_expecting (input_buf, "<", "L_read_cb");

      flags = L_get_next_lcode_token (input_buf);
      num_flags = strlen (flags);

      for (i = 0; i < num_flags; i++)
        {
          switch (flags[i])
            {
              /* Rememeber to include lower and upper case! */
            case 'E':
            case 'e':
              cb->flags = L_SET_BIT_FLAG (cb->flags, L_CB_ENTRANCE_BOUNDARY);
              break;
            case 'H':
            case 'h':
              cb->flags = L_SET_BIT_FLAG (cb->flags, L_CB_HYPERBLOCK);
              break;
            case 'I':
            case 'i':
              cb->flags = L_SET_BIT_FLAG (cb->flags, L_CB_EPILOGUE);
              break;
            case 'N':
            case 'n':
              cb->flags = L_SET_BIT_FLAG (cb->flags,
                                          L_CB_HYPERBLOCK_NO_FALLTHRU);
              break;
            case 'P':
            case 'p':
              cb->flags = L_SET_BIT_FLAG (cb->flags, L_CB_SOFTPIPE);
              break;
            case 'R':
            case 'r':
              cb->flags = L_SET_BIT_FLAG (cb->flags, L_CB_PROLOGUE);
              break;
            case 'S':
            case 's':
              cb->flags = L_SET_BIT_FLAG (cb->flags, L_CB_SUPERBLOCK);
              break;
            case 'T':
            case 't':
              cb->flags = L_SET_BIT_FLAG (cb->flags, L_CB_ROT_REG_ALLOCATED);
              break;
            case 'U':
            case 'u':
              cb->flags = L_SET_BIT_FLAG (cb->flags, L_CB_UNROLLED);
              break;
            case 'V':
            case 'v':
              cb->flags = L_SET_BIT_FLAG (cb->flags,
                                          L_CB_VIOLATES_LC_SEMANTICS);
              break;
            case 'X':
            case 'x':
              cb->flags = L_SET_BIT_FLAG (cb->flags, L_CB_EXIT_BOUNDARY);
              break;
            default:
              L_punt ("L_read_cb: invalid flag in cb %d of <%c>",
                      id, flags[i]);
            }
        }
      L_get_next_lcode_token_expecting (input_buf, ">", "L_read_cb");
    }


  /*
   *  Read flow arcs
   */
  L_get_next_lcode_token_expecting (input_buf, "[", "L_read_cb");

  ch = L_peek_next_char (input_buf);
  while (ch != ']')
    {
      L_Flow *flow = L_read_flow (fn, cb, input_buf);
      cb->dest_flow = L_concat_flow (cb->dest_flow, flow);

      ch = L_peek_next_char (input_buf);
    }

  L_get_next_lcode_token_expecting (input_buf, "]", "L_read_cb");

  /*
   *  Read in cb attributes
   */
  if (L_peek_next_char (input_buf) == '<')
    {
      L_get_next_lcode_token (input_buf);       /* Throw away '<' */
      do
        {
          L_Attr *attr = L_read_attr (fn, input_buf);
          cb->attr = L_concat_attr (cb->attr, attr);
        }
      while (L_peek_next_char (input_buf) != '>');

      L_get_next_lcode_token (input_buf);       /* Throw away '>' */
    }

  /*
   * REH 4/20/95 Point cb to its containing region 
   */
  if (region != NULL)
    {
      L_add_cb_to_region (region, cb);
    }

  L_get_next_lcode_token_expecting (input_buf, ")", "L_read_cb");

  return (cb);
}

void
L_print_cb (FILE * F, L_Func * fn, L_Cb * cb)
{
  L_Flow *flow;
  L_Oper *oper, *parent_op = 0;
  int num_flow, num_attr, flags;
  L_Attr *attr;
  static L_Region *prev_region = NULL;

  /*
   * REH 4/20/95 - Before printing any cb data, print the containing
   *               region data, only if called from L_print_func,
   *               which is true if L_print_all_regions == 1.
   */
  if (L_print_all_regions == 1)
    {
      if (cb->region == NULL)
        {
          /*
             if ( prev_region != NULL )
           */
          if (L_last_printed_region != NULL)
            L_print_region_end (F, prev_region);
          prev_region = NULL;
          L_last_printed_region = NULL;
#if 0
          L_last_printed_region = -1;
#endif
        }
      else
        {
          L_print_region (F, fn, cb->region);
          prev_region = cb->region;
        }
    }

  fprintf (F, "  (cb %d %f", cb->id, cb->weight);

  /*
   *  Print flags - flags are optional
   *
   *  Remember to clear out any flags that should not print!
   *
   *  There are no legal printing cb flags at this point
   */
  flags = cb->flags;
  flags = L_CLR_BIT_FLAG (flags, L_CB_HIDDEN_FLAGS);
  if (flags != 0)
    {
      fprintf (F, " <");
      if (L_EXTRACT_BIT_VAL (flags, L_CB_ENTRANCE_BOUNDARY))
        fprintf (F, "E");
      if (L_EXTRACT_BIT_VAL (flags, L_CB_HYPERBLOCK))
        fprintf (F, "H");
      if (L_EXTRACT_BIT_VAL (flags, L_CB_EPILOGUE))
        fprintf (F, "I");
      if (L_EXTRACT_BIT_VAL (flags, L_CB_HYPERBLOCK_NO_FALLTHRU))
        fprintf (F, "N");
      if (L_EXTRACT_BIT_VAL (flags, L_CB_SOFTPIPE))
        fprintf (F, "P");
      if (L_EXTRACT_BIT_VAL (flags, L_CB_PROLOGUE))
        fprintf (F, "R");
      if (L_EXTRACT_BIT_VAL (flags, L_CB_SUPERBLOCK))
        fprintf (F, "S");
      if (L_EXTRACT_BIT_VAL (flags, L_CB_ROT_REG_ALLOCATED))
        fprintf (F, "T");
      if (L_EXTRACT_BIT_VAL (flags, L_CB_UNROLLED))
        fprintf (F, "U");
      if (L_EXTRACT_BIT_VAL (flags, L_CB_VIOLATES_LC_SEMANTICS))
        fprintf (F, "V");
      if (L_EXTRACT_BIT_VAL (flags, L_CB_EXIT_BOUNDARY))
        fprintf (F, "X");
      fprintf (F, ">");
    }

  /*
   *  Print destination flow arcs.
   */
  num_flow = 0;
  for (flow = cb->dest_flow; flow != NULL; flow = flow->next_flow)
    num_flow += 1;
  if (num_flow > 2)
    {
      for (flow = cb->dest_flow; flow != NULL; flow = flow->next_flow)
        {
          if (flow == cb->dest_flow)
            fprintf (F, "\n\t[");
          else
            fprintf (F, "\n\t ");
          fprintf (F, "(flow %d %d %f)", flow->cc,
                   flow->dst_cb->id, flow->weight);
        }
    }
  else
    {
      fprintf (F, " [");
      for (flow = cb->dest_flow; flow != NULL; flow = flow->next_flow)
        fprintf (F, "(flow %d %d %f)", flow->cc,
                 flow->dst_cb->id, flow->weight);
    }
  fprintf (F, "]");

  /*
   *  Print attributes.
   */
  num_attr = 0;
  for (attr = cb->attr; attr != NULL; attr = attr->next_attr)
    {
      if (L_use_appendix_for_attr == 0
          || L_should_go_in_cb_appendix (attr) == 0)
        num_attr += 1;
    }

  if (num_attr != 0)
    {
      int attr_printed = 0;
      int use_appendix = L_use_appendix_for_attr;

      if ((num_attr > 2) || (num_flow > 2))
        {
          for (attr = cb->attr; attr != NULL; attr = attr->next_attr)
            {
              if (use_appendix == 0 || L_should_go_in_cb_appendix (attr) == 0)
                {
                  if (attr_printed == 0)
                    {
                      fprintf (F, "\n\t<");
                      attr_printed = 1;
                    }
                  else
                    {
                      fprintf (F, "\n\t ");
                    }
                  L_print_attr (F, attr);
                }
            }
        }
      else
        {
          for (attr = cb->attr; attr != NULL; attr = attr->next_attr)
            {
              if (use_appendix == 0 || L_should_go_in_cb_appendix (attr) == 0)
                {
                  if (attr_printed == 0)
                    {
                      fprintf (F, " <");
                      attr_printed = 1;
                    }
                  L_print_attr (F, attr);
                }
            }
        }
      if (attr_printed == 1)
        fprintf (F, ">");
    }

  fprintf (F, ")\n");

  /*
   *  Print all opers.
   */
  for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
    {
      if (!L_print_parent_op)
        {
          if (L_EXTRACT_BIT_VAL (oper->flags, L_OPER_PRINT_CYCLE_DELIMITER))
            {
              putc ('\n', F);
            }
          fprintf (F, "    ");
          L_print_oper (F, oper);
        }
      else
        {
          if ((oper->parent_op != parent_op) && (oper->parent_op != NULL))
            {
              fprintf (F, "    ");
              L_print_oper (F, oper->parent_op);
              parent_op = oper->parent_op;
            }
          if (L_EXTRACT_BIT_VAL (oper->flags, L_OPER_PRINT_CYCLE_DELIMITER))
            {
              putc ('\n', F);
            }
          fprintf (F, "      ");
          L_print_oper (F, oper);
        }
    }
}

/*============================================================
 *
 *  Input/Output routines for an Lcode function
 *
 *============================================================*/
void
L_mark_leaf_func (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *oper;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
        {
          if (L_subroutine_call_opcode (oper))
            {
              fn->flags = L_CLR_BIT_FLAG (fn->flags, L_FUNC_LEAF);
              return;
            }
        }
    }

  fn->flags = L_SET_BIT_FLAG (fn->flags, L_FUNC_LEAF);
}

void
L_read_fn_flags (L_Func * fn, L_Input_Buf * input_buf)
{
  char *flags;
  int i, num_flags;

  /* Assumes the token next token will be the flag string */
  flags = L_get_next_lcode_token (input_buf);
  num_flags = strlen (flags);

  for (i = 0; i < num_flags; i++)
    {
      switch (flags[i])
        {
          /* Rememeber to include lower and upper case! */
        case 'C':
        case 'c':
          fn->flags = L_SET_BIT_FLAG (fn->flags, L_FUNC_COMPILATION_COMPLETE);
          break;
        case 'D':
        case 'd':
          fn->flags = L_SET_BIT_FLAG (fn->flags, L_FUNC_SCHEDULED);
          break;
        case 'E':
        case 'e':
          fn->flags = L_SET_BIT_FLAG (fn->flags, L_FUNC_SIDE_EFFECT_FREE);
          break;
        case 'H':
        case 'h':
          fn->flags = L_SET_BIT_FLAG (fn->flags, L_FUNC_HYPERBLOCK);
          break;
        case 'L':
        case 'l':
          fn->flags = L_SET_BIT_FLAG (fn->flags, L_FUNC_LEAF);
          break;
        case 'R':
        case 'r':
          fn->flags = L_SET_BIT_FLAG (fn->flags, L_FUNC_REGISTER_ALLOCATED);
          break;
        case 'S':
        case 's':
          fn->flags = L_SET_BIT_FLAG (fn->flags, L_FUNC_SUPERBLOCK);
          break;
        case 'A':
        case 'a':
          fn->flags = L_SET_BIT_FLAG (fn->flags, L_FUNC_PRED_REGS_IN_ATTR);
          break;
        case 'M':
        case 'm':
          fn->flags = L_SET_BIT_FLAG (fn->flags, L_FUNC_MASK_PE);
          break;
        case 'P':
        case 'p':
          fn->flags = L_SET_BIT_FLAG (fn->flags, L_FUNC_CC_IN_PREDICATE_REGS);
          break;
        case 'T':
        case 't':
          fn->flags = L_SET_BIT_FLAG (fn->flags, L_FUNC_ROT_REG_ALLOCATED);
          break;
        default:
          L_punt ("L_read_fn_flags: invalid flag in %s of <%c>",
                  fn->name, flags[i]);
        }
    }
  L_get_next_lcode_token_expecting (input_buf, ">", "L_read_fn_flags");
}

/*
 * IF YOU CHANGE THE BEHAVIOR OF THIS FUNCTION, YOU MUST ALTER
 * L_read_fn_attributes_binary() IN THE SAME MANNER!
 */
void
L_read_fn_attributes (L_Func * fn, L_Input_Buf * input_buf)
{
  L_Attr *attr, *next_attr;

  /* Read in each function attribute */
  do
    {
      attr = L_read_attr (L_fn, input_buf);

      if (L_eliminate_sync_arcs &&
          (!strcmp (attr->name, "DEP_PRAGMAS") ||
           !strcmp (attr->name, "JSR_DEP_PRAGMAS")))
        {
          L_delete_attr (NULL, attr);
          attr = NULL;
        }
      else
        {
          fn->attr = L_concat_attr (fn->attr, attr);
        }
    }
  while (L_peek_next_char (input_buf) != '>');

  L_get_next_lcode_token_expecting (input_buf, ">", "L_read_fn_attributes");
  /*
   * Initialize some attributes for validation
   */

  L_func_contains_dep_pragmas = 0;
  L_func_contains_jsr_dep_pragmas = 0;

  L_func_acc_specs = 0;
  L_func_acc_omega = 0;

  for (attr = L_fn->attr; attr != NULL; attr = next_attr)
    {
      next_attr = attr->next_attr;

      if (!strncmp (attr->name, "ARCH:", 5))
	{
	  L_file_arch = L_add_string (L_string_table, attr->name);
	}
      else if (!strncmp (attr->name, "MODEL:", 6))
	{
	  L_file_model = L_add_string (L_string_table, attr->name);
	}
      else if (!strncmp (attr->name, "LMDES:", 6))
	{
	  L_file_lmdes = L_add_string (L_string_table, attr->name);
	}
      else if (!strncmp (attr->name, "DEP_PRAGMAS", 11))
        {
          L_func_contains_dep_pragmas = 1;
          L_func_contains_jsr_dep_pragmas = 1;
        }
      else if (!L_ignore_acc_specs && !strcmp (attr->name, "ACC_SPECS"))
        {
          L_func_acc_specs = 1;
        }
      else if (!L_ignore_acc_specs && !strcmp (attr->name, "ACC_OMEGA"))
        {
          L_func_acc_omega = 1;
        }
      else if (!strncmp (attr->name, "JSR_DEP_PRAGMAS", 11))
	{
	  L_func_contains_jsr_dep_pragmas = 1;
	}
      else if (!strcmp (attr->name, "max_cb_id"))
	{
	  L_fn->max_cb_id = (int) attr->field[0]->value.i;
	}
    }
}

void
L_read_fn (L_Input_Buf * input_buf)
{
  char ch, *token, *name;
  int end_function;
  int type;
  double weight;
  L_Cb *cb = NULL;
  L_Oper *oper;
  L_Region *region = NULL;
  int num_jump_tbls;

  L_Time time;

  L_init_time (&time);
  L_start_time (&time);

  /*
   * This is used to prevent reporting STD_PARM errors
   * on files that only contain data segments.
   */
  L_func_read = 1;


  /* Get function name and weight */
  token = L_get_next_lcode_token (input_buf);
  L_fn = L_new_func (token, 0.0);

  token = L_get_next_lcode_token (input_buf);
  weight = atof (token);
  L_fn->weight = weight;

  /*
   * Read flags - optional
   */
  token = L_get_next_lcode_token (input_buf);
  if (token && *token == '<')
    {
      ch = L_peek_next_char (input_buf);

      /*
       * Read function flags if there are any <xxxx>
       */
      if (ch != '(')
        {
          /* Read in function flags */
          L_read_fn_flags (L_fn, input_buf);

          if ((ch = L_peek_next_char (input_buf)) == '<')
            {
              token = L_get_next_lcode_token (input_buf);
              ch = L_peek_next_char (input_buf);
            }
        }
      /*
       * Read function attributes if there are any <()()()>
       */
      if (ch == '(')
        {
          L_read_fn_attributes (L_fn, input_buf);
        }
      /* 
       * After reading any flags and/or attributes we
       * better get a ')' to terminate the function item
       */
      L_get_next_lcode_token_expecting (input_buf, ")", "L_read_fn");
    }
  else
    {
      if (*token != ')')
        {
          L_print_buf_with_arrow (stderr, input_buf);
          L_punt ("L_read_function: Expecting token ')' but found '%c'.",
                  *token);
        }
    }

  /*
   *  Read until we see the end of the function
   */
  /*
     L_get_next_lcode_token_expecting(input_buf,"(","L_read_fn");
   */

  end_function = 0;
  while (!end_function)
    {

      /*
       * Read the next line of the input stream
       */
      token = L_get_next_lcode_token (input_buf);
      while (token && ((*token == '\0') || (*token == '#')))
        {
          /* Skip "empty" line */
          if (!L_refill_input_buf (input_buf))
            {
              L_print_buf_with_arrow (stderr, input_buf);
              L_punt ("L_read_function: Unexpected end of file!\n");
            }
          token = L_get_next_lcode_token (input_buf);
        }

      if (*token != '(')
        {
          L_print_buf_with_arrow (stderr, input_buf);
          L_punt ("L_read_function: Expecting token '(' but found '%c'.",
                  *token);
        }

      name = L_get_next_lcode_token (input_buf);
      type = L_lcode_id (name);
      switch (type)
        {
        case L_INPUT_REGION:
          region = L_read_region (L_fn, input_buf);
          break;
        case L_INPUT_REGION_END:
          region = L_read_region_end (L_fn, input_buf);
          break;
        case L_INPUT_CB:
          cb = L_read_cb (L_fn, region, input_buf);
          L_insert_cb_after (L_fn, L_fn->last_cb, cb);
          break;
        case L_INPUT_OP:
          oper = L_read_oper (L_fn, input_buf);
          if (cb == NULL)
            {
              L_print_buf_with_arrow (stderr, input_buf);
              L_punt ("L_read_fn: There is no cb for this oper!");
            }
          /* Link to the currently defined parent Lcode oper */
          oper->parent_op = L_fn->last_parent_op;

          /* REH 7/8/95 - Set Cb prologue/epilogue flags */
          if (oper->opc == Lop_PROLOGUE)
            cb->flags = L_SET_BIT_FLAG (cb->flags, L_CB_PROLOGUE);
          else if (oper->opc == Lop_EPILOGUE)
            cb->flags = L_SET_BIT_FLAG (cb->flags, L_CB_EPILOGUE);

          L_insert_oper_after (cb, cb->last_op, oper);
          break;
        case L_INPUT_POP:
          oper = L_read_parent_oper (L_fn, input_buf);

          if (cb == NULL)
            {
              L_print_buf_with_arrow (stderr, input_buf);
              L_punt ("L_read_fn: There is no cb for this oper!");
            }

          /* Mark this oper as a parent */
          oper->flags = L_SET_BIT_FLAG (oper->flags, L_OPER_PARENT);

          /* 
           * Link the oper into the parent oper list to ensure correct
           * freeing of the oper when the function is released.
           */
          oper->next_op = L_fn->last_parent_op;
          L_fn->last_parent_op = oper;
          break;
        case L_INPUT_APPENDIX:
          L_read_appendix (L_fn, input_buf);
          break;
        case L_INPUT_END:
          L_get_next_lcode_token (input_buf);   /* Skip Function Name */
          L_get_next_lcode_token_expecting (input_buf, ")", "L_read_fn");
          end_function = 1;
          break;
        default:
          L_print_buf_with_arrow (stderr, input_buf);
          L_punt ("L_read_fn: Unknown lcode item in function.");
          break;
        }
#if 0
      if (!end_function)
        L_get_next_lcode_token_expecting (input_buf, "(", "L_read_fn");
#endif
    }
  /*
   *  Create matching src flow arcs.
   */
  for (cb = L_fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      L_Flow *ptr, *dup;
      for (ptr = cb->dest_flow; ptr != NULL; ptr = ptr->next_flow)
        {
          L_Cb *dst;
          /* correct neg weight here */
          dup = L_new_flow (ptr->cc, ptr->src_cb, ptr->dst_cb, ptr->weight);
          dst = ptr->dst_cb;
          dst->src_flow = L_concat_flow (dst->src_flow, dup);
        }
    }

  /*
   *  Initialize s_local, s_param, s_swap values in L_Func structure
   *  Values held in defines at top of function.
   */
  for (cb = L_fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
        {
          if (oper->opc == Lop_DEFINE)
            {
              L_Operand *op;
              int value;
              op = (oper->dest[0]);
              if (!L_is_null (op) && L_is_macro (op))
                {
                  switch (op->value.mac)
                    {
                    case L_MAC_LOCAL_SIZE:
                      if (!L_is_ctype_integer (oper->src[0]))
                        L_punt ("L_read_fn: "
                                "src1 of (define $_local) must be integer");
                      value = (int) oper->src[0]->value.i;
                      L_fn->s_local = value;
                      break;
                    case L_MAC_PARAM_SIZE:
                      if (!L_is_ctype_integer (oper->src[0]))
                        L_punt ("L_read_fn: "
                                "src1 of (define $_param) must be integer");
                      value = (int) oper->src[0]->value.i;
                      L_fn->s_param = value;
                      break;
                    case L_MAC_SWAP_SIZE:
                      if (!L_is_ctype_integer (oper->src[0]))
                        L_punt ("L_read_fn: "
                                "src1 of (define $_swap) must be integer");
                      value = (int) oper->src[0]->value.i;
                      L_fn->s_swap = value;
                      break;
                    default:
                      break;
                    }
                }
            }
        }
    }
  /* Determine if this function is a leaf function */
  L_mark_leaf_func (L_fn);

  /* Renaming is for compatibility of older Lcode files with new tbl naming */
  if (L_func_needs_jump_table_renaming (L_fn))
    L_rename_jump_table_labels (L_fn);
  if (!L_func_has_jump_table_info (L_fn))
    L_setup_jump_table_info (L_fn);
  num_jump_tbls = L_num_jump_tables (L_fn);
  if (num_jump_tbls > 0)
    L_fn->jump_tbls = L_read_all_hashtbls (input_buf, num_jump_tbls);

  L_stop_time (&time);
}

void
L_print_func (FILE * F, L_Func * fn)
{
  int num, flags;
  L_Attr *attr, *next_attr;
  L_Cb *cb;
  L_Region *region;
  L_Time time;
  double cpu_time;

  L_init_time (&time);
  L_start_time (&time);

  /* Now unconditionally measure time spent in each function -JCG 6/99 */
  L_stop_time (&L_module_execution_time);

  /* Print out message if cpu_time exceeds threshold for this function.
   * Set threshold to -1 to get no messages. -JCG 6/99
   */
  if (L_cpu_time_print_threshold >= 0.0)
    {
      /* Get cpu time (in seconds) and convert to minutes */
      cpu_time = L_final_time (&L_module_execution_time) / 60.0;

      /* Print message if passed threshold */
      if (cpu_time >= L_cpu_time_print_threshold)
        {
          /* Strip off leading _ in message so names can be used
           * directly in prevent_*_functions list. -JCG 6/99
           */
          fprintf (stderr, "%s: %.2f minutes in %s\n", &fn->name[1],
                   cpu_time, L_curr_pass_name);
        }
    }

  if (L_determine_cpu_time)
    {
      L_annotate_function_with_cpu_time (fn);
    }

  /* Start time again here to handle HtoL case where Lcode function
   * is not read in (so time keeps increasing).  For everything else,
   * this time will be reset when the function is read in. -JCG 6/99
   */
  L_init_time (&L_module_execution_time);


  /*
   *  Insert comments into output file
   */
  if ((!L_generation_info_printed) && (L_output_generation_info) &&
      (!L_output_binary_format))
    {
      L_insert_generic_info_to_output_file (F);
      L_generation_info_printed = 1;
    }

  /*
   * It is possible that the function has been converted from a
   * leaf to a non-leaf function.  Thus, we need to update the
   * function flag.
   */
  L_mark_leaf_func (L_fn);

  if (L_output_binary_format == 1)
    {
      if (L_binary_magic_number_emitted == 0)
        {
          putc (CURRENT_BINARY_VERSION, F);
          fprintf (F, "# Binary Lcode!\n%c", DELIMIT);
          L_binary_magic_number_emitted = 1;
        }
      L_print_func_binary (F, fn);
      return;
    }

  /*
   *  Print function header
   */
  fprintf (F, "(function %s %f", fn->name, fn->weight);

  /*
   *  Print flags - flags are optional
   *
   *  Remember to clear out any flags that should not print!
   */
  flags = fn->flags;
  flags = L_CLR_BIT_FLAG (flags, L_FUNC_HIDDEN_FLAGS);
  if (flags != 0)
    {
      fprintf (F, " <");
      if (L_EXTRACT_BIT_VAL (flags, L_FUNC_COMPILATION_COMPLETE))
        fprintf (F, "C");
      if (L_EXTRACT_BIT_VAL (flags, L_FUNC_SCHEDULED))
        fprintf (F, "D");
      if (L_EXTRACT_BIT_VAL (flags, L_FUNC_REGISTER_ALLOCATED))
        fprintf (F, "R");
      if (L_EXTRACT_BIT_VAL (flags, L_FUNC_SIDE_EFFECT_FREE))
        fprintf (F, "E");
      if (L_EXTRACT_BIT_VAL (flags, L_FUNC_HYPERBLOCK))
        fprintf (F, "H");
      if (L_EXTRACT_BIT_VAL (flags, L_FUNC_LEAF))
        fprintf (F, "L");
      if (L_EXTRACT_BIT_VAL (flags, L_FUNC_SUPERBLOCK))
        fprintf (F, "S");
      if (L_EXTRACT_BIT_VAL (flags, L_FUNC_PRED_REGS_IN_ATTR))
        fprintf (F, "A");
      if (L_EXTRACT_BIT_VAL (flags, L_FUNC_MASK_PE))
        fprintf (F, "M");
      if (L_EXTRACT_BIT_VAL (flags, L_FUNC_CC_IN_PREDICATE_REGS))
        fprintf (F, "P");
      if (L_EXTRACT_BIT_VAL (flags, L_FUNC_ROT_REG_ALLOCATED))
        fprintf (F, "T");
      fprintf (F, ">");
    }

  /* Print function attributes. */

  /*
   * First we delete any existing attribute for input and
   * output file name.
   */
  for (attr = fn->attr; attr != NULL; attr = next_attr)
    {
      next_attr = attr->next_attr;
      if (!strncmp (attr->name, "INFILE:", 7))
        fn->attr = L_delete_attr (fn->attr, attr);

      else if (!strncmp (attr->name, "OUTFILE:", 8))
        fn->attr = L_delete_attr (fn->attr, attr);

#if 0
      /* For now pass the original ARCH, MODEL, LMDES thru (SAM 5 - 94) */
      else if (!strncmp (attr->name, "ARCH:", 5))
        fn->attr = L_delete_attr (fn->attr, attr);

      else if (!strncmp (attr->name, "MODEL:", 6))
        fn->attr = L_delete_attr (fn->attr, attr);

      else if (!strncmp (attr->name, "LMDES:", 6))
        fn->attr = L_delete_attr (fn->attr, attr);
#endif
    }
  /*
   * Create attributes for input and output file name.
   */
#if 0 /* REH - removed as per SAM's request 11/7/94 */
  ptr = (char *) malloc (sizeof (char) * (strlen (L_input_file) + 8));
  sprintf (ptr, "INFILE:%s", L_input_file);
  temp = L_add_string (L_string_table, ptr);
  fn->attr = L_concat_attr (fn->attr, L_new_attr (temp, 0));
  free (ptr);

  ptr = (char *) malloc (sizeof (char) * (strlen (L_output_file) + 9));
  sprintf (ptr, "OUTFILE:%s", L_output_file);
  temp = L_add_string (L_string_table, ptr);
  fn->attr = L_concat_attr (fn->attr, L_new_attr (temp, 0));
  free (ptr);
#endif

#if 0
  Pass the arch, model and lmdes thru as normal attributes if (L_file_arch)
      fn->attr = L_concat_attr (fn->attr, L_new_attr (L_file_arch, 0));

  if (L_file_model)
    fn->attr = L_concat_attr (fn->attr, L_new_attr (L_file_model, 0));

  if (L_file_lmdes)
    fn->attr = L_concat_attr (fn->attr, L_new_attr (L_file_lmdes, 0));
#endif

  /* Need to regenerate the tables before printing attributes */
  if (L_jump_tables_have_changes (fn))
    L_regenerate_all_jump_tables (fn);

  num = 0;
  for (attr = fn->attr; attr != NULL; attr = attr->next_attr)
    {
      num += 1;
    }
  if (num > 2)
    {
      fprintf (F, " <");
      for (attr = fn->attr; attr != NULL; attr = attr->next_attr)
        {
          fprintf (F, "\n\t");
          L_print_attr (F, attr);
        }
      fprintf (F, ">");
    }
  else if (num != 0)
    {
      fprintf (F, " <");
      for (attr = fn->attr; attr != NULL; attr = attr->next_attr)
        {
          L_print_attr (F, attr);
        }
      fprintf (F, ">");
    }
  fprintf (F, ")\n");

  /*
   * Set up printing of regions
   */
  L_print_all_regions = 1;
  L_last_printed_region = NULL;

  /*
   *  Print all cbs
   */
  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      L_print_cb (F, fn, cb);
    }

  /*
   * Clear region presence flags
   */
  L_print_all_regions = 0;
  L_last_printed_region = NULL;
  for (region = fn->first_region; region != NULL;
       region = region->next_region)
    {
      region->flags = L_CLR_BIT_FLAG (region->flags, L_REGION_PRESENCE);
    }

  /*
   * Print the appendix
   */
  if ((L_use_appendix_for_attr) || (L_use_appendix_for_sync_arcs))
    L_print_appendix (F, fn);

  /*
   *  Print function trailer
   */
  fprintf (F, "(end %s)\n", fn->name);

  if (fn->jump_tbls)
    L_print_datalist (F, fn->jump_tbls);

  L_stop_time (&time);
}

void
L_print_oper_list (FILE * F, L_Oper_List * list)
{
  L_Oper_List *ptr;

  for (ptr = list; ptr != NULL; ptr = ptr->next_list)
    L_print_oper (F, ptr->oper);
}


/*============================================================
 *
 *  Input and return type of next lcode item in the file
 *
 *============================================================*/
int
L_get_input (void)
{
  char *token;
  char *keyword;
  int ch;
  int type = L_INPUT_EOF;
  L_Input_Buf *input_buf = &L_input_buf;

  if (!L_input_binary_format)
    {
      /*
       * Read the next line of the input stream
       */
      token = L_get_next_lcode_token (input_buf);
      while (token && ((*token == '\0') || (*token == '#')))
        {
          /* Skip "empty" line */
          if (!L_refill_input_buf (input_buf))
            return (L_INPUT_EOF);

          token = L_get_next_lcode_token (input_buf);
        }
      /*
         * Lcode specifies each Lcode input must begin with a '('
       */
      if (token)
        {
          switch (*token)
            {
            case '(':
              keyword = L_get_next_lcode_token (input_buf);
              type = L_lcode_id (keyword);
              switch (type)
                {
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
                case L_INPUT_RESERVE:
                case L_INPUT_GLOBAL:
                case L_INPUT_WB:
                case L_INPUT_WW:
                case L_INPUT_WI:
                case L_INPUT_WQ:
                case L_INPUT_WF:
                case L_INPUT_WF2:
                case L_INPUT_WS:
                case L_INPUT_ELEMENT_SIZE:
                  /* LCW - new tokens for preserving debugging info - 4/17/96 */
                case L_INPUT_DEF_STRUCT:
                case L_INPUT_DEF_UNION:
                case L_INPUT_DEF_ENUM:
                case L_INPUT_FIELD:
                case L_INPUT_ENUMERATOR:
                case L_INPUT_SKIP:
                  L_read_data (type, input_buf);

                  if (L_check)
		    L_check_data (L_data);
                  break;

                case L_INPUT_EVENT_LIST:
                  L_event_list = L_read_event_list (input_buf);
                  break;

                case L_INPUT_RESULT_LIST:
                  L_result_list = L_read_event_list (input_buf);
                  break;

                case L_INPUT_FUNCTION:
                  L_read_fn (input_buf);

                  /* Now unconditionally measure time -JCG 6/99 */
                  L_init_time (&L_module_execution_time);
                  L_start_time (&L_module_execution_time);
                  L_init_time (&L_module_global_dataflow_time);
                  L_init_time (&PG_bdd_build_time);
                  L_init_time (&PG_ssa_build_time);

                  if (L_check)
                    {
                      L_cb_hash_tbl_check (L_fn->cb_hash_tbl);
                      L_oper_hash_tbl_check (L_fn);
                      L_check_func (L_fn);
                    }
                  break;

                default:
                  L_print_buf_with_arrow (stderr, input_buf);
                  L_punt ("L_process_lcode_item: unknown Lcode item.\n");
                  break;
                }
              break;
            default:
              L_print_buf_with_arrow (stderr, input_buf);
              L_punt ("L_get_input: Unexpected character on line %d\n",
                      input_buf->line_count);
              break;
            }
        }
      else
        type = L_INPUT_EOF;
    }
  else
    {
      ch = L_binary_peek_next_char (L_IN);
      token = L_binary_read_string (L_IN, input_buf);
      if (ch != EOF && L_is_binary_magic_header (ch))
        {
          /* throw away the magic header */
          token = L_binary_read_string (L_IN, input_buf);
        }
      if (token && *token == '#')
        {
          /* throw away the comments at the top of the file */
          token = L_binary_read_string (L_IN, input_buf);
        }
      if (token)
        {
          /* Determine id of Lcode keyword */
          type = L_lcode_id (token);
          switch (type)
            {
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
            case L_INPUT_RESERVE:
            case L_INPUT_GLOBAL:
            case L_INPUT_WB:
            case L_INPUT_WW:
            case L_INPUT_WI:
            case L_INPUT_WQ:
            case L_INPUT_WF:
            case L_INPUT_WF2:
            case L_INPUT_WS:
            case L_INPUT_ELEMENT_SIZE:
              /* LCW - new tokens for preserving debugging info - 4/17/96 */
            case L_INPUT_DEF_STRUCT:
            case L_INPUT_DEF_UNION:
            case L_INPUT_DEF_ENUM:
            case L_INPUT_FIELD:
            case L_INPUT_ENUMERATOR:
            case L_INPUT_SKIP:
              L_read_data_binary (L_IN, type, input_buf);

              if (L_check)
		L_check_data (L_data);
              break;

            case L_INPUT_EVENT_LIST:
              L_event_list = L_read_event_list_binary (L_IN, input_buf);
              break;

            case L_INPUT_RESULT_LIST:
              L_result_list = L_read_event_list_binary (L_IN, input_buf);
              break;

            case L_INPUT_FUNCTION:
              L_read_fn_binary (L_IN, input_buf);

              /* Now unconditionally measure time -JCG 6/99 */
              L_init_time (&L_module_execution_time);
              L_start_time (&L_module_execution_time);
              L_init_time (&L_module_global_dataflow_time);
              L_init_time (&PG_bdd_build_time);
              L_init_time (&PG_ssa_build_time);

              if (L_check)
                {
                  L_cb_hash_tbl_check (L_fn->cb_hash_tbl);
                  L_oper_hash_tbl_check (L_fn);
                  L_check_func (L_fn);
                }
              break;

            default:
              L_punt
                ("L_get_input: unknown Lcode item in binary file -> %s\n",
                 token);
              break;
            }
        }
      else
	{
	  type = L_INPUT_EOF;
	}
    }
  L_token_type = type;
  return (type);
}

/* HCH 5/10/04: add to get global object id's through from Pipa
 * NOTE: bit_field put on to mimic L_read_type for struct fields;
 * currently only passing IDs for globals */
int
L_read_id (L_Input_Buf * input_buf, L_Expr * bit_field)
{
  char *token;
  int id = 0;
  
  L_get_next_lcode_token_expecting (input_buf, "(", "L_read_id");
  token = L_get_next_lcode_token (input_buf);

  if (!strcmp (token, "objid"))
    {
      if (L_peek_next_char (input_buf) != ')')
  	{ 
	  token = L_get_next_lcode_token (input_buf);
	  id = (int) strtol(token, NULL, 10);
	}
    }
  else 
    L_punt ("L_read_id: error parsing\n");
  
  L_get_next_lcode_token_expecting (input_buf, ")", "L_read_id");
  
  return id;
}

/* LCW - read type information into L_Type and bit field length - 4/18/96 */
L_Type *
L_read_type (L_Input_Buf * input_buf, L_Expr * bit_field)
{
  char *token, *str;
  L_Type *ltype;
  L_Dcltr *new_dcltr;
  int n;

  ltype = L_new_type ();
  while (L_peek_next_char (input_buf) != ')')
    {
      L_get_next_lcode_token_expecting (input_buf, "(", "L_read_type");
      token = L_get_next_lcode_token (input_buf);

      if (!strcmp (token, "const"))
        ltype->type |= L_DATA_CONST;
      else if (!strcmp (token, "volatile"))
        ltype->type |= L_DATA_VOLATILE;
      else if (!strcmp (token, "noalias"))
        ltype->type |= L_DATA_NOALIAS;
      else if (!strcmp (token, "register"))
        ltype->type |= L_DATA_REGISTER;
      else if (!strcmp (token, "auto"))
        ltype->type |= L_DATA_AUTO;
      else if (!strcmp (token, "static"))
        ltype->type |= L_DATA_STATIC;
      else if (!strcmp (token, "extern"))
        ltype->type |= L_DATA_EXTERN;
      else if (!strcmp (token, "global"))
        ltype->type |= L_DATA_GLOBAL;
      else if (!strcmp (token, "parameter"))
        ltype->type |= L_DATA_PARAMETER;
      else if (!strcmp (token, "void"))
        ltype->type |= L_DATA_VOID;
      else if (!strcmp (token, "char"))
        ltype->type |= L_DATA_CHAR;
      else if (!strcmp (token, "short"))
        ltype->type |= L_DATA_SHORT;
      else if (!strcmp (token, "int"))
        ltype->type |= L_DATA_INT;
      else if (!strcmp (token, "long"))
        ltype->type |= L_DATA_LONG;
      else if (!strcmp (token, "longlong"))
        ltype->type |= L_DATA_LONGLONG;
      else if (!strcmp (token, "float"))
        ltype->type |= L_DATA_FLOAT;
      else if (!strcmp (token, "double"))
        ltype->type |= L_DATA_DOUBLE;
      else if (!strcmp (token, "signed"))
        ltype->type |= L_DATA_SIGNED;
      else if (!strcmp (token, "unsigned"))
        ltype->type |= L_DATA_UNSIGNED;
      else if (!strcmp (token, "struct"))
        {
          ltype->type |= L_DATA_STRUCT;
          str = L_get_next_lcode_token (input_buf);
          ltype->struct_name = L_add_string (L_string_table, str);
        }
      else if (!strcmp (token, "union"))
        {
          ltype->type |= L_DATA_UNION;
          str = L_get_next_lcode_token (input_buf);
          ltype->struct_name = L_add_string (L_string_table, str);
        }
      else if (!strcmp (token, "enum"))
        {
          ltype->type |= L_DATA_ENUM;
          str = L_get_next_lcode_token (input_buf);
          ltype->struct_name = L_add_string (L_string_table, str);
        }
      else if (!strcmp (token, "a"))
        {
          new_dcltr = L_new_dcltr ();
          new_dcltr->method = L_D_ARRY;
          if (L_peek_next_char (input_buf) != ')')
            new_dcltr->index = L_read_expr (input_buf);
          ltype->dcltr = L_concat_dcltr (ltype->dcltr, new_dcltr);
        }
      else if (!strcmp (token, "p"))
        {
          new_dcltr = L_new_dcltr ();
          new_dcltr->method = L_D_PTR;
          ltype->dcltr = L_concat_dcltr (ltype->dcltr, new_dcltr);
        }
      else if (!strcmp (token, "f"))
        {
          new_dcltr = L_new_dcltr ();
          new_dcltr->method = L_D_FUNC;
          ltype->dcltr = L_concat_dcltr (ltype->dcltr, new_dcltr);
        }
      else if (!strcmp (token, "i"))
        {                       /* a bit field */
          if (bit_field == NULL)
            L_punt ("L_read_type: invalid bit-field.");
          is_bit_field = 1;
          token = L_get_next_lcode_token (input_buf);
          if (L_strtol (token, NULL, &n) == 0)
            L_punt ("L_read_type: not a valid integer.");
          bit_field->value.i = (ITintmax) n;
        }
      L_get_next_lcode_token_expecting (input_buf, ")", "L_read_type");
    }                           /* while */
  return ltype;
}



/*
 * DIA 9/96 - The following functions are for debugging only.  
 * Call them from your favorite debugger.
 */

void
DB_set (Set set)
{
  Set_print (stdout, "DEBUG ", set);
}

void
DB_spit_func (L_Func * fn, char *name)
{
  FILE *F;
  F = L_open_output_file (name);
  /* BCC - added by BCC */
  fprintf (F, "(ms text)\n");
  fprintf (F, "(global %s)\n", fn->name);
  L_print_func (F, fn);
  L_close_output_file (F);
}

void
DB_spit_cb (L_Cb * cb, char *name)
{
  int orig_L_print_all_regions = L_print_all_regions;
  FILE *F;

  F = L_open_output_file (name);
  L_print_all_regions = 0;
  L_print_cb (F, 0, cb);
  L_print_all_regions = orig_L_print_all_regions;
  L_close_output_file (F);
}

void
DB_spit_oper (L_Oper * oper, char *name)
{
  FILE *F;

  F = L_open_output_file (name);
  L_print_oper (F, oper);
  L_close_output_file (F);
}

void
DB_print_oper (L_Oper * oper)
{
  L_print_oper (stdout, oper);
}

void
DB_print_opid (L_Func * fn, int opid)
{
  L_Oper *op;

  op = L_oper_hash_tbl_find_oper (fn->oper_hash_tbl, opid);
  L_print_oper (stdout, op);
}

void
DB_print_func (L_Func * fn)
{
  L_print_func (stdout, fn);
}

void
DB_fn (L_Func * fn)
{
  DB_print_func (fn);
}

void
DB_print_cb (L_Cb * cb)
{
  int orig_L_print_all_regions = L_print_all_regions;
  L_print_all_regions = 0;
  L_print_cb (stdout, 0, cb);
  L_print_all_regions = orig_L_print_all_regions;
}

void
DB_print_cbid (L_Func * fn, int cbid)
{
  int orig_L_print_all_regions = L_print_all_regions;
  L_Cb *cb;

  cb = L_cb_hash_tbl_find (fn->cb_hash_tbl, cbid);

  L_print_all_regions = 0;
  L_print_cb (stdout, 0, cb);
  L_print_all_regions = orig_L_print_all_regions;
}

void
DB_cb (L_Cb * cb)
{
  DB_print_cb (cb);
}

void
DB_test_all (L_Func * fn)
{
  DB_cb (fn->first_cb);
  DB_spit_cb (fn->first_cb, "DEBUG_CB_TEST");
  DB_fn (fn);
  DB_spit_func (fn, "DEBUG_FUNC_TEST");
}

L_Oper *
DB_id2oper(L_Func * fn, int opid)
{
  L_Oper *op;
  op = L_oper_hash_tbl_find_oper (fn->oper_hash_tbl, opid);
  return op;
}

L_Cb*
DB_id2cb(L_Func * fn, int cbid)
{
  L_Cb *cb;
  cb = L_cb_hash_tbl_find (fn->cb_hash_tbl, cbid);
  return cb;
}

void
DB_print_df_set_opid(L_Func *fn, int opid, char *type)
{ 
  L_Oper *op;
  PF_OPER *pf_oper;

  op = DB_id2oper(fn,opid);
  if (!op)
    {
      printf("op id not found\n");
      return;
    }

  if (!strcmp(type,"live"))
    {
      pf_oper =
        (PF_OPER *) HashTable_find_or_null (PF_default_flow->hash_oper_pfoper,
                                            opid);
      if (!pf_oper)
	{
	  printf("pf_oper not found\n");
	  return;	  
	}
      Set_print(stdout,"v_out",pf_oper->info->v_out);
      Set_print(stdout,"v_in",pf_oper->info->v_out);      
    }
  else
    {
      printf("unknown type. valid types: live\n");
    }
}

void
DB_print_set (char *msg, Set s)
{
  Set_print (stdout, msg, s);
}

FILE *
DB_out ()
{
  return stdout;
}

FILE *
DB_err ()
{
  return stderr;
}

void
DB_spit_hash (INT_Symbol_Table * table, char *name)
{
  FILE *out;
  INT_Symbol *symbol;
  int hash_index, lines;

  out = L_open_output_file (name);

  /* Count lines used in table */
  lines = 0;
  for (hash_index = 0; hash_index < table->hash_size; hash_index++)
    {
      if (table->hash[hash_index] != NULL)
        lines++;
    }
  fprintf (out, "%s has %i entries (hash size %i, used %i):\n",
           table->name, table->symbol_count, table->hash_size, lines);

  /* For each hash_index in hash table */
  for (hash_index = 0; hash_index < table->hash_size; hash_index++)
    {
      /* Skip empty lines */
      if (table->hash[hash_index] == NULL)
        continue;

      fprintf (out, "%4i:", hash_index);
      for (symbol = table->hash[hash_index]; symbol != NULL;
           symbol = symbol->next_hash)
        {
#ifdef LP64_ARCHITECTURE
          fprintf (out, " %li", (long)(symbol->value));
#else
          fprintf (out, " %i", (int)(symbol->value));
#endif
          Set_print (out, "DEBUG ", (Set) symbol->data);
        }
      fprintf (out, "\n");
    }

  L_close_output_file (out);
}

void
DB_print_flows (L_Flow * flow)
{
  for (; flow != NULL; flow = flow->next_flow)
    {
      fprintf (stdout, "(flow %d: %d -> %d)\n", flow->cc,
               flow->src_cb->id, flow->dst_cb->id);
    }
}
