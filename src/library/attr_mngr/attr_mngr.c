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
 *      File:   attr_mngr.c
 *      Author: Teresa Johnson
 *      Creation Date:  1994
 *      Copyright (c) 1994 Teresa Johnson, Wen-mei Hwu and
 *                         The Board of Trustees of the University of Illinois.
 *                         All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <library/l_alloc_new.h>
#include <library/i_error.h>
#include <library/attr_mngr.h>

/* L_Alloc_Pool's */
L_Alloc_Pool *L_AttrMngr_pool = NULL;
L_Alloc_Pool *A_Attr_pool = NULL;

L_AttrMngr *
L_create_AttrMngr (char *fn_name, float weight)
{
  L_AttrMngr *L_AM;

  if (!L_AttrMngr_pool)
    L_AttrMngr_pool =
      L_create_alloc_pool ("L_AttrMngr_pool", sizeof (L_AttrMngr), 1);
  if (!A_Attr_pool)
    A_Attr_pool = L_create_alloc_pool ("A_Attr_pool", sizeof (A_Attr), 1);
  L_AM = (L_AttrMngr *) L_alloc (L_AttrMngr_pool);
  L_AM->fn_name = strdup (fn_name);
  L_AM->fn_attr = L_AM->cb_attr = L_AM->op_attr = 0;
  L_AM->num_fn_attr = L_AM->num_cb_attr = L_AM->num_op_attr = 0;
  L_AM->weight = weight;
  return L_AM;
}

A_Attr_Field *
A_new_int_field (int value)
{
  A_Attr_Field *field;

  field = (A_Attr_Field *) malloc (sizeof (A_Attr_Field));
  field->type = A_INT;
  field->value.i = value;
  return field;
}

A_Attr_Field *
A_new_float_field (double value)
{
  A_Attr_Field *field;

  field = (A_Attr_Field *) malloc (sizeof (A_Attr_Field));
  field->type = A_FLOAT;
  field->value.f = value;
  return field;
}

A_Attr_Field *
A_new_string_field (char *value)
{
  A_Attr_Field *field;

  field = (A_Attr_Field *) malloc (sizeof (A_Attr_Field));
  field->type = A_STRING;
  field->value.s = strdup (value);
  return field;
}

A_Attr_Field *
A_new_label_field (char *value)
{
  A_Attr_Field *field;

  field = (A_Attr_Field *) malloc (sizeof (A_Attr_Field));
  field->type = A_LABEL;
  field->value.l = strdup (value);
  return field;
}

void
L_insert_fn_attr (L_AttrMngr * func_attr, char *attr_name, long attr_value)
{
  L_insert_fn_attr_int (func_attr, attr_name, attr_value);
}

void
L_insert_fn_attr_int (L_AttrMngr * func_attr, char *attr_name,
                      long attr_value)
{
  A_Attr_Field **field;

  field = (A_Attr_Field **) malloc (sizeof (A_Attr_Field *));
  field[0] = A_new_int_field ((int) attr_value);
  L_insert_fn_attr_list (func_attr, attr_name, field, 1);
}

void
L_insert_fn_attr_float (L_AttrMngr * func_attr, char *attr_name,
                        float attr_value)
{
  A_Attr_Field **field;

  field = (A_Attr_Field **) malloc (sizeof (A_Attr_Field *));
  field[0] = A_new_float_field (attr_value);
  L_insert_fn_attr_list (func_attr, attr_name, field, 1);
}

void
L_insert_fn_attr_string (L_AttrMngr * func_attr, char *attr_name,
                         char *attr_value)
{
  A_Attr_Field **field;

  field = (A_Attr_Field **) malloc (sizeof (A_Attr_Field *));
  field[0] = A_new_string_field (attr_value);
  L_insert_fn_attr_list (func_attr, attr_name, field, 1);
}

void
L_insert_fn_attr_label (L_AttrMngr * func_attr, char *attr_name,
                        char *attr_value)
{
  A_Attr_Field **field;

  field = (A_Attr_Field **) malloc (sizeof (A_Attr_Field *));
  field[0] = A_new_label_field (attr_value);
  L_insert_fn_attr_list (func_attr, attr_name, field, 1);
}

void
L_insert_fn_attr_list (L_AttrMngr * func_attr, char *attr_name,
                       A_Attr_Field ** field, int max_field)
{
  A_Attr *new_attr;
  if (!func_attr->fn_attr)
    {
      func_attr->fn_attr = (A_Attr *) L_alloc (A_Attr_pool);
      new_attr = func_attr->fn_attr;
    }
  else
    {
      new_attr = func_attr->fn_attr;
      while (new_attr->next_attr)
        new_attr = new_attr->next_attr;
      new_attr->next_attr = (A_Attr *) L_alloc (A_Attr_pool);
      new_attr = new_attr->next_attr;
    }
  new_attr->name = strdup (attr_name);
  new_attr->field = field;
  new_attr->num_fields = max_field;
  new_attr->next_attr = 0;
  func_attr->num_fn_attr++;
}

void
L_insert_cb_attr (L_AttrMngr * func_attr, int cb_num, char *attr_name,
                  long attr_value)
{
  L_insert_cb_attr_int (func_attr, cb_num, attr_name, attr_value);
}

void
L_insert_cb_attr_int (L_AttrMngr * func_attr, int cb_num, char *attr_name,
                      long attr_value)
{
  A_Attr_Field **field;

  field = (A_Attr_Field **) malloc (sizeof (A_Attr_Field *));
  field[0] = A_new_int_field ((int) attr_value);
  L_insert_cb_attr_list (func_attr, cb_num, attr_name, field, 1);
}

void
L_insert_cb_attr_float (L_AttrMngr * func_attr, int cb_num, char *attr_name,
                        float attr_value)
{
  A_Attr_Field **field;

  field = (A_Attr_Field **) malloc (sizeof (A_Attr_Field *));
  field[0] = A_new_float_field (attr_value);
  L_insert_cb_attr_list (func_attr, cb_num, attr_name, field, 1);
}

void
L_insert_cb_attr_string (L_AttrMngr * func_attr, int cb_num, char *attr_name,
                         char *attr_value)
{
  A_Attr_Field **field;

  field = (A_Attr_Field **) malloc (sizeof (A_Attr_Field *));
  field[0] = A_new_string_field (attr_value);
  L_insert_cb_attr_list (func_attr, cb_num, attr_name, field, 1);
}

void
L_insert_cb_attr_label (L_AttrMngr * func_attr, int cb_num, char *attr_name,
                        char *attr_value)
{
  A_Attr_Field **field;

  field = (A_Attr_Field **) malloc (sizeof (A_Attr_Field *));
  field[0] = A_new_label_field (attr_value);
  L_insert_cb_attr_list (func_attr, cb_num, attr_name, field, 1);
}

void
L_insert_cb_attr_list (L_AttrMngr * func_attr, int cb_num, char *attr_name,
                       A_Attr_Field ** field, int max_field)
{
  A_Attr *new_attr;
  if (!func_attr->cb_attr)
    {
      func_attr->cb_attr = (A_Attr *) L_alloc (A_Attr_pool);
      new_attr = func_attr->cb_attr;
    }
  else
    {
      new_attr = func_attr->cb_attr;
      while (new_attr->next_attr)
        new_attr = new_attr->next_attr;
      new_attr->next_attr = (A_Attr *) L_alloc (A_Attr_pool);
      new_attr = new_attr->next_attr;
    }
  new_attr->name = strdup (attr_name);
  new_attr->field = field;
  new_attr->num_fields = max_field;
  new_attr->id_num = cb_num;
  new_attr->next_attr = 0;
  func_attr->num_cb_attr++;
}

void
L_insert_op_attr (L_AttrMngr * func_attr, int op_num, char *attr_name,
                  long attr_value, int dep)
{
  L_insert_op_attr_int (func_attr, op_num, attr_name, attr_value, dep);
}

void
L_insert_op_attr_int (L_AttrMngr * func_attr, int op_num, char *attr_name,
                      long attr_value, int dep)
{
  A_Attr_Field **field;

  field = (A_Attr_Field **) malloc (sizeof (A_Attr_Field *));
  field[0] = A_new_int_field ((int) attr_value);
  L_insert_op_attr_list (func_attr, op_num, attr_name, field, 1, dep);
}

void
L_insert_op_attr_float (L_AttrMngr * func_attr, int op_num, char *attr_name,
                        float attr_value, int dep)
{
  A_Attr_Field **field;

  field = (A_Attr_Field **) malloc (sizeof (A_Attr_Field *));
  field[0] = A_new_float_field (attr_value);
  L_insert_op_attr_list (func_attr, op_num, attr_name, field, 1, dep);
}

void
L_insert_op_attr_string (L_AttrMngr * func_attr, int op_num, char *attr_name,
                         char *attr_value, int dep)
{
  A_Attr_Field **field;

  field = (A_Attr_Field **) malloc (sizeof (A_Attr_Field *));
  field[0] = A_new_string_field (attr_value);
  L_insert_op_attr_list (func_attr, op_num, attr_name, field, 1, dep);
}

void
L_insert_op_attr_label (L_AttrMngr * func_attr, int op_num, char *attr_name,
                        char *attr_value, int dep)
{
  A_Attr_Field **field;

  field = (A_Attr_Field **) malloc (sizeof (A_Attr_Field *));
  field[0] = A_new_label_field (attr_value);
  L_insert_op_attr_list (func_attr, op_num, attr_name, field, 1, dep);
}

void
L_insert_op_attr_list (L_AttrMngr * func_attr, int op_num, char *attr_name,
                       A_Attr_Field ** field, int max_field, int dep)
{
  A_Attr *new_attr;
  if (!func_attr->op_attr)
    {
      func_attr->op_attr = (A_Attr *) L_alloc (A_Attr_pool);
      new_attr = func_attr->op_attr;
    }
  else
    {
      new_attr = func_attr->op_attr;
      while (new_attr->next_attr)
        new_attr = new_attr->next_attr;
      new_attr->next_attr = (A_Attr *) L_alloc (A_Attr_pool);
      new_attr = new_attr->next_attr;
    }
  new_attr->name = strdup (attr_name);
  new_attr->field = field;
  new_attr->num_fields = max_field;
  new_attr->dep = dep;
  new_attr->id_num = op_num;
  new_attr->next_attr = 0;
  func_attr->num_op_attr++;
}

void
L_write_attr_to_file (FILE * out, L_AttrMngr * func_attr)
{
  A_Attr *this_attr;
  int i;

  if (fprintf (out, "# Attribute Manager file --- Version %d\n",
               AM_VERSION) < 0)
    {
      I_punt ("Attribute Manager cannot write to file");
      return;
    }
  fprintf (out, "begin %s %d %d %d %f\n", func_attr->fn_name,
           func_attr->num_fn_attr, func_attr->num_cb_attr,
           func_attr->num_op_attr, func_attr->weight);
  this_attr = func_attr->fn_attr;
  while (this_attr)
    {
      fprintf (out, "%s %d", this_attr->name, this_attr->num_fields);
      for (i = 0; i < this_attr->num_fields; i++)
        {
          if (!this_attr->field[i])
            fprintf (out, " NULL");
          else
            {
              switch (this_attr->field[i]->type)
                {
                case A_INT:
                  fprintf (out, " i %d", this_attr->field[i]->value.i);
                  break;
                case A_FLOAT:
                  fprintf (out, " f %.3f", this_attr->field[i]->value.f);
                  break;
                case A_STRING:
                  fprintf (out, " s %s", this_attr->field[i]->value.s);
                  break;
                case A_LABEL:
                  fprintf (out, " l %s", this_attr->field[i]->value.l);
                  break;
                default:
                  I_punt ("L_write_attr_to_file: Invalid attr type\n");
                }
            }
        }
      fprintf (out, "\n");
      this_attr = this_attr->next_attr;
    }
  this_attr = func_attr->cb_attr;
  while (this_attr)
    {
      fprintf (out, "%d %s %d", this_attr->id_num, this_attr->name,
               this_attr->num_fields);
      for (i = 0; i < this_attr->num_fields; i++)
        {
          if (!this_attr->field[i])
            fprintf (out, " NULL");
          else
            {
              switch (this_attr->field[i]->type)
                {
                case A_INT:
                  fprintf (out, " i %d", this_attr->field[i]->value.i);
                  break;
                case A_FLOAT:
                  fprintf (out, " f %.3f", this_attr->field[i]->value.f);
                  break;
                case A_STRING:
                  fprintf (out, " s %s", this_attr->field[i]->value.s);
                  break;
                case A_LABEL:
                  fprintf (out, " l %s", this_attr->field[i]->value.l);
                  break;
                default:
                  I_punt ("L_write_attr_to_file: Invalid attr type\n");
                }
            }
        }
      fprintf (out, "\n");
      this_attr = this_attr->next_attr;
    }
  this_attr = func_attr->op_attr;
  while (this_attr)
    {
      fprintf (out, "%d %s %d", this_attr->id_num, this_attr->name,
               this_attr->num_fields);
      for (i = 0; i < this_attr->num_fields; i++)
        {
          if (!this_attr->field[i])
            fprintf (out, " NULL");
          else
            {
              switch (this_attr->field[i]->type)
                {
                case A_INT:
                  fprintf (out, " i %d", this_attr->field[i]->value.i);
                  break;
                case A_FLOAT:
                  fprintf (out, " f %.3f", this_attr->field[i]->value.f);
                  break;
                case A_STRING:
                  fprintf (out, " s %s", this_attr->field[i]->value.s);
                  break;
                case A_LABEL:
                  fprintf (out, " l %s", this_attr->field[i]->value.l);
                  break;
                default:
                  I_punt
                    ("L_write_attr_to_file: Invalid attr %s (%d) type %d\n",
                     this_attr->name, i, this_attr->field[i]->type);
                }
            }
        }
      if (this_attr->dep)
        fprintf (out, " %d\n", this_attr->dep);
      else
        fprintf (out, "\n");
      this_attr = this_attr->next_attr;
    }
  fprintf (out, "end %s\n\n", func_attr->fn_name);
}

void
L_free_AttrMngr (L_AttrMngr * func_attr)
{
  L_free_Attr (func_attr->fn_attr);
  L_free_Attr (func_attr->cb_attr);
  L_free_Attr (func_attr->op_attr);
  L_free (L_AttrMngr_pool, func_attr);
}

void
L_free_Attr (A_Attr * Attr)
{
  A_Attr *this_attr, *next;
  int i;

  this_attr = Attr;
  while (this_attr)
    {
      next = this_attr->next_attr;
      for (i = 0; i < this_attr->num_fields; i++)
        if (this_attr->field[i])
          free (this_attr->field[i]);
      free (this_attr->field);
      L_free (A_Attr_pool, this_attr);
      this_attr = next;
    }
}
