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
 *      File:   s_mem_profile.c
 *      Author: Daniel Connors
 *      Creation Date: Oct 1996
 *      Copyright (c) 1996 Daniel Connors, Wen-mei Hwu and
 *                         The Board of Trustees of the University of Illinois.
 *                         All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#include <config.h>
#include "s_main.h"
#include "s_profile.h"
#include "s_mem_profile.h"
#include "s_hash.h"

extern FILE *mem_dep_guide_file;
extern FILE *mem_dep_profile_file;
extern L_Alloc_Pool    *S_Pdc_MemDep_Data_Info_pool;

int 
S_find_pc_for_mem_op_id(S_Fn *fn,int id)
{
  return (fn->guide_table->mem_tab[id]);
}

void
S_build_function_guide_table(S_Fn *fn,int size,int mem_max)
{
  S_Guide_Table *guide_table;
  S_Opc_Info *info;
  S_Oper *op;
  int j;
  
  if ((guide_table = (S_Guide_Table *) malloc(sizeof(S_Guide_Table))) == NULL)
    S_punt("S_build_function_guide_table: Out of memory");
 
  guide_table->num_guide_loads = size;
  if ((guide_table->mem_tab = (int *) malloc((mem_max+1)*sizeof(int))) == NULL)
    S_punt("S_build_function_guide_table: Out of memory");
  
  fn->guide_table = guide_table;
  
  for (j=0;j<fn->op_count;j++) {
    op = &fn->op[j];
    
    info = &opc_info_tab[op->opc];
    if ((info->opc_type != STORE_OPC) && (info->opc_type != LOAD_OPC) &&
	(info->opc_type != JSR_OPC)) 
      continue;
    
    if (op->lcode_id > mem_max) {
      continue;
    }
    guide_table->mem_tab[op->lcode_id] = op->pc;
  }
}


void S_check_fn_head()
{
    S_Fn *fn;

    for (fn=head_fn;fn;fn=fn->next_fn) {
	if (fn->guide_table == (S_Guide_Table *) 0x01) {
	    S_punt("S_check_fn_head\n");
	}
    }
}

void 
S_print_pdc_profile()
{
  S_Fn *fn;
  S_Oper *op;
  S_Guide_Info *guide_info;
  S_Opc_Info *info;
  int pc;
  int j;
  int i;
  int size;
  
  if ((mem_dep_profile_file= fopen (S_mem_dep_profile_file_name, "w")) == NULL)
    S_punt ("Unable to open guide file '%s'.", S_mem_dep_profile_file_name);
  
  /* Generic header for profiler */
  fprintf(mem_dep_profile_file,"(count 1)\n");
  
  /* Go through all of the functions and print data */
  for (fn=head_fn;fn;fn=fn->next_fn) {

      S_check_fn_head();
    
    /* Print function name and count of pdc loads */
    
    /* Access the guide table */
    if (fn->guide_table == NULL) {
      fprintf(mem_dep_profile_file,"(begin %s 0)\n",fn->name);
      fprintf(mem_dep_profile_file,"(end %s)\n",fn->name);
      continue;
    }
    size = fn->guide_table->num_guide_loads;
    fprintf(mem_dep_profile_file,"(begin %s %d)\n",fn->name,size);
    
    for (j=0;j<fn->op_count;j++) {
      op = &fn->op[j];
      pc = op->pc;
      guide_info = (S_Guide_Info *)prof_info[pc].ptr;
      
      if (!guide_info)
	continue;

      info = &opc_info_tab[op->opc];
      if (info->opc_type == LOAD_OPC) {
        fprintf(mem_dep_profile_file,"((ld %d %d %d)",op->lcode_id,
	        guide_info->index,guide_info->call_conflict);
      }
      else if (info->opc_type == STORE_OPC) {
         fprintf(mem_dep_profile_file,"((st %d %d %d)",op->lcode_id,
	      guide_info->index,guide_info->call_conflict);
      }
      else {
         fprintf(mem_dep_profile_file,"((jsr %d %d %d)",op->lcode_id,
	      guide_info->index,guide_info->call_conflict);
      }
      
      
      for (i=0;i<guide_info->index; i++) {
	fprintf(mem_dep_profile_file,"(%d %d)",
		guide_info->guide_id[i],guide_info->conflict[i]);
      }
      fprintf(mem_dep_profile_file,")\n");
    }
    
    fprintf(mem_dep_profile_file,"(end %s)\n",fn->name);
  }
  
  fclose(mem_dep_profile_file);
  
}


S_Pdc_MemDep_Data_Info *S_get_pdc_data_info_for_address(int address)
{
  S_hash_node *data_entry;
  S_Pdc_MemDep_Data_Info *data_info;
  
  data_entry = S_hash_find_pdc_data_info(address);
  
  if (!data_entry)
    {
      data_entry = S_hash_insert_pdc_data_info(address);
      data_info =
	(S_Pdc_MemDep_Data_Info *) L_alloc(S_Pdc_MemDep_Data_Info_pool);
      
      data_info->store_pc = 0;
      data_info->func_no = 0;
      data_entry->ptr = (S_Pdc_MemDep_Data_Info *) data_info;
    }
  else {
    data_info = (S_Pdc_MemDep_Data_Info *) data_entry->ptr;
  }
  
  return data_info;
}


S_Guide_Info *S_create_new_guide_info(int index)
{
  S_Guide_Info *guide_info;
  int i;
  /* 10/25/04 REK Commenting out unused variable to quiet compiler warning. */
#if 0
  int conflict;
#endif
  
  if ((guide_info = (S_Guide_Info *) malloc(sizeof(S_Guide_Info))) == NULL)
    S_punt("S_create_new_guide_info: Out of memory");
  
  if ((guide_info->conflict = (int *) malloc(index*sizeof(int))) == NULL)
    S_punt("S_create_new_guide_info: Out of memory");
  
  if ((guide_info->guide_pc = (int *) malloc(index*sizeof(int))) == NULL)
    S_punt("S_create_new_guide_info: Out of memory");
  
  if ((guide_info->guide_id = (int *) malloc(index*sizeof(int))) == NULL)
    S_punt("S_create_new_guide_info: Out of memory");
  
  guide_info->index = index;
  
  for (i=0;i<index;i++) {
    guide_info->guide_id[i] = 0;
    guide_info->guide_pc[i] = 0;
    guide_info->conflict[i] = 0;
  }
  
  guide_info->call_conflict = 0;
  
  return (guide_info);
}


void
S_read_guide_file()
{
  char lbuf[MAX_LINE_SIZE];
  char sbuf[MAX_LINE_SIZE];
  S_Fn *fn;
  /* 10/25/04 REK Commenting out unused variable to quiet compiler warning. */
#if 0
  S_Oper *op;
#endif
  int i,j;
  int id,pc,size;
  int line=0;
  S_Guide_Info *guide_info;
  int num_guides;
  int guide;
  int mem_max;
  
  if ((mem_dep_guide_file = fopen (S_mem_dep_guide_file_name, "r")) == NULL)
    S_punt ("Unable to open guide file '%s'.", S_mem_dep_guide_file_name);
  
  while (fgets (lbuf, MAX_LINE_SIZE, mem_dep_guide_file) != NULL)
    {
      
      /* Make sure file is in correct format */
      if (strcmp (lbuf, "Lguide version 0\n") != 0)
        {
            S_punt ("Error parsing function name in '%s'.", S_mem_dep_guide_file_name);
        }
      
      /* Read function name */
      if (fscanf (mem_dep_guide_file, "Function %s %d %d\n", sbuf, &size, &mem_max) != 3)
	S_punt ("Error parsing function name in '%s'.", S_mem_dep_guide_file_name);
      
      line++;
      
      if (size == 0)
	continue;
      
      for (fn=head_fn;fn;fn=fn->next_fn) {
	   /* Find function name */
	
	if (!strcmp(fn->name,sbuf))
	  break;
      }
      
      if (!fn)
	S_punt("Error: could not find function %f",sbuf);
      
      S_build_function_guide_table(fn,size,mem_max);
      
      /* Read the number size line */
      
      for (i=0; i < size; i++)
	{
	  if (fscanf(mem_dep_guide_file, "ld %d %d :", &id, &num_guides) != 2) {
	     if (fscanf(mem_dep_guide_file, 
			"st %d %d :", &id, &num_guides) != 2)  {
		 if (fscanf (mem_dep_guide_file, "jsr %d %d :", 
		     &id, &num_guides) != 2)  {
		    S_punt ("%s line %i: Unexpected end of file", 
			    S_mem_dep_guide_file_name, line);
		  }
	      }
	  }
	  line++;
	  
	  guide_info = S_create_new_guide_info(num_guides);
	  
	  /* Set up the guide information */
	  for (j=0;j<num_guides;j++) {
	    
	    if (fscanf (mem_dep_guide_file, "%d", &guide) == 1) {
	      guide_info->guide_id[j] = guide;
	      /* Need to find pc for store id */
		  pc = S_find_pc_for_mem_op_id(fn,guide);
		  guide_info->guide_pc[j] = pc;
	    }
	    else {
	      S_punt ("Error reading function name in '%s'.", S_mem_dep_guide_file_name);
	    }
	    
	  }
	  fscanf (mem_dep_guide_file, "\n");
	  
	  /* Assign guide info to the appropriate S_Oper profile information */
	  pc = S_find_pc_for_mem_op_id(fn,id);
	  prof_info[pc].ptr = (S_Guide_Info *) guide_info;
	  
	}
    }
}

