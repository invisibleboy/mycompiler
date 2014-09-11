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
/*=========================================================================== 
 *	File :	mi_hppa.c 
 *	Desc :	Machine dependent specification.  
 *	Date :	Feb, 1992
 *	Auth :  Richard Hank, Wen-mei Hwu
 *
 *    Modified : modified from m_spec.c code by Pohua Paul Chang.
 *
 *==========================================================================*/

/*****************************************************************************\
 * NOTICE OF CONVENTION                                                      *
 * ------------------------------------------------------------------------- *
 * Mspec links to Pcode, Hcode, and Lcode modules.  In order to allow this   *
 * to take place without requiring front-end modules to link to liblcode.a,  *
 * Mspec code is divided into two classes as follows:                        *
 *  - mi_*.c must not depend on linkage to liblcode.a                        *
 *  - ml_*.c may depend on linkage to liblcode.a                             *
\*****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <stdio.h>
#ifdef M_HPPA_FOR_HCODE
#include <Hcode/h_ccode.h>
#endif
#include <Lcode/l_main.h>
#include "m_hppa.h"

/*--------------------------------------------------------------------------*/
#define M_HPPA_SIZE_VOID	0
#define M_HPPA_SIZE_BIT		1
#define M_HPPA_SIZE_CHAR	8
#define M_HPPA_SIZE_SHORT	16
#define M_HPPA_SIZE_INT		32
#define M_HPPA_SIZE_LONG	32
#define M_HPPA_SIZE_FLOAT	32
#define M_HPPA_SIZE_DOUBLE	64
#define M_HPPA_SIZE_POINTER	32
#define M_HPPA_SIZE_UNION	-1
#define M_HPPA_SIZE_STRUCT	-1
#define M_HPPA_SIZE_BLOCK	-1
#define M_HPPA_SIZE_MAX	 	64

#define M_HPPA_ALIGN_VOID	-1
#define M_HPPA_ALIGN_BIT	1
#define M_HPPA_ALIGN_CHAR	8
#define M_HPPA_ALIGN_SHORT	16
#define M_HPPA_ALIGN_INT	32
#define M_HPPA_ALIGN_LONG	32
#define M_HPPA_ALIGN_FLOAT	32
#define M_HPPA_ALIGN_DOUBLE	64
#define M_HPPA_ALIGN_POINTER	32
#define M_HPPA_ALIGN_UNION	-1	/* depends on the field */
#define M_HPPA_ALIGN_STRUCT	-1
#define M_HPPA_ALIGN_BLOCK	-1
#define M_HPPA_ALIGN_MAX	64
/*--------------------------------------------------------------------------*/
int
M_hppa_type_size (int mtype)
{
  switch (mtype)
    {
    case M_TYPE_VOID:
      return M_HPPA_SIZE_VOID;
    case M_TYPE_BIT_LONG:
      return M_HPPA_SIZE_BIT;
    case M_TYPE_BIT_SHORT:
      return M_HPPA_SIZE_BIT;
    case M_TYPE_BIT_CHAR:
      return M_HPPA_SIZE_BIT;
    case M_TYPE_CHAR:
      return M_HPPA_SIZE_CHAR;
    case M_TYPE_SHORT:
      return M_HPPA_SIZE_SHORT;
    case M_TYPE_INT:
      return M_HPPA_SIZE_INT;
    case M_TYPE_LONG:
      return M_HPPA_SIZE_LONG;
    case M_TYPE_FLOAT:
      return M_HPPA_SIZE_FLOAT;
    case M_TYPE_DOUBLE:
      return M_HPPA_SIZE_DOUBLE;
    case M_TYPE_POINTER:
      return M_HPPA_SIZE_POINTER;
    case M_TYPE_UNION:
      return M_HPPA_SIZE_UNION;
    case M_TYPE_STRUCT:
      return M_HPPA_SIZE_STRUCT;
    case M_TYPE_BLOCK:
      return M_HPPA_SIZE_BLOCK;
    default:
      return -1;
    }
}

int
M_hppa_type_align (int mtype)
{
  switch (mtype)
    {
    case M_TYPE_VOID:
      return M_HPPA_ALIGN_VOID;
    case M_TYPE_BIT_LONG:
      return M_HPPA_ALIGN_BIT;
    case M_TYPE_BIT_SHORT:
      return M_HPPA_ALIGN_BIT;
    case M_TYPE_BIT_CHAR:
      return M_HPPA_ALIGN_BIT;
    case M_TYPE_CHAR:
      return M_HPPA_ALIGN_CHAR;
    case M_TYPE_SHORT:
      return M_HPPA_ALIGN_SHORT;
    case M_TYPE_INT:
      return M_HPPA_ALIGN_INT;
    case M_TYPE_LONG:
      return M_HPPA_ALIGN_LONG;
    case M_TYPE_FLOAT:
      return M_HPPA_ALIGN_FLOAT;
    case M_TYPE_DOUBLE:
      return M_HPPA_ALIGN_DOUBLE;
    case M_TYPE_POINTER:
      return M_HPPA_ALIGN_POINTER;
    case M_TYPE_UNION:
      return M_HPPA_ALIGN_UNION;
    case M_TYPE_STRUCT:
      return M_HPPA_ALIGN_STRUCT;
    case M_TYPE_BLOCK:
      return M_HPPA_ALIGN_BLOCK;
    default:
      return -1;
    }
}

/*
 *	The following functions are for documentation only.
 *	The function overhead is too great for actual usage.
 */
void
M_hppa_char (M_Type type, int unsign)
{
  type->type = M_TYPE_CHAR;
  type->unsign = unsign;
  type->align = M_HPPA_ALIGN_CHAR;
  type->size = M_HPPA_SIZE_CHAR;
  type->nbits = M_HPPA_SIZE_CHAR;
}

void
M_hppa_short (M_Type type, int unsign)
{
  type->type = M_TYPE_SHORT;
  type->unsign = unsign;
  type->align = M_HPPA_ALIGN_SHORT;
  type->size = M_HPPA_SIZE_SHORT;
  type->nbits = M_HPPA_SIZE_SHORT;
}

void
M_hppa_int (M_Type type, int unsign)
{
  type->type = M_TYPE_INT;
  type->unsign = unsign;
  type->align = M_HPPA_ALIGN_INT;
  type->size = M_HPPA_SIZE_INT;
  type->nbits = M_HPPA_SIZE_INT;
}

void
M_hppa_long (M_Type type, int unsign)
{
  type->type = M_TYPE_LONG;
  type->unsign = unsign;
  type->align = M_HPPA_ALIGN_LONG;
  type->size = M_HPPA_SIZE_LONG;
  type->nbits = M_HPPA_SIZE_LONG;
}

int
M_hppa_layout_order (void)
{
  return M_BIG_ENDIAN;
}

void
M_set_model_hppa (char *model_name)
{
  if (!strcasecmp (model_name, "PA_1.0") ||
      !strcasecmp (model_name, "PA-1.0"))
    M_model = M_HP_PA_1_0;
  else if (!strcasecmp (model_name, "PA_1.1") ||
	   !strcasecmp (model_name, "PA-1.1"))
    M_model = M_HP_PA_1_1;
  else if (!strcasecmp (model_name, "PA_7100") ||
	   !strcasecmp (model_name, "PA-7100"))
    M_model = M_HP_PA_7100;
  else if (!strcasecmp (model_name, "PA_X") ||
	   !strcasecmp (model_name, "PA-X"))
    M_model = M_HP_PA_X;
  else if (!strcasecmp (model_name, "PLAYDOH_LCODE") ||
	   !strcasecmp (model_name, "PLAYDOH-LCODE"))
    M_model = M_HP_PLAYDOH_LCODE;
  else if (!strcasecmp (model_name, "PLAYDOH_MCODE") ||
	   !strcasecmp (model_name, "PLAYDOH-MCODE"))
    M_model = M_HP_PLAYDOH_MCODE;
  else
    M_assert (0, "M_set_model_hppa:  Illegal model specified!");
}
