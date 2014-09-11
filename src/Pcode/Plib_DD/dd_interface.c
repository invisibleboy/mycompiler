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


#include <string.h>
#include <Pcode/extension.h>
#include <Pcode/struct.h>
#include <library/i_list.h>
#include "dd_interface.h"

/***********************************************************************/
/* static */

typedef struct _P_PrintDepInfoCtrl
{
  FILE *outf;
  int expr_ext_idx;
}
_P_PrintDepInfoCtrl, *P_PrintDepInfoCtrl;

static long SizeOfDepListToString (List list);
static void DumpDepInfo (FILE *outf, P_DepInfo dep);
static void PrintExprDepInfo (Expr expr, void *data);

/***********************************************************************/

Extension
P_DepList_alloc (void)
{
  P_DepList x;

  x = ALLOCATE (_P_DepList);
  Set_DepList_deps(x, NULL);
  return x;
}

Extension
P_DepList_free (Extension x)
{
  List list;
  P_DepList dep_list;
  P_DepInfo dep_info;

  dep_list = x;
  list = Get_DepList_deps(dep_list);
  List_start (list);
  while ((dep_info = List_next (list))) 
    P_FreeDepInfo (dep_info);
  List_reset(list);
  return NULL;
}


static int
P_DepInfo_dir_write (char *s, P_DepInfo dep, int i)
{
  P_DepDir dir = Get_DepInfo_dir_i (dep, i);
  char *ds;

  switch (dir)
    {
    case DDIR_LT: ds = "lt"; break;
    case DDIR_EQ: ds = "eq"; break;
    case DDIR_LE: ds = "le"; break;
    case DDIR_GT: ds = "gt"; break;
    case DDIR_NE: ds = "ne"; break;
    case DDIR_GE: ds = "ge"; break;
    case DDIR_ALL: ds = "al"; break;
    default: 
      P_punt ("Tried to write bad DepInfo dir");
      return 0;
    }

  return sprintf (s, "%s ", ds);
}


static P_DepDir
P_DepInfo_dir_parse (char *s)
{
  if (!s || !s[0] || !s[1] || s[2])
    P_punt ("Tried to parse bad DepInfo dir");

  if (s[0] == 'e' && s[1] == 'q')
    {
      return DDIR_EQ;
    }
  else if (s[0] == 'a' && s[1] == 'l')
    {
      return DDIR_ALL;
    }
  else if (s[0] == 'n' && s[1] == 'e')
    {
      return DDIR_NE;
    }
  else if (s[0] == 'g')
    {
      if (s[1] == 'e')
	return DDIR_GE;
      else if (s[1] == 't')
	return DDIR_GT;
    }
  else if (s[0] == 'l')
    {
      if (s[1] == 'e')
	return DDIR_LE;
      else if (s[1] == 't')
	return DDIR_LT;
    }
  
  P_punt ("Tried to parse bad DepInfo dir \"%s\"", s);
  return 0;
}


static int
P_DepInfo_dep_type_write (char *s, P_DepInfo dep)
{
  P_DepType dep_type = Get_DepInfo_dep_type (dep);
  char c;

  switch (dep_type)
    {
    case DT_NONE: c = 'N'; break;
    case DT_FLOW: c = 'F'; break;
    case DT_ANTI: c = 'A'; break;
    case DT_OUTPUT: c = 'O'; break;
    case DT_INPUT: c = 'I'; break;
    default: 
      P_punt ("Tried to write bad DepInfo type");
      return 0;
    }
  
  return sprintf(s, "%c ", c);
}

static P_DepType
P_DepInfo_dep_type_parse (char c)
{
  switch (c)
    {
    case 'N': return DT_NONE;
    case 'F': return DT_FLOW;
    case 'A': return DT_ANTI;
    case 'O': return DT_OUTPUT;
    case 'I': return DT_INPUT;
    default: P_punt ("Tried to parse bad DepInfo type '%c'", c);
    }

  return 0;
}



/*
 * (type expr dim (dir)+{dim} (dist)+{dim} (known)+{dim})*
 */
char *
P_DepList_write (char *sig, Extension x)
{
  P_DepList dep_list;
  P_DepInfo dep;
  List list;
  long buf_size;
  char *buf;
  char *s;
  int i;

  dep_list = x;
  list = Get_DepList_deps(dep_list);
  if (list == NULL) 
    return NULL;
  assert (List_size(list) > 0);
  buf_size = SizeOfDepListToString (list);

  buf = (char *) calloc (buf_size, sizeof(char));
  if (buf == NULL)
    P_punt("P_DepList_write: calloc buf");
  s = buf;
  List_start(list);
  while ((dep = List_next(list)))
    {
      /*
       * id 
       */
      s += sprintf(s, "%d:", Get_DepInfo_id(dep));
      assert (s < buf + buf_size);
      /*
       * expr 
       */
      s += sprintf(s, "%d ", Get_DepInfo_expr(dep));
      assert (s < buf + buf_size);
      /*
       * dep_type
       */
      s += P_DepInfo_dep_type_write (s, dep);
      assert (s < buf + buf_size);
      /*
       * node_type
       */
      s += sprintf(s, "%c ", Get_DepInfo_node_type(dep));
      /*
       * depth
       */
      s += sprintf(s, "%d ", Get_DepInfo_depth(dep)); 

      assert (s < buf + buf_size);
      /*
       * (dir)+{depth}
       */
      for (i = 0 ; i < Get_DepInfo_depth(dep) ; i++)
        { 
          s += P_DepInfo_dir_write (s, dep, i);
          assert (s < buf + buf_size);
        }
      /*
       * (dist)+{depth}
       */
      for (i = 0 ; i < Get_DepInfo_depth(dep) ; i++)
        {
          s += sprintf(s, "%d ", Get_DepInfo_dist_i(dep, i));
          assert (s < buf + buf_size);
        }
      /*
       * (known)+{depth}
       */
      for (i = 0 ; i < Get_DepInfo_depth(dep) ; i++)
        {
          s += sprintf(s, "%d ", Get_DepInfo_known_i(dep, i));
          assert (s < buf + buf_size);
        }
    }
  return buf;
}

void 
P_DepList_read (Extension x, char *sig, char *raw)
{
  P_DepList dep_list;
  P_DepInfo dep;
  char *buf;
  char *s;
  int i;
  int n;
  int id;
  int expr;
  P_DepType dep_type;
  char node_type;
  short depth;
  
  dep_list = x;
  if (raw == NULL)
    {
      Set_DepList_deps (dep_list, NULL);
      return;
    }
  buf = strdup(raw);
  s = buf;

  s = strtok (buf, " :");

  while (s)
    {
      /* id */
      id = strtol(s, NULL, 10); 

      s = strtok (NULL, " ");
      /* expr */
      expr = strtol(s, NULL, 10);

      s = strtok (NULL, " ");
      /* dep_type */
      assert (s != NULL);
      dep_type = P_DepInfo_dep_type_parse (s[0]);

      s = strtok (NULL, " ");
      /* node_type */
      assert (s != NULL);
      node_type = s[0];
      assert ((node_type == DEP_HEAD) || (node_type == DEP_TAIL));

      s = strtok (NULL, " ");
      /* depth */
      assert (s != NULL);
      depth = strtol(s, NULL, 10);

      dep = P_NewDepInfo (id, expr, dep_type, node_type, depth);

      /*
       * (dir)+{depth}
       */
      for (i = 0 ; i < depth ; i++)
        {
          s = strtok (NULL, " ");
          assert (s != NULL);
	  n = P_DepInfo_dir_parse (s);
          Set_DepInfo_dir_i(dep, i, n);
        }
      /*
       * (dist)+{depth}
       */
      for (i = 0 ; i < depth ; i++)
        {
          s = strtok (NULL, " ");
          assert (s != NULL);
          n = strtol(s, NULL, 16);
          Set_DepInfo_dist_i(dep, i, n);
        }
      /*
       * (known)+{depth}
       */
      for (i = 0 ; i < depth ; i++)
        {
          s = strtok (NULL, " ");
          assert (s != NULL);
          n = strtol(s, NULL, 16);
          Set_DepInfo_known_i(dep, i, n);
        }

      Set_DepList_deps (dep_list, 
			List_insert_first (Get_DepList_deps (dep_list), dep));

      s =  strtok (NULL, " :");
    }

  return;
}

Extension
P_DepList_copy (Extension x)
{
  P_DepList old_dep_list;
  P_DepList new_dep_list;
  P_DepInfo old_dep;
  P_DepInfo new_dep;
  List old_list;

  old_dep_list = x;
  new_dep_list = P_DepList_alloc();
  old_list = Get_DepList_deps(old_dep_list);
  List_start (old_list);
  while ((old_dep = List_next(old_list)))
    {
      new_dep = P_CopyDepInfo (old_dep);
      Set_DepList_deps (new_dep_list,
                        List_insert_first (Get_DepList_deps(new_dep_list), new_dep));
    }
  return new_dep_list;
}

P_DepInfo
P_NewDepInfo (int id, int expr, P_DepType dep_type, char node_type, short depth)
{
  P_DepInfo dep;
  short i;

  dep = ALLOCATE (_P_DepInfo);
  Set_DepInfo_id (dep, id);
  Set_DepInfo_expr (dep, expr);
  Set_DepInfo_dep_type (dep, dep_type);
  Set_DepInfo_node_type (dep, node_type);
  Set_DepInfo_depth (dep, depth);
  if (depth > 0) 
    {
      Set_DepInfo_dir (dep, calloc (depth, sizeof(short)));
      if (Get_DepInfo_dir (dep) == NULL)
        P_punt("P_NewDepInfo: calloc DepInfo_dir");

      Set_DepInfo_dist (dep, calloc (depth, sizeof(short)));
      if (Get_DepInfo_dist (dep) == NULL)
        P_punt("P_NewDepInfo: calloc DepInfo_dist");
      for (i = 0 ; i < depth ; i++)
        Set_DepInfo_dist_i (dep, i, -1);

      Set_DepInfo_known (dep, calloc (depth, sizeof(char)));
      if (Get_DepInfo_known (dep) == NULL)     
        P_punt("P_NewDepInfo: calloc DepInfo_known");
    }
  else
    {
      Set_DepInfo_dir (dep, NULL);
      Set_DepInfo_dist (dep, NULL);
      Set_DepInfo_known (dep, NULL);
    }
  return dep;
}

P_DepInfo
P_CopyDepInfo (P_DepInfo dep)
{
  P_DepInfo new_dep;
  int i;

  new_dep = P_NewDepInfo (Get_DepInfo_id (dep), 
                          Get_DepInfo_expr (dep), 
                          Get_DepInfo_dep_type (dep),
                          Get_DepInfo_node_type (dep),
                          Get_DepInfo_depth (dep));
  for (i = 0 ; i < Get_DepInfo_depth (dep) ; i++) 
    { 
      Set_DepInfo_dir_i(new_dep, i, Get_DepInfo_dir_i(dep, i));
      Set_DepInfo_dist_i(new_dep, i, Get_DepInfo_dist_i(dep, i));
      Set_DepInfo_known_i(new_dep, i, Get_DepInfo_known_i(dep, i));
    }
  return new_dep;
}

void
P_FreeDepInfo (P_DepInfo dep)
{
  free (Get_DepInfo_dir(dep));
  free (Get_DepInfo_dist(dep));
  free (Get_DepInfo_known(dep));
  DISPOSE (dep);
}

void
PrintFuncDepInfo (FILE *outf, FuncDcl func, int ext_deplist_idx)
{
  Stmt stmt;
  _P_PrintDepInfoCtrl ctrl;  

  ctrl.outf = outf;
  ctrl.expr_ext_idx = ext_deplist_idx;
  
  fprintf (outf, "\n------------------------------------------------\n");
  fprintf (outf, "PrintFuncDepInfo for \"%s\"\n\n", func->name);
  stmt = P_GetFuncDclStmt (func); 
  P_StmtApply (stmt, NULL, PrintExprDepInfo, &ctrl);
  fprintf (outf, "\n------------------------------------------------\n");
}   

/***********************************************************************/

static long
SizeOfDepListToString (List list)
{
  long size;
  P_DepInfo dep;
  int max_digit;

  /*
   * log10(2**(sizeof(int)*8)) = sizeof(int) * 8 * log10(2) 
   */
  max_digit = sizeof(int) * 4 + 1; /* + 1 for space */
  size = 0;
  /*
   * (dep_type id expr node_type depth (dir)+{depth} (dist)+{depth} (known)+{depth})*
   */
  List_start (list);
  while ((dep = List_next(list)))
    {
       size += 5 * max_digit;
       if (Get_DepInfo_depth(dep) > 0)
         size += Get_DepInfo_depth(dep) * 3 * max_digit;
    }
  return size + 1; /* terminating '\0' */
}

static void
DumpDepInfo (FILE *outf, P_DepInfo dep)
{
  int i;

  fprintf (outf, "dep_type = ");
  switch (Get_DepInfo_dep_type(dep))
    {
    case DT_NONE:
      fprintf(outf, "None");
      break;
    case DT_FLOW:
      fprintf(outf, "Flow");
      break;
    case DT_ANTI:
      fprintf(outf, "Anti");
      break;
    case DT_OUTPUT:
      fprintf(outf, "Output");
      break;
    case DT_INPUT:
      fprintf(outf, "Input");
      break;
    default: 
      P_punt("DumpDepInfo: illegal type");
    }  

  switch (Get_DepInfo_node_type (dep))
    {
      case DEP_TAIL:
        fprintf (outf, " source, target expr = %d\n", Get_DepInfo_expr(dep));
        break;
      case DEP_HEAD:
        fprintf (outf, " target, source expr = %d\n", Get_DepInfo_expr(dep));
        break;
      default:
        P_punt ("DumpDepInfo: illegal node_type");
    }

  if (Get_DepInfo_depth (dep) < 0)
    P_punt ("DumpDepInfo: illegal depth");

  if (Get_DepInfo_depth (dep) == 0)
    return;

  fprintf (outf, "depth = %d\n", Get_DepInfo_depth (dep));

  fprintf (outf, "dir = [ ");
  for (i = 0 ; i < Get_DepInfo_depth (dep); i++)
    switch (Get_DepInfo_dir_i(dep, i)) 
      {
      case DDIR_LT:
        fprintf (outf, "<  ");
        break;
      case DDIR_EQ:
        fprintf (outf, "=  ");
        break;
      case DDIR_LE:
        fprintf (outf, "<= ");
        break;
      case DDIR_GT:
        fprintf (outf, ">  ");
        break;
      case DDIR_NE:
        fprintf (outf, "<> ");
        break;
      case DDIR_GE:
        fprintf (outf, ">= ");
        break;
      case DDIR_ALL: 
        fprintf (outf, "*  ");
        break;
      default:
        P_punt ("DumpDepInfo: unknown dir");
        break;
      }
  fprintf (outf, "]\n");
  
  fprintf (outf, "known = [ ");
  for (i = 0 ; i < Get_DepInfo_depth (dep); i++)
    fprintf (outf, "%d ", Get_DepInfo_known_i(dep, i));
  fprintf (outf, "]\n");

  fprintf (outf, "dist = [ ");
  for (i = 0 ; i < Get_DepInfo_depth (dep); i++)
    if (Get_DepInfo_known_i(dep, i))
      fprintf (outf, "%3d ", Get_DepInfo_dist_i(dep, i));
    else
      fprintf (outf, "-   ");
  fprintf (outf, "]\n");
}


static void
PrintExprDepInfo (Expr expr, void *data)
{
  P_PrintDepInfoCtrl ctrl;
  P_DepList dep_list;
  P_DepInfo dep;
  List list;

  ctrl = data;
  dep_list = P_GetExprExtL (expr, ctrl->expr_ext_idx);
  if (Get_DepList_deps (dep_list) == NULL)
    return;
  fprintf (ctrl->outf, "\nExpr (%d) :\n{\n", P_GetExprID (expr));
  list = Get_DepList_deps (dep_list);
  List_start (list);
  while ((dep = List_next (list)))
    {
      fprintf (ctrl->outf, "(\n");
      DumpDepInfo (ctrl->outf, dep);
      fprintf (ctrl->outf, ")\n");
    }
  fprintf (ctrl->outf, "}\n");
}
