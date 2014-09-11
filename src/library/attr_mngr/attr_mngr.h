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
 *      File:   l_AttrMngr.h
 *      Author: Teresa Johnson
 *      Creation Date:  1994
 *      Copyright (c) 1994 Teresa Johnson, Wen-mei Hwu and
 *                         The Board of Trustees of the University of Illinois.
 *                         All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/
#ifndef IMPACT_ATTR_MNGR
#define IMPACT_ATTR_MNGR

/* 10/29/02 REK Adding config.h */
#include <config.h>

/* Version number of Attribute Manager output files */
#define AM_VERSION      3

#define A_INT           0
#define A_FLOAT         1
#define A_STRING        2
#define A_LABEL         3

typedef struct A_Attr_Field
{
  int type;
  union
  {
    int i;
    double f;
    char *s;
    char *l;
  }
  value;
}
A_Attr_Field;

typedef struct A_Attr
{
  char *name;
  A_Attr_Field **field;
  int num_fields;
  int dep;
  struct A_Attr *next_attr;
  int id_num;
}
A_Attr;

typedef struct L_AttrMngr
{
  char *fn_name;
  A_Attr *fn_attr;
  A_Attr *cb_attr;
  A_Attr *op_attr;
  int num_fn_attr;
  int num_cb_attr;
  int num_op_attr;
  float weight;
}
L_AttrMngr;

#ifdef __cplusplus
extern "C"
{
#endif

/* Functions to be called externally */
  L_AttrMngr *L_create_AttrMngr (char *fn_name, float weight);

/* Keep these for backward compatibility */
  void L_insert_fn_attr (L_AttrMngr * func_attr, char *attr_name,
                         long attr_value);
  void L_insert_cb_attr (L_AttrMngr * func_attr, int cb_num, char *attr_name,
                         long attr_value);
  void L_insert_op_attr (L_AttrMngr * func_attr, int op_num, char *attr_name,
                         long attr_value, int dep);

/* New versions */
  void L_insert_fn_attr_int (L_AttrMngr * func_attr, char *attr_name,
                             long attr_value);
  void L_insert_cb_attr_int (L_AttrMngr * func_attr, int cb_num,
                             char *attr_name, long attr_value);
  void L_insert_op_attr_int (L_AttrMngr * func_attr, int op_num,
                             char *attr_name, long attr_value, int dep);
  void L_insert_fn_attr_float (L_AttrMngr * func_attr, char *attr_name,
                               float attr_value);
  void L_insert_cb_attr_float (L_AttrMngr * func_attr, int cb_num,
                               char *attr_name, float attr_value);
  void L_insert_op_attr_float (L_AttrMngr * func_attr, int op_num,
                               char *attr_name, float attr_value, int dep);
  void L_insert_fn_attr_string (L_AttrMngr * func_attr, char *attr_name,
                                char *attr_value);
  void L_insert_cb_attr_string (L_AttrMngr * func_attr, int cb_num,
                                char *attr_name, char *attr_value);
  void L_insert_op_attr_string (L_AttrMngr * func_attr, int op_num,
                                char *attr_name, char *attr_value, int dep);
  void L_insert_fn_attr_label (L_AttrMngr * func_attr, char *attr_name,
                               char *attr_value);
  void L_insert_cb_attr_label (L_AttrMngr * func_attr, int cb_num,
                               char *attr_name, char *attr_value);
  void L_insert_op_attr_label (L_AttrMngr * func_attr, int op_num,
                               char *attr_name, char *attr_value, int dep);

/* New for version 3 */
  void L_insert_fn_attr_list (L_AttrMngr * func_attr, char *attr_name,
                              A_Attr_Field ** field, int max_field);
  void L_insert_cb_attr_list (L_AttrMngr * func_attr, int cb_num,
                              char *attr_name, A_Attr_Field ** field,
                              int max_field);
  void L_insert_op_attr_list (L_AttrMngr * func_attr, int op_num,
                              char *attr_name, A_Attr_Field ** field,
                              int max_field, int dep);

  void L_write_attr_to_file (FILE * out, L_AttrMngr * func_attr);
  void L_free_AttrMngr (L_AttrMngr * func_attr);

/* Functions for internal use only */
  void L_free_Attr (A_Attr * Attr);

#ifdef __cplusplus
}
#endif

#endif
