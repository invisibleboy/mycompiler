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

#include <config.h>

#include <sys/time.h>
#ifndef __WIN32__
#include <sys/resource.h>
#endif
#if 0
#include <machine/m_spec.h>

#include "impact_global.h"
#include "reduce.h"
#include "pcode.h"
#include "pcode_io.h"
#include "struct.h"
#include "parms.h"
#endif

#include "impact_global.h"
#include "pcode.h"
#include "parms.h"
#include "struct.h"
#include "symtab.h"
#include "struct_symtab.h"
#include "io_util.h"
#include "read.h"
#include "read_symtab.h"
#include "extension.h"
#include "write_symtab.h"

extern int P_gen_code (char *prog_name, Parm_Macro_List *external_list,
		       SymbolTable symbol_table, int file);
extern void P_show_help (char *prog_name, Parm_Macro_List *external_list);

int 
main (int argc, char **argv, char **envp)
{
  char            *prog_name = NULL;
  Parm_Macro_List *external_list = NULL;
  int i, exit_status;
#if 0
  int file_key = 0;
#endif
  SymbolTable symbol_table;
  int output_specified = 0;

  /* Program name
   */
  prog_name = argv[0];
  
  /* Setup Parameters 
   */
  external_list = L_create_external_macro_list (argv, envp);
  P_parm_file = L_get_std_parm_name (argv, envp, "STD_PARMS_FILE",
				     "./STD_PARMS");
  L_load_parameters (P_parm_file, external_list, "(Pcode", 
		     P_read_parm_Pcode);

#if 0
  if (resolve_machine_dependent_information)
    {
      L_load_parameters_aliased (P_parm_file, external_list,
				 "(Larchitecture", "(architecture",
				 P_read_parm_arch);

      M_set_machine (P_arch, P_model, P_swarch);
      P_GetIntegerSize ();  
    }
#endif

  /* Set up the default input/output. */
  P_init_io ();

  /* Initialize the extension handlers. */
  P_init_handlers (prog_name, external_list);

#if 0
  output_form = OUTPUT_NONE;
#endif

#if 0
  Init_NewLptr ();
  Init_NewLint ();
#endif
  
  if (argc == 1)
    {
      P_show_help (prog_name, external_list);
      exit (0);
    }

  /* Process Pcode Arguments
   * Supported: -i, -o, -e
   */
  for (i = 1; i < argc; i++)
    {
      if (argv[i][0] != '-')
	{
	  fprintf (Ferr, "Error, unknown Pcode option: %s\n", argv[i]);
	  P_show_help (prog_name, external_list);
	  exit (-1);
	}
      switch (argv[i][1])
	{
	case '\0':
	  fprintf (Ferr, "Error, missing option specifier: %s\n", argv[i]);
	  P_show_help (prog_name, external_list);
	  exit (-1);
	  break;
	case 'h':
	  P_show_help (prog_name, external_list);
	  exit (0);
	  break;
	case 'i':
	case 'o':
	case 'e':
	case 'p':
	  if (argv[i][2] != '\0')
	    {
	      fprintf (Ferr, "Error, unknown Pcode option: %s\n", argv[i]);
	      P_show_help (prog_name, external_list);
	      exit (-1);
	    }
	  if (i + 1 >= argc)
	    {
	      fprintf (Ferr, "Error, filename for option: %s\n", argv[i]);
	      P_show_help (prog_name, external_list);
	      exit (-1);
	    }
	  if (argv[i][1] == 'i')
	    {
	      F_input = argv[i + 1];
	    }
	  if (argv[i][1] == 'o')
	    {
	      F_output = argv[i + 1];
	      output_specified = 1;
	    }
	  if (argv[i][1] == 'e')
	    {
	      F_error = argv[i + 1];
	    }
	  i += 1;		/* move forward to next option */
	  break;
	case 'P':
	case 'F':
	  break;
	default:
	  fprintf (Ferr, "Error, unknown Pcode option: %s\n", argv[i]);
	  P_show_help (prog_name, external_list);
	  exit (-1);
	}
    }  
  

  /* Open error, input, output, and log files 
   */
  if (strcmp (F_error, "stderr") != 0)
    {
      Ferr = fopen (F_error, "w");
      if (Ferr == NULL)
	{
	  fprintf (Ferr, "Error, cannot open error file: %s\n", F_error);
	  exit (-1);
	}
    }
  
  if (!(symbol_table = PST_Open (F_input, F_output, 0)))
    P_punt ("cannot open input file %s", F_input);

  if ((verbose_yes || debug_yes) && strcmp (F_log, "stderr") != 0)
    {
      Flog = fopen (F_log, "w");
      if (Flog == NULL)
	{
	  fprintf (Ferr, "Error, cannot open log file: %s\n", F_log);
	  exit (-1);
	}
    }


#ifndef __WIN32__
  /* ADA 5/29/96: Win95/NT has diffenent way to change priority but IMPACT
     module running on Win95/NT doesn't really care about it */
  setpriority (PRIO_PROCESS, 0, pcode_nice_value);
#endif

  if (verbose_yes)
    {
      fprintf (Flog, ".. [%s]\n", prog_name);
      fprintf (Flog, ".. verbose mode is ON\n");
      fprintf (Flog, ".. debug mode is %s\n", (debug_yes) ? "ON" : "OFF");
      fprintf (Flog, ".. input file is %s\n", F_input);
      fprintf (Flog, ".. log file is %s\n", F_log);
      fprintf (Flog, ".. error file is %s\n", F_error);
#if 0
      if (line_yes)
	fprintf (Flog, ".. include source position in output\n");
      if (parallel_flag)
	fprintf (Flog, ".. generate pstmt warnings\n");
      if (static_array)
	fprintf (Flog, ".. make local arrays static\n");
      else
	fprintf (Flog, ".. leave local arrays automatic\n");
      if (do_dependence)
	fprintf (Flog, ".. forcing dependence analysis\n");
      if (trans_named_funcs_only)
	{
	  fprintf (Flog, ".. Functions to transform: ");
#if 0
	  for (i = 0; i < func_trans_count; i++)
	    fprintf (Flog, "%s ", funcs_to_trans[i]);
#endif
	  fprintf (Flog, "\n");
	}
      fprintf (Flog, ".. transformations are %s\n", transform_string);
#endif
      if (OPEN_STAT_PCODE)
	fprintf (Flog, ".. Pcode statistics file is %s\n", F_stat_pcode);
    }


  /* 
   * Call user program 
   ************************/
  exit_status = P_gen_code (prog_name, external_list, symbol_table,
			    P_GetSymbolTableModifiableFile (symbol_table));

  PST_Close (symbol_table);

  /* Warnings

   */
  L_warn_about_unused_macros (stderr, external_list);

  L_free_parm_macro_list (external_list);

  /* Clean up the extension handlers. */
  P_ExtCleanup ();

  return (exit_status);
}
