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
 *      File:   l_build_prototype_info.c
 *
 *      Usage: Expects all the lcode for the ENTIRE program to be fed in
 *             in a input (i.e., cat *.lc | Lbuild_prototype_info).
 *             Determines a "reasonable" prototype for every function called
 *             or defined in the program.  Outputs this prototype info
 *             in a 'Hcode' format for use by NYU's Elcor emulator.
 *
 *             Initially written to be very conservative, in that, it
 *             will only assume varargs functions if absolutely necessary
 *             (printf will only called a varargs function if the parameters
 *              vary in such a way to flag this to this program).
 *             A list of known varargs functions is then used to place
 *             the varargs argument in the proper place (for those deduced
 *             to be varargs).
 * 
 *             Mismatched pointers will be converted to 'void *' pointers
 *             in prototypes.  
 * 
 *             Mismatched integer types (char, short, int, long) will be 
 *             promoted to to the 'largest' type encountered for a 
 *             particular argument.
 *
 *             If desired, it would not be difficult to make this
 *             aggressively make varargs functions, all pointers void *,
 *             etc., but it seemed that conservativeness was a better
 *             initial choice.
 *   
 *
 *      Authors: John C. Gyllenhaal, Wen-mei Hwu
 *      Creation Date:  May 1998
 *
 *      Enhanced to create libproto.a and to support K&R-C output -ITI/JCG 3/99
 *      Enhanced to handle the corner cases in SPEC 92/95 and
 *      to have the functionality Lemulate needs -ITI/JCG 3/99
 *
\*****************************************************************************/

#include <config.h>
#include <stdio.h>
#include "l_build_prototype_info.h"
#include <library/dynamic_symbol.h>


/* For ease of writing this thing, it is assumed that there is a maximum
 * prototype size.  The front end assumes 8k (as of 5/22/98), 
 # so TYPE_BUF_SIZE should be at least 8k.  This code will punt if
 * this is not large enough.
 */
#define TYPE_BUF_SIZE   8100

/* The info kept on each function in the various tables */
typedef struct
{
  char *info_string;         /* Call info string */
  int diff_index;            /* -1 normally, otherwise the parm that varies */
  int use_old_style;         /* When 1, must emit K&R-style prototype */
}
Call_Info;

/* Prototype table holds the final prototypes generated */
static STRING_Symbol_Table *prototype_table = NULL;

/* Function table holds prototype info for functions we have source for */
static STRING_Symbol_Table *func_table = NULL;

/* JSR table holds deduced prototype info from all the jsr calls */
static STRING_Symbol_Table *jsr_table = NULL;

/* Varargs table holds the parameter index of the varargs parameter
 # for all known varargs function.  Only used when the jsr_table info
 # indicates the function must be a varargs function.
 */
static STRING_Symbol_Table *varargs_table = NULL;

/* Some functions (notably builtins) should not have prototypes. */
static STRING_Symbol_Table *ignore_table = NULL;

/* Useful parser of info strings */
extern void L_get_next_param_type (char *buf, char **info_ptr);


/* Converts part of the type 'postfix' (the stuff after the '+') into
 * Hcode format.  Recursively handles each subtype.  Recursiveness not
 * necessary but used because this is based on the C format conversion 
 * routine which required it.
 * (It also offers flexibility if necessary later).
 *
 * NOTE: This functions parameters MUST NOT overlap (be the same buffer)!
 */
void
L_subtype_to_hcode_postfix (char *dest_buf, char *type_buf,
                            char *incoming_buf)
{
  char temp_buf[TYPE_BUF_SIZE], *type_ptr, *temp_ptr;
  char sub_buf[TYPE_BUF_SIZE];

  /* Point at type_buf for parsing */
  type_ptr = type_buf;

  /* Clear  dest_buf initially */
  dest_buf[0] = 0;

  /* If nothing left to parse, just return incoming buf */
  if (*type_ptr == 0)
    {
      strcpy (dest_buf, incoming_buf);
    }

  /* Handle array types (first index unspecified).
   * Converts 'PA10A20' into '(A) (A (signed 10)) (A (signed 20))'
   */
  else if (type_ptr[0] == 'P' && type_ptr[1] == 'A')
    {
      /* Specify 'array with unspecified index' */
      sprintf (sub_buf, "(A) ");

      /* Skip the P */
      type_ptr++;

      /* Process all the Array specifications */
      while (type_ptr[0] == 'A')
        {
          /* Skip the A */
          type_ptr++;

          /* Print out the first part for this array index */
          strcat (sub_buf, "(A (signed ");

          /* Copy number into temp buf */
          temp_ptr = temp_buf;
          while ((*type_ptr != 0) && isdigit (*type_ptr))
            {
              *temp_ptr = *type_ptr;
              temp_ptr++;
              type_ptr++;
            }
          /* Add closing )) after number */
          temp_ptr[0] = ')';
          temp_ptr[1] = ')';
          temp_ptr[2] = 0;

          /* Copy to format string */
          strcat (sub_buf, temp_buf);

          /* Add a space if there is another array index */
          if (*type_ptr == 'A')
            strcat (sub_buf, " ");
        }

      /* Add to end of any incoming info (and add space) */
      if (incoming_buf[0] != 0)
        {
          strcpy (temp_buf, sub_buf);
          sprintf (sub_buf, "%s %s", incoming_buf, temp_buf);
        }

      /* Convert the rest of the subtype recursively */
      L_subtype_to_hcode_postfix (dest_buf, type_ptr, sub_buf);
    }

  /* Handle function pointers. 
   * Converts 'PF' into 'P (F)' 
   */
  else if ((type_ptr[0] == 'F') ||
           ((type_ptr[0] == 'P') && (type_ptr[1] == 'F')))
    {
      /* Consume optional leading P (but should always be there anyways) */
      if (type_ptr[0] == 'P')
        type_ptr++;

      /* Skip the F */
      type_ptr++;

      /* Tack this on the end of any incoming values.
       * Don't add space if no incoming value.
       */
      if (incoming_buf[0] != 0)
        sprintf (sub_buf, "%s P (F)", incoming_buf);
      else
        sprintf (sub_buf, "P (F)");

      /* Convert the rest of the subtype recursively */
      L_subtype_to_hcode_postfix (dest_buf, type_ptr, sub_buf);
    }

  /* Handle normal pointers */
  else if ((type_ptr[0] == 'P') && (type_ptr[1] != 'F'))
    {
      /* Skip P */
      type_ptr++;

      /* If at end of type string, just add 'P' */
      if (*incoming_buf == 0)
        sprintf (sub_buf, "P");

      /* Otherwise, add ' P' to end of incoming type buf */
      else
        sprintf (sub_buf, "%s P", incoming_buf);


      /* Convert the rest of the subtype recursively */
      L_subtype_to_hcode_postfix (dest_buf, type_ptr, sub_buf);
    }
  /* Otherwise, punt since I don't know what this is */
  else
    {
      L_punt ("L_subtype_to_hcode_postfix: unexected modifier '%s' in '%s'.",
              type_ptr, type_buf);

    }
}

/* Convert the type postfix into hcode format.  
 * Basically finds the stuff after the '+' and hands it 
 * off to L_subtype_to_hcode_postfix() above.
 *
 * NOTE: This functions parameters MUST NOT overlap (be the same buffer)!
 */
void
L_format_hcode_type_postfix (char *formatted_buf, char *raw_buf)
{
  char *raw_ptr;

  /* Sanity check, raw_buf should not be NULL or empty */
  if ((raw_buf == NULL) || (*raw_buf == 0))
    L_punt ("L_hcode_type_postfix: raw_buf empty or NULL!");

  /* Skip to postfix (after '+', if any) */
  raw_ptr = raw_buf;
  while ((*raw_ptr != 0) && (*raw_ptr != '+'))
    {
      raw_ptr++;
    }

  /* Move raw_ptr past '+', if exists */
  if (*raw_ptr == '+')
    raw_ptr++;

  /* Handle the rest of the type info */
  L_subtype_to_hcode_postfix (formatted_buf, raw_ptr, "");

  /* Done, have formatted string now */
}

/* Converts the type prefix (the stuff before the '+') into
 * hcode format.
 *
 * NOTE: This functions parameters MUST NOT overlap (be the same buffer)!
 */
void
L_format_hcode_type_prefix (char *formatted_buf, char *raw_buf)
{
  char temp_buf[TYPE_BUF_SIZE], *temp_ptr, *raw_ptr;

  /* Sanity check, raw_buf should not be NULL or empty */
  if ((raw_buf == NULL) || (*raw_buf == 0))
    L_punt ("L_format_hcode_type_prefix: raw_buf empty or NULL!");

  /* Copy type_name into temp_buf (string before '+') */
  temp_ptr = temp_buf;
  raw_ptr = raw_buf;
  while ((*raw_ptr != 0) && (*raw_ptr != '+'))
    {
      *temp_ptr = *raw_ptr;
      temp_ptr++;
      raw_ptr++;
    }

  /* Terminate temp_buf */
  *temp_ptr = 0;

  /* Move raw_ptr past '+', if exists */
  if (*raw_ptr == '+')
    raw_ptr++;

  /* Handle struct names */
  if ((temp_buf[0] == 'S') && (temp_buf[1] == '_'))
    {
      sprintf (formatted_buf, "STRUCT %s", &temp_buf[2]);
    }

  /* Handle union names */
  else if ((temp_buf[0] == 'U') && (temp_buf[1] == '_'))
    {
      sprintf (formatted_buf, "UNION %s", &temp_buf[2]);
    }
  /* Handle vararg type */
  else if (strcmp (temp_buf, "vararg") == 0)
    {
      sprintf (formatted_buf, "VARARG");
      /* No function name, so return now */
      return;
    }

  /* Handle all the other type types */
  else if (strcmp (temp_buf, "void") == 0)
    sprintf (formatted_buf, "VOID");

  else if (strcmp (temp_buf, "char") == 0)
    sprintf (formatted_buf, "CHAR");

  else if (strcmp (temp_buf, "uchar") == 0)
    sprintf (formatted_buf, "UNSIGNED)(CHAR");

  else if (strcmp (temp_buf, "short") == 0)
    sprintf (formatted_buf, "SHORT");

  else if (strcmp (temp_buf, "ushort") == 0)
    sprintf (formatted_buf, "UNSIGNED)(SHORT");

  else if (strcmp (temp_buf, "int") == 0)
    sprintf (formatted_buf, "INT");

  else if (strcmp (temp_buf, "uint") == 0)
    sprintf (formatted_buf, "UNSIGNED)(INT");

  else if (strcmp (temp_buf, "long") == 0)
    sprintf (formatted_buf, "LONG");

  else if (strcmp (temp_buf, "ulong") == 0)
    sprintf (formatted_buf, "UNSIGNED)(LONG");

  else if (strcmp (temp_buf, "longlong") == 0)
    sprintf (formatted_buf, "LONGLONG");

  else if (strcmp (temp_buf, "ulonglong") == 0)
    sprintf (formatted_buf, "UNSIGNED)(LONGLONG");

  else if (strcmp (temp_buf, "float") == 0)
    sprintf (formatted_buf, "FLOAT");

  else if (strcmp (temp_buf, "double") == 0)
    sprintf (formatted_buf, "DOUBLE");


  /* Otherwise, punt on unknown type */
  else
    {
      L_punt ("L_format_hcode_type_prefix: Unknown type '%s' in '%s'"
              " string!", temp_buf, raw_buf);
    }

}

/* Print out to 'out' in hcode format a prototype for the given function name
 * and the call_info.  Handles both quoted and unquoted raw_info_strings.
 */
void
L_print_hcode_prototype (FILE * out, char *func_name, char *raw_info_string)
{
  char info_string[TYPE_BUF_SIZE], raw_buf[TYPE_BUF_SIZE];
  char return_type_buf[TYPE_BUF_SIZE];
  char prefix_buf[TYPE_BUF_SIZE], postfix_buf[TYPE_BUF_SIZE];
  char *parse_ptr;
  int has_params;

  /* If string is quoted, strip of quotes */
  if (raw_info_string[0] == '"')
    {
      /* Copy raw_info_string into info_string, stripping off quotes */
      strcpy (info_string, &raw_info_string[1]);
      info_string[strlen (info_string) - 1] = 0;
    }

  /* Otherwise, just copy string */
  else
    {
      strcpy (info_string, raw_info_string);
    }

  /* Get "raw" return type for function */
  parse_ptr = info_string;
  L_get_next_param_type (return_type_buf, &parse_ptr);


  /* Print the function name and return type prefix 
   * (the return type postfix, if any, will be printed out last).
   */
  L_format_hcode_type_prefix (prefix_buf, return_type_buf);
  fprintf (out, "(GVAR %s ((EXTERN)(%s)((F", func_name, prefix_buf);

  /* Print out space if there are any parameters */
  if (*parse_ptr != 0)
    {
      has_params = 1;
      fprintf (out, " (");
    }
  else
    {
      has_params = 0;
    }

  /* Print out each parameter */
  while (*parse_ptr != 0)
    {
      /* Get the type of the next parameter */
      L_get_next_param_type (raw_buf, &parse_ptr);

      /* Convert type to formatted string using hcode conventions
       * (which separates the prefix and the postfix).
       */
      L_format_hcode_type_prefix (prefix_buf, raw_buf);
      L_format_hcode_type_postfix (postfix_buf, raw_buf);

      /* Print out the parameter */
      fprintf (out, "(FPARAM ((%s)(%s)))", prefix_buf, postfix_buf);
    }

  /* If has any parameters, print closing ) */
  if (has_params)
    fprintf (out, ")");

  /* Close the "optional" parameter section */
  fprintf (out, ")");

  /* The return type postfix comes after all the function prototypes. */
  L_format_hcode_type_postfix (postfix_buf, return_type_buf);

  /* Print it out with a leading space if there is a return type postfix */
  if (postfix_buf[0] != 0)
    fprintf (out, " %s)))\n", postfix_buf);
  else
    fprintf (out, ")))\n");
}

/* Convert the type postfix (the stuff after the '+') into C format.
 * Do this recursively and piece by piece, since the ordering
 * of the types makes a non-recursive routine difficult to write.
 *
 * NOTE: This functions parameters MUST NOT overlap (be the same buffer)!
 */
void
L_subtype_to_C_string (char *dest_buf, char *type_buf, char *incoming_buf)
{
  char temp_buf[TYPE_BUF_SIZE], *type_ptr, *temp_ptr;
  char sub_buf[TYPE_BUF_SIZE];

  /* Point at type_buf for parsing */
  type_ptr = type_buf;

  /* Terminate dest_buf initially */
  dest_buf[0] = 0;

  /* If nothing, just copy the incoming buf to the outgoing buf */
  if (*type_ptr == 0)
    {
      strcat (dest_buf, incoming_buf);
    }

  /* Handle an array type.
   * Converts 'PA10A20' into '(*name)[10][20]'
   */
  else if (type_ptr[0] == 'P' && type_ptr[1] == 'A')
    {
      /* Wrap (* ) around whatever has already been parsed
       * or the parameter name. -ITI/JCG 4/99
       */
      sprintf (sub_buf, "(*%s)", incoming_buf);

      /* Skip the P */
      type_ptr++;

      /* Process all the Array specifications */
      while (type_ptr[0] == 'A')
        {
          /* Print out this array def */
          /* Skip the A */
          type_ptr++;

          /* Add openning [ */
          strcat (sub_buf, "[");

          /* Copy number into temp buf */
          temp_ptr = temp_buf;
          while ((*type_ptr != 0) && isdigit (*type_ptr))
            {
              *temp_ptr = *type_ptr;
              temp_ptr++;
              type_ptr++;
            }
          /* Add closing ] */
          temp_ptr[0] = ']';
          temp_ptr[1] = 0;

          /* Add to the end of the existing format string */
          strcat (sub_buf, temp_buf);
        }

      /* Convert the rest of the subtype recursively */
      L_subtype_to_C_string (dest_buf, type_ptr, sub_buf);
    }

  /* Handle function pointers */
  else if ((type_ptr[0] == 'F') ||
           ((type_ptr[0] == 'P') && (type_ptr[1] == 'F')))
    {
      /* Consume optional leading P (should always be there now) */
      if (type_ptr[0] == 'P')
        type_ptr++;

      /* Skip the F */
      type_ptr++;

      /* Wrap parameter name (or previously parsed type) in function 
       * pointer specification 
       */
      sprintf (sub_buf, "(*%s)()", incoming_buf);

      /* Convert the rest of the subtype recursively */
      L_subtype_to_C_string (dest_buf, type_ptr, sub_buf);
    }

  /* Handle normal pointers */
  else if ((type_ptr[0] == 'P') && (type_ptr[1] != 'F'))
    {
      /* Skip P */
      type_ptr++;

      /* Add * to front of incoming_buf */
      sprintf (sub_buf, "*%s", incoming_buf);

      /* Convert the rest of the subtype recursively */
      L_subtype_to_C_string (dest_buf, type_ptr, sub_buf);
    }
  /* Otherwise, punt since I don't know what this is */
  else
    {
      L_punt ("L_subtype_to_C_string: unexected modifier '%s' in '%s'.",
              type_ptr, type_buf);

    }
}

/* Convert "raw" type string into C-style format.
 * Uses parm_name to create formatted_buf.  Parm name should be
 * the function name WITH parameters when creating return type.
 *
 * NOTE: This functions parameters MUST NOT overlap (be the same buffer)!
 */
void
L_convert_type_to_C_format (char *formatted_buf, char *raw_buf,
                            char *param_name)
{
  char temp_buf[TYPE_BUF_SIZE], *temp_ptr, *raw_ptr;

  /* Sanity check, raw_buf should not be NULL or empty */
  if ((raw_buf == NULL) || (*raw_buf == 0))
    L_punt ("L_convert_type_to_C_format: raw_buf empty or NULL!");

  /* Copy type_name into temp_buf (string before '+') */
  temp_ptr = temp_buf;
  raw_ptr = raw_buf;
  while ((*raw_ptr != 0) && (*raw_ptr != '+'))
    {
      *temp_ptr = *raw_ptr;
      temp_ptr++;
      raw_ptr++;
    }

  /* Terminate temp_buf */
  *temp_ptr = 0;

  /* Move raw_ptr past '+', if exists */
  if (*raw_ptr == '+')
    raw_ptr++;

  /* Handle struct names */
  if ((temp_buf[0] == 'S') && (temp_buf[1] == '_'))
    {
      sprintf (formatted_buf, "struct %s ", &temp_buf[2]);
    }

  /* Handle union names */
  else if ((temp_buf[0] == 'U') && (temp_buf[1] == '_'))
    {
      sprintf (formatted_buf, "union %s ", &temp_buf[2]);
    }
  /* Handle vararg type */
  else if (strcmp (temp_buf, "vararg") == 0)
    {
      sprintf (formatted_buf, "...");
      /* No function name, so return now */
      return;
    }

  else if (strcmp (temp_buf, "uchar") == 0)
    {
      sprintf (formatted_buf, "unsigned char ");
    }

  else if (strcmp (temp_buf, "ushort") == 0)
    {
      sprintf (formatted_buf, "unsigned short ");
    }

  else if (strcmp (temp_buf, "uint") == 0)
    {
      sprintf (formatted_buf, "unsigned int ");
    }

  else if (strcmp (temp_buf, "ulong") == 0)
    {
      sprintf (formatted_buf, "unsigned long ");
    }

  else if (strcmp (temp_buf, "longlong") == 0)
    {
      sprintf (formatted_buf, "long long ");
    }

  else if (strcmp (temp_buf, "ulonglong") == 0)
    {
      sprintf (formatted_buf, "unsigned long long ");
    }

  /* Handle all the other type types */
  else if ((strcmp (temp_buf, "void") == 0) ||
           (strcmp (temp_buf, "char") == 0) ||
           (strcmp (temp_buf, "short") == 0) ||
           (strcmp (temp_buf, "int") == 0) ||
           (strcmp (temp_buf, "long") == 0) ||
           (strcmp (temp_buf, "float") == 0) ||
           (strcmp (temp_buf, "double") == 0))
    {
      sprintf (formatted_buf, "%s ", temp_buf);
    }

  /* Otherwise, punt on unknown type */
  else
    {
      L_punt ("L_convert_type_to_C_format: Unknown type '%s' in '%s' string!",
              temp_buf, raw_buf);
    }

  /* Handle the rest of the type info */
  L_subtype_to_C_string (temp_buf, raw_ptr, param_name);

  strcat (formatted_buf, temp_buf);

  /* Done, have C formatted string now */
}

/* Print out C prototype given the function name
 * and the call_info.  Handles both quoted and unquoted raw_info_strings.
 */
void
L_print_C_prototype (FILE * out, char *func_name, char *raw_info_string,
                     int ansi_c_format)
{
  char info_string[TYPE_BUF_SIZE], raw_buf[TYPE_BUF_SIZE];
  char formatted_buf[TYPE_BUF_SIZE], name_buf[TYPE_BUF_SIZE];
  char return_type_buf[TYPE_BUF_SIZE], main_buf[TYPE_BUF_SIZE];
  char *parse_ptr;
  int index;

  /* If string is quoted, strip of quotes */
  if (raw_info_string[0] == '"')
    {
      /* Copy raw_info_string into info_string, stripping off quotes */
      strcpy (info_string, &raw_info_string[1]);
      info_string[strlen (info_string) - 1] = 0;
    }

  /* Otherwise, just copy string */
  else
    {
      strcpy (info_string, raw_info_string);
    }

  /* Get "raw" return type for function */
  parse_ptr = info_string;
  L_get_next_param_type (return_type_buf, &parse_ptr);


  /* Print function name and later parameters to formatted_buf, return 
   * type will be wrapped around everything (necessary for returning 
   * function pointers) at the end.
   */
  sprintf (formatted_buf, "%s (", func_name);

  /* Print out each parameter, if in emitting Ansi-C -ITI (JCG) 3/99 */
  if (ansi_c_format)
    {
      index = 1;
      while (*parse_ptr != 0)
        {
          /* Get the next parameter type */
          L_get_next_param_type (raw_buf, &parse_ptr);

          /* Make up a name for the parameter */
          sprintf (name_buf, "p%i", index - 1);

          /* Convert type to formatted string using C conventions,
           * using name_bud as the 'parameter'
           */
          L_convert_type_to_C_format (main_buf, raw_buf, name_buf);

          /* Add to end of formatted string */
          strcat (formatted_buf, main_buf);

          /* Increment parameter id */
          index++;

          /* Add comma if not at end */
          if (*parse_ptr != 0)
            strcat (formatted_buf, ", ");
        }
    }

  /* Add closing ) */
  strcat (formatted_buf, ")");

  /* Convert return type to formatted string using C conventions,
   * using function name + parms as the 'parameter'
   * 
   * Copy formatted_buf into main_buf because buffers must not
   * overlap and I want the result in formatted_buf.
   */
  strcpy (main_buf, formatted_buf);
  L_convert_type_to_C_format (formatted_buf, return_type_buf, main_buf);

  /* Print prototype (added extern -ITI (JCG) 3/99) */
  fprintf (out, "extern %s;\n", formatted_buf);
}

/* Prints out prototypes in hcode format. */
void
L_print_hcode_prototypes (FILE * out)
{
  STRING_Symbol *symbol;
  Call_Info *info_struct;
  char *func_name, *info_string;

  for (symbol = prototype_table->head_symbol; symbol != NULL;
       symbol = symbol->next_symbol)
    {
      func_name = symbol->name;
      info_struct = (Call_Info *) symbol->data;
      info_string = info_struct->info_string;

      L_print_hcode_prototype (out, func_name, info_string);
    }
}

/* Prints out prototypes in C format (with parameters if ansi_c_format == 1).
 * 
 * If print_deduced_parms is 0, for those function prototypes where 
 * the exact definition was not available, print out the K&R-c prototype
 * (with no parameter info). -ITI/JCG 4/99
 * 
 */
void
L_print_C_prototypes (FILE * out, int ansi_c_format, int print_deduced_parms)
{
  STRING_Symbol *symbol;
  Call_Info *info_struct;
  char *func_name, *info_string;
  int use_old_style;

  for (symbol = prototype_table->head_symbol; symbol != NULL;
       symbol = symbol->next_symbol)
    {
      func_name = symbol->name;
      info_struct = (Call_Info *) symbol->data;
      info_string = info_struct->info_string;
      use_old_style = info_struct->use_old_style;


      /* If not Ansi-C or if function requires an old style prototype,
       * print out in K&R-C format.
       */
#if 0
      if (!ansi_c_format || use_old_style)
        {
          L_print_C_prototype (out, func_name, info_string, 0);
        }

      /* Otherwise, if we don't want to print deduced parm types
       * and this prototype was deduced (no actual function source), 
       * print out in K&R-C format.
       */
      else if (!print_deduced_parms &&
               (STRING_find_symbol (func_table, func_name) == NULL))
        {
          L_print_C_prototype (out, func_name, info_string, 0);
        }

      /* Otherwise, print out Ansi-C prototype */
      else
        {
          L_print_C_prototype (out, func_name, info_string, 1);
        }
#endif
      L_print_C_prototype (out, func_name, info_string, 1);
    }
}

/* Debug, print out info gathered in prototype_table, func_table,
 * or the jsr_table.  DO NOT USE ON THE VARARGS_TABLE! 
 */
void
L_print_debug_prototypes (FILE * out, STRING_Symbol_Table * table)
{
  STRING_Symbol *symbol;
  Call_Info *info_struct;
  char *func_name, *info_string;

  fprintf (out, "\nTable %s:\n\n", table->name);
  for (symbol = table->head_symbol; symbol != NULL;
       symbol = symbol->next_symbol)
    {
      func_name = symbol->name;
      info_struct = (Call_Info *) symbol->data;
      info_string = info_struct->info_string;

      fprintf (out, "%s -> %s\n", func_name, info_string);
      L_print_C_prototype (out, func_name, info_string, 1);
      L_print_hcode_prototype (out, func_name, info_string);
      fprintf (out, "\n");
    }
  fprintf (out, "\n");
}


/* Makes a copy of info1, except the return type is void *.
 * Returns 1 if this is legal (and done), 0 otherwise.
 */
int
L_make_return_void_star (char *new_info, char *info1, char *info2)
{
  char *ptr1, *ptr2;

  /* Make sure first return type is a pointer */
  ptr1 = info1;
  while ((*ptr1 != 0) && (*ptr1 != '+'))
    ptr1++;
  /* Only legal if first qualifier is +P or +F */
  if ((ptr1[0] != '+') || ((ptr1[1] != 'P') && (ptr1[1] != 'F')))
    return (0);

  /* Make sure second return type is a pointer */
  ptr2 = info2;
  while ((*ptr2 != 0) && (*ptr2 != '+'))
    ptr2++;
  /* Only legal if first qualifier is +P */
  if ((ptr2[0] != '+') || ((ptr2[1] != 'P') && (ptr2[1] != 'F')))
    return (0);

  /* Always make void *, since I really don't want to mess with fixing
   * the return types for function pointers.
   */
  strcpy (new_info, "void+P");

  /* Advance info1 past return type to '%' or terminator */
  ptr1 = info1;
  while ((*ptr1 != 0) && (*ptr1 != '%'))
    ptr1++;

  /* Tack on end of new_info */
  strcat (new_info, ptr1);

  /* Sucess, return 1 */
  return (1);
}

/* Copies the next parameter type (strips of terminating %) into buf.
 * Buf will be empty if there are not more parameter types left.
 */
void
L_get_next_param_type (char *buf, char **info_ptr)
{
  char *ptr;

  ptr = *info_ptr;

  /* Copy over next parm type into buf */
  while ((*ptr != 0) && (*ptr != '%'))
    {
      *buf = *ptr;
      buf++;
      ptr++;
    }

  /* Skip % */
  if (*ptr == '%')
    ptr++;

  /* Terminate buf */
  *buf = 0;

  /* Move info_ptr to beginning of next parameter */
  *info_ptr = ptr;
}

/* Returns 1 if of char, short, int, or long type, 0 otherwise .
 */
int
L_param_is_int_type (char *parm_type)
{
  if ((strcmp (parm_type, "char") == 0) ||
      (strcmp (parm_type, "uchar") == 0) ||
      (strcmp (parm_type, "short") == 0) ||
      (strcmp (parm_type, "ushort") == 0) ||
      (strcmp (parm_type, "int") == 0) ||
      (strcmp (parm_type, "uint") == 0) ||
      (strcmp (parm_type, "long") == 0) ||
      (strcmp (parm_type, "ulong") == 0) ||
      (strcmp (parm_type, "longlong") == 0) ||
      (strcmp (parm_type, "ulonglong") == 0))
    return (1);
  else
    return (0);
}


/* Returns 1 if +P, +F, or +PF appears after the type name, 0 otherwise 
 * Basically return 1 if normal pointer or function pointer.
 */
int
L_param_is_pointer (char *parm_type)
{
  char *ptr;

  /* Scan for + */
  ptr = parm_type;
  while ((*ptr != 0) && (*ptr != '+'))
    ptr++;

  /* If have '+P' (or +F) it is a pointer, otherwise not */
  if ((ptr[0] == '+') && ((ptr[1] == 'P') || (ptr[1] == 'F')))
    {
      return (1);
    }
  else
    {
      return (0);
    }
}

/* Returns 1 if +F, or +PF appears after the type name, 0 otherwise */
int
L_param_is_func_pointer (char *parm_type)
{
  char *ptr;

  /* Scan for + */
  ptr = parm_type;
  while ((*ptr != 0) && (*ptr != '+'))
    ptr++;

  /* If have '+PF' or +F it is a func pointer, otherwise not */
  if ((ptr[0] == '+') &&
      (((ptr[1] == 'P') && (ptr[2] == 'F')) || (ptr[1] == 'F')))
    {
      return (1);
    }
  else
    {
      return (0);
    }
}

/* Makes a new version of info1 that is compatible with info2 if selected
 * pointers are made void pointers.  Will do its best, even if they
 * have a different number of parameters.
 *
 * Will return the index of the parameter that could not be make
 * compatible.  Returns -1 if everything made compatible.
 */
int
L_make_compatible (char *new_info, char *info1, char *info2)
{
  char buf1[TYPE_BUF_SIZE + 1000], buf2[TYPE_BUF_SIZE + 1000];
  char *ptr1, *ptr2;
  int diff_index, parm_index;

  /* Start with everything compatible */
  diff_index = -1;

  /* Start with empty new_info */
  new_info[0] = 0;

  /* Point to beginning of each info string */
  ptr1 = info1;
  ptr2 = info2;

  /* get the first type of each info string */
  L_get_next_param_type (buf1, &ptr1);
  L_get_next_param_type (buf2, &ptr2);

  /* Copy ptr1's return type to new_info */
  strcat (new_info, buf1);

  /* Start at parm_index 1 */
  parm_index = 1;

  /* Copy all info1 parameter types to new_info, possibly until
   * made void pointer.
   */
  while (*ptr1 != 0)
    {
      /* Get the next param type */
      L_get_next_param_type (buf1, &ptr1);
      L_get_next_param_type (buf2, &ptr2);

      /* If the same, simply concat to new info */
      if (strcmp (buf1, buf2) == 0)
        {
          strcat (new_info, "%");
          strcat (new_info, buf1);
        }
      /* Otherwise, if both are pointers, replace with void *.
       * Since programs can sometime use "int"s as pointers,
       * allow int/pointer combo to be changed into void *
       */
      else if ((L_param_is_pointer (buf1) || L_param_is_int_type (buf1)) &&
               (L_param_is_pointer (buf2) || L_param_is_int_type (buf2)))
        {
          /* Just make void *, since I don't want to mess with getting
           * return type for function pointers right.
           */
          strcat (new_info, "%void+P");
        }

      /* Otherwise, if both are int types, replace with larger int type */
      else if (L_param_is_int_type (buf1) && L_param_is_int_type (buf2))
        {
          /* Set to "larger" int type */
          if ((strcmp (buf1, "ulonglong") == 0) ||
              (strcmp (buf2, "ulonglong") == 0))
            {
              strcat (new_info, "%ulong");
            }
          else if ((strcmp (buf1, "longlong") == 0) ||
                   (strcmp (buf2, "longlong") == 0))
            {
              strcat (new_info, "%long");
            }
          else if ((strcmp (buf1, "ulong") == 0) ||
                   (strcmp (buf2, "ulong") == 0))
            {
              strcat (new_info, "%ulong");
            }
          else if ((strcmp (buf1, "long") == 0) ||
                   (strcmp (buf2, "long") == 0))
            {
              strcat (new_info, "%long");
            }
          else if ((strcmp (buf1, "uint") == 0) ||
                   (strcmp (buf2, "uint") == 0))
            {
              strcat (new_info, "%uint");
            }
          else if ((strcmp (buf1, "int") == 0) || (strcmp (buf2, "int") == 0))
            {
              strcat (new_info, "%int");
            }
          else if ((strcmp (buf1, "ushort") == 0) ||
                   (strcmp (buf2, "ushort") == 0))
            {
              strcat (new_info, "%ushort");
            }

          else if ((strcmp (buf1, "short") == 0) ||
                   (strcmp (buf2, "short") == 0))
            {
              strcat (new_info, "%short");
            }

          else if ((strcmp (buf1, "uchar") == 0) ||
                   (strcmp (buf2, "uchar") == 0))
            {
              strcat (new_info, "%uchar");
            }

          /* Should never get here */
          else
            {
              L_punt ("L_make_compatible: cannot make ints '%s' and '%s' "
                      "compatible!", buf1, buf2);
            }
        }


      /* Otherwise, we cannot make compatible, probably a varargs
       * function.  Just copy buf1 to string.
       */
      else
        {
          /* Record parm index of first diff */
          if (diff_index == -1)
            diff_index = parm_index;

          strcat (new_info, "%");
          strcat (new_info, buf1);
        }

      parm_index++;
    }

  /* If info_string two not through, update diff index if necessary */
  if (*ptr2 != 0)
    {
      /* Record parm index of first diff */
      if (diff_index == -1)
        diff_index = parm_index;
    }

#if 0
  /* Debug */
  printf ("Converted: %s\n"
          "           %s\n"
          "       to: %s (diff_index %i)\n\n", info1, info2, new_info,
          diff_index);
#endif

  return (diff_index);
}

/* Given a function anem and a "raw" call_info attribute string (which
 * usually has quotes around it), add or update the prototype deduction
 * for this function in the passed table.
 *
 * To handle old-style varargs in Ansi-C, we need to detect and
 * emit prototypes properly in K&R-C style when indicated by
 * the 'use_old_style' flag. -ITI/JCG 4/99
 */
void
L_add_call_info (STRING_Symbol_Table * table, char *name,
                 char *raw_info_string, int use_old_style)
{
  STRING_Symbol *symbol;
  Call_Info *info_struct;
  int diff_index;
  int return_type_different, swap_info_strings;
  char *ptr1, *ptr2;
  char info_string[TYPE_BUF_SIZE], compatible_info[TYPE_BUF_SIZE + 1000];

  /* If string is quoted, strip of quotes */
  if (raw_info_string[0] == '"')
    {
      /* Copy raw_info_string into info_string, stripping off quotes */
      strcpy (info_string, &raw_info_string[1]);
      info_string[strlen (info_string) - 1] = 0;
    }

  /* Otherwise, just copy string */
  else
    {
      strcpy (info_string, raw_info_string);
    }

  /* If name not already in table, add it */
  if ((symbol = STRING_find_symbol (table, name)) == NULL)
    {
      /* Malloc structure */
      if ((info_struct = (Call_Info *) malloc (sizeof (Call_Info))) == NULL)
        L_punt ("Out of memory allocating Call_info structure");

      /* Copy info string */
      if ((info_struct->info_string = strdup (info_string)) == NULL)
        L_punt ("Out of memory duping info_string");

      /* No differences yet */
      info_struct->diff_index = -1;

      /* Use indicated style flag */
      info_struct->use_old_style = use_old_style;

      /* Add to symbol table */
      STRING_add_symbol (table, name, (void *) info_struct);
    }
  /* Otherwise, see if same info already present */
  else
    {
      /* Get call info structure */
      info_struct = (Call_Info *) symbol->data;

      /* Assume the params and return types are the same and that the
       * info_strings should not be swapped.
       */
      return_type_different = 0;
      swap_info_strings = 0;

      /* Get pointers to both info strings */
      ptr1 = info_struct->info_string;
      ptr2 = info_string;

      /* Determine if both info strings indicate the same return type */
      while ((*ptr1 != 0) && (*ptr1 != '%') && (*ptr2 != 0) && (*ptr2 != '%'))
        {
          /* If there is a difference, mark it and consume the
           * rest of the return type strings.
           */
          if ((*ptr1 != *ptr2))
            {
              return_type_different = 1;

              while ((*ptr1 != 0) && (*ptr1 != '%'))
                ptr1++;

              while ((*ptr2 != 0) && (*ptr2 != '%'))
                ptr2++;

              break;
            }

          ptr1++;
          ptr2++;
        }

      /* If pointing at '%', advance to next character */
      if (*ptr1 == '%')
        ptr1++;

      if (*ptr2 == '%')
        ptr2++;

      /* If return types are different, pick the non-'int' version.
       * Punt if neither are int types (since it should not happen).
       */
      if (return_type_different)
        {
          /* Is the existing return type "int" */
          if ((info_struct->info_string[0] == 'i') &&
              (info_struct->info_string[1] == 'n') &&
              (info_struct->info_string[2] == 't') &&
              ((info_struct->info_string[3] == '%') ||
               (info_struct->info_string[3] == 0)))
            {
              /* Yes, swap info strings */
              swap_info_strings = 1;
            }

          /* Is the new return type "int" */
          else if ((info_string[0] == 'i') &&
                   (info_string[1] == 'n') &&
                   (info_string[2] == 't') &&
                   ((info_string[3] == '%') || (info_string[3] == 0)))

            {
              /* Yes, don't swap info strings */
              swap_info_strings = 0;
            }

          /* If neither are, assume function returns 'void *'.
           * Change return type of existing info_string to void *
           * (if both are compatible).
           */
          else
            {
              if (!L_make_return_void_star (compatible_info,
                                            info_struct->info_string,
                                            info_string))
                {
                  L_punt ("%s: return type mismatch for call to %s\n"
                          "Previous call info '%s'\n"
                          "New call info '%s'\n"
                          "Expect one to be indicate returning an int or\n"
                          "for both to be compatible with void *!\n",
                          L_fn->name, name, info_struct->info_string,
                          info_string);
                }

              /* Replace info_struct->info_string with compatible string */
              free (info_struct->info_string);
              info_struct->info_string = strdup (compatible_info);

              /* Move ptr1 to appropriate place in new info_string */
              ptr1 = info_struct->info_string;
              while ((*ptr1 != 0) && (*ptr1 != '%'))
                ptr1++;

              if (*ptr1 == '%')
                ptr1++;
            }
        }

      /* If parameter strings are not identical due to different pointer
       * types, make a compatible parameter list with "void *" pointers.
       * This should be able to handle everything except varargs functions
       * and extemely lame coding style!  
       */
      if (strcmp (ptr1, ptr2) != 0)
        {
          /* Will make a compatible version of ptr1 by default,
           * if going to swap info strings, make compatible version
           * of ptr2
           */
          if (!swap_info_strings)
            {
              diff_index = L_make_compatible (compatible_info,
                                              info_struct->info_string,
                                              info_string);
            }
          else
            {
              diff_index = L_make_compatible (compatible_info,
                                              info_string,
                                              info_struct->info_string);

              /* Implicitly swapping strings, so reset flag */
              swap_info_strings = 0;
            }


          /* If could not be make compatible, update diff_index
           * for info_string (if necessary).
           */
          if ((diff_index != -1) &&
              ((info_struct->diff_index == -1) ||
               (diff_index < info_struct->diff_index)))
            {
#if 0
              /* Debug */
              printf ("\n%s %s old_index %i new_index %i\n",
                      info_struct->info_string, info_string,
                      info_struct->diff_index, diff_index);
#endif

              info_struct->diff_index = diff_index;
            }

          /* If either specification wants old style, use the old style */
          if (use_old_style)
            {
              info_struct->use_old_style = use_old_style;
            }

          /* Make info_struct->info_string the new compatible_info string */
          free (info_struct->info_string);
          info_struct->info_string = strdup (compatible_info);
        }

      /* If indicated, swap info strings */
      if (swap_info_strings)
        {
#if 0
          /* Debug */
          printf ("%s swaping %s for %s\n", name, info_string,
                  info_struct->info_string);
#endif

          /* Free old string */
          free (info_struct->info_string);

          /* Make copy of new string */
          info_struct->info_string = strdup (info_string);
        }
    }

}

/* Scan the passed function building up prototype deductions in
 * the func_table and the jsr_table.
 */
void
L_collect_call_info (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *op;
  L_Attr *attr;
  int use_old_style;
  char *info_string, *func_name, *raw_name, *src_ptr, *dest_ptr;
  char temp_buf[TYPE_BUF_SIZE];

  /* Get 'call_info' attribute for the function */
  if ((attr = L_find_attr (fn->attr, "call_info")) == NULL)
    L_punt ("%s: 'call_info' attribute not found!", fn->name);

  /* Sanity check */
  if (attr->field[0] == NULL)
    L_punt ("%s: string expected in 'call_info' attribute.", fn->name);

  /* Sanity check, string expected to fit in buffer of size TYPE_BUF_SIZE */
  if (strlen (attr->field[0]->value.s) >= (TYPE_BUF_SIZE - 1))
    {
      L_punt ("%s: call_info attribute (len %i) too big for bufs (len %i).\n"
              "    Increase value of 'TYPE_BUF_SIZE' and rebuild "
              "Lbuild_prototype_info.\n",
              fn->name, strlen (attr->field[0]->value.s), TYPE_BUF_SIZE);
    }

  /* Copy the info string into a temp buffer.
   * Pcode adds a +F after the return type in the function's call info.
   * Strip out to make match info on call sites.
   */
  dest_ptr = temp_buf;
  src_ptr = attr->field[0]->value.s;
  while ((*src_ptr != 0) && (*src_ptr != '+'))
    {
      *dest_ptr = *src_ptr;
      dest_ptr++;
      src_ptr++;
    }

  /* If the next two characters are '+F', 
   * skip over the '+F' and write the '+' to
   * *dest_ptr with there is something after the 'F'.
   */
  if ((src_ptr[0] == '+') && (src_ptr[1] == 'F'))
    {
      src_ptr += 2;

      if (*src_ptr != 0)
        {
          *dest_ptr = '+';
          dest_ptr++;
        }
    }

  /* Copy the rest of the buffer */
  while ((*src_ptr != 0))
    {
      *dest_ptr = *src_ptr;
      dest_ptr++;
      src_ptr++;
    }

  /* Termiante string */
  *dest_ptr = 0;

  /* Point at temp buf for processing */
  info_string = temp_buf;

  /* Strip leading _ from func name */
  func_name = &fn->name[1];

  /* If has 'old_style_param' attribute, use old style prototype */
  if (L_find_attr (fn->attr, "old_style_param") != NULL)
    {
      use_old_style = 1;
    }

  /* Otherwise, use format flag to determine prototype style */
  else
    {
      use_old_style = 0;
    }

#if 0
  /* Debug */
  printf ("%s %s\n", func_name, info_string);
#endif

  L_add_call_info (func_table, func_name, info_string, use_old_style);

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (op = cb->first_op; op != NULL; op = op->next_op)
        {
          /* Look at every direct (non-register) function call, 
           * to build up call info for building prototypes.
           */
          if (L_subroutine_call_opcode (op) && (op->src[0] != NULL) &&
              L_is_label (op->src[0]))
            {
              /* Make sure has call_info attribute */
              /* Get 'call_info' attribute for the function */
              if ((attr = L_find_attr (op->attr, "call_info")) == NULL)
                {
                  L_punt ("%s op %i: 'call_info' attribute not found!",
                          fn->name, op->id);
                }

              /* Sanity check */
              if (attr->field[0] == NULL)
                {
                  L_punt ("%s op %i: string expected in 'call_info' "
                          "attribute.", fn->name, op->id);
                }

              /* Sanity check, string expected to fit in buffer of 
               * size TYPE_BUF_SIZE 
               */
              if (strlen (attr->field[0]->value.s) >= (TYPE_BUF_SIZE - 1))
                {
                  L_punt ("%s op %i: call_info attribute (len %i) too big "
                          "for bufs (len %i).\n"
                          "    Increase value of 'TYPE_BUF_SIZE' and "
                          "rebuild Lbuild_protype_info.\n",
                          fn->name, op->id,
                          strlen (attr->field[0]->value.s), TYPE_BUF_SIZE);
                }

              /* Get the info string */
              info_string = attr->field[0]->value.s;
              raw_name = op->src[0]->value.l;

              /* Remove leading _ or _$fn_ */
              if ((raw_name[0] == '_') && (raw_name[1] == '$') &&
                  (raw_name[2] == 'f') && (raw_name[3] == 'n') &&
                  (raw_name[4] == '_'))
                {
                  func_name = &raw_name[5];
                }
              else if (raw_name[0] == '_')
                {
                  func_name = &raw_name[1];
                }
              else
                {
                  L_punt ("%s op %i: error parsing function name '%s'.",
                          fn->name, op->id, raw_name);
                }

#if 0
              /* Debug */
              printf ("%s %s\n", func_name, info_string);
#endif

              /* Call site has no info on prototype style, so don't
               * force it to use the old style. -ITI/JCG 4/99 
               */
              L_add_call_info (jsr_table, func_name, info_string, 0);
            }
        }
    }
}

/* Adds 'func_name' to the table of known varargs functions (in 
 * varargs_table) and sets the 'data' pointer to the varargs_index.
 * This index specifies which parameter is the varargs index.
 * For example, for 'int printf (char *, ...);', the varargs
 * index should be 2.  It is not legal C to have a varargs 
 * for the first parameter.
 */
void
L_known_varargs (char *func_name, int varargs_index)
{
  /* Sanity check, make sure not in varargs table already */
  if (STRING_find_symbol (varargs_table, func_name) != NULL)
    L_punt ("L_known_varargs: %s already in table!", func_name);

  /* Sanity check, varargs_index must be >= 2 (at least one fixed
   * argument is needed.
   */
  if (varargs_index < 2)
    L_punt ("L_known_varargs: Index must be >= 2, %s index %i!",
            func_name, varargs_index);

  /* Add varargs info to table */
#ifdef LP64_ARCHITECTURE
  STRING_add_symbol (varargs_table, func_name, (void *)((long)varargs_index));
#else
  STRING_add_symbol (varargs_table, func_name, (void *) varargs_index);
#endif
}

/* Go through all the jsr call_info structures looking for functions
 * that source code is not available for (i.e., not in func_table).
 * For those, determine if the arguments have varied and if so, if
 * they are a known varargs function.  If not, print out a warning, and
 * make up a varargs prototype anyway.
 *
 * If 'func_labels_used' is not NULL, go through this table and create
 * dummy prototypes (int foo();) for those functions not defined or 
 * deduced from the eariler passes.  This is to support Lemulate. -ITI/JCG 4/99
 *
 * Added bit-field flags argument (see l_build_prototype_info.h for flags 
 * defined) to provide clean way to specify some niceties that I wanted to 
 * implement for Lemulate. -ITI/JCG 4/99
 *
 * Note: This deduction may call known varargs functions "non-varargs" if
 * the arguments are always consistant. 
 */
void
L_deduce_prototypes (STRING_Symbol_Table * func_labels_used, int flags)
{
  STRING_Symbol *symbol;
  char *func_name;
  Call_Info *info_struct;
  int varargs_index, diff_index, param_index;
  char varargs_string[TYPE_BUF_SIZE + 1000], *ptr1, *ptr2;

  /* Go through each func call_info entry and add to prototype table */
  for (symbol = func_table->head_symbol; symbol != NULL;
       symbol = symbol->next_symbol)
    {
      /* Get func_name and info_struct for ease of use */
      func_name = symbol->name;
      info_struct = (Call_Info *) symbol->data;

      if (STRING_find_symbol (ignore_table, func_name) != NULL)
        continue;

      L_add_call_info (prototype_table, func_name,
                       info_struct->info_string, info_struct->use_old_style);
    }

  /* Go through each jsr call_info entry */
  for (symbol = jsr_table->head_symbol; symbol != NULL;
       symbol = symbol->next_symbol)
    {
      /* Get func_name and info_struct for ease of use */
      func_name = symbol->name;
      info_struct = (Call_Info *) symbol->data;

      /* If have source for this function, don't need to do anything
       * else.
       */
      if (STRING_find_symbol (func_table, func_name) != NULL)
        continue;

      if (STRING_find_symbol (ignore_table, func_name) != NULL)
        continue;

      /* If all calls to this function have consistent parameters,
       * just add to prototype table (-1 if consistent, > 1 to
       * specify parameter that is different).
       */
      diff_index = info_struct->diff_index;

      /* If flagged to use known varargs info and didn't look like
       * varargs from usage, see if it is a known varargs function
       * and make varargs function if it is.
       */
      if ((flags & DP_USE_KNOWN_VARARGS_INFO) && (diff_index == -1))
        {
          /* See if known varargs function (varargs_index > 0) */
#ifdef LP64_ARCHITECTURE
          varargs_index = (int)((long)STRING_find_symbol_data (varargs_table,
	                                                       func_name));
#else
          varargs_index = (int) STRING_find_symbol_data (varargs_table,
                                                         func_name);
#endif

          /* If it is, change diff_index to the appropriate varargs 
           * possition.
           */
          if (varargs_index > 0)
            {
              diff_index = varargs_index;
            }

        }

      if (diff_index == -1)
        {
          L_add_call_info (prototype_table, func_name,
                           info_struct->info_string,
                           info_struct->use_old_style);
          continue;
        }

      /* See if inconsistency is because it is a known varags function 
       * (varargs_index with be > 1)
       */
#ifdef LP64_ARCHITECTURE
      varargs_index = (int)((long)STRING_find_symbol_data (varargs_table,
	                                                   func_name));
#else
      varargs_index = (int) STRING_find_symbol_data (varargs_table,
                                                     func_name);
#endif
      if (varargs_index > 1)
        {
          /* Sanity check, better not have differences before
           * "varargs" function.
           */
          if (diff_index < varargs_index)
            {
              L_punt ("L_deduce_prototypes: calls to '%s' have parameter\n"
                      "     differences at parameter %i, but the varargs\n"
                      "     parameter is parameter %i!\n\n"
                      "     Unable to reconcile the difference!\n",
                      func_name, diff_index, varargs_index);
            }
        }
      /* Well, warn them we are making up a varargs function for
       * a libarary call we don't know (if DP_SILENT flag not set).
       */
      else
        {
          if (!(flags & DP_SILENT))
            {
              fprintf (stderr,
                       "\nWARNING, Lbuild_prototype_info: '%s' appears\n"
                       "to be a varargs library function (at parameter %i),\n"
                       "but it is not in the known vararg libarary function\n"
                       "list!  Creating varargs function prototype for it "
                       "anyways.\n\n", func_name, diff_index);
            }

          /* Make sure diff_index > 1 */
          if (diff_index < 2)
            {
              L_punt ("L_deduce_prototypes: '%s' appears to be a invalid\n"
                      "  varargs library function (no format string, etc.)\n"
                      "  Unable to create varargs prototype!\n", func_name);
            }

          /* Just assume varargs_index is diff_index */
          varargs_index = diff_index;
        }

      /* Copy over the call_info, but add the varargs 
       * type at the varargs_index.
       */
      ptr1 = info_struct->info_string;
      ptr2 = varargs_string;
      param_index = 0;
      while ((param_index < varargs_index) && (*ptr1 != 0))
        {
          /* '%' separates parameters (return type first) */
          if (*ptr1 == '%')
            param_index++;

          /* Copy string until varargs paramater to varargs_string */
          *ptr2 = *ptr1;

          ptr1++;
          ptr2++;
        }

      /* If tacking varargs at end of ptr1 string, add a % separator */
      if (*ptr1 == 0)
        {
          *ptr2 = '%';
          ptr2++;
        }

      /* Terminate varargs string */
      *ptr2 = 0;

      /* Concatinate varargs parameter type */
      strcat (varargs_string, "vararg");

      /* Add modified string to prototype table */
      L_add_call_info (prototype_table, func_name,
                       varargs_string, info_struct->use_old_style);
    }

  /* If defined, go through each func_labels_used entry */
  if (func_labels_used != NULL)
    {
      for (symbol = func_labels_used->head_symbol; symbol != NULL;
           symbol = symbol->next_symbol)
        {
          /* Get func_name and info_struct for ease of use */
          func_name = symbol->name;
          info_struct = (Call_Info *) symbol->data;

          /* If already have prototype for this function, don't need to 
           * do anything else.
           */
          if (STRING_find_symbol (prototype_table, func_name) != NULL)
            continue;

          if (STRING_find_symbol (ignore_table, func_name) != NULL)
            continue;

          /* Create dummy int name(); prototype and flag that an old
           * style declaration should be used (not currently necessary,
           * but makes sense to me).
           */
          L_add_call_info (prototype_table, func_name, "int", 1);
        }
    }
}

/* Frees the data allocated for one call info structure, used
 * by L_delete_call_info().
 */
static void
L_free_call_info_data (void *data)
{
  Call_Info *call_info;

  /* Cast to appropriate type for ease of use */
  call_info = (Call_Info *) data;

  /* Free the info string */
  free (call_info->info_string);

  /* Free the structure */
  free (call_info);
}

/* Free the tables created to store the call info, if necessary */
void
L_delete_call_info ()
{
  /* Free existing tables, if they exist */
  if (prototype_table != NULL)
    {
      /* Sanity check, all should be non-NULL if prototype_table is */
      if ((func_table == NULL) || (jsr_table == NULL) ||
          (varargs_table == NULL))
        {
          L_punt ("L_delete_call_info: Unexpected NULL tables!");
        }

      /* Free the symbol tables and all the call info data in them */
      STRING_delete_symbol_table (prototype_table, L_free_call_info_data);
      STRING_delete_symbol_table (func_table, L_free_call_info_data);
      STRING_delete_symbol_table (jsr_table, L_free_call_info_data);

      /* Free symbol table, but data is really an int, so don't free */
      STRING_delete_symbol_table (varargs_table, NULL);

      /* Flag that we have deleted everything */
      prototype_table = NULL;
      func_table = NULL;
      jsr_table = NULL;
      varargs_table = NULL;
    }
}

/* Initialize the call info.  If called multiple times, will
 * cleanly destroy all previous state. -ITI/JCG 4/99
 */
void
L_init_call_info ()
{
  /* Free existing tables, if they exist */
  L_delete_call_info ();

  /* Create func and jsr call info tables */
  prototype_table = STRING_new_symbol_table ("prototype_table", 256);
  func_table = STRING_new_symbol_table ("func_table", 256);
  jsr_table = STRING_new_symbol_table ("jsr_table", 256);
  varargs_table = STRING_new_symbol_table ("varargs_table", 256);
  ignore_table = STRING_new_symbol_table ("ignore_table", 16);

  /* Add all known varargs functions to varargs table.
   * Specify which parameter (first parameter == 1) is the 
   * varargs parameter.
   *
   * Can read in from list later if necessary, just hardcode them for now.
   * Basically I did a grep '\.\.\.' *.h *(slash)*.h in /usr/include
   */
  L_known_varargs ("snprintf", 4);
  L_known_varargs ("printf", 2);
  L_known_varargs ("fprintf", 3);
  L_known_varargs ("sprintf", 3);
  L_known_varargs ("scanf", 2);
  L_known_varargs ("fscanf", 3);
  L_known_varargs ("sscanf", 3);
  L_known_varargs ("execl", 3);
  L_known_varargs ("execle", 3);
  L_known_varargs ("execlp", 3);
  L_known_varargs ("ioctl", 3);
  L_known_varargs ("makecontext", 3);
  L_known_varargs ("swapon", 2);
  L_known_varargs ("mpsprint", 3);
  L_known_varargs ("strlog", 6);
  L_known_varargs ("reboot", 2);
  L_known_varargs ("semctl", 4);
  L_known_varargs ("sem_open", 3);
  L_known_varargs ("mq_open", 3);
  L_known_varargs ("mount", 4);
  L_known_varargs ("io_search", 4);
  L_known_varargs ("sysfs", 2);
  L_known_varargs ("fcntl", 3);
  L_known_varargs ("open", 3);
  L_known_varargs ("open64", 3);
  L_known_varargs ("cmn_err", 3);
  L_known_varargs ("ulimit", 2);
  L_known_varargs ("syslog", 2);
  L_known_varargs ("regcmp", 2);
  L_known_varargs ("regex", 2);
  L_known_varargs ("mvprintw", 4);
  L_known_varargs ("mvscanw", 4);
  L_known_varargs ("mvwprintw", 5);
  L_known_varargs ("mvwscanw", 5);
  L_known_varargs ("printw", 2);
  L_known_varargs ("scanw", 2);
  L_known_varargs ("wprintw", 3);
  L_known_varargs ("wscanw", 3);
  L_known_varargs ("tparm", 2);
  L_known_varargs ("msprintf", 2);
  L_known_varargs ("Bitset", 3);
  L_known_varargs ("strblds", 2);
  L_known_varargs ("strbldf", 2);
  L_known_varargs ("__builtin_va_start", 2);

  /* Do not generate prototypes for the following functions */
  STRING_add_symbol (ignore_table, "__builtin_stdarg_start", NULL);
  STRING_add_symbol (ignore_table, "__builtin_varargs_start", NULL);
  STRING_add_symbol (ignore_table, "__builtin_alloca", NULL);
  STRING_add_symbol (ignore_table, "__builtin_memcpy", NULL);
  STRING_add_symbol (ignore_table, "alloca", NULL);
}


/* Convert call_info type field into Lcode ctype.  Use CTYPE_STRUCT to
 * indicate structure or union being passed.
 */
int
L_convert_type_to_ctype (char *raw_buf)
{
  char temp_buf[TYPE_BUF_SIZE], *temp_ptr, *raw_ptr;
  int ctype;

  /* Sanity check, raw_buf should not be NULL or empty */
  if ((raw_buf == NULL) || (*raw_buf == 0))
    L_punt ("L_convert_type_to_ctype: raw_buf empty or NULL!");

  /* Copy type_name into temp_buf (string before '+') */
  temp_ptr = temp_buf;
  raw_ptr = raw_buf;
  while ((*raw_ptr != 0) && (*raw_ptr != '+'))
    {
      *temp_ptr = *raw_ptr;
      temp_ptr++;
      raw_ptr++;
    }

  /* Terminate temp_buf */
  *temp_ptr = 0;

  /* Move raw_ptr past '+', if exists */
  if (*raw_ptr == '+')
    raw_ptr++;

  /* Initialize ctype to 0, to indicate no ctype set yet */
  ctype = 0;

  /* Handle struct names */
  if ((temp_buf[0] == 'S') && (temp_buf[1] == '_'))
    {
      /* Use CTYPE_STRUCT to flag struct or union since no normal
       * L_CTYPE available 
       */
      ctype = CTYPE_STRUCT;
    }

  /* Handle union names */
  else if ((temp_buf[0] == 'U') && (temp_buf[1] == '_'))
    {
      /* Use CTYPE_STRUCT to flag struct or union since no normal
       * L_CTYPE available 
       */
      ctype = CTYPE_STRUCT;
    }

  /* Handle vararg type */
  else if (strcmp (temp_buf, "vararg") == 0)
    {
      L_punt ("L_convert_type_to_ctype: vararg type unexpected!");
    }

  /* Handle INT types */
  else if ((strcmp (temp_buf, "uchar") == 0) ||
	   (strcmp (temp_buf, "ushort") == 0) ||
	   (strcmp (temp_buf, "uint") == 0) ||
	   (strcmp (temp_buf, "ulong") == 0) ||
	   (strcmp (temp_buf, "char") == 0) ||
	   (strcmp (temp_buf, "short") == 0) ||
	   (strcmp (temp_buf, "int") == 0) ||
	   (strcmp (temp_buf, "long") == 0))
    {
      ctype = M_native_int_register_ctype ();
    }

  else if ((strcmp (temp_buf, "longlong") == 0) ||
	   (strcmp (temp_buf, "ulonglong") == 0))
    {
      ctype = L_CTYPE_LLONG;
    }

  /* Handle VOID type */
  else if (strcmp (temp_buf, "void") == 0)
    {
      ctype = L_CTYPE_VOID;
    }

  else if (strcmp (temp_buf, "float") == 0)
    {
      ctype = L_CTYPE_FLOAT;
    }
  else if (strcmp (temp_buf, "double") == 0)
    {
      ctype = L_CTYPE_DOUBLE;
    }

  /* Otherwise, punt on unknown type */
  else
    {
      L_punt ("L_convert_type_to_ctype: Unknown type '%s' in '%s' string!",
	      temp_buf, raw_buf);
    }

  /* If a pointer (if raw_ptr is not at the end of the string),
   * always promote to INT ctype.
   */
  if (*raw_ptr != 0)
    {
      return (M_native_int_register_ctype ());
    }

  /* Otherwise, use ctype determined above */
  else
    {
      return (ctype);
    }
}


/* Finds the call_info attribute in the passed attribute list and
 * returns the return type string in return_type_buf and the parameter
 * type(s) in parameter_type_buf.  Both of these buffers must be
 * at least as big as indicated in buf_size.
 *
 * parameter_type_buf may be NULL if this information is not desired.
 * 
 * The fn and op parameters are used for error messages only.
 * They may be NULL if this information is not available.
 *
 * Will punt if call_info attribute not found or if the buffers
 * will be overflowed.
 */
void
L_get_call_info (L_Func * fn, L_Oper * op, L_Attr * attr_list,
		 char *return_type_buf, char *parameter_type_buf,
		 int buf_size)
{
  char *raw_info, *true_info, *parse_ptr;
  int info_size;
  L_Attr *attr;

  /* Get 'call_info' attribute from the attribute list */
  if ((attr = L_find_attr (attr_list, "call_info")) == NULL)
    {
      if (fn != NULL)
	{
	  fprintf (stderr, "In function '%s':\n", fn->name);
	}
      if (op != NULL)
	{
	  fprintf (stderr, "For operation:\n  ");
	  L_print_oper (stderr, op);
	}
      L_punt ("L_get_call_info: 'call_info' attribute not found!");
    }

  /* Sanity check */
  if (attr->field[0] == NULL)
    {
      if (fn != NULL)
	{
	  fprintf (stderr, "In function '%s':\n", fn->name);
	}
      if (op != NULL)
	{
	  fprintf (stderr, "For operation:\n  ");
	  L_print_oper (stderr, op);
	}
      L_punt ("L_get_call_info: string expected in 'call_info' attribute.");
    }

  /* Get raw_info for ease of use */
  raw_info = attr->field[0]->value.s;

  /* Get length of info string */
  info_size = strlen (raw_info) + 1;

  /* Sanity check, string expected to fit in buffer of size buf_size */
  if (info_size > buf_size)
    {
      if (fn != NULL)
	{
	  fprintf (stderr, "In function '%s':\n", fn->name);
	}
      if (op != NULL)
	{
	  fprintf (stderr, "For operation:\n  ");
	  L_print_oper (stderr, op);
	}
      L_punt ("L_get_call_info: call_info attribute (size %i) "
	      "too big for bufs (size %i).\n", info_size, buf_size);
    }

  /* Malloc true_info buffer to hold raw_info minus any quotes */
  if ((true_info = (char *) malloc (info_size)) == NULL)
    L_punt ("L_get_call_info: Out of memory");

  /* If string is quoted, strip of quotes */
  if (raw_info[0] == '"')
    {
      /* Copy raw_info into true_info, stripping off quotes */
      strcpy (true_info, &raw_info[1]);
      true_info[strlen (true_info) - 1] = 0;
    }

  /* Otherwise, just copy string */
  else
    {
      strcpy (true_info, raw_info);
    }

  /* Get return type specifier from true_info */
  parse_ptr = true_info;
  L_get_next_param_type (return_type_buf, &parse_ptr);

  /* Copy rest of true_info (the parm info) into the parameter_type_buf,
   * if it is not NULL
   */
  if (parameter_type_buf != NULL)
    {
      strcpy (parameter_type_buf, parse_ptr);
    }

  /* Done, free true_info and return */
  free (true_info);
}

