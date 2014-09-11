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
 *	File :		l_get.c
 *	Description :	merge profile information into Lcode
 *	Author : 	Pohua Chang
 *	Revised 7-93	Scott A. Mahlke
 *		New Lcode changes.
 *	Revised 7-94	Teresa Johnson
 *		Major revision (rewrite)!
 *	Revised 2-95	Tom Conte
 *		Get fall-through weight from last branch in cb (if there
 *		is one)
 *      Revised 5-95	John C. Gyllenhaal
 *              Added support for loop interation profiling.
 *      Revised 9-97   Daniel A. Connors
 *              Added support for memory dependence profiling.
 *
 *	(C) Copyright 1990, Pohua Chang
 *	All rights granted to University of Illinois Board of Regents.
 *==========================================================================*/
#include <config.h>
#include "l_prof_read.h"
#include "l_hash.h"

#define INX_VERSION 1

/* Parameters of Lget */
int  Lprofile_util_insert_profile = 0;
char *Lprofile_util_profile_file = NULL;
int  Lprofile_util_insert_mp_profile = 0;
char *Lprofile_util_mp_profile_file = NULL;
int Lprofile_util_debug = 0;
char *Lprofile_util_mode = "insert";
char *Lprofile_util_index_file = NULL;
int  Lprofile_util_insert_mem_dep_profile = 0;
char *Lprofile_util_mem_dep_profile_file = "database.pdc";
char *Lprofile_util_mem_dep_annot_mode = "sync";
int  Lprofile_util_remove_zero_conflict_sync_arcs = 0;
int  Lprofile_util_remove_existing_sync_arcs = 1;

int  Lprofile_util_insert_value_profile = 0;
int  Lprofile_util_value_profile_percentage = 75;
char *Lprofile_util_value_profile_file = NULL;

int Lprofile_util_insert_mem_addr_profile = 0;

int  Lprofile_util_insert_ap_profile = 0;
char *Lprofile_util_ap_profile_file = NULL;

int Lprofile_merge_zero_weight = 1;

#define MAX_CASE	512
static int this_cc[MAX_CASE];
static double weight[MAX_CASE];

extern void L_get_mp_profile (L_Func *fn, char *file_name);
extern void L_get_mem_dep_profile (L_Func *fn, char *file_name);
extern void L_get_value_profile (L_Func *fn, char *file_name);
extern void init_memory_dependence_parameters();
extern void L_get_ap_profile (L_Func *fn, char *file_name);

#define OBJS_STRING_LEN 1000
#undef DEBUG_MEM_ANNOTATION
#undef HEAP_OBJ

static L_Flow *get_br_weight(br_id, oper, flow)
int br_id;
L_Oper *oper;
L_Flow *flow;
{
    int i, num;
    L_Flow *ret_flow = NULL;

    if (flow==NULL)
	L_punt("get_br_weight: flow is NULL");

    if (L_uncond_branch_opcode(oper)) {
	if (!L_is_predicated(oper)) {
	    flow->weight = L_br_weight(br_id);
	}
	else {
	    flow->weight = L_taken_weight(br_id);
	}
	ret_flow = flow->next_flow;
    }

    else if (L_register_branch_opcode(oper)) {
	num = L_cc_weight(br_id, this_cc, weight, MAX_CASE);
	for (i=0; i<num; i++) {
	    int c = this_cc[i];
	    double w = weight[i];
	    L_Flow *ptr;
	    for (ptr=flow; ptr!=NULL; ptr=ptr->next_flow) {
		if (ptr->cc==c) {
		    ptr->weight += w;
		    break;
		}
	    }
	    if (ptr==NULL) {	/* default */
		for (ptr=flow; ptr!=NULL; ptr=ptr->next_flow)
		    if (ptr->cc==L_SWITCH_DEFAULT_CC)
			break;
		if (ptr==NULL) {	
		    L_punt("switch default case is missing");
		}
		ptr->weight += w;
	    }
	}
	ret_flow = NULL;
    }

    else if (L_cond_branch_opcode(oper)) {
	flow->weight = L_taken_weight(br_id);
	ret_flow = flow->next_flow;
    }

    else {
	L_punt("get_br_weight: internal error");
    }

    return (ret_flow);
}

static void get_profile_weight(fn)
L_Func *fn;
{
    int br_id = 0, last_br_id;
    L_Cb *cb;
    L_Oper *oper;
    L_Flow *flow;
    /* 10/25/04 REK Commenting out unused variable to quiet compiler
     *              warning. */
#if 0
    int inst_weight;
    char attr_name[15];
#endif
    int mem_id = 0, malloc_id = 0;
    L_Attr * attr;

    fn->weight = L_profile_func_weight();

#ifdef DEBUG_MEM_ANNOTATION
    fprintf (stderr, "Processing function %s.\n", fn->name);
#endif

    if (Lprofile_util_debug)
	fprintf(stderr, "fn %s : weight %f\n", fn->name, fn->weight);
    for (cb=fn->first_cb; cb!=NULL; cb=cb->next_cb) {
	cb->weight = L_cb_weight(cb->id);
	L_update_cb_loop_info (cb);

	if (Lprofile_util_debug)
	    fprintf(stderr, "\tcb %d\t: weight %f\n", cb->id, cb->weight);
	flow = cb->dest_flow;

	/* Get the flow weights, keeping track of the last branch
	 * in each cb.  (Necessary for Tom Conte's sampled profile)
	 * -JCG 5/26/95
	 */
	last_br_id = -1;		
	for (oper=cb->first_op; oper!=NULL; oper=oper->next_op) {
	    if (L_general_branch_opcode(oper))
	      {
		flow = get_br_weight(br_id, oper, flow);
		last_br_id = br_id;  /* -JCG 5/26/95 */
		br_id++;
	      }
	    /* SER 20040510: HCH MICRO '04 */
	    else if (Lprofile_util_insert_mem_addr_profile &&
		     L_subroutine_call_opcode(oper) && L_is_label(oper->src[0])
		     && (L_matches_fn_name(oper->src[0]->value.l, "malloc") ||
			 L_matches_fn_name(oper->src[0]->value.l, "calloc")))
	      {
		if ((attr = L_find_attr (oper->attr, "malloc_id")))
		  oper->attr = L_delete_attr (oper->attr, attr);
		attr = L_new_attr ("malloc_id", 1);
		L_set_int_attr_field (attr, 0, L_get_malloc_id(malloc_id));
		oper->attr = L_concat_attr (oper->attr, attr);
		malloc_id++;
		br_id++;
	      }
	    else if (Lprofile_util_insert_mem_addr_profile &&
		     (L_load_opcode (oper) || L_store_opcode (oper)))
	      {
		int i,*mem_glob_entry, mem_glob_size, *mem_missing_entry,
		  num_mem = 0, bad = 0, obj_missing_size, num_stack,
		   * mem_heap_entry, mem_heap_size;
		L_AccSpec * accspec, * newspec;
		struct mem_obj *first = NULL, *last = NULL, *current, *next,
		  *new_obj;

		if ((attr = L_find_attr (oper->attr, "regalloc1")))
		  {
		    br_id++;
		    continue;
		  }

		mem_glob_entry = L_get_mem_glob_entry (mem_id, &mem_glob_size);
		mem_heap_entry = L_get_mem_heap_entry (mem_id, &mem_heap_size);
		mem_missing_entry =
		  L_get_mem_missing_entry (mem_id, &obj_missing_size);
		num_stack = L_get_mem_stack_count (mem_id);

#ifdef DEBUG_MEM_ANNOTATION
		fprintf (stderr, "Processing mem_id %d: cb %d, op %d.\n "
			 "  %d global, %d heap, %d missing.\n",
			 mem_id, cb->id, oper->id, mem_glob_size,
			 mem_heap_size, obj_missing_size);
#endif

		for (accspec = oper->acc_info; accspec;
		     accspec = accspec->next)
		  {
		    /* Skip def'ing loads, using stores. */
		    if (L_load_opcode (oper))
		      {
			if (accspec->is_def)
			  {
			    bad = 1;
			    break;
			  }
		      }
		    else
		      if (!(accspec->is_def))
			{
			  bad = 1;
			  break;
			}
		    /* Only process if not a heap obj (high id) */
		    if (accspec->id < HEAP_OBJ_BASE_ADDR)
		      {
			int found = 0;
			for (i = 0; mem_glob_entry[i*2] != 0; i++)
			  {
			    if (mem_glob_entry[i*2] == accspec->id)
			      {
				found = 1;
				break;
			      }
			  }
			num_mem++;
			new_obj = calloc (1, sizeof (mem_obj));
			new_obj->id = accspec->id;
			new_obj->version = accspec->version;
			new_obj->offset = accspec->offset;
			new_obj->size = accspec->size;
			if (found)
			  {
			    new_obj->weight = mem_glob_entry[2*i+1];
#ifdef DEBUG_MEM_ANNOTATION
			    fprintf (stderr, "  Processing profiled global: "
				     "id %d, version %d.\n", new_obj->id,
				     new_obj->version);
#endif
			  }
			else
			  {
			    if (i > mem_glob_size + 1)
			      L_punt ("Ran off the end of the array (%d) "
				      "trying to find id %d in op %d, cb %d.\n"
				      "Missing obj id0.", i, accspec->id,
				      oper->id, cb->id);
			    new_obj->weight = 0;
#ifdef DEBUG_MEM_ANNOTATION
			    fprintf (stderr, "  Processing non-profiled "
				     "global: id %d, version %d.\n",
				     new_obj->id, new_obj->version);
#endif
			  }
			/* insert in list */
			if (first == NULL)
			  {
			    first = new_obj;
			    last = new_obj;
			  }
			else if (first->weight < new_obj->weight)
			  {
			    new_obj->next = first;
			    first->prev = new_obj;
			    first = new_obj;
			  }
			else
			  {
			    current = first;
			    while (current &&
				   (current->weight >= new_obj->weight))
			      current = current->next;
			    if (current == NULL) /* new last */
			      {
				last->next = new_obj;
				new_obj->prev = last;
				last = new_obj;
			      }
			    else /* Slip between current & prev */
			      {
				new_obj->next = current;
				new_obj->prev = current->prev;
				new_obj->prev->next = new_obj;
				current->prev = new_obj;
			      }
			  }
		      }
#ifdef DEBUG_MEM_ANNOTATION
		    else
		      fprintf (stderr, "  Pre-existing heap object found.\n");
#endif
		  }
		if (bad)
		  continue;

#ifdef HEAP_OBJ
		/* Add heap objects to list. */
		for (i = 0; i < mem_heap_size; i++)
		  {
		    num_mem++;
		    new_obj = calloc (1, sizeof (mem_obj));
		    new_obj->id = mem_heap_entry[i * 3];
		    new_obj->weight = mem_heap_entry[i * 3 + 2];

		    if (!first)
		      {
			first = new_obj;
			last = new_obj;
		      }
		    else if (first->weight < new_obj->weight)
		      {
			new_obj->next = first;
			first->prev = new_obj;
			first = new_obj;
		      }
		    else
		      {
			current = first;
			while (current &&
			       (current->weight >= new_obj->weight))
			  current = current->next;
			if (current == NULL) /* new last */
			  {
			    last->next = new_obj;
			    new_obj->prev = last;
			    last = new_obj;
			  }
			else
			  {
			    new_obj->next = current;
			    new_obj->prev = current->prev;
			    current->prev->next = new_obj;
			    current->prev = new_obj;
			  }
		      }
		  }
#else
		for (i = 0; i < mem_heap_size; i++)
		  {
		    current = first;
		    while (current &&
			   (current->id != mem_heap_entry[i*3+1]))
		      current = current->next;
		    if (current != NULL)  /* found */
		      {
#ifdef DEBUG_MEM_ANNOTATION
			fprintf (stderr, "  Processing pre-existing heap obj, "
				 "id %d.\n", current->id);
#endif
			current->weight += mem_heap_entry[i*3+2];
			while (current->prev &&
			       current->weight > current->prev->weight)
			  { /* shift upward in list */
			    int temp;
			    /* swap weight */
			    temp = current->weight;
			    current->weight = current->prev->weight;
			    current->prev->weight = temp;

			    /* swap id */
			    temp = current->id;
			    current->id = current->prev->id;
			    current->prev->id = temp;

			    current = current->prev;
			  }
		      }
		    else  /* not found, make new obj */
		      {
			num_mem++;
			new_obj = calloc (1, sizeof (mem_obj));
			new_obj->id = mem_heap_entry[i*3+1];
			new_obj->weight = mem_heap_entry[i*3+2];

#ifdef DEBUG_MEM_ANNOTATION
			fprintf (stderr, "  Processing new heap obj id %d.\n",
				 new_obj->id);
#endif

			if (!first)
			  {
			    first = new_obj;
			    last = new_obj;
			  }
			else if (first->weight < new_obj->weight)
			  {
			    new_obj->next = first;
			    first->prev = new_obj;
			    first = new_obj;
			  }
			else
			  {
			    current = first;
			    while (current &&
				   (current->weight >= new_obj->weight))
			      current = current->next;
			    if (current == NULL) /* new last */
			      {
				last->next = new_obj;
				new_obj->prev = last;
				last = new_obj;
			      }
			    else
			      {
				new_obj->next = current;
				new_obj->prev = current->prev;
				current->prev->next = new_obj;
				current->prev = new_obj;
			      }
			  }
		      }
		  }
#endif
		/* Insert id0 object: assumed to be present. */
		if (mem_glob_entry[mem_glob_size * 2] != 0)
		  L_punt ("id0 object not at correct location in table.\n");
		new_obj = calloc (1, sizeof(mem_obj));
		new_obj->weight = mem_glob_entry[mem_glob_size * 2 + 1];
		if (last)
		  {
		    last->next = new_obj;
		    new_obj->prev = last;
		    last = new_obj;
		  }
		else
		  {
		    first = new_obj;
		    last = new_obj;
		  }

		/* Delete old attrs and acc_info */
		if ((attr = L_find_attr (oper->attr, "obj_weight")))
		  oper->attr = L_delete_attr (oper->attr, attr);
		if ((attr = L_find_attr (oper->attr, "obj_missing")))
		  oper->attr = L_delete_attr (oper->attr, attr);
		oper->acc_info = L_delete_mem_acc_spec_list (oper->acc_info);

		/* create new OBJS and obj_weight attr */
		attr = L_new_attr ("obj_weight", num_mem + 1);
		oper->attr = L_concat_attr (oper->attr, attr);

		current = first;
		for (i = 0; i < num_mem; i++)
		  {
		    newspec =
		      L_new_mem_acc_spec (L_store_opcode (oper), current->id,
					  current->version, current->offset,
					  current->size);
		    next = current->next;
                    free (current);
		    if (i == 0)
		      oper->acc_info = newspec;
		    else
		      accspec->next = newspec;
		    accspec = newspec;

		    L_set_int_attr_field (attr, i, current->weight);
		    current = next;
		  }
		/* Update id0 */
		L_set_int_attr_field (attr, num_mem, current->weight);

		/* Create obj_missing attr */
		if (obj_missing_size)
		  {
		    attr = L_new_attr ("obj_missing", obj_missing_size);
		    for (i = 0; i < obj_missing_size; i++)
		      L_set_int_attr_field (attr, i, mem_missing_entry[i*2]);
		    oper->attr = L_concat_attr (oper->attr, attr);
		  }

		/* Create stack_count attr */
		if ((attr = L_find_attr (oper->attr, "stack_count")))
		  oper->attr = L_delete_attr (oper->attr, attr);
		if (num_stack)
		  {
		    attr = L_new_attr ("stack_count", 1);
		    L_set_int_attr_field (attr, 0, num_stack);
		    oper->attr = L_concat_attr (oper->attr, attr);
		  }

		mem_id++;
		br_id++;  /* Need to increment this for proper alignment. */
	      }
	    else
	      {
		continue;
	      }
	}

	/* handle fallthru if it exists */
	if (flow) {
	    /* just some extra checking here */
	    if (L_register_branch_opcode(cb->last_op))
		L_punt("get_profile_weight: fallthru for jrg??");
	    if (L_uncond_branch_opcode(cb->last_op) && !L_is_predicated(cb->last_op))
		L_punt("get_profile_weight: fallthru for non-predicated jump??");
	    if (flow->next_flow!=NULL)
		L_punt("get_profile_weight: corrupt flow list");

	    /* there is a legal fallthru */

	    /* If there is a previous branch in this cb, use
	     * the branches fall thru weight to set the cb's fall thru weight,
	     * otherwise use the cb's weight.
	     * (needed for Tom Conte's sampled profile) -JCG 5/26/95
	     */
	    if (last_br_id != -1) {
		flow->weight = L_br_weight(last_br_id)-
		    L_taken_weight(last_br_id);
	    }
	    else
	      flow->weight = cb->weight;

#ifdef OLD_WAY	/* Before Tom Conte's 2-95 sampled profile patch */    
	    weight = cb->weight;
	    ptr = cb->dest_flow;
	    while (ptr!=flow) {
		weight -= ptr->weight;
		ptr = ptr->next_flow;
	    }
	    flow->weight = weight;
#endif /* OLD WAY */ 
	    
	}
	else {
	    if (L_cond_branch_opcode(cb->last_op))
		L_punt("get_profile_weight: no fallthru for cbr??");
	}
    }
}

int 
L_fix_func_weight(FILE *profile, L_Func *fn)
{
    L_Cb *cb;
    L_Oper *oper;
    L_Flow *flow;

    /*
     *	Get profile information for the function.
     */
    if (Lprofile_util_debug)
	fprintf(stderr, "Read profile file %s\n", Lprofile_util_profile_file);

    L_read_profile_func(profile,fn->name);

    if (!Lprofile_merge_zero_weight && (L_profile_func_weight() == 0.0))
      {
	if (fn->weight)
	  L_warn ("Lget [%s()]: preserving original non-zero profile",
		  fn->name);
	return 1;
      }

    /*
     *	Clear all weights.
     */
    fn->weight = 0.0;
    for (cb=fn->first_cb; cb!=NULL; cb=cb->next_cb) {
	cb->weight = 0.0;
	for (oper=cb->first_op; oper!=NULL; oper=oper->next_op) 
	    oper->weight = 0.0;
	for (flow=cb->src_flow; flow!=NULL; flow=flow->next_flow)
	    flow->weight = 0.0;
	for (flow=cb->dest_flow; flow!=NULL; flow=flow->next_flow)
	    flow->weight = 0.0;
    }

    /*
     *	Map information into Lcode.
     */
    get_profile_weight(fn);

    L_cleanup_mem_arrays();

    return 0;
}

void Lget_read_index(FILE *index)
{
        int err,version;
        long position;
        char name[500];

        err = fscanf(index,"# Profile index file --- Version %d\n",
                &version);
        if (err < 1) L_punt("index file: no file header found");
        while ((err = fscanf(index,"%s 0x%lx\n",name,&position)) != -1)
        {
           if (err < 2) L_punt("index file: incorrect format of entry");
           L_insert_position(name,position);
        }
}


static void Lget_insert() 
{
    FILE *profile,*index;

    /* open the profile to insert from */
    if (!(profile = fopen(Lprofile_util_profile_file,"r")))
        L_punt("Cannot open profile file '%s' for reading",
				Lprofile_util_profile_file);

    /* open its corresponding index file */
    if (!(index = fopen(Lprofile_util_index_file,"r")))
        L_punt("Cannot open index file '%s' - create one with Lget",
                                                Lprofile_util_index_file);

    /* read in index file into string hash table */
    L_init_string_hash();
    Lget_read_index(index);

    /* initialize the memory dependence parameters */
    init_memory_dependence_parameters();

    /* open the input Lcode file to insert into */
    L_open_input_file(L_input_file);

    /* process all data and functions within a file */
    while (L_get_input() != L_INPUT_EOF)
      {
        if (L_token_type==L_INPUT_FUNCTION)
	  {
	    int suppress = 0;
            L_define_fn_name (L_fn->name);

	    /* Get control flow profile (if desired) */
	    if (Lprofile_util_insert_profile)
	      suppress = L_fix_func_weight(profile,L_fn);
       
	    if (!suppress)
	      {
		/* Get misprediction profile (if desired) */
		if (Lprofile_util_insert_mp_profile)
		  L_get_mp_profile (L_fn, 
				    Lprofile_util_mp_profile_file);

		/* Get memory dependence profile (if desired) */
		if (Lprofile_util_insert_mem_dep_profile)
		  L_get_mem_dep_profile (L_fn, 
					 Lprofile_util_mem_dep_profile_file);

		if (Lprofile_util_insert_value_profile)
		  L_get_value_profile (L_fn, 
				       Lprofile_util_value_profile_file);

		if (Lprofile_util_insert_ap_profile)
		  L_get_ap_profile (L_fn, 
				    Lprofile_util_ap_profile_file);
	      }

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
    L_delete_string_hash();

}

void Lget_skip_func(FILE *profile,char *fn_name)
{
	char line[500],*c;

	fgets(line,200,profile);
	while (strncmp(line,"(end",4))
		fgets(line,200,profile);
	c = strchr(line,')');
	if (!c) L_punt("profile file: expected ')' after '(end '");
	*c = 0;
	if (strcmp(&line[5],fn_name))
		L_punt("profile file: function name in '(end ' does not match '%s'",fn_name);
}

void Lget_index_func(FILE *profile,FILE *index)
{
        int err,profile_count;
        long position;
	double weight;
        char name[500];

        /* Make the index file for this profile file */
        fprintf(index,"# Profile index file --- Version %d\n",
           INX_VERSION);

	err = fscanf(profile,"(count %d)\n",&profile_count);
	if (err < 1) L_punt("profile file: expected '(count '");
	
        /* get the position of the start of the first function */
        position = ftell(profile);

        /* read each function in the profile file */
        while ((err = fscanf(profile,"(begin %s %lf)\n",
                name,&weight)) != -1)
        {
           if (err < 2) L_punt("profile file: expected '(begin '");

           /* insert this function's position into the index file */
           fprintf(index,"%s %#lx\n",name,position);

           /* skip the rest of this function */
           Lget_skip_func(profile,name);

           /* get the position of the start of the next function */
           position = ftell(profile);
        }
}

void Lget_index()
{
        FILE *profile,*index;

        /* open profile file from which we will generate an index */
        if (!(profile = fopen(Lprofile_util_profile_file,"r")))
           L_punt("Cannot open file '%s' for reading",
				Lprofile_util_profile_file);

        /* open the index file to write into */
        if (!(index = fopen(Lprofile_util_index_file,"w")))
           L_punt("Cannot open file '%s' for writing",Lprofile_util_index_file);

        /* create index file */
        Lget_index_func(profile,index);
}


/*
 *      Read module specific parameters
 */
void L_read_parm_lprofile_utilities (ppi)
Parm_Parse_Info *ppi;
{
        L_read_parm_b(ppi, "insert_profile", 
			  &Lprofile_util_insert_profile);
	L_read_parm_s(ppi, "profile_file", &Lprofile_util_profile_file);
	L_read_parm_s(ppi, "index_file", &Lprofile_util_index_file);

	/* Miss prediction profiling and annotation */
	L_read_parm_b(ppi, "insert_mp_profile", 
			  &Lprofile_util_insert_mp_profile);
	L_read_parm_s(ppi, "mp_profile_file", 
			  &Lprofile_util_mp_profile_file);

        /* Memory dependence profiling and annotation */
        L_read_parm_b(ppi, "insert_mem_dep_profile",
                          &Lprofile_util_insert_mem_dep_profile);
        L_read_parm_s(ppi, "mem_dep_profile_file",
                          &Lprofile_util_mem_dep_profile_file);
        L_read_parm_s(ppi, "mem_dep_annotate_mode",
                          &Lprofile_util_mem_dep_annot_mode);
        L_read_parm_b(ppi, "remove_zero_conflict_sync_arcs",
                          &Lprofile_util_remove_zero_conflict_sync_arcs);
        L_read_parm_b(ppi, "remove_existing_sync_arcs",
                          &Lprofile_util_remove_existing_sync_arcs);

	/* Value profiling */
        L_read_parm_b(ppi, "insert_value_profile",
                          &Lprofile_util_insert_value_profile);
        L_read_parm_i(ppi, "value_profile_percentage",
                          &Lprofile_util_value_profile_percentage);
	L_read_parm_s(ppi, "value_profile_file", 
			  &Lprofile_util_value_profile_file);

	/* Memory address profiling */
	L_read_parm_b(ppi, "insert_mem_addr_profile",
		      &Lprofile_util_insert_mem_addr_profile);

	/* Address prediction profiling and annotation */
	L_read_parm_b(ppi, "insert_ap_profile", 
			  &Lprofile_util_insert_ap_profile);
	L_read_parm_s(ppi, "ap_profile_file", 
			  &Lprofile_util_ap_profile_file);

        /* General */
        L_read_parm_b(ppi, "debug", &Lprofile_util_debug);
        L_read_parm_s(ppi, "mode", &Lprofile_util_mode);

	L_read_parm_b (ppi, "merge_zero_weight", &Lprofile_merge_zero_weight);
}


void L_gen_code(Parm_Macro_List *command_line_macro_list)
{
    /* Load the parameters specific to Lget */
    L_load_parameters (L_parm_file, command_line_macro_list,
                       "(Lprofile_utilities", L_read_parm_lprofile_utilities);

    if (Lprofile_util_profile_file==NULL)
	L_punt("L_gen_code: need to specify file containing profile info");

    /* Call routine appropriate for mode */

    if (L_pmatch (Lprofile_util_mode, "insert"))
	Lget_insert ();

    else if (L_pmatch (Lprofile_util_mode, "index"))
	Lget_index ();

    else
	L_punt ("Lget mode '%s' not supported", Lprofile_util_mode);

}

