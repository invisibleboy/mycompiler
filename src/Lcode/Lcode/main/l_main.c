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
 *	File :		l_main.c
 *	Description :	Main program for LCODE environment.
 *	Author:	Po-hua Chang, Wen-mei Hwu
 *	Creation Date:	June 1990
\*****************************************************************************/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>

/* On HPUX, print out memory usage when module gets huge */
#ifndef __WIN32__
#ifdef _HPUX_SOURCE
#include <sys/param.h>
#include <sys/pstat.h>
#endif
#include <unistd.h>
#endif

#ifdef _HPUX_SOURCE
/* Prints out out memory usage to 'out' if total memory usage bigger than
 * min_size (in k).  Used for triming memory usage of IMPACT.
 */
void
print_mem_usage (FILE *out, double min_size)
{
   int page_size;
   int text_pages;
   int data_pages;
   int stack_pages;
   int pid;
   double text_size, data_size, stack_size, total_size;
   struct pst_status status;

   /* Get the page size on this machine */
   page_size = getpagesize ();

   /* Get this process's process id */
   pid = getpid();

   /* Get the process info for this process */
   if (pstat_getproc(&status, sizeof (status), 0, pid) == -1)
   {
       fprintf (out, "print_mem_usage: pstat_getproc failed for pid %i!\n",
                 pid);
        return;
   }

   /* Get the number of pages used */
   text_pages = status.pst_vtsize;
   data_pages = status.pst_vdsize;
   stack_pages = status.pst_vssize;

   /* Convert to kbytes used */
   text_size = ((double)text_pages) * ((double)page_size) / 1024.0;
   data_size = ((double)data_pages) * ((double)page_size) / 1024.0;
   stack_size = ((double)stack_pages) * ((double)page_size) / 1024.0;
   total_size = text_size + data_size + stack_size;

   /* If total bigger than min_size, print out stats */
   if (total_size >= min_size)
   {
       fprintf (out, "Total memory usage: %.0fK   (Data %.0fK  "
                "Stack %.0fK  Text %.0fK)\n", total_size, data_size,
                stack_size, text_size);
   }
}
#endif

/*
 *	LCODE main 
 */

int main(int argc, char **argv, char **envp)
{
    C_Arg arg[C_MAX_ARG];
    int n_arg, i, j;
    Parm_Macro_List *external_list;
    int read_i, read_o;

    //L_ERR = stderr;

#ifndef __WIN32__
    /*
     * Determine max data segment size
     */
    L_data_segment_start = sbrk(0);
#endif
    
    /* 
     * Initialize these string defaults using strdup since 
     * they are run-time configurable and may be malloced/freed
     * (They are no longer freed, statics may be used.)
     */
    L_arch = "impact";
    L_model = "v1.0";
    L_input_file = "stdin";
    L_output_file = "stdout";
    L_filelist_file = NULL;

    /* save the current Lcode pass name for error messages */
    L_curr_pass_name = argv[0];

    /*
     * Get macro definitions from command line and environment.
     * This is the updated version of command_line_macro_list,
     * any it may be used in the same way.
     */
    external_list = L_create_external_macro_list (argv, envp);
    L_command_line_macro_list = external_list;	/* SAM 10-94 */

    /* 
     * Get L_parm_file from command line (-p path), or environment
     * variable "STD_PARMS_FILE", or default to "./STD_PARMS"
     */
    L_parm_file = L_get_std_parm_name (argv, envp, "STD_PARMS_FILE",
				       "./STD_PARMS");

	
    /*
     * Load simulation parameters now, so that command line arguements
     * will override them if specified.  Pass in all command line
     * macro definitions.
     */
    /* Renamed 'global' to 'Lglobal' -JCG 5/26/98 */
    L_load_parameters_aliased (L_parm_file, external_list,  
		       "(Lglobal", "(global", L_read_parm_global);
    /* Renamed 'architecture' to 'Larchitecture' -JCG 5/26/98 */
    L_load_parameters_aliased (L_parm_file, external_list,  
		       "(Larchitecture", "(architecture", L_read_parm_arch);
    /* Renamed 'file' to 'Lfile' -JCG 5/26/98 */
    L_load_parameters_aliased (L_parm_file, external_list,  
    	 	       "(Lfile", "(file", L_read_parm_file);
    /* 
     * If the first argument does not contain an -, it is
     * ignored by Lcode.  Punt if this is the case.
     */
    if ((argv[1] != NULL) && (argv[1][0] != '-'))
    {
	L_punt ("Unknown command line option '%s'.", 
		argv[1]);
    }

    /* Mark that we have not read the input or output file yet */
    read_i = 0;
    read_o = 0;

    /* Process the rest of the command line arguments */
    n_arg = C_get_arg(argc-1, argv+1, arg, C_MAX_ARG);
    for (i=0; i < n_arg; i++)
    {
	char *option;
	option = arg[i].option;

	/* Get input file name */
	if (! strcmp(option, "-i")) 
	{
	    /* Punt if input file specified twice */
	    if (read_i)
		L_punt ("Parsing command line: -i specified twice.\n");

	    /* Make sure file name specified */
	    if (arg[i].count < 1)
		L_punt("Parsing command line: -i needs input_file_name.");
	    /* Make sure that only one file name specified */
	    if (arg[i].count > 1)
	    {
		fprintf (stderr, "Error parsing command line: -i");
		for (j = 0; j < arg[i].count; j++)
		    fprintf (stderr, " %s", arg[i].spec[j]);
		fprintf (stderr, "\n");
		L_punt ("Cannot specify more than one input file with -i.");
	    }

	   /*
	    L_input_file = C_findstr(arg[i].spec[0]);
	    */
	    L_input_file = strdup(arg[i].spec[0]);

	    /* Mark that we have read an input file name */
	    read_i = 1;
	}

	/* Get output file name */
	else if (! strcmp(option, "-o")) 
	{
	    /* Punt if output file specified twice */
	    if (read_o)
		L_punt ("Parsing command line: -o specified twice.\n");

	    /* Make sure file name specified */
	    if (arg[i].count < 1)
		L_punt("Parsing command line: -o needs output_file_name.");
	    /* Make sure only one file name specified */
	    if (arg[i].count > 1)
	    {
		fprintf (stderr, "Parsing command line: -o");
		for (j = 0; j < arg[i].count; j++)
		    fprintf (stderr, " %s", arg[i].spec[j]);
		fprintf (stderr, "\n");
		L_punt ("Cannot specify more than one output file with -o.");
	    }

	    /*
	    L_output_file = C_findstr(arg[i].spec[0]);
	    */
	    L_output_file = strdup(arg[i].spec[0]);

	    /* Mark that we have read an output file */
	    read_o = 1;
	} 

	/* Ignore -p */
	else if (strcmp (option, "-p") == 0)
	    ;

	/* Ignore -P, -F  (and for now -D), accept to make sure there
	 * are no argument after it.
	 */
	else if ((option[0] == '-') && 
		 ((option[1] == 'P') || (option[1] == 'D') || 
		  (option[1] == 'F')))
	{
	    /* Make sure there is nothing after the -Pmacro_name=val */
//            L_punt("param %s",arg[i].spec[0]);
	    if (arg[i].count > 0)
	    {
		L_punt ("Unknown command line option '%s'.", 
			arg[i].spec[0]);
	    }
	    
	}

	/* Otherwise, punt, unknown commmand line arguments */
	else
	{
	    /* Print out what we are punting on */
	    fprintf (stderr, "Error parsing command line: %s", option);
	    for (j = 0; j < arg[i].count; j++)
		fprintf (stderr, " %s", arg[i].spec[j]);
	    fprintf (stderr, "\n");

	    L_punt ("Unknown command line option.");
	}
    }

    /* 
     * Renice this process to L_nice_value.  
     * Ignore the error codes that will be returned if
     * already reniced or L_nice_value is invalid. 
     */
#ifndef __WIN32__
/* ADA 5/29/96: Win95/NT has diffenent way to change priority but IMPACT
   		module running on Win95/NT doesn't really care about it */
    setpriority(PRIO_PROCESS, 0, L_nice_value);
#endif
    /*
     *	Initialize all symbols
     */
    L_init_symbol();

    if (L_file_processing_model==L_FILE_MODEL_FILE)
       L_OUT = L_open_output_file(L_output_file);

    /*
     *	Create pools for Lcode data structs
     */
    L_setup_alloc_pools();

    /*
     *	SET UP THE ENVIRONMENT.
     */
    M_set_machine(L_arch, L_model, L_swarch);
    M_define_macros(L_macro_symbol_table);
    M_define_opcode_name(L_opcode_symbol_table);

    /*
     * Initialize intrinsic support. -ITI/JWJ 7/99
     */
    L_intrinsic_init();

    /*
     *	PROCESS INPUT.
     */
    L_gen_code(external_list);


    if (L_file_processing_model==L_FILE_MODEL_FILE)
       L_close_output_file(L_OUT);

    /*
     *	One last check of data structure allocation
     */
    if (L_check_data_struct_alloc) {
	L_check_alloc_for_func();
	L_check_alloc_for_data();
	/* REH 4/18/95 */
	L_check_alloc_for_region();
    }

    /* 
     * Print warning about unused command line parameter arguments if
     * we have actually manipulated an Lcode function.  This prevents
     * producing warning messages for files only containing data
     * segments.
     */
    if (L_func_read)
        L_warn_about_unused_macros (stderr, external_list);

    /* If on HPUX and module required more than 256 Megs, let user know */
#ifdef _HPUX_SOURCE
    print_mem_usage (stderr, (256.0 * 1024.0));
#endif
    return(0);
}
