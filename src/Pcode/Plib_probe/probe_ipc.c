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
 *      File:   probe.c
 *      Author: Le-Chun Wu and Wen-mei Hwu
\*****************************************************************************/
/* 07/01/02 REK Changing PP_gen_init_c_code so that the current directory
 *              at build time is used as the target directory for the probe
 *              dump files. */
/* 07/12/02 REK Adding temporary debugging output */

#include <config.h>
#include <stdlib.h>
#include <string.h>
/* 07/01/02 REK #including sys/param.h to get MAXPATHLEN */
#include <sys/param.h>
/* 07/12/02 REK including unistd.h */
#include <unistd.h>
#include "probe.h"
#include <Pcode/query_symtab.h>
#include <Pcode/symtab_i.h>

static int PP_ipc_prof_init = 0;
static int PP_ipc_cnt = 0;
static int PP_inp_cnt = 0;

typedef struct _PP_IPC_TGT {
  char *sym;
  long cnt;
} PP_IPC_TGT;

typedef struct _PP_IPC {
  long cnt;
  int tgt_cnt;
  struct _PP_IPC_TGT *tgt;
} PP_IPC;

static PP_IPC *PP_ipc = NULL;

/* Read in entry ipc iteration profile file -JCG 4/99 */
static void
PP_read_ipc_profile (void)
{
  FILE *in;
  int i, ipc_id, ch, file_ipc_id, entry_count, hits;

  /* Sanity check, should not call twice */
  if (PP_ipc_prof_init)
    P_punt ("PP_read_ipc_profile: already read in!");

  if (!(in = fopen ("profile.ipc", "r")))
    {
      /* REK */
      printf("Plib_probe/probe_ipc.c:%d "
	     "Could not open profile.ipc\n", __LINE__);
      P_punt ("PP_read_ipc_profile: could not open 'profile.ipc' "
	      "for reading!");
    }

  /* Get ipc and input count */
  if (fscanf (in, "%d %d\n", &PP_ipc_cnt, &PP_inp_cnt) != 2)
    P_punt ("PP_read_ipc_profile: error reading ipc and input count!");

  /* Sanity check, input count better be positive */
  if (PP_inp_cnt < 1)
    P_punt ("PP_read_ipc_profile: invalid input count (< 1)!");

  /* Malloc the array to hold all the ipc iteration info */
  if (!(PP_ipc = (PP_IPC *) calloc (PP_ipc_cnt, sizeof (PP_IPC))))
    P_punt ("PP_read_ipc_profile: Out of memory");

  /* Read in each ipc's info */
  for (ipc_id = 0; ipc_id < PP_ipc_cnt; ipc_id++)
    {
      PP_IPC_TGT *p;
      if (fscanf (in, "%d %d %d\n", 
		  &file_ipc_id, &hits, &entry_count) != 3)
	P_punt ("PP_read_ipc_profile: error reading ipc record");

      /* Sanity check, ipc id's better match! */
      if (ipc_id != file_ipc_id)
	{
	  fprintf (stderr, "Expected ipc id %i not %i!\n", ipc_id,
		   file_ipc_id);
	  P_punt ("PP_read_ipc_profile: ipc id mismatch");
	}

      /* Sanity check, better be non-negative number */
      if (entry_count < 0)
	P_punt ("PP_read_ipc_profile: invalid negative entry count");

      /* Set ipc entry count */
      PP_ipc[ipc_id].cnt = hits;
      PP_ipc[ipc_id].tgt_cnt = entry_count;
      if (!(p = calloc (entry_count, sizeof(PP_IPC_TGT))))
	P_punt ("PP_read_ipc_profile: out of memory");      
      PP_ipc[ipc_id].tgt = p;

      for (i = 0; i < entry_count; i++)
	{
	  int hits;
	  char buf[256];
	  if (fscanf (in, "  %255s %d", buf, &hits) != 2)
	    P_punt ("PP_read_ipc_profile: syntax error");

	  p[i].sym = strdup (buf);
	  p[i].cnt = hits;
	}
    }

  /* Sanity check, better only have whitespace left in file */
  while ((ch = getc (in)) != EOF)
    {
      if (!isspace (ch))
	{
	  fprintf (stderr, "Unexpected char '%c' at expected EOF!\n", ch);
	  P_punt ("PP_read_ipc_profile: EOF expected!");
	}
    }

  fclose (in);

  /* Flag that we have read the ipc profile */
  PP_ipc_prof_init = 1;
}

/* Routines to pass back ipc iteration profile back to the routine
 * LCW wrote. -JCG 4/99 
 */
/* Returns the number of iteration entries for this ipc id */
int
PP_get_ipc_max_tgt_id (int ipc_id)
{
  /* Read in the ipc iter profile info, if have not already */
  if (!PP_ipc_prof_init)
    PP_read_ipc_profile ();

  /* Sanity check, make sure ipc_id valid */
  if ((ipc_id < 0) || (ipc_id >= PP_ipc_cnt))
    P_punt ("PP_get_ipc_max_iter_id: invalid ipc id");

  return (PP_ipc[ipc_id].tgt_cnt);
}


static Key
PP_find_function_key (Key scope, char *fname)
{
  Key k = Invalid_Key;
  SymbolTable symtab = PSI_GetTable();

  k = PST_ScopeFindByNameR (symtab, scope, fname, ET_FUNC);

  if (!P_ValidKey (k))
    {
      int i, num_files;

      num_files = P_GetSymbolTableNumFiles (symtab);

      for (i = 0; i <= num_files; i++)
	{
	  Key scope;

	  scope.file = i;
	  scope.sym = 1;

	  k = PST_ScopeFindByName (symtab, scope, fname, ET_FUNC);

	  if (P_ValidKey (k))
	    break;
	}
    }

  return k;
}

void
PP_annotate_ipc (Expr expr, int id)
{
  int i, tgt_cnt;
  PP_IPC_TGT *t;
  char buf1[128];
  double tcnt;

  if (!PP_ipc_prof_init)
    PP_read_ipc_profile ();

  if ((id < 0) || (id >= PP_ipc_cnt))
    P_punt ("PP_get_ipc_cnt: invalid ipc id");

  if (!(tgt_cnt = PP_ipc[id].tgt_cnt))
    return;

  if (!PP_ipc[id].cnt)
    return;

  tcnt = (double) PP_ipc[id].cnt;

  for (i = 0; i < tgt_cnt; i++)
    {
      double rat;
      Pragma p;
      Key callee_key, scope;
      Expr ve;

      t = PP_ipc[id].tgt + i;
      rat = (double) t->cnt / tcnt;
      sprintf (buf1, "IPC-%0.4f", rat);

      scope = PSI_GetExprScope (expr);

      callee_key = PP_find_function_key (scope, t->sym);

      ve = PSI_ScopeNewExprWithOpcode (scope, OP_var);
      P_SetExprVarName (ve, strdup (t->sym));
      P_SetExprVarKey (ve, callee_key);

      p = P_NewPragmaWithSpecExpr (buf1, ve);
      expr->pragma = P_AppendPragmaNext (expr->pragma, p);
    }
  return;
}
