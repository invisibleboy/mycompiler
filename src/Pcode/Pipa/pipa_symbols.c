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
 *      File:    pipa_symbols.c
 *      Author:  Erik Nystrom
 *      Copyright (c) 2003  Erik Nystrom, Wen-mei Hwu
 *              and The Board of Trustees of the University of Illinois.
 *              All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#include "pipa_symbols.h"
#include "pipa_program.h"
#include "pipa_pcode2pointsto.h"
#include <Pcode/symtab.h>

HashTable __typeid2type;
int IPA_max_type_size = 0;
Key IPA_global_key = {0,0};

/**************************************************************************
 * Symbol information
 **************************************************************************/

IPA_symbol_info_t *
IPA_symbol_new ()
{
  IPA_symbol_info_t *tmp;

  tmp = (IPA_symbol_info_t *) calloc (sizeof (IPA_symbol_info_t), 1);

  return tmp;
}

void
IPA_symbol_free (IPA_symbol_info_t * sym)
{
  /* Remove from hash/sym tabs */
  if (sym->id == 0)
    I_punt ("IPA_free_symbol_info: free symbol with 0 id\n");
  free (sym);
}


/* This is used to create keys for intermediate
   nodes that do not explicitly exist in the Pcode */
Key
IPA_symbol_tmpvarkey()
{
  static int cnt = 1;
  Key key;

  key.file = IPA_TEMPVAR_FILEID;
  key.sym = cnt++;
  
  return key;
}

/* This is returns a fake type key that marks
   a pointer-sized type. This is used for the
   type of temporary vars */
Key
IPA_symbol_tmptypekey()
{
  Key key;

  key.file = IPA_TEMPTYPE_FILEID;
  key.sym = 1;
  
  return key;
}


#define SYM_GETKEY(k) (k.sym ^ (k.file << 16))
static Key cmp_key;
static int
IPA_symbol_compare(void * syminfo)
{
  return (P_MatchKey(cmp_key, ((IPA_symbol_info_t *)syminfo)->sym_key));
}

IPA_symbol_info_t *
IPA_symbol_add (IPA_prog_info_t * info,
		IPA_funcsymbol_info_t * fninfo,
                char *sym_name, Key sym_key,
		int kind, Key type_key)
{
  IPA_symbol_info_t *syminfo;
  int hash;
  int size;

  assert(sym_name);

  /* Prevent duplicate symbol */
  hash = SYM_GETKEY(sym_key);
  cmp_key = sym_key;
  syminfo = IPA_htab_find(info->symtab, hash, IPA_symbol_compare);
  if (syminfo)
    {
      syminfo->kind |= kind;
      return syminfo;
    }

#if 0
  printf("SYMBOL [%s:%d in %s] %X -- ",
	 sym_name, info->max_var_id+1, 
	 fninfo->func_name,
	 kind);
  if (PST_IsFunctionType(info->symboltable, type_key))
    printf("FUNC ");
  if (PST_IsPointerType(info->symboltable, type_key))
    printf("PTR ");
  printf("\n");
#endif

  assert(P_ValidKey(sym_key) || sym_key.file == IPA_TEMPVAR_FILEID);
  assert(P_ValidKey(type_key) || type_key.file == IPA_TEMPTYPE_FILEID);

  if (PST_IsFunctionType(info->symboltable, type_key))
    kind |= (IPA_VAR_KIND_GLOBAL | IPA_VAR_KIND_FUNC);

  /* Create new syminfo */
  syminfo = IPA_symbol_new ();
  syminfo->fninfo = fninfo;
  syminfo->its_fninfo = NULL;
  syminfo->id = info->max_var_id++;
  syminfo->kind = kind;
  syminfo->max_version = 1;
  syminfo->sym_key = sym_key;
  syminfo->type_key = type_key;
  syminfo->symbol_name = strdup(sym_name);

  IPA_htab_insert(info->symtab, syminfo, hash);
  HashTable_insert (info->id2sym_htab, syminfo->id, syminfo);

  size = IPA_Pcode_sizeof(info, type_key);
  if (size > IPA_max_type_size)
    IPA_max_type_size = size;


  while (PST_IsPointerType(info->symboltable, type_key) ||
	 PST_IsArrayType(info->symboltable, type_key))
    {
      type_key = PST_GetTypeType(info->symboltable, type_key);
    }
  size = IPA_Pcode_sizeof(info, type_key);
  if (size > IPA_max_type_size)
    IPA_max_type_size = size;  

  return syminfo;
}

IPA_symbol_info_t *
IPA_symbol_find (IPA_prog_info_t * info,
                 Key sym_key)
{
  IPA_symbol_info_t *syminfo;
  int hash;

  /* Prevent duplicate symbol */
  hash = SYM_GETKEY(sym_key);
  cmp_key = sym_key;
  syminfo = IPA_htab_find(info->symtab, hash, IPA_symbol_compare);

  return syminfo;
}

IPA_symbol_info_t *
IPA_symbol_find_by_id (IPA_prog_info_t * info, int id)
{
  IPA_symbol_info_t *syminfo;

  syminfo = (IPA_symbol_info_t *) HashTable_find_or_null (info->id2sym_htab,
                                                          id);

  return syminfo;
}


void
IPA_symbol_dump(FILE *file, IPA_prog_info_t * info)
{
  IPA_symbol_info_t *syminfo;
  IPA_HTAB_ITER iter;
  
  IPA_HTAB_START(iter, info->symtab);
  IPA_HTAB_LOOP(iter)
    {
      syminfo = IPA_HTAB_CUR(iter);

      fprintf(file,"%20.20s ", syminfo->symbol_name);
      fprintf(file,"id%-6d sz%6d:", syminfo->id, IPA_Pcode_sizeof_array(info, syminfo->type_key));
      fprintf(file,"%5s ", (IPA_FLAG_ISSET(syminfo->kind,IPA_VAR_KIND_GLOBAL)?"GBL":""));
      fprintf(file,"%5s ", (IPA_FLAG_ISSET(syminfo->kind,IPA_VAR_KIND_PARAM)?"PRM":""));
      fprintf(file,"%5s ", (IPA_FLAG_ISSET(syminfo->kind,IPA_VAR_KIND_RETURN)?"RET":""));
      fprintf(file,"%5s ", (IPA_FLAG_ISSET(syminfo->kind,IPA_VAR_KIND_HEAP)?"HP":""));
      fprintf(file,"%5s ", (IPA_FLAG_ISSET(syminfo->kind,IPA_VAR_KIND_FUNC)?"FN":""));
      fprintf(file,"%5s ", (IPA_FLAG_ISSET(syminfo->kind,IPA_VAR_KIND_TEMP)?"TMP":""));
      fprintf(file,"%5s ", (IPA_FLAG_ISSET(syminfo->kind,IPA_VAR_KIND_STACK)?"STK":""));
      fprintf(file,"%s\n", syminfo->fninfo->func_name);
    }
}


/**************************************************************************
 * Function and symbol information
 **************************************************************************/

IPA_funcsymbol_info_t *
IPA_funcsymbol_new ()
{
  IPA_funcsymbol_info_t *tmp;

  tmp = (IPA_funcsymbol_info_t *) calloc (sizeof (IPA_funcsymbol_info_t), 1);
  tmp->iface = IPA_interface_new ();

  return tmp;
}

void
IPA_funcsymbol_free (IPA_prog_info_t * info, IPA_funcsymbol_info_t * fninfo)
{
  IPA_callsite_t *cs;

  IPA_interface_free (fninfo->iface);
  /*List_reset(fninfo->tmp_var_ids); */

  List_start (fninfo->callsites);
  while ((cs = List_next (fninfo->callsites)))
    {
      IPA_callsite_free (info, cs);
    }
  List_reset (fninfo->callsites);

  /* Remove from info list */
  info->fninfos = List_remove (info->fninfos, fninfo);

  IPA_cg_cgraph_free (fninfo->consg);
  if (fninfo->lsum_consg != fninfo->consg)
    IPA_cg_cgraph_free (fninfo->lsum_consg);

  {
    IPA_summary_info_t     *sum_info;
    IPA_summary_info_t     *nxt_info;
    for (sum_info = fninfo->sum_info; sum_info;
	 sum_info = nxt_info)
    {
      nxt_info = sum_info->nxt;
      List_reset(sum_info->sum_nodes);
      free(sum_info);
    }
    fninfo->sum_info = NULL;
  }

  free (fninfo);
}

IPA_funcsymbol_info_t *
IPA_funcsymbol_add (IPA_prog_info_t * info, 
		    Key func_key,
		    char *file_name, 
		    char *func_name)
{
  IPA_funcsymbol_info_t *fninfo;
  IPA_symbol_info_t *syminfo = NULL;

  if (P_MatchKey(func_key, IPA_global_key))
    {
      assert(info->globals == NULL);
    }
  else
    {
      syminfo = IPA_symbol_find(info, func_key);
      if (!syminfo)
	I_punt("Symbol for func [%s] does not exist\n",func_name);
      
      if (syminfo->its_fninfo)
	I_punt("Func info for [%s] already exists\n",func_name);
    }

  fninfo = IPA_funcsymbol_new ();
  fninfo->func_key = func_key;
  fninfo->file_name = strdup(file_name);
  fninfo->func_name = strdup(func_name);

  info->fninfos = List_insert_last (info->fninfos, fninfo);

  if (syminfo)
    syminfo->its_fninfo = fninfo;
  else
    info->globals = fninfo;

  return fninfo;
}

IPA_funcsymbol_info_t *
IPA_funcsymbol_find (IPA_prog_info_t * info, Key func_key)
{
  IPA_symbol_info_t *syminfo;

  if (P_MatchKey(func_key, IPA_global_key))
    {
      return info->globals;
    }

  /* Nothing known about this key
   */
  syminfo = IPA_symbol_find(info, func_key);
  if (!syminfo)
    return NULL;

  /* Function symbol exists but no actual function
   */
  if (!syminfo->its_fninfo)
    return NULL;
  
  return syminfo->its_fninfo;
}

void
IPA_funcsymbol_set_ret_id (IPA_prog_info_t * info, Key func_key, int id)
{
  IPA_funcsymbol_info_t *fninfo;

  fninfo = IPA_funcsymbol_find (info, func_key);
  if (!fninfo)
    I_punt ("IPA_append_func_ret_id: func not found\n");

  IPA_interface_set_ret_id (fninfo->iface, id, 1);
}

void
IPA_funcsymbol_append_param_id (IPA_prog_info_t * info,
                                Key func_key, int id)
{
  IPA_funcsymbol_info_t *fninfo;

  fninfo = IPA_funcsymbol_find (info, func_key);
  if (!fninfo)
    I_punt ("IPA_append_func_param_id: func not found\n");

  IPA_interface_append_param_id (fninfo->iface, id);
}


/* This routine is for use by the call graph builder when used
   external to the points-to analysis. */

IPA_funcsymbol_info_t *
IPA_funcsymbol_add_ext (Key func_key,
			char *func_name)
{
  IPA_funcsymbol_info_t *fninfo;

  fninfo = IPA_funcsymbol_new ();
  fninfo->func_key = func_key;
  fninfo->func_name = strdup(func_name);

  return fninfo;
}

