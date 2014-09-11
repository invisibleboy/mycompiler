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
 *      File:   probe.c
 *      Author: Le-Chun Wu and Wen-mei Hwu
\*****************************************************************************/
/* 07/01/02 REK Changing PP_gen_init_c_code so that the current directory
 *              at build time is used as the target directory for the probe
 *              dump files. */

#include <config.h>
#include <stdlib.h>
#include <string.h>
/* 07/01/02 REK #including sys/param.h to get MAXPATHLEN */
#include <sys/param.h>
/* 07/12/02 REK including unistd.h */
#include <unistd.h>
#include "probe.h"
#include <Pcode/cfg.h>
#include "gen_ccode.h"

int PP_probe = 0;		/* Insert profiling probes     */
int PP_probe_ip = 0;		/*  |-Profile indirect calls   */
int PP_probe_lp = 0;		/*  +-Profile loop trip counts */

int PP_annotate = 0;
int PP_annotate_ip = 0;
int PP_annotate_lp = 0;

char *PP_output_tag = NULL;     /* Profile output filenames take the form
				 * profile_dat.[tag.]pid if output_tag is not
				 * NULL. */

FILE *Fprvprobe;
FILE *Fprvloopid;

char *probe_array_name = "_imp_probe_array";
int next_probe = 0;
int next_probe_BB;
int last_lp_id = 0;		/* Tahir - 04/02/04 */
int next_ipc_id = 0;		/* LCW - 3/24/99 */

/* LCW - some external functions for loop iter count profiling - 3/29/99 
 * JCG - updated after filled in routines functionality -4/99 */
extern int PP_get_loop_max_iter_id (int loop_id);
extern int PP_get_loop_max_input_id (void);
extern int PP_get_loop_iter_count (int loop_id, int iter_id);
extern double PP_get_loop_iter_avg_exec (int loop_id, int iter_id);
extern double PP_get_loop_iter_input_exec (int loop_id, int iter_id,
					   int input_id);

/* 07/01/02 REK Modifying this function so that the current directory is
 *              passed to _PP_dump_probe_init as the target directory for
 *              the dump files. */
void
PP_gen_init_c_code (int array_size, int loop_no, int ipc_no)
{
  /* 07/01/02 REK A buffer to hold the cwd. */
  char cwd[MAXPATHLEN];

  /* 07/01/02 REK Get the current directory. */
  if (!getcwd (cwd, MAXPATHLEN))
    I_punt ("PP_gen_init_c_code: Could not get current directory.");

  fprintf (Fdump_probe_code,
	   "#include <stdio.h>\n"
	   "#include <stdlib.h>\n"
	   "int _PP_is_initialized = 0;\n"
	   "long int %s[%d] = {0};\n"
	   "extern struct _PP_iter_table;\n"
	   "extern void _PP_dump_probe_init();\n"
	   "extern void _PP_dump_probe_array();\n"
	   "extern void _PP_dump_loop_iter_init();\n"
	   "extern void _PP_dump_loop_iter_table();\n"
	   "extern void _PP_init_ip_table (int, char *);\n"
	   "extern void _PP_record_ip (int, void *);\n"
	   "extern void _PP_dump_ip (void);\n"
	   "void _PP_initialize()\n"
	   "{\n"
	   "   if (_PP_is_initialized)\n"
	   "      return;\n"
	   "   _PP_is_initialized = 1;\n", probe_array_name, array_size);


  if (PP_probe)
    {
      /* 07/01/02 REK Adding the third argument to _PP_dump_probe_init.
       *              This will dump the file to the build directory. */
      if (PP_output_tag)
	{
	  fprintf (Fdump_probe_code,
		   "   _PP_dump_probe_init(%s, %d, \"%s\", \"%s\");\n",
		   probe_array_name, array_size, cwd, PP_output_tag);

	}
      else
	{	  
	  fprintf (Fdump_probe_code,
		   "   _PP_dump_probe_init(%s, %d, \"%s\", NULL);\n",
		   probe_array_name, array_size, cwd);
	}

      fprintf (Fdump_probe_code, "   atexit(_PP_dump_probe_array);\n");
    }

  if (PP_probe_lp)
    {
      /* 07/01/02 REK Adding the second argument to _PP_dump_loop_iter_init
       *              This will dump the file to the build directory. */
      fprintf (Fdump_probe_code,
	       "   _PP_dump_loop_iter_init(%d, \"%s\");\n", loop_no, cwd);
      fprintf (Fdump_probe_code, "   atexit(_PP_dump_loop_iter_table);\n");
    }

  if (PP_probe_ip)
    {
      fprintf (Fdump_probe_code,
	       "   _PP_init_ip_table(%d, \"%s\");\n", ipc_no, cwd);
      fprintf (Fdump_probe_code, "   atexit(_PP_dump_ip);\n");
    }

  fprintf (Fdump_probe_code, "}\n");
  return;
}



void
PP_annotate_loop_iter_count (Stmt loop_stmt)
{
  int loop_id;
  int max_iter_id, max_input_id;
  int iter_id, input_id;
  char pragmastr[4096], spec_str[256], tmp_str[256];

  if (loop_stmt->type != ST_SERLOOP)
    P_punt ("PP_annotate_loop_iter_count: SerloopHeader flag error");

  if (loop_stmt->ext == 0)
    P_punt ("PP_annotate_loop_iter_count: lack of loop id info");

  loop_id = *(int *) loop_stmt->ext;

  max_iter_id = PP_get_loop_max_iter_id (loop_id);
  max_input_id = PP_get_loop_max_input_id ();

  for (iter_id = 0; iter_id < max_iter_id; iter_id++)
    {
      int iter_count;
      double avg, freq;

      iter_count = PP_get_loop_iter_count (loop_id, iter_id);
      sprintf (spec_str, "\"iter_%d\"", iter_count);

      if (P_FindPragma (loop_stmt->pragma, spec_str))
	P_punt ("PP_annotate_loop_iter_count: duplicated iter count pragma");

      avg = PP_get_loop_iter_avg_exec (loop_id, iter_id);
      sprintf (pragmastr, "\"\\#%e", (double) avg);
      for (input_id = 0; input_id < max_input_id; input_id++)
	{
	  freq = PP_get_loop_iter_input_exec (loop_id, iter_id, input_id);
	  sprintf (tmp_str, "\\#%e", (double) freq);
	  strcat (pragmastr, tmp_str);
	}
      strcat (pragmastr, "\"");

      {
	Expr ex = P_NewStringExpr (pragmastr);
	Pragma prg = P_NewPragmaWithSpecExpr (spec_str, ex);
	loop_stmt->pragma = P_AppendPragmaNext (loop_stmt->pragma, prg);
      }
    }

  if (P_FindPragma (loop_stmt->pragma, "\"iteration_header\""))
    P_punt
      ("PP_annotate_loop_iter_count: duplicated iteration_header pragma");

  sprintf (pragmastr, "\"\\%%%d\\%%%d\"", max_iter_id, max_input_id);

  {
    Expr ex = P_NewStringExpr (pragmastr);
    Pragma prg = P_NewPragmaWithSpecExpr ("\"iteration_header\"", ex);
    loop_stmt->pragma = P_AppendPragmaNext (loop_stmt->pragma, prg);
  }
  return;
}


typedef struct PP_Iter
{
  int iter;			/* Number of iterations weights are for */
  double *weight;		/* Array of weights, indexed by input id */
}
PP_Iter;

typedef struct PP_Loop
{
  PP_Iter *entry;		/* Array of iter entries for this loop */
  int entry_count;		/* Number of iter entries for this loop */
}
PP_Loop;


static int PP_loop_iter_prof_init = 0;
static int PP_loop_count = 0;
static int PP_input_count = 0;
static PP_Loop *PP_loop = NULL;


/* Read in entry loop iteration profile file -JCG 4/99 */
void
PP_read_loop_iter_profile (void)
{
  FILE *in;
  int weight_array_size;
  int loop_id, entry_id, input_id;
  int file_loop_id, entry_count;
  PP_Iter *entry;
  int iter;
  int ch;
  double *weight_array;

  /* Sanity check, should not call twice */
  if (PP_loop_iter_prof_init)
    P_punt ("PP_read_loop_iter_profile: already read in!");

  if ((in = fopen ("profile.iter", "r")) == NULL)
    {
      /* REK */
      printf ("Plib_probe/probe.c:%d Could not open profile.iter\n",
	      __LINE__);
      P_punt ("PP_read_loop_iter_profile: could not open 'profile.iter' "
	      "for reading!");
    }

  /* Get loop and input count */
  if (fscanf (in, "%d %d\n\n", &PP_loop_count, &PP_input_count) != 2)
    P_punt ("PP_read_loop_iter_profile: error reading loop and input count!");

  /* Sanity check, input count better be positive */
  if (PP_input_count < 1)
    P_punt ("PP_read_loop_iter_profile: invalid input count (< 1)!");

  /* Calculate the size of each weight array (out of loop) */
  weight_array_size = PP_input_count * sizeof (double);

  /* Malloc the array to hold all the loop iteration info */
  if ((PP_loop = (PP_Loop *) malloc (PP_loop_count * sizeof (PP_Loop))) ==
      NULL)
    P_punt ("PP_read_loop_iter_profile: Out of memory");

  /* Read in each loop's info */
  for (loop_id = 0; loop_id < PP_loop_count; loop_id++)
    {
      if (fscanf (in, "%d %d\n", &file_loop_id, &entry_count) != 2)
	P_punt ("PP_read_loop_iter_profile: error reading loop id and "
		"entry count!");

      /* Sanity check, loop id's better match! */
      if (loop_id != file_loop_id)
	{
	  fprintf (stderr, "Expected loop id %i not %i!\n", loop_id,
		   file_loop_id);
	  P_punt ("PP_read_loop_iter_profile: loop id mismatch");
	}

      /* Sanity check, better be non-negative number */
      if (entry_count < 0)
	P_punt ("PP_read_loop_iter_profile: invalid negative entry count");

      /* Set loop entry count */
      PP_loop[loop_id].entry_count = entry_count;

      /* Don't malloc zero entry case */
      if (entry_count == 0)
	{
	  entry = NULL;
	}
      else
	{
	  if ((entry = (PP_Iter *) malloc (entry_count * sizeof (PP_Iter))) ==
	      NULL)
	    P_punt ("PP_read_loop_iter_profile: Out of memory");
	}
      PP_loop[loop_id].entry = entry;

      /* Read in each loop entry */
      for (entry_id = 0; entry_id < entry_count; entry_id++)
	{
	  /* Read in the iter count */
	  if (fscanf (in, "%d", &iter) != 1)
	    P_punt ("PP_read_loop_iter_profile: Error reading iter count!");

	  /* Set up entry */
	  entry[entry_id].iter = iter;

	  /* Malloc weight array */
	  if ((weight_array = (double *) malloc (weight_array_size)) == NULL)
	    P_punt ("PP_read_loop_iter_profile: Out of memory");
	  entry[entry_id].weight = weight_array;

	  /* Read in each input weight */
	  for (input_id = 0; input_id < PP_input_count; input_id++)
	    {
	      if (fscanf (in, "%lf", &weight_array[input_id]) != 1)
		P_punt ("PP_read_loop_iter_profile: Error reading weight!");
	    }
	}
    }

  /* Sanity check, better only have whitespace left in file */
  while ((ch = getc (in)) != EOF)
    {
      if (!isspace (ch))
	{
	  fprintf (stderr, "Unexpected char '%c' at expected EOF!\n", ch);
	  P_punt ("PP_read_loop_iter_profile: EOF expected!");
	}
    }

  fclose (in);

  /* Flag that we have read the loop iter profile */
  PP_loop_iter_prof_init = 1;
}

/* Routines to pass back loop iteration profile back to the routine
 * LCW wrote. -JCG 4/99 
 */
/* Returns the number of iteration entries for this loop id */
int
PP_get_loop_max_iter_id (int loop_id)
{
  /* Read in the loop iter profile info, if have not already */
  if (!PP_loop_iter_prof_init)
    PP_read_loop_iter_profile ();

  /* Sanity check, make sure loop_id valid */
  if ((loop_id < 0) || (loop_id >= PP_loop_count))
    P_punt ("PP_get_loop_max_iter_id: invalid loop id");

  return (PP_loop[loop_id].entry_count);
}

/* Returns the number of inputs contained in the loop iteration profile */
int
PP_get_loop_max_input_id (void)
{
  /* Read in the loop iter profile info, if have not already */
  if (!PP_loop_iter_prof_init)
    PP_read_loop_iter_profile ();

  return (PP_input_count);
}

/* Returns number of iterations for the specified entry (iter_id) */
int
PP_get_loop_iter_count (int loop_id, int iter_id)
{
  /* Sanity check, make sure loop_id valid */
  if ((loop_id < 0) || (loop_id >= PP_loop_count))
    P_punt ("PP_get_loop_iter_count: invalid loop id");

  /* Sanity check, make sure iter_id valid */
  if ((iter_id < 0) || (iter_id >= PP_loop[loop_id].entry_count))
    P_punt ("PP_get_loop_iter_count: invalid iter id");

  return (PP_loop[loop_id].entry[iter_id].iter);
}

/* Returns the average weight for this iteration over all inputs */
double
PP_get_loop_iter_avg_exec (int loop_id, int iter_id)
{
  int input_id;
  double sum, avg;

  /* Sanity check, make sure loop_id valid */
  if ((loop_id < 0) || (loop_id >= PP_loop_count))
    P_punt ("PP_get_loop_iter_avg_exec: invalid loop id");

  /* Sanity check, make sure iter_id valid */
  if ((iter_id < 0) || (iter_id >= PP_loop[loop_id].entry_count))
    P_punt ("PP_get_loop_iter_avg_exec: invalid iter id");

  /* Sum up the inputs */
  sum = 0.0;
  for (input_id = 0; input_id < PP_input_count; input_id++)
    sum += PP_loop[loop_id].entry[iter_id].weight[input_id];

  /* Make average */
  avg = sum / ((double) PP_input_count);

  return (avg);
}

/* Returns the weight for the specified input */
double
PP_get_loop_iter_input_exec (int loop_id, int iter_id, int input_id)
{
  /* Sanity check, make sure loop_id valid */
  if ((loop_id < 0) || (loop_id >= PP_loop_count))
    P_punt ("PP_get_loop_iter_input_exec: invalid loop id");

  /* Sanity check, make sure iter_id valid */
  if ((iter_id < 0) || (iter_id >= PP_loop[loop_id].entry_count))
    P_punt ("PP_get_loop_iter_input_exec: invalid iter id");

  /* Sanity check, make sure input_id valid */
  if ((input_id < 0) || (input_id >= PP_input_count))
    P_punt ("PP_get_loop_iter_input_exec: invalid input id");

  return (PP_loop[loop_id].entry[iter_id].weight[input_id]);
}


/* CWL - 01/03/01 modify for generating probed C code */
void
PP_C_insert_middle_probe (FILE * F, int probe_no)
{
  fprintf (F, "%s[%dLL]++", probe_array_name, probe_no);
}

/* CWL - 01/03/01 modify for generating probed C code */
void
PP_C_insert_probe (FILE * F, int probe_no)
{
  PP_C_insert_middle_probe (F, probe_no);
  fprintf (F, ";\n");
}

/* LCW - generate the code to initialize loop iter counters - 3/24/99 */
/* CWL - 01/03/01 modify for generating probed C code */
void
PP_C_gen_loop_iter_counter_initial (FILE * F, int last, int loops_no)
{
  int i;

  for (i = last; i < loops_no; i++)
    {
      Gen_CCODE_Indent (F, 1);
      fprintf (F, "_PP_LP_COUNTER_%d = 0;\n", i);
    }
}

/* CWL - 01/03/01 modify for generating probed C code */
void
PP_C_insert_loop_iter_counter (FILE * F, int lp_counter_no)
{
  Gen_CCODE_Indent (F, 1);
  fprintf (F, "_PP_LP_COUNTER_%d++;\n", lp_counter_no);
}


/* generate the Hcode which calls the function _PP_loop_iter_update and 
   then clear the counter (to 0) */
/* CWL - 01/03/01 modify for generating probed C code */
void
PP_C_gen_update_lp_iter_func (FILE * F, int loop_id)
{
  Gen_CCODE_Indent (F, 1);
  fprintf (F, "_PP_loop_iter_update( %d, _PP_LP_COUNTER_%d );\n",
	   loop_id, loop_id);

  Gen_CCODE_Indent (F, 1);
  fprintf (F, "_PP_LP_COUNTER_%d = 0;\n", loop_id);
}

void
PP_C_gen_update_lp_iter_func_in_middle (FILE * F, int loop_id)
{
  Gen_CCODE_Indent (F, 1);
  fprintf (F, "_PP_loop_iter_update( %d, _PP_LP_COUNTER_%d ),",
	   loop_id, loop_id);

  Gen_CCODE_Indent (F, 1);
  fprintf (F, "_PP_LP_COUNTER_%d = -1", loop_id);
}



/*! \brief Reads Pflatten specific parameters.
 *
 * \param ppi
 *  the parsed param list.
 */
void
PP_read_parm_Pprobe (Parm_Parse_Info * ppi)
{
  L_read_parm_b (ppi, "annotate_pcode", &PP_annotate);
  L_read_parm_b (ppi, "annotate_ipc", &PP_annotate_ip);
  L_read_parm_b (ppi, "annotate_loop", &PP_annotate_lp);
  L_read_parm_b (ppi, "insert_probe", &PP_probe);
  L_read_parm_b (ppi, "insert_ipc_probe", &PP_probe_ip);
  L_read_parm_b (ppi, "insert_loop_probe", &PP_probe_lp);
  L_read_parm_s (ppi, "output_tag", &PP_output_tag);
}
