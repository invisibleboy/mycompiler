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
 *      File:   mdes2.c
 *      Author: John C. Gyllenhaal, Wen-mei Hwu
 *      Creation Date:  April 1996
\*****************************************************************************/


/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <stdio.h>
#include <strings.h>
#include <Lcode/l_main.h>
#include <library/l_alloc_new.h>
#include <library/i_error.h>
#include "lmdes.h"
#include "mdes2.h"

static L_Alloc_Pool *Mdes2_pool = NULL;



Mdes2 *
load_mdes2 (char *file_name)
{
  Mdes2 *mdes2;
  FILE *in;
  int name_len;

  /* For now, make sure ends with .lmdes2 */
  name_len = strlen (file_name);
  if ((name_len < 7) ||
      (file_name[name_len - 7] != '.') ||
      (file_name[name_len - 6] != 'l') ||
      (file_name[name_len - 5] != 'm') ||
      (file_name[name_len - 4] != 'd') ||
      (file_name[name_len - 3] != 'e') ||
      (file_name[name_len - 2] != 's') || (file_name[name_len - 1] != '2'))
    {
      I_punt ("load_mdes2: '%s' must currently end with .lmdes2", file_name);
    }

  /* Open the file for reading */
  if ((in = fopen (file_name, "r")) == NULL)
    {
      I_punt ("load_mdes2: Unable to open mdes2 file '%s' for reading.",
	      file_name);
    }

  /* Initialize mdes2 alloc pool if necessary */
  if (Mdes2_pool == NULL)
    Mdes2_pool = L_create_alloc_pool ("Mdes2", sizeof (Mdes2), 1);

  /* Allocate the mdes2 structure */
  mdes2 = (Mdes2 *) L_alloc (Mdes2_pool);

  /* Get a copy of the file name */
  mdes2->file_name = strdup (file_name);

  /* Read the md version of the machine description */
  mdes2->md_mdes = MD_read_md (in, file_name);
  fclose (in);

  mdes2->version1_mdes = NULL;

  /* Create the schedule manager version of the mdes */
  mdes2->sm_mdes = SM_build_mdes (mdes2);

  /* Return the newly created mdes2 structure */
  return (mdes2);
}
