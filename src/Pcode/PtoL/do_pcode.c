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
/* 10/11/02 REK This module uses the following parameters.
 *              -Fdd_split_compound_expr_stmts=(yes|no)
 *              -Fdebug_sync_arcs=(yes|no)
 *              -Fdebug_yes=(yes|no)
 *              -Fdo_annotate_pcode=(yes|no)
 *              -Fdo_insert_probe=(yes|no)
 *              -Femit_data_type_info=(yes|no)
 *              -Femit_source_info=(yes|no)
 *              -Ffast_mode=(yes|no)
 *              -Fforce_dependence_analysis=(yes|no)
 *              -Fgenerate_abs_instructions=(yes|no)
 *              -Fgenerate_acc_name_attrs=(yes|no)
 *              -Fgenerate_alias_vars=<int>
 *              -Fgenerate_bit_field_operations=(yes|no)
 *              -Fgenerate_hashing_branches=(yes|no)
 *              -Fgenerate_label_attrs=(yes|no)
 *              -Fgenerate_static_branch_attrs=(yes|no)
 *              -Fgen_cast_operand=(yes|no)
 *              -Fhcode_func_prags=(yes|no)
 *              -Fhcode_loop_prags=(yes|no)
 *              -Fhcode_static_prof=(yes|no)
 *              -Fignore_hash_br_seq_weight=(yes|no)
 *              -Fignore_hash_profile_weight=(yes|no)
 *              -Finsert_intrinsics=(yes|no)
 *              -Fintrinsic_support_enabled=(yes|no)
 *              -Fmerge_interprocedural_data=<int>
 *              -Fmulti_alias_relation=<int>
 *              -Fpoints_to_representation=<int>
 *              -Fpropagate_sign_size_ctype_info=(yes|no)
 *              -Fremove_dead_function=(yes|no)
 *              -Fsubstitute_subroutine_call_for_operation=(yes|no)
 *              -Funsafe_type_promotion=(yes|no)
 *              -Fverbose_yes=(yes|no)
 *              -Fglobalize_lvars=(yes|no)
 *              -Fmark_glob_objids=(yes|no) 
 */

#include <config.h>

#include <sys/time.h>
#ifndef __WIN32__
#include <sys/resource.h>
#endif
#include <string.h>

#include "pl_main.h"
#include <library/l_parms.h>
#include <library/list.h>

#include <Pcode/util.h>
#include <Pcode/dd_interface.h>

SymbolTable PL_symtab;
FILE *L_DOUT;

int PL_native_int_reg_ctype = 0;
int PL_native_int_reg_mtype = 0;
int PL_native_int_size = 0;
int PL_native_int_align = 0;
int PL_native_order = M_LITTLE_ENDIAN;

int PL_normalize_loops = 1;
int PL_generate_hashing_branches = 1;
int PL_ignore_hash_profile_weight = 0;
int PL_ignore_hash_br_seq_weight = 0;
int PL_generate_abs_instructions = 0;
int PL_annotate_omega = 0;
int PL_generate_label_attrs = 0;
int PL_generate_static_branch_attrs = 0;
int PL_generate_acc_name_attrs = 0;
int PL_retain_sync_nums = 0;
int PL_debug_sync_arcs = 0;
int PL_gen_bit_field_operations = 0;
int PL_use_subroutine_call = 0;
int PL_initialize_function_live_ins = 1;
int PL_gen_acc_specs = 0;
/* LCW - emit source information - 8/5/97 */
int PL_emit_source_info = 0;
/* Allow emitting of subset of source info that Lemulate needs -ITI/JCG 4/99 */
int PL_emit_data_type_info = 0;

/* MCM/ITI - 1/00 */
int PL_insert_intrinsics = 0;

/* HCH 6/16/04: checking in MICRO '04 object globalization and id marking */ 
int PL_globalize_lvars = 0;
int PL_mark_glob_objids = 0;

int PL_gen_improved_bitfields = 1;

int PL_gen_compliant_struct_return = 0;

int PL_verbose = 0;

/* indexing expr extension for dependence distance/dir */
int expr_ext_deplist_idx = -1;
 
/* USELESS -- JWS */

int PL_generate_sign_extend_operations = 0;

/* END JWS */

/* annotate bitwidths in function attribute */
int PL_annotate_bitwidths = 0;

static void PL_print_data (void);

void
P_read_parm_ptol (Parm_Parse_Info * ppi)
{
  /* See if read parameter matches one of the defined parameters */

  L_read_parm_b (ppi, "?normalize_loops",
		 &PL_normalize_loops);
  L_read_parm_b (ppi, "generate_hashing_branches",
		 &PL_generate_hashing_branches);
  L_read_parm_b (ppi, "ignore_hash_profile_weight",
		 &PL_ignore_hash_profile_weight);
  L_read_parm_b (ppi, "ignore_hash_br_seq_weight",
		 &PL_ignore_hash_br_seq_weight);

  L_read_parm_b (ppi, "generate_abs_instructions",
		 &PL_generate_abs_instructions);

  L_read_parm_b (ppi, "annotate_omega", &PL_annotate_omega);
  L_read_parm_b (ppi, "debug_sync_arcs", &PL_debug_sync_arcs);

  L_read_parm_b (ppi, "generate_label_attrs", &PL_generate_label_attrs);
  L_read_parm_b (ppi, "generate_static_branch_attrs",
		 &PL_generate_static_branch_attrs);
  L_read_parm_b (ppi, "generate_acc_name_attrs", &PL_generate_acc_name_attrs);

  L_read_parm_b (ppi, "substitute_subroutine_call_for_operation",
		 &PL_use_subroutine_call);

  L_read_parm_b (ppi, "initialize_function_live_ins",
		 &PL_initialize_function_live_ins);

  L_read_parm_b (ppi, "emit_source_info", &PL_emit_source_info);
  L_read_parm_b (ppi, "emit_data_type_info", &PL_emit_data_type_info);

  L_read_parm_b (ppi, "insert_intrinsics", &PL_insert_intrinsics);

  L_read_parm_b (ppi, "generate_sign_extend_operations",
		 &PL_generate_sign_extend_operations);

  L_read_parm_b (ppi, "generate_bit_field_operations",
		 &PL_gen_bit_field_operations);

  L_read_parm_b (ppi, "gen_acc_specs", &PL_gen_acc_specs);

  L_read_parm_b (ppi, "globalize_lvars", &PL_globalize_lvars);
  L_read_parm_b (ppi, "mark_glob_objids", &PL_mark_glob_objids);

  L_read_parm_b (ppi, "?gen_improved_bitfields", &PL_gen_improved_bitfields);

  L_read_parm_b (ppi, "?gen_compliant_struct_return", 
		 &PL_gen_compliant_struct_return);

  L_read_parm_b (ppi, "verbose", &PL_verbose);

  L_read_parm_b (ppi, "annotate_bitwidths", &PL_annotate_bitwidths);
  
  return;
}


void 
P_def_handlers (char *prog_name, Parm_Macro_List *external_list)
{
  P_ExtSetupM (ES_EXPR, 
	       P_alloc_memdep,
	       P_free_memdep);
  P_ExtRegisterWriteM (ES_EXPR, P_write_memdep, "DEP");
  P_ExtRegisterReadM(ES_EXPR, P_read_memdep, "DEP");
  P_ExtRegisterCopyM(ES_EXPR, P_copy_memdep);
  P_ExtSetupM (ES_VAR, 
	       P_alloc_vartbl_entry,
	       P_free_vartbl_entry);

  /*
   * read dependence distance/dir (Omega test)
   */
  assert (expr_ext_deplist_idx == -1);
  expr_ext_deplist_idx = P_ExtSetupL (ES_EXPR, P_DepList_alloc, 
				      P_DepList_free);
  P_ExtRegisterReadL (ES_EXPR, expr_ext_deplist_idx, P_DepList_read, "DepL");

  return;
}

int
P_gen_code (char *prog_name, Parm_Macro_List *external_list,
            SymbolTable symbol_table, int file)
{
  int i;
  int last_file;
  int et_types;  

  L_curr_pass_name = prog_name;
  L_parm_file = P_parm_file;
  PL_symtab = symbol_table;

  L_load_parameters_aliased (L_parm_file, external_list,
			     "(Lfile", "(file", L_read_parm_file);
  L_load_parameters (P_parm_file, external_list,
		     "(PtoL", P_read_parm_ptol);

  L_load_parameters_aliased (L_parm_file, external_list,
			     "(Larchitecture", "(architecture",
			     L_read_parm_arch);

  L_load_parameters_aliased (L_parm_file, external_list,
			     "(Lfile", "(file", L_read_parm_file);

  L_load_parameters_aliased (L_parm_file, external_list,
			     "(Lglobal", "(global", L_read_parm_global);

  L_input_file = F_input;
  L_output_file = F_output;

  PLD ("Initializing");

  /* 
   * Initialize Lcode symbols
   */

  L_init_symbol ();

  /*
   *  Create pools for Lcode data structs
   */

  L_setup_alloc_pools ();

  /*
   * Initialize intrinsic support.
   * L_intrinsic_init internally predicated on
   *   the L_intrinsic support enabled flag.
   */

  L_intrinsic_init ();

  /*
   *  Set up the enviroment
   */

  M_set_machine (L_arch, L_model, L_swarch);
  M_define_macros (L_macro_symbol_table);
  M_define_opcode_name (L_opcode_symbol_table);

  PL_native_int_reg_ctype = M_native_int_register_ctype ();
  PL_native_int_reg_mtype = M_native_int_register_mtype ();
  PL_native_int_size = PL_MType_Size(PL_native_int_reg_mtype);
  PL_native_int_align = PL_MType_Align(PL_native_int_reg_mtype);
  PL_native_order = /*M_layout_order ()*/ M_LITTLE_ENDIAN;

  PL_InitTypeSizes ();

  if (PL_native_order != M_LITTLE_ENDIAN &&
      PL_native_order != M_BIG_ENDIAN)
    P_punt ("PtoL: Unknown layout ordering");

  /* Create/Init data */
  L_datalist = L_new_datalist ();
  L_hash_datalist = L_new_datalist ();
  L_string_datalist = L_new_datalist ();
  PL_init_strings ();
  L_fn = NULL;

  PSI_SetTable (symbol_table);
  PSI_SetFlags (STF_READ_ONLY);

  /*
   * Process the Files
   */

  PLD ("Processing structure definitions for __impact_data.lc");

  /* Handle all structure definitions */

  {
    Key key;

    L_OUT = NULL;

    et_types = (ET_TYPE | ET_STRUCT | ET_UNION);

    PLD ("- Ordering type uses");

    PST_OrderTypeUses (symbol_table);

    PLD ("- Extracting struct, union, and type definitions");

    /* Write struct and union definitions and typedefs. */

    for (key = PST_GetTableEntryByType (symbol_table, et_types);
	 P_ValidKey (key);
	 key = PST_GetTableEntryByTypeNext (symbol_table, key, et_types))
      {
	SymTabEntry entry = PST_GetSymTabEntry (symbol_table, key);
	_EntryType e;

	switch ((e = P_GetSymTabEntryType (entry)))
	  {
	  case ET_TYPE_LOCAL:
	  case ET_TYPE_GLOBAL:
	    break;
	  case ET_STRUCT:
	    {
	      StructDcl struct_dcl = P_GetSymTabEntryStructDcl (entry);
	      PL_gen_lcode_struct (L_datalist, struct_dcl);
	    }
	    break;
	  case ET_UNION:
	    {
	      UnionDcl union_dcl = P_GetSymTabEntryUnionDcl (entry);
	      PL_gen_lcode_union (L_datalist, union_dcl);
	    }
	    break;
	  default:
	    P_punt ("Invalid entry type %d", e);
	  }
      }

    PST_ResetOrder (symbol_table);

    PLD ("- Printing structure definitions");

    /* Dump all structure definitions */
    L_OUT = L_DOUT = fopen("__impact_data.lc", "w");
    assert(L_OUT);
    PL_print_data();
  }
  
  PLD ("Processing source files");

  last_file = P_GetSymbolTableNumFiles (symbol_table);

  et_types = (ET_FUNC | ET_VAR_GLOBAL);
  for (i = 1; i <= last_file; i++)
    {
      SymTabEntry entry;
      Key key;

      /* JWS 20040507: Shouldn't be necessary to process non-source files */
      if (symbol_table->ip_table[i]->file_type != FT_SOURCE)
	continue;

      /* JWS 20040507: hack for lib.c inclusion */
      /* REK 20040830: stripping directory name if specified. */
      if (P_NameCheck (symbol_table->ip_table[i]->source_name, "__impact_lib"))
	{
	  PLD ("- Skipping %s", symbol_table->ip_table[i]->source_name);
	  continue;
	}
      else if (P_NameCheck (symbol_table->ip_table[i]->source_name, 
			    "__impact_intrinsic"))
	{
	  PLD ("- Skipping %s", symbol_table->ip_table[i]->source_name);
	  continue;
	}

      {
	char *name = symbol_table->ip_table[i]->in_name;
	char *new_name;
	int len = strlen(name);

	if (L_OUT && L_OUT != L_DOUT)
	  fclose(L_OUT);

	PLD ("- Processing file %s", name);

	new_name = malloc(len + 10);
	strcpy(new_name, name);
	/* Write a null over the last . in in_name, then cat the .lc
	 * extension onto it. */
	strrchr (new_name, '.')[0] = '\0';
	strncat (new_name, ".lc", len);
	  
	L_OUT = fopen(new_name, "w");
	assert(L_OUT);
	free(new_name);
      }

      /* Set up for a new file: clear the string table and
       * invalidate any open section.
       */
      PL_forget_strings ();
      PL_invalidate_last_ms ();

      for (key = PST_GetFileEntryByType (symbol_table, i, et_types);
           P_ValidKey (key);
           key = PST_GetFileEntryByTypeNext (symbol_table, key, et_types))
        {
          entry = PST_GetSymTabEntry (symbol_table, key);

          switch (P_GetSymTabEntryType (entry))
            {
	    case ET_FUNC:
	      {
		FuncDcl func_dcl = P_GetSymTabEntryFuncDcl (entry);
		Key scope_key = P_GetScopeKey(PST_GetScope(PL_symtab, 
							   func_dcl->key));
		PL_file_scope = scope_key.file;
     
#if DEBUG_DepL
  		/* 
  		 * check DepList annotation
		 */
  		PrintFuncDepInfo (stderr, func_dcl, expr_ext_deplist_idx);
#endif

		/* Create and dump function and data definitions */

		if (func_dcl->stmt)
		  {
		    PLD ("  - Processing function %s()", func_dcl->name);
		    PL_gen_lcode_func (func_dcl);
		    PLD ("    Done");
		  }

		PL_print_data();
	      }
	      break;
	    case ET_VAR_GLOBAL:
	      {
		VarDcl var_dcl = P_GetSymTabEntryVarDcl (entry);

		/* Create and dump global data definitions */
		PL_gen_lcode_var (L_datalist, var_dcl);
		PL_print_data();
	      }
	      break;
	    default:
	      break;
	    }
	}
    }

  PLD ("Cleaning up");

  /* Free up data */
  L_delete_datalist (L_datalist);
  L_delete_datalist (L_hash_datalist);
  L_delete_datalist (L_string_datalist);
  L_datalist = L_hash_datalist = L_string_datalist = NULL;
  PL_deinit_strings ();

  PLD ("Done!");

  return (num_func);
}


static void
PL_print_data (void)
{
  L_print_datalist (L_OUT, L_datalist);
  L_delete_all_datalist_element (L_datalist);

  if (L_fn)
    {
      L_print_func (L_OUT, L_fn);
      L_delete_func (L_fn);
      L_fn = NULL;
    }

  L_print_datalist (L_OUT, L_hash_datalist);
  L_delete_all_datalist_element (L_hash_datalist);

  L_print_datalist (L_OUT, L_string_datalist);
  L_delete_all_datalist_element (L_string_datalist);

  return;
}


/* PL_debug (char *fmt, ...)
 * ----------------------------------------------------------------------
 * Print a formatted debugging message, with the current time, to
 * stderr Flush stderr.  
 */

void
PL_debug (char *fmt, ...)
{
  va_list args;
  time_t now;
  char timebuf[64];

  time (&now);
  strftime (timebuf, 64, "%Y%m%d %H:%M:%S", localtime (&now));
  fprintf (stderr, ">PLD %s> ", timebuf);
  va_start (args, fmt);
  vfprintf (stderr, fmt, args);
  va_end (args);
  fprintf (stderr, "\n");
  fflush (stderr);
  return;
}

