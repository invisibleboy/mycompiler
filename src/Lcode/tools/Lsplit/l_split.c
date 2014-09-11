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
 *	File :		l_codegen.c
 *	Description :	pseudo main routine - all Lcode modules should have
 *			a set of functions with similar functionality
 *	Author:	Po-hua Chang
 *	Creation Date:	June 1990
 *
 *      Modification : Daniel Connors
 *                     Added the functionality of printing all data to a
 *                     separate data file.
 *      
 *           20000108  John W. Sias
 *                     Changed handling of global data items to fix loss of
 *                     declarator information.
 *
 * 	Copyright (c) 1991 Po-hua Chang, Wen-mei Hwu. 
 *	All rights reserved.  
 * 	All rights granted to the University of Illinois.
 *	The University of Illinois software License Agreement
 * 	specifies the terms and conditions for redistribution.
\*****************************************************************************/

#include <config.h>
#include <Lcode/l_main.h>

/* Parameters local to Lcode executable */
int Lcode_echo = 0;
int Lcode_print_stat = 0;
int Lcode_unit_time = 0;
int Lcode_run_df_live_variable = 0;
int Lcode_run_loop_detection = 0;

int Lfile_index = 0;
char *Lfile_ext = "lc";

static FILE *fn_fp = NULL;
static FILE *data_fp = NULL;

L_Data *glob_data = NULL;

static void process_input()
{
    char buf[32];
    
    switch (L_token_type) 
      {

      case L_INPUT_MS:
	switch (L_data->N) 
	  {
	  case L_MS_DATA:
	  case L_MS_DATA1:
	  case L_MS_DATA2:
	  case L_MS_SDATA:
	  case L_MS_SDATA1:
	  case L_MS_RODATA:
	  case L_MS_BSS:
	  case L_MS_SBSS:
	    L_print_data(data_fp, L_data);
	    break;
	  case L_MS_TEXT:
	    break;
	  default:
	    L_punt("process_input: illegal input after (ms )");
	  }
	if(glob_data)
	  L_delete_data(glob_data);
	glob_data = NULL;
	L_delete_data(L_data);
        break;

      case L_INPUT_GLOBAL:
	if(glob_data)
	  L_delete_data(glob_data);
	glob_data = L_data;
	break;

      case L_INPUT_EOF:
      case L_INPUT_WB:
      case L_INPUT_WW:
      case L_INPUT_WI:
      case L_INPUT_WQ:
      case L_INPUT_WF:
      case L_INPUT_WF2:
      case L_INPUT_WS:
      case L_INPUT_ELEMENT_SIZE:
      case L_INPUT_DEF_STRUCT:  /* Folded in SAM fix.  -JCG 5/99 */
      case L_INPUT_DEF_UNION:
      case L_INPUT_DEF_ENUM:
      case L_INPUT_FIELD:
      case L_INPUT_ENUMERATOR:

	if (glob_data) 
	  {
	    L_print_data(data_fp, glob_data);
	    L_delete_data(glob_data);
	    glob_data = NULL;
	  }

	L_print_data(data_fp,L_data);
	L_delete_data(L_data);
	break;

      case L_INPUT_VOID:
      case L_INPUT_BYTE:
      case L_INPUT_WORD:
      case L_INPUT_LONG:
      case L_INPUT_LONGLONG:
      case L_INPUT_FLOAT:
      case L_INPUT_DOUBLE:
      case L_INPUT_ALIGN:
      case L_INPUT_ASCII:
      case L_INPUT_ASCIZ:
      case L_INPUT_RESERVE:
    
	if ( !glob_data && L_data->address )
	  fprintf(data_fp,"(global %s)\n",L_data->address->value.l);

	if (glob_data) 
	  {
	    L_print_data(data_fp, glob_data);
	    L_delete_data(glob_data);
	    glob_data = NULL;
	  }
	
        L_print_data(data_fp, L_data);
	L_delete_data(L_data);
        break;

      case L_INPUT_FUNCTION:
	sprintf(buf,"f_%d.%s",Lfile_index++,Lfile_ext);
        fn_fp = fopen(buf,"w");
	if ( fn_fp == NULL )
	  L_punt("process_input: unable to open %s",buf);

	fprintf(fn_fp, "(ms text)\n");	
	if (glob_data) 
	  {
	    L_print_data(fn_fp, glob_data);
	    L_delete_data(glob_data);
	    glob_data = NULL;
	  }
	else 
	  {
	    fprintf(fn_fp,"(global %s)\n",L_fn->name);
	  }

        L_print_func(fn_fp, L_fn);
	L_delete_func(L_fn);
        fclose(fn_fp);
        break;
      default:
        L_punt("process_input: illegal token");
      }
}

void L_gen_code(command_line_macro_list) 
Parm_Macro_List *command_line_macro_list;
{
    char buf[32];

    /* Transfer global variables to local ones */
    Lfile_ext = L_output_file_extension;
    Lfile_index = L_file_index;

    /* Prevent files from receiving pretty impact header */
    L_output_generation_info = 0;

    /* Initialize the output file names and functions pointers */
    glob_data = NULL;
    fn_fp = NULL;
    glob_data = NULL;

    /* Create the data file */
    sprintf(buf,"data.%s",Lfile_ext);
    data_fp = fopen(buf,"w");
    if ( data_fp == NULL )
        L_punt("L_gen_code: unable to open %s\n",buf);
   
    /* Open the input file */
    L_open_input_file(L_input_file);
    while (L_get_input() != L_INPUT_EOF) {
        process_input();
    }

    /* Close the data file */
    fclose(data_fp);   
 
    L_close_input_file(L_input_file);

}
