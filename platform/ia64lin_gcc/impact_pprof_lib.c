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
/*==================================================================
 *  File          : impact_pprof_lib.c
 *  Description   : Support for Pcode profiling
 * 
 *  Created by John C. Gyllenhaal, Le-chun Wu, Wen-mei Hwu 4/99
 * 
 * The following command was used to generate impact_pprof_lib.o: 
 *   gcc -c -O -I${IMPACT_REL_PATH}/include impact_pprof_lib.c
 *==================================================================*/
/* 07/01/02 REK Modifying the profiling code so that the output files are
 *              dumped to a known directory instead of the current one. 
 *              Since the init functions now need to be called with an 
 *              argument specifying a directory, I have modified 
 *              Pcode/Plib_probe/probe.c as well. */
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
/* 07/01/02 REK #including sys/param.h to get MAXPATHLEN */
#include <sys/param.h>

/* 
 * Specialized, trimmed down, and renamed version of 
 * impact/src/library/dynamic_symbol/int_symbol.[ch] for use by
 * the Pcode loop iteration profiler. -JCG 4/99
 *
 * Note: Converted all L_alloc calls back to mallocs.  Changed
 *       all punts into fprintf()s and exit()s.  Did not convert
 *       delete routines since data needed until program finishes
 *       (so no need to free).
 */

/*
 * ITER symbol table structures and prototypes
 */
typedef struct ITER_Symbol
{
    int                         iter;           /* Loop iterations */
    int                         weight;         /* Profile weight */
    struct ITER_Symbol_Table    *table;         /* For deletion of symbol */
    struct ITER_Symbol          *next_hash;     /* For table's hash table */
    struct ITER_Symbol          *prev_hash;
    struct ITER_Symbol          *next_symbol;   /* For table's contents list */
    struct ITER_Symbol          *prev_symbol;

} ITER_Symbol;

typedef struct ITER_Symbol_Table
{
    ITER_Symbol         **hash;         /* Array of size hash_size */
    int                 hash_size;      /* Must be power of 2 */
    int                 hash_mask;      /* AND mask, (hash_size - 1) */
    int                 resize_size;    /* When reached, resize hash table */
    ITER_Symbol         *head_symbol;   /* Contents list */
    ITER_Symbol         *tail_symbol;
    int                 symbol_count;
} ITER_Symbol_Table;

/* Create and initialize ITER_Symbol_Table.
 * Creates a hash table of initial size 2 * expected_size rounded up
 * to the closest power of two.  (Min hash size 32)
 */
static ITER_Symbol_Table *ITER_new_symbol_table ()
{
    ITER_Symbol_Table *table;
    ITER_Symbol **hash;
    unsigned int min_size, hash_size;
    int i;

    /* Start with the minumum hash size of 32.  
     * (Smaller sizes don't work as well with the hashing algorithm)
     */
    hash_size = 32;

    /* Allocate symbol table */
    table = (ITER_Symbol_Table *) malloc (sizeof(ITER_Symbol_Table));
    if (table == NULL)
    {
	fprintf (stderr, "ITER_new_symbol_table: Out of memory");
	exit (1);
    }

    /* Allocate array for hash */
    hash = (ITER_Symbol **) malloc (hash_size * sizeof(ITER_Symbol *));
    if (hash == NULL)
    {
	fprintf (stderr, "ITER_new_symbol_table: Out of memory");
	exit (1);
    }

    /* Initialize hash table */
    for (i=0; i < hash_size; i++)
        hash[i] = NULL;

    /* Initialize fields */
    table->hash = hash;
    table->hash_size = hash_size;
    table->hash_mask = hash_size -1; /* AND mask, works only for power of 2 */
    /* Resize when count at 75% of hash_size */
    table->resize_size = hash_size - (hash_size >> 2);
    table->head_symbol = NULL;
    table->tail_symbol = NULL;
    table->symbol_count = 0;

    return (table);
}

/* Doubles the symbol table hash array size */
static void ITER_resize_symbol_table (ITER_Symbol_Table *table)
{
    ITER_Symbol **new_hash, *symbol, *hash_head;
    int new_hash_size;
    unsigned int new_hash_mask, new_hash_index;
    int i;

    /* Double the size of the hash array */
    new_hash_size = table->hash_size * 2;

    /* Allocate new hash array */
    new_hash = (ITER_Symbol **) malloc (new_hash_size * 
					sizeof (ITER_Symbol *));
    if (new_hash == NULL)
    {
	fprintf (stderr, "ITER_resize_symbol_table: Out of memory");
	exit (1);
    }

    /* Initialize new hash table */
    for (i=0; i < new_hash_size; i++)
	new_hash[i] = NULL;

    /* Get the hash mask for the new hash table */
    new_hash_mask = new_hash_size -1; /* AND mask, works only for power of 2 */
    
    /* Go though all the symbol and add to new hash table.
     * Can totally disreguard old hash links.
     */
    for (symbol = table->head_symbol; symbol != NULL; 
	 symbol = symbol->next_symbol)
    {
	/* Get index into hash table to use for this name */
	new_hash_index = symbol->iter & new_hash_mask;
	
	/* Add symbol to head of linked list */
	hash_head = new_hash[new_hash_index];
	symbol->next_hash = hash_head;
	symbol->prev_hash = NULL;
	if (hash_head != NULL)
	    hash_head->prev_hash = symbol;
	new_hash[new_hash_index] = symbol;
    }

    /* Free old hash table */
    free (table->hash);
   
    /* Initialize table fields for new hash table */
    table->hash = new_hash;
    table->hash_size = new_hash_size;
    table->hash_mask = new_hash_mask;
    /* Resize when count at 75% of new_hash_size */
    table->resize_size = new_hash_size - (new_hash_size >> 2); 
}

/* Adds structure to symbol table, data is not copied!!! 
 * Dynamically increases symbol table's hash array.
 * Returns pointer to added symbol.
 */
static ITER_Symbol *ITER_add_symbol (ITER_Symbol_Table *table, int iter, 
				     int weight)
{
    ITER_Symbol *symbol, *hash_head, *check_symbol, *tail_symbol;
    unsigned int hash_index;
    int symbol_count;

    /* Increase symbol table size if necessary before adding new symbol.  
     * This will change the hash_mask if the table is resized!
     */
    symbol_count = table->symbol_count;
    if (symbol_count >= table->resize_size)
    {
	ITER_resize_symbol_table (table);
    }

    /* Allocate a symbol (pool initialized in create table routine)*/
    symbol = (ITER_Symbol *) malloc (sizeof(ITER_Symbol));
    if (symbol == NULL)
    {
	fprintf (stderr, "ITER_add_symbol: Out of memory");
	exit (1);
    }
    
    /* Initialize fields */
    symbol->iter = iter;
    symbol->weight = weight;
    symbol->table = table;

    /* Get tail symbol for ease of use */
    tail_symbol = table->tail_symbol;

    /* Add to linked list of symbols */
    symbol->next_symbol = NULL;
    symbol->prev_symbol = tail_symbol;

    if (tail_symbol == NULL)
	table->head_symbol = symbol;
    else
	tail_symbol->next_symbol = symbol;
    table->tail_symbol = symbol;

    /* Get index into hash table to use for this iter */
    hash_index = iter & table->hash_mask;
    
    /* Get head symbol in current linked list for ease of use */
    hash_head = table->hash[hash_index];

    
    /* Sanity check (may want to ifdef out later).
     *
     * Check that this symbol's name is not already in the symbol table.
     * Punt if it is, since can cause a major debugging nightmare.
     */
    for (check_symbol = hash_head; check_symbol != NULL;
	 check_symbol = check_symbol->next_hash)
    {
	/* If iter the same, punt */
	if (check_symbol->iter == iter)
	{
	    fprintf (stderr, 
		     "ITER_add_symbol: cannot add '%i', already in table!",
		     iter);
	    exit (1);
	}
    }
    
    /* Add symbol to head of linked list */
    symbol->next_hash = hash_head;
    symbol->prev_hash = NULL;
    if (hash_head != NULL)
	hash_head->prev_hash = symbol;
    table->hash[hash_index] = symbol;

    /* Update table's symbol count */
    table->symbol_count = symbol_count + 1;

    /* Return symbol added */
    return (symbol);
}

/* Returns a ITER_Symbol structure with the desired value, or NULL
 * if the iter is not in the symbol table.
 */
static ITER_Symbol *ITER_find_symbol (ITER_Symbol_Table *table, int iter)
{
    ITER_Symbol *symbol;
    unsigned int hash_index;

    /* Get the index into the hash table */
    hash_index = iter & table->hash_mask;

    /* Search the linked list for matching name */
    for (symbol = table->hash[hash_index]; symbol != NULL; 
	 symbol = symbol->next_hash)
    {
	/* Compare iters to find match */
	if (symbol->iter == iter)
	{
	    return (symbol);
	}
    }
    return (NULL);
}

static int _PP_probe_count = 0;
static long int *_PP_probe_array = NULL;
/* 07/01/02 REK Adding a variable to hold the name of directory to which
 *              the dump file should be written. */
static char *_PP_dump_probe_directory = NULL;
static char *_PP_tag = NULL;

/* Called at beging of program execution to specify the location and
 * size of the probe array.
 */
/* 07/01/02 REK Adding a third argument, output_dir.  This is a pointer to 
 *              a string containing the name of the directory for the file.
 *              The directory name should not end with a slash. */
/*! \brief Initializes the probe array.
 *
 * \param probe_array
 *  the location of the probe array.
 * \param probe_count
 *  the size of the probe array.
 * \param output_dir
 *  the directory where profile output will be saved.
 * \param tag
 *  an extra tag that will be inserted in the profile output files.
 *  Can be null.
 *
 * This function is called at the beginning of program execution to specify
 * the location and size of the probe array.  This variant takes a tag
 * which is included in the profile output name.
 *
 * If \a tag is null, the profile output is saved in
 * <output_dir>/profile_dat.<pid>.  Otherwise, the profile output is saved in
 * <output_dir>/profile_dat.<tag>.<pid>.
 */
void _PP_dump_probe_init (long int *probe_array, int probe_count, 
			  char *output_dir, char *tag)
{
    _PP_probe_array = probe_array;
    _PP_probe_count = probe_count;
    /* 07/01/02 REK Handle the new third argument */
    _PP_dump_probe_directory = output_dir;
    if (tag)
      _PP_tag = tag;
}

/* Called at end of program execution to write control flow profile infomation
 * to a file. 
 */
/* 07/01/02 REK Modifying this function to build a filename with a directory
 *              so that the dump file is written to a given directory instead
 *              of the current one. */
void _PP_dump_probe_array()
{
   FILE *Fprofile;
   int i;
   /* 07/01/02 REK Increasing the size of this buffer from 256 to MAXPATHLEN
    *              to make room for the directory name. */
   char file_name[MAXPATHLEN];

   /* Use PID to create profile dat file name */
   /* 07/01/02 REK Changing this so that the directory name is added. */
   /* 12/08/04 REK Changing this to include the tag if specified. */
   if (_PP_tag == NULL)
   {
       snprintf (file_name, MAXPATHLEN, "%s/profile_dat.%d",
		 _PP_dump_probe_directory, getpid ());
   }
   else
   {
      snprintf (file_name, MAXPATHLEN, "%s/profile_dat.%s.%d",
		_PP_dump_probe_directory, _PP_tag, getpid ());
   }

   if ((Fprofile = fopen(file_name, "w")) == NULL)
   {
       fprintf(stderr, "Error, cannot open profile output file: %s\n",
	       file_name);
       exit(-1);
   }

   /* Dump out entire probe array */
   for (i = 0; i < _PP_probe_count; i++)
   {
       fprintf(Fprofile, "\t%f\n", (double)_PP_probe_array[i]);
   }

   /* Close file */
   fclose(Fprofile);
}

/* Loop iteration profiling support for Pcode profiling -JCG 4/99 */
static int _PP_loop_count = 0;  
static ITER_Symbol_Table **_PP_loop_iter_table = NULL;
/* 07/01/02 REK Adding a variable to hold the name of the directory to which
 *              the dump file should be written. */
static char *_PP_dump_loop_directory = NULL;

/* Called at beginning of program to indicate number of loops in program.
 * Creates appropritate data structures for tracking loop iteration
 * profiling data 
 */
/* 07/01/02 REK Adding a second argument, output_dir.  This is a pointer to
 *              a string containing the name of the directory for the file.
 *              The directory name should not end with a slash. */
void _PP_dump_loop_iter_init(int loop_no, char *output_dir)
{
    int table_size;
    int i;

    /* Record the number of loops in the program */
    _PP_loop_count = loop_no;

    /* 07/01/02 REK Handle the new second argument */
    _PP_dump_loop_directory = output_dir;

    /* Allocate loop iteration table */
    table_size = loop_no * sizeof (ITER_Symbol_Table *);
    _PP_loop_iter_table = (ITER_Symbol_Table **) malloc (table_size);
    if (_PP_loop_iter_table == NULL)
    {
	fprintf (stderr, "_PP_dump_loop_iter_init: Out of memory (size %i)\n",
		 table_size);
	exit (1);
    }

    /* Initialize all the pointers to NULL */
    for (i=0; i < loop_no; i++)
	_PP_loop_iter_table[i] = NULL;
}

/* Called at all potential loop exit points.  If 'iter' is 0, the loop
 * didn't actually iterate and the call should be ignored.
 */
void _PP_loop_iter_update(int loop_id, int iter)
{
    ITER_Symbol_Table *table;
    ITER_Symbol *iter_symbol;

    /* If iter == 0, ignore this call (loop didn't iterate) */
    if (iter == 0)
	return;

    /* Sanity check */
    if (loop_id >= _PP_loop_count)
    {
	fprintf (stderr, "_PP_loop_iter_update: loop %i out of bounds (%i)!\n",
		 loop_id, _PP_loop_count);
	abort();
    }

    /* Get loop iteration table */
    table = _PP_loop_iter_table[loop_id];

    /* If doesn't exist, create */
    if (table == NULL)
    {
	table = ITER_new_symbol_table();
	_PP_loop_iter_table[loop_id] = table;
    }

    /* Find symbol for this iteration count */
    iter_symbol = ITER_find_symbol (table, iter);

    /* If doesn't exist, create with weight 0 */
    if (iter_symbol == NULL)
    {
	iter_symbol = ITER_add_symbol (table, iter, 0);
    }

    /* Update weight */
    iter_symbol->weight++; 
}

/* Called at end of program execution.  Write out loop iteration profile
 * information in single profile run form.  Uses process id to create
 * output file name.
 */
/* 07/01/02 REK Modifying this function to build a filename with a directory
 *              so that the dump file is written to a given directory instead
 *              of the current one. */
void _PP_dump_loop_iter_table()
{
    FILE *Fiter;
    /* 07/01/02 REK Increasing the size of this buffer from 256 to MAXPATHLEN
     *              to make room for the directory name. */
    char file_name[MAXPATHLEN];
    ITER_Symbol_Table *table;
    ITER_Symbol *iter_symbol;
    int loop_id, num_iters;
    
    /* Use PID to create profile iter file name */
    /* 07/01/02 REK Changing this so that the directory name is added. */
    /* 12/08/04 REK Changing this to include the tag if specified. */
    if (_PP_tag == NULL)
    {
	
        snprintf (file_name, MAXPATHLEN, "%s/profile_iter.%d", 
		  _PP_dump_loop_directory, getpid());
    }
    else
    {
        snprintf (file_name, MAXPATHLEN, "%s/profile_iter.%s.%d",
		  _PP_dump_loop_directory, _PP_tag, getpid ());
    }
    if ((Fiter = fopen(file_name, "w")) == NULL)
    {
	fprintf(stderr, "Error, cannot open profile output file: %s\n",
		file_name);
	exit(-1);
    }

    /* Print out the number of loops and number of inputs profiled (always 1)
     */
    fprintf (Fiter, "%i %i\n", _PP_loop_count, 1);

    /* Go through each loop and print out iterations for each */
    for (loop_id=0; loop_id < _PP_loop_count; loop_id++)
    {
	/* Get iter table for this loop id */
	table = _PP_loop_iter_table[loop_id];

	/* Print out if exists */
	if (table != NULL)
	{
	    /* Print out loop id and number of entries for this loop */
	    fprintf (Fiter, "\n%i %i\n", loop_id, table->symbol_count);

	    for (iter_symbol = table->head_symbol; iter_symbol != NULL;
		 iter_symbol = iter_symbol->next_symbol)
	    {
		fprintf (Fiter, "  %i %i\n", iter_symbol->iter,
			 iter_symbol->weight);
	    }
	}

	/* Otherwise, print out loop id and no entries for this loop */
	else
	{
	    fprintf (Fiter, "\n%i %i\n", loop_id, 0);
	}
    }

    /* Close the iter profile file */
    fclose (Fiter);
}

/*
 * Indirect function call hash
 */

#define _PP_IP_TBL_SIZE 16

#if _PP_IP_TBL_SIZE % 2
#error PP_IP_TBL_SIZE must be divisible by 2.
#endif

typedef struct _PP_IP_Entry {
  void *addr;
  struct _PP_IP_Entry *next;
  long hits;
} PP_IP_Entry;

typedef struct _PP_IP_Table {
  PP_IP_Entry *tbl[_PP_IP_TBL_SIZE];
  long hits;
} PP_IP_Table;

int _PP_ip_count = 0;
char * _PP_ip_output = NULL;
PP_IP_Table *_PP_ip_table = NULL;

void 
_PP_init_ip_table (int ip_count, char *ip_output)
{
    /* Record the number of loops in the program */
    _PP_ip_count = ip_count;

    if (!ip_count)
      return;

    _PP_ip_output = ip_output;

    if (!(_PP_ip_table = calloc (ip_count, sizeof (PP_IP_Table))))
      {
	fprintf (stderr, "_PP_init_ip_table: out of memory");
	exit (-1);
      }

  return;
}

void *
_PP_record_ip (int ip_ctx, void *ip)
{
  int idx;
  void *ipc;
  PP_IP_Table *t;
  PP_IP_Entry *p, *q = NULL;

  ipc = (void *)*(long *)ip;

  t = _PP_ip_table + ip_ctx;
  t->hits++;

  idx = ((long)ipc>>3) % _PP_IP_TBL_SIZE;

  p = t->tbl[idx];

  while (p && (p->addr != ipc))
    {
      q = p;
      p = p->next;
    }

  if (p)
    {
      p->hits++;
    }
  else
    {
      if (!(p = malloc (sizeof (PP_IP_Entry))))
	{
	  fprintf (stderr, "_PP_record_ip: out of memory");
	  exit (-1);
	}

      p->addr = ipc;
      p->hits = 1;
      p->next = NULL;
      if (q)
	q->next = p;
      else
	t->tbl[idx] = p;
    }

  return ip;
}


void 
_PP_dump_ip (void)
{
  FILE *Fip;
  char file_name[MAXPATHLEN];
  PP_IP_Table *t;
  PP_IP_Entry *p, *q;
  int i, j;

  if (!_PP_ip_table)
    return;

  /* Use PID to create profile iter file name */
  /* 07/01/02 REK Changing this so that the directory name is added. */
  /* 12/08/04 REK Changing this to include the tag if specified. */
  if (_PP_tag == NULL)
    {
      snprintf (file_name, MAXPATHLEN, "%s/profile_ipc.%d", _PP_ip_output,
		getpid ());
    }
  else
    {
      snprintf (file_name, MAXPATHLEN, "%s/profile_ipc.%s.%d", _PP_ip_output,
		_PP_tag, getpid ());
    }
  if (!(Fip = fopen(file_name, "w")))
    {
      fprintf(stderr, "Error, cannot open profile output file: %s\n",
	      file_name);
      exit(-1);
    }

  /* Print out the number of IPs and number of inputs profiled (always 1)
   */
  fprintf (Fip, "%i %i\n", _PP_ip_count, 1);

  /* Go through each loop and print out iterations for each */
  for (i = 0; i < _PP_ip_count; i++)
    {
      /* Get iter table for this loop id */
      t = _PP_ip_table + i;

      /* Print out loop id and number of entries for this loop */
      fprintf (Fip, "\n%i %i\n", i, t->hits);

      if (!t->hits)
	continue;

      for (j = 0; j < _PP_IP_TBL_SIZE; j++)
	{
	  p = t->tbl[j];
	  while (p)
	    {
	      fprintf (Fip, "  0x%016lx %i\n", p->addr, p->hits);
	      q = p;
	      p = p->next;
	      free (q);
	    }
	}
    }

  free (_PP_ip_table);
  _PP_ip_table = NULL;

  fclose (Fip);
  return;
}
