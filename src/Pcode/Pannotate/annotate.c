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

#include "annotate.h"

#include <Pcode/dom.h>
#include <Pcode/loop.h>
#include <Pcode/util.h>

int PA_merge_profile = 1;

void PA_AnnotateWeights (PC_Graph cfg);
void PA_AnnotateLPCounters (PC_Graph cfg);
void PA_ReadLPCounters (void);

static void PA_AnnotateStmt (Stmt stmt, double wt);
static void PA_AnnotateExpr (Expr expr, double wt);

static void PA_SetLPPragmaExprs (PC_Graph cfg, PC_LpPrag lp_prg,
				 Expr * lp, Expr * fn, Expr * fl);

static void PA_FreeGlobals ();

typedef struct PA_Iter
{
  int iter;			/* Number of iterations weights are for */
  double *weight;		/* Array of weights, indexed by input id */
}
PA_Iter;

typedef struct PA_Loop
{
  PA_Iter *entry;		/* Array of iter entries for this loop */
  int entry_count;		/* Number of iter entries for this loop */
}
PA_Loop;

static int PA_loop_count = 0;
static int PA_input_count = 0;
static PA_Loop *PA_loop = NULL;


static void
PA_read_parm_Pannotate (Parm_Parse_Info * ppi)
{
  L_read_parm_b (ppi, "merge_profile", &PA_merge_profile);
  return;
}


/*! \brief Generates annotated pcode for all functions in a given program
 *
 * \param prog_name
 *  program name
 * \param external_list
 *  list of parm macros
 * \param symbol_table
 *  symbol table
 * \param file
 *  file
 * \return number of funcs processed
 */
int
P_gen_code (char *prog_name, Parm_Macro_List * external_list,
	    SymbolTable symbol_table, int file)
{
  int i, num_files;

  L_load_parameters (P_parm_file, external_list, "(Pprobe",
		     PP_read_parm_Pprobe);
  L_load_parameters (P_parm_file, external_list, "(Pannotate",
		     PA_read_parm_Pannotate);

  PSI_SetTable (symbol_table);

  /* Open the file containing BB weights. */
  Fprofile = fopen ("profile.dat", "r");

  if (PP_annotate_lp)
    {
      /* Read loop iter info from profile.iter. */
      PA_ReadLPCounters ();
    }

  if (PP_annotate_ip)
    {
      next_ipc_id = 0;
    }

  /* LCW - open null file for annotating Pcode - 2/19/96 */
#undef NULLFILE
#ifndef WIN32
#define NULLFILE "/dev/null"
#else
#define NULLFILE "NUL"
#endif

  if (PA_merge_profile)
    {
      if (strcmp (F_annot, "stdin"))
	{
	  Fannot = fopen (F_annot, "r");
	  if (Fannot == NULL)
	    {
	      fprintf (Ferr, "Error, cannot open annot file: %s\n", F_annot);
	      exit (-1);
	    }
	}
      else
	{
	  fprintf (Ferr, "Error, annot file cannot be stdin\n");
	  exit (-1);
	}

      if (strcmp (F_annot_index, "stdin"))
	{
	  Fannot_index = fopen (F_annot_index, "r");
	  if (Fannot_index == NULL)
	    {
	      fprintf (Ferr, "Error, cannot open annot file: %s\n",
		       F_annot_index);
	      exit (-1);
	    }
	}
      else
	{
	  fprintf (Ferr, "Error, annot_index file cannot be stdin\n");
	  exit (-1);
	}

#if 0
      Pannotate_Init ();
#endif

      if (strcmp (F_pcode_position, "stdout"))
	{
	  Fpcode_position = fopen (F_pcode_position, "w");
	  if (Fpcode_position == NULL)
	    {
	      fprintf (Ferr, "Error, cannot open pcode position file: %s\n",
		       F_pcode_position);
	      exit (-1);
	    }
	}
    }


  /*
   * Process the Files
   */

  num_files = P_GetSymbolTableNumFiles (symbol_table);
  for (i = 1; i <= num_files; i++)
    {
      Key key;
      IPSymTabEnt ipe = symbol_table->ip_table[i];

      if (ipe->file_type != FT_SOURCE)
	continue;

      /* JWS 20040507: hack for lib.c inclusion */
      /* REK 20040901: stripping directory name if specified. */
      if (P_NameCheck (ipe->source_name, "__impact_lib") || 
	  P_NameCheck (ipe->source_name, "__impact_intrinsic"))
	{
	  P_warn ("SKIPPING %s", ipe->source_name);
	  continue;
	}

      for (key = PST_GetFileEntryByType (symbol_table, i, ET_ANY);
	   P_ValidKey (key);
	   key = PST_GetFileEntryByTypeNext (symbol_table, key, ET_ANY))
	{
	  SymTabEntry entry = PST_GetSymTabEntry (symbol_table, key);

	  if (P_GetSymTabEntryType (entry) == ET_FUNC)
	    {
	      FuncDcl f = P_GetSymTabEntryFuncDcl (entry);
	      if (P_GetFuncDclStmt (f))
		{
		  PC_Function (f, 1, PC_SPLIT_CRIT | PC_ANNOTATE);
		  PA_AnnotateWeights (PC_cfg);
		  if (PP_annotate_lp)
		    PA_AnnotateLPCounters (PC_cfg);
#if 0
		  if (PA_merge_profile)
		    Pannotate_Func (f);
#endif
		}
	    }
	}
    }

#if 0
  if (PA_merge_profile)
    Pannotate_Finish ();
#endif


  if (PP_annotate_lp)
    PA_FreeGlobals ();

  fclose (Fprofile);
  return (num_func);
}


/*! \brief Annotates the block weights read from profile.dat back onto the blocks
 *   and the individual stmts in the blocks.
 *
 * \param cfg
 *  control flow graph of the function being processed.
 *
 * \return void.
 */
void
PA_AnnotateWeights (PC_Graph cfg)
{
  PC_Block bb;
  PC_PStmt ps;
  double wt;


  /* Read weights from the file and copy them onto their respective PC_Blocks. */
  for (bb = cfg->first_bb; bb; bb = bb->next)
    {
      fscanf (Fprofile, "%lf", &wt);
      bb->weight = wt;
    }

  /* Annotate each PC_Block's weight onto pcode stmts & exprs in the PC_Block. */
  for (bb = cfg->first_bb; bb; bb = bb->next)
    {
      double *pcount;
      /* Annotate all explicitly present pcode stmts in this bb. */
      for (ps = bb->first_ps; ps; ps = ps->succ)
	if (ps->type == PC_T_NatPcode)
	  PA_AnnotateStmt (ps->data.stmt, bb->weight);

      /* Annotate the cond_expr in this bb. */
      PA_AnnotateExpr (bb->cond, bb->weight);

      /* Annotate relevant profile field of if/for/while/switch stmt. */
      List_start (bb->counters);
      while ((pcount = (double *) List_next (bb->counters)))
	*pcount += bb->weight;
    }

  {
    double wt = cfg->first_bb->weight;
    FuncDcl func = cfg->func;
    
    if (!func->profile)
      func->profile = P_NewProfFN ();

    func->profile->count += wt;
  }

  return;
}


/*! \brief For indirect call profiling, finds and annotates calls
 *
 * \param expr
 *  the pcode call expr to be annotated
 *
 * \return void
 */
static void
PA_AnnotateCallExpr (Expr expr)
{
  if (P_IsIndirectFunctionCall (expr))
    PP_annotate_ipc (expr, next_ipc_id++);

  return;
}


/*! \brief  Annotates the profile weight onto expr and Expr-type fields of expr
 *
 * \param expr
 *  the pcode expr to be annotated
 * \param wt
 *  profile weight value to be annotated onto expr
 *
 * \return void
 */
static void
PA_AnnotateExpr (Expr expr, double wt)
{
  if (!expr)
    return;

  if (!expr->profile)
    expr->profile = P_NewProfEXPR_w_wt (wt);
  else
    expr->profile->count += wt;

  if (expr->sibling)
    PA_AnnotateExpr (expr->sibling, wt);
  if (expr->operands)
    PA_AnnotateExpr (expr->operands, wt);
  if (expr->next)
    PA_AnnotateExpr (expr->next, wt);

  if (PP_annotate_ip)
    PA_AnnotateCallExpr (expr);

  return;
}


/*! \brief  Annotates the profile weight onto Expr fields of a stmt
 *
 * \param stmt
 *  the pcode stmt whose Expr fields are to be annotated
 * \param wt
 *  profile weight value to be annotated
 *
 * \return void
 */
static void
PA_AnnotateStmt (Stmt stmt, double wt)
{
  if (!stmt)
    return;

  switch (stmt->type)
    {
    case ST_RETURN:
      PA_AnnotateExpr (stmt->stmtstruct.ret, wt);
      break;
    case ST_EXPR:
      PA_AnnotateExpr (stmt->stmtstruct.expr, wt);
      break;
    default:
      break;
    }
}



/*! \brief Reads loop profile info from profile.iter into a loop array in memory
 *
 * \return void
 */
void
PA_ReadLPCounters (void)
{
  FILE *Floop;
  PA_Iter *it_entry = NULL;
  double *wt_ary;
  int input_id, lp_id, f_lp_id, num_entr, entr_id, it_count;

  Floop = fopen ("profile.iter", "r");

  fscanf (Floop, "%d %d\n\n", &PA_loop_count, &PA_input_count);
  PA_loop = (PA_Loop *) calloc (PA_loop_count, sizeof (PA_Loop));

  /* Read in each loop entry. */
  for (lp_id = 0; lp_id < PA_loop_count; lp_id++)
    {
      fscanf (Floop, "%d %d\n", &f_lp_id, &num_entr);
      PA_loop[lp_id].entry_count = num_entr;
      if (num_entr > 0)
	{
	  it_entry = (PA_Iter *) calloc (num_entr, sizeof (PA_Iter));
	  PA_loop[lp_id].entry = it_entry;
	}
      else
	{
	  PA_loop[lp_id].entry = NULL;
	}

      /* Read in each iter entry for this particular loop. */
      for (entr_id = 0; entr_id < num_entr; entr_id++)
	{
	  fscanf (Floop, "%d", &it_count);
	  it_entry[entr_id].iter = it_count;
	  wt_ary = (double *) calloc (PA_input_count, sizeof (double));
	  it_entry[entr_id].weight = wt_ary;
	  for (input_id = 0; input_id < PA_input_count; input_id++)
	    fscanf (Floop, "%lf", &wt_ary[input_id]);
	}
    }

  fclose (Floop);
}


/*! \brief Returns the max iter id of the particular loop represented by loop_id
 *
 * \param loop_id
 *  uniquely identifying ID of a loop, this ID is read from profile.iter
 *
 * \return max iter id for this loop.
 */
int
PA_get_loop_max_iter_id (int loop_id)
{
  /* Sanity check, make sure loop_id valid */
  if ((loop_id < 0) || (loop_id >= PA_loop_count))
    I_punt ("PA_get_loop_max_iter_id: invalid loop id");

  return (PA_loop[loop_id].entry_count);
}


/*! \brief Returns the # of profiling runs whose info is stored in profile.dat
 *
 * \return # of profiling runs.
 */
int
PA_get_loop_max_input_id (void)
{
  return (PA_input_count);
}


/*! \brief Returns # of iterations of a particular loop for a particular iter_id
 *
 * \param loop_id
 *  uniquely identifying ID of a loop, this ID is read from profile.iter
 * \param iter_id
 *  the ID of a particular iteration of a loop, this ID is read from profile.iter
 *
 * \return # of iterations of this loop for iteration ID equal to iter_id.
 */
int
PA_get_loop_iter_count (int loop_id, int iter_id)
{
  /* Sanity check, make sure loop_id valid */
  if ((loop_id < 0) || (loop_id >= PA_loop_count))
    I_punt ("PA_get_loop_iter_count: invalid loop id");

  /* Sanity check, make sure iter_id valid */
  if ((iter_id < 0) || (iter_id >= PA_loop[loop_id].entry_count))
    I_punt ("PA_get_loop_iter_count: invalid iter id");

  return (PA_loop[loop_id].entry[iter_id].iter);
}


/*! \brief Computes the average weight for this iteration over all profiling runs
 *
 * \param loop_id
 *  uniquely identifying ID of a loop, this ID is read from profile.iter
 * \param iter_id
 *  the ID of a particular iteration of a loop, this ID is read from profile.iter
 *
 * \return the average profile weight of the loop iteration in question.
 */
double
PA_get_loop_iter_avg_exec (int loop_id, int iter_id)
{
  int input_id;
  double sum, avg;

  /* Sanity check, make sure loop_id valid */
  if ((loop_id < 0) || (loop_id >= PA_loop_count))
    I_punt ("PA_get_loop_iter_avg_exec: invalid loop id");

  /* Sanity check, make sure iter_id valid */
  if ((iter_id < 0) || (iter_id >= PA_loop[loop_id].entry_count))
    I_punt ("PA_get_loop_iter_avg_exec: invalid iter id");

  /* Sum up the inputs */
  sum = 0.0;
  for (input_id = 0; input_id < PA_input_count; input_id++)
    sum += PA_loop[loop_id].entry[iter_id].weight[input_id];

  /* Make average */
  avg = sum / ((double) PA_input_count);

  return (avg);
}


/*! \brief Returns the weight for the given loop_id, iter_id & input_id
 *
 * \param loop_id
 *  uniquely identifying ID of a loop, this ID is read from profile.iter
 * \param iter_id
 *  the ID of a particular iteration of a loop, this ID is read from profile.iter
 * \param input_id
 *  in case of multiple profiling runs, input_id identifies the relevant run
 *
 * \return the profile weight of the loop iteration in question.
 */
double
PA_get_loop_iter_input_exec (int loop_id, int iter_id, int input_id)
{
  /* Sanity check, make sure loop_id valid */
  if ((loop_id < 0) || (loop_id >= PA_loop_count))
    I_punt ("PA_get_loop_iter_input_exec: invalid loop id");

  /* Sanity check, make sure iter_id valid */
  if ((iter_id < 0) || (iter_id >= PA_loop[loop_id].entry_count))
    I_punt ("PA_get_loop_iter_input_exec: invalid iter id");

  /* Sanity check, make sure input_id valid */
  if ((input_id < 0) || (input_id >= PA_input_count))
    I_punt ("PA_get_loop_iter_input_exec: invalid input id");

  return (PA_loop[loop_id].entry[iter_id].weight[input_id]);
}


/*! \brief Annotates the loop profiling info in profile.iter back onto the loop 
 *   headers in the control flow graph.
 *
 * \param cfg
 *  control flow graph of the function being processed.
 *
 * Note: lp counter values are stored in pragmas as DoubleExpr and IntExpr as 
 * opposed to StringExpr, as they previously were.
 *
 * \return void.
 */
void
PA_AnnotateLPCounters (PC_Graph cfg)
{
  PC_Loop lp;

  /* process each loop in turn. */

  for (lp = cfg->lp; lp; lp = lp->next)
    {
      int max_iter_id;
      int iter_id, input_id;
      char spec_str[256];
      double hdr_wt = 0.0;
      Expr lp_typ, func, file;
      Expr explst = NULL, *exp = NULL;
      PC_Block h_bb = PC_FindBlock (cfg, lp->head);

      if (!h_bb->lp_prg || !h_bb->lp_prg->ptr)
	{
	  P_warn ("No lp pragma pointer on header bb");
	  continue;
	}

      if (*(h_bb->lp_prg->ptr) && P_FindPragma (*(h_bb->lp_prg->ptr), "LOOP"))
	{
	  P_warn
	    ("Loop pragmas from previous runs, in BB #%d, in func: %s.\n",
	     h_bb->ID, cfg->func->name);
	  P_RemovePragma (*(h_bb->lp_prg->ptr));
	  *(h_bb->lp_prg->ptr) = NULL;
	}

      /* generate lp prag exprs based on the info in h_bb->lp_prg. */
      PA_SetLPPragmaExprs (cfg, h_bb->lp_prg, &lp_typ, &func, &file);
#if DEBUG
      lp_typ->next->next = P_NewIntExpr (lp->ID);
#endif

      *(h_bb->lp_prg->ptr) =
	P_AppendPragmaNext (*(h_bb->lp_prg->ptr),
			    P_NewPragmaWithSpecExpr ("LOOP", lp_typ));
      *(h_bb->lp_prg->ptr) =
	P_AppendPragmaNext (*(h_bb->lp_prg->ptr),
			    P_NewPragmaWithSpecExpr ("FUNC", func));
      *(h_bb->lp_prg->ptr) =
	P_AppendPragmaNext (*(h_bb->lp_prg->ptr),
			    P_NewPragmaWithSpecExpr ("FILE", file));

      max_iter_id = PA_get_loop_max_iter_id (lp->ID);

      explst = P_NewIntExpr (max_iter_id);
      explst->next = P_NewIntExpr (PA_input_count);
      *(h_bb->lp_prg->ptr) =
	P_AppendPragmaNext (*(h_bb->lp_prg->ptr),
			    P_NewPragmaWithSpecExpr ("iteration_header",
						     explst));
      for (iter_id = 0; iter_id < max_iter_id; iter_id++)
	{
	  int iter_count;
	  double avg, freq;

	  iter_count = PA_get_loop_iter_count (lp->ID, iter_id);
	  sprintf (spec_str, "iter_%d", iter_count);
	  avg = PA_get_loop_iter_avg_exec (lp->ID, iter_id);
	  explst = P_NewDoubleExpr (avg);
	  exp = &(explst->next);
	  for (input_id = 0; input_id < PA_input_count; input_id++)
	    {
	      freq = PA_get_loop_iter_input_exec (lp->ID, iter_id, input_id);
	      *(exp) = P_NewDoubleExpr (freq);
	      exp = &((*exp)->next);
	      hdr_wt += (iter_count * freq);	/* # of times hdr executed. */
	    }
	  *(h_bb->lp_prg->ptr) =
	    P_AppendPragmaNext (*(h_bb->lp_prg->ptr),
				P_NewPragmaWithSpecExpr (spec_str, explst));
	}

      /* Checking lp profile values against bb profile values. */
      if (hdr_wt != h_bb->weight)
	P_warn ("Loop profile mismatch: h_bb->weight: %f, hdr_wt: %f.",
		h_bb->weight, hdr_wt);
    }

  return;
}


/*! \brief Generates a list of String Exprs to be added to a loop pragma. 
 *
 * \param cfg
 *  control flow graph of the function being processed.
 * \param lp_prg
 *  lp pragma for this loop, contains the lp type and lp line # info.
 * \param lp
 *  this will be set to an expr list containing the lp type and line #.
 * \param fn
 *  this will be set to an expr list containing the func name and line #.
 * \param fl
 *  this will be set to an expr containing the file name.
 *
 * \return void
 *
 * \sa PC_NewLpPrag ()
 */
static void
PA_SetLPPragmaExprs (PC_Graph cfg, PC_LpPrag lp_prg,
		     Expr * lp, Expr * fn, Expr * fl)
{
  char lp_typ[16];
  _PC_LoopType loop_type;
  int lp_ln;

  loop_type = lp_prg->lp_typ;
  lp_ln = lp_prg->lp_ln;

  switch (loop_type)
    {
    case PC_LT_FOR:
      sprintf (lp_typ, "for");
      break;
    case PC_LT_WHILE:
      sprintf (lp_typ, "while");
      break;
    case PC_LT_DO:
      sprintf (lp_typ, "do");
      break;
    case PC_LT_GOTO:
      sprintf (lp_typ, "goto");	/* a goto -> label type loop. */
      break;
    default:
      P_punt ("PA_SetLPPragmaExprs: Unkown loop type: %d.", loop_type);
    }
  *lp = P_NewStringExpr (lp_typ);
  (*lp)->next = P_NewIntExpr (lp_ln);

  *fn = P_NewStringExpr (cfg->func->name);
  (*fn)->next = P_NewIntExpr (cfg->func->stmt->lineno);

  *fl = P_NewStringExpr (cfg->func->stmt->filename);

  return;
}


/*! \brief Frees up memory allocated thru global variables in this file.
 */
static void
PA_FreeGlobals (void)
{
  /* Tahir - Something's messed up here, free () causes a Seg fault. */
#if 0
  int i, j;
  PA_Loop *lp;
  PA_Iter *it;

  for (i = 0; i < PA_loop_count; i++)
    {
      lp = &(PA_loop[i]);
      for (j = 0; j < lp->entry_count; j++)
	{
	  it = &(lp->entry[j]);
	  if (it && it->weight)
	    free (it->weight);
	}
      if (lp && lp->entry)
	free (lp->entry);
    }

  if (PA_loop)
    free (PA_loop);
  return;
#endif
}
