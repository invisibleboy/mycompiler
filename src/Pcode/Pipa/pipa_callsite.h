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
 *      File:    pipa_callsite.h
 *      Author:  Erik Nystrom
 *      Copyright (c) 2003  Erik Nystrom, Wen-mei Hwu
 *              and The Board of Trustees of the University of Illinois.
 *              All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#ifndef _PIPA_CALLSITE_H_
#define _PIPA_CALLSITE_H_

#include "pipa_common.h"

/**************************************************************************
 * Function parameter / return information
 **************************************************************************/

#define IPA_INTERFACE_NORET  0

typedef struct IPA_interface_t
{
  int  ret_id;
  int  version;
  DYNA_t *param_ids;
}
IPA_interface_t;

IPA_interface_t *IPA_interface_new ();
void IPA_interface_free (IPA_interface_t * iface);
IPA_interface_t *IPA_interface_copy (IPA_interface_t * iface);

void IPA_interface_set_ret_id (IPA_interface_t * iface, int id, int version);
void IPA_interface_append_param_id (IPA_interface_t * iface, int id);
void IPA_interface_set_param_id (IPA_interface_t * iface, int index, int id);
IPA_interface_t *IPA_interface_copyconvert (HashTable id_htab,
                                            IPA_interface_t * iface);

int IPA_interface_get_num_params (IPA_interface_t * iface);
int IPA_interface_get_ret_id (IPA_interface_t * iface);
int IPA_interface_get_param_id (IPA_interface_t * iface, int index);

void IPA_interface_writefile (FILE * file, IPA_interface_t * iface);
void IPA_interface_readfile (FILE * file, IPA_interface_t * iface);


/**************************************************************************
 * Function callsite information
 **************************************************************************/

#define  IPA_CALLSITE_SYMNAME  "CALLEES_cid_%d"

typedef struct IPA_callsite_t
{
  /* An id unique to the caller */
  short cs_id;
  /* Indirect or direct */
  char indirect;

  /*  If direct:   name = name of callee
   *  If indirect: constraint graph node computing callees */
  union
  {
    int  cnode_id;
    struct {
      char *name;
      Key key;
    } dir;
  }
  callee;

  HashTable version_htab;  
  IPA_interface_t *iface;
  struct IPA_funcsymbol_info_t *fninfo;
  Expr call_expr;
}
IPA_callsite_t;

IPA_callsite_t *IPA_callsite_new ();
void IPA_callsite_free (struct IPA_prog_info_t *info, IPA_callsite_t * cs);

int IPA_callsite_subset (IPA_callsite_t * cs, IPA_callsite_t * cs_super);

void IPA_callsite_finish_comp (struct IPA_prog_info_t *info,
                               struct IPA_funcsymbol_info_t *fninfo,
                               IPA_callsite_t * cs);
void IPA_callsite_add (struct IPA_prog_info_t *info,
                       struct IPA_funcsymbol_info_t *fninfo,
                       IPA_callsite_t * cs);
IPA_callsite_t *IPA_callsite_new_prog (struct IPA_prog_info_t *info,
                                       struct IPA_funcsymbol_info_t *fninfo,
				       char *callee_name, Key callee_key);

void IPA_callsite_writefile (FILE * file, IPA_callsite_t * cs);
IPA_callsite_t *IPA_callsite_readfile (FILE * file);

#endif
