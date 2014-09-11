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
 *      File:   inline.c
 *      Author: Ben-Chung Cheng
 *      Copyright (c) 1995 Ben-Chung Cheng, Wen-mei Hwu. All rights reserved.
 *      All rights granted to the University of Illinois.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/
/* 07/10/02 REK Adding code to write the absolute path to the extern.pch file
 *              in the event that we write an empty .pci file.
 */
/* 10/11/02 REK This module takes the following parameters.
 *              -Fexclude_small_from_ratio_limit=(yes|no)
 *              -Ffavor_small_functions=(yes|no)
 *              -Fforce_dependence_analysis=(yes|no)
 *              -Fil_dir=<dir>
 *              -Fil_log_name=<log>
 *              -Finline_function_pointers=(yes|no)
 *              -Finline_self_recursion=(yes|no)
 *              -Fmax_expansion_ratio=<float>
 *              -Fmax_function_size=<int>
 *              -Fmax_sf_size_limit=<int>
 *              -Fmin_expansion_key=<float>
 *              -Fmin_expansion_weight=<float>
 *              -Fprevent_cross_file_inlining=(yes|no)
 *              -Fprevent_inline_functions=
 *              -Fprint_inline_stats=(yes|no)
 *              -Fregroup=(yes|no)
 *              -Fregroup_only=(yes|no)
 *              -Fsize_only=(yes|no)
 *              -Fsmall_function_thresh=<int>
 *              -Fsp_output_spec=<spec>
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
/* 07/10/02 REK Including sys/param.h to get MAXPATHLEN */
#include <sys/param.h>
#include <library/l_parms.h>
#include <library/dynamic_symbol.h>
#include <machine/m_spec.h>
#include <Pcode/pcode.h>
#include <Pcode/query.h>
#include <Pcode/cast.h>
#include <Pcode/symtab.h>
#include <Pcode/struct_symtab.h>
#include <Pcode/symtab_i.h>
#include <Pcode/extension.h>
#include <Pcode/util.h>
#include "pin_inline.h"

double TotalBodySize;
double TotalEBodySize;
double TouchedBodySize;
double TotalStackSize;
double TotalCallWeight;
double Pin_budget;
double Pin_ebody_floor = 8000.0;

/* PARAMETERS */

/* static char *sp_output_spec; */
/* static char *il_log_name; */
/* static char *il_output_dir_string; */
/* Make it compile with gcc4 -KF 10/2005 */
char *sp_output_spec;
char *il_log_name;
char *il_output_dir_string;
static char *Pin_inline_key_cost_str;
int Pin_inline_key_cost = PIN_KEY_SQRT_SIZE;
int inline_inlined_body = 1;
int Pin_adjust_func_weight = 1;

int max_function_size;
int max_sf_size_limit;
double max_expansion_ratio;
double min_expansion_weight;
double min_expansion_key;

double Pin_min_arc_ratio = 0.01;

int inline_function_pointers = 0;
int inline_indir_by_profile = 0;
double indir_thresh = 0.75;
int Pin_no_inlining = 0;
double assumed_body_size = 1.0;
int size_only = 0;
int inline_self_recursion = 1;
int prevent_cross_file_inlining = 0;

char *Pin_body_size_metric_str;
int Pin_body_size_metric = PIN_BODY_TOUCH;

int favor_small_functions = 1;
int small_function_thresh = 3;
int exclude_small_from_ratio_limit = 0;

char *prevent_inline_functions = NULL;
Func_Name_List *prevent_inline_list = NULL;

int inline_all_functions = 0;

/* DEBUG / STATISTICS PARAMETERS */

int Pin_trace_heap = 0;
int print_inline_trace = 0;
int print_inline_stats = 0;
int print_inline_graphs = 0;

/* INTERNAL STATE */

Heap *Pin_arc_heap;

static STRING_Symbol_Table *always_inline_tbl;

FILE *Flog, *Fstat;

int
Pin_reduce_budget (double sub)
{
  Pin_budget -= sub;
  if (Pin_budget < 0.0)
    {
      Pin_budget = 0.0;
      return 1;
    }
  else
    {
      return 0;
    }
}


static void
P_read_parm_Pinline (Parm_Parse_Info * ppi)
{
  L_read_parm_b (ppi, "inline_all_functions", &inline_all_functions);
  L_read_parm_b (ppi, "?print_heap_trace", &Pin_trace_heap);
  L_read_parm_b (ppi, "print_inline_trace", &print_inline_trace);
  L_read_parm_b (ppi, "print_inline_stats", &print_inline_stats);
  L_read_parm_b (ppi, "?print_inline_graphs", &print_inline_graphs);
  L_read_parm_s (ppi, "il_log_name", &il_log_name);
  L_read_parm_i (ppi, "max_sf_size_limit", &max_sf_size_limit);
  L_read_parm_lf (ppi, "max_expansion_ratio", &max_expansion_ratio);
  L_read_parm_i (ppi, "max_function_size", &max_function_size);
  L_read_parm_lf (ppi, "min_expansion_weight", &min_expansion_weight);
  L_read_parm_lf (ppi, "min_expansion_key", &min_expansion_key);
  L_read_parm_s (ppi, "sp_output_spec", &sp_output_spec);
  L_read_parm_s (ppi, "il_dir", &il_output_dir_string);
  L_read_parm_b (ppi, "inline_function_pointers", &inline_function_pointers);
  L_read_parm_b (ppi, "inline_indir_by_profile", &inline_indir_by_profile);
  L_read_parm_lf (ppi, "indir_thresh", &indir_thresh);
  L_read_parm_lf (ppi, "inline_ebody_floor", &Pin_ebody_floor);
  L_read_parm_lf (ppi, "min_arc_ratio", &Pin_min_arc_ratio);
  L_read_parm_b (ppi, "regroup_only", &Pin_no_inlining);
  L_read_parm_b (ppi, "size_only", &size_only);
  L_read_parm_b (ppi, "?inline_self_recursion", &inline_self_recursion);
  L_read_parm_s (ppi, "prevent_inline_functions", &prevent_inline_functions);
  L_read_parm_b (ppi, "prevent_cross_file_inlining",
		 &prevent_cross_file_inlining);

  L_read_parm_b (ppi, "favor_small_functions", &favor_small_functions);
  L_read_parm_i (ppi, "small_function_thresh", &small_function_thresh);
  L_read_parm_b (ppi, "exclude_small_from_ratio_limit",
		 &exclude_small_from_ratio_limit);

  L_read_parm_b (ppi, "inline_inlined_body", &inline_inlined_body);

  L_read_parm_b (ppi, "adjust_func_weight", &Pin_adjust_func_weight);

  Pin_inline_key_cost_str = "sqrt_callee_size";
  L_read_parm_s (ppi, "inline_key_cost", &Pin_inline_key_cost_str);
  Pin_body_size_metric_str = "touched";
  L_read_parm_s (ppi, "body_size_metric", &Pin_body_size_metric_str);

  if (!strcmp (Pin_inline_key_cost_str, "sqrt_callee_size"))
    Pin_inline_key_cost = PIN_KEY_SQRT_SIZE;
  else if (!strcmp (Pin_inline_key_cost_str, "callee_size"))
    Pin_inline_key_cost = PIN_KEY_SIZE;
  else if (!strcmp (Pin_inline_key_cost_str, "mc_callee_size"))
    Pin_inline_key_cost = PIN_KEY_MC_SIZE;
  else
    P_punt ("Invalid inline_key_cost '%s'", Pin_inline_key_cost_str);

  if (!strcmp (Pin_body_size_metric_str, "touched"))
    Pin_body_size_metric = PIN_BODY_TOUCH;
  else if (!strcmp (Pin_body_size_metric_str, "executed"))
    Pin_body_size_metric = PIN_BODY_EXECD;
  else if (!strcmp (Pin_body_size_metric_str, "total"))
    Pin_body_size_metric = PIN_BODY_TOTAL;
  else
    P_punt ("Invalid body_size_metric '%s'", Pin_body_size_metric_str);

  return;
}


/*
 * Purpose of this phase :
 *      1) Annotate the call-through-pointer functions 
 *	2) Find out the weight of each function
 */

static void
Pin_preprocess (void)
{
  SymbolTable symtab = PSI_GetTable ();
  int i, num_files = PSI_GetNumFiles ();
  PinCG_Func cgf;

  Pin_arc_heap = Heap_Create (HEAP_MAX);

  for (i = 1; i <= num_files; i++)
    {
      IPSymTabEnt ipe = symtab->ip_table[i];
      Key key;
      FuncDcl fdcl;
      char *filename;

      if (ipe->file_type != FT_SOURCE)
	continue;
      
      filename = ipe->source_name;

      /* JWS 20040507: hack for lib.c inclusion */
      /* REK 20040901: stripping directory name if specified. */
      if (P_NameCheck (filename, "__impact_lib") ||
	  P_NameCheck (filename, "__impact_intrinsic"))
	{
	  P_warn ("SKIPPING %s", filename);
	  continue;
	}

      for (key = PSI_GetFileEntryByType (i, ET_FUNC); P_ValidKey (key);
	   key = PSI_GetFileEntryByTypeNext (key, ET_FUNC))
	{
	  fdcl = PSI_GetFuncDclEntry (key);

	  cgf = Pin_fdcl_cgf (fdcl);

	  if (print_inline_trace)
	    fprintf (Flog, "> Preprocessing %s:%s()\n",
		     filename, fdcl->name);

	  Pin_preprocess_func (cgf, fdcl, 1.0);

	  if (size_only || cgf->node->weight)
	    TouchedBodySize += cgf->o_bodysize;	  
	}
    }

  i = 0;
  if (print_inline_trace)
    fprintf (Flog, "-------------- INDIR CALL WEIGHTS --------------\n");

  List_start (Pin_callgraph->funcs);
  while ((cgf = (PinCG_Func) List_next (Pin_callgraph->funcs)))
    {
      cgf->indir_weight += cgf->weight;
      if (cgf->indir_weight > 0.0)
	{
	  if (print_inline_trace)
	    fprintf (Flog, "%-20s %f\n", cgf->funcname, cgf->indir_weight);
	  assumed_body_size += cgf->o_bodysize;
	  i++;
	}
    }

  assumed_body_size /= i;
  if (print_inline_trace)
    {
      fprintf (Flog, "%-18s= %f\n", "assumed_body_size", assumed_body_size);
      fprintf (Flog, "--------------         END        --------------\n");
    }

  return;
}

static int Pin_tab = 0;

/* Sort arcs into descending weight order */

static int
PinCG_compare_arcs (const void *a, const void *b)
{
  PinCG_Arc aa = *(PinCG_Arc *) a, ab = *(PinCG_Arc *) b;

  if (aa->weight > ab->weight)
    return -1;
  else if (aa->weight == ab->weight)
    return 0;
  else
    return 1;
}

static void
Pin_expand_callees (Stmt stmt, PinCG_Node n)
{
  PinCG_Arc a, *aa;
  int cnt, i;
  Stmt inlinee;

  if (print_inline_stats)
    {
      int j;
      for (j = 0; j < Pin_tab; j++)
	fprintf (Fstat, "   ");
      fprintf (Fstat, "NODE %4d: %s:%s() (wt %0.3f)\n", n->id, 
	       n->func->orig_filename,
	       n->func->funcname, n->weight);
    }

  if (!(cnt = List_size (n->arcs)))
    return;

  aa = alloca (cnt * sizeof (PinCG_Arc));

  for (i = 0, List_start (n->arcs);
       (a = (PinCG_Arc) List_next (n->arcs)); i++)
    aa[i] = a; 

  /* Sort arcs by weight, in case indir arcs are included */

  qsort ((void *) aa, cnt, sizeof (PinCG_Arc), PinCG_compare_arcs);

  Pin_tab++;

  for (i = 0; i < cnt; i++)
    {
      a = aa[i];

      if (print_inline_stats)
	{
	  char *dec = a->inlined ? (a->indirect ? "I*" : "I-") : "--";
	  int j;
	  for (j = 0; j < Pin_tab; j++)
	    fprintf (Fstat, "   ");
	  fprintf (Fstat, ">%05d> %s-(%0.3f)-> %s:%s() (%d)\n", a->id, dec, 
		   a->weight, a->callee->func->orig_filename,
		   a->callee->func->funcname, a->noinline);
	}

      if (!a->inlined)
	continue;

      if (!(inlinee = Pin_expand_inlined_arc (stmt, a)))
	P_warn ("Failed to perform anticipated inlining");

      if (a->callee->arcs)
	Pin_expand_callees (inlinee, a->callee);
    }

  Pin_tab--;

  return;
}


static void
Pin_set_u_weight_node (PinCG_Node n)
{
  PinCG_Arc a;
  List_start (n->arcs);
  while ((a = (PinCG_Arc)List_next (n->arcs)))
    {
      if (a->inlined)
	{
	  a->callee->u_weight = a->r_an * n->u_weight;
	  Pin_set_u_weight_node (a->callee);
	}
    }
  return;
}


static void
Pin_set_u_weight (PinCG_Func f)
{
  PinCG_Node n = f->node;
  n->u_weight = f->weight;
  Pin_set_u_weight_node (n);
  return;
}


static void
Pin_perform_inlining (void)
{
  SymbolTable symtab = PSI_GetTable ();
  PinCG_Func f;
  int i, num_files;
  Key key;
  FuncDcl fdcl;

  num_files = PSI_GetNumFiles ();
  for (i = 1; i <= num_files; i++)
    {
      IPSymTabEnt ipe = symtab->ip_table[i];

      if (ipe->file_type != FT_SOURCE)
	continue;
      
      /* JWS 20040507: hack for lib.c inclusion */
      if (P_NameCheck (ipe->source_name, "__impact_lib") ||
	  P_NameCheck (ipe->source_name, "__impact_intrinsic"))
	{
	  P_warn ("SKIPPING %s", ipe->source_name);
	  continue;
	}

      for (key = PSI_GetFileEntryByType (i, ET_FUNC); P_ValidKey (key);
	   key = PSI_GetFileEntryByTypeNext (key, ET_FUNC))
	{
	  fdcl = PSI_GetFuncDclEntry (key);

	  f = Pin_fdcl_cgf (fdcl);

	  if (!Pin_adjust_func_weight)
	    Pin_set_u_weight (f);

	  Pin_expand_callees (fdcl->stmt, f->node);
	}
    }

  return;
}


void *
Pin_alloc_fdcl_data (void)
{
  return NULL;
}


void *
Pin_free_fdcl_data (void *d)
{
  return NULL;
}


static void
Pin_build_callgraph (SymbolTable symtab)
{
  Key key;
  int i, num_files = PSI_GetNumFiles ();
  FuncDcl fdcl;

  PinCG_create_graph ();

  for (i = 1; i <= num_files; i++)
    {
      IPSymTabEnt ipe = symtab->ip_table[i];

      if (ipe->file_type != FT_SOURCE)
	continue;
      
      /* JWS 20040507: hack for lib.c inclusion */
      if (P_NameCheck (ipe->source_name, "__impact_lib") ||
	  P_NameCheck (ipe->source_name, "__impact_intrinsic"))
	{
	  P_warn ("SKIPPING %s", ipe->source_name);
	  continue;
	}

      for (key = PSI_GetFileEntryByType (i, ET_FUNC); P_ValidKey (key);
	   key = PSI_GetFileEntryByTypeNext (key, ET_FUNC))
	{
	  PinCG_Func cgf;
	  fdcl = PSI_GetFuncDclEntry (key);

	  cgf = PinCG_create_func (fdcl->name, key);

	  cgf->orig_filename = C_findstr (ipe->source_name);

	  cgf->is_noninline = 0;

	  cgf->is_always_inline =
	    (STRING_find_symbol (always_inline_tbl, fdcl->name) != NULL) ?
	    1 : 0;

	  Pin_fdcl_cgf (fdcl) = cgf;
	}
    }

  return;
}


void
P_def_handlers (char *prog_name, Parm_Macro_List *extern_list)
{
  P_ExtSetupM (ES_FUNC, (Extension (*)(void))Pin_alloc_fdcl_data,
	       (Extension (*)(Extension e))Pin_free_fdcl_data);
  return;
}

/*! \brief Main function for Pinline, invoked by libpcode
 *
 * \param prog_name
 *  Program name (probably Pinline)
 * \param external_list
 *  Parameter info
 * \param symbol_table
 *  the symbol table to process.
 * \param file
 *  the key of the file for which we have write permission.
 *
 * \return
 *  Returns non-zero if there is an error.
 */
int
P_gen_code (char *prog_name, Parm_Macro_List *external_list,
	    SymbolTable symbol_table, int file)
{
  PSI_SetTable (symbol_table);

  if (file != 0)
    P_punt ("Pinline requires a.out (whole program, linked) input");

  /*
   * I. Read parameters and initialize structures / files
   * ----------------------------------------------------------------------
   */

  L_load_parameters (P_parm_file, external_list,
		     "(Pinline", P_read_parm_Pinline);

  if (Pin_no_inlining)
    return num_func;

  if (print_inline_stats)
    {
      if (!(Fstat = fopen ("__impact_inlining_stats", "w")))
	P_punt ("Pinline: failed to open stats file "
		"\"__impact_inlining_stats\"");
    }

  prevent_inline_list = new_Func_Name_List (prevent_inline_functions);

  always_inline_tbl = STRING_new_symbol_table ("always_inline_tbl", 128);

  {
    char func_name[256];
    FILE *fptr;

    if ((fptr = fopen ("impact_always_inline.dat", "rt")) ||
	(fptr = fopen ("../impact_always_inline.dat", "rt")))
      {
	while (fscanf (fptr, "%s", func_name) == 1)
	  STRING_add_symbol (always_inline_tbl, func_name, 0);
	
	fclose (fptr);
      }
  }

  /*
   * II. Construct initial call graph and preprocess files
   * ----------------------------------------------------------------------
   */

  Pin_build_callgraph (symbol_table);
  Pin_preprocess ();

  /*
   * III. Select arcs for inlining and perform inlining
   * ----------------------------------------------------------------------
   */
//print_inline_graphs=1;
//print_inline_trace=1;
  if (print_inline_graphs)
    PinCG_dot_callgraph ("cg-preinline.dot");

  switch (Pin_body_size_metric)
    {
    case PIN_BODY_EXECD:
      Pin_budget = (max_expansion_ratio - 1) * TotalEBodySize;
      if ((TotalEBodySize + Pin_budget) < Pin_ebody_floor)
	Pin_budget = Pin_ebody_floor - TotalEBodySize;	  
      break;
    case PIN_BODY_TOUCH:
      Pin_budget = (max_expansion_ratio - 1) * TouchedBodySize;
      if ((TouchedBodySize + Pin_budget) < Pin_ebody_floor)
	Pin_budget = Pin_ebody_floor - TouchedBodySize;	  
      break;
    case PIN_BODY_TOTAL:
      Pin_budget = (max_expansion_ratio - 1) * TotalBodySize;
      if ((TotalBodySize + Pin_budget) < Pin_ebody_floor)
	Pin_budget = Pin_ebody_floor - TotalBodySize;	  
      break;
    default:
      P_punt ("Invalid body size metric");
    }

  if (print_inline_trace)
    {
      fprintf (Flog, "BUDGET: %f * EBody %f = %f\n",
	       max_expansion_ratio - 1, TotalEBodySize, Pin_budget);
      fprintf (Flog, "--------------        SIZES       --------------\n");
      fprintf (Flog, "TotalBodySize    = %8.0f\n", TotalBodySize);
      fprintf (Flog, "TouchedBodySize  = %8.0f\n", TouchedBodySize);
      fprintf (Flog, "--------------         END        --------------\n");
    }

  /* Construct inlined call graph */
  Pin_inline_callgraph ();

  if (print_inline_graphs)
    PinCG_dot_inlining_graphs ();

  /* Perform selected inlinings */

  Pin_perform_inlining ();

  FreeDeadList ();

  if (print_inline_graphs)
    PinCG_dot_callgraph ("cg-postinline.dot");

  if (strcmp (il_log_name, "stdout"))
    fclose (Flog);

  if (print_inline_stats)
    fclose (Fstat);

  return (num_func);
}

