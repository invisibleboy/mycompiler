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


/****************************************************************************\
 *      File:    pipa_program.c
 *      Author:  Erik Nystrom
 *      Copyright (c) 2003  Erik Nystrom, Wen-mei Hwu
 *              and The Board of Trustees of the University of Illinois.
 *              All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#include "pipa_program.h"
#include "pipa_callgraph.h"

/**************************************************************************
 * General points-to information
 **************************************************************************/

IPA_prog_info_t *
IPA_prog_new ()
{
  IPA_prog_info_t *tmp;

  tmp = (IPA_prog_info_t *) calloc (sizeof (IPA_prog_info_t), 1);

  tmp->fnname2info_stab = STRING_new_symbol_table ("SymbolTable", 100);
  tmp->id2sym_htab = HashTable_create (100);
  tmp->symtab = IPA_htab_new(1);
  tmp->typename2type = STRING_new_symbol_table ("TypeTable", 100);
  tmp->typeid2type = HashTable_create (100);

  tmp->max_var_id = 1;
  tmp->max_type_id = 1;
  tmp->max_cs_id = 1;

  tmp->errfile = fopen("PIPAWARN","w");

  return tmp;
}

void
IPA_prog_free (IPA_prog_info_t * info)
{
  IPA_funcsymbol_info_t *fninfo;
  IPA_HTAB_ITER iter;
  IPA_symbol_info_t *syminfo;

  List_start (info->fninfos);
  while ((fninfo = List_next (info->fninfos)))
    {
      IPA_funcsymbol_free (info, fninfo);
    }
  List_reset (info->fninfos);

  IPA_HTAB_START(iter, info->symtab);
  IPA_HTAB_LOOP(iter)
    {
      syminfo = IPA_HTAB_CUR(iter);
      IPA_symbol_free(syminfo);
    }
  IPA_htab_free(info->symtab);

  HashTable_free (info->id2sym_htab);  
  STRING_delete_symbol_table (info->fnname2info_stab, NULL);
  HashTable_free (info->typeid2type);
  STRING_delete_symbol_table (info->typename2type, NULL);

  IPA_callg_free (info->call_graph);

  free (info);
}

