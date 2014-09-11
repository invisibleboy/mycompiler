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
 *      File :          l_binaryio.c
 *      Description :   Binary Lcode io routines
 *
 *      NOTE: EACH FUNCTION IN THIS FILE HAS AN ASCII ANALOGUE IN THE FILE
 *              l_io.c  BOTH FUNCTIONS MUST BE KEPT IN SYNC.
 *
 *            CARE MUST BE TAKEN WHEN ALTERING THE FORMAT OF THE BINARY
 *            FILE SO THAT EXISTING BINARY LCODE DOES NOT BECOME INVALID.
 *              (IF THIS IS UNAVOIDABLE, A TRANSLATOR MUST BE PROVIDED!)
 *
 *      Original: Richard E. Hank, Wen-mei Hwu, April 1996
 *      Modified : 
 *
\*****************************************************************************/
/* 09/24/02 REK Updating this function to write the completers field from
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
#include <Lcode/l_time.h>

#include "l_operand_symbol.h"
#include "l_binaryio.h"

#define NEW_JUMP_TBL_STUFF

OPERAND_Symbol_Table *L_operand_table;
STRING_Symbol_Table *L_func_string_table;

static L_Operand **L_operand_map;
static unsigned char *L_operand_used;
static char **L_string_map;

static char *L_last_printed_label = "";
static char *L_last_read_label = "";

int L_binary_file_version = 0;

/*===================================
 * 
 *  Detection/Version of binary file
 *
 *===================================*/
int
L_file_is_binary (int ch)
{
  if (ch == EOF)
    return (0);

  return ((ch & DELIMIT) ? 1 : 0);
}

int
L_is_binary_magic_header (int ch)
{
  switch (ch)
    {
    case BINARY_VERSION_1:
    case BINARY_VERSION_2:
      return (1);
    default:
      return (0);
    }
}

void
L_check_binary_file_version (int ch)
{
  switch (ch)
    {
    case BINARY_VERSION_1:
      break;
    default:
      L_punt ("Binary file version is %d, expected version <= %d\n",
              ch & (~DELIMIT), CURRENT_BINARY_VERSION & (~DELIMIT));
    }
  L_binary_file_version = ch;
}

/*================================
 *
 * Binary file i/o predicates
 *
 *================================*/

/* Reads in a string from the input stream.  It is assumed that
 * there is no leading white space and string termination is
 * indicated by the presence of the DELIMIT bit in the last
 * character
 */
char *
L_binary_read_string (FILE * in, L_Input_Buf * input_buf)
{
  int i, resize_size;
  int ch;
  char *buf;

  /* Get values in input_buf into local variables for ease of access */
  buf = input_buf->token_buf;
  resize_size = input_buf->max_token_len - 2;   /* Want space for terminator */

  i = 0;

  /* Read in string */
  while ((ch = getc (in)) != EOF)
    {
      /* Stop at a delimiter character only! */
      if (ch & DELIMIT)
        {
          buf[i++] = ch & (~(DELIMIT));
          break;
        }

      /* Resize buffer if necessary */
      if (i >= resize_size)
        {
          /* Double size of buffer */
          input_buf->max_token_len = input_buf->max_token_len << 1;

          /* Malloc new line buffer */
          if (!(input_buf->token_buf = 
               (char *) realloc (input_buf->token_buf, 
				 input_buf->max_token_len)))
            L_punt ("Out of memory in L_binary_read_string");

          /* Get new values into local variables */
          buf = input_buf->token_buf;
          resize_size = input_buf->max_token_len - 2;
        }

      buf[i] = ch;
      i++;
    }
  /* If buffer is empty return NULL */
  if (i == 0)
    return (NULL);

  /* Terminate buffer, value returned in value_buffer (buf) */
  buf[i] = 0;

  return (buf);
}

char *
L_binary_peek_next_string (FILE * in, L_Input_Buf * input_buf)
{
  int i, j, ch;
  char *token_ptr;

  token_ptr = &L_peek_token[0];
  i = 0;

  /* Read in string */
  while ((ch = getc (in)) != EOF)
    {
      /* Stop at a delimiter character only! */
      if (ch & DELIMIT)
        {
          token_ptr[i++] = ch & (~(DELIMIT));
          break;
        }

      token_ptr[i++] = ch;
    }

  if (i == 0)
    return (NULL);

  token_ptr[i] = '\0';

  /* shove the string back into the input buffer */
  if (ch != EOF)
    {
      ungetc ((DELIMIT | token_ptr[i - 1]), in);
      for (j = i - 2; j >= 0; j--)
        {
          ungetc (token_ptr[j], in);
        }
    }
  else
    {
      for (j = i - 1; j >= 0; j--)
        {
          ungetc (token_ptr[j], in);
        }
    }

  return (token_ptr);
}

/* Reads an integer from the input stream, assuming that
 * the integer is represented in hex.  Uses a fast conversion
 * algorithm that assumes ascii and that all integers are
 * written in base-64.
 */
ITintmax
L_binary_read_intmax (FILE * in)
{
  ITintmax value;
  int negate, delimit, ch;

  negate = 0;
  delimit = 0;
  value = 0;

  /*
   * Assumptions:  1)  only the first character of the integer
   *                   may possibly contain a minus sign.
   *               2)  only the first character of the integer
   *                   may have the SP_CHAR bit set.
   */
  if ((ch = getc (in)) != EOF)
    {
      if (ch == MINUS)
        negate = 1;
      else
        {
          /* If we've found a delimiter, signal end if integer */
          /* and clear the delimiter bit so the digit can be   */
          /* processed as a normal digit and remove the special */
          /* character bits if any.                        */
          delimit = ch & DELIMIT;
          value = ch & (~(DELIMIT | SP_CHAR));

          /* Negate value if necessary */
          if (negate)
            value = -value;

          if (delimit)
            return (value);
        }
    }
  else
    {
      /* Punt if hit EOF */
      if (ch == EOF)
        L_punt ("L_binary_read_int: unexpected EOF");
    }

  while ((ch = getc (in)) != EOF)
    {
      /* If we've found a delimiter, signal end if integer */
      /* and clear the delimiter bit so the digit can be   */
      /* processed as a normal digit.                      */
      delimit = ch & DELIMIT;
      ch = ch & (~DELIMIT);

      value = (value << 6) + ch;

      if (delimit)
        break;
    }
  /* Punt if hit EOF */
  if (ch == EOF)
    L_punt ("L_binary_read_int: unexpected EOF");

  /* Negate value if necessary */
  if (negate)
    value = -value;

  /* Return value read in */
  return (value);
}

/* Reads an integer from the input stream, assuming that
 * the integer is represented in hex.  Uses a fast conversion
 * algorithm that assumes ascii and that all integers are
 * written in base-64.
 */
int
L_binary_read_int (FILE * in)
{
  int value, negate, delimit;
  int ch;

  negate = 0;
  delimit = 0;
  value = 0;

  /*
   * Assumptions:  1)  only the first character of the integer
   *                   may possibly contain a minus sign.
   *               2)  only the first character of the integer
   *                   may have the SP_CHAR bit set.
   */
  if ((ch = getc (in)) != EOF)
    {
      if (ch == MINUS)
        negate = 1;
      else
        {
          /* If we've found a delimiter, signal end if integer */
          /* and clear the delimiter bit so the digit can be   */
          /* processed as a normal digit and remove the special */
          /* character bits if any.                        */
          delimit = ch & DELIMIT;
          value = ch & (~(DELIMIT | SP_CHAR));

          /* Negate value if necessary */
          if (negate)
            value = -value;

          if (delimit)
            return (value);
        }
    }
  else
    {
      /* Punt if hit EOF */
      if (ch == EOF)
        L_punt ("L_binary_read_int: unexpected EOF");
    }

  while ((ch = getc (in)) != EOF)
    {
      /* If we've found a delimiter, signal end if integer */
      /* and clear the delimiter bit so the digit can be   */
      /* processed as a normal digit.                      */
      delimit = ch & DELIMIT;
      ch = ch & (~DELIMIT);

      value = (value << 6) + ch;

      if (delimit)
        break;
    }
  /* Punt if hit EOF */
  if (ch == EOF)
    L_punt ("L_binary_read_int: unexpected EOF");

  /* Negate value if necessary */
  if (negate)
    value = -value;

  /* Return value read in */
  return (value);
}

/* Returns the next character from the input stream,
 * but does not remove it from the input buffer
 */
int
L_binary_peek_next_char (FILE * in)
{
  int ch;

  ch = getc (in);
  if (ch == EOF)
    return (ch);

  ungetc (ch, in);

  return (ch);
}

/* Retrieve and remove the next character from the
 * input stream.
 */
int
L_binary_read_char (FILE * in)
{
  int ch;

  ch = getc (in);

  if (ch == EOF)
    L_punt ("L_binary_read_char: unexpected EOF");

  return (ch);
}

/* Write a string to the output stream and annotate
 * the last character with the DELIMIT bit to denote
 * string termination
 */
void
L_binary_write_string (FILE * F, char *string)
{
  char *ch, *nch;

  ch = string;
  nch = ch + 1;

  while (*ch)
    {
      if (*nch != '\0')
        putc (*ch, F);
      else
        putc (DELIMIT | *ch, F);

      nch++;
      ch++;
    }
}

/* Write a float value to the output
 * stream terminating the number by setting the
 * DELIMIT bit in the last digit
 */
void
L_binary_write_float (FILE * F, float f)
{
  char buf[128];

  sprintf (buf, "%0.8g", f);

  L_binary_write_string (F, buf);
}

/* Write a double value to the output
 * stream terminating the number by setting the
 * DELIMIT bit in the last digit
 */
void
L_binary_write_double (FILE * F, double d)
{
  char buf[128];

  sprintf (buf, "%0.16g", d);

  L_binary_write_string (F, buf);
}

/* Write an integer to the output stream encoded
 * in base-64.  The last digit of the integer contains
 * a DELIMIT bit to denote termination.  If the integer
 * is negative, the number is preceded by the MINUS
 * character.
 */
void
L_binary_write_intmax (FILE * F, ITintmax i)
{
  int j;
  unsigned int value;
  int nibble;
  int non_zero;
  unsigned char hex_buf[sizeof (ITintmax)];

  /*
   * If the number is negative, output the MINUS
   * character and negate the value
   */
  if (i >= 0)
    {
      value = i;
    }
  else
    {
      value = -i;
      putc (MINUS, F);
    }

  /* Generate the base-64 encoding of the integer */
  non_zero = 10;
  for (j = 10; j >= 0; j--)
    {
      nibble = value & 0x3f;

      hex_buf[j] = nibble;

      if (nibble != 0)
        non_zero = j;

      value = value >> 6;
    }

  /* Do not output leading zero digits */
  for (j = non_zero; j < 10; j++)
    putc (hex_buf[j], F);
  putc (DELIMIT | hex_buf[10], F);

}



/* Write an integer to the output stream encoded
 * in base-64.  The last digit of the integer contains
 * a DELIMIT bit to denote termination.  If the integer
 * is negative, the number is preceded by the MINUS
 * character.
 */
void
L_binary_write_int (FILE * F, int i)
{
  int j;
  unsigned int value;
  int nibble;
  int non_zero;
  unsigned char hex_buf[8];

  /*
   * If the number is negative, output the MINUS
   * character and negate the value
   */
  if (i >= 0)
    {
      value = i;
    }
  else
    {
      value = -i;
      putc (MINUS, F);
    }

  /* Generate the base-64 encoding of the integer */
  non_zero = 5;
  for (j = 5; j >= 0; j--)
    {
      nibble = value & 0x3f;

      hex_buf[j] = nibble;

      if (nibble != 0)
        non_zero = j;

      value = value >> 6;
    }

  /* Do not output leading zero digits */
  for (j = non_zero; j < 5; j++)
    putc (hex_buf[j], F);
  putc (DELIMIT | hex_buf[5], F);

}

/* Performs the same task as the above function,
 * except that the first non-zero digit of the integer
 * is flagged with the SP_CHAR bit to aid error 
 * detection.
 */
void
L_binary_write_int_special (FILE * F, int i)
{
  int j;
  unsigned int value;
  int nibble;
  int non_zero;
  unsigned char hex_buf[8];

  /*
   * If the number is negative, output the MINUS
   * character and negate the value
   */
  if (i >= 0)
    {
      value = i;
    }
  else
    {
      value = -i;
      putc (MINUS, F);
    }

  /* Generate the base-64 encoding of the integer */
  non_zero = 5;
  for (j = 5; j >= 0; j--)
    {
      nibble = value & 0x3f;

      hex_buf[j] = nibble;

      if (nibble != 0)
        non_zero = j;

      value = value >> 6;
    }
  hex_buf[non_zero] |= SP_CHAR;

  /* Do not output leading zero digits */
  for (j = non_zero; j < 5; j++)
    {
      putc (hex_buf[j], F);
    }
  putc (DELIMIT | hex_buf[5], F);
}

/*
 * Scan the function and build a hash table of unique 
 * operands.  Operands are found in:
 *      1) Function, Cb, Oper attributes
 *      2) Operation Pred, Dest, Src Operands
 *
 * Scan the function and produce a list of unique strings
 *      1) Attribute names
 *      2) Func, Cb, Oper flag strings
 *      3) Opcode strings
 */
static void
L_build_operand_hash_table (L_Func * fn, int *num_operands, int *num_strings)
{
  int i, nopd, nstr;
  L_Cb *cb;
  L_Oper *oper;
  L_Operand *opd;
  L_Attr *attr;
  char flag_string[33];

  nopd = *num_operands;
  nstr = *num_strings;

  /* Determine function flag string */
  L_func_flags_to_string (flag_string, fn->flags);
  if (*flag_string != '\0' &&
      STRING_find_symbol (L_func_string_table, flag_string) == NULL)
#ifdef LP64_ARCHITECTURE
    STRING_add_symbol (L_func_string_table, flag_string,
		       (void *)((long)nstr++));
#else
    STRING_add_symbol (L_func_string_table, flag_string, (void *) nstr++);
#endif

  /*
   * Grab operand structures and strings from function attributes
   */
  for (attr = fn->attr; attr != NULL; attr = attr->next_attr)
    {
      /* Place the attribute name in the function name table */
      if (STRING_find_symbol (L_func_string_table, attr->name) == NULL)
#ifdef LP64_ARCHITECTURE
        STRING_add_symbol (L_func_string_table, attr->name,
			   (void *)((long)nstr++));
#else
        STRING_add_symbol (L_func_string_table, attr->name, (void *) nstr++);
#endif

      for (i = 0; i < attr->max_field; i++)
        {
          if ((opd = attr->field[i]) == NULL)
            continue;

          if (OPERAND_find_symbol (L_operand_table, opd) == NULL)
#ifdef LP64_ARCHITECTURE
            OPERAND_add_symbol (L_operand_table, opd, (void *)((long)nopd++));
#else
            OPERAND_add_symbol (L_operand_table, opd, (void *) nopd++);
#endif
        }
    }


  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      /* Determine cb flag string */
      L_cb_flags_to_string (flag_string, cb->flags);
      if (*flag_string != '\0' &&
          STRING_find_symbol (L_func_string_table, flag_string) == NULL)
#ifdef LP64_ARCHITECTURE
        STRING_add_symbol (L_func_string_table, flag_string,
			   (void *)((long)nstr++));
#else
        STRING_add_symbol (L_func_string_table, flag_string, (void *) nstr++);
#endif

      /* 
       * Grab operand structures and strings from cb attributes
       */
      for (attr = cb->attr; attr != NULL; attr = attr->next_attr)
        {
          /* Place the attribute name in the function name table */
          if (STRING_find_symbol (L_func_string_table, attr->name) == NULL)
#ifdef LP64_ARCHITECTURE
            STRING_add_symbol (L_func_string_table, attr->name,
                               (void *)((long)nstr++));
#else
            STRING_add_symbol (L_func_string_table, attr->name,
                               (void *) nstr++);
#endif

          for (i = 0; i < attr->max_field; i++)
            {
              if ((opd = attr->field[i]) == NULL)
                continue;
              if (OPERAND_find_symbol (L_operand_table, opd) == NULL)
#ifdef LP64_ARCHITECTURE
                OPERAND_add_symbol (L_operand_table, opd,
				    (void *)((long)nopd++));
#else
                OPERAND_add_symbol (L_operand_table, opd, (void *) nopd++);
#endif
            }
        }
      /*
       * Grab operands from oper src,dest,pred, and attributes 
       */
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
        {
          /* Determine oper flag string */
          L_oper_flags_to_string (flag_string, oper->flags);
          if (*flag_string != '\0' &&
              STRING_find_symbol (L_func_string_table, flag_string) == NULL)
#ifdef LP64_ARCHITECTURE
            STRING_add_symbol (L_func_string_table,
                               flag_string, (void *)((long)nstr++));
#else
            STRING_add_symbol (L_func_string_table,
                               flag_string, (void *) nstr++);
#endif

          /* Place the opcode name in the function string table */
          if (STRING_find_symbol (L_func_string_table, oper->opcode) == NULL)
#ifdef LP64_ARCHITECTURE
            STRING_add_symbol (L_func_string_table, oper->opcode,
                               (void *)((long)nstr++));
#else
            STRING_add_symbol (L_func_string_table, oper->opcode,
                               (void *) nstr++);
#endif

          for (i = 0; i < L_max_dest_operand; i++)
            {
              if ((opd = oper->dest[i]) == NULL)
                continue;
              if (OPERAND_find_symbol (L_operand_table, opd) == NULL)
#ifdef LP64_ARCHITECTURE
                OPERAND_add_symbol (L_operand_table, opd,
				    (void *)((long)nopd++));
#else
                OPERAND_add_symbol (L_operand_table, opd, (void *) nopd++);
#endif
            }
          for (i = 0; i < L_max_src_operand; i++)
            {
              if ((opd = oper->src[i]) == NULL)
                continue;
              if (OPERAND_find_symbol (L_operand_table, opd) == NULL)
#ifdef LP64_ARCHITECTURE
                OPERAND_add_symbol (L_operand_table, opd,
				    (void *)((long)nopd++));
#else
                OPERAND_add_symbol (L_operand_table, opd, (void *) nopd++);
#endif
            }
          for (i = 0; i < L_max_pred_operand; i++)
            {
              if ((opd = oper->pred[i]) == NULL)
                continue;
              if (OPERAND_find_symbol (L_operand_table, opd) == NULL)
#ifdef LP64_ARCHITECTURE
                OPERAND_add_symbol (L_operand_table, opd,
				    (void *)((long)nopd++));
#else
                OPERAND_add_symbol (L_operand_table, opd, (void *) nopd++);
#endif
            }
          for (attr = oper->attr; attr != NULL; attr = attr->next_attr)
            {
              /* Place the attribute name in the function string table */
              if (STRING_find_symbol (L_func_string_table, attr->name) ==
                  NULL)
#ifdef LP64_ARCHITECTURE
                STRING_add_symbol (L_func_string_table, attr->name,
                                   (void *)((long)nstr++));
#else
                STRING_add_symbol (L_func_string_table, attr->name,
                                   (void *) nstr++);
#endif

              for (i = 0; i < attr->max_field; i++)
                {
                  if ((opd = attr->field[i]) == NULL)
                    continue;
                  if (OPERAND_find_symbol (L_operand_table, opd) == NULL)
#ifdef LP64_ARCHITECTURE
                    OPERAND_add_symbol (L_operand_table, opd,
                                        (void *)((long)nopd++));
#else
                    OPERAND_add_symbol (L_operand_table, opd,
                                        (void *) nopd++);
#endif
                }
            }
          /* REH 4-30-96, since the "popc" attribute is added */
          /* dynamically at the time the oper is output, we  */
          /* need to added it here to make sure that the     */
          /* attribute name and operand appear in the hash   */
          /* tables.....                                     */
	  /* 09/24/02 REK Updating to add the completers field along with
	   *              the proc_opc field. */
          attr = L_find_attr (oper->attr, "popc");
          if (attr == NULL && oper->opc != oper->proc_opc)
            {
              attr = L_new_attr ("popc", 2);
              L_set_int_attr_field (attr, 0, oper->proc_opc);
              L_set_int_attr_field (attr, 1, oper->completers);
              oper->attr = L_concat_attr (oper->attr, attr);

              if (STRING_find_symbol (L_func_string_table, "popc") == NULL)
#ifdef LP64_ARCHITECTURE
                STRING_add_symbol (L_func_string_table, "popc",
                                   (void *)((long)nstr++));
#else
                STRING_add_symbol (L_func_string_table, "popc",
                                   (void *) nstr++);
#endif

              opd = attr->field[0];
              if (OPERAND_find_symbol (L_operand_table, opd) == NULL)
#ifdef LP64_ARCHITECTURE
                OPERAND_add_symbol (L_operand_table, opd,
				    (void *)((long)nopd++));
#else
                OPERAND_add_symbol (L_operand_table, opd, (void *) nopd++);
#endif
            }
        }
    }
  *num_operands = nopd;
  *num_strings = nstr;
}

L_Event *
L_read_event_list_binary (FILE * F, L_Input_Buf * buf)
{
  L_punt ("L_read_event_list_binary: Event lists are not yet supported\n",
          F, buf);

  return (NULL);
}

/* Recursively grab the next expression from the input
 * stream.  The recursion is terminated by the presence
 * of the DELIMIT character.
 */
L_Expr *
L_read_expr_binary (FILE * F, L_Input_Buf * buf)
{
  int ch, ival;
  char *token;
  double fval;
  L_Expr *new_expr;

  /*
   * Termination condition for recursion
   */
  if (L_binary_peek_next_char (F) == DELIMIT)
    return (NULL);

  /*
   * Determine the expression type.  Expresion types
   * are indicate using a single character rather than
   * the full expression type name used in the ascii
   * representation.
   */
  ch = L_binary_read_char (F) & (~DELIMIT);

  switch (ch)
    {
    case 'i':
      ival = L_binary_read_int (F);
      new_expr = L_new_expr (L_EXPR_INT);
      new_expr->value.i = ival;
      break;
    case 'f':
      token = L_binary_read_string (F, buf);
      fval = atof (token);
      new_expr = L_new_expr (L_EXPR_FLOAT);
      new_expr->value.f = fval;
      break;
    case 'd':
      token = L_binary_read_string (F, buf);
      fval = atof (token);
      new_expr = L_new_expr (L_EXPR_DOUBLE);
      new_expr->value.f2 = fval;
      break;
    case 'l':
      new_expr = L_new_expr (L_EXPR_LABEL);
      /*
       * The same label appears in successive expressions
       * quite frequently, so we remember the last label
       * used and do not emit a label if it is the same as
       * the last label printed.  Provides a significant
       * reduction in data size in many instances.
       */
      if (L_binary_peek_next_char (F) == DELIMIT)
        {
          new_expr->value.l = L_last_read_label;
        }
      else
        {
          token = L_binary_read_string (F, buf);
          new_expr->value.l = L_add_string (L_string_table, token);
          L_last_read_label = new_expr->value.l;
        }
      break;
    case 's':
      token = L_binary_read_string (F, buf);
      new_expr = L_new_expr (L_EXPR_STRING);
      new_expr->value.s = L_add_string (L_string_table, token);
      break;
    case 'A':
      new_expr = L_new_expr (L_EXPR_ADD);
      new_expr->A = L_read_expr_binary (F, buf);
      new_expr->B = L_read_expr_binary (F, buf);
      break;
    case 'S':
      new_expr = L_new_expr (L_EXPR_SUB);
      new_expr->A = L_read_expr_binary (F, buf);
      new_expr->B = L_read_expr_binary (F, buf);
      break;
    case 'M':
      new_expr = L_new_expr (L_EXPR_MUL);
      new_expr->A = L_read_expr_binary (F, buf);
      new_expr->B = L_read_expr_binary (F, buf);
      break;
    case 'D':
      new_expr = L_new_expr (L_EXPR_DIV);
      new_expr->A = L_read_expr_binary (F, buf);
      new_expr->B = L_read_expr_binary (F, buf);
      break;
    case 'N':
      new_expr = L_new_expr (L_EXPR_NEG);
      new_expr->A = L_read_expr_binary (F, buf);
      break;
    case 'C':
      new_expr = L_new_expr (L_EXPR_COM);
      new_expr->A = L_read_expr_binary (F, buf);
      break;
    default:
      L_punt ("L_read_expr: illegal Expr <%c>", ch);
      return (NULL);
    }

  new_expr->next_expr = L_read_expr_binary (F, buf);

  if (L_binary_read_char (F) != DELIMIT)
    L_punt ("L_read_expr_binary: Unexpected end of expression");

  return new_expr;
}

/* Recursively write an expression to the output stream,
 * terminating the recursion by inserting a DELIMIT character.
 */
void
L_print_expr_binary (FILE * F, L_Expr * expr)
{
  if (expr == 0)
    return;
  switch (expr->type)
    {
    case L_EXPR_INT:
      putc (DELIMIT | 'i', F);
      L_binary_write_int (F, (int) expr->value.i);
      putc (DELIMIT, F);
      break;
    case L_EXPR_FLOAT:
      putc (DELIMIT | 'f', F);
      L_binary_write_float (F, expr->value.f);
      putc (DELIMIT, F);
      break;
    case L_EXPR_DOUBLE:
      putc (DELIMIT | 'd', F);
      L_binary_write_double (F, expr->value.f2);
      putc (DELIMIT, F);
      break;
    case L_EXPR_LABEL:
      putc (DELIMIT | 'l', F);
      if (strcmp (L_last_printed_label, expr->value.l))
        {
          L_binary_write_string (F, expr->value.l);
          L_last_printed_label = expr->value.l;
        }
      putc (DELIMIT, F);
      break;
    case L_EXPR_STRING:
      putc (DELIMIT | 's', F);
      L_binary_write_string (F, expr->value.s);
      putc (DELIMIT, F);
      break;
    case L_EXPR_ADD:
      putc (DELIMIT | 'A', F);
      L_print_expr_binary (F, expr->A);
      L_print_expr_binary (F, expr->B);
      putc (DELIMIT, F);
      break;
    case L_EXPR_SUB:
      putc (DELIMIT | 'S', F);
      L_print_expr_binary (F, expr->A);
      L_print_expr_binary (F, expr->B);
      putc (DELIMIT, F);
      break;
    case L_EXPR_MUL:
      putc (DELIMIT | 'M', F);
      L_print_expr_binary (F, expr->A);
      L_print_expr_binary (F, expr->B);
      putc (DELIMIT, F);
      break;
    case L_EXPR_DIV:
      putc (DELIMIT | 'D', F);
      L_print_expr_binary (F, expr->A);
      L_print_expr_binary (F, expr->B);
      putc (DELIMIT, F);
      break;
    case L_EXPR_NEG:
      putc (DELIMIT | 'N', F);
      L_print_expr_binary (F, expr->A);
      putc (DELIMIT, F);
      break;
    case L_EXPR_COM:
      putc (DELIMIT | 'C', F);
      L_print_expr_binary (F, expr->A);
      putc (DELIMIT, F);
      break;
    default:
      L_punt ("L_print_expr_binary: illegal argument type");
    }
}

/*
 *      L_Datalist_Element - SAM added 7-96
 */
void
L_print_datalist_element_binary (FILE * F, L_Datalist_Element * element)
{
  L_print_data_binary (F, element->data);
}

/*
 *      L_Datalist - SAM added 7-96
 */
void
L_print_datalist_binary (FILE * F, L_Datalist * list)
{
  L_Datalist_Element *curr_element;

  if (list == NULL)
    L_punt ("L_print_datalist_binary: NULL list");

  curr_element = list->first_element;

  while (curr_element != NULL)
    {

      L_print_datalist_element_binary (F, curr_element);

      curr_element = curr_element->next_element;
    }
}

/*
 * The following two functions are responsible for
 * reading/writing L_Data structions to/from the 
 * input/output stream.
 */
void
L_read_data_binary (FILE * F, int type, L_Input_Buf * buf)
{
  int n;
  int i, num_data;
  char *label, *str;
  char *token;

  /* 
   * Create a new data element 
   */
  L_data = L_new_data (type);

  switch (type)
    {
    case L_INPUT_MS:
      label = L_binary_read_string (F, buf);
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
      n = L_binary_read_int (F);
      L_data->N = n;
      L_data->address = L_new_expr (L_EXPR_LABEL);

      label = L_binary_read_string (F, buf);
      L_data->address->value.l = L_add_string (L_string_table, label);

      /* Get initializer expressions if any */
      num_data = L_binary_read_int (F);
      for (i = 0; i < num_data; i++)
        {
          L_data->value = L_read_expr_binary (F, buf);
        }
      break;
    case L_INPUT_ASCII:
    case L_INPUT_ASCIZ:
      str = L_binary_read_string (F, buf);
      L_data->value = L_new_expr (L_EXPR_STRING);
      L_data->value->value.s = L_add_string (L_string_table, str);

      L_data->address = L_new_expr (L_EXPR_LABEL);
      label = L_binary_read_string (F, buf);
      L_data->address->value.l = L_add_string (L_string_table, label);
      break;
    case L_INPUT_RESERVE:
      n = L_binary_read_int (F);
      L_data->N = n;
      break;
    case L_INPUT_GLOBAL:
      token = L_binary_read_string (F, buf);
      L_data->address = L_new_expr (L_EXPR_LABEL);
      L_data->address->value.l = L_add_string (L_string_table, token);
      break;
    case L_INPUT_WB:
    case L_INPUT_WW:
    case L_INPUT_WI:
    case L_INPUT_WQ:
    case L_INPUT_WF:
    case L_INPUT_WF2:
    case L_INPUT_WS:
      L_data->address = L_read_expr_binary (F, buf);
      L_data->value = L_read_expr_binary (F, buf);
      break;
    case L_INPUT_SKIP:
      n = L_binary_read_int (F);
      L_data->N = n;
      break;
    }
}
void
L_print_data_binary (FILE * F, L_Data * data)
{
  int num_data;
  L_Expr *addr, *val;

  if (data == NULL)
    {
      L_punt ("L_print_data: no argument");
    }

  switch (data->type)
    {
    case L_INPUT_MS:
      L_binary_write_string (F, "ms");
      L_binary_write_string (F, L_ms_name (data->N));
      break;
    case L_INPUT_GLOBAL:
    case L_INPUT_VOID:
      addr = data->address;
      if (addr == NULL)
        {
          L_punt ("L_print_data: bad address");
        }
      L_binary_write_string (F, L_lcode_name (data->type));
      L_binary_write_string (F, addr->value.l);
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
      L_binary_write_string (F, L_lcode_name (data->type));
      L_binary_write_int (F, data->N);
      L_binary_write_string (F, addr->value.l);

      num_data = 0;
      for (val = data->value; val != 0; val = val->next_expr)
        num_data += 1;

      L_binary_write_int (F, num_data);
      for (val = data->value; val != 0; val = val->next_expr)
        L_print_expr_binary (F, val);
      break;
    case L_INPUT_ASCII:
    case L_INPUT_ASCIZ:
      addr = data->address;
      val = data->value;
      if ((addr == NULL) || (val == NULL))
        {
          L_punt ("L_print_data: bad pointer");
        }
      L_binary_write_string (F, L_lcode_name (data->type));
      L_binary_write_string (F, val->value.s);
      L_binary_write_string (F, addr->value.l);
      break;
    case L_INPUT_RESERVE:
      L_binary_write_string (F, "reserve");
      L_binary_write_int (F, data->N);
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
      L_binary_write_string (F, L_lcode_name (data->type));
      L_print_expr_binary (F, addr);
      L_print_expr_binary (F, val);
      break;
    case L_INPUT_SKIP:
      L_binary_write_string (F, "skip");
      L_binary_write_int (F, data->N);
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
L_read_hash_tbl_binary (FILE * F, L_Datalist * tbl, L_Input_Buf * input_buf)
{
  char *keyword;
  int i, type, align_value, reserve_value, tbl_size;
  L_Datalist_Element *element;

  /*
   *  1. The first entry of the table should be a data of the form:
   *     (align tbl_entry_size tbl_name)
   */
  keyword = L_binary_read_string (F, input_buf);
  type = L_lcode_id (keyword);

  if (type != L_INPUT_ALIGN)
    {
      L_punt ("L_read_hash_tbl_binary: missing align stmt on line");
    }

  L_read_data_binary (F, type, input_buf);
  align_value = L_data->N;

  /* Create datalist_element and append to tbl */
  element = L_new_datalist_element (L_data);
  L_concat_datalist_element (tbl, element);

  /*
   *  2. The next token should be a data of the form:
   *     (reserve tbl_size)
   */
  keyword = L_binary_read_string (F, input_buf);
  type = L_lcode_id (keyword);

  if (type != L_INPUT_RESERVE)
    {
      L_punt ("L_read_hash_tbl_binary: missing reserve stmt on line");
    }

  L_read_data_binary (F, type, input_buf);
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
      keyword = L_binary_read_string (F, input_buf);
      type = L_lcode_id (keyword);

      if (type != L_INPUT_WI)
        {
          L_punt ("L_read_hash_tbl_binary: missing wi stmt on line");
        }

      L_read_data_binary (F, type, input_buf);
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
L_read_all_hashtbls_binary (FILE * F, int num_tbls, L_Input_Buf * input_buf)
{
  L_Datalist *tbl;
  char *keyword;
  int i, type;
  L_Datalist_Element *element;

  tbl = L_new_datalist ();

  /* Before the tables is an initial (ms data), so grab that first */
  keyword = L_binary_read_string (F, input_buf);
  type = L_lcode_id (keyword);

  if (type != L_INPUT_MS)
    {
      L_punt ("L_read_all_hashtbls_binary: missing ms stmt on line");
    }

  L_read_data_binary (F, type, input_buf);
  /* Create datalist_element and append to tbl */
  element = L_new_datalist_element (L_data);
  L_concat_datalist_element (tbl, element);

  /* Now read all the tables in, append the data items to end of tbl */
  for (i = 0; i < num_tbls; i++)
    {
      L_read_hash_tbl_binary (F, tbl, input_buf);
    }

  return (tbl);
}


/*
 * The following routines read/write binary representations
 * of Dave G's infamous sync arcs.
 *
 */
L_Sync *
L_read_sync_binary (FILE * F, L_Input_Buf * buf)
{
  int id, flags, dist, prof;
  char *name;

  id = L_binary_read_int (F);
  name = L_binary_read_string (F, buf);
  dist = L_binary_read_int (F);
  flags = L_binary_read_int (F);
  prof = L_binary_read_int (F);

  if (L_eliminate_sync_arcs)
    return (NULL);
  else
    return (L_create_new_sync (id, name[0], name[1], dist, flags, prof));
}

void
L_print_sync_binary (FILE * F, L_Sync * sync)
{
  int flags, id, prof;
  char prob, freq = '\0';
  char str[3];

  flags = (int) sync->info;
  prof = (int) sync->prof_info;

  if (IS_DEFINITE_SYNC (flags))
    prob = 'D';
  else if (IS_PROFILE_SYNC (flags))
    prob = 'P';
  else
    prob = 'M';

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

  if (sync->dep_oper)
    id = sync->dep_oper->id;
  else
    id = -1;

  str[0] = prob;
  str[1] = freq;
  str[2] = '\0';

  L_binary_write_int (F, id);
  L_binary_write_string (F, str);
  L_binary_write_int (F, (int) sync->dist);
  L_binary_write_int (F, flags);
  L_binary_write_int (F, prof);
}


/*
 * Rather than print the operand for each 
 * src/dest/pred/attr, we print only the unique id
 * assigned to that operand when it was inserted
 * into the hash table.
 */
void
L_print_operand_id (FILE * F, L_Operand * opd)
{
  OPERAND_Symbol *sym;

  if (opd == NULL)
    L_binary_write_int (F, 0);
  else
    {
      sym = OPERAND_find_symbol (L_operand_table, opd);
      if (sym == NULL)
        {
          fprintf (stderr, "Operand -> ");
          L_print_operand (stderr, opd, 0);
          fprintf (stderr, "\n");
          L_punt ("L_print_operand_id:  Unknown attribute operand!");
        }
#ifdef LP64_ARCHITECTURE
      L_binary_write_int (F, (int)((long)sym->data));
#else
      L_binary_write_int (F, (int) sym->data);
#endif
    }
}

L_Operand *
L_read_operand_binary (FILE * F, L_Input_Buf * buf, L_Func * fn)
{
  unsigned char ch;
  char *string;
  int ival;
  ITintmax imval;
  int omega;
  int ctype, ptype;
  L_Cb *cb;
  L_Operand *operand;
  double fval;

  ch = L_binary_read_char (F);
  if (!(ch & DELIMIT))
    L_punt ("L_read_operand_binary: I'm lost, this is not an operand <%c>!\n",
            ch);

  /* Remove the delimiter bit */
  ch = ch & (~DELIMIT);

  switch (ch)
    {
    case 'c':                   /* cb operand */
      ival = L_binary_read_int (F);
      cb = L_cb_hash_tbl_find_and_alloc (fn->cb_hash_tbl, ival);
      operand = L_new_cb_operand (cb);
      break;
    case 'r':
      ival = L_binary_read_int (F);
      string = L_binary_read_string (F, buf);
      ctype = L_ctype_id (string);
      if (ctype == -1)
        {
          L_punt ("L_read_operand_binary: illegal format (r ctype) <%s>",
                  string);
        }
      if (L_is_ctype_predicate_direct (ctype))
        {
          ptype = L_ptype_id (string);
          if (ptype == -1)
            L_punt ("L_read_operand_binary: illegal (r ptype) <%s>", string);
        }
      else
        ptype = L_PTYPE_NULL;
      operand = L_new_register_operand (ival, ctype, ptype);
      break;
    case 'R':
      ival = L_binary_read_int (F);
      string = L_binary_read_string (F, buf);
      ctype = L_ctype_id (string);
      if (ctype == -1)
        {
          L_punt ("L_read_operand_binary: illegal format (rr ctype) <%s>",
                  string);
        }
      if (L_is_ctype_predicate_direct (ctype))
        {
          ptype = L_ptype_id (string);
          if (ptype == -1)
            L_punt ("L_read_operand_binary: illegal (rr ptype) <%s>", string);
        }
      else
        ptype = L_PTYPE_NULL;
      operand = L_new_rregister_operand (ival, ctype, ptype);
      break;
    case 'e':
      ival = L_binary_read_int (F);
      omega = L_binary_read_int (F);
      string = L_binary_read_string (F, buf);
      ctype = L_ctype_id (string);
      if (ctype == -1)
        {
          L_punt ("L_read_operand_binary: illegal format (evr ctype) <%s>",
                  string);
        }
      if (L_is_ctype_predicate_direct (ctype))
        {
          ptype = L_ptype_id (string);
          if (ptype == -1)
            L_punt ("L_read_operand_binary: illegal (evr ptype) <%s>",
                    string);
        }
      else
        ptype = L_PTYPE_NULL;
      operand = L_new_evr_operand (ival, omega, ctype, ptype);
      break;
    case 'l':
      string = L_binary_read_string (F, buf);
      operand = L_new_label_operand (string, L_CTYPE_GLOBAL_ABS);
      break;
    case '1':                   /* l_l_abs */
      string = L_binary_read_string (F, buf);
      operand = L_new_label_operand (string, L_CTYPE_LOCAL_ABS);
      break;
    case '2':                   /* l_l_gp */
      string = L_binary_read_string (F, buf);
      operand = L_new_label_operand (string, L_CTYPE_GLOBAL_GP);
      break;
    case '3':                   /* l_g_abs */
      string = L_binary_read_string (F, buf);
      operand = L_new_label_operand (string, L_CTYPE_GLOBAL_ABS);
      break;
    case '4':                   /* l_g_gp */
      string = L_binary_read_string (F, buf);
      operand = L_new_label_operand (string, L_CTYPE_GLOBAL_GP);
      break;
    case 'C':
      imval = L_binary_read_intmax (F);
      operand = L_new_int_operand (imval, L_CTYPE_CHAR);
      break;
    case 'D':
      imval = L_binary_read_intmax (F);
      operand = L_new_int_operand (imval, L_CTYPE_UCHAR);
      break;
    case 'S':
      imval = L_binary_read_intmax (F);
      operand = L_new_int_operand (imval, L_CTYPE_SHORT);
      break;
    case 'T':
      imval = L_binary_read_intmax (F);
      operand = L_new_int_operand (imval, L_CTYPE_USHORT);
      break;
    case 'i':
      imval = L_binary_read_intmax (F);
      operand = L_new_int_operand (imval, L_CTYPE_INT);
      break;
    case 'J':
      imval = L_binary_read_intmax (F);
      operand = L_new_int_operand (imval, L_CTYPE_UINT);
      break;
    case 'L':
      imval = L_binary_read_intmax (F);
      operand = L_new_int_operand (imval, L_CTYPE_LONG);
      break;
    case 'M':
      imval = L_binary_read_intmax (F);
      operand = L_new_int_operand (imval, L_CTYPE_ULONG);
      break;
    case 'W':
      imval = L_binary_read_intmax (F);
      operand = L_new_int_operand (imval, L_CTYPE_LLONG);
      break;
    case 'X':
      imval = L_binary_read_intmax (F);
      operand = L_new_int_operand (imval, L_CTYPE_ULLONG);
      break;
    case 'Y':
      imval = L_binary_read_intmax (F);
      operand = L_new_int_operand (imval, L_CTYPE_LLLONG);
      break;
    case 'Z':
      imval = L_binary_read_intmax (F);
      operand = L_new_int_operand (imval, L_CTYPE_ULLLONG);
      break;
    case 'P':
      imval = L_binary_read_intmax (F);
      operand = L_new_int_operand (imval, L_CTYPE_POINTER);
      break;
    case 'f':
      string = L_binary_read_string (F, buf);
      if (L_strtod (string, NULL, &fval) == 0)
        {
          L_punt
            ("L_read_operand_binary: invalid floating point value <%s>\n",
             string);
        }
      operand = L_new_float_operand ((float) fval);
      break;
    case 'd':
      string = L_binary_read_string (F, buf);
      if (L_strtod (string, NULL, &fval) == 0)
        {
          L_punt
            ("L_read_operand_binary: invalid floating point value <%s>\n",
             string);
        }
      operand = L_new_double_operand (fval);
      break;
    case 's':
      string = L_binary_read_string (F, buf);
      operand = L_new_string_operand (string, L_CTYPE_GLOBAL_ABS);
      break;
    case '5':                   /* s_l_abs */
      string = L_binary_read_string (F, buf);
      operand = L_new_string_operand (string, L_CTYPE_LOCAL_ABS);
      break;
    case '6':                   /* s_l_gp */
      string = L_binary_read_string (F, buf);
      operand = L_new_string_operand (string, L_CTYPE_LOCAL_GP);
      break;
    case '7':                   /* s_g_abs */
      string = L_binary_read_string (F, buf);
      operand = L_new_string_operand (string, L_CTYPE_GLOBAL_ABS);
      break;
    case '8':                   /* s_g_gp */
      string = L_binary_read_string (F, buf);
      operand = L_new_string_operand (string, L_CTYPE_GLOBAL_GP);
      break;
    case 'm':
      string = L_binary_read_string (F, buf);
      ival = L_macro_id (string);
      if (ival == -1)
        {
          L_punt ("L_read_operand: unknown macro <%s>\n \
*** Make sure this is %s/%s lcode! ***", string, L_arch, L_model);
        }
      string = L_binary_read_string (F, buf);
      ctype = L_ctype_id (string);
      if (ctype == -1)
        {
          L_punt ("L_read_operand_binary: illegal format (macro ctype) <%s>",
                  string);
        }
      if (L_is_ctype_predicate_direct (ctype))
        {
          ptype = L_ptype_id (string);
          if (ptype == -1)
            L_punt ("L_read_operand_binary: illegal (macro ptype) <%s>",
                    string);
        }
      else
        ptype = L_PTYPE_NULL;
      operand = L_new_macro_operand (ival, ctype, ptype);
      break;
    default:
      L_punt
        ("L_read_operand_binary: Unknown operand type <%c> in input file\n",
         ch);
      return (NULL);
    }
  return (operand);
}

void
L_print_operand_binary (FILE * F, L_Operand * opd)
{

  /*
   * Print a one letter mnemonic representing the
   * operand type
   */
  switch (opd->type)
    {
    case L_OPERAND_CB:
      putc (DELIMIT | 'c', F);
      L_binary_write_int (F, opd->value.cb->id);
      break;
    case L_OPERAND_REGISTER:
      putc (DELIMIT | 'r', F);
      L_binary_write_int (F, opd->value.r);
      if (!L_is_ctype_predicate (opd))
        {
          if (L_output_obsolete_ctype_format)
            L_binary_write_string (F,
                                   L_ctype_name (L_return_old_ctype (opd)));
          else
            L_binary_write_string (F, L_ctype_name (opd->ctype));
        }
      else
        L_binary_write_string (F, L_ptype_name (opd->ptype));
      break;
    case L_OPERAND_RREGISTER:
      putc (DELIMIT | 'R', F);
      L_binary_write_int (F, opd->value.rr);
      if (!L_is_ctype_predicate (opd))
        {
          if (L_output_obsolete_ctype_format)
            L_binary_write_string (F,
                                   L_ctype_name (L_return_old_ctype (opd)));
          else
            L_binary_write_string (F, L_ctype_name (opd->ctype));
        }
      else
        L_binary_write_string (F, L_ptype_name (opd->ptype));
      break;
    case L_OPERAND_EVR:
      putc (DELIMIT | 'e', F);
      L_binary_write_int (F, opd->value.evr.num);
      L_binary_write_int (F, opd->value.evr.omega);
      if (!L_is_ctype_predicate (opd))
        {
          if (L_output_obsolete_ctype_format)
            L_binary_write_string (F,
                                   L_ctype_name (L_return_old_ctype (opd)));
          else
            L_binary_write_string (F, L_ctype_name (opd->ctype));
        }
      else
        L_binary_write_string (F, L_ptype_name (opd->ptype));
      break;
    case L_OPERAND_LABEL:
      if (L_output_obsolete_ctype_format)
        putc (DELIMIT | 'l', F);
      else
        {
          switch (opd->ctype)
            {
            case L_CTYPE_LOCAL_ABS:
              putc (DELIMIT | '1', F);
              break;
            case L_CTYPE_LOCAL_GP:
              putc (DELIMIT | '2', F);
              break;
            case L_CTYPE_GLOBAL_ABS:
              putc (DELIMIT | '3', F);
              break;
            case L_CTYPE_GLOBAL_GP:
              putc (DELIMIT | '4', F);
              break;
            default:
              L_warn ("L_print_operand_binary: illegal ctype %d", opd->ctype);
              break;
            }
        }
      L_binary_write_string (F, opd->value.l);
      break;
    case L_OPERAND_IMMED:
      if (L_is_ctype_integer (opd))
        {
          if (L_output_obsolete_ctype_format)
            putc (DELIMIT | 'i', F);
          else
            {
              switch (opd->ctype)
                {
                case L_CTYPE_CHAR:
                  putc (DELIMIT | 'C', F);
                  break;
                case L_CTYPE_UCHAR:
                  putc (DELIMIT | 'D', F);
                  break;
                case L_CTYPE_SHORT:
                  putc (DELIMIT | 'S', F);
                  break;
                case L_CTYPE_USHORT:
                  putc (DELIMIT | 'T', F);
                  break;
                case L_CTYPE_INT:
                  putc (DELIMIT | 'i', F);
                  break;
                case L_CTYPE_UINT:
                  putc (DELIMIT | 'J', F);
                  break;
                case L_CTYPE_LONG:
                  putc (DELIMIT | 'L', F);
                  break;
                case L_CTYPE_ULONG:
                  putc (DELIMIT | 'M', F);
                  break;
                case L_CTYPE_LLONG:
                  putc (DELIMIT | 'W', F);
                  break;
                case L_CTYPE_ULLONG:
                  putc (DELIMIT | 'X', F);
                  break;
                case L_CTYPE_LLLONG:
                  putc (DELIMIT | 'Y', F);
                  break;
                case L_CTYPE_ULLLONG:
                  putc (DELIMIT | 'Z', F);
                  break;
                case L_CTYPE_POINTER:
                  putc (DELIMIT | 'P', F);
                  break;
                default:
                  L_warn ("L_print_operand_binary: Illegal ctype %d",
                          opd->ctype);
                  break;
                }
            }
          L_binary_write_int (F, opd->value.i);
        }
      if (L_is_ctype_flt (opd))
        {
          putc (DELIMIT | 'f', F);
          L_binary_write_float (F, opd->value.f);
        }
      if (L_is_ctype_dbl (opd))
        {
          putc (DELIMIT | 'd', F);
          L_binary_write_double (F, opd->value.f2);
        }
      break;
    case L_OPERAND_STRING:
      if (L_output_obsolete_ctype_format)
        putc (DELIMIT | 's', F);
      else
        {
          switch (opd->ctype)
            {
            case L_CTYPE_LOCAL_ABS:
              putc (DELIMIT | '5', F);
              break;
            case L_CTYPE_LOCAL_GP:
              putc (DELIMIT | '6', F);
              break;
            case L_CTYPE_GLOBAL_ABS:
              putc (DELIMIT | '7', F);
              break;
            case L_CTYPE_GLOBAL_GP:
              putc (DELIMIT | '8', F);
              break;
            default:
              L_warn ("L_print_operand_binary: Illegal string ctype %d",
                      opd->ctype);
              break;
            }
        }
      L_binary_write_string (F, opd->value.s);
      break;
    case L_OPERAND_MACRO:
      putc (DELIMIT | 'm', F);
      L_binary_write_string (F, L_macro_name (opd->value.mac));
      if (!L_is_ctype_predicate (opd))
        L_binary_write_string (F, L_ctype_name (opd->ctype));
      else
        L_binary_write_string (F, L_ptype_name (opd->ptype));
      break;
    default:
      L_punt ("L_print_operand_binary: Unknown operand type %d\n", opd->type);
    }
}

L_Flow *
L_read_flow_binary (FILE * F, L_Input_Buf * buf, L_Func * fn, L_Cb * src_cb)
{
  char *weight_string;
  int cc, dest;
  double weight;
  L_Cb *dest_cb;

  cc = L_binary_read_int (F);
  dest = L_binary_read_int (F);
  weight_string = L_binary_read_string (F, buf);

  if (L_strtod (weight_string, NULL, &weight) == 0)
    {
      L_punt ("L_read_flow: "
              "The flow weight %s is not a valid floating-point value.",
              weight_string);
    }

  dest_cb = L_cb_hash_tbl_find_and_alloc (fn->cb_hash_tbl, dest);

  return (L_new_flow (cc, src_cb, dest_cb, weight));
}

L_Attr *
L_read_attr_binary (FILE * F)
{
  int i;
  int attr_name_id, opd_id;
  int max_field;
  L_Attr *attr;

  /* Get the name id and the number of fields in this attribute. */
  attr_name_id = L_binary_read_int (F);
  max_field = L_binary_read_int (F);

  attr = L_new_attr (L_string_map[attr_name_id], max_field);
  for (i = 0; i < max_field; i++)
    {
      opd_id = L_binary_read_int (F);

      /* Find the operand.  If the operand has already been */
      /* used, copy it.  If the operand has not been used   */
      /* simply assign a pointer to it and note that it is  */
      /* now used.                                          */
      if (L_operand_used[opd_id])
        attr->field[i] = L_copy_operand (L_operand_map[opd_id]);
      else
        {
          attr->field[i] = L_operand_map[opd_id];
          L_operand_used[opd_id] = 1;
        }
    }
  return (attr);
}

void
L_print_attr_binary (FILE * F, L_Attr * attr)
{
  int i;

  /* Output attribute name and number of operands */

#ifdef LP64_ARCHITECTURE
  L_binary_write_int (F,
                      (int)((long)STRING_find_symbol_data (L_func_string_table,
							   attr->name)));
#else
  L_binary_write_int (F,
                      (int) STRING_find_symbol_data (L_func_string_table,
                                                     attr->name));
#endif

  L_binary_write_int (F, attr->max_field);

  /* Output the operand id for each attribute field */
  for (i = 0; i < attr->max_field; i++)
    L_print_operand_id (F, attr->field[i]);
}

/*
 * The following functions read/write an Lcode oper using the
 * following format:
 *
 *  id|opcode|#src|src|#dest|dest|flags|#pred|pred|attr|sync
 *
 *  The order was chosen to minimize the amount of information
 *  required to represent the operation.  If the trailing items
 *  are empty for the oper nothing is printed, i.e. if an oper
 *  contains no predicates, attributes or sync arcs, then it 
 *  appears in the file as:
 *
 *  id|opcode|#src|src|#dest|dest|flags
 *
 *  The end of the oper is conveyed by setting the SP_CHAR bit
 *  in the id of the next oper or cb.  When an integer is found
 *  with the SP_CHAR bit set, the oper is complete.
 */
L_Oper *
L_read_rest_oper_binary (FILE * F, L_Input_Buf * buf, L_Oper * oper)
{
  unsigned char ch;
  int i;
  int opcode_string_id, flag_string_id;
  int num_src, num_dest, num_pred;
  int num_attr, num_sync;
  L_Attr *attr;

  /* Read opcode string id */
  opcode_string_id = L_binary_read_int (F);
  oper->opc = L_opcode_id (L_string_map[opcode_string_id]);
  oper->proc_opc = oper->opc;   /* Temporarily set proc_opc to */
  /* the opc until a proc_opc    */
  /* attribute is found          */
  oper->opcode = L_opcode_name (oper->opc);

  if (L_opc_vestigial (oper->opc))
    L_convert_to_com (oper);
  else if (L_general_pred_comparison_opcode (oper) ||
           L_cond_branch_opcode (oper) || L_general_comparison_opcode (oper))
    {
      opcode_string_id = L_binary_read_int (F);
      oper->com[0] = (ITint8) opcode_string_id;

      opcode_string_id = L_binary_read_int (F);
      oper->com[1] = (ITint8) opcode_string_id;
    }

  /* If the operation has no flags, operands, syncs or attributes */
  /* the next character will be either and oper id or a cb id     */
  /* the SP_CHAR bit set. Return the oper                         */
  if ((ch = L_binary_peek_next_char (F)) & SP_CHAR)
    {
      return (oper);
    }

  /* Read operation source operands */
  num_src = L_binary_read_int (F);
  for (i = 0; i < num_src; i++)
    {
      int opd_id = L_binary_read_int (F);
      /* Find the operand.  If the operand has already been */
      /* used, copy it.  If the operand has not been used   */
      /* simply assign a pointer to it and note that it is  */
      /* now used.                                          */
      if (L_operand_used[opd_id])
        oper->src[i] = L_copy_operand (L_operand_map[opd_id]);
      else
        {
          oper->src[i] = L_operand_map[opd_id];
          L_operand_used[opd_id] = 1;
        }
    }

  if ((ch = L_binary_peek_next_char (F)) & SP_CHAR)
    {
      return (oper);
    }

  /* Read operation dest operands */
  num_dest = L_binary_read_int (F);
  for (i = 0; i < num_dest; i++)
    {
      int opd_id = L_binary_read_int (F);
      /* Find the operand.  If the operand has already been */
      /* used, copy it.  If the operand has not been used   */
      /* simply assign a pointer to it and note that it is  */
      /* now used.                                          */
      if (L_operand_used[opd_id])
        oper->dest[i] = L_copy_operand (L_operand_map[opd_id]);
      else
        {
          oper->dest[i] = L_operand_map[opd_id];
          L_operand_used[opd_id] = 1;
        }
    }

  if ((ch = L_binary_peek_next_char (F)) & SP_CHAR)
    {
      return (oper);
    }

  /* Get oper flags */
  flag_string_id = L_binary_read_int (F);
  if (flag_string_id != 0)
    {
      char *flag_string = L_string_map[flag_string_id];
      oper->flags = L_oper_flags_string_to_int (flag_string);
    }

  if ((ch = L_binary_peek_next_char (F)) & SP_CHAR)
    {
      return (oper);
    }

  /* Read operation predicate operands */
  num_pred = L_binary_read_int (F);
  for (i = 0; i < num_pred; i++)
    {
      int opd_id = L_binary_read_int (F);
      /* Find the operand.  If the operand has already been */
      /* used, copy it.  If the operand has not been used   */
      /* simply assign a pointer to it and note that it is  */
      /* now used.                                          */
      if (L_operand_used[opd_id])
        oper->pred[i] = L_copy_operand (L_operand_map[opd_id]);
      else
        {
          oper->pred[i] = L_operand_map[opd_id];
          L_operand_used[opd_id] = 1;
        }
    }

  if ((ch = L_binary_peek_next_char (F)) & SP_CHAR)
    {
      return (oper);
    }
  num_attr = L_binary_read_int (F);
  for (i = 0; i < num_attr; i++)
    {
      attr = L_read_attr_binary (F);
      oper->attr = L_concat_attr (oper->attr, attr);
    }

  /*
   * Search for any processor specific opcode.  If one exists, it
   * will be set as the proc_opc.  Otherwise the current opc will
   * be set as the proc_opc
   */
  /* 09/24/02 REK Updating to read the completers field along with the
   *              proc_opc field. */
  attr = L_find_attr (oper->attr, "popc");
  if (attr != NULL)
  {
    oper->proc_opc = (int) attr->field[0]->value.i;
    oper->completers = (int) attr->field[1]->value.i;
  }
  else
  {
    oper->proc_opc = oper->opc;
  }

  if ((ch = L_binary_peek_next_char (F)) & SP_CHAR)
    {
      return (oper);
    }
  num_sync = L_binary_read_int (F);
  for (i = 0; i < num_sync; i++)
    {
      L_Sync *head_sync, *tail_sync;

      tail_sync = L_read_sync_binary (F, buf);

      L_insert_tail_sync_in_oper (oper, tail_sync);

      head_sync = L_copy_sync (tail_sync);
      head_sync->dep_oper = oper;
      L_insert_head_sync_in_oper (tail_sync->dep_oper, head_sync);
    }

  return (oper);
}
L_Oper *
L_read_oper_binary (FILE * F, L_Input_Buf * buf, L_Func * fn)
{
  int op_id, ch;
  L_Oper *oper;

  /* Next character should be an operation id with the */
  /* special character bit set.                        */
  ch = L_binary_peek_next_char (F);
  if (!(ch & SP_CHAR))
    L_punt ("L_read_oper_binary: Binary file contains corrupt oper!\n");

  op_id = L_binary_read_int (F);

  /* DMG - 7/94 - add tbl lookup while reading */
  oper = L_oper_hash_tbl_find_and_alloc_oper (fn->oper_hash_tbl, op_id);

  L_read_rest_oper_binary (F, buf, oper);
  return (oper);
}


L_Oper *
L_read_parent_oper_binary (FILE * F, L_Input_Buf * buf)
{
  int op_id, ch;
  L_Oper *oper;

  /* Next character better be a MINUS */
  ch = L_binary_read_char (F);
  if ((ch != (DELIMIT | MINUS)))
    L_punt
      ("L_read_oper_binary: Binary file corrupt, expected parent oper!\n");

  /* Next character should be an operation id with the */
  /* special character bit set.                        */
  ch = L_binary_peek_next_char (F);
  if (!(ch & SP_CHAR))
    L_punt ("L_read_parent_oper_binary: "
            "Binary file contains corrupt parent oper!\n");

  op_id = L_binary_read_int (F);
  oper = L_new_parent_oper (op_id);

  L_read_rest_oper_binary (F, buf, oper);
  return (oper);
}

void
L_print_oper_binary (FILE * F, L_Oper * oper)
{
  int i;
  int num_attr;
  int num_pred;
  int num_dest;
  int num_src;
  int num_sync;
  int num_flags;
  char flag_string[33];

  L_Attr *attr;

  /* Build operation flag string */
  num_flags = L_oper_flags_to_string (flag_string, oper->flags);
  /*
   *  Determine number of predicate operands
   */
  num_pred = L_max_pred_operand;
  if (oper->pred[L_max_pred_operand] != NULL)
    L_punt ("L_print_oper_binary: too many predicates defined (op=%d)",
            oper->id);
  for (i = L_max_pred_operand - 1; i >= 0; i--)
    {
      if (oper->pred[i] != NULL)
        break;
      num_pred -= 1;
    }
  /*
   *  Determine number of destination operands
   */
  num_dest = L_max_dest_operand;
  if (oper->dest[L_max_dest_operand] != NULL)
    L_punt ("L_print_rest_oper: too many destinations defined (op=%d)",
            oper->id);
  for (i = L_max_dest_operand - 1; i >= 0; i--)
    {
      if (oper->dest[i] != NULL)
        break;
      num_dest -= 1;
    }
  /*
   *  Determine number of source operands
   */
  num_src = L_max_src_operand;
  if (oper->src[L_max_src_operand] != NULL)
    L_punt ("L_print_rest_oper: too many sources defined (op=%d)", oper->id);
  for (i = L_max_src_operand - 1; i >= 0; i--)
    {
      if (oper->src[i] != NULL)
        break;
      num_src -= 1;
    }

  /* Determine number of sync arcs */
  if (oper->sync_info != NULL)
    num_sync = oper->sync_info->num_sync_in;
  else
    num_sync = 0;

  /* REH 4-30-96 - This must also be done ahead of time while */
  /* the function is being scanned for operands to build the  */
  /* operand hash table.                                      */
  /*
   * Search for any processor specific opcode.  If one exists, it
   * will be set as the proc_opc.  Otherwise the current opc will
   * be set as the proc_opc
   */
  /* 09/24/02 REK Updating to write the completers field along with the
   *              proc_opc field. */
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

  /* Determine number of attributes */
  num_attr = 0;
  for (attr = oper->attr; attr != NULL; attr = attr->next_attr)
    num_attr += 1;

  /* Print oper id */
  L_binary_write_int_special (F, oper->id);

  /* Print opcode string id */
#ifdef LP64_ARCHITECTURE
  L_binary_write_int (F,
                      (int)((long)STRING_find_symbol_data (L_func_string_table,
							   oper->opcode)));
#else
  L_binary_write_int (F,
                      (int) STRING_find_symbol_data (L_func_string_table,
                                                     oper->opcode));
#endif

  if (L_general_pred_comparison_opcode (oper) ||
      L_cond_branch_opcode (oper) || L_general_comparison_opcode (oper))
    {
      L_binary_write_int (F, (int) oper->com[0]);
      L_binary_write_int (F, (int) oper->com[1]);
    }

  /* If there is no more information to write  */
  /* for the oper, stop.                       */
  if ((num_src + num_dest) +
      (num_pred + num_flags) + (num_attr + num_sync) == 0)
    {
      return;
    }

  /* Print Source Operands */
  L_binary_write_int (F, num_src);
  for (i = 0; i < num_src; i++)
    {
      L_print_operand_id (F, oper->src[i]);
    }

  /* If there is no more information to write  */
  /* for the oper, stop.                       */
  if (num_dest + (num_pred + num_flags) + (num_attr + num_sync) == 0)
    {
      return;
    }

  /* Print Destination operands */
  L_binary_write_int (F, num_dest);
  for (i = 0; i < num_dest; i++)
    {
      L_print_operand_id (F, oper->dest[i]);
    }

  /* If there is no more information to write  */
  /* for the oper, stop.                       */
  if ((num_pred + num_flags) + (num_attr + num_sync) == 0)
    {
      return;
    }

  /* Print Flags */
  if (num_flags != 0)
#ifdef LP64_ARCHITECTURE
    L_binary_write_int \
      (F, (int)((long)STRING_find_symbol_data (L_func_string_table,
					       flag_string)));
#else
    L_binary_write_int (F,
                        (int) STRING_find_symbol_data (L_func_string_table,
                                                       flag_string));
#endif
  else
    L_binary_write_int (F, 0);

  /* If there is no more information to write  */
  /* for the oper, stop.                       */
  if (num_pred + (num_attr + num_sync) == 0)
    {
      return;
    }

  /* Print predicate operands */
  L_binary_write_int (F, num_pred);
  for (i = 0; i < num_pred; i++)
    L_print_operand_id (F, oper->pred[i]);

  /* If there is no more information to write  */
  /* for the oper, stop.                       */
  if (num_attr + num_sync == 0)
    {
      return;
    }

  /* Print operation attributes */
  L_binary_write_int (F, num_attr);
  for (attr = oper->attr; attr != NULL; attr = attr->next_attr)
    L_print_attr_binary (F, attr);

  if (num_sync == 0)
    {
      return;
    }

  /* Print operation sync arcs */
  L_binary_write_int (F, num_sync);
  for (i = 0; i < num_sync; i++)
    L_print_sync_binary (F, oper->sync_info->sync_in[i]);

}

/*
 * The following functions read/write an Lcode cb using the
 * following format:
 *
 *  id|#flow|#attr|#oper|flows|attrs|opers
 *
 *  The SP_CHAR bit is set on the cb id to delimit the end 
 *  of a previous oper or cb, to aid in error detection.
 */
L_Cb *
L_read_cb_binary (FILE * F, L_Input_Buf * buf, L_Func * fn, int *num_opers)
{
  int i, cb_id, ch;
  int flag_string_id;
  int num_flow, num_attr;
  char *token;
  L_Cb *cb;
  double weight;

  /* Next character should be a cb id with the */
  /* special character bit set.                */
  ch = L_binary_peek_next_char (F);
  if (!(ch & SP_CHAR))
    L_punt ("L_read_oper_binary: Binary file corrupt -> cb id expected!\n");

  cb_id = L_binary_read_int (F);
  if (cb_id < 0)
    {
      L_punt ("L_read_cb: "
              "The id of the current cb <%d> is negative, this is illegal.",
              cb_id);
    }

  /* find if already allocated cb, else allocate new one */
  cb = L_cb_hash_tbl_find_and_alloc (fn->cb_hash_tbl, cb_id);

  /* Get cb flags */
  flag_string_id = L_binary_read_int (F);
  if (flag_string_id != 0)
    {
      char *flag_string = L_string_map[flag_string_id];
      cb->flags = L_cb_flags_string_to_int (flag_string);
    }

  /* Set the cb weight */
  token = L_binary_read_string (F, buf);
  weight = atof (token);
  cb->weight = weight;

  num_flow = L_binary_read_int (F);
  num_attr = L_binary_read_int (F);
  *num_opers = L_binary_read_int (F);

  for (i = 0; i < num_flow; i++)
    {
      L_Flow *flow = L_read_flow_binary (F, buf, fn, cb);
      cb->dest_flow = L_concat_flow (cb->dest_flow, flow);
    }

  for (i = 0; i < num_attr; i++)
    {
      L_Attr *attr = L_read_attr_binary (F);
      cb->attr = L_concat_attr (cb->attr, attr);
    }

  /* 
   * Need to do something about regions
   */

  return (cb);
}

void
L_print_cb_binary (FILE * F, L_Func * fn, L_Cb * cb)
{
  int num_flow;
  int num_oper;
  int num_attr;
  int num_flags;
  char flag_string[33];

  L_Attr *attr;
  L_Flow *flow;
  L_Oper *oper, *parent_op = NULL;

  /* Build cb flag string */
  num_flags = L_cb_flags_to_string (flag_string, cb->flags);

  /* Determine number of cb attributes */
  num_attr = 0;
  for (attr = cb->attr; attr != NULL; attr = attr->next_attr)
    num_attr += 1;

  /* Determine number of destination flow arcs */
  num_flow = 0;
  for (flow = cb->dest_flow; flow != NULL; flow = flow->next_flow)
    num_flow += 1;

  /* Determine number of operations */
  num_oper = 0;
  for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
    {
      num_oper += 1;
    }

  /* Print cb header information */
  L_binary_write_int_special (F, cb->id);

  /* Print cb flags */
  if (num_flags != 0)
#ifdef LP64_ARCHITECTURE
    L_binary_write_int \
      (F, (int)((long)STRING_find_symbol_data (L_func_string_table,
					       flag_string)));
#else
    L_binary_write_int (F,
                        (int) STRING_find_symbol_data (L_func_string_table,
                                                       flag_string));
#endif
  else
    L_binary_write_int (F, 0);

  L_binary_write_double (F, cb->weight);
  L_binary_write_int (F, num_flow);
  L_binary_write_int (F, num_attr);
  L_binary_write_int (F, num_oper);

  /* Print cb flow arcs */
  for (flow = cb->dest_flow; flow != NULL; flow = flow->next_flow)
    {
      L_binary_write_int (F, flow->cc);
      L_binary_write_int (F, flow->dst_cb->id);
      L_binary_write_double (F, flow->weight);
    }

  /* Print cb attributes */
  for (attr = cb->attr; attr != NULL; attr = attr->next_attr)
    L_print_attr_binary (F, attr);


  /* Output each operation in the control block */
  for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
    {
      if (!L_print_parent_op)
        {
          L_print_oper_binary (F, oper);
        }
      else
        {
          if ((oper->parent_op != parent_op) && (oper->parent_op != NULL))
            {
              putc (DELIMIT | MINUS, F);
              L_print_oper_binary (F, oper->parent_op);
              parent_op = oper->parent_op;
            }
          L_print_oper_binary (F, oper);
        }
    }

  /* 
   * Need to do something about regions
   */
  if (cb->region != NULL)
    L_punt
      ("L_print_cb_binary: Binary format does not support regions yet!\n");
}

/*
 * IF YOU CHANGE THE BEHAVIOR OF THIS FUNCTION, YOU MUST ALTER
 * L_read_fn_attributes() IN THE SAME MANNER!
 */
void
L_read_fn_attributes_binary (FILE * F, L_Input_Buf * buf, L_Func * fn,
                             int num_attr)
{
  int i;
  L_Attr *attr, *next_attr;

  for (i = 0; i < num_attr; i++)
    {
      attr = L_read_attr_binary (F);

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
  /*
   * Initialize some attributes for validation
   */
  L_func_contains_dep_pragmas = 0;
  L_func_contains_jsr_dep_pragmas = 0;

  for (attr = L_fn->attr; attr != NULL; attr = next_attr)
    {
      next_attr = attr->next_attr;

      if (!strncmp (attr->name, "ARCH:", 5))
        L_file_arch = L_add_string (L_string_table, attr->name);

      else if (!strncmp (attr->name, "MODEL:", 6))
        L_file_model = L_add_string (L_string_table, attr->name);

      else if (!strncmp (attr->name, "LMDES:", 6))
        L_file_lmdes = L_add_string (L_string_table, attr->name);

      else if (!strncmp (attr->name, "DEP_PRAGMAS", 11))
        L_func_contains_dep_pragmas = 1;

      else if (!strncmp (attr->name, "JSR_DEP_PRAGMAS", 11))
        L_func_contains_jsr_dep_pragmas = 1;

      else if (!strcmp (attr->name, "max_cb_id"))
        L_fn->max_cb_id = (int) attr->field[0]->value.i;
    }
}

/*
 * The following functions read/write an Lcode function using the
 * following format:
 *
 *  function|name|flags|weight|#str|#opd|#attr|#cb|str|opd|attr|cb
 *
 *  Before the body of the function (i.e. the cbs), there is a
 *  list of unique strings and a list of unique operands that
 *  occur within the function.  The ordering implies the index
 *  that is used by opers, operand, and attributes that reference
 *  those strings.
 */
void
L_read_fn_binary (FILE * F, L_Input_Buf * input_buf)
{
  char *token;
  int i, j;
  int num_strings;
  int num_operands;
  int num_attr;
  int num_cb;
  int num_opers;
  int flag_string_id;
  int num_jump_tbls;
  double weight;
  L_Cb *cb;
  L_Oper *oper;

  L_Time time;

  L_init_time (&time);
  L_start_time (&time);

  /*
   * This is used to prevent reporting STD_PARM errors
   * on files that only contain data segments.
   */
  L_func_read = 1;

  /* Get function name */
  token = L_binary_read_string (F, input_buf);
  L_fn = L_new_func (token, 0.0);

  /* Get function flags */
  flag_string_id = L_binary_read_int (F);

  /* Set the function weight */
  token = L_binary_read_string (F, input_buf);
  weight = atof (token);
  L_fn->weight = weight;

  num_strings = L_binary_read_int (F);
  num_operands = L_binary_read_int (F);
  num_attr = L_binary_read_int (F);
  num_cb = L_binary_read_int (F);

  /* Malloc string map.  String indices start   */
  /* with 1, so array is of size num_operands 1 */
  L_string_map = (char **) malloc (sizeof (char *) * (num_strings + 1));

  /* Construct the string map that will be referenced during  */
  /* reading of opers, operands and attributes.  The ordering */
  /* of the strings implies the id that is assigned to the    */
  /* string and corresponds to the id used by structures      */
  /* referencing that string.                                 */
  for (i = 1; i < num_strings + 1; i++)
    {
      token = L_binary_read_string (F, input_buf);
      L_string_map[i] = L_add_string (L_string_table, token);
    }

  /* Now that the strings are extracted, we can */
  /* set the function flags properly.           */
  if (flag_string_id != 0)
    {
      char *flag_string = L_string_map[flag_string_id];
      L_fn->flags = L_func_flags_string_to_int (flag_string);
    }

  /* Malloc operand map and usage table,  operand indices start */
  /* with 1, so arrays are of size num_operands + 1             */
  L_operand_used =
    (unsigned char *) malloc (sizeof (unsigned char) * (num_operands + 1));
  L_operand_map =
    (L_Operand **) malloc (sizeof (L_Operand *) * (num_operands + 1));

  /* Construct the operand map that will be referenced during */
  /* reading of opers and attributes.  The ordering of the    */
  /* operands implies the id that is assigned to the operand  */
  /* and corresponds to the id used by structures referencing */
  /* that operand.                                            */
  L_operand_used[0] = 0;
  L_operand_map[0] = NULL;
  for (i = 1; i < num_operands + 1; i++)
    {
      L_operand_used[i] = 0;
      L_operand_map[i] = L_read_operand_binary (F, input_buf, L_fn);
    }

  /* Get all function attributes */
  if (num_attr != 0)
    L_read_fn_attributes_binary (F, input_buf, L_fn, num_attr);

  /* Read the body of the function */
  for (i = 0; i < num_cb; i++)
    {
      cb = L_read_cb_binary (F, input_buf, L_fn, &num_opers);

      L_insert_cb_after (L_fn, L_fn->last_cb, cb);

      j = 0;
      while (j < num_opers)
        {

          /* 
           * Parent operations start with a MINUS character.
           */
          if (L_binary_peek_next_char (F) == (DELIMIT | MINUS))
            {
              oper = L_read_parent_oper_binary (F, input_buf);

              /* Mark this oper as a parent */
              oper->flags = L_SET_BIT_FLAG (oper->flags, L_OPER_PARENT);

              /* 
               * Link the oper into the parent oper list to ensure correct
               * freeing of the oper when the function is released.
               */
              oper->next_op = L_fn->last_parent_op;
              L_fn->last_parent_op = oper;
            }
          else
            {
              oper = L_read_oper_binary (F, input_buf, L_fn);
              /* Link to the currently defined parent Lcode oper */
              oper->parent_op = L_fn->last_parent_op;

              /* REH 7/8/95 - Set Cb prologue/epilogue flags */
              if (oper->opc == Lop_PROLOGUE)
                cb->flags = L_SET_BIT_FLAG (cb->flags, L_CB_PROLOGUE);
              else if (oper->opc == Lop_EPILOGUE)
                cb->flags = L_SET_BIT_FLAG (cb->flags, L_CB_EPILOGUE);

              L_insert_oper_after (cb, cb->last_op, oper);

              j++;
            }
        }
    }

  /* At this point we better read in the function terminator "end" */
  token = L_binary_read_string (F, input_buf);
  if (L_lcode_id (token) != L_INPUT_END)
    L_punt
      ("L_read_func_binary:  Corrupt binary file, end of function expected\n");

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
                      if (!L_is_int_constant (oper->src[0]))
                        L_punt ("L_read_fn: "
                                "src1 of (define $_local) must be integer");
                      value = (int) oper->src[0]->value.i;
                      L_fn->s_local = value;
                      break;
                    case L_MAC_PARAM_SIZE:
                      if (!L_is_int_constant (oper->src[0]))
                        L_punt ("L_read_fn: "
                                "src1 of (define $_param) must be integer");
                      value = (int) oper->src[0]->value.i;
                      L_fn->s_param = value;
                      break;
                    case L_MAC_SWAP_SIZE:
                      if (!L_is_int_constant (oper->src[0]))
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

#ifdef NEW_JUMP_TBL_STUFF
  /* SAM 7-96: new jump tbl handling calls */

  /* Renaming is for compatibility of older Lcode files with new tbl naming */
  if (L_func_needs_jump_table_renaming (L_fn))
    L_rename_jump_table_labels (L_fn);
  if (!L_func_has_jump_table_info (L_fn))
    L_setup_jump_table_info (L_fn);
  num_jump_tbls = L_num_jump_tables (L_fn);
  if (num_jump_tbls > 0)
    L_fn->jump_tbls =
      L_read_all_hashtbls_binary (F, num_jump_tbls, input_buf);
#endif

  /* Free all the maps allocated for reading this function -JCG 2/25/97 */
  free (L_string_map);
  L_string_map = NULL;

  free (L_operand_used);
  L_operand_used = NULL;

  free (L_operand_map);
  L_operand_map = NULL;

  L_stop_time (&time);
  /*
     fprintf(stderr,"Binary read time %gs\n",L_final_time(&time));
   */
}

void
L_print_func_binary (FILE * F, L_Func * fn)
{
  int num_cb;
  int num_attr;
  int num_flags;
  char flag_string[L_FUNC_MAX_FLAGS + 1];
  L_Cb *cb;
  L_Attr *attr;
  STRING_Symbol *sym;
  int num_strings;

  OPERAND_Symbol *opd_sym;
  int num_operands;

  L_Time time;

  L_init_time (&time);
  L_start_time (&time);

#ifdef NEW_JUMP_TBL_STUFF
  /* Need to regenerate hash tables before printing function's attributes */
  if (L_jump_tables_have_changes (fn))
    L_regenerate_all_jump_tables (fn);
#endif

  L_operand_table = OPERAND_new_symbol_table ("L_Operand Symbol Table", 2048);
  L_func_string_table =
    STRING_new_symbol_table ("Function String Table", 256);

  /* Construct the hash tables to determine the unique */
  /* strings and operands that exist within the body   */
  /* of the function.                                  */
  num_strings = num_operands = 1;
  L_build_operand_hash_table (fn, &num_operands, &num_strings);
  num_operands -= 1;
  num_strings -= 1;

  /* Build function flag string */
  num_flags = L_func_flags_to_string (flag_string, fn->flags);

  /* Determine number of function attributes */
  num_attr = 0;
  for (attr = fn->attr; attr != NULL; attr = attr->next_attr)
    num_attr += 1;

  /* Determine number of cb's */
  num_cb = 0;
  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    num_cb += 1;

  /*
   *  Print function header
   */
  L_binary_write_string (F, "function");
  L_binary_write_string (F, fn->name);

  /* Print function flags */
  if (num_flags != 0)
#ifdef LP64_ARCHITECTURE
    L_binary_write_int \
      (F, (int)((long)STRING_find_symbol_data (L_func_string_table,
					       flag_string)));
#else
    L_binary_write_int (F,
                        (int) STRING_find_symbol_data (L_func_string_table,
                                                       flag_string));
#endif
  else
    L_binary_write_int (F, 0);

  L_binary_write_double (F, fn->weight);
  L_binary_write_int (F, num_strings);
  L_binary_write_int (F, num_operands);
  L_binary_write_int (F, num_attr);
  L_binary_write_int (F, num_cb);

  /* Print function string table */
  for (sym = L_func_string_table->head_symbol;
       sym != NULL; sym = sym->next_symbol)
    {
      L_binary_write_string (F, sym->name);
    }

  /* Print function operand table */
  for (opd_sym = L_operand_table->head_symbol;
       opd_sym != NULL; opd_sym = opd_sym->next_symbol)
    L_print_operand_binary (F, (L_Operand *) opd_sym->value);

  /* Output function attributes */
  for (attr = fn->attr; attr != NULL; attr = attr->next_attr)
    L_print_attr_binary (F, attr);

  /* Output function cb's */
  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    L_print_cb_binary (F, fn, cb);

  /* Terminate function */
  L_binary_write_string (F, "end");

#ifdef NEW_JUMP_TBL_STUFF
  /* SAM: 7-96: new jump tbl stuff */
  if (fn->jump_tbls != NULL)
    L_print_datalist_binary (F, fn->jump_tbls);
#endif

  /* Free up the string and operand hash tables */
  STRING_delete_symbol_table (L_func_string_table, NULL);
  OPERAND_delete_symbol_table (L_operand_table, NULL);

  L_stop_time (&time);
  /*
     fprintf(stderr,"Binary print time %gs\n",L_final_time(&time));
   */
}
