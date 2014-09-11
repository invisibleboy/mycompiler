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
 *	File :		l_prof_read.c
 *	Description :	reading/writing Lcode profile info file
 *	Author : 	Teresa Johnson, Pohua Chang, Scott Mahlke
 *
 *	(C) Copyright 1990, Pohua Chang.
 *	All rights granted to University of Illinois Board of Regents.
 *==========================================================================*/

#include <config.h>
#include "l_prof_read.h"
#include "l_hash.h"

#undef ECHO

#define MAX_CB		65536
#define MAX_BR		65536
#define MAX_CC		4096
#define MAX_MEM         8192
#define MAX_MALLOC       128

struct CC {
	double		weight;
	int 		cc;
	struct CC	*next;
} ;

/* Add iteration information to loop cbs -JCG 5/10/95 
 * As read in information, build attributes that will
 * be inserted into code. 
 */
struct Iter_Header {
        L_Attr		*head;
	L_Attr		*tail;
	int		entry_count; /* Number of iteration entries */
	int		input_count; /* Number inputs profiled on */
};


static struct CC cc[MAX_CC];
static int n_cc;

static int profile_count;
static char fn_name[500];
static double fn_weight;

static char cb_defined[MAX_CB];
static double cb_weight[MAX_CB];
static struct Iter_Header *cb_iter_header[MAX_CB];

#define BR_TYPE		1
#define JP_TYPE		2
#define JP_RG_TYPE	3
#define JP_PR_TYPE	4	/* SAM, 1-94 predicated jump */

static char br_defined[MAX_BR];
static double br_weight[MAX_BR];
static double br_taken_weight[MAX_BR];
static struct CC *br_cc_weight[MAX_BR];

int mem_glob_obj_size[MAX_MEM] = {0};
int * mem_glob_obj_table[MAX_MEM] = {NULL};
int mem_heap_obj_num[MAX_MEM] = {0};
int * mem_heap_obj_table[MAX_MEM] = {NULL};
int malloc_table[MAX_MALLOC] = {0};
int mem_stack_count[MAX_MEM] = {0};
int mem_missing_obj_num[MAX_MEM] = {0};
int * mem_missing_obj_table[MAX_MEM] = {NULL};

#undef DEBUG_READ_OBJ

void L_read_profile_func(profile, func) 
FILE *profile;
char *func;
{
	int i, err, id, c, n;
	double weight, taken_weight;
	struct Iter_Header *iter_header;
	L_Attr *attr;
	/* 10/25/04 REK Commenting out unused variable to quiet compiler
	 *              warning. */
#if 0
	int inst_weight;
#endif
	int    iterations, entry, input;
	double iter_weight, avg_iter_weight, iter_sum;
	char name[500], iter_buf[500];
	int mem_op_id, mem_obj_id, mem_weight, mem_num_glob_objs,
	  mem_num_heap_objs, mem_malloc, malloc_id, stack_count,
	  num_obj_missing;

	/* Initialize profile data holder */
	profile_count = 0;
	fn_weight = 0.0;

	/* read in first line of profile file if it is the first one */
	err = fscanf(profile,"(count %d)\n",&profile_count);
	if (err < 1) ungetc('(',profile);
#ifdef ECHO
	printf("(count %d)\n", profile_count);
#endif

	/* set position to the start of this function */
	fseek(profile,L_get_position(func),0);

	/* read in function weight and make sure we have the right function */
	err = fscanf(profile,"(begin %s %lf)\n",fn_name,&fn_weight);
	if (err < 2) L_punt("profile file: incorrect start of function '%s'",
								fn_name);
#ifdef ECHO
	printf("\n(begin %s %f)\n", fn_name, fn_weight);
#endif
#ifdef DEBUG_READ_OBJ
	fprintf (stderr, "Processing function %s.\n", fn_name);
#endif
	if (strcmp(func,fn_name))
		L_punt("profile file: functions '%s' and '%s' do not match",
					fn_name,func);

	/* init structures */
    	for (i=0; i<MAX_CB; i++) 
	{
	    cb_defined[i] = 0;
	    cb_iter_header[i] = NULL;
	}
    	for (i=0; i<MAX_BR; i++) 
	{
	    br_defined[i] = 0;
	}
    	n_cc = 0;
	mem_op_id = 0;
	malloc_id = 0;

	/* check if next line is "(end ...)" */
	while((err = fscanf(profile,"(end %s\n",fn_name)) != -1)
	{

	 if (err == 1)
	 {
	   if (fn_name[strlen(fn_name)-1] == ')') 
		fn_name[strlen(fn_name)-1] = 0;
#ifdef ECHO
	   printf("(end %s)\n",fn_name);
#endif
	   if (strcmp(func,fn_name))
		L_punt("profile file: '(end %s)' does not match function '%s'",
					fn_name,func);
	   return;
	 }
	
	 /* not "end", so read in and check what kind this is */
	 err = fscanf(profile," %s %d %lf",name,&id,&weight);
	 if (err < 3) L_punt("profile file: Incorrect format of line");

	 /* which kind? */

	 if ((strcmp(name, "cb") == 0) || (strcmp(name, "lcb") == 0)) 
	 {
	   /* If loop cb, get number of lines of loop interation info */
	   if (strcmp(name, "lcb") == 0)
	   {
	       /* Create iter_header structure */
	       iter_header = (struct Iter_Header *)malloc(sizeof(struct Iter_Header));
	       if (iter_header == NULL)
		   L_punt ("Out of memory allocating iter_header");

	       /* Initialize header */
	       iter_header->head = NULL;
	       iter_header->tail = NULL;
	       err = fscanf(profile," %d %d\n", &iter_header->entry_count,
		      &iter_header->input_count);
	       if (err != 2)
		   L_punt ("profile file: Incorrect lcb format");
	   }
	   else
	   {
	       fscanf(profile," )\n");
	       iter_header = NULL;
	   }


#ifdef ECHO
	   printf(" (cb %d %f)\n", id, weight);
#endif
	   if ((id<0) || (id>=MAX_CB))
	      L_punt("L_read_profile_func: cb id is out-of-range");
	   if (cb_defined[id]!=0) 
	   {
	      fprintf(stderr, "# cb %d is multiply defined in fn %s\n",
							id, fn_name);
	      L_punt("L_read_profile_func: too bad!!");
	   }
	   cb_defined[id] = 1;
	   cb_weight[id] = weight;

	   /* Read in any iter info if have loop cb*/
	   if (iter_header != NULL)
	   {
	       for (entry=0; entry < iter_header->entry_count; entry++)
	       {
		   /* Read in iterations */
		   err = fscanf(profile, " ( %d", &iterations);
		   if (err != 1) L_punt ("profile file: illegal iter info");
		   
		   /* Get attribute for iteration */
		   sprintf (iter_buf, "iter_%d", iterations);
		   attr = L_new_attr(iter_buf, iter_header->input_count);

		   /* Initialize the iter_sum */
		   iter_sum = 0.0;

		   /* Read in input info */
		   for (input=1; input <= iter_header->input_count; input++)
		   {
		       err = fscanf(profile, " %lf", &iter_weight);
		       if (err != 1) 
		       {
			   L_punt ("profile file: illegal iter weight %i",
				   input);
		       }

		       /* Set field in attribute */
		       L_set_double_attr_field (attr, input, iter_weight);

		       iter_sum += iter_weight;
		   }

		   /* Get the ending ) */
		   fscanf (profile, ")\n");

		   /* Set the average iteration weight */
		   avg_iter_weight = iter_sum / 
		       ((double) iter_header->input_count);
		   L_set_double_attr_field (attr, 0, avg_iter_weight);
		   
		   /* Add to end of header_info attr list */
		   if (iter_header->tail == NULL)
		       iter_header->head = attr;
		   else
		       iter_header->tail->next_attr = attr;
		   attr->next_attr = NULL;
		   iter_header->tail = attr;
	       }
	       /* Get end ) */
	       fscanf(profile," )\n");
	   }
	   /* Point cb structure to this loop_header */
	   cb_iter_header[id] = iter_header;

	 } 

	 else if (! strcmp(name, "b")) 
	 {
	   err = fscanf(profile," %lf)\n",&taken_weight);
	   if (err < 1) L_punt("profile file: illegal end of br");
#ifdef ECHO
	   printf(" (b %d %f %f)\n", id, weight, taken_weight);
#endif
	   if ((id<0) || (id>=MAX_BR))
	      L_punt("L_read_profile_func: br id is out-of-range");
	   if (br_defined[id]!=0)
	   {
	      fprintf(stderr, "# br %d is multiply defined in fn %s\n",
								id, fn_name);
	      L_punt("L_read_profile_func: too bad!!");
	   }
	   br_defined[id] = BR_TYPE;
	   br_weight[id] = weight;
	   br_taken_weight[id] = taken_weight;
	   br_cc_weight[id] = 0;
	 } 

	 else if (! strcmp(name, "j")) 
	 {
	   fscanf(profile," )\n");
#ifdef ECHO
	   printf(" (j %d %f)\n", id, weight);
#endif
	   if ((id<0) || (id>=MAX_BR))
	      L_punt("L_read_profile_func: br id is out-of-range");
	   if (br_defined[id]!=0) 
	   {
	      fprintf(stderr, "# jump %d is multiply defined in fn %s\n",
								id, fn_name);
	      L_punt("L_read_profile_func: too bad!!");
	   }
	   br_defined[id] = JP_TYPE;
	   br_weight[id] = weight;
	   br_taken_weight[id] = weight;
	   br_cc_weight[id] = 0;
	 } 

	 else if (! strcmp(name, "jrg")) 
	 {
	   fscanf(profile," \n");
	   if ((id<0) || (id>=MAX_BR))
	      L_punt("L_read_profile_func: br id is out-of-range");
	   if (br_defined[id]!=0) 
	   {
	      fprintf(stderr, "# jp_rg %d is multiply defined in fn %s\n",
			id, fn_name);
	      L_punt("L_read_profile_func: too bad!!");
	   }
	   br_defined[id] = JP_RG_TYPE;
	   br_weight[id] = weight;
	   br_taken_weight[id] = weight;
	   br_cc_weight[id] = 0;

	   /* read in all cc's */
	   while ((err = fscanf(profile," ( %d %lf)\n",&c,&weight)) == 2)
	   {
              n = n_cc++;
	      if (n>=MAX_CC)
		L_punt("L_read_profile_func: too many CC");
	      cc[n].cc = c;
	      cc[n].weight = weight;
	      cc[n].next = 0;
	      if (br_cc_weight[id]==0) 
	      {
		br_cc_weight[id] = cc+n;
	      } 
	      else 
	      {
		struct CC *last;
		for (last=br_cc_weight[id]; last->next!=0; )
		    last = last->next;
		last->next = cc+n;
	      }
	   }

	   fscanf(profile," )\n");

#ifdef ECHO
	   printf(" (jrg %d %f\n", id, weight);
	   for (cc_ptr=br_cc_weight[id]; cc_ptr!=0; cc_ptr=cc_ptr->next) 
	   {
	      printf("    (%d %f)\n", cc_ptr->cc, cc_ptr->weight);
	   }
	   printf(" )\n");
#endif
	 } 

	 else if (! strcmp(name, "pj")) 
	 {
	   err = fscanf(profile," %lf)\n",&taken_weight);
	   if (err < 1) L_punt("profile file: illegal end of pj");

#ifdef ECHO
	   printf(" (pj %d %f %f)\n", id, weight, taken_weight);
#endif
	   if ((id<0) || (id>=MAX_BR))
	      L_punt("L_read_profile_func: pj id is out-of-range");
	   if (br_defined[id]!=0) 
	   {
	      fprintf(stderr, "# pj %d is multiply defined in fn %s\n",
			id, fn_name);
	      L_punt("L_read_profile_func: too bad!!");
	   }
	   br_defined[id] = JP_PR_TYPE;
	   br_weight[id] = weight;
	   br_taken_weight[id] = taken_weight;
	   br_cc_weight[id] = 0;
	 } 

	 else if (!strcmp(name, "ld") || !strcmp(name, "st"))
	   {
	     err = fscanf(profile, " %i %i %i %i\n", &mem_num_glob_objs,
			  &mem_num_heap_objs, &stack_count, &num_obj_missing);
#ifdef DEBUG_READ_OBJ
	     fprintf (stderr, "Mem op %d: %d global, %d heap, %d missing\n",
		      mem_op_id, mem_num_glob_objs, mem_num_heap_objs,
		      num_obj_missing);
#endif
	     if ((mem_op_id<0) || (mem_op_id>MAX_MEM))
	       L_punt("L_read_profile_func: mem_op_id is out-of-range, "
		      "may need to increase MAX_MEM");
	     mem_glob_obj_size[mem_op_id] = mem_num_glob_objs;
	     mem_glob_obj_table[mem_op_id] =
	       malloc ((mem_num_glob_objs+1) * 2 * sizeof (int));
	     mem_heap_obj_num[mem_op_id] = mem_num_heap_objs;
	     if (mem_num_heap_objs)
	       mem_heap_obj_table[mem_op_id] =
		 malloc (mem_num_heap_objs * 3 * sizeof (int));

	     mem_stack_count[mem_op_id] = stack_count;
	     if ((mem_missing_obj_num[mem_op_id] = num_obj_missing))
	       mem_missing_obj_table[mem_op_id] =
		 malloc (num_obj_missing * 2 * sizeof(int));

	     /* Read in entries */
	     for (i = 0; i <= mem_num_glob_objs * 2; i += 2)
	       {
		 err = fscanf (profile, " (%i %i)\n", &mem_obj_id,
			       &mem_weight);
		 if (err != 2)
		   {
		     
		     L_punt ("profile file: illegal glob mem entry");
		   }
		 (mem_glob_obj_table[mem_op_id])[i] = mem_obj_id;
		 (mem_glob_obj_table[mem_op_id])[i+1] = mem_weight;
#ifdef DEBUG_READ_OBJ
		 fprintf (stderr, "  Read in obj %d: id %d, amount "
			  "%d\n", i/2, (mem_glob_obj_table[mem_op_id])[i],
			  (mem_glob_obj_table[mem_op_id])[i+1]);
#endif
	       }
	     for (i = 0; i < mem_num_heap_objs * 3; i += 3)
	       {
		 err = fscanf (profile, " (%i %i %i)\n", &mem_obj_id,
			       &mem_malloc, &mem_weight);
		 if (err != 3)
		   L_punt ("profile file: illegal heap mem entry");
		 (mem_heap_obj_table[mem_op_id])[i] = mem_obj_id;
		 (mem_heap_obj_table[mem_op_id])[i+2] = mem_weight;
		 (mem_heap_obj_table[mem_op_id])[i+1] = mem_malloc;
#ifdef DEBUG_READ_OBJ
		 fprintf (stderr, "  Read in heap obj %d: obj id %d, "
			  "malloc %d, amount %d\n", i/3,
			  (mem_heap_obj_table[mem_op_id])[i],
			  (mem_heap_obj_table[mem_op_id])[i+1],
			  (mem_heap_obj_table[mem_op_id])[i+2]);
#endif
	       }
	     for (i = 0; i < num_obj_missing; i++)
	       {
		 err = fscanf (profile, " (%i %i)\n", &mem_obj_id,
			       &mem_weight);
		 if (err != 2)
		   L_punt ("profile file: illegal missing mem entry");
		 (mem_missing_obj_table[mem_op_id])[i*2] = mem_obj_id;
		 (mem_missing_obj_table[mem_op_id])[i*2+1] = mem_weight;
#ifdef DEBUG_READ_OBJ
		 fprintf (stderr, "  Read in missing obj %d: obj id %d, "
			  "amount %d\n", i,
			  (mem_missing_obj_table[mem_op_id])[i*2],
			  (mem_missing_obj_table[mem_op_id])[i*2+1]);
#endif
	       }
	     mem_op_id++;

	     /* Get end */
	     fscanf(profile,")\n");
	   }
	 else if (!strcmp(name, "malloc"))
	   {
	     err = fscanf(profile, " %i)\n", &mem_malloc);
	     malloc_table[malloc_id++] = mem_malloc;
	   }
	 else 
	   {
	     L_punt("profile file: illegal type of profile info '%s'",
		    name);
	   }

	}	/* end of while loop */

	/* reached end of file before seeing end of this function */
	L_punt("profile file: reached end-of-file before seeing (end ...)");
}

double L_profile_func_weight() {
    return (fn_weight);
}

double L_cb_weight(cb_id) 
int cb_id;
{
    if ((cb_id<0) || (cb_id>=MAX_CB))
	L_punt("L_cb_weight: cb id is out of range");
    if (cb_defined[cb_id]==0)
	return 0.0;
    else
    	return (cb_weight[cb_id]);
}

void L_update_cb_loop_info(L_Cb *cb)
{
    L_Attr *attr, *next_attr, *header_attr;
    struct Iter_Header *iter_header;
    int cb_id;

    cb_id = cb->id;

    if ((cb_id<0) || (cb_id>=MAX_CB))
	L_punt("L_lcb_attrs: cb id is out of range");

    /* Unlike the original code, I think the cb should always be defined */
    if (cb_defined[cb_id]==0)
    {
	fprintf (stderr, 
		 "Warning: %s cb %i not in profiling info, weight set to 0.\n",
		 L_fn->name, cb->id);
    }

    /* Remove any existing iter info attributes
     * (Even if not adding info)
     */
    for (attr = cb->attr; attr != NULL; attr = next_attr)
    {
	/* Get next attribute before doing anything */
	next_attr = attr->next_attr;

	/* Delete any existing iteration_header or iter_* attributes */
	if ((strcmp(attr->name, "iteration_header") == 0) ||
	    (strncmp(attr->name, "iter_", 5) == 0))
	{
	    /* For now, call library delete routine */
	    cb->attr = L_delete_attr (cb->attr, attr);
	}
    }
    
    /* If not a loop header, return now */
    if (cb_iter_header[cb_id]==NULL)
	return;

    else if (cb_iter_header[cb_id] == (struct Iter_Header *)-1)
	L_punt ("Error %s cb %i: Cannot update loop info twice!",
		L_fn->name, cb_id);

    /* Add iteration_header and iter_* attributes to the cb */
    else
    {
	/* Get the loop header */
	iter_header = cb_iter_header[cb_id];

	/* Create header attribute */
	header_attr = L_new_attr ("iteration_header", 2);
	L_set_int_attr_field(header_attr, 0,  iter_header->entry_count);
	L_set_int_attr_field(header_attr, 1, iter_header->input_count);

	/* Concatenate the iter_* to header */
	header_attr = L_concat_attr (header_attr, iter_header->head);

	/* Add to cb's attributes */
	cb->attr = L_concat_attr (cb->attr, header_attr);

	/* Free up header, not legal to use again 
	 * (Initialize fields for debugging purposes)
	 */
	iter_header->head = NULL;
	iter_header->tail = NULL;
	iter_header->entry_count = -1;
	iter_header->input_count = -1;

	/* Free header */
	free (iter_header);
	
	/* Set to invalid value for debugging */
	cb_iter_header[cb_id] = (struct Iter_Header *)-1;
    }
}

double L_br_weight(id)
int id;
{
    if ((id<0) || (id>=MAX_BR))
	L_punt("L_br_weight: br id is out of range");
    if (br_defined[id]==0)
	return 0.0;
    else
    	return (br_weight[id]);
}

double L_taken_weight(id)
int id;
{
    if ((id<0) || (id>=MAX_BR))
	L_punt("L_taken_weight: br id is out of range");
    if (br_defined[id]==0)
	return 0.0;
    else
    	return (br_taken_weight[id]);
}

int L_cc_weight(id, cc, weight, len) 
int id, cc[], len;
double weight[];
{
    struct CC *ptr;
    int n;
    if ((id<0) || (id>=MAX_BR))
	L_punt("L_taken_weight: br id is out of range");
    if (br_defined[id]==0)
	return 0;
    if (br_defined[id]!=JP_RG_TYPE) {
	fprintf(stderr, "try to access a non-jump_rg record for switch info\n");
	L_punt("L_cc_weight: not a jump_rg");
    }
    ptr = br_cc_weight[id];
    n = 0;
    for (; ptr!=0; ptr=ptr->next) {
	if (n>=len)
	    L_punt("L_cc_weight: input arrays are too small");
	weight[n] = ptr->weight;
	cc[n] = ptr->cc;
	n++;
    }
    return n;
}

int *
L_get_mem_glob_entry (int id, int * num)
{
  if ((id < 0) || (id > MAX_MEM))
    L_punt ("L_get_mem_glob_entry: id is out of range");

  *num = mem_glob_obj_size[id];

  return mem_glob_obj_table[id];
}

int *
L_get_mem_heap_entry (int id, int *num)
{
  if ((id < 0) || (id > MAX_MEM))
    L_punt ("L_get_mem_heap_entry: id is out of range");

  *num = mem_heap_obj_num[id];

  return mem_heap_obj_table[id];
}

int
L_get_malloc_id (int id)
{
  return malloc_table[id];
}

int *
L_get_mem_missing_entry (int id, int *num)
{
  if ((id < 0) || (id > MAX_MEM))
    L_punt ("L_get_mem_heap_entry: id is out of range");

  *num = mem_missing_obj_num[id];

  return mem_missing_obj_table[id];
}

int
L_get_mem_stack_count (int id)
{
  if ((id < 0) || (id > MAX_MEM))
    L_punt ("L_get_mem_stack_count: id is out of range");
  return mem_stack_count[id];
}

int
L_matches_fn_name (char *fn_name, char *test_name)
{
  int matches;

  /* Assume doesn't match */
  matches = 0;

  /* Has fn_name been prefixed with '_$fn_' (hppa only) */
  if ((fn_name[0] == '_') && (fn_name[1] == '$') &&
      (fn_name[2] == 'f') && (fn_name[3] == 'n') && (fn_name[4] == '_'))
    {
      /* Yes, strip off for comparision */
      if (strcmp (&fn_name[5], test_name) == 0)
        matches = 1;
    }

  /* Has fn_name been prefixed with '_' (normal case) */
  else if (fn_name[0] == '_')
    {
      /* Yes, strip off for comparision */
      if (strcmp (&fn_name[1], test_name) == 0)
        matches = 1;
    }

  else
    L_punt ("L_matches_fn_name: No '_' prefix on '%s'", fn_name);

  return (matches);
}

void
L_cleanup_mem_arrays()
{
  int i;

  for (i = 0; i < MAX_MEM; i++)
    {
      if (mem_glob_obj_table[i])
	{
	  free (mem_glob_obj_table[i]);
	  mem_glob_obj_table[i] = NULL;
	}
      mem_heap_obj_num[i] = 0;
      if (mem_heap_obj_table[i])
	{
	  free (mem_heap_obj_table[i]);
	  mem_heap_obj_table[i] = NULL;
	}
      malloc_table[i] = 0;
      mem_stack_count[i] = 0;
      mem_missing_obj_num[i] = 0;
      if (mem_missing_obj_table[i])
	{
	  free (mem_missing_obj_table[i]);
	  mem_missing_obj_table[i] = NULL;
	}
    }
}
