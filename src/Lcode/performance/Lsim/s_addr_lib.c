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
 *
 *  File:  s_lib.c
 *
 *  Description:  Commonly used functions for the simulator
 *
 *  Creation Date :  July, 1993
 *
 *  Author:  John Gyllenhaal, Roger A. Bringmann, Wen-mei Hwu
 *
 *  Revisions:
 *
\*****************************************************************************/

#include <config.h>
#include "s_main.h"

/*
 * Calculates the function addresses (Lemulate id's) from function order.
 * -ITI (JCG) 1/99
 */
void S_calc_fn_addresses_from_order(fn_list) 
S_Fn *fn_list;
{
    S_Fn *fn;
    int id;

    /* Assign sequential ids to each function as an address */
    id = 1000;  /* Start at 1000 +1 */
    for (fn = fn_list; fn != NULL; fn = fn->next_fn)
    {
	fn->addr = id;
	id++;
    }
}

/*
 * Gets the function addresses (from a list of func names and addresses)
 * for the sfn list passed to it.
 * Punts on error or if not all the function addresses were found.
 */
void S_read_fn_addresses_from_list (fn_list, list_file_name)
S_Fn *fn_list;
char *list_file_name;
{
    char name[1000];
    int address;
    S_Fn *fn;
    FILE *in;
    int not_found;


    /* Open the list file for reading */
    if ((in = fopen (list_file_name, "r")) == NULL)
	S_punt ("Unable to open fn addr list '%s' for reading",
		list_file_name);

    /* Initialized all addresses to zero */
    for (fn = fn_list; fn != NULL; fn = fn->next_fn)
	fn->addr = 0;

    not_found = 0;
    /* For each line in addr list, find function addr is for */
    while (fscanf (in, "%s %x\n", name, &address) == 2)
    {
	/* Find matching name */
	for (fn = fn_list; fn != NULL; fn = fn->next_fn)
	{
	    if (strcmp (fn->name, name) == 0)
	    {
		fn->addr = address;
		break;
	    }
	}
	
	/* Print error message (but punt later) if name not found */
	if (fn == NULL)
	{
	    fprintf (stderr, 
		     "Error: function '%s' in addr list but not source.\n",
		     name);
	    not_found++;
	}
    }

    /* Make sure all the functions have addresses */
    for (fn = fn_list; fn != NULL; fn = fn->next_fn)
    {
	if (fn->addr == 0)
	{
	    fprintf (stderr,
		     "Error: source function '%s' not in addr list.\n",
		     fn->name);
	    not_found++;
	}
    }

    /* Punt now if not all the names were found */
    if (not_found != 0)
	S_punt ("Cannot continue: errors occured loading fn address list.");
}

/*
 * Gets the function address (from the executable) for the sfn list
 * passed to it.
 * Punts on error or if not all the function addresses were found.
 */
#ifndef _HPUX_SOURCE
/* The new Lemulate based framework does not need this functionality anymore.
 * Leave in only for backward compatibility on HPUX systems. -JCG 5/99
 */
void S_read_fn_addresses_from_exec (fn_list, exec_name)
S_Fn *fn_list;
char *exec_name;
{
    S_punt ("S_read_fn_address_from_exec: Should not be called!\n");
}
#else
void S_read_fn_addresses_from_exec (fn_list, exec_name)
S_Fn *fn_list;
char *exec_name;
{
    struct nlist *nlist_array;
    S_Fn	*fn;
    int	fns_in_list, nlist_size, not_found, i;
    FILE *exec_in;

    /* Count the number of functions in the list */
    fns_in_list = 0;
    for (fn = fn_list; fn != NULL; fn = fn->next_fn)
	fns_in_list++;

    /* Allocate nlist array with space for terminator */
    nlist_size = (fns_in_list + 1) * sizeof (struct nlist);
    if ((nlist_array = (struct nlist *) malloc (nlist_size)) == NULL)
	S_punt ("S_read_fn_address: Out of memory");
    
    /* Point array at function names for call */
    for (fn = fn_list, i = 0; fn != NULL; fn = fn->next_fn, i++)
    {
#ifdef linux
#ifndef __ELF__
	nlist_array[i].n_un.n_name = fn->asm_name;
#else
	nlist_array[i].n_name = fn->asm_name;
#endif
#else
	nlist_array[i].n_name = fn->asm_name;
#endif
    }

    /* Terninate nlist array with 0 for name */
#ifdef linux
#ifndef __ELF__
    nlist_array[fns_in_list].n_un.n_name = 0;
#else
    nlist_array[fns_in_list].n_name = 0;
#endif
#else
    nlist_array[fns_in_list].n_name = 0;
#endif

    /* Make sure we have permission to open file (just open and close it) */
    if ((exec_in = fopen (exec_name, "r")) == NULL)
	S_punt ("Unable to open executable '%s' for reading\n", exec_name);
    fclose (exec_in);

    /* Call nlist to get all the fn addresses */
    if (nlist (exec_name, nlist_array) < 0)
	S_punt ("'%s' executable is incompatable with architecture.",
		exec_name);

    /* Get addresses from nlist array for functions */
    not_found = 0;
    for (fn = fn_list, i = 0; fn != NULL; fn = fn->next_fn, i++)
    {
	fn->addr = nlist_array[i].n_value;

	/* Function not found if addr == 0 */
	if (fn->addr == 0)
	{
	    fprintf (stderr, "Error: Function '%s' not found in '%s'.\n",
		     fn->asm_name, exec_name);
	    not_found++;
	}
    }
    
    /* Punt if any not found */
    if (not_found != 0)
	S_punt ("Unable to read all function's addresses.\n");
    
    free (nlist_array);
}
#endif
/*
 * An efficient way to allocate strings that will never (and can never) be 
 * freed. 
 */ 
char *S_alloc_string (size)
int size;
{
    static char *head=NULL;
    static int size_left=0;
    int alloc_size;
    char *ptr;
    
    /* If don't have enough memory allocated, allocate a whole new chunk */
    if (size > size_left)
    {
	alloc_size = 4096;
	if (size > alloc_size)
	    alloc_size = size;
	if ((head = (char *) malloc (alloc_size)) == NULL)
	    S_punt ("S_alloc_string: Out of memory");
	size_left = alloc_size;
    }
    
    /* Get the next 'size' characters from the array */
    ptr = head;
    head += size;
    size_left -= size;
    return (ptr);
}


