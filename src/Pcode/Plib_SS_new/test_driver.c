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
 * 	Copyright (c) 1991 Nancy Warter, Po-hua Chang, Wen-mei Hwu
 *	       	and The Board of Trustees of the University of Illinois.
 *	       	All rights reserved.
 *	The University of Illinois software License Agreement
 * 	specifies the terms and conditions for redistribution.
\*****************************************************************************/

#include <config.h>
#include <Pcode/pcode.h>
#include <Pcode/symtab_i.h>
#include <Pcode/parms.h>
#include <Pcode/struct.h>
#include <Pcode/query.h>
#include <Pcode/write.h>
#include <Pcode/extension.h>
#include <Pcode/cfg.h>
#include "ss_ssa2.h"
#include "ss_induct2.h"
#include "ss_ind_expr.h"
#include <stdio.h>
#include <sys/fcntl.h>

void
P_def_handlers (char *prog_name, Parm_Macro_List * extern_list)
{
  PS_def_handlers ();
  PSS_def_handlers ();
  return;
}

int
P_gen_code (char *prog_name, Parm_Macro_List * external_list,
	    SymbolTable symbol_table, int file)
{
  FuncDcl func;
  PC_Graph cfg;
  Key key;
  int i;

  PSI_SetTable (symbol_table);

  /* clear out the SCC debug files */
  fclose(fopen(DEBUG_FILE, "w"));
  fclose(fopen(STAT_FILE, "w"));

  /*
   * Process the Files
   */
  for (i = 1; i <= PSI_GetNumFiles (); i++)
    {
      if (PSI_GetFileType (i) != FT_SOURCE)
	continue;

      for (key = PSI_GetFileEntryByType (i, ET_FUNC); P_ValidKey (key);
	   key = PSI_GetFileEntryByTypeNext (key, ET_FUNC))
	{
	  PSS_BaseTbl LocalVars;
	  func = PSI_GetFuncDclEntry (key);

	  cfg = PC_Function (func, 0, 0);
	  LocalVars = PSS_ComputeSSA (cfg);

#if 0
	  PC_PrintPcodeGraph (stderr, func->name, cfg);
#endif
	  /* detect induction variables */
	  PSS_Find_SCCs(cfg);

#if 0
	  /* SER test code */
	  PC_IndExpr_Setup (cfg);
#if 1
	  PSS_Find_IVs (cfg->lp_tree);
#endif
#if 1
	  PSS_Find_Loop_Bounds (cfg->lp_tree);
#endif
#if 1
	  PC_IndExpr_Cleanup (cfg);
#endif
#endif
#if 1
	  LocalVars = PSS_BaseTbl_Free (LocalVars);
#endif
#if 0
	  P_FindTopLoops(cfg);
#endif

	}
    }

#if 0
  P_PrintTopLoops();
#endif
  return (0);
}
