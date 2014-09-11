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
 *	File :		l_get_mem_dep.c
 *	Description :	Inserts memory dependence profile into Lcode
 *	Creation Date :	Oct, 1996
 *	Author : 	Daniel A. Connors
 *
 *	(C) Copyright 1996, Daniel A. Connors and Wen-mei Hwu
 *	All rights granted to University of Illinois Board of Regents.
 *==========================================================================*/
#include <config.h>
#include <stdio.h>
#include <Lcode/l_main.h>

extern int  Lprofile_util_insert_mem_dep_profile;
extern char *Lprofile_util_mem_dep_profile_file;
extern char *Lprofile_util_mem_dep_annot_mode;
extern int  Lprofile_util_remove_zero_conflict_sync_arcs;
extern int  Lprofile_util_remove_existing_sync_arcs;

int L_mem_dep_annot_model;
unsigned int ld_ld_mem_dep_stat[11];
unsigned int ld_jsr_mem_dep_stat[11];
unsigned int other_mem_dep_stat[11];
unsigned int zero_weight;

enum {
  MEM_DEP_MODEL_SYNC=0,
  MEM_DEP_MODEL_ATTR
};

void
init_memory_dependence_parameters()
{
  FILE *fptr;
  int i;
  char buffer[16];
  int count;
  double freq;

  fptr = fopen("mem_dep", "rt");
  if (fptr) {
    fscanf(fptr, "%d\n", &zero_weight);
    for (i=0; i <= 10; i++) {
	fscanf(fptr, "%s %d %lf\n", buffer, &count, &freq);
	ld_ld_mem_dep_stat[i] = count;
	fscanf(fptr, "%s %d %lf\n", buffer, &count, &freq);
	ld_jsr_mem_dep_stat[i] = count;
	fscanf(fptr, "%s %d %lf\n", buffer, &count, &freq);
	other_mem_dep_stat[i] = count;
    }
    fclose(fptr);
  }

  if ((L_pmatch (Lprofile_util_mem_dep_annot_mode, "sync-arc"))||
      (L_pmatch (Lprofile_util_mem_dep_annot_mode, "sync"))||
      (L_pmatch (Lprofile_util_mem_dep_annot_mode, "sync arc"))) {
    L_mem_dep_annot_model = MEM_DEP_MODEL_SYNC;
  }
  else if (L_pmatch (Lprofile_util_mem_dep_annot_mode, "attr")) {
    L_mem_dep_annot_model = MEM_DEP_MODEL_ATTR;
  }
  else {
    L_punt("L_get_mem_dep_profile_init: mem dep model must be (sync or attr)");
  }
}

/* Skip 'count' lines in file 'in' */
static void MEM_DEP_skip_lines (FILE *in, int count, char *file_name)
{
  char buf[10000];
  int i;
  
  for (i=0; i < count; i++)
    {
      if (fgets (buf, sizeof(buf), in) == NULL)
	L_punt ("Unexpected EOF in memory dependence profile file '%s'.",
		file_name);
    }
}

void
L_get_mem_dep_profile_init(L_Func *fn)
{
  L_Attr *attr;
  L_Cb *cb;
  L_Oper *op;

  /* Need to make sure that the function oper weights are computed */
  /* so that an effective conflict rate can be made */
  L_compute_oper_weight(fn, 0, 1);
  
  if (Lprofile_util_remove_existing_sync_arcs) {
    
    for (cb = fn->first_cb; cb ; cb = cb->next_cb) {
      for (op = cb->first_op; op; op = op->next_op) {
	L_Sync_Info *sync_info;
	
	sync_info = op->sync_info;
	
	if (sync_info == NULL)
	  continue;
	
#if 0	/* BCC - keep jsr sync arcs */
	L_delete_all_but_jsr_sync (op);
#endif
	L_delete_all_sync (op, 1);
      }
    }
  }
  
  /* Update the function attributes */
  if (L_mem_dep_annot_model == MEM_DEP_MODEL_SYNC) {
    if (!(L_find_attr(fn->attr, "DEP_PRAGMAS"))) {
      attr = L_new_attr("DEP_PRAGMAS", 0);
      fn->attr = L_concat_attr(fn->attr, attr);
    }
  }   
}

int
L_mem_dep_return_pdc_rate(L_Oper *load_op,int conflict_number)
{
  int load_weight;
  int pdc_rate = 0;
  
  /* Compute pdc */
  load_weight = (int) load_op->weight;
  
  if (load_weight != 0 ) {
    pdc_rate = (conflict_number * 100) / load_weight;
    if (conflict_number != 0 &&
	pdc_rate == 0)
	pdc_rate = 1;
  }
  
  /* BCC - temp fix - 11/99 */
  if (pdc_rate > 100)
      pdc_rate = 100;

  if ((pdc_rate < 0)||(pdc_rate > 100))
    L_punt("L_mem_dep_update_sync_arc_info: pdc is coming out wrong");
  
  return (pdc_rate);
}

void 
L_mem_dep_update_sync_arc_info(L_Oper *load_op,L_Oper *store_op,int conflict)
{
  L_Sync *tail_sync;
  L_Sync *head_sync;
  char pdc_rate = 0;
  int load_weight;
  char name[2];
  int dist,flags;

  /* set the default variables for creating new general sync arc */
  name[0] = 'P';
  name[1] = 'I';
  dist = 0;
  flags = SET_PROFILE_CONFLICT(0);

  /* BCC - make the profile-generated sync arcs friendly to opti routines */
  if (conflict)
      flags |= SET_NONLOOP_CARRIED(0);

  if (load_op == NULL)
     L_punt("L_mem_dep_update_sync_arc_info: load op doesn't exist");

  /* don't need to create any new syncs if only annotating non-zero sync arcs */
  if ((conflict==0)&&(Lprofile_util_remove_zero_conflict_sync_arcs==1)) 
   return;

  /* Compute pdc */
  load_weight = (int) load_op->weight;
  
  if (load_weight != 0) {
    if (conflict > load_weight)
      pdc_rate = 100;
    else 
      pdc_rate = (conflict * 100) / load_weight;
  }
  
  /* Sanity check for pdc rate within bounds */
  if ((pdc_rate < 0)||(pdc_rate > 100))
    L_punt("L_mem_dep_update_sync_arc_info: pdc is coming out wrong");
  
  /* Special case conflicts which can rounded to 0% conflict, change these to 1%  */
  if ((pdc_rate == 0)&&(conflict != 0)) {
    pdc_rate = 1;
  }

  /* Need to create sync arcs manually, freedom to view sync and change it  */
  tail_sync = L_create_new_sync (store_op->id, name[0], name[1], dist, flags, pdc_rate);
  L_insert_tail_sync_in_oper (load_op, tail_sync);
  head_sync = L_copy_sync (tail_sync);
  head_sync->dep_oper = load_op;
  L_insert_head_sync_in_oper (store_op, head_sync);
}

/*
 * Reads the memory dependence profile info from 'file_name' and replaces
 * sync arc frequency field with a percentage of conflict "PDC"
 * Potential Data Conflict rate or as an attribute
 */
void L_get_mem_dep_profile (L_Func *fn, char *file_name)
{
  char func_name[100];
  char mem_dep_string[10];
  int func_size;
  /* 10/25/04 REK Commenting out unused variables to quiet compiler warnings.
   */
#if 0
  L_Cb *cb;
  L_Oper *op,*load_op;
  L_Attr *attr;
  int load_id;
#endif
  L_Oper *store_op,*mem_op;
  FILE *in;
  L_Attr *mem_dep_attr;
  int i,j,num_info;
  int store_id,mem_id;
  int conflict_number, conflict_number1;
  int profile_count;
  int err;
  int call_conflict;
  FILE *fptr;
  unsigned total_sync_arcs = 0;
  
  /* Open the profile file for reading */
  if ((in = fopen (file_name, "r")) == NULL)
    L_punt ("Error opening mem dep profile file '%s'.", file_name);
  
  L_get_mem_dep_profile_init(fn);
  
  err = fscanf(in,"(count %d)\n",&profile_count);
  if (err != 1)
    L_punt ("Error opening mem dep profile file '%s'.", file_name);
  
  /* Skip functions until fn->name is found */
  while ((fscanf (in, "(begin %s %i)\n", func_name, &func_size) == 2) &&
	 (strcmp (func_name, fn->name) != 0))
    MEM_DEP_skip_lines (in, func_size + 1, file_name);
  
  /* Make fn->name was found in profile file */
  if (strcmp (func_name, fn->name) != 0)
    L_punt ("Function '%s' not found in memory dependence profile file '%s'.",
	    fn->name, file_name);
  
  /*
   * For each load in data base, create either two attributes or sync arc info
   */
  for (i=0; i < func_size; i++) {
    /* Read load data */
    fscanf (in, "((");
    if (fscanf (in, "ld %i %i %i)",&mem_id, &num_info, &call_conflict) != 3) {
      if (fscanf (in, "st %i %i %i)",&mem_id, &num_info, &call_conflict) != 3) {
        if (fscanf (in, "jsr %i %i %i)",&mem_id, &num_info, &call_conflict) != 3) {
	  L_punt ("Parse error in memory dependence profile file '%s'.", file_name);
	}
      }
    }
    
    mem_op = L_oper_hash_tbl_find_oper(fn->oper_hash_tbl,mem_id);

    if (!mem_op)
      L_punt ("L_get_mem_dep_profile : load op not defined");

    for (j=0; j < num_info; j++) {
	/* Read conflict data */
	if (fscanf (in, "(%d %d)",&store_id, &conflict_number) != 2)
	    L_punt ("Parse error in memory dependence profile file '%s'.", 
		    file_name);

	store_op = L_oper_hash_tbl_find_oper(fn->oper_hash_tbl,store_id);

	if (!store_op)
	    L_punt ("L_get_mem_dep_profile : store op not defined");

	/* Either Change Sync arc or add attribute */
	if (L_mem_dep_annot_model == MEM_DEP_MODEL_SYNC) {
	    L_mem_dep_update_sync_arc_info(mem_op,store_op,conflict_number);
	    if (mem_op->weight != 0) {
		conflict_number1 = 
		    L_mem_dep_return_pdc_rate(mem_op,conflict_number);
		if (L_general_load_opcode(mem_op)) {
		    if (L_general_store_opcode(store_op))
			ld_ld_mem_dep_stat[(conflict_number1+9)/10]++;
		    else
			ld_jsr_mem_dep_stat[(conflict_number1+9)/10]++;
		}
		else
		    other_mem_dep_stat[(conflict_number1+9)/10]++;
	    }
	    else {
		zero_weight++;
	    }
	}
	else if (L_mem_dep_annot_model == MEM_DEP_MODEL_ATTR) {
	    sprintf(mem_dep_string,"dep_sl_%d",j);
	    mem_dep_attr = L_new_attr (mem_dep_string, 2);
	    if (mem_op->weight != 0) {
		conflict_number = L_mem_dep_return_pdc_rate(mem_op,
							    conflict_number);
		if (L_general_load_opcode(mem_op)) {
		    if (L_general_store_opcode(store_op))
			ld_ld_mem_dep_stat[(conflict_number+9)/10]++;
		    else
			ld_jsr_mem_dep_stat[(conflict_number+9)/10]++;
		}
		else 
		    other_mem_dep_stat[(conflict_number+9)/10]++;
	    }
	    else {
		zero_weight++;
	    }
	    L_set_int_attr_field(mem_dep_attr, 0, store_id);
	    L_set_int_attr_field(mem_dep_attr, 1, conflict_number);
	    mem_op->attr = L_concat_attr (mem_op->attr, mem_dep_attr);
	}
    }
    fscanf(in,")\n");
    
#if 0
    if (L_general_store_opcode(mem_op)) {
      for (j=0; j < num_info; j++) {
	/* Read conflict data- ignore, all conflicts duplicated on loads */
	if (fscanf (in, "(%d %d)",&store_id, &conflict_number) != 2)
	  L_punt ("Parse error in memory dependence profile file '%s'.", file_name);
	
	
	store_op = L_oper_hash_tbl_find_oper(fn->oper_hash_tbl,store_id);
	
	if (!store_op)
	  L_punt ("L_get_mem_dep_profile : store op not defined");
	
	/* Either Change Sync arc or add attribute */
	if (L_mem_dep_annot_model == MEM_DEP_MODEL_SYNC) {
	  L_mem_dep_update_sync_arc_info(mem_op,store_op,conflict_number);
	}
      }
      
      /* Read end of data */
      fscanf(in,")\n");
    }
    else if (L_general_load_opcode(mem_op)) {
      if (call_conflict) {
	attr = L_new_attr ("call_conflict", 1);
	L_set_int_attr_field(attr, 0, call_conflict);
	load_op->attr = L_concat_attr (load_op->attr, attr);
      }
      
      for (j=0; j < num_info; j++) {
	/* Read conflict data */
	if (fscanf (in, "(%d %d)",&store_id, &conflict_number) != 2)
	  L_punt ("Parse error in memory dependence profile file '%s'.", file_name);
	
	store_op = L_oper_hash_tbl_find_oper(fn->oper_hash_tbl,store_id);
	
	if (!store_op)
	  L_punt ("L_get_mem_dep_profile : store op not defined");
	
	/* Either Change Sync arc or add attribute */
	if (L_mem_dep_annot_model == MEM_DEP_MODEL_SYNC) {
	  L_mem_dep_update_sync_arc_info(mem_op,store_op,conflict_number);
	  if (mem_op->weight != 0) {
	      conflict_number1 = 
	        L_mem_dep_return_pdc_rate(mem_op,conflict_number);
	    if (L_general_load_opcode(mem_op)) {
	      if (L_general_store_opcode(store_op))
	        ld_ld_mem_dep_stat[(conflict_number1+9)/10]++;
	      else
	        ld_jsr_mem_dep_stat[(conflict_number1+9)/10]++;
	    }
	    else 
	      other_mem_dep_stat[(conflict_number1+9)/10]++;
	  }
	  else {
	      zero_weight++;
	  }
	}
	else if (L_mem_dep_annot_model == MEM_DEP_MODEL_ATTR) {
	  sprintf(mem_dep_string,"dep_sl_%d",j);
	  mem_dep_attr = L_new_attr (mem_dep_string, 2);
	  if (mem_op->weight != 0) {
	    conflict_number = L_mem_dep_return_pdc_rate(mem_op,conflict_number);
	    if (L_general_load_opcode(mem_op)) {
	      if (L_general_store_opcode(store_op))
	        ld_ld_mem_dep_stat[(conflict_number+9)/10]++;
	      else
	        ld_jsr_mem_dep_stat[(conflict_number+9)/10]++;
	    }
	    else
	      other_mem_dep_stat[(conflict_number1+9)/10]++;
	  }
	  else {
	      zero_weight++;
	  }
	  L_set_int_attr_field(mem_dep_attr, 0, store_id);
	  L_set_int_attr_field(mem_dep_attr, 1, conflict_number);
	  mem_op->attr = L_concat_attr (mem_op->attr, mem_dep_attr);
	}
      }
      fscanf(in,")\n");
    }
    else {
      L_punt("L_get_mem_dep_profile : not load or store oper %d",mem_id);
    }
#endif
  }
  
  if (fscanf (in, "(end %s)\n", func_name) != 1) 
    L_punt ("Parse error in memory dependence profile file '%s'.", file_name);
  
  fclose (in);

  for (i = 0; i <=10; i++) {
    total_sync_arcs += ld_ld_mem_dep_stat[i];
    total_sync_arcs += ld_jsr_mem_dep_stat[i];
    total_sync_arcs += other_mem_dep_stat[i];
  }

  fptr = fopen("mem_dep", "wt");
  if (total_sync_arcs) {
    fprintf(fptr, "%d\n", zero_weight);
    for (i = 0; i <=10; i++) {
      fprintf(fptr, "%03d-%03d%% %8d %8.2lf\n", i == 0 ? 0 : (i-1)*10+1, i*10,
	      ld_ld_mem_dep_stat[i], 
	      ((double) ld_ld_mem_dep_stat[i])*100/total_sync_arcs);
      fprintf(fptr, "%03d-%03d%% %8d %8.2lf\n", i == 0 ? 0 : (i-1)*10+1, i*10,
	      ld_jsr_mem_dep_stat[i], 
	      ((double) ld_jsr_mem_dep_stat[i])*100/total_sync_arcs);
      fprintf(fptr, "%03d-%03d%% %8d %8.2lf\n", i == 0 ? 0 : (i-1)*10+1, i*10,
	      other_mem_dep_stat[i], 
	      ((double) other_mem_dep_stat[i])*100/total_sync_arcs);
    }
  }
  else {
    fprintf(fptr, "%d\n", zero_weight);
    for (i = 0; i <=10; i++) {
      fprintf(fptr, "%03d-%03d%% %8d %8.2lf\n", i == 0 ? 0 : (i-1)*10+1, i*10, 
	      0, (double) 0);
      fprintf(fptr, "%03d-%03d%% %8d %8.2lf\n", i == 0 ? 0 : (i-1)*10+1, i*10, 
	      0, (double) 0);
      fprintf(fptr, "%03d-%03d%% %8d %8.2lf\n", i == 0 ? 0 : (i-1)*10+1, i*10, 
	      0, (double) 0);
    }
  }
  fclose(fptr);
}
