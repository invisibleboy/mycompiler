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
/*===========================================================================
 *      File :          l_get_value.c
 *      Description :   Inserts value profile into Lcode
 *      Creation Date : Oct, 1997
 *      Author :        Daniel A. Connors
 *
 *      (C) Copyright 1997, Daniel A. Connors and Wen-mei Hwu
 *      All rights granted to University of Illinois Board of Regents.
 *==========================================================================*/
#include <config.h>
#include <stdio.h>
#include <Lcode/l_main.h>

#define LAST_VALUE_ATTR                   "last_value"
#define STRIDE_VALUE_ATTR                 "stride_value"
#define VALUE_ATTR                        "value"
#define VALUE_WEIGHT_ATTR                 "value_weight"
#define VALUE_LOCALITY_ATTR               "value_locality"

extern int  Lprofile_util_insert_value_profile;
extern int  Lprofile_util_value_profile_percentage;

/* Skip 'count' lines in file 'in' */
static void VALUE_skip_lines (in, count, file_name)
FILE *in;
int count;
char *file_name;
{
  char buf[500];
  int i;
  
  for (i=0; i < count; i++)
    {
      if (fgets (buf, sizeof(buf), in) == NULL)
	L_punt ("Unexpected EOF in value profile file '%s'.",
		file_name);
    }
}



void L_find_and_remove_value_attributes(L_Func *fn)
{
 L_Cb *cb;
 L_Oper *oper;
 L_Attr *attr;
 int i;
 char attr_name[15];

 for (cb = fn->first_cb; cb; cb = cb->next_cb) {
   for (oper = cb->first_op; oper; oper = oper->next_op) {
     
     /* Delete all LAST_VALUE attributes */
     i = 0;
     sprintf(attr_name,"%s_%d",LAST_VALUE_ATTR,i);
     while (((attr = L_find_attr(oper->attr,attr_name)) != NULL)) {
       oper->attr = L_delete_attr (oper->attr, attr);
       i++;
     }
     
     /* Delete all STRIDE_VALUE attributes */
     i = 0;
     sprintf(attr_name,"%s_%d",STRIDE_VALUE_ATTR,i);
     while (((attr = L_find_attr(oper->attr,attr_name)) != NULL)) {
       oper->attr = L_delete_attr (oper->attr, attr);
       i++;
     }

     /* Delete all VALUE_LOCALITY attributes */
     i = 0;
     sprintf(attr_name,"%s_%d",VALUE_LOCALITY_ATTR,i);
     while (((attr = L_find_attr(oper->attr,attr_name)) != NULL)) {
       oper->attr = L_delete_attr (oper->attr, attr);
       i++;
     }

     /* Delete all VALUE attributes */
     i = 0;
     sprintf(attr_name,"%s_%d",VALUE_ATTR,i);
     while (((attr = L_find_attr(oper->attr,attr_name)) != NULL)) {
       oper->attr = L_delete_attr (oper->attr, attr);
       i++;
     }

     /* Delete all VALUE_WEIGHT attributes */
     i = 0;
     sprintf(attr_name,"%s_%d",VALUE_WEIGHT_ATTR,i);
     while (((attr = L_find_attr(oper->attr,attr_name)) != NULL)) {
       oper->attr = L_delete_attr (oper->attr, attr);
       i++;
     }
   }
 }
}


/*
 * Reads the value profile info from 'file_name' and inserts
 * it incode the fn as an attribute.
 */
void L_get_value_profile (fn, file_name)
L_Func *fn;
char *file_name;
{
  L_Oper *value_op;
  L_Attr *attr;
  char func_name[500];
  int func_size;
  int num_info;
  FILE *in;
  double fn_weight;
  int op_weight; 
  int value_weight; 
  int last_weight;
  int stride_weight;
  int i,j;
  int value_id;
  int value;
  int value_percentage;
  int err;
  int profile_count;
  int num_items;
  /* 10/25/04 REK Commenting out unused variable to quiet compiler warning. */
#if 0
  int parm_type;
#endif
  int parm_number;
  char op_type[10];
  char attr_name[15];
  L_Attr *value_attr;
  L_Attr *value_weight_attr;
  int profile_percentage, percentage;
  int num_values;
  int sum_weight;

  /* Open the profile file for reading */
  if ((in = fopen (file_name, "r")) == NULL)
    L_punt ("Error opening value profile file '%s'.", file_name);
  
  err = fscanf(in,"(count %d)\n",&profile_count);
  if (err != 1)
    L_punt ("Error opening value profile file '%s'.", file_name);
  
  /* Skip functions until fn->name is found */
  while ((fscanf (in, "(begin %s %lf %i)\n", func_name, &fn_weight, 
		  &func_size) == 3) &&
	 (strcmp (func_name, fn->name) != 0))
    VALUE_skip_lines (in, func_size + 1, file_name);
  
  /* Make fn->name was found in profile file */
  if (strcmp (func_name, fn->name) != 0)
    L_punt ("Function '%s' not found in value profile file '%s'.",
	    func_name, file_name);
  
  /*
   * For each value operation in data base, create value attributes
   */
  value_percentage = Lprofile_util_value_profile_percentage;
  for (i=0; i < func_size; i++) {
    
    /* Read value operation data */
    num_items = 1;
    
    if (fscanf (in, "( %s  %i %i %i %i %i %i )",
		op_type, &value_id, &op_weight, &num_info, &last_weight, &stride_weight, &parm_number) != 7) {
      L_punt ("Parse error in value profile file '%s'.", file_name);
    }
    
    value_op = L_oper_hash_tbl_find_oper(fn->oper_hash_tbl,value_id);
    
    /* Check the last value weight */
    if (op_weight > 0 ) {
      
      /* Create the last value execution frequency attribute */
      sprintf(attr_name,"%s_%d",LAST_VALUE_ATTR,parm_number);
      attr = L_new_attr (attr_name, 1);
      L_set_int_attr_field(attr, 0, last_weight);
      value_op->attr = L_concat_attr (value_op->attr, attr);

      /* Create the stride value execution frequency attribute */
      sprintf(attr_name,"%s_%d",STRIDE_VALUE_ATTR,parm_number);
      attr = L_new_attr (attr_name, 1);
      L_set_int_attr_field(attr, 0, stride_weight);
      value_op->attr = L_concat_attr (value_op->attr, attr);

      /* Create the base attribute for VALUE */ 
      sprintf(attr_name,"%s_%d",VALUE_ATTR,parm_number);
      value_attr = L_new_attr (attr_name, num_info);
      value_op->attr = L_concat_attr (value_op->attr, value_attr);

      /* Create the base attribute for VALUE_WEIGHT */
      sprintf(attr_name,"%s_%d",VALUE_WEIGHT_ATTR,parm_number);
      value_weight_attr = L_new_attr (attr_name, num_info);
      value_op->attr = L_concat_attr (value_op->attr, value_weight_attr);

      profile_percentage = 0;
      num_values = 0;
      sum_weight = 0;

      /* Scan thru the values that were collected */
      for (j=0; j < num_info; j++) {
	
	/* Read conflict data */
	if (fscanf (in, "( %i : %d )",&value, &value_weight) != 2)
	  L_punt ("Parse error in value profile file '%s'.", file_name);
	
	/* Set the value */
	L_set_int_attr_field(value_attr, j, value);
	
	/* Set the value weight */
	L_set_int_attr_field(value_weight_attr, j, value_weight);

	sum_weight += value_weight;

	if (num_values==0) {

	  percentage = 100*((int)(((float) sum_weight) / ((float) op_weight)));
	  
	  if (percentage >= Lprofile_util_value_profile_percentage) {
	    profile_percentage = percentage;
	    num_values = j+1;
	  }
	}
      }

      if (profile_percentage >= Lprofile_util_value_profile_percentage) {
	/* Create the stride value execution frequency attribute */
	sprintf(attr_name,"%s_%d",VALUE_LOCALITY_ATTR,parm_number);
	attr = L_new_attr (attr_name, 2);
	L_set_int_attr_field(attr, 0, profile_percentage);
	L_set_int_attr_field(attr, 1, num_values);
	value_op->attr = L_concat_attr (value_op->attr, attr);
      }


    }
    
    fscanf(in,"\n");
  }
  
  if (fscanf (in, "(end %s)\n", func_name) != 1)
    L_punt ("Parse error in memory dependence profile file '%s'.", file_name);
  
  fclose (in);
}

