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
 *      File:   s_mem_profile.h
 *      Author: Daniel A. Connors
 *      Creation Date:  1997
 *      Copyright (c) 1997 Daniel A. Connors, Wen-mei Hwu and
 *                         The Board of Trustees of the University of Illinois.
 *                         All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#ifndef _LSIM_S_MEM_PROFILE_H_
#define _LSIM_S_MEM_PROFILE_H_

#include <config.h>

extern char *S_mem_dep_guide_file_name;
extern char *S_mem_dep_profile_file_name;
extern int  S_mem_dep_model;
extern char *S_mem_dep_model_name;
extern char *S_mem_dep_guide_model_name;
extern int  S_mem_dep_guide_model;

enum {
  MEM_DEP_MODEL_NONE=0,
  MEM_DEP_MODEL_UNGUIDED_PDC_COLLECT,
  MEM_DEP_MODEL_GUIDED_PDC_COLLECT
};

enum {
  MEM_DEP_GUIDE_MODEL_NONE=0,
  MEM_DEP_GUIDE_MODEL_LOCAL,
  MEM_DEP_GUIDE_MODEL_SYNC,
};

typedef struct S_Guide_Info
{
  int    index;
  int    *conflict;
  int 	  call_conflict;
  int    *guide_id;
  int    *guide_pc;
} S_Guide_Info;

typedef struct S_Guide_Table
{
  int    *mem_tab;
  int    num_guide_loads;
} S_Guide_Table;

typedef struct S_Pdc_MemDep_Data_Info
{
  int store_pc;
  int func_no;
} S_Pdc_MemDep_Data_Info;

extern S_Pdc_MemDep_Data_Info *S_get_pdc_data_info_for_address(int address);
extern void S_read_guide_file ();
extern void S_print_pdc_profile ();

#endif
