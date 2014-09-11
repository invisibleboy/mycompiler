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

/* Skip 'count' lines in file 'in' */
static void AP_skip_lines (in, count, file_name)
FILE *in;
int count;
char *file_name;
{
  char buf[500];
  int i;
  
  for (i=0; i < count; i++)
    {
      if (fgets (buf, sizeof(buf), in) == NULL)
	L_punt ("Unexpected EOF in ap profile file '%s'.",
		file_name);
    }
}

/*
 * Reads the value profile info from 'file_name' and inserts
 * it incode the fn as an attribute.
 */
void L_get_ap_profile (fn, file_name)
L_Func *fn;
char *file_name;
{
    L_Oper *mem_op;
    L_Attr *attr;
    char func_name[500];
    int func_size;
    /* 10/25/04 REK Commenting out unused variable to quiet compiler
     *              warning. */
#if 0
    int num_info;
#endif
    FILE *in;
    double fn_weight;
    double op_weight; 
    double correct_predictions; 
    double predictions;
    int prediction_rate;
    int i;
    int id;
    int err;
    int profile_count;
   
    /* Open the profile file for reading */
    if ((in = fopen (file_name, "r")) == NULL)
	L_punt ("Error opening ap profile file '%s'.", file_name);

    err = fscanf(in,"(count %d)\n",&profile_count);
    if (err != 1)
      L_punt ("Error opening ap profile file '%s'.", file_name);
    
    /* Skip functions until fn->name is found */
    while ((fscanf (in, "(begin %s %lf %i)\n", func_name, &fn_weight, 
		&func_size) == 3) &&
	   (strcmp (func_name, fn->name) != 0))
	AP_skip_lines (in, func_size + 1, file_name);
    
    /* Make fn->name was found in profile file */
    if (strcmp (func_name, fn->name) != 0)
	L_punt ("Function '%s' not found in value profile file '%s'.",
		func_name, file_name);

  /*
   * For each value operation in data base, create value attributes
   */
  for (i=0; i < func_size; i++) {

    /* Read value operation data */
    if (fscanf (in, "( load %i %lf %lf %lf )",
	 &id, &op_weight, &correct_predictions, &predictions) != 4) {
    }

    mem_op = L_oper_hash_tbl_find_oper(fn->oper_hash_tbl,id);

    /* Check the last value weight */
    if (predictions > 1 ) {
      prediction_rate = (int)((correct_predictions/predictions) * 100);
      attr = L_new_attr ("profile_address_pred", 1);
      L_set_int_attr_field(attr, 0, prediction_rate);
      mem_op->attr = L_concat_attr (mem_op->attr, attr);
    }
    fscanf(in,"\n");
  }

 if (fscanf (in, "(end %s)\n", func_name) != 1)
    L_punt ("Parse error in address prediction profile file '%s'.", file_name);
 
  fclose (in);
}

