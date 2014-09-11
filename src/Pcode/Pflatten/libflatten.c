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
/*! \file
 *
 * Defines the libpcode_flatten library initialization routine.
 */

#include <config.h>
#include <Pcode/pcode.h>
#include <Pcode/extension.h>
#include "flatten.h"
#include "data.h"

/*! \brief Initializes the flattening library.
 *
 * \param prog_name
 *  the module's name.
 * \param external_list
 *  the impact Parms.
 *
 * Any module using the flatten library must call this function in its
 * P_init_handlers() function.  
 */
void
PF_Init (char *prog_name, Parm_Macro_List *external_list)
{
  PF_Indices[ES_FUNC] = P_ExtSetupL (ES_FUNC, (AllocHandler)PF_alloc_func_data,
				     (FreeHandler)PF_free_func_data);
  PF_Indices[ES_EXPR] = P_ExtSetupL (ES_EXPR, (AllocHandler)PF_alloc_expr_data,
				     (FreeHandler)PF_free_expr_data);
  P_ExtRegisterCopyL (ES_EXPR, PF_Indices[ES_EXPR],
		      (CopyHandler)PF_copy_expr_data);
  PF_Indices[ES_STMT] = P_ExtSetupL (ES_STMT, (AllocHandler)PF_alloc_stmt_data,
				     (FreeHandler)PF_free_stmt_data);

  L_load_parameters (P_parm_file, external_list,
		     "(Pflatten", PF_read_parm_Pflatten);

  return;
}

/*! \brief Reads Pflatten specific parameters.
 *
 * \param ppi
 *  the parsed param list.
 */
void
PF_read_parm_Pflatten (Parm_Parse_Info *ppi)
{
  L_read_parm_b (ppi, "debug_flattening", &PF_debug);
  L_read_parm_b (ppi, "?flatten_reduce", &PF_reduce);
}
