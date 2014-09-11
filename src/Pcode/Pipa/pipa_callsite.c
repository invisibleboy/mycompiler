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
 *      File:    pipa_callsite.c
 *      Author:  Erik Nystrom
 *      Copyright (c) 2002  Erik Nystrom, Wen-mei Hwu
 *              and The Board of Trustees of the University of Illinois.
 *              All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#include "pipa_callsite.h"
#include "pipa_program.h"
#include <Pcode/ss_ssa2.h>


/**************************************************************************
 * Function parameter / return information
 **************************************************************************/

IPA_interface_t *
IPA_interface_new ()
{
  IPA_interface_t *tmp;

  tmp = (IPA_interface_t *) calloc (sizeof (IPA_interface_t), 1);
  tmp->param_ids = DYNA_new (1, sizeof (int));
  tmp->ret_id = IPA_INTERFACE_NORET;
  tmp->version = UNDEF_SUBSCR;

  return tmp;
}

void
IPA_interface_free (IPA_interface_t * iface)
{
  DYNA_free (iface->param_ids);
  free (iface);
}

IPA_interface_t *
IPA_interface_copy (IPA_interface_t * iface)
{
  IPA_interface_t *new_iface;

  new_iface = IPA_interface_new ();
  new_iface->ret_id = iface->ret_id;
  DYNA_free (new_iface->param_ids);
  new_iface->param_ids = DYNA_copy (iface->param_ids);

  return new_iface;
}

void
IPA_interface_set_ret_id (IPA_interface_t * iface, int id, int version)
{
  if (id == 0)
    I_punt ("IPA_interface_set_ret_id: id == 0\n");
  if (iface->ret_id != 0 && iface->ret_id != id)
    I_punt ("IPA_interface_set_ret_id: ret_id already set\n");
  iface->ret_id = id;
  iface->version = version;
}

void
IPA_interface_append_param_id (IPA_interface_t * iface, int id)
{
  if (id == 0)
    I_punt ("IPA_interface_append_param_id: id == 0\n");
  DYNA_append_element (iface->param_ids, &id);
}

void
IPA_interface_set_param_id (IPA_interface_t * iface, int index, int id)
{
  DYNA_add_element (iface->param_ids, index, &id);
}

IPA_interface_t *
IPA_interface_copyconvert (HashTable id_htab, IPA_interface_t * iface)
{
  IPA_interface_t *new_iface;
  int i, bound;
  int id;

  new_iface = IPA_interface_new ();

  new_iface->ret_id = IPA_mapid (id_htab, iface->ret_id);

  bound = DYNA_get_indexbound (iface->param_ids);
  for (i = 0; i < bound; i++)
    {
      id = IPA_interface_get_param_id (iface, i);
      IPA_interface_append_param_id (new_iface, IPA_mapid (id_htab, id));
    }

  return new_iface;
}

int
IPA_interface_get_num_params (IPA_interface_t * iface)
{
  return DYNA_get_indexbound (iface->param_ids);
}

int
IPA_interface_get_ret_id (IPA_interface_t * iface)
{
  return iface->ret_id;
}

int
IPA_interface_get_param_id (IPA_interface_t * iface, int index)
{
  int *p;

  p = DYNA_get_element (iface->param_ids, index);

  return *p;
}

void
IPA_interface_writefile (FILE * file, IPA_interface_t * iface)
{
  int i, bound;
  int id;

  fprintf (file, "      ret-id    : ");
  id = IPA_interface_get_ret_id (iface);
  if (id != 0)
    fprintf (file, "%d ", id);
  fprintf (file, "\n");

  bound = DYNA_get_indexbound (iface->param_ids);
  fprintf (file, "      param-ids : ");
  for (i = 0; i < bound; i++)
    {
      id = IPA_interface_get_param_id (iface, i);
      fprintf (file, "%d ", id);
    }
  fprintf (file, "\n");
}

void
IPA_interface_readfile (FILE * file, IPA_interface_t * iface)
{
  int int_data;
  int num;

  IPA_find_file_strtoken (file, "ret-id");
  IPA_find_file_chartoken (file, ':');

  num = fscanf (file, "%d", &int_data);
  if (num == 1)
    IPA_interface_set_ret_id (iface, int_data, 1);
  if (num > 1)
    I_punt ("IPA_interface_readfile: expecting only one ret id");

  IPA_find_file_strtoken (file, "param-ids");
  IPA_find_file_chartoken (file, ':');
  while (1)
    {
      num = fscanf (file, "%d", &int_data);
      if (num != 1)
        break;
      IPA_interface_append_param_id (iface, int_data);
    }
}



/**************************************************************************
 * Function callsite information
 **************************************************************************/

IPA_callsite_t *
IPA_callsite_new ()
{
  IPA_callsite_t *tmp;

  tmp = (IPA_callsite_t *) calloc (sizeof (IPA_callsite_t), 1);
  tmp->iface = IPA_interface_new ();
  tmp->version_htab = HashTable_create(16);

  return tmp;
}

void
IPA_callsite_free (IPA_prog_info_t * info, IPA_callsite_t * cs)
{
  /* NOTE: This does not remove the callsite ptr from the fninfo's list
   */

  IPA_interface_free (cs->iface);
  HashTable_free(cs->version_htab);
  free (cs);
}

#if 0
IPA_callsite_t *
IPA_callsite_copyconvert (IPA_funcsymbol_info_t * fninfo,
                          HashTable id_htab, IPA_callsite_t * cs)
{
  IPA_callsite_t *new_cs;

  assert(0);
  new_cs = IPA_callsite_new ();

  /* These fields are duplicates of the source */
  new_cs->indirect = cs->indirect;
  if (cs->indirect)
    {
    }
  else
    {
      new_cs->callee.dir.name = cs->callee.dir.name;
    }

  new_cs->iface = IPA_interface_copyconvert (id_htab, cs->iface);

  /* These are not the same as the source */
  new_cs->cs_id = fninfo->max_cs_id++;

  return new_cs;
}
#endif

void
IPA_callsite_add (IPA_prog_info_t * info,
                  IPA_funcsymbol_info_t * fninfo, IPA_callsite_t * cs)
{
  assert (cs->cs_id == 0);
  cs->cs_id = info->max_cs_id++;

  fninfo->callsites = List_insert_last (fninfo->callsites, cs);
  cs->fninfo = fninfo;
}

IPA_callsite_t *
IPA_callsite_new_prog (IPA_prog_info_t * info,
                       IPA_funcsymbol_info_t * fninfo,
                       char *callee_name, Key callee_key)
{
  IPA_callsite_t *cs = NULL;

  /* Create a new call site
   */
  cs = IPA_callsite_new ();
  if (!fninfo)
    I_punt ("IPA_new_prog_callsite: function not found\n");

  if (callee_name == NULL)
    {
      cs->indirect = 1;
    }
  else
    {
      cs->indirect = 0;
      cs->callee.dir.name = C_findstr (callee_name);
      cs->callee.dir.key = callee_key;
    }

  IPA_callsite_add (info, fninfo, cs);

  return cs;
}

void
IPA_callsite_writefile (FILE * file, IPA_callsite_t * cs)
{
  fprintf (file, "    CALLEE %d %d : ", cs->cs_id, cs->indirect);

  if (cs->indirect)
    {
    }
  else
    {
      fprintf (file, "%s", cs->callee.dir.name);
    }

  fprintf (file, "\n");

  IPA_interface_writefile (file, cs->iface);
}

IPA_callsite_t *
IPA_callsite_readfile (FILE * file)
{
  IPA_callsite_t *cs;
  char buffer1[250];
  int int_data;

  buffer1[0] = 0;
  fscanf (file, "%s", buffer1);
  if (strcmp ("CALLEE", buffer1))
    return NULL;

  cs = IPA_callsite_new ();

  fscanf (file, "%d", &int_data);
  cs->cs_id = int_data;

  fscanf (file, "%d", &int_data);
  cs->indirect = int_data;

  IPA_find_file_chartoken (file, ':');

  if (cs->indirect)
    {
    }
  else
    {
      fscanf (file, "%s", buffer1);
      cs->callee.dir.name = C_findstr (buffer1);
    }


  cs->iface = IPA_interface_new ();
  IPA_interface_readfile (file, cs->iface);

  return cs;
}
