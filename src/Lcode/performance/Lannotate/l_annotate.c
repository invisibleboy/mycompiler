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
 *      File:   l_annotate.c
 *      Author: Teresa Johnson
 *      Creation Date:  1994
 *      Copyright (c) 1994 Teresa Johnson, Wen-mei Hwu and
 *                         The Board of Trustees of the University of Illinois.
 *                         All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#include <config.h>

#ifndef lint
#define lint
static char copyright[] =
"@(#) Copyright (c) 1994 Teresa Johnson, Wen-mei Hwu and The Board of \n\
 Trustees of the University of Illinois. All rights reserved.\n";
#endif

#include <stdio.h>
#include <Lcode/l_main.h>
#include <library/attr_mngr.h>
#include "l_annotate.h"
#include "l_hash.h"

#define TRUE 1
#define FALSE 0

/* Parameters of Lannotate - see also STD_PARMS */
char *A_mode = "extract";
char *A_inx_file = "wc.annot_index";
char *A_src1_file = "wc.annot";
char *A_src2_file = "wc_sum.annot";
char *A_dest_file = "wc_sum.annot";
char *A_fn_attr_list = "";
char *A_cb_attr_list = "";
char *A_op_attr_list = "";
char *A_type = "average";
int  Lannotate_remove_existing_sync_arcs = 1;
int  Lannotate_gen_sync_arcs = 0;

/* Hold list of function attributes */
L_Attr *fn_attr;

/* version of Attribute Manager file */
int version;

A_Attr_Field **make_attr_field_list(L_Operand **operands,int max_field);

/*
 * Read Lannotate parameters
 */
void L_read_parm_Lannotate (ppi)
Parm_Parse_Info *ppi;
{
        L_read_parm_s(ppi, "mode", &A_mode);
        L_read_parm_s(ppi, "index_file_name", &A_inx_file);
        L_read_parm_s(ppi, "src1_file_name", &A_src1_file);
        L_read_parm_s(ppi, "src2_file_name", &A_src2_file);
        L_read_parm_s(ppi, "dest_file_name", &A_dest_file);
        L_read_parm_s(ppi, "fn_attr_list", &A_fn_attr_list);
        L_read_parm_s(ppi, "cb_attr_list", &A_cb_attr_list);
        L_read_parm_s(ppi, "op_attr_list", &A_op_attr_list);
        L_read_parm_s(ppi, "type", &A_type);
        L_read_parm_b(ppi, "remove_existing_sync_arcs", 
					&Lannotate_remove_existing_sync_arcs);
        L_read_parm_b(ppi, "generate_profiled_sync_arcs", 
					&Lannotate_gen_sync_arcs);
}

/*--------------------------------------------------------------------------*/
/*
 *      Generate Converted Lcode
 */
void L_gen_code(Parm_Macro_List *command_line_macro_list)
{
    /* Load the parmaters specific to Lannotate */
    L_load_parameters (L_parm_file, command_line_macro_list ,
                       "(Lannotate", L_read_parm_Lannotate);

    /* Call routine appropriate for mode */

    if (L_pmatch (A_mode, "extract"))
        Lannotate_extract();

    else if (L_pmatch (A_mode, "insert"))
        Lannotate_insert ();

    else if (L_pmatch (A_mode, "index"))
        Lannotate_index ();

    else if (L_pmatch (A_mode, "combine"))
        Lannotate_combine ();

    else if (L_pmatch (A_mode, "strip"))
        Lannotate_strip ();

    else
        L_punt ("Annotating mode '%s' not supported", A_mode);
}

void Lannotate_extract()
{
    FILE *out;

    /* open the Attribute Manager file we will write to */
    if (!(out = fopen(A_src1_file,"w")))
        L_punt("Cannot open file '%s' for writing",A_src1_file);

    /* open the input Lcode file to extract from */
    L_open_input_file(L_input_file);

    /* process all data and functions within a file */
    while (L_get_input() != L_INPUT_EOF)
    {
        if (L_token_type==L_INPUT_FUNCTION)
        {
            L_define_fn_name (L_fn->name);

	    /* perform extraction */
            Lannotate_extract_func(out, L_fn);

            L_delete_func(L_fn);
        }
        else
        {
            L_delete_data(L_data);
        }
    }

    L_close_input_file(L_input_file);
}


/* Function added for generating sync arcs from memory dependence attributes */
/* DAC 9/9/97 */
void Lannotate_generate_sync_arcs(L_Func *fn)
{
  L_Cb *cb;
  L_Attr *attr;
  L_Oper *op;
  L_Oper *dep_op;
  L_Oper *op_index;
  int dep_id;
  int i;
  char buf[10];
  int load_flag;
  int store_flag;
  L_Oper_List *store_list;
  L_Oper_List *list;
  int dep_flags = SET_NONLOOP_CARRIED(0);
  
  /* Add the sync arc attribute to the function */
  attr = L_new_attr("DEP_PRAGMAS", 0);
  fn->attr = L_concat_attr(fn->attr, attr);

  /* create a store list */
  store_list = L_new_oper_list();

  for (cb = fn->first_cb; cb ; cb = cb->next_cb) {   

      /* Delete all previous sync information */
      if (Lannotate_remove_existing_sync_arcs) {
         for (op = cb->first_op; op; op = op->next_op) {
	  L_delete_all_sync (op, 1);
         }
      }

      for (op = cb->first_op; op; op = op->next_op) {

	  load_flag = L_general_load_opcode(op);
          store_flag = L_general_store_opcode(op);

	  if (load_flag || store_flag) {

	      /* Create a list for stores */
	      if (store_flag) {
		list = L_new_oper_list();
		list->oper = op;
		store_list = L_concat_oper_list(store_list,list);
	      }

	      i = 1;	
	      sprintf(buf,"dep_sl_%d",i);
	      while ((attr = L_find_attr (op->attr, buf)) != 0) 
		{
		  dep_id = attr->field[0]->value.i;
		  
		  /* determine dependent op */
		  dep_op = L_oper_hash_tbl_find_oper(fn->oper_hash_tbl,dep_id);
		  
		  /* Make sync arc dependence */
		  L_add_sync_between_opers(op,dep_op);
		  
		  /* prepare for next */
		  i++;
		  sprintf(buf,"dep_sl_%d",i);
		}

	      i = 1;	
	      sprintf(buf,"dep_ls_%d",i);
	      while ((attr = L_find_attr (op->attr, buf)) != 0) 
		{
		  dep_id = attr->field[0]->value.i;
		  
		  /* determine dependent op */
		  dep_op = L_oper_hash_tbl_find_oper(fn->oper_hash_tbl,dep_id);
		  
		  /* Make sync arc dependence */
		  L_add_sync_between_opers(op,dep_op);
		  
		  /* prepare for next */
		  i++;
		  sprintf(buf,"dep_ls_%d",i);

		  op->attr = L_delete_attr(op->attr, attr);
		}


	      i = 1;	
	      sprintf(buf,"dep_ss_%d",i);
	      while ((attr = L_find_attr (op->attr, buf)) != 0) 
		{
		  dep_id = attr->field[0]->value.i;
		  
		  /* determine dependent op */
		  dep_op = L_oper_hash_tbl_find_oper(fn->oper_hash_tbl,dep_id);
		  
		  /* Make sync arc dependence */
		  L_add_sync_between_opers(op,dep_op);
		  
		  /* prepare for next */
		  i++;
		  sprintf(buf,"dep_ss_%d",i);
		  op->attr = L_delete_attr(op->attr, attr);
		} 

	      i = 1;	
	      sprintf(buf,"dep_ll_%d",i);
	      while ((attr = L_find_attr (op->attr, buf)) != 0) 
		{
		  dep_id = attr->field[0]->value.i;

		  if (op->id == dep_id) {
		    /* Make sync arc dependence */
		    L_add_sync_between_opers(op,op);
		  }
		  else {
		    /* Overflow of memory dependence profile information */
		    /* Serialize the control block's stores */
#if 0
		    for (list=store_list->next_list; list ; list = list->next_list) {

			op_index = list->oper;
			if (L_independent_memory_ops(cb, op_index, op, dep_flags)) 
			  continue;

		        /* Make sync arc dependence */
		        L_add_sync_between_opers(op,dep_op);
		    }
#endif
		  }

		  i++;
		  /* prepare for next */
		  sprintf(buf,"dep_ss_%d",i);
		} 
	    }
	}

	L_delete_all_oper_list(store_list->next_list);
	store_list->next_list = NULL;
    }

  /* delete the store list */
  L_delete_all_oper_list(store_list);
}

void Lannotate_insert()
{
    FILE *annot,*index;

    /* open the Attribute Manager File to insert from */
    if (!(annot = fopen(A_src1_file,"r")))
        L_punt("Cannot open file '%s' for reading",A_src1_file);

    /* open its corresponding index file */
    if (!(index = fopen(A_inx_file,"r")))
	L_punt("Cannot open index file '%s' - create one with Lannotate",
						A_inx_file);

    /* init the hash table to hold the index file info and read into it */
    L_init_string_hash();
    Lannotate_read_index(index);

    /* open the input Lcode file to insert into */
    L_open_input_file(L_input_file);

    /* process all data and functions within a file */
    while (L_get_input() != L_INPUT_EOF)
    {
        if (L_token_type==L_INPUT_FUNCTION)
        {
            L_define_fn_name (L_fn->name);

	    /* perform insertion */
            Lannotate_insert_func(annot,index, L_fn);

	    /* Check for generation of sync arcs */
	    if (Lannotate_gen_sync_arcs)
                Lannotate_generate_sync_arcs(L_fn);

	    /* print out new function */
            L_print_func(L_OUT, L_fn);
    	    L_delete_all_attr(fn_attr);
	    fn_attr = NULL;
            L_delete_func(L_fn);
        }
        else
        {
            L_print_data(L_OUT, L_data);
            L_delete_data(L_data);
        }
    }

    L_close_input_file(L_input_file);
    L_delete_string_hash();
}

void Lannotate_index()
{
	FILE *annot,*index;

	/* open Attribute Manager File from which we will generate an index */
	if (!(annot = fopen(A_src1_file,"r")))
	   L_punt("Cannot open file '%s' for reading",A_src1_file);

	/* open the index file to write into */
	if (!(index = fopen(A_inx_file,"w")))
	   L_punt("Cannot open file '%s' for writing",A_inx_file);

	/* create index file */
        Lannotate_index_func(annot,index);
}

void Lannotate_combine()
{
	FILE *annot1,*annot2,*temp,*dest;
	char c;

	/* open Attribute Manager File 1 */
	if (!(annot1 = fopen(A_src1_file,"r")))
	   L_punt("Cannot open file '%s' for reading",A_src1_file);

	/* open Attribute Manager File 2 */
	if (!(annot2 = fopen(A_src2_file,"r")))
	   L_punt("Cannot open file '%s' for reading",A_src2_file);

	/* create a temp file to initally combine into */
	if (!(temp = tmpfile()))
	   L_punt("Cannot create temp file");

	/* perform combination */
	Lannotate_combine_func(annot1,annot2,temp);

	/* open destination Attribute Manager file (if not stdout) */
	if (!strcmp(A_dest_file,"stdout")) dest = stdout;
	else if (!(dest = fopen(A_dest_file,"w")))
	   L_punt("Cannot open file '%s' for writing",A_dest_file);

	/* copy temp file to destination */
	rewind(temp);
	while ((c = getc(temp)) != -1) putc(c,dest);
}

void Lannotate_strip()
{
    /* open the input Lcode file to strip */
    L_open_input_file(L_input_file);

    /* process all data and functions within a file */
    while (L_get_input() != L_INPUT_EOF)
    {
        if (L_token_type==L_INPUT_FUNCTION)
        {
            L_define_fn_name (L_fn->name);

	    /* perform stripping */
            Lannotate_strip_func(L_fn);

	    /* print out new function */
            L_print_func(L_OUT, L_fn);
            L_delete_func(L_fn);
        }
        else
        {
            L_print_data(L_OUT, L_data);
            L_delete_data(L_data);
        }
    }

    L_close_input_file(L_input_file);
}

void Lannotate_extract_func(FILE *out,L_Func *fn)
{
        L_Cb *cb;
        L_Oper *op;
	L_AttrMngr *L_AM;
	L_Attr *attr;
	char *attr_list,this_attr[1024];
	int n,i,wildcard;

	/* create an Attribute Manager to hold attributes and 
		later print them */
	L_AM = L_create_AttrMngr(fn->name,1.0);

	/* extract each attribute in the function attribute list */
	attr_list = strdup(A_fn_attr_list);
	while (sscanf(attr_list," %s",this_attr) > 0)
	{
	   /* point to next attribute in list */
	   attr_list = &attr_list[1+strlen(this_attr)];
	   if (this_attr[0] == '"') strcpy(this_attr,&this_attr[1]);
	   else if (this_attr[strlen(this_attr)-1] == '"')
		this_attr[strlen(this_attr)-1] = 0;

	   if (this_attr[strlen(this_attr)-1] == '*')
	   {
		wildcard = TRUE;
		this_attr[strlen(this_attr)-1] = 0;
	   }
	   else 
		wildcard = FALSE;

	   /* check function attributes in original Lcode */
	   if (!wildcard)
	   {
	      attr = L_find_attr(fn->attr,this_attr);
	      if (attr)
	      {
		L_insert_fn_attr_list(L_AM,attr->name,
			make_attr_field_list(attr->field,attr->max_field),
			attr->max_field);
	      }
	   }
	   else
	   {
	      n = L_count_attr_prefix_local(fn->attr,this_attr);
	      for (i=0;i<n;i++)
		if ((attr = L_find_attr_prefix_local(fn->attr,this_attr,i)))
		{
		  L_insert_fn_attr_list(L_AM,attr->name,
			make_attr_field_list(attr->field,attr->max_field),
			attr->max_field);
		}
		else
		   L_punt("Attribute %s* number %d not found",this_attr,i);
	   }
	 }

	/* check each control block's attributes */
        for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
        {

	   /* extract each attribute in the cb attribute list */
	   attr_list = strdup(A_cb_attr_list);
	   while (sscanf(attr_list," %s",this_attr) > 0)
	   {
	      /* point to next attribute in list */
	      attr_list = &attr_list[1+strlen(this_attr)];
	      if (this_attr[0] == '"') strcpy(this_attr,&this_attr[1]);
	      else if (this_attr[strlen(this_attr)-1] == '"')
		this_attr[strlen(this_attr)-1] = 0;

	      if (this_attr[strlen(this_attr)-1] == '*')
	      {
		wildcard = TRUE;
		this_attr[strlen(this_attr)-1] = 0;
	      }
	      else 
		wildcard = FALSE;

	      /* check control block attributes in original Lcode */
	      if (!wildcard)
	      {
	      	attr = L_find_attr(cb->attr,this_attr);
	      	if (attr)
		{
		  L_insert_cb_attr_list(L_AM,cb->id,attr->name,
			make_attr_field_list(attr->field,attr->max_field),
			attr->max_field);
		}
	      }
	      else
	      {
	        n = L_count_attr_prefix_local(cb->attr,this_attr);
	      	for (i=0;i<n;i++)
		  if ((attr = L_find_attr_prefix_local(cb->attr,this_attr,i)))
		   {
		      L_insert_cb_attr_list(L_AM,cb->id,attr->name,
			make_attr_field_list(attr->field,attr->max_field),
			attr->max_field);
		   }
		   else
		      L_punt("Attribute %s* number %d not found",this_attr,i);
	      }
	   }

	   /* check each operations's attributes */
           for (op = cb->first_op; op != NULL; op = op->next_op)
           {

	      /* extract each attribute in the operation attribute list */
	      attr_list = strdup(A_op_attr_list);
	      while (sscanf(attr_list," %s",this_attr) > 0)
	      {
	         /* point to next attribute in list */
		 attr_list = &attr_list[1+strlen(this_attr)];
		 if (this_attr[0] == '"') strcpy(this_attr,&this_attr[1]);
		 else if (this_attr[strlen(this_attr)-1] == '"')
			this_attr[strlen(this_attr)-1] = 0;
		
	         if (this_attr[strlen(this_attr)-1] == '*')
	         {
		    wildcard = TRUE;
		    this_attr[strlen(this_attr)-1] = 0;
	         }
	         else 
		    wildcard = FALSE;

	         /* check operation attributes in original Lcode */
	         if (!wildcard)
	         {
	      	    attr = L_find_attr(op->attr,this_attr);
	      	    if (attr)
		    {
		      L_insert_op_attr_list(L_AM,op->id,attr->name,
			make_attr_field_list(attr->field,attr->max_field),
			attr->max_field,L_get_dep_num(op));
		    }
	         }
	         else
	         {
	            n = L_count_attr_prefix_local(op->attr,this_attr);
	      	    for (i=0;i<n;i++)
		       if ((attr = L_find_attr_prefix_local(op->attr,
							    this_attr,i)))
		       {
		         L_insert_op_attr_list(L_AM,op->id,attr->name,
			    make_attr_field_list(attr->field,attr->max_field),
			    attr->max_field,L_get_dep_num(op));
		       }
		       else
		          L_punt("Attribute %s* number %d not found",
								this_attr,i);
	         }
	      }
           }
        }

	/* Write the attributes to the output file */
	L_write_attr_to_file(out,L_AM);
	L_free_AttrMngr(L_AM);
}

void Lannotate_insert_func(FILE *annot,FILE *index,L_Func *fn)
{
    L_Oper *op;
    L_Cb *cb;
    L_Attr *this,*attr,*attr_list;
    int n,N,i;

    N = L_get_num_positions(fn->name);

    for (n=0;n<N;n++)
    {

	/* init the hash tables to hold the cb and op attributes */
	L_init_hash();

	/* read the Attribute Manager file into the hash tables, etc. */
	Lannotate_read_annot(annot,fn->name,n);

	/* for each function attribute that was in the Attribute Manager
		file, insert it into the Lcode function attribute list */
	for (this=fn_attr;this;this=this->next_attr)
	{
	   /* If this attribute already exists in Lcode, update its
		value, else tack the attribute on to the end of the list */
	   if (!(attr = L_find_attr(fn->attr,this->name)))
	   {
	      attr = L_new_attr (this->name, 1);
	      fn->attr = L_concat_attr(fn->attr, attr);
	   }
	   for (i=0;i<this->max_field;i++)
	   {
	     if (!this->field[i])
	     {
	      L_set_attr_field(attr,i,0);
	     }
	     else
	     {
	      switch (L_operand_case_type(this->field[i]))
	      {
		case L_OPERAND_INT:
	      	   L_set_int_attr_field (attr, i, this->field[i]->value.i);
		   break;
		case L_OPERAND_FLOAT:
	      	   L_set_float_attr_field (attr, i, this->field[i]->value.f);
		   break;
		case L_OPERAND_STRING:
	      	   L_set_string_attr_field (attr, i, this->field[i]->value.s);
		   break;
		case L_OPERAND_LABEL:
	      	   L_set_label_attr_field (attr, i, this->field[i]->value.l);
		   break;
		default:
		   L_punt("Attribute type '%c' not known\n",L_return_old_type(attr->field[i]));
	      }
	     }
	   }
	}

	/* Cycle through all control blocks */
        for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
        {
	   /* get the list of attributes specified in the Attribute
		Manager file for this control block */
	   attr_list = L_get_cb_attr(cb->id);

	   /* for each cb attribute that was in the Attribute Manager
		file, insert it into this Lcode cb attribute list */
	   for (this=attr_list;this;this=this->next_attr)
	   {
	      /* If this attribute already exists in Lcode, update its
		value, else tack the attribute on to the end of the list */
	     if (!(attr = L_find_attr(cb->attr,this->name)))
	     {
	     	attr = L_new_attr (this->name, 1);
	     	cb->attr = L_concat_attr(cb->attr, attr);
	     }
	     for (i=0;i<this->max_field;i++)
	     {
	       if (!this->field[i])
	       {
		L_set_attr_field(attr,i,0);
	       }
	       else
	       {
	      	switch (L_operand_case_type(this->field[i]))
	      	{
		 case L_OPERAND_INT:
	      	   L_set_int_attr_field (attr, i, this->field[i]->value.i);
		   break;
		 case L_OPERAND_FLOAT:
	      	   L_set_float_attr_field (attr, i, this->field[i]->value.f);
		   break;
		 case L_OPERAND_STRING:
	      	   L_set_string_attr_field (attr, i, this->field[i]->value.s);
		   break;
		 case L_OPERAND_LABEL:
	      	   L_set_label_attr_field (attr, i, this->field[i]->value.l);
		   break;
		 default:
		   L_punt("Attribute type '%c' not known\n",L_return_old_type(attr->field[i]));
	      	}
	       }
	     }
	   }

	   /* Cycle through all operations */
           for (op = cb->first_op; op != NULL; op = op->next_op)
           {
	      /* get the list of attributes specified in the Attribute
		Manager file for this operation */
	      attr_list = L_get_op_attr(op->id);

	      /* for each op attribute that was in the Attribute Manager
		file, insert it into this Lcode op attribute list */
	      for (this=attr_list;this;this=this->next_attr)
	      {
	        /* If this attribute already exists in Lcode, update its
		   value, else tack the attribute on to the end of the list */
	     	if (!(attr = L_find_attr(op->attr,this->name)))
	     	{
	     	    attr = L_new_attr (this->name, 1);
	     	    op->attr = L_concat_attr(op->attr, attr);
	     	}
	     	for (i=0;i<this->max_field;i++)
	     	{
		  if (!this->field[i])
		  {
		    L_set_attr_field(attr,i,0);
		  }
		  else
		  {
	      	    switch (L_operand_case_type(this->field[i]))
	      	    {
		      case L_OPERAND_INT:
	      	   	L_set_int_attr_field (attr, i, this->field[i]->value.i);
		   	break;
		      case L_OPERAND_FLOAT:
	      	   	L_set_float_attr_field (attr, i, this->field[i]->value.f);
		   	break;
		      case L_OPERAND_STRING:
	      	   	L_set_string_attr_field (attr, i, this->field[i]->value.s);
		   	break;
		      case L_OPERAND_LABEL:
	      	   	L_set_label_attr_field (attr, i, this->field[i]->value.l);
		   	break;
		      default:
		   	L_punt("Attribute type '%c' not known\n",L_return_old_type(attr->field[i]));
	      	    }
		  }
	     	}
	      }
	   }
	}
	
    /* delete all the hash tables and attribute structures that were used */
    L_delete_hash();

    }
}

void Lannotate_index_func(FILE *annot,FILE *index)
{
	int i,err,num_fn_attr,num_cb_attr,num_op_attr;
	float weight;
	long position;
	char name[1024]; /* Function names can get very large */
	L_string_hash_node *node;
	L_position_node *pos_node;

	L_init_string_hash();

	/* Make the index file for this Attribute Manager file */
	fprintf(index,"# Attribute Manager index file --- Version %d\n",
	   INX_VERSION);

	/* read each function in the Attribute Manager file */
	while ((err = fscanf(annot,"# Attribute Manager file --- Version %d\n",
		&version)) != -1)
	{
	   if (!err) L_punt("Incorrect format of Attribute Manager file");
	   if (version > AM_VERSION)
		L_punt("Attribute Manager version %d not known\n",version);

	   /* get the position of the start of the next function */
	   position = ftell(annot);

	   /* now get the name of this function */
	   err = fscanf(annot,"begin %s %d %d %d %f\n",name,&num_fn_attr,
		&num_cb_attr,&num_op_attr,&weight);
	   if (err < 5) L_punt("Incorrect format of Attribute Manager file");

	   /* insert this function's position into the index file hash table */
	   L_insert_position(name,position);

	   /* skip the rest of this function */
	   Lannotate_skip_func(annot,num_fn_attr,num_cb_attr,num_op_attr);
	}

	/* print out all the function names followed by a list of
		positions */
	for (i=0;i<AN_HASH_SIZE;i++)
	{
	   for (node = L_get_string_hash(i);node;node=node->next)
	   {
	      fprintf(index,"%s %d",node->name,node->n);
	      for (pos_node=node->position;pos_node;pos_node=pos_node->next)
	   	fprintf(index," %#lx",pos_node->position);
	      fprintf(index,"\n");
	   }
	}

	L_delete_string_hash();
}

void Lannotate_combine_func(FILE *annot1,FILE *annot2,FILE *out)
{
	int i,id1,id2,version2,err,err2,num_fn_attr1,num_cb_attr1,num_op_attr1;
	int num_fn_attr2,num_cb_attr2,num_op_attr2,average,dep1,dep2;
	float weight1,weight2,ratio1,ratio2;
	char name1[1024],name2[1024],fn_name1[1024],fn_name2[1024],next_char1,next_char2;
	char type1,type2,value1_s[1024],value2_s[1024];
	long value,value1_i,value2_i;
	float value1_f,value2_f,val;
	L_Attr *attr;
	L_AttrMngr *L_AM;
	L_hash_node *node;

	/* read corresponding lines out of each file at the same time - 
		for combine the functions, cb's and op's  should be in
		the same order */

    	if (L_pmatch (A_type, "average")) average = TRUE;
    	else if (L_pmatch (A_type, "add")) average = FALSE;
	else L_punt("Invalid combine type %s, should be add or average",
							A_type);

	/* process each function out of the files */
	while ((err = fscanf(annot1,
		"# Attribute Manager file --- Version %d\n",&version)) != -1 && 
		(err2 = fscanf(annot2,
		"# Attribute Manager file --- Version %d\n",&version2)) != -1)
	{
	   if (!err) L_punt("Incorrect format of Attribute Manager file 1");
	   if (!err2) L_punt("Incorrect format of Attribute Manager file 2");
	   if (version != version2)
		L_punt("Attribute Manager versions must be the same");
	   if (version > AM_VERSION)
		L_punt("Attribute Manager version %d not known\n",version);

	   /* read the function names, etc... */
	   err = fscanf(annot1,"begin %s %d %d %d %f\n",fn_name1,&num_fn_attr1,
		&num_cb_attr1,&num_op_attr1,&weight1);
	   err2 = fscanf(annot2,"begin %s %d %d %d %f\n",fn_name2,&num_fn_attr2,
		&num_cb_attr2,&num_op_attr2,&weight2);
	   if (err < 5) L_punt("Incorrect format of Attribute Manager file 1");
	   if (err2 < 5) L_punt("Incorrect format of Attribute Manager file 2");

	   /* make sure we have the same functions in both files */
	   if (strcmp(fn_name1,fn_name2))
		L_punt("Two different functions '%s' and '%s' - functions shouldbe the same \n    (in the same order) for combine",fn_name1,fn_name2);

	   /* now make sure we have the same number of attributes specified */
	   if (num_fn_attr1 != num_fn_attr2 || num_cb_attr1 != num_cb_attr2 ||
		num_op_attr1 != num_op_attr2)
		L_punt("Function '%s' does not have the same number of all attributes \n    in both files, should be the same (and in same order!) for combine",
		fn_name1);

	   /* get the ratios to multiply attribute values by */
	   ratio1 = weight1/(weight1+weight2);
	   ratio2 = weight2/(weight1+weight2);

	   /* create an Attribute Manager to hold our new combined file */
	   L_AM = L_create_AttrMngr(fn_name1,weight1+weight2);

	   /* read each function attribute from both files and combine them */
	   for (i=0;i<num_fn_attr1;i++)
	   {
	      if (version == 1)
	      {
	      	err = fscanf(annot1,"%s %d\n",name1,&value1_i);
	      	err2 = fscanf(annot2,"%s %d\n",name2,&value2_i);
	      	if (err < 2) 
		   L_punt("Incorrect format of function attributes in Attribute Manager file 1");
	        if (err2 < 2)
		   L_punt("Incorrect format of function attributes in Attribute Manager file 2");
	      }
	      else	/* version == 2 */
	      {
	      	err = fscanf(annot1,"%s %c",name1,&type1);
	      	err2 = fscanf(annot2,"%s %c",name2,&type2);
	      	if (err < 2) 
		   L_punt("Incorrect format of function attributes in Attribute Manager file 1");
	        if (err2 < 2)
		   L_punt("Incorrect format of function attributes in Attribute Manager file 2");

		if (type1 != type2)
		   L_punt("Attribute types must be the same");
	        switch (type1)
	        {
		  case 'i':
		   err = fscanf(annot1," %d\n",&value1_i);
		   err2 = fscanf(annot2," %d\n",&value2_i);
		   break;
		  case 'f':
		   err = fscanf(annot1," %f\n",&value1_f);
		   err2 = fscanf(annot2," %f\n",&value2_f);
		   break;
		  case 's':
		   err = fscanf(annot1," %s\n",&value1_s);
		   err2 = fscanf(annot2," %s\n",&value2_s);
		   break;
		  case 'l':
		   err = fscanf(annot1," %s\n",&value1_s);
		   err2 = fscanf(annot2," %s\n",&value2_s);
		   break;
		  default:
		   L_punt("Attribute type '%c' not known\n",type1);
	        }
	      	if (err < 1) 
		   L_punt("Incorrect format of function attributes in Attribute Manager file 1");
	        if (err2 < 1)
		   L_punt("Incorrect format of function attributes in Attribute Manager file 2");
	      }

	      /* make sure the attributes names match */
	      if (strcmp(name1,name2))
		L_punt("Function attributes '%s' and '%s' different - should be in the same \n    order for combine",name1,name2);

	      /* combine values and make new attribute */
	      switch(type1)
	      {
		case 'i':
    	      	   if (average)
	      	   {
	      		val = ratio1*((float)value1_i) + ratio2*((float)value2_i);
	      		value = Lannotate_round(&val);
	      	   }
    	      	   else
			value = value1_i + value2_i;
	      	   L_insert_fn_attr_int(L_AM,name1,value);
		   break;
		case 'f':
    	      	   if (average)
	      		val = ratio1*((float)value1_f) + ratio2*((float)value2_f);
    	      	   else
			val = value1_f + value2_f;
	      	   L_insert_fn_attr_float(L_AM,name1,val);
		   break;
		case 's':
		   /* for now just use value1_s */
	      	   L_insert_fn_attr_string(L_AM,name1,strdup(value1_s));
		   break;
		case 'l':
		   /* for now just use value1_s */
	      	   L_insert_fn_attr_label(L_AM,name1,strdup(value1_s));
		   break;
	      }
	   }

	   /* read each cb attribute from both files and combine them */
	   for (i=0;i<num_cb_attr1;i++)
	   {
	      if (version == 1)
	      {
	        err = fscanf(annot1,"%d %s %d",&id1,name1,&value1_i);
	        err2 = fscanf(annot2,"%d %s %d",&id2,name2,&value2_i);
	        if (err <= 2) 
		   L_punt("Incorrect format of cb attributes in Attribute Manager file 1");
	        if (err2 <= 2)
		   L_punt("Incorrect format of cb attributes in Attribute Manager file 2");
	      }
	      else	/* version == 2 */
	      {
	      	err = fscanf(annot1,"%d %s %c",&id1,name1,&type1);
	      	err2 = fscanf(annot2,"%d %s %c",&id2,name2,&type2);
	        if (err <= 2) 
		   L_punt("Incorrect format of cb attributes in Attribute Manager file 1");
	        if (err2 <= 2)
		   L_punt("Incorrect format of cb attributes in Attribute Manager file 2");

		if (type1 != type2)
		   L_punt("Attribute types must be the same");
	        switch (type1)
	        {
		  case 'i':
		   err = fscanf(annot1," %d\n",&value1_i);
		   err2 = fscanf(annot2," %d\n",&value2_i);
		   break;
		  case 'f':
		   err = fscanf(annot1," %f\n",&value1_f);
		   err2 = fscanf(annot2," %f\n",&value2_f);
		   break;
		  case 's':
		   err = fscanf(annot1," %s\n",&value1_s);
		   err2 = fscanf(annot2," %s\n",&value2_s);
		   break;
		  case 'l':
		   err = fscanf(annot1," %s\n",&value1_s);
		   err2 = fscanf(annot2," %s\n",&value2_s);
		   break;
		  default:
		   L_punt("Attribute type '%c' not known\n",type1);
	        }
	        if (err < 1) 
		   L_punt("Incorrect format of cb attributes in Attribute Manager file 1");
	        if (err2 < 1)
		   L_punt("Incorrect format of cb attributes in Attribute Manager file 2");
	      }

	      /* make sure the attributes names match */
	      if (strcmp(name1,name2))
		L_punt("Cb attributes '%s' and '%s' different - should be in the same \n    order for combine",name1,name2);

	      /* combine values and make new attribute */
	      switch(type1)
	      {
		case 'i':
    	      	   if (average)
	      	   {
	      		val = ratio1*((float)value1_i) + ratio2*((float)value2_i);
	      		value = Lannotate_round(&val);
	      	   }
    	      	   else
			value = value1_i + value2_i;
	      	   L_insert_cb_attr_int(L_AM,id1,name1,value);
		   break;
		case 'f':
    	      	   if (average)
	      		val = ratio1*((float)value1_f) + ratio2*((float)value2_f);
    	      	   else
			val = value1_f + value2_f;
	      	   L_insert_cb_attr_float(L_AM,id1,name1,val);
		   break;
		case 's':
		   /* for now just use value1_s */
	      	   L_insert_cb_attr_string(L_AM,id1,name1,strdup(value1_s));
		   break;
		case 'l':
		   /* for now just use value1_s */
	      	   L_insert_cb_attr_label(L_AM,id1,name1,strdup(value1_s));
		   break;
	      }
	   }

	   /* read each op attribute from both files and combine them */
	   for (i=0;i<num_op_attr1;i++)
	   {
	      if (version == 1)
	      {
	        err = fscanf(annot1,"%d %s %d",&id1,name1,&value1_i);
	        err2 = fscanf(annot2,"%d %s %d",&id2,name2,&value2_i);
	        if (err <= 2) 
		   L_punt("Incorrect format of op attributes in Attribute Manager file 1");
	        if (err2 <= 2)
		   L_punt("Incorrect format of op attributes in Attribute Manager file 2");
	      }
	      else	/* version == 2 */
	      {
	      	err = fscanf(annot1,"%d %s %c",&id1,name1,&type1);
	      	err2 = fscanf(annot2,"%d %s %c",&id2,name2,&type2);
	        if (err <= 2) 
		   L_punt("Incorrect format of op attributes in Attribute Manager file 1");
	        if (err2 <= 2)
		   L_punt("Incorrect format of op attributes in Attribute Manager file 2");

		if (type1 != type2)
		   L_punt("Attribute types must be the same");
	        switch (type1)
	        {
		  case 'i':
		   err = fscanf(annot1," %d",&value1_i);
		   err2 = fscanf(annot2," %d",&value2_i);
		   break;
		  case 'f':
		   err = fscanf(annot1," %f",&value1_f);
		   err2 = fscanf(annot2," %f",&value2_f);
		   break;
		  case 's':
		   err = fscanf(annot1," %s",&value1_s);
		   err2 = fscanf(annot2," %s",&value2_s);
		   break;
		  case 'l':
		   err = fscanf(annot1," %s",&value1_s);
		   err2 = fscanf(annot2," %s",&value2_s);
		   break;
		  default:
		   L_punt("Attribute type '%c' not known\n",type1);
	        }
	        if (err < 1) 
		   L_punt("Incorrect format of op attributes in Attribute Manager file 1");
	        if (err2 < 1)
		   L_punt("Incorrect format of op attributes in Attribute Manager file 2");
	      }

	      /* See if there is a dep id */
	      next_char1 = (char) fgetc(annot1);
	      next_char2 = (char) fgetc(annot2);
	      dep1 = 0;
	      if (next_char1 == '\n' || next_char2 == '\n')
	      {
		if (!(next_char1 == '\n' && next_char2 == '\n'))
		  L_punt("Attributes do not have the same format");
	      }
	      else
	      {
		err = fscanf(annot1,"%d\n",&dep1);
		err2 = fscanf(annot2,"%d\n",&dep2);
		if (err < 1) 
		  L_punt("Incorrect format of op attributes in Attribute Manager file 1");
		if (err2 < 1)
		  L_punt("Incorrect format of op attributes in Attribute Manager file 2");
		if (dep1 != dep2)
		  L_punt("DEP numbers '%d' and '%d' should be the same",
			dep1,dep2);
	      }

	      /* make sure the attributes names match */
	      if (strcmp(name1,name2))
		L_punt("Op attributes '%s' and '%s' different - should be in the same \n    order for combine",name1,name2);

	      /* combine values and make new attribute */
	      switch(type1)
	      {
		case 'i':
    	      	   if (average)
	      	   {
	      		val = ratio1*((float)value1_i) + ratio2*((float)value2_i);
	      		value = Lannotate_round(&val);
	      	   }
    	      	   else
			value = value1_i + value2_i;
	      	   L_insert_op_attr_int(L_AM,id1,name1,value,dep1);
		   break;
		case 'f':
    	      	   if (average)
	      		val = ratio1*((float)value1_f) + ratio2*((float)value2_f);
    	      	   else
			val = value1_f + value2_f;
	      	   L_insert_op_attr_float(L_AM,id1,name1,val,dep1);
		   break;
		case 's':
		   /* for now just use value1_s */
	      	   L_insert_op_attr_string(L_AM,id1,name1,strdup(value1_s),dep1);
		   break;
		case 'l':
		   /* for now just use value1_s */
	      	   L_insert_op_attr_label(L_AM,id1,name1,strdup(value1_s),dep1);
		   break;
	      }
	   }

	   /* make sure the function is ended properly in the Attribute
		Manager file */
	   err = fscanf(annot1,"end %s\n\n",fn_name1);
	   err2 = fscanf(annot2,"end %s\n\n",fn_name2);
	   if (err < 1)
		L_punt("Incorrect format of source file 1 - expected 'end %s' \n    followed by a blank line",fn_name1);
	   if (err2 < 1)
		L_punt("Incorrect format of source file 2 - expected 'end %s' \n    followed by a blank line",fn_name2);

	   /* print out new combined Attribute Manager file */
	   L_write_attr_to_file(out,L_AM);
	   L_free_AttrMngr(L_AM);
	}
}

void Lannotate_strip_func(L_Func *fn)
{
        L_Cb *cb;
        L_Oper *op;
	L_Attr *attr;
	char *attr_list,this_attr[1024];
	int n,i,wildcard;

	/* extract each attribute in the function attribute list */
	attr_list = strdup(A_fn_attr_list);
	while (sscanf(attr_list," %s",this_attr) > 0)
	{
	   /* point to next attribute in list */
	   attr_list = &attr_list[1+strlen(this_attr)];
	   if (this_attr[0] == '"') strcpy(this_attr,&this_attr[1]);
	   else if (this_attr[strlen(this_attr)-1] == '"')
		this_attr[strlen(this_attr)-1] = 0;

	   if (this_attr[strlen(this_attr)-1] == '*')
	   {
		wildcard = TRUE;
		this_attr[strlen(this_attr)-1] = 0;
	   }
	   else 
		wildcard = FALSE;

	   /* check function attributes in original Lcode */
	   if (!wildcard)
	   {
	      attr = L_find_attr(fn->attr,this_attr);
	      if (attr)
		fn->attr = L_delete_attr(fn->attr,attr);
	   }
	   else
	   {
	      n = L_count_attr_prefix_local(fn->attr,this_attr);
	      for (i=0;i<n;i++)
		if ((attr = L_find_attr_prefix_local(fn->attr,this_attr,i)))
		   fn->attr = L_delete_attr(fn->attr,attr);
		else
		   L_punt("Attribute %s* number %d not found",this_attr,i);
	   }
	 }

	/* check each control block's attributes */
        for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
        {

	   /* extract each attribute in the cb attribute list */
	   attr_list = strdup(A_cb_attr_list);
	   while (sscanf(attr_list," %s",this_attr) > 0)
	   {
	      /* point to next attribute in list */
	      attr_list = &attr_list[1+strlen(this_attr)];
	      if (this_attr[0] == '"') strcpy(this_attr,&this_attr[1]);
	      else if (this_attr[strlen(this_attr)-1] == '"')
		this_attr[strlen(this_attr)-1] = 0;

	      if (this_attr[strlen(this_attr)-1] == '*')
	      {
		wildcard = TRUE;
		this_attr[strlen(this_attr)-1] = 0;
	      }
	      else 
		wildcard = FALSE;

	      /* check control block attributes in original Lcode */
	      if (!wildcard)
	      {
	      	attr = L_find_attr(cb->attr,this_attr);
	      	if (attr)
		   cb->attr = L_delete_attr(cb->attr,attr);
	      }
	      else
	      {
		n = L_count_attr_prefix_local(cb->attr,this_attr);
	      	for (i=0;i<n;i++)
		  if ((attr = L_find_attr_prefix_local(cb->attr,this_attr,i)))
		      cb->attr = L_delete_attr(cb->attr,attr);
		   else
		      L_punt("Attribute %s* number %d not found",this_attr,i);
	      }
	   }

	   /* check each operations's attributes */
           for (op = cb->first_op; op != NULL; op = op->next_op)
           {

	      /* extract each attribute in the operation attribute list */
	      attr_list = strdup(A_op_attr_list);
	      while (sscanf(attr_list," %s",this_attr) > 0)
	      {
	         /* point to next attribute in list */
		 attr_list = &attr_list[1+strlen(this_attr)];
		 if (this_attr[0] == '"') strcpy(this_attr,&this_attr[1]);
		 else if (this_attr[strlen(this_attr)-1] == '"')
			this_attr[strlen(this_attr)-1] = 0;
		
	         if (this_attr[strlen(this_attr)-1] == '*')
	         {
		    wildcard = TRUE;
		    this_attr[strlen(this_attr)-1] = 0;
	         }
	         else 
		    wildcard = FALSE;

	         /* check operation attributes in original Lcode */
	         if (!wildcard)
	         {
	      	    attr = L_find_attr(op->attr,this_attr);
	      	    if (attr)
		       op->attr = L_delete_attr(op->attr,attr);
	         }
	         else
	         {
		    n = L_count_attr_prefix_local(op->attr,this_attr);
	      	    for (i=0;i<n;i++)
		       if ((attr = L_find_attr_prefix_local(op->attr,
							    this_attr,i)))
		          op->attr = L_delete_attr(op->attr,attr);
		       else
		          L_punt("Attribute %s* number %d not found",
								this_attr,i);
	         }
	      }
           }
        }
}

void Lannotate_read_annot(FILE *annot,char *fn_name,int i)
{
	int err,num_fn_attr,num_cb_attr,num_op_attr,weight;
	char name[1024];

	rewind(annot);
	err = fscanf(annot,"# Attribute Manager file --- Version %d\n",
		&version);
	if (!err) L_punt("Incorrect format of source file");
	if (version > AM_VERSION)
	   L_punt("Attribute Manager version %d not known\n",version);
/* This stuff was needed before index files were implemented:
	rewind(annot);
	while ((err = fscanf(annot,"# Attribute Manager file --- Version %d\n",
		&version)) != -1)
	{
	   if (!err) L_punt("Incorrect format of source file");
	   fscanf(annot,"begin %s %d %d %d %f\n",name,&num_fn_attr,
		&num_cb_attr,&num_op_attr,&weight);
	   if (strcmp(name,fn_name)) 
	   {
		Lannotate_skip_func(
			annot,num_fn_attr,num_cb_attr,num_op_attr);
	   }
	   else
	   {
		Lannotate_read_func(annot,num_fn_attr,num_cb_attr,num_op_attr);
		return;
	   }
	}
	L_punt("Function '%s' not found in source file",fn_name);
*/
/*	err = fscanf(annot,"# Attribute Manager file --- Version %d\n",
		&version);
	if (err <= 0)
		L_punt("Incorrect format of Attribute Manager file");
*/
	fseek(annot,L_get_position(fn_name,i),0);
	fscanf(annot,"begin %s %d %d %d %f\n",name,&num_fn_attr,
		&num_cb_attr,&num_op_attr,&weight);
	if (strcmp(fn_name,name)) 
	   L_punt("Function '%s' number %d not found in Attribute Manager file",
			fn_name,i);
	Lannotate_read_func(annot,num_fn_attr,num_cb_attr,num_op_attr);
}

void Lannotate_skip_func(FILE *annot,int num_fn_attr,int num_cb_attr,
	int num_op_attr)
{
	int i;
	char line[1024];

	for (i=0;i<num_fn_attr+num_cb_attr+num_op_attr+2;i++)
		fgets(line,1024,annot);
}

void Lannotate_read_func(FILE *annot,int num_fn_attr,int num_cb_attr,
	int num_op_attr)
{
	int i,err,id,field_num,num_fields;
	char name[1024],fn_name[1024],line[1024],type;
	long value_i;
	float value_f;
	char value_s[1024];
	L_Attr *attr;

	for (i=0;i<num_fn_attr;i++)
	{
	   err = fscanf(annot,"%s",name);
	   if (err < 1) L_punt("Incorrect format of source file");
	   if (version == 1)
	   {
	     attr = L_new_attr (name, 1);
	     err = fscanf(annot," %d",&value_i);
	     if (err < 1) L_punt("Incorrect format of source file");
	     L_set_int_attr_field (attr, 0, value_i);
	   }
	   else /* version == 2 or 3 */
	   {
	     if (version == 3)
	     {
		err = fscanf(annot," %d",&num_fields);
		if (err != 1) L_punt(
		  "Incorrect format of source file: num fields not specified");
	     }
	     else num_fields = 1;
	     attr = L_new_attr (name, num_fields);
	     for (field_num=0;field_num<num_fields;field_num++)
	     {
	      err = fscanf(annot," %c",&type);
	      if (err != 1)
	      	L_punt("Incorrect format of source file");
	      switch (type)
	      {
		case 'i':
		   err = fscanf(annot," %d",&value_i);
	   	   L_set_int_attr_field (attr, field_num, value_i);
		   break;
		case 'f':
		   err = fscanf(annot," %f",&value_f);
	   	   L_set_float_attr_field (attr, field_num, value_f);
		   break;
		case 's':
		   err = fscanf(annot," %s",&value_s);
	   	   L_set_string_attr_field (attr, field_num, value_s);
		   break;
		case 'l':
		   err = fscanf(annot," %s",&value_s);
	   	   L_set_label_attr_field (attr, field_num, value_s);
		   break;
		case 'N':
		   err = fscanf(annot,"ULL");
		   L_delete_operand(attr->field[field_num]);
		   attr->field[field_num] = 0;
		   err = 1;
		   break;
		default:
		   L_punt("Attribute type '%c' not known\n",type);
		   break;
	      }
	      if (err < 1) L_punt("Incorrect format of source file");
	     }
	   }
	   fgets(line,1024,annot);
	   fn_attr = L_concat_attr(fn_attr, attr);
	}
	for (i=0;i<num_cb_attr;i++)
	{
	   err = fscanf(annot,"%d %s",&id,name);
	   if (err < 2) L_punt("Incorrect format of source file");
	   if (version == 1)
	   {
	     attr = L_new_attr (name, 1);
	     err = fscanf(annot," %d",&value_i);
	     if (err < 1) L_punt("Incorrect format of source file");
	     L_set_int_attr_field (attr, 0, value_i);
	   }
	   else 	/* version == 2 or 3 */
	   {
	     if (version == 3)
	     {
		err = fscanf(annot," %d",&num_fields);
		if (err != 1) L_punt(
		  "Incorrect format of source file: num fields not specified");
	     }
	     else num_fields = 1;
	     attr = L_new_attr (name, num_fields);
	     for (field_num=0;field_num<num_fields;field_num++)
	     {
	      err = fscanf(annot," %c",&type);
	      if (err != 1)
	      	L_punt("Incorrect format of source file");
	      switch (type)
	      {
		case 'i':
		   err = fscanf(annot," %d",&value_i);
	   	   L_set_int_attr_field (attr, field_num, value_i);
		   break;
		case 'f':
		   err = fscanf(annot," %f",&value_f);
	   	   L_set_float_attr_field (attr, field_num, value_f);
		   break;
		case 's':
		   err = fscanf(annot," %s",&value_s);
	   	   L_set_string_attr_field (attr, field_num, value_s);
		   break;
		case 'l':
		   err = fscanf(annot," %s",&value_s);
	   	   L_set_label_attr_field (attr, field_num, value_s);
		   break;
		case 'N':
		   err = fscanf(annot,"ULL");
		   L_delete_operand(attr->field[field_num]);
		   attr->field[field_num] = 0;
		   err = 1;
		   break;
		default:
		   L_punt("Attribute type '%c' not known\n",type);
		   break;
	      }
	      if (err < 1) L_punt("Incorrect format of source file");
	     }
	   }
	   fgets(line,1024,annot);
	   L_hash_insert_cb_attr(attr,id);
	}
	for (i=0;i<num_op_attr;i++)
	{
	   err = fscanf(annot,"%d %s",&id,name);
	   if (err < 2) L_punt("Incorrect format of source file");
	   /* Doesn't read and insert dep id's at this point. */
	   if (version == 1)
	   {
	     attr = L_new_attr (name, 1);
	     err = fscanf(annot," %d",&value_i);
	     if (err < 1) L_punt("Incorrect format of source file");
	     L_set_int_attr_field (attr, 0, value_i);
	   }
	   else 	/* version == 2 or 3 */
	   {
	     if (version == 3)
	     {
		err = fscanf(annot," %d",&num_fields);
		if (err != 1) L_punt(
		  "Incorrect format of source file: num fields not specified");
	     }
	     else num_fields = 1;
	     attr = L_new_attr (name, num_fields);
	     for (field_num=0;field_num<num_fields;field_num++)
	     {
	      err = fscanf(annot," %c",&type);
	      if (err != 1)
	      	L_punt("Incorrect format of source file");
	      switch (type)
	      {
		case 'i':
		   err = fscanf(annot," %d",&value_i);
	     	   L_set_int_attr_field (attr, field_num, value_i);
		   break;
		case 'f':
		   err = fscanf(annot," %f",&value_f);
	     	   L_set_float_attr_field (attr, field_num, value_f);
		   break;
		case 's':
		   err = fscanf(annot," %s",&value_s);
	     	   L_set_string_attr_field (attr, field_num, value_s);
		   break;
		case 'l':
		   err = fscanf(annot," %s",&value_s);
	     	   L_set_label_attr_field (attr, field_num, value_s);
		   break;
		case 'N':
		   err = fscanf(annot,"ULL");
		   L_delete_operand(attr->field[field_num]);
		   attr->field[field_num] = 0;
		   err = 1;
		   break;
		default:
		   L_punt("Attribute type '%c' not known (id %d)\n",
							type,id);
		   break;
	      }
	      if (err < 1) L_punt("Incorrect format of source file");
	     }
	   }
	   fgets(line,1024,annot);
	   L_hash_insert_op_attr(attr, id);
	}
	fscanf(annot,"end %s\n\n",fn_name);
}

void Lannotate_read_index(FILE *index)
{
	int err,version,n;
	long position;
	char name[1024];

	err = fscanf(index,"# Attribute Manager index file --- Version %d\n",
		&version);
	if (err < 1) L_punt("Incorrect format of index file");
	while ((err = fscanf(index,"%s %d",name,&n)) != -1)
	{
	   if (err < 2) L_punt("Incorrect format of index file");
	   for (;n;n--)
	   {
	      err = fscanf(index," 0x%lx",&position);
	      if (err < 1) L_punt("Incorrect format of index file");
	      L_insert_position(name,position);
	   }
	   fscanf(index,"\n");
	}
}

long Lannotate_round(float *val)
{
	if (*val - floor(*val) < .5 || (*val - floor(*val) == .5 &&
						!fmod(floor(*val),2)))
		return *val;
	else 
		return *val + 1;
}

int L_count_attr_prefix_local(L_Attr *attr,char *attr_name)
{
        int n = 0;
        L_Attr *this;

        for (this=attr;this;this=this->next_attr)
           if (!strncmp(this->name,attr_name,strlen(attr_name))) n++;
	return n;
}

L_Attr *L_find_attr_prefix_local(L_Attr *attr,char *attr_name,int n)
{
        int ctr = n;
        L_Attr *this = attr;

        if (ctr < 0) return NULL;
        while (this && ctr >= 0)
        {
           if (!strncmp(this->name,attr_name,strlen(attr_name))) ctr--;
           if (ctr >= 0) this = this->next_attr;
        }
        if (ctr >= 0) return NULL;
        return this;
}

int L_get_dep_num(L_Oper *op)
{
	L_Attr *attr;

	attr = L_find_attr(op->attr,"DEP");
        if (!attr) return 0;
	if (!L_is_int_constant(attr->field[0]))
	  L_punt("DEP attribute value not an integer");
	return attr->field[0]->value.i;
}

A_Attr_Field **make_attr_field_list(L_Operand **operands,int max_field)
{
    A_Attr_Field **fields;
    int i;

    fields = (A_Attr_Field **) malloc(sizeof(A_Attr_Field *)*max_field);
    for (i=0;i<max_field;i++)
    {
	fields[i] = (A_Attr_Field *) malloc(sizeof(A_Attr_Field));
	if (!operands[i]) fields[i] = 0;
	else
	{
	  switch (L_operand_case_type(operands[i]))
	  {
	    case L_OPERAND_INT:
		fields[i]->type = A_INT;
		fields[i]->value.i = operands[i]->value.i;
		break;
	    case L_OPERAND_FLOAT:
		fields[i]->type = A_FLOAT;
		fields[i]->value.f = operands[i]->value.f;
		break;
	    case L_OPERAND_STRING:
		fields[i]->type = A_STRING;
		fields[i]->value.s = strdup(operands[i]->value.s);
		break;
	    case L_OPERAND_LABEL:
		fields[i]->type = A_LABEL;
		fields[i]->value.l = strdup(operands[i]->value.l);
		break;
	  }
	}
    }
    return fields;
}
