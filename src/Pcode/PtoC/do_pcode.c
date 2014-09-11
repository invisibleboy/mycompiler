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
#include <unistd.h>
#include <sys/param.h>
#include <Pcode/pcode.h>
#include <Pcode/symtab.h>
#include <Pcode/parms.h>
#include <Pcode/struct.h>
#include <Pcode/struct_symtab.h>
#include <Pcode/query.h>
#include <Pcode/write.h>
#include <Pcode/extension.h>
#include <Pcode/probe.h>
#include <Pcode/gen_ccode.h>
#include <Pcode/symtab_i.h>
#include <Pcode/util.h>

extern int total_lps;

static void
PP_dump_cnt (char *fname, int cnt)
{
  FILE *F;

  if (!(F = fopen (fname, "w")))
    P_punt ("PP_dump_cnt: Unable to write file %s", fname);

  fprintf (F, "%d", cnt);
  fclose (F);
  return;
}

/*! \brief A function to initialize extension field handlers.
 *
 * \param prog_name
 *  the module's name
 * \param extern_list
 *  the command line parameters.
 *
 * This function is called by the Pcode library main() function before
 * any Pcode is read.  The module sets up handlers in this function so
 * that any needed data structures are already attached to the Pcode
 * structures as they are read from disk.
 */
void
P_def_handlers (char *prog_name, Parm_Macro_List *extern_list)
{
#if 0
  P_ExtSetupM (ES_FUNC, (Extension (*)(void))PF_alloc_func_data,
	       (Extension (*)(Extension e))PF_free_func_data);
  P_ExtSetupM (ES_EXPR, (Extension (*)(void))PF_alloc_expr_data,
	       (Extension (*)(Extension e))PF_free_expr_data);
  P_ExtRegisterCopyM (ES_EXPR, (Extension (*)(Extension e))PF_copy_expr_data);
  P_ExtSetupM (ES_STMT, (Extension (*)(void))PF_alloc_stmt_data,
	       (Extension (*)(Extension e))PF_free_stmt_data);
#endif
  return;
}

/*! \brief The entry point for this module.
 *
 * \param prog_name
 *  the module's name.
 * \param external_list
 *  the command line parameters.
 * \param symbol_table
 *  the symbol table.
 * \param file
 *  the key of the file that the module may update.
 *
 * \return
 *  The exit code for the module.
 *
 * In the single file processing model, only modifications to file \a file
 * will be written to disk.  In the multiple file processing model, the
 * module may update any file.  In this case \a file will be 0.
 */
int
P_gen_code (char *prog_name, Parm_Macro_List *external_list,
	    SymbolTable symbol_table, int file)
{
  Key key;
  int i, num_files;
  FILE *Fhout, *Fcout;

  L_load_parameters (P_parm_file, external_list, "(Pprobe", 
		     PP_read_parm_Pprobe);

#if 0
  write_all_files_on_exit = FALSE;
  update_ip_table = FALSE;
#endif
  P_SetSymbolTableFlags (symbol_table, STF_READ_ONLY);

  PSI_SetTable (symbol_table);

  next_probe = 0;
#if 0
  next_loop_id = 0;
#endif
  next_ipc_id = 0;

  if (PP_probe)
    {
     if (!(Fallprobe = fopen ("impact_probe.status", "a")))
       P_punt ("Unable to open status file");

     if (!(Fdump_probe_code = fopen ("impact_dump_probe_rev.c", "w")))
       P_punt ("Unable to open probe dump source file");
    }

  PST_OrderTypeUses (symbol_table);

  num_files = P_GetSymbolTableNumFiles (symbol_table);
  for (i = 1; i <= num_files; i++)
    {
      IPSymTabEnt ipe = symbol_table->ip_table[i];
      char fname[MAXPATHLEN], *ptr;

      if (ipe->file_type != FT_SOURCE)
	continue;

      /* JWS 20040507: hack for lib.c inclusion */
      /* REK 20040830: stripping direcotry name if specified. */
      if (P_NameCheck (ipe->source_name, "__impact_lib"))
	{
	  P_warn ("SKIPPING %s", ipe->source_name);
	  continue;
	}

      if (P_NameCheck (ipe->source_name, "__impact_intrinsic"))
	{
	  P_warn ("SKIPPING __impact_intrinsic.c");
	  continue;
	}

      /* 8/19/04 REK Changing this to use in_name so the _rev.c is
       *             written to the same directory as the original
       *             Pcode file. */
      sprintf (fname, ipe->in_name);

      if (!(ptr = rindex (fname, '.')))
	ptr = fname + strlen (fname);
	
      strcpy (ptr, "_rev.c");

      fprintf (stderr, "> Producing %s\n", fname);

      Fcout = fopen (fname, "w");

      /* 8/19/04 REK #include __impact_pprof.h using its absolute path
       *             so that _rev.cs in other directories can find
       *             this file. */
      getcwd (fname, MAXPATHLEN);
      fprintf (Fcout, "#include \"%s/__impact_pprof.h\"\n", fname);  

      for (key = PST_GetFileEntryByType (symbol_table, i,
					 ET_VAR_GLOBAL | ET_FUNC);
	   P_ValidKey (key);
	   key = PST_GetFileEntryByTypeNext (symbol_table, key,
					     ET_VAR_GLOBAL | ET_FUNC))
	{
	  SymTabEntry entry;
	  _EntryType e;

	  entry = PST_GetSymTabEntry (symbol_table, key);

	  switch ((e = P_GetSymTabEntryType (entry)))
	    {
	    case ET_VAR_GLOBAL:
	      {
		VarDcl vardcl = P_GetSymTabEntryVarDcl (entry);
		_VarQual vq = P_GetVarDclQualifier (vardcl);
		Init init = vardcl->init;

		if ((vq & VQ_STATIC))
		  {
		    fprintf (Fcout, "/* %d:%d */ ", key.file, key.sym);
		    vardcl->init = NULL;
		    Gen_CCODE_GlobalVar (Fcout, vardcl);
		    vardcl->init = init;
		  }
	      }
	      break;
	    case ET_FUNC:
	      {
		FuncDcl f = P_GetSymTabEntryFuncDcl (entry);
		_VarQual vq = P_GetFuncDclQualifier (f);
		Stmt body;

		/* Write a function prototype for every function besides
		 * main().
		 * Set the body of the function to null so that we only
		 * write a prototype. */
		if ((vq & VQ_STATIC) &&
		    strcmp (P_GetFuncDclName (f), "main"))
		  {
		    fprintf (Fcout, "/* %d:%d */ ", key.file, key.sym);
		    body = P_GetFuncDclStmt (f);
		    P_SetFuncDclStmt (f, NULL);
		    Gen_CCODE_Func (Fcout, f);
		    P_SetFuncDclStmt (f, body);
		  }
	      }
	      break;
	    default:
	      P_punt ("Invalid entry type %d", e);
	    }
	}

      for (key = PST_GetFileEntryByType (symbol_table, i, ET_ANY);
	   P_ValidKey (key);
	   key = PST_GetFileEntryByTypeNext (symbol_table, key, ET_ANY))
	{
	  SymTabEntry entry = PST_GetSymTabEntry (symbol_table, key);
	  _EntryType e;

	  switch ((e = P_GetSymTabEntryType (entry)))
	    {
	    case ET_FUNC:
	      {
		FuncDcl f = P_GetSymTabEntryFuncDcl (entry);

		/* We already wrote function prototypes, so if this function
		 * has no body, there's nothing to write this time. */
		if (P_GetFuncDclStmt (f))
		  {
		    fprintf (Fcout, "/* %d:%d */ ", key.file, key.sym);
		    Gen_CCODE_Func (Fcout, f);
		  }
	      }
	      break;
	    case ET_ASM:
	      Gen_CCODE_Asm (Fcout, P_GetSymTabEntryAsmDcl (entry));
	      break;
	    case ET_VAR_GLOBAL:
	      Gen_CCODE_GlobalVar (Fcout, P_GetSymTabEntryVarDcl (entry));
	      break;
	    case ET_TYPE_LOCAL:
	    case ET_TYPE_GLOBAL:
	    case ET_VAR_LOCAL:
	    case ET_STRUCT:
	    case ET_UNION:
	    case ET_ENUM:
	    case ET_ENUMFIELD:
	    case ET_STMT:
	    case ET_EXPR:
	    case ET_FIELD:
	    case ET_LABEL:
	    case ET_SCOPE:
	      break;
	    default:
	      P_punt ("Unsupported entry type %d\n", e);
	    }
	}

      fflush (Fcout);
      fclose (Fcout);
    }

  fprintf (stderr, "> Producing %s\n", "__impact_pprof.h");

  if (!(Fhout = fopen ("__impact_pprof.h", "w")))
    P_punt ("Unable to open header\n");

  /* Write struct and union definitions and typedefs. */

  for (key = PST_GetTableEntryByType (symbol_table,
				      ET_TYPE | ET_STRUCT | ET_UNION);
       P_ValidKey (key);
       key = PST_GetTableEntryByTypeNext (symbol_table, key,
					  ET_TYPE | ET_STRUCT | ET_UNION))
    {
      SymTabEntry entry = PST_GetSymTabEntry (symbol_table, key);
      _EntryType e;

      switch ((e = P_GetSymTabEntryType (entry)))
	{
	case ET_TYPE_LOCAL:
	case ET_TYPE_GLOBAL:
	  Gen_CCODE_Typedef (Fhout, P_GetSymTabEntryTypeDcl (entry));
	  break;
	case ET_STRUCT:
	  Gen_CCODE_Struct (Fhout, P_GetSymTabEntryStructDcl (entry));
	  break;
	case ET_UNION:
	  Gen_CCODE_Union (Fhout, P_GetSymTabEntryUnionDcl (entry));
	  break;
	default:
	  P_punt ("Invalid entry type %d", e);
	}
    }

  PST_ResetOrder (symbol_table);

  /* Write global variable and function declarations. */

  for (key = PST_GetTableEntryByType (symbol_table,
				      ET_VAR_GLOBAL | ET_FUNC);
       P_ValidKey (key);
       key = PST_GetTableEntryByTypeNext (symbol_table, key,
					  ET_VAR_GLOBAL | ET_FUNC))
    {
      SymTabEntry entry = PST_GetSymTabEntry (symbol_table, key);
      _EntryType e;

      switch ((e = P_GetSymTabEntryType (entry)))
	{
	case ET_VAR_GLOBAL:
	  {
	    VarDcl vardcl = P_GetSymTabEntryVarDcl (entry);
	    _VarQual vq = P_GetVarDclQualifier (vardcl);
	    Init init = vardcl->init;

	    if (!(vq & VQ_STATIC))
	      {
		fprintf (Fhout, "/* %d:%d */ ", key.file, key.sym);
		if (!(vq & VQ_EXTERN))
		  fprintf (Fhout, "extern ");
		vardcl->init = NULL;
		Gen_CCODE_GlobalVar (Fhout, vardcl);
		vardcl->init = init;
	      }
	  }
	  break;
	case ET_FUNC:
	  {
	    FuncDcl f = P_GetSymTabEntryFuncDcl (entry);
	    _VarQual vq = P_GetFuncDclQualifier (f);
	    Stmt body;

	    /* Write a function prototype for every function besides
	     * main().
	     * Set the body of the function to null so that we only
	     * write a prototype. */
	    if (!(vq & VQ_STATIC) &&
		strcmp (P_GetFuncDclName (f), "main") &&
		strncmp (P_GetFuncDclName (f), "__builtin_", 10))
	      {
		fprintf (Fhout, "/* %d:%d */ ", key.file, key.sym);
		if (!(vq & VQ_EXTERN))
		  fprintf (Fhout, "extern ");
		body = P_GetFuncDclStmt (f);
		P_SetFuncDclStmt (f, NULL);
		Gen_CCODE_Func (Fhout, f);
		P_SetFuncDclStmt (f, body);
	      }
	  }
	  break;
	default:
	  P_punt ("Invalid entry type %d", e);
	}
    }

  fclose (Fhout);

  if (PP_probe)
    {
#if 1 
      PP_gen_init_c_code (next_probe, total_lps, next_ipc_id);     
#else
      PP_gen_init_c_code (next_probe, next_loop_id, next_ipc_id);
#endif
      fclose (Fallprobe);
      fclose (Fdump_probe_code);

      /* Dump final values for subsequent data collection scripts */

      PP_dump_cnt ("impact_probe.tmp", next_probe);

#if 1
      if (PP_probe_lp)
	PP_dump_cnt ("impact_loop_id.tmp", total_lps);
#else
      if (PP_probe_lp)
	PP_dump_cnt ("impact_loop_id.tmp", next_loop_id);
#endif
      if (PP_probe_ip)
	PP_dump_cnt ("impact_ipc_id.tmp", next_ipc_id);
    }

  return (0);
}

