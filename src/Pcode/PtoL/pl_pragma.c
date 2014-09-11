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
 *	File:	pl_pragma.c
 *	Author:	John W. Sias and Wen-mei Hwu
 *	Revised: 
\*****************************************************************************/
#include <config.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <library/c_basic.h>
#include "pl_main.h"

#define PL_ELIDE_QUOTES 0

#define PL_GENERATE_VAR_ATTRS

/*
 * Functions for generating and reading Hcode pragmas which are automatically 
 * converted to Lcode attributes.
 *
 *  Make the following assumption for pragma format
 *  since pragma only has string component:
 *
 *          pragma->specifier = \"<name><field>*\"
 *          <field> = \$<string> |
 *                    \!<label> |
 *                    \%<integer> |
 *                    \#<real>
 *
 *  <name> must be a valid Lcode identifier and becomes the attribute name.
 *
 *  <string> must be a string which does not contain any pragma delimiter: 
 *           {\$,\!,\%,\#} and becomes a string attribute field.
 *
 *  <label> must be a valid Lcode identifier and becomes a label attribute 
 *          field.
 *
 *  <integer> becomes an integer attribute field.
 *
 *  <real> becomes a double (floating-point) attribute field.
 * 
 *  Fields are converted to attribute field of appropriate type during Lcode
 *  generation.
 */


static char *
PL_find_invalid_char_for_Lcode_ident (char *string)
{
  return strpbrk (string, "#{}()<>[] \b\f\n\r\t\v");
}


static int
PL_num_prefix_backslashes (char *string, char *pos)
{
  int i;

  assert (string <= pos);
  for (i = -1; pos + i >= string && pos[i] == '\\'; i--);
  return -(i + 1);
}

static char *
PL_find_first_pragma_delimiter (char *string)
{
  char *delim, *first = string + strlen (string);

  delim = C_strstr (string, "\\$");
  if (delim != NULL && delim < first &&
      (PL_num_prefix_backslashes (string, delim) % 2 == 0))
    first = delim;

  delim = C_strstr (string, "\\!");
  if (delim != NULL && delim < first &&
      (PL_num_prefix_backslashes (string, delim) % 2 == 0))
    first = delim;

  delim = C_strstr (string, "\\%");
  if (delim != NULL && delim < first &&
      (PL_num_prefix_backslashes (string, delim) % 2 == 0))
    first = delim;

  delim = C_strstr (string, "\\#");
  if (delim != NULL && delim < first &&
      (PL_num_prefix_backslashes (string, delim) % 2 == 0))
    first = delim;

  return (first == string + strlen (string)) ? NULL : first;
}


/**********************/
/* Exported Functions */
/**********************/


/* 
 * Returns attribute name in 'name' formal parameter.  
 * 'Name' parameter must contain enough buffer space for attribute name.
 * Returns pointer to remaining pragma string as function return value.
 */
char *
PL_read_attr_name_from_pragma_str (char *pragmastr, char **name)
{
  char *delim, *inval, save = '\0';
  assert (pragmastr != NULL);

  delim = PL_find_first_pragma_delimiter (pragmastr);
  if (delim)
    {
      save = delim[0];
      delim[0] = '\0';
    }

  if ((inval = strpbrk (pragmastr, "#{}()<>[] \b\f\n\r\t\v")) != 0)
    P_punt ("Pragma name \"%s\" contains invalid character "
	    "ascii=%d (%c) for Lcode identifier",
	    pragmastr, inval[0], inval[0]);

#if PL_ELIDE_QUOTES
  if (pragmastr[0] == '"')
    *name = strdup(pragmastr + 1); /* skip '\"' prefix */
  else
    *name = strdup(pragmastr);
#else
  *name = strdup(pragmastr);
#endif

  if (delim)
    delim[0] = save;
#if PL_ELIDE_QUOTES
  else if ((*name)[strlen(*name) - 1] == '\"')
    (*name)[strlen (*name) - 1] = '\0';
  /* remove trailing '\"' from pragma string if no attr fields */
#endif

  return delim;
}

/*
 * Returns pointer to attribute field in one of 'string', 'integer', 
 * or 'real' formal parameters.  Attribute field type returned by 'type'
 * parameter which contains pointer to delimiter string.  Both 'label' and
 * 'string' attribute fields are returned in 'string' formal parameter.
 * 'String' parameter must contain enough buffer space to copy the field into.
 * Returns pointer to remaining pragma string as function return value.
 */
char *
PL_read_attr_field_from_pragma_str (char *pragmastr, char **string,
				    long int *integer, double *real,
				    char **delim_type)
{
  char *delim1, *delim2, *inval, save = '\0';
  Dyn_str_t *buf = NULL;

  buf = PL_dstr_new(256);

  assert (pragmastr != NULL);
  delim1 = PL_find_first_pragma_delimiter (pragmastr);

  if (delim1 != pragmastr)
    P_punt ("Pragma string must begin with delimiter to read attr field");

  delim2 = PL_find_first_pragma_delimiter (&delim1[2]);
  if (delim2)
    {
      save = delim2[0];
      delim2[0] = '\0';
    }

  switch (delim1[1])
    {
    case '$':
      PL_dstr_strcat(buf, "\"");
      PL_dstr_strcat(buf, &delim1[2]);
      if (buf->str[buf->size - 1] != '\"' || delim2)
	{
	  PL_dstr_strcat(buf, "\"");
	}
      break;

    case '!':
      if ((inval = strpbrk (&delim1[2], "#{}()<>[] \b\f\n\r\t\v")) != 0)
	P_punt ("Pragma label-field \"%s\"\n\tcontains invalid character "
		"ascii=%d (%c) for Lcode label attr",
		&delim1[2], inval[0], inval[0]);

      PL_dstr_strcat(buf, &delim1[2]);      
      if (!delim2 && buf->str[buf->size - 1] == '\"')
	PL_dstr_trunc(buf, 1); /* remove trailing quote */
      break;

    case '%':
      if (!sscanf (&delim1[2], "%ld", integer))
	P_punt ("Empty integer attr field in pragma string");
      break;

    case '#':
      if (!sscanf (&delim1[2], "%le", real))
	P_punt ("Empty double attr field in pragma string");
      break;

    default:
      P_punt ("Internal error: unknown pragma delimiter type");
    }

  *string = PL_dstr2str(buf);
  PL_dstr_free(buf);

  if (delim2)
    delim2[0] = save;
  *delim_type = delim1;
  return delim2;
}


/* Returns 1 if a loop iteration profiling attribute, 0 otherwise -JCG 4/99 */
int
PL_loop_iter_attr (char *buffer)
{
  if (!strcmp (buffer, L_ITER_INFO_HEADER) ||
      !strncmp (buffer, L_ITER_PREFIX, L_ITER_PREFIX_LENGTH))
    return (1);

  /* Otherwise, not a loop iter attribute */
  return (0);
}


/*
 *      (SAM 2-94) Allow only selective pragmas to go
 *      from Hcode to Lcode, because I was sick of looking
 *      at some stupid ones that are generated for Hcode 
 *      debugging!!!  So if you want a pragma to go from
 *      Hcode to Lcode, you need to add it here.
 */
int
PL_selected_pragma (char *buffer)
{
  if ((!strcmp (buffer, "FUNC")) ||
      (!strcmp (buffer, "FILE")) ||
      (!strcmp (buffer, "CONJDISJ") && PL_generate_static_branch_attrs) ||
      (!strcmp (buffer, "IFELSE") && PL_generate_static_branch_attrs) ||
      (!strcmp (buffer, "QUEST") && PL_generate_static_branch_attrs) ||
      /* next comparison should let either ACC_NAME or ACC_NAME_BY_TYPE
       * attributes pass through when parameter is set to yes */
      (!strncmp (buffer, "ACC_NAME", 8) && PL_generate_acc_name_attrs) ||
      (!strncmp (buffer, "ABS_ACC_NAME", 12)) ||
      (!strcmp (buffer, "LOOP")) ||
      (!strcmp (buffer, "SWP_INFO")) ||
      (!strcmp (buffer, "INLINE")) ||
      ((M_arch != M_TAHOE) && !strcmp (buffer, "CALLNAME")) ||
#if 0
      (!strcmp (buffer, "COMPREUSE")) ||
#endif
      (!strcmp (buffer, "call_info")) ||
      (!strcmp (buffer, "old_style_param")) ||
      (!strcmp (buffer, "append_gcc_ellipsis")) ||
      (!strcmp (buffer, "use_ret_as_parm0")) ||
      (!strcmp (buffer, "ret_st")) ||
      (PL_loop_iter_attr (buffer)) ||
      (!strcmp (buffer, "impact_info")) ||
      (!strcmp (buffer, "host_info")) ||
      (!strcmp (buffer, "preprocess_info")) ||
      (!strcmp (buffer, "PCODE_PROBE")) ||
      (!strcmp (buffer, "JSR_SIDE_EFFECT")) ||
      (!strcmp (buffer, "POS")) ||
      (!strcmp (buffer, "SCOPE")) ||
      (!strcmp (buffer, "CALL_CONV")) ||
      (!strcmp (buffer, "NOT-CALLED")) ||
      (!strcmp (buffer, "CALLED")) ||
      (!strcmp (buffer, "RECURSIVE")) ||
      ((!strcmp (buffer, "ACC_SPECS")) && PL_gen_acc_specs) ||
      ((!strcmp (buffer, "ACC_OMEGA")) && PL_gen_acc_specs && 
       PL_annotate_omega)
#ifdef PL_GENERATE_VAR_ATTRS
      ||
      !strcmp (buffer, "Cattr") ||
      !strcmp (buffer, "Cvisibility") ||
      !strcmp (buffer, "Calias") || !strcmp (buffer, "Csection")
#endif
      ||
      !strncmp (buffer, "IPC-", 4) ||
      !strcmp (buffer, "innerloop")
    )
    return 1;
  else
    return 0;
}

/*
 * Lcode attributes
 * ----------------------------------------------------------------------
 */

L_Attr *
PL_gen_single_attr_from_pragma (L_Attr **attr_list, Pragma pragma)
{
  char *buffer = NULL;
  Expr expr;
  char *pragmastr, *delim;
  int field;
  long int integer;
  double real;
  L_Attr *attr = NULL, *old_attr;

  if (!pragma)
    return NULL;

  pragmastr = PL_read_attr_name_from_pragma_str (pragma->specifier, 
						 &buffer);

  if (PL_selected_pragma (buffer))
    {
      /* DMG - combine attributes with same name 3/31/95 */

      if (attr_list)
	{
	  if (!(old_attr = L_find_attr (*attr_list, buffer)))
	    {
	      attr = L_new_attr (buffer, 0);
	      *attr_list = L_concat_attr (*attr_list, attr);
	      field = 0;
	    }
	  else
	    {
	      attr = old_attr;
	      field = attr->max_field;
	    }
	}
      else
	{
	  attr = L_new_attr (buffer, 0);
	  field = 0;
	}

      /*  
       *  GEH - 3/95
       *  Make the following assumption for pragma format
       *  since pragma only has string component:
       *
       *          pragma->specifier = \"<name><field>*\"
       *          <field> = \$<string> |
       *                    \!<label> |
       *                    \%<integer> |
       *                    \#<real>
       *
       *  <name> must be a valid Lcode identifier and becomes the
       *  attribute name.  Fields are converted to attribute field of
       *  appropriate type.  
       */

      while (pragmastr)
	{
	  char *field_buffer = NULL;

	  pragmastr =
	    PL_read_attr_field_from_pragma_str (pragmastr, &field_buffer,
						&integer, &real, &delim);
	  switch (delim[1])
	    {
	    case '$':
	      L_set_string_attr_field (attr, field, field_buffer);
	      break;
	    case '!':
	      L_set_label_attr_field (attr, field, field_buffer);
	      break;
	    case '%':
	      L_set_int_attr_field (attr, field, integer);
	      break;
	    case '#':
	      L_set_double_attr_field (attr, field, real);
	      break;
	    default:
	      P_punt ("Internal error: unknown pragma delimiter type");
	    }
	  field++;
      
	  free(field_buffer);
	}
  
      for (expr = pragma->expr; expr; expr = expr->next)
	{
	  switch (expr->opcode)
	    {
	    case OP_var:
	      {
		char *val = expr->value.var.name;
		char *buf = malloc (strlen (val) + 3);
		sprintf (buf, "\"%s\"", val);
		L_set_string_attr_field (attr, field, buf);
		free (buf);
		break;
	      }
	    case OP_string:
	      {
		char *val = expr->value.string;
		char *buf = malloc (strlen (val) + 3);
		sprintf (buf, "\"%s\"", val);
		L_set_string_attr_field (attr, field, buf);
		free (buf);
		break;
	      }
	    case OP_int:
	      {
		ITintmax val = expr->value.scalar;
		L_set_int_attr_field (attr, field, val);
		break;
	      }
	    case OP_real:
	    case OP_double:
	      {
		double val = expr->value.real;
		L_set_double_attr_field (attr, field, val);
		break;
	      }
	    case OP_float:
	      {
		float val = (float) expr->value.real;
		L_set_float_attr_field (attr, field, val);
		break;
	      }
	    default:
	      P_punt ("PL_gen_attr_from_pragma: "
		      "Unhandled attribute expression %d", expr->opcode);
	    }
	  field++;
	}
    }

  free(buffer);

  return attr;
}

L_Attr *
PL_gen_attr_from_pragma (Pragma pragma)
{
  Pragma ptr;
  L_Attr *new_attr = NULL;

  if (!pragma)
    return (NULL);

  for (ptr = pragma; ptr != NULL; ptr = ptr->next)
    PL_gen_single_attr_from_pragma (&new_attr, ptr);

  return (new_attr);
}


L_Attr *
PL_gen_attr (char *name, int value)
{
  L_Attr *new_attr;

  new_attr = L_new_attr (name, 1);
  L_set_int_attr_field (new_attr, 0, value);

  return (new_attr);
}
