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
 *	File:	do_pcode.c
 *	Author: Nancy Warter and Wen-mei Hwu
 *	Revised 2-96	Teresa Johnson
 *		Major Pcode restructuring
 *	Modified from code written by:	Po-hua Chang
 \****************************************************************************/

#include <config.h>
#include <Pcode/extension.h>
#include <Pcode/pcode.h>
#include <Pcode/query.h>
#include <Pcode/struct.h>
#include <Pcode/symtab.h>
#include <Pcode/symtab_i.h>
#include <Pcode/util.h>
#include <Pcode/dd_interface.h>

extern int DD_DEBUG_OMEGA;
extern int DD_PRINT_OMEGA;

void
P_def_handlers (char *prog_name, Parm_Macro_List * external_list)
{
  P_ExtSetupM (ES_EXPR, P_alloc_memdep, P_free_memdep);
  P_ExtRegisterWriteM (ES_EXPR, P_write_memdep, "DEP");
  P_ExtRegisterReadM (ES_EXPR, P_read_memdep, "DEP");
  P_ExtRegisterCopyM (ES_EXPR, P_copy_memdep);

  /*
   * set up the extension fields for dependence test
   */
  DD_SetUpExtension ();

  return;
}


extern void Pomg_process_func (FuncDcl fdcl);

int
P_gen_code (char *prog_name, Parm_Macro_List * external_list,
	    SymbolTable symbol_table, int file)
{
  int i, last_file;
  SymTabEntry entry;

  L_load_parameters (P_parm_file, external_list, "(Pomega", DD_ReadParameter);

  PSI_SetTable (symbol_table);

  last_file = P_GetSymbolTableNumFiles (symbol_table);
  for (i = 1; i <= last_file; i++)
    {
      Key key;

      if (symbol_table->ip_table[i]->file_type != FT_SOURCE)
	continue;

      if (P_NameCheck (symbol_table->ip_table[i]->source_name,
		       "__impact_lib"))
	{
	  P_warn ("SKIPPING __impact_lib.c");
	  continue;
	}

      if (P_NameCheck (symbol_table->ip_table[i]->source_name,
		       "__impact_intrinsic"))
	{
	  P_warn ("SKIPPING __impact_intrinsic.c");
	  continue;
	}

      for (key = PST_GetFileEntryByType (symbol_table, i, ET_FUNC);
	   P_ValidKey (key);
	   key = PST_GetFileEntryByTypeNext (symbol_table, key, ET_FUNC))
	{
	  entry = PST_GetSymTabEntry (symbol_table, key);

	  switch (P_GetSymTabEntryType (entry))
	    {
	    case ET_FUNC:
	      {
		FuncDcl func_dcl = P_GetSymTabEntryFuncDcl (entry);

		/* Skip functions for which there was no C code */
		if (func_dcl->stmt)
		  {
		    if (DD_DEBUG_OMEGA || DD_PRINT_OMEGA)
		      fprintf (stderr, "Pomega: processing %s()\n",
			       func_dcl->name);
		    DD_DependenceTest (func_dcl);
		  }
	      }
	      break;
	    default:
	      break;
	    }
	}
    }

  return (num_func);
}
