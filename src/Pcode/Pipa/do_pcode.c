/*****************************************************************************\
 *
 *                    Illinois Open Source License
 *                     University of Illinois/NCSA
 *                         Open Source License
 *
 * Copyright (c) 2004, The University of Illinois at Urbana-Champaign.
 * All rights reserved.
 *
 * Developed by:
 *
 *              IMPACT Research Group
 *
 *              University of Illinois at Urbana-Champaign
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
 *      File:    do_pcode.c
 *      Author:  Erik Nystrom
 *      Copyright (c) 2003  Erik Nystrom, Wen-mei Hwu
 *              and The Board of Trustees of the University of Illinois.
 *              All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#include <Pcode/util.h>
#include <Pcode/symtab_i.h>
#include "pipa_common.h"
#include "pipa_program.h"
#include "pipa_callgraph.h"
#include "pipa_symbols.h"
#include "pipa_pcode2pointsto.h"
#include "pipa_driver.h"
#include "pipa_sync_gen.h"

IPA_prog_info_t *
IPA_setup(SymbolTable symbol_table)
{
  IPA_prog_info_t *info;
  
  info = IPA_prog_new ();

  info->cur_fninfo =
    IPA_funcsymbol_add (info,
			IPA_global_key,
			IPA_GLOBAL_VAR_NAME, 
			IPA_GLOBAL_VAR_NAME);
  info->cur_fninfo->is_globals = 1;
  info->symboltable = symbol_table;
  
  info->cur_fninfo->consg = IPA_cg_cgraph_new(info->cur_fninfo);

  IPA_POINTER_SIZE = 
    PST_GetTypeSize (symbol_table,
		     PST_FindPointerToType (symbol_table,
					    PST_FindBasicType(symbol_table,
							      BT_VOID)));
  
  IPA_POINTER_SIZE = 
    PST_GetTypeSize (symbol_table,
		     PST_FindPointerToType (symbol_table,
					    PST_FindBasicType(symbol_table,
							      BT_VOID)));
  
  
  return info;
}

void
IPA_process_global(IPA_prog_info_t *info,
		   VarDcl var)
{
  int var_id;
  IPA_symbol_info_t *sym;
  Key scope_key;
  buildcg_t *bcg;
  Pragma prag;

  scope_key = PST_GetFileScope(info->symboltable, var->key.file);

  /* Skip gvars already seen */
  sym = IPA_Pcode_Get_Sym (info, var->key);
  assert(sym);

  var_id = sym->id;

  /* HCH 5/7/04 */
  prag = P_NewPragmaWithSpecExpr("OBJID",P_NewIntExpr(sym->id));
  P_SetVarDclPragma (var,prag);

  /* A node should exist for evey GLOBAL */
  bcg = IPA_buildcg_start(info, info->globals, var_id, 1, B_BUILD);
  IPA_buildcg_free (bcg);

  DEBUG_IPA (2, printf ("Global VAR %s\n", var->name););
  IPA_BuildEqns_For_Var (info, var, scope_key);

  return;
}

void
IPA_annotate_localvars(IPA_prog_info_t *info,
		       SymbolTable symbol_table)
{
  int i;
  int last_file;
  int et_types;
  
  last_file = P_GetSymbolTableNumFiles (symbol_table);
  et_types = (ET_VAR_LOCAL);
  for (i = 1; i <= last_file; i++)
    {
      Key key;
      
      for (key = PST_GetFileEntryByType (symbol_table, i, et_types);
	   P_ValidKey (key);
	   key = PST_GetFileEntryByTypeNext (symbol_table, key, et_types))
	{
	  IPA_symbol_info_t * sym;
	  VarDcl v;
	  
	  v = PST_GetVarDclEntry (symbol_table, key);
	  
	  sym = IPA_symbol_find (info, key);
	  if (sym)
	    {
	      int var_id;
	      Pragma prag;

	      var_id = sym->id;
#if 0
	      fprintf (stderr, "HCH: FOUND LOC ID %i\n", var_id);
#endif
	      prag = P_NewPragmaWithSpecExpr("OBJID",P_NewIntExpr(var_id));
	      P_SetVarDclPragma (v,prag);
	    }
#if 0
	  else 
	    P_warn ("P_gen_code: didn't find symbol for ET_VAR_LOCAL\n");
#endif
	}
    }
}

void IPA_annotate_malloc_with_objid(IPA_prog_info_t *pinfo)
{
  IPA_callg_node_t *node;
  IPA_cgraph_node_t *cgraph_node;
  IPA_funcsymbol_info_t *ce;
  List_start(pinfo->call_graph->nodes);
  while ((node = List_next(pinfo->call_graph->nodes)))
    {
      IPA_callg_edge_t *edge;
      List_start(node->callee_edges);
      //printf("node: %s\n", node->fninfo->func_name);
      while ((edge = List_next(node->callee_edges)))
        {
          ce = edge->callee->fninfo;
          if (ce == NULL)
            continue;
          //printf(" func: %s\n", ce->func_name);
          if (strcmp(ce->func_name, "malloc") &&
              strcmp(ce->func_name, "calloc") &&
              strcmp(ce->func_name, "valloc"))
            continue;
          List_start(edge->sum_nodes);
          IPA_callsite_t *caller_cs = edge->caller_cs;
          Expr call_expr = caller_cs->call_expr;
          while ((cgraph_node = List_next(edge->sum_nodes)))
            {
              IPA_cg_node_print(stdout, cgraph_node, IPA_PRINT_ASCI);
              P_memdep_core_t dep = P_new_memdep_core();
              dep->id = cgraph_node->data.var_id;
              dep->version = cgraph_node->data.version;
              dep->offset = dep->size = -1;
              dep->is_def = 1;
              P_memdep_t md = P_GetMemDep(call_expr);
              assert(md);
              md->deps = List_insert_last(md->deps, dep);
            }
        }
    }
}

void
IPA_dump_nodemap(IPA_prog_info_t *info)
{
  FILE *file = NULL;
  file = fopen("NODEMAP","w");
  IPA_symbol_dump(file, info);
  fclose(file);
}

void
IPA_process_func(IPA_prog_info_t *info,
		 FuncDcl func)
{
  IPA_BuildEqns_For_Func (func, info);
}

void 
P_def_handlers (char *prog_name, Parm_Macro_List *external_list)
{
  PS_def_handlers ();
  PSS_def_handlers ();

  P_ExtSetupM (ES_EXPR, 
	       P_alloc_memdep,
	       P_free_memdep);
  P_ExtRegisterCopyM(ES_EXPR,
                     P_copy_memdep);
  P_ExtRegisterWriteM (ES_EXPR,
		       P_write_memdep, "DEP");
  P_ExtRegisterCopyM(ES_EXPR,
	               P_copy_memdep);
}

int
P_gen_code (char *prog_name, Parm_Macro_List *external_list,
            SymbolTable symbol_table, int file)
{
  IPA_prog_info_t *pinfo = NULL;
  int i;
  int last_file;
  int et_types;

/*   printf("[%s]\n",prog_name); */

  PSI_SetTable (symbol_table);

  last_file = P_GetSymbolTableNumFiles (symbol_table);

  /* Setup IPA process */
  L_load_parameters (P_parm_file, external_list, "(Pipa", IPA_read_parms);
  IPA_cgraph_minit ();
  IPA_callg_minit ();
  IPA_htab_minit ();
  pinfo = IPA_setup(symbol_table);
  
/*   printf("\n\n##################################################\n"); */
/*   printf("Process Pcode\n"); */
/*   printf("##################################################\n"); */

  pinfo->cur_fninfo = pinfo->globals;
  et_types = (ET_VAR_GLOBAL|ET_FUNC);
  for (i = 1; i <= last_file; i++)
    {
      Key key;
      SymTabEntry entry;
      
      for (key = PST_GetFileEntryByType (symbol_table, i, et_types);
           P_ValidKey (key);
           key = PST_GetFileEntryByTypeNext (symbol_table, key, et_types))
        {
          entry = PST_GetSymTabEntry (symbol_table, key);

          switch (P_GetSymTabEntryType (entry))
            {
	    case ET_VAR_GLOBAL:
	      {
		VarDcl var_dcl = P_GetSymTabEntryVarDcl (entry);
		IPA_Pcode_Add_Sym (pinfo,
				   P_GetVarDclName(var_dcl),
				   P_GetVarDclKey(var_dcl), 
				   P_GetVarDclType(var_dcl), 
				   IPA_VARKIND_GLOBAL);
	      }
	      break;
	    case ET_FUNC:
	      {
		FuncDcl func_dcl = P_GetSymTabEntryFuncDcl (entry);
		IPA_Pcode_Add_Sym (pinfo,
				   P_GetFuncDclName(func_dcl), 
				   P_GetFuncDclKey(func_dcl),
				   P_GetFuncDclType(func_dcl),
				   IPA_VARKIND_GLOBAL);
	      }
	      break;
	    default:
	      break;
	    }
	}
    }

  pinfo->cur_fninfo = NULL;
  et_types = (ET_FUNC | ET_VAR_GLOBAL);
  for (i = 1; i <= last_file; i++)
    {
      Key key;
      SymTabEntry entry;

      if (P_NameCheck (symbol_table->ip_table[i]->source_name, 
		       "__impact_lib"))
        {
	  pinfo->in_library = 1;
        }
      else if (P_NameCheck (symbol_table->ip_table[i]->source_name, 
                            "__impact_intrinsic"))
        {
	  pinfo->in_library = 1;
        }
      else
	{
	  pinfo->in_library = 0;
	}

      for (key = PST_GetFileEntryByType (symbol_table, i, et_types);
           P_ValidKey (key);
           key = PST_GetFileEntryByTypeNext (symbol_table, key, et_types))
        {
          entry = PST_GetSymTabEntry (symbol_table, key);

          switch (P_GetSymTabEntryType (entry))
            {
	    case ET_VAR_GLOBAL:
	      {
		VarDcl var_dcl = P_GetSymTabEntryVarDcl (entry);
		IPA_process_global(pinfo, var_dcl);
	      }
	      break;
	    case ET_FUNC:
	      {
		char *name, *filename;
		FuncDcl func_dcl = P_GetSymTabEntryFuncDcl (entry);
		filename = P_GetFuncDclFilename(func_dcl);
		name = P_GetFuncDclName(func_dcl);

		if (!name)
		  name = "(NULL)";

		if (!filename)
		  filename = "(NULL)";

		DEBUG_IPA (2, printf ("FUNC %s %s\n", filename, name););
		IPA_process_func(pinfo, func_dcl);
	      }
	      break;
	    default:
	      break;
	    }
	}
    }

  IPA_annotate_localvars(pinfo, symbol_table);

/*   printf("\n\n##################################################\n"); */
/*   printf("Interprocedural Analysis\n"); */
/*   printf("##################################################\n"); */

  IPA_dump_nodemap(pinfo);
  
  IPA_Interproc_PointsTo (pinfo);

  IPA_cgraph_pool_info();
  IPA_callg_pool_info();
  IPA_htab_pool_info();

  if (IPA_context_option == IPA_CONTEXT_SENSITIVE)
    IPA_annotate_malloc_with_objid(pinfo);

  if (IPA_sync_gen)
    {
      IPA_TrackTime(1);
/*       printf("\n\n##################################################\n"); */
/*       printf("SYNC GEN "); */
/*       printf("\n##################################################\n"); */

      sync_gen(pinfo);
    }
  IPA_TrackTime(1);

  IPA_cgraph_mfree ();
  IPA_callg_mfree ();
  IPA_htab_mfree ();

  fflush(stdout);
  fflush(stderr);
/*   printf("\n#### DONE ########################################\n"); */

  if (IPA_sync_gen_testonly)
    exit(0);

  return (0);
}
